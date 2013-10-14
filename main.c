#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include "L1Parser.h"
#include "L1Array.h"
#include "L1Region.h"
#include <string.h>
#include <assert.h>
//#include "L1FIRGenerator.h"
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

int main(void)
{
	//L1Lexer* lexer = L1LexerNew((const uint8_t*)"f a = (a 1 2 ? 1; 0); //asdfawef \n [c, d] = z \"a\"; [e, g] = [4, 5]; f /*asdfasdf*/");
	L1Lexer* lexer = L1LexerNew((const uint8_t*)"f a = (a 1 2 ? 1; 0); f");
	L1Array tokenArray;
	L1ArrayInitialize(& tokenArray);
	L1Region* tokenDataRegion = L1RegionNew();
	L1ParserLexedToken token;
	fputs("Lexing:\n", stdout);
	do
	{
		L1LexerLexNext(lexer, & token.type);
		assert(L1LexerErrorTypeNone == L1LexerGetError(lexer));
		const void* bytes = L1LexerGetLastTokenBytes(lexer, & token.byteCount);
		token.bytes = memcpy(L1RegionAllocate(tokenDataRegion, token.byteCount), bytes, token.byteCount);
		//fwrite(bytes, token.byteCount, 1, stdout);
		//fputc('\n', stdout);
		//token.byteCount = 0;
		//token.bytes = NULL;
		L1ArrayAppend(& tokenArray, & token, sizeof(token));
		fputc('\t', stdout);
		switch (token.type)
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
	}while (token.type not_eq L1LexerTokenTypeDone);
	L1LexerDelete(lexer);
	
	fputs("\nParsing:\n", stdout);
	
	L1Parser* parser = L1ParserNew(L1ArrayGetElements(& tokenArray), L1ArrayGetElementCount(& tokenArray));
	const L1ParserASTNode* rootASTNode = L1ParserGetRootASTNode(parser);
	
	PrintASTNode(rootASTNode, 1);
	
	puts("Generating IR:");
	L1IRBuffer* buffer = L1IRBufferNew();
	L1GenerateIR(rootASTNode, buffer, NULL);
	L1IRBufferDelete(buffer);
	
	/*puts("Generating FIR:");
	L1FIRGenerator* generator = L1FIRGeneratorNew(rootASTNode);
	{
		uint64_t nodeCount = 0;
		const L1FIRNode* nodes = L1FIRGeneratorGetNodes(generator, & nodeCount);
		for (uint64_t i = 0; i < nodeCount; i++)
		{
			switch (nodes[i].type)
			{
				case L1FIRNodeTypeNoOperation:
					puts("no_operation");
					break;
				case L1FIRNodeTypeLoadUndefined:
					printf("load_undefined %lu\n", (unsigned long)nodes[i].data.loadUndefined.destination);
					break;
				case L1FIRNodeTypeLoadInteger:
					printf("load_integer %lu \"", (unsigned long)nodes[i].data.loadInteger.destination);
					fwrite(nodes[i].data.loadInteger.digits, nodes[i].data.loadInteger.digitCount, 1, stdout);
					fputc('"', stdout);
					fputc('\n', stdout);
					break;
				case L1FIRNodeTypeLoadString:
					printf("load_string %lu [...]\n", (unsigned long)nodes[i].data.loadString.destination);
					break;
				case L1FIRNodeTypeClosure:
					printf("closure %lu [ ", (unsigned long)nodes[i].data.closure.destination);
					for (uint64_t j = 0; j < nodes[i].data.closure.argumentCount; j++)
					{
						printf("%lu ", (unsigned long)nodes[i].data.closure.arguments[j]);
					}
					printf("] %lu\n", (unsigned long)nodes[i].data.closure.result);
					break;
				case L1FIRNodeTypeCall:
					printf("call %lu %lu [ ", (unsigned long)nodes[i].data.call.destination, (unsigned long)nodes[i].data.call.closure);
					for (uint64_t j = 0; j < nodes[i].data.call.argumentCount; j++)
					{
						printf("%lu ", (unsigned long)nodes[i].data.call.arguments[j]);
					}
					printf("]\n");
					break;
				case L1FIRNodeTypeLet:
					printf("let %lu %lu\n", (unsigned long)nodes[i].data.let.destination, (unsigned long)nodes[i].data.let.source);
					break;
				case L1FIRNodeTypeSplitList:
					puts("split_list");
					break;
				case L1FIRNodeTypeNewList:
					puts("new_list");
					break;
				case L1FIRNodeTypeConsList:
					puts("cons_list");
					break;
				case L1FIRNodeTypeBranch:
					printf("branch %lu %lu %lu %lu\n", (unsigned long)nodes[i].data.branch.destination, (unsigned long)nodes[i].data.branch.condition, (unsigned long)nodes[i].data.branch.resultIfTrue, (unsigned long)nodes[i].data.branch.resultIfFalse);
					break;
			}
		}
	}
	L1FIRGeneratorDelete(generator);*/
	
	L1ParserDelete(parser);
	
	L1ArrayDeinitialize(& tokenArray);
	L1RegionDelete(tokenDataRegion);
	return 0;
}
