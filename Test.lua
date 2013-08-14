local Lex = require "Lex"
local Parse = require "Parse"
local TableToString = require "TableToString"
local Tests = {
	{input = "a = 1; a"},
	{input = "a = __add 1 1; a"},
	{input = "a = 1 + 1; a"},
}

local function FailTest(testName, message)
	error("Failed test " .. tostring(testName) .. " " .. (message or ""), 1)
end

for testName, test in pairs(Tests) do
	print("Running test " .. tostring(testName))
	
	print("Lex(...) = ")
	local status, returnValue
	status, returnValue = pcall(Lex, test.input)
	if not status then FailTest(testName, returnValue) end
	local tokens = returnValue
	print(TableToString(tokens))
	
	print("Parse(...) = ")
	status, returnValue = pcall(Parse, tokens)
	if not status then FailTest(testName, returnValue) end
	local astRootNode = returnValue
	print(TableToString(astRootNode))
end