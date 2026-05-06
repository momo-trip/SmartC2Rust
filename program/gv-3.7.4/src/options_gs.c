/*
**
** options_gs.c
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
#include <stdlib.h>
#include <ctype.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_X11(Shell.h)
#include INC_XAW(SimpleMenu.h)
#include INC_XAW(SmeBSB.h)
#include INC_XAW(AsciiText.h)
#include "Aaa.h"
#include "Button.h"
#include "FileSel.h"
#include "Ghostview.h"
#include "Frame.h"
#include "MButton.h"
#include "Switch.h"

#include "types.h"
#include "callbacks.h"
#include "file.h"
#include "widgets_misc.h"
#include "main_resources.h"
#include "main_globals.h"
#include "misc.h"
#include "note.h"
#include "options.h"
#include "options_gv.h"
#include "resource.h"
#include "ps.h"
#include "doc_misc.h"
#include "version.h"

static Widget   popup=NULL,optionControl;
static Widget   quietToggle,saferToggle,safeDirToggle;
static Widget   scan,conv,gs,x11dev,x11alphadev,arguments;

static void options_gs_setOptionsAtEntry(void);
static void options_gs_create(void);
extern void main_setGhostscriptResources(XrmDatabase);
extern String intern_resources[]; /* defined in resource.c */

/*------------------------------------------------------
  OptionPopupStruct
------------------------------------------------------*/

OptionPopupStruct gv_options_gs_struct = {
  NULL,
  options_gs_create,
  options_gs_setOptionsAtEntry,
  False
};

OptionPopup gv_options_gs = (OptionPopup) &gv_options_gs_struct;

/*------------------------------------------------------
   options_gs_setOptionsAtEntry
------------------------------------------------------*/

static void options_gs_setOptionsAtEntry(void)
{
  BEGINMESSAGE(options_gs_setOptionsAtEntry)

  widgets_setToggle(safeDirToggle, gv_gs_safeDir);
  widgets_setToggle(saferToggle, gv_gs_safer);
  widgets_setToggle(quietToggle, gv_gs_quiet);

  SMESSAGE(gv_gs_interpreter)
  widgets_setText(gs, gv_gs_interpreter);
  SMESSAGE(gv_gs_cmd_scan_pdf)
  widgets_setText(scan, gv_gs_cmd_scan_pdf);
  SMESSAGE(gv_gs_cmd_conv_pdf)
  widgets_setText(conv, gv_gs_cmd_conv_pdf);
  SMESSAGE(gv_gs_x11_device)
  widgets_setText(x11dev, gv_gs_x11_device);
  SMESSAGE(gv_gs_x11_alpha_device)
  widgets_setText(x11alphadev, gv_gs_x11_alpha_device);
  SMESSAGE(gv_gs_arguments)
  widgets_setText(arguments, gv_gs_arguments);

  ENDMESSAGE(options_gs_setOptionsAtEntry)
}

/*------------------------------------------------------
   options_gs_apply
------------------------------------------------------*/

static Boolean options_gs_change (Widget w, String *sP, String *fP)
{
  String v;
  Boolean r=False;
  BEGINMESSAGE(options_gs_change)
  v = widgets_getText(w);
  v = options_squeeze(v);
  if (strcmp(v,*sP)) {
      *fP = *sP;
      *sP = v;
      r = True;
  } else XtFree(v);
  ENDMESSAGE(options_gs_change)
  return(r);
}

static void options_gs_cb_apply(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
   Arg args[10];
   Cardinal n;
   int i;
   Boolean reopen=False;
   String f[6] = {NULL,NULL,NULL,NULL,NULL,NULL};

   BEGINMESSAGE(options_gs_cb_apply)

   i = gv_gs_quiet;
   gv_gs_quiet = SwitchIsSet(quietToggle) ? 1 : 0;
   if (i != gv_gs_quiet) reopen=True;

   i = gv_gs_safer;
   gv_gs_safer = SwitchIsSet(saferToggle) ? 1 : 0;
   if (i != gv_gs_safer) reopen=True;

   i = gv_gs_safeDir;
   gv_gs_safeDir = SwitchIsSet(safeDirToggle) ? 1 : 0;
   if (i != gv_gs_safeDir) reopen=True;

   reopen |= options_gs_change(gs,&gv_gs_interpreter,&(f[0]));
   reopen |= options_gs_change(scan,&gv_gs_cmd_scan_pdf,&(f[1]));
            options_gs_change(conv,&gv_gs_cmd_conv_pdf,&(f[2]));
   reopen |= options_gs_change(x11dev,&gv_gs_x11_device,&(f[3]));
   reopen |= options_gs_change(x11alphadev,&gv_gs_x11_alpha_device,&(f[4]));
   reopen |= options_gs_change(arguments,&gv_gs_arguments,&(f[5]));

   if (reopen) {
     cb_stopInterpreter(page,NULL,NULL);
								n=0;
     XtSetArg(args[n], XtNinterpreter,gv_gs_interpreter);	n++;
     if (gv_gs_safeDir)  XtSetArg(args[n], XtNsafeDir,True);
     else              XtSetArg(args[n], XtNsafeDir,False);
                                                                n++;
     if (gv_gs_safer)  XtSetArg(args[n], XtNsafer,True);
     else              XtSetArg(args[n], XtNsafer,False);
                                                                n++;
     if (gv_gs_quiet)  XtSetArg(args[n], XtNquiet,True);
     else              XtSetArg(args[n], XtNquiet,False);
                                                                n++;
     XtSetArg(args[n], XtNarguments,gv_gs_arguments);           n++;
     XtSetValues(page, args, n);
     if (gv_filename) show_page(REQUEST_REOPEN,NULL);
   }
   for (i=0; i<6 ; i++) XtFree(f[i]);

   ENDMESSAGE(options_gs_cb_apply)
}

/*------------------------------------------------------
   options_gs_cb_save
------------------------------------------------------*/

static void options_gs_setArg(Widget w, String *argiP, String *argvP, int *argnP, String format, String name)
{
  String s;
  s = widgets_getText(w);
  s = options_squeeze(s);
  options_setArg(argiP,argvP,format,name,s);
  ++(*argnP);
  XtFree(s);
}

static
void options_gs_cb_save(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  int    argn = 0;
  String argi[20];
  String argv[20];
  String t = "True";
  String f = "False";

  BEGINMESSAGE(options_gv_cb_save)

  options_setArg(&(argi[argn]),&(argv[argn]),s_version             ,gv_class       ,versionResource);
       ++argn;
  options_gs_setArg(gs         ,&(argi[argn]),&(argv[argn]),&argn,s_gsInterpreter    ,gv_class);
  options_gs_setArg(scan       ,&(argi[argn]),&(argv[argn]),&argn,s_gsCmdScanPDF     ,gv_class);
  options_gs_setArg(conv       ,&(argi[argn]),&(argv[argn]),&argn,s_gsCmdConvPDF     ,gv_class);
  options_gs_setArg(x11dev     ,&(argi[argn]),&(argv[argn]),&argn,s_gsX11Device      ,gv_class);
  options_gs_setArg(x11alphadev,&(argi[argn]),&(argv[argn]),&argn,s_gsX11AlphaDevice ,gv_class);
  options_gs_setArg(arguments  ,&(argi[argn]),&(argv[argn]),&argn,s_gsArguments      ,gv_class);

  options_setArg(&(argi[argn]),&(argv[argn]),s_gsSafeDir        ,gv_class ,SwitchIsSet(safeDirToggle) ? t : f);
  ++argn;
  options_setArg(&(argi[argn]),&(argv[argn]),s_gsSafer          ,gv_class ,SwitchIsSet(saferToggle) ? t : f);
  ++argn;
  options_setArg(&(argi[argn]),&(argv[argn]),s_gsQuiet          ,gv_class ,SwitchIsSet(quietToggle) ? t : f);
  ++argn;

  options_save(argn,argi,argv);
  while (--argn >=0) {
    XtFree(argi[argn]);
    XtFree(argv[argn]);
  }

  ENDMESSAGE(options_gv_cb_save)
}

/*------------------------------------------------------
   options_gs_cb_defaults
------------------------------------------------------*/

static void options_gs_cb_defaults(Widget w _GL_UNUSED, XtPointer client_data _GL_UNUSED, XtPointer call_data _GL_UNUSED)
{
  XrmDatabase db = NULL;
  String *sP = intern_resources;

  BEGINMESSAGE(options_gs_cb_defaults)
  while (*sP) XrmPutLineResource(&db,*sP++);
  main_setGhostscriptResources(db);
  XrmDestroyDatabase(db);
  options_gs_setOptionsAtEntry();
  ENDMESSAGE(options_gs_cb_defaults)
}

/*------------------------------------------------------
   options_gs_create
------------------------------------------------------*/

void options_gs_create(void)
{
   Arg       args[5];
   Cardinal  n;
   Widget    w;

   BEGINMESSAGE(options_gs_create)

   if (popup) {
     INFMESSAGE(popup exists)
     ENDMESSAGE(options_gs_create)
     return;
   }

        						n=0;
        XtSetArg(args[n], XtNallowShellResize, True);	n++;
   popup = XtCreatePopupShell("optiongsPopup",transientShellWidgetClass,toplevel, args, n);
   gv_options_gs->popup = popup;
         						n=0;
   optionControl = XtCreateManagedWidget("optionControl",aaaWidgetClass,popup,args,n);

   safeDirToggle      = XtCreateManagedWidget("safeDir",switchWidgetClass,optionControl,NULL,(Cardinal)0);
   saferToggle        = XtCreateManagedWidget("safer",switchWidgetClass,optionControl,NULL,(Cardinal)0);
   quietToggle        = XtCreateManagedWidget("quiet",switchWidgetClass,optionControl,NULL,(Cardinal)0);
 
   gs           = widgets_createLabeledLineTextField("gs",   optionControl);
   x11dev       = widgets_createLabeledLineTextField("x11dev", optionControl);
   x11alphadev  = widgets_createLabeledLineTextField("x11alphadev", optionControl);
   arguments    = widgets_createLabeledLineTextField("arguments", optionControl);
   scan         = widgets_createLabeledTextField("scan", optionControl);
   conv         = widgets_createLabeledTextField("conv", optionControl);

                                          n=0;
   XtSetArg(args[n], XtNwrap, XawtextWrapWord);  ++n;
   XtSetArg(args[n], XtNresize, XawtextResizeHeight);  ++n;
   XtSetArg(args[n], XtNscrollVertical, XawtextScrollNever);  ++n;
   XtSetValues(scan, args, n);
   XtSetValues(conv, args, n);

        						n=0;
   w = XtCreateManagedWidget("apply", buttonWidgetClass,optionControl, args, n);
         XtAddCallback(w, XtNcallback, options_gs_cb_apply,NULL); 
         XtInstallAccelerators(optionControl, w);
   w = XtCreateManagedWidget("save", buttonWidgetClass,optionControl, args, n);
         XtAddCallback(w, XtNcallback, options_gs_cb_save,NULL);
         XtInstallAccelerators(optionControl, w);
   w = XtCreateManagedWidget("dismiss", buttonWidgetClass,optionControl, args, n);
         XtAddCallback(w, XtNcallback, options_cb_popdown, (XtPointer)gv_options_gs);
         XtInstallAccelerators(optionControl, w);
   w = XtCreateManagedWidget("defaults", buttonWidgetClass,optionControl, args, n);
         XtAddCallback(w, XtNcallback, options_gs_cb_defaults, NULL);
         XtInstallAccelerators(optionControl, w);

   options_gs_setOptionsAtEntry();
   options_realize(popup,optionControl);
                                
   ENDMESSAGE(options_gv_cb_create)
}
