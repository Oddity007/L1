#ifndef L1Lexer_h
#define L1Lexer_h

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum L1LexerTokenType
{
	L1LexerTokenTypeNatural,
	L1LexerTokenTypeIdentifier,
	L1LexerTokenTypeString,
	L1LexerTokenTypeAssign,
	L1LexerTokenTypeOpeningParenthesis,
	L1LexerTokenTypeClosingParenthesis,
	L1LexerTokenTypeOpeningSquareBracket,
	L1LexerTokenTypeClosingSquareBracket,
	L1LexerTokenTypeComma,
	L1LexerTokenTypeTerminal,
	L1LexerTokenTypeQuestionMark,
	L1LexerTokenTypeDone,
	L1LexerTokenTypeLast = L1LexerTokenTypeDone
};
typedef enum L1LexerTokenType L1LexerTokenType;

typedef struct L1Lexer L1Lexer;

L1Lexer* L1LexerNew(const uint8_t* nullTerminatedUTF8Bytes);
void L1LexerLexNext(L1Lexer* self, L1LexerTokenType* tokenType);
uint64_t L1LexerGetCurrentLineNumber(L1Lexer* self);
const uint8_t* L1LexerGetLastTokenBytes(L1Lexer* self, uint64_t* byteCount);
void L1LexerDelete(L1Lexer* self);

#ifdef __cplusplus
}//extern "C"
#endif

#endif