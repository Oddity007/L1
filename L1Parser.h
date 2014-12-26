#ifndef L1Parser_h
#define L1Parser_h

#include "L1Array.h"
#include "L1Lexer.h"

enum L1ParserASTNodeType
{
	L1ParserASTNodeTypeIdentifier,
	L1ParserASTNodeTypeString,
	L1ParserASTNodeTypeNatural,
	
	L1ParserASTNodeTypeEvaluateArgument,
	
	L1ParserASTNodeTypeOverload,
	
	L1ParserASTNodeTypeAssign,
	//L1ParserASTNodeTypeLet,
	L1ParserASTNodeTypeAnnotate,
	
	L1ParserASTNodeTypeLambda,
	L1ParserASTNodeTypePi,
	L1ParserASTNodeTypeADT,
	
	L1ParserASTNodeTypeUnderscore,
	
	L1ParserASTNodeTypeCall,
	
	L1ParserASTNodeTypeSelf,
	L1ParserASTNodeTypeUniverse,

	L1ParserASTNodeTypeArgumentList,
	L1ParserASTNodeTypeConstructorList,
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
			size_t tokenIndex;
		}identifier;
		struct
		{
			size_t tokenIndex;
		}string;
		struct
		{
			size_t tokenIndex;
		}natural;
		struct
		{
			size_t expression;
		}evaluateArgument;
		struct
		{
			size_t first;
			size_t second;
		}overload;
		struct
		{
			size_t destination;
			size_t source;
			size_t followingContext;
		}assign;
		struct
		{
			size_t value;
			size_t type;
		}annotate;
		struct
		{
			size_t argument;
			size_t result;
		}lambda;
		struct
		{
			size_t argument;
			size_t result;
		}pi;
		struct
		{
			size_t constructorList;
		}adt;
		struct
		{
			size_t callee;
			size_t argument;
		}call;
		/*struct
		{
			size_t destination;
			size_t argumentList;
			size_t source;
			size_t followingContext;
		}let;*/
		struct
		{
			size_t previousArgumentList;
			size_t argument;
		}argumentList;
		struct
		{
			size_t previousConstructorList;
			size_t constructorName;
			size_t argumentList;
		}constructorList;
		struct
		{
			size_t level;
		}universe;
	}data;
};

typedef struct L1Parser L1Parser;
struct L1Parser
{
	L1Array
		symbolStack,
		syntaxTreeNodes,
		locationStack;
	size_t currentTokenIndex;
	size_t rootASTNode;
};

enum L1ParserStatusType
{
	L1ParserStatusTypeNone,
	L1ParserStatusTypeDone,
	L1ParserStatusTypeUnexpectedSymbol,
};

typedef enum L1ParserStatusType L1ParserStatusType;

void L1ParserInitialize(L1Parser* self);
void L1ParserDeinitialize(L1Parser* self);

L1ParserStatusType L1ParserParse(L1Parser* self, L1LexerTokenType tokenType, const char* tokenString, size_t tokenStringLength);

const L1ParserASTNode* L1ParserGetASTNodes(L1Parser* self);
size_t L1ParserGetASTNodeCount(L1Parser* self);
size_t L1ParserGetRootASTNodeIndex(L1Parser* self);

/*size_t L1ParserGetASTNodeCount(L1Parser* self);
const unsigned char* L1ParserGetASTNodeTypes(L1Parser* self);
const size_t* L1ParserGetASTNodeData(L1Parser* self);*/

#endif
