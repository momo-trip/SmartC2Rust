/*
**
** main_resources.h
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

#ifndef _GV_MAIN_RESOURCES_H_ 
#define _GV_MAIN_RESOURCES_H_

/* Application resources */

typedef struct _AppResources {
    Boolean	auto_center;		/* whether to automatically center the page */
    int		scale;		        /* default scale */
    int		scale_base;		/* default scale base */
    String      default_orientation;	/* default orientation */
    String      fallback_orientation;	/* fallback orientation */
    String	page;			/* first page to show */
    String	default_pagemedia;	/* default page media */
    String	fallback_pagemedia;	/* fallback page media */
    Boolean	swap_landscape;		/* Landscape comment maps to Seascape */
    String	scratch_dir;		/* temporary directory */
    String	default_save_dir;	/* default directory for saving */
    Boolean	confirm_print;		/* popup dialog on print attempt */
    String	version;		/* ghostview version identifier*/
    String	maximum_width;		/* maximum width of the application*/
    String	maximum_height;		/* maximum height of the application*/
    int		minimum_width;		/* minimum width of the application*/
    int		minimum_height;		/* minimum height of the application*/
    Boolean	auto_resize;		/* should we try to fit the window size to the page size */
    Pixmap	document_bitmap;
    Pixmap	selected_bitmap;
    Pixmap	mark_odd_bitmap;
    Pixmap	mark_even_bitmap;
    Pixmap	mark_current_bitmap;
    Pixmap	mark_unmark_bitmap;
    Pixmap	mark_empty_bitmap;
    String	locator_format;
    Boolean	antialias;
    String	mag_menu;
    Boolean	reverse_scrolling;
    Boolean	scrolling_eye_guide;
    Boolean	respect_dsc;
    int		confirm_quit;
    Boolean	ignore_eof;
    Boolean	watch_file;
    int         watch_file_frequency;
    String      style;
    String      ad;
    String      misc_menu;
    int         title_style;
    Boolean     filename_as_title;
    Boolean     use_bpixmap;
} AppResources;

#ifdef _GV_MAIN_C_
#  define DECLARE_STRING(aaa) \
      static char n_ ## aaa[] = #aaa; \
      char * s_ ## aaa = n_ ## aaa;
#else
#  define DECLARE_STRING(aaa) \
      extern char * s_ ## aaa;
#endif

DECLARE_STRING(miscMenuEntries)
DECLARE_STRING(medias)
DECLARE_STRING(scales)
DECLARE_STRING(printCommand)
DECLARE_STRING(uncompressCommand)

DECLARE_STRING(gsInterpreter)
DECLARE_STRING(gsCmdScanPDF)
DECLARE_STRING(gsCmdConvPDF)
DECLARE_STRING(gsX11Device)
DECLARE_STRING(gsX11AlphaDevice)
DECLARE_STRING(gsArguments)
DECLARE_STRING(gsSafeDir)
DECLARE_STRING(gsSafer)
DECLARE_STRING(gsQuiet)

DECLARE_STRING(infoVerbose)
DECLARE_STRING(autoCenter)
DECLARE_STRING(scale)
DECLARE_STRING(scaleBase)
DECLARE_STRING(orientation)
DECLARE_STRING(fallbackOrientation)
DECLARE_STRING(page)
DECLARE_STRING(pageMedia)
DECLARE_STRING(fallbackPageMedia)
DECLARE_STRING(swapLandscape)
DECLARE_STRING(scratchDir)
DECLARE_STRING(defaultSaveDir)
DECLARE_STRING(confirmPrint)
DECLARE_STRING(version)
DECLARE_STRING(autoResize)
DECLARE_STRING(maximumWidth)
DECLARE_STRING(maximumHeight)
DECLARE_STRING(minimumWidth)
DECLARE_STRING(minimumHeight)
DECLARE_STRING(selectedBitmap)
DECLARE_STRING(documentBitmap)
DECLARE_STRING(toggleOddBitmap)
DECLARE_STRING(toggleEvenBitmap)
DECLARE_STRING(toggleCurrentBitmap)
DECLARE_STRING(unmarkAllBitmap)
DECLARE_STRING(markEmptyBitmap)
DECLARE_STRING(locatorFormat)
DECLARE_STRING(antialias)
DECLARE_STRING(magMenu)
DECLARE_STRING(reverseScrolling)
DECLARE_STRING(scrollingEyeGuide)
DECLARE_STRING(respectDSC)
DECLARE_STRING(confirmQuit)
DECLARE_STRING(ignoreEOF)
DECLARE_STRING(watchFile)
DECLARE_STRING(watchFileFrequency)
DECLARE_STRING(style)
DECLARE_STRING(ad)
DECLARE_STRING(miscMenu)
DECLARE_STRING(titleStyle)
DECLARE_STRING(useBackingPixmap)

/*-------------------------------------------------------------*/

#ifdef _GV_MAIN_C_

#define GV_DEFAULT_VERSION "?"

static XtResource resources[] = {
  {n_autoCenter,n_autoCenter, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, auto_center), XtRImmediate, (XtPointer)True},
  {n_scale, n_scale, XtRInt, sizeof(int),
   XtOffsetOf(AppResources, scale), XtRImmediate, (XtPointer)0},
  {n_scaleBase, n_scaleBase, XtRInt, sizeof(int),
   XtOffsetOf(AppResources, scale_base), XtRImmediate, (XtPointer)1},
  {n_orientation, n_orientation, XtRString, sizeof(String),
   XtOffsetOf(AppResources, default_orientation), XtRImmediate, "Automatic"},
  {n_fallbackOrientation, n_fallbackOrientation, XtRString, sizeof(String),
   XtOffsetOf(AppResources, fallback_orientation), XtRImmediate, "Portrait"},
  {n_page, n_page, XtRString, sizeof(String),
   XtOffsetOf(AppResources, page), XtRImmediate, NULL},
  {n_pageMedia, n_pageMedia, XtRString, sizeof(String),
   XtOffsetOf(AppResources, default_pagemedia), XtRImmediate, "Automatic"},
  {n_fallbackPageMedia, n_fallbackPageMedia, XtRString, sizeof(String),
   XtOffsetOf(AppResources, fallback_pagemedia), XtRImmediate, "A4"},
  {n_swapLandscape, n_swapLandscape, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, swap_landscape), XtRImmediate, (XtPointer)False},
  {n_scratchDir, n_scratchDir, XtRString, sizeof(String),
   XtOffsetOf(AppResources, scratch_dir), XtRImmediate,"?"},
  {n_defaultSaveDir, n_defaultSaveDir, XtRString, sizeof(String),
   XtOffsetOf(AppResources, default_save_dir), XtRImmediate,"?"},
  {n_confirmPrint, n_confirmPrint, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, confirm_print), XtRImmediate, (XtPointer)True},
  {n_version, n_version, XtRString, sizeof(String),
   XtOffsetOf(AppResources, version), XtRImmediate,GV_DEFAULT_VERSION},
  {n_autoResize, n_autoResize, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, auto_resize), XtRImmediate, (XtPointer)True},
  {n_maximumWidth, n_maximumWidth, XtRString, sizeof(String),
   XtOffsetOf(AppResources, maximum_width), XtRImmediate, (XtPointer)"screen-20"},
  {n_maximumHeight, n_maximumHeight, XtRString, sizeof(String),
   XtOffsetOf(AppResources, maximum_height), XtRImmediate, (XtPointer)"screen-44"},
  {n_minimumWidth, n_minimumWidth, XtRInt, sizeof(int),
   XtOffsetOf(AppResources, minimum_width), XtRImmediate, (XtPointer)400},
  {n_minimumHeight, n_minimumHeight, XtRInt, sizeof(int),
   XtOffsetOf(AppResources, minimum_height), XtRImmediate, (XtPointer)400},
  {n_selectedBitmap, n_selectedBitmap, XtRBitmap, sizeof(Pixmap),
   XtOffsetOf(AppResources,selected_bitmap), XtRImmediate, (XtPointer)None},
  {n_documentBitmap, n_documentBitmap, XtRBitmap, sizeof(Pixmap),
   XtOffsetOf(AppResources,document_bitmap), XtRImmediate, (XtPointer)None},
  {n_toggleOddBitmap, n_toggleOddBitmap, XtRBitmap, sizeof(Pixmap),
   XtOffsetOf(AppResources,mark_odd_bitmap), XtRImmediate, (XtPointer)None},
  {n_toggleEvenBitmap, n_toggleEvenBitmap, XtRBitmap, sizeof(Pixmap),
   XtOffsetOf(AppResources,mark_even_bitmap), XtRImmediate, (XtPointer)None},
  {n_toggleCurrentBitmap, n_toggleCurrentBitmap, XtRBitmap, sizeof(Pixmap),
   XtOffsetOf(AppResources,mark_current_bitmap), XtRImmediate, (XtPointer)None},
  {n_unmarkAllBitmap, n_unmarkAllBitmap, XtRBitmap, sizeof(Pixmap),
   XtOffsetOf(AppResources,mark_unmark_bitmap), XtRImmediate, (XtPointer)None},
  {n_markEmptyBitmap, n_markEmptyBitmap, XtRBitmap, sizeof(Pixmap),
   XtOffsetOf(AppResources,mark_empty_bitmap), XtRImmediate, (XtPointer)None},
  {n_locatorFormat, n_locatorFormat, XtRString, sizeof(String),
   XtOffsetOf(AppResources,locator_format),   XtRImmediate, (XtPointer)"%d x %d"},
  {n_antialias, n_antialias, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, antialias), XtRImmediate, (XtPointer)False},
  {n_magMenu, n_magMenu, XtRString, sizeof(String),
   XtOffsetOf(AppResources,mag_menu), XtRImmediate, (XtPointer)NULL},
  {n_reverseScrolling, n_reverseScrolling, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, reverse_scrolling), XtRImmediate, (XtPointer)False},
  {n_scrollingEyeGuide, n_scrollingEyeGuide, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, scrolling_eye_guide), XtRImmediate, (XtPointer)True},
  {n_respectDSC, n_respectDSC, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, respect_dsc), XtRImmediate, (XtPointer)True},
  {n_confirmQuit, n_confirmQuit, XtRInt, sizeof(int),
   XtOffsetOf(AppResources, confirm_quit), XtRImmediate, (XtPointer)1},  /* 0=never,1=when processing,2=always */
  {n_ignoreEOF, n_ignoreEOF, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources,  ignore_eof), XtRImmediate, (XtPointer)True},
  {n_watchFile, n_watchFile, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources,  watch_file), XtRImmediate, (XtPointer)False},
  {n_watchFileFrequency, n_watchFileFrequency, XtRInt, sizeof(int),
   XtOffsetOf(AppResources, watch_file_frequency), XtRImmediate, (XtPointer)1000},
  {n_style, n_style, XtRString, sizeof(String),
   XtOffsetOf(AppResources, style), XtRImmediate,(XtPointer)""},
  {n_ad, n_ad, XtRString, sizeof(String),
   XtOffsetOf(AppResources, ad), XtRImmediate,(XtPointer)""},
  {n_miscMenu, n_miscMenu, XtRString, sizeof(String),
   XtOffsetOf(AppResources, misc_menu), XtRImmediate,"update"},
  {n_titleStyle, n_titleStyle, XtRInt, sizeof(int),
   XtOffsetOf(AppResources, title_style), XtRImmediate, (XtPointer)1},
  {n_useBackingPixmap,n_useBackingPixmap, XtRBoolean, sizeof(Boolean),
   XtOffsetOf(AppResources, use_bpixmap), XtRImmediate, (XtPointer)True},
};

#endif /* _GV_MAIN_C_ */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/

#endif /* _GV_MAIN_RESOURCES_H_ */
