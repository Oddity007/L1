#include "L1IR.h"
//#include "L1HashFunctions.h"
#include <stdio.h>
#include <iso646.h>


//Important header stuff

typedef uint32_t L1IRAnnotatedLocalAddress;

static bool L1IRAnnotatedLocalAddressIsUsable(L1IRAnnotatedLocalAddress address)
{
	return ! (address & 0xFFFF0000);
}

static L1IRLocalAddress L1IRAnnotatedLocalAddressGetLocalAddress(L1IRAnnotatedLocalAddress address)
{
	return (address & 0xFFFF);
}

static L1IRAnnotatedLocalAddress L1IRAnnotateLocalAddress(L1IRLocalAddress address, uint16_t annotation)
{
	return ((uint32_t) annotation << 16) + address;
}


static uint8_t L1IRExtractSlotAnnotation(L1IRSlot slot)
{
	return (slot >> 8) & 0xFF;
}

static void L1IRSetSlotAnnotation(L1IRSlot* slot, uint8_t annotation)
{
	*slot &= 0xFFFFFFFFFFFF00FF;
	*slot |= annotation << 8;
}

static uint16_t L1IRGlobalStateEvaluate(L1IRGlobalState* self, L1IRLocalState* localState, uint32_t calleeAddress, L1IRAnnotatedLocalAddress argumentLocalAddress, L1IRAnnotatedLocalAddress captureLocalAddress, uint16_t* finalArgumentLocalAddress);

static uint16_t L1IRGlobalStateCreateSlot(L1IRGlobalState* self, L1IRLocalState* localState, L1IRSlot slot);

///State Boilerplate

void L1IRLocalStateInitialize(L1IRLocalState* self)
{
	L1ArrayInitialize(& self->slots);
	L1ArrayInitialize(& self->gcBarriers);
	self->callDepth = 0;
}

void L1IRLocalStateDeinitialize(L1IRLocalState* self)
{
	L1ArrayDeinitialize(& self->slots);
	L1ArrayDeinitialize(& self->gcBarriers);
	self->callDepth = 0;
}

L1IRLocalAddress L1IRLocalStateCreateSlot(L1IRLocalState* self, L1IRSlot slot)
{
	L1ArrayPush(& self->slots, & slot, sizeof(L1IRSlot));
	return L1ArrayGetElementCount(& self->slots) - 1;
}

void L1IRGlobalStateInitialize(L1IRGlobalState* self)
{
	L1ArrayInitialize(& self->blocks);
}

void L1IRGlobalStateDeinitialize(L1IRGlobalState* self)
{
	const L1IRGlobalStateBlock* blocks = L1ArrayGetElements(& self->blocks);
	size_t blockCount = L1ArrayGetElementCount(& self->blocks);

	for (size_t i = 0; i < blockCount; i++)
		free(blocks[i].slots);
	L1ArrayDeinitialize(& self->blocks);
}

//Node Graph Traversal Metadata

#define NodeArgumentMask(i) ((uint8_t) 1 << i)

static const uint8_t SlotArgumentDescriptions[L1IRSlotTypeLast + 1] =
{
	[L1IRSlotTypeNone] = 0,
	[L1IRSlotTypeArgument] = NodeArgumentMask(1),
	[L1IRSlotTypeCaptured] = 0,
	[L1IRSlotTypeUnit] = 0,
	[L1IRSlotTypeUnitType] = 0,
	[L1IRSlotTypeCapturedTupleType] = NodeArgumentMask(0) | NodeArgumentMask(1),
	[L1IRSlotTypeCapturedTuple] = NodeArgumentMask(0) | NodeArgumentMask(1),
	[L1IRSlotTypeLambda] = NodeArgumentMask(0),
	[L1IRSlotTypePi] = NodeArgumentMask(0),
	[L1IRSlotTypePair] = NodeArgumentMask(0) | NodeArgumentMask(1),
	[L1IRSlotTypeSigma] = NodeArgumentMask(0),
	[L1IRSlotTypeProjectPair] = NodeArgumentMask(0),
	[L1IRSlotTypeCall] = NodeArgumentMask(0) | NodeArgumentMask(1),
};

static bool SlotTypeArgumentIsLocalAddress(L1IRSlotType type, uint8_t i)
{
	return 0 not_eq (SlotArgumentDescriptions[(uint8_t) type] & NodeArgumentMask(i));
}

#undef NodeArgumentMask

//Garbage Collection / Normalization / Deadcode Eliminator / Stack Compaction

static void CalculateNormalizedOrderings(L1IRSlot* slots, uint16_t* slotRemappings, uint16_t slotStart, uint16_t* localAddress, uint16_t* entryIndex)
{
	if (*localAddress < slotStart) return;
	L1IRSlotType type = L1IRExtractSlotType(slots[*localAddress]);
	if (L1IRExtractSlotAnnotation(slots[*localAddress]))
	{
		*localAddress = slotRemappings[*localAddress - slotStart];
		return;
	}
	uint16_t operands[3] = {0, 0, 0};
	for (uint8_t i = 0; i < 3; i++)
	{
		operands[i] = L1IRExtractSlotOperand(slots[*localAddress], i);
		if (not SlotTypeArgumentIsLocalAddress(type, i)) continue;
		CalculateNormalizedOrderings(slots, slotRemappings, slotStart, operands + i, entryIndex);
	}
	slots[*localAddress] = L1IRMakeSlot(type, operands[0], operands[1], operands[2]);
	L1IRSetSlotAnnotation(slots + *localAddress, 1);
	slotRemappings[*localAddress - slotStart] = *entryIndex;
	(* entryIndex) ++;
}

static uint16_t CompactLocalGarbage(L1IRSlot* slots, uint16_t slotStart, uint16_t slotCount, uint16_t* roots, size_t rootCount)
{
	if (rootCount == 0 or slotCount == slotStart) return slotStart;
	assert (slotCount > slotStart);
	uint16_t* slotRemappings = malloc(sizeof(uint16_t) * (slotCount - slotStart));
	uint16_t normalizedSlotCount = slotStart;
	for (size_t i = 0; i < rootCount; i++)
		CalculateNormalizedOrderings(slots, slotRemappings, slotStart, roots + i, & normalizedSlotCount);
	for (uint16_t i = slotStart; i < normalizedSlotCount; i++)
	{
		uint16_t source = i;
		L1IRSlot slot = slots[source];
		while (L1IRExtractSlotAnnotation(slot))
		{
			uint16_t destination = slotRemappings[source - slotStart];
			slot = slots[destination];
			slots[destination] = slots[source];
			L1IRSetSlotAnnotation(slots + source, 0);
			source = destination;
		}
	}
	free(slotRemappings);
	return normalizedSlotCount;
}

static void PushGCBarrier(L1IRGlobalState* self, L1IRLocalState* localState)
{
	size_t slotCount = L1ArrayGetElementCount(& localState->slots);
	
	L1ArrayPush(& localState->gcBarriers, & slotCount, sizeof(size_t));
}

static void PopGCBarrier(L1IRGlobalState* self, L1IRLocalState* localState, uint16_t* roots, size_t rootCount)
{
	size_t barrierSlotCount = 0;
	L1ArrayPop(& localState->gcBarriers, & barrierSlotCount, sizeof(size_t));
	size_t oldSlotCount = L1ArrayGetElementCount(& localState->slots);
	barrierSlotCount = CompactLocalGarbage(L1ArrayGetElements(& localState->slots), barrierSlotCount, oldSlotCount, roots, rootCount);
	L1ArraySetElementCount(& localState->slots, barrierSlotCount, sizeof(L1IRSlot));
}

//Type equality (gets messy because it can be undecidable for dependent types)

static bool AreEqual(L1IRGlobalState* self, L1IRLocalState* localState, uint16_t value1LocalAddress, uint16_t value2LocalAddress)
{
	const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);
	if (value1LocalAddress == value2LocalAddress) return true;
	L1IRSlot value1Slot = slots[value1LocalAddress];
	L1IRSlot value2Slot = slots[value2LocalAddress];
	if (L1IRExtractSlotType(value1Slot) not_eq L1IRExtractSlotType(value2Slot)) return false;
	L1IRSlotType type = L1IRExtractSlotType(value1Slot);
	switch (type)
	{
		case L1IRSlotTypePi:
		case L1IRSlotTypeSigma:
			{
				PushGCBarrier(self, localState);
				uint32_t pi1Address = ((uint32_t) L1IRExtractSlotOperand(value1Slot, 2) << 16) + L1IRExtractSlotOperand(value1Slot, 1);
				uint16_t pi1ArgumentLocalAddress = 0;
				uint16_t pi1ResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, pi1Address, UINT32_MAX, L1IRExtractSlotOperand(value1Slot, 2), & pi1ArgumentLocalAddress);

				uint32_t pi2Address = ((uint32_t) L1IRExtractSlotOperand(value2Slot, 2) << 16) + L1IRExtractSlotOperand(value2Slot, 1);
				uint16_t pi2ArgumentLocalAddress = 0;
				uint16_t pi2ResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, pi2Address, UINT32_MAX, L1IRExtractSlotOperand(value2Slot, 2), & pi2ArgumentLocalAddress);

				bool areEqual = AreEqual(self, localState, pi1ArgumentLocalAddress, pi2ArgumentLocalAddress) and AreEqual(self, localState, pi1ResultLocalAddress, pi2ResultLocalAddress);
				
				PopGCBarrier(self, localState, NULL, 0);
				return areEqual;
			}
		default:
			for (uint8_t i = 0; i < 3; i++)
			{
				if (SlotTypeArgumentIsLocalAddress(type, i))
				{
					if (not AreEqual(self, localState, L1IRExtractSlotOperand(value1Slot, i), L1IRExtractSlotOperand(value2Slot, i))) return false;
				} 
				else if (L1IRExtractSlotOperand(value1Slot, i) not_eq L1IRExtractSlotOperand(value2Slot, i)) return false;
			}
			return true;
	}
	return false;
}

//Block Creation

L1IRGlobalAddress L1IRGlobalStateCreateBlock(L1IRGlobalState* self, L1IRGlobalStateBlockType type, const L1IRSlot* slots, uint16_t slotCount, L1IRLocalAddress argumentLocalAddress)
{
	const L1IRSlot* normalizedSlots = slots;
	uint16_t normalizedSlotCount = slotCount;
	uint64_t hash = 0;//L1DJB2Hash(normalizedSlots, sizeof(L1IRSlot) * normalizedSlotCount);
	const L1IRGlobalStateBlock* blocks = L1ArrayGetElements(& self->blocks);
	size_t blockCount = L1ArrayGetElementCount(& self->blocks);
	assert (normalizedSlotCount > 0);
	assert (slotCount > 0);

	L1IRGlobalAddress address = 0;
	for (size_t i = 0; i < blockCount; i++)
	{
		if (blocks[i].hash not_eq hash) continue;
		if (blocks[i].slotCount not_eq normalizedSlotCount) continue;
		if (blocks[i].type not_eq type) continue;
		if (memcmp(blocks->slots, normalizedSlots, sizeof(L1IRSlot) * normalizedSlotCount) not_eq 0) continue;
		address = i;
		return address;
	}

	L1IRGlobalStateBlock block;
	block.type = type;
	block.hash = hash;
	block.slotCount = normalizedSlotCount;
	block.slots = memcpy(malloc(sizeof(L1IRSlot) * normalizedSlotCount), normalizedSlots, sizeof(L1IRSlot) * normalizedSlotCount);
	block.argumentLocalAddress = argumentLocalAddress;

	L1ArrayAppend(& self->blocks, & block, sizeof(L1IRGlobalStateBlock));
	address = blockCount;

	return address;
}

/*static uint16_t GetUnusedArgumentID(L1IRGlobalState* self, L1IRLocalState* localState)
{
	const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);
	uint16_t id = 0;
	for (uint16_t i = 0; i < L1ArrayGetElementCount(& localState->slots); i++)
		if (L1IRExtractSlotType(slots[i]) == L1IRSlotTypeArgument and id <= L1IRExtractSlotOperand(slots[i], 0))
			id = L1IRExtractSlotOperand(slots[i], 0) + 1;
	return id;
}*/

static uint16_t L1IRGlobalStateCreateSlot(L1IRGlobalState* self, L1IRLocalState* localState, L1IRSlot slot)
{
	const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);

	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeNone:
		case L1IRSlotTypeArgument:
		case L1IRSlotTypeCaptured:
			break;
		case L1IRSlotTypeUniverse:
		case L1IRSlotTypeUnit:
		case L1IRSlotTypeUnitType:
			break;
		case L1IRSlotTypeLambda:
		case L1IRSlotTypePi:
		case L1IRSlotTypePair:
		case L1IRSlotTypeSigma:
			break;
		case L1IRSlotTypeCapturedTupleType:
		case L1IRSlotTypeCapturedTuple:
			break;
		case L1IRSlotTypeProjectPair:
			{
				L1IRLocalAddress pairSlotLocalAddress = L1IRExtractSlotOperand(slot, 0);
				L1IRSlot pairSlot = slots[pairSlotLocalAddress];
				switch (L1IRExtractSlotType(pairSlot))
				{
					case L1IRSlotTypeArgument:
					case L1IRSlotTypeCaptured:
					case L1IRSlotTypeCall:
						break;
					case L1IRSlotTypePair:
						assert (L1IRExtractSlotOperand(slot, 1) < 2);
						return L1IRExtractSlotOperand(pairSlot, L1IRExtractSlotOperand(slot, 1));
					default:
						abort();
						break;
				}
				break;
			}
		case L1IRSlotTypeCall:
			{
				const uint16_t calleeLocalAddress = L1IRExtractSlotOperand(slot, 0);
				const uint16_t argumentLocalAddress = L1IRExtractSlotOperand(slot, 1);
				const L1IRSlot calleeSlot = slots[calleeLocalAddress];
				//const L1IRSlot argumentSlot = slots[argumentLocalAddress];
				switch (L1IRExtractSlotType(calleeSlot))
				{
					case L1IRSlotTypeLambda:
						{
							L1IRGlobalAddress calleeAddress = ((uint32_t) L1IRExtractSlotOperand(calleeSlot, 2) << 16) + L1IRExtractSlotOperand(calleeSlot, 1);
							uint16_t captureLocalAddress = L1IRExtractSlotOperand(calleeSlot, 2);
							PushGCBarrier(self, localState);
							uint16_t result = L1IRGlobalStateEvaluate(self, localState, calleeAddress, argumentLocalAddress, captureLocalAddress, NULL);
							PopGCBarrier(self, localState, & result, 1);
							return result;
						}
					case L1IRSlotTypeNone:
					case L1IRSlotTypePi:
					case L1IRSlotTypeUniverse:
					case L1IRSlotTypePair:
					case L1IRSlotTypeSigma:
					case L1IRSlotTypeUnit:
					case L1IRSlotTypeUnitType:
					case L1IRSlotTypeCapturedTupleType:
					case L1IRSlotTypeCapturedTuple:
						abort();
						break;
					case L1IRSlotTypeArgument:
					case L1IRSlotTypeCaptured:
					case L1IRSlotTypeCall:
					case L1IRSlotTypeProjectPair:
						break;
				}
			}
			break;
	}

	for (uint16_t i = 0; i < L1ArrayGetElementCount(& localState->slots); i++)
		if (slots[i] == slot) return i;
	L1ArrayPush(& localState->slots, & slot, sizeof(L1IRSlot));
	return L1ArrayGetElementCount(& localState->slots) - 1;
}

static bool L1IRGlobalStateIsOfType(L1IRGlobalState* self, L1IRLocalState* localState, uint16_t valueLocalAddress, uint16_t typeLocalAddress)
{
	const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);
	L1IRSlot valueSlot = slots[valueLocalAddress];
	L1IRSlot typeSlot = slots[typeLocalAddress];
	switch (L1IRExtractSlotType(valueSlot))
	{
		case L1IRSlotTypeCaptured:
			return true;//Don't really like this, but w/e
		case L1IRSlotTypeUniverse:
			return L1IRExtractSlotType(typeSlot) == L1IRSlotTypeUniverse and L1IRExtractSlotOperand(typeSlot, 0) > L1IRExtractSlotOperand(valueSlot, 0);
		case L1IRSlotTypeUnit:
			return L1IRExtractSlotType(typeSlot) == L1IRSlotTypeUnitType;
		case L1IRSlotTypeUnitType:
			return L1IRExtractSlotType(typeSlot) == L1IRSlotTypeUniverse and L1IRExtractSlotOperand(typeSlot, 0) == 0;
		case L1IRSlotTypeArgument:
			return AreEqual(self, localState, L1IRExtractSlotOperand(valueSlot, 1), typeLocalAddress);
		case L1IRSlotTypeCall:
			{
				uint16_t calleeLocalAddress = L1IRExtractSlotOperand(valueSlot, 0);
				uint16_t argumentLocalAddress = L1IRExtractSlotOperand(valueSlot, 1);
				L1IRSlot calleeSlot = slots[calleeLocalAddress];
				L1IRSlotType calleeSlotType = L1IRExtractSlotType(calleeSlot);
				uint16_t calleeTypeLocalAddress = 0;
				assert (calleeSlotType not_eq L1IRSlotTypeLambda);
				if (calleeSlotType not_eq L1IRSlotTypeArgument) return false;
				calleeTypeLocalAddress = L1IRExtractSlotOperand(calleeSlot, 1);
				L1IRSlot calleeTypeSlot = slots[calleeTypeLocalAddress];
				if (L1IRExtractSlotType(calleeTypeSlot) not_eq L1IRSlotTypePi) return false;
				uint32_t piAddress = ((uint32_t) L1IRExtractSlotOperand(calleeTypeSlot, 2) << 16) + L1IRExtractSlotOperand(calleeTypeSlot, 1);
				PushGCBarrier(self, localState);
				uint16_t piResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, piAddress, argumentLocalAddress, L1IRExtractSlotOperand(calleeTypeSlot, 0), NULL);
				//slots = L1ArrayGetElements(& localState->slots);
				bool areEqual = AreEqual(self, localState, piResultLocalAddress, typeLocalAddress);
				PopGCBarrier(self, localState, NULL, 0);
				return areEqual;
			}
		case L1IRSlotTypeProjectPair:
			{
				abort();
				assert (L1IRExtractSlotOperand(valueSlot, 1) < 2);
				uint16_t pairLocalAddress = L1IRExtractSlotOperand(valueSlot, 0);
				L1IRSlot pairSlot = slots[pairLocalAddress];
				assert (L1IRExtractSlotType(pairSlot) not_eq L1IRSlotTypePair);
				if (L1IRExtractSlotType(pairSlot) not_eq L1IRSlotTypeArgument) return false;
				uint16_t sigmaLocalAddress = L1IRExtractSlotOperand(pairSlot, 1);
				L1IRSlot sigmaSlot = slots[sigmaLocalAddress];
				if (L1IRExtractSlotType(sigmaSlot) not_eq L1IRSlotTypeSigma) return false;

				uint32_t sigmaAddress = ((uint32_t) L1IRExtractSlotOperand(sigmaSlot, 2) << 16) + L1IRExtractSlotOperand(sigmaSlot, 1);
				PushGCBarrier(self, localState);
				uint16_t sigmaArgumentLocalAddress = 0;
				uint16_t sigmaResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, sigmaAddress, UINT32_MAX, L1IRExtractSlotOperand(sigmaSlot, 0), & sigmaArgumentLocalAddress);
				//slots = L1ArrayGetElements(& localState->slots)
				bool areEqual = false;
				if (L1IRExtractSlotOperand(valueSlot, 1))
					areEqual = AreEqual(self, localState, sigmaResultLocalAddress, typeLocalAddress);
				else
					areEqual = AreEqual(self, localState, sigmaArgumentLocalAddress, typeLocalAddress);
				PopGCBarrier(self, localState, NULL, 0);
				return areEqual;
			}
		case L1IRSlotTypeNone:
			abort();
			break;
		case L1IRSlotTypeLambda:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypePi) return false;
			{
				PushGCBarrier(self, localState);
				uint16_t lambdaArgumentLocalAddress = 0;
				uint32_t lambdaAddress = ((uint32_t) L1IRExtractSlotOperand(valueSlot, 2) << 16) + L1IRExtractSlotOperand(valueSlot, 1);
				uint16_t lambdaResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, lambdaAddress, UINT32_MAX, L1IRExtractSlotOperand(valueSlot, 2), & lambdaArgumentLocalAddress);

				uint32_t piAddress = ((uint32_t) L1IRExtractSlotOperand(typeSlot, 2) << 16) + L1IRExtractSlotOperand(typeSlot, 1);
				uint16_t piArgumentLocalAddress = 0;
				uint16_t piResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, piAddress, lambdaArgumentLocalAddress, L1IRExtractSlotOperand(typeSlot, 2), & piArgumentLocalAddress);

				bool areEqual = AreEqual(self, localState, lambdaArgumentLocalAddress, piArgumentLocalAddress) and AreEqual(self, localState, lambdaResultLocalAddress, piResultLocalAddress);
				PopGCBarrier(self, localState, NULL, 0);
				return areEqual;
			}
		case L1IRSlotTypePi:
		case L1IRSlotTypeSigma:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypeUniverse) return false;
			{
				uint32_t piAddress = ((uint32_t) L1IRExtractSlotOperand(typeSlot, 2) << 16) + L1IRExtractSlotOperand(typeSlot, 1);
				uint16_t piArgumentLocalAddress = 0;
				PushGCBarrier(self, localState);
				uint16_t piResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, piAddress, UINT32_MAX, L1IRExtractSlotOperand(typeSlot, 0), & piArgumentLocalAddress);
				slots = L1ArrayGetElements(& localState->slots);
				assert (L1IRExtractSlotType(slots[piResultLocalAddress]) == L1IRSlotTypeArgument);
				uint16_t argumentTypeLocalAddress = L1IRExtractSlotOperand(slots[piResultLocalAddress], 1);
				bool isOfType = L1IRGlobalStateIsOfType(self, localState, argumentTypeLocalAddress, typeLocalAddress) and L1IRGlobalStateIsOfType(self, localState, piResultLocalAddress, typeLocalAddress);
				PopGCBarrier(self, localState, NULL, 0);
				return isOfType;
			}
		case L1IRSlotTypeCapturedTupleType:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypeUniverse) return false;
			return L1IRGlobalStateIsOfType(self, localState, L1IRExtractSlotOperand(valueSlot, 0), typeLocalAddress) and L1IRGlobalStateIsOfType(self, localState, L1IRExtractSlotOperand(valueSlot, 1), typeLocalAddress);
		case L1IRSlotTypeCapturedTuple:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypeCapturedTupleType) return false;
			return L1IRGlobalStateIsOfType(self, localState, L1IRExtractSlotOperand(valueSlot, 0), L1IRExtractSlotOperand(typeSlot, 0)) and L1IRGlobalStateIsOfType(self, localState, L1IRExtractSlotOperand(valueSlot, 1), L1IRExtractSlotOperand(typeSlot, 1));
		case L1IRSlotTypePair:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypeSigma) return false;
			{
				PushGCBarrier(self, localState);
				uint16_t pairFirstLocalAddress = L1IRExtractSlotOperand(valueSlot, 0);
				uint16_t pairSecondLocalAddress = L1IRExtractSlotOperand(valueSlot, 1);

				uint32_t sigmaAddress = ((uint32_t) L1IRExtractSlotOperand(typeSlot, 2) << 16) + L1IRExtractSlotOperand(typeSlot, 1);
				uint16_t sigmaResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, sigmaAddress, pairFirstLocalAddress, L1IRExtractSlotOperand(typeSlot, 0), NULL);

				bool isOfType = L1IRGlobalStateIsOfType(self, localState, pairSecondLocalAddress, sigmaResultLocalAddress);
				PopGCBarrier(self, localState, NULL, 0);
				return isOfType;
			}
	}
	return false;
}

/*
There are 3 situations in which this can be called.
1. Invoked by user call.  (At root, has argument but no captures, local state slots are fully evaluated with no partial values)
2. Invoked to optimize a function. (At root, no argument and no captures, nothing in local state slots)
3. Invoked as part of type checking. (At root, no argument (possibly) but has captures, local state slots possibly contains partial captures and arguments)
*/

/*enum EvaluationMode
{
	EvaluationModeUserRoot,
	EvaluationModeUserInternal,
	EvaluationModeOptimizeRoot,
	EvaluationModeOptimizeInternal,
	EvaluationModeTypeCheckingRoot,
	EvaluationModeTypeCheckingInternal
};

typedef struct EvaluationMode EvaluationMode;*/

static uint16_t L1IRGlobalStateEvaluate(L1IRGlobalState* self, L1IRLocalState* localState, uint32_t calleeAddress, L1IRAnnotatedLocalAddress argumentLocalAddress, L1IRAnnotatedLocalAddress captureLocalAddress, uint16_t* finalArgumentLocalAddress)
{
	assert (L1ArrayGetElementCount(& self->blocks) > calleeAddress);
	const L1IRGlobalStateBlock* block = calleeAddress + (const L1IRGlobalStateBlock*) L1ArrayGetElements(& self->blocks);

	localState->callDepth++;

	assert (block->slotCount > 0);
	assert (block->slots);
	uint16_t* mergingSlotRemappings = malloc(sizeof(uint16_t) * block->slotCount);
	
	PushGCBarrier(self, localState);

	for (uint16_t i = 0; i < block->slotCount; i++)
	{
		const L1IRSlot prototypeSlot = block->slots[i];
		L1IRSlotType type = L1IRExtractSlotType(prototypeSlot);
		uint16_t operands[3] = {0, 0, 0};
		for (uint8_t j = 0; j < 3; j++) operands[j] = L1IRExtractSlotOperand(prototypeSlot, j);
		for (uint8_t j = 0; j < 3; j++) operands[j] = SlotTypeArgumentIsLocalAddress(type, j) ? mergingSlotRemappings[operands[j]]: operands[j];
		switch (type)
		{
			case L1IRSlotTypeArgument:
				{
					assert(operands[0] == 0);
					//GetUnusedArgumentID(self, localState)
					if (not L1IRAnnotatedLocalAddressIsUsable(argumentLocalAddress))
						mergingSlotRemappings[i] = L1IRGlobalStateCreateSlot(self, localState, L1IRMakeSlot(L1IRSlotTypeArgument, localState->callDepth - 1, operands[1], 0));
					else
						mergingSlotRemappings[i] = L1IRAnnotatedLocalAddressGetLocalAddress(argumentLocalAddress);
				}
				if (finalArgumentLocalAddress) *finalArgumentLocalAddress = mergingSlotRemappings[i];
				assert (L1IRGlobalStateIsOfType(self, localState, mergingSlotRemappings[i], operands[1]));
				break;
			case L1IRSlotTypeCaptured:
				if (L1IRAnnotatedLocalAddressIsUsable(captureLocalAddress))
				{
					const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);
					L1IRSlot pairSlot = slots[L1IRAnnotatedLocalAddressGetLocalAddress(captureLocalAddress)];
					assert (L1IRExtractSlotType(pairSlot) not_eq L1IRSlotTypeCapturedTuple);
					for(uint16_t i = 0; i < operands[0]; i++)
					{
						assert (L1IRExtractSlotType(pairSlot) == L1IRSlotTypeCapturedTuple);
						pairSlot = slots[L1IRExtractSlotOperand(pairSlot, 1)];
						i++;
					}
					assert (L1IRExtractSlotType(pairSlot) == L1IRSlotTypeCapturedTuple);
					mergingSlotRemappings[i] = L1IRExtractSlotOperand(pairSlot, 0);
				}
				else
				{
					assert (localState->callDepth == 1);
					assert (not L1IRAnnotatedLocalAddressIsUsable(argumentLocalAddress));
					mergingSlotRemappings[i] = L1IRGlobalStateCreateSlot(self, localState, L1IRMakeSlot(L1IRSlotTypeCaptured, operands[0], 0, 0));
				}
				break;
			case L1IRSlotTypeNone:
				break;
			case L1IRSlotTypeUniverse:
			case L1IRSlotTypeUnit:
			case L1IRSlotTypeUnitType:
			case L1IRSlotTypeLambda:
			case L1IRSlotTypePi:
			case L1IRSlotTypeCall:
			case L1IRSlotTypeCapturedTupleType:
			case L1IRSlotTypeCapturedTuple:
			case L1IRSlotTypePair:
			case L1IRSlotTypeSigma:
			case L1IRSlotTypeProjectPair:
				mergingSlotRemappings[i] = L1IRGlobalStateCreateSlot(self, localState, L1IRMakeSlot(type, operands[0], operands[1], operands[2]));
				break;
		}
	}

	uint16_t resultLocalAddress = mergingSlotRemappings[block->slotCount - 1];

	free(mergingSlotRemappings);
	
	localState->callDepth--;

	PopGCBarrier(self, localState, & resultLocalAddress, 0);

	return resultLocalAddress;
}

uint16_t L1IRGlobalStateCall(L1IRGlobalState* self, L1IRLocalState* localState, uint32_t calleeAddress, uint16_t argumentLocalAddress)
{
	return L1IRGlobalStateEvaluate(self, localState, calleeAddress, argumentLocalAddress, UINT32_MAX, NULL);
}

