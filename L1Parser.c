#include "L1Parser.h"
#include <stdlib.h>
#include "L1Region.h"
#include <iso646.h>
#include "L1Array.h"
#include "L1Lexer.h"
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>

struct L1Parser
{
	const L1ParserASTNode* rootASTNode;
	L1Region* region;
	L1ParserErrorType lastError;
	//jmp_buf env;
};

/*static void ThrowError(L1Parser* self, L1ParserErrorType type)
{
	self->lastError = type;
	longjmp(self->env, 1);
	assert(false);
}*/

#include <stdio.h>

static void* CloneBytes(L1Parser* parser, const void* bytes, size_t byteCount)
{
	return memcpy(L1RegionAllocate(parser->region, byteCount), bytes, byteCount);
}

static L1ParserASTNode* ParserASTNodeFromToken(L1Parser* self, const L1ParserLexedToken* token)
{
	L1ParserASTNode* node = NULL;
	switch (token->type)
	{
		case L1LexerTokenTypeNatural:
			node = L1RegionAllocate(self->region, sizeof(L1ParserASTNode));
			node->type = L1ParserASTNodeTypeNatural;
			node->data.natural.bytes = CloneBytes(self, token->bytes, token->byteCount);
			node->data.natural.byteCount = token->byteCount;
			break;
		case L1LexerTokenTypeString:
			node = L1RegionAllocate(self->region, sizeof(L1ParserASTNode));
			node->type = L1ParserASTNodeTypeString;
			node->data.string.bytes = CloneBytes(self, token->bytes, token->byteCount);
			node->data.string.byteCount = token->byteCount;
			break;
		case L1LexerTokenTypeIdentifier:
			node = L1RegionAllocate(self->region, sizeof(L1ParserASTNode));
			node->type = L1ParserASTNodeTypeIdentifier;
			node->data.identifier.bytes = CloneBytes(self, token->bytes, token->byteCount);
			node->data.identifier.byteCount = token->byteCount;
			break;
	/*	case L1LexerTokenTypeDone:
			break;*/
		default:
			/*printf("%s: %i\n", __func__, (int) token->type);
			assert(false);
			ThrowError(self, L1ParserErrorTypeUnexpectedToken);
			assert(false);*/
			break;
	}
	return node;
}

typedef struct Rule Rule;
struct Rule
{
	const uint8_t* symbols;
	uint8_t
		symbol,
		symbolCount,
		action;
};

static const void* HandleAction(L1Parser* self, const void* matchedSymbolData[], Rule rule);

static uint64_t Parse(L1Parser* self, const L1ParserLexedToken* tokens, uint64_t tokenCount, const void** data, uint8_t currentNonterminalSymbol, const Rule* rules, uint64_t ruleCount)
{
	*data = NULL;
	for (uint64_t i = 0; i < ruleCount; i++)
	{
		const Rule rule = rules[i];
		if (rule.symbol == currentNonterminalSymbol)
		{
			const void* matchedSymbolData[rule.symbolCount];
			for (int j = 0; j < rule.symbolCount; j++) matchedSymbolData[j] = NULL;
			bool matched = true;
			uint64_t currentTokenIndex = 0;
			for (uint8_t j = 0; j < rule.symbolCount; j++)
			{
				matchedSymbolData[j] = NULL;
				uint8_t symbol = rule.symbols[j];
				if(currentTokenIndex >= tokenCount)
				{
					matched = false;
					break;
				}
				if(tokens[currentTokenIndex].type == symbol)
				{
					matchedSymbolData[j] = ParserASTNodeFromToken(self, tokens + currentTokenIndex);
					//assert(matchedSymbolData[j]);
					currentTokenIndex++;
				}
				else
				{
					bool symbolIsRule = false;
					for (uint64_t k = 0; k < ruleCount; k++)
					{
						if(symbol == rules[k].symbol)
						{
							symbolIsRule = true;
							break;
						}
					}
					if(symbolIsRule)
					{
						uint64_t tokensRead = Parse(self, tokens + currentTokenIndex, tokenCount - currentTokenIndex, matchedSymbolData + j, symbol, rules, ruleCount);
						if(not tokensRead)
						{
							matched = false;
							break;
						}
						currentTokenIndex += tokensRead;
					}
					else
					{
						matched = false;
						break;
					}
				}
			}
			if(matched)
			{
				*data = HandleAction(self, matchedSymbolData, rule);
				return currentTokenIndex;
			}
		}
	}
	//ThrowError(self, L1ParserErrorTypeUnknown);
	return 0;
}

static const L1ParserASTNodeLinkedList* Cons(L1Parser* parser, const L1ParserASTNode* head, const L1ParserASTNodeLinkedList* tail)
{
	L1ParserASTNodeLinkedList* list = L1RegionAllocate(parser->region, sizeof(L1ParserASTNodeLinkedList));
	list->head = head;
	list->tail = tail;
	return list;
}

static L1ParserASTNode* CreateListNode(L1Parser* parser, const L1ParserASTNodeLinkedList* elements, const L1ParserASTNode* sublist)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeList;
	node->data.list.elements = elements;
	node->data.list.sublist = sublist;
	return node;
}

static L1ParserASTNode* CreateAssignmentNode(L1Parser* parser, const L1ParserASTNode* destination, const L1ParserASTNodeLinkedList* arguments, const L1ParserASTNode* source, const L1ParserASTNode* followingContext, bool isMeta)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeAssignment;
	node->data.assignment.destination = destination;
	node->data.assignment.arguments = arguments;
	node->data.assignment.source = source;
	node->data.assignment.followingContext = followingContext;
	node->data.assignment.isMeta = isMeta;
	return node;
}

static L1ParserASTNode* CreateAnonymousFunctionNode(L1Parser* parser, const L1ParserASTNodeLinkedList* arguments, const L1ParserASTNode* source)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeAnonymousFunction;
	node->data.anonymousFunction.arguments = arguments;
	node->data.anonymousFunction.source = source;
	return node;
}

static L1ParserASTNode* CreateOptionNode(L1Parser* parser, const L1ParserASTNode* construction, const L1ParserASTNode* defaultConstruction)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeOption;
	node->data.option.construction = construction;
	node->data.option.defaultConstruction = defaultConstruction;
	return node;
}

static L1ParserASTNode* CreateCallNode(L1Parser* parser, const L1ParserASTNode* callee, const L1ParserASTNodeLinkedList* arguments)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeCall;
	node->data.call.callee = callee;
	node->data.call.arguments = arguments;
	return node;
}

static L1ParserASTNode* CreateConstraintNode(L1Parser* parser, const L1ParserASTNode* expression, const L1ParserASTNode* constraint, const L1ParserASTNode* followingContext)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeConstraint;
	node->data.constraint.expression = expression;
	node->data.constraint.constraint = constraint;
	node->data.constraint.followingContext = followingContext;
	return node;
}

static L1ParserASTNode* CreateAnyNode(L1Parser* parser, const L1ParserASTNode* source)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeAny;
	node->data.any.source = source;
	return node;
}

static L1ParserASTNode* CreateInlineConstraintNode(L1Parser* parser, const L1ParserASTNode* expression, const L1ParserASTNode* constraint)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeInlineConstraint;
	node->data.inlineConstraint.expression = expression;
	node->data.inlineConstraint.constraint = constraint;
	return node;
}

static L1ParserASTNode* CreateMetasymbolNode(L1Parser* parser, const L1ParserASTNode* source)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeMetasymbol;
	node->data.metasymbol.source = source;
	return node;
}

#include "L1ParserGeneratedPortion"

L1Parser* L1ParserNew(const L1ParserLexedToken* tokens, uint64_t tokenCount)
{
	L1Parser* self = calloc(1, sizeof(L1Parser));
	self->region = L1RegionNew();
	const void* rootASTNode = NULL;
	self->lastError = L1ParserErrorTypeNone;
	uint64_t tokensRead = 0;
	//if(not setjmp(self->env))
	{
		assert(self->lastError == L1ParserErrorTypeNone);
		tokensRead = Parse(self, tokens, tokenCount, & rootASTNode, ProgramSymbol, Rules, RuleCount);
		assert(self->lastError == L1ParserErrorTypeNone);
		assert(tokensRead > 0);
		assert(rootASTNode);
	}
	/*else
	{
		assert(self->lastError != L1ParserErrorTypeNone);
		assert(rootASTNode == NULL);
		//Error
	}*/
	
	self->rootASTNode = rootASTNode;
	return self;
}

L1ParserErrorType L1ParserGetError(L1Parser* self)
{
	return self->lastError;
}

const L1ParserASTNode* L1ParserGetRootASTNode(L1Parser* self)
{
	return self->rootASTNode;
}

void L1ParserDelete(L1Parser* self)
{
	if (not self) return;
	L1RegionDelete(self->region);
	free(self);
}