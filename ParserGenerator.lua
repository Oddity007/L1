local Tokens = {"Natural", "String", "Identifier", "Terminal", "OpenParenthesis", "CloseParenthesis", "SingleEqual", "SingleColon", "DoubleColon", "SingleBarArrow", "DoubleBarArrow", "Dollar", "Percent", "Ampersand", "Declare", "Comma", "OpenBracket", "CloseBracket", "Done",}

local Actions = {
	setRoot = "self->rootASTNode = PopNodeLocation(self);",
	
	pushIdentifier = "L1ParserASTNode node; node.type = L1ParserASTNodeTypeIdentifier; node.data.identifier.tokenIndex = self->currentTokenIndex; PushNode(self, & node);",
	pushNatural = "L1ParserASTNode node; node.type = L1ParserASTNodeTypeNatural; node.data.natural.tokenIndex = self->currentTokenIndex; PushNode(self, & node);",
	pushString = "L1ParserASTNode node; node.type = L1ParserASTNodeTypeString; node.data.string.tokenIndex = self->currentTokenIndex; PushNode(self, & node);",
	
	pushEvaluateArgument = "size_t expression = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeEvaluateArgument); node.data.evaluateArgument.expression = expression; PushNode(self, & node);",
	
	pushOverload = "size_t second = PopNodeLocation(self); size_t first = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeOverload); node.data.overload.first = first; node.data.overload.second = second; PushNode(self, & node);",
	
	pushCall = "size_t argument = PopNodeLocation(self); size_t callee = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeCall); node.data.call.callee = callee; node.data.call.argument = argument; PushNode(self, & node);",
	
	pushLambda = "size_t result = PopNodeLocation(self); size_t argument = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeLambda); node.data.lambda.argument = argument; node.data.lambda.result = result; PushNode(self, & node);",
	pushPi = "size_t result = PopNodeLocation(self); size_t argument = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypePi); node.data.pi.argument = argument; node.data.pi.result = result; PushNode(self, & node);",
	
	pushAnnotate = "size_t type = PopNodeLocation(self); size_t value = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeAnnotate); node.data.annotate.value = value; node.data.annotate.type = type; PushNode(self, & node);",
	
	pushAssign = "size_t followingContext = PopNodeLocation(self); size_t source = PopNodeLocation(self); size_t destination = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeAssign); node.data.assign.destination = destination; node.data.assign.source = source; node.data.assign.followingContext = followingContext; PushNode(self, & node);",
	pushDefine = "size_t followingContext = PopNodeLocation(self); size_t source = PopNodeLocation(self); size_t destination = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeDefine); node.data.define.destination = destination; node.data.define.source = source; node.data.define.followingContext = followingContext; PushNode(self, & node);",
	
	pushDeclare = "size_t followingContext = PopNodeLocation(self); size_t destination = PopNodeLocation(self); L1ParserASTNode node; node.type = (L1ParserASTNodeTypeDeclare); node.data.declare.destination = destination; node.data.declare.followingContext = followingContext; PushNode(self, & node);"
}

--[[local Rules = {
		{type = "Program", "Exp", action = Actions.setRoot},
		
		{type = "Exp", "ChainedClosedExp", "ExpFollow"},
		{type = "ExpFollow", "SingleEqual", "ChainedClosedExp", "Terminal", "Exp", action = Actions.pushAssign},
		{type = "ExpFollow", "DoubleColon", "ChainedClosedExp", "Terminal", "Exp", action = Actions.pushDefine},
		{type = "ExpFollow", "SingleColon", "ChainedClosedExp", action = Actions.pushAnnotate},
		{type = "ExpFollow", "Ampersand", "ChainedClosedExp", action = Actions.pushOverload},
		{type = "ExpFollow", ""},
		
		{type = "ChainedClosedExp", "ClosedExp", "ChainedClosedExpFollow"},
		{type = "ChainedClosedExpFollow", "SingleBarArrow", "ChainedClosedExp", action = Actions.pushLambda},
		{type = "ChainedClosedExpFollow", "DoubleBarArrow", "ChainedClosedExp", action = Actions.pushPi},
		
		{type = "DoChainedClosedExpFollowCallAction", "", action = Actions.pushCall},
		
		{type = "ChainedClosedExpFollow", "ClosedExp", "DoChainedClosedExpFollowCallAction", "ChainedClosedExpFollow"},
		{type = "ChainedClosedExpFollow", ""},
		
		{type = "ClosedExp", "Percent", "ClosedExp", action = Actions.pushHideArgument},
		{type = "ClosedExp", "Dollar", "ClosedExp", action =  Actions.pushEvaluateArgument},
		{type = "ClosedExp", "Identifier", action = Actions.pushIdentifier},
		{type = "ClosedExp", "String", aciton = Actions.pushString},
		{type = "ClosedExp", "Natural", action = Actions.pushNatural},
		{type = "ClosedExp", "OpenParenthesis", "Exp", "CloseParenthesis"},
}]]

local Rules = {
		{type = "Program", "Exp", action = Actions.setRoot},
		
		{type = "Exp", "ClosedExp", "ExpFollow"},
		--{type = "Exp", "ChainedClosedExp", "ExpFollow"},
		{type = "Exp", "Declare", "ClosedExp", "Terminal", "Exp", action = Actions.pushDeclare},
		{type = "ExpFollow", "SingleEqual", "ChainedClosedExp", "Terminal", "Exp", action = Actions.pushAssign},
		{type = "ExpFollow", "DoubleColon", "ChainedClosedExp", "Terminal", "Exp", action = Actions.pushDefine},
		{type = "ExpFollow", "SingleColon", "ChainedClosedExp", action = Actions.pushAnnotate},
		{type = "ExpFollow", ""},
		
		{type = "ChainedClosedExp", "ClosedExp", "ChainedClosedExpFollow"},
		{type = "ChainedClosedExpFollow", "SingleBarArrow", "ChainedClosedExp", action = Actions.pushLambda},
		{type = "ChainedClosedExpFollow", "DoubleBarArrow", "ChainedClosedExp", action = Actions.pushPi},
		
		{type = "ChainedClosedExpAmpersandFollow", "Ampersand", "ChainedClosedExp", action = Actions.pushOverload},
		
		{type = "DoChainedClosedExpFollowCallAction", "", action = Actions.pushCall},
		
		{type = "ChainedClosedExpFollow", "ChainedClosedExpCallFollow"},
		
		{type = "ChainedClosedExpCallFollow", "ClosedExp", "DoChainedClosedExpFollowCallAction", "ChainedClosedExpCallFollow"},
		{type = "ChainedClosedExpCallFollow", "ChainedClosedExpAmpersandFollow"},
		{type = "ChainedClosedExpCallFollow", ""},
		
		{type = "ExpList", "Exp", "ExpListFollow"},
		{type = "ExpListFollow", "Comma", "ExpListFollow"},
		
		{type = "ClosedExp", "Dollar", "ClosedExp", action =  Actions.pushEvaluateArgument},
		{type = "ClosedExp", "Identifier", action = Actions.pushIdentifier},
		{type = "ClosedExp", "String", aciton = Actions.pushString},
		{type = "ClosedExp", "Natural", action = Actions.pushNatural},
		{type = "ClosedExp", "OpenBracket", "ExpList", "CloseBracket"},
		{type = "ClosedExp", "OpenParenthesis", "Exp", "CloseParenthesis"},
}

local function IsNonterminal(rules, symbol)
	for _, rule in ipairs(rules) do
		if symbol == rule.type then return true end
	end
	return false
end

local function FirstOld(rules, symbol, encountered)
	assert(symbol)
	--if not encountered then encountered = {} end
	local r = {}
	--if encountered[symbol] then return r end
	--encountered[symbol] = true
	local isTerminal = true
	for i, rule in ipairs(rules) do
		if rule.type == symbol then
			local first = First(rules, rule[1], encountered)
			for s, _ in pairs(first) do
				r[s] = true
			end
			if first[""] and rule[2] then
				local second = First(rules, rule[2], encountered)
				for s, _ in pairs(second) do
					r[s] = true
				end
			end
			--[[local hasEpsilon = false
			local hasEncounteredNonEpsilon = false;
			for _, s in ipairs(rule) do
				repeat
					local first = First(rules, s, encountered)
					for f, _ in pairs(first) do
						if f == "" then
							hasEpsilon = true
						else
							r[f] = true
						end
					end
					if not first[""] then
						hasEncounteredNonEpsilon = true
					end
				until hasEncounteredNonEpsilon
				if hasEpsilon then
					r[""] = true
				end
			end]]
			--[[for f, _ in pairs(First(rules, rule[1])) do
				if r[f] then
					error("First/First Conflict")
				end
				r[f] = true
			end
			if rule[#rule] == "" then
				r[""] = true
			end]]
			isTerminal = false
		end
	end
	if isTerminal then
		r[symbol] = true
	end
	return r
end

local function FollowOld(rules, symbol, encountered)
	if not encountered then encountered = {} end
	if encountered[symbol] then 
		--error("Recursion")
		return {}
	end
	encountered[symbol] = true
	assert(IsNonterminal(rules, symbol))
	local r = {}
	if symbol == "Program" then
		r["Done"] = true
	end
	--[[for _, rule in ipairs(rules) do
		for i, s in ipairs(rule) do
			if s == symbol then
				if rule[i + 1] then
					local hasEpsilon = false
					for f, _ in pairs(First(rules, rule[i + 1])) do
						if f == "" then
							hasEpsilon = true
						else
							r[f] = true
						end
					end
					if hasEpsilon then
						for f, _ in pairs(Follow(rules, rule.type, encountered)) do
							r[f] = true
						end
					end
				else
					for f, _ in pairs(Follow(rules, rule.type, encountered)) do
						r[f] = true
					end
					--r["Done"] = true
				end
			end
		end
	end]]
		for _, rule in ipairs(rules) do
			for i, s in ipairs(rule) do
				if s == symbol then
					if rule[i + 1] then
						local hasEpsilon = false
						for f, _ in pairs(First(rules, rule[i + 1])) do
							if f == "" then
								hasEpsilon = true
							else
								r[f] = true
							end
						end
						if hasEpsilon then
							for f, _ in pairs(Follow(rules, rule.type, encountered)) do
								r[f] = true
							end
						end
					else
						for f, _ in pairs(Follow(rules, rule.type, encountered)) do
							r[f] = true
						end
						r["Done"] = true
					end
				end
			end
		end
	--[[for _, rule in ipairs(rules) do
		for i, s in ipairs(rule) do
			if s == symbol then
				if rule[i + 1] then
					for f, _ in pairs(First(rules, rule[i + 1])) do
						r[f] = true
					end
				else
					for f, _ in pairs(Follow(rules, rule.type, encountered)) do
						r[f] = true
					end
				end
			end
		end
	end]]
	return r
end

local function Union(a, b)
	local r = {}
	for s, _ in pairs(a) do
		r[s] = true
	end
	for s, _ in pairs(b) do
		r[s] = true
	end
	return r
end

local function First(rules, of)
	local isTerminal = true
	local result = {}
	for _, rule in ipairs(rules) do
		if rule.type == of then
			isTerminal = false
			local accumulated = {}
			for _, s in ipairs(rule) do
				local first = First(rules, s)
				accumulated = Union(first, accumulated)
				if not first[""] then break end
			end
			result = Union(result, accumulated)
		end
	end
	if isTerminal then
		return {[of] = true}
	else
		return result
	end
end

local function Follow(rules, of, encountered)
	if not encountered then encountered = {} end
	if encountered[of] then return {} end
	encountered[of] = true
	assert(IsNonterminal(rules, of))
	local result = {}
	if of == "Program" then
		result["Done"] = true
	end
	for _, rule in ipairs(Rules) do
		for i, s in ipairs(rule) do
			if s == of then
				local needsRuleFollow = true
				if rule[i + 1] then
					needsRuleFollow = false
					local first = First(rules, rule[i + 1])
					if first[""] then needsRuleFollow = true end
					first[""] = nil
					result = Union(result, first)
				end
				if needsRuleFollow then
					if of ~= rule.type then
						result = Union(result, Follow(rules, rule.type, encountered))
					end
				end
			end
		end
	end
	return result
end

local function GenParseTable(rules)
	--[[local t = {}
	for i, r in ipairs(rules) do
		t[r.type] = t[r.type] or {}
	end
	for i, r in ipairs(rules) do
		local first = First(rules, r[1])
		for s, _ in pairs(first) do
			if s == "" then
				for s2, _ in pairs(Follow(rules, r.type)) do
					t[r.type][s2] = i
				end
			else
				t[r.type][s] = i
			end
		end
	end
	return t]]
	local t = {}
	for i, r in ipairs(rules) do
		t[r.type] = t[r.type] or {}
	end
	for i, rule in ipairs(rules) do
		local first = First(rules, rule[1])
		local hasEpsilon = false
		for s, _ in pairs(first) do
			if s == "" then
				hasEpsilon = true
			else
			--	assert(not t[rule.type][s])
				if t[rule.type][s] then
					if t[rule.type][s] ~= i then
						error("Existing rule " .. tostring(t[rule.type][s]) .. " collides with rule " .. tostring(i) .. " for nonterminal " .. rule.type .. " and terminal " .. s)
					end
				else
					t[rule.type][s] = i
				end
			end
		end
		if hasEpsilon then
			for s2, _ in pairs(Follow(rules, rule.type)) do
				--assert(not t[rule.type][s2])
				if t[rule.type][s2] then
					if t[rule.type][s2] ~= i then
						error("Existing rule " .. tostring(t[rule.type][s2]) .. " collides with rule " .. tostring(i) .. " for nonterminal " .. rule.type .. " and terminal " .. s2)
					end
				else
					t[rule.type][s2] = i
				end
			end
		end
	end
	return t
end

local function MaxRuleLength(rules)
	local l = 0
	for _, r in ipairs(rules) do
		if #r > l then l = #r end
	end
	return l
end

--[[for s, _ in pairs(Follow(Rules, "ExpFollow")) do
	print(s)
end]]
do
	print("/*\n")
	local encountered = {}
	for _, rule in ipairs(Rules) do
		if not encountered[rule.type] then
			encountered[rule.type] = true
			print(rule.type)
			print("\tFirst:")
			for s, _ in pairs(First(Rules, rule.type)) do
				print("\t\t" .. s)
			end
			print("\tFollow:")
			for s, _ in pairs(Follow(Rules, rule.type)) do
				print("\t\t" .. s)
			end
		end
	end
	print("*/\n")
end

do
	local t = GenParseTable(Rules)
	
	local output = {}
	
	local terminalIDs = {}
	local terminals = {}
	local nextTerminalID = 0
	
	--Make sure that tokens map to typeIDs directly
	for _, v in ipairs(Tokens) do
		terminalIDs[v] = nextTerminalID
		terminals[#terminals + 1] = v
		nextTerminalID = nextTerminalID + 1
		--print("Terminal " .. v .. " " .. tostring(nextTerminalID - 1))
	end
	
	terminalIDs[""] = nextTerminalID
	terminals[#terminals + 1] = ""
	nextTerminalID = nextTerminalID + 1
	
	local ruleTerminationID = nextTerminalID
	nextTerminalID = nextTerminalID + 1
	
	--Generate Integer Mappings
	
	local nonterminalIDs = {}
	local nonterminals = {}
	local nonterminalOffset = nextTerminalID
	output[#output + 1] = "static const unsigned char NonterminalOffset = "
	output[#output + 1] = tostring(nonterminalOffset)
	output[#output + 1] = ";\n"
	local nextNonterminalID = nonterminalOffset
	nextTerminalID = nil
	
	for _, rule in ipairs(Rules) do
		nonterminalIDs[rule.type] = nextNonterminalID
		nonterminals[#nonterminals + 1] = rule.type
		nextNonterminalID = nextNonterminalID + 1
		--print("Nonterminal " .. rule.type .. " " .. tostring(nextNonterminalID - 1))
	end
	
	--nonterminalIDs[""] = nextNonterminalID
	--nonterminals[#nonterminals + 1] = ""
	--nextNonterminalID = nextNonterminalID + 1
	
	output[#output + 1] = "static const unsigned char RuleTerminationID = "
	output[#output + 1] = tostring(ruleTerminationID)
	output[#output + 1] = ";\n"
	output[#output + 1] = "static const unsigned char ProgramNonterminalID = "
	output[#output + 1] = tostring(nonterminalIDs["Program"])
	output[#output + 1] = ";\n"
	
	local actionIDs = {}
	local nextActionID = nextNonterminalID
	local actionOffset = nextActionID
	output[#output + 1] = "static const unsigned char ActionOffset = "
	output[#output + 1] = tostring(actionOffset)
	output[#output + 1] = ";\n"
	nextNonterminalID = nil
	do
		for _, rule in ipairs(Rules) do
			if rule.action then
				actionIDs[rule.action] = nextActionID
				nextActionID = nextActionID + 1
			end
		end
	end
	
	--Generate rule table
	do
		output[#output + 1] = "static const unsigned char RuleTable["
		output[#output + 1] = tostring(#Rules)
		output[#output + 1] = "]["
		output[#output + 1] = tostring(MaxRuleLength(Rules) + 2)
		output[#output + 1] = "]=\n{\n"
		for _, rule in ipairs(Rules) do
			output[#output + 1] = "\t{"
			for _, symbol in ipairs(rule) do
				if symbol == "" then
				elseif nonterminalIDs[symbol] then
					output[#output + 1] = tostring(nonterminalIDs[symbol])
					output[#output + 1] = ","
				elseif terminalIDs[symbol] then
					output[#output + 1] = tostring(terminalIDs[symbol])
					output[#output + 1] = ","
				else
					error("Invalid symbol: " .. symbol)
				end
			end
			if rule.action then
				output[#output + 1] = actionIDs[rule.action]
				output[#output + 1] = ", "
			end
			output[#output + 1] = tostring(ruleTerminationID)
			output[#output + 1] = "},\n"
		end
		output[#output + 1] = "};\n"
	end
	
	--Generate prediction table
	do
		output[#output + 1] = "static const unsigned char ParseTable["
		output[#output + 1] = tostring(#nonterminals)
		output[#output + 1] = "]["
		output[#output + 1] = tostring(#terminals)
		output[#output + 1] = "]=\n{\n"
		for _, nonterminal in ipairs(nonterminals) do
			output[#output + 1] = "\t{"
			for _, terminal in ipairs(terminals) do
				local ruleID = 0
				if nonterminal == "" then
					error("Epsilon in nonterminals")
				else
					ruleID = t[nonterminal][terminal] or 0
				end
				output[#output + 1] = tostring(ruleID)
				output[#output + 1] = ","
			end
			output[#output + 1] = "},\n"
		end
		output[#output + 1] = "};\n"
	end
	
	do
		output[#output + 1] = "static const char* SymbolName(unsigned char c)\n{\n\tswitch(c)\n\t{\n"
		local encountered = {}
		for id, nonterminal in ipairs(nonterminals) do
			if not encountered[nonterminal] then
				output[#output + 1] = "\t\tcase "
				output[#output + 1] = tostring(nonterminalIDs[nonterminal])
				output[#output + 1] = ": return "
				output[#output + 1] = "\""
				output[#output + 1] = nonterminal
				output[#output + 1] = "\";\n"
				encountered[nonterminal] = true
			end
		end
		for id, terminal in ipairs(terminals) do
			if not encountered[terminal] then
				output[#output + 1] = "\t\tcase "
				output[#output + 1] = tostring(terminalIDs[terminal])
				output[#output + 1] = ": return "
				output[#output + 1] = "\""
				output[#output + 1] = terminal
				output[#output + 1] = "\";\n"
				encountered[terminal] = true
			end
		end
		output[#output + 1] = "\t\tdefault: return NULL;\n"
		output[#output + 1] = "\t}\n}\n";
	end
	
	do
		output[#output + 1] = "static void HandleAction(L1Parser* self, unsigned char action, const char* tokenString, size_t tokenStringLength)\n{\n\tswitch(action)\n\t{\n"
		for action, id in pairs(actionIDs) do
			output[#output + 1] = "\t\tcase "
			output[#output + 1] = tostring(id)
			output[#output + 1] = ": "
			output[#output + 1] = "{"
			output[#output + 1] = action
			output[#output + 1] = "}; break;\n"
		end
		output[#output + 1] = "\t\tdefault: break;\n"
		output[#output + 1] = "\t}\n}\n";
	end
	
	print(table.concat(output, ""))
end
