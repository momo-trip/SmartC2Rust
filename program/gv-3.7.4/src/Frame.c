/*
**
** Frame.c -- File composition widget
**
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
** Author:   Jose E. Marchesi (jemarch@gnu.org)
**           GNU Project
**
*/
#include "ac_config.h"

#include <stdio.h>

#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XMU(Converters.h)
#include INC_XMU(CharSet.h)
#include INC_XAW(XawInit.h)
#include "FrameP.h"

/*
#define MESSAGES
#define MESSAGES1
*/
#include "message.h"

/****************************************************************
 *
 * Frame Resources
 *
 ****************************************************************/
#define offset(name) XtOffsetOf(FrameRec, frame.name)
static XtResource resources[] = {
    {XtNhSpace, XtCHSpace, XtRDimension, sizeof(Dimension),
        offset(h_space_nat), XtRImmediate, (XtPointer)4 },
    {XtNvSpace, XtCVSpace, XtRDimension, sizeof(Dimension),
        offset(v_space_nat), XtRImmediate, (XtPointer)4 },
    {XtNframeType, XtCFrameType, XtRFrameType, sizeof(XawFrameType),
        offset(frame_type), XtRImmediate, (XtPointer) XawCHISELED },
    {XtNshadowWidth, XtCShadowWidth, XtRDimension, sizeof(Dimension),
	offset(shadow_width_nat), XtRImmediate, (XtPointer) 2},
    {XtNtopShadowPixel, XtCTopShadowPixel, XtRPixel, sizeof(Pixel),
	offset(top_shadow_pixel), XtRString, XtDefaultForeground},
    {XtNbottomShadowPixel, XtCBottomShadowPixel, XtRPixel, sizeof(Pixel),
	offset(bot_shadow_pixel), XtRString, XtDefaultForeground},
    {XtNresize, XtCBoolean, XtRBoolean, sizeof(Boolean),
	offset(resize), XtRImmediate, (XtPointer) True},
};
#undef offset

#define FW_FRAME       fw->frame
#define FW_CORE        fw->core
#define FW_COMPOSITE   fw->composite
  
#define FW_HSPACE      FW_FRAME.h_space
#define FW_VSPACE      FW_FRAME.v_space
#define FW_SHADOW      FW_FRAME.shadow_width
#define FW_RESIZE      FW_FRAME.resize

#define FW_NAT_HSPACE  FW_FRAME.h_space_nat
#define FW_NAT_VSPACE  FW_FRAME.v_space_nat
#define FW_NAT_SHADOW  FW_FRAME.shadow_width_nat
#define FW_CHILD_NAT_WIDTH FW_FRAME.child_width_nat
#define FW_CHILD_NAT_HEIGHT FW_FRAME.child_height_nat
#define FW_CHILD_NAT_BORDER FW_FRAME.child_border_nat

#define FW_CHILD_P     FW_COMPOSITE.children
#define FW_CHILD       (*(FW_CHILD_P))

/***************************************************************************
 *
 * Frame  class record
 *
 ***************************************************************************/

static void ClassInitialize(void);
static void Resize(Widget);
static void Redisplay(Widget,XEvent *,Region);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void InsertChild(Widget);
static void ChangeManaged(Widget);
static XtGeometryResult GeometryManager(Widget,XtWidgetGeometry*,XtWidgetGeometry*);
static XtGeometryResult QueryGeometry(Widget,XtWidgetGeometry*,XtWidgetGeometry*);
static XtGeometryResult LayoutFrame(FrameWidget);
static void Destroy(Widget);
static void GetDesiredSizeOfChild(Widget);
static void GetNaturalSize(FrameWidget,Dimension*,Dimension*);

#define SuperClass ((CompositeWidgetClass)&compositeClassRec)

FrameClassRec frameClassRec = {
  {
    /* superclass	  */	(WidgetClass)SuperClass,
    /* class_name	  */	"Frame",
    /* size		  */	sizeof(FrameRec),
    /* class_initialize	  */	ClassInitialize,
    /* class_part_init    */	NULL,
    /* class inited	  */	FALSE,
    /* initialize	  */	Initialize,
    /* initialize_hook	  */	NULL,		
    /* realize		  */	XtInheritRealize,
    /* actions		  */	NULL,
    /* num_actions	  */	0,
    /* resources	  */	resources,
    /* resource_count	  */	XtNumber(resources),
    /* xrm_class	  */	NULLQUARK,
    /* compress_motion	  */	FALSE,
    /* compress_exposure  */	XtExposeCompressMultiple,
    /* compress_enterleave*/	FALSE,
    /* visible_interest	  */	FALSE,
    /* destroy		  */	Destroy,
    /* resize		  */	Resize,
    /* expose		  */	Redisplay,
    /* set_values	  */	NULL,
    /* set_values_hook	  */	NULL,			
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook	  */	NULL,
    /* accept_focus	  */	NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets	  */	NULL,
    /* tm_table		  */	NULL,
    /* query_geometry	  */	QueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension	  */	NULL
  },
  { /* composite_class fields */
    /* geometry_manager	  */	GeometryManager,
    /* change_managed	  */	ChangeManaged,
    /* insert_child	  */	InsertChild,
    /* delete_child	  */	XtInheritDeleteChild,
    /* extension	  */	NULL
  },
  { /* frame_class fields */
    /* dummy              */    NULL
  }
};

WidgetClass frameWidgetClass =	(WidgetClass) (&frameClassRec);

/****************************************************************
 * Private Routines
 ****************************************************************/

/*---------------------------------------------------*/
/* FrameConvertStringToFrameType */
/*---------------------------------------------------*/

#define done(type, value)  {			\
   if (to->addr != NULL) {			\
      if (to->size < sizeof(type)) {		\
	 to->size = sizeof(type);		\
	 return False;				\
      }						\
      *(type*)(to->addr) = (value);		\
   } else {					\
      static type static_val;			\
      static_val = (value);			\
      to->addr = (XtPointer)&static_val;	\
   }						\
   to->size = sizeof(type);			\
   ENDMESSAGE1(FrameConvertStringToFrameType)	\
   return True;					\
}

Boolean
FrameConvertStringToFrameType(Display *display, XrmValue *args _GL_UNUSED, Cardinal *num_args _GL_UNUSED, XrmValuePtr from, XrmValuePtr to, XtPointer *converter_data _GL_UNUSED)
{
   String s = (String) from->addr;

   BEGINMESSAGE1(FrameConvertStringToFrameType)
   if (XmuCompareISOLatin1(s, "raised")   == 0) done(XawFrameType, XawRAISED);
   if (XmuCompareISOLatin1(s, "sunken")   == 0) done(XawFrameType, XawSUNKEN);
   if (XmuCompareISOLatin1(s, "chiseled") == 0) done(XawFrameType, XawCHISELED);
   if (XmuCompareISOLatin1(s, "ledged")   == 0) done(XawFrameType, XawLEDGED);
   if (XmuCompareISOLatin1(s, "massiveRaised") == 0) done(XawFrameType,XawFrameMassiveRaised);
   XtDisplayStringConversionWarning(display, s, XtRFrameType);
   done(XawFrameType, XawRAISED);
}

/*---------------------------------------------------*/
/* ClassInitialize */
/*---------------------------------------------------*/

static void
ClassInitialize(void)
{
   BEGINMESSAGE(ClassInitialize)
   XawInitializeWidgetSet();
   XtSetTypeConverter(XtRString, XtRFrameType, FrameConvertStringToFrameType,
		     (XtConvertArgList)NULL, 0, XtCacheNone, NULL);
   ENDMESSAGE(ClassInitialize)
}

/*---------------------------------------------------*/
/* Initialize */
/*---------------------------------------------------*/

static GC
shadow_getGC (Widget w, Pixel pixel)
{
   Screen    *scn = XtScreen (w);
   XtGCMask  valuemask;
   XGCValues myXGCV;
   GC        gc;

   BEGINMESSAGE(shadow_getGC)
   if (DefaultDepthOfScreen(scn) > 1) {
      valuemask = GCForeground;
      myXGCV.foreground = pixel;
      gc = XtGetGC(w, valuemask, &myXGCV);
   }
   else gc = (GC) NULL;
   ENDMESSAGE(shadow_getGC)
   return gc;
}

static void
Initialize(Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
   FrameWidget fw = (FrameWidget) new;

   BEGINMESSAGE(Initialize)
   FW_FRAME.top_shadow_GC = shadow_getGC(new,FW_FRAME.top_shadow_pixel);
   FW_FRAME.bot_shadow_GC = shadow_getGC(new,FW_FRAME.bot_shadow_pixel);
   if (!FW_FRAME.top_shadow_GC || !FW_FRAME.bot_shadow_GC) {
      INFMESSAGE(disallowing shadows)
      FW_FRAME.shadow_width_nat = 0;
   }

   if (FW_CORE.width == 0)  FW_CORE.width  = 1;  
   if (FW_CORE.height == 0) FW_CORE.height = 1;
   FW_SHADOW = FW_HSPACE = FW_VSPACE = 0;
   FW_CHILD_NAT_WIDTH = FW_CHILD_NAT_HEIGHT = FW_CHILD_NAT_BORDER = 0;

   ENDMESSAGE(Initialize)
}

/*---------------------------------------------------*/
/* Destroy */
/*---------------------------------------------------*/

static void
Destroy (Widget w)
{
   FrameWidget fw = (FrameWidget) w;

   BEGINMESSAGE(Destroy)
   XtReleaseGC(w,FW_FRAME.top_shadow_GC);
   XtReleaseGC(w,FW_FRAME.bot_shadow_GC);
   ENDMESSAGE(Destroy)
}

/*---------------------------------------------------*/
/* Resize */
/*---------------------------------------------------*/

#define MIN_CHILD 4

static void
Resize(Widget w)
{
   int x,y,cw,ch,hs,vs,bw,sw;
   FrameWidget fw = (FrameWidget) w;

   BEGINMESSAGE(Resize)

   hs  = (int)(FW_NAT_HSPACE);
   vs  = (int)(FW_NAT_VSPACE);
   sw  = (int)(FW_NAT_SHADOW);
   bw  = (int)(FW_CHILD_NAT_BORDER);

   ch  = (int)(FW_CORE.height) - 2*vs -2*sw;
   cw  = (int)(FW_CORE.width)  - 2*hs -2*sw;

   if ((ch<MIN_CHILD || cw<MIN_CHILD) && (hs>0||vs>0||sw>0)) {
      INFMESSAGE(adjusting size of decorations)
      while (ch<MIN_CHILD && vs>0) { ch +=2; --vs; }
      while (cw<MIN_CHILD && hs>0) { cw +=2; --hs; }
      while ((cw<MIN_CHILD || ch<MIN_CHILD) && sw>0) { ch+=2; cw +=2; --sw; }
   }

   if (cw <= 0 || ch <= 0) {
      INFMESSAGE(child too small; will place it off screen)
      cw = ch = 1;
      sw = hs = vs = 0;
      x = -1 - 2*bw;
      y = -1 - 2*bw;
   } else {
      x = hs+sw-bw;
      y = vs+sw-bw;
   }
   IIMESSAGE(hs,vs)
   IIMESSAGE(cw,ch)
   IIMESSAGE(bw,sw)

   FW_HSPACE= (Dimension) hs;
   FW_VSPACE= (Dimension) vs;
   FW_SHADOW= (Dimension) sw;

   XtConfigureWidget(FW_CHILD,x,y,((Dimension)cw),((Dimension)ch),((Dimension)bw));

   ENDMESSAGE(Resize)
}

/*---------------------------------------------------*/
/* Redisplay */
/*---------------------------------------------------*/

#define topPolygon(i,xx,yy)		\
  top_polygon[i].x = (short) (xx);	\
  top_polygon[i].y = (short) (yy)

#define bottomPolygon(i,xx,yy)		\
  bottom_polygon[i].x = (short) (xx);	\
  bottom_polygon[i].y = (short) (yy)

void
FrameDrawFrame (Widget gw, int x, int y, int w, int h, XawFrameType frame_type, int fw, GC lightgc, GC darkgc)
{
   XPoint top_polygon[6];
   XPoint bottom_polygon[6];
  
   BEGINMESSAGE1(FrameDrawFrame)

   if (lightgc == (GC)NULL ){
      XtWarning("FrameDrawFrame: lightgc is NULL.");
      ENDMESSAGE1(FrameDrawFrame)
      return;
   }
   if (darkgc == (GC)NULL ){
      XtWarning("FrameDrawFrame: darkgc is NULL.");
      ENDMESSAGE1(FrameDrawFrame)
      return;
   }
   if (!XtIsRealized(gw)) {
      XtWarning("FrameDrawFrame: widget is not realized.");
      ENDMESSAGE1(FrameDrawFrame)
      return;
   }
  
   if (frame_type == XawRAISED || frame_type == XawSUNKEN ) {
      topPolygon (0,x    ,y    ); bottomPolygon (0,x+w  ,y+h    ); 
      topPolygon (1,x+w  ,y    ); bottomPolygon (1,x    ,y+h    );
      topPolygon (2,x+w-fw,y+fw); bottomPolygon (2,x+fw  ,y+h-fw);
      topPolygon (3,x+fw,y+fw  ); bottomPolygon (3,x+w-fw,y+h-fw);
      topPolygon (4,x+fw,y+h-fw); bottomPolygon (4,x+w-fw,y+fw  );
      topPolygon (5,x    ,y+h  ); bottomPolygon (5,x+w  ,y      );
      if (frame_type == XawSUNKEN) {
         XFillPolygon(XtDisplayOfObject(gw), XtWindowOfObject(gw), darkgc,
		      top_polygon, 6, Nonconvex, CoordModeOrigin);
         XFillPolygon(XtDisplayOfObject(gw), XtWindowOfObject(gw), lightgc,
	    	      bottom_polygon, 6, Nonconvex, CoordModeOrigin);
      } else {
	 XFillPolygon(XtDisplayOfObject(gw), XtWindowOfObject(gw), lightgc,
		      top_polygon, 6, Nonconvex, CoordModeOrigin);
	 XFillPolygon(XtDisplayOfObject(gw), XtWindowOfObject(gw), darkgc,
		      bottom_polygon, 6, Nonconvex, CoordModeOrigin);
      }
   }
   else if (frame_type == XawFrameMassiveRaised) {
     if (fw>=3) {
        int it,mt,ot;
        ot = 1;
        it = 1;
        mt = fw-ot-it;
        FrameDrawFrame(gw, x, y, w, h, XawRAISED, ot, lightgc, darkgc);
        FrameDrawFrame(gw,x+mt+ot,y+mt+ot,w-2*mt-2*ot,h-2*mt-2*ot,
		       XawSUNKEN, it, lightgc, darkgc);
     }
   }
   else if ( frame_type == XawLEDGED ) {
     int it,ot;
     it = ot = fw/2;
     if (fw&1) it += 1;
     FrameDrawFrame(gw, x, y, w, h, XawRAISED, ot, lightgc, darkgc);
     FrameDrawFrame(gw,x+ot, y+ot,w-2*ot, h-2*ot,
		  XawSUNKEN, it, lightgc, darkgc);
   }
   else if ( frame_type == XawCHISELED ) {
     int it,ot;
     it = ot = fw/2;
     if (fw&1) it += 1;
     FrameDrawFrame(gw, x, y, w, h, XawSUNKEN, ot, lightgc, darkgc);
     FrameDrawFrame(gw,x+ot,y+ot,w-2*ot, h-2*ot,
		    XawRAISED, it, lightgc, darkgc);
   }

   ENDMESSAGE1(FrameDrawFrame)

}
#undef topPolygon
#undef bottomPolygon

static void
Redisplay(Widget w, XEvent *event _GL_UNUSED, Region region _GL_UNUSED)
{
   FrameWidget fw = (FrameWidget) w;
   int wh,ww,sw,bw;

   BEGINMESSAGE1(Redisplay)

   bw = (int)FW_CORE.border_width;
   ww = (int)FW_CORE.width;
   wh = (int)FW_CORE.height;
   sw = (int)FW_SHADOW; 
   if (sw == 0 || 2*sw>ww || 2*sw>wh) {
      INFMESSAGE(not enough space to display anything) ENDMESSAGE1(Redisplay)
      return;
   }
   FrameDrawFrame(w,0,0,ww,wh,FW_FRAME.frame_type,sw,
		  FW_FRAME.top_shadow_GC,
		  FW_FRAME.bot_shadow_GC);
   ENDMESSAGE1(Redisplay)
}

/*---------------------------------------------------*/
/* GetDesiredSizeOfChild */
/*---------------------------------------------------*/

static void
GetDesiredSizeOfChild(Widget child)
{
   FrameWidget fw;

   BEGINMESSAGE(GetDesiredSizeOfChild)
   fw = (FrameWidget) XtParent(child);
   if (XtIsManaged(child)) {
      XtWidgetGeometry desired;
      INFSMESSAGE(is managed,XtName(child))
      XtQueryGeometry (child, (XtWidgetGeometry *)NULL, &desired);
      FW_CHILD_NAT_BORDER = desired.border_width;
      FW_CHILD_NAT_WIDTH  = desired.width;
      FW_CHILD_NAT_HEIGHT = desired.height;
   } else {
      INFSMESSAGE(not managed,XtName(child))
      FW_CHILD_NAT_BORDER = 0;
      FW_CHILD_NAT_WIDTH  = 0;
      FW_CHILD_NAT_HEIGHT = 0;
   }
   IIMESSAGE(FW_CHILD_NAT_WIDTH,FW_CHILD_NAT_HEIGHT)
   IMESSAGE(FW_CHILD_NAT_BORDER)
   ENDMESSAGE(GetDesiredSizeOfChild)
}

/*---------------------------------------------------*/
/* InsertChild */
/*---------------------------------------------------*/

static void
InsertChild(Widget child)
{
   BEGINMESSAGE(InsertChild)
   (*SuperClass->composite_class.insert_child) (child);
   GetDesiredSizeOfChild(child);
   ENDMESSAGE(InsertChild)
}

/*---------------------------------------------------*/
/* GetNaturalSize */
/*---------------------------------------------------*/

static void
GetNaturalSize(FrameWidget fw, Dimension *wP, Dimension *hP)
{
   BEGINMESSAGE(GetNaturalSize)
   *wP = FW_CHILD_NAT_WIDTH  + 2*FW_NAT_SHADOW +2*FW_NAT_HSPACE;
   *hP = FW_CHILD_NAT_HEIGHT + 2*FW_NAT_SHADOW +2*FW_NAT_VSPACE;
   ENDMESSAGE(GetNaturalSize)
}

/*---------------------------------------------------*/
/* ChangeManaged */
/*---------------------------------------------------*/

static void
ChangeManaged(Widget w)
{
   FrameWidget fw = (FrameWidget) w;

   BEGINMESSAGE(ChangeManaged)
   GetDesiredSizeOfChild(FW_CHILD);
   LayoutFrame(fw);
   ENDMESSAGE(ChangeManaged)
}

/*---------------------------------------------------*/
/* GeometryManager */
/*---------------------------------------------------*/

#define IS_REQUEST(fff) (request->request_mode & fff)

static XtGeometryResult
GeometryManager(Widget child, XtWidgetGeometry *request, XtWidgetGeometry *geometry_return _GL_UNUSED)
{
   FrameWidget fw;
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

   fw = (FrameWidget) XtParent(child);
   changed = 0;
   if (IS_REQUEST(CWBorderWidth)) {
      IIMESSAGE(request->border_width,child->core.border_width)
      FW_CHILD_NAT_BORDER = request->border_width;
      if (FW_CHILD_NAT_BORDER != child->core.border_width) changed = 1;
   }
   if (IS_REQUEST(CWWidth)) {
      IIMESSAGE(request->width,child->core.width)
      FW_CHILD_NAT_WIDTH = request->width;
      if (FW_CHILD_NAT_WIDTH != child->core.width) changed = 1;
   }
   if (IS_REQUEST(CWHeight)) {
      IIMESSAGE(request->height,child->core.height)
      FW_CHILD_NAT_HEIGHT = request->height;
      if (FW_CHILD_NAT_HEIGHT != child->core.height) changed = 1;
   }

   if (changed) {
      answer = LayoutFrame(fw);
      INFIIMESSAGE(new size of child:,child->core.width,child->core.height)
      INFIMESSAGE(new border width of child:,child->core.border_width)
      ENDMESSAGE(GeometryManager)
      return answer;
   } else {
      ENDMESSAGE(GeometryManager)
      return XtGeometryYes;
   }
}

/*---------------------------------------------------*/
/* QueryGeometry */
/*---------------------------------------------------*/

static XtGeometryResult
QueryGeometry(Widget w, XtWidgetGeometry *request, XtWidgetGeometry *preferred_return)
{
   FrameWidget fw = (FrameWidget)w;
   Dimension nw,nh;

   BEGINMESSAGE(QueryGeometry)
   if (fw->composite.children && fw->composite.children[0])
     GetDesiredSizeOfChild(fw->composite.children[0]);
   GetNaturalSize(fw,&nw,&nh);
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
/* LayoutFrame */
/*---------------------------------------------------*/

static XtGeometryResult
LayoutFrame(FrameWidget fw)
{
   XtWidgetGeometry request;
   XtGeometryResult answer;

   BEGINMESSAGE(LayoutFrame)

   GetNaturalSize(fw,&request.width,&request.height);

   if (FW_RESIZE && (request.width != fw->core.width || request.height != fw->core.height)) {
      request.request_mode = (CWWidth | CWHeight);
      INFMESSAGE(will request new geometry from parent)
      answer = XtMakeGeometryRequest((Widget) fw, &request, &request);
      switch (answer) {
         case XtGeometryYes:
            INFMESSAGE(XtGeometryYes)
            INFMESSAGE(parent reconfigured window)
            break;
         case XtGeometryAlmost:
            INFIIMESSAGE(XtGeometryAlmost:,request.width,request.height)
            INFMESSAGE(requesting approval of these values)
            answer = XtMakeGeometryRequest((Widget) fw, &request, &request);
            if (answer!=XtGeometryYes) {
               INFIIMESSAGE(parent proposes,request.width,request.height)
               fprintf(stderr,"FrameWidget: Warning, parent didn't accept the size he proposed.");
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
         Resize((Widget)fw);
         answer = XtGeometryDone;
      }
   } else {
     INFMESSAGE(XtGeometryDone)
     Resize((Widget)fw);
     answer = XtGeometryDone;
   }

   ENDMESSAGE(LayoutFrame)
   return answer;
}
