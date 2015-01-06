#ifndef L1IR_h
#define L1IR_h

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

enum L1IRSlotType
{
	L1IRSlotTypeNone,
	
	L1IRSlotTypeArgument, //id (16), type (16)
	L1IRSlotTypeCaptured, //index (16)

	//Warning: Introduces general recursion
	L1IRSlotTypeSelf,

	L1IRSlotTypeUniverse, //index (16)
	
	L1IRSlotTypeUnit,
	L1IRSlotTypeUnitType,

	L1IRSlotTypeCapturedTupleType, //arg(16), args (16)
	L1IRSlotTypeCapturedTuple, //arg(16), args (16)
	
	L1IRSlotTypeLambda, //captures (16), prototype block address (hi16, lo16)
	L1IRSlotTypePi, //captures (16), prototype block address (hi16, lo16)

	L1IRSlotTypePair, //first (16), second (16)
	L1IRSlotTypeSigma, //captures (16), prototype block address (hi16, lo16)

	L1IRSlotTypeProjectPair, //pair (16), index (16)

	L1IRSlotTypeCall, //callee (16), argument (16)

	//L1IRSlotTypeLast = L1IRSlotTypeCall,

	L1IRSlotTypeADT, //captures (16), prototype block address (hi16, lo16)
	L1IRSlotTypeConstructor, //adt (16), tag (16), argtype (16)
	L1IRSlotTypeConstructorOf, //adt (16), tag (16)
	L1IRSlotTypeConstructedOf, //adt (16), tag (16), arg (16)

	L1IRSlotTypeBeginDeconstruction, //val (16)
	L1IRSlotTypeDeconstruct, //dcons (16), tag (16), handler (16)
	L1IRSlotTypeDeconstructed, //val (16)
	L1IRSlotTypeEndDeconstruction, //dcons (16)

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

#include <stdbool.h>

#include "L1Array.h"

typedef struct L1IRGlobalState L1IRGlobalState;
struct L1IRGlobalState
{
	L1Array blocks;
};

typedef uint32_t L1IRGlobalAddress;
typedef uint16_t L1IRLocalAddress;

enum L1IRGlobalStateBlockType
{
	L1IRGlobalStateBlockTypeLambda,
	L1IRGlobalStateBlockTypePi,
	L1IRGlobalStateBlockTypeSigma,
	L1IRGlobalStateBlockTypeADT,
};
typedef enum L1IRGlobalStateBlockType L1IRGlobalStateBlockType;

typedef struct L1IRGlobalStateBlock L1IRGlobalStateBlock;
struct L1IRGlobalStateBlock
{
	L1IRGlobalStateBlockType type;
	uint16_t slotCount, argumentLocalAddress;
	L1IRSlot* slots;
	uint64_t hash;
};

typedef struct L1IRLocalState L1IRLocalState;
struct L1IRLocalState
{
	L1Array slots, gcBarriers;
	size_t callDepth;
};

void L1IRLocalStateInitialize(L1IRLocalState* self);
void L1IRLocalStateDeinitialize(L1IRLocalState* self);
L1IRLocalAddress L1IRLocalStateCreateSlot(L1IRLocalState* self, L1IRSlot slot);

void L1IRGlobalStateInitialize(L1IRGlobalState* self);
void L1IRGlobalStateDeinitialize(L1IRGlobalState* self);
L1IRGlobalAddress L1IRGlobalStateCreateBlock(L1IRGlobalState* self, L1IRGlobalStateBlockType type, const L1IRSlot* slots, uint16_t slotCount, L1IRLocalAddress argumentLocalAddress);
L1IRLocalAddress L1IRGlobalStateCall(L1IRGlobalState* self, L1IRLocalState* localState, L1IRGlobalAddress calleeAddress, L1IRLocalAddress argumentLocalAddress);

#endif

