local OutputDirectory = ...

require "IRDefinitions"

local function OutputSlotDescriptions()
	file = io.open(OutputDirectory .. "/L1IRSlotDescriptions", "w")
	file:write("#define NodeMask(i) ((uint8_t) 1 << i)\n\n")
	file:write("static const uint8_t SlotDescriptions[L1IRSlotTypeLast + 1] = \n")
	file:write("{\n")
	for _, definition in ipairs(SlotTypeDefinitions) do
		local type = definition.type
		file:write("\t[L1IRSlotType" .. type .. "] = ")
		if definition.isImplicitRoot then
			file:write("NodeMask(3) | ")
		end
		for i, operand in ipairs(definition) do
			if operand.type == "localref" then
				file:write("NodeMask(" .. tostring(i - 1) .. ") | ")
			end
		end
		file:write("0,\n")
	end
	file:write("};\n")
	file:write("\n")
	file:write [[
static bool SlotTypeArgumentIsLocalAddress(L1IRSlotType type, uint8_t i)
{
	assert (i < 3);
	return 0 not_eq (SlotDescriptions[(uint8_t) type] & NodeMask(i));
}

static bool IsImplicitRoot(L1IRSlotType type)
{
	return 0 not_eq (SlotDescriptions[(uint8_t) type] & NodeMask(3));
}

]]
	file:write("#undef NodeArgumentMask\n")
	file:close()
end

local function OutputSlotAccessors()
	file = io.open(OutputDirectory .. "/L1IRSlotAccessors", "w")
	for _, definition in ipairs(SlotTypeDefinitions) do
		local type = definition.type
		for i, operand in ipairs(definition) do
			file:write("static uint16_t " .. type .. "_" .. operand.name .. "(L1IRSlot slot)")
			file:write([[
{
	assert(L1IRExtractSlotType(slot) == L1IRSlotType]] .. type .. [[);
	return L1IRExtractSlotOperand(slot, ]] .. tostring(i - 1) .. [[);
}

]])
		end
	end
	file:close()
end

local function OutputSlotDebugInfo()
	file = io.open(OutputDirectory .. "/L1IRSlotDebugInfo", "w")
	file:write("static char* L1IRSlotTypeAsString(L1IRSlotType type)\n{\n\tswitch(type)\n\t{\n")
	for _, definition in ipairs(SlotTypeDefinitions) do
		local type = definition.type
		file:write("\t\tcase L1IRSlotType" .. type .. ": return \"" .. type .. "\";\n")
	end
	file:write("\t}\n}\n")
	file:close()
end

local function OutputSlotTypeDefinitions()
	file = io.open(OutputDirectory .. "/L1IRSlotTypeDefinitions", "w")
	file:write("enum L1IRSlotType\n{\n")
	local lastDefinition = nil
	for _, definition in ipairs(SlotTypeDefinitions) do
		lastDefinition = definition
		local type = definition.type
		file:write("\tL1IRSlotType" .. type .. ",\n")
	end
	assert(lastDefinition)
	file:write("\tL1IRSlotTypeLast = " .. "L1IRSlotType" .. lastDefinition.type .. "\n};\ntypedef enum L1IRSlotType L1IRSlotType;\n")
	file:close()
end

do
	OutputSlotDescriptions()
	OutputSlotAccessors()
	OutputSlotDebugInfo()
	OutputSlotTypeDefinitions()
end
