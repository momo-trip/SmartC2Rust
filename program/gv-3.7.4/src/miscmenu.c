/*
**
** miscmenu.c
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(SimpleMenu.h)
#include INC_XAW(SmeBSB.h)
#include INC_XAW(SmeLine.h) 
#include INC_XAW(Cardinals.h)
#include INC_XAW(Scrollbar.h)

#include "types.h"
#include "actions.h"
#include "callbacks.h"
#include "miscmenu.h"
#include "options.h"
#include "main_resources.h"
#include "main_globals.h"

#include <string.h>

static MiscMenuEntryStruct miscmenu_entries[] = {
  { "update",cb_checkFile,(XtPointer)CHECK_FILE_DATE,2 },
  { "redisplay",cb_redisplay,NULL,3 },
  { "toggle_current" , cb_setPageMark, (XtPointer)(SPM_CURRENT|SPM_TOGGLE),1 },
  { "toggle_even"    , cb_setPageMark, (XtPointer)(SPM_EVEN|SPM_TOGGLE),1 },
  { "toggle_odd"     , cb_setPageMark, (XtPointer)(SPM_ODD|SPM_TOGGLE),1 },
  { "unmark",cb_setPageMark,(XtPointer)(SPM_ALL|SPM_UNMARK),1 },
  { "stop",cb_stopInterpreter,NULL,0 },
  { "print_all", cb_print, (XtPointer)PAGE_MODE_ALL,3 },
  { "print_marked",cb_print , (XtPointer)(PAGE_MODE_MARKED|PAGE_MODE_CURRENT),1 },
  { "save_all", cb_save, (XtPointer)PAGE_MODE_ALL, 3 },
  { "save_marked",cb_save , (XtPointer)(PAGE_MODE_MARKED|PAGE_MODE_CURRENT),1 },
  { "line",NULL,NULL,0 },
  { NULL,NULL,NULL,0 }
};

/*##################################################
  miscmenu_freeMiscMenuEntries
##################################################*/

void miscmenu_freeMiscMenuEntries(entries)
  MiscMenuEntry *entries;
{
  int i=0;
  BEGINMESSAGE(miscmenu_freeMiscMenuEntries)
  while (entries[i]) {
    XtFree((XtPointer)entries[i]);
    i++;
  }
  XtFree((XtPointer)entries);
  ENDMESSAGE(miscmenu_freeMiscMenuEntries)
}

/*##################################################
  miscmenu_parseMiscMenuEntries
##################################################*/

static MiscMenuEntry miscmenu_mallocMiscMenuEntry(void)
{
  MiscMenuEntry entry;
  entry = (MiscMenuEntry) XtMalloc(sizeof(MiscMenuEntryStruct));
  memset((void*)entry ,0,sizeof(MiscMenuEntryStruct));
  return entry;
}

MiscMenuEntry *miscmenu_parseMiscMenuEntries(s)
  char *s;
{
  char *c,*nl;
  MiscMenuEntry *entries,*mentries,entry;
  int i,j,n,have_entry=0;
  char name[100];

  BEGINMESSAGE(miscmenu_parseMiscMenuEntries)
  if (!s) s = "";
  s =options_squeezeMultiline(s);
  for (n=1,c=s; (c = strchr(c,'\n')); n++, c++);
  INFIMESSAGE(number of entries,n)
  mentries = entries = (MiscMenuEntry*) XtMalloc((n+2)*sizeof(MiscMenuEntry));
  c=s;
  if (*s) while (n>0) {
    nl = strchr(c,'\n'); 
    if (nl) *nl='\0';
    name[0]='\0';
    if (*c == '#' || *c == '!') i=0;
    else i=sscanf(c," %99s ",name);
    if (i==1) {
      for (j=0; miscmenu_entries[j].name; j++)
	if (!strcasecmp(miscmenu_entries[j].name,name)) { 
	  entry = miscmenu_mallocMiscMenuEntry();
	  *entry = miscmenu_entries[j];
	  have_entry = 1;
	  *entries++ = entry;
	}
    }
    n--;
    if (!nl) break;
    c=++nl;
  }
  if (!have_entry) {
    entry = miscmenu_mallocMiscMenuEntry();
    *entry = miscmenu_entries[0]; 
    *entries++ = entry;
  }
  *entries = (MiscMenuEntry) NULL;
  XtFree(s);
  ENDMESSAGE(miscmenu_parseMiscMenuEntries)
  return(mentries);
}

/*############################################################*/
/* miscmenu_a_miscMenu */
/*############################################################*/

#define MISC_MENU_IDLE		(1<<0)
#define MISC_MENU_STATE_1	(1<<1)
#define MISC_MENU_STATE_2	(1<<2)
#define MISC_MENU_INIT		(1<<3)
#define MISC_MENU_POPUP		(1<<4)
#define MISC_MENU_RESET		(1<<5)
#define MISC_MENU_CHECK		(1<<6)

void miscmenu_a_miscMenu(w, event, params, num_params)
  Widget w;
  XEvent *event;
  String *params;
  Cardinal *num_params; 
{
  static int mode = MISC_MENU_IDLE;
  static Widget gvw = (Widget)NULL;
  static Widget menuwidget = (Widget)NULL;
  static int xo=0,yo=0;
  int x,y;

  BEGINMESSAGE(miscmenu_a_miscMenu)

  if (!event) {
    INFMESSAGE(received reset request)
    if (menuwidget) XtDestroyWidget(menuwidget);
    menuwidget = (Widget)NULL;
    gvw = (Widget)NULL;
    mode = MISC_MENU_IDLE;
    ENDMESSAGE(miscmenu_a_miscMenu)
    return;
  }

  if (*num_params < 1) {
    INFMESSAGE(no parameter)
    ENDMESSAGE(miscmenu_a_miscMenu)
    return;
  }

# define MISC_MENU_HAVE(aaa,bbb) (!strcmp(params[0],(aaa)) && mode&(bbb))
  if      MISC_MENU_HAVE("init" ,  MISC_MENU_IDLE)    mode = (MISC_MENU_INIT	| MISC_MENU_STATE_1);
  else if MISC_MENU_HAVE("popup" , MISC_MENU_STATE_1) mode = (MISC_MENU_POPUP	| MISC_MENU_STATE_2);
  else if MISC_MENU_HAVE("check" , MISC_MENU_STATE_2) mode = (MISC_MENU_CHECK	| MISC_MENU_STATE_2);
  else if MISC_MENU_HAVE("reset" , (MISC_MENU_IDLE|MISC_MENU_STATE_1|MISC_MENU_STATE_2)) 
                                                      mode = (MISC_MENU_RESET	| MISC_MENU_STATE_2);
  else {
    INFMESSAGE(no mode)
    ENDMESSAGE(miscmenu_a_miscMenu)
    return;
  }
#   undef MISC_MENU_HAVE

  if (mode & MISC_MENU_INIT) {
    INFMESSAGE(MISC_MENU_INIT)
    if (w==page && (event->type == ButtonRelease || event->type == ButtonPress)) {
      xo = (int) event->xbutton.x_root;
      yo = (int) event->xbutton.y_root;
      gvw = w;
    } else {
      INFMESSAGE(event not in main window or not a button press or button release)
      mode = MISC_MENU_IDLE;
    }
    ENDMESSAGE(miscmenu_a_miscMenu)
    return;
  }

  if (mode & MISC_MENU_POPUP) {
    Arg args[4];
    Cardinal n;
    Widget entry=NULL;

    INFMESSAGE(MISC_MENU_POPUP)

    if (event->type != ButtonRelease && event->type != ButtonPress) {
      INFMESSAGE(event not a button press or button release)
      miscmenu_a_miscMenu(gvw,(XEvent*)NULL,NULL,NULL);
      ENDMESSAGE(miscmenu_a_miscMenu)
      return;
    }
    x = (int) event->xbutton.x_root;
    y = (int) event->xbutton.y_root;
    if (abs(x-xo)>1 || abs(y-yo)>1) {
      INFMESSAGE(succesive  events are unrelated)
      miscmenu_a_miscMenu(gvw,(XEvent*)NULL,NULL,NULL);
      ENDMESSAGE(miscmenu_a_miscMenu)
      return;
    }
    menuwidget = XtCreatePopupShell("miscMenu", simpleMenuWidgetClass,w,NULL,(Cardinal)0);
    {
      int i;
      MiscMenuEntry e;
      int menu_x, menu_y;
      Dimension menu_width,menu_height,entry_height,menu_border;
      Dimension screen_width,screen_height;
      Position button_x, button_y;
      for (i=0; gv_miscmenu_entries[i]; i++) {
        e = gv_miscmenu_entries[i];
        SMESSAGE(e->name)
	if (strcasecmp(e->name,"line")) {
	  entry = XtCreateManagedWidget(e->name, smeBSBObjectClass,menuwidget,NULL,(Cardinal)0);
	  if (e->cb) XtAddCallback(entry,XtNcallback,e->cb,e->client_data);
	  if (e->sensitivity_type)  {
	    Boolean b;
	    if      (e->sensitivity_type==1) b = (toc_text != NULL);
	    else if (e->sensitivity_type==2) b = (gv_filename != NULL);
	    else    b = (gv_psfile != NULL || (gv_gs_arguments && *gv_gs_arguments));
	    XtSetSensitive(entry,b);
	  }
	} else {
	  XtCreateManagedWidget("line", smeLineObjectClass,menuwidget,NULL,(Cardinal)0);
	} 
      }

      if (!XtIsRealized(menuwidget)) XtRealizeWidget(menuwidget);

                                                        n=0;
      XtSetArg(args[n], XtNheight, &entry_height);      ++n;
      XtGetValues(entry, args, n);

                                                        n=0;
      XtSetArg(args[n], XtNwidth, &menu_width);         ++n;
      XtSetArg(args[n], XtNheight, &menu_height);       ++n;
      XtSetArg(args[n], XtNborderWidth, &menu_border);  ++n;
      XtGetValues(menuwidget, args, n);

      XtTranslateCoords(w, event->xbutton.x, event->xbutton.y, &button_x, &button_y);
      menu_x = button_x-menu_width/2 -menu_border;
      menu_y = button_y-entry_height/2;

      screen_width = WidthOfScreen(XtScreen(menuwidget));
      screen_height = HeightOfScreen(XtScreen(menuwidget));

      if( menu_x + menu_width > screen_width && menu_width < screen_width )
	menu_x = screen_width - menu_width;
      if( menu_y + menu_height > screen_height && menu_height < screen_height )
	menu_y = screen_height - menu_height;

      n=0;
      XtSetArg(args[n], XtNx, menu_x);                  n++;
      XtSetArg(args[n], XtNy, menu_y);                  n++;
      XtSetValues(menuwidget, args, n);
      XtPopup(menuwidget,XtGrabExclusive);           
    }
    ENDMESSAGE(miscmenu_a_miscMenu)
    return;
  }

  if (mode & MISC_MENU_CHECK) {
    INFMESSAGE(MISC_MENU_CHECK)
    if (menuwidget) { 
      Arg args[5];
      Cardinal n;
      Position ulx,uly,lrx,lry,evx,evy;
      int rx,ry;
      Dimension width,height;
                                                         n=0;
      XtSetArg(args[n], XtNwidth,  &width);              n++;
      XtSetArg(args[n], XtNheight, &height);             n++;
      XtGetValues(menuwidget, args, n);
      XtTranslateCoords(menuwidget, 0, 0, &ulx, &uly);
      XtTranslateCoords(menuwidget, (Position)width, (Position)height, &lrx, &lry);
      IIMESSAGE(ulx,uly)
      IIMESSAGE(lrx,lry)
      {
	Window root, child;
	int dummyx, dummyy;
	unsigned int dummymask;
	XQueryPointer(XtDisplay(w), XtWindow(w), &root, &child, &rx, &ry,
		      &dummyx, &dummyy, &dummymask);
      }
      evx = (Position)rx;
      evy = (Position)ry;
      IIMESSAGE(evx,evy)
      if (evx<=ulx || evx >= lrx || evy <= uly || evy >= lry) {
	INFMESSAGE(pointer outside window)
        mode = mode | MISC_MENU_RESET;
      }
    }
  }

  if (mode & MISC_MENU_RESET) {
    INFMESSAGE(MISC_MENU_RESET)
    miscmenu_a_miscMenu(gvw,(XEvent*)NULL,NULL,NULL);
  }

  ENDMESSAGE(miscmenu_a_miscMenu)
}
























