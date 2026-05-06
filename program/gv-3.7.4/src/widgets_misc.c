/*
**
** widgets_misc.c
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
*/

#include "ac_config.h"


/*
#define MESSAGES
*/
#include "message.h"

#include <stdio.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(AsciiText.h)
#include INC_XAW(Cardinals.h)
#include "Button.h"
#include "Frame.h"
#include "Switch.h"

#include "types.h"
#include "main_resources.h"
#include "main_globals.h"
#include "widgets_misc.h"

/*##################################################*/
/* widgets_setSelectedBitmap */
/*##################################################*/

void 
widgets_setSelectedBitmap(w,on)
  Widget w;
  int on;
{
  Arg args[1];
  Pixmap bitmap;

  BEGINMESSAGE(widgets_setSelectedBitmap)
  if (on) bitmap = app_res.selected_bitmap;
  else    bitmap = None;
  XtSetArg(args[0], XtNleftBitmap, bitmap);
  XtSetValues(w,args, ONE);   
  ENDMESSAGE(widgets_setSelectedBitmap)
}

/*##################################################*/
/* widgets_setToggle */
/*##################################################*/

void
widgets_setToggle(w,value)
   Widget   w;
   int      value;
{
   BEGINMESSAGE(widgets_setToggle)
   if (value) ButtonSet(w,NULL,NULL,NULL);
   else ButtonUnset(w,NULL,NULL,NULL);
   ENDMESSAGE(widgets_setToggle)
}

/*##################################################*/
/* widgets_createLabeledLineTextField */
/* Create a label/single-line-text-field */
/* widget pair. */
/*##################################################*/

static char *LineTextTr =  "\
<Key>Return: no-op()\n\
<Key>Escape: no-op()\n\
<Key>Down: no-op()\n\
<Key>Up: no-op()\n\
<Key>Linefeed: no-op()\n\
Ctrl<Key>J: no-op()\n\
Ctrl<Key>M: no-op()\n\
Ctrl<Key>N: no-op()\n\
Ctrl<Key>O: no-op()\n\
Ctrl<Key>P: no-op()\n\
Ctrl<Key>R: no-op()\n\
Ctrl<Key>S: no-op()\n\
Ctrl<Key>V: no-op()\n\
Ctrl<Key>Z: no-op()\n\
Meta<Key>V: no-op()\n\
Meta<Key>Z: no-op()\n\
<Key>BackSpace: delete-previous-character()\n\
<Key>Delete: delete-next-character()\n\
<Key>Right: forward-character()\n\
<Key>Left: backward-character()\n\
<MapNotify>:display-caret(on,always)display-caret(off,always)\n\
<EnterNotify>:display-caret(on,always)\n\
<LeaveNotify>:display-caret(off,always)\
";

Widget
widgets_createLabeledLineTextField(name, parent)
   String   name;
   Widget   parent;
{
   Arg      args[9];
   Cardinal n;
   char     tmp[50];
   Widget   frame,text;
   static XtTranslations trans=NULL;
   
   INFMESSAGE(executing widgets_createLabeledLineTextField)
          sprintf(tmp,"%sLabel",name);

          XtSetArg(args[0], XtNresize, False);
   XtCreateManagedWidget(tmp,labelWidgetClass,parent,args,ONE);

          sprintf(tmp,"%sFrame",name);
   frame = XtCreateManagedWidget(tmp,frameWidgetClass,parent,NULL,ZERO);
          								n=0;
          XtSetArg(args[n], XtNtype, XawAsciiString);			n++;
          XtSetArg(args[n], XtNuseStringInPlace, False);		n++;
          XtSetArg(args[n], XtNscrollHorizontal, XawtextScrollNever);	n++;
          XtSetArg(args[n], XtNscrollVertical, XawtextScrollNever);	n++;
          XtSetArg(args[n], XtNdisplayCaret, False);			n++;
          XtSetArg(args[n], XtNeditType, XawtextEdit);			n++;
          XtSetArg(args[n], XtNresize, XawtextResizeWidth);		n++;
   text = XtCreateManagedWidget(name,asciiTextWidgetClass,frame,args,n);
          if (!trans) trans=XtParseTranslationTable(LineTextTr);
          XtOverrideTranslations(text,trans);

   return text;
}

/*##################################################*/
/* widgets_createLabeledTextField */
/* Create a label/text-field widget pair. */
/*##################################################*/

static char *TextTr =  "\
<Key>Escape: no-op()\n\
Ctrl<Key>J: no-op()\n\
Ctrl<Key>M: no-op()\n\
Ctrl<Key>N: no-op()\n\
Ctrl<Key>O: no-op()\n\
Ctrl<Key>P: no-op()\n\
Ctrl<Key>R: no-op()\n\
Ctrl<Key>S: no-op()\n\
Ctrl<Key>V: no-op()\n\
Ctrl<Key>Z: no-op()\n\
Meta<Key>V: no-op()\n\
Meta<Key>Z: no-op()\n\
<Key>BackSpace: delete-previous-character()\n\
<Key>Delete: delete-next-character()\n\
<Key>Right: forward-character()\n\
<Key>Left: backward-character()\n\
<MapNotify>:display-caret(on,always)display-caret(off,always)\n\
<EnterNotify>:display-caret(on,always)\n\
<LeaveNotify>:display-caret(off,always)\
";

Widget
widgets_createLabeledTextField(name, parent)
   String   name;
   Widget   parent;
{
   Arg      args[9];
   Cardinal n;
   char     tmp[50];
   Widget   frame,text;
   static XtTranslations trans=NULL;
   
   INFMESSAGE(executing widgets_createLabeledTextField)   
          sprintf(tmp,"%sLabel",name);

          XtSetArg(args[0], XtNresize, False);
   XtCreateManagedWidget(tmp,labelWidgetClass,parent,args,ONE);

          sprintf(tmp,"%sFrame",name);
   frame = XtCreateManagedWidget(tmp,frameWidgetClass,parent,NULL,ZERO);
          								n=0;
          XtSetArg(args[n], XtNtype, XawAsciiString);			n++;
          XtSetArg(args[n], XtNuseStringInPlace, False);		n++;
          XtSetArg(args[n], XtNscrollHorizontal, XawtextScrollNever);	n++;
          XtSetArg(args[n], XtNscrollVertical, XawtextScrollWhenNeeded);n++;
          XtSetArg(args[n], XtNdisplayCaret, False);			n++;
          XtSetArg(args[n], XtNeditType, XawtextEdit);			n++;
          XtSetArg(args[n], XtNresize, XawtextResizeWidth);		n++;
   text = XtCreateManagedWidget(name,asciiTextWidgetClass,frame,args,n);
          if (!trans) trans=XtParseTranslationTable(TextTr);
          XtOverrideTranslations(text,trans);

   return text;
}

/*##################################################*/
/* widgets_getText */
/*##################################################*/

char *
widgets_getText(w)
   Widget w;
{
   Arg    args[1];
   String value;

   BEGINMESSAGE(widgets_getText)
   XtSetArg(args[0], XtNstring, &value);
   XtGetValues(w, args, ONE);
   ENDMESSAGE(widgets_getText)
   return value;
}

/*##################################################*/
/* widgets_setText */
/*##################################################*/

void
widgets_setText(w,value)
  Widget w;
  String value;
{
  Arg args[1];

  BEGINMESSAGE(widgets_setText)
  if (value) {
    XtSetArg(args[0], XtNstring, value);
    XtSetValues(w,args,ONE);
    XtSetArg(args[0], XtNinsertPosition, strlen(value));
    XtSetValues(w,args,ONE);
  }
  ENDMESSAGE(widgets_setText)
}

/*##################################################*/
/* widgets_preferButton */
/*##################################################*/

void 
widgets_preferButton(w,prefer)
   Widget   w;
   int prefer;
{
   BEGINMESSAGE(widgets_preferButton)
   if (w) {
     if (prefer) ButtonHighlight(w,NULL,NULL,NULL);
     else        ButtonReset(w,NULL,NULL,NULL);
   }
   ENDMESSAGE(widgets_preferButton)
#if 0
      XEvent event;
      Position rx,ry;
      XtTranslateCoords(w, 0, 0, &rx, &ry);
      event.xcrossing.type        = prefer ? EnterNotify : LeaveNotify;
      event.xcrossing.display     = XtDisplay(w);
      event.xcrossing.window      = XtWindow(w);
      event.xcrossing.root        = RootWindowOfScreen(XtScreen(w));
      event.xcrossing.subwindow   = None;
      event.xcrossing.time        = 0;
      event.xcrossing.x           = 0;
      event.xcrossing.y           = 0;
      event.xcrossing.x_root      = (int)rx;
      event.xcrossing.y_root      = (int)ry;
      event.xcrossing.mode        = 0;
      event.xcrossing.detail      = 0;
      event.xcrossing.same_screen = 0;
      event.xcrossing.focus       = 0;
      event.xcrossing.state       = 0;
      XSendEvent(XtDisplay(w),XtWindow(w), False,
                 prefer ? EnterWindowMask : LeaveWindowMask, &event);
#endif
}










