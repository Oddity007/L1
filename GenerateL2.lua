local function AddStatement(state, statement)
	if not state.statements then state.statements = {} end
	state.statements[#state.statements + 1] = statement
end

local function GenerateUniqueIdentifier(state)
	local i = (state.nextIdentifier or 0) + 1
	state.nextIdentifier = i
	return i
end

local function GetUniqueIdentifierForFIRNode(state, node)
	if not state.identifierMappings then state.identifierMappings = {} end
	local identifier = state.identifierMappings[node]
	if identifier then return identifier end
	
	local selfIdentifier = GenerateUniqueIdentifier(state)
	
	if node[1] == "unique identifier" then
		--we don't need to do anything special
	elseif node[1] == "load_undefined" then
		AddStatement(state, {"load_undefined", selfIdentifier})
	elseif node[1] == "load_integer" then
		AddStatement(state, {"load_integer", selfIdentifier, node[2]})
	elseif node[1] == "load_string" then
		error("Not yet implemented")
	elseif node[1] == "load_multiply" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"multiply_integers", d, a, c})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_divide" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"divide_integers", d, a, c})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_add" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"add_integers", d, a, c})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_subtract" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"subtract_integers", d, a, c})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_greater" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"lesser_equal_integers", d, c, a})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_greater_equal" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"lesser_integers", d, c, a})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_lesser" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"lesser_integers", d, a, c})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_lesser_equal" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"lesser_equal_integers", d, a, c})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_equal" then
		local a = GenerateUniqueIdentifier(state)
		local b = GenerateUniqueIdentifier(state)
		local c = GenerateUniqueIdentifier(state)
		local d = GenerateUniqueIdentifier(state)
		AddStatement(state, {"equal_integers", d, a, c})
		AddStatement(state, {"closure", b, c, d})
		AddStatement(state, {"closure", selfIdentifier, a, b})
	elseif node[1] == "load_empty_list" then
		error("Not yet implemented")
	elseif node[1] == "prepend_sublist" then
		error("Not yet implemented")
	elseif node[1] == "cons" then
		--cons x xs = i => (i ? x; xs (__subtract i 1)); cos
		local head = GetUniqueIdentifierForFIRNode(state, node[2])
		local tail = GetUniqueIdentifierForFIRNode(state, node[3])
		local consedListArgumentIdentifier = GenerateUniqueIdentifier(state)
		local consedListResultIdentifier = GenerateUniqueIdentifier(state)
		local constantOneIdentifier = GenerateUniqueIdentifier(state)
		AddStatement(state, {"load_integer", constantOneIdentifier, "1"})
		local consedListArgumentMinusOneIdentifier = GenerateUniqueIdentifier(state)
		AddStatement(state, {"subtract_integers", consedListArgumentMinusOneIdentifier, consedListArgumentIdentifier, constantOneIdentifier})
		local resultIfArgumentIsNotZeroIdentifier = GenerateUniqueIdentifier(state)
		AddStatement(state, {"call", resultIfArgumentIsNotZeroIdentifier, tail, consedListArgumentMinusOneIdentifier})
		AddStatement(state, {"branch", consedListResultIdentifier, consedListArgumentIdentifier, head, resultIfArgumentIsNotZeroIdentifier})
		AddStatement(state, {"closure", selfIdentifier, consedListArgumentIdentifier, consedListResultIdentifier})
	elseif node[1] == "call" then
		AddStatement(state, {"call", selfIdentifier, GetUniqueIdentifierForFIRNode(state, node[2]), GetUniqueIdentifierForFIRNode(state, node[3])})
	elseif node[1] == "redirect" then
		AddStatement(state, {"let", selfIdentifier, GetUniqueIdentifierForFIRNode(state, node[2])})
	elseif node[1] == "closure" then
		AddStatement(state, {"closure", selfIdentifier, GetUniqueIdentifierForFIRNode(state, node[2]), GetUniqueIdentifierForFIRNode(state, node[3])})
	elseif node[1] == "branch" then
		AddStatement(state, {"branch", selfIdentifier, GetUniqueIdentifierForFIRNode(state, node[2]), GetUniqueIdentifierForFIRNode(state, node[3]), GetUniqueIdentifierForFIRNode(state, node[4])})
	else
		error("Unknown operation")
	end
	
	return selfIdentifier
end

return function(fir)
	local state = {}
	AddStatement(state, {"let", 0, GetUniqueIdentifierForFIRNode(state, fir)})
	return state.statements
end