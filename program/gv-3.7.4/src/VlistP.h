/*
**
** VlistP.h
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

#ifndef _VlistP_h
#define _VlistP_h

#include "paths.h"
#include "Vlist.h"
#include INC_XAW(LabelP.h)

typedef struct _VlistClass {
  int makes_compiler_happy;  /* not used */
} VlistClassPart;

typedef struct _VlistClassRec {
  CoreClassPart    core_class;
  SimpleClassPart  simple_class;
  ThreeDClassPart  threeD_class;
  LabelClassPart   label_class;
  VlistClassPart   vlist_class;
} VlistClassRec;

extern VlistClassRec vlistClassRec;

typedef struct {
  /* resources */
  String         vlist;
  String         highlighted_geometry;
  String         selected_geometry;
  Dimension      mark_shadow_width;
  Dimension      selected_shadow_width;
  Dimension      highlighted_shadow_width;
  Pixel          mark_background;
  Pixel          selected_background;
  Pixel          highlighted_background;
  Boolean        allow_marks;
  /* private state */
  int entries;
  GC background_GC;
  GC mark_background_GC;
  GC selected_background_GC;
  GC highlighted_background_GC;
  int firstVisible;
  int selected;
  int highlighted;
  int hulx;
  int huly;
  int hlrx;
  int hlry;
  int sulx;
  int suly;
  int slrx;
  int slry;
  int ydelta;
  int yofs;
  XtCallbackList report_callbacks;
} VlistPart;

typedef struct _VlistRec {
  CorePart       core;
  SimplePart     simple;
  ThreeDPart     threeD;
  LabelPart      label;
  VlistPart      vlist;
} VlistRec;

#endif /* _VlistP_h */


