#ifndef L1IRState_h
#define L1IRState_h

#include <stdint.h>

#include "L1IR.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "L1Array.h"

typedef struct L1IRGlobalState L1IRGlobalState;
typedef struct L1IRLocalState L1IRLocalState;

struct L1IRGlobalState
{
	L1Array blocks;
};

struct L1IRLocalState
{
	L1Array slots, gcBarriers;
	size_t callDepth;
};

typedef uint16_t L1IRGlobalAddress;
typedef uint16_t L1IRLocalAddress;

enum L1IRGlobalStateBlockType
{
	L1IRGlobalStateBlockTypeForeignFunction,
	L1IRGlobalStateBlockTypeLambda,
	L1IRGlobalStateBlockTypePi,
	L1IRGlobalStateBlockTypeSigma,
	L1IRGlobalStateBlockTypeADT,
};
typedef enum L1IRGlobalStateBlockType L1IRGlobalStateBlockType;

typedef struct L1IRGlobalStateBlock L1IRGlobalStateBlock;

typedef L1IRLocalAddress (*L1IRGlobalStateBlockCallback)(L1IRGlobalState* globalState, L1IRLocalState* localState, L1IRGlobalAddress selfGlobalAddress, L1IRLocalAddress argumentLocalAddress, L1IRLocalAddress* finalArgumentLocalAddress, void* userdata);

struct L1IRGlobalStateBlock
{
	L1IRGlobalStateBlockType type;
	union
	{
		struct
		{
			L1IRSlot* slots;
			uint16_t slotCount;
		} native;
		
		struct
		{
			L1IRGlobalStateBlockCallback callback;
			void* userdata;
		} foreign;
	};
};

static bool L1IRGlobalStateBlockIsNative(const L1IRGlobalStateBlock* block)
{
	return true;
	//return block->type not_eq L1IRGlobalStateBlockTypeForeignFunction;
}

void L1IRLocalStateInitialize(L1IRLocalState* self);
void L1IRLocalStateDeinitialize(L1IRLocalState* self);
L1IRLocalAddress L1IRLocalStateCreateSlot(L1IRLocalState* self, L1IRSlot slot);

void L1IRGlobalStateInitialize(L1IRGlobalState* self);
void L1IRGlobalStateDeinitialize(L1IRGlobalState* self);
L1IRGlobalAddress L1IRGlobalStateCreateNativeBlock(L1IRGlobalState* self, L1IRGlobalStateBlockType type, const L1IRSlot* slots, uint16_t slotCount, L1IRLocalAddress argumentLocalAddress);
L1IRGlobalAddress L1IRGlobalStateCreateForeignBlock(L1IRGlobalState* self, L1IRGlobalStateBlockType type, L1IRGlobalStateBlockCallback callback, void* userdata);
L1IRLocalAddress L1IRGlobalStateCall(L1IRGlobalState* self, L1IRLocalState* localState, L1IRGlobalAddress calleeAddress, L1IRLocalAddress argumentLocalAddress);

//Internal

enum L1IRGlobalStateEvaluationFlags
{
	L1IRGlobalStateEvaluationFlagHasArgument = 1,
	L1IRGlobalStateEvaluationFlagHasCaptured = 1 << 1,
	L1IRGlobalStateEvaluationFlagHasArgumentAndCaptured = L1IRGlobalStateEvaluationFlagHasArgument | L1IRGlobalStateEvaluationFlagHasCaptured,
};

typedef uint32_t L1IRGlobalStateEvaluationFlags;

L1IRLocalAddress L1IRGlobalStateEvaluate(L1IRGlobalState* self, L1IRLocalState* localState, L1IRGlobalStateEvaluationFlags flags, L1IRGlobalAddress calleeAddress, L1IRLocalAddress argumentLocalAddress, L1IRLocalAddress captureLocalAddress, L1IRLocalAddress* finalArgumentLocalAddress);

uint16_t L1IRGlobalStateCreateSlot(L1IRGlobalState* self, L1IRLocalState* localState, L1IRSlot slot);

bool L1IRGlobalStateIsOfType(L1IRGlobalState* self, L1IRLocalState* localState, uint16_t valueLocalAddress, uint16_t typeLocalAddress);

void L1IRGlobalStatePushGCBarrier(L1IRGlobalState* self, L1IRLocalState* localState);
void L1IRGlobalStatePopGCBarrier(L1IRGlobalState* self, L1IRLocalState* localState, uint16_t* roots, size_t rootCount);

#endif

