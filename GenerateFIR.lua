local Class = require "Class"

local function GetConstantIndex(state, constant)
	if not state.constantIndices then state.constantIndices = {} end
	local index = state.constantIndices[constant]
	if index then return index end
	if not state.nextConstantIndex then state.nextConstantIndex = 0 end
	index = state.nextConstantIndex
	state.nextConstantIndex = state.nextConstantIndex + 1
	state.constantIndices[constant] = index
	return index
end

local function GenerateIdentifier(state)
	if not state.nextIdentifier then state.nextIdentifier = 0 end
	local i = state.nextIdentifier
	state.nextIdentifier = state.nextIdentifier + 1
	return i
end

local function PushScopeAndMapNonuniqueIdentifierToUniqueIdentifier(state, nonunique, unique)
	if not state.bindings then state.bindings = {} end
	state.bindings[#state.bindings + 1] = {nonunique, unique}
end

local function FindUniqueIdentifierForNonuniqueIdentifier(state, nonunique)
	if not state.bindings then return nil end
	for i = #state.bindings, 1, -1 do
		local binding = state.bindings[i]
		if nonunique == binding[1] then return binding[2] end
	end
	return nil
end

local function PopScope(state)
	state.bindings[#state.bindings] = nil
end

local function GetUndefinedIdentifier(state)
	error("use the undefined op instead")
	--[[if not state.undefinedIdentifier then state.undefinedIdentifier = GenerateIdentifier(state) end
	return state.undefinedIdentifier]]
end

local function Generate(node, outputStatements, state)
	if node.type == "number" then
		outputStatements[#outputStatements + 1] = "loadinteger"
		local destinationIdentifier = GenerateIdentifier(state)
		outputStatements[#outputStatements + 1] = destinationIdentifier
		outputStatements[#outputStatements + 1] = node.data
		--GetConstantIndex(state, node.data)
		return destinationIdentifier
	elseif node.type == "string" then
		outputStatements[#outputStatements + 1] = "loadstring"
		local destinationIdentifier = GenerateIdentifier(state)
		outputStatements[#outputStatements + 1] = destinationIdentifier
		outputStatements[#outputStatements + 1] = node.data
		--GetConstantIndex(state, node.data)
		return destinationIdentifier
	elseif node.type == "identifier" then
		if node.data == "__undefined" then
			local identifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "loadundefined"
			outputStatements[#outputStatements + 1] = identifier
			return identifier
		elseif node.data == "__match" then
			local identifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "loadmatch"
			outputStatements[#outputStatements + 1] = identifier
			return identifier
		elseif node.data == "__true" then
			local identifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "loadtrue"
			outputStatements[#outputStatements + 1] = identifier
			return identifier
		elseif node.data == "__false" then
			local identifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "loadtrue"
			outputStatements[#outputStatements + 1] = identifier
			return identifier
		elseif node.data == "__multiply" then
			local identifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "loadmultiply"
			outputStatements[#outputStatements + 1] = identifier
			return identifier
		elseif node.data == "__add" then
			local identifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "loadadd"
			outputStatements[#outputStatements + 1] = identifier
			return identifier
		else
			local identifier = FindUniqueIdentifierForNonuniqueIdentifier(state, node.data)
			if identifier then return identifier end
			error("Could not find identifier: " .. tostring(node.data))
		end
	elseif node.type == "list" then
		local destinationIdentifier = GenerateIdentifier(state)
		outputStatements[#outputStatements + 1] = "emptylist"
		outputStatements[#outputStatements + 1] = destinationIdentifier
		
		for _, element in ipairs(node.elements) do
			if element.type == "sublist" then
				local elementIdentifier = Generate(element.expression, outputStatements, state)
				local resultIdentifier = GenerateIdentifier(state)
				outputStatements[#outputStatements + 1] = "mergelists"
				outputStatements[#outputStatements + 1] = resultIdentifier
				outputStatements[#outputStatements + 1] = elementIdentifier
				outputStatements[#outputStatements + 1] = destinationIdentifier
				destinationIdentifier = resultIdentifier
			else
				local elementIdentifier = Generate(element, outputStatements, state)
				local resultIdentifier = GenerateIdentifier(state)
				outputStatements[#outputStatements + 1] = "conslist"
				outputStatements[#outputStatements + 1] = resultIdentifier
				outputStatements[#outputStatements + 1] = elementIdentifier
				outputStatements[#outputStatements + 1] = destinationIdentifier
				destinationIdentifier = resultIdentifier
			end
		end
		return destinationIdentifier
		--[[local destinationIdentifier = GenerateIdentifier(state)
		outputStatements[#outputStatements + 1] = "closure"
		outputStatements[#outputStatements + 1] = destinationIdentifier
		outputStatements[#outputStatements + 1] = GenerateIdentifier(state)
		outputStatements[#outputStatements + 1] = GetUndefinedIdentifier(state)
		for _, element in ipairs(node.elements) do
			if element.type == "sublist" then
				
			else
				local elementIdentifier = Generate(element, outputStatements, state)
				local resultIdentifier = GenerateIdentifier(state)
				outputStatements[#outputStatements + 1] = "closure"
				outputStatements[#outputStatements + 1] = resultIdentifier
				outputStatements[#outputStatements + 1] = GenerateIdentifier(state)
				outputStatements[#outputStatements + 1] = GetUndefinedIdentifier(state)
				destinationIdentifier = resultIdentifier
			end
		end
		return destinationIdentifier
		error("Not yet implemented")]]
	elseif node.type == "call" then
		local calleeIdentifier = Generate(node.callee, outputStatements, state)
		local destinationIdentifier
		for _, argument in ipairs(node.arguments) do
			local argumentIdentifier = Generate(argument, outputStatements, state)
			destinationIdentifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "call"
			outputStatements[#outputStatements + 1] = destinationIdentifier
			outputStatements[#outputStatements + 1] = calleeIdentifier
			outputStatements[#outputStatements + 1] = argumentIdentifier
			calleeIdentifier = destinationIdentifier
		end
		return calleeIdentifier
	elseif node.type == "anonymous function" then
		local endingIndex
	
		for _, argument in ipairs(node.arguments) do
			if argument.type ~= "identifier" then
				error("Invalid argument type: " .. argument.type)
			end
			local argumentIdentifier = GenerateIdentifier(state)
			PushScopeAndMapNonuniqueIdentifierToUniqueIdentifier(state, argument.data, sourceIdentifier)
			outputStatements[#outputStatements + 1] = "closure"
			outputStatements[#outputStatements + 1] = nil
			outputStatements[#outputStatements + 1] = argumentIdentifier
			outputStatements[#outputStatements + 1] = nil
			endingIndex = #outputStatements
		end
		
		local destinationIdentifier = GenerateIdentifier(state)
		local resultIdentifier = Generate(node.body, outputStatements, state)
		
		for i = #node.arguments, 1, -1 do
			endingIndex = endingIndex - 4
			outputStatements[endingIndex + 2] = destinationIdentifier
			outputStatements[endingIndex + 4] = resultIdentifier
			resultIdentifier = destinationIdentifier
			PopScope(state)
		end
		
		return destinationIdentifier
	elseif node.type == "function assignment" then
		--[[
			factorial x = multiply x (factorial (subtract x 1));
			factorial 0 = 1;
			factorial
			
			map f [x, ...xs] = [f x, ...map f xs];
			map _ [] = [];
			map
		]]
		
		local functionIdentifier
		error("functionIdentifier needs a value")
		local resultIdentifier = GenerateIdentifier(state)
		local finalResultIdentifier = resultIdentifier
		local argumentIdentifierStatementIndices = {}
		--[[
			closure 1 2 3
			multiply 3 2 2
			call 1 4
			
			closure 1 2 3
			closure 3 4 5
			multiply 5 2 4
		]]
		
		PushScopeAndMapNonuniqueIdentifierToUniqueIdentifier(state, node.functionName.data, functionIdentifier)
		
		local numberOfIdentifierArguments = 0
		
		for i = #node.arguments, 1, -1 do
			functionIdentifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = "closure"
			outputStatements[#outputStatements + 1] = functionIdentifier
			local argumentIdentifier = GenerateIdentifier(state)
			outputStatements[#outputStatements + 1] = argumentIdentifier
			local argumentNode = node.arguments[i]
			if argumentNode.type == "identifier" then
				outputStatements[#outputStatements + 1] = resultIdentifier
				resultIdentifier = functionIdentifier
				
				numberOfIdentifierArguments = numberOfIdentifierArguments + 1
				PushScopeAndMapNonuniqueIdentifierToUniqueIdentifier(state, argumentNode.data, argumentIdentifier)
			
			else
				error("Invalid argument")
			--[[if argumentNode.type == "guard" then
				outputStatements[#outputStatements + 1] = resultIdentifier
				local guardValueIdentifier = Generate(argumentNode.expression, outputStatements, state)
				local conditionIdentifier = GenerateIdentifier(state)
				
				local undefinedIdentifier = GenerateIdentifier(state)
				outputStatements[#outputStatements + 1] = "loadundefined"
				outputStatements[#outputStatements + 1] = undefinedIdentifier
				
				outputStatements[#outputStatements + 1] = "branch"
				outputStatements[#outputStatements + 1] = resultIdentifier
				outputStatements[#outputStatements + 1] = guardValueIdentifier
				outputStatements[#outputStatements + 1] = resultIdentifier
				outputStatements[#outputStatements + 1] = undefinedIdentifier
				
				resultIdentifier = functionIdentifier]]
			end
		end
		
		outputStatements[#outputStatements + 1] = "let"
		outputStatements[#outputStatements + 1] = finalResultIdentifier
		outputStatements[#outputStatements + 1] = Generate(node.body, outputStatements, state)
		
		local followingContextIdentifier = Generate(node.followingContext, outputStatements, state)
		
		for _ = 1, numberOfIdentifierArguments do
			PopScope(state)
		end
		
		PopScope(state)
		
		return followingContextIdentifier
	elseif node.type == "assignment" then
		if node.destination.type ~= "identifier" then error("Not yet implemented") end
		local sourceIdentifier = Generate(node.source, outputStatements, state)
		PushScopeAndMapNonuniqueIdentifierToUniqueIdentifier(state, node.destination.data, sourceIdentifier)
		local resultIdentifier = Generate(node.followingContext, outputStatements, state)
		PopScope(state)
		return resultIdentifier
	elseif node.type == "branch" then
		local conditionIdentifier = Generate(node.condition, outputStatements, state)
		local resultIfTrueIdentifier = Generate(node.resultIfTrue, outputStatements, state)
		local resultIfFalseIdentifier = Generate(node.resultIfFalse, outputStatements, state)
		local destinationIdentifier = GenerateIdentifier(state)
		outputStatements[#outputStatements + 1] = "branch"
		outputStatements[#outputStatements + 1] = destinationIdentifier
		outputStatements[#outputStatements + 1] = conditionIdentifier
		outputStatements[#outputStatements + 1] = resultIfTrueIdentifier
		outputStatements[#outputStatements + 1] = resultIfFalseIdentifier
		return destinationIdentifier
	end
end

return function(root)
	local outputStatements = {}
	local state = {}
	local destinationIdentifier = GenerateIdentifier(state)
	local resultIdentifier = Generate(root, outputStatements, state)
	outputStatements[#outputStatements + 1] = "let"
	outputStatements[#outputStatements + 1] = destinationIdentifier
	outputStatements[#outputStatements + 1] = resultIdentifier
	return outputStatements
end
