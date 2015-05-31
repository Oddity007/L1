#include "L1IRState.h"
#include <iso646.h>

#include "L1IRSlotDescriptions"
#include "L1IRSlotAccessors"


static uint16_t WalkDeconstructionChain(const L1IRSlot* slots, uint16_t localAddress)
{
	while (L1IRExtractSlotType(slots[localAddress]) == L1IRSlotTypeDeconstruction)
		localAddress = Deconstruction_default(slots[localAddress]);
	
	return localAddress;
}

static uint16_t L1IRGlobalStateCreateSlotRaw(L1IRGlobalState* self, L1IRLocalState* localState, L1IRSlot slot)
{
	const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);
	size_t slotCount = L1ArrayGetElementCount(& localState->slots);

	for (uint16_t i = 0; i < L1ArrayGetElementCount(& localState->slots); i++)
		if (slots[i] == slot) return i;
	L1ArrayPush(& localState->slots, & slot, sizeof(L1IRSlot));
	return L1ArrayGetElementCount(& localState->slots) - 1;
}

uint16_t L1IRGlobalStateCreateSlot(L1IRGlobalState* self, L1IRLocalState* localState, L1IRSlot slot)
{
	const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);
	size_t slotCount = L1ArrayGetElementCount(& localState->slots);
	
	if (slotCount > 0 and L1IRExtractSlotType(slots[slotCount - 1]) == L1IRSlotTypeError)
		return slotCount - 1;
	
	switch (L1IRExtractSlotType(slot))
	{
		case L1IRSlotTypeConstructorOf:
			{
				uint16_t adtLocalAddress = ConstructorOf_adt(slot);
				L1IRSlot adtSlot = slots[adtLocalAddress];
				while (L1IRExtractSlotType(adtSlot) == L1IRSlotTypeExtendADT)
				{
					if (ExtendADT_tag(adtSlot) == ConstructorOf_tag(slot))
						return L1IRGlobalStateCreateSlot(self, localState, L1IRMakeSlot(L1IRSlotTypeConstructor, ConstructorOf_adt(slot), ExtendADT_tag(adtSlot), ExtendADT_argumentType(adtSlot)));
					adtLocalAddress = ExtendADT_adt(adtSlot);
					adtSlot = slots[adtLocalAddress];
				}
			}
			break;
		case L1IRSlotTypeDeconstruction:
			{
				uint16_t beginLocalAddress = WalkDeconstructionChain(slots, Deconstruction_default(slot));
				uint16_t beginSlot = slots[beginLocalAddress];
				switch (L1IRExtractSlotType(beginSlot))
				{
					case L1IRSlotTypeDeconstructionSuccess:
						return beginLocalAddress;
					case L1IRSlotTypeBeginDeconstruction:
						{
							L1IRSlot constructedSlot = slots[BeginDeconstruction_constructed(beginSlot)];
							if (L1IRExtractSlotType(constructedSlot) not_eq L1IRSlotTypeConstructedOf)
								break;
							if (ConstructedOf_tag(constructedSlot) not_eq Deconstruction_tag(slot))
								return beginLocalAddress;
							//to do: assert that constructed is of correct type
							return L1IRGlobalStateCreateSlot(self, localState, L1IRMakeSlot(L1IRSlotTypeCall, Deconstruction_callee(slot), ConstructedOf_argument(constructedSlot), 0));
						}
						break;
					default:
						abort();
						break;
				}
			}
			break;
		case L1IRSlotTypeEndDeconstruction:
			{
				L1IRSlot deconstructionSlot = slots[EndDeconstruction_deconstruction(slot)];
				switch (L1IRExtractSlotType(deconstructionSlot))
				{
					case L1IRSlotTypeDeconstructionSuccess:
						return DeconstructionSuccess_result(deconstructionSlot);
					case L1IRSlotTypeBeginDeconstruction:
						abort();
						//Match Failed
						break;
					default:
						break;
				}
			}
			break;
		case L1IRSlotTypeProjectPair:
			switch (L1IRExtractSlotType(slots[ProjectPair_pair(slot)]))
			{
				case L1IRSlotTypeArgument:
				case L1IRSlotTypeCaptured:
				case L1IRSlotTypeCall:
					break;
				case L1IRSlotTypePair:
					assert (ProjectPair_index(slot) < 2);
					return L1IRExtractSlotOperand(slots[ProjectPair_pair(slot)], ProjectPair_index(slot));
				default:
					abort();
					break;
			}
			break;
		case L1IRSlotTypeCall:
			{
				const L1IRSlot calleeSlot = slots[Call_callee(slot)];
				switch (L1IRExtractSlotType(calleeSlot))
				{
					case L1IRSlotTypeLambda:
						{
							L1IRGlobalStatePushGCBarrier(self, localState);
							uint16_t result = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasCaptured | L1IRGlobalStateEvaluationFlagHasArgument, Lambda_prototype(calleeSlot), Call_argument(slot), Lambda_captured(calleeSlot), NULL);
							L1IRGlobalStatePopGCBarrier(self, localState, & result, 1);
							return result;
						}
					case L1IRSlotTypeConstructor:
						if (L1IRGlobalStateIsOfType(self, localState, Call_argument(slot), Constructor_argumentType(calleeSlot)))
							return L1IRGlobalStateCreateSlot(self, localState, L1IRMakeSlot(L1IRSlotTypeConstructedOf, Constructor_adt(calleeSlot), Constructor_tag(calleeSlot), Call_argument(slot)));
						else
							return L1IRGlobalStateCreateSlot(self, localState, L1IRMakeSlot(L1IRSlotTypeError, L1IRErrorTypeTypeChecking, 0, 0));
					case L1IRSlotTypeConstructorOf:
					case L1IRSlotTypeArgument:
					case L1IRSlotTypeCaptured:
					case L1IRSlotTypeCall:
					case L1IRSlotTypeProjectPair:
					case L1IRSlotTypeSelf:
						break;
					default:
						abort();
						break;
				}
			}
			break;
		default:
			break;
	}

	return L1IRGlobalStateCreateSlotRaw(self, localState, slot);
}
