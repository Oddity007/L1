#ifndef L1IR_h
#define L1IR_h

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

/*enum L1IRSlotType
{
	L1IRSlotTypeUniverse,
	
	L1IRSlotTypeUnit,
	L1IRSlotTypeUnitType,
	
	L1IRSlotTypeDeconstructorLambda,
	L1IRSlotTypeLambda,
	L1IRSlotTypeForall,
	L1IRSlotTypeCall,

	L1IRSlotTypePair,
	L1IRSlotTypeExists,
	L1IRSlotTypeProject,
	
	L1IRSlotTypeArgument,
	
	L1IRSlotTypeRecursive,
	
	L1IRSlotTypeADT,
	L1IRSlotTypeExtendADT,
	L1IRSlotTypeConstructor,
	L1IRSlotTypeConstructed,

	L1IRSlotTypeRawData32Extended,
	L1IRSlotTypeRawData32,
	
	L1IRSlotTypeError,

	L1IRSlotTypeLast = L1IRSlotTypeError,
	
};*/

#include "L1IRSlotTypeDefinitions"

enum L1IRErrorType
{
	L1IRErrorTypeUnknown,
	L1IRErrorTypeTypeChecking,
	L1IRErrorTypeInvalidInstruction,
	L1IRErrorTypeUnmatchableValue,
	L1IRErrorTypeNoSuchFieldInNamespace,
	L1IRErrorTypeLast = L1IRErrorTypeNoSuchFieldInNamespace,
};
typedef enum L1IRErrorType L1IRErrorType;

enum
{
	L1IRSlotAnnotationFlagIsConfirmedReachable = 1,
	//L1IRSlotAnnotationFlagIsSingleUse = 1 << 1,
	L1IRSlotAnnotationFlagIsNormalized = 1 << 2,
};

//typedef enum L1IRErrorType L1IRErrorType;

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
	*slot |= (uint64_t) (annotation << 8u);
}

static void L1IRUpdateSlotAnnotationFlags(L1IRSlot* slot, uint8_t flags, bool to)
{
	uint8_t annotation = L1IRExtractSlotAnnotation(* slot);
	if (to)
		annotation |= flags;
	else
		annotation &= ~ flags;
	L1IRSetSlotAnnotation(slot, annotation);
}

static L1IRSlot L1IRAttachSlotAnnotation(L1IRSlot slot, uint8_t annotation)
{
	L1IRSetSlotAnnotation(& slot, annotation);
	return slot;
}

#endif

