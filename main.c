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
//#include "L1Parser.h"

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
	L1Lexer lexer;
	L1LexerInit(& lexer);
	lexer.userdata = file;
	lexer.pump = PumpCharacter;
	lexer.peek = PeekCharacter;
	
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
	}
	
	end:
	fclose(file);
	L1LexerDeinit(& lexer);
    return 0;
}