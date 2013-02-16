#include "L1Parser.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <iso646.h>
#include <assert.h>
#include <string.h>

/*
Strength of binding increases going downwards

assignment = identifier assign expression
expression = identifier yield expression
expression = assignment terminal expression
expression = expression questionmark expression
expression = expression doublequestionmark expression
expression = expression add expression
expression = expression sub expression
expression = expression div expression
expression = expression mul expression
expression = expression expression
expression = oparen expression cparen
expression = identifier
expression = literal
*/

typedef struct Page Page;
struct Page
{
	Page* next;
	size_t size, used;
	void* data;
};

static Page* PageNew(size_t size)
{
	Page* self = calloc(1, sizeof(Page));
	self->size = size;
	self->data = calloc(1, size);
	return self;
}

static void* PageAllocate(Page* self, size_t amount)
{
	size_t possiblyUsed = self->used + amount;
	if(possiblyUsed <= self->size)
	{
		void* location = self->data + self->used;
		self->used = possiblyUsed;
		return location;
	}
	size_t greatestSize = (self->size > amount) ? self->size : amount;
	self->next = PageNew(greatestSize);
	return PageAllocate(self->next, amount);
}

void PageDelete(Page* self)
{
	if(not self) return;
	free(self->data);
	PageDelete(self->next);
}

struct L1Parser
{
	Page* page;
	/*L1ParserASTNode* nodes;
	size_t nodesAllocated, nodesUsed;
	L1ParserBlob* blobs;
	size_t blobsUsed, blobsAllocated;*/
};

typedef struct LexerToken LexerToken;
struct LexerToken
{
	L1LexerTokenType type;
	char* data;
};

static L1ParserASTNode* L1ParserCreateASTNode(L1Parser* self, L1ParserASTNodeType type)
{
	/*self->nodesUsed++;
	if(self->nodesUsed > self->nodesAllocated)
	{
		self->nodesAllocated = self->nodesUsed * 2;
		self->nodes = realloc(self->nodes, sizeof(L1ParserASTNode) * self->nodesAllocated);
	}
	L1ParserASTNode* node = self->nodes + self->nodesUsed - 1;
	node = calloc(1, sizeof(L1ParserASTNode));*/
	L1ParserASTNode* node = PageAllocate(self->page, sizeof(L1ParserASTNode));
	//memset(node, 0, sizeof(L1ParserASTNode));
	node->type = type;
	return node;
}

static L1ParserBlob* L1ParserCreateBlob(L1Parser* self, uint64_t byteCount, const uint8_t* bytes)
{
	/*self->blobsUsed++;
	if(self->blobsUsed > self->blobsAllocated)
	{
		self->blobsAllocated = self->blobsUsed * 2;
		self->blobs = realloc(self->blobs, sizeof(L1ParserBlob) * self->blobsAllocated);
	}
	L1ParserBlob* blob = self->blobs + self->blobsUsed - 1;
	blob = calloc(1, sizeof(L1ParserBlob));
	memset(blob, 0, sizeof(L1ParserBlob));*/
	L1ParserBlob* blob = PageAllocate(self->page, sizeof(L1ParserBlob));
	blob->byteCount = byteCount;
	blob->bytes = memcpy(malloc(byteCount), bytes, byteCount);
	return blob;
}

static L1ParserBlob* L1ParserCreateBlobFromCString(L1Parser* self, const char* string)
{
	return L1ParserCreateBlob(self, strlen(string)+1, (const uint8_t*) string);
}

//Utility Functions

static char* CloneString(const char* string, size_t size)
{
	if(not string) return NULL;
	if(size == 0) size = strlen(string) + 1;
	return (char*) memcpy(malloc(size), string, size);
}

/*static const char* ParserASTNodeTypeAsString(L1ParserASTNodeType self)
{
	switch (self)
	{
		case L1ParserASTNodeTypeAddition: return "addition";
		case L1ParserASTNodeTypeDivision: return "division";
		case L1ParserASTNodeTypeGreater: return "greater";
		case L1ParserASTNodeTypeLesser: return "lesser";
		case L1ParserASTNodeTypeMultiplication: return "multiplication";
		case L1ParserASTNodeTypeSubtraction: return "subtraction";
		case L1ParserASTNodeTypeElipsis: return "elipsis";
		case L1ParserASTNodeTypeAssignment: return "assignment";
		case L1ParserASTNodeTypeBranch: return "branch";
		case L1ParserASTNodeTypeFunction: return "function";
		case L1ParserASTNodeTypeList: return "list ";
		case L1ParserASTNodeTypeIdentifier: return "identifier";
		case L1ParserASTNodeTypeNone: return "none";
		case L1ParserASTNodeTypeString: return "string";
		case L1ParserASTNodeTypeCompoundExpression: return "compound expression";
		case L1ParserASTNodeTypeNumber: return "number";
		case L1ParserASTNodeTypeEqual: return "equal";
		case L1ParserASTNodeTypeCall: return "call";
	}
	return NULL;
}*/

//Parsing Functions

/*static void DeleteNode(const L1ParserASTNode* node)
{
	static int indentation = 0;
	if(not node) return;
	for (int i=0; i<indentation; i++) putc('\t', stdout);
	printf("%s\n", ParserASTNodeTypeAsString(node->type));
	indentation++;
	switch (node->type)
	{
		case L1ParserASTNodeTypeAddition:
		case L1ParserASTNodeTypeDivision:
		case L1ParserASTNodeTypeEqual:
		case L1ParserASTNodeTypeGreater:
		case L1ParserASTNodeTypeLesser:
		case L1ParserASTNodeTypeMultiplication:
		case L1ParserASTNodeTypeSubtraction:
			DeleteNode(node->data.operation.left);
			DeleteNode(node->data.operation.right);
			break;
		case L1ParserASTNodeTypeElipsis:
			break;
		case L1ParserASTNodeTypeAssignment:
			DeleteNode(node->data.assignment.destination);
			DeleteNode(node->data.assignment.source);
			break;
		case L1ParserASTNodeTypeCompoundExpression:
			DeleteNode(node->data.compoundExpression.head);
			DeleteNode(node->data.compoundExpression.tail);
			break;
		case L1ParserASTNodeTypeFunction:
			DeleteNode(node->data.function.argument);
			DeleteNode(node->data.function.expression);
			break;
		case L1ParserASTNodeTypeIdentifier:
			for (int i=0; i<indentation; i++) putc('\t', stdout);
			printf("value: %s\n", node->data.identifier.utf8Bytes);
			free(node->data.identifier.utf8Bytes);
			break;
		case L1ParserASTNodeTypeList:
			DeleteNode(node->data.list.head);
			DeleteNode(node->data.list.tail);
			break;
		case L1ParserASTNodeTypeNumber:
			for (int i=0; i<indentation; i++) putc('\t', stdout);
			printf("value: %lu / %lu\n", (long unsigned) node->data.number.numerator, (long unsigned) node->data.number.denominator);
			break;
		case L1ParserASTNodeTypeString:
			for (int i=0; i<indentation; i++) putc('\t', stdout);
			printf("value: %s\n", node->data.string.utf8Bytes);
			free(node->data.string.utf8Bytes);
			break;
		case L1ParserASTNodeTypeBranch:
			DeleteNode(node->data.branch.condition);
			DeleteNode(node->data.branch.result);
			break;
		case L1ParserASTNodeTypeNone:
			break;
		case L1ParserASTNodeTypeCall:
			DeleteNode(node->data.call.callee);
			DeleteNode(node->data.call.argument);
			break;
	}
	indentation--;
	//DeleteNode(node->next);
	free((void*)node);
}
*/

static bool SplitTokenStreamWithMultipleDividers(L1Parser* self, const LexerToken* tokens, uint32_t tokenCount, L1LexerTokenType* dividerTokenTypes, uint32_t dividerCount, uint32_t* leftStartOut, uint32_t* leftCountOut, uint32_t* rightStartOut, uint32_t* rightCountOut, L1LexerTokenType* tokenTypeSplitOnOut)
{
	uint32_t parenthesisIndent = 0;
	bool wasSplit = false;
	for (uint32_t i = 0; i < tokenCount; i++)
	{
		L1LexerTokenType tokenType = tokens[i].type;
		switch (tokenType)
		{
			case L1LexerTokenTypeOpeningParenthesis:
				parenthesisIndent++;
				break;
			case L1LexerTokenTypeClosingParenthesis:
				assert(parenthesisIndent);
				parenthesisIndent--;
				break;
			default:
				if(parenthesisIndent) break;
				{
					bool hasFoundDivider = false;
					for (uint32_t i = 0; i < dividerCount; i++)
					{
						if(tokenType == dividerTokenTypes[i])
						{
							hasFoundDivider = true;
							break;
						}
					}
					if(not wasSplit and hasFoundDivider)
					{
						if(tokenTypeSplitOnOut) *tokenTypeSplitOnOut = tokenType;
						*leftStartOut = 0;
						*leftCountOut = i - 0;
						*rightStartOut = i + 1;
						*rightCountOut = tokenCount - (i + 1);
						wasSplit = true;
					}
				}
				break;
		}
	}
	return wasSplit;
}

static bool SplitTokenStream(L1Parser* self, const LexerToken* tokens, uint32_t tokenCount, L1LexerTokenType dividerTokenType, uint32_t* leftStartOut, uint32_t* leftCountOut, uint32_t* rightStartOut, uint32_t* rightCountOut)
{
	return SplitTokenStreamWithMultipleDividers(self, tokens, tokenCount, & dividerTokenType, 1, leftStartOut, leftCountOut, rightStartOut, rightCountOut, NULL);
}

static L1ParserASTNode* ParseExpression(L1Parser* self, const LexerToken* tokens, uint32_t tokenCount, uint32_t* tokensReadOut, bool isChained)
{
	if(tokenCount == 0) return NULL;
		
	uint32_t leftStart = 0;
	uint32_t leftCount = 0;
	uint32_t rightStart = 0;
	uint32_t rightCount = 0;
	uint32_t nextStart = 0;
	uint32_t nextCount = 0;
	uint32_t tokensRead = 0;
		
	L1ParserASTNode* node = NULL;
	L1LexerTokenType tokenTypeSplitOn;
	
	if(SplitTokenStream(self, tokens, tokenCount, L1LexerTokenTypeTerminal, & leftStart, & leftCount, & rightStart, & rightCount))
	{
		node = L1ParserCreateASTNode(self, L1ParserASTNodeTypeCompoundExpression);
		node->data.compoundExpression.head = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.compoundExpression.tail = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		tokensRead += tokenCount;
	}
	else if(SplitTokenStream(self, tokens, tokenCount, L1LexerTokenTypeAssign, & leftStart, & leftCount, & rightStart, & rightCount))
	{
		node = L1ParserCreateASTNode(self, L1ParserASTNodeTypeAssignment);
		node->data.assignment.destination = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.assignment.source = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		tokensRead += tokenCount;
	}
	else if(SplitTokenStream(self, tokens, tokenCount, L1LexerTokenTypeQuestionMark, & leftStart, & leftCount, & rightStart, & rightCount))
	{
		node = L1ParserCreateASTNode(self, L1ParserASTNodeTypeBranch);
		node->data.branch.condition = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.branch.result = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		tokensRead += tokenCount;
	}
	/*else if(SplitTokenStream(self, tokens, tokenCount, L1LexerTokenTypeComma, & leftStart, & leftCount, & rightStart, & rightCount))
	{
		node = calloc(1, sizeof(L1ParserASTNode));
		node->type = L1ParserASTNodeTypeList;
		node->data.list.head = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.list.tail = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		//abort();
		tokensRead += tokenCount;
	}*/
	else if(SplitTokenStream(self, tokens, tokenCount, L1LexerTokenTypeYield, & leftStart, & leftCount, & rightStart, & rightCount))
	{
		node = L1ParserCreateASTNode(self, L1ParserASTNodeTypeFunction);
		node->data.functionLiteral.argument = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.functionLiteral.expression = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		tokensRead += tokenCount;
	}
	/*else if(SplitTokenStreamWithMultipleDividers(self, tokens, tokenCount, ((L1LexerTokenType[]){L1LexerTokenTypeGreater, L1LexerTokenTypeLesser, L1LexerTokenTypeEqual}), 3, & leftStart, &leftCount, &rightStart, & rightCount, & tokenTypeSplitOn))
	{
		node = calloc(1, sizeof(L1ParserASTNode));
		L1ParserASTNodeType type;
		switch (tokenTypeSplitOn)
		{
			case L1LexerTokenTypeGreater: type = L1ParserASTNodeTypeGreater; break;
			case L1LexerTokenTypeEqual: type = L1ParserASTNodeTypeEqual; break;
			case L1LexerTokenTypeLesser: type = L1ParserASTNodeTypeLesser; break;
			default: abort(); break;
		}
		node->type = type;
		node->data.operation.left = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.operation.right = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		tokensRead += tokenCount;
	}*/
	/*else if(SplitTokenStreamWithMultipleDividers(self, tokens, tokenCount, ((L1LexerTokenType[]){L1LexerTokenTypeAddition, L1LexerTokenTypeSubtraction}), 2, & leftStart, &leftCount, &rightStart, & rightCount, & tokenTypeSplitOn))
	{
		node = calloc(1, sizeof(L1ParserASTNode));
		L1ParserASTNodeType type;
		switch (tokenTypeSplitOn)
		{
			case L1LexerTokenTypeAddition: type = L1ParserASTNodeTypeAddition; break;
			case L1LexerTokenTypeSubtraction: type = L1ParserASTNodeTypeSubtraction; break;
			default: abort();
		}
		node->type = type;
		node->data.operation.left = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.operation.right = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		tokensRead += tokenCount;
	}*/
	/*else if(SplitTokenStreamWithMultipleDividers(self, tokens, tokenCount, ((L1LexerTokenType[]){L1LexerTokenTypeMultiplication, L1LexerTokenTypeDivision}), 2, & leftStart, &leftCount, &rightStart, & rightCount, & tokenTypeSplitOn))
	{
		node = calloc(1, sizeof(L1ParserASTNode));
		L1ParserASTNodeType type;
		switch (tokenTypeSplitOn)
		{
			case L1LexerTokenTypeMultiplication: type = L1ParserASTNodeTypeMultiplication; break;
			case L1LexerTokenTypeDivision: type = L1ParserASTNodeTypeDivision; break;
			default: abort();
		}
		node->type = type;
		node->data.operation.left = ParseExpression(self, tokens + leftStart, leftCount, NULL, false);
		node->data.operation.right = ParseExpression(self, tokens + rightStart, rightCount, NULL, false);
		tokensRead += tokenCount;
	}*/
	else if(tokens[0].type == L1LexerTokenTypeIdentifier)
	{
		node = L1ParserCreateASTNode(self, L1ParserASTNodeTypeIdentifier);
		{
			node->data.identifier.name = L1ParserCreateBlobFromCString(self, tokens[0].data);
		}
		tokensRead++;
	}
	else if(tokens[0].type == L1LexerTokenTypeString)
	{
		node = L1ParserCreateASTNode(self, L1ParserASTNodeTypeString);
		{
			node->data.string.data = L1ParserCreateBlobFromCString(self, tokens[0].data);
		}
		tokensRead++;
	}
	else if(tokens[0].type == L1LexerTokenTypeNumber)
	{
		node = L1ParserCreateASTNode(self, L1ParserASTNodeTypeNumber);
		{
			node->data.number.base10Digits = L1ParserCreateBlobFromCString(self, tokens[0].data);
		}
		tokensRead++;
	}
	/*else if(tokens[0].type == L1LexerTokenTypeElipsis)
	{
		node = calloc(1, sizeof(L1ParserASTNode));
		node->type = L1ParserASTNodeTypeElipsis;
		tokensRead++;
	}*/
	else if(tokens[0].type == L1LexerTokenTypeOpeningParenthesis)
	{
		uint32_t depth = 1;
		uint32_t count = 0;
		for (uint32_t i = 1; i<tokenCount; i++)
		{
			if(tokens[i].type == L1LexerTokenTypeOpeningParenthesis)
			{
				depth++;
			}
			else if(tokens[i].type == L1LexerTokenTypeClosingParenthesis)
			{
				assert(depth);
				depth--;
				if(depth == 0) break;
			}
			count++;
		}
		assert(depth == 0);
		tokensRead = 1 + 1 + count;
		//printf("%i\n", (int)tokenCount - tokensRead);
		node = ParseExpression(self, tokens + 1, count, NULL, false);
	}
	
	if(not isChained)
	{
		L1ParserASTNode* callee = node;
		L1ParserASTNode* argument = NULL;
		do
		{
			argument = ParseExpression(self, tokens + tokensRead, tokenCount - tokensRead, & tokensRead, true);
			if(argument)
			{
				L1ParserASTNode* callNode = L1ParserCreateASTNode(self, L1ParserASTNodeTypeCall);
				callNode->data.call.callee = callee;
				callNode->data.call.argument = argument;
				callee = callNode;
			}
		}
		while(argument);
		node = callee;
	}
	
	if(tokensReadOut) *tokensReadOut += tokensRead;
	
	return node;
}

//Exported Functions Functions

L1Parser* L1ParserNew(void)
{
	L1Parser* self = calloc(1, sizeof(L1Parser));
	const static size_t defaultPageSize = 1024;
	self->page = PageNew(defaultPageSize);
	return self;
}

const L1ParserASTNode* L1ParserParse(L1Parser* self, L1Lexer* lexer)
{
	//assert(not self->nodes);
	L1ParserASTNode* node = NULL;
	LexerToken* tokens = NULL;
	uint32_t tokenCountAllocated = 0;
	uint32_t tokenCount = 0;
	//puts("Lexing");
	do
	{
		L1LexerError error = L1LexerLex(lexer);
		switch (error)
		{
			case L1LexerErrorNone:
				tokenCount++;
				if(tokenCountAllocated < tokenCount)
				{
					tokenCountAllocated = tokenCount * 2;
					tokens = realloc(tokens, sizeof(LexerToken) * tokenCountAllocated);
				}
				tokens[tokenCount-1].data = CloneString((char*) lexer->buffer, lexer->bufferUsed);
				tokens[tokenCount-1].type = lexer->lastTokenType;
				break;
			case L1LexerErrorUnknown:
				//fprintf(stderr, "error in lexing on line #%i, column #%i", (int)lexer->currentLineNumber, (int)lexer->currentCharacterOfLineNumber);
				goto end;
		}
		//printf("Token type \"%s\" with data of \"%s\"\n", L1LexerTokenTypeAsString(lexer->lastTokenType), lexer->buffer);
	}while (lexer->lastTokenType not_eq L1LexerTokenTypeDone);
	
	//puts("Parsing");
	
	/*for (uint32_t i = 0; i<tokenCount; i++)
	{
		puts(L1LexerTokenTypeAsString(tokens[i].type));
	}*/
	
	uint32_t tokensRead = 0;
	node = ParseExpression(self, tokens, tokenCount - 1, & tokensRead, false);
	
	for (uint32_t i = 0; i < tokenCount; i++) free(tokens[i].data);
	free(tokens);
	tokens = NULL;
	
	end:
	
	return node;
}

void L1ParserDelete(L1Parser* self)
{
	PageDelete(self->page);
	//DeleteNode(self->rootNode);
}

void L1ParserPrintASTNode(L1Parser* self, const L1ParserASTNode* node, uint64_t indentation)
{
	if(not node) return;
	for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
	switch (node->type)
	{
		case L1ParserASTNodeTypeNumber:
			printf("(Number) %s\n", node->data.number.base10Digits->bytes);
			break;
		case L1ParserASTNodeTypeString:
			printf("(String) \"%s\"\n", node->data.string.data->bytes);
			break;
		case L1ParserASTNodeTypeFunction:
			printf("(Function)\n");
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Argument:\n");
			L1ParserPrintASTNode(self, node->data.functionLiteral.argument, indentation+1);
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Expression:\n");
			L1ParserPrintASTNode(self, node->data.functionLiteral.expression, indentation+1);
			break;
		case L1ParserASTNodeTypeIdentifier:
			printf("(Identifier) %s\n", node->data.identifier.name->bytes);
			break;
		case L1ParserASTNodeTypeAssignment:
			printf("(Assignment)\n");
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Destination:\n");
			L1ParserPrintASTNode(self, node->data.assignment.destination, indentation+1);
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Source:\n");
			L1ParserPrintASTNode(self, node->data.assignment.source, indentation+1);
			break;
		case L1ParserASTNodeTypeCompoundExpression:
			printf("(Compound Expression)\n");
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Head:\n");
			L1ParserPrintASTNode(self, node->data.compoundExpression.head, indentation+1);
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Tail:\n");
			L1ParserPrintASTNode(self, node->data.compoundExpression.tail, indentation+1);
			break;
		case L1ParserASTNodeTypeBranch:
			printf("(Branch)\n");
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Condition:\n");
			L1ParserPrintASTNode(self, node->data.branch.condition, indentation+1);
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Result:\n");
			L1ParserPrintASTNode(self, node->data.branch.result, indentation+1);
			break;
		case L1ParserASTNodeTypeCall:
			printf("(Call)\n");
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Callee:\n");
			L1ParserPrintASTNode(self, node->data.call.callee, indentation+1);
			for (uint64_t i = 0; i < indentation; i++) putc('\t', stdout);
			printf("+ Argument:\n");
			L1ParserPrintASTNode(self, node->data.call.argument, indentation+1);
			break;
		default:
			printf("(Unknown)\n");
			break;
	}
}
