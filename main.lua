function table.val_to_str ( v )
  if "string" == type( v ) then
    v = string.gsub( v, "\n", "\\n" )
    if string.match( string.gsub(v,"[^'\"]",""), '^"+$' ) then
      return "'" .. v .. "'"
    end
    return '"' .. string.gsub(v,'"', '\\"' ) .. '"'
  else
    return "table" == type( v ) and table.tostring( v ) or
      tostring( v )
  end
end

function table.key_to_str ( k )
  if "string" == type( k ) and string.match( k, "^[_%a][_%a%d]*$" ) then
    return k
  else
    return "[" .. table.val_to_str( k ) .. "]"
  end
end

function table.tostring( tbl )
  local result, done = {}, {}
  for k, v in ipairs( tbl ) do
    table.insert( result, table.val_to_str( v ) )
    done[ k ] = true
  end
  for k, v in pairs( tbl ) do
    if not done[ k ] then
      table.insert( result,
        table.key_to_str( k ) .. "=" .. table.val_to_str( v ) )
    end
  end
  return "{" .. table.concat( result, ", " ) .. "}"
end



local Lex = require "Lex"
local Parse = require "Parse"
--local GenerateHLIR = require "GenerateHLIR"
--local EvaluateHLIR = require "EvaluateHLIR"
local GenerateFIR = require "GenerateFIR"

local inputFile = assert(io.open("sample1.l1", "r"))
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
print(table.tostring(fir))
--print("HLIR:")
--local hlir = GenerateHLIR(astRootNode)
--print(table.tostring(hlir))
--local f = {"call", {"lambda", "a", {"modulo", {"integer", "100"}, {"get", "a"}}}, {"integer", "30"}}
--{"select", {"equal", f, {"integer", "10"}}, {"boolean", "false"}, {"boolean", "true"}}
--print("HLIR Evaluation:")
--local evaluatedResult = EvaluateHLIR(hlir)
--print(table.tostring(evaluatedResult))
--print("{" .. table.concat(evaluatedResult, ", ") .. "}")