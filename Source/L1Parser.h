#ifndef L1Parser_h
#define L1Parser_h

#include "L1Array.h"
#include "L1Lexer.h"
#include "L1IRState.h"

typedef struct L1Parser L1Parser;
struct L1Parser
{
	L1Array
		symbolStack,
		localAddressStack,
		tokenIDStack,
		tokens,
		errorStack,
		bindingStack;
	size_t currentTokenIndex;
	L1IRState irstate;
	L1IRAddress root;
	uint16_t nextADTTag, stateDepth;
};

enum L1ParserStatusType
{
	L1ParserStatusTypeNone,
	L1ParserStatusTypeDone,
	L1ParserStatusTypeUnexpectedSymbol,
	L1ParserStatusTypeUnknown,
};

typedef enum L1ParserStatusType L1ParserStatusType;

void L1ParserInitialize(L1Parser* self);
void L1ParserDeinitialize(L1Parser* self);

L1ParserStatusType L1ParserParse(L1Parser* self, L1LexerTokenType tokenType, const char* tokenString, size_t tokenStringLength);

L1IRState* L1ParserGetIRState(L1Parser* self);

#endif
