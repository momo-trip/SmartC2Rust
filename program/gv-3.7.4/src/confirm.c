/*
**
** confirm.c
**
** Copyright (C) 1995, 1996 Johannes Plass
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
** Author:   Johannes Plass (plass@dipmza.physik.uni-mainz.de)
**           Department of Physic
**           Johannes Gutenberg-University
**           Mainz, Germany
**
**           Jose E. Marchesi (jemarch@gnu.org)
**           GNU Project
**
*/
#include "ac_config.h"

#include "config.h"

#include <stdio.h>

/*
#define MESSAGES
*/
#include "message.h"

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(Label.h)
#include INC_X11(Shell.h)

#include "Aaa.h"
#include "Button.h"
#include "Ghostview.h"

#include "types.h"
#include "main_resources.h"
#include "main_globals.h"
#include "popup.h"
#include "widgets_misc.h"

/*### Application dependencies ##############################################*/

#define CONFIRM_TOPLEVEL	toplevel
#define CONFIRM_POPUP_NAME	"confirmPopup"
#define CONFIRM_POPUP		confirmpopup

#include "confirm.h"

static Bool confirmPopupCreated = False;
static Bool confirmPopupVisible = False;
static Widget doneButton=NULL,cancelButton=NULL,confirmAaa=NULL;
static Widget message1=NULL,message2=NULL,message3=NULL;
static int preferedButton=0,initialButton=0;

/*########################################################
   cb_popdownConfirmPopup
#########################################################*/

void cb_popdownConfirmPopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_popdownConfirmPopup)
  if (confirmPopupVisible) {
    XtPopdown(CONFIRM_POPUP);
    confirmPopupVisible=False;
  }
  if (confirmPopupCreated==True) {
    INFMESSAGE(destroying confirmpopup)
    XtDestroyWidget(confirmpopup);
    confirmpopup=NULL;
    confirmPopupCreated = False;
    message1=message2=message3=NULL;
  }
  ENDMESSAGE(cb_popdownConfirmPopup)
}

/*########################################################
   cb_popupConfirmPopup
#########################################################*/

void cb_popupConfirmPopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_popupConfirmPopup)
  if (!confirmPopupVisible) {
    if (!confirmPopupCreated) makeConfirmPopup();
    if      (initialButton==CONFIRM_BUTTON_DONE)   preferedButton=CONFIRM_BUTTON_CANCEL;
    else if (initialButton==CONFIRM_BUTTON_CANCEL) preferedButton=CONFIRM_BUTTON_DONE;
    action_preferConfirmPopupButton(NULL, NULL, NULL, NULL);
    popup_positionPopup(confirmpopup,viewFrame,POPUP_POSITION_POS,4,4);
    XtPopup(CONFIRM_POPUP,XtGrabNone);
    confirmPopupVisible=True;
  }
  ENDMESSAGE(cb_popupConfirmPopup)
}

/*########################################################
   action_preferConfirmPopupButton
#########################################################*/

void
action_preferConfirmPopupButton(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  Widget pref,pref_old;

  BEGINMESSAGE(action_preferConfirmPopupButton)
  if (preferedButton==CONFIRM_BUTTON_DONE) {
    pref=cancelButton;
    pref_old=doneButton;
    preferedButton=CONFIRM_BUTTON_CANCEL;
  } else {
    pref=doneButton;
    pref_old=cancelButton;
    preferedButton=CONFIRM_BUTTON_DONE;
  }
  widgets_preferButton(pref,1);
  widgets_preferButton(pref_old,0);
  XtInstallAccelerators(confirmAaa,pref);
  ENDMESSAGE(action_preferConfirmPopupButton)
}

/*########################################################
    ConfirmPopupSetButton
#########################################################*/

void ConfirmPopupSetButton(int button, XtCallbackProc callback)
{
  Widget response=NULL;

  BEGINMESSAGE(ConfirmPopupSetButton)
  if (!confirmPopupCreated) makeConfirmPopup();
  if       (button&CONFIRM_BUTTON_DONE)   response = doneButton;
  else if  (button&CONFIRM_BUTTON_CANCEL) response = cancelButton;

  if (response && callback) {
    XtRemoveAllCallbacks(response,XtNcallback);
    XtAddCallback(response, XtNcallback,callback,NULL);
  }
  ENDMESSAGE(ConfirmPopupSetButton)
}

/*########################################################
    ConfirmPopupSetMessage
#########################################################*/

void ConfirmPopupSetMessage(String which, String message)
{
  Arg args[3];
  Cardinal n;
  char *name=NULL;
  Widget *labelP = NULL;

  BEGINMESSAGE(ConfirmPopupSetMessage)
  if (!confirmPopupCreated) makeConfirmPopup();
  if      (!strcmp(which,"1")) { labelP = &message1; name="message1"; }
  else if (!strcmp(which,"2")) { labelP = &message2; name="message2"; }
  else if (!strcmp(which,"3")) { labelP = &message3; name="message3"; }
  else {
    ENDMESSAGE(ConfirmPopupSetMessage)
    return;
  }
  if (message) {
        	  			  n=0;
    XtSetArg(args[n], XtNlabel, message); n++;
    XtSetArg(args[n], XtNborderWidth, 0); n++;
    if (!(*labelP))
      *labelP=XtCreateManagedWidget(name,labelWidgetClass,confirmAaa, args, n);
    else
      XtSetValues(*labelP,args,n);
  } else {
    if (*labelP) {
        	  			  n=0;
      XtSetArg(args[n], XtNheight,0);     n++;
      XtSetValues(*labelP,args,n);
      XtDestroyWidget(*labelP);
     *labelP = NULL;
    }
  }
  ENDMESSAGE(ConfirmPopupSetMessage)
}

/*########################################################
    ConfirmPopupSetInitialButton
#########################################################*/

void ConfirmPopupSetInitialButton(int button)
{
  BEGINMESSAGE(ConfirmPopupSetInitialButton)
  initialButton=button;
  ENDMESSAGE(ConfirmPopupSetInitialButton)
}

/*########################################################
   makeConfirmPopup
#########################################################*/

static char *tabTrans =
"\
<Key>Tab: GV_TogConfPrefBut()\
";

void makeConfirmPopup(void)
{
  Arg args[7];
  Cardinal n;
  static XtTranslations tab_trans=(XtTranslations)NULL;

  BEGINMESSAGE(makeConfirmPopup)

					                n=0;
        XtSetArg(args[n], XtNallowShellResize, True);   n++;
  CONFIRM_POPUP = XtCreatePopupShell(CONFIRM_POPUP_NAME,transientShellWidgetClass,CONFIRM_TOPLEVEL, args, n);

					                n=0;
        XtSetArg(args[n], XtNresizeWidth, True);	n++;
        XtSetArg(args[n], XtNresizeHeight, True);	n++;
   confirmAaa = XtCreateManagedWidget("confirmAaa",aaaWidgetClass,CONFIRM_POPUP,args,n);
 
			                                n=0;
   doneButton=XtCreateManagedWidget("done",buttonWidgetClass,confirmAaa,args,n);
   cancelButton=XtCreateManagedWidget("cancel",buttonWidgetClass,confirmAaa,args,n);

   message1=message2=message3=NULL;
 
   XtRealizeWidget(CONFIRM_POPUP);

   XtSetKeyboardFocus(confirmAaa,confirmAaa);
   if (!tab_trans) tab_trans=XtParseTranslationTable(tabTrans);
   XtOverrideTranslations(confirmAaa,tab_trans);

   XSetWMProtocols(XtDisplay(CONFIRM_POPUP),XtWindow(CONFIRM_POPUP),&wm_delete_window,1);
   confirmPopupCreated=True;
                              
   ENDMESSAGE(makeConfirmPopup)
}
