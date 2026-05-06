/*
**
** FileSel.h
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


#ifndef _XawFileSelection_h
#define _XawFileSelection_h

#include INC_X11(Xfuncproto.h)

#define XawFileSelectionRescan 	        (1<<0)
#define XawFileSelectionFilter 	        (1<<1)
#define XawFileSelectionDefaultScan 	(1<<2)
#define XawFileSelectionDone 		(1<<3)
#define XawFileSelectionCancel 	        (1<<4)

/* New Fields */

#define XtNreverseScrolling "reverseScrolling"
#define XtCReverseScrolling "ReverseScrolling"

#define XtNtmpDir "tmpDir"
#define XtCTmpDir "TmpDir"

#define XtNpath "path"
#define XtCPath "Path"

#define XtNfilter "filter"
#define XtCFilter "Filter"

#define XtNfilters "filters"
#define XtCFilters "Filters"

#define XtNdirs "dirs"
#define XtCDirs "Dirs"

#define XtNhighlightPixel "highlightPixel"
#define XtCHighlightPixel "HighlightPixel"

#define XtNbuttons "buttons"
#define XtCButtons "Buttons"

#define XtNpreferredButton "preferredButton"
#define XtCPreferredButton "PreferredButton"

/* Class record constant */

extern WidgetClass file_selectionWidgetClass;

typedef struct _FileSelectionClassRec	*FileSelectionWidgetClass;
typedef struct _FileSelectionRec	*FileSelectionWidget;

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
   PUBLIC ROUTINES
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/   

_XFUNCPROTOBEGIN

extern void			XawFileSelectionSetPath (
    Widget		/* FileSelectionWidget */,
    String		/* String to set */
);

extern char *			XawFileSelectionGetPath (
    Widget		/* FileSelectionWidget */
);

extern void			XawFileSelectionScan (
    Widget		/* FileSelectionWidget */,
    int			/* indicates the scan mode */
);

extern void			XawFileSelectionAddButton (
   Widget              /* FileSelectionWidget */,
   int                 /* indicates the desired button position */,
   XtCallbackProc      /* callback for the button */,
   XtPointer           /* callback parameter */
);

extern void			XawFileSelectionRemoveButton (
   Widget               /* FileSelectionWidget */,
   int                  /* indicates the position of the button */
);

extern void			XawFileSelectionPreferButton (
   Widget               /* FileSelectionWidget */,
   int                  /* indicates the position of the button */
);

_XFUNCPROTOEND

#endif /* _FileSelection_h */
