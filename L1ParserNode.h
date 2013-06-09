#ifndef L1ParserNode_h
#define L1ParserNode_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <stdint.h>
typedef enum
{
	L1ParserNodeTypeIdentifier,
	L1ParserNodeTypeNumber,
	L1ParserNodeTypeString,
	L1ParserNodeTypeBranch,
	L1ParserNodeTypeAssignment,
	L1ParserNodeTypeCall,
	L1ParserNodeTypeList,
	L1ParserNodeTypeEllipsis
}L1ParserNodeType;

typedef struct L1ParserNode L1ParserNode;

struct L1ParserNode
{
	L1ParserNodeType type;
	union
	{
		struct
		{
			const uint8_t* characters;
			uint64_t characterCount;
		}identifier;
		struct
		{
			const uint8_t* characters;
			uint64_t characterCount;
		}number;
		struct
		{
			const uint8_t* characters;
			uint64_t characterCount;
		}string;
		struct
		{
			L1ParserNode* condition;
			L1ParserNode* resultIfTrue;
			L1ParserNode* resultIfFalse;
		}branch;
		struct
		{
			L1ParserNode* destination;
			L1ParserNode* source;
			L1ParserNode* context;
		}assignment;
		struct
		{
			L1ParserNode* callee;
			L1ParserNode* argument;
		}call;
		struct
		{
			L1ParserNode* head;
			L1ParserNode* tail;
		}list;
		struct
		{
			L1ParserNode* trailingExpression;
		}ellipsis;
	}data;
};

L1ParserNode* L1ParserNodeNew(L1ParserNodeType type);
void L1ParserNodePrint(const L1ParserNode* self, uint32_t indent);
void L1ParserNodeDelete(L1ParserNode* self);

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
