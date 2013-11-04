#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include "L1Parser.h"
#include "L1Array.h"
#include "L1Region.h"
#include <string.h>
#include <assert.h>
#include "L1GenerateIR.h"

static void PrintASTNode(const L1ParserASTNode* node, int indentLevel)
{
	if(not node) return;
	for (int i = 0; i < indentLevel; i++) fputc('\t', stdout);
	switch (node->type)
	{
		case L1ParserASTNodeTypeNatural:
			fputs("Natural: ", stdout);
			for (uint64_t j = 0; j < node->data.natural.byteCount; j++) fputc(node->data.natural.bytes[j], stdout);
			fputc('\n', stdout);
			break;
		case L1ParserASTNodeTypeString:
			fputs("String: ", stdout);
			for (uint64_t j = 0; j < node->data.string.byteCount; j++) fputc(node->data.string.bytes[j], stdout);
			fputc('\n', stdout);
			break;
		case L1ParserASTNodeTypeIdentifier:
			fputs("Identifier: ", stdout);
			for (uint64_t j = 0; j < node->data.identifier.byteCount; j++) fputc(node->data.identifier.bytes[j], stdout);
			fputc('\n', stdout);
			break;
		case L1ParserASTNodeTypeCall:
		{
			fputs("Call: \n", stdout);
			for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			fputs("(callee) \n", stdout);
			PrintASTNode(node->data.call.callee, indentLevel + 2);
			//for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			const L1ParserASTNodeLinkedList* arguments = node->data.call.arguments;
			while (arguments)
			{
				for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
				fputs("(argument) \n", stdout);
				PrintASTNode(arguments->head, indentLevel + 2);
				arguments = arguments->tail;
			}
		}
			break;
		case L1ParserASTNodeTypeAssignment:
		{
			fputs("Assignment: \n", stdout);
			for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			fputs("(destination) \n", stdout);
			PrintASTNode(node->data.assignment.destination, indentLevel + 2);
			
			const L1ParserASTNodeLinkedList* arguments = node->data.assignment.arguments;
			while (arguments)
			{
				for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
				fputs("(argument) \n", stdout);
				PrintASTNode(arguments->head, indentLevel + 2);
				arguments = arguments->tail;
			}
			
			for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			fputs("(source) \n", stdout);
			PrintASTNode(node->data.assignment.source, indentLevel + 2);
			
			for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			fputs("(following context) \n", stdout);
			PrintASTNode(node->data.assignment.followingContext, indentLevel + 2);
		}
			break;
		case L1ParserASTNodeTypeBranch:
			fputs("Branch: \n", stdout);
			for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			fputs("(condition) \n", stdout);
			PrintASTNode(node->data.branch.condition, indentLevel + 2);
			for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			fputs("(result if true) \n", stdout);
			PrintASTNode(node->data.branch.resultIfTrue, indentLevel + 2);
			for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
			fputs("(result if false) \n", stdout);
			PrintASTNode(node->data.branch.resultIfFalse, indentLevel + 2);
			break;
		case L1ParserASTNodeTypeList:
		{
			fputs("List: \n", stdout);
			const L1ParserASTNodeLinkedList* elements = node->data.list.elements;
			while (elements)
			{
				for (int i = 0; i < indentLevel + 1; i++) fputc('\t', stdout);
				fputs("(element) \n", stdout);
				PrintASTNode(elements->head, indentLevel + 2);
				elements = elements->tail;
			}
		}
			break;
		default:
			abort();
			break;
	}
}

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
		switch (L1LexerGetError(lexer))
		{
			case L1LexerErrorTypeNone:
				break;
			case L1LexerErrorTypeInvalidSequence:
				abort();
				break;
			case L1LexerErrorTypeStringDidNotTerminate:
				abort();
				break;
		}
		const void* bytes = L1LexerGetLastTokenBytes(lexer, & token.byteCount);
		token.bytes = memcpy(L1RegionAllocate(tokenDataRegion, token.byteCount), bytes, token.byteCount);
		printf("Token: %i\n", (int) token.type);
		L1ArrayAppend(& tokenArray, & token, sizeof(token));
	}while (token.type not_eq L1LexerTokenTypeDone);
	
	L1LexerDelete(lexer);
	
	L1Parser* parser = L1ParserNew(L1ArrayGetElements(& tokenArray), L1ArrayGetElementCount(& tokenArray));
	L1ParserErrorType errorType = L1ParserGetError(parser);
	switch (errorType)
	{
		case L1ParserErrorTypeNone:
			{
				const L1ParserASTNode* rootASTNode = L1ParserGetRootASTNode(parser);
				
				assert(rootASTNode);
				
				PrintASTNode(rootASTNode, 0);
				
				L1ByteBuffer* buffer = L1ByteBufferNew();
				
				L1GenerateIR(rootASTNode, buffer, NULL);
				
				{
					FILE* outputFile = fopen(outputPath, "wb");
					assert(outputFile);
					uint8_t header[] = "L1IRV1\0";
					fwrite(header, sizeof(header), 1, outputFile);
					size_t byteCount = 0;
					const void* bytes = L1ByteBufferGetBytes(buffer, & byteCount);
					{
						uint64_t byteCount64 = byteCount;
						fwrite(& byteCount64, sizeof(uint64_t), 1, outputFile);
					}
					fwrite(bytes, byteCount, 1, outputFile);
					fclose(outputFile);
				}
				
				L1ByteBufferDelete(buffer);
			}
			break;
		case L1ParserErrorTypeUnexpectedToken:
			puts("Parser Error: Unexpected token.");
			break;
		case L1ParserErrorTypeUnknown:
			printf("Parser Error: Unknown\n");
			break;
	}
	
	/*L1IRBuffer* buffer = L1IRBufferNew();
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
	
	L1IRBufferDelete(buffer);*/
	
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
