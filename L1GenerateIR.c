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

static uint64_t GenerateUndefined(uint64_t* nextID, const L1GenerateIROutputFunctions* outputFunctions, void* userdata)
{
	uint64_t destination = GenerateID(nextID);
	outputFunctions->loadUndefined(destination, userdata);
	return destination;
}

static uint64_t Generate(const L1ParserASTNode* astNode, const Binding* binding, uint64_t* nextID, const L1GenerateIROutputFunctions* outputFunctions, void* userdata)
{
	assert(outputFunctions);
	assert(astNode);
	switch (astNode->type)
	{
		case L1ParserASTNodeTypeNatural:
			{
				uint64_t destination = GenerateID(nextID);
				outputFunctions->loadInteger(destination, astNode->data.natural.byteCount, astNode->data.natural.bytes, userdata);
				return destination;
			}
		case L1ParserASTNodeTypeString:
			{
				uint64_t destination = GenerateID(nextID);
				outputFunctions->loadInteger(destination, astNode->data.string.byteCount, astNode->data.string.bytes, userdata);
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
				uint64_t callee = Generate(astNode->data.call.callee, binding, nextID, outputFunctions, userdata);
				uint64_t destination = GenerateID(nextID);
				
				L1Array argumentArray;
				L1ArrayInitialize(& argumentArray);
				{
					const L1ParserASTNodeLinkedList* elements = astNode->data.call.arguments;
					while (elements)
					{
						uint64_t argument = Generate(elements->head, binding, nextID, outputFunctions, userdata);
						L1ArrayAppend(& argumentArray, & argument, sizeof(uint64_t));
						elements = elements->tail;
					}
				}
				
				outputFunctions->call(destination, callee, L1ArrayGetElementCount(& argumentArray), L1ArrayGetElements(& argumentArray), userdata);
				
				L1ArrayDeinitialize(& argumentArray);
				
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
							
							source = Generate(astNode->data.assignment.source, bindings + i - 1, nextID, outputFunctions, userdata);
							
							free(bindings);
						}
						
						outputFunctions->closure(closureDestination, source, argumentCount, arguments, userdata);
						
						source = closureDestination;
						
						free(arguments);
						
						binding = binding->previous;
					}
					else
					{
						source = Generate(astNode->data.assignment.source, binding, nextID, outputFunctions, userdata);
					}
					
					Binding newBinding;
					newBinding.bytes = astNode->data.assignment.destination->data.identifier.bytes;
					newBinding.byteCount = astNode->data.assignment.destination->data.identifier.byteCount;
					newBinding.source = source;
					newBinding.previous = binding;
					
					return Generate(astNode->data.assignment.followingContext, & newBinding, nextID, outputFunctions, userdata);
				}
				abort();
			}
		case L1ParserASTNodeTypeBranch:
			{
				uint64_t destination = GenerateID(nextID);
				uint64_t condition = Generate(astNode->data.branch.condition, binding, nextID, outputFunctions, userdata);
				
				uint64_t resultIfTrue = astNode->data.branch.resultIfTrue ? Generate(astNode->data.branch.resultIfTrue, binding, nextID, outputFunctions, userdata) : GenerateUndefined(nextID, outputFunctions, userdata);
				uint64_t resultIfFalse = astNode->data.branch.resultIfFalse ? Generate(astNode->data.branch.resultIfFalse, binding, nextID, outputFunctions, userdata) : GenerateUndefined(nextID, outputFunctions, userdata);
				
				outputFunctions->branch(destination, condition, resultIfTrue, resultIfFalse, userdata);
				
				return destination;
			}
		case L1ParserASTNodeTypeList:
			{
				uint64_t destination = GenerateID(nextID);
				
				L1Array elementArray;
				L1ArrayInitialize(& elementArray);
				{
					const L1ParserASTNodeLinkedList* elements = astNode->data.list.elements;
					while (elements)
					{
						uint64_t element = Generate(elements->head, binding, nextID, outputFunctions, userdata);
						L1ArrayAppend(& elementArray, & element, sizeof(uint64_t));
						elements = elements->tail;
					}
				}
				
				outputFunctions->list(destination, L1ArrayGetElementCount(& elementArray), L1ArrayGetElements(& elementArray), userdata);
				
				L1ArrayDeinitialize(& elementArray);
				
				return destination;
			}
	}
}

uint64_t L1GenerateIR(const L1ParserASTNode* node, uint64_t* nextID, const L1GenerateIROutputFunctions* outputFunctions, void* userdata)
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
		outputFunctions->loadIntegerLessThan(binding->source, userdata);
		const static uint8_t bytes[] = "__integer_less_than";
		binding->bytes = bytes;
		binding->byteCount = strlen((const char*)bytes);
	}
	
	Binding loadBooleanFromIntegerBinding;
	loadBooleanFromIntegerBinding.previous = binding;
	binding = & loadBooleanFromIntegerBinding;
	{
		binding->source = GenerateID(nextID);
		outputFunctions->loadBooleanFromInteger(binding->source, userdata);
		const static uint8_t bytes[] = "__boolean_from_integer";
		binding->bytes = bytes;
		binding->byteCount = strlen((const char*)bytes);
	}
	
	uint64_t source = Generate(node, binding, nextID, outputFunctions, userdata);
	outputFunctions->export(source, userdata);
	return source;
}
