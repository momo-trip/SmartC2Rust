/*
**
** Vlist.c
**
** Copyright (C) 1995, 1996, 1997 Johannes Plass
** Copyright (C) 2004 Jose E. Marchesi
** modified 2008 by Bernhard R. Link
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*
#define MESSAGES
*/
#include "message.h"

#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_XMU(Misc.h)
#include INC_XAW(XawInit.h)
#include "VlistP.h"
#include "Frame.h"
#include INC_XMU(Converters.h)

#define VLIST_MARK_LEFT_INDENT  4
#define VLIST_MARK_LABEL_INTERNAL_WIDTH 11
#define VLIST_MARK_LABEL_INDENT 4
#define VLIST_MARK_VERTICAL_INDENT 3
#define VLIST_MARK_WIDTH 5

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

static char defaultTranslations[] = "";

#define offset(field) XtOffsetOf(VlistRec, field)
static XtResource resources[] = { 
   {XtNreportCallback, XtCReportCallback, XtRCallback, sizeof(XtPointer),
	offset(vlist.report_callbacks), XtRCallback, (XtPointer) NULL },
   {XtNselectedShadowWidth, XtCShadowWidth, XtRDimension, sizeof(Dimension),
	offset(vlist.selected_shadow_width), XtRImmediate, (XtPointer) 1},
   {XtNmarkShadowWidth, XtCShadowWidth, XtRDimension, sizeof(Dimension),
	offset(vlist.mark_shadow_width), XtRImmediate, (XtPointer) 1},
   {XtNhighlightedShadowWidth, XtCShadowWidth, XtRDimension, sizeof(Dimension),
	offset(vlist.highlighted_shadow_width), XtRImmediate, (XtPointer) 1},
   {XtNhighlightedGeometry, XtCHighlightedGeometry, XtRString, sizeof(String),
      offset(vlist.highlighted_geometry), XtRImmediate, (XtPointer)"2 0 -2 1"},
   {XtNselectedGeometry, XtCSelectedGeometry, XtRString, sizeof(String),
      offset(vlist.selected_geometry), XtRImmediate, (XtPointer)"12 0 -2 1"},
   {XtNvlist,  XtCVlist, XtRString, sizeof(String),
      offset(vlist.vlist), XtRString, NULL},
   {XtNmarkBackground, XtCMarkBackground, XtRPixel, sizeof(Pixel),
	offset(vlist.mark_background), XtRString, XtDefaultBackground},
   {XtNselectedBackground, XtCSelectedBackground, XtRPixel, sizeof(Pixel),
	offset(vlist.selected_background), XtRString, XtDefaultBackground},
   {XtNhighlightedBackground, XtCHighlightedBackground, XtRPixel, sizeof(Pixel),
	offset(vlist.highlighted_background), XtRString, XtDefaultBackground},
   {XtNshadowWidth, XtCShadowWidth, XtRDimension, sizeof(Dimension),
      offset(threeD.shadow_width), XtRImmediate, (XtPointer) 2},
   {XtNallowMarks, XtCAllowMarks, XtRBoolean, sizeof(Boolean),
      offset(vlist.allow_marks), XtRImmediate, (XtPointer)True},
   {XtNinternalWidth, XtCWidth, XtRDimension,  sizeof(Dimension),
	offset(label.internal_width), XtRImmediate, (XtPointer)VLIST_MARK_LABEL_INTERNAL_WIDTH},
   {XtNborderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
      XtOffsetOf(RectObjRec,rectangle.border_width), XtRImmediate,
      (XtPointer)0}
};
#undef offset

static Boolean SetValues(Widget,Widget,Widget,ArgList,Cardinal*);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void Redisplay(Widget,XEvent*,Region);
static void Destroy(Widget);
static void PaintVlistWidget(Widget,XEvent*,Region);
static void ClassInitialize(void);
static void Resize(Widget);
static void PaintMarkOfEntry(Widget,Region,int,int,Boolean);
static void PaintMarkMarkOfEntry(Widget,Region,int,Boolean);


#define SuperClass ((LabelWidgetClass)&labelClassRec)

VlistClassRec vlistClassRec = {
  {
    (WidgetClass) SuperClass,		/* superclass		  */	
    "Vlist",				/* class_name		  */
    sizeof(VlistRec),			/* size			  */
    ClassInitialize,			/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    FALSE,				/* class_inited		  */
    Initialize,				/* initialize		  */
    NULL,				/* initialize_hook	  */
    XtInheritRealize,			/* realize		  */
    NULL,				/* actions		  */
    0,					/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* resource_count	  */
    NULLQUARK,				/* xrm_class		  */
    TRUE,				/* compress_motion	  */
    TRUE,				/* compress_exposure	  */
    TRUE,				/* compress_enterleave    */
    FALSE,				/* visible_interest	  */
    Destroy,				/* destroy		  */
    Resize,				/* resize		  */
    Redisplay,				/* expose		  */
    SetValues,				/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,		/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL				/* extension		  */
  },  /* CoreClass fields initialization */
  {
    XtInheritChangeSensitive		/* change_sensitive	*/
  },  /* SimpleClass fields initialization */
  {
    XtInheritXaw3dShadowDraw,           /* shadowdraw           */
  },  /* ThreeD Class fields initialization */
  {
    0,                                     /* field not used    */
  },  /* LabelClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* VlistClass fields initialization */
};

  /* for public consumption */
WidgetClass vlistWidgetClass = (WidgetClass) &vlistClassRec;

/*---------------------------------------------------*/
/* ClassInitialize */
/*---------------------------------------------------*/

static void ClassInitialize(void)
{
  BEGINMESSAGE(ClassInitialize)
  XawInitializeWidgetSet();
  ENDMESSAGE(ClassInitialize)
}

/*---------------------------------------------------*/
/* Initialize */
/*---------------------------------------------------*/

static void
Initialize(Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
  VlistWidget vw = (VlistWidget) new;
  String s="";
  char *c;
  XGCValues values;

  BEGINMESSAGE(Initialize)
  if (vw->vlist.vlist) s = vw->vlist.vlist;
  vw->vlist.vlist = XtNewString(s);
  c = vw->vlist.vlist;
  vw->vlist.firstVisible = 0;
  vw->vlist.selected = -1;
  vw->vlist.highlighted = -1;
  vw->vlist.entries = (int)strlen(vw->vlist.vlist);
  sscanf(vw->vlist.highlighted_geometry,"%d %d %d %d",
        &(vw->vlist.hulx),&(vw->vlist.huly),&(vw->vlist.hlrx),&(vw->vlist.hlry));
  sscanf(vw->vlist.selected_geometry,"%d %d %d %d",
        &(vw->vlist.sulx),&(vw->vlist.suly),&(vw->vlist.slrx),&(vw->vlist.slry));
  /* mark background GC */
  values.foreground	= vw->vlist.mark_background;
  vw->vlist.mark_background_GC = XtGetGC((Widget)vw,(unsigned) GCForeground,&values);
  /* selected background GC */
  values.foreground = vw->vlist.selected_background;
  vw->vlist.selected_background_GC = XtGetGC((Widget)vw,(unsigned) GCForeground,&values);
  /* highlighted background GC */
  values.foreground = vw->vlist.highlighted_background;
  vw->vlist.highlighted_background_GC = XtGetGC((Widget)vw,(unsigned) GCForeground,&values);
  /* background GC */
  values.foreground	= vw->core.background_pixel;
  values.graphics_exposures = False;
  vw->vlist.background_GC = XtGetGC((Widget)vw,(unsigned) GCForeground | GCGraphicsExposures,&values);

  /* TODO: check if this works here in international mode, or if it has
   * to be moved to Realize... */
#ifdef HAVE_XAW3D_INTERNATIONAL
  if( vw->simple.international == True ) {
    XFontSetExtents *ext = XExtentsOfFontSet(vw->label.fontset);
    vw->vlist.yofs = (ext->max_ink_extent.y<0)?-ext->max_ink_extent.y:ext->max_ink_extent.y;
    vw->vlist.ydelta = ext->max_ink_extent.height;
  } else {
#endif
    vw->vlist.yofs = vw->label.font->max_bounds.ascent;
    vw->vlist.ydelta = vw->label.font->max_bounds.ascent + vw->label.font->max_bounds.descent;
#ifdef HAVE_XAW3D_INTERNATIONAL
  }
#endif

  ENDMESSAGE(Initialize)
}

/*---------------------------------------------------*/
/* Redisplay */
/*---------------------------------------------------*/

static void
Redisplay(Widget w, XEvent *event, Region region)
{
  BEGINMESSAGE(Redisplay)
  PaintVlistWidget(w, event, region);
  ENDMESSAGE(Redisplay)
}

/*---------------------------------------------------*/
/* Resize */
/*---------------------------------------------------*/

static void shiftLabel(VlistWidget vw)
{
  BEGINMESSAGE(shiftLabel)
  if (vw->vlist.allow_marks) {
    vw->label.label_x = vw->label.internal_width +
                        vw->threeD.shadow_width  +
                        VLIST_MARK_LABEL_INDENT;
  }
  ENDMESSAGE(shiftLabel)
}

static void Resize(Widget w)
{
  VlistWidget vw = (VlistWidget) w;
  BEGINMESSAGE(Resize)
  (*vlistWidgetClass->core_class.superclass->core_class.resize)(w);
  shiftLabel(vw);
  ENDMESSAGE(Resize)
}

/*---------------------------------------------------*/
/* SetValues */
/*---------------------------------------------------*/

static Boolean
SetValues (Widget current, Widget request _GL_UNUSED, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
  VlistWidget cvw = (VlistWidget) current;
  VlistWidget nvw = (VlistWidget) new;
  Boolean changed = False;

  BEGINMESSAGE(SetValues)
  if (nvw->vlist.vlist == NULL) nvw->vlist.vlist = "";
  if (cvw->vlist.vlist != nvw->vlist.vlist) {
    XtFree((char *)cvw->vlist.vlist);
    nvw->vlist.vlist = XtNewString(nvw->vlist.vlist);
    nvw->vlist.entries = (int)strlen(nvw->vlist.vlist);
    if (!nvw->vlist.entries) nvw->vlist.entries =- 1;
    SMESSAGE(nvw->vlist.vlist)
    IMESSAGE(nvw->vlist.entries)
    changed = True;
  }
  ENDMESSAGE(SetValues)
  return (changed);
}

/*---------------------------------------------------*/
/* Destroy */
/*---------------------------------------------------*/

static void
Destroy(Widget w)
{
  VlistWidget vw = (VlistWidget)w;

  BEGINMESSAGE(Destroy)
  XtFree(vw->vlist.vlist);
  XtReleaseGC(w,vw->vlist.background_GC);
  XtReleaseGC(w,vw->vlist.mark_background_GC);
  XtReleaseGC(w,vw->vlist.selected_background_GC);
  XtReleaseGC(w,vw->vlist.highlighted_background_GC);
  ENDMESSAGE(Destroy)
}

/*---------------------------------------------------*/
/* PaintEntryString */
/*---------------------------------------------------*/

static void
PaintEntryString(Widget w, int entry)
{
  VlistWidget vw = (VlistWidget)w;
  char * s;
  int i;
  int yofs = vw->vlist.yofs, ydelta = vw->vlist.ydelta;

  yofs -= ydelta * vw->vlist.firstVisible;

  BEGINMESSAGE1(PaintEntryString)
  s = vw->label.label;
  i = entry;
  if (s) while (i > 0 && (s = strchr(s,'\n'))) { s++; i--; }
  if (s) {
    char *nl = strchr(s,'\n');
    int len;
    if (nl)
      len = nl - s;
    else
      len = strlen(s);
#ifdef HAVE_XAW3D_INTERNATIONAL
    if( vw->simple.international == True )
      XmbDrawString(XtDisplay(w), XtWindow(w), vw->label.fontset,
		    vw->label.normal_GC,
		    vw->label.label_x, vw->label.label_y + yofs + entry*ydelta,
		    s, len);
    else
#endif
      XDrawString(XtDisplay(w), XtWindow(w), vw->label.normal_GC,
		vw->label.label_x, vw->label.label_y + yofs + entry*ydelta,
		s, len);
  }
  ENDMESSAGE1(PaintEntryString)
}

/*---------------------------------------------------*/
/* PaintMarkOfEntry */
/*---------------------------------------------------*/

static int
PaintMark(Widget w, Region region, int entry, int style, Boolean erase)
{
  VlistWidget vw = (VlistWidget)w;
  int ss,ulx,uly,lrx,lry,ret=0;
  Position x,y;
  Dimension width,height;
  Dimension sw;
  GC gc;

  BEGINMESSAGE(PaintMark)
  IIMESSAGE(entry,erase)
  if (entry < 0 || entry >= vw->vlist.entries) {
    INFMESSAGE(invalid entry)
    ENDMESSAGE(PaintMark)
    return(ret);
  }
  if (entry < vw->vlist.firstVisible) {
    ENDMESSAGE(PaintMark)
    return 0;
  }
  if (style<0) { /* highlighted */
    INFMESSAGE(highlighted entry)
    ulx = vw->vlist.hulx;
    uly = vw->vlist.huly;
    lrx = vw->vlist.hlrx;
    lry = vw->vlist.hlry;
    sw  = vw->vlist.highlighted_shadow_width;
    gc  = vw->vlist.highlighted_background_GC;
  } else { /* selected */
    INFMESSAGE(selected entry)
    ulx = vw->vlist.sulx;
    uly = vw->vlist.suly;
    lrx = vw->vlist.slrx;
    lry = vw->vlist.slry;
    sw  = vw->vlist.selected_shadow_width;
    gc  = vw->vlist.selected_background_GC;
  }
  x = (Position) ulx;
  y = (Position) (((int) vw->label.label_y) +
		  ((entry - vw->vlist.firstVisible)*vw->vlist.ydelta) +
		  uly);
  width = (Dimension)((int) vw->core.width - ulx + lrx);
  height= (Dimension)(vw->vlist.ydelta - uly + lry + .5);
  ss = XawSUNKEN;
  if (region == NULL || XRectInRegion(region,x,y,width,height) != RectangleOut) {
    if (erase) {
      INFMESSAGE(clearing entry)
      XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w),vw->vlist.background_GC,x,y,width,height);
    } else {
      INFMESSAGE(drawing entry)
      XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w),gc,x,y,width,height);
      FrameDrawFrame(w,x,y,width,height,ss,sw,
		vw->threeD.top_shadow_GC,
		vw->threeD.bot_shadow_GC);
    }
    ret=1;
  }
  ENDMESSAGE(PaintMark)
  return ret;
}

static void
PaintMarkOfEntry(Widget w, Region region, int entry, int style, Boolean erase)
{
  VlistWidget vw = (VlistWidget)w;
  int pstyle,dstyle,d,p;
  int drawn=0,pdrawn=0;
  int order=0;

  BEGINMESSAGE(PaintMarkOfEntry)
  if (entry < 0 || entry >= vw->vlist.entries) {
    INFMESSAGE(invalid entry)
    ENDMESSAGE(PaintMarkOfEntry)
    return;
  }
  d = entry; dstyle=style;
  if (style<0) { p=vw->vlist.selected; pstyle=1; }
  else         { p=vw->vlist.highlighted; pstyle=-1; }
  if (d+1 == p)      order = 10;    
  else if (d-1 == p) { if (erase) order=10; else order=01; }
  else if (d==p)     { if (dstyle<0) order=10; else order=01; }
  else order = 999;
  IIMESSAGE(d,p)
  IMESSAGE(order)
  if (order==10 || order == 999) {
    drawn = PaintMark(w, region, d, dstyle,erase);
    if (order==10) pdrawn = PaintMark(w, region, p, pstyle,False);
  } else if (order==01) {
    pdrawn = PaintMark(w, region, p, pstyle,False);
    drawn = PaintMark(w, region, d, dstyle,erase);
  }
  if (pdrawn) {
    PaintEntryString(w,p);
    PaintMarkMarkOfEntry(w,region,p,False);
  }
  if (drawn && p!=d) {
    PaintEntryString(w,d);
    PaintMarkMarkOfEntry(w,region,d,False);
  }
  ENDMESSAGE(PaintMarkOfEntry)
}

/*---------------------------------------------------*/
/* PaintMarkMarkOfEntry */
/*---------------------------------------------------*/

static void
PaintMarkMarkOfEntry(Widget w, Region region, int entry, Boolean erase)
{
  VlistWidget vw = (VlistWidget)w;

  BEGINMESSAGE1(PaintMarkMarkOfEntry)
  if (vw->vlist.allow_marks) {
    int ss;
    Position x,y;
    Dimension width,height;
    char *vlist = vw->vlist.vlist;
    Boolean paint=False;

    if (entry < vw->vlist.firstVisible)
      return;

    if (vlist[entry] == '*') paint = True;
    if (paint || erase) {
      x = (Position) (VLIST_MARK_LEFT_INDENT);
      y = (Position) (((int) vw->label.label_y) +
		      VLIST_MARK_VERTICAL_INDENT +
		      ((entry-vw->vlist.firstVisible)*vw->vlist.ydelta));
      width = (Dimension) (VLIST_MARK_WIDTH);
      height= (Dimension)(vw->vlist.ydelta + 0.5 - 2*VLIST_MARK_VERTICAL_INDENT);
      ss = XawSUNKEN;
      if (region == NULL || XRectInRegion(region,x,y,width,height) != RectangleOut) {
	if (paint) {
	  INFMESSAGE(drawing mark)
	    XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w),vw->vlist.mark_background_GC,x,y,width,height);
	  FrameDrawFrame(w,x,y,width,height,ss,vw->vlist.mark_shadow_width,
			 vw->threeD.top_shadow_GC,
			 vw->threeD.bot_shadow_GC);
	} else {
	  INFMESSAGE(clearing mark)
	    XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w),
			   (entry == vw->vlist.highlighted) ? vw->vlist.highlighted_background_GC : vw->vlist.background_GC,
			   x,y,width,height);
	}
      }
    }
  }
  ENDMESSAGE1(PaintMarkMarkOfEntry)
}

/*---------------------------------------------------*/
/* PaintMarksOfEntries */
/*---------------------------------------------------*/

static void
PaintMarksOfEntries(Widget w, XEvent *event _GL_UNUSED, Region region)
{
  VlistWidget vw = (VlistWidget)w;

  BEGINMESSAGE(PaintMarksOfEntries)
  PaintMarkOfEntry(w, region, vw->vlist.highlighted,-1, False);
  if (vw->vlist.allow_marks) {
    int i= vw->vlist.firstVisible;
    while (i < vw->vlist.entries) {
      if (i != vw->vlist.highlighted)
	PaintMarkMarkOfEntry(w, region, i, False);
      i++;
    }
  }
  if (vw->vlist.selected != vw->vlist.highlighted && vw->vlist.entries >= 0)
    PaintMarkOfEntry(w, region, vw->vlist.selected,1, False);
  ENDMESSAGE(PaintMarksOfEntries)
}

/*---------------------------------------------------*/
/* PaintVlistWidget */
/*---------------------------------------------------*/

static void
PaintVlistWidget(Widget w, XEvent *event, Region region)
{
  Dimension width;
  char *s, *o;
  int i, y;
  VlistWidget vw = (VlistWidget)w;
  XRectangle rectangle;

  BEGINMESSAGE(PaintVlistWidget)
  /* better not allow the widget to grow that large, but that needs fixing
   * of the Clip widget */
  shiftLabel(vw);
  PaintMarksOfEntries(w, event, region);
  width = vw->threeD.shadow_width;
  o = vw->label.label;
  s = vw->label.label;
  i = vw->vlist.firstVisible;
  if (s) while (i > 0 && (s = strchr(s,'\n'))) { s++; i--; }
  /* This still fails when the list is too long and does not print anything.
   * Though that is perhaps best fixed by making Clip to enforce a real window
   * height...
  vw->label.label = s;
  vw->threeD.shadow_width = 0;
  (*SuperClass->core_class.expose) (w, event, region);
  vw->threeD.shadow_width =  width;
  vw->label.label = o;
  until Clip is extended, just manually draw each line:
  */
  if (region)
    XClipBox(region, &rectangle);
  else {
    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.width = vw->core.width;
    rectangle.height = vw->core.height;
    if( rectangle.height >= 0x4000 )
	    rectangle.height = 0x3fff;
  }
  y = vw->label.label_y + vw->vlist.yofs;
  i = 0;

  while (s != NULL) {
    char *nl = strchr(s, '\n');
    int len;
    if (nl)
      len = nl - s;
    else
      len = strlen(s);
    if (y - vw->vlist.yofs > rectangle.y + rectangle.height)
      break;
    if (y + (vw->vlist.ydelta - vw->vlist.yofs) >= rectangle.y) {
#ifdef HAVE_XAW3D_INTERNATIONAL
      if( vw->simple.international == True )
	XmbDrawString(XtDisplay(w), XtWindow(w), vw->label.fontset,
		      vw->label.normal_GC, vw->label.label_x, y, s, len);
      else
#endif
	XDrawString(XtDisplay(w), XtWindow(w), vw->label.normal_GC,
		    vw->label.label_x, y, s, len);
    }
    if (nl)
      s = nl + 1;
    else
      s = NULL;
    y += vw->vlist.ydelta;
  }
  ENDMESSAGE(PaintVlistWidget)
}

/*####################################################################*/
/*####################################################################*/
/* Public Routines */
/*####################################################################*/
/*####################################################################*/

/*###################################################*/
/* VlistSelected */
/*###################################################*/

int
VlistSelected(Widget w)
{
  VlistWidget vw = (VlistWidget)w;

  BEGINMESSAGE(VlistSelected)
  ENDMESSAGE(VlistSelected)
  return(vw->vlist.selected);
}

/*###################################################*/
/* VlistHighlighted */
/*###################################################*/

int
VlistHighlighted(Widget w)
{
  VlistWidget vw = (VlistWidget)w;

  BEGINMESSAGE(VlistHighlighted)
  ENDMESSAGE(VlistHighlighted)
  return(vw->vlist.highlighted);
}

/*###################################################*/
/* VlistEntries */
/*###################################################*/

int
VlistEntries(Widget w)
{
  VlistWidget vw = (VlistWidget)w;

  BEGINMESSAGE(VlistEntries)
  ENDMESSAGE(VlistEntries)
  return(vw->vlist.entries);
}

/*###################################################*/
/* VlistVlist */
/*###################################################*/

char*
VlistVlist(Widget w)
{
  VlistWidget vw = (VlistWidget)w;

  BEGINMESSAGE(VlistVlist)
  ENDMESSAGE(VlistVlist)
  return(vw->vlist.vlist);
}

/*###################################################*/
/* VlistEntryOfPosition */
/*###################################################*/

int
VlistEntryOfPosition(Widget w, int y)
{
  VlistWidget vw = (VlistWidget)w;
  int entry = -1;

  BEGINMESSAGE(VlistEntryOfPosition)
  y = y - (int) vw->label.label_y;
  if (vw->vlist.ydelta > 0) {
    if (y < 0) entry = -1;
    else       entry = y/vw->vlist.ydelta;
  }
  if (entry >= vw->vlist.entries)
	  entry = vw->vlist.entries-1;
  if (entry >= 0)
    entry += vw->vlist.firstVisible;
  IMESSAGE(entry)
  ENDMESSAGE(VlistEntryOfPosition)
  return(entry);
}

/*###################################################*/
/* VlistPositionOfEntry */
/*###################################################*/

void
VlistPositionOfEntry(Widget w, int e, int *yuP, int *ylP)
{
  VlistWidget vw = (VlistWidget)w;
  float h;
  BEGINMESSAGE(VlistPositionOfEntry)
  *yuP = *ylP = (int) vw->label.label_y;
  if (e >= vw->vlist.firstVisible && vw->vlist.entries > 0) {
    e -= vw->vlist.firstVisible;
    h = vw->vlist.ydelta;
    *yuP += (int)((float)e*h);
    *ylP += (int)((float)(e+1)*h);
  }
  ENDMESSAGE(VlistPositionOfEntry)
}

/*###################################################*/
/* VlistChangeMark */
/*###################################################*/

void
VlistChangeMark(Widget w, int entry,int change)
{
  VlistWidget vw = (VlistWidget)w;
  char *vlist=vw->vlist.vlist;
  Boolean paint;
  int lb,ub,st;

  BEGINMESSAGE(VlistChangeMark)
  switch (entry) {
    case XawVlistAll:
      lb=0;
      ub=vw->vlist.entries-1;
      st=1;
      break;
    case XawVlistEven:
      lb=1;
      ub=vw->vlist.entries-1;
      st=2;
      break;
    case XawVlistOdd:
      lb=0;
      ub=vw->vlist.entries-1;
      st=2;
      break;
    case XawVlistCurrent:
      lb=vw->vlist.selected;
      ub=vw->vlist.selected;
      st=1;
      break;
    default:
      lb=entry;
      ub=entry;
      st=1;
      break;
  }

  if (ub < 0 || ub >= vw->vlist.entries) {
    INFMESSAGE(invalid setup)
    ENDMESSAGE(VlistChangeMark)
    return;
  }

  entry=lb;
  while (entry <= ub) {
    paint=False;
    switch (change) {
      case XawVlistToggle:
	switch (vlist[entry]) {
          case '*':
	    vlist[entry]=' ';
	    paint=True;
	    break;
          default:
	    vlist[entry]='*';
	    paint=True;
	    break;
        }
	break;
      case XawVlistSet:
	vlist[entry]='*';
	paint=True;
	break;
      case XawVlistUnset:
	vlist[entry]=' ';
	paint=True;
	break;
      default:
	INFMESSAGE(invalid action)
	break;
    }
    if (paint) PaintMarkMarkOfEntry(w,NULL,entry,True);
    entry += st;
  }
  ENDMESSAGE(VlistChangeMark)
}

/*---------------------------------------------------*/
/* vlist_change_mark */
/*---------------------------------------------------*/

static void
vlist_change_mark(Widget w, int e, int change, int kind)
{
  VlistWidget vw = (VlistWidget)w;
  int *eP;

  BEGINMESSAGE(vlist_change_mark)
  if (kind < 0) eP = &(vw->vlist.highlighted);
  else eP = &(vw->vlist.selected);
  switch (change) {
    case XawVlistSet:
      if (*eP >= 0 && *eP < vw->vlist.entries) PaintMarkOfEntry(w,NULL,*eP,kind,True);
      *eP = -1;
      if (e >= 0 && e < vw->vlist.entries) {
	PaintMarkOfEntry(w,NULL,e,kind,False);
	*eP = e;
      }
      break;
    case XawVlistUnset:
      if (*eP >= 0 && *eP < vw->vlist.entries) PaintMarkOfEntry(w,NULL,*eP,kind,True);
      *eP = -1;
      break;
    default:
      INFMESSAGE(invalid action)
      break;
  }
  ENDMESSAGE(vlist_change_mark)
}

/*###################################################*/
/* VlistChangeSelected */
/*###################################################*/

void
VlistChangeSelected(Widget w, int entry, int change)
{
  BEGINMESSAGE(VlistChangeSelected)
  vlist_change_mark(w,entry,change,1);
  ENDMESSAGE(VlistChangeSelected)
}

/*###################################################*/
/* VlistChangeHighlighted */
/*###################################################*/

void
VlistChangeHighlighted(Widget w, int entry, int change)
{
  BEGINMESSAGE(VlistChangeHighlighted)
  vlist_change_mark(w,entry,change,-1);
  ENDMESSAGE(VlistChangeHighlighted)
}

/*###################################################*/
/* VlistGetFirstVisible */
/*###################################################*/

int VlistGetFirstVisible(Widget w)
{
  VlistWidget vw = (VlistWidget)w;

  return vw->vlist.firstVisible;
}

/*###################################################*/
/* VlistSetFirstVisible */
/*###################################################*/

void VlistSetFirstVisible(Widget w, int newf)
{
  VlistWidget vw = (VlistWidget)w;
  unsigned int height;

  BEGINMESSAGE(VlistSetFirstVisible)
  IIMESSAGE(newf,vw->vlist.entries)
  if (newf < 0)
    newf = 0;
  else if (newf >= vw->vlist.entries)
    newf = vw->vlist.entries - 1;
  if (newf == -1) {
    ENDMESSAGE(VlistSetFirstVisible)
    return;
  }
  IIMESSAGE(newf,vw->vlist.firstVisible)
  if (newf != vw->vlist.firstVisible) {
    vw->vlist.firstVisible = newf;
    /* better not allow the widget to grow that large, but that needs fixing
     * of the Clip widget */
    height = vw->core.height;
    if( height >= 0x8000 )
	    height = 0x3fff;
    IIMESSAGE(vw->core.height,height)
    XFillRectangle(XtDisplayOfObject(w), XtWindowOfObject(w), vw->vlist.background_GC,
		    vw->core.x, vw->core.y, vw->core.width, height);
    Redisplay(w, NULL, NULL);
    if (vw->vlist.report_callbacks)
      XtCallCallbackList (w, vw->vlist.report_callbacks, (XtPointer)0);
  }
  ENDMESSAGE(VlistSetFirstVisible)
}
/*###################################################*/
/* VlistMoveFirstVisible */
/*###################################################*/

extern int debug_p;

void VlistMoveFirstVisible(Widget w, int start, int ydiff)
{
  VlistWidget vw = (VlistWidget)w;
  int ly;
  int newf;

  BEGINMESSAGE(VlistMoveFirstVisible)
  IIMESSAGE(start,ydiff)
  ly = vw->vlist.ydelta;
  if (debug_p) fprintf(stderr, "move: start=%d ydiff=%d ly=%d\n", start, ydiff, ly);

  if (ydiff > 0 && ly > ydiff) ydiff=ly;
  if (ydiff < 0 && ly > -ydiff) ydiff=-ly;

  if (ydiff >= 0)
	  ydiff += ly/2;
  else
	  ydiff -= ly/2;

  newf = start + ydiff/ly;
  VlistSetFirstVisible(w, newf);
  ENDMESSAGE(VlistMoveFirstVisible)
}

/*###################################################*/
/* VlistScrollPosition */
/*###################################################*/

float VlistScrollPosition(Widget w)
{
  VlistWidget vw = (VlistWidget)w;
  float position;

#if 0
  printf("Scroll position %d/%d=%f\n",
		  vw->vlist.firstVisible,(int)(vw->vlist.entries),
		  vw->vlist.firstVisible/(float)(vw->vlist.entries));
#endif
  position = vw->vlist.firstVisible/(float)vw->vlist.entries;
  INFMESSAGE(VlistScrollPosition)
  IIMESSAGE(vw->vlist.firstVisible,vw->vlist.entries)
  FMESSAGE(position)
  return position;
}

float VlistVisibleLength(Widget w, unsigned int height)
{
  VlistWidget vw = (VlistWidget)w;
  float percent;
  int entriesvisible = -1;

  if (vw->vlist.ydelta > 0) {
    entriesvisible = height/vw->vlist.ydelta;
  }
#if 0
  printf("fitting %d entries of height %d in %d",
		  entriesvisible, (int)vw->vlist.ydelta, (int)height);
#endif
  if (entriesvisible >= vw->vlist.entries)
	  entriesvisible = vw->vlist.entries;
#if 0
  printf(", visible percents %d/%d=%f\n",
		  entriesvisible,(int)(vw->vlist.entries),
		  entriesvisible/(float)(vw->vlist.entries));
#endif
  percent = entriesvisible/(float)(vw->vlist.entries);
  INFMESSAGE(VlistVisibleLength)
  IIMESSAGE(entriesvisible,vw->vlist.entries)
  FMESSAGE(percent)
  return percent;
}

int VlistMaxEntriesVisible(Widget w, int height)
{
  VlistWidget vw = (VlistWidget)w;
  int entriesvisible = -1;

  if (vw->vlist.ydelta > 0) {
    entriesvisible = (height - vw->label.label_y)/vw->vlist.ydelta;
  }

  INFIMESSAGE(VlistMaxEntriesVisible,entriesvisible)

  return entriesvisible;
}
