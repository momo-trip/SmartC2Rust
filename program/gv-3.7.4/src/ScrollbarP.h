/*
**
** ScrollbarP.h
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

#ifndef _ScrollbarP_h
#define _ScrollbarP_h

#include "paths.h"
#include INC_XAW(ThreeDP.h)
#include "Scrollbar.h"

typedef struct {
     /* public */
    Pixel		foreground;		/* thumb foreground color */
    XtOrientation	orientation;		/* horizontal or vertical */
    XtCallbackList	scrollProc;		/* proportional scroll */
    XtCallbackList	thumbProc;		/* jump (to position) scroll */
    XtCallbackList	jumpProc;		/* same as thumbProc but pass data by ref */
    Pixmap		thumb;			/* thumb color */
    float		top;			/* relative position of top of thumb  */
    float		shown;			/* What percent is shown in the win */
    Dimension		length;			/* either height or width */
    Dimension		thickness;		/* either width or height */
    Dimension		min_thumb;		/* minium size for the thumb. */
    Dimension		thumb_top_indent;
    Dimension		thumb_side_indent;
    Boolean		show_arrows;		/* if the arrows are shown */
    Dimension		arrow_top_indent; 	/* arrow indentation top <-> border */
    Dimension		arrow_side_indent;    	/* arrow indentation side <-> border */
    int			interval;		/* time base for autoscroll (milliseconds) */
    int			delay;			/* time base for autoscroll (milliseconds) */
    Boolean		use_dynamic;		/* if the dynamic arrow scrolling is used */
    /* private */
    Dimension		dynamic;		/* dynamic autoscroll time interval */ 
    XtIntervalId	timer_id;		/* autorepeat timer; remove on destruction */
    char		scroll_mode;		/* 0:none 1:up/back 2:track 3:down/forward */
    GC			gc;			/* a (shared) gc */
    Dimension		topLoc;			/* position of top of thumb */
    Dimension		shownLength;		/* shown length of thumb    */
    Dimension		shadow_base;            /* the shadow width used to derive arrow_shadow, etc.    */
    int			arrow_width;
    int			arrow_height;
    int			arrow_top_to_border;
    int			arrow_to_border;
    int			arrow_shadow;
    int			thumb_shadow;
    int			thumb_to_arrow;
    int			thumb_to_border;
} ScrollbarPart;

typedef struct _ScrollbarRec {
    CorePart		core;
    SimplePart		simple;
    ThreeDPart		threeD;
    ScrollbarPart	scrollbar;
} ScrollbarRec;

typedef struct {int empty;} ScrollbarClassPart;

typedef struct _ScrollbarClassRec {
    CoreClassPart	core_class;
    SimpleClassPart	simple_class;
    ThreeDClassPart	threeD_class;
    ScrollbarClassPart	scrollbar_class;
} ScrollbarClassRec;


#define _EXTERN_ extern

_EXTERN_ ScrollbarClassRec scrollbarClassRec;

#undef _EXTERN_

#endif /* _ScrollbarP_h */
