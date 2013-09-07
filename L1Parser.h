#ifndef L1Parser_h
#define L1Parser_h

#include <stdint.h>
#include "L1Lexer.h"

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
	L1ParserASTNodeTypeBranch,
	L1ParserASTNodeTypeList,
};

typedef enum L1ParserASTNodeType L1ParserASTNodeType;

typedef struct L1ParserASTNode L1ParserASTNode;
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
			const L1ParserASTNode** arguments;
			uint64_t argumentCount;
		}call;
		struct
		{
			const L1ParserASTNode* destination;
			const L1ParserASTNode** arguments;
			uint64_t argumentCount;
			const L1ParserASTNode* source;
			const L1ParserASTNode* followingContext;
		}assignment;
		struct
		{
			const L1ParserASTNode* condition;
			const L1ParserASTNode* resultIfTrue;
			const L1ParserASTNode* resultIfFalse;
		}branch;
		struct
		{
			L1ParserASTNode* elements;
			uint64_t elementCount;
		}list;
	}data;
};

typedef struct L1ParserLexedToken L1ParserLexedToken;
struct L1ParserLexedToken
{
	L1LexerTokenType type;
	const uint8_t* bytes;
	uint64_t byteCount;
};

L1Parser* L1ParserNew(const L1ParserLexedToken* tokens, uint64_t tokenCount);
const L1ParserASTNode* L1ParserGetRootASTNode(L1Parser* self);
void L1ParserDelete(L1Parser* self);

#ifdef __cplusplus
}//extern "C"
#endif

#endif
