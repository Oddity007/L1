--[[
closure result length
call result function arguments[]
number result digits[]
string result characters[]
branch result condition a b 
]]

local function ProcessNodeForConstants(node, state)
	if node.type == "number" or node.type == "string" then
		print(node.type, node.data)
		local index = state.constantMapping[node.data]
		if not index then
			index = #state.constants + 1
			state.constants[index] = node.data
			state.constantMapping[node.data] = index
		end
	elseif node.type == "anonymous function" then
		for _, v in ipairs(node.arguments) do
			ProcessNodeForConstants(v, state)
		end
		ProcessNodeForConstants(node.body, state)
	elseif node.type == "call" then
		ProcessNodeForConstants(node.callee, state)
		for _, v in ipairs(node.arguments) do
			ProcessNodeForConstants(v, state)
		end
	elseif node.type == "assignment" then
		ProcessNodeForConstants(node.destination, state)
		ProcessNodeForConstants(node.source, state)
		ProcessNodeForConstants(node.context, state)
	elseif node.type == "branch" then
		ProcessNodeForConstants(node.condition, state)
		ProcessNodeForConstants(node.resultIfTrue, state)
		ProcessNodeForConstants(node.resultIfFalse, state)
	end
end

return function(astRootNode)
	local state = {instructions = {}, constants = {}, constantMapping = {}}
	ProcessNodeForConstants(astRootNode, state)
	for _, v in pairs(state.constants) do
		print(v)
	end
end
