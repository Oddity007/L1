#ifndef L1IR_h
#define L1IR_h

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

enum L1IRSlotType
{
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
	L1IRSlotTypeExtendADT,
	L1IRSlotTypeConstructor,
	L1IRSlotTypeConstructorOf,
	L1IRSlotTypeConstructedOf,
	L1IRSlotTypeDeconstruction,
	L1IRSlotTypeDeconstructionSuccess,
	L1IRSlotTypeBeginDeconstruction,
	L1IRSlotTypeEndDeconstruction,

	L1IRSlotTypeRawData32Extended,
	L1IRSlotTypeRawData48,

	L1IRSlotTypeUnresolvedSymbol,
	L1IRSlotTypeError,

	L1IRSlotTypeLast = L1IRSlotTypeError,
	
};

enum L1IRErrorType
{
	L1IRErrorType,
	L1IRErrorTypeTypeChecking,
	L1IRErrorTypeInvalidInstruction,
};

typedef enum L1IRSlotType L1IRSlotType;

typedef uint_least64_t L1IRSlot;

static L1IRSlot L1IRMakeSlot(L1IRSlotType type, uint16_t operand1, uint16_t operand2, uint16_t operand3)
{
	L1IRSlot slot = 0;
	slot |= ((L1IRSlot) type) & 0xFFu;
	slot |= ((L1IRSlot) operand1 << 16u * 1u);
	slot |= ((L1IRSlot) operand2 << 16u * 2u);
	slot |= ((L1IRSlot) operand3 << 16u * 3u);
	return slot;
}

static L1IRSlotType L1IRExtractSlotType(L1IRSlot slot)
{
	return (L1IRSlotType) (slot & 0xFFu);
}

static uint16_t L1IRExtractSlotOperand(L1IRSlot slot, size_t i)
{
	assert(i < 3u);
	return (uint16_t) ((slot >> 16u * (i + 1)) & 0xFFFFu);
}

static uint8_t L1IRExtractSlotAnnotation(L1IRSlot slot)
{
	return (slot >> 8u) & 0xFFu;
}

static void L1IRSetSlotAnnotation(L1IRSlot* slot, uint8_t annotation)
{
	*slot &= 0xFFFFFFFFFFFF00FFul;
	*slot |= (annotation << 8u);
}

static L1IRSlot L1IRAttachSlotAnnotation(L1IRSlot slot, uint8_t annotation)
{
	L1IRSetSlotAnnotation(& slot, annotation);
	return slot;
}

#endif

