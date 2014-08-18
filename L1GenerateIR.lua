local function hexPairToInt(h, l)
	local t = {A=10, B=11, C=12, D=13, E=14, F=15}
	for i = 0, 9 do
		t[tostring(i)] = i
	end
	return t[h] * 16 + t[l]
end

local function hexToString(h)
	local t = {}
	for i = #h - 1, 1, -2 do
		local ch = h:sub(i, i)
		local cl = h:sub(i + 1, i + 1)
		assert(ch)
		assert(cl)
		t[#t + 1] = string.char(hexPairToInt(ch, cl))
	end
	return table.concat(t, "")
end

local function hexToInt(h)
	--return tonumber(hexToString(h))
	assert(h)
	print(h)
	local t = {A=10, B=11, C=12, D=13, E=14, F=15}
	for i = 0, 9 do
		t[tostring(i)] = i
	end
	local v = 0
	for i = 1, #h do
		local c = h:sub(i, i)
		print(c, type(c))
		assert(t["0"])
		assert(t[c])
		v = v * 16 + t[c]
	end
	return v
end

local function intToHex(i)
	return string.format("%x", i)
end

local function OutputStatement(state, type, operand1, operand2, operand3)
	if not state.statements then state.statements = {} end
	for i, statement in ipairs(state.statements) do
		if statement and statement[1] == type and statement[2] == operand1 and statement[3] == operand2 and statement[4] == operand3 then
			return i
		end
	end
	state.statements[#state.statements + 1] = {type, operand1, operand2, operand3}
	return #state.statements
end

local function LookupIdentifier(state, name)
	assert(name)
	if hexToString(name) == "__universe" then
		error("__universe must precede a natural constant")
		--return OutputStatement(state, "universefn")
	elseif hexToString(name) == "__adt" then
		return OutputStatement(state, "newadt")
	end
	local bindings = state.bindings
	if not bindings then error("undefined identifier " .. hexToString(name)) end
	local argumentIndex = 0
	for i = #bindings, 1, -1 do
		local binding = bindings[i]
		if binding.name == name then
			if binding.identifierType == "argument" then
				return OutputStatement(state, "argument", intToHex(argumentIndex)), binding.identifierType
			else
				return binding.irnode, binding.identifierType
			end
		end
		if binding.identifierType == "argument" then argumentIndex = argumentIndex + 1 end
	end
	return nil, nil
	--error("Undefined identifier")
end

local function PushBinding(state, name, irnode)
	local bindings = state.bindings or {}
	bindings[#bindings + 1] = {name = name, irnode = irnode, identifierType = "binding"}
	state.bindings = bindings
end

local function PopBinding(state)
	assert(state.bindings)
	assert(#state.bindings > 0)
	assert(state.bindings[#state.bindings].identifierType == "binding")
	state.bindings[#state.bindings] = nil
end

local function PushArgument(state, name)
	local bindings = state.bindings or {}
	bindings[#bindings + 1] = {name = name, identifierType = "argument"}
	state.bindings = bindings
end

local function PopArgument(state)
	assert(state.bindings)
	assert(#state.bindings > 0)
	assert(state.bindings[#state.bindings].identifierType == "argument")
	state.bindings[#state.bindings] = nil
end

local function PushADT(state, name, irnode)
	local bindings = state.bindings or {}
	bindings[#bindings + 1] = {name = name, irnode = irnode, identifierType = "adt"}
	state.bindings = bindings
end

local function PopADT(state)
	assert(state.bindings)
	assert(#state.bindings > 0)
	assert(state.bindings[#state.bindings].identifierType == "adt")
	state.bindings[#state.bindings] = nil
end

local function IsUniverseCall(state, astnode)
	return astnode._type == "Call" and astnode.callee._type == "Identifier" and hexToString(astnode.callee.data) == "__universe" and astnode.argument._type == "Natural"
end

local function IsADTName(state, name)
	assert(name)
	local irnode, identifierType = LookupIdentifier(state, name)
	return identifierType == "adt"
end

local function IsADTInvocation(state, astnode)
	if astnode._type == "Identifier" then
		return IsADTName(state, astnode.data)
	elseif astnode._type == "Call" then
		return IsADTInvocation(state, astnode.callee)
	else
		return false
	end
end

local function ExtractDestinationRootCallee(state, astnode, arguments)
	if astnode._type == "Identifier" then
		return astnode
	elseif astnode._type == "Call" then
		assert(astnode.callee._type == "Identifier" or astnode.callee._type == "Call")
		arguments[#arguments + 1] = astnode.argument
		return ExtractDestinationRootCallee(state, astnode.callee, arguments)
	end
	error("NYI")
end

local function Generate(state, astnode)
	if astnode._type == "Identifier" then
		if hexToString(astnode.data) == "__universe" then
			error("__universe must precede a natural constant")
		end
		local irnode = LookupIdentifier(state, astnode.data)
		if not irnode then error("Undefined identifier") end
		return irnode
	elseif astnode._type == "String" then
		return OutputStatement(state, "string", astnode.data)
	elseif astnode._type == "Natural" then
		return OutputStatement(state, "natural", astnode.data)
	elseif astnode._type == "Assign" then
		--[[local arguments = {}
		local destination = ExtractDestinationRootCallee(state, astnode.destination, arguments)
		local source = astnode.source
		for i = #arguments, 1, -1 do
			source = {_type = "Lambda", argument = arguments[i], result = source}
		end
		local sourceirnode = Generate(state, source)]]
		
		local destination = astnode.destination
		assert(destination._type == "Identifier")
		local sourceirnode = Generate(state, astnode.source)
		
		PushBinding(state, astnode.destination.data, sourceirnode)
		local followingirnode = Generate(state, astnode.followingContext)
		PopBinding(state)
		return followingirnode
	elseif astnode._type == "Define" then
		--[[local arguments = {}
		local destination = ExtractDestinationRootCallee(state, astnode.destination, arguments)
		local source = astnode.source
		for i = #arguments, 1, -1 do
			source = {_type = "Pi", argument = arguments[i], result = source}
		end
		local sourceirnode = Generate(state, source)]]
		
		local destination = astnode.destination
		assert(destination._type == "Identifier")
		local sourceirnode = Generate(state, astnode.source)
		
		local constructorirnode = OutputStatement(state, "constructor_of", sourceirnode, intToHex(state.constructorTag))
		state.constructorTag = state.constructorTag + 1
		PushBinding(state, destination.data, constructorirnode)
		local followingirnode = Generate(state, astnode.followingContext)
		PopBinding(state)
		return OutputStatement(state, "assert_constructor_validity", constructorirnode, followingirnode)
		--[[local sourceirnode = Generate(state, astnode.source)
		if IsUniverseCall(state, astnode.source) then
			assert(astnode.destination._type == "Identifier")
			local adtirnode = OutputStatement(state, "adt", sourceirnode, intToHex(state.adtTag))
			state.adtTag = state.adtTag + 1
			PushADT(state, astnode.destination.data, adtirnode)
			local followingirnode = Generate(state, astnode.followingContext)
			PopADT(state)
			return followingirnode
		elseif IsADTInvocation(state, astnode.source) then
			error("NYI")
		else
			error("Constructors definitions as values is not yet implemented.")
		end]]
	elseif astnode._type == "Annotate" then
		local valueirnode = Generate(state, astnode.value)
		local typeirnode = Generate(state, astnode.type)
		return OutputStatement(state, "annotate", valueirnode, typeirnode)
	elseif astnode._type == "Lambda" then
		assert(astnode.argument._type == "Annotate")
		assert(astnode.argument.value._type == "Identifier")
		local typeirnode = Generate(state, astnode.argument.type)
		PushArgument(state, astnode.argument.value.data)
		local resultirnode = Generate(state, astnode.result)
		PopArgument(state)
		return OutputStatement(state, "lambda", typeirnode, resultirnode)
	elseif astnode._type == "Pi" then
		assert(astnode.argument._type == "Annotate")
		assert(astnode.argument.value._type == "Identifier")
		local typeirnode = Generate(state, astnode.argument.type)
		PushArgument(state, astnode.argument.value.data)
		local resultirnode = Generate(state, astnode.result)
		PopArgument(state)
		return OutputStatement(state, "pi", typeirnode, resultirnode)
	elseif astnode._type == "Call" then
		if IsUniverseCall(state, astnode) then
			return OutputStatement(state, "universe", astnode.argument.data)
		end
		local calleeirnode = Generate(state, astnode.callee)
		local argumentirnode = Generate(state, astnode.argument)
		return OutputStatement(state, "call", calleeirnode, argumentirnode)
	elseif astnode._type == "Overload" then
		error("NYI")
	elseif astnode._type == "Declare" then
		error("NYI")
		assert(astnode.destination._type == "Identifier")
		PushArgument(state, astnode.destination.data)
		local followingirnode = Generate(state, astnode.followingContext)
		PopArgument(state)
		return followingirnode
	else
		error("NYI")
	end
end

local function AreSame(state, irnode1, irnode2)
	local statement1 = state.statements[irnode1]
	local statement2 = state.statements[irnode2]
	if statement1[1] == "pi" and statement2[1] == "pi" then
		return AreSame(state, statement1[2], statement2[2]) and AreSame(state, statement1[3], statement2[3])
	elseif statement1[1] == "lambda" and statement2[1] == "lambda" then
		return AreSame(state, statement1[2], statement2[2]) and AreSame(state, statement1[3], statement2[3])
	elseif statement1[1] == "call" and statement2[1] == "call" then
		return AreSame(state, statement1[2], statement2[2]) and AreSame(state, statement1[3], statement2[3])
	elseif statement1[1] == "annotate" and statement2[1] == "annotate" then
		return AreSame(state, statement1[2], statement2[2]) and AreSame(state, statement1[3], statement2[3])
	elseif statement1[1] == "assert_constructor_validity" and statement2[1] == "assert_constructor_validity" then
		return AreSame(state, statement1[2], statement2[2]) and AreSame(state, statement1[3], statement2[3])
	elseif statement1[1] == "constructor_of" and statement2[1] == "constructor_of" then
		if #statement1[4] ~= #statement2[4] then return false end
		for i, statement in ipairs(statement1[4]) do
			if not AreSame(state, statement1[4][i], statement2[4][i]) then
				return false
			end
		end
		return AreSame(state, statement1[2], statement2[2]) and statement1[3] == statement2[3]
	elseif statement1[1] == "universe" and statement2[1] == "universe" then
		return statement1[2] == statement2[2]
	elseif statement1[1] == "natural" and statement2[1] == "natural" then
		return statement1[2] == statement2[2]
	elseif statement1[1] == "string" and statement2[1] == "string" then
		return statement1[2] == statement2[2]
	elseif statement1[1] == "argument" and statement2[1] == "argument" then
		return statement1[2] == statement2[2]
	elseif statement1[1] == "newadt" and statement2[1] == "newadt" then
		return true
	elseif statement1[1] == "natural_type" and statement2[1] == "natural_type" then
		return true
	elseif statement1[1] == "string_type" and statement2[1] == "string_type" then
		return true
	elseif statement1[1] == "natural" and statement2[1] == "natural" then
		return statement1[2] == statement2[2]
	elseif statement1[1] == "string" and statement2[1] == "string" then
		return statement1[2] == statement2[2]
	else
		return false
	end
end

local function ReplaceArg(state, baseargindex, irnode, context)
	local statement = state.statements[irnode]
	if statement[1] == "argument" then
		local argindex = hexToInt(statement[2])
		if argindex < baseargindex then
			return context.arguments[argindex + 1]
		else
			return OutputStatement(state, "argument", intToHex(argindex - baseargindex))
		end
	elseif statement[1] == "pi" then
		local typeirnode = ReplaceArg(state, baseargindex, statement[2], context)
		local resultirnode = ReplaceArg(state, baseargindex + 1, statement[3], context)
		return OutputStatement(state, statement[1], typeirnode, resultirnode)
	elseif statement[1] == "lambda" then
		local typeirnode = ReplaceArg(state, baseargindex, statement[2], context)
		local resultirnode = ReplaceArg(state, baseargindex + 1, statement[3], context)
		return OutputStatement(state, statement[1], typeirnode, resultirnode)
	elseif statement[1] == "call" then
		local calleeirnode = ReplaceArg(state, baseargindex, statement[2], context)
		local argirnode = ReplaceArg(state, baseargindex, statement[3], context)
		return OutputStatement(state, statement[1], calleeirnode, argirnode)
	elseif statement[1] == "annotate" then
		local valueirnode = ReplaceArg(state, baseargindex, statement[2], context)
		local typeirnode = ReplaceArg(state, baseargindex, statement[3], context)
		return OutputStatement(state, statement[1], valueirnode, typeirnode)
	elseif statement[1] == "assert_constructor_validity" then
		local constructorirnode = ReplaceArg(state, baseargindex, statement[2], context)
		local followingirnode = ReplaceArg(state, baseargindex, statement[3], context)
		return OutputStatement(state, statement[1], constructorirnode, followingirnode)
	elseif statement[1] == "constructor_of" then
		local typeirnode = ReplaceArg(state, baseargindex, statement[2], context)
		local argirnodes = {}
		for i, argirnode in ipairs(statement[4]) do
			argirnodes[i] = ReplaceArg(state, baseargindex, argirnode, context)
		end
		return OutputStatement(state, statement[1], typeirnode, statement[3], argirnodes)
	elseif statement[1] == "newadt" then
		return irnode
	elseif statement[1] == "natural" then
		return irnode
	elseif statement[1] == "string" then
		return irnode
	elseif statement[1] == "natural_type" then
		return irnode
	elseif statement[1] == "string_type" then
		return irnode
	elseif statement[1] == "universe" then
		return irnode
	else
		error("NYI")
	end
end

local function TypeOf(state, irnode, context)
	local statement = state.statements[irnode]
	if statement[1] == "argument" then
		local argindex = hexToInt(statement[2])
		return context.argumentTypes[argindex + 1]
	elseif statement[1] == "pi" then
		local typeirnode = TypeOf(state, statement[2], context)
		local typeirnodestatement = state.statements[typeirnode]
		local resultirnode = TypeOf(state, statement[3], context)
		local resultirnodestatement = state.statements[resultirnode]
		if typeirnodestatement[1] == "universe" and resultirnodestatement[1] == "universe" then
			assert(typeirnodestatement[2])
			assert(resultirnodestatement[2])
			local universeLevel = 1 + math.max(hexToInt(typeirnodestatement[2]), hexToInt(resultirnodestatement[2]))
			assert(universeLevel)
			assert(intToHex)
			return OutputStatement(state, "universe", intToHex(universeLevel))
		end
		error("Invalid type")
	elseif statement[1] == "lambda" then
		context.argumentTypes[#context.argumentTypes + 1] = statement[2]
		local resultirnode = TypeOf(state, statement[3], context)
		return OutputStatement(state, "pi", statement[2], resultirnode)
	elseif statement[1] == "call" then
		local calleeirnode = ReplaceArg(state, baseargindex, statement[2], context)
		local argirnode = ReplaceArg(state, baseargindex, statement[3], context)
		return OutputStatement(state, statement[1], calleeirnode, argirnode)
	elseif statement[1] == "universe" then
		return OutputStatement(state, "universe", intToHex(1 + hexToInt(statement[2])))
	elseif statement[1] == "annotate" then
		return TypeOf(state, statement[2], context)
	elseif statement[1] == "assert_constructor_validity" then
		return TypeOf(state, statement[3], context)
	elseif statement[1] == "constructor_of" then
		error("NYI")
	elseif statement[1] == "newadt" then
		error("NYI")
	elseif statement[1] == "natural" then
		return OutputStatement(state, "natural_type")
	elseif statement[1] == "string" then
		return OutputStatement(state, "string_type")
	elseif statement[1] == "natural_type" then
		return OutputStatement(state, "universe", intToHex(0))
	elseif statement[1] == "string_type" then
		return OutputStatement(state, "universe", intToHex(0))
	else
		error("NYI")
	end
end

local function Evaluate(state, irnode)
	local statement = state.statements[irnode]
	if statement[1] == "string" then
		return irnode
	elseif statement[1] == "natural" then
		return irnode
	elseif statement[1] == "universe" then
		return irnode
	elseif statement[1] == "natural_type" then
		return irnode
	elseif statement[1] == "string_type" then
		return irnode
	elseif statement[1] == "newadt" then
		error("NYI")
	elseif statement[1] == "constructor_of" then
		error("NYI")
	elseif statement[1] == "lambda" then
		local typeirnode = Evaluate(state, statement[2])
		return OutputStatement(state, statement[1], typeirnode, statement[3])
	elseif statement[1] == "pi" then
		local typeirnode = Evaluate(state, statement[2])
		return OutputStatement(state, statement[1], typeirnode, statement[3])
	elseif statement[1] == "call" then
		local argumentirnode = Evaluate(state, statement[3])
		local calleeirnode = Evaluate(state, statement[2])
		local calleestatement = state.statements[calleeirnode]
		if calleestatement[1] == "lambda" then
			local typeirnode = Evaluate(state, calleestatement[2])
			if not AreSame(state, typeirnode, TypeOf(state, argumentirnode, {arguments = {}, argumentTypes = {}})) then
				error("Argument type mismatch")
			end
			local freestandingresultirnode = ReplaceArg(state, 0, calleestatement[3], {arguments = {argumentirnode}, argumentTypes = {typeirnode}})
			return Evaluate(state, freestandingresultirnode)
		else
			error("NYI")
		end
	elseif statement[1] == "annotate" then
		local valueirnode = Evaluate(state, statement[2])
		local typeirnode = Evaluate(state, statement[3])
		if not AreSame(state, typeirnode, TypeOf(state, valueirnode, {arguments = {}, argumentTypes = {}})) then
			error("Annotation type mismatch")
		end
		return valueirnode
		--OutputStatement(state, statement[1], valueirnode, typeirnode)
	elseif statement[1] == "assert_constructor_validity" then
		error("NYI")
	else
		error("NYI")
	end
end

local json = require "dkjson"

local filename = ...
--local file = io.open(filename, "r")

local ast = json.decode(io.read("*a"))
--local ast = json.decode(file:read("*all"))
--file:close()

local function tprint (tbl, indent)
  if type(tbl) ~= "table" then
    formatting = string.rep("  ", indent)
    print(formatting .. tostring(tbl))
    return
  end
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    else
      print(formatting .. tostring(v))
    end
  end
end

do
	local state = {constructorTag = 0, adtTag = 0}
	local rootID = Generate(state, ast)
	print("Generated: root " .. tostring(rootID))
	tprint(state)
	local evaluatedRootID = Evaluate(state, rootID)
	print("Evaluated: root " .. tostring(evaluatedRootID))
	tprint(state)
end