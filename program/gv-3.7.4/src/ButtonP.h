/*
**
** ButtonP.h
**
** Copyright (C) 1995, 1996, 1997 Johannes Plass
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

#ifndef _ButtonP_h
#define _ButtonP_h

#include "paths.h"
#include "Button.h"
#include INC_XAW(CommandP.h)
#include "Frame.h"

typedef struct _ButtonClass {
  int makes_compiler_happy;  /* not used */
} ButtonClassPart;

typedef struct _ButtonClassRec {
  CoreClassPart    core_class;
  SimpleClassPart  simple_class;
  ThreeDClassPart  threeD_class;
  LabelClassPart   label_class;
  CommandClassPart command_class;
  ButtonClassPart  button_class;
} ButtonClassRec;

extern ButtonClassRec buttonClassRec;

typedef struct {
  /* resources */
  XawFrameType set_frame_style;
  XawFrameType unset_frame_style;
  XawFrameType highlighted_frame_style;
  Pixel        set_background;
  Pixel        highlighted_background;
  /* private state */
  GC set_background_GC;
  GC highlighted_background_GC;
  int highlighted;
} ButtonPart;

typedef struct _ButtonRec {
  CorePart       core;
  SimplePart     simple;
  ThreeDPart     threeD;
  LabelPart      label;
  CommandPart    command;
  ButtonPart        button;
} ButtonRec;

#endif /* _TocP_h */


