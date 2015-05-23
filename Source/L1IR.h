#ifndef L1IR_h
#define L1IR_h

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

enum L1IRSlotType
{
	//Warning: Introduces general recursion
	L1IRSlotTypeSelf,

	L1IRSlotTypeUniverse,
	
	L1IRSlotTypeUnit,
	L1IRSlotTypeUnitType,
	
	L1IRSlotTypeLambda,
	L1IRSlotTypePi,

	L1IRSlotTypePair,
	L1IRSlotTypeSigma,

	L1IRSlotTypeProjectPair,

	L1IRSlotTypeCall,
	L1IRSlotTypeCallCapture,
	
	L1IRSlotTypeArgument,
	L1IRSlotTypeCaptured,
	
	L1IRSlotTypeADT,
	L1IRSlotTypeConstructor,
	L1IRSlotTypeConstructorOf,
	L1IRSlotTypeConstructedOf,
	L1IRSlotTypeDeconstruction,
	L1IRSlotTypeDeconstructionSuccess,
	L1IRSlotTypeBeginDeconstruction,
	L1IRSlotTypeEndDeconstruction,

	L1IRSlotTypeLast = L1IRSlotTypeEndDeconstruction,
	
};

typedef enum L1IRSlotType L1IRSlotType;

typedef uint_least64_t L1IRSlot;

static L1IRSlot L1IRMakeSlot(L1IRSlotType type, uint16_t operand1, uint16_t operand2, uint16_t operand3)
{
	L1IRSlot slot = 0;
	slot |= ((L1IRSlot) type) & 0xFF;
	slot |= ((L1IRSlot) operand1 << 16 * 1);
	slot |= ((L1IRSlot) operand2 << 16 * 2);
	slot |= ((L1IRSlot) operand3 << 16 * 3);
	return slot;
}

static L1IRSlotType L1IRExtractSlotType(L1IRSlot slot)
{
	return (L1IRSlotType) (slot & 0xFF);
}

static uint16_t L1IRExtractSlotOperand(L1IRSlot slot, size_t i)
{
	assert(i < 3);
	return (uint16_t) ((slot >> 16 * (i + 1)) & 0xFFFF);
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

static L1IRSlot L1IRAttachSlotAnnotation(L1IRSlot slot, uint8_t annotation)
{
	L1IRSetSlotAnnotation(& slot, annotation);
	return slot;
}

#endif

