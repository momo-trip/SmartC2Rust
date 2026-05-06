/*
**
** Aaa_intern.h
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
 
#ifndef	_AAA_INTERN_H_
#define	_AAA_INTERN_H_

/*
#define MESSAGES
#define MEMDEBUG
*/

/*---------------------------------------------------------*/
/* Include */
/*---------------------------------------------------------*/

#include    <stdio.h>
#include    <stdlib.h>

#include    "paths.h"
#include    INC_X11(Xlib.h)
#include    INC_X11(Xresource.h)
#include    INC_X11(IntrinsicP.h)
#include    INC_X11(cursorfont.h)
#include    INC_X11(StringDefs.h)

#include    INC_XMU(Misc.h)
#include    INC_XMU(Converters.h)
#define DESTROY_EXPLICITLY
#include    "AaaP.h"

#ifdef MEMDEBUG
#   define MESSAGES
#endif

#include    "message.h"

#ifdef MEMDEBUG
#   define MEMMESSAGE(sss)      INFMESSAGE(sss)
#   define MEMBEGINMESSAGE(sss) BEGINMESSAGE(sss)
#   define MEMENDMESSAGE(sss)   ENDMESSAGE(sss)
#else
#   define MEMMESSAGE(sss)
#   define MEMBEGINMESSAGE(sss)
#   define MEMENDMESSAGE(sss)
#endif

/*---------------------------------------------------------*/
/* Bison Configuration */
/*---------------------------------------------------------*/

typedef union {
    int		    ival;
    XrmQuark	    qval;
    BoxPtr	    bval;
    BoxParamsPtr    pval;
    GlueRec	    gval;
    LayoutDirection lval;
    ExprPtr	    eval;
    Operator	    oval;
} YYSTYPE;

typedef struct _LayoutConverterStruct {
   String    base;
   String    oldfrom;
   String    from;
   LayoutPtr *to;
}  LayoutConverterStruct, *LayoutConverterArg;

#define YYPARSE_PARAM  layoutConverterArg 
#define YYLEX_PARAM    layoutConverterArg

/* ###todo###
   Currently I have no time to take care of alloca used in the dynamic stack
   extension, therefore we just show a polite message and exit in case a stack
   overflow occurs ###jp###.
   By the way, YYINITDEPTH=200 is already pretty much, I never got a stack size
   larger than 40 with the layouts I use. ###jp###   
*/
#define YYINITDEPTH 200
#define yyoverflow(a1,a2,a3,a4,a5,a6)				\
{								\
   fprintf(stderr,"yyoverflow: stack overflow\n");		\
   fprintf(stderr,"   maximal stacksize: %d\n",yystacksize);	\
   fprintf(stderr,"   used             : %d\n",size);		\
   fprintf(stderr,"Overflow forces image exit, bye bye.\n");	\
   exit(0);						        \
}

extern int layout_parse (void *);
extern void layout_scan_error (char *, void *);
extern int layout_lex (YYSTYPE *, void *);

#define layout_error(sss)       layout_scan_error(sss,YYLEX_PARAM)

/*---------------------------------------------------------*/
/* Macros */
/*---------------------------------------------------------*/

#define New(ttt)      (ttt *) XtMalloc(sizeof(ttt))
#define Dispose(xxx)  XtFree((XtPointer)xxx)
#define Some(ttt,nnn) (ttt *) XtMalloc(sizeof(ttt)*nnn)
#define More(xxx,ttt,nnn)   (			\
 (xxx) ?					\
 (ttt *) XtRealloc(xxx,sizeof(ttt)*nnn) :	\
 (ttt *) XtMalloc(sizeof(ttt)*nnn)		\
)

#define GlueEqual(a,b)			\
 ( (a).order == (b).order && (a).value == (b).value )

#define AddGlue(r,a,b)			\
   if (a.order == b.order) {		\
      r.order = a.order;		\
      r.value = a.value + b.value;	\
   } else {				\
      if (a.order > b.order) r = a;	\
      else                   r = b;	\
   }

#define MinGlue(r,b)				\
   if (r.order == b.order) {			\
      if (r.value > b.value) r.value = b.value;	\
   } else {					\
      if (r.order > b.order) r = b;		\
   }

#define SubGlue(r,a,b)			\
   if (a.order == b.order) {		\
      r.order = a.order;		\
      r.value = a.value - b.value;	\
   } else {				\
      if (a.order > b.order)		\
         r = a;				\
      else {				\
         r.order = b.order;		\
         r.value = -b.value;		\
      }					\
   }

#define ZeroGlue(g)	((g).value = 0, (g).order = 0, (g).expr = 0)
#define IsZeroGlue(g)	((g).value == 0)

#define QuarkToWidget(l,q)  XtNameToWidget((Widget) l, (char *) XrmQuarkToString(q))

#endif /* _AAA_INTERN_H_ */
