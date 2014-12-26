#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include <stdbool.h>
#include "L1Array.h"
#include <string.h>
#include <assert.h>
#include "L1Parser.h"
#include "L1IR.h"
#include "L1GenerateIR.h"

static void PrintHex(FILE* outputFile, const char* bytes, size_t byteCount)
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
}

static void PrintAST(FILE* outputFile, FILE* logFile, const L1ParserASTNode* nodes, size_t nodeCount, size_t currentNodeIndex, const char** tokenStrings, const size_t* tokenStringLengths)
{
	if (not currentNodeIndex) abort();
	/*switch (nodes[currentNodeIndex - 1].type)
	{
		case L1ParserASTNodeTypeIdentifier:
			fprintf(outputFile, "{\"_type\":\"Identifier\",");
			fprintf(outputFile, "\"data\":\"");
			{
				size_t i = nodes[currentNodeIndex - 1].data.identifier.tokenIndex - 2;
				//fprintf(stderr, "node %u\n", (unsigned int) i);
				PrintHex(outputFile, tokenStrings[i], tokenStringLengths[i]);
			}
			fprintf(outputFile, "\"}");
			break;
		case L1ParserASTNodeTypeString:
			fprintf(outputFile, "{\"_type\":\"String\",");
			fprintf(outputFile, "\"data\":\"");
			{
				size_t i = nodes[currentNodeIndex - 1].data.string.tokenIndex - 2;
				//fprintf(stderr, "node %u\n", (unsigned int) i);
				PrintHex(outputFile, tokenStrings[i], tokenStringLengths[i]);
			}
			fprintf(outputFile, "\"}");
			break;
		case L1ParserASTNodeTypeNatural:
			fprintf(outputFile, "{\"_type\":\"Natural\",");
			fprintf(outputFile, "\"data\":\"");
			{
				size_t i = nodes[currentNodeIndex - 1].data.natural.tokenIndex - 2;
				//fprintf(stderr, "node %u\n", (unsigned int) i);
				PrintHex(outputFile, tokenStrings[i], tokenStringLengths[i]);
			}
			fprintf(outputFile, "\"}");
			break;
		case L1ParserASTNodeTypeEvaluateArgument:
			fprintf(outputFile, "{\"_type\":\"EvaluateArgument\",");
			fprintf(outputFile, "\"expression\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.evaluateArgument.expression, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeOverload:
			fprintf(outputFile, "{\"_type\":\"Overload\",");
			fprintf(outputFile, "\"first\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.overload.first, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"second\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.overload.second, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeAssign:
			fprintf(outputFile, "{\"_type\":\"Assign\",");
			fprintf(outputFile, "\"destination\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.assign.destination, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"source\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.assign.source, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"followingContext\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.assign.followingContext, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeDefine:
			fprintf(outputFile, "{\"_type\":\"Define\",");
			fprintf(outputFile, "\"destination\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.define.destination, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"source\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.define.source, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"followingContext\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.define.followingContext, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeAnnotate:
			fprintf(outputFile, "{\"_type\":\"Annotate\",");
			fprintf(outputFile, "\"value\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.annotate.value, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"type\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.annotate.type, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeLambda:
			fprintf(outputFile, "{\"_type\":\"Lambda\",");
			fprintf(outputFile, "\"result\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.lambda.result, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"argument\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.lambda.argument, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypePi:
			fprintf(outputFile, "{\"_type\":\"Pi\",");
			fprintf(outputFile, "\"result\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.pi.result, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"argument\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.pi.argument, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeUnderscore:
			abort();
			break;
		case L1ParserASTNodeTypeCall:
			fprintf(outputFile, "{\"_type\":\"Call\",");
			fprintf(outputFile, "\"callee\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.call.callee, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"argument\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.call.argument, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
		case L1ParserASTNodeTypeDeclare:
			fprintf(outputFile, "{\"_type\":\"Declare\",");
			fprintf(outputFile, "\"destination\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.declare.destination, tokenStrings, tokenStringLengths);
			fprintf(outputFile, ",\"followingContext\":");
			PrintAST(outputFile, logFile, nodes, nodeCount, nodes[currentNodeIndex - 1].data.declare.followingContext, tokenStrings, tokenStringLengths);
			fprintf(outputFile, "}");
			break;
	}*/
}

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
	OutputTypeAST,
	OutputTypeIR,
}OutputType;

int main(int argc, const char** argv)
{
	OutputType outputType = OutputTypeIR;
	FILE* inputFile = NULL;
	FILE* outputFile = stdout;
	for (int i = 0; i < argc; i++)
	{
		const char* arg = argv[i];
		
		if (strcmp(arg, "--ast") == 0)
		{
			outputType = OutputTypeAST;
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
	//L1LexerInitialize(& lexer, "nats :: __universe 0;\nzero :: nats;\nsucc (x : nats) :: nats;\nconst = (%T : __universe 0) (a : T) (b : T) -> a;\nid = (a : __universe 0) -> a;\nbranch = (0 a b -> a & 1 a b -> b);\nid :(a : __universe 0) => __universe 0");
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
							if (outputType == OutputTypeAST)
							{
								PrintAST(outputFile, stderr, L1ParserGetASTNodes(& parser), L1ParserGetASTNodeCount(& parser), L1ParserGetRootASTNodeIndex(& parser), L1ArrayGetElements(& tokenStrings), L1ArrayGetElements(& tokenStringLengths));
							}
							else if (outputType == OutputTypeIR)
							{
								L1IRGlobalState globalState;
								L1IRGlobalStateInitialize(& globalState);
								L1IRLocalState localState;
								L1IRLocalStateInitialize(& localState);

								fputs("\nRunning block...\n", stderr);
								L1IRLocalAddress resultLocalAddress = L1GenerateIR(& globalState, & localState, L1ParserGetASTNodes(& parser), L1ParserGetASTNodeCount(& parser), L1ParserGetRootASTNodeIndex(& parser), L1ArrayGetElements(& tokenStrings), L1ArrayGetElements(& tokenStringLengths), L1ArrayGetElementCount(& tokenStrings));
								fprintf(stderr, "result: #%u\n", (unsigned) resultLocalAddress);
								for (size_t i = 0; i < L1ArrayGetElementCount(& localState.slots); i++)
								{
									L1IRSlot slot = ((const L1IRSlot*) L1ArrayGetElements(& localState.slots))[i];
									fprintf(stderr, "#%u: %u (%u, %u, %u)\n", (unsigned) i, (unsigned) L1IRExtractSlotType(slot), (unsigned) L1IRExtractSlotOperand(slot, 0), (unsigned) L1IRExtractSlotOperand(slot, 1), (unsigned) L1IRExtractSlotOperand(slot, 2));
								}

								L1IRLocalStateDeinitialize(& localState);
								L1IRGlobalStateDeinitialize(& globalState);
							}
							goto done;
						case L1ParserStatusTypeUnexpectedSymbol:
							fprintf(stderr, "Unexpected symbol at line %u\n", (unsigned int) L1LexerGetCurrentLineNumber(& lexer));
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

	/*
	//fputs("\nNow testing IR...\n", stderr);
	
	L1IRGlobalState globalState;
	L1IRGlobalStateInitialize(& globalState);
	L1IRLocalState localState;
	L1IRLocalStateInitialize(& localState);

	const L1IRSlot slots[] = 
	{
		L1IRMakeSlot(L1IRSlotTypeUnitType, 0, 0, 0),
		L1IRMakeSlot(L1IRSlotTypeArgument, 0, 0, 0),
	};
	const uint16_t slotCount = sizeof(slots) / sizeof(slots[0]);
	fputs("\nCreating block\n", stderr);
	L1IRGlobalAddress simpleBlockAddress = L1IRGlobalStateCreateBlock(& globalState, L1IRGlobalStateBlockTypeLambda, slots, slotCount, 1);
	//L1ArrayPush(& computationBuffer.slots, (L1IRSlot[1]){L1IRMakeSlot(L1IRSlotTypeUnit, 0, 0, 0)}, sizeof(L1IRSlot));
	L1IRLocalAddress unitLocalAddress = L1IRLocalStateCreateSlot(& localState, L1IRMakeSlot(L1IRSlotTypeUnit, 0, 0, 0));
	fputs("\nRunning block...\n", stderr);
	uint16_t resultLocalAddress = L1IRGlobalStateCall(& globalState, & localState, simpleBlockAddress, unitLocalAddress);
	L1IRLocalAddress resultLocalAddress = L1GenerateIR(& globalState, & localState, const L1ParserASTNode* nodes, size_t nodeCount, size_t rootNodeIndex, const unsigned char* const* tokenStrings, const size_t* tokenStringLengths, size_t tokenStringCount);
	//assert(resultLocalAddress);
	//assert(L1ArrayGetElementCount(& computationSlots) == 1);
	//assert(L1IRExtractSlotType(* (const L1IRSlot*) L1ArrayGetElements(& computationSlots)) == L1IRSlotTypeUnit);
	fprintf(stderr, "result: #%u\n", (unsigned) resultLocalAddress);
	for (size_t i = 0; i < L1ArrayGetElementCount(& localState.slots); i++)
	{
		L1IRSlot slot = ((const L1IRSlot*) L1ArrayGetElements(& localState.slots))[i];
		fprintf(stderr, "#%u: %u (%u, %u, %u)\n", (unsigned) i, (unsigned) L1IRExtractSlotType(slot), (unsigned) L1IRExtractSlotOperand(slot, 0), (unsigned) L1IRExtractSlotOperand(slot, 1), (unsigned) L1IRExtractSlotOperand(slot, 2));
	}


	L1IRLocalStateDeinitialize(& localState);
	L1IRGlobalStateDeinitialize(& globalState);*/
	
	return 0;
}
