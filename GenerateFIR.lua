local function PushBinding(state, identifier, value)
	state.bindings = state.bindings or {}
	state.bindings[#state.bindings + 1] = {identifier, value}
end

local function PopBinding(state)
	local bindings = state.bindings
	if not bindings then error("Stack underflow") end
	bindings[#state.bindings] = nil
end

local function GenerateUniqueIdentifier(state)
	state.lastUniqueIdentifierID = (state.lastUniqueIdentifierID or 0) + 1
	return {"unique identifier", state.lastUniqueIdentifierID}
end

local function FindBinding(state, identifier)
	local bindings = state.bindings
	if not bindings then return nil end
	for i = #bindings, 1, -1 do
		local binding = bindings[i]
		if binding[1] == identifier then
			return binding[2]
		end
	end
	return nil
end

local function Generate(node, state)
	if node.type == "number" then
		return {"loadinteger", node.data}
	elseif node.type == "string" then
		return {"loadstring", node.data}
	elseif node.type == "identifier" then
		if node.data == "__undefined" then
			return {"loadundefined"}
		elseif node.data == "__true" then
			return {"loadtrue"}
		elseif node.data == "__false" then
			return {"loadfalse"}
		elseif node.data == "__multiply" then
			return {"loadmultiply"}
		elseif node.data == "__add" then
			return {"loadadd"}
		elseif node.data == "__equal" then
			return {"loadequal"}
		elseif node.data == "__subtract" then
			return {"loadequal"}
		else
			local value = FindBinding(state, node.data)
			if value then return value end
			error("Use of undefined identifier: " .. tostring(node.data))
		end
	elseif node.type == "list" then
		local tail = {"loademptylist"}
		
		for i = #node.elements, 1, -1 do
			local element = node.elements[i]
			if element.type == "sublist" then
				tail = {"prependsublist", Generate(element, state), tail}
			else
				tail = {"cons", Generate(element, state), tail}
			end
		end
		return tail
	elseif node.type == "call" then
		--[[local callee = Generate(node.callee, state)
		local call = {"call", callee, #node.arguments}
		for _, argument in ipairs(node.arguments) do
			call[#call + 1] = Generate(argument, state)
		end
		return call]]
		--error("Not yet implemented")
		local call = Generate(node.callee, state)
		for i = #node.arguments, 1, -1 do
			call = {"call", call, Generate(node.arguments[i], state)}
		end
		return call
	elseif node.type == "anonymous function" then
		error("Not yet implemented")
	elseif node.type == "function assignment" then
		assert(node.functionName.type == "identifier")
		local undefined = {"loadundefined"}
		local lastDefinition = FindBinding(state, node.functionName.data)
		
		PushBinding(state, "__super", lastDefinition or undefined)
		
		local functionRedirection = {"redirect"}
		
		local argumentUniqueIdentifiers = {}
		for i = 1, #node.arguments do
			local argument = node.arguments[i]
			local argumentUniqueIdentifier = GenerateUniqueIdentifier(state)
			argumentUniqueIdentifiers[#argumentUniqueIdentifiers + 1] = argumentUniqueIdentifier
		end
		
		local numberOfBindings = 0
		
		for i, argument in ipairs(node.arguments) do
			local argumentUniqueIdentifier = argumentUniqueIdentifiers[i]
			if argument.type == "identifier" then
				PushBinding(state, argument.data, argumentUniqueIdentifier)
				numberOfBindings = numberOfBindings + 1
			end
		end
		
		PushBinding(state, "__self", functionRedirection)
		local body = Generate(node.body, state)
		PopBinding(state)
		
		for i = 1, numberOfBindings do
			PopBinding(state)
		end
		
		PopBinding(state)
		
		for i = #node.arguments, 1, -1 do
			local argument = node.arguments[i]
			local argumentUniqueIdentifier = argumentUniqueIdentifiers[i]
			if argument.type == "identifier" then
				body = {"closure", argumentUniqueIdentifier, body}
			elseif argument.type == "eval" then
				local equality = {"call", {"call", {"loadequal"}, Generate(argument.expression, state)}, argumentUniqueIdentifier}
				local lastDefinitionCallee = lastDefinition
				if lastDefinition then
					for j = 1, i do
						local argumentUniqueIdentifier = argumentUniqueIdentifiers[j]
						lastDefinitionCallee = {"call", lastDefinitionCallee, argumentUniqueIdentifier}
					end
				end
				
				local condition = {"branch", equality, body, lastDefinitionCallee}
				body = {"closure", argumentUniqueIdentifier, condition}
			else
				error("Invalid argument type")
			end
		end
		
		functionRedirection[2] = body
		
		return body
		--[[local rootFunctionValue = {"closure"}
		
		local lastDefinition = FindBinding(state, node.functionName.data) or {"loadundefined"}
		--PushBinding(state, node.functionName.data, rootFunctionValue)
		PushBinding(state, "__super", lastDefinition)
		PushBinding(state, "__self", rootFunctionValue)
		
		--local body, finalGuardBranch = nil, nil
		
		local arguments = {}
		
		local numberOfBindings = 0
		for i, argumentNode in ipairs(node.arguments) do
			if argumentNode.type ~= "guard" then
				local argument = GenerateUniqueIdentifier(state)
				arguments[i] = argument
				PushBinding(state, argumentNode.data, argument)
				numberOfBindings = numberOfBindings + 1
			end
		end
		
		for i, argumentNode in ipairs(node.arguments) do
			if argumentNode.type == "identifier" then
				local argument = arguments[i]
			elseif argumentNode.type == "guard" then
				lastDefinition = {"branch", Generate(argumentNode.expression, state), , lastDefinition}
				if not rootBody then rootBody = body end
			else
				error("Not yet implemented")
			end
		end
		
		local numberOfActualArguments = 0
		for i, argumentNode in ipairs(node.arguments) do
			if argumentNode.type == "identifier" then
				local argument = GenerateUniqueIdentifier(state)
				PushBinding(state, argumentNode.data, argument)
				--functionValue[#functionValue + 1] = argument
				functionBodyValue[nextFunctionBodyValueIndex] = {"closure", }functionBodyValue
				nextFunctionBodyValueIndex = 
				numberOfBindings = numberOfBindings + 1
				numberOfActualArguments = numberOfActualArguments + 1
				elseif argumentNode.type == "guard" then
				body = {"branch", Generate(argumentNode.expression, state), {"call", }, {"loadundefined"}}
				if not rootBody then rootBody = body end
			else
				error("Not yet implemented")
			end
		end
		
		--local body = Generate(node.body, state)
		
		local followingContext = Generate(node.followingContext, state)
		
		for i = 1, numberOfBindings do
			PopBinding(state)
		end
		
		PopBinding(state)
		PopBinding(state)
		--PopBinding(state)
		
		return followingContext
		error("Not yet implemented")]]
	elseif node.type == "assignment" then
		if node.destination.type ~= "identifier" then error("Not yet implemented") end
		local source = Generate(node.source, state)
		PushBinding(state, node.destination.data, source)
		local result = Generate(node.followingContext, state)
		PopBinding(state)
		return result
	elseif node.type == "branch" then
		return {"branch", Generate(node.condition, state), Generate(node.resultIfTrue, state), Generate(node.resultIfFalse, state)}
	end
end

return function(root)
	local state = {}
	return Generate(root, state)
end
