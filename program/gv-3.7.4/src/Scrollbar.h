/*
**
** Scrollbar.h
**
** Copyright (C) 1994, 1995, 1996, 1997 Johannes Plass
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

#ifndef _Scrollbar_h
#define _Scrollbar_h

#include "paths.h"
#include INC_XMU(Converters.h)
#include INC_X11(Xfuncproto.h)

/****************************************************************
 *
 * Scrollbar Widget
 *
 ****************************************************************/

/* Scrollbar resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 accelerators	     Accelerators	AcceleratorTable NULL
 ancestorSensitive   AncestorSensitive	Boolean		True
 background	     Background		Pixel		XtDefaultBackground
 backgroundPixmap    Pixmap		Pixmap		XtUnspecifiedPixmap
 borderColor	     BorderColor	Pixel		XtDefaultForeground
 borderPixmap	     Pixmap		Pixmap		XtUnspecifiedPixmap
 borderWidth	     BorderWidth	Dimension	1
 colormap	     Colormap		Colormap	parent's colormap
 cursor		     Cursor		Cursor		None
 cursorName	     Cursor		String		NULL
 depth		     Depth		int		parent's depth
 destroyCallback     Callback		XtCallbackList	NULL
 foreground	     Foreground		Pixel		XtDefaultForeground
 height		     Height		Dimension	length or thickness
 insensitiveBorder   Insensitive	Pixmap		GreyPixmap
 jumpProc	     Callback		XtCallbackList	NULL
 length		     Length		Dimension	1
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 minimumThumb	     MinimumThumb	Dimension	7
 orientation	     Orientation	XtOrientation	XtorientVertical
 pointerColor	     Foreground		Pixel		XtDefaultForeground
 pointerColorBackground Background	Pixel		XtDefaultBackground
 screen		     Screen		Screen		parent's screen
 sensitive	     Sensitive		Boolean		True
 shown		     Shown		Float		0.0
 thickness	     Thickness		Dimension	14
 thumb		     Thumb		Bitmap		GreyPixmap
 thumbProc	     Callback		XtCallbackList	NULL
 topOfThumb	     TopOfThumb		Float		0.0
 translations	     Translations	TranslationTable see source or doc
 width		     Width		Dimension	thickness or length
 x		     Position		Position	0
 y		     Position		Position	0

*/

/* 
 * Most things we need are in StringDefs.h 
 */

#define XtNminimumThumb "minimumThumb"
#define XtCMinimumThumb "MinimumThumb"

#define XtCShown "Shown"

#define XtNtopOfThumb "topOfThumb"
#define XtCTopOfThumb "TopOfThumb"

#define XtNshowArrows "showArrows"
#define XtCShowArrows "ShowArrows"

#define XtNthumbTopIndent "thumbTopIndent"
#define XtCThumbTopIndent "ThumbTopIndent"

#define XtNthumbSideIndent "thumbSideIndent"
#define XtCThumbSideIndent "ThumbSideIndent"

#define XtNarrowSideIndent "arrowSideIndent"
#define XtCArrowSideIndent "ArrowSideIndent"

#define XtNarrowTopIndent "arrowTopIndent"
#define XtCArrowTopIndent "ArrowTopIndent"

#define XtNinterval  "interval"
/* #define XtCInterval  "Interval"          changed on 4.11.94 ###jp###*/
#define XtNdelay  "delay"
#define XtCDelay  "Delay"

#define XtNuseDynamic "useDynamic"
#define XtCUseDynamic "UseDynamic"

typedef struct _ScrollbarRec	  *ScrollbarWidget;
typedef struct _ScrollbarClassRec *ScrollbarWidgetClass;


#define _EXTERN_ extern

_EXTERN_ WidgetClass scrollbarWidgetClass;

#undef _EXTERN_

_XFUNCPROTOBEGIN

extern void XawScrollbarSetThumb(
       Widget	/* scrollbar */,
#if NeedWidePrototypes
       double	/* top */,
       double	/* shown */
#else
       float	/* top */,
       float	/* shown */
#endif
);

_XFUNCPROTOEND

#endif /* _Scrollbar_h */
