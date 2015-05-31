#include "L1IRState.h"
#include <iso646.h>

#include "L1IRSlotDescriptions"
#include "L1IRSlotAccessors"

//Type equality (gets messy because it can be undecidable for dependent types).

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
				L1IRGlobalStatePushGCBarrier(self, localState);
				uint16_t
					pi1ArgumentLocalAddress,
					pi2ArgumentLocalAddress;
				uint16_t pi1ResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasArgument, L1IRExtractSlotOperand(value1Slot, 1), 0, L1IRExtractSlotOperand(value1Slot, 0), & pi1ArgumentLocalAddress);
				uint16_t pi2ResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasArgument, L1IRExtractSlotOperand(value2Slot, 1), 0, L1IRExtractSlotOperand(value2Slot, 0), & pi2ArgumentLocalAddress);

				bool areEqual = AreEqual(self, localState, pi1ArgumentLocalAddress, pi2ArgumentLocalAddress) and AreEqual(self, localState, pi1ResultLocalAddress, pi2ResultLocalAddress);
				
				L1IRGlobalStatePopGCBarrier(self, localState, NULL, 0);
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

bool L1IRGlobalStateIsOfType(L1IRGlobalState* self, L1IRLocalState* localState, uint16_t valueLocalAddress, uint16_t typeLocalAddress)
{
	const L1IRSlot* slots = L1ArrayGetElements(& localState->slots);
	L1IRSlot valueSlot = slots[valueLocalAddress];
	L1IRSlot typeSlot = slots[typeLocalAddress];
	switch (L1IRExtractSlotType(valueSlot))
	{
		case L1IRSlotTypeCall:
			{
				L1IRSlot calleeSlot = slots[Call_callee(valueSlot)];
				assert (L1IRExtractSlotType(calleeSlot) not_eq L1IRSlotTypeLambda);
				if (L1IRExtractSlotType(calleeSlot) not_eq L1IRSlotTypeArgument)
					return false;
				L1IRSlot calleeTypeSlot = slots[Argument_type(calleeSlot)];
				if (L1IRExtractSlotType(calleeTypeSlot) not_eq L1IRSlotTypePi)
					return false;
				
				L1IRGlobalStatePushGCBarrier(self, localState);
				uint16_t piResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasCaptured | L1IRGlobalStateEvaluationFlagHasArgument, Pi_prototype(calleeTypeSlot), Call_argument(valueSlot), L1IRExtractSlotOperand(calleeTypeSlot, 0), NULL);
				bool areEqual = AreEqual(self, localState, piResultLocalAddress, typeLocalAddress);
				L1IRGlobalStatePopGCBarrier(self, localState, NULL, 0);
				
				return areEqual;
			}
		case L1IRSlotTypeProjectPair:
			{
				assert (L1IRExtractSlotOperand(valueSlot, 1) < 2);
				uint16_t pairLocalAddress = L1IRExtractSlotOperand(valueSlot, 0);
				L1IRSlot pairSlot = slots[pairLocalAddress];
				assert (L1IRExtractSlotType(pairSlot) not_eq L1IRSlotTypePair);
				if (L1IRExtractSlotType(pairSlot) not_eq L1IRSlotTypeArgument) return false;
				uint16_t sigmaLocalAddress = L1IRExtractSlotOperand(pairSlot, 1);
				L1IRSlot sigmaSlot = slots[sigmaLocalAddress];
				if (L1IRExtractSlotType(sigmaSlot) not_eq L1IRSlotTypeSigma) return false;

				uint16_t sigmaAddress = L1IRExtractSlotOperand(sigmaSlot, 1);
				L1IRGlobalStatePushGCBarrier(self, localState);
				uint16_t sigmaArgumentLocalAddress = 0;
				uint16_t sigmaResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasCaptured, sigmaAddress, 0, L1IRExtractSlotOperand(sigmaSlot, 0), & sigmaArgumentLocalAddress);
				//slots = L1ArrayGetElements(& localState->slots)
				bool areEqual = false;
				if (L1IRExtractSlotOperand(valueSlot, 1))
					areEqual = AreEqual(self, localState, sigmaResultLocalAddress, typeLocalAddress);
				else
					areEqual = AreEqual(self, localState, sigmaArgumentLocalAddress, typeLocalAddress);
				L1IRGlobalStatePopGCBarrier(self, localState, NULL, 0);
				return areEqual;
			}
		case L1IRSlotTypeLambda:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypePi) return false;
			{
				uint16_t lambdaArgumentLocalAddress, piArgumentLocalAddress;
				
				L1IRGlobalStatePushGCBarrier(self, localState);
				
				uint16_t lambdaResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasCaptured, Lambda_prototype(valueSlot), 0, Lambda_captured(valueSlot), & lambdaArgumentLocalAddress);
				uint16_t piResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasCaptured | L1IRGlobalStateEvaluationFlagHasArgument, Pi_prototype(typeSlot), lambdaArgumentLocalAddress, Pi_captured(typeSlot), & piArgumentLocalAddress);

				bool areEqual = AreEqual(self, localState, lambdaArgumentLocalAddress, piArgumentLocalAddress) and AreEqual(self, localState, lambdaResultLocalAddress, piResultLocalAddress);
				L1IRGlobalStatePopGCBarrier(self, localState, NULL, 0);
				return areEqual;
			}
		case L1IRSlotTypePi:
		case L1IRSlotTypeSigma:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypeUniverse) return false;
			{
				uint16_t piAddress = L1IRExtractSlotOperand(valueSlot, 1);
				uint16_t piArgumentLocalAddress = 0;
				L1IRGlobalStatePushGCBarrier(self, localState);
				uint16_t piResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasCaptured, piAddress, 0, L1IRExtractSlotOperand(valueSlot, 0), & piArgumentLocalAddress);
				slots = L1ArrayGetElements(& localState->slots);
				assert (L1IRExtractSlotType(slots[piResultLocalAddress]) == L1IRSlotTypeArgument);
				uint16_t argumentTypeLocalAddress = L1IRExtractSlotOperand(slots[piResultLocalAddress], 1);
				bool isOfType = L1IRGlobalStateIsOfType(self, localState, argumentTypeLocalAddress, typeLocalAddress) and L1IRGlobalStateIsOfType(self, localState, piResultLocalAddress, typeLocalAddress);
				L1IRGlobalStatePopGCBarrier(self, localState, NULL, 0);
				return isOfType;
			}
		case L1IRSlotTypeConstructor:
		case L1IRSlotTypeConstructorOf:
			//To do: Finish this
			return true;
		case L1IRSlotTypeADT:
		case L1IRSlotTypeExtendADT:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypeUniverse) return false;
			//To do: Finish this case
			return true;
		case L1IRSlotTypePair:
			if (L1IRExtractSlotType(typeSlot) not_eq L1IRSlotTypeSigma) return false;
			{
				L1IRGlobalStatePushGCBarrier(self, localState);
				uint16_t sigmaResultLocalAddress = L1IRGlobalStateEvaluate(self, localState, L1IRGlobalStateEvaluationFlagHasCaptured | L1IRGlobalStateEvaluationFlagHasArgument, Sigma_prototype(typeSlot), Pair_first(valueSlot), Sigma_captured(typeSlot), NULL);
				bool isOfType = L1IRGlobalStateIsOfType(self, localState, Pair_second(valueSlot), sigmaResultLocalAddress);
				L1IRGlobalStatePopGCBarrier(self, localState, NULL, 0);
				return isOfType;
			}
		case L1IRSlotTypeSelf:
		case L1IRSlotTypeCaptured:
			return true;//Don't really like this, but w/e
		case L1IRSlotTypeUniverse:
			return L1IRExtractSlotType(typeSlot) == L1IRSlotTypeUniverse and Universe_index(typeSlot) > Universe_index(valueSlot);
		case L1IRSlotTypeUnit:
			return L1IRExtractSlotType(typeSlot) == L1IRSlotTypeUnitType;
		case L1IRSlotTypeUnitType:
			return L1IRExtractSlotType(typeSlot) == L1IRSlotTypeUniverse and Universe_index(typeSlot) == 0;
		case L1IRSlotTypeArgument:
			return AreEqual(self, localState, Argument_type(valueSlot), typeLocalAddress);
		case L1IRSlotTypeConstructedOf:
			return AreEqual(self, localState, L1IRExtractSlotOperand(valueSlot, 0), typeLocalAddress);
		default:
			abort();
			break;
	}
	return false;
}
