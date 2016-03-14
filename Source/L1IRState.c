#include "L1IRState.h"
#include <iso646.h>

///State Boilerplate

typedef struct ExternalSlotReference ExternalSlotReference;
struct ExternalSlotReference
{
	bool isActive;
	L1IRAddress slotRef;
};

void L1IRStateInitialize(L1IRState* self)
{
	L1ArrayInitialize(& self->slots);
	L1ArrayInitialize(& self->externalSlotRefs);
	//L1ArrayInitialize(& self->slotReferenceCounts);
}

void L1IRStateDeinitialize(L1IRState* self)
{
	L1ArrayDeinitialize(& self->slots);
	L1ArrayDeinitialize(& self->externalSlotRefs);
	//L1ArrayDeinitialize(& self->slotReferenceCounts);
}

L1IRStateExternalSlotRef L1IRStateAcquireExternalSlotRef(L1IRState* self, L1IRAddress slotRef)
{
	ExternalSlotReference reference;
	reference.isActive = true;
	reference.slotRef = slotRef;
	
	
	ExternalSlotReference* references = L1ArrayGetElements(& self->externalSlotRefs);
	size_t referenceCount = L1ArrayGetElementCount(& self->externalSlotRefs);
	
	for (size_t i = 0; i < referenceCount; i++)
	{
		if (references[i].isActive)
		{
			references[i] = reference;
			return i;
		}
	}
	
	L1ArrayPush(& self->externalSlotRefs, & reference, sizeof(ExternalSlotReference));
	return referenceCount;
}

void L1IRStateReleaseExternalSlotRef(L1IRState* self, L1IRStateExternalSlotRef ref)
{
	ExternalSlotReference* references = L1ArrayGetElements(& self->externalSlotRefs);
	size_t referenceCount = L1ArrayGetElementCount(& self->externalSlotRefs);
	assert(ref < referenceCount);
	assert(references[ref].isActive);
	references[ref].isActive = false;
	
	for (size_t i = referenceCount; i-- > 0;)
	{
		if (not references[i].isActive)
			L1ArrayPop(& self->externalSlotRefs, NULL, sizeof(ExternalSlotReference));
	}
	
	//L1ArrayPop(& self->externalSlotRefs, & reference, sizeof(ExternalSlotReference));
	//if (L1ArrayGetElementCount(& slot->externalSlotRefs) == )
}

L1IRAddress L1IRStateGetExternalSlotRefCurrentSlotRef(L1IRState* self, L1IRStateExternalSlotRef ref)
{
	ExternalSlotReference* references = L1ArrayGetElements(& self->externalSlotRefs);
	size_t referenceCount = L1ArrayGetElementCount(& self->externalSlotRefs);
	assert(ref < referenceCount);
	assert(references[ref].isActive);
	return references[ref].slotRef;
}

/*L1IRAddress L1IRStateRetainSlot(L1IRState* self, L1IRAddress slotRef)
{
	size_t* referenceCounts = L1ArrayGetElements(& self->slotReferenceCounts);
	size_t referenceCountCount = L1ArrayGetElementCount(& self->slotReferenceCounts);
	assert(slotRef < referenceCountCount);
	referenceCounts[slotRef]++;
	return slotRef;
}

void L1IRStateReleaseSlot(L1IRState* self, L1IRAddress slotRef)
{
	size_t* referenceCounts = L1ArrayGetElements(& self->slotReferenceCounts);
	size_t referenceCountCount = L1ArrayGetElementCount(& self->slotReferenceCounts);
	assert(slotRef < referenceCountCount);
	assert(referenceCounts[slotRef] > 0);
	referenceCounts[slotRef]--;
}*/

#include <stdio.h>
#include "L1IRSlotDebugInfo"

L1IRAddress L1IRStateCreateSlotRaw(L1IRState* self, L1IRSlot slot)
{
	const L1IRSlot* slots = L1ArrayGetElements(& self->slots);
	size_t slotCount = L1ArrayGetElementCount(& self->slots);
	
	for (L1IRAddress i = 0; i < slotCount; i++)
		if (slots[i] == slot)
			return i;
	
	//fprintf(stdout, "created #%u: %s (%u, %u, %u)\n", (unsigned) L1ArrayGetElementCount(& self->slots), L1IRSlotTypeAsString(L1IRExtractSlotType(slot)), (unsigned) L1IRExtractSlotOperand(slot, 0), (unsigned) L1IRExtractSlotOperand(slot, 1), (unsigned) L1IRExtractSlotOperand(slot, 2));
	
	//L1ArrayPush(& self->slots, (size_t[]){0}, sizeof(L1IRSlot));
	L1ArrayPush(& self->slots, & slot, sizeof(L1IRSlot));
	return L1ArrayGetElementCount(& self->slots) - 1;
}

#include "L1IRSlotDescriptions"
//#include <stdio.h>

static L1IRSlot GetSlot(L1IRState* self, L1IRAddress address)
{
	//printf("%s(_, %u): slot count %u\n", __func__, (unsigned) address, (unsigned) L1ArrayGetElementCount(& self->slots));
	assert(address < L1ArrayGetElementCount(& self->slots));
	const L1IRSlot* slots = L1ArrayGetElements(& self->slots);
	return slots[address];
}

static bool SlotDependsOnSlot(L1IRState* self, L1IRAddress dependentLocalAddress, L1IRAddress dependencyLocalAddress)
{
	if (dependencyLocalAddress == dependentLocalAddress) return true;
	if (dependencyLocalAddress > dependentLocalAddress) return false;
	const L1IRSlot* slots = L1ArrayGetElements(& self->slots);
	L1IRSlot dependentSlot = slots[dependentLocalAddress];
	L1IRSlotType type = L1IRExtractSlotType(dependentSlot);
	for (uint8_t i = 0; i < 3; i++)
		if (SlotTypeArgumentIsLocalAddress(type, i))
			if (SlotDependsOnSlot(self, L1IRExtractSlotOperand(dependentSlot, i), dependencyLocalAddress))
				return true;
	return false;
}

L1IRSlot L1IRStateGetSlot(L1IRState* self, L1IRAddress address)
{
	return GetSlot(self, address);
}

#include "L1IRSlotAccessors"

void L1IRStateCollectGarbage(L1IRState* self)
{
	ExternalSlotReference* references = L1ArrayGetElements(& self->externalSlotRefs);
	size_t referenceCount = L1ArrayGetElementCount(& self->externalSlotRefs);
	size_t slotCount = L1ArrayGetElementCount(& self->slots);
	L1IRSlot* slots = L1ArrayGetElements(& self->slots);
	
	L1IRAddress* remaps = malloc(sizeof(L1IRAddress) * slotCount);
	
	//Do external -> internal tag propagation
	for (size_t i = 0; i < referenceCount; i++)
	{
		if (not references[i].isActive)
			continue;
		assert(references[i].slotRef < slotCount);
		L1IRUpdateSlotAnnotationFlags(slots + references[i].slotRef, L1IRSlotAnnotationFlagIsConfirmedReachable, true);
	}
	
	//Propagate tags
	for (size_t i = slotCount; i-- > 0; )
	{
		L1IRSlot slot = slots[i];
		L1IRSlotType type = L1IRExtractSlotType(slot);
		uint8_t annotation = L1IRExtractSlotAnnotation(slot);
		if (not (annotation & L1IRSlotAnnotationFlagIsConfirmedReachable))
			continue;
		for (uint8_t j = 0; j < 3; j++)
			if (SlotTypeArgumentIsLocalAddress(type, j))
				L1IRUpdateSlotAnnotationFlags(slots + L1IRExtractSlotOperand(slot, j), L1IRSlotAnnotationFlagIsConfirmedReachable, true);
	}
	
	//Compact heap
	size_t newSlotCount = 0;
	for (size_t i = 0; i < slotCount; i++)
	{
		L1IRSlot slot = slots[i];
		uint8_t annotation = L1IRExtractSlotAnnotation(slot);
		if (not (annotation & L1IRSlotAnnotationFlagIsConfirmedReachable))
			continue;
		L1IRSlotType type = L1IRExtractSlotType(slot);
		uint16_t operands[3];
		for (uint8_t j = 0; j < 3; j++)
		{
			operands[j] = L1IRExtractSlotOperand(slot, j);
			if (SlotTypeArgumentIsLocalAddress(type, j))
			{
				assert(operands[j] < i);
				operands[j] = remaps[operands[j]];
			}
		}
		slots[newSlotCount] = L1IRMakeSlot(type, operands[0], operands[1], operands[2]);
		remaps[i] = newSlotCount;
		newSlotCount++;
	}
	
	//Remap external references
	for (size_t i = 0; i < referenceCount; i++)
	{
		if (not references[i].isActive)
			continue;
		assert(references[i].slotRef < slotCount);
		references[i].slotRef = remaps[references[i].slotRef];
	}
	
	free(remaps);
	
	L1ArraySetElementCount(& self->slots, newSlotCount, sizeof(L1IRSlot));
}

static L1IRAddress TypeOf(L1IRState* self, L1IRAddress slotRef);
static L1IRAddress Normalize(L1IRState* self, L1IRAddress slotRef);
static L1IRAddress Substitute(L1IRState* self, L1IRAddress slotRef, L1IRAddress argumentRef);
static L1IRAddress SubstituteWithCaptureChain(L1IRState* self, L1IRAddress slotRef, L1IRAddress capturesRef);

static L1IRAddress CreatePair(L1IRState* self, L1IRAddress firstRef, L1IRAddress secondRef)
{
	return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypePair, firstRef, secondRef, 0));
}

static L1IRAddress CreateUnit(L1IRState* self)
{
	return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypeUnit, 0, 0, 0));
}

static L1IRAddress CreateError(L1IRState* self, L1IRErrorType errorType)
{
	abort();
	return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypeError, errorType, 0, 0));
}

static bool AreEqual(L1IRState* self, L1IRAddress value1Ref, L1IRAddress value2Ref)
{
	value1Ref = Normalize(self, value1Ref);
	value2Ref = Normalize(self, value2Ref);
	return value1Ref == value2Ref;
}

static bool IsOfType(L1IRState* self, L1IRAddress valueRef, L1IRAddress typeRef)
{
	//return true;
	if (L1IRExtractSlotType(GetSlot(self, typeRef)) == L1IRSlotTypeUnknown)
		return true;
	return AreEqual(self, TypeOf(self, valueRef), typeRef);
}

static bool IsRawData(L1IRState* self, L1IRAddress slotRef)
{
	L1IRSlot slot = GetSlot(self, slotRef);
	L1IRSlotType slotType = L1IRExtractSlotType(slot);
	return (slotType == L1IRSlotTypeRawData32) or (slotType == L1IRSlotTypeRawData32Extended);
}

static bool IsValue(L1IRState* self, L1IRAddress slotRef)
{
	L1IRSlot slot = GetSlot(self, slotRef);
	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeNormalize:
		case L1IRSlotTypeSubstitute:
		case L1IRSlotTypeForall:
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypeRecursive:
		case L1IRSlotTypeLambda:
		case L1IRSlotTypeArgument:
		case L1IRSlotTypeConstructed:
		case L1IRSlotTypeCall:
		case L1IRSlotTypePair:
		case L1IRSlotTypeProject:
		case L1IRSlotTypeLookup:
		case L1IRSlotTypeEndMatch:
		case L1IRSlotTypeError:
			return true;
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeRawData32Extended:
		case L1IRSlotTypeBeginMatch:
		case L1IRSlotTypeMatchCase:
		case L1IRSlotTypeMatchFailure:
		case L1IRSlotTypeMatchSuccess:
			return false;
	}
	return false;
}

/*static bool IsType(L1IRState* self, L1IRAddress slotRef)
{
	L1IRSlot slot = GetSlot(self, slotRef);
	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypeForall:
			return true;
		case L1IRSlotTypeUnit:
			return false;
		case L1IRSlotTypeNormalize:
		case L1IRSlotTypeSubstitute:
		case L1IRSlotTypeRecursive:
		case L1IRSlotTypeLambda:
		case L1IRSlotTypeArgument:
		case L1IRSlotTypeConstructed:
		case L1IRSlotTypeCall:
		case L1IRSlotTypePair:
		case L1IRSlotTypeProject:
		case L1IRSlotTypeLookup:
		case L1IRSlotTypeEndMatch:
			return true;
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeError:
		case L1IRSlotTypeRawData32Extended:
		case L1IRSlotTypeBeginMatch:
		case L1IRSlotTypeMatchCase:
		case L1IRSlotTypeMatchFailure:
		case L1IRSlotTypeMatchSuccess:
			return false;
	}
	return false;
}*/

static bool IsPotentiallyADT(L1IRState* self, L1IRAddress slotRef)
{
	L1IRSlot slot = GetSlot(self, slotRef);
	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypeRecursive:
		case L1IRSlotTypeNormalize:
		case L1IRSlotTypeSubstitute:
		case L1IRSlotTypeArgument:
		case L1IRSlotTypeCall:
		case L1IRSlotTypeProject:
		case L1IRSlotTypeEndMatch:
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeError:
			return true;
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeForall:
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeLambda:
		case L1IRSlotTypeConstructed:
		case L1IRSlotTypePair:
		case L1IRSlotTypeLookup:
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeRawData32Extended:
		case L1IRSlotTypeBeginMatch:
		case L1IRSlotTypeMatchCase:
		case L1IRSlotTypeMatchFailure:
		case L1IRSlotTypeMatchSuccess:
			return false;
	}
	return false;
}

static bool IsMatch(L1IRState* self, L1IRAddress slotRef)
{
	L1IRSlot slot = GetSlot(self, slotRef);
	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypeRecursive:
		case L1IRSlotTypeNormalize:
		case L1IRSlotTypeSubstitute:
		case L1IRSlotTypeArgument:
		case L1IRSlotTypeCall:
		case L1IRSlotTypeProject:
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeForall:
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeLambda:
		case L1IRSlotTypeConstructed:
		case L1IRSlotTypePair:
		case L1IRSlotTypeLookup:
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeError:
		case L1IRSlotTypeRawData32Extended:
			return false;
		case L1IRSlotTypeBeginMatch:
		case L1IRSlotTypeMatchCase:
		case L1IRSlotTypeMatchFailure:
		case L1IRSlotTypeMatchSuccess:
			return true;
	}
	return false;
}

static bool IsValid(L1IRState* self, L1IRAddress slotRef);

static bool IsValidSlot(L1IRState* self, L1IRSlot slot)
{
	//L1IRSlot slot = GetSlot(self, slotRef);
	L1IRSlot slotType = L1IRExtractSlotType(slot);
	
	for (uint8_t i = 0; i < 3; i++)
	{
		if (SlotTypeArgumentIsLocalAddress(slotType, i))
			if (not IsValid(self, L1IRExtractSlotOperand(slot, i)))
				return false;
	}
	
	switch (slotType)
	{
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeNormalize:
		case L1IRSlotTypeSubstitute:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeADT:
		case L1IRSlotTypeForall:
		case L1IRSlotTypeLambda:
		case L1IRSlotTypeRecursive:
		case L1IRSlotTypeArgument:
		case L1IRSlotTypeCall:
		case L1IRSlotTypePair:
		case L1IRSlotTypeBeginMatch:
		{
			for (uint8_t i = 0; i < 3; i++)
			{
				if (SlotTypeArgumentIsLocalAddress(slotType, i))
					if (not IsValue(self, L1IRExtractSlotOperand(slot, i)))
						return false;
			}
			return true;
		}
		case L1IRSlotTypeError:
			return Error_type(slot) <= (uint16_t) L1IRErrorTypeLast;
		case L1IRSlotTypeProject:
			return IsValue(self, Project_pair(slot)) and (Project_index(slot) < 2);
		case L1IRSlotTypeRawData32Extended:
		{
			L1IRAddress extensionRef = RawData32Extended_extension(slot);
			return IsRawData(self, extensionRef);
		}
		case L1IRSlotTypeExtendADT:
			return IsPotentiallyADT(self, ExtendADT_adt(slot)) and IsRawData(self, ExtendADT_name(slot)) and IsValue(self, ExtendADT_constructor(slot));
		case L1IRSlotTypeConstructed:
			return IsPotentiallyADT(self, Constructed_adt(slot)) and IsRawData(self, Constructed_name(slot)) and IsValue(self, Constructed_captures(slot));
		case L1IRSlotTypeLookup:
			return IsPotentiallyADT(self, Lookup_namespace(slot)) and IsRawData(self, Lookup_name(slot));
		case L1IRSlotTypeMatchCase:
			return IsMatch(self, MatchCase_match(slot)) and IsRawData(self, MatchCase_name(slot)) and IsValue(self, MatchCase_handler(slot));
		case L1IRSlotTypeEndMatch:
			return IsMatch(self, EndMatch_match(slot)) and IsValue(self, EndMatch_resultType(slot));
		case L1IRSlotTypeMatchFailure:
			return IsValue(self, MatchFailure_value(slot));
		case L1IRSlotTypeMatchSuccess:
			return IsValue(self, MatchSuccess_result(slot));
	}
	return false;
}

static bool IsValid(L1IRState* self, L1IRAddress slotRef)
{
	L1IRSlot slot = GetSlot(self, slotRef);
	return IsValidSlot(self, slot);
}

static L1IRAddress TypeOf(L1IRState* self, L1IRAddress slotRef)
{
	L1IRSlot slot = GetSlot(self, slotRef);
	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeError:
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeNormalize:
		case L1IRSlotTypeSubstitute:
			break;
		case L1IRSlotTypeUnit:
			return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypeUnitType, 0, 0, 0));
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeRawData32Extended:
		case L1IRSlotTypeRawData32:
			return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypeUnknown, 0, 0, 0));
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypeForall:
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypePairType:
			return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypeUniverse, 0, 0, 0));
		case L1IRSlotTypeRecursive:
			return Recursive_argumentType(slot);
		case L1IRSlotTypeLambda:
			return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypeForall, Lambda_argumentType(slot), TypeOf(self, Lambda_result(slot)), 0));
		case L1IRSlotTypeArgument:
			return Argument_type(slot);
		case L1IRSlotTypeConstructed:
			return Constructed_adt(slot);
		case L1IRSlotTypeCall:
			return TypeOf(self, Substitute(self, Call_callee(slot), CreatePair(self, CreateUnit(self), Call_argument(slot))));
		case L1IRSlotTypePair:
			return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypePairType, TypeOf(self, Pair_first(slot)), TypeOf(self, Pair_second(slot)), 0));
		case L1IRSlotTypeProject:
		{
			L1IRAddress pairTypeSlotRef = Normalize(self, TypeOf(self, Project_pair(slot)));
			L1IRSlot pairTypeSlot = GetSlot(self, pairTypeSlotRef);
			if (L1IRExtractSlotType(pairTypeSlot) not_eq L1IRSlotTypePairType)
				return CreateError(self, L1IRErrorTypeTypeChecking);
			return (Project_index(slot) ? PairType_second : PairType_first) (pairTypeSlot);
		}
		case L1IRSlotTypeLookup:
		{
			//To do.  Need to be able to get the type of a constructor.
			return L1IRStateCreateSlot(self, L1IRMakeSlot(L1IRSlotTypeUnknown, 0, 0, 0));
		}
		case L1IRSlotTypeBeginMatch:
		case L1IRSlotTypeMatchCase:
			return CreateError(self, L1IRErrorTypeInvalidInstruction);
		case L1IRSlotTypeEndMatch:
			return EndMatch_resultType(slot);
	}
	return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(L1IRSlotTypeTypeOf, slotRef, 0, 0));
}

static L1IRAddress Normalize(L1IRState* self, L1IRAddress slotRef)
{
	//printf("%s(%p, %u)...\n", __func__, (void*) self, (unsigned) slotRef);
	L1IRSlot slot = GetSlot(self, slotRef);
	L1IRSlotType type = L1IRExtractSlotType(slot);
	switch (type)
	{
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeError:
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypePair:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeRawData32Extended:
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypeConstructed:
		case L1IRSlotTypeArgument:
			break;
		case L1IRSlotTypeBeginMatch:
			//return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(type, BeginMatch_value(slot), BeginMatch_type(slot), 0));
		{
			L1IRAddress valueRef = Normalize(self, BeginMatch_value(slot));
			L1IRAddress typeRef = Normalize(self, BeginMatch_type(slot));
			if (not IsOfType(self, valueRef, typeRef))
				return CreateError(self, L1IRErrorTypeTypeChecking);
			return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(L1IRSlotTypeMatchFailure, valueRef, 0, 0));
		}
		case L1IRSlotTypeMatchCase:
			//return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(type, MatchCase_match(slot), MatchCase_name(slot), 0));
		{
			L1IRAddress matchRef = Normalize(self, MatchCase_match(slot));
			L1IRSlot matchSlot = GetSlot(self, matchRef);
			L1IRAddress matchNameRef = Normalize(self, MatchCase_name(slot));
			switch(L1IRExtractSlotType(matchSlot))
			{
				case L1IRSlotTypeBeginMatch:
				case L1IRSlotTypeMatchCase:
					break;
				case L1IRSlotTypeMatchFailure:
				{
					L1IRSlot constructedSlot = GetSlot(self, MatchFailure_value(matchSlot));
					//printf("Line %u: %u\n", (unsigned) __LINE__, (unsigned) MatchFailure_value(matchSlot));
					if (L1IRExtractSlotType(constructedSlot) not_eq L1IRSlotTypeConstructed)
						return CreateError(self, L1IRErrorTypeInvalidInstruction);
					//printf("Line %u: %u\n", (unsigned) __LINE__, (unsigned) MatchFailure_value(matchSlot));
					L1IRAddress constructedNameRef = Constructed_name(constructedSlot);
					//printf("Line %u: %u\n", (unsigned) __LINE__, (unsigned) MatchFailure_value(matchSlot));
					
					if (not AreEqual(self, constructedNameRef, matchNameRef))
						return matchRef;
					//printf("Line %u: %u\n", (unsigned) __LINE__, (unsigned) MatchFailure_value(matchSlot));
					
					L1IRAddress result = SubstituteWithCaptureChain(self, MatchCase_handler(slot), Constructed_captures(constructedSlot));
					
					return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(L1IRSlotTypeMatchSuccess, result, 0, 0));
				}
				case L1IRSlotTypeMatchSuccess:
					return matchRef;
				default:
					return CreateError(self, L1IRErrorTypeInvalidInstruction);
			}
			
			return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(type, MatchCase_match(slot), MatchCase_name(slot), 0));
			break;
		}
		case L1IRSlotTypeEndMatch:
		{
			L1IRAddress matchRef = Normalize(self, EndMatch_match(slot));
			L1IRSlot matchSlot = GetSlot(self, matchRef);
			//return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(type, EndMatch_match(slot), EndMatch_resultType(slot), 0));
			switch(L1IRExtractSlotType(matchSlot))
			{
				case L1IRSlotTypeBeginMatch:
				case L1IRSlotTypeMatchCase:
					break;
				case L1IRSlotTypeMatchFailure:
					return CreateError(self, L1IRErrorTypeUnmatchableValue);
				case L1IRSlotTypeMatchSuccess:
				{
					L1IRAddress resultTypeRef = Normalize(self, EndMatch_resultType(slot));
					L1IRAddress result = Normalize(self, MatchSuccess_result(matchSlot));
					if (not IsOfType(self, result, resultTypeRef))
						return CreateError(self, L1IRErrorTypeTypeChecking);
					return result;
				}
				default:
					return CreateError(self, L1IRErrorTypeInvalidInstruction);
			}
			break;
		}
		case L1IRSlotTypeLookup:
		{
			L1IRAddress namespaceRef = Normalize(self, Lookup_namespace(slot));
			L1IRAddress nameRef = Normalize(self, Lookup_name(slot));
			L1IRSlot namespaceSlot = GetSlot(self, namespaceRef);
			L1IRSlot nameSlot = GetSlot(self, nameRef);
			
			if (not (L1IRExtractSlotType(nameSlot) == L1IRSlotTypeRawData32 or L1IRExtractSlotType(nameSlot) == L1IRSlotTypeRawData32Extended))
				return CreateError(self, L1IRErrorTypeInvalidInstruction);
			
			switch (L1IRExtractSlotType(namespaceSlot))
			{
				case L1IRSlotTypeADT:
				case L1IRSlotTypeExtendADT:
				{
					while (L1IRExtractSlotType(namespaceSlot) == L1IRSlotTypeExtendADT)
					{
						if (AreEqual(self, ExtendADT_name(namespaceSlot), nameRef))
							return Substitute(self, ExtendADT_constructor(namespaceSlot), Lookup_namespace(slot));
						namespaceSlot = GetSlot(self, ExtendADT_adt(namespaceSlot));
					}
					
					if (L1IRExtractSlotType(namespaceSlot) not_eq L1IRSlotTypeADT)
						return CreateError(self, L1IRErrorTypeInvalidInstruction);
					return CreateError(self, L1IRErrorTypeNoSuchFieldInNamespace);
				}
				default:
					return CreateError(self, L1IRErrorTypeInvalidInstruction);
			}
			
			break;
		}
		case L1IRSlotTypeRecursive:
		{
			L1IRAddress argumentTypeRef = Normalize(self, Recursive_argumentType(slot));
			if (L1IRExtractSlotType(GetSlot(self, argumentTypeRef)) == L1IRSlotTypeError)
				return CreateError(self, L1IRErrorTypeTypeChecking);
			return Normalize(self, Substitute(self, Recursive_result(slot), slotRef));
		}
		case L1IRSlotTypeLambda:
			return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(L1IRSlotTypeLambda, Normalize(self, Lambda_argumentType(slot)), Lambda_result(slot), 0));
		case L1IRSlotTypeForall:
			return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(L1IRSlotTypeForall, Normalize(self, Forall_argumentType(slot)), Forall_result(slot), 0));
		case L1IRSlotTypeCall:
		{
			L1IRAddress calleeRef = Normalize(self, Call_callee(slot));
			L1IRAddress argumentRef = Normalize(self, Call_argument(slot));
			L1IRSlot calleeSlot = GetSlot(self, calleeRef);
			if (L1IRExtractSlotType(calleeSlot) not_eq L1IRSlotTypeLambda)
				return CreateError(self, L1IRErrorTypeTypeChecking);
			if (not IsOfType(self, argumentRef, Lambda_argumentType(calleeSlot)))
				return CreateError(self, L1IRErrorTypeTypeChecking);
			return Normalize(self, Substitute(self, Lambda_result(calleeSlot), argumentRef));
		}
		case L1IRSlotTypeProject:
		{
			L1IRAddress pairRef = Normalize(self, Project_pair(slot));
			L1IRSlot pairSlot = GetSlot(self, pairRef);
			if (L1IRExtractSlotType(pairSlot) == L1IRSlotTypePair)
				return CreateError(self, L1IRErrorTypeTypeChecking);
			return (Project_index(slot) ? Pair_second : Pair_first) (pairSlot);
		}
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeNormalize:
		case L1IRSlotTypeSubstitute:
			return L1IRStateCreateSlot(self, slotRef);
	}
	
	uint16_t operands[3];
	for (uint8_t i = 0; i < 3; i++)
	{
		operands[i] = L1IRExtractSlotOperand(slot, i);
		if (SlotTypeArgumentIsLocalAddress(type, i))
			operands[i] = Normalize(self, operands[i]);
	}
	
	return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(type, operands[0], operands[1], operands[2]));
}

static L1IRAddress Substitute(L1IRState* self, L1IRAddress slotRef, L1IRAddress argumentRef)
{
	
	L1IRSlot slot = GetSlot(self, slotRef);
	L1IRSlotType type = L1IRExtractSlotType(slot);
	switch (type)
	{
		case L1IRSlotTypeError:
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeRecursive:
		case L1IRSlotTypeLambda:
		case L1IRSlotTypeForall:
		case L1IRSlotTypeCall:
		case L1IRSlotTypePair:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeProject:
		case L1IRSlotTypeRawData32Extended:
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypeConstructed:
		case L1IRSlotTypeLookup:
		case L1IRSlotTypeBeginMatch:
		case L1IRSlotTypeMatchCase:
		case L1IRSlotTypeEndMatch:
		case L1IRSlotTypeTypeOf:
		case L1IRSlotTypeNormalize:
			break;
		case L1IRSlotTypeArgument:
		{
			assert(Argument_index(slot) > 0);
			if (Argument_index(slot) == 1)
				return argumentRef;
			return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(L1IRSlotTypeArgument, Argument_index(slot) - 1, Substitute(self, Argument_type(slot), argumentRef), 0));
		}
		case L1IRSlotTypeSubstitute:
		{
			L1IRAddress root = L1IRStateCreateSlotRaw(self, L1IRMakeSlot(type, Substitute_root(slot), Substitute(self, Substitute_argument(slot), argumentRef), 0));
			return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(L1IRSlotTypeSubstitute, root, argumentRef, 0));
			//substitute xs o substitute ys = substitute (map (substitute xs) ys .. xs)
			//abort();
		}
	}
	
	uint16_t operands[3];
	for (uint8_t i = 0; i < 3; i++)
	{
		operands[i] = L1IRExtractSlotOperand(slot, i);
		if (SlotTypeArgumentIsLocalAddress(type, i))
			operands[i] = Substitute(self, operands[i], argumentRef);
	}
	
	return L1IRStateCreateSlotRaw(self, L1IRMakeSlot(type, operands[0], operands[1], operands[2]));
}

/*static L1IRAddress SubstituteWithCaptureChain(L1IRState* self, L1IRAddress slotRef, L1IRAddress capturesRef)
{
	L1IRSlot capturesSlot = GetSlot(self, capturesRef);
	while (L1IRExtractSlotType(capturesSlot) == L1IRSlotTypePair)
	{
		slotRef = Substitute(self, slotRef, Pair_second(capturesSlot));
		capturesSlot = GetSlot(Pair_first(capturesSlot));
	}
	
	if (L1IRExtractSlotType(capturesSlot) not_eq L1IRSlotTypeUnit)
		return CreateError(self, L1IRErrorTypeInvalidInstruction);
	return slotRef;
}*/

static L1IRAddress SubstituteWithCaptureChain(L1IRState* self, L1IRAddress slotRef, L1IRAddress capturesRef)
{
	L1IRSlot capturesSlot = GetSlot(self, capturesRef);
	
	if (L1IRExtractSlotType(capturesSlot) == L1IRSlotTypeUnit)
		return slotRef;
	if (L1IRExtractSlotType(capturesSlot) == L1IRSlotTypePair)
		return Substitute(self, SubstituteWithCaptureChain(self, slotRef, Pair_first(capturesSlot)), Pair_second(capturesSlot));
	
	return CreateError(self, L1IRErrorTypeInvalidInstruction);
}

L1IRAddress L1IRStateCreateSlot(L1IRState* self, L1IRSlot slot)
{
	//fprintf(stdout, "creating: %s (%u, %u, %u)\n", L1IRSlotTypeAsString(L1IRExtractSlotType(slot)), (unsigned) L1IRExtractSlotOperand(slot, 0), (unsigned) L1IRExtractSlotOperand(slot, 1), (unsigned) L1IRExtractSlotOperand(slot, 2));

	assert(IsValidSlot(self, slot));
	
	//assert(L1IRValidateSlot(self, slot));

	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeError:
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeUnitType:
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypeUnknown:
		case L1IRSlotTypeRecursive:
		case L1IRSlotTypeLambda:
		case L1IRSlotTypeForall:
		case L1IRSlotTypeCall:
		case L1IRSlotTypePair:
		case L1IRSlotTypePairType:
		case L1IRSlotTypeProject:
		case L1IRSlotTypeArgument:
		case L1IRSlotTypeRawData32Extended:
		case L1IRSlotTypeRawData32:
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
		case L1IRSlotTypeConstructed:
		case L1IRSlotTypeLookup:
		case L1IRSlotTypeBeginMatch:
		case L1IRSlotTypeMatchCase:
		case L1IRSlotTypeEndMatch:
			break;
		case L1IRSlotTypeTypeOf:
			return TypeOf(self, TypeOf_root(slot));
		case L1IRSlotTypeNormalize:
			return Normalize(self, Normalize_root(slot));
		case L1IRSlotTypeSubstitute:
			return Substitute(self, Substitute_root(slot), Substitute_argument(slot));
	}
	
	return L1IRStateCreateSlotRaw(self, slot);
}
