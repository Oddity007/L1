#ifndef L1Lexer_h
#define L1Lexer_h

#include <stdlib.h>
#include <stdint.h>

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
//	L1LexerTokenTypeEqual,
//	L1LexerTokenTypeGreaterEqual,
//	L1LexerTokenTypeLesserEqual,
//	L1LexerTokenTypeGreater,
//	L1LexerTokenTypeLesser,
//	L1LexerTokenTypeAddition,
//	L1LexerTokenTypeSubtraction,
//	L1LexerTokenTypeDivision,
//	L1LexerTokenTypeMultiplication,
//	L1LexerTokenTypeDot,
	L1LexerTokenTypeElipsis,
	L1LexerTokenTypeIdentifier,
	L1LexerTokenTypeOpeningParenthesis,
	L1LexerTokenTypeClosingParenthesis,
	L1LexerTokenTypeOpeningSquareBracket,
	L1LexerTokenTypeClosingSquareBracket,
	L1LexerTokenTypeNumber,
	L1LexerTokenTypeString,
//	L1LexerTokenTypeHash,
	L1LexerTokenTypeQuestionMark,
	L1LexerTokenTypeTypeQualifier,
//	L1LexerTokenTypeDoubleQuestionMark,
//	L1LexerTokenTypeYield
	L1LexerTokenTypeLast = L1LexerTokenTypeTypeQualifier
}L1LexerTokenType;

const char* L1LexerTokenTypeAsString(L1LexerTokenType self);

typedef struct L1Lexer L1Lexer;
struct L1Lexer
{
	//L1Lexer State
	uint_least64_t currentLineNumber;
	uint_least64_t currentCharacterOfLineNumber;
	L1LexerTokenType lastTokenType;
	
	//Buffering text
	uint8_t* buffer;
	uint_least64_t bufferUsed, bufferAllocated;
	
	//Error handling
	L1LexerError lastError;
	
	//Input
	uint_least32_t (*pump)(L1Lexer* self);
	uint_least32_t (*peek)(L1Lexer* self);
	void* userdata;
	
	//L1Lexer configurations
};

//void L1LexerAddStringToBuffer(L1Lexer* self, const char* string);
//void L1LexerClearBuffer(L1Lexer* self);
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