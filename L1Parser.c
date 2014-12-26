#include "L1Parser.h"

static void PushNode(L1Parser* self, const L1ParserASTNode* node)
{
	if (node) L1ArrayPush(& self->syntaxTreeNodes, node, sizeof(L1ParserASTNode));
	size_t location = node ? L1ArrayGetElementCount(& self->syntaxTreeNodes) : 0;
	L1ArrayPush(& self->locationStack, & location, sizeof(size_t));
}

static size_t PopNodeLocation(L1Parser* self)
{
	size_t location = 0;
	L1ArrayPop(& self->locationStack, & location, sizeof(size_t));
	return location;
}

#include "L1ParserGeneratedPortion"

void L1ParserInitialize(L1Parser* self)
{
	L1ArrayInitialize(& self->symbolStack);
	L1ArrayInitialize(& self->syntaxTreeNodes);
	unsigned short top = L1LexerTokenTypeDone;
	L1ArrayPush(& self->symbolStack, & top, sizeof(unsigned short));
	top = ProgramNonterminalID;
	L1ArrayPush(& self->symbolStack, & top, sizeof(unsigned short));
	L1ArrayInitialize(& self->locationStack);
	self->currentTokenIndex = 0;
	self->rootASTNode = 0;
}

void L1ParserDeinitialize(L1Parser* self)
{
	L1ArrayDeinitialize(& self->locationStack);
	L1ArrayDeinitialize(& self->symbolStack);
	L1ArrayDeinitialize(& self->syntaxTreeNodes);
}

L1ParserStatusType L1ParserParse(L1Parser* self, L1LexerTokenType tokenType, const char* tokenString, size_t tokenStringLength)
{
	self->currentTokenIndex ++;
	assert(L1ArrayGetElementCount(& self->symbolStack) > 0);
	unsigned short top;
	L1ArrayPeek(& self->symbolStack, & top, sizeof(unsigned short));	

	while (top >= NonterminalOffset)
	{
		L1ArrayPop(& self->symbolStack, & top, sizeof(unsigned short));
		if (top < ActionOffset)
		{
			const unsigned char ruleIndex = ParseTable[top - NonterminalOffset][(size_t) tokenType];
			if (not ruleIndex)
			{
				return L1ParserStatusTypeUnexpectedSymbol;
			}
			const unsigned char* symbols = RuleTable[ruleIndex - 1];
			size_t symbolCount = 0;
			for (; symbols[symbolCount] not_eq RuleTerminationID; symbolCount++);
			for (size_t i = symbolCount; i-- > 0;)
			{
				unsigned short symbol = symbols[i];
				L1ArrayPush(& self->symbolStack, & symbol, sizeof(unsigned short));
			}
		}
		else
		{
			HandleAction(self, top, tokenString, tokenStringLength);
		}
		
		L1ArrayPeek(& self->symbolStack, & top, sizeof(unsigned short));
	}
	
	{
		if (top == (unsigned short) tokenType)
		{
			L1ArrayPop(& self->symbolStack, NULL, sizeof(unsigned short));
			if (tokenType == L1LexerTokenTypeDone)
				return L1ParserStatusTypeDone;
			else
				return L1ParserStatusTypeNone;
		}
		else
		{
			return L1ParserStatusTypeUnexpectedSymbol;
		}
	}
	
	return L1ParserStatusTypeNone;
}

const L1ParserASTNode* L1ParserGetASTNodes(L1Parser* self)
{
	return L1ArrayGetElements(& self->syntaxTreeNodes);
}

size_t L1ParserGetASTNodeCount(L1Parser* self)
{
	return L1ArrayGetElementCount(& self->syntaxTreeNodes);
}

size_t L1ParserGetRootASTNodeIndex(L1Parser* self)
{
	return self->rootASTNode;
}
