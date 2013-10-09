#ifndef L1FIRGenerator_h
#define L1FIRGenerator_h

#include "L1Parser.h"
#include "L1FIRNode.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct L1FIRGenerator L1FIRGenerator;

L1FIRGenerator* L1FIRGeneratorNew(const L1ParserASTNode* node);
const L1FIRNode* L1FIRGeneratorGetNodes(L1FIRGenerator* self, uint64_t* nodeCount);
void L1FIRGeneratorDelete(L1FIRGenerator* self);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
