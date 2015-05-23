#ifndef L1Array_h
#define L1Array_h

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct L1Array L1Array;
struct L1Array
{
	void* elements;
	size_t elementCount, elementByteCount, allocatedElementCount;
};

void L1ArrayInitialize(L1Array* self);

void L1ArrayDeinitialize(L1Array* self);

void L1ArraySetElementCount(L1Array* self, size_t elementCount, size_t elementByteCount);

void L1ArrayAppend(L1Array* self, const void* bytes, size_t elementByteCount);

void* L1ArrayGetElements(L1Array* self);

size_t L1ArrayGetElementCount(L1Array* self);

void L1ArrayPush(L1Array* self, const void* bytes, size_t elementByteCount);
void L1ArrayInsert(L1Array* self, const void* bytes, size_t elementByteCount, size_t elementIndex);
void L1ArrayPop(L1Array* self, void* bytes, size_t elementByteCount);
void L1ArrayPeek(L1Array* self, void* bytes, size_t elementByteCount);

#ifdef __cplusplus
}//extern "C"
#endif

#endif
