#ifndef L1IRBuffer_h
#define L1IRBuffer_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum L1IRBufferStatementType L1IRBufferStatementType;

enum L1IRBufferStatementType
{
	L1IRBufferStatementTypeNoOperation,
	
	L1IRBufferStatementTypeLoadInteger,
	L1IRBufferStatementTypeLoadUndefined,
	
	L1IRBufferStatementTypeExport,
	
	L1IRBufferStatementTypeClosure,
	L1IRBufferStatementTypeCall,
	
	L1IRBufferStatementTypeSplitList,
	
	L1IRBufferStatementTypeLoadEmptyList,
	L1IRBufferStatementTypeConsList,
	
	L1IRBufferStatementTypeBranch,
};

typedef struct L1IRBuffer L1IRBuffer;

L1IRBuffer* L1IRBufferNew(void);
void L1IRBufferDelete(L1IRBuffer* self);

/*const uint8_t* L1IRBufferGetStatement(L1IRBuffer* self, uint64_t statementID);

bool L1IRBufferGetNextStatementID(L1IRBuffer* self, uint64_t* statementID);*/

void L1IRBufferCreateNoOperationStatement(L1IRBuffer* self);
void L1IRBufferCreateLoadUndefinedStatement(L1IRBuffer* self, uint64_t destination);
void L1IRBufferCreateLoadIntegerStatement(L1IRBuffer* self, uint64_t destination, const uint8_t* integerBytes, uint64_t integerByteCount);
void L1IRBufferCreateExportStatement(L1IRBuffer* self, uint64_t source);
void L1IRBufferCreateClosureStatement(L1IRBuffer* self, uint64_t destination, uint64_t result, const uint64_t* arguments, uint64_t argumentCount);
void L1IRBufferCreateCallStatement(L1IRBuffer* self, uint64_t destination, uint64_t callee, const uint64_t* arguments, uint64_t argumentCount);
void L1IRBufferCreateSplitListStatement(L1IRBuffer* self, uint64_t headDestination, uint64_t tailDestination, uint64_t source);
void L1IRBufferCreateLoadEmptyListStatement(L1IRBuffer* self, uint64_t destination);
void L1IRBufferCreateConsListStatement(L1IRBuffer* self, uint64_t destination, uint64_t head, uint64_t tail);
void L1IRBufferCreateBranchStatement(L1IRBuffer* self, uint64_t destination, uint64_t condition, uint64_t resultIfTrue, uint64_t resultIfFalse);

void L1IRBufferPrint(L1IRBuffer* self);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
