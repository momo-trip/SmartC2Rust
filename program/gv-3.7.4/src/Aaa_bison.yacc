
%pure_parser

%{
#include "Aaa_intern.h"
%}

%type	<bval>	    box boxes compositebox
%type	<pval>	    bothparams oneparams
%type	<gval>	    glue opStretch opShrink
%type	<lval>	    orientation
%type	<eval>	    signedExpr simpleExpr expr 

%token		    OC CC OA CA OP CP
%token	<qval>	    NAME
%token	<ival>	    NUMBER
%token	<ival>	    INFINITY
%token		    VERTICAL HORIZONTAL

%token		    EQUAL DOLLAR

%left	<oval>	    PLUS MINUS
%left	<oval>	    TIMES DIVIDE PERCENTOF
%right	<oval>	    PERCENT
%nonassoc	    WIDTH HEIGHT
%right	<oval>	    UMINUS UPLUS

%%
layout		:   compositebox
		    { *(((LayoutConverterArg)layoutConverterArg)->to) = (LayoutPtr) $1; }
		;
box		:   NAME bothparams
		    {
			BoxPtr	box = New(LBoxRec);
                        MEMMESSAGE(WidgetBox)
			box->nextSibling = 0;
			box->type = WidgetBox;
			box->params = *$2;
			Dispose ($2);
			box->u.widget.quark = $1;
			$$ = box;
		    }
		|   signedExpr oneparams
		    {
			BoxPtr	box = New(LBoxRec);
                        MEMMESSAGE(GlueBox)
			box->nextSibling = 0;
			box->type = GlueBox;
			box->params = *$2;
			Dispose ($2);
			box->u.glue.expr = $1;
			$$ = box;
		    }
		|   NAME EQUAL signedExpr
		    {
			BoxPtr	box = New(LBoxRec);
                        MEMMESSAGE(VariableBox)
			box->nextSibling = 0;
			box->type = VariableBox;
			box->u.variable.quark = $1;
			box->u.variable.expr = $3;
			ZeroGlue (box->params.stretch[LayoutHorizontal]); /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutHorizontal]);  /*###jp###*/
			ZeroGlue (box->params.stretch[LayoutVertical]);   /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutVertical]);    /*###jp###*/
			$$ = box;
		    }
		|   compositebox
		    {
			$$ = $1;
		    }
		;
compositebox	:   orientation OC boxes CC
		    {
			BoxPtr	box = New(LBoxRec);
			BoxPtr	child;

                        MEMMESSAGE(BoxBox)
			box->nextSibling = 0;
			box->parent = 0;
			box->type = BoxBox;
			box->u.box.dir = $1;
			box->u.box.firstChild = $3;
			ZeroGlue (box->params.stretch[LayoutHorizontal]); /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutHorizontal]);  /*###jp###*/
			ZeroGlue (box->params.stretch[LayoutVertical]);   /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutVertical]);    /*###jp###*/

			for (child = $3; child; child = child->nextSibling) 
			{
			    if (child->type == GlueBox) 
			    {
				child->params.stretch[!$1].expr = 0;
				child->params.shrink[!$1].expr = 0;
				child->params.stretch[!$1].order = 100000;
				child->params.shrink[!$1].order = 100000;
				child->params.stretch[!$1].value = 1;
				child->params.shrink[!$1].value = 1;
			    }
			    child->parent = box;
			}
			$$ = box;
		    }
		;
boxes		:   box boxes
		    { 
			$1->nextSibling = $2;
			$$ = $1;
		    }
		|   box
		    {	$$ = $1; }
		;
bothparams	:   OA opStretch opShrink TIMES opStretch opShrink CA
		    {	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			p->stretch[LayoutHorizontal] = $2;
			p->shrink[LayoutHorizontal] = $3;
			p->stretch[LayoutVertical] = $5;
			p->shrink[LayoutVertical] = $6;
			$$ = p;
		    }
		|
		    {	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			ZeroGlue (p->stretch[LayoutHorizontal]);
			ZeroGlue (p->shrink[LayoutHorizontal]);
			ZeroGlue (p->stretch[LayoutVertical]);
			ZeroGlue (p->shrink[LayoutVertical]);
			$$ = p;
		    }
		;
oneparams 	:   OA opStretch opShrink CA
		    {	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			p->stretch[LayoutHorizontal] = $2;
			p->shrink[LayoutHorizontal] = $3;
			p->stretch[LayoutVertical] = $2;
			p->shrink[LayoutVertical] = $3;
			$$ = p;
		    }
		|
		    {	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			ZeroGlue (p->stretch[LayoutHorizontal]);
			ZeroGlue (p->shrink[LayoutHorizontal]);
			ZeroGlue (p->stretch[LayoutVertical]);
			ZeroGlue (p->shrink[LayoutVertical]);
			$$ = p;
		    }
		;
opStretch	:   PLUS glue
		    { $$ = $2; }
		|
		    { ZeroGlue ($$); }
		;
opShrink	:   MINUS glue
		    { $$ = $2; }
		|
		    { ZeroGlue ($$); }
		;
glue		:   simpleExpr INFINITY
		    { $$.order = $2; $$.expr = $1; }
		|   simpleExpr
		    { $$.order = 0; $$.expr = $1; }
		|   INFINITY
		    { $$.order = $1; $$.expr = 0; $$.value = 1; }
		;
signedExpr	:   MINUS simpleExpr	    %prec UMINUS
		    {
			$$ = New(ExprRec);
                        MEMMESSAGE(ExprRec)
			$$->type = Unary;
			$$->u.unary.op = $1;
			$$->u.unary.down = $2;
		    }
		|   PLUS simpleExpr	    %prec UPLUS
		    { $$ = $2; }
		|   simpleExpr
		;
simpleExpr    	:   WIDTH NAME
		    {	$$ = New(ExprRec);
                        MEMMESSAGE(Width)
			$$->type = Width;
			$$->u.width = $2;
		    }
		|   HEIGHT NAME
		    {	$$ = New(ExprRec);
                        MEMMESSAGE(Height)
			$$->type = Height;
			$$->u.height = $2;
		    }
		|   OP expr CP
		    { $$ = $2; }
		|   simpleExpr PERCENT
		    {
			$$ = New(ExprRec);
			$$->type = Unary;
			$$->u.unary.op = $2;
			$$->u.unary.down = $1;
		    }
		|   NUMBER
		    {	$$ = New(ExprRec);
                        MEMMESSAGE(Constant)
			$$->type = Constant;
			$$->u.constant = $1;
		    }
		|   DOLLAR NAME
		    {	$$ = New(ExprRec);
                        MEMMESSAGE(Variable)
			$$->type = Variable;
			$$->u.variable = $2;
		    }
		;
expr		:   expr PLUS expr
		    { binary: ;
			$$ = New(ExprRec);
                        MEMMESSAGE(Binary)
			$$->type = Binary;
			$$->u.binary.op = $2;
			$$->u.binary.left = $1;
			$$->u.binary.right = $3;
		    }
		|   expr MINUS expr
		    { goto binary; }
		|   expr TIMES expr
		    { goto binary; }
		|   expr DIVIDE expr
		    { goto binary; }
		|   expr PERCENTOF expr
		    { goto binary; }
		|   MINUS expr		    %prec UMINUS
		    { unary: ;
			$$ = New(ExprRec);
                        MEMMESSAGE(Unary)
			$$->type = Unary;
			$$->u.unary.op = $1;
			$$->u.unary.down = $2;
		    }
		|   PLUS expr		    %prec UPLUS
		    { $$ = $2; }
		|   simpleExpr
		;
orientation	:   VERTICAL
		{   $$ = LayoutVertical; }
		|   HORIZONTAL
		{   $$ = LayoutHorizontal; }
		;
%%
