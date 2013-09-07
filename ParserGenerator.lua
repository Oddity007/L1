local Tokens = {"Natural", "Identifier", "String", "Assign", "OpeningParenthesis", "ClosingParenthesis", "OpeningSquareBracket", "ClosingSquareBracket", "Comma", "Terminal", "QuestionMark", "Done",}
local Rules = {
	{type = "program", "openexpression", "Done", action = "return arguments[0];"},
	
	{type = "openexpression", "branch", action = "return arguments[0];"},
	{type = "openexpression", "assignment", action = "return arguments[0];"},
	{type = "openexpression", "chainedexpression", action = "return arguments[0];"},
	
	{type = "branch", "chainedexpression", "QuestionMark", "chainedexpression", "Terminal", "openexpression", action = "return CreateBranchNode(parser, arguments[0], arguments[2], arguments[4]);"},
	
	{type = "assignment", "closedexpression", "assignment_arguments", "Assign", "chainedexpression", "Terminal", "openexpression", action = "return CreateAssignmentNode(parser, arguments[0], arguments[1], arguments[3], arguments[5]);"},
	
	{type = "assignment_arguments", "assignment_target", "assignment_arguments", action = "return Cons(parser, arguments[0], arguments[1]);"},
	{type = "assignment_arguments", action = "return NULL;"},
	
	{type = "assignment_target", "Identifier", action = "return arguments[0];"},
	{type = "assignment_target", "OpeningSquareBracket", "assignment_target_list_body", "ClosingSquareBracket", action = "return CreateListNode(parser, arguments[1]);"},
	
	{type = "assignment_target_list_body", "assignment_target", "Comma", "assignment_target_list_body", action = "return Cons(parser, arguments[0], arguments[2]);"},
	{type = "assignment_target_list_body", "assignment_target", action = "return Cons(parser, arguments[0], NULL);"},
	{type = "assignment_target_list_body", action = "return NULL;"},
	
	{type = "chainedexpression", "closedexpression", "chainedexpression_arguments", action = "return CreateCallNode(parser, arguments[0], arguments[1]);"},
	{type = "chainedexpression", "closedexpression", action = "return arguments[0];"},
	
	{type = "chainedexpression_arguments", "closedexpression", "chainedexpression_arguments", action = "return Cons(parser, arguments[0], arguments[1]);"},
	{type = "chainedexpression_arguments", "closedexpression", action = "return Cons(parser, arguments[0], NULL);"},
	
	{type = "closedexpression", "Identifier", action = "return arguments[0];"},
	{type = "closedexpression", "Natural", action = "return arguments[0];"},
	{type = "closedexpression", "String", action = "return arguments[0];"},
	{type = "closedexpression", "OpeningParenthesis", "openexpression", "ClosingParenthesis", action = "return arguments[1];"},
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
		output[#output + 1] = "static uint8_t rule_string_"
		output[#output + 1] = tostring(i)
		output[#output + 1] = "["
		output[#output + 1] = tostring(#rule)
		output[#output + 1] = "] = {"
		for _, symbol in ipairs(rule) do
			output[#output + 1] = tostring(identifiers[symbol])
			output[#output + 1] = ", "
		end
		output[#output + 1] = "};\n"
	end
	--Generate the action table
	local actions = {}
	local actionID = 0
	--Generate the action handler
	output[#output + 1] = "static void* HandleAction(L1Parser* parser, void* arguments[], Rule rule)\n{\n\tswitch(rule.action)\n\t{\n"
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
	output[#output + 1] = "static Rule Rules[] = {"
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
	output[#output + 1] = "uint8_t ProgramSymbol = "
	output[#output + 1] = tostring(identifiers["program"])
	output[#output + 1] = ";\n"
	output[#output + 1] = "uint8_t RuleCount = "
	output[#output + 1] = tostring(#Rules)
	output[#output + 1] = ";\n"
	print(table.concat(output, ""))
end

