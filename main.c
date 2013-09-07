#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include "L1Parser.h"
#include "L1Array.h"
#include "L1Region.h"

int main(void)
{
	L1Lexer* lexer = L1LexerNew((const uint8_t*)"a = __add 1 2; a");
	L1Array tokenArray;
	L1ArrayInitialize(& tokenArray);
	L1Region* tokenDataRegion = L1RegionNew();
	L1LexerTokenType tokenType;
	/*do
	{
		L1LexerLexNext(lexer, & tokenType);
		switch (tokenType)
		{
			case L1LexerTokenTypeNatural:
				fputs("Natural: ", stdout);
				{
					uint64_t digitByteCount;
					const uint8_t* digitBytes = L1LexerGetLastTokenBytes(lexer, & digitByteCount);
					for (uint64_t i = 0; i < digitByteCount; i++) fputc(digitBytes[i], stdout);
					fputc('\n', stdout);
				}
				break;
			case L1LexerTokenTypeIdentifier:
				fputs("Identifier: ", stdout);
				{
					uint64_t byteCount;
					const uint8_t* bytes = L1LexerGetLastTokenBytes(lexer, & byteCount);
					for (uint64_t i = 0; i < byteCount; i++) fputc(bytes[i], stdout);
					fputc('\n', stdout);
				}
				break;
			case L1LexerTokenTypeString:
				fputs("String: ", stdout);
				{
					uint64_t byteCount;
					const uint8_t* bytes = L1LexerGetLastTokenBytes(lexer, & byteCount);
					for (uint64_t i = 0; i < byteCount; i++) fputc(bytes[i], stdout);
					fputc('\n', stdout);
				}
				break;
			case L1LexerTokenTypeAssign:
				fputs("Assign\n", stdout);
				break;
			case L1LexerTokenTypeOpeningParenthesis:
				fputs("Opening Parenthesis\n", stdout);
				break;
			case L1LexerTokenTypeClosingParenthesis:
				fputs("Closing Parenthesis\n", stdout);
				break;
			case L1LexerTokenTypeOpeningSquareBracket:
				fputs("Opening Square Bracket\n", stdout);
				break;
			case L1LexerTokenTypeClosingSquareBracket:
				fputs("Closing Square Bracket\n", stdout);
				break;
			case L1LexerTokenTypeComma:
				fputs("Comma\n", stdout);
				break;
			case L1LexerTokenTypeTerminal:
				fputs("Terminal\n", stdout);
				break;
			case L1LexerTokenTypeQuestionMark:
				fputs("Question Mark\n", stdout);
				break;
			case L1LexerTokenTypeDone:
				break;
		}
	}while (tokenType not_eq L1LexerTokenTypeDone);*/
	L1ParserLexedToken token;
	do
	{
		L1LexerLexNext(lexer, & token.type);
		token.bytes = L1LexerGetLastTokenBytes(lexer, & token.byteCount);
	}while (tokenType not_eq L1LexerTokenTypeDone);
	L1LexerDelete(lexer);
	
	L1Parser* parser = L1ParserNew(L1ArrayGetElements(& tokenArray), L1ArrayGetElementCount(& tokenArray));
	const L1ParserASTNode* rootASTNode = L1ParserGetRootASTNode(parser);
	
	
	
	L1ParserDelete(parser);
	L1ArrayDeinitialize(& tokenArray);
	L1RegionDelete(tokenDataRegion);
	return 0;
}
