#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include <stdbool.h>
#include "L1Array.h"
#include <string.h>
#include <assert.h>
#include "L1Parser.h"
#include "L1IRState.h"

#include "L1IRSlotDebugInfo"

/*static void PrintHex(FILE* outputFile, const char* bytes, size_t byteCount)
{
	//assert(byteCount > 0);
	//fputs("Hex of ", stderr);
	//if (byteCount)
	//		fwrite(bytes, byteCount, 1, stderr);
	//fputs("\n", stderr);
	char chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	for (size_t i = byteCount; i-- > 0;)
	{
		fputc(chars[(bytes[i] >> 4) & 0xF], outputFile);
		fputc(chars[(bytes[i] >> 0) & 0xF], outputFile);
	}
}*/

static char* LoadFileAsString(FILE* file)
{
	assert(file);
	
	fseek (file, 0, SEEK_END);
	
	size_t length = ftell(file);
	
	rewind(file);
	
	char* string = malloc(length + 1);
	
	if (1 not_eq fread(string, length, 1, file))
		abort();
	string[length] = 0;
	
	return string;
}

static char* CloneString(const char* s, size_t length)
{
	if (not length) return NULL;
	if (not s) return NULL;
	return memcpy(malloc(length), s, length);
}

typedef enum
{
	OutputTypeIR,
	OutputTypeRun,
}OutputType;

int main(int argc, const char** argv)
{
	OutputType outputType = OutputTypeRun;
	FILE* inputFile = NULL;
	FILE* outputFile = stdout;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		
		if (strcmp(arg, "--ir") == 0)
		{
			outputType = OutputTypeIR;
		}
		else if (strcmp(arg, "--run") == 0)
		{
			outputType = OutputTypeRun;
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
	
	if (not inputFile)
	{
		fputs("No input\n", stderr);
		return 1;
	}
	
	if (not outputFile)
	{
		fputs("No output specified\n", stderr);
		return 1;
	}
	
	char* codeString = LoadFileAsString(inputFile);
	
	L1Lexer lexer;
	L1LexerInitialize(& lexer, codeString);
	
	L1Parser parser;
	L1ParserInitialize(& parser);
	
	L1Array tokenStrings;
	L1Array tokenStringLengths;
	L1ArrayInitialize(& tokenStrings);
	L1ArrayInitialize(& tokenStringLengths);
	
	size_t tokenID = 0;
	while (true)
	{
		L1LexerTokenType type = L1LexerLex(& lexer);
		switch (L1LexerGetError(& lexer))
		{
			case L1LexerErrorNone:
				{
					//printf("%i ", (int) type);
					size_t length = L1LexerGetPreviousTokenDataStringLength(& lexer);
					const char* string = CloneString(L1LexerGetPreviousTokenDataString(& lexer), length);
					L1ArrayPush(& tokenStrings, & string, sizeof(const char*));
					L1ArrayPush(& tokenStringLengths, & length, sizeof(size_t));
					tokenID++;
					//fprintf(stderr, "Token #%u ", (unsigned int) tokenID);
					//if (length)
					//	fwrite(string, length, 1, stderr);
					//fputs("\n", stderr);
					switch (L1ParserParse(& parser, type, string, length))
					{
						case L1ParserStatusTypeNone:
							//fprintf(stderr, "tokenID = %u, parser.currentTokenIndex = %u\n", (unsigned int) tokenID, (unsigned int) parser.currentTokenIndex);
							assert(tokenID == parser.currentTokenIndex);
							break;
						case L1ParserStatusTypeDone:
							if (outputType == OutputTypeRun)
							{

								fputs("\nRunning block...\n", stderr);
								//L1IRGlobalAddress globalAddress = 0;//L1GenerateIR(& globalState, L1ParserGetASTNodes(& parser), L1ParserGetASTNodeCount(& parser), L1ParserGetRootASTNodeIndex(& parser), L1ArrayGetElements(& tokenStrings), L1ArrayGetElements(& tokenStringLengths), L1ArrayGetElementCount(& tokenStrings));
								/*L1IRLocalAddress unitLocalAddress = L1IRLocalStateCreateSlot(& localState, L1IRMakeSlot(L1IRSlotTypeUnit, 0, 0, 0));
								uint16_t resultLocalAddress = L1IRGlobalStateCall(& parser.globalState, & localState, parser.root, unitLocalAddress);
								fprintf(stderr, "result: #%u\n", (unsigned) resultLocalAddress);
								for (size_t i = 0; i < L1ArrayGetElementCount(& localState.slots); i++)
								{
									L1IRSlot slot = ((const L1IRSlot*) L1ArrayGetElements(& localState.slots))[i];
									fprintf(stderr, "#%u: %u (%u, %u, %u)\n", (unsigned) i, (unsigned) L1IRExtractSlotType(slot), (unsigned) L1IRExtractSlotOperand(slot, 0), (unsigned) L1IRExtractSlotOperand(slot, 1), (unsigned) L1IRExtractSlotOperand(slot, 2));
								}*/
								
								L1IRState* irstate = L1ParserGetIRState(& parser);
								fprintf(stdout, "Root: %u\n", (unsigned) parser.root);
								
								{
									for (size_t i = 0; i < L1ArrayGetElementCount(& irstate->slots); i++)
									{
										L1IRSlot slot = ((const L1IRSlot*) L1ArrayGetElements(& irstate->slots))[i];
										fprintf(stdout, "#%u: %s (%u, %u, %u)\n", (unsigned) i, L1IRSlotTypeAsString(L1IRExtractSlotType(slot)), (unsigned) L1IRExtractSlotOperand(slot, 0), (unsigned) L1IRExtractSlotOperand(slot, 1), (unsigned) L1IRExtractSlotOperand(slot, 2));
									}
								}
								
								//fprintf(stdout, "Old Root: %u\n", (unsigned) parser.root);
								L1IRAddress newRoot = L1IRStateCreateSlot(irstate, L1IRMakeSlot(L1IRSlotTypeNormalize, parser.root, 0, 0));
								L1IRStateExternalSlotRef rootExternalSlotRef = L1IRStateAcquireExternalSlotRef(irstate, newRoot);
								L1IRStateCollectGarbage(irstate);
								newRoot = L1IRStateGetExternalSlotRefCurrentSlotRef(irstate, rootExternalSlotRef);
								fprintf(stdout, "Root: %u\n", (unsigned) newRoot);
								
								{
									for (size_t i = 0; i < L1ArrayGetElementCount(& irstate->slots); i++)
									{
										L1IRSlot slot = ((const L1IRSlot*) L1ArrayGetElements(& irstate->slots))[i];
										fprintf(stdout, "#%u: %s (%u, %u, %u)\n", (unsigned) i, L1IRSlotTypeAsString(L1IRExtractSlotType(slot)), (unsigned) L1IRExtractSlotOperand(slot, 0), (unsigned) L1IRExtractSlotOperand(slot, 1), (unsigned) L1IRExtractSlotOperand(slot, 2));
									}
								}
								
								L1IRStateReleaseExternalSlotRef(irstate, rootExternalSlotRef);
							}
							goto done;
						case L1ParserStatusTypeUnexpectedSymbol:
							fprintf(stderr, "Unexpected symbol at line %u\n", (unsigned int) L1LexerGetCurrentLineNumber(& lexer));
							goto done;
						case L1ParserStatusTypeUnknown:
							fprintf(stderr, "Unknown error at line %u\n", (unsigned int) L1LexerGetCurrentLineNumber(& lexer));
							goto done;
					}
				}
				break;
			case L1LexerErrorUnexpectedCharacter:
				fputs("Unexpected Character\n", stderr);
				goto done;
			case L1LexerErrorInvalidEscapeSequence:
				fputs("Invalid Escape Sequence\n", stderr);
				goto done;
			case L1LexerErrorUnterminatedString:
				fputs("Unterminated String\n", stderr);
				goto done;
			case L1LexerErrorUnterminatedComment:
				fputs("Unterminated Comment\n", stderr);
				goto done;
		}
		
		if (type == L1LexerTokenTypeDone) goto done;
	}
	
	done:
	
	for (size_t i = 0; i < L1ArrayGetElementCount(& tokenStrings); i++)
	{
		char** tokenStringsElements = L1ArrayGetElements(& tokenStrings);
		free(tokenStringsElements[i]);
	}

	L1ArrayDeinitialize(& tokenStrings);
	L1ArrayDeinitialize(& tokenStringLengths);
	
	L1ParserDeinitialize(& parser);
	
	L1LexerDeinitialize(& lexer);
	
	free(codeString);
	
	return 0;
}
