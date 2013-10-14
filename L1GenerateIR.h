#ifndef L1GenerateIR_h
#define L1GenerateIR_h

#include "L1Parser.h"
#include "L1IRBuffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

uint64_t L1GenerateIR(const L1ParserASTNode* node, L1IRBuffer* buffer, uint64_t* nextID);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
