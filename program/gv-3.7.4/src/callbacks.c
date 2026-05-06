/*
**
** callbacks.c
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
** Authors:   Johannes Plass (plass@thep.physik.uni-mainz.de)
**            Department of Physics
**            Johannes Gutenberg-University
**            Mainz, Germany
**
**            Jose E. Marchesi (jemarch@gnu.org)
**            GNU Project
*/
#include "ac_config.h"

/*
#define MESSAGES
*/
#include "message.h"

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

#include <inttypes.h>

#ifndef BUFSIZ
#   define BUFSIZ 1024
#endif

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_X11(Shell.h)
#include INC_XAW(Cardinals.h)
#include INC_XAW(Scrollbar.h)
#include "Clip.h"
#include "FileSel.h"
#include "Ghostview.h"
#include "Vlist.h"
#include INC_X11(IntrinsicP.h)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <locale.h>
#include <langinfo.h>
#include <iconv.h>
#include INC_X11(Xatom.h)

#include "types.h"
#include "actions.h"
#include "callbacks.h"
#include "confirm.h"
#include "file.h"
#include "ps.h"
#include "doc_misc.h"
#include "info.h"
#include "popup.h"
#include "process.h"
#include "dialog.h"
#include "magmenu.h"
#include "main_resources.h"
#include "main_globals.h"
#include "media.h"
#include "misc.h"
#include "miscmenu.h"
#include "note.h"
#include "save.h"
#include "resource.h"
#include "scale.h"
#include "misc_private.h"
#include "version.h"
#include "widgets_misc.h"
#include "VlistP.h"

static char* save_directory = NULL;
static char* open_directory = NULL;

static int last_psx;
static int last_psy;

/*############################################################*/
/* cb_showTitle */
/*############################################################*/

void setTitle ( Display* dpy, Window w, char* title, int icon);
XTextProperty* char_to_xtp ( Display* dpy, char* s );

void setTitle ( Display* dpy, Window w, char* title, int icon) {
	char* from;
	iconv_t cd;
	char* inptr;
	size_t insize;
	char utf8title [4096];
	char* outptr;
	size_t outsize;
	Atom net_wm_name;
	Atom net_wm_icon_name;
        Atom utf8_string;
	
	net_wm_name = XInternAtom (dpy, "_NET_WM_NAME", False);
	net_wm_icon_name = XInternAtom (dpy, "_NET_WM_ICON_NAME", False);
	utf8_string = XInternAtom (dpy, "UTF8_STRING" ,False);

	from  = nl_langinfo (CODESET);
	cd  = iconv_open ("UTF-8",from);
	inptr = title;
	outptr = utf8title;
	insize = strlen(title);
	outsize = sizeof (utf8title);
	memset (&utf8title, 0, sizeof(utf8title));
	iconv (cd, &inptr, &insize, &outptr, &outsize);

	if (icon) {
		XSetWMIconName (
			dpy, w, char_to_xtp (dpy,utf8title)
			);
		XChangeProperty (
			dpy, w, net_wm_icon_name, utf8_string, 8,
			PropModeReplace, (unsigned char *)utf8title,
			strlen (utf8title)
			);
	} else {
		XSetWMName (
			dpy, w, char_to_xtp (dpy,utf8title)
			);
		XChangeProperty (
			dpy, w, net_wm_name, utf8_string, 8,
			PropModeReplace, (unsigned char *)utf8title,
			strlen (utf8title)
			);
	}
}

XTextProperty* char_to_xtp ( Display* dpy, char* s ) {
	static XTextProperty tp = { 0, 0, 0, 0 };
	static int free_prop = True;
	int errCode = 0;
	char* tl[2];
	if ( tp.value ) {
		if ( free_prop ) {
			XFree( tp.value );
		}
		tp.value = 0;
		free_prop = True;
	}
	tl[0] = s;
	tl[1] = 0;
	errCode = XmbTextListToTextProperty (
		dpy,tl, 1, XStdICCTextStyle, &tp
	);
	if ( errCode < 0 ) {
		tp.value = (unsigned char*)s;
		tp.encoding = XA_STRING;
		tp.format = 8;
		tp.nitems = strlen (s);
		free_prop = False;
	}
    return &tp;
}

void
cb_showTitle(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  String t=NULL,s;
  Arg args[2];
  Cardinal n;
  Pixmap bitmap=None;

  BEGINMESSAGE(cb_showTitle)
  if (app_res.title_style != 0) {
    if (app_res.title_style == 1 && doc && doc->title)
      {
	t = doc->title;
        bitmap = app_res.document_bitmap;
      }
    else if (gv_filename) 
      {
	t = gv_filename;
      }
    if (!t) t = s = XtNewString(versionIdentification[0]);
    else {
      s = XtMalloc((4+strlen(t)+1)*sizeof(char));
      sprintf(s,"gv: %s",t);
    }
  } else {
    t = s = XtNewString(versionIdentification[0]);
  }
  if (w && XtWindow(w)) setTitle(gv_display, XtWindow(w), s, 0);
  if (w && XtWindow(w)) setTitle(gv_display, XtWindow(w), t, 1);
					n=0;
  XtSetArg(args[n], XtNtitle, s);	n++;
  XtSetArg(args[n], XtNiconName, t);	n++;
  XtSetValues(toplevel,args,n);

  if (show_title) {
                                    n=0;
   XtSetArg(args[n], XtNlabel, t);  n++;
   XtSetValues(titlebutton, args, n);
   if (titlemenu) XtDestroyWidget(titlemenu);
   titlemenu = build_label_menu(titlebutton, "title", t, bitmap);
  }
  XtFree(s);
  ENDMESSAGE(cb_showTitle)
}

/*############################################################*/
/* cb_newtocScrollbar */
/*############################################################*/

void
cb_newtocScrollbar(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data)
{
  BEGINMESSAGE(cb_newtocScrollbar)
  if (((int)(intptr_t)client_data)==1) {
    int dy = (int)(intptr_t)call_data;
    /* Just scroll one position less... */
    int ly = ((VlistWidget)newtoc)->vlist.ydelta;
    if (dy>ly) dy-=ly;
    if (dy<-ly) dy+=ly;
    VlistMoveFirstVisible(newtoc, VlistGetFirstVisible(newtoc), dy);
  } else {
    float *percent = (float *) call_data;
    VlistSetFirstVisible(newtoc, (int)(VlistEntries(newtoc)**percent+0.5));
  }
  ENDMESSAGE(cb_newtocScrollbar)
}

/*##################################################################*/
/* cb_newtocVisibleAdjust */
/*##################################################################*/

void cb_newtocVisibleAdjust(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  int entries;

  BEGINMESSAGE(cb_newtocClipAdjust)

  entries = VlistEntries(newtoc);

  if (entries <= 0)
    XawScrollbarSetThumb(newtocScroll, 0.0, 1.0);
  else
    XawScrollbarSetThumb(newtocScroll,
			 VlistScrollPosition(newtoc),
		         VlistVisibleLength(newtoc,newtocClip->core.height)); 

  ENDMESSAGE(cb_newtocClipAdjust)
}

/*############################################################*/
/* cb_adjustSlider */
/*############################################################*/

void
cb_adjustSlider(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
    static Dimension opw=0,oph=0,opvw=0,opvh=0;
    static Position opvx=0,opvy=0;
    XawPannerReport *report = (XawPannerReport*) call_data; 
    Dimension pw  = (Dimension) (report->canvas_width);
    Dimension ph  = (Dimension) (report->canvas_height);
    Dimension pvw = (Dimension) (report->slider_width);
    Dimension pvh = (Dimension) (report->slider_height);
    Position  pvx = (Position)  (report->slider_x);
    Position  pvy = (Position)  (report->slider_y);

    BEGINMESSAGE(cb_adjustSlider)
    if (gv_scroll_mode == SCROLL_MODE_PANNER) {
       INFMESSAGE(aborting due to wrong scroll mode) ENDMESSAGE(view_cb_adjustSlider)
       return;
    }
    if (!show_panner) {INFMESSAGE(panner not used)ENDMESSAGE(cb_adjustSlider)return;}
    
    if ((pw!=opw)||(ph!=oph)||(pvw!=opvw)||(pvh!=opvh)||(pvx!=opvx)||(pvy!=opvy)) {
       Arg args[5];
       Dimension sw,sh,cw,ch,bw;
       Position  sx,sy;
       static Dimension osw=0,osh=0;
       static Position  osx=0,osy=0;

       INFMESSAGE(detected changes)
       XtSetArg(args[0], XtNwidth,&cw);
       XtSetArg(args[1], XtNheight,&ch);
       XtSetArg(args[2], XtNborderWidth,&bw);
       XtGetValues(panner, args, THREE);

       sw = (Dimension) ((cw*pvw+pw/2)/pw);
       sh = (Dimension) ((ch*pvh+ph/2)/ph);
       if (pw>pvw) sx = (Position) (((cw-sw)*pvx+(pw-pvw)/2)/(pw-pvw)); else sx = 0;
       if (ph>pvh) sy = (Position) (((ch-sh)*pvy+(ph-pvh)/2)/(ph-pvh)); else sy = 0;

       IIMESSAGE(cw,ch)
       IIMESSAGE(sw,sh) IIMESSAGE(sx,sy)
       IIMESSAGE(pw,ph) IIMESSAGE(pvw,pvh) IIMESSAGE(pvx,pvy)

       INFMESSAGE(redisplaying slider)
       XtConfigureWidget(slider,sx,sy,sw,sh,bw);
       osw=sw; osh=sh; osx=sx; osy=sy;
       opw=pw; oph=ph; opvw=pvw; opvh=pvh; opvx=pvx; opvy=pvy;
    }
    ENDMESSAGE(cb_adjustSlider)
}

/*##################################################################*/
/* cb_antialias */
/*##################################################################*/

void cb_antialias(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_antialias)
  if (client_data) {
    app_res.antialias = app_res.antialias ? False : True;
    if (gv_filename) {
      cb_stopInterpreter(page,NULL,NULL);
      cb_reopen(page,NULL,NULL);
    }
  }
  widgets_setSelectedBitmap(antialiasEntry, app_res.antialias ? 1 : 0);
  ENDMESSAGE(cb_antialias)
}

/*##################################################################*/
/* cb_useBackingPixmap */
/*##################################################################*/

void cb_useBackingPixmap(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  int i = (int)(intptr_t)client_data;

  BEGINMESSAGE(cb_useBackingPixmap)
  if (i&1) {
    app_res.use_bpixmap = app_res.use_bpixmap ? False : True;
  }
  if (i&2) {
    Arg args[1];
    XtSetArg(args[0],XtNuseBackingPixmap,app_res.use_bpixmap);
    XtSetValues(page,args,(Cardinal)1);
  }
  if (i&4 && gv_filename) cb_reopen(page,NULL,NULL);
  ENDMESSAGE(cb_useBackingPixmap)
}

/*##################################################################*/
/* cb_handleDSC */
/*##################################################################*/

void cb_handleDSC(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_handleDSC)
  if (client_data) {
    gv_scanstyle = (gv_scanstyle & SCANSTYLE_IGNORE_DSC) ?
      (gv_scanstyle & ~SCANSTYLE_IGNORE_DSC) :
      (gv_scanstyle |  SCANSTYLE_IGNORE_DSC);
    if (gv_filename) {
      cb_stopInterpreter(page,NULL,NULL);
      cb_reopen(page,NULL,NULL);
    }
  }
  widgets_setSelectedBitmap(dscEntry, (gv_scanstyle & SCANSTYLE_IGNORE_DSC) ? 0 : 1);
  ENDMESSAGE(cb_handleDSC)
}

/*##################################################################*/
/* cb_handleEOF */
/*##################################################################*/

void cb_handleEOF(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_handleEOF)
  if (client_data) {
    gv_scanstyle = (gv_scanstyle & SCANSTYLE_IGNORE_EOF) ?
      (gv_scanstyle & ~SCANSTYLE_IGNORE_EOF) :
      (gv_scanstyle |  SCANSTYLE_IGNORE_EOF);
    if (gv_filename) {
      cb_stopInterpreter(page,NULL,NULL);
      cb_reopen(page,NULL,NULL);
    }
  }
  widgets_setSelectedBitmap(eofEntry,(gv_scanstyle & SCANSTYLE_IGNORE_EOF));
  ENDMESSAGE(cb_handleEOF)
}

/*##################################################################*/
/* cb_stopInterpreter */
/*##################################################################*/

void cb_stopInterpreter(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_stopInterpreter)
   GhostviewDisableInterpreter(page);
   ENDMESSAGE(cb_stopInterpreter)
}

/*##################################################################*/
/* cb_pageAdjustNotify */
/*##################################################################*/

void
cb_pageAdjustNotify(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
  BEGINMESSAGE(cb_pageAdjustNotify)
  if (gv_scroll_mode == SCROLL_MODE_GHOSTVIEW) {
    String params[2];
    Cardinal num_params=2;
    params[0]= "adjusted";
    params[1]= (char*) call_data;
    action_movePage(page,(XEvent*)NULL,params,&num_params);
  }
  ENDMESSAGE(cb_pageAdjustNotify)
}

/*##################################################################*/
/* cb_checkFile */
/*##################################################################*/

void
cb_checkFile(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
    int changed;

    BEGINMESSAGE(cb_checkFile)
    changed = check_file(((int)(intptr_t)client_data));
    if (changed==1)
    {
       cb_stopInterpreter(page,NULL,NULL);
       show_page(current_page,NULL);
    }
    ENDMESSAGE(cb_checkFile)
}

/*##################################################################*/
/* cb_watchFile */
/*##################################################################*/

/*------------------------------------------------------------*/
/* watch_file */
/*------------------------------------------------------------*/

static void watch_file (XtPointer client_data, XtIntervalId *idp _GL_UNUSED)
{
  static XtIntervalId timer = (XtIntervalId) 0;
  unsigned long t = (unsigned long) app_res.watch_file_frequency;

  BEGINMESSAGE(watch_file)
  /* notification after timeout */
  if ((int)(intptr_t)client_data && app_res.watch_file) {
    if (!file_fileIsNotUseful(gv_filename)) {
      int error;
      String s;
      struct stat sbuf;

      INFMESSAGE(checking file)
      s = XtNewString(gv_filename);
      error = stat(s, &sbuf);
      if (!error && mtime != sbuf.st_mtime && sbuf.st_mtime < time(NULL))
         cb_checkFile(NULL,(XtPointer)(CHECK_FILE_VERSION|CHECK_FILE_DATE),NULL);
      XtFree(s);
    }
  }
  if (timer) {
    XtRemoveTimeOut(timer);
    timer = (XtIntervalId) 0;
  }
  if (app_res.watch_file) {
    INFMESSAGE(adding timeout)
    timer = XtAppAddTimeOut(app_con,t,watch_file,(XtPointer)1);
  }
  ENDMESSAGE(watch_file)
}

void
cb_watchFile(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_watchFile)
  if (client_data) {
    app_res.watch_file = app_res.watch_file ? False : True;
  }
  watch_file(NULL,NULL);
  widgets_setSelectedBitmap(watchFileEntry,(app_res.watch_file ? 1 : 0));
  ENDMESSAGE(cb_watchFile)
}

/*##################################################################*/
/* cb_print */
/*##################################################################*/

static char *make_pagelist(int mode)
{
   Boolean mode_valid=False;
   char *pagelist=NULL;

   BEGINMESSAGE(make_pagelist)
   if (toc_text && (mode&(PAGE_MODE_CURRENT|PAGE_MODE_MARKED))) {
      char *tmp;
      pagelist = XtNewString(VlistVlist(newtoc));
      if (mode&PAGE_MODE_MARKED) {
	tmp = pagelist;
        while (*tmp) if (*tmp++=='*') { mode_valid=True; break; }
	if (!mode_valid && (mode&PAGE_MODE_CURRENT)) mode=PAGE_MODE_CURRENT;
      }
      if (mode==PAGE_MODE_CURRENT) {
	tmp = pagelist;
        while (*tmp) { *tmp=' '; tmp++; }
	pagelist[current_page]='*';
	mode_valid=True;
      }
   }
   if (!mode_valid) {
      XtFree(pagelist);
      pagelist=NULL;
   }
   ENDMESSAGE(make_pagelist)
   return pagelist;
}

static char *get_pagelist(int *modep)
{
   char *pagelist=NULL;
   int mode= *modep;

   BEGINMESSAGE(get_pagelist)
   if (toc_text && (mode&(PAGE_MODE_CURRENT|PAGE_MODE_MARKED))) {
      if (mode&PAGE_MODE_MARKED) {
         pagelist=make_pagelist(PAGE_MODE_MARKED);
         if (pagelist) mode=PAGE_MODE_MARKED;
      }
      if (!pagelist && (mode&PAGE_MODE_CURRENT)) {
         pagelist=make_pagelist(PAGE_MODE_CURRENT);
         if (pagelist) mode=PAGE_MODE_CURRENT;
      }
      if (!pagelist) mode=PAGE_MODE_INVALID;
   } else if (mode==PAGE_MODE_ALL) {
      pagelist=NULL; /* all pages */
   } else {
      mode=PAGE_MODE_INVALID;
   }
   *modep=mode;
   ENDMESSAGE(get_pagelist)
   return pagelist;
}

void
cb_print(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
    char *prompt=GV_PRINT_MESSAGE;
    char *buttonlabel=GV_PRINT_BUTTON_LABEL;
    char *message;
    char *pagelist=NULL;

    BEGINMESSAGE(cb_print)

    if (!gv_filename) {
       INFMESSAGE(no file)
       ENDMESSAGE(cb_print)
       return;
    }

    gv_print_mode = (int)(intptr_t)client_data;
    pagelist=get_pagelist(&gv_print_mode);
    XtFree(pagelist);
    if (gv_print_mode==PAGE_MODE_INVALID) {
       INFMESSAGE(invalid print mode)
       ENDMESSAGE(cb_print)
       return;
    }

    if (app_res.confirm_print) {
       if        (gv_print_mode==PAGE_MODE_MARKED) {
          message=GV_PRINT_MARKED_MESSAGE; INFMESSAGE(printing marked pages)
       } else if (gv_print_mode == PAGE_MODE_CURRENT) {
          message=GV_PRINT_PAGE_MESSAGE;   INFMESSAGE(printing current page)
       } else {
          message=GV_PRINT_ALL_MESSAGE;    INFMESSAGE(printing document)
       }
       DialogPopupSetPrompt(prompt);
       DialogPopupSetMessage(message);
       DialogPopupSetButton(DIALOG_BUTTON_DONE,buttonlabel,cb_doPrint);
       DialogPopupSetButton(DIALOG_BUTTON_CANCEL,NULL,cb_cancelPrint);
       DialogPopupSetText(gv_print_command);
       cb_popupDialogPopup((Widget)NULL,NULL,NULL);
       ENDMESSAGE(cb_print)
       return;
    }   
    cb_doPrint((Widget)NULL,NULL,(XtPointer)gv_print_command);
    ENDMESSAGE(cb_print)
}

/*##################################################################*/
/* cb_print_pos */
/*##################################################################*/

void
cb_print_pos(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
    char *pagelist=NULL;
    static char buf[MAX_LOCATOR_LENGTH];

    BEGINMESSAGE(cb_print_pos)

    if (!gv_filename) {
       INFMESSAGE(no file)
       ENDMESSAGE(cb_print_pos)
       return;
    }

    gv_print_mode = (intptr_t)client_data;
    pagelist=get_pagelist(&gv_print_mode);
    XtFree(pagelist);

    DialogPopupSetMessage(putTexCommandLabel);
    DialogPopupSetPrompt(texCommandLabel);
    DialogPopupSetButton(DIALOG_BUTTON_DONE,"Save",cb_doPrintPos);
    DialogPopupSetButton(DIALOG_BUTTON_CANCEL,NULL,cb_cancelPrint);

    sprintf(buf, "\\PutAtPos(%i,%i){%s}", last_psx, last_psy, "") ;
    DialogPopupSetText(buf);
    cb_popupDialogPopup((Widget)NULL,NULL,NULL);
    ENDMESSAGE(cb_print_pos)
}

void
cb_doPrintPos(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
    String print_command;
    FILE* posfile = fopen(gv_savepos_filename, "a");

    BEGINMESSAGE(cb_doPrintPos)

    if (call_data) print_command = (String)(call_data);  /* dialog was not used */  
    print_command = DialogPopupGetText(); /* dialog was used */  
    if (!print_command) print_command="";

    if (posfile == NULL) {
       INFMESSAGE(cannot open file for writting)
       ENDMESSAGE(cb_doPrintPos)
       return;
    }
    fprintf(posfile, "%s\n", print_command);
    fclose(posfile);
    cb_popdownNotePopup((Widget)NULL,(XtPointer)NULL,NULL);
    cb_popdownDialogPopup((Widget)NULL,(XtPointer)NULL,NULL);
    ENDMESSAGE(cb_doPrintPos)
}

/*##################################################################*/
/* cb_doPrint */
/*##################################################################*/

void
cb_doPrint(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
    String print_command;
    String error=NULL;
    char *pagelist=NULL;

    BEGINMESSAGE(cb_doPrint)

    if (call_data) print_command = (String)(call_data);  /* dialog was not used */  
    else           print_command = DialogPopupGetText(); /* dialog was used */  
    if (!print_command) print_command="";
    SMESSAGE(print_command)

    cb_popdownNotePopup((Widget)NULL,(XtPointer)NULL,NULL);

    pagelist=get_pagelist(&gv_print_mode);
    if (gv_print_mode != PAGE_MODE_INVALID) {
       SaveData sd          = save_allocSaveData();
       sd->save_fn          = NULL;
       sd->src_fn           = gv_filename_unc ?
	                        XtNewString(gv_filename_unc) :
	                        XtNewString(gv_filename);
       sd->conv_fn          = NULL;
       sd->pagelist         = pagelist ? XtNewString(pagelist) : NULL;
       sd->print_cmd        = print_command ? XtNewString(print_command) : NULL;
       sd->convert          = gv_filename_dsc ? 1 : 0;
       sd->save_to_file     = (gv_print_kills_file || pagelist) ? 1 : 0;
       sd->save_to_printer  = 1;
       sd->print_kills_file = gv_print_kills_file;
       sd->scanstyle        = gv_scanstyle;
       error = save_saveFile(sd);
    }
    if (error) {
       NotePopupShowMessage(error);
       XtFree(error);
    } else {
       cb_popdownDialogPopup((Widget)NULL,(XtPointer)NULL,NULL);
    }
    XtFree(pagelist);

    ENDMESSAGE(cb_doPrint)
}

/*##################################################################*/
/* cb_cancelPrint */
/*##################################################################*/

void
cb_cancelPrint(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    BEGINMESSAGE(cb_cancelPrint)
    cb_popdownNotePopup((Widget)NULL,(XtPointer)NULL,NULL);
    cb_popdownDialogPopup((Widget)NULL,(XtPointer)NULL,NULL);
    ENDMESSAGE(cb_cancelPrint)
}

/*##################################################################*/
/* cb_save */
/*##################################################################*/

void
cb_save(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg args[10];
    Cardinal n;  
    char *title="Save";
    char *buttonlabel;
    Widget button = XtNameToWidget(FileSel,"button2");
    char *pagelist;
    char *name;
    char *path;
    char ext[20];
    size_t pathlen;
    char default_path[GV_MAX_FILENAME_LENGTH];

    BEGINMESSAGE(cb_save)

    gv_save_mode = (int)(intptr_t)client_data;
    pagelist=get_pagelist(&gv_save_mode);
    XtFree(pagelist);
    if (gv_save_mode==PAGE_MODE_INVALID) {
       INFMESSAGE(invalid save mode)
       ENDMESSAGE(cb_save)
       return;
    }

    path=".";
    if (!save_directory && app_res.default_save_dir) path=app_res.default_save_dir;
    else if (save_directory)                         path=save_directory;
    XawFileSelectionSetPath(FileSel,path);
    XawFileSelectionScan(FileSel,XawFileSelectionRescan);
    path = XawFileSelectionGetPath(FileSel);
    name = file_locateFilename(gv_filename_raw);
    ext[0]='\0';

    if        (gv_save_mode==PAGE_MODE_MARKED) {
       XawFileSelectionRemoveButton(FileSel, 3);
       buttonlabel=saveMarkedPagesLabel;  INFMESSAGE(saving marked pages)
       strcpy(ext,"_pages");
    } else if (gv_save_mode==PAGE_MODE_CURRENT) {
       XawFileSelectionRemoveButton(FileSel, 3);
       buttonlabel=saveCurrentPageLabel;  INFMESSAGE(saving current page)
       if (0<=current_page && current_page <= 9998) sprintf(ext,"_page_%d",(current_page+1));
       else strcpy(ext,"_page");
    } else {
       buttonlabel=saveDocumentLabel;      INFMESSAGE(saving all pages)
       if (gv_filename_dsc) {
          Widget button3;
          XawFileSelectionAddButton(FileSel, 3, cb_doSave, (XtPointer)FILE_TYPE_PDF);
          button3 = XtNameToWidget(FileSel,"button3");
          n=0;
          XtSetArg(args[n], XtNlabel, saveAsPDFLabel); ++n;
          XtSetValues(button3,args,n);
       }
    }

    /*  We assume the if ext was filled, then we'll definitely write PDF. */
    name = XtNewString(name);
    if (gv_filename_dsc && *ext) name=file_pdfname2psname(name);

    pathlen = strlen(path)+strlen(name)+strlen(ext);
    if (pathlen<GV_MAX_FILENAME_LENGTH-1) {
       sprintf(default_path,"%s%s",path,name);
       strcat(default_path,ext);
       XawFileSelectionSetPath(FileSel,default_path);
    }
    XtFree(name);

    n=0;
    XtSetArg(args[n], XtNtitle,title); ++n;
    XtSetValues(FileSel_popup, args, n);
    n=0;
    XtSetArg(args[n], XtNlabel, buttonlabel); ++n;
    XtSetValues(button,args,n);

    XtRemoveAllCallbacks(button, XtNcallback);
    XtAddCallback(button, XtNcallback,cb_doSave,NULL);

    XawFileSelectionPreferButton(FileSel,2);

    popup_positionPopup(FileSel_popup,viewFrame,POPUP_POSITION_POS,4,4);
    cb_popupPopup(w, (XtPointer)FileSel_popup, call_data);
    ENDMESSAGE(cb_save)
}

/*##################################################################*/
/* cb_doSave */
/*##################################################################*/

void
cb_doSave(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
    String name;
    String error=NULL;
    char *pagelist;
    int type;

    BEGINMESSAGE(cb_doSave)
    if (client_data) type = (int)(intptr_t)client_data;
    else type = FILE_TYPE_PS;

    name = XawFileSelectionGetPath(FileSel);
    if (file_fileIsDir(name)) {
      XawFileSelectionScan(FileSel,XawFileSelectionRescan);
      ENDMESSAGE(cb_doSave)
      return;
    }
    cb_popdownNotePopup((Widget)NULL,(XtPointer)NULL,NULL);
    XtFree(save_directory);
    save_directory= file_getDirOfPath(name);
    SMESSAGE(name)
    pagelist=get_pagelist(&gv_save_mode);
    if (gv_save_mode != PAGE_MODE_INVALID) {
       SaveData sd          = save_allocSaveData();
       sd->save_fn          = name ? XtNewString(name) : NULL;
       sd->src_fn           = gv_filename_unc ?
	                        XtNewString(gv_filename_unc) :
	                        XtNewString(gv_filename);
       sd->conv_fn          = NULL;
       sd->pagelist         = pagelist ? XtNewString(pagelist) : NULL;
       sd->print_cmd        = NULL;
       sd->convert          = (gv_filename_dsc && type==FILE_TYPE_PS) ? 1 : 0;
       sd->save_to_file     = 1;
       sd->save_to_printer  = 0;
       sd->print_kills_file = gv_print_kills_file;
       sd->scanstyle        = gv_scanstyle;
       error = save_saveFile(sd);
    }
    if (error) {
       NotePopupShowMessage(error);
       XtFree(error);
    } else {
       XtPopdown(FileSel_popup);
    }    
    XtFree(pagelist);
    ENDMESSAGE(cb_doSave)
}

/*##################################################################*/
/* cb_openFile */
/*##################################################################*/

void
cb_openFile(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
    Arg args[1];
    Cardinal n;
    Widget button = XtNameToWidget(FileSel,"button2");

    BEGINMESSAGE(cb_openFile)

    XawFileSelectionRemoveButton(FileSel, 3);
    n=0;
    XtSetArg(args[n], XtNtitle, openFileLabel); ++n;
    XtSetValues(FileSel_popup, args, n);
    n=0;
    XtSetArg(args[n], XtNlabel, openFileLabel); ++n;
    XtSetValues(button, args, n);
    XtRemoveAllCallbacks(button, XtNcallback);
    XtAddCallback(button, XtNcallback,cb_doOpenFile,NULL);

    {
       char *path=".";
       if (open_directory) path=open_directory;
       XawFileSelectionSetPath(FileSel,path);
    }
    XawFileSelectionScan(FileSel,XawFileSelectionRescan);
    XawFileSelectionPreferButton(FileSel,2);
    popup_positionPopup(FileSel_popup,viewFrame,POPUP_POSITION_POS,4,4);
    cb_popupPopup(w, (XtPointer)FileSel_popup, call_data);
    ENDMESSAGE(cb_openFile)
}   

/*##################################################################*/
/* cb_doOpenFile */
/*##################################################################*/

void
cb_doOpenFile(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    String name,error;

    BEGINMESSAGE(cb_doOpenFile)
    name = XawFileSelectionGetPath(FileSel);
    SMESSAGE(name)
    XtFree(open_directory);
    open_directory=file_getDirOfPath(name);
    SMESSAGE(open_directory)
    if ((error = misc_testFile(name))) {
      XawFileSelectionScan(FileSel,XawFileSelectionRescan);
      XtFree(error);
    } else {
      cb_popdownNotePopup((Widget)NULL,(XtPointer)NULL,NULL);
      XtPopdown(FileSel_popup);
      show_page(REQUEST_NEW_FILE,(XtPointer)name);
    }
    ENDMESSAGE(cb_doOpenFile)
}

/*##################################################################*/
/* cb_reopen */
/* Explicitly reopen the file. */
/*##################################################################*/

void
cb_reopen(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    char *error=NULL;
    BEGINMESSAGE(reopen_file)

    if ((error = misc_testFile(gv_filename))) {
       NotePopupShowMessage(error);
       XtFree(error);
    } else {
       cb_popdownNotePopup((Widget)NULL,(XtPointer)NULL,NULL);
       show_page(REQUEST_REOPEN,NULL);
    }
    ENDMESSAGE(reopen_file)
}

/*##################################################################*/
/* cb_savepos */
/*##################################################################*/

void
cb_savepos(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    FILE* posfile = fopen(gv_savepos_filename, "a");

    BEGINMESSAGE(cb_savepos)
    if (posfile == NULL) {
       INFMESSAGE(cannot open file for writting)
       ENDMESSAGE(cb_savepos)
       return;
    }
    fprintf(posfile, "%i %i\n", last_psx, last_psy);
    fclose(posfile);
    ENDMESSAGE(cb_savepos)
}

/*##################################################################*/
/* cb_presentation */
/*##################################################################*/

void
cb_presentation(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    int pid;
    sighandler_t sigold;

    BEGINMESSAGE(cb_presentation)

    sigold = signal(SIGCLD, SIG_IGN); /* Avoid zombies */
    if (!(pid = fork()))
    {
       /* We have to close all open file descriptors so the child does not
          inherit them */
       int i;
       for (i=3; i<256; i++) 
          close(i);
       execl("/usr/bin/env", "env", gv_bin, "--presentation", gv_filename, (char*)0);
       printf("Cannot exec %s\n", gv_bin);
       exit(1);
    }
    signal(SIGCLD, sigold); /* restore signal handler */
    
    ENDMESSAGE(cb_presentation)
}

/*##################################################################*/
/* cb_redisplay */
/*##################################################################*/

void
cb_redisplay(Widget w, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    BEGINMESSAGE(cb_redisplay)
    if (w && (XtClass(w) == ghostviewWidgetClass) && (w != page)) {
       INFMESSAGE(redisplay on zoom widget not enabled)
       ENDMESSAGE(cb_redisplay)
       return;
    }    
    show_page(REQUEST_REDISPLAY,NULL);
    ENDMESSAGE(cb_redisplay)
}

/*##################################################################*/
/* cb_page */
/*##################################################################*/

void
cb_page(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  int np;
  int cp;
  int k = 0;
  char *s;

  BEGINMESSAGE(cb_page)
  if (gv_filename && client_data) {
    s = (char*)client_data;
    if (*s=='-' || *s=='+') {  
      k = 1;
      np = atoi(s);
    } else if (*s=='=' && *(s+1)=='h') {  
      np = VlistHighlighted(newtoc);
      if (np >=0) { 
	k=2;
	VlistChangeHighlighted(newtoc,np,XawVlistUnset);
      }
    } else {
      k = 2;
      if (!isdigit(*s)) s++;
      np = atoi(s)-1;
    }
    
    if ((k==1 && np) || k==2) {
      if (toc_text) {
	if (gv_pending_page_request>NO_CURRENT_PAGE) cp=gv_pending_page_request;
	else cp = VlistSelected(newtoc);
	np = np + ((k==1) ? cp : 0);
	np=doc_putPageInRange(doc,np);
	IIMESSAGE(np,current_page)
	misc_setPageMarker(np,0,NULL,True);
	if (np != current_page) show_page(np,NULL);
      } else {
	if (k==1 && np > 0) show_page(np,NULL);
      }
    }
  }
  ENDMESSAGE(cb_page)
}
         
/*##################################################################*/
/* cb_positionPage */
/*##################################################################*/

void
cb_positionPage(Widget w, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  Widget clip,control,gvw;
  int clw,clh,cow,coh,px=0,py=0,x=0,y=0;
  Boolean center=False;
  Boolean scroll=False;
  Boolean have_pagepos=False;

  BEGINMESSAGE(cb_positionPage)

  if (w && (XtClass(w) == ghostviewWidgetClass)) {
    gvw     = w;
    control = XtParent(w);
    clip    = XtParent(control);
  } else {
    gvw     = page;
    control = viewControl;
    clip    = viewClip;
  }
  clw = (int)clip->core.width;
  clh = (int)clip->core.height;
  cow = (int)control->core.width;
  coh = (int)control->core.height;

  if ((int)(intptr_t)client_data) center = True;
  if (!center) have_pagepos=misc_restorePagePosition(&px,&py);
  if (app_res.auto_center == True) center = True;

  if (have_pagepos) {
    GhostviewReturnStruct grs;
    Position ocx,ocy,cx,cy;

    INFMESSAGE(using saved page position)
    cx = ((Position)clip->core.width)/2  - control->core.x - page->core.x;
    cy = ((Position)clip->core.height)/2 - control->core.y - page->core.y;
    
    GhostviewGetAreaOfBB (gvw,px,py,px,py,&grs);
    ocx = (int) grs.psx;
    ocy = (int) grs.psy;
    x = (int)control->core.x - (int)(ocx - cx);
    y = (int)control->core.y - (int)(ocy - cy);
    scroll=True;
  } else if (center) {
    INFMESSAGE(centering)
    x = -(cow - clw)/2;
    y = -(coh - clh)/2;
    scroll=True;
  }
  if (scroll) {
    INFIIMESSAGE(setting position to,x,y)
    ClipWidgetSetCoordinates(clip,x,y);
  }
  ENDMESSAGE(cb_positionPage)
}

/*##################################################################*/
/* cb_setPageMark */
/* Set/unset the 'page marked' property */
/*##################################################################*/

void
cb_setPageMark(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
    int r=(int)(intptr_t)client_data;
    int entry=XawVlistInvalid,change=XawVlistInvalid;

    BEGINMESSAGE(cb_setPageMark)
    if (!toc_text) {
      INFMESSAGE(no toc) ENDMESSAGE(cb_setPageMark)
      return;
    }
    if      (r & SPM_ALL)     entry =  XawVlistAll;
    else if (r & SPM_EVEN)    entry =  XawVlistEven;
    else if (r & SPM_ODD)     entry =  XawVlistOdd;
    else if (r & SPM_CURRENT) entry =  XawVlistCurrent;
    if      (r & SPM_MARK)    change = XawVlistSet;
    else if (r & SPM_UNMARK)  change = XawVlistUnset;
    else if (r & SPM_TOGGLE)  change = XawVlistToggle;
    VlistChangeMark(newtoc,entry,change);
    ENDMESSAGE(cb_setPageMark)
}

/*##################################################################*/
/* cb_autoResize */
/*##################################################################*/

void
cb_autoResize(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  Arg args[1];

  BEGINMESSAGE(cb_autoResize)
  if (client_data) {
    app_res.auto_resize = !(app_res.auto_resize);
    show_page(REQUEST_TOGGLE_RESIZE,NULL);
  }
  if (show_autoResize) {
    if (app_res.auto_resize) XtSetArg(args[0], XtNlabel,GV_AUTO_RESIZE_YES);
    else                     XtSetArg(args[0], XtNlabel,GV_AUTO_RESIZE_NO);
    XtSetValues(w_autoResize, args,ONE);
  }
  widgets_setSelectedBitmap(sizeEntry, app_res.auto_resize ? 1 : 0);
  ENDMESSAGE(cb_autoResize)
}

/*##################################################################*/
/* cb_setScale */
/*##################################################################*/

void
cb_setScale(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
  int i=(int)(intptr_t)client_data;

  BEGINMESSAGE(cb_setScale)
  i = scale_checkScaleNum(gv_scales,i);
  if (i>=0) {
    if (i&SCALE_BAS) gv_scale_base = i&SCALE_VAL;
    else             gv_scale = i&SCALE_VAL;
  }
  if (i>=0) {
    misc_savePagePosition();
    show_page(REQUEST_NEW_SCALE,NULL);
    misc_resetPagePosition();
  }
  ENDMESSAGE(cb_setScale)
}

/*##################################################################*/
/* cb_setOrientation */
/*##################################################################*/

void
cb_setOrientation(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data)
{
    int o = (int)(intptr_t) client_data;
    int changed = 1;

    BEGINMESSAGE(cb_setOrientation)
    switch (o) {
       case O_AUTOMATIC:
            INFMESSAGE(swapping auto orientation)
            if (gv_orientation_auto != 0) gv_orientation_auto = 0;
            else gv_orientation_auto = 1;
            break;
       case O_PORTRAIT:
       case O_SEASCAPE:
       case O_LANDSCAPE:
       case O_UPSIDEDOWN:
            gv_orientation = o;
            INFIMESSAGE(new orientation,gv_orientation)
            break;
       case O_SWAP_LANDSCAPE:
            INFMESSAGE(swapping landscape)
            if (gv_swap_landscape != 0) gv_swap_landscape = 0;
            else gv_swap_landscape = 1;
            break;
       default:
            INFMESSAGE(unknown orientation)
            changed = 0;
            break;
    }
    if (!call_data) {
      if (changed) {
	misc_savePagePosition();
	if (gv_scales[gv_scale]->scale > 0 || app_res.auto_resize)
	   show_page(REQUEST_NEW_ORIENTATION,NULL);
	else
	   show_page(REQUEST_SETUP,NULL);
	misc_resetPagePosition();
      }
    }
    ENDMESSAGE(cb_setOrientation)
}

/*##################################################################*/
/* cb_setPagemedia */
/*##################################################################*/

void
cb_setPagemedia(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data)
{
   int media = (int)(intptr_t)client_data;
   int media_bbox = doc ? doc->nummedia : 0;

   BEGINMESSAGE(cb_setPagemedia)
   if (media==MEDIA_ID_AUTO) {
     INFMESSAGE(toggling automatic media detection)
     gv_pagemedia_auto = gv_pagemedia_auto ? 0 : 1;
   } else if (media==media_bbox) {
     INFMESSAGE(changing to bounding box)
     if (doc_mediaIsOk(doc,current_page,media)) gv_pagemedia = media;
     else call_data = (XtPointer)1;
   } else {
     INFIMESSAGE(changing to pagemedia,media)
     gv_pagemedia = media;
   }
   if (!call_data) {
     if (gv_scales[gv_scale]->scale > 0 || app_res.auto_resize)
	   show_page(REQUEST_NEW_PAGEMEDIA,NULL);
     else
	   show_page(REQUEST_SETUP,NULL);
   }
   ENDMESSAGE(cb_setPagemedia)
}

/*##################################################################*/
/* cb_track */
/* track mouse pointer */
/*##################################################################*/

void
cb_track(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data)
{
    GhostviewReturnStruct *p = (GhostviewReturnStruct *)call_data;

    BEGINMESSAGE1(cb_track)
    /* locator events have zero width and height */
    if ((p->width == 0)&&(p->height == 0)) {
        if (show_locator) {
           static char buf[MAX_LOCATOR_LENGTH];
           static int x,y;
           if ((x != p->psx) || (y != p->psy) || (buf[0]='\0')) {
    	      sprintf(buf, app_res.locator_format, p->psx, p->psy);
              update_label(locator,buf);
           }
           x=p->psx; y=p->psy;
        }
        last_psx = p->psx;
        last_psy = p->psy;
        ENDMESSAGE1(cb_track)
	return;
    }


   ENDMESSAGE1(cb_track)
}

/*##################################################################*/
/* cb_message */
/* Process messages from ghostscript */
/* Refresh occurs when window was resized unexpectedly */
/*##################################################################*/

void
cb_message(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;
    char *error;

    BEGINMESSAGE(cb_message)
    if (!strcmp((char *) call_data, "Failed")) {
        INFMESSAGE(Failed)
	if ((Widget)client_data == page) {
            error = "\nError: PostScript interpreter failed in main window.\n\n";
	} else {
            error = "\nError: PostScript interpreter failed in zoom window.\n\n";
	}
	cb_appendInfoPopup((Widget)NULL,(XtPointer)NULL,(XtPointer)error);
    } else if (!strcmp((char *) call_data, "BadAlloc")) {
        INFMESSAGE(BadAlloc)
	if ((Widget)client_data == page) {
	    error = "\nWarning: Could not allocate backing pixmap in main window.\n\n";
	} else {
	    error = "\nWarning: Could not allocate backing pixmap in zoom window.\n\n";
	}
	cb_appendInfoPopup((Widget)NULL,(XtPointer)NULL,(XtPointer)error);
    } else if (!strcmp((char *) call_data, "Refresh")) {
        INFMESSAGE(Refresh)
	if (toc_text) {
	    GhostviewSendPS(w, gv_psfile, doc->beginprolog,
			    doc->lenprolog, False);
	    GhostviewSendPS(w, gv_psfile, doc->beginsetup,
			    doc->lensetup, False);
	    if (doc->pageorder == DESCEND)
		i = (doc->numpages - 1) - current_page;
	    else
		i = current_page;
	    GhostviewSendPS(w, gv_psfile, doc->pages[i].begin,
			    doc->pages[i].len, False);
	}
    } else if (!strcmp((char *) call_data, "Page")) {
        INFMESSAGE(completed page)
	if (w && (XtClass(w) == ghostviewWidgetClass) && (w != page)) {
	  INFMESSAGE(killing interpreter in zoom window)
	  GhostviewDisableInterpreter(w);
	}
	if ((gv_pending_page_request!=NO_CURRENT_PAGE) && (toc_text) && ((Widget)client_data == page)) {
           INFIMESSAGE(pending request for, gv_pending_page_request)
           show_page(gv_pending_page_request,NULL);
	}
    }
    ENDMESSAGE(cb_message)
}

/*##################################################################*/
/* cb_destroy */
/* Destroy popup window */
/*##################################################################*/

void
cb_destroy(Widget w _GL_UNUSED, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
    BEGINMESSAGE(cb_destroy)
    XtDestroyWidget((Widget)client_data);
    ENDMESSAGE(cb_destroy)
}

/*------------------------------------------------------------------*/
/* cb_shutdown */
/*------------------------------------------------------------------*/

static void
cb_shutdown(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   BEGINMESSAGE(cb_shutdown)

   if (gv_psfile) fclose(gv_psfile);
   if (gv_filename_dsc) unlink(gv_filename_dsc);
   if (gv_filename_unc) unlink(gv_filename_unc);
   process_kill_all_processes();

#if defined(DUMP_XTMEM) || defined(DUMP_MEM) 
   XtFree(gv_filename_dsc);
   XtFree(gv_filename_unc);
   XtFree(gv_filename_old);
   XtFree(gv_filename_raw);
   XtFree(gv_filename);
   psfree(doc);
   psfree(olddoc);
   XtFree(gv_scales_res);
   magmenu_freeMagMenuEntries(gv_magmenu_entries);
   miscmenu_freeMiscMenuEntries(gv_miscmenu_entries);
   scale_freeScales(gv_scales);
   XtFree(gv_medias_res);
   media_freeMedias(gv_medias);
   XtFree(gv_user_defaults_file);
   XtFree(scaleEntry);
   XtFree(open_directory);
   XtFree(save_directory);
   XtFree(toc_text);
   XtFree(pagemediaEntry);
   XtFree(gv_dirs);
   XtFree(gv_filters);
   XtFree(gv_filter);
   XtFree(gv_magmenu_entries_res);
   XtFree(gv_miscmenu_entries_res);
   XtFree(gv_print_command);
   XtFree(gv_uncompress_command);
   XtFree(gv_gs_interpreter);
   XtFree(gv_gs_cmd_scan_pdf);
   XtFree(gv_gs_cmd_conv_pdf);
   XtFree(gv_gs_x11_device);
   XtFree(gv_gs_x11_alpha_device);
   XtFree(gv_gs_arguments);
   resource_freeData();
#endif
   XtDestroyApplicationContext(app_con);
   ENDMESSAGE(cb_shutdown)
   ENDMESSAGE(exiting gv)
   clean_safe_tempdir();
   exit(EXIT_STATUS_NORMAL);
}

/*##################################################################*/
/* cb_destroyGhost */
/* destroy callback for Ghostview widgets. */
/* The disable interpreter call ensures that ghostscript is killed. */
/* Once the count goes to 0, we are sure that all forked processes have */
/* been killed and that we can safely exit. */
/*##################################################################*/

void
cb_destroyGhost(Widget w, XtPointer client_data, XtPointer call_data _GL_UNUSED)
{
    BEGINMESSAGE(cb_destroyGhost)
    GhostviewDisableInterpreter((Widget) client_data);
    num_ghosts--;
    if (num_ghosts) {
       ENDMESSAGE(cb_destroyGhost)
       return;
    }
    gv_exiting=1;
    if (dying) old_Xerror(XtDisplay(w), &bomb);
    /* Okay, okay, I'm a little pedantic. But I want to see the line
           MemDebug:   Stack is CLEAN !
           XtMemDebug: Stack is CLEAN !
       when activating the memory debug routines. For this we have to ensure
       that all destroy routines of all widgets are executed before leaving
       the application. So we just create a new shell, hook a destroy
       callback to it and destroy it immediately. The trick is that the
       creation of the shell is delayed until the main loop next becomes idle;
       and this will be after the dust of the above destruction has settled down.
    */
    toplevel = XtAppCreateShell("shutdown",gv_class,applicationShellWidgetClass,gv_display,NULL,0);
    XtAddCallback(toplevel,XtNdestroyCallback,cb_shutdown,(XtPointer)NULL);
    XtDestroyWidget(toplevel);
}

/*##################################################################*/
/* cb_quitGhostview */
/* Start application folding up by Destroying the top level widget. */
/* The application exits when the last interpreter is killed during */
/* a destroy callback from ghostview widgets. */
/*##################################################################*/

void
cb_quitGhostview(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  char *message=NULL;
 
  BEGINMESSAGE(cb_quitGhostview)
  if (app_res.confirm_quit>=1) {
    message = process_disallow_quit();
    if (message || app_res.confirm_quit>=2) {
      ConfirmPopupSetMessage("2", message);
      ConfirmPopupSetMessage("3", quitConfirmLabel);
      ConfirmPopupSetButton(CONFIRM_BUTTON_DONE,cb_doQuit);
      ConfirmPopupSetButton(CONFIRM_BUTTON_CANCEL,cb_cancelQuit);
      ConfirmPopupSetInitialButton(CONFIRM_BUTTON_CANCEL);
      cb_popupConfirmPopup((Widget)NULL,NULL,NULL);
      ENDMESSAGE(cb_quitGhostview)
      return;
    }
  }
  cb_doQuit((Widget)NULL,NULL,(XtPointer)NULL);
  ENDMESSAGE(cb_quitGhostview)
}

/*##################################################################*/
/* cb_cancelQuit */
/*##################################################################*/

void
cb_cancelQuit(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_cancelQuit)
  cb_popdownConfirmPopup((Widget)NULL,(XtPointer)NULL,NULL);
  ENDMESSAGE(cb_cancelQuit)
}

/*##################################################################*/
/* cb_doQuit */
/*##################################################################*/

void
cb_doQuit(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  BEGINMESSAGE(cb_doQuit)
  cb_popdownConfirmPopup((Widget)NULL,(XtPointer)NULL,NULL);
  XtUnmapWidget(toplevel);   
  XtDestroyWidget(toplevel);
  ENDMESSAGE(cb_doQuit)
}

/*##################################################################*/
/* cb_setPassword */
/*##################################################################*/

static void
cb_setPassword(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    String password;

    password = DialogPopupGetText();
    if (!password) password="";

    XtFree(gv_pdf_password);
    gv_pdf_password = XtNewString(password);

    cb_popdownDialogPopup((Widget)NULL,(XtPointer)NULL,NULL);
    cb_reopen(page,0,0);
}

/*##################################################################*/
/* cb_cancelPassword */
/*##################################################################*/

static void
cb_cancelPassword(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
    cb_popdownDialogPopup((Widget)NULL,(XtPointer)NULL,NULL);
}

/*##################################################################*/
/* cb_askPassword */
/* Ask for the password needed to view a pdf file */
/*##################################################################*/
void
cb_askPassword(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  DialogPopupSetPrompt(passwordPromptLabel);
  DialogPopupSetMessage(passwordRequiredLabel);
  DialogPopupSetButton(DIALOG_BUTTON_DONE,NULL,cb_setPassword);
  DialogPopupSetButton(DIALOG_BUTTON_CANCEL,NULL,cb_cancelPassword);
  DialogPopupSetText("");
  cb_popupDialogPopup((Widget)NULL,NULL,NULL);
  return;
}
