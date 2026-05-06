/*
**
** version.c
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
#include INC_XMU(Misc.h)
#include INC_XMU(Converters.h)
#include INC_XAW(Form.h)
#include INC_XAW(Label.h)
#include INC_X11(Shell.h)
#include "AaaP.h"
#include "Button.h"

#include "types.h"
#include "main_resources.h"
#include "main_globals.h"
#include "popup.h"
#include "resource.h"

/*### local declarations and application dependencies ##################*/

#include "versionp.h"

static void makeVersionPopup(Widget, String);

static	Bool	versionPanelCreated = False;
static	Bool	version_up = False;

/*###############################################################################
   cb_popdownVersionPopup
   Pop down the version information panel.
###############################################################################*/

void
cb_popdownVersionPopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_popdownVersionPopup)
   if (version_up) {
      XtPopdown(versionpopup);
      version_up = False;
   }
   if (versionPanelCreated==True) {
      INFMESSAGE(destroying versionpopup)
      XtDestroyWidget(versionpopup);
      versionpopup=NULL;
      versionPanelCreated = False;
   }
   ENDMESSAGE(cb_popdownVersionPopup)
}   

/*###############################################################################
   cb_popupVersionPopup
   Pop up the version information panel.
###############################################################################*/

void
cb_popupVersionPopup(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_popupVersionPopup)
   if (!version_up) {
      if (versionPanelCreated==False) {
         makeVersionPopup(VERSION_TOPLEVEL,"versionPopup");
	 popup_positionPopup(versionpopup,viewFrame,POPUP_POSITION_POS,4,4);
      }
      XtPopup(versionpopup,XtGrabExclusive);
      version_up=True;
   }
   ENDMESSAGE(cb_popupVersionPopup)
}   

/*########################################################################
   SetVersionPopupLayout
########################################################################*/

/*

static char *VersionPopup_layout = etc. etc.

static void SetVersionPopupLayout(Widget l)
{
   static XrmValue  	from,to;
   static Bool 		success=False;
   static BoxPtr	layout;
   Arg args[5];
   Cardinal n;

   BEGINMESSAGE(SetVersionPopupLayout)
   SMESSAGE(XtName(l))

   from.size = strlen(VersionPopup_layout)+1; from.addr = VersionPopup_layout;
   to.size = sizeof(BoxPtr *); to.addr = (char*) ((BoxPtr *) &layout);

   if (success==False) {
      INFMESSAGE(converting)
      success = XtConvertAndStore (
                   l,
                   XtRString, (XrmValuePtr) &(from),
                   XtRLayout, (XrmValuePtr) &(to)
                );
   }

   if (success) {
                                                   n=0;
      XtSetArg(args[n],XtNlayout,(BoxPtr) layout); ++n;
      XtSetValues(l,args,n);
      ENDMESSAGE(SetVersionPopupLayout)
      return;
   }

   XtError("Version Popup cannot set internal Layout.");
}

*/

/*###############################################################################
   makeVersionPopup
   create the X panel to display version info
###############################################################################*/

void
makeVersionPopup(Widget parent, String name)
{
   char		**line;
   Arg		args[10];
   Cardinal	n;
   Widget	versionControl;
   Widget	versionDone;
   Dimension	w, h;

   BEGINMESSAGE(makeVersionPopup)
   if (versionPanelCreated==True) {
      INFMESSAGE(version popup already exists) ENDMESSAGE(makeVersionPopup)
   }  

                  n=0;
                  XtSetArg(args[n], XtNtitle, VERSION_SHELL_TITLE);  n++;
   versionpopup = XtCreatePopupShell(name,transientShellWidgetClass,parent,args,n);

                  n=0;
   versionControl  = XtCreateManagedWidget("versionControl",aaaWidgetClass,versionpopup,args,n);
/*
                  SetVersionPopupLayout(versionControl);
*/

                  line = versionIdentification;
		  n=0;
		  XtSetArg(args[n], XtNborderWidth, 0); n++;
		  XtSetArg(args[n], XtNjustify, XtJustifyCenter);  n++;
                  {
                     char i = 'a';
                     char widgetName[15];
                     while (*line != NULL)  {
                        sprintf(widgetName,"versionText%c",i);
                        XtSetArg(args[n], XtNlabel, *line); 
                        XtCreateManagedWidget(widgetName,labelWidgetClass,versionControl,args,(n+1));
                        line++; i++;
                     }
                  }        

		  n=0;
		  XtSetArg(args[n], XtNborderWidth, 0);            n++;
		  XtSetArg(args[n], XtNjustify, XtJustifyCenter);  n++;
                  XtSetArg(args[n], XtNlabel, copyright);          n++; 
                  XtCreateManagedWidget("license",labelWidgetClass,versionControl,args,n);

		  n=0;
   versionDone  = XtCreateManagedWidget("versionDone",buttonWidgetClass,versionControl, args, n);
	          XtAddCallback(versionDone, XtNcallback, cb_popdownVersionPopup, NULL);

                  line = author;
		  n=0;
		  XtSetArg(args[n], XtNborderWidth, 0); n++;
		  XtSetArg(args[n], XtNjustify, XtJustifyCenter);  n++;
                  {
                     char i = 'a';
                     char widgetName[10];
                     while (*line != NULL)  {
                        char tmp[1000];
                        sprintf(widgetName,"author%c",i);
			if ( strcmp(copyrightTranslationLabel, "N/A") )
			{
			   char tmp2[1000];
			   strcpy(tmp2, "Translation:         ");
			   strcat(tmp2, copyrightTranslationLabel);
			   sprintf(tmp, *line, tmp2);
			}
			else
			{
			   sprintf(tmp, *line, "");
			}
                        XtSetArg(args[n], XtNlabel, tmp); 
                        XtCreateManagedWidget(widgetName,labelWidgetClass,versionControl,args,(n+1));
                        line++; i++;
                     }
                  }

   XtRealizeWidget(versionpopup);
   
   AaaWidgetGetNaturalSize((AaaWidget)versionControl,&w,&h);
   INFIMESSAGE(natural width:,w) INFIMESSAGE(natural height:,h)
                                       n=0;
   XtSetArg(args[n], XtNminWidth, w);  ++n;
   XtSetArg(args[n], XtNminHeight, h); ++n;
   XtSetArg(args[n], XtNmaxWidth, w);  ++n;
   XtSetArg(args[n], XtNmaxHeight, h); ++n;
   XtSetValues(versionpopup, args, n);
   XSetWMProtocols(XtDisplay(versionpopup),XtWindow(versionpopup),&wm_delete_window,1);

   versionPanelCreated = True;

   ENDMESSAGE(makeVersionPopup)
}
