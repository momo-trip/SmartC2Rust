/*
**
** Frame.h
**
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
** Author:   Jose E. Marchesi (jemarch@gnu.org)
**           GNU Project
**
*/

#ifndef _XawFrame_h
#define _XawFrame_h

#include "paths.h"
#include INC_X11(Xfuncproto.h)

#define XtNshadowWidth "shadowWidth"
#define XtCShadowWidth "ShadowWidth"
#define XtNtopShadowPixel "topShadowPixel"
#define XtCTopShadowPixel "TopShadowPixel"
#define XtNbottomShadowPixel "bottomShadowPixel"
#define XtCBottomShadowPixel "BottomShadowPixel"
#define XtNuserData "userData"
#define XtCUserData "UserData"

typedef enum {
  XawRAISED,
  XawSUNKEN,
  XawCHISELED,
  XawLEDGED,
  XawFrameMassiveRaised
} XawFrameType;
 
#define XtNframeType "frameType"
#define XtCFrameType "FrameType"
#define XtRFrameType "FrameType"

#ifndef XtNhSpace             
#define XtNhSpace "hSpace"
#endif
#ifndef XtNvSpace             
#define XtNvSpace "vSpace"
#endif
#ifndef XtCHSpace             
#define XtCHSpace "HSpace"
#endif
#ifndef XtCVSpace             
#define XtCVSpace "VSpace"
#endif

/* Class record constants */

extern WidgetClass frameWidgetClass;

typedef struct _FrameClassRec *FrameWidgetClass;
typedef struct _FrameRec      *FrameWidget;

_XFUNCPROTOBEGIN

extern Boolean                  FrameConvertStringToFrameType (
    Display*		/* dpy */,
    XrmValue*		/* args */,
    Cardinal*		/* num_args */,
    XrmValuePtr		/* fromVal */,
    XrmValuePtr		/* toVal */,
    XtPointer*		/* converter_data */
);

extern void                     FrameDrawFrame (
    Widget,        /* widget */
    int,           /* x */
    int,           /* y */
    int,           /* width */
    int,           /* height */
    XawFrameType,  /* frame_type */
    int,           /* shadow width */
    GC,            /* light GC */
    GC             /* dark GC */
);

_XFUNCPROTOEND

#endif /* _XawFrame_h */
