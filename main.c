#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include "L1Parser.h"
#include "L1Array.h"
#include "L1Region.h"
#include <string.h>
#include <assert.h>
#include "L1GenerateIR.h"

static char* LoadFileAsString(const char* filename)
{
	FILE* file=fopen(filename, "r");
	assert(file);
	
	fseek (file, 0, SEEK_END);
	
	size_t length = ftell(file);
	
	rewind(file);
	
	char* string = malloc(length + 1);
	
	fread(string, 1, length, file);
	string[length] = 0;
	
	fclose(file);
	return string;
}

static void CompileFile(const char* inputPath, const char* outputPath)
{
	char* codeString = LoadFileAsString(inputPath);
	L1Lexer* lexer = L1LexerNew((const uint8_t*)codeString);
	
	L1Array tokenArray;
	L1ArrayInitialize(& tokenArray);
	L1Region* tokenDataRegion = L1RegionNew();
	L1ParserLexedToken token;
	
	do
	{
		L1LexerLexNext(lexer, & token.type);
		assert(L1LexerErrorTypeNone == L1LexerGetError(lexer));
		const void* bytes = L1LexerGetLastTokenBytes(lexer, & token.byteCount);
		token.bytes = memcpy(L1RegionAllocate(tokenDataRegion, token.byteCount), bytes, token.byteCount);
		
		L1ArrayAppend(& tokenArray, & token, sizeof(token));
	}while (token.type not_eq L1LexerTokenTypeDone);
	
	L1LexerDelete(lexer);
	
	L1Parser* parser = L1ParserNew(L1ArrayGetElements(& tokenArray), L1ArrayGetElementCount(& tokenArray));
	const L1ParserASTNode* rootASTNode = L1ParserGetRootASTNode(parser);
	
	assert(rootASTNode);
	
	L1IRBuffer* buffer = L1IRBufferNew();
	L1GenerateIR(rootASTNode, buffer, NULL);
	//L1IRBufferPrint(buffer);
	
	{
		FILE* outputFile = fopen(outputPath, "wb");
		assert(outputFile);
		uint8_t header[] = "L1IRV1\0";
		fwrite(header, sizeof(header), 1, outputFile);
		size_t byteCount = 0;
		const void* bytes = L1IRBufferGetBytes(buffer, & byteCount);
		{
			uint64_t byteCount64 = byteCount;
			fwrite(& byteCount64, sizeof(uint64_t), 1, outputFile);
		}
		fwrite(bytes, byteCount, 1, outputFile);
		fclose(outputFile);
	}
	
	L1IRBufferDelete(buffer);
	
	L1ParserDelete(parser);
	
	L1ArrayDeinitialize(& tokenArray);
	L1RegionDelete(tokenDataRegion);
	
	free(codeString);
}

int main(int argc, const char** argv)
{
	const char* outputPath = "a.out";
	const char* inputPath = NULL;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		
		if (arg[0] == '-' and arg[1] == 'i')
		{
			assert(i < argc - 1);
			i++;
			inputPath = argv[i];
		}
		else if (arg[0] == '-' and arg[1] == 'o')
		{
			assert(i < argc - 1);
			i++;
			outputPath = argv[i];
		}
	}
	assert(inputPath);
	CompileFile(inputPath, outputPath);
	
	return 0;
}
