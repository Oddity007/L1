#include "L1ParserASTNodePostProcessing.h"
#include <iso646.h>

static bool L1ParserASTNodeVerifyIntegrity_internal(const L1ParserASTNode* self)
{
	switch (self->type)
	{
		case L1ParserASTNodeTypeNone:
				return false;
			break;
		case L1ParserASTNodeTypeNumber:
		case L1ParserASTNodeTypeString:
				return true;
			break;
		case L1ParserASTNodeTypeIdentifier:
			return true;
			break;
		case L1ParserASTNodeTypeCompoundExpression:
				if(not (self->data.compoundExpression.head and self->data.compoundExpression.tail)) return false;
				if(not (self->data.compoundExpression.head->type == L1ParserASTNodeTypeBranch or self->data.compoundExpression.head->type == L1ParserASTNodeTypeAssignment)) return false;
				if(self->data.compoundExpression.tail->type == L1ParserASTNodeTypeBranch or self->data.compoundExpression.tail->type == L1ParserASTNodeTypeBranch) return false;
				return true;
			break;
		case L1ParserASTNodeTypeAssignment:
				if(not (self->data.assignment.source and self->data.assignment.destination)) return false;
				if(self->data.assignment.source->type not_eq L1ParserASTNodeTypeIdentifier) return false;
				if(self->data.assignment.source->type == L1ParserASTNodeTypeBranch or self->data.assignment.source->type == L1ParserASTNodeTypeAssignment) return false;
				return true;
			break;
		case L1ParserASTNodeTypeBranch:
				return true;
			break;
		case L1ParserASTNodeTypeCall:
				return true;
			break;
		default:
			break;
	}
	return false;
}

bool L1ParserASTNodeVerifyIntegrity(const L1ParserASTNode* self)
{
	return L1ParserASTNodeVerifyIntegrity_internal(self);
}