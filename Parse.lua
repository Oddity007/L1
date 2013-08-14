--[[
Grammar:
	definedparameter = string 
	definedparameter = number
	definedparameter = identifier
	definedparameter = oparen definedparameter cparen
	definedparameter = eval oparen expression cparen
	assignment = identifier (definedparameter)* (qqmark cexpression)? assign cexpression terminal expression
	assignment = list assign cexpression terminal expression
	branch = cexpression qmark cexpression terminal expression
	parameters = cexpression+
	cexpression = oparen expression cparen parameters?
	cexpression = string parameters?
	cexpression = number parameters?
	cexpression = identifier parameters?
	expression = assignment
	expression = branch
	expression = cexpression
	
Reduced Grammar:
	expression = identifier+ yield chainedExpression
	expression = identifier assign chainedExpression terminal expression
	expression = cexpression qmark chainedExpression terminal expression
	expression = chainedExpression
	chainedExpression = closedExpression chainedExpression
	chainedExpression = closedExpression
	closedExpression = oparen expression cparen
	closedExpression = identifier
	closedExpression = number
	closedExpression = string
]]
--[[
local ParseChainedExpression, ParseExpression, ParseParenthesizedExpression, ParseClosedExpression

local function SplitTokens(tokens, first, last, splittingTokenType)
	local level = 0
	for i = first, last do
		local tokenType = tokens[i].type
		if tokenType == "opening parenthesis" then
			level = level + 1
		elseif tokenType == "closing parenthesis" then
			level = level - 1
		elseif level == 0 and tokenType == splittingTokenType then
			return first, i - 1, i + 1, last
		end
	end
end

ParseExpression = function (tokens, first, last)
	local first1, last1, first2, last2 = SplitTokens(tokens, first, last, "terminal")
	if first1 and last1 and first2 and last2 then
		local lastExpression, _ = ParseExpression(tokens, first2, last2)
		first1, last1, first2, last2 = SplitTokens(tokens, first, last, "assign")
		if first1 and last1 and first2 and last2 then
			if first1 ~= last1 then error("Hit an error") end
			if tokens[first1].type ~= "identifier" then error("Hit an error") end
			local destination = {type = "identifier", data = tokens[first1].data}
			local source = ParseChainedExpression(tokens, first2, last2)
			return {type = "assignment", destination = destination, source = source, context = lastExpression}, last - first
		end
		
		first1, last1, first2, last2 = SplitTokens(tokens, first, last, "single question mark")
		if first1 and last1 and first2 and last2 then
			local condition = ParseChainedExpression(tokens, first1, last1)
			local resultIfTrue = ParseChainedExpression(tokens, first2, last2)
			return {type = "branch", condition = condition, resultIfTrue = resultIfTrue, resultIfFalse = lastExpression}, last - first
		end
		
		error("Hit an error")
	end
	
	first1, last1, first2, last2 = SplitTokens(tokens, first, last, "yield")
	if first1 and last1 and first2 and last2 then
		local arguments = {}
		for i = first1, last1 do
			local token = tokens[i]
			if token.type == "identifier" then
				arguments[#arguments + 1] = {type = "identifier", data = token.data}
			else error("Hit an error") end
		end
		local body = ParseChainedExpression(tokens, first2, last2)
		local expression = {type = "closure", arguments = arguments, body = body}, last - first
		return expression
	end
	
	return ParseChainedExpression(tokens, first, last)
end

ParseParenthesizedExpression = function (tokens, first, last)
	if tokens[first].type == "opening parenthesis" then
		if (last - first) < 2 then error("Hit an error") end
		local level = 0
		for i = first, last do
			if tokens[i].type == "opening parenthesis" then
				level = level + 1
			elseif tokens[i].type == "closing parenthesis" then
				level = level - 1
				if level == 0 then
					local subexpression, _ = ParseExpression(tokens, first + 1, i - 1)
					return subexpression, last - first
				end
			end
		end
	end
	error("Hit an error")
end

ParseClosedExpression = function (tokens, first, last)
	if (last - first) == 0 then error("Hit an error") end
	if tokens[first].type == "opening parenthesis" then
		local subexpression, count = ParseParenthesizedExpression(tokens, first, last)
		return subexpression, count
	elseif tokens[first].type == "string" then
		return {type = "string", data = tokens[first].data}, 1
	elseif tokens[first].type == "number" then
		return {type = "number", data = tokens[first].data}, 1
	elseif tokens[first].type == "identifier" then
		return {type = "identifier", data = tokens[first].data}, 1
	end
	error("Hit an error")
end

ParseChainedExpression = function (tokens, first, last)
	local expressions = {}
	repeat
		local expression, count = ParseClosedExpression(tokens, first, last)
		expressions[#expressions] = expression
		first = first + count
	until first >= last
	if #expressions == 1 then return expressions[0] end
	local callee = expressions[0]
	expressions[0] = nil
	return {type = "call", arguments = expressions, callee = callee}, last - first
	
	
	local callee, count = ParseClosedExpression(tokens, first, last)
	first = first + count
	if first >= last then return callee end
	local expressions = {}
	while first < last do
		local expression, count = ParseClosedExpression(tokens, first, last)
		expressions[#expressions + 1] = expression
		first = first + count
	end
	return {type = "call", arguments = expressions, callee = callee}, last - first
end

return function(tokens)
	local root = ParseExpression(tokens, 1, #tokens)
	return root
end]]

--[[
	expression = identifier yield chainedExpression
	expression = identifier assign chainedExpression terminal expression
	expression = identifier assignmentTargets yield chainedExpression
	expression = assignmentTargets yield chainedExpression
	expression = chainedExpression qmark chainedExpression terminal expression
	expression = chainedExpression
	
	assignmentList = assignmentTarget comma assignmentList
	assignmentList = assignmentTarget
	
	assignmentTargets = assignmentTarget assignmentTargets
	assignmentTargets = ellipsis assignmentTarget
	assignmentTargets = assignmentTarget
	
	assignmentTarget = identifier
	assignmentTarget = string
	assignmentTarget = number
	assignmentTarget = oparen assignmentTarget cparen
	assignmentTarget = obrack assignmentList cbrack
	assignmentTarget = obrack cbrack
	assignmentTarget = eval oparen expression cparen
	assignmentTarget = qqmark oparen expression cparen
	
	chainedExpression = closedExpression chainedExpression
	chainedExpression = closedExpression
	
	closedExpressionList = closedExpression comma closedExpressionList
	closedExpressionList = closedExpression
	
	closedExpression = obrack closedExpressionList cbrack
	closedExpression = obrack cbrack
	closedExpression = oparen expression cparen
	closedExpression = identifier
	closedExpression = number
	closedExpression = string
]]

--[[local Rules = 
{
	{type = "program", "expression", "done", action = function(nodes) return nodes[1] end},
	
	{type = "expression", "assignmentTargets", "yield", "chainedExpression", action = function(nodes) return {type = "anonymous function", } end},
	{type = "expression", "identifier", "assign", "chainedExpression", "terminal", "expression", action = function(nodes) return {type = "assignment", destination = nodes[1], source = nodes[3], context = nodes[5]} end},
	{type = "expression", "assignmentTargets", "yield", "chainedExpression", action = function(nodes) return {type = "assignment", destination = nodes[1], source = nodes[3], context = nodes[5]} end},
	{type = "expression", "assignmentTargets", "yield", "chainedExpression"},
	{type = "expression", "chainedExpression", "single question mark", "chainedExpression", "terminal", "expression"},
	{type = "expression", "chainedExpression"},
	
	{type = "expression", "assignmentTarget", "comma", "assignmentList"},
	{type = "expression", "assignmentTarget"},
	
	{type = "assignmentTargets", "assignmentTarget", "assignmentTargets"},
	{type = "assignmentTargets", "ellipsis", "assignmentTarget"},
	{type = "assignmentTargets", "assignmentTarget"},
	
	{type = "assignmentTarget", "identifier"},
	{type = "assignmentTarget", "string"},
	{type = "assignmentTarget", "number"},
	{type = "assignmentTarget", "opening parenthesis", "assignmentTarget", "closing parenthesis"},
	{type = "assignmentTarget", "opening square bracket", "assignmentList", "closing square bracket"},
	{type = "assignmentTarget", "opening square bracket", "closing square bracket"},
	{type = "assignmentTarget", "eval", "opening parenthesis", "expression", "closing parenthesis"},
	{type = "assignmentTarget", "double question mark", "opening parenthesis", "expression", "closing parenthesis"},
	
	{type = "chainedExpression", "closedExpression", "chainedExpression"},
	{type = "chainedExpression", "closedExpression"},
	
	{type = "closedExpressionList", "closedExpression", "comma", "closedExpressionList"},
	{type = "closedExpressionList", "closedExpression"},
	
	{type = "closedExpression", "opening square bracket", "closedExpressionList", "closing square bracket"},
	{type = "closedExpression", "opening square bracket", "closing square bracket"},
	{type = "closedExpression", "opening parenthesis", "expression", "closing parenthesis"},
	{type = "closedExpression", "identifier"},
	{type = "closedExpression", "number"},
	{type = "closedExpression", "string"}
}]]

local function reverse(list)
	if not list then return nil end
	local lengthPlusOne = #list + 1
	local newList = {}
	for i, v in ipairs(list) do
		newList[lengthPlusOne - i] = v
	end
	return newList
end

local Rules = 
{
	{type = "program", "expression", "done", action = function(nodes) return nodes[1] end},
	
	{type = "expression", "assignmentTargets", "yield", "chainedOrUnchainedClosedExpression", action = function(nodes) return {type = "anonymous function", arguments = reverse(nodes[1]), body = nodes[3]} end},
	{type = "expression", "identifier", "assignmentTargets", "assign", "chainedOrUnchainedClosedExpression", "terminal", "expression", action = function(nodes) return {type = "function assignment", functionName = nodes[1], arguments = reverse(nodes[2]), body = nodes[4], followingContext = nodes[6]} end},
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
}

local HasRuleCache = {}
local function HasRule(symbol)
	local hasRule = HasRuleCache[symbol]
	if hasRule ~= nil then return hasRule end
	for _, rule in ipairs(Rules) do
		if rule.type == symbol then
			HasRuleCache[symbol] = true
			return true
		end
	end
	HasRuleCache[symbol] = false
	return false
end

local function Parse(tokens, firstOverall, lastOverall, currentNonterminal)
	for _, rule in ipairs(Rules) do
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
					local node, count = Parse(tokens, first, last, a)
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
	local node = Parse(tokens, 1, #tokens, "program")
	return node
end
