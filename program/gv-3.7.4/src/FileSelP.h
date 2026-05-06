/*
**
** FileSelP.h
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

#ifndef _XawFileSelectionP_h
#define _XawFileSelectionP_h

#include "paths.h"
#include "AaaP.h"

#include <dirent.h>

#include "FileSel.h"

/*####################################################################
   Private Delarations
####################################################################*/

typedef struct {
	String	*entry;
	int	num_of_entries;
} FSDir;


typedef struct _FileSelectionClassPart {
    int foo;			/* keep compiler happy. */
} FileSelectionClassPart;


typedef struct {
    /* resources */
    Pixel	highlight_pixel;
    String      path;
    String      filter;
    String      filters;
    String      dirs;
    int         buttons;
    int		preferred_button;    
    Boolean     reverse_scrolling;
    /* private data */
    Widget	pathframeFS;
    Widget	pathFS;
    Widget	filterframeFS;
    Widget	filterFS;
    Widget	old_textfieldFS;
    Widget	rescanbuttonFS;
    Widget	filtersbuttonFS;
    Widget	filtersmenuFS;
    Widget	dirsmenuFS;
    Widget	dirsbuttonFS;
    Widget	panedFS;
    Widget	topscrollFS;
    Widget	topframeFS;
    Widget	topclipFS;
    Widget	topaaaFS;
    Widget	toplistFS;
    Widget	curscrollFS;
    Widget	curframeFS;
    Widget	curclipFS;
    Widget	curaaaFS;
    Widget	curlistFS;
    Widget	subscrollFS;
    Widget	subframeFS;
    Widget	subclipFS;
    Widget	subaaaFS;
    Widget	sublistFS;
    Widget	button1FS;
    Widget	button2FS;
    Widget	button3FS;
    Widget	button4FS;
    FSDir       topdir;
    int         topdir_alloc;
    FSDir       curdir;
    int         curdir_alloc;
    FSDir       subdir;
    int         subdir_alloc;
    String      app_dir;
    String      current_path;
    char        *current_dir;
    char        *current_file;
    String      tmp_dir;
    String      path_field_value;
    String      filter_field_value;    
    int         internal_buttons;    
    unsigned long multiclick;
    Pixel	old_highlight_pixel;
} FileSelectionPart;

typedef struct _FileSelectionClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    AaaClassPart        aaa_class;
    FileSelectionClassPart   file_selection_class;
} FileSelectionClassRec;

extern FileSelectionClassRec file_selectionClassRec;

/**************************************************************************
 *
 * Full instance record declaration
 *
 **************************************************************************/

typedef struct {int empty;} FileSelectionConstraintsPart;

typedef struct _FileSelectionConstraintsRec {
    SubInfoRec	aaa;
    FileSelectionConstraintsPart file_selection;
} FileSelectionConstraintsRec, *FileSelectionConstraints;

typedef struct _FileSelectionRec {
    CorePart       core;
    CompositePart  composite;
    ConstraintPart constraint;
    AaaPart        aaa;
    FileSelectionPart file_selection;
} FileSelectionRec;

#endif /* _XawFileSelectionP_h */
