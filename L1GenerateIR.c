#include "L1GenerateIR.h"
#include "L1Array.h"
#include <stdlib.h>
#include <iso646.h>
#include <assert.h>
#include "L1Region.h"
#include <string.h>

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

static uint64_t Generate(const L1ParserASTNode* astNode, const Binding* binding, L1IRBuffer* buffer, uint64_t* nextID)
{
	assert(buffer);
	assert(astNode);
	switch (astNode->type)
	{
		case L1ParserASTNodeTypeNatural:
			{
				uint64_t destination = GenerateID(nextID);
				L1IRBufferCreateLoadIntegerStatement(buffer, destination, astNode->data.identifier.bytes, astNode->data.natural.byteCount);
				return destination;
			}
		case L1ParserASTNodeTypeString:
			{
				uint64_t destination = GenerateID(nextID);
				L1IRBufferCreateLoadIntegerStatement(buffer, destination, astNode->data.string.bytes, astNode->data.string.byteCount);
				return destination;
			}
		case L1ParserASTNodeTypeIdentifier:
			{
				assert(binding);
				const Binding* b = binding;
				while (b)
				{
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
				uint64_t argumentCount = 0;
				
				{
					const L1ParserASTNodeLinkedList* elements = astNode->data.call.arguments;
					while (elements)
					{
						argumentCount++;
						elements = elements->tail;
					}
				}
				
				uint64_t* arguments = malloc(sizeof(uint64_t) * argumentCount);
				
				{
					uint64_t i = 0;
					const L1ParserASTNodeLinkedList* elements = astNode->data.call.arguments;
					while (elements)
					{
						arguments[i] = Generate(elements->head, binding, buffer, nextID);
						i++;
						elements = elements->tail;
					}
				}
				
				uint64_t closure = Generate(astNode->data.call.callee, binding, buffer, nextID);
				uint64_t destination = GenerateID(nextID);
				L1IRBufferCreateCallStatement(buffer, destination, closure, arguments, argumentCount);
				
				free(arguments);
				
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
					
						L1IRBufferCreateClosureStatement(buffer, closureDestination, source, arguments, argumentCount);
						
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
				L1IRBufferCreateBranchStatement(buffer, destination, condition, resultIfTrue, resultIfFalse);
				return destination;
			}
		case L1ParserASTNodeTypeList:
			abort();
			{
				uint64_t destination = GenerateID(nextID);
				L1IRBufferCreateLoadEmptyListStatement(buffer, destination);
				
				uint64_t elementCount = 0;
				{
					const L1ParserASTNodeLinkedList* a = astNode->data.list.elements;
					while (a)
					{
						elementCount++;
						a = a->tail;
					}
				}
				
				const L1ParserASTNode** elements = calloc(1, sizeof(const L1ParserASTNode*) * elementCount);
				{
					const L1ParserASTNodeLinkedList* a = astNode->data.list.elements;
					for (uint64_t i = 0; i < elementCount; i++)
					{
						elements[i] = a->head;
						a = a->tail;
					}
				}
				
				{
					for (uint64_t i = elementCount; i-- > 0;)
					{
						uint64_t newDestination = GenerateID(nextID);
						L1IRBufferCreateConsListStatement(buffer, newDestination, Generate(elements[i], binding, buffer, nextID), destination);
						destination = newDestination;
					}
				}
				
				free(elements);
				
				return destination;
			}
	}
}

uint64_t L1GenerateIR(const L1ParserASTNode* node, L1IRBuffer* buffer, uint64_t* nextID)
{
	uint64_t nextID_fallback = 0;
	if (not nextID) nextID = & nextID_fallback;
	uint64_t source = Generate(node, NULL, buffer, nextID);
	L1IRBufferCreateExportStatement(buffer, source);
	return source;
}
