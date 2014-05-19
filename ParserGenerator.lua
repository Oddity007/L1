local Tokens = {"Natural", "Identifier", "String", "Assign", "OpeningParenthesis", "ClosingParenthesis", "OpeningSquareBracket", "ClosingSquareBracket", "Comma", "Terminal", "QuestionMark", "ExclaimationMark", "SingleColon", "DoubleColon", "Yield", "ConstructorYield", "SingleDot", "DoubleDot", "TripleDot", "SingleQuote", "Dollar", "Bar", "Declare", "Construct", "Import", "Done",}

local Rules = {
	{type = "program", "openexpression", "Done", action = "return arguments[0];"},
	
	{type = "openexpression", "Declare", "Identifier", "Terminal", "openexpression", action = "return CreateDeclareNode(parser, arguments[1], arguments[3], false);"},
	{type = "openexpression", "Declare", "SingleDot", "Identifier", "Terminal", "openexpression", action = "return CreateDeclareNode(parser, arguments[2], arguments[4], true);"},
	{type = "openexpression", "assignment", action = "return arguments[0];"},
	{type = "openexpression", "constraint", action = "return arguments[0];"},
	{type = "openexpression", "option", action = "return arguments[0];"},
	{type = "openexpression", "anonymousfunction", action = "return arguments[0];"},
	{type = "openexpression", "inlineconstraint", action = "return arguments[0];"},
	{type = "openexpression", "chainedexpression", action = "return arguments[0];"},
	
	--{type = "option", "chainedexpression", "Bar", "option", action = "return CreateOptionNode(parser, arguments[0], arguments[2]);"},{type = "option", "chainedexpression", "Bar", "chainedexpression", action = "return CreateOptionNode(parser, arguments[0], arguments[2]);"},
	{type = "option", "chainedexpression", "Bar", "chainedexpression", action = "return CreateOptionNode(parser, arguments[0], arguments[2]);"},{type = "option", "chainedexpression", "Bar", "chainedexpression", action = "return CreateOptionNode(parser, arguments[0], arguments[2]);"},
	
	{type = "anonymousfunction", "Yield", "chainedexpression", action = "return CreateAnonymousFunctionNode(parser, NULL, arguments[1]);"},
	--{type = "anonymousfunction", "chainedexpression_arguments", "Yield", "anonymousfunction", action = "return CreateAnonymousFunctionNode(parser, arguments[0], arguments[2]);"},
	{type = "anonymousfunction", "chainedexpression_arguments", "Yield", "chainedexpression", action = "return CreateAnonymousFunctionNode(parser, arguments[0], arguments[2]);"},
	
	{type = "assignment", "Identifier", "Assign", "chainedexpression", "Terminal", "openexpression", action = "return CreateAssignmentNode(parser, arguments[0], NULL,  arguments[2], arguments[4], false);"},
	{type = "assignment", "Identifier", "chainedexpression_arguments", "Assign", "chainedexpression", "Terminal", "openexpression", action = "return CreateAssignmentNode(parser, arguments[0], arguments[1], arguments[3], arguments[5], false);"},
	
	{type = "assignment", "SingleDot", "Identifier", "Assign", "chainedexpression", "Terminal", "openexpression", action = "return CreateAssignmentNode(parser, arguments[1], NULL,  arguments[3], arguments[5], true);"},
	{type = "assignment", "SingleDot", "Identifier", "chainedexpression_arguments", "Assign", "chainedexpression", "Terminal", "openexpression", action = "return CreateAssignmentNode(parser, arguments[1], arguments[2],  arguments[4], arguments[6], true);"},
	
	{type = "constraint", "chainedexpression", "DoubleColon", "chainedexpression", "Terminal", "openexpression", action = "return CreateConstraintNode(parser, arguments[0], arguments[2], arguments[4]);"},
	
	--{type = "inlineconstraint", "chainedexpression", "SingleColon", "inlineconstraint", action = "return CreateInlineConstraintNode(parser, arguments[0], arguments[2]);"},
	{type = "inlineconstraint", "chainedexpression", "SingleColon", "chainedexpression", action = "return CreateInlineConstraintNode(parser, arguments[0], arguments[2]);"},
	
	{type = "chainedexpression", "Construct", "chainedexpression", action = "return CreateConstructNode(parser, arguments[1]);"},
	{type = "chainedexpression", "Import", "String", action = "return CreateImportNode(parser, arguments[1]);"},
	{type = "chainedexpression", "closedexpression", "chainedexpression_metaarguments", action = "return CreateMetacallNode(parser, arguments[0], arguments[1]);"},
	{type = "chainedexpression", "closedexpression", "chainedexpression_arguments", action = "return CreateCallNode(parser, arguments[0], arguments[1]);"},
	{type = "chainedexpression", "closedexpression", action = "return arguments[0];"},
	
	{type = "chainedexpression_arguments", "closedexpression", "chainedexpression_arguments", action = "return Cons(parser, arguments[0], arguments[1]);"},
	{type = "chainedexpression_arguments", "closedexpression", action = "return Cons(parser, arguments[0], NULL);"},
	
	{type = "chainedexpression_metaarguments", "SingleDot", "Identifier", "chainedexpression_metaarguments", action = "return Cons(parser, arguments[1], arguments[2]);"},
	{type = "chainedexpression_metaarguments", "SingleDot", "Identifier", action = "return Cons(parser, arguments[1], NULL);"},
	
	--{type = "closedexpression", "unannotatedclosedexpression", "closedexpression_args", action = "return CreateMetacallNode(parser, arguments[0], arguments[1]);"},
	{type = "closedexpression", "unannotatedclosedexpression", action = "return arguments[0];"},
	
	{type = "unannotatedclosedexpression", "Dollar", "Identifier", action = "return CreateEvalNode(parser, arguments[1]);"},
	{type = "unannotatedclosedexpression", "Identifier", action = "return arguments[0];"},
	{type = "unannotatedclosedexpression", "Natural", action = "return arguments[0];"},
	{type = "unannotatedclosedexpression", "String", action = "return arguments[0];"},
	{type = "unannotatedclosedexpression", "OpeningParenthesis", "openexpression", "ClosingParenthesis", action = "return arguments[1];"},
	{type = "unannotatedclosedexpression", "OpeningSquareBracket", "ClosingSquareBracket", action = "return CreateListNode(parser, NULL, NULL);"},
	{type = "unannotatedclosedexpression", "OpeningSquareBracket", "list_body", "TripleDot", "chainedexpression", "ClosingSquareBracket", action = "return CreateListNode(parser, arguments[1], arguments[3]);"},
	{type = "unannotatedclosedexpression", "OpeningSquareBracket", "list_body", "ClosingSquareBracket", action = "return CreateListNode(parser, arguments[1], NULL);"},
	
	{type = "list_body", "chainedexpression", "Comma", "list_body", action = "return Cons(parser, arguments[0], arguments[2]);"},
	{type = "list_body", "chainedexpression", "Comma", action = "return Cons(parser, arguments[0], NULL);"},
	{type = "list_body", "chainedexpression", action = "return Cons(parser, arguments[0], NULL);"},
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

