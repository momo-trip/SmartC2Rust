/*
**
** Switch.c
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
#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(XawInit.h)
#include "SwitchP.h"

static void ClassInitialize(void);

#define superclass ((ButtonWidgetClass)&buttonClassRec)

static char defaultTranslations[] = 
"Any<BtnDown>:highlight()\n\
Any<BtnUp>:toggle()";

#define offset(field) XtOffsetOf(SwitchRec, field)
static XtResource resources[] = {
  {XtNhighlightedFrameStyle, XtCHighlightedFrameStyle, XtRFrameType, sizeof(XawFrameType),
   offset(button.highlighted_frame_style), XtRImmediate, (XtPointer) XawSUNKEN },
  {XtNunsetFrameStyle, XtCUnsetFrameStyle, XtRFrameType, sizeof(XawFrameType),
   offset(button.unset_frame_style), XtRImmediate, (XtPointer) XawCHISELED },
  {XtNsetFrameStyle, XtCSetFrameStyle, XtRFrameType, sizeof(XawFrameType),
   offset(button.set_frame_style), XtRImmediate, (XtPointer) XawSUNKEN },
};
#undef offset

static XtActionsRec actionsList[] = {
  {"toggle",	SwitchToggle},
};

SwitchClassRec switchClassRec = {
  {
    (WidgetClass) superclass,		/* superclass		  */	
    "Switch",				/* class_name		  */
    sizeof(SwitchRec),     	  	/* size			  */
    ClassInitialize,			/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    FALSE,				/* class_inited		  */
    NULL,				/* initialize		  */
    NULL,				/* initialize_hook	  */
    XtInheritRealize,			/* realize		  */
    actionsList,			/* actions		  */
    XtNumber(actionsList),		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* resource_count	  */
    NULLQUARK,				/* xrm_class		  */
    FALSE,				/* compress_motion	  */
    TRUE,				/* compress_exposure	  */
    TRUE,				/* compress_enterleave    */
    FALSE,				/* visible_interest	  */
    NULL,				/* destroy		  */
    XtInheritResize,			/* resize		  */
    XtInheritExpose,			/* expose		  */
    NULL,				/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,               	/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL				/* extension		  */
  },  /* CoreClass fields initialization */
  {
    XtInheritChangeSensitive		/* change_sensitive	  */ 
  },  /* SimpleClass fields initialization */
  {
    XtInheritXaw3dShadowDraw,           /* shadowdraw           */
  },  /* ThreeDClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* LabelClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* CommandClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* ButtonClass fields initialization */
  {
    0,                                     /* field not used    */
  }  /* SwitchClass fields initialization */
};

WidgetClass switchWidgetClass = (WidgetClass) &switchClassRec;


/*---------------------------------------------------*/
/* Private Procedures */
/*---------------------------------------------------*/

static void ClassInitialize(void)
{
  XawInitializeWidgetSet();
}

/*###################################################*/
/* Public Procedures */
/*###################################################*/

void
SwitchToggle(w,event,params,num_params)
  Widget w;
  XEvent *event;
  String *params;
  Cardinal *num_params;
{
  SwitchWidget sw = (SwitchWidget)w;

  BEGINMESSAGE(SwitchToggle)
  if (sw->command.set) {
    sw->command.highlighted = 0;
    ButtonReset(w,event,params,num_params);
  }
  else ButtonSet(w,event,params,num_params);
  ENDMESSAGE(SwitchToggle)
}

int SwitchIsSet(w)
  Widget w;
{
  SwitchWidget sw = (SwitchWidget)w;

  BEGINMESSAGE(SwitchIsSet)
  if (sw->command.set) return(1);
  else return(0);
  ENDMESSAGE(SwitchIsSet)
}
