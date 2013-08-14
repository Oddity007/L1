local TableToString = require "TableToString"

local Lex = require "Lex"
local Parse = require "Parse"
--local GenerateHLIR = require "GenerateHLIR"
--local EvaluateHLIR = require "EvaluateHLIR"
local GenerateFIR = require "GenerateFIR"

require "Test"

--[[local inputFile = assert(io.open("sample1.l1", "r"))
local input = inputFile:read("*all")
print("TEXT:")
print(input)
print("TOKENS:")
local tokens = Lex(input)
for _, v in ipairs(tokens) do
	print(v.type, v.data, v.lineNumber)
end
print("AST:")
local astRootNode = Parse(tokens)
print(table.tostring(astRootNode))
print("FIR:")
local fir = GenerateFIR(astRootNode)
print(table.tostring(fir))]]

--print("HLIR:")
--local hlir = GenerateHLIR(astRootNode)
--print(table.tostring(hlir))
--local f = {"call", {"lambda", "a", {"modulo", {"integer", "100"}, {"get", "a"}}}, {"integer", "30"}}
--{"select", {"equal", f, {"integer", "10"}}, {"boolean", "false"}, {"boolean", "true"}}
--print("HLIR Evaluation:")
--local evaluatedResult = EvaluateHLIR(hlir)
--print(table.tostring(evaluatedResult))
--print("{" .. table.concat(evaluatedResult, ", ") .. "}")