#ifndef L1ByteBuffer_h
#define L1ByteBuffer_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>

typedef struct L1ByteBuffer L1ByteBuffer;

L1ByteBuffer* L1ByteBufferNew(void);
void L1ByteBufferDelete(L1ByteBuffer* self);

size_t L1ByteBufferWrite(L1ByteBuffer* self, const void* bytes, size_t byteCount);
void L1ByteBufferRead(L1ByteBuffer* self, size_t index, void* bytes, size_t byteCount);

static size_t L1ByteBufferWriteUInt8(L1ByteBuffer* self, uint8_t value)
{
	return L1ByteBufferWrite(self, & value, sizeof(value));
}

static size_t L1ByteBufferWriteUInt64(L1ByteBuffer* self, uint64_t value)
{
	return L1ByteBufferWrite(self, & value, sizeof(value));
}

const void* L1ByteBufferGetBytes(L1ByteBuffer* self, size_t* byteCount);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif