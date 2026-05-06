/* 
 * FrameP.h - Private definitions for Frame widget
 * 
 * Author:	Vladimir Romanovski 
 *
 * Date:	Mon Feb 27, 1995
 */

/***********************************************************************
 *
 * Frame Widget Private Data
 *
 ***********************************************************************/

#ifndef _XawFrameP_h
#define _XawFrameP_h

#include INC_X11(ConstrainP.h)
#include "Frame.h"


/* New fields for the Frame widget class record */

typedef struct _FrameClassPart{
  XtPointer       dummy;
} FrameClassPart;

typedef struct _FrameClassRec {
  CoreClassPart        core_class;
  CompositeClassPart   composite_class;
  FrameClassPart       frame_class;
} FrameClassRec;

/* New fields for the frame  widget. */

typedef struct _FramePart {
  Dimension     h_space_nat;
  Dimension     v_space_nat;
  Dimension	shadow_width_nat;
  XawFrameType  frame_type;
  Pixel		top_shadow_pixel;
  Pixel		bot_shadow_pixel;
  Boolean	resize;
  /* Private resources. */
  GC		top_shadow_GC;
  GC		bot_shadow_GC;
  Dimension     h_space;
  Dimension     v_space;
  Dimension	shadow_width;
  Dimension     child_width_nat;
  Dimension     child_height_nat;
  Dimension     child_border_nat; 
} FramePart;

typedef  struct _FrameRec{
  CorePart 	 core;
  CompositePart  composite;
  FramePart	 frame;
} FrameRec;

extern FrameClassRec frameClassRec;

#endif  /* _XawFrameP_h */
