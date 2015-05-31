local function Slot(type, argument0, argument1, argument2)
	return {type = type; argument0, argument1, argument2}
end

local function RootSlot(type, argument0, argument1, argument2)
	return {type = type, isImplicitRoot = true; argument0, argument1, argument2}
end

local function Argument(name, type)
	return {type = type, name = name}
end

--Must also update L1IR.h
SlotTypeDefinitions = {
	--
	RootSlot("Error", Argument("type", "errortype")),
	
	Slot("Unit"),
	Slot("UnitType"),
	Slot("Universe", Argument("index", "uint16")),
	
	Slot("Lambda", Argument("captured", "localref"), Argument("prototype", "globalref")),
	Slot("Pi", Argument("captured", "localref"), Argument("prototype", "globalref")),
	Slot("Call", Argument("callee", "localref"), Argument("argument", "localref")),
	
	Slot("Pair", Argument("first", "localref"), Argument("second", "localref")),
	Slot("Sigma", Argument("captured", "localref"), Argument("prototype", "globalref")),
	Slot("ProjectPair", Argument("pair", "localref"), Argument("index", "uint16")),
	
	Slot("CallCapture", Argument("captured", "localref"), Argument("captures", "localref")),
	
	Slot("Self"),
	Slot("Argument", Argument("index", "uint16"), Argument("type", "localref")),
	Slot("Captured", Argument("index", "uint16")),

	Slot("RawData32Extended", Argument("extension", "localref"), Argument("high", "uint16"), Argument("low", "uint16")),
	Slot("RawData48", Argument("high", "uint16"), Argument("mid", "uint16"), Argument("low", "uint16")),

	Slot("ADT", Argument("captured", "localref"), Argument("prototype", "globalref")),
	Slot("ExtendADT", Argument("adt", "localref"), Argument("tag", "uint16"), Argument("argumentType", "localref")),
	Slot("Constructor", Argument("adt", "localref"), Argument("tag", "uint16"), Argument("argumentType", "localref")),
	Slot("ConstructorOf", Argument("adt", "localref"), Argument("tag", "uint16")),
	Slot("ConstructedOf", Argument("adt", "localref"), Argument("tag", "uint16"), Argument("argument", "localref")),
	Slot("Deconstruction", Argument("default", "localref"), Argument("tag", "uint16"), Argument("callee", "localref")),
	Slot("DeconstructionSuccess", Argument("result", "localref")),
	Slot("BeginDeconstruction", Argument("constructed", "localref"), Argument("type", "localref")),
	Slot("EndDeconstruction", Argument("deconstruction", "localref")),
}
