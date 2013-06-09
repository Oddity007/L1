%include 
{
#include <assert.h>
#include <stdio.h>
#include "L1ParserNode.h"
}

%name L1Parser

%token_prefix L1ParserTokenType
%token_type {L1ParserNode*}
%extra_argument {L1ParserNode** outNode}

%nonassoc Assign.
%nonassoc QuestionMark.
%nonassoc TypeQualifier.
%nonassoc Ellipsis.
%right Terminal.
%right Comma.

program ::= expression(B) Done. {* outNode = B; puts("Parsed program.");}

compoundExpression(A) ::= expression(B) Assign expression(C) Terminal compoundExpression(D). 
{
	A = L1ParserNodeNew(L1ParserNodeTypeAssignment);
	A->data.assignment.destination = B;
	A->data.assignment.source = C;
	A->data.assignment.context = D;
}
compoundExpression(A) ::= expression(B) QuestionMark expression(C) Terminal compoundExpression(D).
{
	A = L1ParserNodeNew(L1ParserNodeTypeBranch);
	A->data.branch.condition = B;
	A->data.branch.resultIfTrue = C;
	A->data.branch.resultIfFalse = D;
}

expression(A) ::= expression(B) simpleExpression(C). 
{
	A = L1ParserNodeNew(L1ParserNodeTypeCall);
	A->data.call.callee = B;
	A->data.call.argument = C;
}
expression(A) ::= simpleExpression(B). {A = B;}

simpleExpression(A) ::= OpeningParenthesis compoundExpression(B) ClosingParenthesis. {A = B;}
simpleExpression(A) ::= OpeningParenthesis expression(B) ClosingParenthesis. {A = B;}
simpleExpression(A) ::= OpeningSquareBracket list(B) ClosingSquareBracket. {A = B;}
simpleExpression(A) ::= OpeningSquareBracket ClosingSquareBracket. {A = L1ParserNodeNew(L1ParserNodeTypeList);}
simpleExpression(A) ::= Identifier(B). {A = B;}
simpleExpression(A) ::= Number(B). {A = B;}
simpleExpression(A) ::= String(B). {A = B;}

list(A) ::= expression(B) Comma list(C). 
{
	A = L1ParserNodeNew(L1ParserNodeTypeList);
	A->data.list.head = B;
	A->data.list.tail = C;
}
list(A) ::= Ellipsis expression(C).
{
	A = L1ParserNodeNew(L1ParserNodeTypeEllipsis);
	A->data.ellipsis.trailingExpression = C;
}
list(A) ::= Ellipsis.
{
	A = L1ParserNodeNew(L1ParserNodeTypeEllipsis);
}
list(A) ::= expression(B). {A = B;}