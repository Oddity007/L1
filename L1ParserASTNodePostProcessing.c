#include "L1Parser.h"
#include <iso646.h>
#include <assert.h>
#include <stdio.h>

static bool L1ParserASTNodeVerifyIntegrity_internal_2(const L1ParserASTNode* self);

static bool L1ParserASTNodeVerifyIntegrity_internal(const L1ParserASTNode* self)
{
	bool result = L1ParserASTNodeVerifyIntegrity_internal_2(self);
	if(not result)
	{
		puts("Error");
		L1ParserPrintASTNode(NULL, self, 0);
		puts("");
	}
	return result;
}

static bool L1ParserASTNodeVerifyIntegrity_internal_2(const L1ParserASTNode* self)
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
				return L1ParserASTNodeVerifyIntegrity_internal(self->data.compoundExpression.head) and L1ParserASTNodeVerifyIntegrity_internal(self->data.compoundExpression.tail);
			break;
		case L1ParserASTNodeTypeAssignment:
				if(not (self->data.assignment.source and self->data.assignment.destination)) return false;
				if(self->data.assignment.destination->type not_eq L1ParserASTNodeTypeIdentifier) return false;
				/*if(self->data.assignment.source->type == L1ParserASTNodeTypeBranch or self->data.assignment.source->type == L1ParserASTNodeTypeAssignment) return false;*/
				return L1ParserASTNodeVerifyIntegrity_internal(self->data.assignment.source) and L1ParserASTNodeVerifyIntegrity_internal(self->data.assignment.destination);
			break;
		case L1ParserASTNodeTypeBranch:
				return L1ParserASTNodeVerifyIntegrity_internal(self->data.branch.condition) and L1ParserASTNodeVerifyIntegrity_internal(self->data.branch.result);
			break;
		case L1ParserASTNodeTypeCall:
				return L1ParserASTNodeVerifyIntegrity_internal(self->data.call.callee) and L1ParserASTNodeVerifyIntegrity_internal(self->data.call.argument);
			break;
		case L1ParserASTNodeTypeFunction:
				return L1ParserASTNodeVerifyIntegrity_internal(self->data.functionLiteral.argument) and L1ParserASTNodeVerifyIntegrity_internal(self->data.functionLiteral.expression);
			break;
	}
	return false;
}

bool L1ParserASTNodeVerifyIntegrity(const L1ParserASTNode* self)
{
	return L1ParserASTNodeVerifyIntegrity_internal(self);
}