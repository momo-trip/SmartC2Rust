/*
**
** note.c
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

#include "config.h"

#include <stdio.h>

#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(Label.h)
#include INC_X11(Shell.h)
#include "Aaa.h"
#include "Button.h"
#include "popup.h"

#ifndef max
#   define max(a, b)    ((a) > (b) ? (a) : (b))
#endif

/*### Application dependencies ##############################################*/

#define NOTE_TOPLEVEL	toplevel       			/* the Application Shell   */
#define NOTE_POPUP	notepopup
#define NOTE_POPUP_NAME	"notePopup"

#include "types.h"
#include "main_resources.h"
#include "main_globals.h"
#include "note.h"
#include "widgets_misc.h"

static Bool notePopupVisible = False;
static Bool notePopupCreated = False;
static Widget doneButton=NULL;

/*###############################################################################
   cb_popupNotePopup
###############################################################################*/

void cb_popupNotePopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_popupNotePopup)
   if (!notePopupVisible) {
      if (!notePopupCreated) makeNotePopup(); 
      popup_positionPopup(NOTE_POPUP,NOTE_TOPLEVEL,POPUP_POSITION_CENTER,0,0);
      widgets_preferButton(doneButton,1);
      XtPopup(NOTE_POPUP, XtGrabNone);
      notePopupVisible = True;
   }
   ENDMESSAGE(cb_popupNotePopup)
}      

/*###############################################################################
   cb_popdownNotePopup
###############################################################################*/

void cb_popdownNotePopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_popdownNotePopup)
   if (notePopupVisible) {
      XtPopdown(NOTE_POPUP);
      notePopupVisible = False;
   }
   ENDMESSAGE(cb_popdownNotePopup)
}

/*###############################################################################
   makeNotePopup
###############################################################################*/

void makeNotePopup(void)
{
   Arg          args[10];
   Cardinal     n;
   Widget   noteAaa;
   Widget   messageLabel;   

   BEGINMESSAGE(makeNotePopup)

							n=0;
        XtSetArg(args[n], XtNallowShellResize, True);	n++;
   NOTE_POPUP = XtCreatePopupShell(NOTE_POPUP_NAME,transientShellWidgetClass,NOTE_TOPLEVEL, args, n);

							n=0;
   noteAaa = XtCreateManagedWidget("noteAaa",aaaWidgetClass,NOTE_POPUP,args,n);

							n=0;
         XtSetArg(args[n], XtNresize, True);		++n;
   messageLabel = XtCreateManagedWidget("message", labelWidgetClass,noteAaa, args, n);

							n=0;
   doneButton = XtCreateManagedWidget("done", buttonWidgetClass,noteAaa, args, n);
         XtAddCallback(doneButton, XtNcallback, cb_popdownNotePopup,(XtPointer)NULL);

   XtRealizeWidget(NOTE_POPUP);

						n=0;
   XtSetArg(args[n], XtNminWidth, 100);		++n;
   XtSetArg(args[n], XtNminHeight, 50);		++n;
   XtSetArg(args[n], XtNmaxWidth, XtUnspecifiedShellInt);  ++n;
   XtSetArg(args[n], XtNmaxHeight, XtUnspecifiedShellInt); ++n;
   XtSetValues(NOTE_POPUP, args, n);

   XtSetKeyboardFocus(noteAaa,doneButton);
                                 
   XSetWMProtocols(XtDisplay(NOTE_POPUP),XtWindow(NOTE_POPUP),&wm_delete_window,1);
   notePopupCreated =True;

   ENDMESSAGE(makeNotePopup)
}

/*###############################################################################
    NotePopupShowMessage
###############################################################################*/

void
NotePopupShowMessage(String message)
{
    Arg args[5];
    Cardinal n;
    Widget label;

    BEGINMESSAGE(NotePopupShowMessage)
    if (!message) {INFMESSAGE(no message) ENDMESSAGE(NotePopupShowMessage) return;}
    if (!notePopupCreated) makeNotePopup(); 
    label = XtNameToWidget(NOTE_POPUP, "*message");
    n=0;
    XtSetArg(args[n], XtNlabel, message); n++;
    XtSetValues(label, args, n);
    cb_popupNotePopup((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);
    ENDMESSAGE(NotePopupShowMessage)
}
