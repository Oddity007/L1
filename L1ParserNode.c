#include "L1ParserNode.h"
#include <stdio.h>

L1ParserNode* L1ParserNodeNew(L1ParserNodeType type)
{
	L1ParserNode* self = calloc(1, sizeof(L1ParserNode));
	self->type = type;
	return self;
}

static void Indent(uint32_t indent)
{
	for (uint32_t i = 0; i < indent; i++) fputc('\t', stdout);
}

void L1ParserNodePrint(const L1ParserNode* self, uint32_t indent)
{
	switch(self->type)
	{
		case L1ParserNodeTypeIdentifier:
			Indent(indent);
			fputs("Identifier\n",stdout);
			Indent(indent+1);
			fwrite(self->data.identifier.characters, self->data.identifier.characterCount, 1, stdout);
			break;
		case L1ParserNodeTypeNumber:
			Indent(indent);
			fputs("Number\n",stdout);
			Indent(indent+1);
			fwrite(self->data.number.characters, self->data.number.characterCount, 1, stdout);
			break;
		case L1ParserNodeTypeString:
			Indent(indent);
			fputs("String\n",stdout);
			Indent(indent+1);
			fwrite(self->data.string.characters, self->data.string.characterCount, 1, stdout);
			break;
		case L1ParserNodeTypeBranch:
			Indent(indent);
			fputs("Branch\n",stdout);
			Indent(indent+1);
			fputs("Condition\n",stdout);
			L1ParserNodePrint(self->data.branch.condition, indent + 2);
			Indent(indent+1);
			fputs("Result if true\n",stdout);
			L1ParserNodePrint(self->data.branch.resultIfTrue, indent + 2);
			Indent(indent+1);
			fputs("Result if false\n",stdout);
			L1ParserNodePrint(self->data.branch.resultIfFalse, indent + 2);
			break;
		case L1ParserNodeTypeAssignment:
			Indent(indent);
			fputs("Assignment\n",stdout);
			Indent(indent + 1);
			fputs("Destination\n",stdout);
			L1ParserNodePrint(self->data.assignment.destination, indent + 2);
			Indent(indent + 1);
			fputs("Source\n",stdout);
			L1ParserNodePrint(self->data.assignment.source, indent + 2);
			Indent(indent + 1);
			fputs("Context\n",stdout);
			L1ParserNodePrint(self->data.assignment.context, indent + 2);
			break;
		case L1ParserNodeTypeCall:
			Indent(indent);
			fputs("Call\n",stdout);
			Indent(indent + 1);
			fputs("Callee\n",stdout);
			L1ParserNodePrint(self->data.call.callee, indent + 2);
			Indent(indent + 1);
			fputs("Argument\n",stdout);
			L1ParserNodePrint(self->data.call.argument, indent + 2);
			break;
		case L1ParserNodeTypeList:
			Indent(indent);
			fputs("List\n",stdout);
			Indent(indent + 1);
			fputs("Head\n",stdout);
			L1ParserNodePrint(self->data.list.head, indent + 2);
			Indent(indent + 1);
			fputs("Tail\n",stdout);
			L1ParserNodePrint(self->data.list.tail, indent + 2);
			break;
		case L1ParserNodeTypeEllipsis:
			Indent(indent);
			fputs("Ellipsis\n",stdout);
			Indent(indent + 1);
			fputs("Trailing Expression\n",stdout);
			L1ParserNodePrint(self->data.ellipsis.trailingExpression, indent + 2);
			break;
	}
	fputc('\n', stdout);
}

void L1ParserNodeDelete(L1ParserNode* self)
{
	switch(self->type)
	{
		case L1ParserNodeTypeIdentifier:
			//free(self->data.identifier.characters);
			break;
		case L1ParserNodeTypeNumber:
			//free(self->data.number.characters);
			break;
		case L1ParserNodeTypeString:
			//free(self->data.string.characters);
			break;
		case L1ParserNodeTypeBranch:
			L1ParserNodeDelete(self->data.branch.condition);
			L1ParserNodeDelete(self->data.branch.resultIfTrue);
			L1ParserNodeDelete(self->data.branch.resultIfFalse);
			break;
		case L1ParserNodeTypeAssignment:
			L1ParserNodeDelete(self->data.assignment.destination);
			L1ParserNodeDelete(self->data.assignment.source);
			L1ParserNodeDelete(self->data.assignment.context);
			break;
		case L1ParserNodeTypeCall:
			L1ParserNodeDelete(self->data.call.callee);
			L1ParserNodeDelete(self->data.call.argument);
			break;
		case L1ParserNodeTypeList:
			L1ParserNodeDelete(self->data.list.head);
			L1ParserNodeDelete(self->data.list.tail);
			break;
		case L1ParserNodeTypeEllipsis:
			L1ParserNodeDelete(self->data.ellipsis.trailingExpression);
			break;
	}
	free(self);
}