#include "L1Array.h"
#include <iso646.h>

void L1ArrayInitialize(L1Array* self)
{
	self->elements = NULL;
	self->elementCount = 0;
	self->elementByteCount = 0;
	self->allocatedElementCount = 0;
}

void L1ArrayDeinitialize(L1Array* self)
{
	free(self->elements);
}

void L1ArraySetElementCount(L1Array* self, size_t elementCount, size_t elementByteCount)
{
	assert(elementByteCount not_eq 0);
	assert(self->elementByteCount == 0 or self->elementByteCount == elementByteCount);
	
	self->elementByteCount = elementByteCount;
	
	self->elementCount = elementCount;
	
	if (self->allocatedElementCount >= elementCount)
	{
		if (self->allocatedElementCount / 4 >= elementCount)
		{
			self->allocatedElementCount /= 4;
			self->elements = realloc(self->elements, self->allocatedElementCount * elementByteCount);
		}
		return;
	}
	
	if (self->allocatedElementCount == 0) self->allocatedElementCount = 1;
	
	do
	{
		self->allocatedElementCount *= 2;
	}
	while (self->allocatedElementCount < elementCount);
	
	self->elements = realloc(self->elements, self->allocatedElementCount * elementByteCount);
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

void* L1ArrayGetElements(L1Array* self)
{
	return self->elements;
}

size_t L1ArrayGetElementCount(L1Array* self)
{
	return self->elementCount;
}

void L1ArrayPush(L1Array* self, const void* bytes, size_t elementByteCount)
{
	L1ArrayAppend(self, bytes, elementByteCount);
}

void L1ArrayPop(L1Array* self, void* bytes, size_t elementByteCount)
{
	size_t elementCount = L1ArrayGetElementCount(self);
	assert(elementCount > 0);
	if (bytes) memcpy(bytes, elementByteCount * (elementCount - 1) + (char*) L1ArrayGetElements(self), elementByteCount);
	L1ArraySetElementCount(self, elementCount - 1, elementByteCount);
}

void L1ArrayPeek(L1Array* self, void* bytes, size_t elementByteCount)
{
	size_t elementCount = L1ArrayGetElementCount(self);
	assert(elementCount > 0);
	if (bytes) memcpy(bytes, elementByteCount * (elementCount - 1) + (char*) L1ArrayGetElements(self), elementByteCount);
}
