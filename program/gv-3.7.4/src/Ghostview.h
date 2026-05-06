/*
 * Ghostview.h -- Public header file for Ghostview widget.
 * Copyright (C) 1992  Timothy O. Theisen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU gv; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. *
 *
 *   Author: Tim Theisen           Systems Programmer
 * Internet: tim@cs.wisc.edu       Department of Computer Sciences
 *     UUCP: uwvax!tim             University of Wisconsin-Madison
 *    Phone: (608)262-0438         1210 West Dayton Street
 *      FAX: (608)262-9777         Madison, WI   53706
 *
 *   Author: Jose E. Marchesi (jemarch@gnu.org)
 *           GNU Project
*/

#ifndef _Ghostview_h
#define _Ghostview_h
/* Be sure that FILE* is defined */
#include <stdio.h>

/****************************************************************
 *
 * Ghostview widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 arguments	     Arguments		String		NULL
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 bottomMargin	     Margin		int		0
 busyCursor	     Cursor		Cursor		watch
 callback	     Callback		Pointer		NULL
 cursor		     Cursor		Cursor		crosshair
 destroyCallback     Callback		Pointer		NULL
 filename	     Filename		String		"-"
 foreground	     Foreground 	Pixel		XtDefaultForeground
 height		     Height		Dimension	0
 infoVerbose         InfoVerbose        InfoVerbose     0
 interpreter	     Interpreter	String		"gs"
 leftMargin	     Margin		int		0
 llx		     BoundingBox	Int		0
 lly		     BoundingBox	Int		0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 messageCallback     Callback		Pointer		NULL
 orientation	     Orientation	PageOrientation	Portrait
 outputCallback      Callback		Pointer		NULL
 palette	     Palette		Palette		Color
 quiet		     Quiet		Boolean		True
 rightMargin	     Margin		int		0
 safeDir	     SafeDir		Boolean		True
 safer		     Safer		Boolean		True
 topMargin	     Margin		int		0
 urx		     BoundingBox	Int		612
 ury		     BoundingBox	Int		792
 useBackingPixmap    UseBackingPixmap	Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0

*/

/* define any special resource names here that are not in INC_X11(StringDefs.h) */

#define XtNbusyCursor "busyCursor"
#define XtNscrollCursor "scrollCursor"
#define XtNcursor "cursor"
#define XtNfilename "filename"
#define XtNmessageCallback "messageCallback"
#define XtNoutputCallback "outputCallback"
#define XtNpalette "palette"
#define XtNarguments "arguments"
#define XtNquiet "quiet"
#define XtNinfoVerbose "infoVerbose"
#define XtNxinerama "xinerama"
#define XtNllx "llx"
#define XtNlly "lly"
#define XtNurx "urx"
#define XtNury "ury"
#define XtNuseBackingPixmap "useBackingPixmap"
#define XtNlxdpi "lxdpi"
#define XtNlydpi "lydpi"
#define XtNrightMargin "rightMargin"
#define XtNleftMargin "leftMargin"
#define XtNbottomMargin "bottomMargin"
#define XtNtopMargin "topMargin"
#define XtNpreferredWidth  "preferredWidth"
#define XtNpreferredHeight "preferredHeight"
#define XtNsafeDir "safeDir"
#define XtNsafer "safer"
#define XtNinterpreter "interpreter"

#define XtCBoundingBox "BoundingBox"
#define XtCFilename "Filename"
#define XtCPalette "Palette"
#define XtCArguments "Arguments"
#define XtCQuiet "Quiet"
#define XtCinfoVerbose "infoVerbose"
#define XtCxinerama "Xinerama"
#define XtCLResolution "LResolution"
#define XtCUseBackingPixmap "UseBackingPixmap"
#define XtCPreferredWidth  "PreferredWidth"
#define XtCPreferredHeight "PreferredHeight"
#define XtCSafer "Safer"
#define XtCSafeDir "SafeDir"
#define XtCInterpreter "Interpreter"

#if 0
#define XtNhighlightPixel "highlightPixel"
#define XtCHighlightPixel "HighlightPixel"
#endif

/******************************************************************************
 * XmuCvtStringToPageOrientation
 */
/* Number represents clockwise rotation of the paper in degrees */
typedef enum {
    XtPageOrientationUnspecified =  -1,	/* Unspecified orientation */
    XtPageOrientationPortrait	 =   0,	/* Normal portrait orientation */
    XtPageOrientationLandscape	 =  90,	/* Normal landscape orientation */
    XtPageOrientationUpsideDown  = 180,	/* Don't think this will be used much */
    XtPageOrientationSeascape	 = 270	/* Landscape rotated the other way */
} XtPageOrientation;

#ifndef GV_CODE
#   define XtEportrait "portrait"
#   define XtElandscape "landscape"
#   define XtEupsideDown "upside-down"
#   define XtEseascape "seascape"
   extern Boolean XmuCvtStringToPageOrientation();
#endif /* GV_CODE */
#define XtRPageOrientation "PageOrientation"
#define XtRinfoVerbose "InfoVerbose"
#define XtRxinerama "xinerama"

/******************************************************************************
 * XmuCvtStringToPalette
 */
typedef enum {
    XtPaletteMonochrome,
    XtPaletteGrayscale,
    XtPaletteColor
} XtPalette;
#define XtEmonochrome "monochrome"
#define XtEgrayscale "grayscale"
#define XtEcolor "color"
#define XtRPalette "Palette"
extern Boolean XmuCvtStringToPalette(Display*,XrmValue*,Cardinal*,XrmValue*,XrmValue*,XtPointer*);

/* declare specific GhostviewWidget class and instance datatypes */

typedef struct _GhostviewClassRec*	GhostviewWidgetClass;
typedef struct _GhostviewRec*		GhostviewWidget;

/* declare the class constant */

extern WidgetClass ghostviewWidgetClass;

/*###################################################################################*/
/* Public routines */
/*###################################################################################*/

extern void			GhostviewDrawRectangle (
    Widget,
    int,
    int,
    int,
    int
);

/* The structure returned by the regular callback */

typedef struct _GhostviewReturnStruct {
    int width, height;
    int	psx, psy;
    float xdpi, ydpi;
} GhostviewReturnStruct;

extern void			GhostviewCoordsPStoX (
    Widget,
    int,
    int,
    int*,
    int*
);

extern void			GhostviewCoordsXtoPS (
    Widget,
    int,
    int,
    int*,
    int*
);

extern void			GhostviewGetBBofArea (
   Widget                 /* w  */ ,
   int                    /* x1 */ ,
   int                    /* y1 */ ,
   int                    /* x2 */ ,
   int                    /* y2 */ ,
   GhostviewReturnStruct* /* p  */
);

extern void			GhostviewGetAreaOfBB (
   Widget                /* w    */  ,
   int                   /* psx1 */  ,
   int                   /* psy1 */  ,
   int                   /* psx2 */  ,
   int                   /* psy2 */  ,
   GhostviewReturnStruct* p
);


extern void			GhostviewEnableInterpreter (
    Widget	/* w */
);

extern void			GhostviewDisableInterpreter (
    Widget	/* w */
);

extern void			GhostviewState (
    Widget	/* w */           ,
    Boolean* /* processflag_p */  ,
    Boolean* /* busyflag_p */     ,
    Boolean* /* inputflag_p */
);

extern Boolean			GhostviewIsInterpreterReady (
    Widget	/* w */
);

extern Boolean			GhostviewIsBusy (
    Widget	/* w */
);

extern Boolean			GhostviewIsInterpreterRunning (
    Widget	/* w */
);

extern Boolean			GhostviewSendPS (
    Widget	/* widget */,
    FILE*	/* fp */,
    gv_off_t	/* begin */,
    gv_off_t	/* len */,
    Bool	/* close */
);

extern Boolean			GhostviewNextPage (
    Widget	/* w */
);

extern void			GhostviewClearBackground (
    Widget	/* w */
);

extern void			GhostviewSetup (
    Widget	/* w */
);

#endif /* _Ghostview_h */




