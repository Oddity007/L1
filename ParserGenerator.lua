local Tokens = {"Natural", "Identifier", "String", "Assign", "OpeningParenthesis", "ClosingParenthesis", "OpeningSquareBracket", "ClosingSquareBracket", "Comma", "Terminal", "QuestionMark", "Done",}
local Rules = {
	{type = "program", "openexpression", "Done", action = "return arguments[0];"},
	
	{type = "openexpression", "branch", action = ""},
	{type = "openexpression", "assignment", action = ""},
	{type = "openexpression", "chainedexpression", action = ""},
	
	{type = "branch", "chainedexpression", "QuestionMark", "chainedexpression", "Terminal", "openexpression", action = ""},
	
	{type = "assignment", "closedexpression", "assignment_arguments", "Assign", "chainedexpression", "Terminal", "openexpression", action = ""},
	
	{type = "assignment_arguments", "assignment_target", "assignment_arguments", action = ""},
	{type = "assignment_arguments", action = ""},
	
	{type = "assignment_target", "Identifier", action = ""},
	{type = "assignment_target", "OpeningSquareBracket", "assignment_target_list_body", "ClosingSquareBracket", action = ""},
	
	{type = "assignment_target_list_body", "assignment_target", "Comma", "assignment_target_list_body", action = ""},
	{type = "assignment_target_list_body", "assignment_target", "Comma", action = ""},
	{type = "assignment_target_list_body", "assignment_target", action = ""},
	
	{type = "chainedexpression", "closedexpression", "chainedexpression", action = ""},
	{type = "chainedexpression", "closedexpression", action = ""},
	
	{type = "closedexpression", "Identifier", action = ""},
	{type = "closedexpression", "Natural", action = ""},
	{type = "closedexpression", "String", action = ""},
	{type = "closedexpression", "OpeningParenthesis", "openexpression", "ClosingParenthesis", action = ""},
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
	local output = {"#include <stdint.h>\n"}
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
		output[#output + 1] = "\t\tcase "
		local action = actions[rule.action]
		if not action then
			action = actionID
			actions[rule.action] = action
			actionID = actionID + 1
		end
		output[#output + 1] = tostring(action)
		output[#output + 1] = ":{"
		output[#output + 1] = rule.action
		output[#output + 1] = "} break;\n"
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
	print(table.concat(output, ""))
end

