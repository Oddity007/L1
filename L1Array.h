#ifndef L1Array_h
#define L1Array_h

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct L1Array L1Array;
struct L1Array
{
	void* elements;
	size_t elementCount;
};

static void L1ArrayInitialize(L1Array* self)
{
	self->elements = NULL;
	self->elementCount = 0;
}

static void L1ArrayDeinitialize(L1Array* self)
{
	free(self->elements);
}

static void L1ArraySetElementCount(L1Array* self, size_t elementCount, size_t elementByteCount)
{
	self->elementCount = elementCount;
	self->elements = realloc(self->elements, elementCount * elementByteCount);
}

static void L1ArrayAppend(L1Array* self, const void* bytes, size_t elementByteCount)
{
	L1ArraySetElementCount(self, self->elementCount + 1, elementByteCount);
	const char* s = bytes;
	char* d = self->elements + (self->elementCount - 1) * elementByteCount;
	for (size_t i = 0; i < elementByteCount; i++)
	{
		d[i] = s[i];
	}
}

static void* L1ArrayGetElements(L1Array* self)
{
	return self->elements;
}

static size_t L1ArrayGetElementCount(L1Array* self)
{
	return self->elementCount;
}

#ifdef __cplusplus
}//extern "C"
#endif

#endif