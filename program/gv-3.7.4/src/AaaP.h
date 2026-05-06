/*
 * $XConsortium: LayoutP.h,v 1.2 92/01/22 18:03:08 keith Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 modified by Johannes Plass (plass@thep.physik.uni-mainz.de)
 ###jp### 02/06
*/

#ifndef _XawLayoutP_h
#define _XawLayoutP_h

#include "paths.h"
#include "Aaa.h"
#include INC_X11(ConstrainP.h)

typedef enum _BoxType { BoxBox, WidgetBox, GlueBox, VariableBox } BoxType;
    
typedef enum _LayoutDirection {
    LayoutHorizontal = 0, LayoutVertical = 1
} LayoutDirection;

typedef enum _Operator {
    Plus, Minus, Times, Divide, Percent
} Operator;

typedef enum _ExprType {
    Constant,
    Binary,
    Unary,
    Width,
    Height,
    Variable
} ExprType;

typedef struct _Expr *ExprPtr;

typedef struct _Expr {
    ExprType	type;
    union {
	double	    constant;
	struct {
	    Operator	op;
	    ExprPtr	left, right;
	} binary;
	struct {
	    Operator    op;
	    ExprPtr	down;
	} unary;
	XrmQuark    width;
	XrmQuark    height;
	XrmQuark    variable;
    } u;
} ExprRec;

typedef struct _Glue {
    int		order;
    double	value;
    ExprPtr	expr;
} GlueRec, *GluePtr;

typedef struct _BoxParams {
    GlueRec stretch[2];
    GlueRec shrink[2];
} BoxParamsRec, *BoxParamsPtr;

typedef struct _Box *BoxPtr;

typedef BoxPtr	LayoutPtr;

typedef struct _Box {
    BoxPtr	    nextSibling;
    BoxPtr	    parent;
    BoxParamsRec    params;
    int		    size[2];
    int		    natural[2];
    BoxType	    type;
    union {
	struct {
	    BoxPtr	    firstChild;
	    LayoutDirection dir;
	} box;
	struct {
	    XrmQuark	    quark;
	    Widget	    widget;
	} widget;
	struct {
	    ExprPtr	    expr;
	} glue;
	struct {
	    XrmQuark	    quark;
	    ExprPtr	    expr;
	} variable;
    } u;
} LBoxRec; /* this conflicted with Box's BoxRec, besides, it's not used anywhere */

typedef struct _SubInfo {
    int	    naturalSize[2];
    int	    naturalBw;
    Boolean allow_resize;
    Dimension forced_width;
    Dimension forced_height;
} SubInfoRec, *SubInfoPtr;

/*********************************************************************
 *
 * Aaa Widget Private Data
 *
 *********************************************************************/

/* New fields for the Aaa widget class record */

typedef struct _AaaClassPart {
    int foo;			/* keep compiler happy. */
} AaaClassPart;

/* Full Class record declaration */
typedef struct _AaaClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    AaaClassPart        aaa_class;
} AaaClassRec;

extern AaaClassRec aaaClassRec;

typedef struct _AaaConstraintsRec {
    SubInfoRec	aaa;
} AaaConstraintsRec, *AaaConstraints;

#define SubInfo(w)  (&(((AaaConstraints) (w)->core.constraints)->aaa))

/* New Fields for the Aaa widget record */

typedef struct {
    /* resources */
    LayoutPtr	layout;
    Boolean	debug;
    Boolean	resize_width;
    Boolean	resize_height;
    Dimension	maximum_width;
    Dimension	maximum_height;
    Dimension	minimum_width;
    Dimension	minimum_height;
} AaaPart;

/**************************************************************************
 *
 * Full instance record declaration
 *
 **************************************************************************/

typedef struct _AaaRec {
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    AaaPart        aaa;
} AaaRec;
#endif


