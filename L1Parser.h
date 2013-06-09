#ifndef L1Parser_h
#define L1Parser_h

#ifdef __cplusplus
extern "C"
{
#endif

#include "L1ParserInternal.h"
#include "L1ParserNode.h"

void *L1ParserAlloc(void *(*mallocProc)(size_t));
void L1Parser(void*, int, L1ParserNode*, L1ParserNode** outNode);
void L1ParserFree(void *p, void (*freeProc)(void*));

#ifdef __cplusplus
}
//extern "C"
#endif

#endif