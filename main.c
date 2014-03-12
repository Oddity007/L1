#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include "L1Parser.h"
#include "L1Array.h"
#include "L1Region.h"
#include <string.h>
#include <assert.h>

static void printHex(FILE* outputFile, const uint8_t* bytes, size_t byteCount)
{
	char chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	for (size_t i = 0; i < byteCount; i++)
	{
		fputc(chars[(bytes[i] >> 4) & 0xF], outputFile);
		fputc(chars[(bytes[i] >> 0) & 0xF], outputFile);
	}
}

static void PrintASTNodeJSON(FILE* outputFile, const L1ParserASTNode* node)
{
	if (not node)
	{
		fprintf(outputFile, "{\"type\" : \"undefined\"}");
		return;
	}
	switch (node->type)
	{
		case L1ParserASTNodeTypeNatural:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"natural\", \"value\" : \"");
			fwrite(node->data.natural.bytes, node->data.natural.byteCount, 1, outputFile);
			//printHex(outputFile, node->data.natural.bytes, node->data.natural.byteCount);
			fprintf(outputFile, "\"}");
			break;
		case L1ParserASTNodeTypeString:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"string\", \"value\" : \"");
			printHex(outputFile, node->data.string.bytes, node->data.string.byteCount);
			fprintf(outputFile, "\"}");
			break;
		case L1ParserASTNodeTypeIdentifier:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"identifier\", \"value\" : \"");
			fwrite(node->data.identifier.bytes, node->data.identifier.byteCount, 1, outputFile);
			//printHex(outputFile, node->data.identifier.bytes, node->data.identifier.byteCount);
			fprintf(outputFile, "\"}");
			break;
		case L1ParserASTNodeTypeCall:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"call\", ");
			fprintf(outputFile, "\"callee\" : ");
			PrintASTNodeJSON(outputFile, node->data.call.callee);
			fprintf(outputFile, ", \"arguments\" : [");
			for (const L1ParserASTNodeLinkedList* arguments = node->data.call.arguments; arguments; arguments = arguments->tail)
			{
				PrintASTNodeJSON(outputFile, arguments->head);
				if (arguments->tail) fprintf(outputFile, ", ");
			}
			fprintf(outputFile, "]");
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeAssignment:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"assignment\", ");
			fprintf(outputFile, "\"isConstructor\" : ");
			fprintf(outputFile, node->data.assignment.isConstructor ? "true" : "false");
			fprintf(outputFile, ", \"destination\" : ");
			PrintASTNodeJSON(outputFile, node->data.assignment.destination);
			fprintf(outputFile, ", \"source\" : ");
			PrintASTNodeJSON(outputFile, node->data.assignment.source);
			fprintf(outputFile, ", \"followingContext\" : ");
			PrintASTNodeJSON(outputFile, node->data.assignment.followingContext);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeBranch:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"branch\", ");
			fprintf(outputFile, "\"condition\" : ");
			PrintASTNodeJSON(outputFile, node->data.branch.condition);
			fprintf(outputFile, ", \"resultIfTrue\" : ");
			PrintASTNodeJSON(outputFile, node->data.branch.resultIfTrue);
			fprintf(outputFile, ", \"resultIfFalse\" : ");
			PrintASTNodeJSON(outputFile, node->data.branch.resultIfTrue);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeList:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"list\", ");
			fprintf(outputFile, "\"elements\" : [");
			for (const L1ParserASTNodeLinkedList* elements = node->data.list.elements; elements; elements = elements->tail)
			{
				PrintASTNodeJSON(outputFile, elements->head);
				if (elements->tail) fprintf(outputFile, ", ");
			}
			fprintf(outputFile, "]");
			fprintf(outputFile, ", \"sublist\" : ");
			PrintASTNodeJSON(outputFile, node->data.list.sublist);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeConstructorConstraint:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"list\", ");
			fprintf(outputFile, "\"expression\" : ");
			PrintASTNodeJSON(outputFile, node->data.constructorConstraint.expression);
			fprintf(outputFile, ", \"construction\" : ");
			PrintASTNodeJSON(outputFile, node->data.constructorConstraint.construction);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeEval:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"eval\", ");
			fprintf(outputFile, "\"expression\" : ");
			PrintASTNodeJSON(outputFile, node->data.eval.expression);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeAnonymousFunction:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"anonymousFunction\", ");
			fprintf(outputFile, ", \"isConstructor\" : ");
			fprintf(outputFile, node->data.anonymousFunction.isConstructor ? "true" : "false");
			fprintf(outputFile, ", \"arguments\" : [");
			for (const L1ParserASTNodeLinkedList* arguments = node->data.anonymousFunction.arguments; arguments; arguments = arguments->tail)
			{
				PrintASTNodeJSON(outputFile, arguments->head);
				if (arguments->tail) fprintf(outputFile, ", ");
			}
			fprintf(outputFile, "]");
			fprintf(outputFile, ", \"source\" : ");
			PrintASTNodeJSON(outputFile, node->data.anonymousFunction.source);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeOption:
			fprintf(outputFile, "{");
			fprintf(outputFile, "\"type\" : \"option\", ");
			fprintf(outputFile, "\"construction\" : ");
			PrintASTNodeJSON(outputFile, node->data.option.construction);
			fprintf(outputFile, ", \"defaultConstruction\" : ");
			PrintASTNodeJSON(outputFile, node->data.option.defaultConstruction);
			fprintf(outputFile, "}");
			break;
	}
}

static char* LoadFileAsString(FILE* file)
{
	assert(file);
	
	fseek (file, 0, SEEK_END);
	
	size_t length = ftell(file);
	
	rewind(file);
	
	char* string = malloc(length + 1);
	
	fread(string, 1, length, file);
	string[length] = 0;
	
	return string;
	
	/*L1Array characterArray;
	L1ArrayInitialize(& characterArray);
	while (not feof(file))
	{
		char character = fgetc(file);
		L1ArrayAppend(& characterArray, & character, sizeof(char));
	}
	L1ArrayDeinitialize(& characterArray);
	
	size_t size = L1ArrayGetElementCount(& characterArray);
	char* characters = calloc(1, size + 1);
	//characters[size] = 0;
	return memcpy(characters, L1ArrayGetElements(& characterArray), size);*/
}

typedef enum
{
	OutputTypeJSON,
	OutputTypeLuaTable
}OutputType;

static void Compile(FILE* inputFile, FILE* logFile,  FILE* outputFile, OutputType outputType)
{
	char* codeString = LoadFileAsString(inputFile);
	//fputs(codeString, logFile);
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
		//fprintf(logFile, "Token: %i\n", (int) token.type);
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
				
				switch (outputType)
				{
					case OutputTypeLuaTable:
						{
							//PrintASTNodeLuaTable(outputFile, rootASTNode);
							abort();
						}
						break;
					
					case OutputTypeJSON:
						{
							PrintASTNodeJSON(outputFile, rootASTNode);
						}
						break;
					
					default:
						abort();
						break;
				}
			}
			break;
		case L1ParserErrorTypeUnexpectedToken:
			fprintf(logFile, "Parser Error: Unexpected token.\n");
			break;
		case L1ParserErrorTypeUnknown:
			fprintf(logFile, "Parser Error: Unknown\n");
			break;
	}
	
	L1ParserDelete(parser);
	
	L1ArrayDeinitialize(& tokenArray);
	L1RegionDelete(tokenDataRegion);
	
	free(codeString);
}

int main(int argc, const char** argv)
{
	OutputType outputType = OutputTypeJSON;
	FILE* inputFile = NULL;
	FILE* outputFile = stdout;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		
		if (strcmp(arg, "--lua") == 0)
		{
			outputType = OutputTypeLuaTable;
		}
		else if (strcmp(arg, "--json") == 0)
		{
			outputType = OutputTypeJSON;
		}
		else if (strcmp(arg, "-i") == 0)
		{
			assert(argc - 1 > i);
			assert(not inputFile);
			inputFile = fopen(argv[i + 1], "r");
			i++;
		}
		else if (strcmp(arg, "-it") == 0)
		{
			assert(argc - 1 > i);
			assert(not inputFile);
			inputFile = tmpfile();
			fputs(argv[i + 1], inputFile);
			i++;
		}
		else if (strcmp(arg, "-o") == 0)
		{
			assert(argc - 1 > i);
			assert(outputFile == stdout);
			outputFile = fopen(argv[i + 1], "w");
			i++;
		}
	}
	
	if (inputFile)
	{
		Compile(inputFile, stderr, outputFile, outputType);
		fclose(inputFile);
	}
	
	return 0;
}
