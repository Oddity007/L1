local Tokens = {"Natural", "Identifier", "String", "Assign", "OpeningParenthesis", "ClosingParenthesis", "OpeningSquareBracket", "ClosingSquareBracket", "Comma", "Terminal", "QuestionMark", "ExclaimationMark", "SingleColon", "DoubleColon", "Yield", "ConstructorYield", "SingleDot", "DoubleDot", "TripleDot", "SingleQuote", "Dollar", "Bar", "Done",}

local Rules = {
	{type = "program", "openexpression", "Done", action = "return arguments[0];"},
	
	{type = "openexpression", "assignment", action = "return arguments[0];"},
	{type = "openexpression", "branch", action = "return arguments[0];"},
	{type = "openexpression", "annotatedchainedexpression", action = "return arguments[0];"},
	
	{type = "option", "chainedexpression", "Bar", "annotatedchainedexpression", action = "return CreateOptionNode(parser, arguments[0], arguments[2]);"},
	
	{type = "constraint", "chainedexpression", "SingleColon", "annotatedchainedexpression", action = "return CreateConstructorConstraintNode(parser, arguments[0], arguments[2]);"},
	
	{type = "anonymousfunction", "chainedexpression_arguments", "Yield", "annotatedchainedexpression", action = "return CreateAnonymousFunctionNode(parser, arguments[0], arguments[2], false);"},
	{type = "anonymousfunction", "chainedexpression_arguments", "ConstructorYield", "annotatedchainedexpression", action = "return CreateAnonymousFunctionNode(parser, arguments[0], arguments[2], true);"},
	
	{type = "branch", "annotatedchainedexpression", "QuestionMark", "annotatedchainedexpression", "Terminal", "openexpression", action = "return CreateBranchNode(parser, arguments[0], arguments[2], arguments[4]);"},
	{type = "branch", "annotatedchainedexpression", "QuestionMark", "Terminal", "openexpression", action = "return CreateBranchNode(parser, arguments[0], NULL, arguments[3]);"},
	{type = "branch", "annotatedchainedexpression", "QuestionMark", "annotatedchainedexpression", "Terminal", action = "return CreateBranchNode(parser, arguments[0], arguments[2], NULL);"},
	
	{type = "assignment", "annotatedchainedexpression", "Assign", "annotatedchainedexpression", "Terminal", "openexpression", action = "return CreateAssignmentNode(parser, arguments[0], arguments[2], arguments[4], false);"},
	{type = "assignment", "annotatedchainedexpression", "DoubleColon", "annotatedchainedexpression", "Terminal", "openexpression", action = "return CreateAssignmentNode(parser, arguments[0], arguments[2], arguments[4], true);"},
	
	{type = "annotatedchainedexpression", "option", action = "return arguments[0];"},
	{type = "annotatedchainedexpression", "anonymousfunction", action = "return arguments[0];"},
	{type = "annotatedchainedexpression", "constraint", action = "return arguments[0];"},
	{type = "annotatedchainedexpression", "chainedexpression", action = "return arguments[0];"},
	
	{type = "chainedexpression", "closedexpression", "chainedexpression_arguments", action = "return CreateCallNode(parser, arguments[0], arguments[1]);"},
	{type = "chainedexpression", "closedexpression", action = "return arguments[0];"},
	
	{type = "chainedexpression_arguments", "closedexpression", "chainedexpression_arguments", action = "return Cons(parser, arguments[0], arguments[1]);"},
	{type = "chainedexpression_arguments", "closedexpression", action = "return Cons(parser, arguments[0], NULL);"},
	
	{type = "closedexpression", "Identifier", action = "return arguments[0];"},
	{type = "closedexpression", "Natural", action = "return arguments[0];"},
	{type = "closedexpression", "String", action = "return arguments[0];"},
	{type = "closedexpression", "OpeningParenthesis", "openexpression", "ClosingParenthesis", action = "return arguments[1];"},
	{type = "closedexpression", "OpeningSquareBracket", "ClosingSquareBracket", action = "return CreateListNode(parser, NULL, NULL);"},
	{type = "closedexpression", "OpeningSquareBracket", "list_body", "TripleDot", "chainedexpression", "ClosingSquareBracket", action = "return CreateListNode(parser, arguments[1], arguments[3]);"},
	{type = "closedexpression", "OpeningSquareBracket", "list_body", "ClosingSquareBracket", action = "return CreateListNode(parser, arguments[1], NULL);"},
	{type = "closedexpression", "SingleQuote", "closedexpression", action = "return CreateEvalNode(parser, arguments[1]);"},
	
	{type = "list_body", "annotatedchainedexpression", "Comma", "list_body", action = "return Cons(parser, arguments[0], arguments[2]);"},
	{type = "list_body", "annotatedchainedexpression", "Comma", action = "return Cons(parser, arguments[0], NULL);"},
	{type = "list_body", "annotatedchainedexpression", action = "return Cons(parser, arguments[0], NULL);"},
	{type = "list_body", action = "return NULL;"},
}

do
	local identifiers = {}
	local identifierID = 0
	for _, token in ipairs(Tokens) do
		identifiers[token] = identifierID
		identifierID = identifierID + 1
	end
	--Generate the symbol identifiers
	for _, rule in ipairs(Rules) do
		local identifier = identifiers[rule.type]
		if not identifier then
			identifiers[rule.type] = identifierID
			identifierID = identifierID + 1
		end
	end
	--Start outputting stuff
	local output = {}
	--{"#include <stdint.h>\n"}
	--Generate the symbol strings
	for i, rule in ipairs(Rules) do
		output[#output + 1] = "static const uint8_t rule_string_"
		output[#output + 1] = tostring(i)
		output[#output + 1] = "["
		output[#output + 1] = tostring((#rule > 0) and #rule or 1)
		output[#output + 1] = "] = {"
		if #rule > 0 then
			for _, symbol in ipairs(rule) do
				output[#output + 1] = tostring(identifiers[symbol])
				output[#output + 1] = ", "
			end
		else
			output[#output + 1] = "0"
		end
		output[#output + 1] = "};\n"
	end
	--Generate the action table
	local actions = {}
	local actionID = 0
	--Generate the action handler
	output[#output + 1] = "static const void* HandleAction(L1Parser* parser, const void* arguments[], Rule rule)\n{\n\tswitch(rule.action)\n\t{\n"
	for i, rule in ipairs(Rules) do
		local action = actions[rule.action]
		if not action then
			action = actionID
			actions[rule.action] = action
			actionID = actionID + 1
			output[#output + 1] = "\t\tcase "
			output[#output + 1] = tostring(action)
			output[#output + 1] = ":{"
			output[#output + 1] = rule.action
			output[#output + 1] = "} break;\n"
		end
	end
	output[#output + 1] = "\t\tdefault: return NULL;\n\t}\n\treturn NULL;\n}\n"
	--Generate the rule table
	output[#output + 1] = "static const Rule Rules[] = {"
	for i, rule in ipairs(Rules) do
		output[#output + 1] = "{"
		output[#output + 1] = "rule_string_"
		output[#output + 1] = tostring(i)
		output[#output + 1] = ", "
		output[#output + 1] = tostring(identifiers[rule.type])
		output[#output + 1] = ", "
		output[#output + 1] = #rule
		output[#output + 1] = ", "
		local action = actions[rule.action]
		output[#output + 1] = tostring(action)
		output[#output + 1] = "}, "
	end
	output[#output + 1] = "};\n"
	output[#output + 1] = "static const uint8_t ProgramSymbol = "
	output[#output + 1] = tostring(identifiers["program"])
	output[#output + 1] = ";\n"
	output[#output + 1] = "static const uint8_t RuleCount = "
	output[#output + 1] = tostring(#Rules)
	output[#output + 1] = ";\n"
	print(table.concat(output, ""))
end

