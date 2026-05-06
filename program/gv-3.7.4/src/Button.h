/*
**
** Button.h
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
*/

#ifndef _Button_h_
#define _Button_h_

#include "paths.h"
#include INC_XAW(Command.h)
#include "Frame.h"

#define XtNsetFrameStyle "setFrameStyle"
#define XtCSetFrameStyle "SetFrameStyle"
#define XtNunsetFrameStyle "unsetFrameStyle"
#define XtCUnsetFrameStyle "UnsetFrameStyle"
#define XtNhighlightedFrameStyle "highlightedFrameStyle"
#define XtCHighlightedFrameStyle "HighlightedFrameStyle"
#define XtNsetShadowWidth "setShadowWidth"
#define XtNunsetShadowWidth "unsetShadowWidth"
#define XtCButtonShadowWidth "ButtonShadowWidth"
#define XtNhighlightedBackground "highlightedBackground"
#define XtCHighlightedBackground "HighlightedBackground"
#define XtNsetBackground "setBackground"
#define XtCSetBackground "SetBackground"
#define XtNunsetBackground "unsetBackground"
#define XtCUnsetBackground "UnsetBackground"

extern WidgetClass buttonWidgetClass;

typedef struct _ButtonClassRec   *ButtonWidgetClass;
typedef struct _ButtonRec        *ButtonWidget;

extern void ButtonSet       (Widget, XEvent *, String *, Cardinal *);
extern void ButtonUnset     (Widget, XEvent *, String *, Cardinal *);
extern void ButtonReset     (Widget, XEvent *, String *, Cardinal *);
extern void ButtonHighlight (Widget, XEvent *, String *, Cardinal *);

#endif /* _Button_h_ */

