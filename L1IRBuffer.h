#ifndef L1IRBuffer_h
#define L1IRBuffer_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

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

size_t L1IRBufferCreateNoOperationStatement(L1IRBuffer* self);
size_t L1IRBufferCreateLoadUndefinedStatement(L1IRBuffer* self, uint64_t destination);
size_t L1IRBufferCreateLoadIntegerStatement(L1IRBuffer* self, uint64_t destination, const uint8_t* integerBytes, uint64_t integerByteCount);
size_t L1IRBufferCreateExportStatement(L1IRBuffer* self, uint64_t source);
size_t L1IRBufferCreateClosureStatement(L1IRBuffer* self, uint64_t destination, uint64_t result, const uint64_t* arguments, uint64_t argumentCount);
size_t L1IRBufferCreateCallStatement(L1IRBuffer* self, uint64_t destination, uint64_t callee, const uint64_t* arguments, uint64_t argumentCount);
size_t L1IRBufferCreateSplitListStatement(L1IRBuffer* self, uint64_t headDestination, uint64_t tailDestination, uint64_t source);
size_t L1IRBufferCreateLoadEmptyListStatement(L1IRBuffer* self, uint64_t destination);
size_t L1IRBufferCreateConsListStatement(L1IRBuffer* self, uint64_t destination, uint64_t head, uint64_t tail);
size_t L1IRBufferCreateBranchStatement(L1IRBuffer* self, uint64_t destination, uint64_t condition, uint64_t resultIfTrue, uint64_t resultIfFalse);

L1IRBufferStatementType L1IRBufferGetStatementType(L1IRBuffer* self, size_t statementIndex);

void L1IRBufferGetLoadUndefinedStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* destination);
void L1IRBufferGetLoadIntegerStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* destination, uint8_t* integerBytes, uint64_t* integerByteCount);
void L1IRBufferGetExportStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* source);
void L1IRBufferGetClosureStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* destination, uint64_t* result, uint64_t* arguments, uint64_t* argumentCount);
void L1IRBufferGetCallStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* destination, uint64_t* callee, uint64_t* arguments, uint64_t* argumentCount);
void L1IRBufferGetSplitListStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* headDestination, uint64_t* tailDestination, uint64_t* source);
void L1IRBufferGetLoadEmptyListStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* destination);
void L1IRBufferGetConsListStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* destination, uint64_t* head, uint64_t* tail);
void L1IRBufferGetBranchStatement(L1IRBuffer* self, size_t statementIndex, uint64_t* destination, uint64_t* condition, uint64_t* resultIfTrue, uint64_t* resultIfFalse);

size_t L1IRBufferGetNextStatement(L1IRBuffer* self, size_t last);

void L1IRBufferPrint(L1IRBuffer* self);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
