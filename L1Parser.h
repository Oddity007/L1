#ifndef L1Parser_h
#define L1Parser_h

#ifdef __cplusplus
extern "C"
{
#endif

#include "L1Lexer.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct L1Parser L1Parser;
typedef enum
{
	L1ParserASTNodeTypeNone,
	L1ParserASTNodeTypeNumber,
	L1ParserASTNodeTypeString,
	L1ParserASTNodeTypeFunction,
//	L1ParserASTNodeTypeList,
	L1ParserASTNodeTypeIdentifier,
//	L1ParserASTNodeTypeElipsis,
	L1ParserASTNodeTypeAssignment,
	L1ParserASTNodeTypeCompoundExpression,
	L1ParserASTNodeTypeBranch,
//	L1ParserASTNodeTypeAddition,
//	L1ParserASTNodeTypeSubtraction,
//	L1ParserASTNodeTypeMultiplication,
//	L1ParserASTNodeTypeDivision,
//	L1ParserASTNodeTypeGreater,
//	L1ParserASTNodeTypeLesser,
//	L1ParserASTNodeTypeEqual,
	L1ParserASTNodeTypeCall
}L1ParserASTNodeType;

typedef struct L1ParserBlob L1ParserBlob;
struct L1ParserBlob
{
	uint64_t byteCount;
	const uint8_t* bytes;
};

typedef struct L1ParserASTNode L1ParserASTNode;

struct L1ParserASTNode
{
	L1ParserASTNodeType type;
	union
	{
		struct
		{
			const L1ParserBlob* base10Digits;
		}number;
		struct
		{
			const L1ParserBlob* data;
		}string;
		struct
		{
			const L1ParserASTNode* argument;
			const L1ParserASTNode* expression;
		}functionLiteral;
		/*struct
		{
			const L1ParserASTNode* arguments;
			uint32_t argumentCount;
			const L1ParserASTNode* expression;
		}functionDefinition;
		struct
		{
			const L1ParserASTNode* predicateExpression;
			const L1ParserASTNode* arguments;
			uint32_t argumentCount;
			const L1ParserASTNode* expression;
		}functionPredicatedDefinition;*/
		struct
		{
			const L1ParserBlob* name;
		}identifier;
		struct
		{
			const L1ParserASTNode* destination;
			const L1ParserASTNode* source;
		}assignment;
		struct
		{
			const L1ParserASTNode* head;
			const L1ParserASTNode* tail;
		}compoundExpression;
		struct
		{
			const L1ParserASTNode* condition;
			const L1ParserASTNode* result;
		}branch;
		struct
		{
			const L1ParserASTNode* callee;
			const L1ParserASTNode* argument;
		}call;
	}data;
};

L1Parser* L1ParserNew(void);
const L1ParserASTNode* L1ParserParse(L1Parser* self, L1Lexer* lexer);
void L1ParserDelete(L1Parser* self);

void L1ParserPrintASTNode(L1Parser* self, const L1ParserASTNode* node, uint64_t indentation);

bool L1ParserASTNodeVerifyIntegrity(const L1ParserASTNode* self);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
