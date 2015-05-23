#ifndef L1GenerateIR_h
#define L1GenerateIR_h

#include "L1Parser.h"
#include "L1IRState.h"

L1IRGlobalAddress L1GenerateIR(L1IRGlobalState* globalState, const L1ParserASTNode* nodes, size_t nodeCount, size_t rootNodeIndex, const unsigned char* const* tokenStrings, const size_t* tokenStringLengths, size_t tokenStringCount);

#endif

