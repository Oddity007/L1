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

#include <stdio.h>
#include <inttypes.h>

void L1IRBufferPrint(L1IRBuffer* self)
{
	const uint8_t* bytes = L1ArrayGetElements(& self->byteArray);
	size_t byteCount = L1ArrayGetElementCount(& self->byteArray);
	for (uint64_t i = 0; i < byteCount;)
	{
		switch (bytes[i])
		{
			case L1IRBufferStatementTypeNoOperation:
				puts("no_operation");
				i++;
				break;
			case L1IRBufferStatementTypeLoadUndefined:
				{
					uint64_t destination;
					i++;
					memcpy(& destination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("load_undefined %" PRIu64 "\n", destination);
				}
				break;
			case L1IRBufferStatementTypeLoadInteger:
				{
					uint64_t destination;
					i++;
					memcpy(& destination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					uint64_t integerByteCount;
					memcpy(& integerByteCount, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("load_integer %" PRIu64 " %" PRIu64 " [ ", destination, integerByteCount);
					for (uint64_t j = 0; j < integerByteCount; j++)
					{
						printf("%" PRIu8 " ", bytes[i]);
						i++;
					}
					puts("]");
				}
				break;
			case L1IRBufferStatementTypeExport:
				{
					uint64_t source;
					i++;
					memcpy(& source, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("export %" PRIu64 "\n", source);
				}
				break;
			case L1IRBufferStatementTypeClosure:
				{
					i++;
					uint64_t destination, result, argumentCount;
					memcpy(& destination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& result, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& argumentCount, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("closure %" PRIu64 " %" PRIu64 " %" PRIu64 " [ ", destination, result, argumentCount);
					for (uint64_t j = 0; j < argumentCount; j++)
					{
						uint64_t argument;
						memcpy(& argument, bytes + i, sizeof(uint64_t));
						printf("%" PRIu64 " ", argument);
						i += sizeof(uint64_t);
					}
					puts("]");
				}
				break;
			case L1IRBufferStatementTypeCall:
				{
					i++;
					uint64_t destination, callee, argumentCount;
					memcpy(& destination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& callee, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& argumentCount, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("call %" PRIu64 " %" PRIu64 " %" PRIu64 " [ ", destination, callee, argumentCount);
					for (uint64_t j = 0; j < argumentCount; j++)
					{
						uint64_t argument;
						memcpy(& argument, bytes + i, sizeof(uint64_t));
						printf("%" PRIu64 " ", argument);
						i += sizeof(uint64_t);
					}
					puts("]");
				}
				break;
			case L1IRBufferStatementTypeSplitList:
				{
					i++;
					uint64_t headDestination, tailDestination, source;
					memcpy(& headDestination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& tailDestination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& source, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("split_list %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", headDestination, tailDestination, source);
				}
				break;
			case L1IRBufferStatementTypeLoadEmptyList:
				{
					uint64_t destination;
					i++;
					memcpy(& destination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("load_empty_list %" PRIu64 "\n", destination);
				}
				break;
			case L1IRBufferStatementTypeConsList:
				{
					i++;
					uint64_t destination, head, tail;
					memcpy(& destination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& head, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& tail, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("cons_list %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", destination, head, tail);
				}
				break;
			case L1IRBufferStatementTypeBranch:
				{
					i++;
					uint64_t destination, condition, resultIfTrue, resultIfFalse;
					memcpy(& destination, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& condition, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& resultIfTrue, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					memcpy(& resultIfFalse, bytes + i, sizeof(uint64_t));
					i += sizeof(uint64_t);
					printf("branch %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", destination, condition, resultIfTrue, resultIfFalse);
				}
				break;
			default:
				puts("unknown_data");
				return;
		}
	}
}
