/*
**
** confirm.h
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
** Author:   Johannes Plass (plass@dipmza.physik.uni-mainz.de)
**           Department of Physics
**           Johannes Gutenberg-University
**           Mainz, Germany
**
**           Jose E. Marchesi (jemarch@gnu.org)
**           GNU Project
**
*/


#ifndef	_GV_CONFIRM_H_
#define	_GV_CONFIRM_H_

#define CONFIRM_BUTTON_DONE   (1<<0)
#define CONFIRM_BUTTON_CANCEL (1<<1)

extern void cb_popdownConfirmPopup          (Widget, XtPointer, XtPointer);
extern void cb_popupConfirmPopup            (Widget, XtPointer, XtPointer);
extern void action_preferConfirmPopupButton (Widget, XEvent *, String *,
                                             Cardinal *);
extern void makeConfirmPopup                (void);
extern void ConfirmPopupSetButton           (int, XtCallbackProc);
extern void ConfirmPopupSetMessage          (String, String);
extern void ConfirmPopupSetInitialButton    (int);

#endif	/* _GV_CONFIRM_H_ */

