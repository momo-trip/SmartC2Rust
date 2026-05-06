/*
**
** misc.h
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
**
*/

#ifndef	_GV_MISC_H_
#define	_GV_MISC_H_

extern void    misc_drawEyeGuide (Widget, int, int, int);
extern void    misc_savePagePosition (void);
extern int     misc_restorePagePosition (int *, int *);
extern void    misc_resetPagePosition (void);
extern void    misc_setPageMarker (int /* entry */,
                                   int /* kind */,
                                   XEvent*, Boolean);
extern String  misc_testFile (String);
extern String  misc_changeFile (String);
extern String  close_file (FILE *, String);
extern int     check_file (int);
extern void    show_page (int, XtPointer);
extern Boolean setup_ghostview (void);
extern void    setup_layout_ghostview (void);
extern void    misc_buildPagemediaMenu (void);
extern int     catch_Xerror (Display*, XErrorEvent*);
extern char *  quote_filename (char*);
extern Widget  build_label_menu(Widget, String, String, Pixmap);

#endif	/* _GV_MISC_H_ */











