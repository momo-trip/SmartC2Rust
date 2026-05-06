/*
 * Aaa.c
 * Version 1.3
 *
 * Johannes Plass, 5/96
 * plass@thep.physik.uni-mainz.de
 *
 * Please note that this is a (only slightly) modified version of
 * Keith Packard's Layout widget.
 *
*/ 

/*
 * $XConsortium: Layout.c,v 1.1 91/09/13 18:51:44 keith Exp $
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

#include "ac_config.h"

/*
#define MESSAGES
*/
#include "message.h"

/* Uncomment this if the resource "debug" should have an effect.
   Otherwise the code for layout debugging is not included.
#define USE_LAYOUT_DEBUG
*/

#include "Aaa_intern.h"
#include INC_XMU(Converters.h)

#define ForAllChildren(pw, childP)						\
  for ( (childP) = (pw)->composite.children ;					\
        (childP) < (pw)->composite.children + (pw)->composite.num_children ;	\
        (childP)++ ) if (!XtIsManaged(*childP)) ; else

#   ifdef MIN
#      undef MIN
#   endif
#   define MIN(_a_,_b_) ((_a_)<(_b_) ? (_a_):(_b_))
#   ifdef MAX
#      undef MAX
#   endif
#   define MAX(_a_,_b_) ((_a_)>(_b_) ? (_a_):(_b_))


#define offset(field) XtOffsetOf(AaaRec, aaa.field)
static XtResource resources[] = {
    {XtNlayout, XtCLayout, XtRLayout, sizeof (BoxPtr),
        offset(layout), XtRLayout, NULL },
#   ifdef USE_LAYOUT_DEBUG
       {XtNdebug, XtCBoolean, XtRBoolean, sizeof(Boolean),
            offset(debug), XtRImmediate, (XtPointer) TRUE},
#   endif /* USE_LAYOUT_DEBUG */
    {XtNresizeWidth, XtCResize, XtRBoolean, sizeof(Boolean),
         offset(resize_width), XtRImmediate, (XtPointer) TRUE}, /* introduced in 11/94 ###jp### */
    {XtNresizeHeight, XtCResize, XtRBoolean, sizeof(Boolean),
         offset(resize_height), XtRImmediate, (XtPointer) TRUE}, /* introduced in 11/94 ###jp### */
    {XtNmaximumWidth, XtCMaximumWidth, XtRDimension, sizeof(Dimension),
         offset(maximum_width), XtRImmediate, (XtPointer)0}, /* introduced in 11/94 ###jp### */
    {XtNmaximumHeight, XtCMaximumHeight, XtRDimension, sizeof(Dimension),
         offset(maximum_height), XtRImmediate, (XtPointer)0}, /* introduced in 11/94 ###jp### */
    {XtNminimumWidth, XtCMinimumWidth, XtRDimension, sizeof(Dimension),
         offset(minimum_width), XtRImmediate, (XtPointer)0}, /* introduced in 11/94 ###jp### */
    {XtNminimumHeight, XtCMinimumHeight, XtRDimension, sizeof(Dimension),
         offset(minimum_height), XtRImmediate, (XtPointer)0}, /* introduced in 11/94 ###jp### */
    {XtNborderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
	XtOffsetOf(RectObjRec,rectangle.border_width),XtRImmediate,(XtPointer)0} 
};
#undef offset

#define offset(field) XtOffsetOf(AaaConstraintsRec, aaa.field)
static XtResource aaaConstraintResources[] = {
    {XtNresizable, XtCResizable, XtRBoolean, sizeof(Boolean),
	offset(allow_resize), XtRImmediate, (XtPointer) True},
    {XtNforcedWidth, XtCWidth, XtRDimension, sizeof(Dimension),
         offset(forced_width), XtRImmediate, (XtPointer)0},
    {XtNforcedHeight, XtCHeight, XtRDimension, sizeof(Dimension),
         offset(forced_height), XtRImmediate, (XtPointer)0},
};
#undef offset

static void ClassInitialize(void);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void ComputeNaturalSizes(AaaWidget,BoxPtr,LayoutDirection);
static void Destroy(Widget);
static void Resize(Widget);
static Boolean SetValues(Widget,Widget,Widget,ArgList,Cardinal*);
static XtGeometryResult GeometryManager(Widget,XtWidgetGeometry*,XtWidgetGeometry*);
static void ChangeManaged(Widget);
static void InsertChild(Widget);
static XtGeometryResult QueryGeometry(Widget,XtWidgetGeometry*,XtWidgetGeometry*);
static void GetDesiredSize(Widget);

static void AaaLayout(AaaWidget,Bool);
static void AaaGetNaturalSize(AaaWidget,Dimension*,Dimension*);
static void AaaFreeLayout(BoxPtr);

#define SuperClass ((ConstraintWidgetClass)&constraintClassRec)

AaaClassRec aaaClassRec = {
   {
/* core class fields */
    /* superclass         */   (WidgetClass) SuperClass,
    /* class name         */   "Aaa",
    /* size               */   sizeof(AaaRec),
    /* class_initialize   */   ClassInitialize,
    /* class_part init    */   NULL,
    /* class_inited       */   FALSE,
    /* initialize         */   Initialize,
    /* initialize_hook    */   NULL,
    /* realize            */   XtInheritRealize,
    /* actions            */   NULL,
    /* num_actions        */   0,
    /* resources          */   resources,
    /* resource_count     */   XtNumber(resources),
    /* xrm_class          */   NULLQUARK,
    /* compress_motion    */   FALSE,
    /* compress_exposure  */   0,
    /* compress_enterleave*/   FALSE,
    /* visible_interest   */   FALSE,
    /* destroy            */   Destroy,
    /* resize             */   Resize,
    /* expose             */   NULL,
    /* set_values         */   SetValues,
    /* set_values_hook    */   NULL,
    /* set_values_almost  */   XtInheritSetValuesAlmost,
    /* get_values_hook    */   NULL,
    /* accept_focus       */   NULL,
    /* version            */   XtVersion,
    /* callback_private   */   NULL,
    /* tm_table           */   NULL,
    /* query_geometry     */   QueryGeometry,
    /* display_accelerator*/   XtInheritDisplayAccelerator,
    /* extension          */   NULL
   }, {
/* composite class fields */
    /* geometry_manager   */   GeometryManager,
    /* change_managed     */   ChangeManaged,
    /* insert_child       */   InsertChild,
    /* delete_child       */   XtInheritDeleteChild,
    /* extension          */   NULL
   }, {
/* constraint class fields */
    /* subresources       */   aaaConstraintResources,
    /* subresource_count  */   XtNumber(aaaConstraintResources),
    /* constraint_size    */   sizeof(AaaConstraintsRec),
    /* initialize         */   NULL,
    /* destroy            */   NULL,
    /* set_values         */   NULL,
    /* extension          */   NULL
   }, {
/* dummy Aaa fields */
    /* foo                */   0,
   }
};

WidgetClass aaaWidgetClass = (WidgetClass) &aaaClassRec;


/*######################################################################*/
/* Functions used for layout debugging */
/*######################################################################*/

#ifdef USE_LAYOUT_DEBUG

static void
PrintGlue (g)
    GlueRec g;
{
    if (g.order == 0 || g.value != 1.0) (void) printf ("%g", g.value);
    if (g.order > 0)
    {
       (void) printf ("%s", "inf");
       if (g.order > 1) (void) printf ("%d", g.order); 
    }
}

static void
PrintDirection (dir)
    LayoutDirection dir;
{
    switch (dir) {
    case LayoutHorizontal:
        (void) printf ("%s", "horizontal");
        break;
    case LayoutVertical:
        (void) printf ("%s", "vertical");
        break;
    default:
        (void) printf ("Unknown layout direction %d\n", dir);
        break;  
    }
}

static void
TabTo(level)
    int level;
{
    while (level--)
        (void) printf ("%s", "   ");
}

static void
PrintBox (box, level)
    BoxPtr          box;
    int             level;
{
    BoxPtr      child;
    
    TabTo (level);
    switch (box->type) {
    case BoxBox:
        PrintDirection (box->u.box.dir);
        (void) printf ("%s\n", "BoxBox");
        for (child = box->u.box.firstChild; child; child = child->nextSibling)
            PrintBox (child, level+1);
            TabTo (level);
        break;
    case WidgetBox:
        (void) printf ("%s", XrmQuarkToString (box->u.widget.quark));
        break;
    case GlueBox:
        (void) printf ("%s", "glue");
        break;
    case VariableBox:
        (void) printf ("variable %s", XrmQuarkToString (box->u.variable.quark));
        break;
    }
    (void) printf ("%s", "<");
    (void) printf ("%s", "+"); 
    PrintGlue (box->params.stretch[LayoutHorizontal]);
    (void) printf ("%s", "-"); 
    PrintGlue (box->params.shrink[LayoutHorizontal]);
    (void) printf ("%s", "*");
    (void) printf ("%s", "+"); 
    PrintGlue (box->params.stretch[LayoutVertical]);
    (void) printf ("%s", "-"); 
    PrintGlue (box->params.shrink[LayoutVertical]);
    (void) printf ("%s", ">");
    (void) printf (" size: %dx%d", box->size[0], box->size[1]);
    (void) printf (" natural: %dx%d\n", box->natural[0], box->natural[1]);
}

#endif /* USE_LAYOUT_DEBUG */

/************************************************************
 *
 * Semi-public routines. 
 *
 ************************************************************/

/*      Function Name: ClassInitialize
 *      Description: The Aaa widgets class initialization proc.
 *      Arguments: none.
 *      Returns: none.
 */

static Boolean
CvtStringToLayout (Display *dpy _GL_UNUSED, XrmValue *args _GL_UNUSED, Cardinal *num_args _GL_UNUSED, XrmValue *from, XrmValue *to, XtPointer *converter_data _GL_UNUSED)
{
    LayoutConverterStruct layoutConverterStruct;

    BEGINMESSAGE(CvtStringToLayout)

    layoutConverterStruct.base    =
    layoutConverterStruct.oldfrom =
    layoutConverterStruct.from    = (String) from->addr;
    layoutConverterStruct.to      = (LayoutPtr *) to->addr;
    if (layout_parse((void*)(&layoutConverterStruct)) == 0) {
       INFMESSAGE(success) 
       ENDMESSAGE(CvtStringToLayout)
       return TRUE;
    } else {
       INFMESSAGE(error)
       ENDMESSAGE(CvtStringToLayout)
       return FALSE;
    }
}

static void
Destroy(w)
   Widget w;
{
   AaaWidget l = (AaaWidget) w;

   BEGINMESSAGE(Destroy)
   if (l->aaa.layout) AaaFreeLayout (l->aaa.layout);
   ENDMESSAGE(Destroy)
}

static void 
ClassInitialize(void)
{
    BEGINMESSAGE(ClassInitialize)
    XtSetTypeConverter ( XtRString, XtRLayout, CvtStringToLayout,
        (XtConvertArgList)NULL, (Cardinal)0, XtCacheNone,(XtDestructor)NULL );
    ENDMESSAGE(ClassInitialize)
}

static XtGeometryResult GeometryManager(Widget child, XtWidgetGeometry *request, XtWidgetGeometry *reply _GL_UNUSED)
{
  AaaWidget    w = (AaaWidget) XtParent(child);
  SubInfoPtr   p = SubInfo(child);
  Boolean      changed=False;
  int	       bw;

  BEGINMESSAGE(GeometryManager)
  INFSMESSAGE(received request from:,XtName(child))

  if (!(request->request_mode & (CWWidth | CWHeight | CWBorderWidth))) {
    INFMESSAGE(request not of interest) ENDMESSAGE(GeometryManager)
    return XtGeometryYes;
  }
  if (request->request_mode & XtCWQueryOnly) {
    /* query requests are not properly implemented ... ###jp### */
    INFMESSAGE(request is query only and will be denied) ENDMESSAGE(GeometryManager)
    return XtGeometryNo;
  }
  if (!(p->allow_resize)) {
    INFMESSAGE(child is not allowed to resize)
    ENDMESSAGE(GeometryManager)
    return XtGeometryNo;
  }
  INFIIMESSAGE(current size of child:,child->core.width,child->core.height)

  bw = p->naturalBw;
  if ((request->request_mode & CWBorderWidth) && request->border_width != child->core.border_width) {
    INFMESSAGE(request for new border width)
    changed = True;
    p->naturalBw = request->border_width;
    if (p->forced_width) p->naturalSize[LayoutHorizontal] = p->forced_width;
    else p->naturalSize[LayoutHorizontal] = p->naturalSize[LayoutHorizontal]-2*bw+2*request->border_width;
    if (p->forced_height) p->naturalSize[LayoutVertical] = p->forced_height;
    else p->naturalSize[LayoutVertical] = p->naturalSize[LayoutVertical] -2*bw+2*request->border_width;
  }
  if ((request->request_mode & CWWidth) && request->width != child->core.width) {
    INFMESSAGE(request for new width)
    changed = True;
    if (p->forced_width) p->naturalSize[LayoutHorizontal] = p->forced_width;
    else p->naturalSize[LayoutHorizontal] = request->width + 2*p->naturalBw;
  }
  if ((request->request_mode & CWHeight) && request->height != child->core.height) {
    INFMESSAGE(request for new height)
    changed = True;
    if (p->forced_height) p->naturalSize[LayoutVertical] = p->forced_height;
    else p->naturalSize[LayoutVertical] = request->height + 2*p->naturalBw;
  }
  if (changed) {
    AaaLayout(w,TRUE);
    INFIIMESSAGE(new size of child:,child->core.width,child->core.height)
    ENDMESSAGE(GeometryManager)
    return XtGeometryDone;
  } else {
    ENDMESSAGE(GeometryManager)
    return XtGeometryYes;
  }
}

static void Initialize(Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
   AaaWidget l= (AaaWidget) new;
   Dimension prefwidth,prefheight;
   BEGINMESSAGE(Initialize)
   if ( (l->core.width  <= l->aaa.minimum_width)  ||
        (l->core.height <= l->aaa.minimum_height) )  {
      AaaGetNaturalSize (l, &prefwidth, &prefheight);
      prefwidth = MAX(l->aaa.minimum_width,prefwidth);
      if (prefwidth<1)  prefwidth =1;
      prefheight= MAX(l->aaa.minimum_height,prefheight);
      if (prefheight<1) prefheight=1;
      if (l->core.width  <= l->aaa.minimum_width)  l->core.width  = prefwidth;
      if (l->core.height <= l->aaa.minimum_height) l->core.height = prefheight;
  }
  INFIIMESSAGE(size:,l->core.width,l->core.height)
  ENDMESSAGE(Initialize)
}

static void ChangeManaged(Widget gw)
{
    AaaWidget        w = (AaaWidget) gw;
    Widget              *children;

    BEGINMESSAGE(ChangeManaged)
    ForAllChildren (w, children) GetDesiredSize (*children);
    AaaLayout ((AaaWidget)w,TRUE);
    ENDMESSAGE(ChangeManaged)
}

static void
GetDesiredSize (Widget child)
{
    XtWidgetGeometry    desired;
    SubInfoPtr          p;
    
    XtQueryGeometry (child, (XtWidgetGeometry *) NULL, &desired);
    p = SubInfo (child);
    p->naturalBw = desired.border_width;
    if (p->forced_width)
      p->naturalSize[LayoutHorizontal] = p->forced_width;
    else
      p->naturalSize[LayoutHorizontal] = desired.width + desired.border_width * 2;
    if (p->forced_height)
	p->naturalSize[LayoutVertical] = p->forced_height;
      else
	p->naturalSize[LayoutVertical] = desired.height + desired.border_width * 2;;
    INFSMESSAGE1(GetDesiredSize for:,XtName(child))
}

static void InsertChild (Widget child)
{
    (*SuperClass->composite_class.insert_child) (child);
    GetDesiredSize (child);
}

static void
Resize(Widget gw)
{
    BEGINMESSAGE(Resize)
    IIMESSAGE(gw->core.width,gw->core.height)
    IMESSAGE(gw->core.border_width)
    AaaLayout ((AaaWidget) gw, FALSE);
    IIMESSAGE(gw->core.width,gw->core.height)
    IMESSAGE(gw->core.border_width)
    ENDMESSAGE(Resize)
}

/* ARGSUSED */
static Boolean
SetValues(Widget gold, Widget greq _GL_UNUSED, Widget gnew, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
    AaaWidget    old = (AaaWidget) gold,
                    new = (AaaWidget) gnew;

    BEGINMESSAGE(SetValues)
    if (old->aaa.layout != new->aaa.layout) {
       AaaLayout(new,TRUE);
    }
    ENDMESSAGE(SetValues)
    return FALSE;

} /* SetValues */

static XtGeometryResult
QueryGeometry (Widget gw, XtWidgetGeometry *request, XtWidgetGeometry *preferred_return)
{
    AaaWidget        w = (AaaWidget) gw;
    XtGeometryResult    result;
    XtWidgetGeometry    preferred_size;

    BEGINMESSAGE(QueryGeometry)
    if (request && !(request->request_mode & (CWWidth|CWHeight))) {
       INFMESSAGE(return XtGeometryYes)
       ENDMESSAGE(QueryGeometry)
       return XtGeometryYes;
    }
    AaaGetNaturalSize (w, &preferred_size.width, &preferred_size.height);
    preferred_return->request_mode = 0;
    result = XtGeometryYes;
    if (!request) {
        preferred_return->width = preferred_size.width;
        preferred_return->height= preferred_size.height;
        if (preferred_size.width != w->core.width) {
            preferred_return->request_mode |= CWWidth;
            result = XtGeometryAlmost;
        }
        if (preferred_size.height != w->core.height) {
            preferred_return->request_mode |= CWHeight;
            result = XtGeometryAlmost;
        }
    } else {
        if (request->request_mode & CWWidth) {
            if (preferred_size.width > request->width)
            {
                if (preferred_size.width == w->core.width)
                    result = XtGeometryNo;
                else if (result != XtGeometryNo) {
                    result = XtGeometryAlmost;
                    preferred_return->request_mode |= CWWidth;
                    preferred_return->width = preferred_size.width;
                }
            }
        }
        if (request->request_mode & CWHeight) {
            if (preferred_size.height > request->height)
            {
                if (preferred_size.height == w->core.height)
                    result = XtGeometryNo;
                else if (result != XtGeometryNo) {
                    result = XtGeometryAlmost;
                    preferred_return->request_mode |= CWHeight;
                    preferred_return->height = preferred_size.height;
                }
            }
        }
    }
    ENDMESSAGE(QueryGeometry)
    return result;
}

/*
 * Layout section.  Exports AaaGetNaturalSize and
 * AaaLayout to above section
 */

static ExprPtr
LookupVariable (BoxPtr child, XrmQuark quark)
{
    BoxPtr      parent, box;

    while ((parent = child->parent))
    {
        for (box = parent->u.box.firstChild; box != child; box = box->nextSibling) {
            if (box->type == VariableBox && box->u.variable.quark == quark) {
               return box->u.variable.expr;
            }
        }
        child = parent;
    }
    return((ExprPtr)NULL);
}

static double
Evaluate (AaaWidget l, BoxPtr box, ExprPtr expr, double natural)
{
    double      left, right, down;
    Widget      widget;
    SubInfoPtr  info;

    switch (expr->type) {
       case Constant:
                return expr->u.constant;
       case Binary:
                left = Evaluate (l, box, expr->u.binary.left, natural);
                right = Evaluate (l, box, expr->u.binary.right, natural);
                switch (expr->u.binary.op) {
                   case Plus:
                      return left + right;
                   case Minus:
                      return left - right;
                   case Times:
                      return left * right;
                   case Divide:
                      return left / right;
                   case Percent:
                      return right * left / 100.0;
                 }
        case Unary:
                 down = Evaluate (l, box, expr->u.unary.down, natural);
                 switch (expr->u.unary.op) {
                    case Percent:
                      return natural * down / 100.0;
                    case Minus:
                      return -down;
                    case Plus:
                    case Times:
                    case Divide:
                      /* should never be reached */
                      INFMESSAGE(### Warning: in Evaluate: this code should never be reached)
                      return(0);
                 }
        case Width:
                 widget = QuarkToWidget (l, expr->u.width);
                 if (!widget) return 0;
                 info = SubInfo (widget);
                 return info->naturalSize[LayoutHorizontal];
        case Height:
                 widget = QuarkToWidget (l, expr->u.height);
                 if (!widget) return 0;
                 info = SubInfo (widget);
                 return info->naturalSize[LayoutVertical];
        case Variable:
	         {
                    ExprPtr tmp;
                    tmp = LookupVariable (box, expr->u.variable);
                    if (!tmp) {
                       char    buf[256];
                       (void) sprintf (buf, "Aaa Widget: Variable '%s' is undefined.\n",
                                       XrmQuarkToString (expr->u.variable));
                       XtError (buf);
                       return 0.0;
                    }
                    return Evaluate (l, box, tmp, natural);
                 }
    }
    /* should never be reached */
    INFMESSAGE(### Warning: in Evaluate: end of routine should never be reached)
    return(0);
}

static void
DisposeExpr (ExprPtr expr)
{
    MEMBEGINMESSAGE(DisposeExpr)
    if (!expr) {
       MEMMESSAGE(null pointer) MEMENDMESSAGE(DisposeExpr)
       return;
    }
    switch (expr->type) {
       case Constant:
               MEMMESSAGE(freeing Constant)
               break;
       case Binary:
               MEMMESSAGE(freeing Binary left and right)
               DisposeExpr (expr->u.binary.left);
               DisposeExpr (expr->u.binary.right);
               break;
       case Unary:
               MEMMESSAGE(freeing Unary)
               DisposeExpr (expr->u.unary.down);
               break;
       case Width:
               MEMMESSAGE(freeing Width)
               break;
       case Height:
               MEMMESSAGE(freeing Height)
               break;
       case Variable:
               MEMMESSAGE(freeing Variable)
               break;
       default:
               MEMMESSAGE(?? trying to free unknown expression)
               fprintf(stderr,"AaaWidget DisposeExpr: accessed illegal memory location\n");
               MEMENDMESSAGE(DisposeExpr)
               return;
    }
    Dispose (expr);
    expr = (ExprPtr)NULL; /*###jp###*/
    MEMENDMESSAGE(DisposeExpr)
}

#define CheckGlue(l, box, glue, n) {					\
   if (glue.expr) glue.value = Evaluate (l, box, glue.expr, n);		\
   if (glue.order == 0 && glue.value == 0)       glue.order = -1;	\
   else if (glue.order == -1 && glue.value != 0) glue.order = 0;	\
}

#define DoStretch(l, box, dir) \
    CheckGlue (l, box, box->params.stretch[dir], (double) box->natural[dir]);
        
#define DoShrink(l, box, dir) \
    CheckGlue (l, box, box->params.shrink[dir], (double) box->natural[dir])

/* compute the natural sizes of a box */
static void
ComputeNaturalSizes (AaaWidget l, BoxPtr box, LayoutDirection dir)
{
    BoxPtr      child;
    Widget      w;
    SubInfoPtr  info;
    int         minStretchOrder, minShrinkOrder;
    LayoutDirection thisDir;
    
    switch (box->type) {
       case VariableBox:
               box->natural[LayoutHorizontal] = 0;
               box->natural[LayoutVertical] = 0;
               break;
       case WidgetBox:
               w = box->u.widget.widget = QuarkToWidget (l, box->u.widget.quark);
               if (!w) {
                  box->natural[LayoutHorizontal] = 0;
                  box->natural[LayoutVertical] = 0;
               }
               else {
                  info = SubInfo (w);
                  box->natural[LayoutHorizontal] = info->naturalSize[LayoutHorizontal];
                  box->natural[LayoutVertical] = info->naturalSize[LayoutVertical];
               }
               DoStretch (l, box, dir);
               DoShrink (l, box, dir);
               DoStretch (l, box, !dir);
               DoShrink (l, box, !dir);
               break;
       case GlueBox:
               box->natural[dir] = Evaluate (l, box, box->u.glue.expr, 0.0);
               box->natural[!dir] = 0;
               DoStretch (l, box, dir);
               DoShrink (l, box, dir);
               break;
       case BoxBox:
               thisDir = box->u.box.dir;
               box->natural[0] = 0;
               box->natural[1] = 0;
               minStretchOrder = 100000;
               minShrinkOrder = 100000;
               ZeroGlue (box->params.shrink[thisDir]);
               ZeroGlue (box->params.stretch[thisDir]);
               box->params.shrink[!thisDir].order = 100000;
               box->params.stretch[!thisDir].order = 100000;
               for (child = box->u.box.firstChild; child; child = child->nextSibling) {
                   ComputeNaturalSizes (l, child, thisDir);
                  /*
                   * along box axis:
                   *  normal size += child normal size
                   *  shrink += child shrink
                   *  stretch += child stretch
                  */

                   box->natural[thisDir] += child->natural[thisDir];
                   AddGlue (box->params.shrink[thisDir],
                        box->params.shrink[thisDir],
                        child->params.shrink[thisDir]);
                   AddGlue (box->params.stretch[thisDir],
                        box->params.stretch[thisDir],
                        child->params.stretch[thisDir]);
                  /*
                   * normal to box axis:
                   *  normal size = maximum child normal size of minimum shrink order
                   *  shrink = difference between normal size and minimum shrink
                   *  stretch = minimum child stretch
                  */
                   if (box->natural[!thisDir] >= child->natural[!thisDir]) {
                      if (child->params.stretch[!thisDir].order < minShrinkOrder) {
                         box->natural[!thisDir] = child->natural[!thisDir];
                         minStretchOrder = child->params.stretch[!thisDir].order;
                         if (child->params.shrink[!thisDir].order < minShrinkOrder)
                            minShrinkOrder = child->params.shrink[!thisDir].order;
                      }
                   }
                   else {
                      if (child->params.shrink[!thisDir].order <= minStretchOrder) {
                         box->natural[!thisDir] = child->natural[!thisDir];
                         minShrinkOrder = child->params.shrink[!thisDir].order;
                         if (child->params.stretch[!thisDir].order < minStretchOrder)
                            minStretchOrder = child->params.stretch[!thisDir].order;
                      }
                   }
                   MinGlue (box->params.stretch[!thisDir],child->params.stretch[!thisDir]);
                   MinGlue (box->params.shrink[!thisDir] ,child->params.shrink[!thisDir]);
                }
                if (box->params.shrink[!thisDir].order <= 0) {
                   int      minSize;
                   int      largestMinSize;
            
                   largestMinSize = 0;
                   for (child = box->u.box.firstChild; child; child = child->nextSibling) {
                       if (child->params.shrink[!thisDir].order <= 0) {
                          minSize = child->natural[!thisDir] -
                          child->params.shrink[!thisDir].value;
                          if (minSize > largestMinSize) largestMinSize = minSize;
                       }
                   }
                   box->params.shrink[!thisDir].value = box->natural[!thisDir] - largestMinSize;
                   if (box->params.shrink[!thisDir].value == 0) box->params.shrink[!thisDir].order = -1;
                   else box->params.shrink[!thisDir].order = 0;
                }
    }
}

/* given the boxs geometry, set the geometry of the pieces */

#define GluePart(a,b,dist)		\
  ((a) ? ((int) (((a) * (dist)) / (b) + \
  ((dist >= 0) ? 0.5 : -0.5))) : 0)

static Bool
ComputeSizes (BoxPtr box)
{
    LayoutDirection dir;
    BoxPtr          child;
    GlueRec         stretch;
    GlueRec         shrink;
    GlueRec         totalGlue[2];
    double          remainingGlue;
    GluePtr         glue;
    int             size;
    int             totalSizes;
    int             totalChange[2];
    int             change;
    int             remainingChange;
    Bool            shrinking;
    Bool            happy;
    int             i;
    int             maxGlue;

    dir = box->u.box.dir;
    size = box->size[dir];
    
    stretch = box->params.stretch[dir];
    shrink = box->params.shrink[dir];
    
    /* pick the correct adjustment parameters based on the change direction */
    
    totalChange[0] = size - box->natural[dir];

    shrinking = totalChange[0] < 0;
    
    totalChange[1] = 0;
    totalGlue[1].order = 100000;
    totalGlue[1].value = 0;
    maxGlue = 1;
    if (shrinking) 
    {
        totalGlue[0] = shrink;
        /* for first-order infinites, shrink it to zero and then
         * shrink the zero-orders
         */
        
        if (shrink.order == 1) { 
            totalSizes = 0;
            remainingGlue = 0;

            for (child = box->u.box.firstChild; child; child = child->nextSibling) 
            {
                switch (child->params.shrink[dir].order) {
                case 0:
                    remainingGlue += (int)child->params.shrink[dir].value;
                    break;
                case 1:
                    totalSizes += (int)child->natural[dir];
                    break;
                }
            }            
            if (totalSizes < -totalChange[0])
            {
                totalGlue[1] = shrink;
                totalGlue[0].order = 0;
                totalGlue[0].value = remainingGlue;
                totalChange[1] = -totalSizes;
                totalChange[0] = totalChange[0] - totalChange[1];
                maxGlue = 2;
            }
        }
        if (totalGlue[0].order <= 0 && totalChange[0] > totalGlue[0].value) {
            totalChange[0] = totalGlue[0].value;
        }
    }
    else totalGlue[0] = stretch;
        
    /* adjust each box */
    totalSizes = 0;
    remainingGlue = totalGlue[0].value + totalGlue[1].value;
    remainingChange = totalChange[0] + totalChange[1];
    happy = True;
    for (child = box->u.box.firstChild; child; child = child->nextSibling) 
    {
        if (shrinking) glue = &child->params.shrink[dir];
        else           glue = &child->params.stretch[dir];
    
        child->size[dir] = child->natural[dir];
        for (i = 0; i < maxGlue; i++) {
           if (glue->order == totalGlue[i].order) {
              remainingGlue -= glue->value;
              if (remainingGlue <= 0) {
                 change = remainingChange;
              } else {
                 change = GluePart (glue->value, totalGlue[i].value, totalChange[i]);
              }
              child->size[dir] += change;
              remainingChange -= change;
           }
        }
        child->size[!dir] = box->size[!dir];
        totalSizes += child->size[dir];
        if (child->type == BoxBox) if (!ComputeSizes (child)) happy = False;
    }
    return totalSizes == box->size[dir] && happy;
}

static void
SetSizes (BoxPtr box, Position x, Position y)
{
    BoxPtr      child;
    int         width, height;
    int         bw;
    Widget      w;
    SubInfoPtr  info;

    BEGINMESSAGE1(SetSizes)
    switch (box->type) {
    case WidgetBox:
        w = box->u.widget.widget;
        if (w)
        {
	  /*info = (SubInfoPtr) w->core.constraints;*/
 	    info = SubInfo(w);
            width = box->size[LayoutHorizontal];
            height = box->size[LayoutVertical];
            bw = info->naturalBw;
            width = width - bw*2;
            height = height - bw*2;
            /* Widgets which grow too small are placed off screen */
            if (width <= 0 || height <= 0) 
            {
                width = 1;
                height = 1;
                x = -1 - 2*bw;
                y = -1 - 2*bw;
            }
            INFSMESSAGE1(configuring widget:,XtName(w))
            XtConfigureWidget (w, x, y, 
                              (Dimension)width, (Dimension)height, 
                              (Dimension)bw);
        }
        break;
    case BoxBox:
        for (child = box->u.box.firstChild; child; child = child->nextSibling) 
        {
            SetSizes (child, x, y);
            if (box->u.box.dir == LayoutHorizontal)
                x += child->size[LayoutHorizontal];
            else
                y += child->size[LayoutVertical];
        }
        break;
    case GlueBox:
    case VariableBox:
        /* should never be reached */
        break;
    }
    ENDMESSAGE1(SetSizes)
}

static void
AaaFreeLayout (BoxPtr box)
{
    BoxPtr  child, next;
    
    MEMBEGINMESSAGE(AaaFreeLayout)
 
   if (!box) {
      MEMMESSAGE(null pointer) MEMENDMESSAGE(AaaFreeLayout)
      return;
   }

   switch (box->type) {
    case BoxBox:
        MEMMESSAGE(### dispose BoxBox)
        for (child = box->u.box.firstChild; child; child = next)
        {
            next = child->nextSibling;
            AaaFreeLayout (child);
        }
        break;
    case WidgetBox:
        MEMMESSAGE(### dispose WidgetBox)
        DisposeExpr (box->params.stretch[LayoutHorizontal].expr);
        DisposeExpr (box->params.shrink[LayoutHorizontal].expr);
        DisposeExpr (box->params.stretch[LayoutVertical].expr);
        DisposeExpr (box->params.shrink[LayoutVertical].expr);
        break;
    case GlueBox:
        MEMMESSAGE(### dispose GlueBox)
        DisposeExpr (box->u.glue.expr);
        DisposeExpr (box->params.stretch[LayoutHorizontal].expr);
        DisposeExpr (box->params.shrink[LayoutHorizontal].expr);
        DisposeExpr (box->params.stretch[LayoutVertical].expr);
        DisposeExpr (box->params.shrink[LayoutVertical].expr);
        break;
    case VariableBox: /* added this case ###jp### */
        MEMMESSAGE(### dispose VariableBox)
        DisposeExpr (box->u.variable.expr);
        break;
    default:
        MEMMESSAGE(### Warning: probably accessed illegal memory location)
        fprintf(stderr,"AaaFreeLayout: probably accessed illegal memory location\n");
        MEMENDMESSAGE(AaaFreeLayout)
        return;
    }

    Dispose (box);
    box = (BoxPtr)NULL; /* ###jp###*/
    MEMENDMESSAGE(AaaFreeLayout)
}


static void
AaaGetNaturalSize (AaaWidget l, Dimension *widthp, Dimension *heightp)
{
    BoxPtr              box;

    BEGINMESSAGE(AaaGetNaturalSize)
    box = l->aaa.layout;
    if (box) {
#      ifdef MESSAGES
          {
             Dimension w,h;
             w=box->natural[LayoutHorizontal];
             h=box->natural[LayoutVertical];
             INFIIMESSAGE(before calculation:,w,h)
          }
#      endif
       ComputeNaturalSizes (l, box, LayoutHorizontal);
       *widthp = box->natural[LayoutHorizontal];
       *heightp = box->natural[LayoutVertical];
    } else {
       INFMESSAGE(no layout available)
       *widthp = 0;
       *heightp = 0;
    }
    INFIIMESSAGE(natural size:,*widthp,*heightp)
    ENDMESSAGE(AaaGetNaturalSize)
}

static void
AaaLayout(AaaWidget l, Bool attemptResize)
{
    BoxPtr              box = l->aaa.layout;
    Dimension           width, height;
    Dimension           pwidth, pheight;

    BEGINMESSAGE(AaaLayout)

    if (!box) { INFMESSAGE(no layout specified) ENDMESSAGE(AaaLayout) return; }

    if (l->aaa.maximum_width  > 0) l->aaa.maximum_width  = MAX(l->aaa.maximum_width,l->core.width);
    if (l->aaa.maximum_height > 0) l->aaa.maximum_height = MAX(l->aaa.maximum_height,l->core.height);
    if (l->aaa.minimum_width  > 0) l->aaa.minimum_width  = MIN(l->aaa.minimum_width,l->core.width);
    if (l->aaa.minimum_height > 0) l->aaa.minimum_height = MIN(l->aaa.minimum_height,l->core.height);

#   ifdef MESSAGES
    { char *name=XtName((Widget)l);
      Dimension w,h;
      INFSMESSAGE(layouting widget:,name)
      w=l->core.width; h=l->core.height;
      INFIIMESSAGE(current size:,w,h)
      w=l->aaa.maximum_width; h=l->aaa.maximum_height;
      INFIIMESSAGE(new maximum size:,w,h)
      w=l->aaa.minimum_width; h=l->aaa.minimum_height;
      INFIIMESSAGE(new minimum size:,w,h)
      if (l->aaa.resize_width)       { INFMESSAGE(width change is allowed) }
      else                              { INFMESSAGE(width change is not allowed) }
      if (l->aaa.resize_height)      { INFMESSAGE(height change is allowed) }
      else                              { INFMESSAGE(height change is not allowed) }
    }
#   endif

    AaaGetNaturalSize (l, &pwidth, &pheight);

    box->size[LayoutHorizontal] = l->core.width; 
    box->size[LayoutVertical]   = l->core.height;
    if (!l->aaa.resize_width && !l->aaa.resize_height) attemptResize=False;

#   ifdef MESSAGES
       if (attemptResize) { INFMESSAGE(may attempt to resize) }
       else               { INFMESSAGE(will not attempt to resize) }
#   endif

    if (attemptResize) {
       if (!(l->aaa.resize_width))  {
          pwidth = l->core.width;
       } else {
          if (l->aaa.maximum_width > 0)  pwidth  = MIN(pwidth,l->aaa.maximum_width);
          if (l->aaa.minimum_width > 0)  pwidth  = MAX(pwidth,l->aaa.minimum_width);
       }
       if (!(l->aaa.resize_height)) {
          pheight = l->core.height;
       } else {
          if (l->aaa.maximum_height > 0) pheight = MIN(pheight,l->aaa.maximum_height);
          if (l->aaa.minimum_height > 0) pheight = MAX(pheight,l->aaa.minimum_height);
       }
       if ((pwidth == l->core.width) && (pheight == l->core.height)) {
          INFMESSAGE(resize attempt is unnecessary)
          attemptResize=False;
       }
    }

    if (attemptResize) {
       XtGeometryResult result; 
       INFIIMESSAGE(proposing resize:,pwidth,pheight)
       result = XtMakeResizeRequest ((Widget) l,pwidth,pheight,&width, &height);
       switch (result) {
           case XtGeometryYes:
              INFMESSAGE(XtGeometryYes)
              break;
           case XtGeometryAlmost:
              INFIIMESSAGE(XtGeometryAlmost:,width,height)
              result = XtMakeResizeRequest ((Widget) l,width, height,&width, &height);
              if (result!=XtGeometryYes) 
                 fprintf(stderr,"Aaa widget: Warning, parent denied to set the size he proposed.");
              break;
           case XtGeometryNo:
              INFMESSAGE(XtGeometryNo)
              break;
           case XtGeometryDone:
	      /* should never be reached */
              INFMESSAGE(XtGeometryDone)
              break;
       }
       box->size[LayoutHorizontal] = l->core.width;
       box->size[LayoutVertical]   = l->core.height;
    }

    ComputeSizes(box);
#   ifdef USE_LAYOUT_DEBUG
       if (l->aaa.debug) { PrintBox (box, 0); fflush (stdout); }
#   endif /* USE_LAYOUT_DEBUG */
    INFMESSAGE(adjusting children)
    SetSizes (box,0,0);

#   ifdef MESSAGES
    {
      Dimension w,h;
      w=l->core.width; h=l->core.height; INFIIMESSAGE(new size:,w,h)
    }
#   endif
    ENDMESSAGE(AaaLayout)
}

/*###################### Public Routines ################################*/
                                         /* ###jp### 1/94 */

void
AaaWidgetGetNaturalSize(
    AaaWidget    l,
    Dimension       *wp,
    Dimension       *hp
)
{
  BEGINMESSAGE(AaaWidgetGetNaturalSize)
/*
  *wp = (l->aaa.layout)->natural[LayoutHorizontal];
  *hp = (l->aaa.layout)->natural[LayoutVertical];
*/
  AaaGetNaturalSize(l, wp, hp);
  INFIIMESSAGE(natural:,*wp,*hp)
  ENDMESSAGE(AaaWidgetGetNaturalSize)
}

void
AaaWidgetAllowResize(AaaWidget l, Boolean wflag, Boolean hflag)
{
  BEGINMESSAGE(AaaWidgetAllowResize)
  l->aaa.resize_width  = wflag;
  l->aaa.resize_height = hflag;
# ifdef MESSAGES
  if (wflag) {INFMESSAGE(horizontal resizing is enabled)} 
  else       {INFMESSAGE(horizontal resizing is disabled)}
  if (hflag) {INFMESSAGE(vertical resizing is enabled)} 
  else       {INFMESSAGE(vertical resizing is disabled)}
# endif
  ENDMESSAGE(AaaWidgetAllowResize)
}


#if 0 /* ########## NOT USED ########### */

void
AaaWidgetSetSizeBounds(
    AaaWidget  l,
    Dimension *minw_p,
    Dimension *minh_p,
    Dimension *maxw_p,
    Dimension *maxh_p
)
{
  BEGINMESSAGE(AaaWidgetSetSizeBounds)
  if (minw_p) l->aaa.minimum_width  = *minw_p;
  if (minh_p) l->aaa.minimum_height = *minh_p;
  if (maxw_p) l->aaa.maximum_width  = *maxw_p;
  if (maxh_p) l->aaa.maximum_height = *maxh_p;
  ENDMESSAGE(AaaWidgetSetSizeBounds)
}

void
AaaWidgetGetSizeBounds(
    AaaWidget  l,
    Dimension *minw_p,
    Dimension *minh_p,
    Dimension *maxw_p,
    Dimension *maxh_p
)
{
  BEGINMESSAGE(AaaWidgetGetSizeBounds)
  if (minw_p) *minw_p = l->aaa.minimum_width ;
  if (minh_p) *minh_p = l->aaa.minimum_height;
  if (maxw_p) *maxw_p = l->aaa.maximum_width ;
  if (maxh_p) *maxh_p = l->aaa.maximum_height;
  ENDMESSAGE(AaaWidgetGetSizeBounds)
}

#endif /* ########## NOT USED ########### */
