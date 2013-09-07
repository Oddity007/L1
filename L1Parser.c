#include "L1Parser.h"
#include <stdlib.h>
#include "L1Region.h"
#include <iso646.h>
#include "L1Array.h"
#include "L1Lexer.h"
#include <stdbool.h>

struct L1Parser
{
	L1ParserASTNode* rootASTNode;
	L1Region* region;
};

static L1ParserASTNode* ParserASTNodeFromToken(L1Parser* self, const L1ParserLexedToken* token)
{
	L1ParserASTNode* node = NULL;
	switch (token->type)
	{
		case L1LexerTokenTypeNatural:
			node = L1RegionAllocate(self->region, sizeof(L1ParserASTNode));
			node->type = L1ParserASTNodeTypeNatural;
			node->data.natural.bytes = token->bytes;
			node->data.natural.byteCount = token->byteCount;
			break;
		case L1LexerTokenTypeString:
			node = L1RegionAllocate(self->region, sizeof(L1ParserASTNode));
			node->type = L1ParserASTNodeTypeString;
			node->data.string.bytes = token->bytes;
			node->data.string.byteCount = token->byteCount;
			break;
		case L1LexerTokenTypeIdentifier:
			node = L1RegionAllocate(self->region, sizeof(L1ParserASTNode));
			node->type = L1ParserASTNodeTypeIdentifier;
			node->data.identifier.bytes = token->bytes;
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

typedef struct RuleNode RuleNode;
struct RuleNode
{
	uint8_t id;
	void (*action)(void** nodeDatas, uint64_t nodeDataCount);
	RuleNode** children;
	uint64_t childCount;
};

static uint64_t ParseBranch(L1Parser* self, const L1ParserLexedToken* tokens, uint64_t tokenCount, void** returnedData)
{
	*returnedData = NULL;
	uint64_t readCount = 0;
	void* subdata = NULL;
	readCount = ParseChainedExpression(self, tokens, tokenCount, & subdata);
	if(readCount)
	{
		*returnedData = subdata;
		return readCount;
	}
	readCount = ParseAssignment(self, tokens, tokenCount, & subdata);
	if(readCount)
	{
		*returnedData = subdata;
		return readCount;
	}
	readCount = ParseChainedExpression(self, tokens, tokenCount, & subdata);
	if(readCount)
	{
		*returnedData = subdata;
		return readCount;
	}
	return 0;
}

static uint64_t ParseOpenExpression(L1Parser* self, const L1ParserLexedToken* tokens, uint64_t tokenCount, void** returnedData)
{
	*returnedData = NULL;
	uint64_t readCount = 0;
	void* subdata = NULL;
	readCount = ParseBranch(self, tokens, tokenCount, & subdata);
	if(readCount)
	{
		*returnedData = subdata;
		return readCount;
	}
	readCount = ParseAssignment(self, tokens, tokenCount, & subdata);
	if(readCount)
	{
		*returnedData = subdata;
		return readCount;
	}
	readCount = ParseChainedExpression(self, tokens, tokenCount, & subdata);
	if(readCount)
	{
		*returnedData = subdata;
		return readCount;
	}
	return 0;
}


static uint64_t ParseProgram(L1Parser* self, const L1ParserLexedToken* tokens, uint64_t tokenCount, void** data)
{
	
	return tokenCount;
}

static L1ParserASTNode* Parse(L1Parser* self, const L1ParserLexedToken* tokens, uint64_t tokenCount, uint64_t* readCount)
{
	for (uint64_t i = 0; i < tokenCount; i++)
	{
		
	}
}

L1Parser* L1ParserNew(const L1ParserLexedToken* tokens, uint64_t tokenCount)
{
	L1Parser* self = calloc(1, sizeof(L1Parser));
	self->region = L1RegionNew();
	uint64_t readCount = 0;
	self->rootASTNode = Parse(self, tokens, tokenCount - 1, & readCount);
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
