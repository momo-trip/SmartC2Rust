/*
**
** magmenu.c
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

#include <stdlib.h>
#include <math.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(SimpleMenu.h)
#include INC_XAW(SmeBSB.h)
#include INC_XAW(SmeLine.h) 
#include INC_XAW(Cardinals.h)
#include INC_XAW(Scrollbar.h)
#include "Ghostview.h"

#include "types.h"
#include "actions.h"
#include "magmenu.h"
#include "options.h"
#include "main_resources.h"
#include "main_globals.h"
#include "zoom.h"

/*##################################################
  magmenu_freeMagMenuEntries
##################################################*/

void magmenu_freeMagMenuEntries(entries)
  MagMenuEntry *entries;
{
  int i=0;
  BEGINMESSAGE(magmenu_freeMagMenuEntries)
  while (entries[i]) {
    XtFree(entries[i]->name);
    XtFree((XtPointer)entries[i]);
    i++;
  }
  XtFree((XtPointer)entries);
  ENDMESSAGE(magmenu_freeMagMenuEntries)
}

/*##################################################
  magmenu_parseMagMenuEntries
##################################################*/

static MagMenuEntry magmenu_mallocMagMenuEntry(void)
{
  MagMenuEntry entry;
  entry = (MagMenuEntry) XtMalloc(sizeof(MagMenuEntryStruct));
  memset((void*)entry ,0,sizeof(MagMenuEntryStruct));
  return entry;
}

MagMenuEntry *magmenu_parseMagMenuEntries(s)
  char *s;
{
  char *c,*nl;
  MagMenuEntry *entries,*mentries,entry;
  int i,n,have_entry=0;
  float f;
  char name[100];

  BEGINMESSAGE(magmenu_parseMagMenuEntries)
  if (!s) s = "";
  s =options_squeezeMultiline(s);
  for (n=1,c=s; (c = strchr(c,'\n')); n++, c++);
  INFIMESSAGE(number of medias,n)
  mentries = entries = (MagMenuEntry*) XtMalloc((n+2)*sizeof(MagMenuEntry));
  c=s;
  if (*s) while (n>0) {
    nl = strchr(c,'\n'); 
    if (nl) *nl='\0';
    name[0]='\0';
    if (*c == '#' || *c == '!') i=0;
    else i=sscanf(c," %[^,] , %f ",name,&f);
    if (i==2 && f>0) {
      entry = magmenu_mallocMagMenuEntry();
      entry->name = XtNewString(name);
      entry->scale= sqrt(f);
      have_entry = 1;
      INFSMESSAGE(found entry,entry->name)
      FMESSAGE(entry->scale)
      *entries++ = entry;
    }
    n--;
    if (!nl) break;
    c=++nl;
  }
  if (!have_entry) {
    entry = magmenu_mallocMagMenuEntry();
    entry->name = XtNewString("  2  ");
    entry->scale= sqrt(2.0);
    *entries++ = entry;
  }
  *entries = (MagMenuEntry) NULL;
  XtFree(s);
  ENDMESSAGE(magmenu_parseMagMenuEntries)
  return(mentries);
}

/*############################################################*/
/* magmenu_a_magMenu */
/*############################################################*/

typedef struct
{
   Position locx1,locy1,locx2,locy2;
   Position oldx,oldy,oldwidth,oldheight;
   Widget menuwidget;
} magMenu_data;

#define MAG_INIT_0	(1<<0)
#define MAG_INIT_1	(1<<1)
#define MAG_INIT_2	(1<<2)
#define MAG_BEGIN	(1<<3)
#define MAG_EXTEND	(1<<4)
#define MAG_CHOOSE	(1<<5)
#define MAG_SHOW	(1<<6)
#define MAG_RESET	(1<<7)
#define MAG_CHECK	(1<<8)

void
magmenu_a_magMenu (w, event, params, num_params)
   Widget w;
   XEvent *event;
   String *params;
   Cardinal *num_params; 
{
   static magMenu_data *d = (magMenu_data *)NULL;
   static int mode=MAG_INIT_0;
   GhostviewReturnStruct ret_val;
   Bool popup_zoom = False;
   static Widget gvw = NULL;

   BEGINMESSAGE(magmenu_a_magMenu)

   if (!event) {
      INFMESSAGE(received reset request)
      if (d) {
         INFMESSAGE(resetting)
         if (d->menuwidget) XtDestroyWidget(d->menuwidget);
         XtFree((char*)d);
         d = (magMenu_data *)NULL;
      }
      mode = MAG_INIT_0;
      ENDMESSAGE(magmenu_a_magMenu)
      return;
   }

   if (*num_params < 1) {
      INFMESSAGE(no parameter) ENDMESSAGE(magmenu_a_magMenu)
      return;
   }

#   define MAG_HAVE(aaa,bbb) (!strcmp(params[0],(aaa)) && mode&(bbb))
    if      MAG_HAVE("begin"  , MAG_INIT_0) mode = (MAG_BEGIN		| MAG_INIT_1);
    else if MAG_HAVE("extend" , MAG_INIT_1) mode = (MAG_EXTEND		| MAG_INIT_1);
    else if MAG_HAVE("choose" , MAG_INIT_1) mode = (MAG_CHOOSE		| MAG_INIT_2);
    else if MAG_HAVE("check"  , MAG_INIT_2) mode = (MAG_CHECK		| MAG_INIT_2);
    else if MAG_HAVE("show"   , MAG_INIT_2) mode = (MAG_SHOW|MAG_RESET	| MAG_INIT_2);
    else if MAG_HAVE("reset"  , (MAG_INIT_0|MAG_INIT_1|MAG_INIT_2)) 
                                            mode = (MAG_RESET		| MAG_INIT_2);
    else {
         INFMESSAGE(no mode) ENDMESSAGE(magmenu_a_magMenu)
         return;
    }
#   undef MAG_HAVE

    if (mode&MAG_BEGIN) {
       INFMESSAGE(MAG_BEGIN)
       gvw = w;
       d = (magMenu_data *) XtMalloc(sizeof(magMenu_data));
       d->locx1 = event->xbutton.x;
       d->locy1 = event->xbutton.y;
       d->menuwidget = NULL;
       IIMESSAGE(d->locx1,d->locy1)
       d->oldwidth=d->oldheight=0;
       ENDMESSAGE(magmenu_a_magMenu)
       return;
    }

    if (mode&MAG_EXTEND) {
       Position x,y;
       Dimension width,height;
       INFMESSAGE(MAG_EXTEND)
       d->locx2=event->xbutton.x;
       d->locy2=event->xbutton.y;
       if (d->locx2 > d->locx1) { x=d->locx1; width =(Dimension)(d->locx2-d->locx1); }
       else                     { x=d->locx2; width =(Dimension)(d->locx1-d->locx2); }
       if (d->locy2 > d->locy1) { y=d->locy1; height=(Dimension)(d->locy2-d->locy1); }
       else                     { y=d->locy2; height=(Dimension)(d->locy1-d->locy2); }

       if (d->oldwidth && d->oldheight)
          GhostviewDrawRectangle(gvw,d->oldx,d->oldy,d->oldwidth,d->oldheight);
       if (width && height)
          GhostviewDrawRectangle(gvw,x,y,width,height);
       d->oldx=x; d->oldy=y; d->oldwidth=width; d->oldheight=height;
       ENDMESSAGE(magmenu_a_magMenu)
       return;
    }

    if (mode&MAG_CHOOSE) {
       Arg args[5];
       Cardinal n;
       Widget entry = NULL;
       String name = "magMenu";
       int i;

       INFMESSAGE(MAG_CHOOSE)
       d->menuwidget = XtCreatePopupShell(name, simpleMenuWidgetClass,w,NULL,(Cardinal)0);
       for (i=0; gv_magmenu_entries[i] ; i++)
          entry = XtCreateManagedWidget(gv_magmenu_entries[i]->name, smeBSBObjectClass,d->menuwidget,NULL,(Cardinal)0);
       {
	 int menu_x, menu_y;
	 Dimension menu_width,entry_height,menu_border,menu_height;
	 Dimension screen_width,screen_height;
	 Position button_x, button_y;
	 
	 if (!XtIsRealized(d->menuwidget)) XtRealizeWidget(d->menuwidget);
	                                                   n=0;
	 XtSetArg(args[n], XtNheight, &entry_height);      ++n;
	 XtGetValues(entry, args, n);
	                                                   n=0;
	 XtSetArg(args[n], XtNwidth, &menu_width);         ++n;
	 XtSetArg(args[n], XtNheight, &menu_height);       ++n;
	 XtSetArg(args[n], XtNborderWidth, &menu_border);  ++n;
	 XtGetValues(d->menuwidget, args, n);

	 XtTranslateCoords(w, event->xbutton.x, event->xbutton.y, &button_x, &button_y);
	 menu_x = button_x-menu_width/2 -menu_border;
	 menu_y = button_y-entry_height/2;

	 screen_width = WidthOfScreen(XtScreen(d->menuwidget));
	 screen_height = HeightOfScreen(XtScreen(d->menuwidget));

	 if( menu_x + menu_width > screen_width && menu_width < screen_width )
	   menu_x = screen_width - menu_width;
	 if( menu_y + menu_height > screen_height && menu_height < screen_height )
	   menu_y = screen_height - menu_height;
                                                           n=0;
         XtSetArg(args[n], XtNx, menu_x);                  n++;
	 XtSetArg(args[n], XtNy, menu_y);                  n++;
	 XtSetValues(d->menuwidget, args, n);
	 XtPopup(d->menuwidget,XtGrabExclusive);           
       }

       d->locx2=event->xbutton.x;
       d->locy2=event->xbutton.y;
       IIMESSAGE(d->locx2,d->locy2)
       ENDMESSAGE(magmenu_a_magMenu)
       return;
    }

    if (mode&MAG_SHOW) {
      Widget entry = XawSimpleMenuGetActiveEntry(d->menuwidget);
      INFMESSAGE(MAG_SHOW)
      if (entry) {
	Position locx,locy;
	char *s;
	int i=0;
	float scale;

	s = XtName(entry);
	i=0; while (strcmp(gv_magmenu_entries[i]->name,s)) i++;
	scale = gv_magmenu_entries[i]->scale;

	locx = (d->locx1+d->locx2)/2;
	locy = (d->locy1+d->locy2)/2;
	if (abs(d->locx1-d->locx2)<4 || abs(d->locy1-d->locy2)<4) {
	  int dd=250/scale;
	  d->locx1 = locx-dd;  d->locx2 = locx+dd;
	  d->locy1 = locy-dd; d->locy2 = locy+dd;
	}
	GhostviewGetBBofArea(gvw,d->locx1,d->locy1,d->locx2,d->locy2,&ret_val);
	ret_val.xdpi = (ret_val.xdpi/scale);
	ret_val.ydpi = (ret_val.ydpi/scale);
	if (ret_val.width && ret_val.height) popup_zoom = True;
      }
    }

   if (mode&MAG_CHECK) {
      INFMESSAGE(MAG_CHECK)
      if (d->menuwidget) { 
         Arg args[5];
         Cardinal n;
         Position ulx,uly,lrx,lry,evx,evy;
         int rx,ry;
         Dimension width,height;
                                                            n=0;
         XtSetArg(args[n], XtNwidth,  &width);              n++;
         XtSetArg(args[n], XtNheight, &height);             n++;
         XtGetValues(d->menuwidget, args, n);
         XtTranslateCoords(d->menuwidget, 0, 0, &ulx, &uly);
         XtTranslateCoords(d->menuwidget, (Position)width, (Position)height, &lrx, &lry);
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
            if (d->oldwidth && d->oldheight)
               GhostviewDrawRectangle(gvw, d->oldx,d->oldy,d->oldwidth,d->oldheight);
            magmenu_a_magMenu(gvw,(XEvent*)NULL,NULL,NULL);
         } else {
            INFMESSAGE(pointer in window)
            ENDMESSAGE(magmenu_a_magMenu)
            return;
         }
      }

   }

   if (mode&MAG_RESET) {
      INFMESSAGE(MAG_RESET)
      if (d) {
         if (d->oldwidth && d->oldheight)
            GhostviewDrawRectangle(gvw,d->oldx,d->oldy,d->oldwidth,d->oldheight);
      }
      magmenu_a_magMenu(gvw,(XEvent*)NULL,NULL,NULL);
   }


   if (popup_zoom) {
      INFMESSAGE(popping up zoom window)
      zoom_createZoom(gvw,(XtPointer)(&ret_val));
   }
   gvw = NULL;

   ENDMESSAGE(magmenu_a_magMenu)
}
