#ifndef L1Lexer_h
#define L1Lexer_h

#include <stdlib.h>
#include "L1Array.h"
#include <iso646.h>
#include <string.h>
#include <assert.h>

enum L1LexerTokenType
{
	L1LexerTokenTypeNatural,
	L1LexerTokenTypeString,
	
	L1LexerTokenTypeIdentifier,
	
	L1LexerTokenTypeTerminal,
	
	L1LexerTokenTypeOpenParenthesis,
	L1LexerTokenTypeCloseParenthesis,
	
	L1LexerTokenTypeSingleEqual,
	L1LexerTokenTypeSingleColon,
	L1LexerTokenTypeDoubleColon,
	
	L1LexerTokenTypeSingleBarArrow,
	L1LexerTokenTypeDoubleBarArrow,
	
	//L1LexerTokenTypeUnderscore,
	//L1LexerTokenTypeUniverse,
	
	//L1LexerTokenTypeMinus,
	
	L1LexerTokenTypeDollar,
	L1LexerTokenTypePercent,
	L1LexerTokenTypeAmpersand,
	
	L1LexerTokenTypeDeclare,
	
	L1LexerTokenTypeComma,
	L1LexerTokenTypeOpenBracket,
	L1LexerTokenTypeCloseBracket,
	
	L1LexerTokenTypeDone,
};
typedef enum L1LexerTokenType L1LexerTokenType;

enum L1LexerError
{
	L1LexerErrorNone,
	L1LexerErrorInvalidEscapeSequence,
	L1LexerErrorUnterminatedString,
	L1LexerErrorUnterminatedComment,
	L1LexerErrorUnexpectedCharacter,
};
typedef enum L1LexerError L1LexerError;

typedef struct L1Lexer L1Lexer;
struct L1Lexer
{
	L1Array
		characterBuffer;
	size_t
		currentLineNumber;
	const char
		*input,
		*inputEnd;
	L1LexerError
		error;
};

void L1LexerInitialize(L1Lexer* self, const char* input);

void L1LexerDeinitialize(L1Lexer* self);

L1LexerError L1LexerGetError(L1Lexer* self);

size_t L1LexerGetCurrentLineNumber(L1Lexer* self);

const char* L1LexerGetPreviousTokenDataString(L1Lexer* self);

size_t L1LexerGetPreviousTokenDataStringLength(L1Lexer* self);

L1LexerTokenType L1LexerLex(L1Lexer* self);



#endif
