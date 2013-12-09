#ifndef L1GenerateIR_h
#define L1GenerateIR_h

#include "L1Parser.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
	void (*loadInteger)(uint64_t destination, uint64_t byteCount, const uint8_t* bytes, void* userdata);
	void (*call)(uint64_t destination, uint64_t callee, uint64_t argumentCount, const uint64_t* arguments, void* userdata);
	void (*closure)(uint64_t destination, uint64_t result, uint64_t argumentCount, const uint64_t* arguments, void* userdata);
	void (*branch)(uint64_t destination, uint64_t condition, uint64_t resultIfTrue, uint64_t resultIfFalse, void* userdata);
	void (*list)(uint64_t destination, uint64_t elementCount, const uint64_t* elements, void* userdata);
	void (*loadIntegerLessThan)(uint64_t destination, void* userdata);
	void (*loadBooleanFromInteger)(uint64_t destination, void* userdata);
	void (*export)(uint64_t source, void* userdata);
	void (*loadUndefined)(uint64_t destination, void* userdata);
	void (*loadIntegerAdd)(uint64_t destination, void* userdata);
}L1GenerateIROutputFunctions;

uint64_t L1GenerateIR(const L1ParserASTNode* node, uint64_t* nextID, const L1GenerateIROutputFunctions* outputFunctions, void* userdata);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
