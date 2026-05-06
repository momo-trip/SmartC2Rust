/*
**
** Clip.c
**
** Copyright (C) 1995, 1996, 1997 Johannes Plass
** Copyright (C) 2004 Jose E. Marchesi
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with GNU gv; see the file COPYING.  If not, write to
** the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA 02111-1307, USA.
** 
** Authors:   Johannes Plass (plass@thep.physik.uni-mainz.de)
**            Department of Physics
**            Johannes Gutenberg-University
**            Mainz, Germany
**
**            Jose E. Marchesi (jemarch@gnu.org)
**            GNU Project
** 
*/
#include "ac_config.h"

/*
#define MESSAGES
*/
#include "message.h"

#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(XawInit.h)
#include INC_XMU(Misc.h)
#include "ClipP.h"

#include <stdio.h>

#define CW_CORE        cw->core
#define CW_CLIP        cw->clip
#define CW_COMPOSITE   cw->composite
  
#define CW_RESIZE      CW_CLIP.resize

#define CW_CHILD_NAT_WIDTH  CW_CLIP.child_width_nat
#define CW_CHILD_NAT_HEIGHT CW_CLIP.child_height_nat
#define CW_CHILD_NAT_X      CW_CLIP.child_x_nat
#define CW_CHILD_NAT_Y      CW_CLIP.child_y_nat
#define CW_CHILD_NAT_BORDER CW_CLIP.child_border_nat

#define CW_CHILD_P     CW_COMPOSITE.children
#define CW_CHILD       (CW_COMPOSITE.children[0])

/*
 * resources for the clip
 */
static XtResource resources[] = {
#define poff(field) XtOffsetOf(ClipRec, clip.field)
    { XtNreportCallback, XtCReportCallback, XtRCallback, sizeof(XtPointer),
	poff(report_callbacks), XtRCallback, (XtPointer) NULL },
#if 0
    { XtNchildX, XtCChildX, XtRInt, sizeof(int),
	poff(child_x), XtRImmediate, (XtPointer) 0 },
    { XtNchildY, XtCChildY, XtRInt, sizeof(int),
	poff(child_y), XtRImmediate, (XtPointer) 0 },
#endif
#undef poff
};


/*
 * widget class methods used below
 */
static void InsertChild(Widget);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
#if 0
static Boolean SetValues();
#endif
static void Realize(Widget,XtValueMask*,XSetWindowAttributes*);	/* set gravity and upcall */
static void Resize(Widget);		/* report new size */
static XtGeometryResult GeometryManager(Widget,XtWidgetGeometry*,XtWidgetGeometry*);  /* deal with child requests */
static void ChangeManaged(Widget);		/* somebody added a new widget */
static XtGeometryResult QueryGeometry(Widget,XtWidgetGeometry*,XtWidgetGeometry*);  /* say how big would like to be */
static XtGeometryResult LayoutClip(ClipWidget);

#define SuperClass ((CompositeWidgetClass)&compositeClassRec)

ClipClassRec clipClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass)SuperClass,
    /* class_name		*/	"Clip",
    /* widget_size		*/	sizeof(ClipRec),
    /* class_initialize		*/	XawInitializeWidgetSet,
    /* class_part_initialize	*/	NULL,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	NULL,
    /* num_actions		*/	0,
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	NULL,
    /* resize			*/	Resize,
    /* expose			*/	NULL,
    /* set_values		*/	NULL,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	NULL,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* composite fields */
    /* geometry_manager		*/	GeometryManager,
    /* change_managed		*/	ChangeManaged,
    /* insert_child		*/	InsertChild,
    /* delete_child		*/	XtInheritDeleteChild,
    /* extension		*/	NULL
  },
  { /* clip fields */
    /* ignore                   */	0
  }
};

WidgetClass clipWidgetClass = (WidgetClass) &clipClassRec;


/*****************************************************************************
 *                                                                           *
 *			       utility routines                              *
 *                                                                           *
 *****************************************************************************/

/*---------------------------------------------------*/
/* SendReport */
/*---------------------------------------------------*/

static void SendReport (ClipWidget cw, unsigned int changed)
{
   Widget child = CW_CHILD;

   BEGINMESSAGE(SendReport)
   if (cw->clip.report_callbacks && child) {
      XawPannerReport prep;
      prep.changed       = changed;
      prep.slider_x      = -child->core.x;	/* clip is "inner" */
      prep.slider_y      = -child->core.y;	/* child is outer since it is larger */
      prep.slider_width  = cw->core.width;
      prep.slider_height = cw->core.height;
      prep.canvas_width  = child->core.width;
      prep.canvas_height = child->core.height;
      IIMESSAGE(prep.slider_x,prep.slider_y)
      XtCallCallbackList ((Widget)cw, cw->clip.report_callbacks,(XtPointer) &prep);
   }
   ENDMESSAGE(SendReport)
}

/*****************************************************************************
 *                                                                           *
 *			 Clip Widget Class Methods                       *
 *                                                                           *
 *****************************************************************************/


static void Realize (gw, valueMask, attributes)
    register Widget gw;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{

   BEGINMESSAGE(Realize)
   SMESSAGE(XtName(gw))
    attributes->bit_gravity = NorthWestGravity;
    *valueMask |= CWBitGravity;

    if (gw->core.width < 1) gw->core.width = 1;
    if (gw->core.height < 1) gw->core.height = 1;
    (*clipWidgetClass->core_class.superclass->core_class.realize)
	(gw, valueMask, attributes);
   ENDMESSAGE(Realize)
}

/*---------------------------------------------------*/
/* Initialize */
/*---------------------------------------------------*/

static void
Initialize(Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
   ClipWidget cw = (ClipWidget) new;

   BEGINMESSAGE(Initialize)
   SMESSAGE(XtName(new))
   if (CW_CORE.width == 0)  CW_CORE.width  = 1;  
   if (CW_CORE.height == 0) CW_CORE.height = 1;
   CW_CHILD_NAT_WIDTH = CW_CHILD_NAT_HEIGHT = CW_CHILD_NAT_BORDER =
   CW_CHILD_NAT_X = CW_CHILD_NAT_Y = 0;
   ENDMESSAGE(Initialize)
}

#if 0
/*---------------------------------------------------*/
/* SetValues */
/*---------------------------------------------------*/

static Boolean
SetValues(current, request, new)
  Widget current, request, new;
{
  ClipWidget cw = (ClipWidget) new;
  ClipWidget cc = (ClipWidget) current;
  if (cw->clip.child_x != cc->clip.child_x ||
      cw->clip.child_y != cc->clip.child_y) {
    ClipWidgetSetCoordinates(new,cw->clip.child_x,cw->clip.child_y);
  }
  return(False);
}
#endif

/*---------------------------------------------------*/
/* Resize */
/*---------------------------------------------------*/

static void Resize (Widget w)
{
   ClipWidget cw = (ClipWidget) w;
   Widget child  = CW_CHILD;

   BEGINMESSAGE(Resize)
   SMESSAGE(XtName(w))
   if (child) {
      Position minx, miny;
      Position x = CW_CHILD_NAT_X;
      Position y = CW_CHILD_NAT_Y;
      Dimension width  = CW_CHILD_NAT_WIDTH;
      Dimension height = CW_CHILD_NAT_HEIGHT;

      INFIIMESSAGE(size of clip,cw->core.width,cw->core.height)
      INFIIMESSAGE(current size of child,width,height)
      if (width  < cw->core.width)   width  = cw->core.width;
      if (height < cw->core.height)  height = cw->core.height;
      minx = ((Position) cw->core.width)  - ((Position) width);
      miny = ((Position) cw->core.height) - ((Position) height);
      if (x < minx) x = minx;
      if (y < miny) y = miny;
      if (x > 0) x = 0;
      if (y > 0) y = 0;
      INFIIMESSAGE(resizing child to,width,height)
      XtConfigureWidget (CW_CHILD, x, y, width, height, (Dimension) 0);
   }
   SendReport (cw, (unsigned int) (XawPRCanvasWidth | XawPRCanvasHeight));

   ENDMESSAGE(Resize)
}

/*---------------------------------------------------*/
/* GetDesiredSizeOfChild */
/*---------------------------------------------------*/

static void
GetDesiredSizeOfChild(Widget child)
{
   ClipWidget cw;
   XtWidgetGeometry desired;

   BEGINMESSAGE(GetDesiredSizeOfChild)
   cw = (ClipWidget) XtParent(child);
   XtQueryGeometry (child, (XtWidgetGeometry *)NULL, &desired);
   CW_CHILD_NAT_BORDER = desired.border_width;
   CW_CHILD_NAT_WIDTH  = desired.width;
   CW_CHILD_NAT_HEIGHT = desired.height;
   CW_CHILD_NAT_X      = desired.x;
   CW_CHILD_NAT_Y      = desired.y;
   IIMESSAGE(CW_CHILD_NAT_WIDTH,CW_CHILD_NAT_HEIGHT) IMESSAGE(CW_CHILD_NAT_BORDER)
   ENDMESSAGE(GetDesiredSizeOfChild)
}

/*---------------------------------------------------*/
/* InsertChild */
/*---------------------------------------------------*/

static void
InsertChild(Widget child)
{
   BEGINMESSAGE(InsertChild)
   SMESSAGE(XtName(child))
   (*SuperClass->composite_class.insert_child) (child);
   GetDesiredSizeOfChild(child);
   ENDMESSAGE(InsertChild)
}

/*---------------------------------------------------*/
/* GetNaturalSize */
/*---------------------------------------------------*/

static void
GetNaturalSize(ClipWidget cw, Dimension *wP, Dimension *hP)
{
   BEGINMESSAGE(GetNaturalSize)
   *wP = CW_CHILD_NAT_WIDTH;
   *hP = CW_CHILD_NAT_HEIGHT;
   ENDMESSAGE(GetNaturalSize)
}

/*---------------------------------------------------*/
/* QueryGeometry */
/*---------------------------------------------------*/

static XtGeometryResult
QueryGeometry(Widget w, XtWidgetGeometry *request, XtWidgetGeometry *preferred_return)
{
   ClipWidget cw = (ClipWidget)w;
   Dimension nw,nh;

   BEGINMESSAGE(QueryGeometry)
   SMESSAGE(XtName(w))
   if (CW_CHILD_P && CW_CHILD) GetDesiredSizeOfChild(CW_CHILD);
   GetNaturalSize(cw,&nw,&nh);
   preferred_return->request_mode = (CWWidth|CWHeight);
   preferred_return->width  = nw;
   preferred_return->height = nh; 
   if (    !(request->request_mode & CWWidth)
        || !(request->request_mode & CWHeight) 
        || ((request->request_mode & CWWidth)  && nw != request->width)
        || ((request->request_mode & CWHeight) && nh != request->height)
      ) {
      INFMESSAGE(XtGeometryAlmost) ENDMESSAGE(QueryGeometry)
      return XtGeometryAlmost;
   }
   if ((nw == w->core.width) && (nh == w->core.height)) {
      INFMESSAGE(XtGeometryNo) ENDMESSAGE(QueryGeometry)
      return XtGeometryNo;
   }

   INFMESSAGE(XtGeometryYes) ENDMESSAGE(QueryGeometry)
   return XtGeometryYes;
}

/*---------------------------------------------------*/
/* GeometryManager */
/*---------------------------------------------------*/

#define IS_REQUEST(fff) (request->request_mode & fff)

static XtGeometryResult
GeometryManager(Widget child, XtWidgetGeometry *request, XtWidgetGeometry *geometry_return _GL_UNUSED)
{
   ClipWidget cw;
   XtGeometryResult answer;
   int changed;

   BEGINMESSAGE(GeometryManager)

   INFSMESSAGE(received request from child, XtName(child))

   if (!(request->request_mode & (CWWidth | CWHeight | CWBorderWidth))) {
      INFMESSAGE(request not of interest) ENDMESSAGE(GeometryManager)
      return XtGeometryYes;
   }
   if (request->request_mode & XtCWQueryOnly) {
      /* query requests are not properly implemented ... ###jp### */
      INFMESSAGE(request is query only and will be denied) ENDMESSAGE(GeometryManager)
      return XtGeometryNo;
   }
   INFIIMESSAGE(current size of child:,child->core.width,child->core.height)

   cw = (ClipWidget) XtParent(child);
   changed = 0;
   if (IS_REQUEST(CWBorderWidth)) {
      IIMESSAGE(request->border_width,child->core.border_width)
      CW_CHILD_NAT_BORDER = request->border_width;
      if (CW_CHILD_NAT_BORDER != child->core.border_width) changed = 1;
   }
   if (IS_REQUEST(CWWidth)) {
      IIMESSAGE(request->width,child->core.width)
      CW_CHILD_NAT_WIDTH = request->width;
      if (CW_CHILD_NAT_WIDTH != child->core.width) changed = 1;
   }
   if (IS_REQUEST(CWHeight)) {
      IIMESSAGE(request->height,child->core.height)
      CW_CHILD_NAT_HEIGHT = request->height;
      if (CW_CHILD_NAT_HEIGHT != child->core.height) changed = 1;
   }
   if (IS_REQUEST(CWX)) {
      IIMESSAGE(request->x,child->core.x)
      CW_CHILD_NAT_X = request->x;
      if (CW_CHILD_NAT_X != child->core.x) changed = 1;
   }
   if (IS_REQUEST(CWY)) {
      IIMESSAGE(request->y,child->core.y)
      CW_CHILD_NAT_Y = request->y;
      if (CW_CHILD_NAT_Y != child->core.y) changed = 1;
   }

   if (changed) {
      answer = LayoutClip(cw);
      INFIIMESSAGE(new size of child:,child->core.width,child->core.height)
      INFIIMESSAGE(new position of child:,child->core.x,child->core.y)
      INFIMESSAGE(new border width of child:,child->core.border_width)
      ENDMESSAGE(GeometryManager)
      return answer;
   } else {
      ENDMESSAGE(GeometryManager)
      return XtGeometryYes;
   }
}

/*---------------------------------------------------*/
/* ChangeManaged */
/*---------------------------------------------------*/

static void ChangeManaged(Widget w)
{
   ClipWidget cw = (ClipWidget) w;
   Widget child = CW_CHILD;

   BEGINMESSAGE(ChangeManaged)
   SMESSAGE(XtName(w))
   if (child) {
      if (!XtIsRealized (w)) {
         XtWidgetGeometry geom, retgeom;
         geom.request_mode = 0;
         if (cw->core.width == 0) {
            geom.width = child->core.width;
            geom.request_mode |= CWWidth;
	 }
	 if (cw->core.height == 0) {
            geom.height = child->core.height;
            geom.request_mode |= CWHeight;
         }
         if (geom.request_mode &&
            XtMakeGeometryRequest (w, &geom, &retgeom) == XtGeometryAlmost) {
	       (void) XtMakeGeometryRequest (w, &retgeom, (XtWidgetGeometry *)NULL);
	 }
      }
      XtResizeWidget (child, Max (child->core.width,  cw->core.width),
                             Max (child->core.height, cw->core.height), 0);
      SendReport (cw, (unsigned int) XawPRAll);
   }
   ENDMESSAGE(ChangeManaged)
}

/*---------------------------------------------------*/
/* LayoutClip */
/*---------------------------------------------------*/

static XtGeometryResult
LayoutClip(ClipWidget cw)
{
   XtWidgetGeometry request;
   XtGeometryResult answer;

   BEGINMESSAGE(LayoutClip)

   GetNaturalSize(cw,&request.width,&request.height);

   if (request.width != cw->core.width || request.height != cw->core.height) {
      request.request_mode = (CWWidth | CWHeight);
      INFMESSAGE(will request new geometry from parent)
      answer = XtMakeGeometryRequest((Widget) cw, &request, &request);
      switch (answer) {
         case XtGeometryYes:
            INFMESSAGE(XtGeometryYes)
            INFMESSAGE(parent reconfigured window)
            break;
         case XtGeometryAlmost:
            INFIIMESSAGE(XtGeometryAlmost:,request.width,request.height)
            INFMESSAGE(requesting approval of these values)
            answer = XtMakeGeometryRequest((Widget) cw, &request, &request);
            if (answer!=XtGeometryYes) {
               INFIIMESSAGE(parent proposes,request.width,request.height)
               fprintf(stderr,"ClipWidget: Warning, parent didn't accept the size he proposed.");
               INFMESSAGE(giving up)
               answer = XtGeometryNo;
            } else {
               INFMESSAGE(XtGeometryYes)
               INFMESSAGE(parent reconfigured window)
            }
            break;
         case XtGeometryNo:
            INFMESSAGE(XtGeometryNo)
            answer = XtGeometryNo;
            break;
         case XtGeometryDone:
            INFMESSAGE(XtGeometryDone)
	    /* never reached */
            break;
      }
      if (answer == XtGeometryYes) {
         Resize((Widget)cw);
         answer = XtGeometryDone;
      }
   } else {
     INFMESSAGE(XtGeometryDone)
     Resize((Widget)cw);
     answer = XtGeometryDone;
   }

   ENDMESSAGE(LayoutClip)
   return answer;
}

/*###################################################*/
/* ClipWidgetCheckCoordinates */
/*###################################################*/

void
ClipWidgetCheckCoordinates(Widget w, int x, int y, int *xP, int *yP)
{
  ClipWidget cw = (ClipWidget)w;
  Widget  child = CW_CHILD;


  BEGINMESSAGE(ClipWidgetCheckCoordinates)
  *xP = x;
  *yP = y;
  if (*xP + (int)child->core.width < (int)cw->core.width) 
    *xP = (int)cw->core.width-(int)child->core.width;
  if (*xP > 0) *xP = 0;
  if (*yP + (int)child->core.height < (int)cw->core.height) 
    *yP = (int)cw->core.height-(int)child->core.height;
  if (*yP > 0) *yP = 0;
  ENDMESSAGE(ClipWidgetCheckCoordinates)
}

/*###################################################*/
/* ClipWidgetSetCoordinates */
/*###################################################*/

void
ClipWidgetSetCoordinates(Widget w, int x, int y)
{
   ClipWidget cw = (ClipWidget)w;
   Widget  child = CW_CHILD;
   int px,py;

   BEGINMESSAGE(ClipWidgetSetCoordinates)
   ClipWidgetCheckCoordinates(w, x, y, &px,&py);
   XtMoveWidget(child, (Position)px, (Position)py);
   CW_CHILD_NAT_X = (Position)px;
   CW_CHILD_NAT_Y = (Position)py;
   SendReport (cw, (XawPRSliderX | XawPRSliderY));
   ENDMESSAGE(ClipWidgetSetCoordinates)
}
