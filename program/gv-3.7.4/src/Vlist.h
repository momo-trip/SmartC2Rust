/*
**
** Vlist.h
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

#ifndef _Vlist_h_
#define _Vlist_h_

#include "paths.h"
#include INC_XAW(Label.h)
#include INC_XAW(Reports.h)

#define XawVlistAll     -13
#define XawVlistEven    -12
#define XawVlistOdd     -11
#define XawVlistCurrent -10
#define XawVlistInvalid  -1
#define XawVlistSet       1
#define XawVlistUnset     2
#define XawVlistToggle    3

#define XtNvlist "vlist"
#define XtCVlist "Vlist"
#define XtNmarkShadowWidth "markShadowWidth"
#define XtNselectedShadowWidth "selectedShadowWidth"
#define XtNhighlightedShadowWidth "highlightedShadowWidth"
#define XtNmarkBackground "markBackground"
#define XtCMarkBackground "MarkBackground"
#define XtNselectedBackground "selectedBackground"
#define XtCSelectedBackground "SelectedBackground"
#define XtNhighlightedBackground "highlightedBackground"
#define XtCHighlightedBackground "HighlightedBackground"
#define XtNhighlightedGeometry "highlightedGeometry"
#define XtCHighlightedGeometry "HighlightedGeometry"
#define XtNselectedGeometry "selectedGeometry"
#define XtCSelectedGeometry "SelectedGeometry"
#define XtNallowMarks "allowMarks"
#define XtCAllowMarks "AllowMarks"

extern WidgetClass vlistWidgetClass;

typedef struct _VlistClassRec   *VlistWidgetClass;
typedef struct _VlistRec        *VlistWidget;

extern int   VlistHighlighted (Widget);
extern int   VlistSelected (Widget);
extern int   VlistEntries (Widget);
extern char* VlistVlist (Widget);
extern int   VlistEntryOfPosition (Widget, int);
extern void  VlistPositionOfEntry (Widget, int, int*, int*);
extern void  VlistChangeMark (Widget, int, int);
extern void  VlistChangeSelected (Widget, int, int);
extern void  VlistChangeHighlighted (Widget, int, int);
extern int   VlistGetFirstVisible(Widget);
extern void  VlistSetFirstVisible(Widget, int);
extern void  VlistMoveFirstVisible(Widget, int, int);
extern float VlistScrollPosition(Widget);
extern float VlistVisibleLength(Widget, unsigned int);
extern int   VlistMaxEntriesVisible (Widget, int);

#endif /* _Vlist_h_ */
