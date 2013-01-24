#ifndef L1Lexer_h
#define L1Lexer_h

#include <stdbool.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	L1LexerErrorNone,
	L1LexerErrorUnknown
}L1LexerError;

typedef enum
{
	L1LexerTokenTypeDone,
	L1LexerTokenTypeTerminal,
	L1LexerTokenTypeComma,
	L1LexerTokenTypeAssign,
	L1LexerTokenTypeEqual,
	L1LexerTokenTypeGreaterEqual,
	L1LexerTokenTypeLesserEqual,
	L1LexerTokenTypeGreater,
	L1LexerTokenTypeLesser,
	L1LexerTokenTypeAddition,
	L1LexerTokenTypeSubtraction,
	L1LexerTokenTypeDivision,
	L1LexerTokenTypeMultiplication,
	L1LexerTokenTypeDot,
	L1LexerTokenTypeElipsis,
	L1LexerTokenTypeIdentifier,
	L1LexerTokenTypeOpeningParenthesis,
	L1LexerTokenTypeClosingParenthesis,
	L1LexerTokenTypeOpeningDoubleBracket,
	L1LexerTokenTypeClosingDoubleBracket,
	L1LexerTokenTypeNumber,
	L1LexerTokenTypeString,
	L1LexerTokenTypeHash,
	L1LexerTokenTypeQuestionMark,
	L1LexerTokenTypeDoubleQuestionMark,
	L1LexerTokenTypeYield
}L1LexerTokenType;

const char* L1LexerTokenTypeAsString(L1LexerTokenType self);

typedef struct L1Lexer L1Lexer;
struct L1Lexer
{
	//L1Lexer State
	size_t currentLineNumber;
	long currentCharacterOfLineNumber;
	L1LexerTokenType lastTokenType;
	
	//Buffering text
	char* buffer;
	size_t bufferUsed, bufferAllocated;
	
	//Error handling
	L1LexerError lastError;
	
	//Input
	char (*pump)(L1Lexer* self);
	char (*peek)(L1Lexer* self);
	void* userdata;
	
	//L1Lexer configurations
};

void L1LexerAddStringToBuffer(L1Lexer* self, const char* string);
void L1LexerClearBuffer(L1Lexer* self);
//const char* L1LexerGetBuffer(L1Lexer* self);

void L1LexerInit(L1Lexer* self);
void L1LexerDeinit(L1Lexer* self);

//Returns L1LexerErrorNone if no error occurs
L1LexerError L1LexerLex(L1Lexer* self);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif