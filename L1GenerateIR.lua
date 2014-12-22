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
end