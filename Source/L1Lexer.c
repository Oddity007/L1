#include "L1Lexer.h"
#include <stdlib.h>
#include <iso646.h>
#include <limits.h>
#include "L1Array.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
//#include <assert.h>

void L1LexerInitialize(L1Lexer* self, const char* input)
{
	L1ArrayInitialize(& self->characterBuffer);
	self->currentLineNumber = 0;
	self->input = input;
	self->inputEnd = input + strlen(input);
	assert(* self->inputEnd == 0);
	self->error = L1LexerErrorNone;
}

void L1LexerDeinitialize(L1Lexer* self)
{
	L1ArrayDeinitialize(& self->characterBuffer);
}

L1LexerError L1LexerGetError(L1Lexer* self)
{
	return self->error;
}

const char* L1LexerGetPreviousTokenDataString(L1Lexer* self)
{
	return (const char*) L1ArrayGetElements(& self->characterBuffer);
}

size_t L1LexerGetPreviousTokenDataStringLength(L1Lexer* self)
{
	return L1ArrayGetElementCount(& self->characterBuffer);
}

/*
bool isRestrictedCharacter(char c)
{
	switch (c)
	{
		case
		default: return false;
	}
}*/

size_t L1LexerGetCurrentLineNumber(L1Lexer* self)
{
	return self->currentLineNumber + 1;
}

static void Mul10Add(L1Array* byteBuffer, unsigned char digit)
{
	assert(digit < 10);
	size_t byteCount = L1ArrayGetElementCount(byteBuffer);
	if (byteCount == 0)
	{
		L1ArrayPush(byteBuffer, & digit, 1);
		return;
	}
	unsigned char* bytes = L1ArrayGetElements(byteBuffer);
	unsigned int accumulator = digit;
	for (size_t i = 0; i < byteCount; i++)
	{
		accumulator += bytes[i];
		bytes[i] = accumulator bitand 0xFF;
		accumulator >>= 8;
	}
	if (accumulator > 0)
	{
		assert(accumulator == 1);
		unsigned char carriedBit = 1;
		L1ArrayPush(byteBuffer, & carriedBit, 1);
	}
}

L1LexerTokenType L1LexerLex(L1Lexer* self)
{
//	assert(self->input <= self->inputEnd);
	self->error = L1LexerErrorNone;
	L1ArraySetElementCount(& self->characterBuffer, 0, 1);
	while (* self->input)
	{
		char c = * self->input;
		self->input++;
		switch (c)
		{
			case '\0':
				return L1LexerTokenTypeDone;
			case '\n':
				self->currentLineNumber++;
			case '\t':
			case ' ':
			case '\r':
				break;
			case '.':
				return L1LexerTokenTypePeriod;
			case ';':
				return L1LexerTokenTypeTerminal;
			case '(':
				return L1LexerTokenTypeOpenParenthesis;
			case ')':
				return L1LexerTokenTypeCloseParenthesis;
			case ',':
				return L1LexerTokenTypeComma;
			case ':':
				return L1LexerTokenTypeSingleColon;
			case '?':
				return L1LexerTokenTypeMatch;
			case '|':
				return L1LexerTokenTypeBar;
			case '=':
				return L1LexerTokenTypeSingleEqual;
			case '-':
				if (* self->input == '>')
				{
					self->input++;
					return L1LexerTokenTypeSingleLineArrow;
				}
				self->error = L1LexerErrorUnexpectedCharacter;
				return L1LexerTokenTypeDone;
			case '$':
				return L1LexerTokenTypeDollar;
			case '&':
				return L1LexerTokenTypeAmpersand;
			case '#':
				if (strncmp(self->input, "let", 3) == 0)
				{
					self->input += 3;
					return L1LexerTokenTypeLet;
				}
				if (strncmp(self->input, "fn", 2) == 0)
				{
					self->input += 2;
					return L1LexerTokenTypeFn;
				}
				if (strncmp(self->input, "pi", 2) == 0)
				{
					self->input += 2;
					return L1LexerTokenTypePi;
				}
				if (strncmp(self->input, "sg", 2) == 0)
				{
					self->input += 2;
					return L1LexerTokenTypeSigma;
				}
				if (strncmp(self->input, "adt", 3) == 0)
				{
					self->input += 3;
					return L1LexerTokenTypeADT;
				}
				if (strncmp(self->input, "self", 4) == 0)
				{
					self->input += 4;
					return L1LexerTokenTypeSelf;
				}
				if (strncmp(self->input, "universe", 8) == 0)
				{
					self->input += 8;
					return L1LexerTokenTypeUniverse;
				}
				if (strncmp(self->input, "declare", 7) == 0)
				{
					self->input += 7;
					return L1LexerTokenTypeDeclare;
				}
				self->error = L1LexerErrorUnexpectedCharacter;
				return L1LexerTokenTypeDone;
			case '"':
				while (* self->input)
				{
					char c = * self->input;
					self->input++;
					switch (c)
					{
						case '"':
							return L1LexerTokenTypeString;
						case '\\':
							{
								char c = * self->input;
								self->input++;
								char e;
								switch (c)
								{
									case '\\': e = '\\'; break;
									case 'n': e = '\n'; break;
									case 'r': e = '\r'; break;
									case '"': e = '"'; break;
									case 't': e = '\t'; break;
									default:
										self->error =
											L1LexerErrorInvalidEscapeSequence;
										return L1LexerTokenTypeDone;
								}
								L1ArrayAppend(& self->characterBuffer, & e, 1);
							}
							break;
						case '\n':
							self->currentLineNumber++;
						default:
							L1ArrayAppend(& self->characterBuffer, & c, 1);
							break;
					}
				}
				self->error = L1LexerErrorUnterminatedString;
				return L1LexerTokenTypeDone;
			case '/':
				if (* self->input == '/')
				{
					while (* self->input and ('\n' not_eq * self->input))
						self->input++;
					break;
				}
				else if (* self->input == '*')
				{
					self->input++;
					while (* self->input)
					{
						if (self->input[0] == '\n')
							self->currentLineNumber++;
						if (self->input[0] == '*' and self->input[1] == '/')
						{
							self->input += 2;
							goto found_end_of_multline_comment;
						}
						self->input++;
					}
					if (not (* self->input))
					{
						self->error = L1LexerErrorUnterminatedComment;
						return L1LexerTokenTypeDone;
					}
					found_end_of_multline_comment:
					break;
				}
				break;
			default:
				if ('0' <= c and c <= '9')
				{
					//L1ArrayAppend(& self->characterBuffer, & c, 1);
					Mul10Add(& self->characterBuffer, c - '0');
					char c;
					while ('0' <= (c = * (self->input ++)) and c <= '9')
					{
						Mul10Add(& self->characterBuffer, c - '0');
						//L1ArrayAppend(& self->characterBuffer, & c, 1);
					}
					self->input--;
					return L1LexerTokenTypeNatural;
				}
				{
					/*static const char restrictedCharacters[UCHAR_MAX] =
					{
						[' '] = 1,
						['\r'] = 1,
						['\n'] = 1,
						['\t'] = 1,
						['/'] = 1,
						['('] = 1,
						[')'] = 1,
						['.'] = 1,
						['?'] = 1,
						['|'] = 1,
						[';'] = 1,
						['-'] = 1,
						['$'] = 1,
						['='] = 1,
						[':'] = 1,
						['&'] = 1,
						['#'] = 1,
						[','] = 1,
						['\0'] = 1,
					};
					do
					{
						L1ArrayAppend(& self->characterBuffer, & c, 1);
						c = * self->input;
						self->input++;
					}
					while (not restrictedCharacters[(unsigned char) c]);*/
					if (not isalpha(c))
					{
						self->error = L1LexerErrorUnexpectedCharacter;
						return L1LexerTokenTypeDone;
					}
						
					do
					{
						L1ArrayAppend(& self->characterBuffer, & c, 1);
						c = * self->input;
						self->input++;
					}
					while (isalnum(c));
					self->input--;
					return L1LexerTokenTypeIdentifier;
				}
				break;
		}
	}
	return L1LexerTokenTypeDone;
}
