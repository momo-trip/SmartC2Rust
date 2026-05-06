/*
 * Ghostview.c -- Ghostview widget.
 * Copyright (C) 1992  Timothy O. Theisen
 * Copyright (C) 2004  Jose E. Marchesi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU gv; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *   Author: Tim Theisen           Systems Programmer
 * Internet: tim@cs.wisc.edu       Department of Computer Sciences
 *     UUCP: uwvax!tim             University of Wisconsin-Madison
 *    Phone: (608)262-0438         1210 West Dayton Street
 *      FAX: (608)262-9777         Madison, WI   53706
*/

/*
 * this code was modified by Johannes Plass,
 * plass@thep.physik.uni-mainz.de
 * February 1995, March 1996 ###jp###
 *
*/
#include "ac_config.h"


/*
#define MESSAGES
*/
#include "message.h"

#include "config.h"

#include "paths.h"
#include INC_X11(IntrinsicP.h)
#include INC_X11(StringDefs.h)
#include INC_X11(Xatom.h)
#include INC_X11(Xproto.h)
#include INC_X11(Xos.h)
#include INC_X11(Xfuncs.h)
#include "GhostviewP.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include <ctype.h>
#include <stdlib.h>
#include <signal.h>

#include "types.h"
#include "main_resources.h"
#include "main_globals.h"
#include "misc.h"
#include "actions.h"

#include "resource.h"

#ifndef XlibSpecificationRelease
typedef char *XPointer;
#endif

#include <gv_signal.h>
#ifdef SIGNALRETURNSINT
#define SIGVAL int
#else
#define SIGVAL void
#endif

#ifdef NON_BLOCKING_IO
#include <fcntl.h>
/* if POSIX O_NONBLOCK is not available, use O_NDELAY */
#if !defined(O_NONBLOCK) && defined(O_NDELAY)
#define O_NONBLOCK O_NDELAY
#endif
#endif

#include <errno.h>
/* Both error returns are checked for non-blocking I/O. */
/* Manufacture the other error code if only one exists. */
#if !defined(EWOULDBLOCK) && defined(EAGAIN)
#define EWOULDBLOCK EAGAIN
#endif
#if !defined(EAGAIN) && defined(EWOULDBLOCK)
#define EAGAIN EWOULDBLOCK
#endif

/* GV_BUFSIZ is set to the minimum POSIX PIPE_BUF to ensure that
 * nonblocking writes to ghostscript will work properly.
 */
#define GV_BUFSIZ 512

#define ALLOW_PDF /*###jp### 02/29/96 */

#define CURSOR_RESET   1
#define CURSOR_BUSY    2
#define CURSOR_SCROLL  3
#define CURSOR_NORMAL  4

static XtResource resources[] = {
#define offset(field) XtOffsetOf(GhostviewRec, ghostview.field)
    { XtNarguments, XtCArguments, XtRString, sizeof(String),
	  offset(arguments), XtRString, (XtPointer)NULL },
    { XtNbottomMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(bottom_margin), XtRImmediate, (XtPointer)0 },
    { XtNbusyCursor, XtCCursor, XtRCursor, sizeof(XtPointer),
	  offset(busy_cursor), XtRString, "watch" },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(callback), XtRCallback, (XtPointer)NULL },
    { XtNcursor, XtCCursor, XtRCursor, sizeof(XtPointer),
	  offset(cursor), XtRString, "crosshair" },
    { XtNfilename, XtCFilename, XtRString, sizeof(String),
	  offset(filename), XtRString, (XtPointer)NULL },
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	  offset(foreground), XtRString, XtDefaultForeground},
    { XtNleftMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(left_margin), XtRImmediate, (XtPointer)0 },
    { XtNllx, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(llx), XtRImmediate, (XtPointer)0 },
    { XtNlly, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(lly), XtRImmediate, (XtPointer)0 },
    { XtNmessageCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(message_callback), XtRCallback, (XtPointer)NULL },
    { XtNorientation, XtCOrientation, XtRPageOrientation,
	  sizeof(XtPageOrientation), offset(orientation), XtRImmediate,
	  (XtPointer)XtPageOrientationPortrait },
    { XtNoutputCallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	  offset(output_callback), XtRCallback, (XtPointer)NULL },
    { XtNpalette, XtCPalette, XtRPalette, sizeof(XtPalette),
	  offset(palette), XtRImmediate, (XtPointer)XtPaletteColor },
    { XtNquiet, XtCQuiet, XtRBoolean, sizeof(Boolean),
	  offset(quiet), XtRImmediate, (XtPointer)True },
    { XtNinfoVerbose, XtCinfoVerbose, XtRinfoVerbose, sizeof(int),
	  offset(infoVerbose), XtRImmediate, (XtPointer)0 },
    { XtNxinerama, XtCxinerama, XtRxinerama, sizeof(int),
	  offset(xinerama), XtRImmediate, (XtPointer)0 },
    { XtNrightMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(right_margin), XtRImmediate, (XtPointer)0 },
    { XtNscrollCursor, XtCCursor, XtRCursor, sizeof(XtPointer),
	  offset(scroll_cursor), XtRString, "plus" },
    { XtNtopMargin, XtCMargin, XtRInt, sizeof(int),
	  offset(top_margin), XtRImmediate, (XtPointer)0 },
    { XtNuseBackingPixmap, XtCUseBackingPixmap, XtRBoolean, sizeof(Boolean),
	  offset(use_bpixmap), XtRImmediate, (XtPointer)False },
    { XtNurx, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(urx), XtRImmediate, (XtPointer)612 },
    { XtNury, XtCBoundingBox, XtRInt, sizeof(int),
	  offset(ury), XtRImmediate, (XtPointer)792 },
    { XtNlxdpi, XtCLResolution, XtRInt, sizeof(long),
	  offset(lxdpi), XtRImmediate, (XtPointer)0 },
    { XtNlydpi, XtCLResolution, XtRInt, sizeof(long),
	  offset(lydpi), XtRImmediate, (XtPointer)0 },
    {XtNpreferredWidth, XtCPreferredWidth, XtRDimension, sizeof(Dimension),
       offset(pref_width), XtRImmediate, (XtPointer)1},
    {XtNpreferredHeight, XtCPreferredHeight, XtRDimension, sizeof(Dimension),
       offset(pref_height), XtRImmediate, (XtPointer)1},
    {XtNsafeDir, XtCSafeDir, XtRBoolean, sizeof(Boolean),
          offset(safeDir), XtRImmediate, (XtPointer)True },
    {XtNsafer, XtCSafer, XtRBoolean, sizeof(Boolean),
          offset(safer), XtRImmediate, (XtPointer)True },
    {XtNinterpreter, XtCInterpreter, XtRString, sizeof(String),
          offset(interpreter), XtRString, "gs" },
#if 0
    {XtNhighlightPixel, XtCHighlightPixel, XtRPixel, sizeof(Pixel),
	offset(highlight_pixel), XtRString, XtDefaultForeground},
#endif
#undef offset
};

static void Message(Widget,XEvent*,String*,Cardinal*);
static void Notify(Widget,XEvent*,String*,Cardinal*);
static void action_changeCursor(Widget,XEvent*,String*,Cardinal*);
static void Input(XtPointer,int *,XtInputId*);
static void Output(XtPointer,int*,XtInputId*);

static void ClassInitialize(void);
static void ClassPartInitialize(WidgetClass);
static void Initialize(Widget,Widget,ArgList,Cardinal*);
static void Realize(Widget,XtValueMask*,XSetWindowAttributes*);
static void Redisplay(Widget,XEvent *,Region);
static void Destroy(Widget);
static void Resize(Widget);
static Boolean SetValues(Widget,Widget,Widget,ArgList,Cardinal*);
static XtGeometryResult QueryGeometry(Widget,XtWidgetGeometry *,XtWidgetGeometry *);

static void Layout(Widget,Boolean,Boolean);
static Boolean ComputeSize(Widget,Boolean,Boolean,Dimension*,Dimension*);
static void SetBackground(Widget,Bool);
static Boolean Setup(Widget);
static void StartInterpreter(Widget);
static void StopInterpreter(Widget);
static void InterpreterFailed(Widget,const char *);
static void ChangeCursor(GhostviewWidget,int);

static XtActionsRec actions[] =
{
    {"message",	Message},
    {"notify",	Notify},
    {"cursor",	action_changeCursor},
};

static char translations[] =
"<Message>:message()\n\
";

GhostviewClassRec ghostviewClassRec = {
  { /* core fields */
    /* superclass		*/	(WidgetClass) &coreClassRec,
    /* class_name		*/	"Ghostview",
    /* widget_size		*/	sizeof(GhostviewRec),
    /* class_initialize		*/	ClassInitialize,
    /* class_part_initialize	*/	ClassPartInitialize,
    /* class_inited		*/	FALSE,
    /* initialize		*/	Initialize,
    /* initialize_hook		*/	NULL,
    /* realize			*/	Realize,
    /* actions			*/	actions,
    /* num_actions		*/	XtNumber(actions),
    /* resources		*/	resources,
    /* num_resources		*/	XtNumber(resources),
    /* xrm_class		*/	NULLQUARK,
    /* compress_motion		*/	TRUE,
    /* compress_exposure	*/	TRUE,
    /* compress_enterleave	*/	TRUE,
    /* visible_interest		*/	FALSE,
    /* destroy			*/	Destroy,
    /* resize			*/	Resize,
    /* expose			*/	Redisplay,
    /* set_values		*/	SetValues,
    /* set_values_hook		*/	NULL,
    /* set_values_almost	*/	XtInheritSetValuesAlmost,
    /* get_values_hook		*/	NULL,
    /* accept_focus		*/	NULL,
    /* version			*/	XtVersion,
    /* callback_private		*/	NULL,
    /* tm_table			*/	translations,
    /* query_geometry		*/	QueryGeometry,
    /* display_accelerator	*/	XtInheritDisplayAccelerator,
    /* extension		*/	NULL
  },
  { /* ghostview fields */
    /* ghostview		*/	NULL,
    /* gv_colors		*/	NULL,
    /* next			*/	NULL,
    /* page			*/	NULL,
    /* done			*/	NULL
  }
};


WidgetClass ghostviewWidgetClass = (WidgetClass)&ghostviewClassRec;


/*###################################################################################*/
/* 
 * Double pixmap routines
 */
/*###################################################################################*/

static Pixmap pix = 0;

static void Copy_pixmap(Widget w)
{
   GhostviewWidget gvw = (GhostviewWidget) w;

  if(!gvw->ghostview.use_bpixmap){
    if(pix) {
      Display *dpy = XtDisplay(w);
      int scr = DefaultScreen(dpy);
      GC gc = DefaultGC(dpy, scr);
      Window win = XtWindow(w);
      int x;
      unsigned int gwidth, gheight, dummy;
      Window r;
    
      XGetGeometry(dpy, win, &r, &x, &x, &gwidth, &gheight, &dummy, &dummy);
      XCopyArea(dpy, pix, win, gc, 0,0, gwidth, gheight, 0,0);
    }
  }      

}

static void Realize_pixmap(Widget w)
{
    if(!pix) {
        Display *dpy = XtDisplay(w);
        int scr = DefaultScreen(dpy);
        Window win = XtWindow(w);
        int x;
	unsigned int gwidth, gheight, dummy;
        Window r;

        XGetGeometry(dpy, win, &r, &x, &x, &gwidth, &gheight, &dummy, &dummy);
/*        printf("Realize_pixmap %d %d\n", gwidth, gheight);         */
        pix =  XCreatePixmap(dpy, win, gwidth, gheight, DefaultDepth(dpy,scr));
    } 
}

static void
Redisplay(Widget w, XEvent *event _GL_UNUSED, Region region _GL_UNUSED)
{

  BEGINMESSAGE(Redisplay)
  Copy_pixmap(w);
  ENDMESSAGE(Redisplay)
}

/*###################################################################################*/
/* Message action routine.
 * Passes ghostscript message events back to application via
 * the message callback.  It also marks the interpreter as
 * being not busy at the end of page, and stops the interpreter
 * when it send a "done" message.
 */
/*###################################################################################*/

static void
Message(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
   GhostviewWidget gvw = (GhostviewWidget) w;
   GhostviewWidgetClass gvc = (GhostviewWidgetClass) XtClass(w);

   BEGINMESSAGE(Message)
   gvw->ghostview.mwin = event->xclient.data.l[0];
   if (event->xclient.message_type== XmuInternAtom(XtDisplay(w),gvc->ghostview_class.page)) {
      INFMESSAGE(interpreter completed page)
      gvw->ghostview.busy = False;
      ChangeCursor(gvw,CURSOR_RESET);
      XtCallCallbackList(w, gvw->ghostview.message_callback, "Page");
   } 
   else if (event->xclient.message_type==XmuInternAtom(XtDisplay(w),gvc->ghostview_class.done)) {
      INFMESSAGE(stopping interpreter NOW)
      StopInterpreter(w);  
      XtCallCallbackList(w, gvw->ghostview.message_callback, "Done");
   }
    
    if(!gvw->ghostview.use_bpixmap){
      if(!pix) {
        StopInterpreter(w);
        Realize_pixmap(w);
        show_page(REQUEST_REDISPLAY,NULL);
      } else {
        Copy_pixmap(w);
      }
    }      
   ENDMESSAGE(Message)
}

static void
action_changeCursor(Widget w, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{
   int which=0;

   BEGINMESSAGE(action_changeCursor)
   if (*num_params == 1) {
      if      (*(params[0]) == 'n' || *(params[0]) == 'N') which = CURSOR_NORMAL;
      else if (*(params[0]) == 's' || *(params[0]) == 'S') which = CURSOR_SCROLL;
      if (which) ChangeCursor((GhostviewWidget)w, which);
   }
   ENDMESSAGE(action_changeCursor)
}

/*----------------------------------------------------------*/
/* ChangeCursor */
/*----------------------------------------------------------*/

static void
ChangeCursor(GhostviewWidget gvw, int which)
{
   Widget w = (Widget) gvw;
   Cursor cursor;

   BEGINMESSAGE(ChangeCursor)

   cursor = (Cursor) NULL;
   switch (which) {
      case CURSOR_RESET:
           if (!gvw->ghostview.busy) {
              if (gvw->ghostview.cursor_type == CURSOR_SCROLL)
                 cursor = gvw->ghostview.scroll_cursor;
              else if (gvw->ghostview.cursor_type == CURSOR_NORMAL)
                 cursor = gvw->ghostview.cursor;
           }
           break;
      case CURSOR_NORMAL:
           if (!gvw->ghostview.busy) cursor = gvw->ghostview.cursor;
           gvw->ghostview.cursor_type = CURSOR_NORMAL;
           break;
      case CURSOR_SCROLL:
           if (!gvw->ghostview.busy) cursor = gvw->ghostview.scroll_cursor;
           gvw->ghostview.cursor_type = CURSOR_SCROLL;
           break;
      case CURSOR_BUSY:
           cursor = gvw->ghostview.busy_cursor; 
           break;
      default:
           break;
   }
   if (cursor) XDefineCursor(XtDisplay(w), XtWindow(w),cursor);
    
   ENDMESSAGE(ChangeCursor)
}

/*###################################################################################*/
/* Notify action routine.
 * Calculates where the user clicked in the default user coordinate system.
 * Call the callbacks with the point of click.
 */
/*###################################################################################*/

static void
Notify(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  GhostviewWidget gvw = (GhostviewWidget) w;
  GhostviewReturnStruct ret_val;
  int psx,psy;

  BEGINMESSAGE1(Notify)
  GhostviewCoordsXtoPS(w,event->xbutton.x,event->xbutton.y,&psx,&psy);
#if 0
  {
    int wx,wy;
    GhostviewCoordsPStoX(w,psx,psy,&wx,&wy);
    printf("evx=%d evy=%d\n",event->xbutton.x,event->xbutton.y);
    printf("psx=%d psy=%d\n",psx,psy);
    printf("wx =%d wy =%d\n\n",wx,wy);
  }
#endif
  ret_val.width  = 0;
  ret_val.height = 0;
  ret_val.psx = psx;
  ret_val.psy = psy;
  XtCallCallbackList(w, gvw->ghostview.callback, (XtPointer) &ret_val);
  ENDMESSAGE1(Notify)
}

#ifndef SEEK_SET
#   define SEEK_SET 0
#endif

static Boolean broken_pipe = False;

static SIGVAL
CatchPipe(int i _GL_UNUSED)
{
    broken_pipe = True;
#ifdef SIGNALRETURNSINT
    return 0;
#endif
}


/*###################################################################################*/
/* Input - Feed data to ghostscript's stdin.
 * Write bytes to ghostscript using non-blocking I/O.
 * Also, pipe signals are caught during writing.  The return
 * values are checked and the appropriate action is taken.  I do
 * this at this low level, because it may not be appropriate for
 * SIGPIPE to be caught for the overall application.
 */
/*###################################################################################*/

static void
Input(XtPointer client_data, int *source _GL_UNUSED, XtInputId *id _GL_UNUSED)
{
    Widget w = (Widget) client_data;
    GhostviewWidget gvw = (GhostviewWidget) w;
    int bytes_written;
    SIGVAL (*oldsig)(int);

    BEGINMESSAGE(Input)

    oldsig = signal(SIGPIPE, CatchPipe);

#ifdef NON_BLOCKING_IO
    do {
#endif

	if (gvw->ghostview.buffer_bytes_left == 0) {

	    /* Get a new section if required */
	    if (gvw->ghostview.ps_input && gvw->ghostview.bytes_left == 0) {
		struct record_list *ps_old = gvw->ghostview.ps_input;
		gvw->ghostview.ps_input = ps_old->next;
		if (ps_old->close) fclose(ps_old->fp);
		XtFree((char *)ps_old);
	    }

	    /* Have to seek at the beginning of each section */
	    if (gvw->ghostview.ps_input &&
		gvw->ghostview.ps_input->seek_needed) {
		if (gvw->ghostview.ps_input->len > 0)
		    GV_FSEEK(gvw->ghostview.ps_input->fp,
			  gvw->ghostview.ps_input->begin, SEEK_SET);
		gvw->ghostview.ps_input->seek_needed = False;
		gvw->ghostview.bytes_left = gvw->ghostview.ps_input->len;
	    }

	    if (gvw->ghostview.bytes_left > GV_BUFSIZ) {
		gvw->ghostview.buffer_bytes_left =
			fread(gvw->ghostview.input_buffer,
			      sizeof (char), GV_BUFSIZ,
			      gvw->ghostview.ps_input->fp);
	    } else if (gvw->ghostview.bytes_left > 0) {
		gvw->ghostview.buffer_bytes_left =
			fread(gvw->ghostview.input_buffer,
			      sizeof (char), gvw->ghostview.bytes_left,
			      gvw->ghostview.ps_input->fp);
	    } else {
		gvw->ghostview.buffer_bytes_left = 0;
	    }
	    if (gvw->ghostview.bytes_left > 0 &&
		gvw->ghostview.buffer_bytes_left == 0) {
		InterpreterFailed(w,"bytes left");	/* Error occurred */
	    }
	    gvw->ghostview.input_buffer_ptr = gvw->ghostview.input_buffer;
	    gvw->ghostview.bytes_left -= gvw->ghostview.buffer_bytes_left;
	}

	if (gvw->ghostview.buffer_bytes_left > 0) {
	    bytes_written = write(gvw->ghostview.interpreter_input,
				  gvw->ghostview.input_buffer_ptr,
				  gvw->ghostview.buffer_bytes_left);

#if 0
{
 char tmp[5000];
 strncpy(tmp,gvw->ghostview.input_buffer_ptr,gvw->ghostview.buffer_bytes_left);
 tmp[gvw->ghostview.buffer_bytes_left] = '\0';
 strcat(tmp,"###END###\n");
 printf(tmp);
}
#endif
	    if (broken_pipe) {
		broken_pipe = False;
		InterpreterFailed(w,"broken pope");		/* Something bad happened */
	    } else if (bytes_written == -1) {
		if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
		    InterpreterFailed(w,"write to pipe failed");	/* Something bad happened */
		}
	    } else {
	       gvw->ghostview.buffer_bytes_left -= bytes_written;
	       gvw->ghostview.input_buffer_ptr  += bytes_written;
/* 
   hack alert: if the last char of a section is not a <lf> we append a <lf>
               by hand. This we do since gs doesn't seem to finish a page properly if
               the file's records are terminated by <cr>. (###jp### 02.06.96)
*/
               IMESSAGE1(bytes_written)
               if (gvw->ghostview.bytes_left == 0 &&
                   gvw->ghostview.buffer_bytes_left == 0 &&
                   bytes_written > 0 &&
                   *(gvw->ghostview.input_buffer_ptr-1) != '\n') {
                  int b;
                  INFMESSAGE(################## appending artificial LF)
                  b=write(gvw->ghostview.interpreter_input,"\n",1);
                  if (broken_pipe) {
                     broken_pipe = False;
                     InterpreterFailed(w,"broken pipe writing NL");		/* Something bad happened */
                  } else if (b == -1) {
                     if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
                        InterpreterFailed(w,"writing NL to pipe failed");	/* Something bad happened */
                     }
                  }
               }
	    }
	}
#ifdef NON_BLOCKING_IO
    } while(gvw->ghostview.ps_input &&
	    gvw->ghostview.buffer_bytes_left == 0);
#endif
    signal(SIGPIPE, oldsig);
    if (gvw->ghostview.ps_input == NULL &&
	gvw->ghostview.buffer_bytes_left == 0) {

	INFMESSAGE(Input has no bytes left)
        {
           /* gs8.57 starts by doing a peekstring for 1023 bytes */
           enum gs_peek_enum { PEEK_SIZE = 1023 };
           char peek_buf[ PEEK_SIZE ];
           int b;
           memset(peek_buf, '\n', PEEK_SIZE);
           INFMESSAGE(################## writing junk for peek)
           b = write(gvw->ghostview.interpreter_input, peek_buf, PEEK_SIZE);
           if (broken_pipe) {
	      broken_pipe = False;
	      InterpreterFailed(w,"broken pipe writing peek buf");
           } else if (b == -1) {
	      if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
	         InterpreterFailed(w,"writing peek buf to pipe failed");	/* Something bad happened */
	      }
	   }
	}

	if (gvw->ghostview.interpreter_input_id != None) {
	    XtRemoveInput(gvw->ghostview.interpreter_input_id);
	    gvw->ghostview.interpreter_input_id = None;
	}
    }
    ENDMESSAGE(Input)

}

/*###################################################################################*/
/* Output - receive I/O from ghostscript's stdout and stderr.
 * Pass this to the application via the output_callback. */
/*###################################################################################*/

static void
Output(XtPointer client_data, int *source, XtInputId *id _GL_UNUSED)
{
    Widget w = (Widget) client_data;
    GhostviewWidget gvw = (GhostviewWidget) w;
    char buf[GV_BUFSIZ+1];
    int bytes = 0;

    BEGINMESSAGE(Output)

    if (*source == gvw->ghostview.interpreter_output) {
	bytes = read(gvw->ghostview.interpreter_output, buf, GV_BUFSIZ);
	if (bytes == 0) { /* EOF occurred */
	    INFMESSAGE(EOF occured in interpreter_output)
	    close(gvw->ghostview.interpreter_output);
	    gvw->ghostview.interpreter_output = -1;
	    XtRemoveInput(gvw->ghostview.interpreter_output_id);
            ENDMESSAGE(Output)
	    return;
	} else if (bytes == -1) {
	    INFMESSAGE(something bad happened in interpreter_output)
	    InterpreterFailed(w,"read stdout from pipe failed");		/* Something bad happened */
            ENDMESSAGE(Output)
	    return;
	}
    } else if (*source == gvw->ghostview.interpreter_error) {
	bytes = read(gvw->ghostview.interpreter_error, buf, GV_BUFSIZ);
	if (bytes == 0) { /* EOF occurred */
	    INFMESSAGE(EOF occurred in interpreter_error)
	    close(gvw->ghostview.interpreter_error);
	    gvw->ghostview.interpreter_error = -1;
	    XtRemoveInput(gvw->ghostview.interpreter_error_id);
            ENDMESSAGE(Output)
	    return;
	} else if (bytes == -1) {
	    INFMESSAGE(something bad happened in interpreter_error)
	    InterpreterFailed(w,"read stderr from pipe failed");		/* Something bad happened */
            ENDMESSAGE(Output)
	    return;
	}
    }
    if (bytes > 0) {
	buf[bytes] = '\0';
	XtCallCallbackList(w, gvw->ghostview.output_callback, (XtPointer) buf);
    }
    ENDMESSAGE(Output)
}


/*###################################################################################*/
/* Register the type converter required for the PageOrientation. */
/* Register the type converter required for the Palette. */
/* This routine is called exactly once. */
/*###################################################################################*/

static void
ClassInitialize(void)
{
    BEGINMESSAGE(ClassInitialize)
#ifndef GV_CODE
    XtSetTypeConverter(XtRString, XtRPageOrientation,
		       XmuCvtStringToPageOrientation, NULL, 0,
		       XtCacheAll, NULL);
#endif
    XtSetTypeConverter(XtRString, XtRPalette,
		       XmuCvtStringToPalette, NULL, 0,
		       XtCacheAll, NULL);
    ENDMESSAGE(ClassInitialize)
}

/*###################################################################################*/
/* Get atoms needed to communicate with ghostscript. */
/* This routine is called once per display. */
/*###################################################################################*/

static void
ClassPartInitialize(class)
    WidgetClass class;
{
    GhostviewWidgetClass gvc = (GhostviewWidgetClass)class;
    BEGINMESSAGE(ClassPartInitialize)
    gvc->ghostview_class.ghostview = XmuMakeAtom("GHOSTVIEW");
    gvc->ghostview_class.gv_colors = XmuMakeAtom("GHOSTVIEW_COLORS");
    gvc->ghostview_class.next = XmuMakeAtom("NEXT");
    gvc->ghostview_class.page = XmuMakeAtom("PAGE");
    gvc->ghostview_class.done = XmuMakeAtom("DONE");
    ENDMESSAGE(ClassPartInitialize)
}

/*###################################################################################*/
/* Initialize private state. */
/*###################################################################################*/

static void
Initialize(Widget request, Widget new, ArgList args _GL_UNUSED, Cardinal *num_args _GL_UNUSED)
{
    XGCValues	values;
    XtGCMask	mask;
    GhostviewWidget ngvw = (GhostviewWidget) new;
    GhostviewWidget rgvw = (GhostviewWidget) request;

    BEGINMESSAGE(Initialize)

    values.foreground = new->core.background_pixel;
    values.background = new->core.background_pixel;
    values.graphics_exposures = False;
    mask = GCGraphicsExposures|GCForeground|GCBackground;
    ngvw->ghostview.gc = XtGetGC(new, mask, &values);
    values.foreground   = new->core.background_pixel;
    values.background   = new->core.background_pixel;
    values.line_width   = 1;
    values.function     = GXxor;
    values.graphics_exposures = False;
    mask=(GCFunction|GCGraphicsExposures|GCBackground|GCForeground|GCLineWidth);
    ngvw->ghostview.highlight_gc = XtGetGC(new,mask,&values); 

    ngvw->ghostview.mwin = None;
    ngvw->ghostview.disable_start = False;
    ngvw->ghostview.interpreter_pid = -1;
    ngvw->ghostview.input_buffer = NULL;
    ngvw->ghostview.bytes_left = 0;
    ngvw->ghostview.input_buffer_ptr = NULL;
    ngvw->ghostview.buffer_bytes_left = 0;
    ngvw->ghostview.ps_input = NULL;
    ngvw->ghostview.interpreter_input = -1;
    ngvw->ghostview.interpreter_output = -1;
    ngvw->ghostview.interpreter_error = -1;
    ngvw->ghostview.interpreter_input_id = None;
    ngvw->ghostview.interpreter_output_id = None;
    ngvw->ghostview.interpreter_error_id = None;
    ngvw->ghostview.gs_width = 0;
    ngvw->ghostview.gs_height = 0;
    ngvw->ghostview.changed = False;
    ngvw->ghostview.busy = False;
    ngvw->ghostview.cursor_type = CURSOR_NORMAL;
    ngvw->ghostview.background_cleared = 0;
    if (ngvw->ghostview.lxdpi) {
       ngvw->ghostview.xdpi  = (float)ngvw->ghostview.lxdpi/1000.0;
       ngvw->ghostview.lxdpi = 0;
       INFFMESSAGE(new xdpi,ngvw->ghostview.xdpi)
    }
    if (ngvw->ghostview.lydpi) {
       ngvw->ghostview.ydpi  = (float)ngvw->ghostview.lydpi/1000.0;
       ngvw->ghostview.lydpi = 0;
       INFFMESSAGE(new ydpi,ngvw->ghostview.ydpi)
    }

    /* Compute window size */
    Layout(new, (rgvw->core.width == 0), (rgvw->core.height == 0));
    ENDMESSAGE(Initialize)
}

/*###################################################################################*/
/* Create Window and start interpreter if needed */
/*###################################################################################*/

static void
Realize(Widget w, Mask *valueMask, XSetWindowAttributes *attributes)
{
    GhostviewWidget gvw = (GhostviewWidget) w;

    BEGINMESSAGE(Realize)
    if (gvw->ghostview.cursor != None) {
	attributes->cursor = gvw->ghostview.cursor;
	*valueMask |= CWCursor;
    }

    XtCreateWindow(w,(unsigned int)InputOutput,(Visual *)CopyFromParent,*valueMask,attributes);
/*    Realize_pixmap(w);  */
    IMESSAGE(gvw->core.width) IMESSAGE(gvw->core.height) 

    ENDMESSAGE(Realize)
}

/*###################################################################################*/
/* Destroy routine: kill the interpreter and release the GC */
/*###################################################################################*/

static void
Destroy(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;

    BEGINMESSAGE(Destroy)
    StopInterpreter(w);
    XtReleaseGC(w, gvw->ghostview.gc);
    XtReleaseGC(w, gvw->ghostview.highlight_gc);
    XtFree(gvw->ghostview.input_buffer);
    if (gvw->core.background_pixmap != XtUnspecifiedPixmap)
	XFreePixmap(XtDisplay(w), gvw->core.background_pixmap);
    ENDMESSAGE(Destroy)
}

/*###################################################################################*/
/* SetBackground */
/*###################################################################################*/

static void
SetBackground(Widget w, Bool clear)
{
   GhostviewWidget gvw = (GhostviewWidget) w;

   BEGINMESSAGE(SetBackground)
   if (!XtIsRealized(w)) {
      INFMESSAGE(not realized) ENDMESSAGE(SetBackground)
      return;
   }
   if (clear) {
      if (!gvw->ghostview.background_cleared) {
         if ((gvw->core.background_pixmap != None) &&
            (gvw->core.background_pixmap != XtUnspecifiedPixmap)) {
            INFMESSAGE(filling background pixmap)
	    XFillRectangle(XtDisplay(w),gvw->core.background_pixmap,
		           gvw->ghostview.gc,
		           0,0,gvw->core.width,gvw->core.height);
         }
         INFMESSAGE(clearing window)
         XClearWindow(XtDisplay(w), XtWindow(w));
         gvw->ghostview.background_cleared = 1;
      }
#ifdef MESSAGES
      else { INFMESSAGE(background already cleared) }
#endif
   } else {
      if ((gvw->core.background_pixmap != XtUnspecifiedPixmap)) {
         INFMESSAGE(setting background pixmap)
         XSetWindowBackgroundPixmap(XtDisplay(w),XtWindow(w),gvw->core.background_pixmap);
      }
   }
   ENDMESSAGE(SetBackground)
}

/*###################################################################################*/
/* Resize */
/*###################################################################################*/

static void
Resize(Widget w _GL_UNUSED)
{
  BEGINMESSAGE(Resize)
  ENDMESSAGE(Resize)
}

/*###################################################################################*/
/* SetValues routine.  Set new private state, based on changed values
 * in the widget.  Always returns False, because redisplay is never needed.
 */
/*###################################################################################*/

static Boolean
SetValues(Widget current, Widget request, Widget new, ArgList unused1 _GL_UNUSED, Cardinal *unused2 _GL_UNUSED)
{
    GhostviewWidget cgvw = (GhostviewWidget) current;
    GhostviewWidget rgvw = (GhostviewWidget) request;
    GhostviewWidget ngvw = (GhostviewWidget) new;
    String cfilename;
    String rfilename;
    String carguments;
    String rarguments;

    BEGINMESSAGE(SetValues)
    cfilename = cgvw->ghostview.filename;
    if (cfilename == NULL) cfilename = "(null)";
    rfilename = rgvw->ghostview.filename;
    if (rfilename == NULL) rfilename = "(null)";
    carguments = cgvw->ghostview.arguments;
    if (carguments == NULL) carguments = "(null)";
    rarguments = rgvw->ghostview.arguments;
    if (rarguments == NULL) rarguments = "(null)";

    if (ngvw->ghostview.lxdpi) {
       ngvw->ghostview.xdpi  = (float)ngvw->ghostview.lxdpi/1000.0;
       ngvw->ghostview.lxdpi = 0;
       INFFMESSAGE(new xdpi,ngvw->ghostview.xdpi)
    }
    if (ngvw->ghostview.lydpi) {
       ngvw->ghostview.ydpi  = (float)ngvw->ghostview.lydpi/1000.0;
       ngvw->ghostview.lydpi = 0;
       INFFMESSAGE(new ydpi,ngvw->ghostview.ydpi)
    }

    if (XtIsRealized(new) && !ngvw->ghostview.busy &&
	(cgvw->ghostview.cursor != ngvw->ghostview.cursor)) {
        ChangeCursor(ngvw,CURSOR_RESET);
    }
    if (XtIsRealized(new) && ngvw->ghostview.busy &&
	(cgvw->ghostview.busy_cursor != ngvw->ghostview.busy_cursor)) {
        ChangeCursor(ngvw,CURSOR_BUSY);
    }
    if (cgvw->core.background_pixel != rgvw->core.background_pixel) {
	XGCValues	values;
	XtGCMask	mask;

	XtReleaseGC(current, cgvw->ghostview.gc);
	values.foreground = new->core.background_pixel;
	values.background = new->core.background_pixel;
	values.graphics_exposures = False;
	mask = GCGraphicsExposures|GCForeground|GCBackground;
	ngvw->ghostview.gc = XtGetGC(new, mask, &values);
    }
    IIMESSAGE(ngvw->core.width,ngvw->core.height)
    if ( ( (cgvw->core.width == ngvw->core.width)		&&
	   (cgvw->core.height == ngvw->core.height)		&&
           (cgvw->core.border_width == ngvw->core.border_width)
         ) &&
         ( (cgvw->core.background_pixel != ngvw->core.background_pixel)		||
	   (cgvw->ghostview.foreground != ngvw->ghostview.foreground)		||
	   (cgvw->ghostview.palette != ngvw->ghostview.palette)			||
	   strcmp(cgvw->ghostview.interpreter, ngvw->ghostview.interpreter)	||
	   strcmp(carguments, rarguments)					||
	   (cgvw->ghostview.quiet != ngvw->ghostview.quiet)			||
	   (cgvw->ghostview.infoVerbose != ngvw->ghostview.infoVerbose)			||
	   (cgvw->ghostview.safer != ngvw->ghostview.safer)			||
	   (cgvw->ghostview.safeDir != ngvw->ghostview.safeDir)			||
	   strcmp(cfilename, rfilename)						||
           (cgvw->ghostview.orientation != ngvw->ghostview.orientation)		||
	   (cgvw->ghostview.use_bpixmap != ngvw->ghostview.use_bpixmap)		||  
           (cgvw->ghostview.xdpi != ngvw->ghostview.xdpi)			||
	   (cgvw->ghostview.ydpi != ngvw->ghostview.ydpi)			||
	   (cgvw->ghostview.bottom_margin != ngvw->ghostview.bottom_margin)	||
	   (cgvw->ghostview.left_margin != ngvw->ghostview.left_margin)		||
	   (cgvw->ghostview.right_margin != ngvw->ghostview.right_margin)	||
	   (cgvw->ghostview.top_margin != ngvw->ghostview.top_margin)		||
           (cgvw->ghostview.llx != ngvw->ghostview.llx)				||
	   (cgvw->ghostview.lly != ngvw->ghostview.lly)				||
	   (cgvw->ghostview.urx != ngvw->ghostview.urx)				||
	   (cgvw->ghostview.ury != ngvw->ghostview.ury)
       ) ) {	
          INFMESSAGE(change detected - redisplay unnecessary)
          ngvw->ghostview.changed = True;
          ComputeSize(new, True, True, &(ngvw->ghostview.pref_width), &(ngvw->ghostview.pref_height));
#         ifdef MESSAGES
            {
              Dimension w=ngvw->ghostview.pref_width,h=ngvw->ghostview.pref_height;
              INFIIMESSAGE(preferred size:,w,h)
            }
#         endif
    }

    ENDMESSAGE(SetValues)
    return(False);
}

/*###################################################################################*/
/*	Function Name: QueryGeometry
 *	Description: This tells the parent what size we would like to be
 *                   given certain constraints.
 *	Arguments: w - the widget.
 *                 intended - what the parent intends to do with us.
 *                 requested - what we want to happen.
 */
/*###################################################################################*/

static XtGeometryResult 
QueryGeometry(Widget w, XtWidgetGeometry *intended, XtWidgetGeometry *requested)
{
    Dimension new_width, new_height;
    Boolean change, width_req, height_req;
    
    BEGINMESSAGE(QueryGeometry)
    INFIMESSAGE(received request:,intended->request_mode)

    width_req  = (Boolean) (intended->request_mode & CWWidth);
    height_req = (Boolean) (intended->request_mode & CWHeight);

    if (!width_req && !height_req) {
       INFMESSAGE(not a width or height request) ENDMESSAGE(QueryGeometry) 
       return(XtGeometryYes);
    }    

    if (width_req) new_width = intended->width;
    else new_width = w->core.width;
    if (height_req) new_height = intended->height;
    else new_height = w->core.height;

    requested->request_mode = 0;
    change = ComputeSize(w, !width_req, !height_req, &new_width, &new_height);

    requested->request_mode |= CWWidth;
    requested->width = new_width;
    requested->request_mode |= CWHeight;
    requested->height = new_height;

    if (change) {INFMESSAGE(agree almost) ENDMESSAGE(QueryGeometry) return(XtGeometryAlmost);}
    ENDMESSAGE(QueryGeometry)
    return(XtGeometryYes);
}

/*###################################################################################*/
/* Layout */
/* Layout the widget. */
/*###################################################################################*/

static void
Layout(Widget w, Boolean xfree, Boolean yfree)
{
    Dimension width = w->core.width;
    Dimension height = w->core.height;

    BEGINMESSAGE(Layout)
    if (ComputeSize(w, xfree, yfree, &width, &height)) {
       XtWidgetGeometry request, reply;
       Boolean changed = False;

       INFMESSAGE(geometry has changed; requesting size change)
       request.request_mode = CWWidth | CWHeight;
       request.width = width;
       request.height = height;
    
       switch ( XtMakeGeometryRequest(w, &request, &reply) ) {
          case XtGeometryYes:
             changed = True;
             break;
          case XtGeometryNo:
             break;
	  case XtGeometryDone:
	     /* should not be reached */
             break;
          case XtGeometryAlmost:
	      ComputeSize(w, (request.height != reply.height),
                             (request.width != reply.width),
                             &(reply.width), &(reply.height));
	      request = reply;
	      switch (XtMakeGeometryRequest(w, &request, &reply) ) {
	         case XtGeometryYes:
	            changed = True;
	            break;
	         case XtGeometryNo:
	            break;
	         case XtGeometryDone:
		    /* should not be reached */
                    break;
	         case XtGeometryAlmost:
	            request = reply;
	            ComputeSize(w, FALSE, FALSE, &(request.width), &(request.height));
	            request.request_mode = CWWidth | CWHeight;
	            XtMakeGeometryRequest(w, &request, &reply);
	            changed = True;
	            break;
	      }
	      break;
       }
    }
    ENDMESSAGE(Layout)
}

/*###################################################################################*/
/* ComputeSize */
/* Compute new size of window, sets xdpi and ydpi if necessary.
 * returns True if new window size is different */
/*###################################################################################*/


static Boolean SetProperty(Widget w, Pixmap bpixmap);

static Boolean
ComputeSize(Widget w, Boolean xfree, Boolean yfree, Dimension *width, Dimension *height)
  /* xfree, yfree: Am I allowed to change width or height */
{
  GhostviewWidget gvw = (GhostviewWidget) w;
  Dimension new_width = *width;
  Dimension new_height = *height;
  float newxdpi=72, newydpi=72,xdpi,ydpi;
  Boolean change;
  int dpsw,dpsh;

  BEGINMESSAGE(ComputeSize)

  switch (gvw->ghostview.orientation) {
    case XtPageOrientationLandscape:
    case XtPageOrientationSeascape:
      dpsw = gvw->ghostview.ury-gvw->ghostview.lly+1;
      dpsh = gvw->ghostview.urx-gvw->ghostview.llx+1;
      xdpi = gvw->ghostview.ydpi;
      ydpi = gvw->ghostview.xdpi;
      break;
    case XtPageOrientationPortrait:
    case XtPageOrientationUpsideDown:
    default:
      dpsw = gvw->ghostview.urx-gvw->ghostview.llx+1;
      dpsh = gvw->ghostview.ury-gvw->ghostview.lly+1;
      xdpi = gvw->ghostview.xdpi;
      ydpi = gvw->ghostview.ydpi;
      break;
  }
  if (dpsw<1) dpsw = 1;
  if (dpsh<1) dpsh = 1;
  xdpi = gvw->ghostview.xdpi;
  ydpi = gvw->ghostview.ydpi;
  if (xfree && yfree) {
    INFMESSAGE(x and y are free)
    new_width  = (((float)dpsw*72.0)/xdpi + 0.5);
    new_height = (((float)dpsh*72.0)/ydpi + 0.5);
  } else {
    if (!xfree) newxdpi = (float)dpsw * 72.0 / (float)gvw->core.width;
    if (!yfree) newydpi = (float)dpsh * 72.0 / (float)gvw->core.height;
    if (xfree) {
      INFMESSAGE(x is free)
      newxdpi = newydpi * xdpi / ydpi;
      new_width = (((float)dpsw*72.0)/newxdpi+0.5);
    } else if (yfree) {
      INFMESSAGE(y is free)
      newydpi = newxdpi * ydpi / xdpi;
      new_height = (((float)(dpsh)*72.0)/newydpi+0.5);
    }
    gvw->ghostview.xdpi = newxdpi;
    gvw->ghostview.ydpi = newydpi;
  }

  FMESSAGE(gvw->ghostview.xdpi)
  FMESSAGE(gvw->ghostview.ydpi)
  change = (new_width != *width) || (new_height != *height);
  *width = new_width; 
  *height = new_height;
  gvw->ghostview.pref_width  = new_width; 
  gvw->ghostview.pref_height = new_height;
#if 0
    printf("--------------\n");
    if (xfree) printf(" x is free\n");
    else       printf(" x is not free\n");
    if (yfree) printf(" y is free\n");
    else       printf(" y is not free\n");
    printf(" xdpi=%f ydpi=%f\n",gvw->ghostview.xdpi,gvw->ghostview.ydpi);
    printf(" width=%d  height=%d\n",new_width,new_height);
    printf(" llx=%d lly=%d  urx=%d ury=%d\n",
       gvw->ghostview.llx, gvw->ghostview.lly, gvw->ghostview.urx,  gvw->ghostview.ury
    );
    printf("--------------\n");
#endif
    ENDMESSAGE(ComputeSize)
    return (change);
}

/*###################################################################################*/
/* Catch the alloc error when there is not enough resources for the
 * backing pixmap.  Automatically shut off backing pixmap and let the
 * user know when this happens.
 */
/*###################################################################################*/

static Boolean alloc_error;
static XErrorHandler oldhandler;

static int
catch_alloc (Display *dpy, XErrorEvent *err)
{
    if (err->error_code == BadAlloc) alloc_error = True;
    if (alloc_error) return 0;
    return oldhandler(dpy, err);
}

/*###################################################################################*/
/* Setup - sets up the backing pixmap, and GHOSTVIEW property and
 * starts interpreter if needed.
 * NOTE: the widget must be realized before calling Setup().
 * Returns True if a new interpreter was started, False otherwise.
 */
/*###################################################################################*/

static Boolean
Setup(Widget w)
{
   GhostviewWidget gvw = (GhostviewWidget) w;
   Pixmap bpixmap;

   BEGINMESSAGE(Setup)

   if (!gvw->ghostview.changed &&
      (gvw->core.width == gvw->ghostview.gs_width) &&
      (gvw->core.height == gvw->ghostview.gs_height)) {
      INFMESSAGE(nothing changed) ENDMESSAGE(Setup)
      return False;
   }

   StopInterpreter(w);

   if ((gvw->core.width != gvw->ghostview.gs_width) ||
      (gvw->core.height != gvw->ghostview.gs_height) ||
      (!gvw->ghostview.use_bpixmap)) {
      INFMESSAGE(geometry has changed or backingPixmap no longer used)
      if (gvw->core.background_pixmap != XtUnspecifiedPixmap) {
         INFMESSAGE(freeing previous background pixmap)
         XFreePixmap(XtDisplay(w), gvw->core.background_pixmap);
         gvw->core.background_pixmap = XtUnspecifiedPixmap;
         INFMESSAGE(setting background pixmap to 'None')
         XSetWindowBackgroundPixmap(XtDisplay(w), XtWindow(w), None);
      }
   }

   if (gvw->ghostview.use_bpixmap) {
      INFMESSAGE(setting up backing pixmap)
      if (gvw->core.background_pixmap == XtUnspecifiedPixmap) {
         INFMESSAGE(backing pixmap will be allocated)
         /* Get a Backing Pixmap, but be ready for the BadAlloc. */
         XSync(XtDisplay(w), False);  /* Get to known state */
         oldhandler = XSetErrorHandler(catch_alloc);
         alloc_error = False;
         bpixmap = XCreatePixmap(XtDisplay(w), XtWindow(w),
	     		         gvw->core.width, gvw->core.height,
			         gvw->core.depth);
	 XSync(XtDisplay(w), False);  /* Force the error */
	 if (alloc_error) {
            INFMESSAGE(allocation error for backing pixmap)
	    XtCallCallbackList(w, gvw->ghostview.message_callback,"BadAlloc");
	    if (bpixmap != None) {
	       XFreePixmap(XtDisplay(w), bpixmap);
	       XSync(XtDisplay(w), False);  /* Force the error */
	       bpixmap = None;
	    }
	 }
	 oldhandler = XSetErrorHandler(oldhandler);
	 if (bpixmap != None) {
            INFMESSAGE(setting background pixmap)
	    gvw->core.background_pixmap = bpixmap;
            XSetWindowBackgroundPixmap(XtDisplay(w), XtWindow(w), bpixmap);
         }
      } else {
	 bpixmap = gvw->core.background_pixmap;
      }
   } else {
      bpixmap = None;
   }

   {
      XSetWindowAttributes xswa;
      if (bpixmap != None) xswa.backing_store = NotUseful;
      else                 xswa.backing_store = Always;
      XChangeWindowAttributes(XtDisplay(w), XtWindow(w), CWBackingStore, &xswa);
   }

   return SetProperty(w,bpixmap);
}


 
static Boolean
SetProperty(Widget w, Pixmap bpixmap)
{
   GhostviewWidget gvw = (GhostviewWidget) w;
   GhostviewWidgetClass gvc = (GhostviewWidgetClass) XtClass(w);
   char buf[GV_BUFSIZ];
   float xdpi,ydpi;

   gvw->ghostview.gs_width = gvw->core.width;
   gvw->ghostview.gs_height = gvw->core.height;

   { 
     int dpsw,dpsh;
     switch (gvw->ghostview.orientation) {
       case XtPageOrientationLandscape:
       case XtPageOrientationSeascape:
	 dpsw = gvw->ghostview.ury-gvw->ghostview.lly+1;
	 dpsh = gvw->ghostview.urx-gvw->ghostview.llx+1;
	 break;
       case XtPageOrientationPortrait:
       case XtPageOrientationUpsideDown:
       default:
	 dpsw = gvw->ghostview.urx-gvw->ghostview.llx+1;
	 dpsh = gvw->ghostview.ury-gvw->ghostview.lly+1;
	 break;
     }
     if (dpsw<1) dpsw=1; if (dpsh<1) dpsh = 1;
     xdpi =  (float)gvw->core.width * 72.0 / (float)dpsw;
     ydpi =  (float)gvw->core.height* 72.0 / (float)dpsh;
  }

   IIMESSAGE(gvw->ghostview.gs_width,gvw->ghostview.gs_height)
   sprintf(buf, "%ld %d %d %d %d %d %g %g %d %d %d %d",
	   bpixmap, gvw->ghostview.orientation,
	   (int)gvw->ghostview.llx, (int)gvw->ghostview.lly,
	   gvw->ghostview.urx, gvw->ghostview.ury,
	   xdpi, ydpi,
	   gvw->ghostview.left_margin, gvw->ghostview.bottom_margin,
	   gvw->ghostview.right_margin, gvw->ghostview.top_margin);

   XChangeProperty(XtDisplay(w), XtWindow(w),
		   XmuInternAtom(XtDisplay(w), gvc->ghostview_class.ghostview),
		   XA_STRING, 8, PropModeReplace,
		   (unsigned char *)buf, strlen(buf));
   INFSMESSAGE(atom 1:, buf)
   sprintf(buf, "%s %d %d",
	   gvw->ghostview.palette == XtPaletteMonochrome ? "Monochrome" :
	   gvw->ghostview.palette == XtPaletteGrayscale  ? "Grayscale" :
	   gvw->ghostview.palette == XtPaletteColor      ? "Color" : "?",
	   (int)gvw->ghostview.foreground, (int)gvw->core.background_pixel);
   XChangeProperty(XtDisplay(w), XtWindow(w),
		   XmuInternAtom(XtDisplay(w), gvc->ghostview_class.gv_colors),
		   XA_STRING, 8, PropModeReplace,
		   (unsigned char *)buf, strlen(buf));
   INFSMESSAGE(atom 2:, buf)

  /* Be sure to update properties */

   XSync(XtDisplay(w), False);

   gvw->ghostview.background_cleared = 0;
   if(!pix || w != page) SetBackground(w,True);

   ENDMESSAGE(Setup)
   return True;
}


/* This routine starts the interpreter.  It sets the DISPLAY and 
 * GHOSTVIEW environment variables.  The GHOSTVIEW environment variable
 * contains the Window that ghostscript should write on.
 *
 * This routine also opens pipes for stdout and stderr and initializes
 * application input events for them.  If input to ghostscript is not
 * from a file, a pipe for stdin is created.  This pipe is setup for
 * non-blocking I/O so that the user interface never "hangs" because of
 * a write to ghostscript.
 */

static void
StartInterpreter(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    int	std_in[2];
    int	std_out[2];
    int	std_err[2];
    char buf[GV_BUFSIZ];
#define NUM_ARGS 100
    char *argv[NUM_ARGS];
    char *arguments = NULL;
    char *cptr;
    char *device = NULL;
    char *dptr;
    int argc = 0;
    int ret;
    void* toBeFreed = 0;
    String filename = 0;

    BEGINMESSAGE(StartInterpreter)

  if(w == page && !gvw->ghostview.use_bpixmap) {
/*    printf("StartInterpreter on page\n"); */
    SetProperty(w,None);
    if (!pix) SetBackground(w,True); 
  }
    else {
/*    printf("StartInterpreter old\n"); */
    SetBackground(w,True);
  }

    StopInterpreter(w);

    /* Clear the window before starting a new interpreter. */
    if (gvw->ghostview.disable_start) return;

    argv[argc++] = gvw->ghostview.interpreter;
    if (app_res.antialias) dptr = device = XtNewString(gv_gs_x11_alpha_device);
    else                   dptr = device = XtNewString(gv_gs_x11_device);
    while (isspace(*dptr)) dptr++;
    while (*dptr) {
	 argv[argc++] = dptr;
	 while (*dptr && !isspace(*dptr)) dptr++;
	 if (*dptr) *dptr++ = '\0';
	 if (argc + 2 >= NUM_ARGS) {
	    fprintf(stderr, "Too many arguments to interpreter.\n");
	    clean_safe_tempdir();
	    exit(EXIT_STATUS_ERROR);
	 }
	 while (isspace(*dptr)) dptr++;
    }
    argv[argc++] = "-dNOPROMPT";
    argv[argc++] = "-dNOPAUSE";
    if (gv_pdf_password)
    {
       char* password;
       
       String parameter = malloc(100);
       toBeFreed = (void*) parameter;
       strcpy(parameter, "-sPDFPassword=");
       password = quote_filename(gv_pdf_password);
       strcat(parameter, password);
       XtFree(password);
       argv[argc++] = parameter;
    }
    
    
    if (gvw->ghostview.quiet) argv[argc++] = "-dQUIET";
    if (gvw->ghostview.safer) 
      {
	argv[argc++] = "-P-";
	argv[argc++] = "-dSAFER";
#   ifdef ALLOW_PDF
	/* The file created by pdf2dsc opens the original
	   pdf file with the read operator. */
	if (gv_filename_dsc && (!gvw->ghostview.filename || !strcmp(gvw->ghostview.filename,"-")))
	   argv[argc++] = "-dDELAYSAFER";
#   endif
      }

    if (gvw->ghostview.arguments) {
	cptr = arguments = XtNewString(gvw->ghostview.arguments);
	while (isspace(*cptr)) cptr++;
	while (*cptr) {
	    argv[argc++] = cptr;
	    while (*cptr && !isspace(*cptr)) cptr++;
	    if (*cptr) *cptr++ = '\0';
	    if (argc + 2 >= NUM_ARGS) {
		fprintf(stderr, "Too many arguments to interpreter.\n");
		clean_safe_tempdir();
		exit(EXIT_STATUS_ERROR);
	    }
	    while (isspace(*cptr)) cptr++;
	}
    }

#   ifdef ALLOW_PDF
    if (gvw->ghostview.filename && strcmp(gvw->ghostview.filename,"-")) {
          if ( *(gvw->ghostview.filename) == '-' ) {
	     filename = malloc( strlen(gvw->ghostview.filename) + 3);
	     strcpy( filename, "./" );
	     strcat( filename, gvw->ghostview.filename);
             argv[argc++] = filename;
	  }
	  else
             argv[argc++] = gvw->ghostview.filename;
          argv[argc++] = "-c";
          argv[argc++] = "quit";
    } else
#   endif
    if (gvw->ghostview.filename && !strcmp(gvw->ghostview.filename,"-")) {
       /* The ghostscript documentation states that the "-" argument
	  tells gs that input is coming from a pipe rather than from stdin.
	  One of the side effects of the "-" argument is that gs does not
	  flush output at each line of input, but rather accumulates input
	  and flushes only when the buffer is full.  Since we want gs to
	  flush output at each line of input, we therefore cannot send "-".
	  Unfortunately not sending "-" has the side effect that gs no
	  longer reads correctly through either multiple PostScript files
	  or PostScript files with multiple pages.  Ah well.
	  Should gs ever acquire a -DFLUSH argument, then send gs that argument,
	  and uncomment the following line.  */
       /* argv[argc++] = "-"; */
    } else {
       argv[argc++] = "-";
    }
    argv[argc++] = NULL;

    if (gvw->ghostview.filename == NULL) {
	ret = pipe(std_in);
	if (ret == -1) {
	    perror("Could not create pipe");
	    clean_safe_tempdir();
	    exit(EXIT_STATUS_ERROR);
	}
    } else if (strcmp(gvw->ghostview.filename, "-")) {
#      ifdef ALLOW_PDF
          ret = pipe(std_in);
          if (ret == -1) {
             perror("Could not create pipe");
	     clean_safe_tempdir();
             exit(EXIT_STATUS_ERROR);
          }
#      else
          std_in[0] = open(gvw->ghostview.filename, O_RDONLY, 0);
#      endif
    }
    ret = pipe(std_out);
    if (ret == -1) {
	perror("Could not create pipe");
	clean_safe_tempdir();
	exit(EXIT_STATUS_ERROR);
    }
    ret = pipe(std_err);
    if (ret == -1) {
	perror("Could not create pipe");
	clean_safe_tempdir();
	exit(EXIT_STATUS_ERROR);
    }

    gvw->ghostview.changed = False;
    if (!((gvw->ghostview.filename && strcmp(gvw->ghostview.filename, "-") == 0)
       && (gv_gs_arguments && *gv_gs_arguments))) {
       gvw->ghostview.busy = True;
       ChangeCursor(gvw,CURSOR_BUSY);
    }

/*
    printf("StartInterpreter:\n");
    printf("%s", argv[0]);
    for (argc = 1; argv[argc] != NULL; argc++) {
        printf(" %s", argv[argc]);
    }
    printf("\n");
*/

    gvw->ghostview.interpreter_pid = fork();

    if (gvw->ghostview.interpreter_pid == 0) { /* child */
	close(std_out[0]);
	close(std_err[0]);
	dup2(std_out[1], 1);
	close(std_out[1]);
	dup2(std_err[1], 2);
	close(std_err[1]);
         if(pix && w == page){
           sprintf(buf, "%ld %ld", XtWindow(w), pix );
         } else {
           sprintf(buf, "%ld", XtWindow(w));
         }
	gnu_gv_setenv("GHOSTVIEW", buf, True);
	gnu_gv_setenv("DISPLAY", XDisplayString(XtDisplay(w)), True);
	if (gvw->ghostview.filename == NULL) {
	    close(std_in[1]);
	    dup2(std_in[0], 0);
	    close(std_in[0]);
	} else if (strcmp(gvw->ghostview.filename, "-")) {
#          ifdef ALLOW_PDF
	      close(std_in[1]);
	      dup2(std_in[0], 0);
	      close(std_in[0]);
#          else
	      dup2(std_in[0], 0);
	      close(std_in[0]);
#          endif
	}
    	if (gvw->ghostview.safeDir) {
		if (chdir(gv_safe_gs_workdir) != 0) {
			sprintf(buf, "Chdir to %s failed",
					gv_safe_gs_workdir);
			perror(buf);
			_exit(EXIT_STATUS_ERROR);
		}
    	}
	execvp(argv[0], argv);
	sprintf(buf, execOfFailedLabel, argv[0]);
	perror(buf);
	_exit(EXIT_STATUS_ERROR);
    } else {
        if (toBeFreed)
	   free(toBeFreed);
        if (filename)
	   free(filename);
	if (gvw->ghostview.filename == NULL) {
#ifdef NON_BLOCKING_IO
	    int result;
#endif
	    close(std_in[0]);

#ifdef NON_BLOCKING_IO
	    result = fcntl(std_in[1], F_GETFL, 0);
	    result = result | O_NONBLOCK;
	    result = fcntl(std_in[1], F_SETFL, result);
#endif
	    gvw->ghostview.interpreter_input = std_in[1];
	    gvw->ghostview.interpreter_input_id = None;
	} else if (strcmp(gvw->ghostview.filename, "-")) {
	    gvw->ghostview.interpreter_input = std_in[1];
	    gvw->ghostview.interpreter_input_id = None;
	    close(std_in[0]);
	}
	close(std_out[1]);
	gvw->ghostview.interpreter_output = std_out[0];
	gvw->ghostview.interpreter_output_id = 
	    XtAppAddInput(XtWidgetToApplicationContext(w), std_out[0],
			  (XtPointer)XtInputReadMask, Output, (XtPointer)w);
	close(std_err[1]);
	gvw->ghostview.interpreter_error = std_err[0];
	gvw->ghostview.interpreter_error_id = 
	    XtAppAddInput(XtWidgetToApplicationContext(w), std_err[0],
			  (XtPointer)XtInputReadMask, Output, (XtPointer)w);
    }
    XtFree(arguments);
    XtFree(device);
    gvw->ghostview.background_cleared=0;

    ENDMESSAGE(StartInterpreter)
}

/* Stop the interperter, if present, and remove any Input sources. */
/* Also reset the busy state. */
int restarted = 0;

static void
StopInterpreter(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(StopInterpreter)
    if (gvw->ghostview.interpreter_pid >= 0)
      {
	kill(gvw->ghostview.interpreter_pid, SIGTERM);
	gvw->ghostview.interpreter_pid = -1;
	wait(0);
      }
    if (gvw->ghostview.interpreter_input >= 0) 
      {
	INFMESSAGE(removing interpreter input)
	close(gvw->ghostview.interpreter_input);
	gvw->ghostview.interpreter_input = -1;
	if (gvw->ghostview.interpreter_input_id != None) 
	  {
	    XtRemoveInput(gvw->ghostview.interpreter_input_id);
	    gvw->ghostview.interpreter_input_id = None;
	  }
	while (gvw->ghostview.ps_input) 
	  {
	    struct record_list *ps_old = gvw->ghostview.ps_input;
	    gvw->ghostview.ps_input = ps_old->next;

	    if (ps_old->close) fclose(ps_old->fp);

	    XtFree((char *)ps_old);

	  }
      }
    if (gvw->ghostview.interpreter_output >= 0) {
        INFMESSAGE(closing interpreter output)
	close(gvw->ghostview.interpreter_output);
	gvw->ghostview.interpreter_output = -1;
	XtRemoveInput(gvw->ghostview.interpreter_output_id);
    }
    if (gvw->ghostview.interpreter_error >= 0) {
        INFMESSAGE(closing interpreter error)
	close(gvw->ghostview.interpreter_error);
	gvw->ghostview.interpreter_error = -1;
	XtRemoveInput(gvw->ghostview.interpreter_error_id);
    }
    gvw->ghostview.busy = False;
    ChangeCursor(gvw,CURSOR_RESET);
    ENDMESSAGE(StopInterpreter)
}


/*###################################################################################*/
/* InterpreterFailed */
/* The interpeter failed, Stop what's left and notify application */
/*###################################################################################*/

static void
InterpreterFailed(Widget w, const char *mesg)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(InterpreterFailed)
    StopInterpreter(w);
    fprintf(stderr, "Error: %s, errno %d\n", mesg, errno);
    XtCallCallbackList(w, gvw->ghostview.message_callback, "Failed");
    ENDMESSAGE(InterpreterFailed)
}

/*###################################################################################*/
/*###################################################################################*/
/*
 *	Public Routines
 */
/*###################################################################################*/
/*###################################################################################*/

void
GhostviewDrawRectangle(w,x,y,width,height)
  Widget w;
  int x,y,width,height;
{
  GhostviewWidget gvw = (GhostviewWidget) w;

  BEGINMESSAGE(GhostviewDrawRectangle)
  XDrawRectangle(XtDisplay(w),XtWindow(w),gvw->ghostview.highlight_gc,x,y,width,height);
  if (gvw->core.background_pixmap != XtUnspecifiedPixmap)
     XDrawRectangle(XtDisplay(w),gvw->core.background_pixmap,gvw->ghostview.highlight_gc,x,y,width,height);
  ENDMESSAGE(GhostviewDrawRectangle)
}

/*------------------------------------------------------------*/
/* GhostviewOrientCoords */
/*------------------------------------------------------------*/

#define GV_O_P (1<<0)
#define GV_O_U (1<<1)
#define GV_O_L (1<<2)
#define GV_O_S (1<<3)
#define GV_O_H ((GV_O_L) | (GV_O_S))
#define GV_O_V ((GV_O_P) | (GV_O_U))

static void GhostviewOrientCoords(GhostviewWidget gvw, int x, int y, int *xP, int *yP, int *widthP, int *heightP, int *orientP)
{
  BEGINMESSAGE(GhostviewOrientCoords)
  switch (gvw->ghostview.orientation) {
    case XtPageOrientationPortrait:   *orientP = GV_O_P; break;
    case XtPageOrientationUpsideDown: *orientP = GV_O_U; break;
    case XtPageOrientationLandscape:  *orientP = GV_O_L; break;
    case XtPageOrientationSeascape:   *orientP = GV_O_S; break;
    default:                          *orientP = GV_O_P; break;
  }
  if (*orientP & GV_O_V) {
    *widthP     = gvw->core.width-1;
    *heightP    = gvw->core.height-1;
    if (xP) *xP = x;
    if (yP) *yP = y;
  } else {
    *widthP     = gvw->core.height-1;
    *heightP    = gvw->core.width-1;
    if (xP) *xP = y;
    if (yP) *yP = x;
  }
  if (*widthP  < 1) *widthP=1;
  if (*heightP < 1) *heightP=1;
  ENDMESSAGE(GhostviewOrientCoords)
}

/*------------------------------------------------------------*/
/* GhostviewCornersPS */
/*------------------------------------------------------------*/

static void GhostviewCornersPS(GhostviewWidget gvw, int o, int *pslxP, int *psrxP, int *pslyP, int *psuyP)
{
  if (o & GV_O_P) {
    *pslxP=gvw->ghostview.llx; *psrxP=gvw->ghostview.urx;
    *pslyP=gvw->ghostview.ury; *psuyP=gvw->ghostview.lly;
  } else if (o & GV_O_L) {
    *pslxP=gvw->ghostview.llx; *psrxP=gvw->ghostview.urx;
    *pslyP=gvw->ghostview.lly; *psuyP=gvw->ghostview.ury;
  } else if (o & GV_O_U) {
    *pslxP=gvw->ghostview.urx; *psrxP=gvw->ghostview.llx;
    *pslyP=gvw->ghostview.lly; *psuyP=gvw->ghostview.ury;
  } else {
    *pslxP=gvw->ghostview.urx; *psrxP=gvw->ghostview.llx;
    *pslyP=gvw->ghostview.ury; *psuyP=gvw->ghostview.lly;
  }
}

/*############################################################*/
/* GhostviewCoordsXtoPS */
/*############################################################*/

void
GhostviewCoordsXtoPS(
  Widget w,
  int wx,
  int wy,
  int *psxP,
  int *psyP
)
{
  GhostviewWidget gvw = (GhostviewWidget) w;
  int o,x,y,pslx,psrx,psly,psuy,wrx,wly;

  BEGINMESSAGE(GhostviewCoordsXtoPS)
  GhostviewOrientCoords(gvw,wx,wy,&x,&y,&wrx,&wly,&o);
  GhostviewCornersPS(gvw,o,&pslx,&psrx,&psly,&psuy);
  *psxP = (pslx + ((float)x*(float)(psrx-pslx))/(float)wrx + 0.5);
  *psyP = (psly + ((float)y*(float)(psuy-psly))/(float)wly + 0.5);
#if 1
  {
    int xx,yy;
    IIMESSAGE(wx,wy)
    IIMESSAGE(*psxP,*psyP)
    GhostviewCoordsPStoX(w,*psxP,*psyP,&xx,&yy);
    IIMESSAGE(xx,yy)
  }
#endif
  ENDMESSAGE(GhostviewCoordsXtoPS)
}

/*############################################################*/
/* GhostviewCoordsPStoX */
/*############################################################*/

void
GhostviewCoordsPStoX(
  Widget w,
  int psx,
  int psy,
  int *wxP,
  int *wyP
)
{
  GhostviewWidget gvw = (GhostviewWidget) w;
  int o,pslx,psrx,psly,psuy,wrx,wly,*xP,*yP;

  BEGINMESSAGE(GhostviewCoordsPStoX)
  GhostviewOrientCoords(gvw,0,0,NULL,NULL,&wrx,&wly,&o);
  GhostviewCornersPS(gvw,o,&pslx,&psrx,&psly,&psuy);
  if (o & GV_O_V) { xP = wxP; yP=wyP; }
  else            { xP = wyP; yP=wxP; }
  *xP = (((float)(psx-pslx)*(float)wrx)/(float)(psrx-pslx)+0.5);
  *yP = (((float)(psy-psly)*(float)wly)/(float)(psuy-psly)+0.5);
  IIMESSAGE(*xP,*yP)
  ENDMESSAGE(GhostviewCoordsPStoX)
}

/*############################################################*/
/* GhostviewGetBBOfArea */
/*############################################################*/

void
GhostviewGetBBofArea (
  Widget   w,
  int x1,
  int y1,
  int x2,
  int y2,
  GhostviewReturnStruct* p
)
{
  GhostviewWidget gvw = (GhostviewWidget) w;
  int psx1,psy1,psx2,psy2;

  BEGINMESSAGE(GhostviewGetBBOfArea) 
  GhostviewCoordsXtoPS(w,x1,y1,&psx1,&psy1);
  GhostviewCoordsXtoPS(w,x2,y2,&psx2,&psy2);
  p->psx    = psx1 < psx2 ? psx1 : psx2;
  p->psy    = psy1 < psy2 ? psy1 : psy2;
  p->width  = abs(psx1-psx2)+1;
  p->height = abs(psy1-psy2)+1;
  p->xdpi   = gvw->ghostview.xdpi;
  p->ydpi   = gvw->ghostview.ydpi;
  IIMESSAGE(p->psx,p->psy)
  IIMESSAGE(p->width,p->height)
  ENDMESSAGE(GhostviewGetBBOfArea) 
}

/*############################################################*/
/* GhostviewGetAreaOfBB */
/*############################################################*/

void
GhostviewGetAreaOfBB (
  Widget   w,
  int psx1,
  int psy1,
  int psx2,
  int psy2,
  GhostviewReturnStruct* p
)
{
  int x1,y1,x2,y2;

  BEGINMESSAGE(GhostviewGetAreaOfBB) 
  GhostviewCoordsPStoX(w,psx1,psy1,&x1,&y1);
  GhostviewCoordsPStoX(w,psx2,psy2,&x2,&y2);
  p->psx    = x1 < x2 ? x1 : x2;
  p->psy    = y1 < y2 ? y1 : y2;
  p->width  = abs(x1-x2)+1;
  p->height = abs(y1-y2)+1;
  IIMESSAGE(p->psx,p->psy)
  IIMESSAGE(p->width,p->height)
  ENDMESSAGE(GhostviewGetAreaOfBB) 
}


/*###################################################################################*/
/* GhostviewClearBackground */
/*###################################################################################*/

void
GhostviewClearBackground(Widget w)
{
    BEGINMESSAGE(GhostviewClearBackground)
      if (XtIsRealized(w)) { if(!pix || w != page) SetBackground(w,True); }
    ENDMESSAGE(GhostviewClearBackground)
}

/*###################################################################################*/
/* GhostviewSetup */
/*###################################################################################*/

void
GhostviewSetup(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(GhostviewSetup)
    gvw->ghostview.disable_start = True;
    if (XtIsRealized(w)) Setup(w);
    ENDMESSAGE(GhostviewSetup)
}

/*###################################################################################*/
/* GhostviewDisableInterpreter:
 * Stop any interpreter and disable new ones from starting.
 */
/*###################################################################################*/
void
GhostviewDisableInterpreter(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(GhostviewDisableInterpreter)
    gvw->ghostview.disable_start = True;
    if (XtIsRealized(w)) StopInterpreter(w);
    ENDMESSAGE(GhostviewDisableInterpreter)
}

/*###################################################################################*/
/* GhostviewEnableInterpreter:
 * Allow an interpreter to start and start one if the widget is
 * currently realized.
 */
/*###################################################################################*/

void
GhostviewEnableInterpreter(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(GhostviewEnableInterpreter)
    gvw->ghostview.disable_start = False;
    if (XtIsRealized(w)) StartInterpreter(w);
    ENDMESSAGE(GhostviewEnableInterpreter)
}

/*###################################################################################*/
/* GhostviewState
 */
/*###################################################################################*/

void
GhostviewState(
    Widget w,
    Boolean *processflag_p,
    Boolean *busyflag_p,
    Boolean *inputflag_p
)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(GhostviewState)
    if (processflag_p) *processflag_p = (gvw->ghostview.interpreter_pid != -1) ? True: False;
    if (busyflag_p)    *busyflag_p    = (!(gvw->ghostview.busy))               ? True: False;
    if (inputflag_p)   *inputflag_p   = (gvw->ghostview.ps_input == NULL)      ? True: False;
#   ifdef MESSAGES
       if (gvw->ghostview.interpreter_pid != -1) {INFMESSAGE(have interpreter)}
       else                                      {INFMESSAGE(have no interpreter)}
       if (!(gvw->ghostview.busy))               {INFMESSAGE(widget idle)}
       else                                      {INFMESSAGE(widget busy)}
       if (gvw->ghostview.ps_input == NULL)      {INFMESSAGE(no pending input)}
       else                                      {INFMESSAGE(have pending input)}
#   endif
    ENDMESSAGE(GhostviewState)
}

/*###################################################################################*/
/* GhostviewIsInterpreterReady:
 * Returns True if the interpreter is ready for new input.
 */
/*###################################################################################*/

Boolean
GhostviewIsInterpreterReady(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(GhostviewIsInterpreterReady)
#   ifdef MESSAGES
       if (gvw->ghostview.interpreter_pid == -1) {INFMESSAGE(no interpreter process)}
       if (gvw->ghostview.busy)                  {INFMESSAGE(interpreter busy)}
       if (gvw->ghostview.ps_input != NULL)      {INFMESSAGE(interpreter still has input)}
#   endif
    ENDMESSAGE(GhostviewIsInterpreterReady)
    return gvw->ghostview.interpreter_pid != -1 && 
           !gvw->ghostview.busy &&
	   gvw->ghostview.ps_input == NULL;
}

/*###################################################################################*/
/* GhostviewIsBusy:
 * Returns True if the interpreter is running but the widget is busy
 */
/*###################################################################################*/

Boolean
GhostviewIsBusy(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(GhostviewIsBusy)
    ENDMESSAGE(GhostviewIsBusy)
    return (((gvw->ghostview.interpreter_pid != -1)&&(gvw->ghostview.busy)));
}

/*###################################################################################*/
/* GhostviewIsInterpreterRunning:
 * Returns True if the interpreter is running.
 */
/*###################################################################################*/

Boolean
GhostviewIsInterpreterRunning(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    BEGINMESSAGE(GhostviewIsInterpreterRunning)
    ENDMESSAGE(GhostviewIsInterpreterRunning)
    if(gvw->ghostview.disable_start) return 0;
    return gvw->ghostview.interpreter_pid != -1;
}

/*###################################################################################*/
/* GhostviewSendPS:
 *   Queue a portion of a PostScript file for output to ghostscript.
 *   fp: FILE * of the file in question.  NOTE: if you have several
 *   Ghostview widgets reading from the same file.  You must open
 *   a unique FILE * for each widget.
 *   SendPS does not actually send the PostScript, it merely queues it
 *   for output.
 *   begin: position in file (returned from ftell()) to start.
 *   len:   number of bytes to write.
 *
 *   If an interpreter is not running, nothing is queued and
 *   False is returned.
 */
/*###################################################################################*/

Boolean
GhostviewSendPS(Widget w, FILE *fp, gv_off_t begin, gv_off_t len, Bool doclose)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    struct record_list *ps_new;

    if (gvw->ghostview.interpreter_input < 0) return False;
    ps_new = (struct record_list *) XtMalloc(sizeof (struct record_list));
    ps_new->fp = fp;
    ps_new->begin = begin;
    ps_new->len = len;
    ps_new->seek_needed = True;
    ps_new->close = doclose;
    ps_new->next = NULL;

    if (gvw->ghostview.input_buffer == NULL) {
	gvw->ghostview.input_buffer = XtMalloc(GV_BUFSIZ);
    }

    if (gvw->ghostview.ps_input == NULL) {
	gvw->ghostview.input_buffer_ptr = gvw->ghostview.input_buffer;
	gvw->ghostview.bytes_left = len;
	gvw->ghostview.buffer_bytes_left = 0;
	gvw->ghostview.ps_input = ps_new;
	gvw->ghostview.interpreter_input_id =
		XtAppAddInput(XtWidgetToApplicationContext(w),
			      gvw->ghostview.interpreter_input,
			      (XtPointer)XtInputWriteMask, Input, (XtPointer)w);
    } else {
	struct record_list *p = gvw->ghostview.ps_input;
	while (p->next != NULL) {
	    p = p->next;
	}
	p->next = ps_new;
    }
    return True;
}

/*###################################################################################*/
/* GhostviewNextPage:
 *   Tell ghostscript to start the next page.
 *   Returns False if ghostscript is not running, or not ready to start
 *   another page.
 *   If another page is started.  Sets the busy flag and cursor.
 */
/*###################################################################################*/

Boolean
GhostviewNextPage(Widget w)
{
    GhostviewWidget gvw = (GhostviewWidget) w;
    GhostviewWidgetClass gvc = (GhostviewWidgetClass) XtClass(w);
    XEvent event;

    BEGINMESSAGE(GhostviewNextPage)
    if (gvw->ghostview.interpreter_pid < 0) {
       INFMESSAGE(no interpreter active) ENDMESSAGE(GhostviewNextPage) return False;
    }
    if (gvw->ghostview.mwin == None) {
       INFMESSAGE(no window) ENDMESSAGE(GhostviewNextPage) return False;
    }

    if (!gvw->ghostview.busy) {
	if (!((gvw->ghostview.filename && strcmp(gvw->ghostview.filename, "-") == 0)
	   && (gv_gs_arguments && *gv_gs_arguments))) {
	    gvw->ghostview.busy = True;
	    ChangeCursor(gvw,CURSOR_BUSY);
	}
	event.xclient.type = ClientMessage;
	event.xclient.display = XtDisplay(w);
	event.xclient.window = gvw->ghostview.mwin;
	event.xclient.message_type = XmuInternAtom(XtDisplay(w), gvc->ghostview_class.next);
	event.xclient.format = 32;
	XSendEvent(XtDisplay(w), gvw->ghostview.mwin, False, 0, &event);
	XFlush(XtDisplay(w));	/* And push it out */
        INFMESSAGE(ok) ENDMESSAGE(GhostviewNextPage) return True;
    } else {
        INFMESSAGE(ghostview busy) ENDMESSAGE(GhostviewNextPage) return False;
    }
}

/*###################################################################################*/
/* Palette Conversion Routine.
 * Returns True if Conversion is successful.
 */
/*###################################################################################*/

#define	done(type, value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}

Boolean
XmuCvtStringToPalette(Display *dpy, XrmValue *args _GL_UNUSED, Cardinal *num_args _GL_UNUSED, XrmValue *fromVal, XrmValue *toVal, XtPointer *data _GL_UNUSED)
{
    static XrmQuark		XrmQEmonochrome;
    static XrmQuark		XrmQEgrayscale;
    static XrmQuark		XrmQEcolor;
    static int			haveQuarks;
    XrmQuark    q;
    char	*str = (XPointer) fromVal->addr;
    char        lowerName[100];

    if (str == NULL) return False;

    if (!haveQuarks) {
	XrmQEmonochrome = XrmStringToQuark(XtEmonochrome);
	XrmQEgrayscale  = XrmStringToQuark(XtEgrayscale);
	XrmQEcolor      = XrmStringToQuark(XtEcolor);
	haveQuarks = 1;
    }

    XmuCopyISOLatin1Lowered(lowerName, str);

    q = XrmStringToQuark(lowerName);

    if (q == XrmQEmonochrome)
	done(XtPalette, XtPaletteMonochrome);
    if (q == XrmQEgrayscale)
	done(XtPalette, XtPaletteGrayscale);
    if (q == XrmQEcolor)
	done(XtPalette, XtPaletteColor);

    XtDisplayStringConversionWarning(dpy, str, XtRPalette);
    return False;
}

