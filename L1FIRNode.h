#ifndef L1FIRNode_h
#define L1FIRNode_h

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

enum L1FIRNodeType
{
	L1FIRNodeTypeNoOperation,

	L1FIRNodeTypeLoadUndefined,
	L1FIRNodeTypeLoadInteger,
	L1FIRNodeTypeLoadString,
	
	L1FIRNodeTypeClosure,
	L1FIRNodeTypeCall,
	
	L1FIRNodeTypeLet,
	
	L1FIRNodeTypeSplitList,
	
	L1FIRNodeTypeNewList,
	L1FIRNodeTypeConsList,
	//L1FIRNodeTypeNewListAppend,
	
	/*L1FIRNodeTypeNewTable,
	L1FIRNodeTypeSetTableElement,
	L1FIRNodeTypeGetTableElement,
	
	L1FIRNodeTypeConsPrepend,
	L1FIRNodeTypeConsAppend,
	L1FIRNodeTypeHead,
	L1FIRNodeTypeTail,*/
	
	L1FIRNodeTypeBranch
};
typedef enum L1FIRNodeType L1FIRNodeType;

typedef struct L1FIRNode L1FIRNode;

struct L1FIRNode
{
	L1FIRNodeType type;
	union
	{
		struct
		{
			uint64_t destination;
		}
			loadUndefined;
		struct
		{
			uint64_t destination;
			uint64_t digitCount;
			const uint8_t* digits;
		}
			loadInteger;
		struct
		{
			uint64_t destination;
			uint64_t byteCount;
			const uint8_t* bytes;
		}
			loadString;
		struct
		{
			uint64_t destination;
			const uint64_t* arguments;
			uint64_t argumentCount;
			uint64_t result;
		}
			closure;
		struct
		{
			uint64_t destination;
			uint64_t closure;
			const uint64_t* arguments;
			uint64_t argumentCount;
		}
			call;
		struct
		{
			uint64_t destination;
			uint64_t source;
		}
			let;
		/*struct
		{
			uint64_t destination;
			uint64_t head;
			uint64_t tail;
		}
			consPrepend;
		struct
		{
			uint64_t destination;
			uint64_t head;
			uint64_t tail;
		}
			consAppend;
		struct
		{
			uint64_t destination;
			uint64_t list;
		}
			head;
		struct
		{
			uint64_t destination;
			uint64_t list;
		}
			tail;
		struct
		{
			uint64_t destination;
		}
			newTable;
		struct
		{
			uint64_t destination;
			uint64_t table;
			uint64_t index;
			uint64_t value;
		}
			setTableElement;
		struct
		{
			uint64_t destination;
			uint64_t table;
			uint64_t index;
		}
			getTableElement;*/
		/*struct
		{
			uint64_t destination;
		}
			newList;
		struct
		{
			uint64_t destination;
			uint64_t head;
			uint64_t tail;
		}
			newListAppend;*/
		struct
		{
			uint64_t destination;
			uint64_t head;
			uint64_t tail;
		}
			newList;
		struct
		{
			uint64_t destination;
			uint64_t head;
			uint64_t tail;
		}
			consList;
		struct
		{
			uint64_t destination;
			uint64_t list;
			uint64_t head;
			uint64_t tail;
		}
			splitList;
		struct
		{
			uint64_t destination;
			uint64_t condition;
			uint64_t resultIfTrue;
			uint64_t resultIfFalse;
		}
			branch;
	}
		data;
};

#ifdef __cplusplus
}
//extern "C"
#endif

#endif
