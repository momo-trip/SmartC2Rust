/*
**
** misc_private.c
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
#include "ac_config.h"

/*
#define MESSAGES
*/
#include "message.h"

#include <stdlib.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(Cardinals.h)
#include INC_X11(IntrinsicP.h)
#include INC_XAW(LabelP.h)
#include "GhostviewP.h"

#include "types.h"
#include "misc_private.h"
#include "VlistP.h"

/*############################################################*/
/* update_label */
/*############################################################*/

void
update_label(widget,text)
   Widget widget;
   char   *text;
{
   LabelWidget w = (LabelWidget) widget;
   VlistWidget vw = (VlistWidget) widget;
   int shadow;

   BEGINMESSAGE1(update_label)

   if (!XtIsRealized(widget)) {
      INFMESSAGE(not realized) ENDMESSAGE(update_label)
      return;
   }

   shadow = w->threeD.shadow_width;
   if ((w->core.width > shadow+shadow) && (w->core.height > shadow+shadow)) {
      XClearArea(XtDisplay(widget),XtWindow(widget),
                 shadow,shadow,
                 (Dimension)(w->core.width-shadow-shadow),
                 (Dimension)(w->core.height-shadow-shadow), (text) ? False : True);
   }
   if (text) {      /* most of the following comes from X11/Xaw/Label.c */
      Position x,y;
      INFSMESSAGE(update_label,text)
#ifdef HAVE_XAW3D_INTERNATIONAL
      if( vw->simple.international == True ) {
	y = w->label.label_y - XExtentsOfFontSet(w->label.fontset)->max_logical_extent.y;
      } else {
#endif
	y = w->label.label_y + w->label.font->max_bounds.ascent;
#ifdef HAVE_XAW3D_INTERNATIONAL
      }
#endif
      if (w->label.justify == XtJustifyCenter) {
         unsigned int width;
         int len = (int) strlen(text);
#ifdef HAVE_XAW3D_INTERNATIONAL
	 if( vw->simple.international == True ) {
	   XFontSet     fs = w->label.fontset;
	   width = XmbTextEscapement(fs, text, (int)len );
	 } else {
#endif
	   XFontStruct *fs = w->label.font;
	   if   (w->label.encoding) width = XTextWidth16 (fs, (XChar2b*)text, (int)(len/2) );
	   else                     width = XTextWidth   (fs, text          , (int)(len)   );
#ifdef HAVE_XAW3D_INTERNATIONAL
	 }
#endif
         x = (Position) ((w->core.width-width)/2);
      } else {
         x = w->label.internal_width + w->threeD.shadow_width;
      }

#ifdef HAVE_XAW3D_INTERNATIONAL
      if( vw->simple.international == True ) {
	XmbDrawString(XtDisplay(widget), XtWindow(widget),
		      w->label.fontset, w->label.normal_GC,
		      x, y, text, (int)(strlen(text)));
      } else {
#endif
	if (w->label.encoding) {
	   XDrawString16(XtDisplay(widget), XtWindow(widget),
                         w->label.normal_GC,
	  	         x, y,(XChar2b*)text, (int)(strlen(text)));
	} else {
           XDrawString(XtDisplay(widget), XtWindow(widget),
                       w->label.normal_GC,
	  	       x, y, text, (int)(strlen(text)));
	}
#ifdef HAVE_XAW3D_INTERNATIONAL
      }
#endif
   }

   ENDMESSAGE1(update_label)
}

