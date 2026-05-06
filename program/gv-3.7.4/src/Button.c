/*
**
** Button.c
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
*/
#include "ac_config.h"

#include <stdio.h>
/*
#define MESSAGES
*/
#include "message.h"

#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XMU(Misc.h)
#include INC_XAW(XawInit.h)
#include "ButtonP.h"
#include INC_XMU(Converters.h)
#include INC_XMU(CharSet.h)

static void ClassInitialize(void);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void Redisplay(Widget,XEvent *,Region);
static void Destroy(Widget);
static void PaintButtonWidget(Widget,XEvent *,Region);
static void GetSetBackgroundGC(ButtonWidget);
static void GetHighlightedBackgroundGC(ButtonWidget);

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

static char defaultTranslations[] = 
"<EnterWindow>:highlight()\n\
<LeaveWindow>:reset()\n\
<Btn1Down>:set()\n\
<Btn1Up>:notify()unset()";

#define offset(field) XtOffsetOf(ButtonRec, field)
static XtResource resources[] = { 
  {XtNsetFrameStyle, XtCSetFrameStyle, XtRFrameType, sizeof(XawFrameType),
        offset(button.set_frame_style), XtRImmediate, (XtPointer) XawSUNKEN },
  {XtNunsetFrameStyle, XtCUnsetFrameStyle, XtRFrameType, sizeof(XawFrameType),
        offset(button.unset_frame_style), XtRImmediate, (XtPointer) XawRAISED },
  {XtNhighlightedFrameStyle, XtCHighlightedFrameStyle, XtRFrameType, sizeof(XawFrameType),
        offset(button.highlighted_frame_style), XtRImmediate, (XtPointer) XawRAISED },
  {XtNsetBackground, XtCSetBackground, XtRPixel, sizeof(Pixel),
	offset(button.set_background), XtRString, XtDefaultBackground},
  {XtNhighlightedBackground, XtCHighlightedBackground, XtRPixel, sizeof(Pixel),
	offset(button.highlighted_background), XtRString, XtDefaultBackground},
  {XtNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
        offset(command.highlight_thickness), XtRImmediate,(XtPointer)0},
  {XtNborderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
        XtOffsetOf(RectObjRec,rectangle.border_width), XtRImmediate,
        (XtPointer)0}
};
#undef offset

static XtActionsRec actionsList[] = {
  {"highlight",	ButtonHighlight},
  {"set",	ButtonSet},
  {"reset",	ButtonReset},
  {"unset",	ButtonUnset},
};

#define SuperClass ((CommandWidgetClass)&commandClassRec)

ButtonClassRec buttonClassRec = {
  {
    (WidgetClass) SuperClass,		/* superclass		  */	
    "Button",				/* class_name		  */
    sizeof(ButtonRec),			/* size			  */
    ClassInitialize,			/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    FALSE,				/* class_inited		  */
    Initialize,				/* initialize		  */
    NULL,				/* initialize_hook	  */
    XtInheritRealize,			/* realize		  */
    actionsList,			/* actions		  */
    XtNumber(actionsList),		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* resource_count	  */
    NULLQUARK,				/* xrm_class		  */
    TRUE,				/* compress_motion	  */
    TRUE,				/* compress_exposure	  */
    TRUE,				/* compress_enterleave    */
    FALSE,				/* visible_interest	  */
    Destroy,				/* destroy		  */
    XtInheritResize,			/* resize		  */
    Redisplay,				/* expose		  */
    NULL,				/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,		/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL				/* extension		  */
  },  /* CoreClass fields initialization */
  {
    XtInheritChangeSensitive		/* change_sensitive	*/
  },  /* SimpleClass fields initialization */
  {
    XtInheritXaw3dShadowDraw,           /* shadowdraw           */
  },  /* ThreeD Class fields initialization */
  {
    0,                                     /* field not used    */
  },  /* LabelClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* CommandClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* ButtonClass fields initialization */
};

  /* for public consumption */
WidgetClass buttonWidgetClass = (WidgetClass) &buttonClassRec;

/*---------------------------------------------------*/
/* ClassInitialize */
/*---------------------------------------------------*/

static void ClassInitialize()
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

static void
Initialize(Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
  ButtonWidget bw = (ButtonWidget) new;

  BEGINMESSAGE(Initialize)
  GetSetBackgroundGC(bw);
  GetHighlightedBackgroundGC(bw);
  bw->button.highlighted = 0;
  ENDMESSAGE(Initialize)
}

/*---------------------------------------------------*/
/* Redisplay */
/*---------------------------------------------------*/

static void 
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
  BEGINMESSAGE(Redisplay)
  PaintButtonWidget(w, event, region);
  ENDMESSAGE(Redisplay)
}

/*---------------------------------------------------*/
/* Destroy */
/*---------------------------------------------------*/

static void 
Destroy(w)
Widget w;
{
  ButtonWidget bw = (ButtonWidget)w;

  BEGINMESSAGE(Destroy)
  XtReleaseGC(w,bw->button.set_background_GC);
  XtReleaseGC(w,bw->button.highlighted_background_GC);
  ENDMESSAGE(Destroy)
}

/*---------------------------------------------------*/
/* GetHighlightedBackgroundGC */
/*---------------------------------------------------*/

static void GetHighlightedBackgroundGC(bw)
  ButtonWidget bw;
{
  XGCValues	values;

  BEGINMESSAGE(GetHighlightedBackgroundGC)
  values.foreground	= bw->button.highlighted_background;
  values.background	= bw->label.foreground;
  bw->button.highlighted_background_GC = XtGetGC((Widget)bw,(unsigned) (GCForeground | GCBackground),&values);
  ENDMESSAGE(GetHighlightedBackgroundGC)
}

/*---------------------------------------------------*/
/* GetSetBackgroundGC */
/*---------------------------------------------------*/

static void GetSetBackgroundGC(bw)
  ButtonWidget bw;
{
  XGCValues	values;

  BEGINMESSAGE(GetSetBackgroundGC)
  values.foreground	= bw->button.set_background;
  values.background	= bw->label.foreground;
  bw->button.set_background_GC = XtGetGC((Widget)bw,(unsigned) (GCForeground | GCBackground),&values);
  ENDMESSAGE(GetSetBackgroundGC)
}

/*---------------------------------------------------*/
/* PaintButtonWidget */
/*---------------------------------------------------*/

static void 
PaintButtonWidget(w, event, region)
Widget w;
XEvent *event;
Region region;
{
  ButtonWidget bw = (ButtonWidget)w;
  int wh,ww,sw,fs;
  GC cgc,lgc,gc;
  Boolean set;

  BEGINMESSAGE(PaintButtonWidget)

  if (bw->command.set) {
    gc = bw->button.set_background_GC;
  } else if (bw->button.highlighted) {
    gc = bw->button.highlighted_background_GC;
  } else {
    gc = bw->command.inverse_GC;
  }
  XFillRectangle(XtDisplay(w), XtWindow(w),gc,0,0,bw->core.width,bw->core.height);
  region = NULL;
  lgc = bw->label.normal_GC;
  sw = (int) bw->threeD.shadow_width;
  set = bw->command.set;
  bw->command.set = False;
#if 0
  bw->threeD.shadow_width = 0;
#endif
  cgc = bw->command.normal_GC;
  if (bw->label.pixmap != None) {
    bw->command.normal_GC = gc;
  }
  (*SuperClass->core_class.expose) (w, event, region);
  bw->command.set = set;
  bw->threeD.shadow_width = (Dimension) sw;
  bw->label.normal_GC = lgc;
  bw->command.normal_GC = cgc;

  ww = (int)bw->core.width;
  wh = (int)bw->core.height;
  sw = (int)bw->threeD.shadow_width;
  if      (bw->command.set)        fs = (int)bw->button.set_frame_style;
  else if (bw->button.highlighted) fs = (int)bw->button.highlighted_frame_style;
  else                             fs = (int)bw->button.unset_frame_style;
  if (sw == 0 || 2*sw>ww || 2*sw>wh) {
    INFMESSAGE(not enough space to display anything) ENDMESSAGE(Redisplay)
    return;
  }
  FrameDrawFrame(w,0,0,ww,wh,fs,sw,bw->threeD.top_shadow_GC,bw->threeD.bot_shadow_GC);
  ENDMESSAGE(PaintButtonWidget)
}

/*####################################################################*/
/*####################################################################*/
/* Public Routines */
/*####################################################################*/
/*####################################################################*/


/*##################################################-*/
/* ACTIONS */
/*##################################################-*/

void
ButtonHighlight(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  ButtonWidget bw = (ButtonWidget)w;
  Boolean set;

  BEGINMESSAGE(ButtonHighlight)
  bw->button.highlighted=1;
  set = bw->command.set;
  bw->command.set=False;
  if (XtIsRealized(w)) PaintButtonWidget(w, event, (Region)NULL);
  bw->command.set=set;
  ENDMESSAGE(ButtonHighlight)
}

void
ButtonSet(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  ButtonWidget bw = (ButtonWidget)w;

  BEGINMESSAGE(ButtonSet)
  if (!bw->command.set) {
    bw->command.set= TRUE;
    if (XtIsRealized(w)) PaintButtonWidget(w, event, (Region)NULL);
  }
  ENDMESSAGE(ButtonSet)
}

void
ButtonUnset(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  ButtonWidget bw = (ButtonWidget)w;

  BEGINMESSAGE(ButtonUnset)
  bw->command.set = FALSE;
  if (XtIsRealized(w)) PaintButtonWidget(w, event, (Region)NULL);
  ENDMESSAGE(ButtonUnset)
}

void 
ButtonReset(w,event,params,num_params)
Widget w;
XEvent *event;
String *params;		/* unused */
Cardinal *num_params;   /* unused */
{
  ButtonWidget bw = (ButtonWidget)w;

  BEGINMESSAGE(ButtonReset)
  bw->button.highlighted = 0;
  ButtonUnset(w, event, params, num_params);
  ENDMESSAGE(ButtonReset)
}

