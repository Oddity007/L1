#ifndef L1IRState_h
#define L1IRState_h

#include <stdint.h>

#include "L1IR.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "L1Array.h"

typedef struct L1IRState L1IRState;

typedef uint16_t L1IRAddress;
typedef size_t L1IRStateExternalSlotRef;

struct L1IRState
{
	L1Array slots, externalSlotRefs;
	/*L1IRAddress (*foreignTypeOf)(L1IRState* state, L1IRAddress nameRef);
	L1IRAddress (*foreignSubstitute)(L1IRState* state, L1IRAddress nameRef, L1IRAddress argumentRef);
	L1IRAddress (*foreignNormalize)(L1IRState* state, L1IRAddress nameRef);
	void (*foreignDealloc)(L1IRState* state, L1IRAddress nameRef);*/
	//L1Array slotReferenceCounts;
};

/*typedef struct L1IRStateSlotView L1IRStateSlotView;

struct L1IRStateSlotView
{
	L1IRState* state;
	L1IRAddress viewRef;
};*/

void L1IRStateInitialize(L1IRState* self);
void L1IRStateDeinitialize(L1IRState* self);
void L1IRStateCollectGarbage(L1IRState* self);
L1IRAddress L1IRStateCreateSlot(L1IRState* self, L1IRSlot slot);
L1IRSlot L1IRStateGetSlot(L1IRState* self, L1IRAddress address);

//L1IRAddress L1IRStateRetainSlot(L1IRState* self, L1IRAddress slotRef);
//void L1IRStateReleaseSlot(L1IRState* self, L1IRAddress slotRef);

//void L1IRStateSlotViewInitialize(L1IRStateSlotView* self, L1IRState* state, L1IRAddress slotRef);
//void L1IRStateSlotViewDeinitialize(L1IRStateSlotView* self);

L1IRStateExternalSlotRef L1IRStateAcquireExternalSlotRef(L1IRState* self, L1IRAddress slotRef);
void L1IRStateReleaseExternalSlotRef(L1IRState* self, L1IRStateExternalSlotRef ref);
L1IRAddress L1IRStateGetExternalSlotRefCurrentSlotRef(L1IRState* self, L1IRStateExternalSlotRef ref);

#endif
