/*
**
** Clip.h
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
**        
*/

#ifndef _XawClip_h
#define _XawClip_h

#include INC_XAW(Reports.h)

/*****************************************************************************
 * 
 * Clip Widget (subclass of Composite)
 * 
 * This widget is similar to a viewport without scrollbars.  Child movement
 * is done by external panners or scrollbars.
 * 
 * Parameters:
 * 
 *  Name		Class		Type		Default
 *  ----		-----		----		-------
 * 
 *  background		Background	Pixel		XtDefaultBackground
 *  border	        BorderColor	Pixel		XtDefaultForeground
 *  borderWidth		BorderWidth	Dimension	1
 *  height		Height		Dimension	0
 *  reportCallback	ReportCallback	Pointer		NULL
 *  width		Width		Dimension	0
 *  x 			Position	Position	0
 *  y			Position	Position	0
 * 
 *****************************************************************************/

#define XtNchildX "childX"
#define XtCChildX "ChildX"
#define XtNchildY "childY"
#define XtCChildY "ChildY"

extern WidgetClass clipWidgetClass;

typedef struct _ClipClassRec *ClipWidgetClass;
typedef struct _ClipRec      *ClipWidget;

extern void ClipWidgetCheckCoordinates (
    Widget	/* w */,
    int		/* x */,
    int		/* y */,
    int*	/* xP */,
    int*	/* yP */
);

extern void ClipWidgetSetCoordinates (
    Widget	/* w */,
    int		/* x */,
    int		/* y */
);

#endif /* _XawClip_h */
