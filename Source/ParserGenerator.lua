local Tokens = {"Natural", "String", "Identifier", "Terminal", "OpenParenthesis", "CloseParenthesis", "SingleEqual", "SingleColon", "SingleLineArrow", "Bar", "Ampersand", "Declare", "Comma", "Self", "Universe", "Let", "Fn", "Pi", "Sigma", "ADT", "Match", "Period", "Dollar", "Done",}

local Actions = {}
local Actions = {
	beginProgram = "",
	endProgram = "self->root = PopLocalAddress(self);",
	
	outputCall = "L1IRAddress argument = PopLocalAddress(self); L1IRAddress callee = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeCall, callee, argument, 0));",
	
	beginLetBody = "PushBinding(self, PopTokenID(self), PopLocalAddress(self), BindingTypeLet);",
	endLetBody = "PopBinding(self);",
	
	beginBlockCapture = "self->stateDepth++;",
	bindArgument = "L1IRAddress argumentType = PopLocalAddress(self); PushLocalAddress(self, argumentType); PushBinding(self, PopTokenID(self), CreateSlot(self, L1IRSlotTypeArgument, self->stateDepth, argumentType, 0), BindingTypeArgument);",
	endFnExp = "PopBinding(self); L1IRAddress result = PopLocalAddress(self); L1IRAddress argumentType = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeLambda, argumentType, result, 0));",
	endPiExp = "PopBinding(self); L1IRAddress result = PopLocalAddress(self); L1IRAddress argumentType = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeForall, argumentType, result, 0));",
	--endSigmaExp = "PopBinding(self); L1IRAddress result = PopLocalAddress(self); L1IRAddress argumentType = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeExists, argumentType, result, 0));",
	
	outputString = "PushLocalAddress(self, CreateString(self, tokenString, tokenStringLength));",
	--outputSelf = "PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeSelf, 0, 0, 0));",
	outputUniverse = "PushLocalAddress(self, CreateUniverse(self, tokenString, tokenStringLength));",
	lookupIdentifier = "PushLocalAddress(self, LookupBinding(self, GetTokenID(self, tokenString, tokenStringLength)));",
	pushIdentifier = "PushTokenID(self, GetTokenID(self, tokenString, tokenStringLength));",

	--[[beginADT = "PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeADT, self->nextADTTag, 0, 0)); self->nextADTTag++;",
	extendADT = "
		L1IRLocalAddress argumentType = PopLocalAddress(self);
		L1IRLocalAddress adt = PopLocalAddress(self);
		PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeExtendADT, adt, argumentType, 0));
	",
	outputUnitType = "PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeUnitType, 0, 0, 0));",]]
	
	beginADT = "self->stateDepth++; L1IRAddress argument = CreateSlot(self, L1IRSlotTypeArgument, self->stateDepth, CreateSlot(self, L1IRSlotTypeUniverse, 0, 0, 0), 0); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeADT, self->nextADTTag, 0, 0)); PushBinding(self, GetTokenID(self, \"self\", 4), argument, BindingTypeLet); self->nextADTTag++;",
	endADT = "PopBinding(self); L1IRAddress result = PopLocalAddress(self); /*PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeRecursive, CreateSlot(self, L1IRSlotTypeUniverse, 0, 0, 0), result, 0));*/ PushLocalAddress(self, result); self->stateDepth--;",
	extendADT = [[
		//PopBinding(self);
		//PopBinding(self);
		//PopBinding(self);
		//PopBinding(self);
		L1IRAddress constructed = PopLocalAddress(self);
		L1IRAddress name = PopLocalAddress(self);
		L1IRAddress adt = PopLocalAddress(self);
		PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeExtendADT, adt, name, constructed));
	]],
	-----------
	bindConsDefArgument = [[
		self->stateDepth++;
		L1IRAddress argumentType = PopLocalAddress(self);
		L1IRAddress captures = PopLocalAddress(self);
		L1IRAddress name = PopLocalAddress(self);
		L1IRAddress adt = PopLocalAddress(self);
		PushLocalAddress(self, argumentType);
		L1IRAddress argument = CreateSlot(self, L1IRSlotTypeArgument, self->stateDepth, argumentType, 0);
		PushBinding(self, PopTokenID(self), argument, BindingTypeArgument);
		
		captures = CreateSlot(self, L1IRSlotTypePair, captures, argument, 0);

		PushLocalAddress(self, adt);
		PushLocalAddress(self, name);
		PushLocalAddress(self, captures);
	]],
	endConsDef = [[PopBinding(self); L1IRAddress result = PopLocalAddress(self); L1IRAddress argumentType = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeLambda, argumentType, result, 0)); self->stateDepth--;]],
	endInnerConsDef = [[
		L1IRAddress captures = PopLocalAddress(self);
		L1IRAddress name = PopLocalAddress(self);
		L1IRAddress adt = PopLocalAddress(self);
		//PushLocalAddress(self, adt);
		//PushLocalAddress(self, name);
		adt = LookupBinding(self, GetTokenID(self, "self", 4));
		PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeConstructed, adt, name, captures));
	]],
	
	beginConsDefArgList = [[
		L1IRAddress name = PopLocalAddress(self);
		L1IRAddress adt = PopLocalAddress(self);
		PushLocalAddress(self, adt);
		PushLocalAddress(self, name);
		PushLocalAddress(self, adt);
		PushLocalAddress(self, name);
		PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeUnit, 0, 0, 0));
		//PushBinding(self, GetTokenID(self, "#consdefadt", strlen("#consdefadt")), adt, BindingTypeLet);
		//PushBinding(self, GetTokenID(self, "#consdefname", strlen("#consdefname")), name, BindingTypeLet);
		//PushBinding(self, GetTokenID(self, "#consdefcaptures", strlen("#consdefcaptures")), CreateSlot(self, L1IRSlotTypeUnit, 0, 0, 0), BindingTypeLet);
	]],
	
	outputUnit = "PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeUnit, 0, 0, 0));",
	
	outputUnknown = "PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeUnknown, 0, 0, 0));",
	
	beginMatch = "L1IRAddress value = PopLocalAddress(self); L1IRAddress typeRef = CreateSlot(self, L1IRSlotTypeTypeOf, value, 0, 0); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeBeginMatch, value, typeRef, 0));",
	endMatch = "L1IRAddress match = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeEndMatch, match, CreateSlot(self, L1IRSlotTypeUnknown, 0, 0, 0), 0));",
	endConsDes = "L1IRAddress handler = PopLocalAddress(self); L1IRAddress name = PopLocalAddress(self); L1IRAddress match = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeMatchCase, match, name, handler));",
	
	--endConsDesInnerArgument = "PopBinding(self); L1IRAddress result = PopLocalAddress(self); L1IRAddress argumentType = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeLambda, argumentType, result, 0)); self->stateDepth--;",
	endConsDesInnerArgument = "PopBinding(self); L1IRAddress result = PopLocalAddress(self); L1IRAddress argumentType = PopLocalAddress(self); PushLocalAddress(self, result); self->stateDepth--;",
	
	lookupField = "L1IRAddress name = PopLocalAddress(self); L1IRAddress parent = PopLocalAddress(self); PushLocalAddress(self, CreateSlot(self, L1IRSlotTypeLookup, parent, name, 0));",
}

local Rules = {
		--{type = "Program", "DoBeginProgram", "Exp", action = Actions.endProgram},
		{type = "Program", "Exp", action = Actions.endProgram},
		--{type = "Program", "Exp", action = Actions.endProgram},
		
		{type = "Exp", "ChainedClosedExp"},
		{type = "Exp", "Let", "PushedIdentifier", "SingleEqual", "ChainedClosedExp", "DoExpLetBodyBegin", "ExpFollow", "DoExpLetBodyEnd"},
		--{type = "Exp", "Declare", "PushedIdentifier", "ExpFollow"},
		{type = "ExpFollow", "Terminal", "Exp"},
		
		{type = "ChainedClosedExp", "ClosedExp", "ChainedClosedExpFollow"},
		
		{type = "ChainedClosedExp", "ADT", "DoBeginADT", "ConsDefList", action = Actions.endADT},
		
		{type = "ChainedClosedExp", "Fn", "FirstFnExp"},
		{type = "FirstFnExp", "ArgDef", "FnExpFollow", action = Actions.endFnExp},
		{type = "FnExp", "ArgDef", "FnExpFollow", action = Actions.endFnExp},
		{type = "FnExpFollow", "FnExp"},
		{type = "FnExpFollow", "SingleLineArrow", "ChainedClosedExp"},
		
		{type = "ChainedClosedExp", "Pi", "FirstPiExp"},
		{type = "FirstPiExp", "ArgDef", "PiExpFollow", action = Actions.endPiExp},
		{type = "PiExp", "ArgDef", "PiExpFollow", action = Actions.endPiExp},
		{type = "PiExpFollow", "PiExp"},
		{type = "PiExpFollow", "SingleLineArrow", "ChainedClosedExp"},
		
		--{type = "ChainedClosedExp", "Sigma", "FirstSigmaExp"},
		--{type = "FirstSigmaExp", "ArgDef", "SigmaExpFollow", action = Actions.endSigmaExp},
		--{type = "SigmaExp", "ArgDef", "SigmaExpFollow", action = Actions.endSigmaExp},
		--{type = "SigmaExpFollow", "SigmaExp"},
		--{type = "SigmaExpFollow", "SingleLineArrow", "ChainedClosedExp"},
		
		{type = "ChainedClosedExpFollow", "ClosedExp", "DoChainedClosedExpFollowCallAction", "ChainedClosedExpFollow"},
		--{type = "ChainedClosedExpFollow", "Ampersand", "ChainedClosedExp", action = Actions.outputPair},
		--{type = "ChainedClosedExpFollow", "SingleColon", "ChainedClosedExp"},
		{type = "ChainedClosedExpFollow", "Match", "DoChainedClosedExpFollowMatchBeginAction", "ConsDesList", "DoChainedClosedExpFollowMatchEndAction"},
		{type = "ChainedClosedExpFollow", ""},
		
		{type = "ArgDef", "OpenParenthesis", "PushedIdentifier", "SingleColon", "DoBeginBlockCapture", "ChainedClosedExp", "CloseParenthesis", action = Actions.bindArgument},
		{type = "ConsDefArgDef", "OpenParenthesis", "PushedIdentifier", "SingleColon", "DoBeginBlockCapture", "ChainedClosedExp", "CloseParenthesis", action = Actions.bindConsDefArgument},
		
		{type = "LookupIdentifier", "Identifier", action = Actions.lookupIdentifier},
		{type = "LookupField", "Period", "IdentifierAsString", action = Actions.lookupField},
		
		{type = "ClosedExp", "LookupIdentifier", "ClosedExpFollow"},
		--{type = "ClosedExp", "String", action = Actions.outputString},
		--{type = "ClosedExp", "Self", "ClosedExpFollow", action = Actions.outputSelf},
		{type = "ClosedExp", "Universe", "Natural", action = Actions.outputUniverse},
		{type = "ClosedExp", "OpenParenthesis", "Exp", "CloseParenthesis", "ClosedExpFollow"},
		{type = "ClosedExpFollow", "LookupField", "ClosedExpFollow"},
		{type = "ClosedExpFollow", ""},

		{type = "ConsDef", "Period", "IdentifierAsString", "DoBeginConsDefArgList", "ConsDefArgDefList", action = Actions.extendADT},
		
		--{type = "ConsDefArgDef", "OpenParenthesis", "PushedIdentifier", "SingleColon", "DoBeginBlockCapture", "ChainedClosedExp", "CloseParenthesis", action = Actions.bindConsDefArgument},
		--{type = "ConsDefArgDefList", "ConsDefArgDef", "ConsDefArgDefList", action = Actions.endConsDef},
		{type = "ConsDefArgDefList", "ConsDefArgDef", "ConsDefArgDefList", action = Actions.endConsDef},
		{type = "ConsDefArgDefList", "", action = Actions.endInnerConsDef},
		
		{type = "ConsDefList", "ConsDef", "ConsDefListFollow"},
		{type = "ConsDefList", ""},
		{type = "ConsDefListFollow", "Bar", "ConsDefList"},
		{type = "ConsDefListFollow", ""},
		
		
		{type = "ConsDesInner", "SingleLineArrow", "ClosedExp"},
		{type = "ConsDesInner", "ArgDef", "ConsDesInner", action = Actions.endConsDesInnerArgument},
		
		{type = "ConsDes", "Period", "IdentifierAsString", "DoBeginConsDesArguments", "ConsDesInner", action = Actions.endConsDes},
		--{type = "DefaultConsDes", "ArgDef", "SingleLineArrow", "ClosedExp"},
		--{type = "ConsDesList", "ConsDes", "ConsDesListFollow"},
		--{type = "ConsDesListFollow", "Bar", "ConsDesList"},
		--{type = "ConsDesListFollow", ""},
		
		{type = "ConsDesList", "ConsDes", "ConsDesListFollow"},
		--{type = "ConsDesList", ""},
		{type = "ConsDesListFollow", "Bar", "ConsDesList"},
		--{type = "ConsDesListFollow", "Bar", "DefaultConsDes"},
		{type = "ConsDesListFollow", ""},
		
		
		{type = "PushedIdentifier", "Identifier", action = Actions.pushIdentifier},
		{type = "IdentifierAsString", "Identifier", action = Actions.outputString},
		
		{type = "DoBeginConsDefArgList", "", action = Actions.beginConsDefArgList},
		{type = "DoBeginADT", "", action = Actions.beginADT},
		--{type = "DoBindIdentifier", "", action = Actions.bindIdentifier},
		--{type = "DoBindArgument", "", action = Actions.bindArgument},
		{type = "DoBeginBlockCapture", "", action = Actions.beginBlockCapture},
		{type = "DoBeginProgram", "", action = Actions.beginProgram},
		{type = "DoExpLetBodyBegin", "", action = Actions.beginLetBody},
		{type = "DoExpLetBodyEnd", "", action = Actions.endLetBody},
		{type = "DoChainedClosedExpFollowCallAction", "", action = Actions.outputCall},
		{type = "DoChainedClosedExpFollowMatchBeginAction", "", action = Actions.beginMatch},
		{type = "DoChainedClosedExpFollowMatchEndAction", "", action = Actions.endMatch},
		{type = "DoNamedConsDesListAction", "", action = Actions.outputDestruct},
		{type = "DoDefaultDestructBegin", "", action = Actions.defaultDestructBegin},
		{type = "DoBeginConsDesArguments", "", action = Actions.beginConsDesArguments},
}

local function IsNonterminal(rules, symbol)
	for _, rule in ipairs(rules) do
		if symbol == rule.type then return true end
	end
	return false
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

--[[local function Follow(rules, of, encountered)
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
end]]

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
					local first = First(rules, rule[i + 1])
					local hadEpsilon = false
					if first[""] then
						result = Union(result, Follow(rules, rule[i + 1], encountered))
					else
						needsRuleFollow = false 
					end
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
			--for s2, _ in pairs(Follow(rules, rule[1])) do
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
