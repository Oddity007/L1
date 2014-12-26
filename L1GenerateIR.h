#ifndef L1GenerateIR_h
#define L1GenerateIR_h

#include "L1Parser.h"
#include "L1IR.h"

L1IRLocalAddress L1GenerateIR(L1IRGlobalState* globalState, L1IRLocalState* localState, const L1ParserASTNode* nodes, size_t nodeCount, size_t rootNodeIndex, const unsigned char* const* tokenStrings, const size_t* tokenStringLengths, size_t tokenStringCount);

#endif

