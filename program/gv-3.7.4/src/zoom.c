/*
**
** zoom.c
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

#   include <sys/types.h>
#   include <sys/stat.h>


#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_X11(Shell.h)
#include INC_XAW(Cardinals.h)
#include "Aaa.h"
#include "Button.h"
#include "Clip.h"
#include "Frame.h"
#include "Ghostview.h"

#include "types.h"
#include "config.h"
#include "callbacks.h"
#include "ps.h"
#include "doc_misc.h"
#include "main_resources.h"
#include "main_globals.h"
#include "popup.h"
#include "widgets_misc.h"
#include "zoom.h"

static char* zoom_popup      = "zoomPopup";
static char* zoom_control    = "zoomControl";
static char* zoom_page       = "page";
static char* zoom_dismiss    = "dismiss";
#if 0
static char* zoom_redisplay  = "redisplay";
#endif

/*##################################################################*/
/* zoom_createZoom */
/* popup zoom window */
/*##################################################################*/

void
zoom_createZoom(Widget w _GL_UNUSED, XtPointer call_data)
{
    Arg args[25];
    Cardinal n;
    struct stat sbuf;

    GhostviewReturnStruct *p = (GhostviewReturnStruct *)call_data;
    Boolean b;
    int llx;
    int lly;
    int urx;
    int ury;
    int bottom_margin;
    int left_margin;
    int right_margin;
    int top_margin;
    int i;
    XtPageOrientation xto;
    Widget zshell,zcontrol,zviewFrame,zviewClip,zviewControl,zpage,zdismiss;
    FILE *zoomfile;
    char *filename;

    BEGINMESSAGE(zoom_createZoom)

    if (!(gv_psfile != NULL || (gv_gs_arguments && *gv_gs_arguments))) {
       INFMESSAGE(no file) ENDMESSAGE(zoom_createZoom)
       return;
    }

    if (strcmp(gv_filename, "-")) {
       stat(gv_filename, &sbuf);
       if (mtime != sbuf.st_mtime) {
	  INFMESSAGE1(file has changed) ENDMESSAGE1(zoom_createZoom)return;
       }
    }

    filename = (gv_filename_dsc ? gv_filename_dsc : (gv_filename_unc ? gv_filename_unc : gv_filename));

    llx = p->psx;
    lly = p->psy;
    urx = p->psx + p->width  - 1; if (urx<llx) urx=llx;
    ury = p->psy + p->height - 1; if (ury<lly) ury=lly;
    IIMESSAGE(llx,lly) IIMESSAGE(urx,ury)

    /* Make sure zoom window doesn't go off the edge of the page */
    if (llx < current_llx) {	llx = current_llx;	urx = llx + p->width;	}
    if (lly < current_lly) {	lly = current_lly;	ury = lly + p->height;	}
    if (urx > current_urx) {	urx = current_urx;	llx = urx - p->width;	}
    if (ury > current_ury) {	ury = current_ury;	lly = ury - p->height;	}
    if (llx < current_llx)	llx = current_llx;
    if (lly < current_lly)	lly = current_lly;
    bottom_margin = lly - current_lly;
    left_margin   = llx - current_llx;
    right_margin  = current_urx - urx;
    top_margin    = current_ury - ury;

							       n=0;
       XtSetArg(args[n], XtNminWidth,  70);		       n++;
       XtSetArg(args[n], XtNminHeight, 70);		       n++;
       XtSetArg(args[n], XtNallowShellResize, True);	       n++; 
    zshell = XtCreatePopupShell(zoom_popup, topLevelShellWidgetClass,toplevel,args,n);

                               				       n=0;
       XtSetArg(args[n], XtNminimumWidth, 70);			n++;
       XtSetArg(args[n], XtNminimumHeight,70);			n++; 
    zcontrol = XtCreateManagedWidget(zoom_control,aaaWidgetClass,zshell,args,n);

								n=0;
   zviewFrame = XtCreateManagedWidget("zviewFrame", frameWidgetClass,zcontrol,args,n);

									n=0;
   zviewClip = XtCreateManagedWidget("zviewClip", clipWidgetClass,zviewFrame,args,n);
       XtAddCallback(zviewClip, XtNreportCallback,cb_pageAdjustNotify,(XtPointer)NULL);

									n=0;
   zviewControl = XtCreateManagedWidget("zviewControl", aaaWidgetClass,zviewClip,args,n);

   xto     = doc_convDocOrientToXtOrient(gv_orientation, gv_swap_landscape );

							       n=0;
       XtSetArg(args[n], XtNorientation,xto);                  n++;
       XtSetArg(args[n], XtNllx, llx);      	               n++;
       XtSetArg(args[n], XtNlly, lly);      	               n++;
       XtSetArg(args[n], XtNurx, urx);      	               n++;
       XtSetArg(args[n], XtNury, ury);      	               n++;
       XtSetArg(args[n], XtNbottomMargin, bottom_margin);      n++;
       XtSetArg(args[n], XtNleftMargin, left_margin);          n++;
       XtSetArg(args[n], XtNrightMargin, right_margin);        n++;
       XtSetArg(args[n], XtNtopMargin, top_margin);            n++;
       XtSetArg(args[n], XtNbottomMargin, bottom_margin);      n++;
       XtSetArg(args[n], XtNlxdpi, (1000*p->xdpi));            n++;
       XtSetArg(args[n], XtNlydpi, (1000*p->ydpi));            n++;
       XtSetArg(args[n], XtNinterpreter,gv_gs_interpreter);    n++;
       b = gv_gs_safeDir ? True : False;
       XtSetArg(args[n], XtNsafeDir,b);                        n++;
       b = gv_gs_safer ? True : False;
       XtSetArg(args[n], XtNsafer,b);                          n++;
       b = gv_gs_quiet ? True : False;
       XtSetArg(args[n], XtNquiet,b);                          n++;
       b = app_res.use_bpixmap ? True : False;
       XtSetArg(args[n], XtNuseBackingPixmap,b);               n++;
       XtSetArg(args[n], XtNarguments,gv_gs_arguments);        n++;
       if (!toc_text) {
          XtSetArg(args[n], XtNfilename, filename);	       n++;
       }
    zpage = XtCreateManagedWidget(zoom_page, ghostviewWidgetClass,zviewControl, args, n);
    num_ghosts++;
    XtAddCallback(zpage, XtNcallback, cb_track, (XtPointer)NULL);
    XtAddCallback(zpage, XtNmessageCallback, cb_message, (XtPointer)zpage);
    XtAddCallback(zpage, XtNdestroyCallback, cb_destroyGhost,(XtPointer)zpage); 

							    n=0;
    zdismiss = XtCreateManagedWidget(zoom_dismiss, buttonWidgetClass,zcontrol, args, n);
    XtAddCallback(zdismiss, XtNcallback,cb_destroy,(XtPointer)zshell);
    XtInstallAccelerators(zcontrol, zdismiss);

    XtRealizeWidget(zshell);
    {
       Dimension page_width,page_height;
       INFMESSAGE(adjusting size)
                                                               n=0;
       XtSetArg(args[n], XtNpreferredWidth,  &page_width);     n++;
       XtSetArg(args[n], XtNpreferredHeight, &page_height);    n++;
       XtGetValues(zpage, args, n);
       IIMESSAGE(page_width,page_height)
                                                               n=0;
       XtSetArg(args[n], XtNwidth,  page_width);               n++;
       XtSetArg(args[n], XtNheight, page_height);              n++;
       XtSetValues(zpage, args, n);
    }
    XtInstallAccelerators(zcontrol,zdismiss);
    widgets_preferButton(zdismiss,1);

    {
      GhostviewReturnStruct rs;
      INFMESSAGE(positioning zoom popup)
      IIMESSAGE(llx,lly) IIMESSAGE(urx,ury)
      GhostviewGetAreaOfBB(page,llx,lly,urx,ury,&rs);
      popup_positionPopup(
          zshell,page,POPUP_POSITION_POS_CENTER,
          ((int)(rs.psx)+(int)(rs.width/2)),
          ((int)(rs.psy)+(int)(rs.height/2))
      );
    }
    GhostviewSetup(zpage);
    
    cb_positionPage(zpage,(XtPointer)1,(XtPointer)NULL);
    
    XSetWMProtocols(gv_display, XtWindow(zshell), &wm_delete_window, 1);
    XtPopup(zshell, XtGrabNone);

    if (toc_text) {
	if (!filename) {
	    zoomfile = NULL;
	} else if (strcmp(filename, "-")) {
	    zoomfile = fopen(filename, "r");
	} else {
	    zoomfile = stdin;
        }
	if (zoomfile == NULL) {
	    ENDMESSAGE(zoom_createZoom)
	    return;
	}
	
        GhostviewEnableInterpreter(zpage);
	
	GhostviewSendPS(zpage, zoomfile, doc->beginprolog,
			doc->lenprolog, False);
	GhostviewSendPS(zpage, zoomfile, doc->beginsetup,
			doc->lensetup, False);
	if (doc->pageorder == DESCEND)
	    i = (doc->numpages - 1) - current_page;
	else
	    i = current_page;
	GhostviewSendPS(zpage, zoomfile, doc->pages[i].begin,
			doc->pages[i].len, True);
    } else GhostviewEnableInterpreter(zpage);
    ENDMESSAGE(zoom_createZoom)
}



#if 0
							    n=0;
    zredisplay = XtCreateManagedWidget(zoom_redisplay, buttonWidgetClass,zcontrol, args, n);
    if (toc_text) {
       INFMESSAGE(have useful toc)
       XtAddCallback(zredisplay, XtNcallback, cb_redisplay, (XtPointer)current_page);
    } else {
       INFMESSAGE(doc frame does not have useful toc)
       XtAddCallback(zredisplay, XtNcallback, cb_redisplay, (XtPointer)0);
    }
#endif
