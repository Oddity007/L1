#include "L1Array.h"
#include <iso646.h>
#include <string.h>
#include "L1IRBuffer.h"

struct L1IRBuffer
{
	L1Array byteArray;
};

L1IRBuffer* L1IRBufferNew(void)
{
	L1IRBuffer* self = calloc(1, sizeof(L1IRBuffer));
	L1ArrayInitialize(& self->byteArray);
	
	return self;
}

void L1IRBufferDelete(L1IRBuffer* self)
{
	if (not self) return;
	
	L1ArrayDeinitialize(& self->byteArray);
	free(self);
}

/*const uint8_t* L1IRBufferGetStatement(L1IRBuffer* self, uint64_t statementID)
{
	return statementID + (const uint8_t*) L1ArrayGetElements(& self->byteArray);
}

bool L1IRBufferGetNextStatementID(L1IRBuffer* self, uint64_t* statementID)
{
	uint64_t i = statementID ? *statementID : 0;
	if (i >= L1ArrayGetElementCount(& self->byteArray)) return false;
	
	const uint8_t* bytes = L1ArrayGetElements(& self->byteArray);
	
	switch (bytes[i])
	{
		case L1IRBufferStatementTypeNoOperation:
			i++;
			break;
		case L1IRBufferStatementTypeLoadInteger:
			{
				i += 1 + sizeof(uint64_t);
				uint64_t byteCount;
				memcpy(& byteCount, bytes + i, sizeof(uint64_t));
				i += sizeof(uint64_t) + byteCount;
			}
			break;
		case L1IRBufferStatementTypeLoadUndefined:
			i += 1 + sizeof(uint64_t);
			break;
		case L1IRBufferStatementTypeExport:
			i += 1 + sizeof(uint64_t);
			break;
		case L1IRBufferStatementTypeClosure:
			break;
		default:
			abort();
			break;
	}
	
	if (statementID) *statementID = i;
	
	return true;
	
	abort();
	return false;
}*/

static void L1IRBufferAppendBytes(L1IRBuffer* self, const void* bytes, size_t byteCount)
{
	for (size_t i = 0; i < byteCount; i++)
	{
		L1ArrayAppend(& self->byteArray, (const uint8_t*)bytes + i, 1);
	}
}

void L1IRBufferCreateNoOperationStatement(L1IRBuffer* self)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeNoOperation}, 1);
}

void L1IRBufferCreateLoadUndefinedStatement(L1IRBuffer* self, uint64_t destination)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeLoadUndefined}, 1);
	L1IRBufferAppendBytes(self, & destination, sizeof(uint64_t));
}

void L1IRBufferCreateLoadIntegerStatement(L1IRBuffer* self, uint64_t destination, const uint8_t* integerBytes, uint64_t integerByteCount)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeLoadInteger}, 1);
	L1IRBufferAppendBytes(self, & destination, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & integerByteCount, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, integerBytes, integerByteCount);
}

void L1IRBufferCreateExportStatement(L1IRBuffer* self, uint64_t source)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeExport}, 1);
	L1IRBufferAppendBytes(self, & source, sizeof(uint64_t));
}

void L1IRBufferCreateClosureStatement(L1IRBuffer* self, uint64_t destination, uint64_t result, const uint64_t* arguments, uint64_t argumentCount)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeClosure}, 1);
	L1IRBufferAppendBytes(self, & destination, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & result, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & argumentCount, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, arguments, argumentCount * sizeof(uint64_t));
}

void L1IRBufferCreateCallStatement(L1IRBuffer* self, uint64_t destination, uint64_t callee, const uint64_t* arguments, uint64_t argumentCount)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeCall}, 1);
	L1IRBufferAppendBytes(self, & destination, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & callee, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & argumentCount, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, arguments, argumentCount * sizeof(uint64_t));
}

void L1IRBufferCreateSplitListStatement(L1IRBuffer* self, uint64_t headDestination, uint64_t tailDestination, uint64_t source)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeSplitList}, 1);
	L1IRBufferAppendBytes(self, & headDestination, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & tailDestination, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & source, sizeof(uint64_t));
}

void L1IRBufferCreateLoadEmptyListStatement(L1IRBuffer* self, uint64_t destination)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeLoadEmptyList}, 1);
	L1IRBufferAppendBytes(self, & destination, sizeof(uint64_t));
}

void L1IRBufferCreateConsListStatement(L1IRBuffer* self, uint64_t destination, uint64_t head, uint64_t tail)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeConsList}, 1);
	L1IRBufferAppendBytes(self, & destination, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & head, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & tail, sizeof(uint64_t));
}

void L1IRBufferCreateBranchStatement(L1IRBuffer* self, uint64_t destination, uint64_t condition, uint64_t resultIfTrue, uint64_t resultIfFalse)
{
	L1IRBufferAppendBytes(self, (const uint8_t[1]){L1IRBufferStatementTypeBranch}, 1);
	L1IRBufferAppendBytes(self, & destination, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & condition, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & resultIfTrue, sizeof(uint64_t));
	L1IRBufferAppendBytes(self, & resultIfFalse, sizeof(uint64_t));
}

/*uint64_t L1IRBufferCreateLoadUndefinedStatement(L1IRBuffer* self)
{
	uint8_t byte = L1IRBufferStatementTypeLoadUndefined;
	L1IRBufferAppendBytes(self, & byte, 1);
}

uint64_t L1IRBufferCreateLoadIntegerStatement(L1IRBuffer* self, const uint8_t* integerBytes, uint64_t integerByteCount)
{
	uint8_t* bytes = malloc(1 + sizeof(uint64_t) + integerByteCount);
	bytes[0] = L1IRBufferStatementTypeLoadInteger;
	memcpy(bytes + 1, & integerByteCount, sizeof(uint64_t));
	memcpy(bytes + 1 + sizeof(uint64_t), integerBytes, integerByteCount);
	uint64_t statementID = L1IRBufferCreateStatement(self, bytes, 1 + sizeof(uint64_t) + integerByteCount);
	free(bytes);
	return statementID;
}

uint64_t L1IRBufferCreateExportStatement(L1IRBuffer* self, uint64_t exportedStatementID)
{
	uint8_t bytes[1 + sizeof(uint64_t)];
	bytes[0] = L1IRBufferStatementTypeExport;
	memcpy(bytes + 1, & exportedStatementID, sizeof(uint64_t));
	return L1IRBufferCreateStatement(self, bytes, 1 + sizeof(uint64_t));
}

uint64_t L1IRBufferCreateClosureStatement(L1IRBuffer* self, uint64_t result, const uint64_t* arguments, uint64_t argumentCount)
{
	uint8_t* bytes = malloc(1 + 2 * sizeof(uint64_t) + sizeof(uint64_t) * argumentCount);
	bytes[0] = L1IRBufferStatementTypeClosure;
	memcpy(bytes + 1, & result, sizeof(uint64_t));
	memcpy(bytes + 1 + sizeof(uint64_t), & argumentCount, sizeof(uint64_t));
	memcpy(bytes + 1 + 2 * sizeof(uint64_t), arguments, sizeof(uint64_t) * argumentCount);
	uint64_t statementID = L1IRBufferCreateStatement(self, bytes, 1 + 2 * sizeof(uint64_t) + sizeof(uint64_t) * argumentCount);
	free(bytes);
	return statementID;
}

uint64_t L1IRBufferCreateCallStatement(L1IRBuffer* self, uint64_t callee, const uint64_t* arguments, uint64_t argumentCount)
{
	uint8_t* bytes = malloc(1 + 2 * sizeof(uint64_t) + sizeof(uint64_t) * argumentCount);
	bytes[0] = L1IRBufferStatementTypeCall;
	memcpy(bytes + 1, & callee, sizeof(uint64_t));
	memcpy(bytes + 1 + sizeof(uint64_t), & argumentCount, sizeof(uint64_t));
	memcpy(bytes + 1 + 2 * sizeof(uint64_t), arguments, sizeof(uint64_t) * argumentCount);
	uint64_t statementID = L1IRBufferCreateStatement(self, bytes, 1 + 2 * sizeof(uint64_t) + sizeof(uint64_t) * argumentCount);
	free(bytes);
	return statementID;
}

uint64_t L1IRBufferCreateGetListHeadStatement(L1IRBuffer* self, uint64_t list)
{
	uint8_t* bytes = malloc(1 + sizeof(uint64_t));
	bytes[0] = L1IRBufferStatementTypeGetListHead;
	memcpy(bytes + 1, & list, sizeof(uint64_t));
	uint64_t statementID = L1IRBufferCreateStatement(self, bytes, 1 + sizeof(uint64_t));
	free(bytes);
	return statementID;
}

uint64_t L1IRBufferCreateGetListTailStatement(L1IRBuffer* self, uint64_t list)
{
	uint8_t* bytes = malloc(1 + sizeof(uint64_t));
	bytes[0] = L1IRBufferStatementTypeGetListTail;
	memcpy(bytes + 1, & list, sizeof(uint64_t));
	uint64_t statementID = L1IRBufferCreateStatement(self, bytes, 1 + sizeof(uint64_t));
	free(bytes);
	return statementID;
}

uint64_t L1IRBufferCreateLoadEmptyListStatement(L1IRBuffer* self)
{
	uint8_t byte = L1IRBufferStatementTypeLoadEmptyList;
	return L1IRBufferCreateStatement(self, & byte, 1);
}

uint64_t L1IRBufferCreateConsListStatement(L1IRBuffer* self, uint64_t head, uint64_t tail)
{
	uint8_t* bytes = malloc(1 + 2 * sizeof(uint64_t));
	bytes[0] = L1IRBufferStatementTypeConsList;
	memcpy(bytes + 1, & head, sizeof(uint64_t));
	memcpy(bytes + 1 + sizeof(uint64_t), & tail, sizeof(uint64_t));
	uint64_t statementID = L1IRBufferCreateStatement(self, bytes, 1 + 2 * sizeof(uint64_t));
	free(bytes);
	return statementID;
}*/
