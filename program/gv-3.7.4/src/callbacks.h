/*
**
** callbacks.h
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

#ifndef	_GV_CALLBACKS_H_
#define	_GV_CALLBACKS_H_

extern void cb_showTitle           (Widget, XtPointer, XtPointer);
extern void cb_newtocScrollbar     (Widget, XtPointer, XtPointer);
extern void cb_newtocVisibleAdjust (Widget, XtPointer, XtPointer);
extern void cb_adjustSlider        (Widget, XtPointer, XtPointer);
extern void cb_antialias           (Widget, XtPointer, XtPointer);
extern void cb_useBackingPixmap    (Widget, XtPointer, XtPointer);
extern void cb_handleDSC           (Widget, XtPointer, XtPointer);
extern void cb_handleEOF           (Widget, XtPointer, XtPointer);
extern void cb_stopInterpreter     (Widget, XtPointer, XtPointer);
extern void cb_pageAdjustNotify    (Widget, XtPointer, XtPointer);
extern void cb_checkFile           (Widget, XtPointer, XtPointer);
extern void cb_watchFile           (Widget, XtPointer, XtPointer);
extern void cb_print               (Widget, XtPointer, XtPointer);
extern void cb_print_pos           (Widget, XtPointer, XtPointer);
extern void cb_doPrint             (Widget, XtPointer, XtPointer);
extern void cb_doPrintPos          (Widget, XtPointer, XtPointer);
extern void cb_cancelPrint         (Widget, XtPointer, XtPointer);
extern void cb_save                (Widget, XtPointer, XtPointer);
extern void cb_doSave              (Widget, XtPointer, XtPointer);
extern void cb_openFile            (Widget, XtPointer, XtPointer);
extern void cb_doOpenFile          (Widget, XtPointer, XtPointer);
extern void cb_reopen              (Widget, XtPointer, XtPointer);
extern void cb_savepos             (Widget, XtPointer, XtPointer);
extern void cb_presentation        (Widget, XtPointer, XtPointer);
extern void cb_redisplay           (Widget, XtPointer, XtPointer);
extern void cb_page                (Widget, XtPointer, XtPointer);
extern void cb_positionPage        (Widget, XtPointer, XtPointer);
extern void cb_setPageMark         (Widget, XtPointer, XtPointer);
extern void cb_autoResize          (Widget, XtPointer, XtPointer);
extern void cb_setScale            (Widget, XtPointer, XtPointer);
extern void cb_setOrientation      (Widget, XtPointer, XtPointer);
extern void cb_setPagemedia        (Widget, XtPointer, XtPointer);
extern void cb_track               (Widget, XtPointer, XtPointer);
extern void cb_message             (Widget, XtPointer, XtPointer);
extern void cb_destroy             (Widget, XtPointer, XtPointer);
extern void cb_destroyGhost        (Widget, XtPointer, XtPointer);
extern void cb_quitGhostview       (Widget, XtPointer, XtPointer);
extern void cb_doQuit              (Widget, XtPointer, XtPointer);
extern void cb_cancelQuit          (Widget, XtPointer, XtPointer);
extern void cb_askPassword         (Widget, XtPointer, XtPointer);

#endif /* _GV_CALLBACKS_H_ */
