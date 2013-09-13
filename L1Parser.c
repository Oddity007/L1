#include "L1Parser.h"
#include <stdlib.h>
#include "L1Region.h"
#include <iso646.h>
#include "L1Array.h"
#include "L1Lexer.h"
#include <stdbool.h>
#include <string.h>

struct L1Parser
{
	const L1ParserASTNode* rootASTNode;
	L1Region* region;
};

//#include <stdio.h>

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
		default:
			node = NULL;
			break;
	}
	return node;
}

/*
program = openexpression done

openexpression = branch
openexpression = assignment
openexpression = chainedexpression

branch = chainedexpression questionmark chainedexpression terminal openexpression

assignment = closedexpression assignment_arguments assign chainedexpression terminal openexpression
assignment_arguments = assignment_target assignment_arguments
assignment_arguments = .
assignment_target = identifier
assignment_target = openingsquarebracket assignment_target_list_body closingsquarebracket
assignment_target_list_body = assignment_target comma assignment_target_list_body
assignment_target_list_body = assignment_target comma
assignment_target_list_body = assignment_target
assignment_arguments = closedexpression

chainedexpression = closedexpression chainedexpression
chainedexpression = closedexpression

closedexpression = identifier
closedexpression = natural
closedexpression = string
closedexpression = openingparenthesis openexpression closingparenthesis
*/

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
		//printf("rule %lu has symbol %lu, as viewed by nonterminal %lu\n", (unsigned long) i, (unsigned long) rule.symbol, (unsigned long) currentNonterminalSymbol);
		if (rule.symbol == currentNonterminalSymbol)
		{
			const void* matchedSymbolData[rule.symbolCount];
			bool matched = true;
			uint64_t currentTokenIndex = 0;
			//printf("(rule %lu, nonterminal: %lu)\n", (unsigned long) i, (unsigned long)currentNonterminalSymbol);
			for (uint8_t j = 0; j < rule.symbolCount; j++)
			{
				matchedSymbolData[j] = NULL;
				uint8_t symbol = rule.symbols[j];
				if(currentTokenIndex >= tokenCount)
				{
					matched = false;
					break;
				}
				//puts("heh");
				if(tokens[currentTokenIndex].type == symbol)
				{
					//puts("lo");
					matchedSymbolData[j] = ParserASTNodeFromToken(self, tokens + currentTokenIndex);
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
						//printf("(calling parse with symbol %lu, nonterminal: %lu)\n", (unsigned long) symbol, (unsigned long)currentNonterminalSymbol);
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
						//printf("(failing match because it is not a symbol, with symbol %lu, nonterminal: %lu)\n", (unsigned long) symbol, (unsigned long)currentNonterminalSymbol);
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
	//printf("(first token type: %lu, nonterminal: %lu)\n", (long unsigned) tokens->type, (long unsigned) currentNonterminalSymbol);
	return 0;
}

static const L1ParserASTNodeLinkedList* Cons(L1Parser* parser, const L1ParserASTNode* head, const L1ParserASTNodeLinkedList* tail)
{
	L1ParserASTNodeLinkedList* list = L1RegionAllocate(parser->region, sizeof(L1ParserASTNodeLinkedList));
	list->head = head;
	list->tail = tail;
	return list;
}

static L1ParserASTNode* CreateListNode(L1Parser* parser, const L1ParserASTNodeLinkedList* elements)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeList;
	node->data.list.elements = elements;
	return node;
}

static L1ParserASTNode* CreateBranchNode(L1Parser* parser, const L1ParserASTNode* condition, const L1ParserASTNode* resultIfTrue, const L1ParserASTNode* resultIfFalse)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeBranch;
	node->data.branch.condition = condition;
	node->data.branch.resultIfTrue = resultIfTrue;
	node->data.branch.resultIfFalse = resultIfFalse;
	return node;
}

static L1ParserASTNode* CreateAssignmentNode(L1Parser* parser, const L1ParserASTNode* destination, const L1ParserASTNodeLinkedList* arguments, const L1ParserASTNode* source, const L1ParserASTNode* followingContext)
{
	L1ParserASTNode* node = L1RegionAllocate(parser->region, sizeof(L1ParserASTNode));
	node->type = L1ParserASTNodeTypeAssignment;
	node->data.assignment.destination = destination;
	node->data.assignment.arguments = arguments;
	node->data.assignment.source = source;
	node->data.assignment.followingContext = followingContext;
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

#include "L1ParserGeneratedPortion"

L1Parser* L1ParserNew(const L1ParserLexedToken* tokens, uint64_t tokenCount)
{
	L1Parser* self = calloc(1, sizeof(L1Parser));
	self->region = L1RegionNew();
	const void* rootASTNode = NULL;
	if(not Parse(self, tokens, tokenCount, & rootASTNode, ProgramSymbol, Rules, RuleCount))
	{
		abort();
	}
	self->rootASTNode = rootASTNode;
	return self;
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