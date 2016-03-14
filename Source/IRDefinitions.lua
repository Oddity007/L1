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
	Slot("Universe"),
	Slot("Unknown"),

	Slot("Recursive", Argument("argumentType", "localref"), Argument("result", "localref")),
	--Slot("Namespace", Argument("argumentType", "localref"), Argument("result", "localref")),
	
	Slot("Lambda", Argument("argumentType", "localref"), Argument("result", "localref")),
	Slot("Forall", Argument("argumentType", "localref"), Argument("result", "localref")),
	Slot("Call", Argument("callee", "localref"), Argument("argument", "localref")),
	
	Slot("Pair", Argument("first", "localref"), Argument("second", "localref")),
	Slot("PairType", Argument("first", "localref"), Argument("second", "localref")),
	Slot("Project", Argument("pair", "localref"), Argument("index", "uint16")),
	
	Slot("Argument", Argument("index", "uint16"), Argument("type", "localref")),

	Slot("RawData32Extended", Argument("extension", "localref"), Argument("high", "uint16"), Argument("low", "uint16")),
	Slot("RawData32", Argument("tag", "uint16"), Argument("high", "uint16"), Argument("low", "uint16")),

	Slot("ADT", Argument("tag", "uint16")),
	Slot("ExtendADT", Argument("adt", "localref"), Argument("name", "localref"), Argument("constructor", "localref")),
	Slot("Constructed", Argument("adt", "localref"), Argument("name", "localref"), Argument("captures", "localref")),
	
	Slot("Lookup", Argument("namespace", "localref"), Argument("name", "localref")),
	
	Slot("BeginMatch", Argument("value", "localref"), Argument("type", "localref")),
	Slot("MatchCase", Argument("match", "localref"), Argument("name", "localref"), Argument("handler", "localref")),
	Slot("EndMatch", Argument("match", "localref"), Argument("resultType", "localref")),
	Slot("MatchSuccess", Argument("result", "localref")),
	Slot("MatchFailure", Argument("value", "localref")),
	

	Slot("TypeOf", Argument("root", "localref")),
	Slot("Normalize", Argument("root", "localref")),
	Slot("Substitute", Argument("root", "localref"), Argument("argument", "localref")),
	--Slot("Foreign", Argument("name", "localref")),
}
