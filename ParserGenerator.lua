local Rules = {}

--[[
bool program(Token* tokens, uint64_t tokenCount, void** data, uint64_t* tokenIndex)

]]

--[[do
	local outputStatements = {}
	for _, rule in ipairs(Rules) do
		outputStatements[#outputStatements + 1] = "int "
		outputStatements[#outputStatements + 1] = rule.type
		outputStatements[#outputStatements + 1] = "(L1ParserLexedToken* tokens, uint64_t tokenCount, void** returnedData, uint64_t* tokenIndex)\n{"
		outputStatements[#outputStatements + 1] = "\tvoid* data = NULL;\n"
		for _, symbol in ipairs(rule) do
			outputStatements[#outputStatements + 1] = "\t"
			outputStatements[#outputStatements + 1] = "if(0 == "
			outputStatements[#outputStatements + 1] = symbol
			outputStatements[#outputStatements + 1] = "(tokens, tokenCount, & data, tokenIndex)) return false;\n"
		end
		outputStatements[#outputStatements + 1] = "\treturn true;"
		outputStatements[#outputStatements + 1] = "\treturn true;"
		outputStatements[#outputStatements + 1] = "}"
	end
end]]