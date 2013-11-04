#include "L1ByteBuffer.h"
#include "L1Array.h"
#include <string.h>

struct L1ByteBuffer
{
	L1Array byteArray;
};

L1ByteBuffer* L1ByteBufferNew(void)
{
	L1ByteBuffer* self = malloc(sizeof(L1ByteBuffer));
	L1ArrayInitialize(& self->byteArray);
	return self;
}

void L1ByteBufferDelete(L1ByteBuffer* self)
{
	L1ArrayDeinitialize(& self->byteArray);
	free(self);
}

size_t L1ByteBufferWrite(L1ByteBuffer* self, const void* bytes, size_t byteCount)
{
	size_t index = L1ArrayGetElementCount(& self->byteArray);
	for (size_t i = 0; i < byteCount; i++)
	{
		L1ArrayAppend(& self->byteArray, i + (const char*) bytes, 1);
	}
	return index;
}

void L1ByteBufferRead(L1ByteBuffer* self, size_t index, void* bytes, size_t byteCount)
{
	memcpy(bytes, index + (const char*) L1ArrayGetElements(& self->byteArray), byteCount);
}

const void* L1ByteBufferGetBytes(L1ByteBuffer* self, size_t* byteCount)
{
	if (byteCount) *byteCount = L1ArrayGetElementCount(& self->byteArray);
	return L1ArrayGetElements(& self->byteArray);
}