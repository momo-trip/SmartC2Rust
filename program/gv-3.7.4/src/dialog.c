/*
**
** dialog.c
**
** Copyright (C) 1995, 1996, 1997 Johannes Plass
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

#include "config.h"
#ifdef HAVE_XAW3D_INTERNATIONAL
   #include "VlistP.h"
#endif

#include <stdio.h>

/*
#define MESSAGES
*/
#include "message.h"

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_X11(Xos.h)
#include INC_XAW(AsciiText.h)
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

#ifndef max
#   define max(a, b)    ((a) > (b) ? (a) : (b))
#endif

/*### Application dependencies ##############################################*/

#define DIALOG_TOPLEVEL		toplevel
#define DIALOG_POPUP_NAME	"dialogPopup"
#define DIALOG_POPUP		dialogpopup

#include "note.h"
#include "dialog.h"

static Bool dialogPopupCreated = False;
static Bool dialogPopupVisible = False;
static Widget doneButton=NULL,cancelButton=NULL,dialogText=NULL;
static int preferedButton=0;

/*###############################################################################
   cb_popdownDialogPopup
###############################################################################*/

void cb_popdownDialogPopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_popdownDialogPopup)
   if (dialogPopupVisible) {
      cb_popdownNotePopup((Widget)NULL,(XtPointer)NULL,NULL);
      XtPopdown(DIALOG_POPUP);
      dialogPopupVisible=False;
   }
   ENDMESSAGE(cb_popdownDialogPopup)
}

/*###############################################################################
   cb_popupDialogPopup
###############################################################################*/

void cb_popupDialogPopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_popupDialogPopup)
   if (!dialogPopupVisible) {
      if (!dialogPopupCreated) makeDialogPopup();
      preferedButton=DIALOG_BUTTON_CANCEL;
      action_preferDialogPopupButton(NULL, NULL, NULL, NULL);
      popup_positionPopup(dialogpopup,viewFrame,POPUP_POSITION_POS,4,4);
      XtPopup(DIALOG_POPUP,XtGrabNone);
      dialogPopupVisible=True;
   }
   ENDMESSAGE(cb_popupDialogPopup)
}

/*###############################################################################
   action_preferDialogPopupButton
###############################################################################*/

void
action_preferDialogPopupButton(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
   Widget pref,pref_old;

   BEGINMESSAGE(action_preferDialogPopupButton)
   if (preferedButton==DIALOG_BUTTON_DONE) {
      pref=cancelButton;
      pref_old=doneButton;
      preferedButton=DIALOG_BUTTON_CANCEL;
   } else {
      pref=doneButton;
      pref_old=cancelButton;
      preferedButton=DIALOG_BUTTON_DONE;
   }
   widgets_preferButton(pref,1);
   widgets_preferButton(pref_old,0);
   XtInstallAccelerators(dialogText,pref_old);
   XtInstallAccelerators(dialogText,pref);
   ENDMESSAGE(action_preferDialogPopupButton)
}

/*###############################################################################
   makeDialogPopup
###############################################################################*/

static char *tabTrans =
"\
<Key>Tab: GV_TogDialPrefBut()\n\
<EnterNotify>:no-op()\n\
<LeaveNotify>:no-op()\n\
<MapNotify>:display-caret(on,always)\
";

void makeDialogPopup(void)
{
   Arg          args[7];
   Cardinal     n;
   Dimension	minw,minh;
   Widget   dialogAaa;
   static XtTranslations tab_trans=(XtTranslations)NULL;

   BEGINMESSAGE(makeDialogPopup)

					n=0;
        XtSetArg(args[n], XtNallowShellResize, True);  n++;
   DIALOG_POPUP = XtCreatePopupShell(DIALOG_POPUP_NAME,transientShellWidgetClass,DIALOG_TOPLEVEL, args, n);

					n=0;
        XtSetArg(args[n], XtNresizeWidth, True);	n++;
        XtSetArg(args[n], XtNresizeHeight, True);	n++;
        XtSetArg(args[n], XtNmaximumWidth, 0);	 	n++;
        XtSetArg(args[n], XtNmaximumHeight,0);	 	n++;
   dialogAaa = XtCreateManagedWidget("dialogAaa",aaaWidgetClass,DIALOG_POPUP,args,n);
 
        				n=0;
   XtCreateManagedWidget("message", labelWidgetClass,dialogAaa, args, n);

        				n=0;
   doneButton=XtCreateManagedWidget("done",buttonWidgetClass,dialogAaa,args,n);
   cancelButton=XtCreateManagedWidget("cancel",buttonWidgetClass,dialogAaa,args,n);
 
   dialogText=widgets_createLabeledLineTextField("dialog_text", dialogAaa);

   XtRealizeWidget(DIALOG_POPUP);

   AaaWidgetGetNaturalSize((AaaWidget)dialogAaa,&minw,&minh);
   IMESSAGE(minw) IMESSAGE(minh)

                                          n=0;
   XtSetArg(args[n], XtNminWidth, minw);  ++n;
   XtSetArg(args[n], XtNminHeight, minh); ++n;
   XtSetArg(args[n], XtNmaxWidth, XtUnspecifiedShellInt);  ++n;
   XtSetArg(args[n], XtNmaxHeight, minh); ++n;
   XtSetValues(DIALOG_POPUP, args, n);

   /*>>>*/
      if (!tab_trans) tab_trans=XtParseTranslationTable(tabTrans);
      XtOverrideTranslations(dialogText,tab_trans);
      XtSetKeyboardFocus(dialogAaa,dialogText);
					      n=0;
      XtSetArg(args[n],XtNdisplayCaret,True); n++;
      XtSetValues(dialogText, args, n);
   /*<<<*/


   XSetWMProtocols(XtDisplay(DIALOG_POPUP),XtWindow(DIALOG_POPUP),&wm_delete_window,1);
   dialogPopupCreated=True;
                              
   ENDMESSAGE(makeDialogPopup)
}

/*###############################################################################
    DialogPopupSetButton
###############################################################################*/

void DialogPopupSetButton(int button, String label, XtCallbackProc callback)
{
    Arg args[5];
    Cardinal n;
    Widget response=NULL;

    BEGINMESSAGE(DialogPopupSetButton)
    if (!dialogPopupCreated) makeDialogPopup();
    if       (button&DIALOG_BUTTON_DONE)   response = doneButton;
    else if  (button&DIALOG_BUTTON_CANCEL) response = cancelButton;

    if (response) {
       if (label) {
          n=0;
          XtSetArg(args[n], XtNlabel, label);  n++;
          XtSetValues(response, args, n);
       }
       if (callback) {
          XtRemoveAllCallbacks(response,XtNcallback);
          XtAddCallback(response, XtNcallback,callback,NULL);
       }
    }
    ENDMESSAGE(DialogPopupSetButton)
}

/*###############################################################################
    DialogPopupClearText
###############################################################################*/

void DialogPopupClearText(void)
{
    Arg args[5];
    Cardinal n;
    Widget response;

    BEGINMESSAGE(DialogPopupClearText)
    if (!dialogPopupCreated) makeDialogPopup();
    response = XtNameToWidget(DIALOG_POPUP, "*dialog_text");
    n=0;
    XtSetArg(args[n], XtNstring, "");  n++;
    XtSetValues(response, args, n);
    ENDMESSAGE(DialogPopupClearText)
}

/*###############################################################################
    DialogPopupSetText
###############################################################################*/

void DialogPopupSetText(String s)
{
    Arg args[10];
    Cardinal n;
    Widget response;
#ifdef HAVE_XAW3D_INTERNATIONAL
    VlistWidget vw;
#endif
    Dimension width;
#   define DIALOG_POPUP_FREE_SPACE 50
#   define DIALOG_POPUP_TEXT_MIN_WIDTH 100
    String os;
    Dimension lm,rm,bw,nw;
#ifdef HAVE_XAW3D_INTERNATIONAL
    XFontSet     fontset;
#endif
    XFontStruct *font;

    BEGINMESSAGE(DialogPopupSetText)
    if (!s) {INFMESSAGE(no text) ENDMESSAGE(DialogPopupSetText) return;}
    if (!dialogPopupCreated) makeDialogPopup();
    response = XtNameToWidget(DIALOG_POPUP, "*dialog_text");
    						n=0;
#ifdef HAVE_XAW3D_INTERNATIONAL
    vw = (VlistWidget)response;
    if( vw->simple.international == True ) {
      XtSetArg(args[n], XtNfontSet, &fontset);	n++;
    } else {
#endif
      XtSetArg(args[n], XtNfont, &font);	n++;
#ifdef HAVE_XAW3D_INTERNATIONAL
    }
#endif
    XtSetArg(args[n], XtNleftMargin, &lm);	n++;
    XtSetArg(args[n], XtNrightMargin, &rm);	n++;
    XtSetArg(args[n], XtNborderWidth, &bw);	n++;
    XtSetArg(args[n], XtNstring, &os);		n++;
    XtSetArg(args[n], XtNwidth, &width);	n++;
    XtGetValues(response, args, n);
    						n=0;
#ifdef HAVE_XAW3D_INTERNATIONAL
    if( vw->simple.international == True )
      nw=	XmbTextEscapement( fontset, s, strlen(s));
    else
#endif
      nw= ((font->max_bounds.width+font->min_bounds.width)*((Dimension)strlen(s))+1)/2;
    nw += lm+rm+DIALOG_POPUP_FREE_SPACE+2*bw;

    if (nw<DIALOG_POPUP_TEXT_MIN_WIDTH) nw=DIALOG_POPUP_TEXT_MIN_WIDTH;
    if (nw>width) {
      INFIMESSAGE(new width,nw)
      XtSetArg(args[n], XtNwidth,nw);	n++;
    }
    XtSetArg(args[n], XtNstring, s);		n++;
    XtSetValues(response, args, n);
    						n=0;
    XtSetArg(args[n], XtNinsertPosition,strlen(s));	n++;
    XtSetValues(response, args, n);
 
    ENDMESSAGE(DialogPopupSetText)
}

/*###############################################################################
    DialogPopupGetText
###############################################################################*/
 
String DialogPopupGetText(void)
{
    Arg args[5];
    Cardinal n;
    Widget response;
    String s;
    static char string[256];
    int l;

    BEGINMESSAGE(DialogPopupGetText)
    if (!dialogPopupCreated) makeDialogPopup();
    response = XtNameToWidget(DIALOG_POPUP, "*dialog_text");
    n=0;
    XtSetArg(args[n], XtNstring, &s);   n++;
    XtGetValues(response, args, n);
    l= strlen(s);
    if (l<255) strcpy(string,s);
    else { strncpy(string,s,255); string[255]='\0'; }
    ENDMESSAGE(DialogPopupGetText)
    return string;
}

/*###############################################################################
    DialogPopupSetPrompt
###############################################################################*/

void DialogPopupSetPrompt(String newprompt)
{
    Arg args[5];
    Cardinal n;
    Widget label;

    BEGINMESSAGE(DialogPopupSetPrompt)
    if (!dialogPopupCreated) makeDialogPopup();
    label = XtNameToWidget(DIALOG_POPUP, "*dialog_textLabel");
    n=0;
    XtSetArg(args[n], XtNlabel, newprompt); n++;
    XtSetValues(label, args, n);
    ENDMESSAGE(DialogPopupSetPrompt)
}

/*###############################################################################
    DialogPopupSetMessage
###############################################################################*/

void DialogPopupSetMessage(String message)
{
    Arg args[5];
    Cardinal n;
    Widget label;

    BEGINMESSAGE(DialogPopupSetMessage)
    if (!dialogPopupCreated) makeDialogPopup();
    label = XtNameToWidget(DIALOG_POPUP, "*message");
    n=0;
    XtSetArg(args[n], XtNlabel, message); n++;
    XtSetValues(label, args, n);
    ENDMESSAGE(DialogPopupSetMessage)
}
