/*
**
** actions.c
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
*+            Jose E. Marchesi (jemarch@gnu.org)
**            GNU Project
*/
#include "ac_config.h"

#include "message.h"

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <inttypes.h>

#include "paths.h"
#include INC_X11(Intrinsic.h)
#include INC_X11(StringDefs.h)
#include INC_XAW(Cardinals.h)
#include INC_XAW(SimpleMenu.h)
#include INC_XAW(SmeBSB.h)
#include INC_XAW(SmeLine.h) 
#include INC_XAW(Scrollbar.h)
#include INC_XAW(MenuButton.h)
#include INC_X11(IntrinsicP.h)
#include "Aaa.h"
#include "Clip.h"
#include "Ghostview.h"
#include "MButton.h"
#include "Vlist.h"

#include "types.h"
#include "actions.h"
#include "callbacks.h"
#include "ps.h"
#include "doc_misc.h"
#include "dialog.h"  /* include the popup headers for action_delete_window */
#include "note.h"
#include "info.h"
#include "main_resources.h"
#include "main_globals.h"
#include "misc.h"
#include "options.h"
#include "version.h"

/*##################################################################*/
/* action_shellConfigureNotify */
/*##################################################################*/

void
action_shellConfigureNotify(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  static int width=0,height=0;
  int new_width=0,new_height=0;

  BEGINMESSAGE(action_shellConfigureNotify)
  if (w==toplevel && event->type == ConfigureNotify) {
     new_width = ((XConfigureEvent*)event)->width;
     new_height = ((XConfigureEvent*)event)->height;
     IIMESSAGE(new_width,new_height)
  }
  if (new_width && new_height) {
    if (gv_scales[gv_scale]->scale <= 0 && !app_res.auto_resize)
    {
       misc_savePagePosition();
       show_page(REQUEST_NEW_SCALE,NULL);
       misc_resetPagePosition();
    }
    if (width != new_width || height != new_height) {
       misc_setPageMarker(0,2,event,True); /* bring selected in sight */
       width = new_width;
       height=new_height;
    }
    INFMESSAGE(action_shellConfigureNotify calling XawScrollbarSetThumb)
    XawScrollbarSetThumb(newtocScroll,
		   VlistScrollPosition(newtoc),
		   VlistVisibleLength(newtoc,newtocClip->core.height));

  }
  ENDMESSAGE(action_shellConfigureNotify)
}

/*##################################################################*/
/* action_page */
/*##################################################################*/

static int scroll_success(Widget clip, int e, int x, int y, int nx, int ny)
{
  int tx,ty;
  ClipWidgetCheckCoordinates(clip,nx,ny,&tx,&ty);
  if (((e&PAGE_H) && tx != x) || ((e&PAGE_V) && ty != y)) {
    INFMESSAGE(scrolling has effect)
    return(1);
  } else {
    INFMESSAGE(scrolling has no effect)
    return(0);
  }
}

static void
catch_timer(XtPointer client_data, XtIntervalId *idp _GL_UNUSED)
{
  Widget w = (Widget) client_data;
  Cardinal n = 0;
  BEGINMESSAGE(catch_timer)
  action_page(w,NULL,NULL,&n);
  ENDMESSAGE(catch_timer)
}

void
action_page(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  int a,e,x,y,nx,ny,cw,ch,d;
  float f;
  Widget clip,gvw,aaa;
  int i;
  char *s;
  static String *old_params = NULL;
  static int old_num_params = 0;
  String *pars;
  int    npars;
  static XtIntervalId timer = 0;
  int page_changed = 0;
  int success=0;
  int    old;
  float dc = 2.4;
  int dp;
  int z=0;

  BEGINMESSAGE(action_page)

  if (timer) { XtRemoveTimeOut(timer); timer = 0; }

  if (w && (XtClass(w) != ghostviewWidgetClass)) {
     Widget p=w;
     gvw=NULL;
     while (p && !gvw) {
       if (XtIsSubclass(p,shellWidgetClass)) gvw = XtNameToWidget(p,"*page");
       p=XtParent(p);
     }
  } else gvw = w;
  aaa = XtParent(gvw);
  clip = XtParent(aaa);
  SMESSAGE(XtName(gvw)) SMESSAGE(XtName(aaa)) SMESSAGE(XtName(clip))
  if (strstr(XtName(aaa),"zoomControl")) z=1;

  cw = (int)clip->core.width;
  ch = (int)clip->core.height;

  i = -1;
  while (!success) {
    ++i;
    if (i > 0 && i == old_num_params) {
       INFMESSAGE(freeing old parameters)
       i = 0;
       while (i<old_num_params) XtFree(old_params[i++]);
       i = 0;
       XtFree((XtPointer)old_params);
       old_num_params=0;
       old_params = NULL;
    }
    if (i < old_num_params) {
      INFMESSAGE(using old parameters)
      pars = old_params;
      npars = old_num_params;
      old = 1;
    } else {
      INFMESSAGE(using new parameters)
      pars = params;
      npars = *num_params;
      old = 0;
    }
    if (i == npars) {
      INFMESSAGE(no more parameters)
      ENDMESSAGE(action_page)
      return;
    };

    a = e = dp = 0;
    s = pars[i];
    INFSMESSAGE(handling parameter,s)
    if (*s == '!') { e |= PAGE_NOT; s++; } 
    if      (!strcmp(s,"up"))               { a = PAGE_UP;                  e |= PAGE_V; }
    else if (!strcmp(s,"down"))             { a = PAGE_DOWN;                e |= PAGE_V; }
    else if (!strcmp(s,"left"))             { a = PAGE_LEFT;                e |= PAGE_H; }
    else if (!strcmp(s,"right"))            { a = PAGE_RIGHT;               e |= PAGE_H; }
    else if (!strcmp(s,"top"))              { a = PAGE_TOP;                 e |= PAGE_V; }
    else if (!strcmp(s,"bottom"))           { a = PAGE_BOTTOM;              e |= PAGE_V; }
    else if (!strcmp(s,"leftedge"))         { a = PAGE_LEFTEDGE;            e |= PAGE_H; }
    else if (!strcmp(s,"rightedge"))        { a = PAGE_RIGHTEDGE;           e |= PAGE_H; }
    else if (!z && !strcmp(s,"redisplay"))  { a = PAGE_REDISPLAY;           e |= 0; }
    else if (!strcmp(s,"center"))           { a = PAGE_CENTER;              e |= PAGE_V|PAGE_H; }
    else if (!strcmp(s,"right-top"))        { a = PAGE_RIGHT|PAGE_TOP;      e |= PAGE_H; }
    else if (!strcmp(s,"left-top"))         { a = PAGE_LEFT|PAGE_TOP;       e |= PAGE_H; }
    else if (!strcmp(s,"right-bottom"))     { a = PAGE_RIGHT|PAGE_BOTTOM;   e |= PAGE_H; }
    else if (!strcmp(s,"left-bottom"))      { a = PAGE_LEFT|PAGE_BOTTOM;    e |= PAGE_H; }
    else if (!strcmp(s,"down-leftedge"))    { a = PAGE_DOWN|PAGE_LEFTEDGE;  e |= PAGE_V; }
    else if (!strcmp(s,"up-leftedge"))      { a = PAGE_UP|PAGE_LEFTEDGE;    e |= PAGE_V; }
    else if (!strcmp(s,"down-rightedge"))   { a = PAGE_DOWN|PAGE_RIGHTEDGE; e |= PAGE_V; }
    else if (!strcmp(s,"up-rightedge"))     { a = PAGE_UP|PAGE_RIGHTEDGE;   e |= PAGE_V; }
    else if (!strcmp(s,"top-leftedge"))     { a = PAGE_TOP|PAGE_LEFTEDGE;   e |= PAGE_H|PAGE_V; }
    else if (!strcmp(s,"bottom-rightedge")) { a = PAGE_BOTTOM|PAGE_RIGHTEDGE; e |= PAGE_H|PAGE_V; }
    else if (!z && !strncmp(s,"page",4)) {
      s += 4;
      a = PAGE_PAGE; e |= PAGE_PAGE;
    }
    else if (!z && !strncmp(s,"highlight",9) && toc_text) {
      int p,aa=0,h,ho;
      s += 9;
      if (*s=='=') { s++; aa=1; }
      p = atoi(s);
      ho = h = VlistHighlighted(newtoc) + 1;
      if (!aa) {
	if (h<=0) h =  VlistSelected(newtoc) + 1;
	h = h + p - 1;
      } else {
        if (ho<0) h = p;
	else {
	  ho = 10*h + p - 1;
	  h = doc_putPageInRange(doc,ho);
	  if (h != ho) h = p - 1;
	}
      }
      h = doc_putPageInRange(doc,h);
      misc_setPageMarker(h,1,event,False);
    }

    nx = x = (int)aaa->core.x;
    ny = y = (int)aaa->core.y;

    if (a & PAGE_TOP)       ny = 0;
    if (a & PAGE_BOTTOM)    ny = -(int)aaa->core.height - 1000;
    if (a & PAGE_LEFTEDGE)  nx = 0;
    if (a & PAGE_RIGHTEDGE) nx = -(int)aaa->core.width - 1000;
    if (a & (PAGE_LEFT|PAGE_RIGHT)) {
      d = (int)aaa->core.width-cw;
      if (d > 0) {
	/* scrolling left or right */
	f = (dc*(float)d)/(float)cw;
        if (f<1) f = 1;
        d = (int)(((f/(float)abs((int)f))*(float)cw)/dc+1.0);
	if (a & PAGE_LEFT) nx = x + d;
	else               nx = x - d;
       }
    }
    if (a & (PAGE_UP|PAGE_DOWN)) {
       d = (int)aaa->core.height - ch;
       if (d>0) {
	 /* scrolling up or down */
	 f = (dc*(float)d)/(float)ch;
	 if (f<1) f = 1;
	 d = (int)(((f/(float)abs((int)f))*(float)ch)/dc+1.0);
	 if (a & PAGE_UP)   ny = y + d;
	 else               ny = y - d;
       }
    }
    if (a & PAGE_CENTER) {
      nx = -((int)aaa->core.width - cw)/2;
      ny = -((int)aaa->core.height - ch)/2;
    }

    if (a & PAGE_REDISPLAY) {
      if (XtIsSensitive(redisplayEntry)) {
	cb_redisplay(gvw,NULL,NULL);
      }
    }
    if (a & PAGE_PAGE) {
       int cp = current_page;
       cb_page((Widget)w,(XtPointer)s,(XtPointer)NULL);
       IIMESSAGE(cp,current_page)
       if (cp != current_page ||
           (!toc_text && atoi(s)>0)) page_changed = 1;
    }
    if (page_changed) {
      if (e & PAGE_PAGE) success = 1;
      if (!old) goto save_params;
    }

    if (e&(PAGE_H|PAGE_V) && scroll_success(clip,e,x,y,nx,ny)) {
      ClipWidgetSetCoordinates(clip,nx,ny);
      success = 1;
      if (!GhostviewIsBusy(gvw)) misc_drawEyeGuide(gvw,EYEGUIDE_DRAW,x,y);
    }
    if (e&PAGE_NOT) success = success ? 0 : 1;

#   ifdef MESSAGES
      if (success) { INFMESSAGE(command successful) }
      else { INFMESSAGE(command failed) }
#   endif
  }
  ENDMESSAGE(action_page)
  return;

save_params:
  INFMESSAGE(saving params)
  ++i;
  if (i<(int)*num_params) {
    old_num_params = *num_params - i;
    INFIMESSAGE(saving,old_num_params)
    old_params = (String*) XtMalloc(old_num_params*sizeof(String));
    old=0;
    while (i < (int)*num_params) {
      old_params[old] = XtNewString(params[i]);
      INFSMESSAGE(saving,old_params[old])
      ++i; old++;
    }
    timer = XtAppAddTimeOut(app_con,10,catch_timer,(XtPointer)w);
  }
  ENDMESSAGE(action_page)
}

/*##################################################################*/
/* action_menuPopdown */
/*##################################################################*/

void
action_menuPopdown(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  BEGINMESSAGE(action_menuPopdown)
  while (w) {
    w = XtParent(w);
    if (w && XtClass(w) == mbuttonWidgetClass) {
      ButtonUnset(w, event, params, num_params);
      w = (Widget)NULL;
    }
  }
  ENDMESSAGE(action_menuPopdown)
}

/*##################################################################*/
/* action_toc */
/*##################################################################*/

void
action_toc(Widget w _GL_UNUSED, XEvent *event, String *params, Cardinal *num_params)
{
  static int xf,yf,xo,yo,xp,yp,dyo;
  static Boolean scroll_initialized=False;
  static int startvisible;
  static Boolean scrolling=False;
  static int entryo = -1;
  Boolean toggle_mark=False;
  int entry,dy;
  Widget tocslider = newtocControl;
  Widget tocpanner = newtocClip;

  BEGINMESSAGE(action_toc)

  if (!strcmp(params[0],"scroll") && scroll_initialized) {
    int x,y,dh,ph;
    float deltay,m;
    ph = (int)tocpanner->core.height; if (ph<1) ph = 1;
    dh = ph - (int)tocslider->core.height;
    deltay = (float)(event->xbutton.y_root - yo);
    if (!app_res.reverse_scrolling) deltay = -deltay;
    m  = 1.3*((float)tocslider->core.height/(float)ph);
    if (m<=1) m = 1;
    y = yp -(int)(m*deltay);
    yo = (int) event->xbutton.y_root;

    ph = (int)tocpanner->core.width; if (ph<1) ph = 1;
    dh = ph - (int)tocslider->core.width;
    deltay = (float)(event->xbutton.x_root - xo);
    if (!app_res.reverse_scrolling) deltay = -deltay;
    m  = 1.3*((float)tocslider->core.width/(float)ph);
    if (m<=1) m = 1;
    x = xp +(int)(m*deltay);
    if (x<dh) x=dh; if (x>0) x=0;
    xo = (int) event->xbutton.x_root;

    if (x!=xp) {
      ClipWidgetSetCoordinates(tocpanner,x,0);
      xp = x;
    }
    if (y!=yp) {
      VlistMoveFirstVisible(newtoc, startvisible, y);
      yp = y;
    }
    if (abs(xf-xo) > 6 || abs(yf-yo) > 6) scrolling = True;
  }
  else if (!strcmp(params[0],"scrollon")) {
    scroll_initialized = True;
    scrolling = False;
    xp = (int) tocslider->core.x; xf = xo = (int) event->xbutton.x_root;
    yp = 0; yf = yo = (int) event->xbutton.y_root;
    startvisible = VlistGetFirstVisible(newtoc);
  }
  else if (!strcmp(params[0],"scrolloff")) {
    scroll_initialized = False;
    scrolling = False;
  }
  else if (!strcmp(params[0],"select")) {
    if (!scrolling && toc_text) {
      entry = VlistEntryOfPosition(newtoc,(int)event->xbutton.y);
      if (entry >=0 && entry < doc->numpages) {
	misc_setPageMarker(entry,0,event,True);
        show_page(entry,NULL);
      }
    }
  }
  else if (!strcmp(params[0],"toggleon")) {
    toggle_mark = True;
    dyo = 0;
    yo = (int) event->xbutton.y_root;    
  } else if (!strcmp(params[0],"toggleextend")) {
    entry = VlistEntryOfPosition(newtoc,(int)event->xbutton.y);
    dy = yo - (int) event->xbutton.y_root;
    if (dy*dyo < 0 || entry != entryo) toggle_mark = True;
    dyo = dy;
    yo = (int) event->xbutton.y_root;
  } else if (!strcmp(params[0],"toggleoff")) {
    entryo=-1;
  } else if (!strcmp(params[0],"highlight")) {
    entry = VlistEntryOfPosition(newtoc,(int)event->xbutton.y);
    if (entry != VlistHighlighted(newtoc))
      VlistChangeHighlighted(newtoc,entry,XawVlistSet);
  } else if (!strcmp(params[0],"unhighlight")) {
    entry = VlistEntryOfPosition(newtoc,(int)event->xbutton.y);
    VlistChangeHighlighted(newtoc,entry,XawVlistUnset);
  }
  
  if (toggle_mark) {
    int st;
    scroll_initialized = False;
    entry = VlistEntryOfPosition(newtoc,(int)event->xbutton.y);
    if (entryo==-1 || entryo==entry) entryo=entry-1;
    if (entry<=entryo) st = -1;
    else st = 1;
    while (entryo != entry) {
      entryo += st;
      if (entryo>=0) VlistChangeMark(newtoc,entryo,XawVlistToggle);
    }
  }
  ENDMESSAGE(action_toc)
}

/*############################################################*/
/* action_otherPage */
/*############################################################*/

void
action_otherPage(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  static int xo=0,yo=0;
  static Time to = 0;
  int x,y,vpwx,vpwy;
  Dimension vpww;
  Time t;
  Position px,py;
  Arg args[1];
  Widget vpw;
 
  BEGINMESSAGE1(action_otherPage)

  if (event->type != ButtonRelease || w != page) {
    INFMESSAGE1(event not a button release or not in main window)
    ENDMESSAGE1(action_otherPage)
    return;
  }

  vpw = XtParent(XtParent(w));
  XtTranslateCoords(vpw, 0, 0, &px, &py);
  vpwx = (int)px;
  vpwy = (int)py;
  x = ((int) event->xbutton.x_root) - vpwx + 1;
  y = ((int) event->xbutton.y_root) - vpwy + 1;
  t = ((XMotionEvent*)event)->time;
#if 0
  printf("to=%d xo=%d yo=%d\n",(int)to,xo,yo);
  printf("t= %d x= %d y= %d\n",(int)t,x,y);
#endif 
  if (t - to < 400) {
    if (xo - x < 2 && yo - y < 2) {
      XtSetArg(args[0], XtNwidth,  &vpww);
      XtGetValues(vpw, args, ONE);
      if (2*x<(int)vpww) cb_page(w,(XtPointer)"-1",(XtPointer)NULL);
      else               cb_page(w,(XtPointer)"+1",(XtPointer)NULL);
    }
  }
  to = t;
  xo = x;
  yo = y;
  ENDMESSAGE1(action_otherPage)
}

/*############################################################*/
/* action_movePage */
/*############################################################*/

void
action_movePage(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
#   define HISTORY_POINTS 10
#   define DECAY_TIME 200
    int x,y;
    static int posx[HISTORY_POINTS+1],posix;
    static int posy[HISTORY_POINTS+1],posiy;
    static int xp,yp,pw,ph,pvw,pvh;
    static Bool initialized=False;
    static Time to;
    Widget vpw;

    BEGINMESSAGE1(action_movePage)

    if (XtClass(w) != ghostviewWidgetClass) {
       INFMESSAGE1(not a ghostview widget) ENDMESSAGE1(action_movePage)
       return;
    }
    vpw = XtParent(XtParent(w));
 
    if (*num_params) {
       if (!strcmp(params[0],"adjusted")) { /* called by cb_pageAdjustNotify */
          XawPannerReport *rep= (XawPannerReport*) params[1];
          INFMESSAGE(adjusting)
	  xp  = (int) (-rep->slider_x);
	  yp  = (int) (-rep->slider_y);
          pw  = (int) (rep->canvas_width);
          ph  = (int) (rep->canvas_height);
          pvw = (int) (rep->slider_width);
          pvh = (int) (rep->slider_height);
          IIMESSAGE1(xp,yp) IIMESSAGE1(pw,ph) IIMESSAGE1(pvw,pvh)
          initialized = True;
       } else if (!strcmp(params[0],"move")) {
          INFMESSAGE(moving)
          if (event->type != MotionNotify) goto break_movePage;
          if (initialized==True && pvw && pvh) {
             int dx,dy;
             double relfactor=1.0;  /* some default value */
             double absfactor=0.0;  /* some default value */
             x = (int) event->xbutton.x_root;
             y = (int) event->xbutton.y_root;

             if ((((XMotionEvent*)event)->time - to) > DECAY_TIME) {
                if (posix>0) { posx[0]=posx[posix]; posix=0; }
                if (posiy>0) { posy[0]=posy[posiy]; posiy=0; }
             }
             if (posix>0 && (x-posx[posix])*(posx[posix]-posx[posix-1]) < 0) {
                               posx[0]=posx[posix]; posix=0;
             }
             if (posiy>0 && (y-posy[posiy])*(posy[posiy]-posy[posiy-1]) < 0) {
                               posy[0]=posy[posiy]; posiy=0;
             }
             to = ((XMotionEvent*)event)->time;
             ++posix;
             ++posiy;

             if (posix>HISTORY_POINTS) {
                posix=1;
                while (posix<=HISTORY_POINTS) { posx[posix-1]=posx[posix]; posix++; }
                posix=HISTORY_POINTS;
             }
             posx[posix] = x;
             if (posiy>HISTORY_POINTS) {
                posiy=1;
                while (posiy<=HISTORY_POINTS) { posy[posiy-1]=posy[posiy]; posiy++; }
                posiy=HISTORY_POINTS;
             }
             posy[posiy] = y;

             dx = (x - posx[0])/(posix);
             dy = (y - posy[0])/(posiy);
#if 0
             printf("time=%d x=%d y=%d dx=%d dy=%d\n",(int)to,x,y,dx,dy);
             printf("posix=%d posx[posix]=%d posx[0]=%d\n",posix,posx[posix],posx[0]);
             printf("posiy=%d posy[posiy]=%d posy[0]=%d\n",posiy,posy[posiy],posy[0]);
#endif
             if (dx || dy) {
                if (*num_params>=2) relfactor = atof((char*)(params[1]));
                relfactor = relfactor >= 0 ? (relfactor<=100 ? relfactor : 100) : 0;
                if (*num_params>=3) absfactor = atof((char*)(params[2]));
                absfactor = absfactor >= 0 ? (absfactor<=200 ? absfactor : 200) : 0;
                DDMESSAGE1(absfactor,relfactor)
                if (app_res.reverse_scrolling) { dx = -dx; dy = -dy; }
                xp = (int) (xp-(dx*absfactor)-(relfactor*pw*dx)/pvw);
                yp = (int) (yp-(dy*absfactor)-(relfactor*ph*dy)/pvh);
                ClipWidgetSetCoordinates(vpw,xp,yp);
             }
          }
       } else if (!strcmp(params[0],"start")) {
           Position positx,posity;
           Arg args[2];
           INFMESSAGE(start)
           if (event->type != ButtonPress) goto break_movePage;
           initialized = False;
           gv_scroll_mode = SCROLL_MODE_GHOSTVIEW;
           XtSetArg(args[0], XtNx, (Position*)&positx);
           XtSetArg(args[1], XtNy, (Position*)&posity);
           XtGetValues(XtParent(w), args, TWO);
           xp = (int)positx; yp = (int)posity;
           posix=posiy=0;
           posx[0] = (int) event->xbutton.x_root;
           posy[0] = (int) event->xbutton.y_root;
           to = ((XMotionEvent*)event)->time;
           IIMESSAGE1(xp,yp)
           ClipWidgetSetCoordinates(vpw,xp,yp);
       } else if (!strcmp(params[0],"stop")) {
           INFMESSAGE(stop)
           gv_scroll_mode = SCROLL_MODE_NONE;
           initialized = False;
       }
    }
    ENDMESSAGE1(action_movePage)
    return;

break_movePage:
    INFMESSAGE1(interrupting due to wrong event type)
    initialized = False;
    gv_scroll_mode = SCROLL_MODE_NONE;
    ENDMESSAGE1(action_movePage) return;
}

/*##################################################################*/
/* action_panner */
/*##################################################################*/

void
action_panner(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    int x,y,cw,ch;
    static int xo,yo,xp,yp;
    static Bool initialized=False;

    BEGINMESSAGE(action_panner)

    if (!strcmp(params[0],"move") && initialized) {
       INFMESSAGE1(move)
       y = yp + (int)event->xbutton.y_root - yo;
       ch = (int)w->core.height-(int)slider->core.height;
       if (y>ch) y=ch; else if (y<0) y=0;
       x = xp + (int)event->xbutton.x_root - xo;
       cw = (int)w->core.width-(int)slider->core.width;
       if (x>cw) x=cw; else if (x<0) x=0;
       if (x!=xp || y!=yp) {
          int pxp,pyp,dw,dh;
          XtMoveWidget(slider,x,y);
          IIMESSAGE(x,y) IIMESSAGE(xp,yp) IIMESSAGE(xo,yo)
          dw = (int)viewControl->core.width  - (int)viewClip->core.width;
          dh = (int)viewControl->core.height - (int)viewClip->core.height;
          if (cw) pxp = (x*dw+cw/2)/cw; else pxp = 0;
          if (ch) pyp = (y*dh+ch/2)/ch; else pyp = 0;
          IIMESSAGE(x,y) IIMESSAGE(xp,yp) IIMESSAGE(xo,yo) IIMESSAGE(pxp,pyp)
          ClipWidgetSetCoordinates(viewClip,-pxp,-pyp);
          xp = x; xo = (int) event->xbutton.x_root;
          yp = y; yo = (int) event->xbutton.y_root;
       }
    }
    else if (strcmp(params[0],"on") == 0) {
       INFMESSAGE(on)
       gv_scroll_mode = SCROLL_MODE_PANNER;
       initialized = True;
       xp = (int) slider->core.x; xo = (int) event->xbutton.x_root;
       yp = (int) slider->core.y; yo = (int) event->xbutton.y_root;
    }
    else if (strcmp(params[0],"off") == 0) {
       INFMESSAGE(off)
       gv_scroll_mode = SCROLL_MODE_NONE;
       initialized = False;
    }
    ENDMESSAGE(action_panner)
}

/*##################################################################*/
/* action_handleDSC */
/* Call the cb_handleDSC callback */
/*##################################################################*/

void
action_handleDSC(Widget w, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_handleDSC)
  cb_handleDSC(w, (XtPointer)1, NULL);
  ENDMESSAGE(action_handleDSC)
}

/*##################################################################*/
/* action_antialias */
/* Call the cb_antialias callback */
/*##################################################################*/

void
action_antialias(Widget w, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_antialias)
  cb_antialias(w, (XtPointer)1, NULL);
  ENDMESSAGE(action_antialias)
}

/*##################################################################*/
/* action_quit */
/* Call the quit callback to stop ghostview */
/*##################################################################*/

void
action_quit(Widget w, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_quit)
  cb_quitGhostview(w, NULL, NULL);
  ENDMESSAGE(action_quit)
}

/*##################################################################*/
/* action_open */
/* Popup the open file dialog box. */
/*##################################################################*/

void
action_open(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_open)
  cb_openFile((Widget)NULL,(XtPointer)NULL, NULL);
  ENDMESSAGE(action_open)
}

/*##################################################################*/
/* action_reopen */
/*##################################################################*/

void
action_reopen(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_reopen)
  if (!XtIsSensitive(reopenEntry)) {INFMESSAGE(insensitive) ENDMESSAGE(action_reopen) return; }
  cb_reopen((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);
  ENDMESSAGE(action_reopen)
}

/*##################################################################*/
/* action_savepos */
/*##################################################################*/

void
action_savepos(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_savepos)
  if (!XtIsSensitive(saveposEntry)) {INFMESSAGE(insensitive) ENDMESSAGE(action_savepos) return; }
  cb_savepos((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);
  ENDMESSAGE(action_savepos)
}

/*##################################################################*/
/* action_presentation */
/*##################################################################*/

void
action_presentation(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_savepos)
  if (!XtIsSensitive(presentationEntry)) {INFMESSAGE(insensitive) ENDMESSAGE(action_presentation) return; }
  cb_presentation((Widget)NULL,(XtPointer)NULL,(XtPointer)NULL);
  ENDMESSAGE(action_presentation)
}

/*##################################################################*/
/* action_save */
/* Popup the save file dialog box. */
/*##################################################################*/

void
action_save(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{
    BEGINMESSAGE(action_save)
    if (*num_params<1)  {
       INFMESSAGE(no parameter)
       ENDMESSAGE(action_save)
       return;
    }
    SMESSAGE(params[0])
    if (!strcmp(params[0],"marked")) {
       INFMESSAGE(saving marked)
       if (!XtIsSensitive(saveMarkedEntry)) {
          INFMESSAGE(save denied)
          ENDMESSAGE(action_save)return;
       }
       cb_save((Widget)NULL,(XtPointer)(PAGE_MODE_CURRENT|PAGE_MODE_MARKED),NULL);
    } else if (!strcmp(params[0],"all")) {
       INFMESSAGE(saving all)
       if (!XtIsSensitive(saveAllEntry)) {
          INFMESSAGE(save denied)
          ENDMESSAGE(action_save)
          return;
       }
       cb_save((Widget)NULL,(XtPointer)(PAGE_MODE_ALL),NULL);
    }
    ENDMESSAGE(action_save)
}

/*##################################################################*/
/* action_print */
/* Popup the print file dialog box. */
/*##################################################################*/

void
action_print(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{

    BEGINMESSAGE(action_print)
    if (*num_params<1)  {
       INFMESSAGE(no parameter)
       ENDMESSAGE(action_print)
       return;
    }
    if (!strcmp(params[0],"marked")) {
       if (!XtIsSensitive(printMarkedEntry)) {
          INFMESSAGE(print denied)
          ENDMESSAGE(action_print)
          return;
       }
       cb_print((Widget)NULL,(XtPointer)(PAGE_MODE_CURRENT|PAGE_MODE_MARKED),NULL);
    } else if (!strcmp(params[0],"all")) {
       if (!XtIsSensitive(printAllEntry)) {
          INFMESSAGE(print denied)
          ENDMESSAGE(action_print)
          return;
       }
       cb_print((Widget)NULL,(XtPointer)PAGE_MODE_ALL,NULL);
    }
    ENDMESSAGE(action_print)
}

void
action_print_pos(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{

    BEGINMESSAGE(action_print_pos)
    cb_print_pos((Widget)NULL,0,NULL);
    ENDMESSAGE(action_print_pos)
}

/*##################################################################*/
/* action_setPageMark */
/* Call the cb_setPageMark callback */
/*##################################################################*/

void
action_setPageMark(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{
    char *current   = "current";
    char *even      = "even";
    char *odd       = "odd";
    char *mark      = "mark";
    char *toggle    = "toggle";
    int spm=0;

    BEGINMESSAGE(action_setPageMark)
    if (*num_params<2) { INFMESSAGE(no parameters)ENDMESSAGE(action_setPageMark)return; }

    if (!strcmp(params[0],even))           spm=spm|SPM_EVEN;
    else if (!strcmp(params[0],odd))       spm=spm|SPM_ODD;
    else if (!strcmp(params[0],current))   spm=spm|SPM_CURRENT;
    else                                   spm=spm|SPM_ALL;
    if      (!strcmp(params[1],toggle))    spm=spm|SPM_TOGGLE;
    else if (!strcmp(params[1],mark))      spm=spm|SPM_MARK;
    else                                   spm=spm|SPM_UNMARK;
   
    cb_setPageMark((Widget)NULL,(XtPointer)(intptr_t)spm,NULL);

    ENDMESSAGE(action_setPageMark)
}

/*##################################################################*/
/* action_autoResize */
/* Call the cb_autoResize callback */
/*##################################################################*/

void
action_autoResize(Widget w, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
    BEGINMESSAGE(action_autoResize)
    cb_autoResize(w, (XtPointer)1, NULL);
    ENDMESSAGE(action_autoResize)
}

/*##################################################################*/
/* action_setScale */
/*##################################################################*/

void
action_setScale(Widget w, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{
    int i;

    BEGINMESSAGE(action_setScale)
    if (*num_params < 1) {
      INFMESSAGE(no parameters)
      ENDMESSAGE(action_setScale)
      return;
    }
    if (!strcmp(params[0],"+"))      i = (gv_scale+1)|SCALE_ABS;
    else if (!strcmp(params[0],"-")) i = (gv_scale-1)|SCALE_ABS;
    else {
      i = atoi(params[0]);
      if (i<0) i = (-i)|SCALE_MIN;
      i |= SCALE_REL;
    }
    cb_setScale(w, (XtPointer)(intptr_t)i, NULL);
    ENDMESSAGE(action_setScale)
}

/*##################################################################*/
/* action_setOrientation */
/* Set orientation action routine.  Converts text parameter
 * to XtPageOrientation and calls cb_setOrientation callback */
/*##################################################################*/

void
action_setOrientation(Widget w, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{
    int o;

    BEGINMESSAGE(action_setOrientation)
    if (*num_params != 1) {
       INFMESSAGE(no parameter)
       ENDMESSAGE(action_set_orientation)
       return;
    }
    o = doc_convStringToDocOrient(params[0]);
    if (o != O_UNSPECIFIED) cb_setOrientation(w, (XtPointer)(intptr_t)o, NULL);
    ENDMESSAGE(action_setOrientation)
}

/*##################################################################*/
/* action_setPagemedia */
/* Set pagemedia action routine.  Converts text parameter
 * to index into the pagemedia widgets and calls the cb_setPagemedia
 * callback. */
/*##################################################################*/

void
action_setPagemedia(Widget w, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{
    int m;

    BEGINMESSAGE(action_setPagemedia)
    if (*num_params != 1) {
       INFMESSAGE(no parameter)
       ENDMESSAGE(action_set_pagemedia) 
       return;
    }

    m = doc_convStringToPageMedia(doc,params[0]);
    if (m!= MEDIA_ID_INVALID) cb_setPagemedia(w, (XtPointer)(intptr_t)m, NULL);

    ENDMESSAGE(action_setPagemedia)
}

/*##################################################################*/
/* action_dismissPopup */
/* dismiss a popup window */
/*##################################################################*/

#define IS_ZOOM(sss) (!strcmp(XtName(sss),"zoomPopup"))
void
action_dismissPopup(Widget w, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
   Widget s;
   BEGINMESSAGE(action_dismissPopup)
   if (w) {
      INFSMESSAGE(calling widget:,XtName(w))
      if (XtClass(w) == aaaWidgetClass)	s = XtParent(w);
      else					s = w;
      if      (s==infopopup)       {INFMESSAGE(infopopup)     cb_popdownInfoPopup((Widget)NULL,NULL,NULL);    }
      else if (s==dialogpopup)     {INFMESSAGE(dialogpopup)   cb_popdownDialogPopup((Widget)NULL,NULL,NULL);  }
      else if (s==notepopup)       {INFMESSAGE(notepopup)     cb_popdownNotePopup((Widget)NULL,NULL,NULL);    }
      else if (s==gv_options_fs->popup)    { INFMESSAGE(options fs)    options_cb_popdown((Widget)NULL,(XtPointer)gv_options_fs,NULL);  }
      else if (s==gv_options_gs->popup)    { INFMESSAGE(options gs)    options_cb_popdown((Widget)NULL,(XtPointer)gv_options_gs,NULL);  }
      else if (s==gv_options_gv->popup)    { INFMESSAGE(options gv)    options_cb_popdown((Widget)NULL,(XtPointer)gv_options_gv,NULL);  }
      else if (s==gv_options_setup->popup) { INFMESSAGE(options setup) options_cb_popdown((Widget)NULL,(XtPointer)gv_options_setup,NULL);  }
      else if (s==versionpopup)    {INFMESSAGE(versionpopup)  cb_popdownVersionPopup((Widget)NULL,NULL,NULL); }
      else if (s==FileSel_popup)   {INFMESSAGE(Filesel_popup) XtPopdown(s);				    }
      else if IS_ZOOM(s)           {INFMESSAGE(zoomPopup)     XtDestroyWidget(s);                   }
   }
   ENDMESSAGE(action_dismissPopup)
}

/*##################################################################*/
/* action_deleteWindow */
/* Implement WM_DELETE_WINDOW protocol */
/*##################################################################*/

void
action_deleteWindow(Widget w, XEvent *event, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
   BEGINMESSAGE(action_deleteWindow)
   if (w) {
      INFSMESSAGE(widget:,XtName(w))
      if (event->type == ClientMessage && (Atom)event->xclient.data.l[0] == wm_delete_window) {
         if (w==toplevel)     { INFMESSAGE(toplevel) cb_quitGhostview((Widget)NULL,NULL,NULL); }
         else                 { action_dismissPopup(w,NULL,NULL,NULL); }
      }
   }
   ENDMESSAGE(action_deleteWindow)
}

/*##################################################################*/
/* action_eraseLocator */
/* Pop down locator window */
/*##################################################################*/

void
action_eraseLocator(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
    Arg args[1];

    BEGINMESSAGE1(action_eraseLocator)
    if (!show_locator) {INFMESSAGE1(no locator)ENDMESSAGE1(action_erase_locator)return;}
    XtSetArg(args[0], XtNlabel, "");
    XtSetValues(locator, args, ONE);
    ENDMESSAGE1(action_eraseLocator)
}

/*##################################################################*/
/* action_checkFile */
/* Check to see if file changed */
/*##################################################################*/

void
action_checkFile(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params, Cardinal *num_params)
{
    BEGINMESSAGE(action_checkFile)
    if (*num_params != 1) {INFMESSAGE(no parameters) ENDMESSAGE(action_checkFile) return;}
    if (!strcmp(params[0],"date"))
       cb_checkFile((Widget)NULL,(XtPointer)CHECK_FILE_DATE,NULL);
    else if (!strcmp(params[0],"version"))
       cb_checkFile((Widget)NULL,(XtPointer)CHECK_FILE_VERSION,NULL);
    ENDMESSAGE(action_checkFile)
}

/*##################################################################*/
/* action_watchFile */
/*##################################################################*/

void
action_watchFile(Widget w _GL_UNUSED, XEvent *event _GL_UNUSED, String *params _GL_UNUSED, Cardinal *num_params _GL_UNUSED)
{
  BEGINMESSAGE(action_watchFile)
  cb_watchFile(NULL,(XtPointer)1,NULL);
  ENDMESSAGE(action_watchFile)
}



void
clean_safe_tempdir(void)
{
   if (gv_safe_gs_tempdir)
   {
      chdir("/");
      rmdir(gv_safe_gs_workdir);
   }
}
