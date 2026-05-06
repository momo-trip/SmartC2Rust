/*
**
** Scrollbar.c
**
** Copyright (C) 1994, 1995, 1996, 1997 Johannes Plass
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
** Author:   Johannes Plass (plass@thep.physik.uni-mainz.de)
**           Department of Physics
**           Johannes Gutenberg-University
**           Mainz, Germany
**
**           Jose E. Marchesi (jemarch@gnu.org)
**           GNU Project
**
*/

#include "ac_config.h"
/*
#define MESSAGES
*/
#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <inttypes.h>

#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(XawInit.h)
#include INC_XMU(Drawing.h)
#include "ScrollbarP.h"
#include "actions.h"


/* Private definitions. */

static char defaultTranslations[] =
    "<Btn1Down>:   NotifyScroll()\n\
     <Btn2Down>:   MoveThumb() NotifyThumb() \n\
     <Btn3Down>:   NotifyScroll()\n\
     <Btn1Motion>: HandleThumb() HandleAutoscroll() \n\
     <Btn2Motion>: MoveThumb() NotifyThumb() \n\
     <Btn3Motion>: HandleThumb() \n\
     <BtnUp>:      EndScroll()";

static float floatZero = 0.0;

#define Offset(field) XtOffsetOf(ScrollbarRec, scrollbar.field)
static XtResource resources[] = {
  {XtNlength, XtCLength, XtRDimension, sizeof(Dimension),
       Offset(length), XtRImmediate, (XtPointer) 1},
  {XtNthickness, XtCThickness, XtRDimension, sizeof(Dimension),
       Offset(thickness), XtRImmediate, (XtPointer) 14},
  {XtNorientation, XtCOrientation, XtROrientation, sizeof(XtOrientation),
      Offset(orientation), XtRImmediate, (XtPointer) XtorientVertical},
  {XtNscrollProc, XtCCallback, XtRCallback, sizeof(XtPointer),
       Offset(scrollProc), XtRCallback, NULL},
  {XtNthumbProc, XtCCallback, XtRCallback, sizeof(XtPointer),
       Offset(thumbProc), XtRCallback, NULL},
  {XtNjumpProc, XtCCallback, XtRCallback, sizeof(XtPointer),
       Offset(jumpProc), XtRCallback, NULL},
  {XtNtopOfThumb, XtCTopOfThumb, XtRFloat, sizeof(float),
       Offset(top), XtRFloat, (XtPointer)&floatZero},
  {XtNthumb, XtCThumb, XtRBitmap, sizeof(Pixmap),
       Offset(thumb), XtRImmediate, (XtPointer) None},
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
       Offset(foreground), XtRString, XtDefaultForeground},
  {XtNshown, XtCShown, XtRFloat, sizeof(float),
       Offset(shown), XtRFloat, (XtPointer)&floatZero},
  {XtNminimumThumb, XtCMinimumThumb, XtRDimension, sizeof(Dimension),
       Offset(min_thumb), XtRImmediate, (XtPointer) 7},
  {XtNshowArrows, XtCShowArrows, XtRBoolean, sizeof(Boolean),
       Offset(show_arrows), XtRImmediate, (XtPointer) TRUE},
  {XtNthumbTopIndent, XtCThumbTopIndent, XtRDimension, sizeof(Dimension),
       Offset(thumb_top_indent), XtRImmediate, (XtPointer) 1},
  {XtNthumbSideIndent, XtCThumbSideIndent, XtRDimension, sizeof(Dimension),
       Offset(thumb_side_indent), XtRImmediate, (XtPointer) 0},
  {XtNarrowTopIndent, XtCArrowTopIndent, XtRDimension, sizeof(Dimension),
       Offset(arrow_top_indent), XtRImmediate, (XtPointer) 0},
  {XtNarrowSideIndent, XtCArrowSideIndent, XtRDimension, sizeof(Dimension),
       Offset(arrow_side_indent), XtRImmediate, (XtPointer) 0},
  {XtNuseDynamic, XtCUseDynamic, XtRBoolean, sizeof(Boolean),
       Offset(use_dynamic), XtRImmediate, (XtPointer) TRUE},
  {XtNinterval, XtCInterval, XtRInt, sizeof(int),
       Offset(interval), XtRImmediate, (XtPointer) 100},
  {XtNdelay, XtCDelay, XtRInt, sizeof(int),
       Offset(delay), XtRImmediate, (XtPointer) 300},
};
#undef Offset

static void ClassInitialize(void);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void Destroy(Widget);
static void Realize(Widget,XtValueMask*,XSetWindowAttributes*);
static void Resize(Widget);
static void Redisplay(Widget,XEvent*,Region);
static Boolean SetValues(Widget,Widget,Widget,ArgList,Cardinal*);

static void HandleThumb(Widget,XEvent*,String*,Cardinal*);
static void HandleAutoscroll(Widget,XEvent*,String*,Cardinal*);
static void MoveThumb(Widget,XEvent*,String*,Cardinal*);
static void NotifyThumb(Widget,XEvent*,String*,Cardinal*);
static void NotifyScroll(Widget,XEvent*,String*,Cardinal*);
static void EndScroll(Widget,XEvent*,String*,Cardinal*);

static XtActionsRec actions[] = {
    {"HandleThumb",	HandleThumb},
    {"HandleAutoscroll",HandleAutoscroll},
    {"MoveThumb",	MoveThumb},
    {"NotifyThumb",	NotifyThumb},
    {"NotifyScroll",	NotifyScroll},
    {"EndScroll",	EndScroll}
};

ScrollbarClassRec scrollbarClassRec = {
  { /* core fields */
    /* superclass       */	(WidgetClass) &threeDClassRec,
    /* class_name       */	"Scrollbar",
    /* size             */	sizeof(ScrollbarRec),
    /* class_initialize	*/	ClassInitialize,
    /* class_part_init  */	NULL,
    /* class_inited	*/	FALSE,
    /* initialize       */	Initialize,
    /* initialize_hook  */	NULL,
    /* realize          */	Realize,
    /* actions          */	actions,
    /* num_actions	*/	XtNumber(actions),
    /* resources        */	resources,
    /* num_resources    */	XtNumber(resources),
    /* xrm_class        */	NULLQUARK,
    /* compress_motion	*/	TRUE,
    /* compress_exposure*/	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */	FALSE,
    /* destroy          */	Destroy,
    /* resize           */	Resize,
    /* expose           */	Redisplay,
    /* set_values       */	SetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost*/	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */	NULL,
    /* version          */	XtVersion,
    /* callback_private */	NULL,
    /* tm_table         */	defaultTranslations,
    /* query_geometry	*/	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension        */	NULL
  },
  { /* simple fields */
    /* change_sensitive	*/	XtInheritChangeSensitive
  },
  { /* threeD fields */
    /* shadowdraw	*/	XtInheritXaw3dShadowDraw
  },
  { /* scrollbar fields */
    /* ignore		*/	0 /*###jp### changed 21.10.94*/
  }

};

WidgetClass scrollbarWidgetClass = (WidgetClass)&scrollbarClassRec;

/*###############################################################################
    PREPROCESSOR INSTRUCTIONS
###############################################################################*/

#if defined(MIN)
#   undef MIN
#endif
#if defined(MAX)
#   undef MAX
#endif
#define MIN(_x_,_y_)	((_x_)<(_y_)?(_x_):(_y_))
#define MAX(_x_,_y_)	((_x_)>(_y_)?(_x_):(_y_))
#define PUT_IN_RANGE(min,num,max) MAX((min),MIN((num),(max)))

#define SCROLLBARWIDGET		ScrollbarWidget sbw = (ScrollbarWidget)

#define IS_REALIZED		(XtIsRealized((Widget)sbw)) /*###jp### changed 21.10.94*/
#define IS_HORIZONTAL		(sbw->scrollbar.orientation == XtorientHorizontal)
#define IS_VERTICAL		(sbw->scrollbar.orientation == XtorientVertical)
#define DISPLAY			XtDisplay(sbw)
#define WINDOW			XtWindow(sbw)

#define SBW			sbw
/* public */
#define WIDTH			sbw->core.width
#define HEIGHT			sbw->core.height
#define BACKGROUND_PIXEL	sbw->core.background_pixel
#define CURSOR_NAME		sbw->simple.cursor_name
#define SHADOW			sbw->threeD.shadow_width
#define BOT_SHADOW_GC		sbw->threeD.bot_shadow_GC
#define TOP_SHADOW_GC		sbw->threeD.top_shadow_GC

#define FOREGROUND		sbw->scrollbar.foreground
#define MIN_THUMB		sbw->scrollbar.min_thumb
#define SCROLLBAR_GC		sbw->scrollbar.gc
#define THUMB			sbw->scrollbar.thumb
#define LENGTH			sbw->scrollbar.length
#define SHOWN			sbw->scrollbar.shown
#define SHOWNLENGTH		sbw->scrollbar.shownLength
#define TOP			sbw->scrollbar.top
#define TOPLOC			sbw->scrollbar.topLoc
#define THICKNESS		sbw->scrollbar.thickness
#define THUMB_SIDE_INDENT	sbw->scrollbar.thumb_side_indent
#define THUMB_TOP_INDENT	sbw->scrollbar.thumb_top_indent
#define SHOW_ARROWS		sbw->scrollbar.show_arrows
#define ARROW_TOP_INDENT	sbw->scrollbar.arrow_top_indent
#define ARROW_SIDE_INDENT	sbw->scrollbar.arrow_side_indent
#define INTERVAL		sbw->scrollbar.interval
#define DELAY			sbw->scrollbar.delay

/* private */
#define TIMER			sbw->scrollbar.timer_id
#define DYNAMIC			sbw->scrollbar.dynamic
#define SCROLLMODE		sbw->scrollbar.scroll_mode
#define SHADOW_BASE		sbw->scrollbar.shadow_base
#define ARROW_WIDTH		sbw->scrollbar.arrow_width
#define ARROW_HEIGHT		sbw->scrollbar.arrow_height
#define ARROW_TOP_TO_BORDER	sbw->scrollbar.arrow_top_to_border
#define ARROW_TO_BORDER		sbw->scrollbar.arrow_to_border
#define ARROW_SHADOW		sbw->scrollbar.arrow_shadow
#define THUMB_SHADOW		sbw->scrollbar.thumb_shadow
#define THUMB_TO_ARROW		sbw->scrollbar.thumb_to_arrow
#define THUMB_TO_BORDER		sbw->scrollbar.thumb_to_border
#define USE_DYNAMIC		sbw->scrollbar.use_dynamic

#define USE_FILL		Display *dpy = DISPLAY; Window wndw = WINDOW
#define FILL(point,count,gc)       XFillPolygon(dpy,wndw,gc,point,count,Complex,CoordModeOrigin)
#define FILLrect(xc,yc,wid,hei,gc) XFillRectangle(dpy,wndw,gc,(int)(xc),(int)(yc),(unsigned int)(wid),(unsigned int)(hei))
#define DRAWline(xs,ys,xe,ye,gc)   XDrawLine(dpy,wndw,gc,(int)(xs),(int)(ys),(int)(xe),(int)(ye))
#define DRAWlines(point,count,gc)  XDrawLines(dpy,wndw,gc,point,count,CoordModeOrigin)
#define CLEAR(xc,yc,wid,hei)       XClearArea(dpy,wndw,(int)(xc),(int)(yc),(unsigned int)(wid),(unsigned int)(hei),FALSE)

#define SET_TIMER(widget,interval)                                  \
  	TIMER = XtAppAddTimeOut (                                   \
                  XtWidgetToApplicationContext(((Widget)(widget))), \
                  ((unsigned long)(interval)),                      \
                  RepeatNotify,                                     \
                  ((XtPointer)((Widget)(widget)))                   \
                )
#define DISABLED	((XtIntervalId) 0)
#define DESTROY_TIMER	if (TIMER) { XtRemoveTimeOut(TIMER); TIMER = DISABLED; }

#define CALLscrollProc(widget,data) \
           XtCallCallbacks ( ((Widget)(widget)),XtNscrollProc,((XtPointer)(intptr_t)(data)))

#define POINT(name,xcoord,ycoord)  name.x=(short)(xcoord);name.y=(short)(ycoord)

/*### Definitions related to the Scrollbar Geometry ###########################*/

#define  NICE_DIMENSION			5

#define _SCROLL_THICKNESS_		((int)THICKNESS-2*THUMB_TO_BORDER)
#define _SCROLL_LENGTH_			((int)LENGTH-2*(ARROW_TOP_TO_BORDER+ARROW_HEIGHT+THUMB_TO_ARROW)-1)

#define _RIGHT_END_OF_SCROLL_REGION_	((int)THICKNESS-1-THUMB_TO_BORDER)
#define _LEFT_START_OF_SCROLL_REGION_	(THUMB_TO_BORDER)
#define _START_OF_SCROLL_REGION_	(ARROW_TOP_TO_BORDER+ARROW_HEIGHT+THUMB_TO_ARROW)
#define _END_OF_SCROLL_REGION_		((int)LENGTH-1-ARROW_TOP_TO_BORDER-ARROW_HEIGHT-THUMB_TO_ARROW)

/*### Identificational Definitions ############################################*/

/* Scroll Mode */
#define NOSCROLL	0
#define CONTINUOUS	2
#define ARROW_UP	1
#define ARROW_DOWN	3
#define PAGE_UP		4
#define PAGE_DOWN	5

/* identifies upper/left, lower/right */
#define UPPER_PART	1
#define LOWER_PART	0

/* arrow identifier */
#define  UPPER_ARROW	0
#define  LOWER_ARROW	1
#define  BOTH_ARROWS	2
#define  NORMAL_ARROW	1
#define  INVERTED_ARROW	0

#define _SC_INIT_VALUE_ 9999

/*### Options at Compiling Time ###############################################*/

#define MIN_INTERVAL	8
#define MAX_INTERVAL	800
#define DYNAMIC_REGION	40.0
#define MIN_DELAY	10    /* the minimum delay between clicking on an arrow and */
                              /* the start of the autoscroll action                 */

#define ARROW_SCROLL_AMOUNT (MAX(100,LENGTH)/20)
#define PAGE_SCROLL_AMOUNT  LENGTH

/*###############################################################################
    EraseThumbRegion
###############################################################################*/

static void EraseThumbRegion(ScrollbarWidget SBW, Dimension yt, Dimension yb, Dimension xl, Dimension xr)
{
   USE_FILL;

   BEGINMESSAGE(EraseThumbRegion)
   if (IS_VERTICAL) { CLEAR(xl,yt,xr-xl+1,yb-yt+1); }
   else             { CLEAR(yt,xl,yb-yt+1,xr-xl+1); }
   ENDMESSAGE(EraseThumbRegion)
}

/*###############################################################################
    PaintThumbCover
###############################################################################*/

static void PaintThumbCover(ScrollbarWidget SBW, Dimension yt, Dimension yb,Dimension xl,Dimension xr)
{
   USE_FILL;

   BEGINMESSAGE(PaintThumbCover)
   if (IS_VERTICAL) FILLrect(xl,yt,xr+1-xl,yb-yt+1,SCROLLBAR_GC);
   else             FILLrect(yt,xl,yb-yt+1,xr+1-xl,SCROLLBAR_GC);
   ENDMESSAGE(PaintThumbCover)
}

/*###############################################################################
    PaintShadowAtSidesOfThumb
###############################################################################*/

static void PaintShadowAtSidesOfThumb(ScrollbarWidget SBW, Dimension yt, Dimension yb, Dimension xl, Dimension xr, Dimension s)
{
   USE_FILL;

   BEGINMESSAGE(PaintShadowAtSidesOfThumb)
   if (IS_VERTICAL) {
      FILLrect(xl,yt,s+1,yb-yt+1,TOP_SHADOW_GC); FILLrect(xr+1-s,yt,s,yb-yt+1,BOT_SHADOW_GC);
   }  else {
      FILLrect(yt,xl,yb-yt+1,s+1,TOP_SHADOW_GC); FILLrect(yt,xr+1-s,yb-yt+1,s,BOT_SHADOW_GC);
   }
   ENDMESSAGE(PaintShadowAtSidesOfThumb)
}

/*###############################################################################
    PaintShadowAtEndOfThumb
###############################################################################*/

static void PaintShadowAtEndOfThumb(ScrollbarWidget SBW, Dimension yt, Dimension yb, Dimension xl, Dimension xr, Dimension s, int part)
{
   XPoint shadowPT[4];
   USE_FILL;

   BEGINMESSAGE(PaintShadowAtEndOfThumb)

   if (IS_VERTICAL) {
      if (part==UPPER_PART) {
         POINT(shadowPT[3],xl,yt);                POINT(shadowPT[2],xr+1,yt);
              POINT(shadowPT[0],xl+s,yb);  POINT(shadowPT[1],xr-s+1,yb);
      } else {
              POINT(shadowPT[3],xl+s,yt+1);  POINT(shadowPT[2],xr-s+1,yt+1);
         POINT(shadowPT[0],xl,yb+1);                POINT(shadowPT[1],xr+1,yb+1);
      }
   } else {
      if (part==UPPER_PART) {
         POINT(shadowPT[0],yt,xl);
              POINT(shadowPT[1],yb,xl+s);
              POINT(shadowPT[2],yb,xr-s+1);
         POINT(shadowPT[3],yt,xr+1);
      } else {
              POINT(shadowPT[0],yb+1,xl);
         POINT(shadowPT[1],yt+1,xl+s);
         POINT(shadowPT[2],yt+1,xr-s+1);
              POINT(shadowPT[3],yb+1,xr+1);
      }
   }
   if (part==UPPER_PART) { FILL(shadowPT,4,TOP_SHADOW_GC); }
   else                  { FILL(shadowPT,4,BOT_SHADOW_GC); }

   ENDMESSAGE(PaintShadowAtEndOfThumb)
}

/*###############################################################################
    PaintWholeThumb
###############################################################################*/

static void PaintWholeThumb(ScrollbarWidget SBW, Dimension yt, Dimension yb, Dimension xl, Dimension xr, Dimension s)
{
   BEGINMESSAGE(PaintWholeThumb)
   if (s) { 
      PaintShadowAtSidesOfThumb(SBW,yt,yb,xl,xr,s);
      PaintShadowAtEndOfThumb(SBW,yt,yt+s,xl,xr,s,UPPER_PART);
      PaintShadowAtEndOfThumb(SBW,yb-s,yb,xl,xr,s,LOWER_PART);
   }
   PaintThumbCover(SBW,yt+s,yb-s,xl+s,xr-s);
   ENDMESSAGE(PaintWholeThumb)
}

/*###############################################################################
    PaintMiddleOfThumb
###############################################################################*/

static void PaintMiddleOfThumb(ScrollbarWidget SBW, Dimension yt, Dimension yb, Dimension xl, Dimension xr, Dimension s)
{
   BEGINMESSAGE(PaintMiddleOfThumb)
   if ((int)s) PaintShadowAtSidesOfThumb(SBW,yt,yb,xl,xr,s);
   PaintThumbCover(SBW,yt,yb,xl+s,xr-s);
   ENDMESSAGE(PaintMiddleOfThumb)
}

/*###############################################################################
    PaintEndOfThumb
###############################################################################*/

static void PaintEndOfThumb(ScrollbarWidget SBW, Dimension yt, Dimension yb, Dimension xl, Dimension xr, Dimension s, int part)
{
   BEGINMESSAGE(PaintEndOfThumb)
   if (part==UPPER_PART) {
      if (s) {
         PaintShadowAtSidesOfThumb(SBW,yt,yb,xl,xr,s);
         PaintShadowAtEndOfThumb(SBW,yt,yt+s,xl,xr,s,UPPER_PART);
      }
      PaintThumbCover(SBW,yt+s,yb,xl+s,xr-s);
   } else {
      if (s) {
         PaintShadowAtSidesOfThumb(SBW,yt,yb,xl,xr,s);
         PaintShadowAtEndOfThumb(SBW,yb-s,yb,xl,xr,s,LOWER_PART);
      }
      PaintThumbCover(SBW,yt,yb-s,xl+s,xr-s);
   }
   ENDMESSAGE(PaintEndOfThumb)
}

/*###############################################################################
    GetRelativeThumbLocation
###############################################################################*/

static void GetRelativeThumbLocation(ScrollbarWidget SBW, Dimension *newtop, Dimension *newbot)
{
   Dimension scroll_length = (Dimension) _SCROLL_LENGTH_-MIN_THUMB;
   Dimension end_of_scroll_region = (Dimension) _END_OF_SCROLL_REGION_;

   BEGINMESSAGE(GetRelativeThumbLocation)
   *newtop = (Dimension) (_START_OF_SCROLL_REGION_+scroll_length*TOP);
   *newtop = MIN(*newtop,end_of_scroll_region);
   *newbot = (Dimension)(*newtop+scroll_length*SHOWN);
   *newbot = MAX(*newbot,(Dimension)(*newtop+MIN_THUMB));
   *newbot = MIN(*newbot,end_of_scroll_region);
   IIMESSAGE(*newtop,*newbot)
   ENDMESSAGE(GetRelativeThumbLocation)
}

/*###############################################################################
    AdjustThumbShadow
###############################################################################*/

static Boolean AdjustThumbShadow(ScrollbarWidget SBW, Dimension newtop, Dimension newbot)
{
    Dimension xl = (Dimension) _LEFT_START_OF_SCROLL_REGION_;
    Dimension xr = (Dimension) _RIGHT_END_OF_SCROLL_REGION_;
    int s;
    Boolean changed = False;

    BEGINMESSAGE(AdjustThumbShadow)
    IIMESSAGE(newtop,newbot) IIMESSAGE(xl,xr) IMESSAGE(THUMB_SHADOW)
    s = MIN((int)(newbot-newtop+1),(int)(xr-xl+1))/2;
    s = MAX(0,s); s = MIN(s,(int)SHADOW);
    if (s != THUMB_SHADOW) {
       THUMB_SHADOW = s;
       INFIMESSAGE(had to adjust thumb shadow:,THUMB_SHADOW)
       changed = True;
    } else {
       INFMESSAGE(thumb shadow is just nice)
    }
    ENDMESSAGE(AdjustThumbShadow)
    return changed;
}

/*###############################################################################
    PaintThumb
###############################################################################*/

static void PaintThumb(ScrollbarWidget SBW)
{
    Dimension xl = (Dimension) _LEFT_START_OF_SCROLL_REGION_;
    Dimension xr = (Dimension) _RIGHT_END_OF_SCROLL_REGION_;
    Dimension oldtop, oldbot, newtop, newbot;

    BEGINMESSAGE(PaintThumb)

    if (!IS_REALIZED)                { INFMESSAGE(not realized)     ENDMESSAGE(PaintThumb) return; }
    if ((LENGTH<1) || (THICKNESS<1)) { INFMESSAGE(not enough space) ENDMESSAGE(PaintThumb) return; }

    GetRelativeThumbLocation(SBW,&newtop,&newbot);
    oldtop = TOPLOC; oldbot = oldtop + SHOWNLENGTH-1;
    IIMESSAGE(newtop,oldtop) IIMESSAGE(newbot,oldbot)
    IIMESSAGE(xr,xl) IMESSAGE(THUMB_SHADOW)

    if (AdjustThumbShadow(SBW,newtop,newbot)) {
       INFMESSAGE(doing full repaint with adjusted thumb shadow)
       if (oldbot>oldtop) EraseThumbRegion(SBW,oldtop,oldbot,xl,xr);
       if (newbot>newtop) PaintWholeThumb(SBW,newtop,newbot,xl,xr,(Dimension)THUMB_SHADOW);  
    } else {
       Dimension s = (Dimension) THUMB_SHADOW;
       if ((newtop >= oldbot) || (newbot <= oldtop)) {
          INFMESSAGE(doing full repaint)
          if (oldbot>oldtop) EraseThumbRegion(SBW,oldtop,oldbot,xl,xr);
          if (newbot>newtop) PaintWholeThumb(SBW,newtop,newbot,xl,xr,s);
       } else {
          INFMESSAGE(doing partial repaint)
          if (newtop > oldtop) EraseThumbRegion(SBW,oldtop,newtop-1,xl,xr);
          if (newbot < oldbot) EraseThumbRegion(SBW,newbot+1,oldbot,xl,xr);
          if (newtop < oldtop) PaintMiddleOfThumb(SBW,newtop,MIN(newbot,oldtop+s),xl,xr,s);
          if (newbot > oldbot) PaintMiddleOfThumb(SBW,MAX(newtop,oldbot-s),newbot,xl,xr,s);
          if ((newtop!=oldtop)&&(s)) PaintShadowAtEndOfThumb(SBW,newtop,newtop+s,xl,xr,s,UPPER_PART);
          if ((newbot!=oldbot)&&(s)) PaintShadowAtEndOfThumb(SBW,newbot-s,newbot,xl,xr,s,LOWER_PART);
       }
    }
    TOPLOC = newtop;
    SHOWNLENGTH = newbot-newtop+1;

    ENDMESSAGE(PaintThumb)
}

/*###############################################################################
    PaintArrows
###############################################################################*/

#define vPOLY_4(name,x0,y0,x1,y1,x2,y2,x3,y3)   \
      name[0].x=(short)(x0);name[0].y=(short)(y0);\
      name[1].x=(short)(x1);name[1].y=(short)(y1);\
      name[2].x=(short)(x2);name[2].y=(short)(y2);\
      name[3].x=(short)(x3);name[3].y=(short)(y3);\
      name[4].x=(short)(x0);name[4].y=(short)(y0)

#define hPOLY_4(name,x0,y0,x1,y1,x2,y2,x3,y3)   \
      name[0].x=(short)(y0);name[0].y=(short)(x0);\
      name[1].x=(short)(y1);name[1].y=(short)(x1);\
      name[2].x=(short)(y2);name[2].y=(short)(x2);\
      name[3].x=(short)(y3);name[3].y=(short)(x3);\
      name[4].x=(short)(y0);name[4].y=(short)(x0)

#define COORD(name,value) short name = (short) (value)

static void PaintArrows(ScrollbarWidget SBW, int which, int mode)
{
   BEGINMESSAGE(PaintArrows)

   if ((!IS_REALIZED) || ((ARROW_HEIGHT<1) || (ARROW_WIDTH<1)) || (!SHOW_ARROWS)) {
      ENDMESSAGE(PaintArrows)
      return; 
   }

   {  GC 	ts_gc = ((mode==NORMAL_ARROW) ? TOP_SHADOW_GC : BOT_SHADOW_GC);
      GC 	bs_gc = ((mode==NORMAL_ARROW) ? BOT_SHADOW_GC : TOP_SHADOW_GC);
      XPoint	ls[5];
      XPoint	rs[5];
      XPoint	bs[5];
      XPoint	cover[5];
      USE_FILL;
      int ATB  = ARROW_TO_BORDER;
      int ATTB = ARROW_TOP_TO_BORDER;
      int AH   = ARROW_HEIGHT-1;
      int AW   = ARROW_WIDTH-1;
      int AS   = ARROW_SHADOW;
      int D_AS = (int) (1.5*ARROW_SHADOW);

      if (which==UPPER_ARROW || which==BOTH_ARROWS) {
         if (IS_VERTICAL) {
            COORD(y,ATTB);
            COORD(yy,ATTB+2*AS);
            COORD(yyy,ATTB+AH-AS); 
            COORD(yyyy,ATTB+AH);
            COORD(lll,ATB); COORD(ll,ATB+D_AS); COORD(l,ATB+AW/2);
            COORD(r,ATB+(AW+1)/2); COORD(rr,ATB+AW-D_AS); COORD(rrr,ATB+AW); 
            vPOLY_4(cover,r,yy,rr,yyy,ll,yyy,l,yy);
            if (AS) {
               vPOLY_4(ls,l,y,lll,yyyy,ll,yyy,l,yy);
               vPOLY_4(rs,r,y,rrr,yyyy,rr,yyy,r,yy);
               vPOLY_4(bs,lll,yyyy+1,rrr,yyyy+1,rrr-AS,yyyy+1-AS,lll+AS,yyyy+1-AS);
               FILL(bs,5,bs_gc);
               if (AS==1) { FILL(cover,5,SCROLLBAR_GC); DRAWlines(cover,5,SCROLLBAR_GC);}
               FILL(ls,5,ts_gc);		DRAWlines(ls,5,ts_gc);
               FILL(rs,5,bs_gc);		DRAWlines(rs,5,bs_gc);
               if (AS!=1) { FILL(cover,5,SCROLLBAR_GC); DRAWlines(cover,5,SCROLLBAR_GC);}
            } else {
               FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);
            } 
         } else {
            COORD(x,ATTB);
            COORD(xx,ATTB+2*AS);
            COORD(xxx,ATTB+AH-AS); 
            COORD(xxxx,ATTB+AH);
            COORD(l,ATB+(AW+1)/2); COORD(ll,ATB+AW-D_AS); COORD(lll,ATB+AW);
            COORD(r,ATB+AW/2); COORD(rr,ATB+D_AS); COORD(rrr,ATB);
            hPOLY_4(cover,ll,xxx,rr,xxx,r,xx,l,xx);                    
            if (AS) {
               hPOLY_4(ls,lll,xxxx,ll,xxx,l,xx,l,x);
               hPOLY_4(rs,rrr,xxxx,rr,xxx,r,xx,r,x);
               hPOLY_4(bs,lll,xxxx,rrr,xxxx,rr,xxx,ll,xxx);
               FILL(bs,5,bs_gc);		DRAWlines(bs,5,bs_gc);
               FILL(ls,5,bs_gc);		DRAWlines(ls,5,bs_gc);
               FILL(rs,5,ts_gc);		DRAWlines(rs,5,ts_gc);
               FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);
            } else {
               FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);
            }
         }
      }
      if (which==LOWER_ARROW || which==BOTH_ARROWS) {
         if (IS_VERTICAL) {
            COORD(L,LENGTH-1);
            COORD(y,L-ATTB);
            COORD(yy,L-ATTB-2*AS);
            COORD(yyy,L-ATTB-AH+AS); 
            COORD(yyyy,L-ATTB-AH);
            COORD(lll,ATB); COORD(ll,ATB+D_AS); COORD(l,ATB+AW/2);
            COORD(r,ATB+(AW+1)/2); COORD(rr,ATB+AW-D_AS); COORD(rrr,ATB+AW);
            vPOLY_4(cover,ll,yyy,l,yy,r,yy,rr,yyy);
            if (AS) {
               vPOLY_4(ls,lll,yyyy,l,y,l,yy,ll,yyy);
               vPOLY_4(rs,rrr,yyyy,r,y,r,yy,rr,yyy);
               vPOLY_4(bs,rrr,yyyy,rr,yyy,ll,yyy,lll,yyyy);
               FILL(bs,5,ts_gc);		DRAWlines(bs,5,ts_gc);
               FILL(ls,5,ts_gc);		DRAWlines(ls,5,ts_gc);
               if (AS==1) { FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);}
               FILL(rs,5,bs_gc);		DRAWlines(rs,5,bs_gc);
               if (AS!=1) { FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);}
            } else {
               FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);
            } 
         } else {
            COORD(L,LENGTH-1);
            COORD(x,L-ATTB);
            COORD(xx,L-ATTB-2*AS);
            COORD(xxx,L-ATTB-AH+AS); 
            COORD(xxxx,L-ATTB-AH);
            COORD(lll,ATB); COORD(ll,ATB+D_AS); COORD(l,ATB+AW/2);
            COORD(r,ATB+(AW+1)/2); COORD(rr,ATB+AW-D_AS); COORD(rrr,ATB+AW);
            hPOLY_4(cover,ll,xxx,rr,xxx,r,xx,l,xx);                    
            if (AS) {
               hPOLY_4(ls,lll,xxxx,ll,xxx,l,xx,l,x);
               hPOLY_4(rs,rrr,xxxx,rr,xxx,r,xx,r,x);
               hPOLY_4(bs,lll,xxxx,rrr,xxxx,rr,xxx,ll,xxx);
               FILL(bs,5,ts_gc);		DRAWlines(bs,5,ts_gc);
               if (AS==1) { FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);}
               FILL(ls,5,ts_gc);		DRAWlines(ls,5,ts_gc);
               FILL(rs,5,bs_gc);		DRAWlines(rs,5,bs_gc);
               if (AS!=1) { FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);}
            } else {
               FILL(cover,5,SCROLLBAR_GC);	DRAWlines(cover,5,SCROLLBAR_GC);
            }
         }
      } 
   }
   ENDMESSAGE(PaintArrows)
}

/*###############################################################################
    CreateGC
###############################################################################*/

static void CreateGC(Widget w)
{
    SCROLLBARWIDGET w;
    XGCValues gcValues;
    XtGCMask mask;
    unsigned int depth = 1;

    BEGINMESSAGE(CreateGC)
    if (THUMB == XtUnspecifiedPixmap) {
        THUMB = XmuCreateStippledPixmap (XtScreen(w),
					(Pixel) 1, (Pixel) 0, depth);
    } else if (THUMB != None) {
	Window root;
	int x, y;
	unsigned int width, height, bw;
	if (XGetGeometry (XtDisplay(w), THUMB, &root, &x, &y,
			 &width, &height, &bw, &depth) == 0) {
	    XtAppError (XtWidgetToApplicationContext (w),
	       "Scrollbar Widget: Could not get geometry of thumb pixmap.");
	}
    }

    gcValues.foreground = FOREGROUND;
    gcValues.background = BACKGROUND_PIXEL;
    mask = GCForeground | GCBackground;

    if (THUMB != None) {
	if (depth == 1) {
	    gcValues.fill_style = FillOpaqueStippled;
	    gcValues.stipple = SBW->scrollbar.thumb;
	    mask |= GCFillStyle | GCStipple;
	}
	else {
	    gcValues.fill_style = FillTiled;
	    gcValues.tile = SBW->scrollbar.thumb;
	    mask |= GCFillStyle | GCTile;
	}
    }
    /* the creation should be non-caching, because */
    /* we now set and clear clip masks on the gc returned */
    SCROLLBAR_GC = XtGetGC(w, mask, &gcValues);

    ENDMESSAGE(CreateGC)
}

/*###############################################################################
    SetDimensions
###############################################################################*/

static void SetDimensions(ScrollbarWidget SBW)
{
   Dimension tmp;

   BEGINMESSAGE(SetDimensions)

   if (IS_VERTICAL) { LENGTH = HEIGHT; THICKNESS = WIDTH;  }
   else             { LENGTH = WIDTH;  THICKNESS = HEIGHT; }

   IIMESSAGE(THICKNESS,LENGTH)

   INFMESSAGE(### shadow width #############)
   if (SHADOW_BASE == _SC_INIT_VALUE_) SHADOW_BASE = SHADOW;
   SHADOW = SHADOW_BASE;
   tmp = MIN(THICKNESS,LENGTH);
   if (tmp<4*SHADOW) {
      if (tmp>=4) SHADOW = tmp/4; else SHADOW = tmp/2;
      INFMESSAGE(adjusted shadow)
   }
   IMESSAGE(SHADOW)

   INFMESSAGE(### arrow geometry ###########)
   if (SHOW_ARROWS) {
      ARROW_TO_BORDER = (int) (SHADOW+ARROW_SIDE_INDENT);
      ARROW_TOP_TO_BORDER  = (int) (SHADOW+ARROW_TOP_INDENT);
      ARROW_WIDTH       = (int)THICKNESS-2*ARROW_TO_BORDER;
      ARROW_HEIGHT      = ((int)LENGTH-(int)MIN_THUMB)/2-(int)THUMB_TOP_INDENT-ARROW_TOP_TO_BORDER;

      if ((ARROW_WIDTH < NICE_DIMENSION) && (ARROW_SIDE_INDENT)) {
         while (ARROW_TO_BORDER>(int)SHADOW) {
            --ARROW_TO_BORDER;
            ARROW_WIDTH = ((int)THICKNESS-2*ARROW_TO_BORDER);
            if (ARROW_WIDTH>=NICE_DIMENSION) break;
         }
         INFMESSAGE(adjusted arrow side indent)
      }
      if ((ARROW_HEIGHT<ARROW_WIDTH) && (ARROW_TOP_INDENT)) {
         while (ARROW_TOP_TO_BORDER>(int)SHADOW) {
            --ARROW_TOP_TO_BORDER;
            ARROW_HEIGHT = ((int)LENGTH-(int)MIN_THUMB)/2-(int)THUMB_TOP_INDENT-ARROW_TOP_TO_BORDER;
            if (ARROW_HEIGHT>=NICE_DIMENSION) break;
         }
         ARROW_HEIGHT = MAX(0,ARROW_HEIGHT);
         INFMESSAGE(adjusted arrow top indent)
      }
      ARROW_WIDTH  = MIN(ARROW_WIDTH,ARROW_HEIGHT);
      if ((ARROW_WIDTH+(int)THICKNESS)&1) --ARROW_WIDTH;
      ARROW_HEIGHT = ARROW_WIDTH; if (ARROW_HEIGHT) ARROW_HEIGHT -= (1-(ARROW_WIDTH&1));
      ARROW_TO_BORDER = ((int)THICKNESS - ARROW_WIDTH)/2;
      ARROW_SHADOW = (ARROW_WIDTH-1)/3; ARROW_SHADOW = PUT_IN_RANGE(0,ARROW_SHADOW,(int)SHADOW);
   } else {
      ARROW_TO_BORDER = ARROW_TOP_TO_BORDER = (int) SHADOW;
      ARROW_WIDTH = ARROW_HEIGHT = 0;
   }
   IIMESSAGE(ARROW_WIDTH,ARROW_HEIGHT)
   IIMESSAGE(ARROW_TO_BORDER,ARROW_TOP_TO_BORDER) IMESSAGE(ARROW_SHADOW)

   INFMESSAGE(### thumb geometry ###########)
   THUMB_SHADOW        = (int) SHADOW;
   THUMB_TO_BORDER   = (int) (THUMB_SIDE_INDENT+SHADOW);
   THUMB_TO_ARROW      = (int) (THUMB_TOP_INDENT);

   if ((int)THICKNESS-2*THUMB_TO_BORDER<NICE_DIMENSION) {
      THUMB_TO_BORDER = ((int)THICKNESS-NICE_DIMENSION)/2;
      THUMB_TO_BORDER = MAX(THUMB_TO_BORDER,(int)SHADOW);
      INFMESSAGE(adjusted thumb side indent)
   }
   if (((int)LENGTH-2*(ARROW_TOP_TO_BORDER+ARROW_HEIGHT+THUMB_TO_ARROW)<(int)MIN_THUMB) && (THUMB_TO_ARROW)) {
      THUMB_TO_ARROW = ((int)LENGTH-(int)MIN_THUMB)/2-ARROW_TOP_TO_BORDER-ARROW_HEIGHT;
      THUMB_TO_ARROW = MAX(0,THUMB_TO_ARROW);
      INFMESSAGE(adjusted minimal distance thumb to arrow)
   }
   IMESSAGE(MIN_THUMB) IIMESSAGE(THUMB_TO_BORDER,THUMB_TO_ARROW)

   ENDMESSAGE(SetDimensions)
}

/*###############################################################################
    ClassInitialize
###############################################################################*/

static void ClassInitialize(void)
{
    static Boolean initialized = FALSE;

    BEGINMESSAGE(ClassInitialize)
    if (initialized) {INFMESSAGE(class already initialized) return; }
    XawInitializeWidgetSet();
    XtAddConverter( XtRString, XtROrientation, XmuCvtStringToOrientation,
		    (XtConvertArgList)NULL, (Cardinal) 0);
    initialized = TRUE;
    ENDMESSAGE(ClassInitialize)
}

/*###############################################################################
    Initialize
###############################################################################*/

static void Initialize(Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
/* request: what the client asked for */
/* new: what we're going to give him */
{
    SCROLLBARWIDGET new;

    BEGINMESSAGE(Initialize)
    CreateGC(new);
    if (WIDTH == 0)  WIDTH  = (IS_VERTICAL)   ? THICKNESS : LENGTH;
    if (HEIGHT == 0) HEIGHT = (IS_HORIZONTAL) ? THICKNESS : LENGTH;
    SHADOW_BASE = _SC_INIT_VALUE_;
    TOPLOC      = 0;
    SHOWNLENGTH = 1;
    INTERVAL    = MAX(INTERVAL,MIN_INTERVAL);
    DELAY       = MAX(DELAY,MIN_DELAY);
    SCROLLMODE  = NOSCROLL;
    TIMER       = DISABLED;
    DYNAMIC     = 0;
    /*
       how could we ever come away without the following line ...???
       1/95 ###jp###
    */
    SetDimensions(SBW); /*#test#*/
    ENDMESSAGE(Initialize)
}

/*###############################################################################
    Realize
###############################################################################*/

static void Realize(Widget w, Mask *valueMask, XSetWindowAttributes *attributes)
{
    SCROLLBARWIDGET w;
    BEGINMESSAGE(Realize)
    if(CURSOR_NAME == NULL) XtVaSetValues(w, XtNcursorName, "top_left_arrow", NULL);
    /* dont set the cursor of the window to anything */
    *valueMask &= ~CWCursor;
    /*
     * The Simple widget actually stuffs the value in the valuemask.
     */
    (*scrollbarWidgetClass->core_class.superclass->core_class.realize) (w, valueMask, attributes);
    ENDMESSAGE(Realize)
}

/*###############################################################################
    SetValues
###############################################################################*/

static Boolean SetValues(Widget old, Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
   SCROLLBARWIDGET 	new;
   ScrollbarWidget 	old_sbw = (ScrollbarWidget) old;
   Boolean 		redraw 	= FALSE;

   BEGINMESSAGE(SetValues)

   if (SHOWN < 0.0 || SHOWN > 1.0 || TOP < 0.0 || TOP > 1.0) {
      INFMESSAGE(### bad values for thumb position/length ###)
      FMESSAGE(SHOWN) FMESSAGE(TOP)
      SHOWN = old_sbw->scrollbar.shown; TOP = old_sbw->scrollbar.top;
      SHOWN = PUT_IN_RANGE(0.0,SHOWN,1.0);
      TOP   = PUT_IN_RANGE(0.0,TOP,  1.0);
      INFMESSAGE(corrected to)
      FMESSAGE(SHOWN) FMESSAGE(TOP)
   }
   FMESSAGE(SHOWN) FMESSAGE(TOP)
   if ((TOP != old_sbw->scrollbar.top) || (SHOWN != old_sbw->scrollbar.shown)) {
      INFMESSAGE(thumb position/length has changed)
      redraw = TRUE;
   }

   if (FOREGROUND != old_sbw->scrollbar.foreground ||
      BACKGROUND_PIXEL != old_sbw->core.background_pixel ||
      THUMB != old_sbw->scrollbar.thumb) {
      INFMESSAGE(GC has changed)
      XtReleaseGC((Widget)SBW, SCROLLBAR_GC); /*###jp### changed 21.10.94*/
      CreateGC(new);
      redraw = TRUE;
   }

   if ( (SHADOW != old_sbw->threeD.shadow_width)				||
        (SHOW_ARROWS != old_sbw->scrollbar.show_arrows)			||
        (ARROW_TOP_INDENT != old_sbw->scrollbar.arrow_top_indent)	||
        (ARROW_SIDE_INDENT != old_sbw->scrollbar.arrow_side_indent)	||
        (THUMB_SIDE_INDENT != old_sbw->scrollbar.thumb_side_indent)	||
        (THUMB_TOP_INDENT != old_sbw->scrollbar.thumb_top_indent)	||
        (MIN_THUMB != old_sbw->scrollbar.min_thumb)  ) {
        INFMESSAGE(internal scrollbar geometry has changed)
        SHADOW_BASE=_SC_INIT_VALUE_;
        SetDimensions(SBW);
        redraw =TRUE;
   }
   INTERVAL = MAX(INTERVAL,MIN_INTERVAL);
   DELAY = MAX(DELAY,MIN_DELAY);

   if (!IS_REALIZED) { INFMESSAGE(not realized) redraw=FALSE; }

#  ifdef MESSAGES
      if (redraw) { INFMESSAGE(need redisplay) }
      else        { INFMESSAGE(redisplay unnecessary) }
#  endif

   ENDMESSAGE(SetValues)
   return redraw;
}

/*###############################################################################
   Resize
###############################################################################*/

static void Resize(Widget w)
{
   SCROLLBARWIDGET w;
   XEvent event;

   BEGINMESSAGE(Resize)
   SetDimensions(SBW);
   event.xexpose.x = 0; event.xexpose.y = 0;
   event.xexpose.width = WIDTH; event.xexpose.height = HEIGHT;
   Redisplay(w, (XEvent *) &event, (Region)NULL);
   ENDMESSAGE(Resize)
}

/*###############################################################################
   Redisplay
   compress_exposure is TRUE, so 'event' contains the bounding box for the
   'region' to be exposed. Graphics actions will be restricted to this
   bounding box to minimize visual disturbances.
###############################################################################*/

static void Redisplay(Widget w, XEvent *event, Region region)
{
   SCROLLBARWIDGET w;
   ScrollbarWidgetClass swclass = (ScrollbarWidgetClass) XtClass (w);
   Dimension s;
   Dimension newtop, newbot;
   Dimension xl = (Dimension) _LEFT_START_OF_SCROLL_REGION_;
   Dimension xr = (Dimension) _RIGHT_END_OF_SCROLL_REGION_;
   Dimension yt,yb;

   BEGINMESSAGE(Redisplay)

   if (!IS_REALIZED) { INFMESSAGE(not realized) ENDMESSAGE(Redisplay) return; }
   if ((LENGTH<1) || (THICKNESS<1)) { 
      INFMESSAGE(not enough space) ENDMESSAGE(Redisplay) return;
   }

   if ((region) && (XRectInRegion(region,0,0,WIDTH,HEIGHT) == RectangleOut)) {
      INFMESSAGE(request out of region) ENDMESSAGE(Redisplay)
      return;
   }

   GetRelativeThumbLocation(SBW,&newtop,&newbot);
   AdjustThumbShadow(SBW,newtop,newbot);
   s = (Dimension)(THUMB_SHADOW);
   if (event) {
      if (IS_HORIZONTAL) { yt=(Dimension)(event->xexpose.x); yb=(Dimension)(yt+event->xexpose.width);  }
      else               { yt=(Dimension)(event->xexpose.y); yb=(Dimension)(yt+event->xexpose.height); }
      if (SHOW_ARROWS) {
         Dimension soar = (Dimension) (ARROW_TOP_TO_BORDER);
         Dimension sosr = (Dimension) _START_OF_SCROLL_REGION_;
         Dimension eosr = (Dimension) (_END_OF_SCROLL_REGION_+THUMB_TO_ARROW);
         Dimension eoar = (Dimension) (LENGTH-1-soar);
         if ((yb>=soar) && (yt<sosr))
            PaintArrows(SBW,UPPER_ARROW,(SCROLLMODE==ARROW_UP)   ? INVERTED_ARROW : NORMAL_ARROW);
         if ((yb>eosr) && (yt<=eoar)) 
            PaintArrows(SBW,LOWER_ARROW,(SCROLLMODE==ARROW_DOWN) ? INVERTED_ARROW : NORMAL_ARROW);
      }
      if ((yb>=newtop) && (yt<=newbot)) {
         if ((yt<=newtop+s) && (yb+s>=newbot))     PaintWholeThumb(SBW,newtop,newbot,xl,xr,s);
         else if ((yt<=newtop+s) && (yb+s<newbot)) PaintEndOfThumb(SBW,newtop,yb,xl,xr,s,UPPER_PART);
         else if ((yt>newtop+s) && (yb+s>=newbot)) PaintEndOfThumb(SBW,yt,newbot,xl,xr,s,LOWER_PART);
         else if ((yt>newtop+s) && (yb+s<newbot))  PaintMiddleOfThumb(SBW,yt,yb,xl,xr,s);
      }
   } else {
      /* should never come here */
      printf("  Scrollbarwidget: no event forced exit\n");
      clean_safe_tempdir();
      exit(0);
   }
   TOPLOC = newtop;
   SHOWNLENGTH = newbot-newtop+1;

   (*swclass->threeD_class.shadowdraw) (w, event, region, sbw->threeD.relief, FALSE);

   ENDMESSAGE(Redisplay)
}

/*###############################################################################
    Destroy
###############################################################################*/

static void Destroy (w)
    Widget w;
{
    SCROLLBARWIDGET w;
    BEGINMESSAGE(Destroy)
    DESTROY_TIMER;
    XtReleaseGC (w, SCROLLBAR_GC);
    ENDMESSAGE(Destroy)
}

/*###############################################################################
   CompareEvents
###############################################################################*/

#define CHECK(field) if (newEvent->field != oldEvent->field) return False;

static Boolean CompareEvents (XEvent *oldEvent, XEvent *newEvent)
{
    INFMESSAGE(executing Procedure: CompareEvents)
    CHECK(xany.display);
    CHECK(xany.type);
    CHECK(xany.window);

    switch (newEvent->type) {
    case MotionNotify:
	CHECK(xmotion.state);
	break;
    case ButtonPress:
    case ButtonRelease:
	CHECK(xbutton.state);
	CHECK(xbutton.button);
	break;
    case KeyPress:
    case KeyRelease:
	CHECK(xkey.state);
	CHECK(xkey.keycode);
	break;
    case EnterNotify:
    case LeaveNotify:
	CHECK(xcrossing.mode);
	CHECK(xcrossing.detail);
	CHECK(xcrossing.state);
	break;
    }

    return True;
}

/*###############################################################################
    PeekNotifyEvent
###############################################################################*/

struct EventData {
   XEvent 	*oldEvent;
   int 		count;
};

static Bool PeekNotifyEvent (Display *dpy, XEvent *event, char *args)
{
   struct EventData *eventData = (struct EventData*)args;
   INFMESSAGE(executing Procedure: PeekNotifyEvent)
   return ((++eventData->count == QLength(dpy)) /* since PeekIf blocks */
   	    || CompareEvents(event, eventData->oldEvent));
}

/*###############################################################################
    LookAhead
###############################################################################*/

static Boolean LookAhead (Widget w, XEvent *event)
{
   XEvent 	newEvent;
   struct 	EventData eventData;

   BEGINMESSAGE(LookAhead)
   if (QLength (XtDisplay (w)) == 0) {
      INFMESSAGE(aborting) ENDMESSAGE(LookAhead)
      return False;
   }

   eventData.count = 0;
   eventData.oldEvent = event;

   XPeekIfEvent (XtDisplay (w), &newEvent, PeekNotifyEvent, (char*)&eventData);

   ENDMESSAGE(LookAhead)
   return CompareEvents (event, &newEvent);
}

/*###############################################################################
    ExtractPosition
###############################################################################*/

#define GET_EVENT_POS(kind) *position = (is_vertical) ? event->kind.y : event->kind.x

static void ExtractPosition(XEvent *event, Position *position, Boolean is_vertical)
{
   BEGINMESSAGE(ExtractPosition)
   switch( event->type ) {
   case MotionNotify:
      GET_EVENT_POS(xmotion);    break;
   case ButtonPress:
   case ButtonRelease:
      GET_EVENT_POS(xbutton);    break;
   case KeyPress:
   case KeyRelease:
      GET_EVENT_POS(xkey);       break;
   case EnterNotify:
   case LeaveNotify:
      GET_EVENT_POS(xcrossing);  break;
   default:
      *position = 0;
   }
   ENDMESSAGE(ExtractPosition)
}

/*###############################################################################
    HandleThumb
###############################################################################*/

static void HandleThumb (Widget w, XEvent *event, String *params, Cardinal *num_params)
{
   SCROLLBARWIDGET w;
   Position position;
   Position teoar = (Position) ARROW_TOP_TO_BORDER+ARROW_HEIGHT-1;
   Position beoar = (Position) LENGTH-teoar+1;

   BEGINMESSAGE(HandleThumb)
   if ((SCROLLMODE != NOSCROLL) && (SCROLLMODE != CONTINUOUS)) {
      INFMESSAGE(aborting due to wrong scrollmode) ENDMESSAGE(HandleThumb)
      return;
   }

   ExtractPosition(event,&position,(IS_VERTICAL));
   if ( (SCROLLMODE==CONTINUOUS) ||
        ((position >= TOPLOC) && (position < TOPLOC + SHOWNLENGTH)
          && (position>teoar) && (position<beoar)) ) {
      XtCallActionProc(w, "MoveThumb", event, params, *num_params);
      XtCallActionProc(w, "NotifyThumb", event, params, *num_params);
   }
   ENDMESSAGE(HandleThumb)
}

/*###############################################################################
    HandleAutoscroll
###############################################################################*/

static void HandleAutoscroll(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params)
{
   SCROLLBARWIDGET w;
   static Position old_position;
   Position position;
   int diff;
   float rel_diff;

   BEGINMESSAGE(HandleAutoscroll)

   if ((SCROLLMODE != ARROW_UP) && (SCROLLMODE != ARROW_DOWN)) {
      INFMESSAGE(returning due to wrong scroll mode) ENDMESSAGE(HandleAutoscroll)
      return;
   }
   if (!USE_DYNAMIC) {
      DYNAMIC = INTERVAL;
      ENDMESSAGE(HandleAutoscroll)
      return;
   }

   ExtractPosition(event,&position,(IS_VERTICAL));
   if (*num_params == _SC_INIT_VALUE_) {
      INFMESSAGE(... initializing)
      old_position = position;
      ENDMESSAGE(HandleAutoscroll)
      return;
   }

   DYNAMIC = PUT_IN_RANGE(MIN_INTERVAL,DYNAMIC,MAX_INTERVAL);   IMESSAGE(DYNAMIC)
   diff = (SCROLLMODE==ARROW_UP) ? ((int)position)-((int)old_position):((int)old_position)-((int)position);
   rel_diff = PUT_IN_RANGE(-1.0,((float)diff)/DYNAMIC_REGION,1.0);
   if (rel_diff>0) DYNAMIC = INTERVAL + (Dimension)((MAX_INTERVAL-(int)INTERVAL)*rel_diff);
   else            DYNAMIC = INTERVAL + (Dimension)((MIN_INTERVAL-(int)INTERVAL)*(-rel_diff));
   IMESSAGE(DYNAMIC)

   ENDMESSAGE(HandleAutoscroll)
}

/*###############################################################################
    RepeatNotify
###############################################################################*/

static void RepeatNotify (XtPointer client_data, XtIntervalId *idp _GL_UNUSED)
{
    SCROLLBARWIDGET client_data;
    int call_data=0;

    BEGINMESSAGE(RepeatNotify)
    if (SCROLLMODE == NOSCROLL || SCROLLMODE == CONTINUOUS) {
	DESTROY_TIMER;
        INFMESSAGE(aborting due to wrong scroll_mode) ENDMESSAGE(RepeatNotify)
	return;
    }

    switch (SCROLLMODE) {
       case ARROW_UP:
            call_data = -ARROW_SCROLL_AMOUNT;
            break;
       case ARROW_DOWN:
            call_data = ARROW_SCROLL_AMOUNT;
            break;
       case PAGE_UP:
            call_data = -PAGE_SCROLL_AMOUNT;
            break;
       case PAGE_DOWN:
            call_data = PAGE_SCROLL_AMOUNT;
            break;
    }

    CALLscrollProc(client_data,call_data);
    SET_TIMER( client_data, DYNAMIC);

    ENDMESSAGE(RepeatNotify)
}

/*###############################################################################
    NotifyScroll
###############################################################################*/


static void NotifyScroll(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
    SCROLLBARWIDGET w;
    Position tsoar = (Position) ARROW_TOP_TO_BORDER;
    Position teoar = (Position) ARROW_TOP_TO_BORDER+ARROW_HEIGHT-1;
    Position beoar = (Position) LENGTH-teoar+1;
    Position bsoar = (Position) LENGTH-tsoar-1;
    Position position;
    Cardinal init_num_params = _SC_INIT_VALUE_;

    BEGINMESSAGE(NotifyScroll)

    if (SCROLLMODE==CONTINUOUS) {
       INFMESSAGE(aborting since CONTINUOUS scroll in progress) ENDMESSAGE(NotifyScroll)
       return;
    }

    ExtractPosition(event,&position,(IS_VERTICAL));
    if ((position>=tsoar) && (position <= teoar)) {
        if (SHOW_ARROWS) {                              INFMESSAGE(handle upper/left arrow)
           PaintArrows(SBW,UPPER_ARROW,INVERTED_ARROW);
	   SCROLLMODE = ARROW_UP;
           CALLscrollProc(w,((int)-ARROW_SCROLL_AMOUNT));
           DYNAMIC = INTERVAL;
           HandleAutoscroll(w, event, NULL, &init_num_params);
           SET_TIMER(w,DELAY);
        }
    } else if ((position >= beoar) && (position<=bsoar)) {
        if (SHOW_ARROWS) {                              INFMESSAGE(handle lower/right arrow)
           PaintArrows(SBW,LOWER_ARROW,INVERTED_ARROW);
	   SCROLLMODE = ARROW_DOWN;
           CALLscrollProc(w,((int)ARROW_SCROLL_AMOUNT));
           DYNAMIC = INTERVAL;
           HandleAutoscroll(w, event, NULL, &init_num_params);
           SET_TIMER(w,DELAY);
        }
    } else if ((position>teoar) &&(position < TOPLOC)) { INFMESSAGE(handle Zone above Thumb)
	SCROLLMODE = PAGE_UP;
        CALLscrollProc(w,((int)-PAGE_SCROLL_AMOUNT));
        DYNAMIC = INTERVAL;
        SET_TIMER(w,DELAY);
    } else if ((position<beoar)&&(position > TOPLOC + SHOWNLENGTH)) { INFMESSAGE(handle Zone below Thumb)
	SCROLLMODE = PAGE_DOWN;
        CALLscrollProc(w,((int)PAGE_SCROLL_AMOUNT));
        DYNAMIC = INTERVAL;
        SET_TIMER(w,DELAY);
    }
    ENDMESSAGE(NotifyScroll);
}

/*###############################################################################
    EndScroll
###############################################################################*/

/* ARGSUSED */
static void EndScroll(Widget w, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
    SCROLLBARWIDGET w;

    BEGINMESSAGE(EndScroll)
    if (SHOW_ARROWS) {
       if (SCROLLMODE==ARROW_UP)   PaintArrows(SBW,UPPER_ARROW,NORMAL_ARROW);
       if (SCROLLMODE==ARROW_DOWN) PaintArrows(SBW,LOWER_ARROW,NORMAL_ARROW);
    }
    SCROLLMODE=NOSCROLL;
    DESTROY_TIMER;
    ENDMESSAGE(EndScroll)
}

/*###############################################################################
    MoveThumb
###############################################################################*/

static void MoveThumb(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
   SCROLLBARWIDGET w;
   Position position;
   static Position old_position;
   float losr         = (float)(_SCROLL_LENGTH_-MIN_THUMB);
   Position tosr = (Position) ARROW_TOP_TO_BORDER+ARROW_HEIGHT+THUMB_TO_ARROW;
   Position bosr = (Position) LENGTH-1-tosr;
   BEGINMESSAGE(MoveThumb)
   if (LookAhead (w, event)) {
      INFMESSAGE(aborting due to LookAhead) ENDMESSAGE(MoveThumb)
      return;
   }

   if (!event->xmotion.same_screen) {
      INFMESSAGE(aborting due to wrong screen) ENDMESSAGE(MoveThumb)
      return;
   }

   ExtractPosition(event,&position,(IS_VERTICAL));
   position = PUT_IN_RANGE(tosr,position,bosr);

   if (SCROLLMODE != CONTINUOUS) {
       if ((position < TOPLOC) || (position >= TOPLOC + SHOWNLENGTH)) {
         TOP = PUT_IN_RANGE(0.0,((float)(position-tosr))/losr, 1.0-SHOWN);
        }
      SCROLLMODE=CONTINUOUS;
   } else {
      TOP = PUT_IN_RANGE(0.0,TOP+((float)(position-old_position))/losr, 1.0-SHOWN);
   }
   old_position = position;
   PaintThumb(SBW);
   XFlush(XtDisplay (w));
   ENDMESSAGE(MoveThumb)
}

/*###############################################################################
    NotifyThumb
###############################################################################*/

static void NotifyThumb(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
   register SCROLLBARWIDGET w;

   BEGINMESSAGE(NotifyThumb)
   if (LookAhead (w, event)) {
      INFMESSAGE(returning due to LookAhead) ENDMESSAGE(NotifyThumb)
      return;
   }
    /*
       thumbProc is not pretty, but is necessary for backwards
       compatibility on those architectures for which it work{s,ed};
       the intent is to pass a (truncated) float by value.
    */
   XtCallCallbacks (w, XtNthumbProc, *(XtPointer*)&TOP);
   XtCallCallbacks (w, XtNjumpProc, (XtPointer)&TOP);
   ENDMESSAGE(NotifyThumb)
}

/********************************************************************************
 *
 *  Public routines.
 *
 *******************************************************************************/

/*###############################################################################
    XawScrollbarSetThumb
    Set the scroll bar to the given location.
###############################################################################*/

void XawScrollbarSetThumb (Widget w,
#if NeedWidePrototypes
                                     double top, double shown
#else
                                     float top, float shown
#endif
                          )
{
   SCROLLBARWIDGET w;

   BEGINMESSAGE(XawScrollbarSetThumb)

   if (SCROLLMODE == CONTINUOUS) {
      INFMESSAGE(aborting since CONTINUOUS scroll in progress) ENDMESSAGE(XawScrollbarSetThumb)
      return;
   }

   SHOWN       = PUT_IN_RANGE(0.0,shown,1.0);  FMESSAGE(SHOWN)
   TOP         = PUT_IN_RANGE(0.0,top,  1.0);  FMESSAGE(TOP)
   PaintThumb(SBW);

   ENDMESSAGE(XawScrollbarSetThumb)
}

