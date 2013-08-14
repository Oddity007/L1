local function reverse(list)
	if not list then return nil end
	local lengthPlusOne = #list + 1
	local newList = {}
	for i, v in ipairs(list) do
		newList[lengthPlusOne - i] = v
	end
	return newList
end

local function NewFunctionAssignmentNode(name, arguments, body, followingContext)
	return {
		type = "function assignment", 
		functionName = name, arguments = arguments, body = body, followingContext = followingContext
	}
end

local function NewAnonymousFunctionNode(arguments, body)
	return {type = "anonymous function", arguments = arguments, body = body}
end

local function NewAssignmentNode(destination, source, followingContext)
	return {type = "assignment", destination = destination, source = source, followingContext = followingContext}
end

local function NewBranchNode(condition, resultIfTrue, resultIfFalse)
	return {type = "branch", condition = condition, resultIfTrue = resultIfTrue, resultIfFalse = resultIfFalse}
end

local function NewEvalNode(expression)
	return {type = "eval", expression = expression}
end

local function NewGuardNode(expression)
	return {type = "guard", expression = expression}
end

local function NewListNode(elements)
	return {type = "list", elements = elements}
end

local function NewSublistNode(expression)
	return {type = "sublist", expression = expression}
end

local function NewCallNode(callee, arguments)
	return {type = "call", callee = callee, arguments = arguments}
end

local function PassThroughFirstNode(nodes)
	return nodes[1]
end

local ParserRules = 
{
	{type = "program", "expression", "done", action = PassThroughFirstNode},
	
	{type = "expression", "assignmentTargets", "yield", "chainedOrUnchainedClosedExpression", action = function(nodes) return NewAnonymousFunctionNode(reverse(nodes[1]), nodes[3]) end},
	{type = "expression", "identifier", "assignmentTargets", "assign", "chainedOrUnchainedClosedExpression", "terminal", "expression", action = function(nodes) return NewFunctionAssignmentNode(nodes[1], reverse(nodes[2]), nodes[4], nodes[6]) end},
	{type = "expression", "assignmentTarget", "assign", "chainedOrUnchainedClosedExpression", "terminal", "expression", action = function(nodes) return NewAssignmentNode(nodes[1], nodes[3], nodes[5]) end},
	{type = "expression", "chainedOrUnchainedClosedExpression", "single question mark", "chainedOrUnchainedClosedExpression", "terminal", "expression", action = function(nodes) return NewBranchNode(nodes[1], nodes[3], nodes[5]) end},
	{type = "expression", "chainedExpression", action = PassThroughFirstNode},
	{type = "expression", "closedExpression", action = PassThroughFirstNode},
	
	{type = "chainedOrUnchainedClosedExpression", "chainedExpression", action = PassThroughFirstNode},
	{type = "chainedOrUnchainedClosedExpression", "closedExpression", action = PassThroughFirstNode},
	
	{type = "assignmentTargets", "assignmentTarget", "assignmentTargets", 
		action = function(nodes)
			local targets = nodes[2]
			targets[#targets + 1] = nodes[1]
			return targets
		end
	},
	{type = "assignmentTargets", "assignmentTarget", "end", action = function(nodes) return {nodes[1]} end},
	{type = "assignmentTarget", "eval", "closedExpression", action = function(nodes) return NewEvalNode(nodes[2]) end},
	{type = "assignmentTarget", "double question mark", "closedExpression", action = function(nodes) return NewGuardNode(nodes[2]) end},
	{type = "assignmentTarget", "identifier", action = PassThroughFirstNode},
	{type = "assignmentTarget", "number", action = PassThroughFirstNode},
	{type = "assignmentTarget", "string", action = PassThroughFirstNode},
	{type = "assignmentTarget", "opening square bracket", "assignmentTargetListBody", "closing square bracket", action = function(nodes) return NewListNode(reverse(nodes[2])) end},
	{type = "assignmentTargetListBody", "assignmentTarget", "comma", "assignmentTargetListBody",
		action = function(nodes)
			local elements = nodes[3]
			elements[#elements + 1] = nodes[1]
			return elements
		end
	},
	{type = "assignmentTargetListBody", "ellipsis", "assignmentTarget", 
		action = function(nodes)
			return {NewSublistNode(nodes[2])}
		end
	},
	{type = "assignmentTargetListBody", "assignmentTarget", 
		action = function(nodes)
			return {nodes[1]}
		end
	},
	{type = "assignmentTargetListBody", "end", action = function(nodes) return {} end},
	
	{type = "chainedExpression", "closedExpression", "chainedExpressionArguments", action = function(nodes) return NewCallNode(nodes[1], reverse(nodes[2])) end},
	{type = "chainedExpressionArguments", "closedExpression", "chainedExpressionArguments", 
		action = function(nodes)
			local arguments = nodes[2]
			arguments[#arguments + 1] = nodes[1]
			return arguments
		end
	},
	{type = "chainedExpressionArguments", "closedExpression", "end", action = function(nodes) return {nodes[1]} end},
	
	{type = "closedExpression", "opening parenthesis", "expression", "closing parenthesis", action = function(nodes) return nodes[2] end},
	{type = "closedExpression", "identifier", action = PassThroughFirstNode},
	{type = "closedExpression", "number", action = PassThroughFirstNode},
	{type = "closedExpression", "string", action = PassThroughFirstNode},
	
	{type = "closedExpression", "opening square bracket", "expressionListBody", "closing square bracket", action = function(nodes) return NewListNode(reverse(nodes[2])) end},
	
	{type = "expressionListBody", "expression", "comma", "expressionListBody",
		action = function(nodes)
			local elements = nodes[3]
			elements[#elements + 1] = nodes[1]
			return elements
		end
	},
	{type = "expressionListBody", "ellipsis", "expression", 
		action = function(nodes)
			return {NewSublistNode(nodes[2])}
		end
	},
	{type = "expressionListBody", "expression", 
		action = function(nodes)
			return {nodes[1]}
		end
	},
	{type = "expressionListBody", "end", action = function(nodes) return {} end},
	
	{type = "end", action = function(nodes) return {} end}
}

--[[local CommonActions = 
{
	passThroughFirstNode = function(nodes) return nodes[1] end,
}

local HumanFriendlyRules = 
{
	program = 
	{
		{"expression", "done", action = CommonActions.passThroughFirstNode}
	},
	
	expression = 
	{
		{
			"assignmentTargets", "yield", "chainedOrUnchainedClosedExpression",
			action = 
			function(arguments, function(nodes)
				return {
					type = "anonymous function",
					arguments = reverse(nodes[1]),
					body = nodes[3]
				}
			end
		},
		{
			"identifier", "assignmentTargets", "assign", "chainedOrUnchainedClosedExpression", "terminal", "expression",
			action = 
			function(nodes)
				return {
					type = "function assignment",
					functionName = nodes[1],
					arguments = reverse(nodes[2]),
					body = nodes[4],
					followingContext = nodes[6]
				}
			end
		},
		{
			"assignmentTarget",
			"assign",
			"chainedOrUnchainedClosedExpression",
			"terminal",
			"expression",
			action = function(nodes)
			return {type = "assignment", destination = nodes[1], source = nodes[3], followingContext = nodes[5]} end}
	},
	
	{type = "expression", "assignmentTarget", "assign", "chainedOrUnchainedClosedExpression", "terminal", "expression", action = function(nodes) return {type = "assignment", destination = nodes[1], source = nodes[3], followingContext = nodes[5]} end},
	{type = "expression", "chainedOrUnchainedClosedExpression", "single question mark", "chainedOrUnchainedClosedExpression", "terminal", "expression", action = function(nodes) return {type = "branch", condition = nodes[1], resultIfTrue = nodes[3], resultIfFalse = nodes[5]} end},
	{type = "expression", "chainedExpression", action = function(nodes) return nodes[1] end},
	{type = "expression", "closedExpression", action = function(nodes) return nodes[1] end},
	
	{type = "chainedOrUnchainedClosedExpression", "chainedExpression", action = function(nodes) return nodes[1] end},
	{type = "chainedOrUnchainedClosedExpression", "closedExpression", action = function(nodes) return nodes[1] end},
	
	{type = "assignmentTargets", "assignmentTarget", "assignmentTargets", 
		action = function(nodes)
			local targets = nodes[2]
			targets[#targets + 1] = nodes[1]
			return targets
		end
	},
	{type = "assignmentTargets", "assignmentTarget", "end", action = function(nodes) return {nodes[1]} end},
	{type = "assignmentTarget", "eval", "closedExpression", action = function(nodes) return {type = "eval", expression = nodes[2]} end},
	{type = "assignmentTarget", "double question mark", "closedExpression", action = function(nodes) return {type = "guard", expression = nodes[2]} end},
	{type = "assignmentTarget", "identifier", action = function(nodes) return nodes[1] end},
	{type = "assignmentTarget", "number", action = function(nodes) return nodes[1] end},
	{type = "assignmentTarget", "string", action = function(nodes) return nodes[1] end},
	{type = "assignmentTarget", "opening square bracket", "assignmentTargetListBody", "closing square bracket", action = function(nodes) return {type = "list", elements = reverse(nodes[2])} end},
	{type = "assignmentTargetListBody", "assignmentTarget", "comma", "assignmentTargetListBody",
		action = function(nodes)
			local elements = nodes[3]
			elements[#elements + 1] = nodes[1]
			return elements
		end
	},
	{type = "assignmentTargetListBody", "ellipsis", "assignmentTarget", 
		action = function(nodes)
			return {{type = "sublist", expression = nodes[2]}}
		end
	},
	{type = "assignmentTargetListBody", "assignmentTarget", 
		action = function(nodes)
			return {nodes[1]}
		end
	},
	{type = "assignmentTargetListBody", "end", action = function(nodes) return {} end},
	
	{type = "chainedExpression", "closedExpression", "chainedExpressionArguments", action = function(nodes) return {type = "call", callee = nodes[1], arguments = reverse(nodes[2])} end},
	{type = "chainedExpressionArguments", "closedExpression", "chainedExpressionArguments", 
		action = function(nodes)
			local arguments = nodes[2]
			arguments[#arguments + 1] = nodes[1]
			return arguments
		end
	},
	{type = "chainedExpressionArguments", "closedExpression", "end", action = function(nodes) return {nodes[1]} end},
	
	{type = "closedExpression", "opening parenthesis", "expression", "closing parenthesis", action = function(nodes) return nodes[2] end},
	{type = "closedExpression", "identifier", action = function(nodes) return nodes[1] end},
	{type = "closedExpression", "number", action = function(nodes) return nodes[1] end},
	{type = "closedExpression", "string", action = function(nodes) return nodes[1] end},
	
	{type = "closedExpression", "opening square bracket", "expressionListBody", "closing square bracket", action = function(nodes) return {type = "list", elements = reverse(nodes[2])} end},
	
	{type = "expressionListBody", "expression", "comma", "expressionListBody",
		action = function(nodes)
			local elements = nodes[3]
			elements[#elements + 1] = nodes[1]
			return elements
		end
	},
	{type = "expressionListBody", "ellipsis", "expression", 
		action = function(nodes)
			return {{type = "sublist", expression = nodes[2]}}
		end
	},
	{type = "expressionListBody", "expression", 
		action = function(nodes)
			return {nodes[1]}
		end
	},
	{type = "expressionListBody", "end", action = function(nodes) return {} end},
	
	{type = "end", action = function(nodes) return {} end}
}]]

--[[local function GenerateParserFriendlyRules(humanFriendlyRules)
	local parserFriendlyRules = {}
	for type, branches in pairs(humanFriendlyRules) do
		for _, branch in ipairs(branches) do
			local parserFriendlyRule = {type = type, action = branch.action}
			for i, item in ipairs(branch) do
				parserFriendlyRule[i] = item
			end
			parserFriendlyRules[#parserFriendlyRules + 1] = parserFriendlyRule
		end
	end
	return parserFriendlyRules
end]]

local function Parse(tokens, firstOverall, lastOverall, currentNonterminal, rules, HasRuleCache)
	local function HasRule(symbol)
		local hasRule = HasRuleCache[symbol]
		if hasRule ~= nil then return hasRule end
		for _, rule in ipairs(rules) do
			if rule.type == symbol then
				HasRuleCache[symbol] = true
				return true
			end
		end
		HasRuleCache[symbol] = false
		return false
	end
	
	for _, rule in ipairs(rules) do
		if currentNonterminal == rule.type then
			local first, last, matched = firstOverall, lastOverall, true
			local matchedNodes, matchedNodeCount = {}, 0
			for _, a in ipairs(rule) do
				if first > last then
					matched = false
					break
				end
				if tokens[first].type == a then
					matchedNodeCount = matchedNodeCount + 1
					matchedNodes[matchedNodeCount] = {type = tokens[first].type, data = tokens[first].data}
					first = first + 1
				elseif HasRule(a) then
					local node, count = Parse(tokens, first, last, a, rules, HasRuleCache)
					if count == nil then
						matched = false
						break
					end
					matchedNodeCount = matchedNodeCount + 1
					matchedNodes[matchedNodeCount] = node
					first = first + count
				else
					matched = false
					break
				end
			end
			if matched then
				local node = nil
				if rule.action then node = rule.action(matchedNodes) end
				return node or {}, first - firstOverall
			end
		end
	end
	return nil, nil
end

return function(tokens)
	local HasRuleCache = {}
	local node = Parse(tokens, 1, #tokens, "program", ParserRules, HasRuleCache)
	return node
end
