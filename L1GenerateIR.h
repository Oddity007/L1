#ifndef L1GenerateIR_h
#define L1GenerateIR_h

#include "L1Parser.h"
#include "L1ByteBuffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

uint64_t L1GenerateIR(const L1ParserASTNode* node, L1ByteBuffer* buffer, uint64_t* nextID);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
