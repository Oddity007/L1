#include "L1Array.h"

void L1ArrayInitialize(L1Array* self)
{
	self->elements = NULL;
	self->elementCount = 0;
}

void L1ArrayDeinitialize(L1Array* self)
{
	free(self->elements);
}

void L1ArraySetElementCount(L1Array* self, size_t elementCount, size_t elementByteCount)
{
	self->elementCount = elementCount;
	self->elements = realloc(self->elements, elementCount * elementByteCount);
}

void L1ArrayAppend(L1Array* self, const void* bytes, size_t elementByteCount)
{
	L1ArraySetElementCount(self, self->elementCount + 1, elementByteCount);
	const char* s = bytes;
	char* d = self->elements;
	d += (self->elementCount - 1) * elementByteCount;
	for (size_t i = 0; i < elementByteCount; i++)
	{
		d[i] = s[i];
	}
}
