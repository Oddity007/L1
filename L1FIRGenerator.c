#include "L1FIRGenerator.h"
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

struct L1FIRGenerator
{
	L1Array nodeArray;
	uint64_t nextIdentifier;
	L1Region* region;
};

static uint64_t GenerateID(L1FIRGenerator* generator)
{
	assert(0 < generator->nextIdentifier + 1);
	return generator->nextIdentifier++;
}

static uint64_t Generate(L1FIRGenerator* generator, const L1ParserASTNode* astNode, const Binding* binding)
{
	assert(generator);
	assert(astNode);
	switch (astNode->type)
	{
		case L1ParserASTNodeTypeNatural:
			{
				uint64_t destination = GenerateID(generator);
				
				uint64_t byteCount = astNode->data.natural.byteCount;
				const uint8_t* bytes = memcpy(L1RegionAllocate(generator->region, byteCount), astNode->data.identifier.bytes, byteCount);
				
				L1FIRNode firNode;
				firNode.type = L1FIRNodeTypeLoadInteger;
				firNode.data.loadInteger.destination = destination;
				firNode.data.loadInteger.digits = bytes;
				firNode.data.loadInteger.digitCount = byteCount;
				L1ArrayAppend(& generator->nodeArray, & firNode, sizeof(firNode));
				
				return destination;
			}
		case L1ParserASTNodeTypeString:
			{
				uint64_t destination = GenerateID(generator);
				
				uint64_t byteCount = astNode->data.string.byteCount;
				const uint8_t* bytes = memcpy(L1RegionAllocate(generator->region, byteCount), astNode->data.string.bytes, byteCount);
				
				L1FIRNode firNode;
				firNode.type = L1FIRNodeTypeLoadString;
				firNode.data.loadString.destination = destination;
				firNode.data.loadString.bytes = bytes;
				firNode.data.loadString.byteCount = byteCount;
				L1ArrayAppend(& generator->nodeArray, & firNode, sizeof(firNode));
				
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
				
				uint64_t* arguments = L1RegionAllocate(generator->region, sizeof(uint64_t) * argumentCount);
				
				{
					uint64_t i = 0;
					const L1ParserASTNodeLinkedList* elements = astNode->data.call.arguments;
					while (elements)
					{
						arguments[i] = Generate(generator, elements->head, binding);
						i++;
						elements = elements->tail;
					}
				}
				
				uint64_t destination = GenerateID(generator);
				assert(astNode->data.call.callee);
				uint64_t closure = Generate(generator, astNode->data.call.callee, binding);
				
				L1FIRNode firNode;
				firNode.type = L1FIRNodeTypeCall;
				firNode.data.call.arguments = arguments;
				firNode.data.call.closure = closure;
				firNode.data.call.destination = destination;
				firNode.data.call.argumentCount = argumentCount;
				L1ArrayAppend(& generator->nodeArray, & firNode, sizeof(firNode));
				
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
						uint64_t closureDestination = GenerateID(generator);
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
						uint64_t* arguments = L1RegionAllocate(generator->region, sizeof(uint64_t) * argumentCount);
						{
							Binding* bindings = calloc(1, sizeof(Binding) * argumentCount);
							uint64_t i = 0;
							const L1ParserASTNodeLinkedList* a = astNode->data.assignment.arguments;
							while (a)
							{
								assert(a->head->type == L1ParserASTNodeTypeIdentifier);
								arguments[i] = GenerateID(generator);
								Binding b;
								b.previous = i ? bindings + i  - 1: binding;
								b.source = arguments[i];
								b.bytes = a->head->data.identifier.bytes;
								b.byteCount = a->head->data.identifier.byteCount;
								bindings[i] = b;
								i++;
								a = a->tail;
							}
							
							source = Generate(generator, astNode->data.assignment.source, bindings + i - 1);
							
							free(bindings);
						}
					
						L1FIRNode firNode;
						firNode.type = L1FIRNodeTypeClosure;
						firNode.data.closure.arguments = arguments;
						firNode.data.closure.argumentCount = argumentCount;
						firNode.data.closure.result = source;
						firNode.data.closure.destination = closureDestination;
						source = closureDestination;
						L1ArrayAppend(& generator->nodeArray, & firNode, sizeof(firNode));
						
						binding = binding->previous;
					}
					else
					{
						source = Generate(generator, astNode->data.assignment.source, binding);
					}
					
					Binding newBinding;
					newBinding.bytes = astNode->data.assignment.destination->data.identifier.bytes;
					newBinding.byteCount = astNode->data.assignment.destination->data.identifier.byteCount;
					newBinding.source = source;
					newBinding.previous = binding;
					
					return Generate(generator, astNode->data.assignment.followingContext, & newBinding);
				}
				abort();
			}
		case L1ParserASTNodeTypeBranch:
			{
				uint64_t condition = Generate(generator, astNode->data.branch.condition, binding);
				uint64_t resultIfTrue = Generate(generator, astNode->data.branch.resultIfTrue, binding);
				uint64_t resultIfFalse = Generate(generator, astNode->data.branch.resultIfFalse, binding);
				uint64_t destination = GenerateID(generator);
				L1FIRNode firNode;
				firNode.type = L1FIRNodeTypeBranch;
				firNode.data.branch.destination = destination;
				firNode.data.branch.condition = condition;
				firNode.data.branch.resultIfTrue = resultIfTrue;
				firNode.data.branch.resultIfFalse = resultIfFalse;
				L1ArrayAppend(& generator->nodeArray, & firNode, sizeof(firNode));
				return destination;
			}
		case L1ParserASTNodeTypeList:
			abort();
			{
				uint64_t destination = GenerateID(generator);
				
				{
					L1FIRNode firNode;
					firNode.type = L1FIRNodeTypeNewList;
					firNode.data.newList.destination = destination;
					L1ArrayAppend(& generator->nodeArray, & firNode, sizeof(firNode));
				}
				
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
						L1FIRNode firNode;
						firNode.type = L1FIRNodeTypeConsList;
						firNode.data.consList.head = Generate(generator, elements[i], binding);
						firNode.data.consList.tail = destination;
						destination = GenerateID(generator);
						firNode.data.consList.destination = destination;
						L1ArrayAppend(& generator->nodeArray, & firNode, sizeof(firNode));
					}
				}
				
				free(elements);
				
				return destination;
			}
	}
}

L1FIRGenerator* L1FIRGeneratorNew(const L1ParserASTNode* node)
{
	assert(sizeof(size_t) <= sizeof(uint64_t));
	L1FIRGenerator* self = calloc(1, sizeof(L1FIRGenerator));
	self->nextIdentifier = 1;
	L1ArrayInitialize(& self->nodeArray);
	self->region = L1RegionNew();
	
	uint64_t finalResult = Generate(self, node, NULL);
	
	{
		L1FIRNode firNode;
		firNode.type = L1FIRNodeTypeLet;
		firNode.data.let.destination = 0;
		firNode.data.let.source = finalResult;
		L1ArrayAppend(& self->nodeArray, & firNode, sizeof(firNode));
	}
	return self;
}

const L1FIRNode* L1FIRGeneratorGetNodes(L1FIRGenerator* self, uint64_t* nodeCount)
{
	if(nodeCount) *nodeCount = L1ArrayGetElementCount(& self->nodeArray);
	return L1ArrayGetElements(& self->nodeArray);
}

void L1FIRGeneratorDelete(L1FIRGenerator* self)
{
	if(not self) return;
	L1RegionDelete(self->region);
	L1ArrayDeinitialize(& self->nodeArray);
	free(self);
}
