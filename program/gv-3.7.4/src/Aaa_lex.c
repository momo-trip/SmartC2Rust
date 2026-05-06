/*
**
** Aaa_lex.c
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
#include "ac_config.h"

/*
#define MESSAGES
*/

/*
#define TEST_SCAN
*/
#ifdef TEST_SCAN
#   define MESSAGES
#endif

#include "Aaa_intern.h"
#include "Aaa_bison.h"

#define RETURN(rrr) { r = rrr; goto scan_end; }
#define WORD_MAXLEN 20

#define ERROR                  -1
#define END                    0
#define EOS                    '\0'

#define KEYWORD_START          '|'
#define INF_START              '['
#define INF_END                ']'
#define KEYWORD_WIDTH          "width"
#define KEYWORD_HEIGHT         "height"
#define KEYWORD_HBOX           "h"
#define KEYWORD_VBOX           "v"
#define KEYWORD_OF             "of"

#define IS_KEYWORD_WIDTH(sss)  (!strcmp(sss,KEYWORD_WIDTH))
#define IS_KEYWORD_HEIGHT(sss) (!strcmp(sss,KEYWORD_HEIGHT))
#define IS_KEYWORD_HBOX(sss)   (!strcmp(sss,KEYWORD_HBOX))
#define IS_KEYWORD_VBOX(sss)   (!strcmp(sss,KEYWORD_VBOX))
#define IS_KEYWORD_OF(sss)     (!strcmp(sss,KEYWORD_OF))

#define IS_OC(ccc)             ((ccc) == '{')
#define IS_CC(ccc)             ((ccc) == '}')
#define IS_OP(ccc)             ((ccc) == '(')
#define IS_CP(ccc)             ((ccc) == ')')
#define IS_OA(ccc)             ((ccc) == '<')
#define IS_CA(ccc)             ((ccc) == '>')
#define IS_EQUAL(ccc)          ((ccc) == '=')
#define IS_DOLLAR(ccc)         ((ccc) == '$')
#define IS_PLUS(ccc)           ((ccc) == '+')
#define IS_MINUS(ccc)          ((ccc) == '-')
#define IS_TIMES(ccc)          ((ccc) == '*')
#define IS_DIVIDE(ccc)         ((ccc) == '/')
#define IS_PERCENT(ccc)        ((ccc) == '%')
#define IS_INF_START(ccc)      ((ccc) == INF_START)
#define IS_INF_END(ccc)        ((ccc) == INF_END)
#define IS_KEYWORD_START(ccc)  ((ccc) == KEYWORD_START)

#define IS_DIGIT(ccc)          ((ccc)>='0' && ((ccc) <= '9'))
#define IS_SPACE(ccc)          ((ccc)==' ' || (ccc)=='\n' || (ccc) =='\t')
#define IS_EOS(ccc)            ((ccc)== EOS)
#define IS_CHAR(ccc)           (((ccc) >='A' && (ccc)<='Z') || ((ccc) >='a' && (ccc)<='z'))
#define IS_MIXED(ccc)          (IS_CHAR((ccc)) || IS_DIGIT((ccc)) || (ccc)=='_')

void
layout_scan_error(s,layoutConverterStructP)
    char    *s;
    void *layoutConverterStructP;
{
    LayoutConverterArg layoutConverterArg = (LayoutConverterArg) (layoutConverterStructP);
    char    *t;
    char    *c,*base;
    int have_space;

    base = (char*) layoutConverterArg->base;
    c    = (char*) layoutConverterArg->oldfrom;

    t = c - 35;
    if (t < base) t = base;
    have_space=0;
    while (*t && t < c + 35) {
       if (IS_SPACE(*t)) have_space=1;
       else {
          if (have_space) fputc(' ', stderr);
          fputc(*t, stderr);
          have_space=0;
       }
       t++;
    }
    if (!*t) fprintf(stderr,"<EOF>");
    fprintf(stderr,"\n");
    t = c - 35;
    have_space=0;
    if (t < base) t = base;
    while (*t && t < c + 35) {
       if (IS_SPACE(*t)) have_space=1;
       else {
          if (have_space) fputc(' ', stderr);
          if (t == c) { fputc('^',stderr); break; }
          else        { fputc(' ',stderr); }
          have_space=0;
       }
       t++;
    }
    if (!*t) fputc('^',stderr);
    fprintf(stderr,"\n");
    fprintf(stderr,"%s\n", s);
}


static void
maxlen_exceeded(void *layoutConverterStructP)
{
   layout_scan_error("Expression too long.",layoutConverterStructP);
}

int
layout_lex(v,layoutConverterStructP)
   YYSTYPE *v;
   void *layoutConverterStructP;
{
   char *c,*oldc;
   LayoutConverterArg layoutConverterArg = (LayoutConverterArg) (layoutConverterStructP);
   int r;

   BEGINMESSAGE1(layout_lex)

   oldc = c = (char*) layoutConverterArg->from;

   if (!c) {
      INFMESSAGE(layout_lex: received null pointer) ENDMESSAGE1(layout_lex)
      return (END);
   }

scan_start:

   while (IS_SPACE(*c)) c++;

   layoutConverterArg->oldfrom = (String) c;

        if (IS_EOS(*c))       RETURN(END)
   else if (IS_OC(*c))        RETURN(OC)
   else if (IS_CC(*c))        RETURN(CC)
   else if (IS_OP(*c))        RETURN(OP)
   else if (IS_CP(*c))        RETURN(CP)
   else if (IS_OA(*c))        RETURN(OA)
   else if (IS_CA(*c))        RETURN(CA)
   else if (IS_EQUAL(*c))     RETURN(EQUAL)
   else if (IS_DOLLAR(*c))    RETURN(DOLLAR)
   else if (IS_PLUS(*c))      { v->oval = Plus;   RETURN(PLUS)   }
   else if (IS_MINUS(*c))     { v->oval = Minus;  RETURN(MINUS)  }
   else if (IS_TIMES(*c))     { v->oval = Times;  RETURN(TIMES)  }
   else if (IS_DIVIDE(*c))    { v->oval = Divide; RETURN(DIVIDE) }
   else if (IS_PERCENT(*c))   {
        char *cold = c;
        ++c; while (IS_SPACE(*c)) ++c;
        if (IS_CHAR(*c)) {
           char s[WORD_MAXLEN+1]; int i=0;
           while (IS_CHAR(*c) && i<WORD_MAXLEN) { s[i++] = *c++; } c--; s[i] = EOS;
           if (i==WORD_MAXLEN) { maxlen_exceeded(layoutConverterStructP); RETURN(ERROR) }
           if (IS_KEYWORD_OF(s)) {
              v->oval = Percent;
              RETURN(PERCENTOF)
           }
        }
        c = cold;
        v->oval = Percent;
        RETURN(PERCENT)
   }
   else if (IS_INF_START(*c)) {
        char s[WORD_MAXLEN+1]; int i=0;
        c++; while (IS_DIGIT(*c) && i<WORD_MAXLEN) { s[i++] = *c++; } s[i] = EOS;
        if (i==WORD_MAXLEN) { maxlen_exceeded(layoutConverterStructP); RETURN(ERROR) }
        if (IS_INF_END(*c)) {
           v->ival = atoi(s);
           RETURN(INFINITY)
        } else {
           layout_scan_error("No match.",layoutConverterStructP);
           RETURN(ERROR)
        }
   }
   else if (IS_DIGIT(*c)) {
        char s[WORD_MAXLEN+1]; int i=0;
        while (IS_DIGIT(*c) && i<WORD_MAXLEN) { s[i++] = *c++; } c--; s[i] = EOS;
        if (i==WORD_MAXLEN) { maxlen_exceeded(layoutConverterStructP); RETURN(ERROR) }
        v->ival = atoi(s);
        RETURN(NUMBER)
   }
   else if (IS_KEYWORD_START(*c)) {
        char s[WORD_MAXLEN+1]; int i=0;
        c++; while (IS_SPACE(*c)) c++;
        while (i<WORD_MAXLEN && IS_CHAR(*c))
           { s[i++] = *c++; } c--; s[i] = EOS;
        if (i==WORD_MAXLEN) { maxlen_exceeded(layoutConverterStructP); RETURN(ERROR) }
        if      (IS_KEYWORD_HBOX(s))   RETURN(HORIZONTAL)
        else if (IS_KEYWORD_VBOX(s))   RETURN(VERTICAL)
        else if (IS_KEYWORD_WIDTH(s))  RETURN(WIDTH)
        else if (IS_KEYWORD_HEIGHT(s)) RETURN(HEIGHT)
        else if (s[0]==EOS) {
           layout_scan_error("Missing keyword.",layoutConverterStructP);
           RETURN(ERROR)
        } else {
           layout_scan_error("Unrecognized keyword.",layoutConverterStructP);
           RETURN(ERROR)
        }
   }
   else if (IS_MIXED(*c)) {
        int i=0; char s[WORD_MAXLEN+1];
        do { s[i++] = *c++; } while (i<WORD_MAXLEN && IS_MIXED(*c)); c--; s[i] = EOS;
        if (i==WORD_MAXLEN) { maxlen_exceeded(layoutConverterStructP); RETURN(ERROR) }
        v->qval = XrmStringToQuark (s);
        RETURN(NAME)
   }
   else {
        layoutConverterArg->oldfrom = (String) c;
        layout_scan_error("Ignoring character.",layoutConverterStructP);
        c++;
        goto scan_start;
   }

scan_end:

#ifdef MESSAGES
   switch (r) {
   case OC:
      INFMESSAGE(layout_lex: OC)
      break;
   case CC:
      INFMESSAGE(layout_lex: CC)
      break;
   case OA:
      INFMESSAGE(layout_lex: OA)
      break;
   case CA:
      INFMESSAGE(layout_lex: CA)
      break;
   case OP:
      INFMESSAGE(layout_lex: OP)
      break;
   case CP:
      INFMESSAGE(layout_lex: CP)
      break;
   case NAME:
      INFIMESSAGE(layout_lex: NAME,v->qval)
      INFSMESSAGE(the quark has the value,XrmQuarkToString(v->qval))
      break;
   case NUMBER:
      INFIMESSAGE(layout_lex: NUMBER,v->ival)
      break;
   case INFINITY:
      INFIMESSAGE(layout_lex: INFINITY,v->ival)
      break;
   case VERTICAL:
      INFMESSAGE(layout_lex: VERTICAL)
      break;
   case HORIZONTAL:
      INFMESSAGE(layout_lex: HORIZONTAL)
      break;
   case EQUAL:
      INFMESSAGE(layout_lex: EQUAL)
      break;
   case DOLLAR:
      INFMESSAGE(layout_lex: DOLLAR)
      break;
   case PLUS:
      INFMESSAGE(layout_lex: PLUS)
      break;
   case MINUS:
      INFMESSAGE(layout_lex: MINUS)
      break;
   case TIMES:
      INFMESSAGE(layout_lex: TIMES)
      break;
   case DIVIDE:
      INFMESSAGE(layout_lex: DIVIDE)
      break;
   case PERCENTOF:
      INFMESSAGE(layout_lex: PERCENTOF)
      break;
   case PERCENT:
      INFMESSAGE(layout_lex: PERCENT)
      break;
   case WIDTH:
      INFMESSAGE(layout_lex: WIDTH)
      break;
   case HEIGHT:
      INFMESSAGE(layout_lex: HEIGHT)
      break;
   case UMINUS:
      INFMESSAGE(layout_lex: UMINUS)
      break;
   case UPLUS:
      INFMESSAGE(layout_lex: UPLUS)
      break;
   }
#endif

   ENDMESSAGE1(layout_lex)

   if (r != END) c++;
   layoutConverterArg->from = (String) c;
   return r;
}


#ifdef TEST_SCAN

#define teststring                     "\
|vbox{	 				\
  frame = 8                             \
  s     = 8% of |width fileButton       \
  ss    = 4% of |height tagsFrame       \
  sss   = 2                             \
  $frame                                \
  |hbox{				\
    $frame                              \
    |vbox {				\
      fileButton<+[1]*>                 \
      $ss                               \
      pageButton<+[1]*-100%>            \
      1<+2[1]>                          \
      autoResize<+[1]*-100%>            \
      1<+2[1]>                          \
      optFrame<+[1]*-100%>              \
      1<+2[1]>                          \
      tagsFrame<+[1]*-100%>             \
      1<+2[1]>                          \
      |hbox{				\
        0<+[1]>                         \
        pannerFrame<*-100%>             \
        0<+[1]>                         \
      }                                 \
      1<+2[1]>                          \
      |hbox {				\
        0<+[1]>                         \
        tocFrame<*+200[1]-100%>         \
        0<+[1]>                         \
      }                                 \
    }                                   \
    $ss                                 \
    |vbox {				\
      |hbox{				\
        orientationButton<+[2]-[3]*>	\
        $ss<-90%>                       \
        magstepButton<+[2]-[3]*>	\
        $ss<-90%>                       \
        pagemediaButton<+[2]-[3]*>	\
        $ss<-90%>                       \
        |vbox {                         \
          0<+[1]>                       \
           |hbox{                       \
             titleButton<+15[1]-[3]*>   \
             $ss<-90%>                  \
             dateButton<+5[3]-[4]*>     \
          }                             \
          0<+[1]>                       \
        }                               \
      }                                 \
      $ss                               \
      0<+[2]>                           \
      |hbox{                            \
        0<+[2]>                         \
        pageview<-100%*-100%>           \
        0<+[2]>                         \
      }                                 \
      0<+[2]>                           \
      $ss                               \
      |hbox{                            \
        locator<-[3]*>                  \
        $ss<-[1]>                       \
        message<+50[2]-10[3]*>          \
        $ss<-[1]>                       \
        status<+[2]-10[3]*>             \
      }                                 \
    }                                   \
    $frame                              \
  }                                     \
  $frame                                \
}"

int main(argc,argv)
   int argc;
   char **argv;
{
   char *str = teststring;
   int r;
   YYSTYPE v;

   MAINBEGINMESSAGE(main)

   while ((r=layout_lex(&v,&str)) != END);

   MAINENDMESSAGE(main)
   exit(0);
   return 0;
}

#endif /* TEST_SCAN */
