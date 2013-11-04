#include "L1GenerateIR.h"
#include "L1Array.h"
#include <stdlib.h>
#include <iso646.h>
#include <assert.h>
#include "L1Region.h"
#include <string.h>
#include "L1IRStatementDefines.h"
//#include <stdio.h>

typedef struct Binding Binding;
struct Binding
{
	const uint8_t* bytes;
	uint64_t byteCount;
	uint64_t source;
	const Binding* previous;
};

static uint64_t GenerateID(uint64_t* nextID)
{
	return (*nextID)++;
}

static uint64_t Generate(const L1ParserASTNode* astNode, const Binding* binding, L1ByteBuffer* buffer, uint64_t* nextID)
{
	assert(buffer);
	assert(astNode);
	switch (astNode->type)
	{
		case L1ParserASTNodeTypeNatural:
			{
				uint64_t destination = GenerateID(nextID);
				L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeLoadInteger);
				L1ByteBufferWriteUInt64(buffer, destination);
				L1ByteBufferWriteUInt64(buffer, astNode->data.natural.byteCount);
				L1ByteBufferWrite(buffer, astNode->data.natural.bytes, astNode->data.natural.byteCount);
				return destination;
			}
		case L1ParserASTNodeTypeString:
			{
				uint64_t destination = GenerateID(nextID);
				L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeLoadInteger);
				L1ByteBufferWriteUInt64(buffer, destination);
				L1ByteBufferWriteUInt64(buffer, astNode->data.string.byteCount);
				L1ByteBufferWrite(buffer, astNode->data.string.bytes, astNode->data.string.byteCount);
				return destination;
			}
		case L1ParserASTNodeTypeIdentifier:
			{
				assert(binding);
				const Binding* b = binding;
				
				/*printf("Searching for: ");
				fwrite(astNode->data.identifier.bytes, astNode->data.identifier.byteCount, 1, stdout);
				printf("(byte count: %i)\n", (int)astNode->data.identifier.byteCount);*/
				
				while (b)
				{
					/*printf("Found: ");
					fwrite(b->bytes, b->byteCount, 1, stdout);
					printf("(byte count: %i)\n", (int)b->byteCount);*/
					if((b->byteCount == astNode->data.identifier.byteCount) and (0 == memcmp(b->bytes, astNode->data.identifier.bytes, b->byteCount)))
					{
						return b->source;
					}
					b = b->previous;
				}
				
				abort();
			}
			break;
		case L1ParserASTNodeTypeCall:
			{
				uint64_t callee = Generate(astNode->data.call.callee, binding, buffer, nextID);
				uint64_t destination = GenerateID(nextID);
				
				L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeCall);
				L1ByteBufferWriteUInt64(buffer, destination);
				L1ByteBufferWriteUInt64(buffer, callee);
				
				//Unfortunately we need to iterate through it twice to find the length first
				{
					uint64_t i = 0;
					const L1ParserASTNodeLinkedList* elements = astNode->data.call.arguments;
					while (elements)
					{
						i++;
						elements = elements->tail;
					}
					L1ByteBufferWriteUInt64(buffer, i);
				}
				
				{
					uint64_t i = 0;
					const L1ParserASTNodeLinkedList* elements = astNode->data.call.arguments;
					while (elements)
					{
						uint64_t argument = Generate(elements->head, binding, buffer, nextID);
						L1ByteBufferWriteUInt64(buffer, argument);
						i++;
						elements = elements->tail;
					}
				}
				
				return destination;
			}
		case L1ParserASTNodeTypeAssignment:
			{
				assert(astNode->data.assignment.destination);
				
				
				if (astNode->data.assignment.destination->type == L1ParserASTNodeTypeIdentifier)
				{
					uint64_t source;
					if (astNode->data.assignment.arguments)
					{
						uint64_t closureDestination = GenerateID(nextID);
						Binding selfBinding;
						{
							const static uint8_t bytes[] = "__self";
							selfBinding.bytes = bytes;
							selfBinding.byteCount = sizeof(bytes);
						}
						selfBinding.previous = binding;
						selfBinding.source = closureDestination;
						binding = & selfBinding;
						
						uint64_t argumentCount = 0;
						{
							const L1ParserASTNodeLinkedList* a = astNode->data.assignment.arguments;
							while (a)
							{
								argumentCount++;
								a = a->tail;
							}
						}
						
						assert(argumentCount > 0);
						uint64_t* arguments = malloc(sizeof(uint64_t) * argumentCount);
						{
							Binding* bindings = calloc(1, sizeof(Binding) * argumentCount);
							uint64_t i = 0;
							const L1ParserASTNodeLinkedList* a = astNode->data.assignment.arguments;
							while (a)
							{
								assert(a->head->type == L1ParserASTNodeTypeIdentifier);
								arguments[i] = GenerateID(nextID);
								
								Binding b;
								b.previous = i ? bindings + i  - 1: binding;
								b.source = arguments[i];
								b.bytes = a->head->data.identifier.bytes;
								b.byteCount = a->head->data.identifier.byteCount;
								bindings[i] = b;
								i++;
								a = a->tail;
							}
							
							source = Generate(astNode->data.assignment.source, bindings + i - 1, buffer, nextID);
							
							free(bindings);
						}
						
						L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeClosure);
						L1ByteBufferWriteUInt64(buffer, closureDestination);
						L1ByteBufferWriteUInt64(buffer, source);
						L1ByteBufferWriteUInt64(buffer, argumentCount);
						for (uint64_t i = 0; i < argumentCount; i++)
						{
							L1ByteBufferWriteUInt64(buffer, arguments[i]);
						}
						
						source = closureDestination;
						
						free(arguments);
						
						binding = binding->previous;
					}
					else
					{
						source = Generate(astNode->data.assignment.source, binding, buffer, nextID);
					}
					
					Binding newBinding;
					newBinding.bytes = astNode->data.assignment.destination->data.identifier.bytes;
					newBinding.byteCount = astNode->data.assignment.destination->data.identifier.byteCount;
					newBinding.source = source;
					newBinding.previous = binding;
					
					return Generate(astNode->data.assignment.followingContext, & newBinding, buffer, nextID);
				}
				abort();
			}
		case L1ParserASTNodeTypeBranch:
			{
				uint64_t destination = GenerateID(nextID);
				uint64_t condition = Generate(astNode->data.branch.condition, binding, buffer, nextID);
				uint64_t resultIfTrue = Generate(astNode->data.branch.resultIfTrue, binding, buffer, nextID);
				uint64_t resultIfFalse = Generate(astNode->data.branch.resultIfFalse, binding, buffer, nextID);
				
				L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeBranch);
				L1ByteBufferWriteUInt64(buffer, destination);
				L1ByteBufferWriteUInt64(buffer, condition);
				L1ByteBufferWriteUInt64(buffer, resultIfTrue);
				L1ByteBufferWriteUInt64(buffer, resultIfFalse);
				
				return destination;
			}
		case L1ParserASTNodeTypeList:
			//abort();
			{
				uint64_t destination = GenerateID(nextID);
				
				L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeList);
				L1ByteBufferWriteUInt64(buffer, destination);
				
				{
					uint64_t elementCount = 0;
					const L1ParserASTNodeLinkedList* a = astNode->data.list.elements;
					while (a)
					{
						elementCount++;
						a = a->tail;
					}
					L1ByteBufferWriteUInt64(buffer, elementCount);
				}
				
				{
					const L1ParserASTNodeLinkedList* a = astNode->data.list.elements;
					while (a)
					{
						uint64_t element = Generate(a->head, binding, buffer, nextID);
						L1ByteBufferWriteUInt64(buffer, element);
						a = a->tail;
					}
				}
				
				return destination;
			}
	}
}

uint64_t L1GenerateIR(const L1ParserASTNode* node, L1ByteBuffer* buffer, uint64_t* nextID)
{
	//L1IRStatementTypeLoadIntegerLessThan
	assert(node);
	uint64_t nextID_fallback = 0;
	if (not nextID) nextID = & nextID_fallback;
	
	Binding* binding = NULL;
	
	Binding integerLessThanBinding;
	integerLessThanBinding.previous = binding;
	binding = & integerLessThanBinding;
	{
		binding->source = GenerateID(nextID);
		L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeLoadIntegerLessThan);
		L1ByteBufferWriteUInt64(buffer, binding->source);
		const static uint8_t bytes[] = "__integer_less_than";
		binding->bytes = bytes;
		binding->byteCount = strlen((const char*)bytes);
	}
	
	Binding loadBooleanFromIntegerBinding;
	loadBooleanFromIntegerBinding.previous = binding;
	binding = & loadBooleanFromIntegerBinding;
	{
		binding->source = GenerateID(nextID);
		L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeLoadBooleanFromInteger);
		L1ByteBufferWriteUInt64(buffer, binding->source);
		const static uint8_t bytes[] = "__boolean_from_integer";
		binding->bytes = bytes;
		binding->byteCount = strlen((const char*)bytes);
	}
	
	uint64_t source = Generate(node, binding, buffer, nextID);
	L1ByteBufferWriteUInt8(buffer, L1IRStatementTypeExport);
	L1ByteBufferWriteUInt64(buffer, source);
	return source;
}
