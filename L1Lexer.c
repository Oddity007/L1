#include "L1Lexer.h"
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

const char* L1LexerTokenTypeAsString(L1LexerTokenType self)
{
	switch (self)
	{
//		case L1LexerTokenTypeYield: return "yield";
		case L1LexerTokenTypeDone: return "done";
//		case L1LexerTokenTypeAddition: return "addition";
		case L1LexerTokenTypeAssign: return "assign";
		case L1LexerTokenTypeClosingSquareBracket: return "closing square bracket";
		case L1LexerTokenTypeClosingParenthesis: return "closing parenthesis";
		case L1LexerTokenTypeComma: return "comma";
		case L1LexerTokenTypeElipsis: return "elipsis";
		case L1LexerTokenTypeOpeningSquareBracket: return "opening square bracket";
//		case L1LexerTokenTypeDivision: return "division";
//		case L1LexerTokenTypeDot: return "dot";
//		case L1LexerTokenTypeEqual: return "equal";
//		case L1LexerTokenTypeGreater: return "greater";
//		case L1LexerTokenTypeGreaterEqual: return "greater equal";
//		case L1LexerTokenTypeHash: return "hash";
		case L1LexerTokenTypeIdentifier: return "identifier";
//		case L1LexerTokenTypeLesser: return "lesser";
//		case L1LexerTokenTypeLesserEqual: return "lesser equal";
//		case L1LexerTokenTypeMultiplication: return "multiplication";
		case L1LexerTokenTypeNumber: return "number";
		case L1LexerTokenTypeOpeningParenthesis: return "opening parenthesis";
		case L1LexerTokenTypeQuestionMark: return "question mark";
		case L1LexerTokenTypeString: return "string";
//		case L1LexerTokenTypeSubtraction: return "subtraction";
		case L1LexerTokenTypeTerminal: return "terminal";
//		case L1LexerTokenTypeDoubleQuestionMark: return "double question mark";
		case L1LexerTokenTypeTypeQualifier: return "type qualifier";
	}
	return NULL;
}


//////////////////////

static char L1LexerPump(L1Lexer* self)
{
	char current = 0;
	if(self->pump) current = self->pump(self);
	if (current == '\n')
	{
		self->currentLineNumber++;
		self->currentCharacterOfLineNumber=1;
	}
	else self->currentCharacterOfLineNumber++;
	return current;
}

static uint_least32_t L1LexerPeek(L1Lexer* self)
{
	if(self->peek) return self->peek(self);
	return 0;
}

static void L1LexerAddStringToBuffer(L1Lexer* self, const uint8_t* string)
{
	if(string==NULL) return;
	if(string[0]==0) return;
	uint_least64_t length = 0;
	while(string[length])
	{
		length++;
	}

	if(self->bufferUsed + length + 1 > self->bufferAllocated)
	{
		self->bufferAllocated = self->bufferUsed + length + 15;
		self->buffer = realloc(self->buffer, self->bufferAllocated);
	}
	if(not self->buffer) abort();
	memcpy(self->buffer + self->bufferUsed, string, length);
	
	self->bufferUsed += length;
	self->buffer[self->bufferUsed] = '\0';
}

static void L1LexerAddCharacterToBuffer(L1Lexer* self, const uint_least32_t c)
{
	if(c==0) return;
	assert(c < 256);
	const uint8_t string[2] = {c, 0};
	L1LexerAddStringToBuffer(self, string);
}

static void L1LexerClearBuffer(L1Lexer* self)
{
	free(self->buffer);
	self->buffer = NULL;
	self->bufferUsed = 0;
	self->bufferAllocated = 0;
}

//const char* L1LexerGetBuffer(L1Lexer* self){return self->buffer;}

static inline void L1LexerSetBuffer(L1Lexer* self, const uint8_t* string)
{
	L1LexerClearBuffer(self);
	L1LexerAddStringToBuffer(self, string);
}

static inline bool IsAlpha(uint_least32_t c)
{
	if('A' <= c and c <= 'Z') return true;
	if('a' <= c and c <= 'z') return true;
	return false;
}

static inline bool IsDigit(uint_least32_t c)
{
	if('0' <= c and c <= '9') return true;
	return false;
}

/*static bool L1LexerLexNumber(L1Lexer* self, uint_least32_t c)
{
	bool alreadyHadDecimalPoint = false;
	if(c == '.')
	{
		L1LexerAddCharacterToBuffer(self, '.');
		c = L1LexerPump(self);
		alreadyHadDecimalPoint = true;
	}
	while (c)
	{
		if(not (IsDigit(c) or c=='.')) break;
		if(alreadyHadDecimalPoint and c=='.') return false;
		else if(c=='.') alreadyHadDecimalPoint = true;
		L1LexerAddCharacterToBuffer(self, c);
		uint_least32_t next = L1LexerPeek(self);
		if(not (IsDigit(next) or next=='.')) return true;
		c = L1LexerPump(self);
	}
	return false;
}*/

static bool L1LexerLexNumber(L1Lexer* self, uint_least32_t c)
{
	while (c)
	{
		if(not IsDigit(c)) break;
		L1LexerAddCharacterToBuffer(self, c);
		uint_least32_t next = L1LexerPeek(self);
		if(not IsDigit(next)) return true;
		c = L1LexerPump(self);
	}
	return false;
}

/*static bool L1LexerLexIdentifier(L1Lexer* self, uint_least32_t c)
{
	bool firstCharacter = true;
	while (c)
	{
		if(IsAlpha(c) or c == '_' or (firstCharacter and c == '#') or ((not firstCharacter) and IsDigit(c)))
		{
			L1LexerAddCharacterToBuffer(self, c);
			uint_least32_t next = L1LexerPeek(self);
			if(not (IsAlpha(next) or next == '_' or IsDigit(next))) return true;
		}
		else break;
		firstCharacter=false;
		c = L1LexerPump(self);
	}
	return false;
}*/

static bool L1LexerLexIdentifier(L1Lexer* self, uint_least32_t c)
{
	bool firstCharacter = true;
	while (c)
	{
		if(IsAlpha(c) or c == '_' or ((not firstCharacter) and IsDigit(c)))
		{
			L1LexerAddCharacterToBuffer(self, c);
			uint_least32_t next = L1LexerPeek(self);
			if(not (IsAlpha(next) or next == '_' or IsDigit(next))) return true;
		}
		else break;
		firstCharacter=false;
		c = L1LexerPump(self);
	}
	return false;
}

static bool L1LexerLexString(L1Lexer* self, uint_least32_t c)
{
	if(c not_eq '"') return false;
	while ((c = L1LexerPump(self)))
	{
		switch (c)
		{
			case '"':
				return true;
			case '\\':
				switch (L1LexerPeek(self))
				{
					case 'n':
						L1LexerAddCharacterToBuffer(self, '\n');
						L1LexerPump(self);
						break;
					case 't':
						L1LexerAddCharacterToBuffer(self, '\t');
						L1LexerPump(self);
						break;
					case '\\':
						L1LexerAddCharacterToBuffer(self, '\\');
						L1LexerPump(self);
						break;
					case '"':
						L1LexerAddCharacterToBuffer(self, '"');
						L1LexerPump(self);
						break;
				}
				break;
			default:
				L1LexerAddCharacterToBuffer(self, c);
				break;
		}
	}
	return false;
}

static void L1LexerEatSingleLineComment(L1Lexer* self, uint_least32_t c)
{
	while (c)
	{
		if(L1LexerPeek(self) == '\n') return;
		c = L1LexerPump(self);
	}
}

static void L1LexerEatMultiLineComment(L1Lexer* self, uint_least32_t c)
{
	while (c)
	{
		uint_least32_t lastCharacter = c;
		c = L1LexerPump(self);
		if(lastCharacter == '*' and c == '/') return;
		if (lastCharacter == '\0') return;
	}
}

L1LexerError L1LexerLex(L1Lexer* self)
{
	uint_least32_t c = 0;
	L1LexerTokenType tokenType = 0;
	L1LexerError error = L1LexerErrorNone;
	L1LexerClearBuffer(self);
	while((c = L1LexerPump(self)))
	{
		switch (c)
		{
			case '\0':
				tokenType = L1LexerTokenTypeDone;
				goto end;
			case ';':
				tokenType = L1LexerTokenTypeTerminal;
				goto end;
			case '=':
				/*if(L1LexerPeek(self) == '=')
				{
					L1LexerPump(self);
					tokenType = L1LexerTokenTypeEqual;
				}
				else*/ tokenType = L1LexerTokenTypeAssign;
				goto end;
			/*case '<':
				if(L1LexerPeek(self)=='=')
				{
					L1LexerPump(self);
					tokenType = L1LexerTokenTypeLesserEqual;
				}
				else tokenType = L1LexerTokenTypeLesser;
				goto end;*/
			/*case '>':
				if(L1LexerPeek(self) == '=')
				{
					L1LexerPump(self);
					tokenType = L1LexerTokenTypeGreaterEqual;
				}
				else tokenType = L1LexerTokenTypeGreater;
				goto end;
				break;*/
			case '.':
				if(L1LexerPeek(self) == '.')
				{
					L1LexerPump(self);
					if(L1LexerPeek(self) == '.')
					{
						L1LexerPump(self);
						tokenType = L1LexerTokenTypeElipsis;
					}
					else
					{
						error = L1LexerErrorUnknown;
					}
				}
				else if(IsDigit(L1LexerPeek(self)))
				{
					if(L1LexerLexNumber(self, c)) tokenType = L1LexerTokenTypeNumber;
				}
				else
				{
					error = L1LexerErrorUnknown;
					//tokenType = L1LexerTokenTypeDot;
				}
				goto end;
			/*case '#':
				tokenType = L1LexerTokenTypeHash;
				goto end;*/
			case ',':
				tokenType = L1LexerTokenTypeComma;
				goto end;
			/*case '+':
				tokenType = L1LexerTokenTypeAddition;
				goto end;
			case '-':
				tokenType = L1LexerTokenTypeSubtraction;
				goto end;
			case '*':
				tokenType = L1LexerTokenTypeMultiplication;
				goto end;*/
			case '(':
				tokenType = L1LexerTokenTypeOpeningParenthesis;
				goto end;
			case ')':
				tokenType = L1LexerTokenTypeClosingParenthesis;
				goto end;
			case '?':
				/*if(L1LexerPeek(self) == '?')
				{
					L1LexerPump(self);
					tokenType = L1LexerTokenTypeDoubleQuestionMark;
				}
				else*/ tokenType = L1LexerTokenTypeQuestionMark;
				goto end;
			case '[':
				tokenType = L1LexerTokenTypeOpeningSquareBracket;
				goto end;
			case ']':
				tokenType = L1LexerTokenTypeClosingSquareBracket;
				goto end;
			/*case '|':
				tokenType = L1LexerTokenTypeYield;
				goto end;*/
			case '\"':
				if(L1LexerLexString(self, c)) tokenType=L1LexerTokenTypeString;
				else error=L1LexerErrorUnknown;
				goto end;
			case ':':
				if(L1LexerPeek(self) == ':')
				{
					L1LexerPump(self);
					tokenType = L1LexerTokenTypeTypeQualifier;
				}
				else
				{
					error = L1LexerErrorUnknown;
				}
				goto end;
			case '\n':
			case '\r':
			case ' ':
			case '\t':
				break;
			case '/':
				if(L1LexerPeek(self) == '/')
				{
					L1LexerEatSingleLineComment(self, c);
					break;
				}
				else if(L1LexerPeek(self) == '*')
				{
					L1LexerPump(self);
					L1LexerEatMultiLineComment(self, L1LexerPump(self));
					break;
				}
				//tokenType = L1LexerTokenTypeDivision;
				//goto end;
			default:
				if(L1LexerLexNumber(self, c)) tokenType = L1LexerTokenTypeNumber;
				else if(L1LexerLexIdentifier(self, c)) tokenType = L1LexerTokenTypeIdentifier;
				else error=L1LexerErrorUnknown;
				if(tokenType or error) goto end;
				break;
		}
	};
	
	//This is where all the gotos go to.
	end:
	
	self->lastTokenType = tokenType;
	self->lastError = error;
	return error;
}

void L1LexerInit(L1Lexer* self)
{
	memset(self, 0, sizeof(L1Lexer));
	self->currentLineNumber=1;
}

void L1LexerDeinit(L1Lexer* self)
{
	L1LexerClearBuffer(self);
}