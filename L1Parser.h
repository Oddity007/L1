#ifndef L1Parser_h
#define L1Parser_h

#include <stdint.h>
#include "L1Lexer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct L1Parser L1Parser;

enum L1ParserASTNodeType
{
	L1ParserASTNodeTypeNatural,
	L1ParserASTNodeTypeString,
	L1ParserASTNodeTypeIdentifier,
	L1ParserASTNodeTypeCall,
	L1ParserASTNodeTypeAssignment,
	L1ParserASTNodeTypeList,
	L1ParserASTNodeTypeAnonymousFunction,
	L1ParserASTNodeTypeOption,
	L1ParserASTNodeTypeConstraint,
	L1ParserASTNodeTypeAny,
	L1ParserASTNodeTypeInlineConstraint,
	L1ParserASTNodeTypeMetasymbol
};

typedef enum L1ParserASTNodeType L1ParserASTNodeType;
typedef struct L1ParserASTNode L1ParserASTNode;
typedef struct L1ParserASTNodeLinkedList L1ParserASTNodeLinkedList;

struct L1ParserASTNodeLinkedList
{
	const L1ParserASTNode* head;
	const L1ParserASTNodeLinkedList* tail;
};

struct L1ParserASTNode
{
	L1ParserASTNodeType type;
	union
	{
		struct
		{
			const uint8_t* bytes;
			uint64_t byteCount;
		}natural;
		struct
		{
			const uint8_t* bytes;
			uint64_t byteCount;
		}string;
		struct
		{
			const uint8_t* bytes;
			uint64_t byteCount;
		}identifier;
		struct
		{
			const L1ParserASTNode* callee;
			const L1ParserASTNodeLinkedList* arguments;
		}call;
		struct
		{
			const L1ParserASTNode* destination;
			const L1ParserASTNodeLinkedList* arguments;
			const L1ParserASTNode* source;
			const L1ParserASTNode* followingContext;
			bool isMeta;
		}assignment;
		struct
		{
			const L1ParserASTNodeLinkedList* elements;
			const L1ParserASTNode* sublist;
		}list;
		struct
		{
			const L1ParserASTNodeLinkedList* arguments;
			const L1ParserASTNode* source;
		}anonymousFunction;
		struct
		{
			const L1ParserASTNode* construction;
			const L1ParserASTNode* defaultConstruction;
		}option;
		struct
		{
			const L1ParserASTNode* expression;
			const L1ParserASTNode* constraint;
			const L1ParserASTNode* followingContext;
		}constraint;
		struct
		{
			const L1ParserASTNode* source;
		}any;
		struct
		{
			const L1ParserASTNode* expression;
			const L1ParserASTNode* constraint;
		}inlineConstraint;
		struct
		{
			const L1ParserASTNode* source;
		}metasymbol;
	}data;
};

enum L1ParserErrorType
{
	L1ParserErrorTypeNone,
	L1ParserErrorTypeUnknown,
	L1ParserErrorTypeUnexpectedToken,
};

typedef enum L1ParserErrorType L1ParserErrorType;

typedef struct L1ParserLexedToken L1ParserLexedToken;
struct L1ParserLexedToken
{
	L1LexerTokenType type;
	const uint8_t* bytes;
	uint64_t byteCount;
};

L1Parser* L1ParserNew(const L1ParserLexedToken* tokens, uint64_t tokenCount);
L1ParserErrorType L1ParserGetError(L1Parser* self);
const L1ParserASTNode* L1ParserGetRootASTNode(L1Parser* self);
void L1ParserDelete(L1Parser* self);

#ifdef __cplusplus
}//extern "C"
#endif

#endif
