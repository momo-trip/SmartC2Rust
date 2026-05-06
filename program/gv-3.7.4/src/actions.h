/*
**
** actions.h
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

#ifndef	_GV_ACTIONS_H_
#define	_GV_ACTIONS_H_

extern void action_shellConfigureNotify (Widget,XEvent*,String*,Cardinal*);
extern void action_page           (Widget, XEvent *, String *, Cardinal *);
extern void action_menuPopdown    (Widget, XEvent *, String *, Cardinal *);
extern void action_toc            (Widget, XEvent *, String *, Cardinal *);
extern void action_otherPage      (Widget, XEvent *, String *, Cardinal *);
extern void action_movePage       (Widget, XEvent *, String *, Cardinal *);
extern void action_panner         (Widget, XEvent *, String *, Cardinal *);
extern void action_antialias      (Widget, XEvent *, String *, Cardinal *);
extern void action_quit           (Widget, XEvent *, String *, Cardinal *);
extern void action_handleDSC      (Widget, XEvent *, String *, Cardinal *);
extern void action_handleEOF      (Widget, XEvent *, String *, Cardinal *);
extern void action_open           (Widget, XEvent *, String *, Cardinal *);
extern void action_reopen         (Widget, XEvent *, String *, Cardinal *);
extern void action_savepos        (Widget, XEvent *, String *, Cardinal *);
extern void action_presentation   (Widget, XEvent *, String *, Cardinal *);
extern void action_save           (Widget, XEvent *, String *, Cardinal *);
extern void action_print          (Widget, XEvent *, String *, Cardinal *);
extern void action_print_pos      (Widget, XEvent *, String *, Cardinal *);
extern void action_center         (Widget, XEvent *, String *, Cardinal *);
extern void action_setPageMark    (Widget, XEvent *, String *, Cardinal *);
extern void action_autoResize     (Widget, XEvent *, String *, Cardinal *);
extern void action_setScale       (Widget, XEvent *, String *, Cardinal *);
extern void action_setOrientation (Widget, XEvent *, String *, Cardinal *);
extern void action_setPagemedia   (Widget, XEvent *, String *, Cardinal *);
extern void action_dismissPopup   (Widget, XEvent *, String *, Cardinal *);
extern void action_deleteWindow   (Widget, XEvent *, String *, Cardinal *);
extern void action_eraseLocator   (Widget, XEvent *, String *, Cardinal *);
extern void action_checkFile      (Widget, XEvent *, String *, Cardinal *);
extern void action_watchFile      (Widget, XEvent *, String *, Cardinal *);
extern void clean_safe_tempdir    (void);

#endif	/* _GV_ACTIONS_H_ */
