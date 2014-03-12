local L1GenerateIR = require "L1GenerateIR"

-- Print contents of `tbl`, with indentation.
-- `indent` sets the initial level of indentation.
local function tprint (tbl, indent)
  if type(tbl) ~= "table" then
    formatting = string.rep("  ", indent)
    print(formatting .. tostring(tbl))
    return
  end
  if not indent then indent = 0 end
  for k, v in pairs(tbl) do
    formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      print(formatting)
      tprint(v, indent+1)
    else
      print(formatting .. tostring(v))
    end
  end
end

local function FileAsString(filename)
	local f = io.open(filename, "r")
	local s = f:read("*all")
	f:close()
	return s
end

local json = require "dkjson"
local ast = json.decode(FileAsString("build/sample_ast.json"))
print "JSON:"
assert(type(ast) == "table")
tprint(ast, 0)
local result, roughOps = L1GenerateIR(ast)
tprint(roughOps, 0)
print ("Result: " .. tostring(result))