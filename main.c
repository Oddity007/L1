//
//  main.cpp
//  L1c
//
//  Created by Oliver Daids on 10/30/12.
//  Copyright (c) 2012 Oliver Daids. All rights reserved.
//

#include <stdio.h>
#include <iso646.h>
#include "L1Lexer.h"
#include <assert.h>
#include "L1Parser.h"
#include <stdlib.h>
#include <string.h>

static uint_least32_t PumpCharacter(L1Lexer* self)
{
	int c=fgetc((FILE*)self->userdata);
	if(c==EOF) c=0;
	return c;
}

static uint_least32_t PeekCharacter(L1Lexer* self)
{
	int c=fgetc((FILE*)self->userdata);
	ungetc(c, (FILE*)self->userdata);
	if(c==EOF) c=0;
	return c;
}

int main(int argc, const char** argv)
{
	FILE* file = fopen("sample1.l1", "r");
	assert(file);
	L1Lexer lexer;
	L1LexerInit(& lexer);
	lexer.userdata = file;
	lexer.pump = PumpCharacter;
	lexer.peek = PeekCharacter;
	
	/*L1Parser* parser = L1ParserNew();
	assert(L1ParserParse(parser, & lexer));
	L1ParserDelete(parser);*/
	
	/*while(not feof(file))
	{
		L1LexerError error = L1LexerLex(& lexer);
		switch (error)
		{
			case L1LexerErrorNone:
				break;
			case L1LexerErrorUnknown:
				fprintf(stderr, "error in lexing on line #%i, column #%i", (int)lexer.currentLineNumber, (int)lexer.currentCharacterOfLineNumber);
				goto end;
		}
		printf("Token type \"%s\" with data of \"%s\"\n", L1LexerTokenTypeAsString(lexer.lastTokenType), lexer.buffer);
	}*/
	
	void* parser = L1ParserAlloc(malloc);
	L1ParserNode* rootNode = NULL;
	while(not feof(file))
	{
		L1LexerError error = L1LexerLex(& lexer);
		switch (error)
		{
			case L1LexerErrorNone:
				break;
			case L1LexerErrorUnknown:
				fprintf(stderr, "error in lexing on line #%i, column #%i", (int)lexer.currentLineNumber, (int)lexer.currentCharacterOfLineNumber);
				goto end;
		}
		printf("Token type \"%s\" with data of \"%s\"\n", L1LexerTokenTypeAsString(lexer.lastTokenType), lexer.buffer);
		int parserTokenType = 0;
		L1ParserNode* tokenNode = NULL;
		switch (lexer.lastTokenType)
		{
			case L1LexerTokenTypeDone:
				parserTokenType = L1ParserTokenTypeDone;
				break;
			case L1LexerTokenTypeTerminal:
				parserTokenType = L1ParserTokenTypeTerminal;
				break;
			case L1LexerTokenTypeComma:
				parserTokenType = L1ParserTokenTypeComma;
				break;
			case L1LexerTokenTypeAssign:
				parserTokenType = L1ParserTokenTypeAssign;
				break;
			case L1LexerTokenTypeElipsis:
				parserTokenType = L1ParserTokenTypeEllipsis;
				break;
			case L1LexerTokenTypeIdentifier:
				parserTokenType = L1ParserTokenTypeIdentifier;
				tokenNode = L1ParserNodeNew(L1ParserNodeTypeIdentifier);
				tokenNode->data.identifier.characters = strdup(lexer.buffer);
				tokenNode->data.identifier.characterCount = strlen(tokenNode->data.identifier.characters);
				break;
			case L1LexerTokenTypeOpeningParenthesis:
				parserTokenType = L1ParserTokenTypeOpeningParenthesis;
				break;
			case L1LexerTokenTypeClosingParenthesis:
				parserTokenType = L1ParserTokenTypeClosingParenthesis;
				break;
			case L1LexerTokenTypeOpeningSquareBracket:
				parserTokenType = L1ParserTokenTypeOpeningSquareBracket;
				break;
			case L1LexerTokenTypeClosingSquareBracket:
				parserTokenType = L1ParserTokenTypeClosingSquareBracket;
				break;
			case L1LexerTokenTypeNumber:
				parserTokenType = L1ParserTokenTypeNumber;
				tokenNode->data.number.characters = strdup(lexer.buffer);
				tokenNode->data.number.characterCount = strlen(tokenNode->data.identifier.characters);
				break;
			case L1LexerTokenTypeString:
				parserTokenType = L1ParserTokenTypeString;
				tokenNode->data.string.characters = strdup(lexer.buffer);
				tokenNode->data.string.characterCount = strlen(tokenNode->data.identifier.characters);
				break;
			case L1LexerTokenTypeQuestionMark:
				parserTokenType = L1ParserTokenTypeQuestionMark;
				break;
			case L1LexerTokenTypeTypeQualifier:
				parserTokenType = L1ParserTokenTypeTypeQualifier;
				break;
		}
		L1Parser(parser, parserTokenType, tokenNode, & rootNode);
	}
	
	L1ParserNodePrint(rootNode, 0);
	
	L1ParserFree(parser, free);
	
	end:
	fclose(file);
	L1LexerDeinit(& lexer);
    return 0;
}
