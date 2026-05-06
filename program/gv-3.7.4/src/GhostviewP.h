/*
 * ghostviewp.h -- Private header file for Ghostview widget.
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
 * Boston, MA 02111-1307, USA.
 *
 *   Author: Tim Theisen           Systems Programmer
 * Internet: tim@cs.wisc.edu       Department of Computer Sciences
 *     UUCP: uwvax!tim             University of Wisconsin-Madison
 *    Phone: (608)262-0438         1210 West Dayton Street
 *      FAX: (608)262-9777         Madison, WI   53706
*/

#ifndef _GhostviewP_h
#define _GhostviewP_h

#include "paths.h"
#include "Ghostview.h"
#include INC_XMU(Atoms.h)
#include INC_XMU(CharSet.h)
#include <stdio.h>

typedef struct {
    AtomPtr ghostview;
    AtomPtr gv_colors;
    AtomPtr next;
    AtomPtr page;
    AtomPtr done;
} GhostviewClassPart;

typedef struct _GhostviewClassRec {
    CoreClassPart	core_class;
    GhostviewClassPart	ghostview_class;
} GhostviewClassRec;

extern GhostviewClassRec ghostviewClassRec;

/* structure to describe section of file to send to ghostscript */
struct record_list {
    FILE *fp;
    gv_off_t begin;
    gv_off_t len;
    Boolean seek_needed;
    Boolean close;
    struct record_list *next;
};

typedef struct {
    /* resources */
    Pixel		foreground;
    Cursor		cursor;
    Cursor		busy_cursor;
    Cursor		scroll_cursor;
    int			cursor_type;
    XtCallbackList	callback;
    XtCallbackList	message_callback;
    XtCallbackList	output_callback;
    String		interpreter;
    Boolean		quiet;
    int                 infoVerbose;
    Boolean		safeDir;
    int                 xinerama;
    Boolean		safer;
    Boolean		use_bpixmap;
    String		arguments;
    String		filename;
    XtPageOrientation	orientation;
    XtPalette		palette;
    float		xdpi;
    float		ydpi;
    long		lxdpi; /* use lxdi,lydpi for setting resolution resource */
    long		lydpi; /* and propagate it to xdpi,ydpi in "SetValues". */
    int			llx;
    int			lly;
    int			urx;
    int			ury;
    int			left_margin;
    int			bottom_margin;
    int			right_margin;
    int			top_margin;
#if 0
    Pixel		highlight_pixel;
#endif
    /* private state */
    GC                  highlight_gc;
    GC			gc;		/* GC used to clear window */
    Window		mwin;		/* destination of ghostsript messages */
    Boolean		disable_start;	/* whether to fork ghostscript */
    int			interpreter_pid;/* pid of ghostscript, -1 if none */
    struct record_list	*ps_input;	/* pointer it gs input queue */
    char		*input_buffer;	/* pointer to input buffer */
    gv_off_t		bytes_left;	/* bytes left in section */
    char		*input_buffer_ptr; /* pointer into input buffer */
    unsigned int	buffer_bytes_left; /* bytes left in buffer */
    int			interpreter_input; /* fd gs stdin, -1 if None */
    int			interpreter_output; /* fd gs stdout, -1 if None */
    int			interpreter_error; /* fd gs stderr, -1 if None */
    XtInputId		interpreter_input_id; /* XtInputId for above */
    XtInputId		interpreter_output_id; /* XtInputId for above */
    XtInputId		interpreter_error_id; /* XtInputId for above */
    Dimension		gs_width;	/* Width of window at last Setup() */
    Dimension		gs_height;	/* Height of window at last Setup() */
    Boolean		busy;		/* Is gs busy drawing? */
    Boolean		changed;	/* something changed since Setup()? */
    Dimension		pref_width; /*#test#*/
    Dimension		pref_height;
    int                 background_cleared;
} GhostviewPart;

typedef struct _GhostviewRec {
    CorePart		core;
    GhostviewPart	ghostview;
} GhostviewRec;

#endif /* _GhostviewP_h */
