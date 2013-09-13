#include "L1Lexer.h"
#include <iso646.h>
#include <stdio.h>
#include "L1Parser.h"
#include "L1Array.h"
#include "L1Region.h"
#include <string.h>

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
	L1Lexer* lexer = L1LexerNew((const uint8_t*)"f a = (a 1 2 ? 1; 0); [c, d] = z \"a\"; f");
	L1Array tokenArray;
	L1ArrayInitialize(& tokenArray);
	L1Region* tokenDataRegion = L1RegionNew();
	L1ParserLexedToken token;
	fputs("Lexing:\n", stdout);
	do
	{
		L1LexerLexNext(lexer, & token.type);
		const void* bytes = L1LexerGetLastTokenBytes(lexer, & token.byteCount);
		token.bytes = memcpy(L1RegionAllocate(tokenDataRegion, token.byteCount), bytes, token.byteCount);
		//fwrite(bytes, token.byteCount, 1, stdout);
		//fputc('\n', stdout);
		//token.byteCount = 0;
		//token.bytes = NULL;
		L1ArrayAppend(& tokenArray, & token, sizeof(token));
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
	L1ParserDelete(parser);
	
	L1ArrayDeinitialize(& tokenArray);
	L1RegionDelete(tokenDataRegion);
	return 0;
}
