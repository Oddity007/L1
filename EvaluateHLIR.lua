require "dependencies/bigint"

local function Evaluate(node, variableChain)
	if node[1] == "lambda" then
		return node
	elseif node[1] == "call" then
		local callee = Evaluate(node[2], variableChain)
		local argument = Evaluate(node[3], variableChain)
		if callee[1] == "lambda" then
			local newVariableChain = {name = callee[2], value = argument, previous = variableChain}
			local result = Evaluate(callee[3], newVariableChain)
			return result
		else
			error("Unexpected callee type: " .. callee[1])
		end
	elseif node[1] == "get" then
		local currentVariableChain = variableChain
		local value = nil
		repeat
			if currentVariableChain.name == node[2] then
				value = currentVariableChain.value
			else
				currentVariableChain = currentVariableChain.previous
				if not currentVariableChain then
					error("Undefined variable named: " .. node[2])
				end
			end
		until value
		return value
	elseif node[1] == "boolean" then
		return node
	elseif node[1] == "integer" then
		return node
	elseif node[1] == "add" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		assert(operand2[1] == "integer")
		return {"integer", tostring(bigint(operand1[2]) + bigint(operand2[2]))}
	elseif node[1] == "subtract" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		assert(operand2[1] == "integer")
		return {"integer", tostring(bigint(operand1[2]) - bigint(operand2[2]))}
	elseif node[1] == "multiply" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		assert(operand2[1] == "integer")
		return {"integer", tostring(bigint(operand1[2]) * bigint(operand2[2]))}
	elseif node[1] == "divide" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		assert(operand2[1] == "integer")
		return {"integer", tostring(bigint(operand1[2]) / bigint(operand2[2]))}
	elseif node[1] == "modulo" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		assert(operand2[1] == "integer")
		local a = bigint(operand1[2])
		local b = bigint(operand2[2])
		--% doesn't work as expected
		return {"integer", tostring(a - (a / b) * b)}
	elseif node[1] == "negate" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		return {"integer", tostring(-bigint(operand1[2]))}
	elseif node[1] == "equal" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		if operand1[1] == "integer" and operand2[1] == "integer" then
			local areEqual = (bigint(operand1[2]) == bigint(operand2[2]))
			return {"boolean", areEqual and "true" or "false"}
		elseif operand1[1] == "boolean" and operand2[1] == "boolean" then
			local areEqual = (operand1[2] == operand2[2])
			return {"boolean", areEqual and "true" or "false"}
		end
	elseif node[1] == "lesser than" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		assert(operand2[1] == "integer")
		local result = (bigint(operand1[2]) < bigint(operand2[2]))
		return {"boolean", result and "true" or "false"}
	elseif node[1] == "lesser than or equal to" then
		local operand1 = Evaluate(node[2], variableChain)
		assert(operand1[1] == "integer")
		local operand2 = Evaluate(node[3], variableChain)
		assert(operand2[1] == "integer")
		local result = (bigint(operand1[2]) <= bigint(operand2[2]))
		return {"boolean", result and "true" or "false"}
	elseif node[1] == "select" then
		local condition = Evaluate(node[2], variableChain)
		assert(condition[1] == "boolean")
		if condition[2] == "true" then
			return Evaluate(node[3], variableChain)
		else
			return Evaluate(node[4], variableChain)
		end
	end
	
	error("Invalid node type: " .. node[1])
end

return function(root)
	return Evaluate(root, nil)
end
