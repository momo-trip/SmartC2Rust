/*
**
** MButton.c
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
#include "MButtonP.h"

static void ClassInitialize(void);
static void PopupMenu(Widget,XEvent*,String*,Cardinal*);

#define superclass ((ButtonWidgetClass)&buttonClassRec)

static char defaultTranslations[] = 
"Any<BtnDown>:set()PopupMenu()";

#define offset(field) XtOffsetOf(MButtonRec, field)
static XtResource resources[] = {
  {XtNmenuName, XtCMenuName, XtRString, sizeof(String), 
    offset(mbutton.menu_name), XtRString, (XtPointer)"menu"},
  {XtNsetFrameStyle, XtCSetFrameStyle, XtRFrameType, sizeof(XawFrameType),
   offset(button.set_frame_style), XtRImmediate, (XtPointer) XawRAISED },
};
#undef offset

static XtActionsRec actionsList[] =
{
  {"PopupMenu",	PopupMenu}
};

MButtonClassRec mbuttonClassRec = {
  {
    (WidgetClass) superclass,		/* superclass		  */	
    "MButton",				/* class_name		  */
    sizeof(MButtonRec),     	  	/* size			  */
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
  }  /* MbuttonClass fields initialization */
};

  /* for public consumption */
WidgetClass mbuttonWidgetClass = (WidgetClass) &mbuttonClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

static void ClassInitialize(void)
{
  XawInitializeWidgetSet();
  XtRegisterGrabAction(PopupMenu, True, 
		       (unsigned int)(ButtonPressMask | ButtonReleaseMask),
		       GrabModeAsync, GrabModeAsync);
}

void
PopupMenu(Widget w, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params)
{
  MButtonWidget mbw = (MButtonWidget) w;
  Widget temp;
  Widget menu;
  Arg args[5];
  Cardinal n;
  int menu_x, menu_y;
  Dimension menu_width, menu_height,button_width,button_height;
  Position button_x, button_y;

  BEGINMESSAGE(PopupMenu)

  temp = w;
  menu = NULL;
  while(temp != NULL) {
    menu = XtNameToWidget(temp, mbw->mbutton.menu_name);
    if (!menu) temp = XtParent(temp);
    else break;
  }

  if (!menu) {
#if 0
    char error_buf[BUFSIZ];
    (void) sprintf(error_buf, "Mbutton: %s %s.","Could not find menu widget named", mbw->mbutton.menu_name);
    XtAppWarning(XtWidgetToApplicationContext(w), error_buf);
#endif
    ENDMESSAGE(PopupMenu)
    return;
  }
  if (!XtIsRealized(menu)) XtRealizeWidget(menu);

  menu_width    = menu->core.width  + 2 * menu->core.border_width;
  menu_height   = menu->core.height + 2 * menu->core.border_width;
  button_width  = w->core.width     + 2 * w->core.border_width;
  button_height = w->core.height    + 2 * w->core.border_width;

  XtTranslateCoords(w, 0, 0, &button_x, &button_y);
  menu_x = button_x;
  menu_y = button_y;
  if (*num_params==0) menu_y += button_height;

  if (menu_x >= 0) {
    int scr_width = WidthOfScreen(XtScreen(menu));
    if (menu_x + menu_width > scr_width) menu_x = scr_width - menu_width;
  }
  if (menu_x < 0)  menu_x = 0;

  if (menu_y >= 0) {
    int scr_height = HeightOfScreen(XtScreen(menu));
    if (menu_y + menu_height > scr_height) menu_y = scr_height - menu_height;
  }
  if (menu_y < 0) menu_y = 0;

  n = 0;
  XtSetArg(args[n], XtNx, menu_x); n++;
  XtSetArg(args[n], XtNy, menu_y); n++;
  if (*num_params > 0 && menu_width<button_width) {
     XtSetArg(args[n], XtNwidth, button_width-2*menu->core.border_width); n++;
  }
  XtSetValues(menu, args, n);

  XtPopupSpringLoaded(menu);
  ENDMESSAGE(PopupMenu)
}

