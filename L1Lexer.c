#include "L1Lexer.h"
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

struct L1Lexer
{
	uint8_t* bufferBytes;
	uint64_t bufferByteCount;
	L1LexerTokenType lastTokenType;
	uint64_t currentLineNumber;
	const uint8_t* inputBytes;
	L1LexerErrorType lastErrorType;
};

L1Lexer* L1LexerNew(const uint8_t* nullTerminatedUTF8Bytes)
{
	L1Lexer* self = calloc(1, sizeof(L1Lexer));
	self->inputBytes = nullTerminatedUTF8Bytes;
	return self;
}

static void AddCharacterToBuffer(L1Lexer* self, uint8_t c)
{
	self->bufferByteCount++;
	self->bufferBytes = realloc(self->bufferBytes, self->bufferByteCount);
	self->bufferBytes[self->bufferByteCount - 1] = c;
}

static void ClearBuffer(L1Lexer* self)
{
	free(self->bufferBytes);
	self->bufferBytes = NULL;
	self->bufferByteCount = 0;
}

void L1LexerLexNext(L1Lexer* self, L1LexerTokenType* tokenType)
{
	assert(tokenType);
	ClearBuffer(self);
	*tokenType = L1LexerTokenTypeDone;
	assert(self->lastErrorType == L1LexerErrorTypeNone);
	while(*self->inputBytes)
	{
		switch (*self->inputBytes)
		{
			case '\n':
				self->currentLineNumber++;
			case ' ':
			case '\r':
			case '\t':
				self->inputBytes++;
				break;
			case ':':
				self->inputBytes++;
				if (self->inputBytes[0] not_eq ':')
				{
					*tokenType = L1LexerTokenTypeSingleColon;
					goto end;
				}
				
				self->inputBytes++;
				*tokenType = L1LexerTokenTypeDoubleColon;
				
				goto end;
			case '=':
				self->inputBytes++;
				*tokenType = L1LexerTokenTypeAssign;
				goto end;
			case '-':
				self->inputBytes++;
				if (self->inputBytes[0] == '>')
				{
					*tokenType = L1LexerTokenTypeYield;
					self->inputBytes++;
				}
				else
				{
					self->lastErrorType = L1LexerErrorTypeInvalidSequence;
					*tokenType = L1LexerTokenTypeDone;
				}
				goto end;
			case '(':
				*tokenType = L1LexerTokenTypeOpeningParenthesis;
				self->inputBytes++;
				goto end;
			case ')':
				*tokenType = L1LexerTokenTypeClosingParenthesis;
				self->inputBytes++;
				goto end;
			case '.':
				self->inputBytes++;
				if (self->inputBytes[0] not_eq '.')
				{
					*tokenType = L1LexerTokenTypeSingleDot;
					goto end;
				}
				self->inputBytes++;
				if (self->inputBytes[0] not_eq '.')
				{
					*tokenType = L1LexerTokenTypeDoubleDot;
					goto end;
				}
				
				*tokenType = L1LexerTokenTypeTripleDot;
				self->inputBytes++;
				goto end;
			case '[':
				*tokenType = L1LexerTokenTypeOpeningSquareBracket;
				self->inputBytes++;
				goto end;
			case ']':
				*tokenType = L1LexerTokenTypeClosingSquareBracket;
				self->inputBytes++;
				goto end;
			case ',':
				*tokenType = L1LexerTokenTypeComma;
				self->inputBytes++;
				goto end;
			case ';':
				*tokenType = L1LexerTokenTypeTerminal;
				self->inputBytes++;
				goto end;
			case '?':
				*tokenType = L1LexerTokenTypeQuestionMark;
				self->inputBytes++;
				goto end;
			case '!':
				*tokenType = L1LexerTokenTypeExclaimationMark;
				self->inputBytes++;
				goto end;
			case '|':
				*tokenType = L1LexerTokenTypeBar;
				self->inputBytes++;
				goto end;
			case '$':
				*tokenType = L1LexerTokenTypeDollar;
				self->inputBytes++;
				goto end;
			case '\'':
				*tokenType = L1LexerTokenTypeSingleQuote;
				self->inputBytes++;
				goto end;
			case '/':
				if(self->inputBytes[1] == '/')
				{
					while(*self->inputBytes and *self->inputBytes not_eq '\n') self->inputBytes++;
					break;
				}
				else if (self->inputBytes[1] == '*')
				{
					self->inputBytes += 2;
					while(*self->inputBytes)
					{
						if(self->inputBytes[0] == '*' and self->inputBytes[1] == '/')
						{
							break;
						}
						self->inputBytes++;
					}
					self->inputBytes += 2;
					break;
				}
				else
				{
					self->lastErrorType = L1LexerErrorTypeInvalidSequence;
					*tokenType = L1LexerTokenTypeDone;
					goto end;
				}
				break;
			case '"':
				*tokenType = L1LexerTokenTypeString;
				self->inputBytes++;
				while (*self->inputBytes)
				{
					switch (*self->inputBytes)
					{
						case '"':
							self->inputBytes++;
							goto end;
						case '\\':
							self->inputBytes++;
							switch (*self->inputBytes)
							{
								case '\\':
									AddCharacterToBuffer(self, '\\');
									break;
								case 'n':
									AddCharacterToBuffer(self, '\n');
									break;
								case 'r':
									AddCharacterToBuffer(self, '\r');
									break;
								case '"':
									AddCharacterToBuffer(self, '"');
									break;
								case 't':
									AddCharacterToBuffer(self, '\t');
									break;
								default:
									self->lastErrorType = L1LexerErrorTypeInvalidSequence;
									*tokenType = L1LexerTokenTypeDone;
									goto end;
							}
							break;
						case '\n':
							self->currentLineNumber++;
						default:
							AddCharacterToBuffer(self, *self->inputBytes);
							break;
					}
					self->inputBytes++;
				}
				self->lastErrorType = L1LexerErrorTypeStringDidNotTerminate;
				*tokenType = L1LexerTokenTypeDone;
				goto end;
			default:
				while (*self->inputBytes and *self->inputBytes >= '0' and *self->inputBytes <= '9')
				{
					*tokenType = L1LexerTokenTypeNatural;
					AddCharacterToBuffer(self, *self->inputBytes);
					self->inputBytes++;
				}
				if(*tokenType == L1LexerTokenTypeNatural) goto end;
				
				*tokenType = L1LexerTokenTypeIdentifier;
				const uint8_t reservedCharacters[] = " \n\t\r=()[]\",;?/:$>.'!|-";
				const uint8_t* rcp;
				while (*self->inputBytes)
				{
					rcp = reservedCharacters;
					bool characterIsReserved = false;
					while(*rcp)
					{
						if (*rcp == *self->inputBytes)
						{
							characterIsReserved = true;
							break;
						}
						rcp++;
					}
					if(characterIsReserved) goto end;
					AddCharacterToBuffer(self, *self->inputBytes);
					self->inputBytes++;
				}
				goto end;
		}
	}
	end:
	self->lastTokenType = *tokenType;
}

uint64_t L1LexerGetCurrentLineNumber(L1Lexer* self)
{
	return self->currentLineNumber;
}

L1LexerErrorType L1LexerGetError(L1Lexer* self)
{
	return self->lastErrorType;
}

const uint8_t* L1LexerGetLastTokenBytes(L1Lexer* self, uint64_t* byteCount)
{
	switch (self->lastTokenType)
	{
		case L1LexerTokenTypeNatural:
		case L1LexerTokenTypeString:
		case L1LexerTokenTypeIdentifier:
			break;
		default:
			if (byteCount) *byteCount = 0;
			return NULL;
	}
	if(byteCount) *byteCount = self->bufferByteCount;
	return self->bufferBytes;
}

void L1LexerDelete(L1Lexer* self)
{
	free(self->bufferBytes);
}
