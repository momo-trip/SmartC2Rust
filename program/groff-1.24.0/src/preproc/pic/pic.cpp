/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30705

/* Bison version string.  */
#define YYBISON_VERSION "3.7.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 19 "../src/preproc/pic/pic.ypp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h> // EDOM, ERANGE, errno
#include <math.h> // atan2(), cos(), floor(), fmod(), log10(), pow(),
		  // sin(), sqrt()
#include <stdcountof.h>
#include <stdio.h> // fflush(), fprintf(), snprintf(), sprintf(), stderr
#include <stdlib.h> // rand(), srand(), system()
#include <string.h> // strcat(), strchr(), strcmp(), strcpy(), strlen()

#ifdef NEED_DECLARATION_RAND
#undef rand
extern "C" {
  int rand();
}
#endif /* NEED_DECLARATION_RAND */

#ifdef NEED_DECLARATION_SRAND
#undef srand
extern "C" {
#ifdef RET_TYPE_SRAND_IS_VOID
  void srand(unsigned int);
#else
  int srand(unsigned int);
#endif
}
#endif /* NEED_DECLARATION_SRAND */

#include "pic.h"
#include "ptable.h"
#include "object.h"

extern int delim_flag;
extern void copy_rest_thru(const char *, const char *);
extern void copy_file_thru(const char *, const char *, const char *);
extern void push_body(const char *);
extern void do_for(char *var, double from, double to,
		   int by_is_multiplicative, double by, char *body);
extern void do_lookahead();

/* Maximum number of characters produced by printf("%g") */
#define GDIGITS 14

int yylex();
void yyerror(const char *);

void reset(const char *nm);
void reset_all();

place *lookup_label(const char *);
void define_label(const char *label, const place *pl);

direction current_direction;
position current_position;

implement_ptable(place)

PTABLE(place) top_table;

PTABLE(place) *current_table = &top_table;
saved_state *current_saved_state = 0 /* nullptr */;

object_list olist;

const char *ordinal_postfix(int n);
const char *object_type_name(object_type type);
char *format_number(const char *fmt, double n);
char *do_sprintf(const char *fmt, const double *v, int nv);


#line 146 "src/preproc/pic/pic.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_SRC_PREPROC_PIC_PIC_HPP_INCLUDED
# define YY_YY_SRC_PREPROC_PIC_PIC_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    LABEL = 258,                   /* LABEL  */
    VARIABLE = 259,                /* VARIABLE  */
    NUMBER = 260,                  /* NUMBER  */
    TEXT = 261,                    /* TEXT  */
    COMMAND_LINE = 262,            /* COMMAND_LINE  */
    DELIMITED = 263,               /* DELIMITED  */
    ORDINAL = 264,                 /* ORDINAL  */
    TH = 265,                      /* TH  */
    LEFT_ARROW_HEAD = 266,         /* LEFT_ARROW_HEAD  */
    RIGHT_ARROW_HEAD = 267,        /* RIGHT_ARROW_HEAD  */
    DOUBLE_ARROW_HEAD = 268,       /* DOUBLE_ARROW_HEAD  */
    LAST = 269,                    /* LAST  */
    BOX = 270,                     /* BOX  */
    CIRCLE = 271,                  /* CIRCLE  */
    ELLIPSE = 272,                 /* ELLIPSE  */
    ARC = 273,                     /* ARC  */
    LINE = 274,                    /* LINE  */
    POLYGON = 275,                 /* POLYGON  */
    ARROW = 276,                   /* ARROW  */
    MOVE = 277,                    /* MOVE  */
    SPLINE = 278,                  /* SPLINE  */
    HEIGHT = 279,                  /* HEIGHT  */
    RADIUS = 280,                  /* RADIUS  */
    FIGNAME = 281,                 /* FIGNAME  */
    WIDTH = 282,                   /* WIDTH  */
    DIAMETER = 283,                /* DIAMETER  */
    UP = 284,                      /* UP  */
    DOWN = 285,                    /* DOWN  */
    RIGHT = 286,                   /* RIGHT  */
    LEFT = 287,                    /* LEFT  */
    FROM = 288,                    /* FROM  */
    TO = 289,                      /* TO  */
    AT = 290,                      /* AT  */
    WITH = 291,                    /* WITH  */
    BY = 292,                      /* BY  */
    THEN = 293,                    /* THEN  */
    SOLID = 294,                   /* SOLID  */
    DOTTED = 295,                  /* DOTTED  */
    DASHED = 296,                  /* DASHED  */
    CHOP = 297,                    /* CHOP  */
    SAME = 298,                    /* SAME  */
    INVISIBLE = 299,               /* INVISIBLE  */
    LJUST = 300,                   /* LJUST  */
    RJUST = 301,                   /* RJUST  */
    ABOVE = 302,                   /* ABOVE  */
    BELOW = 303,                   /* BELOW  */
    OF = 304,                      /* OF  */
    THE = 305,                     /* THE  */
    WAY = 306,                     /* WAY  */
    BETWEEN = 307,                 /* BETWEEN  */
    AND = 308,                     /* AND  */
    HERE = 309,                    /* HERE  */
    DOT_N = 310,                   /* DOT_N  */
    DOT_E = 311,                   /* DOT_E  */
    DOT_W = 312,                   /* DOT_W  */
    DOT_S = 313,                   /* DOT_S  */
    DOT_NE = 314,                  /* DOT_NE  */
    DOT_SE = 315,                  /* DOT_SE  */
    DOT_NW = 316,                  /* DOT_NW  */
    DOT_SW = 317,                  /* DOT_SW  */
    DOT_C = 318,                   /* DOT_C  */
    DOT_MID = 319,                 /* DOT_MID  */
    DOT_V = 320,                   /* DOT_V  */
    DOT_START = 321,               /* DOT_START  */
    DOT_END = 322,                 /* DOT_END  */
    DOT_X = 323,                   /* DOT_X  */
    DOT_Y = 324,                   /* DOT_Y  */
    DOT_HT = 325,                  /* DOT_HT  */
    DOT_WID = 326,                 /* DOT_WID  */
    DOT_RAD = 327,                 /* DOT_RAD  */
    SIN = 328,                     /* SIN  */
    COS = 329,                     /* COS  */
    ATAN2 = 330,                   /* ATAN2  */
    LOG = 331,                     /* LOG  */
    EXP = 332,                     /* EXP  */
    SQRT = 333,                    /* SQRT  */
    K_MAX = 334,                   /* K_MAX  */
    K_MIN = 335,                   /* K_MIN  */
    INT = 336,                     /* INT  */
    RAND = 337,                    /* RAND  */
    SRAND = 338,                   /* SRAND  */
    COPY = 339,                    /* COPY  */
    THRU = 340,                    /* THRU  */
    TOP = 341,                     /* TOP  */
    BOTTOM = 342,                  /* BOTTOM  */
    UPPER = 343,                   /* UPPER  */
    LOWER = 344,                   /* LOWER  */
    SH = 345,                      /* SH  */
    PRINT = 346,                   /* PRINT  */
    CW = 347,                      /* CW  */
    CCW = 348,                     /* CCW  */
    FOR = 349,                     /* FOR  */
    DO = 350,                      /* DO  */
    IF = 351,                      /* IF  */
    ELSE = 352,                    /* ELSE  */
    ANDAND = 353,                  /* ANDAND  */
    OROR = 354,                    /* OROR  */
    NOTEQUAL = 355,                /* NOTEQUAL  */
    EQUALEQUAL = 356,              /* EQUALEQUAL  */
    LESSEQUAL = 357,               /* LESSEQUAL  */
    GREATEREQUAL = 358,            /* GREATEREQUAL  */
    LEFT_CORNER = 359,             /* LEFT_CORNER  */
    RIGHT_CORNER = 360,            /* RIGHT_CORNER  */
    NORTH = 361,                   /* NORTH  */
    SOUTH = 362,                   /* SOUTH  */
    EAST = 363,                    /* EAST  */
    WEST = 364,                    /* WEST  */
    CENTER = 365,                  /* CENTER  */
    END = 366,                     /* END  */
    START = 367,                   /* START  */
    RESET = 368,                   /* RESET  */
    UNTIL = 369,                   /* UNTIL  */
    PLOT = 370,                    /* PLOT  */
    THICKNESS = 371,               /* THICKNESS  */
    FILL = 372,                    /* FILL  */
    COLORED = 373,                 /* COLORED  */
    OUTLINED = 374,                /* OUTLINED  */
    SHADED = 375,                  /* SHADED  */
    XSLANTED = 376,                /* XSLANTED  */
    YSLANTED = 377,                /* YSLANTED  */
    ALIGNED = 378,                 /* ALIGNED  */
    SPRINTF = 379,                 /* SPRINTF  */
    COMMAND = 380,                 /* COMMAND  */
    DEFINE = 381,                  /* DEFINE  */
    UNDEF = 382                    /* UNDEF  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define LABEL 258
#define VARIABLE 259
#define NUMBER 260
#define TEXT 261
#define COMMAND_LINE 262
#define DELIMITED 263
#define ORDINAL 264
#define TH 265
#define LEFT_ARROW_HEAD 266
#define RIGHT_ARROW_HEAD 267
#define DOUBLE_ARROW_HEAD 268
#define LAST 269
#define BOX 270
#define CIRCLE 271
#define ELLIPSE 272
#define ARC 273
#define LINE 274
#define POLYGON 275
#define ARROW 276
#define MOVE 277
#define SPLINE 278
#define HEIGHT 279
#define RADIUS 280
#define FIGNAME 281
#define WIDTH 282
#define DIAMETER 283
#define UP 284
#define DOWN 285
#define RIGHT 286
#define LEFT 287
#define FROM 288
#define TO 289
#define AT 290
#define WITH 291
#define BY 292
#define THEN 293
#define SOLID 294
#define DOTTED 295
#define DASHED 296
#define CHOP 297
#define SAME 298
#define INVISIBLE 299
#define LJUST 300
#define RJUST 301
#define ABOVE 302
#define BELOW 303
#define OF 304
#define THE 305
#define WAY 306
#define BETWEEN 307
#define AND 308
#define HERE 309
#define DOT_N 310
#define DOT_E 311
#define DOT_W 312
#define DOT_S 313
#define DOT_NE 314
#define DOT_SE 315
#define DOT_NW 316
#define DOT_SW 317
#define DOT_C 318
#define DOT_MID 319
#define DOT_V 320
#define DOT_START 321
#define DOT_END 322
#define DOT_X 323
#define DOT_Y 324
#define DOT_HT 325
#define DOT_WID 326
#define DOT_RAD 327
#define SIN 328
#define COS 329
#define ATAN2 330
#define LOG 331
#define EXP 332
#define SQRT 333
#define K_MAX 334
#define K_MIN 335
#define INT 336
#define RAND 337
#define SRAND 338
#define COPY 339
#define THRU 340
#define TOP 341
#define BOTTOM 342
#define UPPER 343
#define LOWER 344
#define SH 345
#define PRINT 346
#define CW 347
#define CCW 348
#define FOR 349
#define DO 350
#define IF 351
#define ELSE 352
#define ANDAND 353
#define OROR 354
#define NOTEQUAL 355
#define EQUALEQUAL 356
#define LESSEQUAL 357
#define GREATEREQUAL 358
#define LEFT_CORNER 359
#define RIGHT_CORNER 360
#define NORTH 361
#define SOUTH 362
#define EAST 363
#define WEST 364
#define CENTER 365
#define END 366
#define START 367
#define RESET 368
#define UNTIL 369
#define PLOT 370
#define THICKNESS 371
#define FILL 372
#define COLORED 373
#define OUTLINED 374
#define SHADED 375
#define XSLANTED 376
#define YSLANTED 377
#define ALIGNED 378
#define SPRINTF 379
#define COMMAND 380
#define DEFINE 381
#define UNDEF 382

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 96 "../src/preproc/pic/pic.ypp"

	char *str;
	int n;
	double x;
	struct pair_s { double x, y; } pair;
	struct if_data_s { double x; char *body; } if_data;
	struct lstr_s { char *str; const char *filename; int lineno; } lstr;
	struct dv_s { double *v; int nv; int maxv; } dv;
	struct by_s { double val; int is_multiplicative; } by;
	place pl;
	object *obj;
	corner crn;
	vertex ver;
	path *pth;
	object_spec *spec;
	saved_state *pstate;
	graphics_state state;
	object_type obtype;

#line 473 "src/preproc/pic/pic.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_PREPROC_PIC_PIC_HPP_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_LABEL = 3,                      /* LABEL  */
  YYSYMBOL_VARIABLE = 4,                   /* VARIABLE  */
  YYSYMBOL_NUMBER = 5,                     /* NUMBER  */
  YYSYMBOL_TEXT = 6,                       /* TEXT  */
  YYSYMBOL_COMMAND_LINE = 7,               /* COMMAND_LINE  */
  YYSYMBOL_DELIMITED = 8,                  /* DELIMITED  */
  YYSYMBOL_ORDINAL = 9,                    /* ORDINAL  */
  YYSYMBOL_TH = 10,                        /* TH  */
  YYSYMBOL_LEFT_ARROW_HEAD = 11,           /* LEFT_ARROW_HEAD  */
  YYSYMBOL_RIGHT_ARROW_HEAD = 12,          /* RIGHT_ARROW_HEAD  */
  YYSYMBOL_DOUBLE_ARROW_HEAD = 13,         /* DOUBLE_ARROW_HEAD  */
  YYSYMBOL_LAST = 14,                      /* LAST  */
  YYSYMBOL_BOX = 15,                       /* BOX  */
  YYSYMBOL_CIRCLE = 16,                    /* CIRCLE  */
  YYSYMBOL_ELLIPSE = 17,                   /* ELLIPSE  */
  YYSYMBOL_ARC = 18,                       /* ARC  */
  YYSYMBOL_LINE = 19,                      /* LINE  */
  YYSYMBOL_POLYGON = 20,                   /* POLYGON  */
  YYSYMBOL_ARROW = 21,                     /* ARROW  */
  YYSYMBOL_MOVE = 22,                      /* MOVE  */
  YYSYMBOL_SPLINE = 23,                    /* SPLINE  */
  YYSYMBOL_HEIGHT = 24,                    /* HEIGHT  */
  YYSYMBOL_RADIUS = 25,                    /* RADIUS  */
  YYSYMBOL_FIGNAME = 26,                   /* FIGNAME  */
  YYSYMBOL_WIDTH = 27,                     /* WIDTH  */
  YYSYMBOL_DIAMETER = 28,                  /* DIAMETER  */
  YYSYMBOL_UP = 29,                        /* UP  */
  YYSYMBOL_DOWN = 30,                      /* DOWN  */
  YYSYMBOL_RIGHT = 31,                     /* RIGHT  */
  YYSYMBOL_LEFT = 32,                      /* LEFT  */
  YYSYMBOL_FROM = 33,                      /* FROM  */
  YYSYMBOL_TO = 34,                        /* TO  */
  YYSYMBOL_AT = 35,                        /* AT  */
  YYSYMBOL_WITH = 36,                      /* WITH  */
  YYSYMBOL_BY = 37,                        /* BY  */
  YYSYMBOL_THEN = 38,                      /* THEN  */
  YYSYMBOL_SOLID = 39,                     /* SOLID  */
  YYSYMBOL_DOTTED = 40,                    /* DOTTED  */
  YYSYMBOL_DASHED = 41,                    /* DASHED  */
  YYSYMBOL_CHOP = 42,                      /* CHOP  */
  YYSYMBOL_SAME = 43,                      /* SAME  */
  YYSYMBOL_INVISIBLE = 44,                 /* INVISIBLE  */
  YYSYMBOL_LJUST = 45,                     /* LJUST  */
  YYSYMBOL_RJUST = 46,                     /* RJUST  */
  YYSYMBOL_ABOVE = 47,                     /* ABOVE  */
  YYSYMBOL_BELOW = 48,                     /* BELOW  */
  YYSYMBOL_OF = 49,                        /* OF  */
  YYSYMBOL_THE = 50,                       /* THE  */
  YYSYMBOL_WAY = 51,                       /* WAY  */
  YYSYMBOL_BETWEEN = 52,                   /* BETWEEN  */
  YYSYMBOL_AND = 53,                       /* AND  */
  YYSYMBOL_HERE = 54,                      /* HERE  */
  YYSYMBOL_DOT_N = 55,                     /* DOT_N  */
  YYSYMBOL_DOT_E = 56,                     /* DOT_E  */
  YYSYMBOL_DOT_W = 57,                     /* DOT_W  */
  YYSYMBOL_DOT_S = 58,                     /* DOT_S  */
  YYSYMBOL_DOT_NE = 59,                    /* DOT_NE  */
  YYSYMBOL_DOT_SE = 60,                    /* DOT_SE  */
  YYSYMBOL_DOT_NW = 61,                    /* DOT_NW  */
  YYSYMBOL_DOT_SW = 62,                    /* DOT_SW  */
  YYSYMBOL_DOT_C = 63,                     /* DOT_C  */
  YYSYMBOL_DOT_MID = 64,                   /* DOT_MID  */
  YYSYMBOL_DOT_V = 65,                     /* DOT_V  */
  YYSYMBOL_DOT_START = 66,                 /* DOT_START  */
  YYSYMBOL_DOT_END = 67,                   /* DOT_END  */
  YYSYMBOL_DOT_X = 68,                     /* DOT_X  */
  YYSYMBOL_DOT_Y = 69,                     /* DOT_Y  */
  YYSYMBOL_DOT_HT = 70,                    /* DOT_HT  */
  YYSYMBOL_DOT_WID = 71,                   /* DOT_WID  */
  YYSYMBOL_DOT_RAD = 72,                   /* DOT_RAD  */
  YYSYMBOL_SIN = 73,                       /* SIN  */
  YYSYMBOL_COS = 74,                       /* COS  */
  YYSYMBOL_ATAN2 = 75,                     /* ATAN2  */
  YYSYMBOL_LOG = 76,                       /* LOG  */
  YYSYMBOL_EXP = 77,                       /* EXP  */
  YYSYMBOL_SQRT = 78,                      /* SQRT  */
  YYSYMBOL_K_MAX = 79,                     /* K_MAX  */
  YYSYMBOL_K_MIN = 80,                     /* K_MIN  */
  YYSYMBOL_INT = 81,                       /* INT  */
  YYSYMBOL_RAND = 82,                      /* RAND  */
  YYSYMBOL_SRAND = 83,                     /* SRAND  */
  YYSYMBOL_COPY = 84,                      /* COPY  */
  YYSYMBOL_THRU = 85,                      /* THRU  */
  YYSYMBOL_TOP = 86,                       /* TOP  */
  YYSYMBOL_BOTTOM = 87,                    /* BOTTOM  */
  YYSYMBOL_UPPER = 88,                     /* UPPER  */
  YYSYMBOL_LOWER = 89,                     /* LOWER  */
  YYSYMBOL_SH = 90,                        /* SH  */
  YYSYMBOL_PRINT = 91,                     /* PRINT  */
  YYSYMBOL_CW = 92,                        /* CW  */
  YYSYMBOL_CCW = 93,                       /* CCW  */
  YYSYMBOL_FOR = 94,                       /* FOR  */
  YYSYMBOL_DO = 95,                        /* DO  */
  YYSYMBOL_IF = 96,                        /* IF  */
  YYSYMBOL_ELSE = 97,                      /* ELSE  */
  YYSYMBOL_ANDAND = 98,                    /* ANDAND  */
  YYSYMBOL_OROR = 99,                      /* OROR  */
  YYSYMBOL_NOTEQUAL = 100,                 /* NOTEQUAL  */
  YYSYMBOL_EQUALEQUAL = 101,               /* EQUALEQUAL  */
  YYSYMBOL_LESSEQUAL = 102,                /* LESSEQUAL  */
  YYSYMBOL_GREATEREQUAL = 103,             /* GREATEREQUAL  */
  YYSYMBOL_LEFT_CORNER = 104,              /* LEFT_CORNER  */
  YYSYMBOL_RIGHT_CORNER = 105,             /* RIGHT_CORNER  */
  YYSYMBOL_NORTH = 106,                    /* NORTH  */
  YYSYMBOL_SOUTH = 107,                    /* SOUTH  */
  YYSYMBOL_EAST = 108,                     /* EAST  */
  YYSYMBOL_WEST = 109,                     /* WEST  */
  YYSYMBOL_CENTER = 110,                   /* CENTER  */
  YYSYMBOL_END = 111,                      /* END  */
  YYSYMBOL_START = 112,                    /* START  */
  YYSYMBOL_RESET = 113,                    /* RESET  */
  YYSYMBOL_UNTIL = 114,                    /* UNTIL  */
  YYSYMBOL_PLOT = 115,                     /* PLOT  */
  YYSYMBOL_THICKNESS = 116,                /* THICKNESS  */
  YYSYMBOL_FILL = 117,                     /* FILL  */
  YYSYMBOL_COLORED = 118,                  /* COLORED  */
  YYSYMBOL_OUTLINED = 119,                 /* OUTLINED  */
  YYSYMBOL_SHADED = 120,                   /* SHADED  */
  YYSYMBOL_XSLANTED = 121,                 /* XSLANTED  */
  YYSYMBOL_YSLANTED = 122,                 /* YSLANTED  */
  YYSYMBOL_ALIGNED = 123,                  /* ALIGNED  */
  YYSYMBOL_SPRINTF = 124,                  /* SPRINTF  */
  YYSYMBOL_COMMAND = 125,                  /* COMMAND  */
  YYSYMBOL_DEFINE = 126,                   /* DEFINE  */
  YYSYMBOL_UNDEF = 127,                    /* UNDEF  */
  YYSYMBOL_128_ = 128,                     /* '.'  */
  YYSYMBOL_129_ = 129,                     /* '('  */
  YYSYMBOL_130_ = 130,                     /* '`'  */
  YYSYMBOL_131_ = 131,                     /* '['  */
  YYSYMBOL_132_ = 132,                     /* ','  */
  YYSYMBOL_133_ = 133,                     /* '<'  */
  YYSYMBOL_134_ = 134,                     /* '>'  */
  YYSYMBOL_135_ = 135,                     /* '+'  */
  YYSYMBOL_136_ = 136,                     /* '-'  */
  YYSYMBOL_137_ = 137,                     /* '*'  */
  YYSYMBOL_138_ = 138,                     /* '/'  */
  YYSYMBOL_139_ = 139,                     /* '%'  */
  YYSYMBOL_140_ = 140,                     /* '!'  */
  YYSYMBOL_141_ = 141,                     /* '^'  */
  YYSYMBOL_142_ = 142,                     /* ';'  */
  YYSYMBOL_143_ = 143,                     /* '='  */
  YYSYMBOL_144_ = 144,                     /* ':'  */
  YYSYMBOL_145_ = 145,                     /* '{'  */
  YYSYMBOL_146_ = 146,                     /* '}'  */
  YYSYMBOL_147_ = 147,                     /* ']'  */
  YYSYMBOL_148_ = 148,                     /* ')'  */
  YYSYMBOL_149_ = 149,                     /* '\''  */
  YYSYMBOL_YYACCEPT = 150,                 /* $accept  */
  YYSYMBOL_top = 151,                      /* top  */
  YYSYMBOL_element_list = 152,             /* element_list  */
  YYSYMBOL_middle_element_list = 153,      /* middle_element_list  */
  YYSYMBOL_optional_separator = 154,       /* optional_separator  */
  YYSYMBOL_separator = 155,                /* separator  */
  YYSYMBOL_placeless_element = 156,        /* placeless_element  */
  YYSYMBOL_157_1 = 157,                    /* $@1  */
  YYSYMBOL_158_2 = 158,                    /* $@2  */
  YYSYMBOL_159_3 = 159,                    /* $@3  */
  YYSYMBOL_160_4 = 160,                    /* $@4  */
  YYSYMBOL_161_5 = 161,                    /* $@5  */
  YYSYMBOL_162_6 = 162,                    /* $@6  */
  YYSYMBOL_163_7 = 163,                    /* $@7  */
  YYSYMBOL_macro_name = 164,               /* macro_name  */
  YYSYMBOL_reset_variables = 165,          /* reset_variables  */
  YYSYMBOL_print_args = 166,               /* print_args  */
  YYSYMBOL_print_arg = 167,                /* print_arg  */
  YYSYMBOL_simple_if = 168,                /* simple_if  */
  YYSYMBOL_169_8 = 169,                    /* $@8  */
  YYSYMBOL_until = 170,                    /* until  */
  YYSYMBOL_any_expr = 171,                 /* any_expr  */
  YYSYMBOL_text_expr = 172,                /* text_expr  */
  YYSYMBOL_optional_by = 173,              /* optional_by  */
  YYSYMBOL_element = 174,                  /* element  */
  YYSYMBOL_175_9 = 175,                    /* @9  */
  YYSYMBOL_176_10 = 176,                   /* $@10  */
  YYSYMBOL_optional_element = 177,         /* optional_element  */
  YYSYMBOL_object_spec = 178,              /* object_spec  */
  YYSYMBOL_179_11 = 179,                   /* @11  */
  YYSYMBOL_text = 180,                     /* text  */
  YYSYMBOL_sprintf_args = 181,             /* sprintf_args  */
  YYSYMBOL_position = 182,                 /* position  */
  YYSYMBOL_position_not_place = 183,       /* position_not_place  */
  YYSYMBOL_between = 184,                  /* between  */
  YYSYMBOL_expr_pair = 185,                /* expr_pair  */
  YYSYMBOL_place = 186,                    /* place  */
  YYSYMBOL_label = 187,                    /* label  */
  YYSYMBOL_ordinal = 188,                  /* ordinal  */
  YYSYMBOL_optional_ordinal_last = 189,    /* optional_ordinal_last  */
  YYSYMBOL_nth_primitive = 190,            /* nth_primitive  */
  YYSYMBOL_object_type = 191,              /* object_type  */
  YYSYMBOL_label_path = 192,               /* label_path  */
  YYSYMBOL_relative_path = 193,            /* relative_path  */
  YYSYMBOL_path = 194,                     /* path  */
  YYSYMBOL_corner = 195,                   /* corner  */
  YYSYMBOL_vertex = 196,                   /* vertex  */
  YYSYMBOL_expr = 197,                     /* expr  */
  YYSYMBOL_expr_lower_than = 198,          /* expr_lower_than  */
  YYSYMBOL_expr_not_lower_than = 199       /* expr_not_lower_than  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  6
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2745

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  150
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  269
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  471

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   382


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   140,     2,     2,     2,   139,     2,   149,
     129,   148,   137,   135,   132,   136,   128,   138,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   144,   142,
     133,   143,   134,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   131,     2,   147,   141,     2,   130,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   145,     2,   146,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   311,   311,   312,   321,   326,   328,   332,   334,   338,
     339,   343,   351,   356,   368,   370,   372,   374,   376,   381,
     386,   393,   392,   408,   416,   418,   415,   429,   431,   428,
     441,   440,   449,   458,   457,   471,   472,   477,   478,   482,
     487,   492,   500,   502,   521,   528,   530,   541,   540,   552,
     553,   558,   560,   565,   571,   577,   579,   581,   583,   585,
     587,   589,   596,   600,   605,   613,   627,   633,   641,   648,
     654,   647,   663,   673,   674,   679,   681,   683,   685,   690,
     697,   704,   711,   718,   725,   730,   737,   745,   744,   771,
     777,   783,   789,   795,   814,   821,   828,   835,   842,   849,
     856,   863,   870,   877,   892,   904,   910,   918,   927,   934,
     959,   963,   969,   975,   981,   987,   992,   998,  1004,  1010,
    1017,  1026,  1033,  1049,  1066,  1071,  1076,  1081,  1086,  1091,
    1096,  1101,  1109,  1119,  1129,  1139,  1149,  1155,  1163,  1165,
    1177,  1182,  1204,  1206,  1212,  1221,  1223,  1228,  1233,  1238,
    1243,  1248,  1253,  1259,  1264,  1272,  1273,  1277,  1282,  1288,
    1290,  1296,  1303,  1309,  1315,  1322,  1331,  1341,  1343,  1352,
    1354,  1362,  1364,  1369,  1384,  1402,  1404,  1406,  1408,  1410,
    1412,  1414,  1416,  1418,  1420,  1425,  1427,  1435,  1439,  1441,
    1449,  1451,  1457,  1463,  1469,  1475,  1484,  1486,  1488,  1490,
    1492,  1494,  1496,  1498,  1500,  1502,  1504,  1506,  1508,  1510,
    1512,  1514,  1516,  1518,  1520,  1522,  1524,  1526,  1528,  1530,
    1532,  1534,  1536,  1538,  1540,  1542,  1544,  1546,  1551,  1556,
    1561,  1567,  1576,  1578,  1583,  1588,  1596,  1598,  1605,  1612,
    1619,  1626,  1633,  1635,  1637,  1639,  1647,  1655,  1668,  1670,
    1672,  1681,  1690,  1703,  1712,  1721,  1730,  1732,  1734,  1736,
    1743,  1749,  1754,  1756,  1758,  1760,  1762,  1764,  1766,  1768
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "LABEL", "VARIABLE",
  "NUMBER", "TEXT", "COMMAND_LINE", "DELIMITED", "ORDINAL", "TH",
  "LEFT_ARROW_HEAD", "RIGHT_ARROW_HEAD", "DOUBLE_ARROW_HEAD", "LAST",
  "BOX", "CIRCLE", "ELLIPSE", "ARC", "LINE", "POLYGON", "ARROW", "MOVE",
  "SPLINE", "HEIGHT", "RADIUS", "FIGNAME", "WIDTH", "DIAMETER", "UP",
  "DOWN", "RIGHT", "LEFT", "FROM", "TO", "AT", "WITH", "BY", "THEN",
  "SOLID", "DOTTED", "DASHED", "CHOP", "SAME", "INVISIBLE", "LJUST",
  "RJUST", "ABOVE", "BELOW", "OF", "THE", "WAY", "BETWEEN", "AND", "HERE",
  "DOT_N", "DOT_E", "DOT_W", "DOT_S", "DOT_NE", "DOT_SE", "DOT_NW",
  "DOT_SW", "DOT_C", "DOT_MID", "DOT_V", "DOT_START", "DOT_END", "DOT_X",
  "DOT_Y", "DOT_HT", "DOT_WID", "DOT_RAD", "SIN", "COS", "ATAN2", "LOG",
  "EXP", "SQRT", "K_MAX", "K_MIN", "INT", "RAND", "SRAND", "COPY", "THRU",
  "TOP", "BOTTOM", "UPPER", "LOWER", "SH", "PRINT", "CW", "CCW", "FOR",
  "DO", "IF", "ELSE", "ANDAND", "OROR", "NOTEQUAL", "EQUALEQUAL",
  "LESSEQUAL", "GREATEREQUAL", "LEFT_CORNER", "RIGHT_CORNER", "NORTH",
  "SOUTH", "EAST", "WEST", "CENTER", "END", "START", "RESET", "UNTIL",
  "PLOT", "THICKNESS", "FILL", "COLORED", "OUTLINED", "SHADED", "XSLANTED",
  "YSLANTED", "ALIGNED", "SPRINTF", "COMMAND", "DEFINE", "UNDEF", "'.'",
  "'('", "'`'", "'['", "','", "'<'", "'>'", "'+'", "'-'", "'*'", "'/'",
  "'%'", "'!'", "'^'", "';'", "'='", "':'", "'{'", "'}'", "']'", "')'",
  "'\\''", "$accept", "top", "element_list", "middle_element_list",
  "optional_separator", "separator", "placeless_element", "$@1", "$@2",
  "$@3", "$@4", "$@5", "$@6", "$@7", "macro_name", "reset_variables",
  "print_args", "print_arg", "simple_if", "$@8", "until", "any_expr",
  "text_expr", "optional_by", "element", "@9", "$@10", "optional_element",
  "object_spec", "@11", "text", "sprintf_args", "position",
  "position_not_place", "between", "expr_pair", "place", "label",
  "ordinal", "optional_ordinal_last", "nth_primitive", "object_type",
  "label_path", "relative_path", "path", "corner", "vertex", "expr",
  "expr_lower_than", "expr_not_lower_than", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,    46,    40,
      96,    91,    44,    60,    62,    43,    45,    42,    47,    37,
      33,    94,    59,    61,    58,   123,   125,    93,    41,    39
};
#endif

#define YYPACT_NINF (-148)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-211)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -139,  -148,     9,  -148,   742,  -120,  -148,  -117,   -84,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -108,  -148,  -148,  -148,  -148,     5,  -148,  1080,    20,  1167,
      35,  1602,   -81,  1080,  -148,  -148,  -139,  -148,     8,   -40,
    -148,   865,  -148,  -148,  -139,  1167,   -78,   138,   -18,  -148,
      66,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,  -148,  1689,  1776,
    -148,  -148,   -45,   -35,   -33,   -31,   -29,   -24,   -22,   -20,
       6,    26,    29,  -148,  -148,    21,   182,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  1254,  1167,  1602,  1602,
    1080,  -148,  -148,    31,  -148,  -148,   295,  2451,    62,   285,
    -148,    40,   111,  2352,  -148,   -47,   -32,  1167,  1167,   145,
      48,    74,   295,  2553,  -148,  -148,  2233,   188,  1080,  -139,
    -139,  -148,   706,  -148,   232,  -148,  -148,  -148,  -148,  1602,
    1602,  1602,  1602,  2213,  2213,  2038,  2126,  1863,  1863,  1863,
    1428,  1950,  -148,  -148,  2213,  2213,  2213,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  1602,  2213,    10,    10,    10,
    1602,  1602,  -148,  -148,  2562,   575,  -148,  1167,  -148,  -148,
    -148,  -148,   230,  -148,  1167,  2562,  1167,  2562,  1167,  1167,
    1167,  1167,  1167,  1167,  1167,  1167,  1167,   437,  1167,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,   103,   153,   112,
     -51,  2364,   135,   261,   149,   149,  -148,  1950,  1950,  -148,
    -148,  -148,  -148,  -148,   304,  -148,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,  -148,   164,  -148,  -148,
      52,   186,    52,   268,  -148,  1602,  1602,  1602,  1602,  1602,
    1602,  1602,  1602,  1602,  1602,  1602,  1602,  1602,  1602,  1602,
    1863,  1863,  1602,  -148,   149,  -148,  1167,  1167,    10,    10,
    1167,  1167,  -148,  -148,   172,   742,   176,  -148,  -148,   324,
    2562,  2562,  2562,  2562,  2562,  2562,  2562,  2562,    31,  2352,
      31,    31,  2464,   308,   308,   343,   993,    31,  2320,  -148,
    -148,    40,   111,  1341,  -148,  2509,  2562,  2562,  2562,  2562,
    2562,  -148,  -148,  -148,  2562,  2562,  -117,   -84,    95,   106,
    -148,    31,    81,   272,  -148,   339,  -148,  2482,  2491,   200,
     206,   223,   209,   210,   211,   228,   244,   220,  -148,   233,
     234,  -148,  1863,  1950,  1950,  -148,  -148,  1863,  1863,  -148,
    -148,  -148,  -148,  -148,   186,   186,   327,   347,  2577,   419,
     419,   120,   120,  2562,   120,   120,   143,   143,   149,   149,
     149,   149,   -48,   177,   236,   372,  -148,   347,   288,  2604,
    -148,  -148,  -148,   347,   288,  2604,  -119,  -148,  -148,  -148,
    -148,  -148,  2333,  2333,  -148,   251,   384,  -148,   112,  2535,
    -148,   274,  -148,  -148,  -148,  -148,  1167,  -148,  -148,  -148,
    1167,  1167,  -148,  -148,  -148,   -73,   241,   242,   -43,   217,
     340,  1863,  1863,  1602,  -148,  1602,  -148,   742,  -148,  -148,
    2333,  -148,   274,   385,  -148,   245,   249,   252,  -148,  -148,
    -148,  1863,  1863,  -148,    31,  -102,   677,  2562,  -148,  -148,
     253,  -148,  -148,  -148,  -148,  -148,   114,   131,  -148,  1515,
     307,  -148,  -148,   255,  1602,  2562,  -148,  -148,  2562,   391,
    -148
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       7,     9,     0,     3,     2,     8,     1,     0,     0,   138,
      18,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       0,    14,    15,    17,    16,     0,    21,     0,     0,     0,
      36,     0,     0,     0,    87,    69,     7,    72,    35,    32,
       5,    65,    84,    10,     7,     0,     0,     0,    23,    27,
       0,   166,   235,   236,   169,   171,   210,   209,   165,   196,
     197,   198,   199,   200,   201,   202,   203,   204,     0,     0,
     205,   206,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   207,   208,     0,     0,   215,   216,   221,
     222,   223,   224,   225,   227,   226,     0,     0,     0,     0,
      20,    42,    45,    46,   142,   145,   143,   159,     0,     0,
     167,     0,     0,    44,   232,   233,     0,     0,     0,     0,
      52,     0,     0,    51,   233,    39,    85,     0,    19,     7,
       7,     4,     8,    40,     0,    33,   126,   127,   128,     0,
       0,     0,     0,    94,    96,    98,   100,     0,     0,     0,
       0,     0,   109,   110,   111,   113,   122,   124,   125,   132,
     133,   134,   135,   129,   130,     0,   115,     0,     0,     0,
       0,     0,   137,   131,    93,     0,    12,     0,    38,    37,
      11,    24,     0,    22,     0,   230,     0,   228,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   213,
     211,   217,   219,   214,   212,   218,   220,     0,     0,   145,
     143,    51,   233,     0,   248,   269,    43,     0,     0,   237,
     238,   239,   240,   241,     0,   160,   161,   184,   172,   175,
     176,   177,   178,   179,   180,   181,   182,     0,   173,   174,
       0,   162,     0,     0,   155,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    61,   269,    47,     0,     0,     0,     0,
       0,     0,    86,   140,     0,     0,     0,     6,    41,     0,
      89,    90,    91,    92,    95,    97,    99,   101,   102,     0,
     103,   104,   166,   169,   171,     0,     0,   107,   188,   190,
     105,   187,   106,     0,   108,     0,   112,   114,   123,   136,
     116,   120,   121,   119,   117,   118,   166,   235,   210,   209,
      66,     0,    67,    68,    13,     0,    28,    51,    51,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   260,     0,
       0,   249,     0,     0,     0,   158,   144,     0,     0,   170,
     146,   148,   168,   183,   163,   164,     0,   267,   268,   266,
     265,   262,   264,   157,   234,   263,   242,   243,   244,   245,
     246,   247,     0,     0,     0,     0,    55,    56,    58,    59,
      54,    53,    57,   267,    60,   268,     0,    88,    70,    34,
     195,   187,     0,     0,   185,     0,     0,   189,     0,    51,
      25,    49,   231,   229,   250,   251,     0,   253,   254,   255,
       0,     0,   258,   259,   261,     0,   146,   148,     0,     0,
       0,     0,     0,     0,    48,     0,   139,    73,   194,   193,
       0,   186,    49,     0,    29,     0,     0,     0,   150,   147,
     149,     0,     0,   156,   151,     0,    62,   141,    74,    71,
       0,    26,    50,   252,   256,   257,   151,     0,   153,     0,
       0,   191,   152,   153,     0,    63,    30,   154,    64,     0,
      31
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -148,  -148,   148,  -148,     2,   368,  -148,  -148,  -148,  -148,
    -148,  -148,  -148,  -148,  -148,  -148,   373,   -77,  -148,  -148,
     -23,    11,   -90,  -148,  -131,  -148,  -148,  -148,  -148,  -148,
       4,  -148,    14,   235,   194,   -49,     3,   -86,  -148,  -148,
    -148,   -83,  -148,  -147,  -148,  -100,   -92,   -27,  -148,    37
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     3,    36,   275,     5,    37,    50,   325,   432,
     182,   401,   469,   279,   180,    38,   100,   101,    39,   375,
     434,   207,   120,   460,    40,   130,   427,   449,    41,   129,
     121,   386,   103,   104,   260,   105,   122,   107,   108,   109,
     110,   238,   298,   299,   300,   111,   112,   123,   114,   124
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     113,   277,     4,     1,   126,   421,   113,   225,    42,     6,
     441,    48,   133,   425,   174,   226,     9,   219,   220,   221,
     222,   223,    43,   216,   116,   241,   239,    44,   263,   426,
     106,   102,   458,   217,   218,    47,   106,   102,   131,   125,
     119,   185,   187,    51,   320,   173,   175,   209,   127,    54,
     301,   216,   199,   200,    55,    51,   176,   135,   302,    45,
      46,    54,   217,   218,   115,   177,    55,   181,   227,   211,
     115,   214,   215,   113,   183,   438,   228,   229,   230,   231,
     232,   233,   234,   235,   188,   236,   261,   217,   218,   240,
      49,   264,   217,   218,   189,   -17,   190,   346,   191,   210,
     192,   113,   304,   106,   102,   193,   -16,   194,   213,   195,
     208,   262,   280,   281,   282,   283,   284,   285,   286,   287,
     289,   289,   289,   289,   305,   201,   202,   306,   307,   308,
     272,   106,   102,   212,    32,   196,    42,   115,   309,   310,
     134,   178,   179,   314,   315,   390,   266,   267,   289,   395,
     106,   106,   106,   106,   354,   197,   355,   327,   198,   328,
     242,   288,   290,   291,   297,   115,   217,   218,   350,   351,
      97,   311,   312,   313,   268,   269,   376,   378,   323,    42,
     382,   384,    97,   265,   115,   115,   115,   115,   324,   321,
     305,   305,   391,   237,   273,   213,   301,   213,   397,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   339,   340,
     392,   393,   115,   203,   204,   241,  -142,  -142,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
     369,   370,   371,   289,   289,   374,   278,   -17,   326,   377,
     379,   -17,   -17,   383,   385,   428,   429,   209,   -16,   217,
     218,   341,   -16,   -16,   398,   254,   255,   256,   257,   258,
     345,   259,   462,   106,   106,   463,   217,   218,   348,   211,
     423,   349,   380,   381,   372,   373,   399,   274,   276,    42,
     256,   257,   258,   450,   259,   342,   205,   206,   343,   344,
     259,   227,   391,   391,   416,   417,   448,   115,   115,   210,
     229,   230,   231,   232,   233,   234,   235,   352,   236,   422,
     208,   353,   217,   218,   224,   289,   305,   305,   356,   387,
     289,   289,   388,   229,   230,   231,   232,   233,   234,   235,
     391,   236,   389,   212,   245,   246,   247,   248,   249,   250,
     219,   220,   221,   222,   223,   106,   394,   400,   404,   442,
     106,   106,   217,   218,   405,   406,   415,   407,   408,   409,
     410,   418,   419,   219,   220,   221,   222,   223,   412,   252,
     253,   254,   255,   256,   257,   258,   411,   259,   420,   115,
     424,   413,   414,   430,   115,   115,   266,   431,   433,   439,
     440,   452,   443,   453,   289,   289,   446,   454,   447,   470,
     455,   461,   466,   467,   132,   347,   128,  -143,  -143,   451,
     322,     0,     0,     0,   289,   289,   237,   435,     0,     0,
       0,   436,   437,     0,   106,   106,     0,     0,     0,     0,
       0,    42,   465,     0,     0,   444,   445,   468,     0,   237,
      51,    52,    53,     9,   106,   106,    54,   247,   248,   249,
     250,    55,     0,     0,     0,   456,   457,     0,   115,   115,
       0,     0,     0,     0,     0,     0,     0,     0,    56,    57,
       0,     0,     0,     0,     0,     0,     0,     0,   115,   115,
     252,   253,   254,   255,   256,   257,   258,     0,   259,     0,
       0,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,   249,   250,    83,    84,    85,    86,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    87,    88,    89,    90,    91,    92,    93,    94,    95,
       0,     0,   252,   253,   254,   255,   256,   257,   258,     0,
     259,    32,     0,     0,     0,     0,   117,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,   118,   316,   317,
      53,     9,    10,     0,    54,   338,     0,     0,     0,    55,
      11,    12,    13,    14,    15,    16,    17,    18,    19,     0,
       0,    20,     0,     0,    21,    22,   318,   319,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    25,
       0,    83,    84,    85,    86,    26,    27,     0,     0,    28,
       0,    29,     0,     0,     0,     0,     0,     0,     0,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    30,     0,
      31,     0,     0,     0,     0,     0,     0,     0,     0,    32,
      33,     0,     0,     0,    96,    97,    34,     0,     0,     7,
       8,    98,     9,    10,   459,    99,     0,     0,     0,     0,
      35,    11,    12,    13,    14,    15,    16,    17,    18,    19,
       0,     0,    20,     0,     0,    21,    22,    23,    24,     0,
       0,     0,     0,     0,     0,     7,     8,     0,     9,    10,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,     0,     0,    20,     0,
       0,    21,    22,    23,    24,   245,   246,   247,   248,   249,
     250,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,     0,     0,     0,     0,     0,    26,    27,     0,     0,
      28,     0,    29,     0,     0,     0,     0,     0,     0,     0,
     252,   253,   254,   255,   256,   257,   258,     0,   259,    30,
       0,    31,     0,     0,     0,     0,    25,     0,     0,     0,
      32,    33,    26,    27,     0,     0,    28,    34,    29,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    43,     0,
       0,    35,     0,     0,     0,    30,     0,    31,     0,     0,
       0,     0,     0,     0,     0,     0,    32,    33,    51,    52,
      53,     9,     0,    34,    54,     0,   136,   137,   138,    55,
       0,     0,     0,     0,     0,     0,     0,    35,     0,   139,
     140,     0,   141,   142,   143,   144,   145,   146,   147,   148,
     149,   150,   151,   152,   153,   154,   155,   156,   157,   158,
     159,   160,   161,   162,     0,     0,     0,     0,     0,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,    83,    84,    85,    86,     0,     0,   163,   164,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    87,
      88,    89,    90,    91,    92,    93,    94,    95,     0,     0,
       0,   165,   166,   167,   168,   169,   170,   171,   172,    32,
       0,     0,     0,     0,   117,    97,    51,    52,    53,     9,
       0,    98,    54,     0,     0,    99,     0,    55,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    56,    57,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,    83,
      84,    85,    86,    51,    52,    53,     9,     0,     0,    54,
       0,     0,     0,     0,    55,     0,     0,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     0,     0,     0,     0,
       0,    56,    57,     0,     0,     0,     0,    32,     0,     0,
       0,   295,    96,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,   118,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,    83,    84,    85,    86,
      51,    52,    53,     9,     0,     0,    54,     0,     0,     0,
       0,    55,     0,     0,    87,    88,    89,    90,    91,    92,
      93,    94,    95,     0,     0,     0,     0,     0,    56,    57,
       0,     0,     0,     0,    32,     0,     0,     0,     0,    96,
      97,     0,     0,     0,     0,     0,    98,     0,     0,     0,
      99,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,     0,     0,    83,    84,    85,    86,    51,    52,    53,
       9,     0,     0,    54,     0,     0,     0,     0,    55,     0,
       0,    87,    88,    89,    90,    91,    92,    93,    94,    95,
       0,     0,     0,     0,     0,    56,    57,     0,     0,     0,
       0,    32,     0,     0,     0,     0,   117,    97,     0,     0,
       0,     0,     0,    98,     0,     0,     0,   118,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,     0,     0,     0,     0,     0,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,     0,     0,
      83,    84,    85,    86,    51,    52,    53,     9,     0,     0,
      54,     0,     0,     0,     0,    55,     0,     0,    87,    88,
      89,    90,    91,    92,    93,    94,    95,     0,     0,     0,
       0,     0,    56,    57,     0,     0,     0,     0,    32,     0,
       0,     0,     0,    96,    97,     0,     0,     0,     0,     0,
      98,     0,     0,     0,   118,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,     0,     0,    83,    84,    85,
      86,   292,    52,    53,     0,     0,     0,   293,     0,     0,
       0,     0,   294,     0,     0,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     0,     0,     0,     0,     0,    56,
      57,     0,     0,     0,     0,    32,     0,     0,     0,     0,
     303,    97,     0,     0,     0,     0,     0,    98,     0,     0,
       0,   118,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,    83,    84,    85,    86,    51,    52,
      53,     0,     0,     0,    54,     0,     0,     0,     0,    55,
       0,     0,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     0,     0,     0,     0,     0,    56,    57,     0,     0,
       0,     0,     0,     0,     0,     0,   295,   296,    97,     0,
       0,     0,     0,     0,    98,     0,     0,     0,    99,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,     0,
       0,    83,    84,    85,    86,    51,    52,    53,     0,     0,
       0,    54,     0,     0,     0,     0,    55,     0,     0,    87,
      88,    89,    90,    91,    92,    93,    94,    95,     0,     0,
       0,     0,     0,    56,    57,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   117,    97,     0,     0,     0,     0,
       0,    98,   464,     0,     0,    99,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,     0,     0,    83,    84,
      85,    86,    51,    52,    53,     0,     0,     0,    54,     0,
       0,     0,     0,    55,     0,     0,    87,    88,    89,    90,
      91,    92,    93,    94,    95,     0,     0,     0,     0,     0,
      56,    57,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   117,    97,     0,     0,     0,     0,     0,    98,     0,
       0,     0,    99,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,     0,     0,    83,    84,    85,    86,    51,
      52,    53,     0,     0,     0,    54,     0,     0,     0,     0,
      55,     0,     0,    87,    88,    89,    90,    91,    92,    93,
      94,    95,     0,     0,     0,     0,     0,    56,    57,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   117,   184,
       0,     0,     0,     0,     0,    98,     0,     0,     0,    99,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,    83,    84,    85,    86,    51,    52,    53,     0,
       0,     0,    54,     0,     0,     0,     0,    55,     0,     0,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     0,
       0,     0,     0,     0,    56,    57,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   117,   186,     0,     0,     0,
       0,     0,    98,     0,     0,     0,    99,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,    83,
      84,    85,    86,    51,    52,    53,     0,     0,     0,    54,
       0,     0,     0,     0,    55,     0,     0,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     0,     0,     0,     0,
       0,    56,    57,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    96,    97,     0,     0,     0,     0,     0,    98,
       0,     0,     0,    99,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,     0,     0,    83,    84,    85,    86,
       0,    51,    52,    53,     0,     0,     0,    54,     0,     0,
       0,     0,    55,     0,    87,    88,    89,    90,    91,    92,
      93,    94,    95,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   303,
      97,     0,     0,     0,     0,     0,    98,  -210,     0,     0,
      99,     0,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,     0,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,     0,     0,    83,    84,    85,    86,     0,    51,
      52,    53,     0,     0,     0,    54,     0,     0,     0,     0,
      55,     0,    87,    88,    89,    90,    91,    92,    93,    94,
      95,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   117,    97,     0,
       0,     0,     0,     0,    98,  -209,     0,     0,    99,     0,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
       0,     0,    83,    84,    85,    86,    51,    52,    53,     0,
       0,     0,    54,     0,     0,     0,     0,    55,     0,     0,
      87,    88,    89,    90,    91,    92,    93,    94,    95,     9,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   117,    97,     0,     0,     0,
       0,     0,    98,     0,     0,     0,    99,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,     0,     0,    83,
      84,    85,    86,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     0,     0,     0,     0,
       0,   245,   246,   247,   248,   249,   250,     0,     0,     0,
       0,     0,   117,    97,     0,     0,     0,     0,     0,    98,
       0,    56,    57,    99,     0,     0,     0,    32,     0,     0,
       0,     0,     0,     0,    56,    57,   252,   253,   254,   255,
     256,   257,   258,     0,   259,    59,    60,    61,    62,    63,
      64,    65,    66,    67,     0,     0,    70,    71,    59,    60,
      61,    62,    63,    64,    65,    66,    67,     0,     0,    70,
      71,   243,     0,     0,   244,     0,    83,    84,    85,    86,
       0,     0,     0,   243,     0,     0,   244,     0,     0,    83,
      84,    85,    86,     0,    87,    88,    89,    90,    91,    92,
      93,    94,    95,     0,     0,     0,     0,    87,    88,    89,
      90,    91,    92,    93,    94,    95,     0,     0,   396,     0,
     245,   246,   247,   248,   249,   250,     0,     0,     0,     0,
       0,   295,   270,   271,   247,   248,   249,   250,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   251,   252,   253,   254,   255,   256,
     257,   258,     0,   259,     0,     0,   251,   252,   253,   254,
     255,   256,   257,   258,     0,   259,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    59,
      60,    61,    62,    63,    64,    65,    66,    67,     0,     0,
      70,    71,     0,     0,     0,     0,     0,    83,    84,    85,
      86,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      83,    84,    85,    86,     0,    87,    88,    89,    90,    91,
      92,    93,    94,    95,     0,     0,     0,     0,    87,    88,
      89,    90,    91,    92,    93,    94,    95,     0,     0,   224,
     270,   271,   247,   248,   249,   250,     0,     0,     0,   270,
     271,   247,   248,   249,   250,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   245,   246,   247,
     248,   249,   250,     0,     0,   252,   253,   254,   255,   256,
     257,   258,     0,   259,   252,   253,   254,   255,   256,   257,
     258,   402,   259,   270,   271,   247,   248,   249,   250,     0,
     403,   251,   252,   253,   254,   255,   256,   257,   258,     0,
     259,   270,   271,   247,   248,   249,   250,     0,     0,     0,
     245,   246,   247,   248,   249,   250,     0,   251,   252,   253,
     254,   255,   256,   257,   258,   245,   259,   247,   248,   249,
     250,     0,     0,     0,     0,     0,   252,   253,   254,   255,
     256,   257,   258,     0,   259,   252,   253,   254,   255,   256,
     257,   258,   270,   259,   247,   248,   249,   250,     0,     0,
     252,   253,   254,   255,   256,   257,   258,     0,   259,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   252,   253,   254,
     255,   256,   257,   258,     0,   259
};

static const yytype_int16 yycheck[] =
{
      27,   132,     0,   142,    31,    53,    33,   107,     4,     0,
      53,     6,     4,   132,    41,   107,     6,    68,    69,    70,
      71,    72,   142,   100,     4,   111,   109,   144,   118,   148,
      27,    27,   134,   135,   136,   143,    33,    33,    36,     4,
      29,    68,    69,     3,   175,    41,    44,    96,   129,     9,
     150,   128,    31,    32,    14,     3,    45,    97,   150,   143,
     144,     9,   135,   136,    27,   143,    14,    85,     6,    96,
      33,    98,    99,   100,     8,   148,    14,    15,    16,    17,
      18,    19,    20,    21,   129,    23,   133,   135,   136,    49,
      85,   118,   135,   136,   129,     0,   129,   148,   129,    96,
     129,   128,   151,   100,   100,   129,     0,   129,    97,   129,
      96,   143,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   104,   105,   154,   155,   156,
     126,   128,   128,    96,   124,   129,   132,   100,   165,   166,
     132,     3,     4,   170,   171,   292,    98,    99,   175,   296,
     147,   148,   149,   150,   240,   129,   242,   184,   129,   186,
      49,   147,   148,   149,   150,   128,   135,   136,   217,   218,
     130,   167,   168,   169,   100,   101,   266,   267,   175,   175,
     270,   271,   130,    38,   147,   148,   149,   150,   177,   175,
     217,   218,   292,   131,     6,   184,   296,   186,   298,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     293,   294,   175,    31,    32,   301,   135,   136,   245,   246,
     247,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,     4,   142,     8,   266,
     267,   146,   147,   270,   271,   392,   393,   296,   142,   135,
     136,   148,   146,   147,   303,   135,   136,   137,   138,   139,
     148,   141,   148,   260,   261,   134,   135,   136,   133,   296,
      34,    10,   268,   269,   260,   261,   303,   129,   130,   275,
     137,   138,   139,   430,   141,   132,   104,   105,   135,   136,
     141,     6,   392,   393,   343,   344,   427,   260,   261,   296,
      15,    16,    17,    18,    19,    20,    21,     3,    23,   132,
     296,   147,   135,   136,   128,   342,   343,   344,    50,   147,
     347,   348,   146,    15,    16,    17,    18,    19,    20,    21,
     430,    23,     8,   296,    98,    99,   100,   101,   102,   103,
      68,    69,    70,    71,    72,   342,     3,     8,   148,   132,
     347,   348,   135,   136,   148,   132,   342,   148,   148,   148,
     132,   347,   348,    68,    69,    70,    71,    72,   148,   133,
     134,   135,   136,   137,   138,   139,   132,   141,    51,   342,
       8,   148,   148,   132,   347,   348,    98,     3,   114,   148,
     148,     6,    52,   148,   421,   422,   423,   148,   425,     8,
     148,   148,    95,   148,    36,   211,    33,   135,   136,   432,
     175,    -1,    -1,    -1,   441,   442,   131,   406,    -1,    -1,
      -1,   410,   411,    -1,   421,   422,    -1,    -1,    -1,    -1,
      -1,   427,   459,    -1,    -1,   421,   422,   464,    -1,   131,
       3,     4,     5,     6,   441,   442,     9,   100,   101,   102,
     103,    14,    -1,    -1,    -1,   441,   442,    -1,   421,   422,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   441,   442,
     133,   134,   135,   136,   137,   138,   139,    -1,   141,    -1,
      -1,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,   102,   103,    86,    87,    88,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,   133,   134,   135,   136,   137,   138,   139,    -1,
     141,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,   140,     3,     4,
       5,     6,     7,    -1,     9,   148,    -1,    -1,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    -1,
      -1,    26,    -1,    -1,    29,    30,    31,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    86,    87,    88,    89,    90,    91,    -1,    -1,    94,
      -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
     125,    -1,    -1,    -1,   129,   130,   131,    -1,    -1,     3,
       4,   136,     6,     7,    37,   140,    -1,    -1,    -1,    -1,
     145,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      -1,    -1,    26,    -1,    -1,    29,    30,    31,    32,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,    -1,     6,     7,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    26,    -1,
      -1,    29,    30,    31,    32,    98,    99,   100,   101,   102,
     103,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      84,    -1,    -1,    -1,    -1,    -1,    90,    91,    -1,    -1,
      94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     133,   134,   135,   136,   137,   138,   139,    -1,   141,   113,
      -1,   115,    -1,    -1,    -1,    -1,    84,    -1,    -1,    -1,
     124,   125,    90,    91,    -1,    -1,    94,   131,    96,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   142,    -1,
      -1,   145,    -1,    -1,    -1,   113,    -1,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,     3,     4,
       5,     6,    -1,   131,     9,    -1,    11,    12,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   145,    -1,    24,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    -1,    -1,    -1,    -1,    -1,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    -1,
      -1,    86,    87,    88,    89,    -1,    -1,    92,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    -1,    -1,
      -1,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      -1,    -1,    -1,    -1,   129,   130,     3,     4,     5,     6,
      -1,   136,     9,    -1,    -1,   140,    -1,    14,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    -1,    -1,    86,
      87,    88,    89,     3,     4,     5,     6,    -1,    -1,     9,
      -1,    -1,    -1,    -1,    14,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    31,    32,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,   128,   129,   130,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,   140,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    -1,    -1,    86,    87,    88,    89,
       3,     4,     5,     6,    -1,    -1,     9,    -1,    -1,    -1,
      -1,    14,    -1,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,    -1,    -1,    -1,    -1,    31,    32,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
     140,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    -1,    -1,    86,    87,    88,    89,     3,     4,     5,
       6,    -1,    -1,     9,    -1,    -1,    -1,    -1,    14,    -1,
      -1,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,
      -1,   124,    -1,    -1,    -1,    -1,   129,   130,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,   140,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    -1,    -1,
      86,    87,    88,    89,     3,     4,     5,     6,    -1,    -1,
       9,    -1,    -1,    -1,    -1,    14,    -1,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    31,    32,    -1,    -1,    -1,    -1,   124,    -1,
      -1,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,   140,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    -1,
      -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    -1,    -1,    86,    87,    88,
      89,     3,     4,     5,    -1,    -1,    -1,     9,    -1,    -1,
      -1,    -1,    14,    -1,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,    31,
      32,    -1,    -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,
     129,   130,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,   140,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    -1,    -1,    86,    87,    88,    89,     3,     4,
       5,    -1,    -1,    -1,     9,    -1,    -1,    -1,    -1,    14,
      -1,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   128,   129,   130,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,   140,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    -1,    -1,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    -1,
      -1,    86,    87,    88,    89,     3,     4,     5,    -1,    -1,
      -1,     9,    -1,    -1,    -1,    -1,    14,    -1,    -1,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    -1,    -1,
      -1,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   129,   130,    -1,    -1,    -1,    -1,
      -1,   136,   137,    -1,    -1,   140,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    -1,    -1,    86,    87,
      88,    89,     3,     4,     5,    -1,    -1,    -1,     9,    -1,
      -1,    -1,    -1,    14,    -1,    -1,   104,   105,   106,   107,
     108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,
      31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   129,   130,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,   140,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    -1,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    -1,    -1,    86,    87,    88,    89,     3,
       4,     5,    -1,    -1,    -1,     9,    -1,    -1,    -1,    -1,
      14,    -1,    -1,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    -1,    -1,    -1,    31,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,   140,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      -1,    -1,    86,    87,    88,    89,     3,     4,     5,    -1,
      -1,    -1,     9,    -1,    -1,    -1,    -1,    14,    -1,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,    -1,
      -1,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,   140,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    -1,    -1,    86,
      87,    88,    89,     3,     4,     5,    -1,    -1,    -1,     9,
      -1,    -1,    -1,    -1,    14,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    31,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,   140,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    -1,    -1,    86,    87,    88,    89,
      -1,     3,     4,     5,    -1,    -1,    -1,     9,    -1,    -1,
      -1,    -1,    14,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,
     130,    -1,    -1,    -1,    -1,    -1,   136,    49,    -1,    -1,
     140,    -1,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    -1,    -1,    86,    87,    88,    89,    -1,     3,
       4,     5,    -1,    -1,    -1,     9,    -1,    -1,    -1,    -1,
      14,    -1,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   129,   130,    -1,
      -1,    -1,    -1,    -1,   136,    49,    -1,    -1,   140,    -1,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      -1,    -1,    86,    87,    88,    89,     3,     4,     5,    -1,
      -1,    -1,     9,    -1,    -1,    -1,    -1,    14,    -1,    -1,
     104,   105,   106,   107,   108,   109,   110,   111,   112,     6,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   129,   130,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,   140,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    -1,    -1,    86,
      87,    88,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    98,    99,   100,   101,   102,   103,    -1,    -1,    -1,
      -1,    -1,   129,   130,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    31,    32,   140,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    32,   133,   134,   135,   136,
     137,   138,   139,    -1,   141,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    -1,    -1,    66,    67,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    -1,    66,
      67,    49,    -1,    -1,    52,    -1,    86,    87,    88,    89,
      -1,    -1,    -1,    49,    -1,    -1,    52,    -1,    -1,    86,
      87,    88,    89,    -1,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    -1,    -1,    -1,    -1,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,   128,    -1,
      98,    99,   100,   101,   102,   103,    -1,    -1,    -1,    -1,
      -1,   128,    98,    99,   100,   101,   102,   103,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   132,   133,   134,   135,   136,   137,
     138,   139,    -1,   141,    -1,    -1,   132,   133,   134,   135,
     136,   137,   138,   139,    -1,   141,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    -1,    -1,
      66,    67,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    -1,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,    -1,    -1,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,   128,
      98,    99,   100,   101,   102,   103,    -1,    -1,    -1,    98,
      99,   100,   101,   102,   103,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    99,   100,
     101,   102,   103,    -1,    -1,   133,   134,   135,   136,   137,
     138,   139,    -1,   141,   133,   134,   135,   136,   137,   138,
     139,   149,   141,    98,    99,   100,   101,   102,   103,    -1,
     149,   132,   133,   134,   135,   136,   137,   138,   139,    -1,
     141,    98,    99,   100,   101,   102,   103,    -1,    -1,    -1,
      98,    99,   100,   101,   102,   103,    -1,   132,   133,   134,
     135,   136,   137,   138,   139,    98,   141,   100,   101,   102,
     103,    -1,    -1,    -1,    -1,    -1,   133,   134,   135,   136,
     137,   138,   139,    -1,   141,   133,   134,   135,   136,   137,
     138,   139,    98,   141,   100,   101,   102,   103,    -1,    -1,
     133,   134,   135,   136,   137,   138,   139,    -1,   141,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   133,   134,   135,
     136,   137,   138,   139,    -1,   141
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   142,   151,   152,   154,   155,     0,     3,     4,     6,
       7,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      26,    29,    30,    31,    32,    84,    90,    91,    94,    96,
     113,   115,   124,   125,   131,   145,   153,   156,   165,   168,
     174,   178,   180,   142,   144,   143,   144,   143,     6,    85,
     157,     3,     4,     5,     9,    14,    31,    32,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    86,    87,    88,    89,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   129,   130,   136,   140,
     166,   167,   180,   182,   183,   185,   186,   187,   188,   189,
     190,   195,   196,   197,   198,   199,     4,   129,   140,   171,
     172,   180,   186,   197,   199,     4,   197,   129,   166,   179,
     175,   154,   155,     4,   132,    97,    11,    12,    13,    24,
      25,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    92,    93,   116,   117,   118,   119,   120,
     121,   122,   123,   180,   197,   154,   171,   143,     3,     4,
     164,    85,   160,     8,   130,   197,   130,   197,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,    31,
      32,   104,   105,    31,    32,   104,   105,   171,   182,   185,
     186,   197,   199,   171,   197,   197,   167,   135,   136,    68,
      69,    70,    71,    72,   128,   195,   196,     6,    14,    15,
      16,    17,    18,    19,    20,    21,    23,   131,   191,   191,
      49,   187,    49,    49,    52,    98,    99,   100,   101,   102,
     103,   132,   133,   134,   135,   136,   137,   138,   139,   141,
     184,   133,   143,   172,   197,    38,    98,    99,   100,   101,
      98,    99,   180,     6,   152,   154,   152,   174,     4,   163,
     197,   197,   197,   197,   197,   197,   197,   197,   182,   197,
     182,   182,     3,     9,    14,   128,   129,   182,   192,   193,
     194,   195,   196,   129,   185,   197,   197,   197,   197,   197,
     197,   180,   180,   180,   197,   197,     3,     4,    31,    32,
     174,   182,   183,   186,   171,   158,     8,   197,   197,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   148,   171,
     171,   148,   132,   135,   136,   148,   148,   184,   133,    10,
     185,   185,     3,   147,   187,   187,    50,   197,   197,   197,
     197,   197,   197,   197,   197,   197,   197,   197,   197,   197,
     197,   197,   182,   182,   197,   169,   172,   197,   172,   197,
     180,   180,   172,   197,   172,   197,   181,   147,   146,     8,
     193,   195,   191,   191,     3,   193,   128,   195,   185,   197,
       8,   161,   149,   149,   148,   148,   132,   148,   148,   148,
     132,   132,   148,   148,   148,   182,   185,   185,   182,   182,
      51,    53,   132,    34,     8,   132,   148,   176,   193,   193,
     132,     3,   159,   114,   170,   171,   171,   171,   148,   148,
     148,    53,   132,    52,   182,   182,   197,   197,   174,   177,
     193,   170,     6,   148,   148,   148,   182,   182,   134,    37,
     173,   148,   148,   134,   137,   197,    95,   148,   197,   162,
       8
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   150,   151,   151,   152,   153,   153,   154,   154,   155,
     155,   156,   156,   156,   156,   156,   156,   156,   156,   156,
     156,   157,   156,   156,   158,   159,   156,   160,   161,   156,
     162,   156,   156,   163,   156,   156,   156,   164,   164,   165,
     165,   165,   166,   166,   167,   167,   167,   169,   168,   170,
     170,   171,   171,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   173,   173,   173,   174,   174,   174,   174,   175,
     176,   174,   174,   177,   177,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   179,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   180,   180,
     181,   181,   182,   182,   182,   183,   183,   183,   183,   183,
     183,   183,   183,   183,   183,   184,   184,   185,   185,   186,
     186,   186,   186,   186,   186,   186,   187,   187,   187,   188,
     188,   189,   189,   190,   190,   191,   191,   191,   191,   191,
     191,   191,   191,   191,   191,   192,   192,   193,   193,   193,
     194,   194,   194,   194,   194,   194,   195,   195,   195,   195,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   195,   195,   195,   195,   195,   195,   195,   195,
     195,   195,   195,   195,   195,   195,   195,   195,   196,   196,
     196,   196,   197,   197,   198,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199,
     199,   199,   199,   199,   199,   199,   199,   199,   199,   199
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     3,     1,     3,     0,     1,     1,
       2,     3,     3,     4,     1,     1,     1,     1,     1,     2,
       2,     0,     3,     2,     0,     0,     7,     0,     0,     6,
       0,    10,     1,     0,     4,     1,     1,     1,     1,     2,
       2,     3,     1,     2,     1,     1,     1,     0,     5,     0,
       2,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     0,     2,     3,     1,     4,     4,     4,     0,
       0,     6,     1,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     3,     0,     4,     3,
       3,     3,     3,     2,     2,     3,     2,     3,     2,     3,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     3,     2,     3,     2,     3,     3,     3,     3,
       3,     3,     2,     3,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     3,     2,     1,     5,
       0,     3,     1,     1,     3,     1,     3,     5,     3,     5,
       5,     5,     7,     6,     8,     1,     4,     3,     3,     1,
       2,     2,     2,     3,     3,     1,     1,     1,     3,     1,
       3,     1,     2,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     2,     3,     1,     1,     2,
       1,     5,     4,     3,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     1,     1,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     2,     4,
       2,     4,     1,     1,     3,     1,     1,     2,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     2,     3,
       4,     4,     6,     4,     4,     4,     6,     6,     4,     4,
       3,     4,     3,     3,     3,     3,     3,     3,     3,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 3: /* top: element_list  */
#line 313 "../src/preproc/pic/pic.ypp"
                {
		  if (olist.head)
		    print_picture(olist.head);
		}
#line 2475 "src/preproc/pic/pic.cpp"
    break;

  case 4: /* element_list: optional_separator middle_element_list optional_separator  */
#line 322 "../src/preproc/pic/pic.ypp"
                { (yyval.pl) = (yyvsp[-1].pl); }
#line 2481 "src/preproc/pic/pic.cpp"
    break;

  case 5: /* middle_element_list: element  */
#line 327 "../src/preproc/pic/pic.ypp"
                { (yyval.pl) = (yyvsp[0].pl); }
#line 2487 "src/preproc/pic/pic.cpp"
    break;

  case 6: /* middle_element_list: middle_element_list separator element  */
#line 329 "../src/preproc/pic/pic.ypp"
                { (yyval.pl) = (yyvsp[-2].pl); }
#line 2493 "src/preproc/pic/pic.cpp"
    break;

  case 11: /* placeless_element: FIGNAME '=' macro_name  */
#line 344 "../src/preproc/pic/pic.ypp"
                {
		  delete[] graphname;
		  graphname = new char[strlen((yyvsp[0].str)) + 1];
		  strcpy(graphname, (yyvsp[0].str));
		  delete[] (yyvsp[0].str);
		}
#line 2504 "src/preproc/pic/pic.cpp"
    break;

  case 12: /* placeless_element: VARIABLE '=' any_expr  */
#line 352 "../src/preproc/pic/pic.ypp"
                {
		  define_variable((yyvsp[-2].str), (yyvsp[0].x));
		  free((yyvsp[-2].str));
		}
#line 2513 "src/preproc/pic/pic.cpp"
    break;

  case 13: /* placeless_element: VARIABLE ':' '=' any_expr  */
#line 357 "../src/preproc/pic/pic.ypp"
                {
		  place *p = lookup_label((yyvsp[-3].str));
		  if (!p) {
		    lex_error("variable '%1' not defined", (yyvsp[-3].str));
		    YYABORT;
		  }
		  p->obj = 0 /* nullptr */;
		  p->x = (yyvsp[0].x);
		  p->y = 0.0;
		  free((yyvsp[-3].str));
		}
#line 2529 "src/preproc/pic/pic.cpp"
    break;

  case 14: /* placeless_element: UP  */
#line 369 "../src/preproc/pic/pic.ypp"
                { current_direction = UP_DIRECTION; }
#line 2535 "src/preproc/pic/pic.cpp"
    break;

  case 15: /* placeless_element: DOWN  */
#line 371 "../src/preproc/pic/pic.ypp"
                { current_direction = DOWN_DIRECTION; }
#line 2541 "src/preproc/pic/pic.cpp"
    break;

  case 16: /* placeless_element: LEFT  */
#line 373 "../src/preproc/pic/pic.ypp"
                { current_direction = LEFT_DIRECTION; }
#line 2547 "src/preproc/pic/pic.cpp"
    break;

  case 17: /* placeless_element: RIGHT  */
#line 375 "../src/preproc/pic/pic.ypp"
                { current_direction = RIGHT_DIRECTION; }
#line 2553 "src/preproc/pic/pic.cpp"
    break;

  case 18: /* placeless_element: COMMAND_LINE  */
#line 377 "../src/preproc/pic/pic.ypp"
                {
		  olist.append(make_command_object((yyvsp[0].lstr).str, (yyvsp[0].lstr).filename,
						   (yyvsp[0].lstr).lineno));
		}
#line 2562 "src/preproc/pic/pic.cpp"
    break;

  case 19: /* placeless_element: COMMAND print_args  */
#line 382 "../src/preproc/pic/pic.ypp"
                {
		  olist.append(make_command_object((yyvsp[0].lstr).str, (yyvsp[0].lstr).filename,
						   (yyvsp[0].lstr).lineno));
		}
#line 2571 "src/preproc/pic/pic.cpp"
    break;

  case 20: /* placeless_element: PRINT print_args  */
#line 387 "../src/preproc/pic/pic.ypp"
                {
		  fprintf(stderr, "%s\n", (yyvsp[0].lstr).str);
		  delete[] (yyvsp[0].lstr).str;
		  fflush(stderr);
		}
#line 2581 "src/preproc/pic/pic.cpp"
    break;

  case 21: /* $@1: %empty  */
#line 393 "../src/preproc/pic/pic.ypp"
                { delim_flag = 1; }
#line 2587 "src/preproc/pic/pic.cpp"
    break;

  case 22: /* placeless_element: SH $@1 DELIMITED  */
#line 395 "../src/preproc/pic/pic.ypp"
                {
		  delim_flag = 0;
		  if (!want_unsafe_mode)
		    lex_error("unsafe to run command '%1'; ignoring",
			      (yyvsp[0].str));
		  else {
		    int retval = system((yyvsp[0].str));
		    if (retval < 0)
		      lex_error("error running command '%1': system()"
			" returned %2", (yyvsp[0].str), retval);
		  }
		  delete[] (yyvsp[0].str);
		}
#line 2605 "src/preproc/pic/pic.cpp"
    break;

  case 23: /* placeless_element: COPY TEXT  */
#line 409 "../src/preproc/pic/pic.ypp"
                {
		  if (yychar < 0)
		    do_lookahead();
		  do_copy((yyvsp[0].lstr).str);
		  // do not delete the filename
		}
#line 2616 "src/preproc/pic/pic.cpp"
    break;

  case 24: /* $@2: %empty  */
#line 416 "../src/preproc/pic/pic.ypp"
                { delim_flag = 2; }
#line 2622 "src/preproc/pic/pic.cpp"
    break;

  case 25: /* $@3: %empty  */
#line 418 "../src/preproc/pic/pic.ypp"
                { delim_flag = 0; }
#line 2628 "src/preproc/pic/pic.cpp"
    break;

  case 26: /* placeless_element: COPY TEXT THRU $@2 DELIMITED $@3 until  */
#line 420 "../src/preproc/pic/pic.ypp"
                {
		  if (yychar < 0)
		    do_lookahead();
		  copy_file_thru((yyvsp[-5].lstr).str, (yyvsp[-2].str), (yyvsp[0].str));
		  // do not delete the filename
		  delete[] (yyvsp[-2].str);
		  delete[] (yyvsp[0].str);
		}
#line 2641 "src/preproc/pic/pic.cpp"
    break;

  case 27: /* $@4: %empty  */
#line 429 "../src/preproc/pic/pic.ypp"
                { delim_flag = 2; }
#line 2647 "src/preproc/pic/pic.cpp"
    break;

  case 28: /* $@5: %empty  */
#line 431 "../src/preproc/pic/pic.ypp"
                { delim_flag = 0; }
#line 2653 "src/preproc/pic/pic.cpp"
    break;

  case 29: /* placeless_element: COPY THRU $@4 DELIMITED $@5 until  */
#line 433 "../src/preproc/pic/pic.ypp"
                {
		  if (yychar < 0)
		    do_lookahead();
		  copy_rest_thru((yyvsp[-2].str), (yyvsp[0].str));
		  delete[] (yyvsp[-2].str);
		  delete[] (yyvsp[0].str);
		}
#line 2665 "src/preproc/pic/pic.cpp"
    break;

  case 30: /* $@6: %empty  */
#line 441 "../src/preproc/pic/pic.ypp"
                { delim_flag = 1; }
#line 2671 "src/preproc/pic/pic.cpp"
    break;

  case 31: /* placeless_element: FOR VARIABLE '=' expr TO expr optional_by DO $@6 DELIMITED  */
#line 443 "../src/preproc/pic/pic.ypp"
                {
		  delim_flag = 0;
		  if (yychar < 0)
		    do_lookahead();
		  do_for((yyvsp[-8].str), (yyvsp[-6].x), (yyvsp[-4].x), (yyvsp[-3].by).is_multiplicative, (yyvsp[-3].by).val, (yyvsp[0].str));
		}
#line 2682 "src/preproc/pic/pic.cpp"
    break;

  case 32: /* placeless_element: simple_if  */
#line 450 "../src/preproc/pic/pic.ypp"
                {
		  if (yychar < 0)
		    do_lookahead();
		  if ((yyvsp[0].if_data).x != 0.0)
		    push_body((yyvsp[0].if_data).body);
		  delete[] (yyvsp[0].if_data).body;
		}
#line 2694 "src/preproc/pic/pic.cpp"
    break;

  case 33: /* $@7: %empty  */
#line 458 "../src/preproc/pic/pic.ypp"
                { delim_flag = 1; }
#line 2700 "src/preproc/pic/pic.cpp"
    break;

  case 34: /* placeless_element: simple_if ELSE $@7 DELIMITED  */
#line 460 "../src/preproc/pic/pic.ypp"
                {
		  delim_flag = 0;
		  if (yychar < 0)
		    do_lookahead();
		  if ((yyvsp[-3].if_data).x != 0.0)
		    push_body((yyvsp[-3].if_data).body);
		  else
		    push_body((yyvsp[0].str));
		  free((yyvsp[-3].if_data).body);
		  free((yyvsp[0].str));
		}
#line 2716 "src/preproc/pic/pic.cpp"
    break;

  case 36: /* placeless_element: RESET  */
#line 473 "../src/preproc/pic/pic.ypp"
                { define_variable("scale", 1.0); }
#line 2722 "src/preproc/pic/pic.cpp"
    break;

  case 39: /* reset_variables: RESET VARIABLE  */
#line 483 "../src/preproc/pic/pic.ypp"
                {
		  reset((yyvsp[0].str));
		  delete[] (yyvsp[0].str);
		}
#line 2731 "src/preproc/pic/pic.cpp"
    break;

  case 40: /* reset_variables: reset_variables VARIABLE  */
#line 488 "../src/preproc/pic/pic.ypp"
                {
		  reset((yyvsp[0].str));
		  delete[] (yyvsp[0].str);
		}
#line 2740 "src/preproc/pic/pic.cpp"
    break;

  case 41: /* reset_variables: reset_variables ',' VARIABLE  */
#line 493 "../src/preproc/pic/pic.ypp"
                {
		  reset((yyvsp[0].str));
		  delete[] (yyvsp[0].str);
		}
#line 2749 "src/preproc/pic/pic.cpp"
    break;

  case 42: /* print_args: print_arg  */
#line 501 "../src/preproc/pic/pic.ypp"
                { (yyval.lstr) = (yyvsp[0].lstr); }
#line 2755 "src/preproc/pic/pic.cpp"
    break;

  case 43: /* print_args: print_args print_arg  */
#line 503 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.lstr).str = new char[strlen((yyvsp[-1].lstr).str) + strlen((yyvsp[0].lstr).str) + 1];
		  strcpy((yyval.lstr).str, (yyvsp[-1].lstr).str);
		  strcat((yyval.lstr).str, (yyvsp[0].lstr).str);
		  delete[] (yyvsp[-1].lstr).str;
		  delete[] (yyvsp[0].lstr).str;
		  if ((yyvsp[-1].lstr).filename) {
		    (yyval.lstr).filename = (yyvsp[-1].lstr).filename;
		    (yyval.lstr).lineno = (yyvsp[-1].lstr).lineno;
		  }
		  else if ((yyvsp[0].lstr).filename) {
		    (yyval.lstr).filename = (yyvsp[0].lstr).filename;
		    (yyval.lstr).lineno = (yyvsp[0].lstr).lineno;
		  }
		}
#line 2775 "src/preproc/pic/pic.cpp"
    break;

  case 44: /* print_arg: expr  */
#line 522 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.lstr).str = new char[GDIGITS + 1];
		  sprintf((yyval.lstr).str, "%g", (yyvsp[0].x));
		  (yyval.lstr).filename = 0 /* nullptr */;
		  (yyval.lstr).lineno = 0;
		}
#line 2786 "src/preproc/pic/pic.cpp"
    break;

  case 45: /* print_arg: text  */
#line 529 "../src/preproc/pic/pic.ypp"
                { (yyval.lstr) = (yyvsp[0].lstr); }
#line 2792 "src/preproc/pic/pic.cpp"
    break;

  case 46: /* print_arg: position  */
#line 531 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.lstr).str = new char[GDIGITS + 2 + GDIGITS + 1];
		  sprintf((yyval.lstr).str, "%g, %g", (yyvsp[0].pair).x, (yyvsp[0].pair).y);
		  (yyval.lstr).filename = 0 /* nullptr */;
		  (yyval.lstr).lineno = 0;
		}
#line 2803 "src/preproc/pic/pic.cpp"
    break;

  case 47: /* $@8: %empty  */
#line 541 "../src/preproc/pic/pic.ypp"
                { delim_flag = 1; }
#line 2809 "src/preproc/pic/pic.cpp"
    break;

  case 48: /* simple_if: IF any_expr THEN $@8 DELIMITED  */
#line 543 "../src/preproc/pic/pic.ypp"
                {
		  delim_flag = 0;
		  (yyval.if_data).x = (yyvsp[-3].x);
		  (yyval.if_data).body = (yyvsp[0].str);
		}
#line 2819 "src/preproc/pic/pic.cpp"
    break;

  case 49: /* until: %empty  */
#line 552 "../src/preproc/pic/pic.ypp"
                { (yyval.str) = 0; }
#line 2825 "src/preproc/pic/pic.cpp"
    break;

  case 50: /* until: UNTIL TEXT  */
#line 554 "../src/preproc/pic/pic.ypp"
                { (yyval.str) = (yyvsp[0].lstr).str; }
#line 2831 "src/preproc/pic/pic.cpp"
    break;

  case 51: /* any_expr: expr  */
#line 559 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[0].x); }
#line 2837 "src/preproc/pic/pic.cpp"
    break;

  case 52: /* any_expr: text_expr  */
#line 561 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[0].x); }
#line 2843 "src/preproc/pic/pic.cpp"
    break;

  case 53: /* text_expr: text EQUALEQUAL text  */
#line 566 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.x) = strcmp((yyvsp[-2].lstr).str, (yyvsp[0].lstr).str) == 0;
		  delete[] (yyvsp[-2].lstr).str;
		  delete[] (yyvsp[0].lstr).str;
		}
#line 2853 "src/preproc/pic/pic.cpp"
    break;

  case 54: /* text_expr: text NOTEQUAL text  */
#line 572 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.x) = strcmp((yyvsp[-2].lstr).str, (yyvsp[0].lstr).str) != 0;
		  delete[] (yyvsp[-2].lstr).str;
		  delete[] (yyvsp[0].lstr).str;
		}
#line 2863 "src/preproc/pic/pic.cpp"
    break;

  case 55: /* text_expr: text_expr ANDAND text_expr  */
#line 578 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 && (yyvsp[0].x) != 0.0); }
#line 2869 "src/preproc/pic/pic.cpp"
    break;

  case 56: /* text_expr: text_expr ANDAND expr  */
#line 580 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 && (yyvsp[0].x) != 0.0); }
#line 2875 "src/preproc/pic/pic.cpp"
    break;

  case 57: /* text_expr: expr ANDAND text_expr  */
#line 582 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 && (yyvsp[0].x) != 0.0); }
#line 2881 "src/preproc/pic/pic.cpp"
    break;

  case 58: /* text_expr: text_expr OROR text_expr  */
#line 584 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 || (yyvsp[0].x) != 0.0); }
#line 2887 "src/preproc/pic/pic.cpp"
    break;

  case 59: /* text_expr: text_expr OROR expr  */
#line 586 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 || (yyvsp[0].x) != 0.0); }
#line 2893 "src/preproc/pic/pic.cpp"
    break;

  case 60: /* text_expr: expr OROR text_expr  */
#line 588 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 || (yyvsp[0].x) != 0.0); }
#line 2899 "src/preproc/pic/pic.cpp"
    break;

  case 61: /* text_expr: '!' text_expr  */
#line 590 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[0].x) == 0.0); }
#line 2905 "src/preproc/pic/pic.cpp"
    break;

  case 62: /* optional_by: %empty  */
#line 596 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.by).val = 1.0;
		  (yyval.by).is_multiplicative = 0;
		}
#line 2914 "src/preproc/pic/pic.cpp"
    break;

  case 63: /* optional_by: BY expr  */
#line 601 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.by).val = (yyvsp[0].x);
		  (yyval.by).is_multiplicative = 0;
		}
#line 2923 "src/preproc/pic/pic.cpp"
    break;

  case 64: /* optional_by: BY '*' expr  */
#line 606 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.by).val = (yyvsp[0].x);
		  (yyval.by).is_multiplicative = 1;
		}
#line 2932 "src/preproc/pic/pic.cpp"
    break;

  case 65: /* element: object_spec  */
#line 614 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pl).obj = (yyvsp[0].spec)->make_object(&current_position,
					   &current_direction);
		  if ((yyval.pl).obj == 0 /* nullptr */)
		    YYABORT;
		  delete (yyvsp[0].spec);
		  if ((yyval.pl).obj)
		    olist.append((yyval.pl).obj);
		  else {
		    (yyval.pl).x = current_position.x;
		    (yyval.pl).y = current_position.y;
		  }
		}
#line 2950 "src/preproc/pic/pic.cpp"
    break;

  case 66: /* element: LABEL ':' optional_separator element  */
#line 628 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pl) = (yyvsp[0].pl);
		  define_label((yyvsp[-3].str), & (yyval.pl));
		  free((yyvsp[-3].str));
		}
#line 2960 "src/preproc/pic/pic.cpp"
    break;

  case 67: /* element: LABEL ':' optional_separator position_not_place  */
#line 634 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pl).obj = 0 /* nullptr */;
		  (yyval.pl).x = (yyvsp[0].pair).x;
		  (yyval.pl).y = (yyvsp[0].pair).y;
		  define_label((yyvsp[-3].str), & (yyval.pl));
		  free((yyvsp[-3].str));
		}
#line 2972 "src/preproc/pic/pic.cpp"
    break;

  case 68: /* element: LABEL ':' optional_separator place  */
#line 642 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pl) = (yyvsp[0].pl);
		  define_label((yyvsp[-3].str), & (yyval.pl));
		  free((yyvsp[-3].str));
		}
#line 2982 "src/preproc/pic/pic.cpp"
    break;

  case 69: /* @9: %empty  */
#line 648 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.state).x = current_position.x;
		  (yyval.state).y = current_position.y;
		  (yyval.state).dir = current_direction;
		}
#line 2992 "src/preproc/pic/pic.cpp"
    break;

  case 70: /* $@10: %empty  */
#line 654 "../src/preproc/pic/pic.ypp"
                {
		  current_position.x = (yyvsp[-2].state).x;
		  current_position.y = (yyvsp[-2].state).y;
		  current_direction = (yyvsp[-2].state).dir;
		}
#line 3002 "src/preproc/pic/pic.cpp"
    break;

  case 71: /* element: '{' @9 element_list '}' $@10 optional_element  */
#line 660 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pl) = (yyvsp[-3].pl);
		}
#line 3010 "src/preproc/pic/pic.cpp"
    break;

  case 72: /* element: placeless_element  */
#line 664 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pl).obj = 0 /* nullptr */;
		  (yyval.pl).x = current_position.x;
		  (yyval.pl).y = current_position.y;
		}
#line 3020 "src/preproc/pic/pic.cpp"
    break;

  case 73: /* optional_element: %empty  */
#line 673 "../src/preproc/pic/pic.ypp"
                {}
#line 3026 "src/preproc/pic/pic.cpp"
    break;

  case 74: /* optional_element: element  */
#line 675 "../src/preproc/pic/pic.ypp"
                {}
#line 3032 "src/preproc/pic/pic.cpp"
    break;

  case 75: /* object_spec: BOX  */
#line 680 "../src/preproc/pic/pic.ypp"
                { (yyval.spec) = new object_spec(BOX_OBJECT); }
#line 3038 "src/preproc/pic/pic.cpp"
    break;

  case 76: /* object_spec: CIRCLE  */
#line 682 "../src/preproc/pic/pic.ypp"
                { (yyval.spec) = new object_spec(CIRCLE_OBJECT); }
#line 3044 "src/preproc/pic/pic.cpp"
    break;

  case 77: /* object_spec: ELLIPSE  */
#line 684 "../src/preproc/pic/pic.ypp"
                { (yyval.spec) = new object_spec(ELLIPSE_OBJECT); }
#line 3050 "src/preproc/pic/pic.cpp"
    break;

  case 78: /* object_spec: ARC  */
#line 686 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(ARC_OBJECT);
		  (yyval.spec)->dir = current_direction;
		}
#line 3059 "src/preproc/pic/pic.cpp"
    break;

  case 79: /* object_spec: LINE  */
#line 691 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(LINE_OBJECT);
		  lookup_variable("lineht", & (yyval.spec)->segment_height);
		  lookup_variable("linewid", & (yyval.spec)->segment_width);
		  (yyval.spec)->dir = current_direction;
		}
#line 3070 "src/preproc/pic/pic.cpp"
    break;

  case 80: /* object_spec: POLYGON  */
#line 698 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(POLYGON_OBJECT);
		  lookup_variable("lineht", & (yyval.spec)->segment_height);
		  lookup_variable("linewid", & (yyval.spec)->segment_width);
		  (yyval.spec)->dir = current_direction;
		}
#line 3081 "src/preproc/pic/pic.cpp"
    break;

  case 81: /* object_spec: ARROW  */
#line 705 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(ARROW_OBJECT);
		  lookup_variable("lineht", & (yyval.spec)->segment_height);
		  lookup_variable("linewid", & (yyval.spec)->segment_width);
		  (yyval.spec)->dir = current_direction;
		}
#line 3092 "src/preproc/pic/pic.cpp"
    break;

  case 82: /* object_spec: MOVE  */
#line 712 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(MOVE_OBJECT);
		  lookup_variable("moveht", & (yyval.spec)->segment_height);
		  lookup_variable("movewid", & (yyval.spec)->segment_width);
		  (yyval.spec)->dir = current_direction;
		}
#line 3103 "src/preproc/pic/pic.cpp"
    break;

  case 83: /* object_spec: SPLINE  */
#line 719 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(SPLINE_OBJECT);
		  lookup_variable("lineht", & (yyval.spec)->segment_height);
		  lookup_variable("linewid", & (yyval.spec)->segment_width);
		  (yyval.spec)->dir = current_direction;
		}
#line 3114 "src/preproc/pic/pic.cpp"
    break;

  case 84: /* object_spec: text  */
#line 726 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(TEXT_OBJECT);
		  (yyval.spec)->text = new text_item((yyvsp[0].lstr).str, (yyvsp[0].lstr).filename, (yyvsp[0].lstr).lineno);
		}
#line 3123 "src/preproc/pic/pic.cpp"
    break;

  case 85: /* object_spec: PLOT expr  */
#line 731 "../src/preproc/pic/pic.ypp"
                {
		  lex_warning("'plot' is deprecated; use 'sprintf'"
			      " instead");
		  (yyval.spec) = new object_spec(TEXT_OBJECT);
		  (yyval.spec)->text = new text_item(format_number(0, (yyvsp[0].x)), 0, -1);
		}
#line 3134 "src/preproc/pic/pic.cpp"
    break;

  case 86: /* object_spec: PLOT expr text  */
#line 738 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = new object_spec(TEXT_OBJECT);
		  (yyval.spec)->text = new text_item(format_number((yyvsp[0].lstr).str, (yyvsp[-1].x)),
					   (yyvsp[0].lstr).filename, (yyvsp[0].lstr).lineno);
		  delete[] (yyvsp[0].lstr).str;
		}
#line 3145 "src/preproc/pic/pic.cpp"
    break;

  case 87: /* @11: %empty  */
#line 745 "../src/preproc/pic/pic.ypp"
                {
		  saved_state *p = new saved_state;
		  (yyval.pstate) = p;
		  p->x = current_position.x;
		  p->y = current_position.y;
		  p->dir = current_direction;
		  p->tbl = current_table;
		  p->prev = current_saved_state;
		  current_position.x = 0.0;
		  current_position.y = 0.0;
		  current_table = new PTABLE(place);
		  current_saved_state = p;
		  olist.append(make_mark_object());
		}
#line 3164 "src/preproc/pic/pic.cpp"
    break;

  case 88: /* object_spec: '[' @11 element_list ']'  */
#line 760 "../src/preproc/pic/pic.ypp"
                {
		  current_position.x = (yyvsp[-2].pstate)->x;
		  current_position.y = (yyvsp[-2].pstate)->y;
		  current_direction = (yyvsp[-2].pstate)->dir;
		  (yyval.spec) = new object_spec(BLOCK_OBJECT);
		  olist.wrap_up_block(& (yyval.spec)->oblist);
		  (yyval.spec)->tbl = current_table;
		  current_table = (yyvsp[-2].pstate)->tbl;
		  current_saved_state = (yyvsp[-2].pstate)->prev;
		  delete (yyvsp[-2].pstate);
		}
#line 3180 "src/preproc/pic/pic.cpp"
    break;

  case 89: /* object_spec: object_spec HEIGHT expr  */
#line 772 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->height = (yyvsp[0].x);
		  (yyval.spec)->flags |= HAS_HEIGHT;
		}
#line 3190 "src/preproc/pic/pic.cpp"
    break;

  case 90: /* object_spec: object_spec RADIUS expr  */
#line 778 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->radius = (yyvsp[0].x);
		  (yyval.spec)->flags |= HAS_RADIUS;
		}
#line 3200 "src/preproc/pic/pic.cpp"
    break;

  case 91: /* object_spec: object_spec WIDTH expr  */
#line 784 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->width = (yyvsp[0].x);
		  (yyval.spec)->flags |= HAS_WIDTH;
		}
#line 3210 "src/preproc/pic/pic.cpp"
    break;

  case 92: /* object_spec: object_spec DIAMETER expr  */
#line 790 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->radius = (yyvsp[0].x)/2.0;
		  (yyval.spec)->flags |= HAS_RADIUS;
		}
#line 3220 "src/preproc/pic/pic.cpp"
    break;

  case 93: /* object_spec: object_spec expr  */
#line 796 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  switch ((yyval.spec)->dir) {
		  case UP_DIRECTION:
		    (yyval.spec)->segment_pos.y += (yyvsp[0].x);
		    break;
		  case DOWN_DIRECTION:
		    (yyval.spec)->segment_pos.y -= (yyvsp[0].x);
		    break;
		  case RIGHT_DIRECTION:
		    (yyval.spec)->segment_pos.x += (yyvsp[0].x);
		    break;
		  case LEFT_DIRECTION:
		    (yyval.spec)->segment_pos.x -= (yyvsp[0].x);
		    break;
		  }
		}
#line 3243 "src/preproc/pic/pic.cpp"
    break;

  case 94: /* object_spec: object_spec UP  */
#line 815 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->dir = UP_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.y += (yyval.spec)->segment_height;
		}
#line 3254 "src/preproc/pic/pic.cpp"
    break;

  case 95: /* object_spec: object_spec UP expr  */
#line 822 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->dir = UP_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.y += (yyvsp[0].x);
		}
#line 3265 "src/preproc/pic/pic.cpp"
    break;

  case 96: /* object_spec: object_spec DOWN  */
#line 829 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->dir = DOWN_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.y -= (yyval.spec)->segment_height;
		}
#line 3276 "src/preproc/pic/pic.cpp"
    break;

  case 97: /* object_spec: object_spec DOWN expr  */
#line 836 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->dir = DOWN_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.y -= (yyvsp[0].x);
		}
#line 3287 "src/preproc/pic/pic.cpp"
    break;

  case 98: /* object_spec: object_spec RIGHT  */
#line 843 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->dir = RIGHT_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.x += (yyval.spec)->segment_width;
		}
#line 3298 "src/preproc/pic/pic.cpp"
    break;

  case 99: /* object_spec: object_spec RIGHT expr  */
#line 850 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->dir = RIGHT_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.x += (yyvsp[0].x);
		}
#line 3309 "src/preproc/pic/pic.cpp"
    break;

  case 100: /* object_spec: object_spec LEFT  */
#line 857 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->dir = LEFT_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.x -= (yyval.spec)->segment_width;
		}
#line 3320 "src/preproc/pic/pic.cpp"
    break;

  case 101: /* object_spec: object_spec LEFT expr  */
#line 864 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->dir = LEFT_DIRECTION;
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.x -= (yyvsp[0].x);
		}
#line 3331 "src/preproc/pic/pic.cpp"
    break;

  case 102: /* object_spec: object_spec FROM position  */
#line 871 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= HAS_FROM;
		  (yyval.spec)->from.x = (yyvsp[0].pair).x;
		  (yyval.spec)->from.y = (yyvsp[0].pair).y;
		}
#line 3342 "src/preproc/pic/pic.cpp"
    break;

  case 103: /* object_spec: object_spec TO position  */
#line 878 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  if ((yyval.spec)->flags & HAS_SEGMENT)
		    (yyval.spec)->segment_list = new segment((yyval.spec)->segment_pos,
						   (yyval.spec)->segment_is_absolute,
						   (yyval.spec)->segment_list);
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.x = (yyvsp[0].pair).x;
		  (yyval.spec)->segment_pos.y = (yyvsp[0].pair).y;
		  (yyval.spec)->segment_is_absolute = 1;
		  (yyval.spec)->flags |= HAS_TO;
		  (yyval.spec)->to.x = (yyvsp[0].pair).x;
		  (yyval.spec)->to.y = (yyvsp[0].pair).y;
		}
#line 3361 "src/preproc/pic/pic.cpp"
    break;

  case 104: /* object_spec: object_spec AT position  */
#line 893 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= HAS_AT;
		  (yyval.spec)->at.x = (yyvsp[0].pair).x;
		  (yyval.spec)->at.y = (yyvsp[0].pair).y;
		  if ((yyval.spec)->type != ARC_OBJECT) {
		    (yyval.spec)->flags |= HAS_FROM;
		    (yyval.spec)->from.x = (yyvsp[0].pair).x;
		    (yyval.spec)->from.y = (yyvsp[0].pair).y;
		  }
		}
#line 3377 "src/preproc/pic/pic.cpp"
    break;

  case 105: /* object_spec: object_spec WITH path  */
#line 905 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= HAS_WITH;
		  (yyval.spec)->with = (yyvsp[0].pth);
		}
#line 3387 "src/preproc/pic/pic.cpp"
    break;

  case 106: /* object_spec: object_spec WITH vertex  */
#line 911 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= HAS_WITH;
		  (yyval.spec)->with = new path((yyvsp[0].ver).crn);
		  (yyval.spec)->vertex_number = (yyvsp[0].ver).vertex_number;
		  (yyval.spec)->is_edge = (yyvsp[0].ver).is_edge;
		}
#line 3399 "src/preproc/pic/pic.cpp"
    break;

  case 107: /* object_spec: object_spec WITH position  */
#line 919 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= HAS_WITH;
		  position pos;
		  pos.x = (yyvsp[0].pair).x;
		  pos.y = (yyvsp[0].pair).y;
		  (yyval.spec)->with = new path(pos);
		}
#line 3412 "src/preproc/pic/pic.cpp"
    break;

  case 108: /* object_spec: object_spec BY expr_pair  */
#line 928 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= HAS_SEGMENT;
		  (yyval.spec)->segment_pos.x += (yyvsp[0].pair).x;
		  (yyval.spec)->segment_pos.y += (yyvsp[0].pair).y;
		}
#line 3423 "src/preproc/pic/pic.cpp"
    break;

  case 109: /* object_spec: object_spec THEN  */
#line 935 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  if (!((yyval.spec)->flags & HAS_SEGMENT))
		    switch ((yyval.spec)->dir) {
		    case UP_DIRECTION:
		      (yyval.spec)->segment_pos.y += (yyval.spec)->segment_width;
		      break;
		    case DOWN_DIRECTION:
		      (yyval.spec)->segment_pos.y -= (yyval.spec)->segment_width;
		      break;
		    case RIGHT_DIRECTION:
		      (yyval.spec)->segment_pos.x += (yyval.spec)->segment_width;
		      break;
		    case LEFT_DIRECTION:
		      (yyval.spec)->segment_pos.x -= (yyval.spec)->segment_width;
		      break;
		    }
		  (yyval.spec)->segment_list = new segment((yyval.spec)->segment_pos,
						 (yyval.spec)->segment_is_absolute,
						 (yyval.spec)->segment_list);
		  (yyval.spec)->flags &= ~HAS_SEGMENT;
		  (yyval.spec)->segment_pos.x = (yyval.spec)->segment_pos.y = 0.0;
		  (yyval.spec)->segment_is_absolute = 0;
		}
#line 3452 "src/preproc/pic/pic.cpp"
    break;

  case 110: /* object_spec: object_spec SOLID  */
#line 960 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);	// nothing
		}
#line 3460 "src/preproc/pic/pic.cpp"
    break;

  case 111: /* object_spec: object_spec DOTTED  */
#line 964 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= IS_DOTTED;
		  lookup_variable("dashwid", & (yyval.spec)->dash_width);
		}
#line 3470 "src/preproc/pic/pic.cpp"
    break;

  case 112: /* object_spec: object_spec DOTTED expr  */
#line 970 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= IS_DOTTED;
		  (yyval.spec)->dash_width = (yyvsp[0].x);
		}
#line 3480 "src/preproc/pic/pic.cpp"
    break;

  case 113: /* object_spec: object_spec DASHED  */
#line 976 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= IS_DASHED;
		  lookup_variable("dashwid", & (yyval.spec)->dash_width);
		}
#line 3490 "src/preproc/pic/pic.cpp"
    break;

  case 114: /* object_spec: object_spec DASHED expr  */
#line 982 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= IS_DASHED;
		  (yyval.spec)->dash_width = (yyvsp[0].x);
		}
#line 3500 "src/preproc/pic/pic.cpp"
    break;

  case 115: /* object_spec: object_spec FILL  */
#line 988 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= IS_DEFAULT_FILLED;
		}
#line 3509 "src/preproc/pic/pic.cpp"
    break;

  case 116: /* object_spec: object_spec FILL expr  */
#line 993 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= IS_FILLED;
		  (yyval.spec)->fill = (yyvsp[0].x);
		}
#line 3519 "src/preproc/pic/pic.cpp"
    break;

  case 117: /* object_spec: object_spec XSLANTED expr  */
#line 999 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= IS_XSLANTED;
		  (yyval.spec)->xslanted = (yyvsp[0].x);
		}
#line 3529 "src/preproc/pic/pic.cpp"
    break;

  case 118: /* object_spec: object_spec YSLANTED expr  */
#line 1005 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= IS_YSLANTED;
		  (yyval.spec)->yslanted = (yyvsp[0].x);
		}
#line 3539 "src/preproc/pic/pic.cpp"
    break;

  case 119: /* object_spec: object_spec SHADED text  */
#line 1011 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= (IS_SHADED | IS_FILLED);
		  (yyval.spec)->shaded = new char[strlen((yyvsp[0].lstr).str)+1];
		  strcpy((yyval.spec)->shaded, (yyvsp[0].lstr).str);
		}
#line 3550 "src/preproc/pic/pic.cpp"
    break;

  case 120: /* object_spec: object_spec COLORED text  */
#line 1018 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= (IS_SHADED | IS_OUTLINED | IS_FILLED);
		  (yyval.spec)->shaded = new char[strlen((yyvsp[0].lstr).str)+1];
		  strcpy((yyval.spec)->shaded, (yyvsp[0].lstr).str);
		  (yyval.spec)->outlined = new char[strlen((yyvsp[0].lstr).str)+1];
		  strcpy((yyval.spec)->outlined, (yyvsp[0].lstr).str);
		}
#line 3563 "src/preproc/pic/pic.cpp"
    break;

  case 121: /* object_spec: object_spec OUTLINED text  */
#line 1027 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= IS_OUTLINED;
		  (yyval.spec)->outlined = new char[strlen((yyvsp[0].lstr).str)+1];
		  strcpy((yyval.spec)->outlined, (yyvsp[0].lstr).str);
		}
#line 3574 "src/preproc/pic/pic.cpp"
    break;

  case 122: /* object_spec: object_spec CHOP  */
#line 1034 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  // line chop chop means line chop 0 chop 0
		  if ((yyval.spec)->flags & IS_DEFAULT_CHOPPED) {
		    (yyval.spec)->flags |= IS_CHOPPED;
		    (yyval.spec)->flags &= ~IS_DEFAULT_CHOPPED;
		    (yyval.spec)->start_chop = (yyval.spec)->end_chop = 0.0;
		  }
		  else if ((yyval.spec)->flags & IS_CHOPPED) {
		    (yyval.spec)->end_chop = 0.0;
		  }
		  else {
		    (yyval.spec)->flags |= IS_DEFAULT_CHOPPED;
		  }
		}
#line 3594 "src/preproc/pic/pic.cpp"
    break;

  case 123: /* object_spec: object_spec CHOP expr  */
#line 1050 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  if ((yyval.spec)->flags & IS_DEFAULT_CHOPPED) {
		    (yyval.spec)->flags |= IS_CHOPPED;
		    (yyval.spec)->flags &= ~IS_DEFAULT_CHOPPED;
		    (yyval.spec)->start_chop = 0.0;
		    (yyval.spec)->end_chop = (yyvsp[0].x);
		  }
		  else if ((yyval.spec)->flags & IS_CHOPPED) {
		    (yyval.spec)->end_chop = (yyvsp[0].x);
		  }
		  else {
		    (yyval.spec)->start_chop = (yyval.spec)->end_chop = (yyvsp[0].x);
		    (yyval.spec)->flags |= IS_CHOPPED;
		  }
		}
#line 3615 "src/preproc/pic/pic.cpp"
    break;

  case 124: /* object_spec: object_spec SAME  */
#line 1067 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= IS_SAME;
		}
#line 3624 "src/preproc/pic/pic.cpp"
    break;

  case 125: /* object_spec: object_spec INVISIBLE  */
#line 1072 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= IS_INVISIBLE;
		}
#line 3633 "src/preproc/pic/pic.cpp"
    break;

  case 126: /* object_spec: object_spec LEFT_ARROW_HEAD  */
#line 1077 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= HAS_LEFT_ARROW_HEAD;
		}
#line 3642 "src/preproc/pic/pic.cpp"
    break;

  case 127: /* object_spec: object_spec RIGHT_ARROW_HEAD  */
#line 1082 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= HAS_RIGHT_ARROW_HEAD;
		}
#line 3651 "src/preproc/pic/pic.cpp"
    break;

  case 128: /* object_spec: object_spec DOUBLE_ARROW_HEAD  */
#line 1087 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= (HAS_LEFT_ARROW_HEAD|HAS_RIGHT_ARROW_HEAD);
		}
#line 3660 "src/preproc/pic/pic.cpp"
    break;

  case 129: /* object_spec: object_spec CW  */
#line 1092 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= IS_CLOCKWISE;
		}
#line 3669 "src/preproc/pic/pic.cpp"
    break;

  case 130: /* object_spec: object_spec CCW  */
#line 1097 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags &= ~IS_CLOCKWISE;
		}
#line 3678 "src/preproc/pic/pic.cpp"
    break;

  case 131: /* object_spec: object_spec text  */
#line 1102 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  text_item **p;
		  for (p = & (yyval.spec)->text; *p; p = &(*p)->next)
		    ;
		  *p = new text_item((yyvsp[0].lstr).str, (yyvsp[0].lstr).filename, (yyvsp[0].lstr).lineno);
		}
#line 3690 "src/preproc/pic/pic.cpp"
    break;

  case 132: /* object_spec: object_spec LJUST  */
#line 1110 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  if ((yyval.spec)->text) {
		    text_item *p;
		    for (p = (yyval.spec)->text; p->next; p = p->next)
		      ;
		    p->adj.h = LEFT_ADJUST;
		  }
		}
#line 3704 "src/preproc/pic/pic.cpp"
    break;

  case 133: /* object_spec: object_spec RJUST  */
#line 1120 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  if ((yyval.spec)->text) {
		    text_item *p;
		    for (p = (yyval.spec)->text; p->next; p = p->next)
		      ;
		    p->adj.h = RIGHT_ADJUST;
		  }
		}
#line 3718 "src/preproc/pic/pic.cpp"
    break;

  case 134: /* object_spec: object_spec ABOVE  */
#line 1130 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  if ((yyval.spec)->text) {
		    text_item *p;
		    for (p = (yyval.spec)->text; p->next; p = p->next)
		      ;
		    p->adj.v = ABOVE_ADJUST;
		  }
		}
#line 3732 "src/preproc/pic/pic.cpp"
    break;

  case 135: /* object_spec: object_spec BELOW  */
#line 1140 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  if ((yyval.spec)->text) {
		    text_item *p;
		    for (p = (yyval.spec)->text; p->next; p = p->next)
		      ;
		    p->adj.v = BELOW_ADJUST;
		  }
		}
#line 3746 "src/preproc/pic/pic.cpp"
    break;

  case 136: /* object_spec: object_spec THICKNESS expr  */
#line 1150 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-2].spec);
		  (yyval.spec)->flags |= HAS_THICKNESS;
		  (yyval.spec)->thickness = (yyvsp[0].x);
		}
#line 3756 "src/preproc/pic/pic.cpp"
    break;

  case 137: /* object_spec: object_spec ALIGNED  */
#line 1156 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.spec) = (yyvsp[-1].spec);
		  (yyval.spec)->flags |= IS_ALIGNED;
		}
#line 3765 "src/preproc/pic/pic.cpp"
    break;

  case 138: /* text: TEXT  */
#line 1164 "../src/preproc/pic/pic.ypp"
                { (yyval.lstr) = (yyvsp[0].lstr); }
#line 3771 "src/preproc/pic/pic.cpp"
    break;

  case 139: /* text: SPRINTF '(' TEXT sprintf_args ')'  */
#line 1166 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.lstr).filename = (yyvsp[-2].lstr).filename;
		  (yyval.lstr).lineno = (yyvsp[-2].lstr).lineno;
		  (yyval.lstr).str = do_sprintf((yyvsp[-2].lstr).str, (yyvsp[-1].dv).v, (yyvsp[-1].dv).nv);
		  delete[] (yyvsp[-1].dv).v;
		  free((yyvsp[-2].lstr).str);
		}
#line 3783 "src/preproc/pic/pic.cpp"
    break;

  case 140: /* sprintf_args: %empty  */
#line 1177 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.dv).v = 0;
		  (yyval.dv).nv = 0;
		  (yyval.dv).maxv = 0;
		}
#line 3793 "src/preproc/pic/pic.cpp"
    break;

  case 141: /* sprintf_args: sprintf_args ',' expr  */
#line 1183 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.dv) = (yyvsp[-2].dv);
		  if ((yyval.dv).nv >= (yyval.dv).maxv) {
		    if ((yyval.dv).nv == 0) {
		      (yyval.dv).v = new double[4];
		      (yyval.dv).maxv = 4;
		    }
		    else {
		      double *oldv = (yyval.dv).v;
		      (yyval.dv).maxv *= 2;
		      (yyval.dv).v = new double[(yyval.dv).maxv];
		      memcpy((yyval.dv).v, oldv, ((yyval.dv).nv * sizeof(double)));
		      delete[] oldv;
		    }
		  }
		  (yyval.dv).v[(yyval.dv).nv] = (yyvsp[0].x);
		  (yyval.dv).nv += 1;
		}
#line 3816 "src/preproc/pic/pic.cpp"
    break;

  case 142: /* position: position_not_place  */
#line 1205 "../src/preproc/pic/pic.ypp"
                { (yyval.pair) = (yyvsp[0].pair); }
#line 3822 "src/preproc/pic/pic.cpp"
    break;

  case 143: /* position: place  */
#line 1207 "../src/preproc/pic/pic.ypp"
                {
		  position pos = (yyvsp[0].pl);
		  (yyval.pair).x = pos.x;
		  (yyval.pair).y = pos.y;
		}
#line 3832 "src/preproc/pic/pic.cpp"
    break;

  case 144: /* position: '(' place ')'  */
#line 1213 "../src/preproc/pic/pic.ypp"
                {
		  position pos = (yyvsp[-1].pl);
		  (yyval.pair).x = pos.x;
		  (yyval.pair).y = pos.y;
		}
#line 3842 "src/preproc/pic/pic.cpp"
    break;

  case 145: /* position_not_place: expr_pair  */
#line 1222 "../src/preproc/pic/pic.ypp"
                { (yyval.pair) = (yyvsp[0].pair); }
#line 3848 "src/preproc/pic/pic.cpp"
    break;

  case 146: /* position_not_place: position '+' expr_pair  */
#line 1224 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (yyvsp[-2].pair).x + (yyvsp[0].pair).x;
		  (yyval.pair).y = (yyvsp[-2].pair).y + (yyvsp[0].pair).y;
		}
#line 3857 "src/preproc/pic/pic.cpp"
    break;

  case 147: /* position_not_place: '(' position '+' expr_pair ')'  */
#line 1229 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (yyvsp[-3].pair).x + (yyvsp[-1].pair).x;
		  (yyval.pair).y = (yyvsp[-3].pair).y + (yyvsp[-1].pair).y;
		}
#line 3866 "src/preproc/pic/pic.cpp"
    break;

  case 148: /* position_not_place: position '-' expr_pair  */
#line 1234 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (yyvsp[-2].pair).x - (yyvsp[0].pair).x;
		  (yyval.pair).y = (yyvsp[-2].pair).y - (yyvsp[0].pair).y;
		}
#line 3875 "src/preproc/pic/pic.cpp"
    break;

  case 149: /* position_not_place: '(' position '-' expr_pair ')'  */
#line 1239 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (yyvsp[-3].pair).x - (yyvsp[-1].pair).x;
		  (yyval.pair).y = (yyvsp[-3].pair).y - (yyvsp[-1].pair).y;
		}
#line 3884 "src/preproc/pic/pic.cpp"
    break;

  case 150: /* position_not_place: '(' position ',' position ')'  */
#line 1244 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (yyvsp[-3].pair).x;
		  (yyval.pair).y = (yyvsp[-1].pair).y;
		}
#line 3893 "src/preproc/pic/pic.cpp"
    break;

  case 151: /* position_not_place: expr between position AND position  */
#line 1249 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (1.0 - (yyvsp[-4].x))*(yyvsp[-2].pair).x + (yyvsp[-4].x)*(yyvsp[0].pair).x;
		  (yyval.pair).y = (1.0 - (yyvsp[-4].x))*(yyvsp[-2].pair).y + (yyvsp[-4].x)*(yyvsp[0].pair).y;
		}
#line 3902 "src/preproc/pic/pic.cpp"
    break;

  case 152: /* position_not_place: '(' expr between position AND position ')'  */
#line 1254 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (1.0 - (yyvsp[-5].x))*(yyvsp[-3].pair).x + (yyvsp[-5].x)*(yyvsp[-1].pair).x;
		  (yyval.pair).y = (1.0 - (yyvsp[-5].x))*(yyvsp[-3].pair).y + (yyvsp[-5].x)*(yyvsp[-1].pair).y;
		}
#line 3911 "src/preproc/pic/pic.cpp"
    break;

  case 153: /* position_not_place: expr_not_lower_than '<' position ',' position '>'  */
#line 1260 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (1.0 - (yyvsp[-5].x))*(yyvsp[-3].pair).x + (yyvsp[-5].x)*(yyvsp[-1].pair).x;
		  (yyval.pair).y = (1.0 - (yyvsp[-5].x))*(yyvsp[-3].pair).y + (yyvsp[-5].x)*(yyvsp[-1].pair).y;
		}
#line 3920 "src/preproc/pic/pic.cpp"
    break;

  case 154: /* position_not_place: '(' expr_not_lower_than '<' position ',' position '>' ')'  */
#line 1265 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (1.0 - (yyvsp[-6].x))*(yyvsp[-4].pair).x + (yyvsp[-6].x)*(yyvsp[-2].pair).x;
		  (yyval.pair).y = (1.0 - (yyvsp[-6].x))*(yyvsp[-4].pair).y + (yyvsp[-6].x)*(yyvsp[-2].pair).y;
		}
#line 3929 "src/preproc/pic/pic.cpp"
    break;

  case 157: /* expr_pair: expr ',' expr  */
#line 1278 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pair).x = (yyvsp[-2].x);
		  (yyval.pair).y = (yyvsp[0].x);
		}
#line 3938 "src/preproc/pic/pic.cpp"
    break;

  case 158: /* expr_pair: '(' expr_pair ')'  */
#line 1283 "../src/preproc/pic/pic.ypp"
                { (yyval.pair) = (yyvsp[-1].pair); }
#line 3944 "src/preproc/pic/pic.cpp"
    break;

  case 159: /* place: label  */
#line 1289 "../src/preproc/pic/pic.ypp"
                { (yyval.pl) = (yyvsp[0].pl); }
#line 3950 "src/preproc/pic/pic.cpp"
    break;

  case 160: /* place: label corner  */
#line 1291 "../src/preproc/pic/pic.ypp"
                {
		  path pth((yyvsp[0].crn));
		  if (!pth.follow((yyvsp[-1].pl), & (yyval.pl)))
		    YYABORT;
		}
#line 3960 "src/preproc/pic/pic.cpp"
    break;

  case 161: /* place: label vertex  */
#line 1297 "../src/preproc/pic/pic.ypp"
                {
		  (yyvsp[-1].pl).obj->set_vertex_number((yyvsp[0].ver).vertex_number);
		  path pth((yyvsp[0].ver).crn);
		  if (!pth.follow((yyvsp[-1].pl), & (yyval.pl)))
		    YYABORT;
		}
#line 3971 "src/preproc/pic/pic.cpp"
    break;

  case 162: /* place: corner label  */
#line 1304 "../src/preproc/pic/pic.ypp"
                {
		  path pth((yyvsp[-1].crn));
		  if (!pth.follow((yyvsp[0].pl), & (yyval.pl)))
		    YYABORT;
		}
#line 3981 "src/preproc/pic/pic.cpp"
    break;

  case 163: /* place: corner OF label  */
#line 1310 "../src/preproc/pic/pic.ypp"
                {
		  path pth((yyvsp[-2].crn));
		  if (!pth.follow((yyvsp[0].pl), & (yyval.pl)))
		    YYABORT;
		}
#line 3991 "src/preproc/pic/pic.cpp"
    break;

  case 164: /* place: vertex OF label  */
#line 1316 "../src/preproc/pic/pic.ypp"
                {
		  (yyvsp[0].pl).obj->set_vertex_number((yyvsp[-2].ver).vertex_number);
		  path pth((yyvsp[-2].ver).crn);
		  if (!pth.follow((yyvsp[0].pl), & (yyval.pl)))
		    YYABORT;
		}
#line 4002 "src/preproc/pic/pic.cpp"
    break;

  case 165: /* place: HERE  */
#line 1323 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pl).x = current_position.x;
		  (yyval.pl).y = current_position.y;
		  (yyval.pl).obj = 0;
		}
#line 4012 "src/preproc/pic/pic.cpp"
    break;

  case 166: /* label: LABEL  */
#line 1332 "../src/preproc/pic/pic.ypp"
                {
		  place *p = lookup_label((yyvsp[0].str));
		  if (!p) {
		    lex_error("there is no place '%1'", (yyvsp[0].str));
		    YYABORT;
		  }
		  (yyval.pl) = *p;
		  free((yyvsp[0].str));
		}
#line 4026 "src/preproc/pic/pic.cpp"
    break;

  case 167: /* label: nth_primitive  */
#line 1342 "../src/preproc/pic/pic.ypp"
                { (yyval.pl).obj = (yyvsp[0].obj); }
#line 4032 "src/preproc/pic/pic.cpp"
    break;

  case 168: /* label: label '.' LABEL  */
#line 1344 "../src/preproc/pic/pic.ypp"
                {
		  path pth((yyvsp[0].str));
		  if (!pth.follow((yyvsp[-2].pl), & (yyval.pl)))
		    YYABORT;
		}
#line 4042 "src/preproc/pic/pic.cpp"
    break;

  case 169: /* ordinal: ORDINAL  */
#line 1353 "../src/preproc/pic/pic.ypp"
                { (yyval.n) = (yyvsp[0].n); }
#line 4048 "src/preproc/pic/pic.cpp"
    break;

  case 170: /* ordinal: '`' any_expr TH  */
#line 1355 "../src/preproc/pic/pic.ypp"
                {
		  // XXX Check for overflow (and non-integers?).
		  (yyval.n) = int((yyvsp[-1].x));
		}
#line 4057 "src/preproc/pic/pic.cpp"
    break;

  case 171: /* optional_ordinal_last: LAST  */
#line 1363 "../src/preproc/pic/pic.ypp"
                { (yyval.n) = 1; }
#line 4063 "src/preproc/pic/pic.cpp"
    break;

  case 172: /* optional_ordinal_last: ordinal LAST  */
#line 1365 "../src/preproc/pic/pic.ypp"
                { (yyval.n) = (yyvsp[-1].n); }
#line 4069 "src/preproc/pic/pic.cpp"
    break;

  case 173: /* nth_primitive: ordinal object_type  */
#line 1370 "../src/preproc/pic/pic.ypp"
                {
		  int count = 0;
		  object *p;
		  for (p = olist.head; p != 0; p = p->next)
		    if (p->type() == (yyvsp[0].obtype) && ++count == (yyvsp[-1].n)) {
		      (yyval.obj) = p;
		      break;
		    }
		  if (p == 0) {
		    lex_error("there is no %1%2 %3", (yyvsp[-1].n), ordinal_postfix((yyvsp[-1].n)),
			      object_type_name((yyvsp[0].obtype)));
		    YYABORT;
		  }
		}
#line 4088 "src/preproc/pic/pic.cpp"
    break;

  case 174: /* nth_primitive: optional_ordinal_last object_type  */
#line 1385 "../src/preproc/pic/pic.ypp"
                {
		  int count = 0;
		  object *p;
		  for (p = olist.tail; p != 0; p = p->prev)
		    if (p->type() == (yyvsp[0].obtype) && ++count == (yyvsp[-1].n)) {
		      (yyval.obj) = p;
		      break;
		    }
		  if (p == 0) {
		    lex_error("there is no %1%2 last %3", (yyvsp[-1].n),
			      ordinal_postfix((yyvsp[-1].n)), object_type_name((yyvsp[0].obtype)));
		    YYABORT;
		  }
		}
#line 4107 "src/preproc/pic/pic.cpp"
    break;

  case 175: /* object_type: BOX  */
#line 1403 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = BOX_OBJECT; }
#line 4113 "src/preproc/pic/pic.cpp"
    break;

  case 176: /* object_type: CIRCLE  */
#line 1405 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = CIRCLE_OBJECT; }
#line 4119 "src/preproc/pic/pic.cpp"
    break;

  case 177: /* object_type: ELLIPSE  */
#line 1407 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = ELLIPSE_OBJECT; }
#line 4125 "src/preproc/pic/pic.cpp"
    break;

  case 178: /* object_type: ARC  */
#line 1409 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = ARC_OBJECT; }
#line 4131 "src/preproc/pic/pic.cpp"
    break;

  case 179: /* object_type: LINE  */
#line 1411 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = LINE_OBJECT; }
#line 4137 "src/preproc/pic/pic.cpp"
    break;

  case 180: /* object_type: POLYGON  */
#line 1413 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = POLYGON_OBJECT; }
#line 4143 "src/preproc/pic/pic.cpp"
    break;

  case 181: /* object_type: ARROW  */
#line 1415 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = ARROW_OBJECT; }
#line 4149 "src/preproc/pic/pic.cpp"
    break;

  case 182: /* object_type: SPLINE  */
#line 1417 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = SPLINE_OBJECT; }
#line 4155 "src/preproc/pic/pic.cpp"
    break;

  case 183: /* object_type: '[' ']'  */
#line 1419 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = BLOCK_OBJECT; }
#line 4161 "src/preproc/pic/pic.cpp"
    break;

  case 184: /* object_type: TEXT  */
#line 1421 "../src/preproc/pic/pic.ypp"
                { (yyval.obtype) = TEXT_OBJECT; }
#line 4167 "src/preproc/pic/pic.cpp"
    break;

  case 185: /* label_path: '.' LABEL  */
#line 1426 "../src/preproc/pic/pic.ypp"
                { (yyval.pth) = new path((yyvsp[0].str)); }
#line 4173 "src/preproc/pic/pic.cpp"
    break;

  case 186: /* label_path: label_path '.' LABEL  */
#line 1428 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pth) = (yyvsp[-2].pth);
		  (yyval.pth)->append((yyvsp[0].str));
		}
#line 4182 "src/preproc/pic/pic.cpp"
    break;

  case 187: /* relative_path: corner  */
#line 1436 "../src/preproc/pic/pic.ypp"
                { (yyval.pth) = new path((yyvsp[0].crn)); }
#line 4188 "src/preproc/pic/pic.cpp"
    break;

  case 188: /* relative_path: label_path  */
#line 1440 "../src/preproc/pic/pic.ypp"
                { (yyval.pth) = (yyvsp[0].pth); }
#line 4194 "src/preproc/pic/pic.cpp"
    break;

  case 189: /* relative_path: label_path corner  */
#line 1442 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pth) = (yyvsp[-1].pth);
		  (yyval.pth)->append((yyvsp[0].crn));
		}
#line 4203 "src/preproc/pic/pic.cpp"
    break;

  case 190: /* path: relative_path  */
#line 1450 "../src/preproc/pic/pic.ypp"
                { (yyval.pth) = (yyvsp[0].pth); }
#line 4209 "src/preproc/pic/pic.cpp"
    break;

  case 191: /* path: '(' relative_path ',' relative_path ')'  */
#line 1452 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.pth) = (yyvsp[-3].pth);
		  (yyval.pth)->set_ypath((yyvsp[-1].pth));
		}
#line 4218 "src/preproc/pic/pic.cpp"
    break;

  case 192: /* path: ORDINAL LAST object_type relative_path  */
#line 1458 "../src/preproc/pic/pic.ypp"
                {
		  lex_warning("'%1%2 last %3' in 'with' argument ignored",
			      (yyvsp[-3].n), ordinal_postfix((yyvsp[-3].n)), object_type_name((yyvsp[-1].obtype)));
		  (yyval.pth) = (yyvsp[0].pth);
		}
#line 4228 "src/preproc/pic/pic.cpp"
    break;

  case 193: /* path: LAST object_type relative_path  */
#line 1464 "../src/preproc/pic/pic.ypp"
                {
		  lex_warning("'last %1' in 'with' argument ignored",
			      object_type_name((yyvsp[-1].obtype)));
		  (yyval.pth) = (yyvsp[0].pth);
		}
#line 4238 "src/preproc/pic/pic.cpp"
    break;

  case 194: /* path: ORDINAL object_type relative_path  */
#line 1470 "../src/preproc/pic/pic.ypp"
                {
		  lex_warning("'%1%2 %3' in 'with' argument ignored",
			      (yyvsp[-2].n), ordinal_postfix((yyvsp[-2].n)), object_type_name((yyvsp[-1].obtype)));
		  (yyval.pth) = (yyvsp[0].pth);
		}
#line 4248 "src/preproc/pic/pic.cpp"
    break;

  case 195: /* path: LABEL relative_path  */
#line 1476 "../src/preproc/pic/pic.ypp"
                {
		  lex_warning("initial '%1' in 'with' argument ignored", (yyvsp[-1].str));
		  delete[] (yyvsp[-1].str);
		  (yyval.pth) = (yyvsp[0].pth);
		}
#line 4258 "src/preproc/pic/pic.cpp"
    break;

  case 196: /* corner: DOT_N  */
#line 1485 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north; }
#line 4264 "src/preproc/pic/pic.cpp"
    break;

  case 197: /* corner: DOT_E  */
#line 1487 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::east; }
#line 4270 "src/preproc/pic/pic.cpp"
    break;

  case 198: /* corner: DOT_W  */
#line 1489 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::west; }
#line 4276 "src/preproc/pic/pic.cpp"
    break;

  case 199: /* corner: DOT_S  */
#line 1491 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south; }
#line 4282 "src/preproc/pic/pic.cpp"
    break;

  case 200: /* corner: DOT_NE  */
#line 1493 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north_east; }
#line 4288 "src/preproc/pic/pic.cpp"
    break;

  case 201: /* corner: DOT_SE  */
#line 1495 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south_east; }
#line 4294 "src/preproc/pic/pic.cpp"
    break;

  case 202: /* corner: DOT_NW  */
#line 1497 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north_west; }
#line 4300 "src/preproc/pic/pic.cpp"
    break;

  case 203: /* corner: DOT_SW  */
#line 1499 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south_west; }
#line 4306 "src/preproc/pic/pic.cpp"
    break;

  case 204: /* corner: DOT_C  */
#line 1501 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::center; }
#line 4312 "src/preproc/pic/pic.cpp"
    break;

  case 205: /* corner: DOT_START  */
#line 1503 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::start; }
#line 4318 "src/preproc/pic/pic.cpp"
    break;

  case 206: /* corner: DOT_END  */
#line 1505 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::end; }
#line 4324 "src/preproc/pic/pic.cpp"
    break;

  case 207: /* corner: TOP  */
#line 1507 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north; }
#line 4330 "src/preproc/pic/pic.cpp"
    break;

  case 208: /* corner: BOTTOM  */
#line 1509 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south; }
#line 4336 "src/preproc/pic/pic.cpp"
    break;

  case 209: /* corner: LEFT  */
#line 1511 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::west; }
#line 4342 "src/preproc/pic/pic.cpp"
    break;

  case 210: /* corner: RIGHT  */
#line 1513 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::east; }
#line 4348 "src/preproc/pic/pic.cpp"
    break;

  case 211: /* corner: UPPER LEFT  */
#line 1515 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north_west; }
#line 4354 "src/preproc/pic/pic.cpp"
    break;

  case 212: /* corner: LOWER LEFT  */
#line 1517 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south_west; }
#line 4360 "src/preproc/pic/pic.cpp"
    break;

  case 213: /* corner: UPPER RIGHT  */
#line 1519 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north_east; }
#line 4366 "src/preproc/pic/pic.cpp"
    break;

  case 214: /* corner: LOWER RIGHT  */
#line 1521 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south_east; }
#line 4372 "src/preproc/pic/pic.cpp"
    break;

  case 215: /* corner: LEFT_CORNER  */
#line 1523 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::west; }
#line 4378 "src/preproc/pic/pic.cpp"
    break;

  case 216: /* corner: RIGHT_CORNER  */
#line 1525 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::east; }
#line 4384 "src/preproc/pic/pic.cpp"
    break;

  case 217: /* corner: UPPER LEFT_CORNER  */
#line 1527 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north_west; }
#line 4390 "src/preproc/pic/pic.cpp"
    break;

  case 218: /* corner: LOWER LEFT_CORNER  */
#line 1529 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south_west; }
#line 4396 "src/preproc/pic/pic.cpp"
    break;

  case 219: /* corner: UPPER RIGHT_CORNER  */
#line 1531 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north_east; }
#line 4402 "src/preproc/pic/pic.cpp"
    break;

  case 220: /* corner: LOWER RIGHT_CORNER  */
#line 1533 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south_east; }
#line 4408 "src/preproc/pic/pic.cpp"
    break;

  case 221: /* corner: NORTH  */
#line 1535 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::north; }
#line 4414 "src/preproc/pic/pic.cpp"
    break;

  case 222: /* corner: SOUTH  */
#line 1537 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::south; }
#line 4420 "src/preproc/pic/pic.cpp"
    break;

  case 223: /* corner: EAST  */
#line 1539 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::east; }
#line 4426 "src/preproc/pic/pic.cpp"
    break;

  case 224: /* corner: WEST  */
#line 1541 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::west; }
#line 4432 "src/preproc/pic/pic.cpp"
    break;

  case 225: /* corner: CENTER  */
#line 1543 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::center; }
#line 4438 "src/preproc/pic/pic.cpp"
    break;

  case 226: /* corner: START  */
#line 1545 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::start; }
#line 4444 "src/preproc/pic/pic.cpp"
    break;

  case 227: /* corner: END  */
#line 1547 "../src/preproc/pic/pic.ypp"
                { (yyval.crn) = &object::end; }
#line 4450 "src/preproc/pic/pic.cpp"
    break;

  case 228: /* vertex: DOT_V expr  */
#line 1552 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.ver).crn = &object::vertex;
		  (yyval.ver).vertex_number = (yyvsp[0].x);
		}
#line 4459 "src/preproc/pic/pic.cpp"
    break;

  case 229: /* vertex: DOT_V '`' expr '\''  */
#line 1557 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.ver).crn = &object::vertex;
		  (yyval.ver).vertex_number = (yyvsp[-1].x);
		}
#line 4468 "src/preproc/pic/pic.cpp"
    break;

  case 230: /* vertex: DOT_MID expr  */
#line 1562 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.ver).crn = &object::point;
		  (yyval.ver).vertex_number = (yyvsp[0].x);
		  (yyval.ver).is_edge = 1;
		}
#line 4478 "src/preproc/pic/pic.cpp"
    break;

  case 231: /* vertex: DOT_MID '`' expr '\''  */
#line 1568 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.ver).crn = &object::point;
		  (yyval.ver).vertex_number = (yyvsp[-1].x);
		  (yyval.ver).is_edge = 1;
		}
#line 4488 "src/preproc/pic/pic.cpp"
    break;

  case 232: /* expr: expr_lower_than  */
#line 1577 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[0].x); }
#line 4494 "src/preproc/pic/pic.cpp"
    break;

  case 233: /* expr: expr_not_lower_than  */
#line 1579 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[0].x); }
#line 4500 "src/preproc/pic/pic.cpp"
    break;

  case 234: /* expr_lower_than: expr '<' expr  */
#line 1584 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) < (yyvsp[0].x)); }
#line 4506 "src/preproc/pic/pic.cpp"
    break;

  case 235: /* expr_not_lower_than: VARIABLE  */
#line 1589 "../src/preproc/pic/pic.ypp"
                {
		  if (!lookup_variable((yyvsp[0].str), & (yyval.x))) {
		    lex_error("there is no variable '%1'", (yyvsp[0].str));
		    YYABORT;
		  }
		  free((yyvsp[0].str));
		}
#line 4518 "src/preproc/pic/pic.cpp"
    break;

  case 236: /* expr_not_lower_than: NUMBER  */
#line 1597 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[0].x); }
#line 4524 "src/preproc/pic/pic.cpp"
    break;

  case 237: /* expr_not_lower_than: place DOT_X  */
#line 1599 "../src/preproc/pic/pic.ypp"
                {
		  if ((yyvsp[-1].pl).obj != 0)
		    (yyval.x) = (yyvsp[-1].pl).obj->origin().x;
		  else
		    (yyval.x) = (yyvsp[-1].pl).x;
		}
#line 4535 "src/preproc/pic/pic.cpp"
    break;

  case 238: /* expr_not_lower_than: place DOT_Y  */
#line 1606 "../src/preproc/pic/pic.ypp"
                {
		  if ((yyvsp[-1].pl).obj != 0)
		    (yyval.x) = (yyvsp[-1].pl).obj->origin().y;
		  else
		    (yyval.x) = (yyvsp[-1].pl).y;
		}
#line 4546 "src/preproc/pic/pic.cpp"
    break;

  case 239: /* expr_not_lower_than: place DOT_HT  */
#line 1613 "../src/preproc/pic/pic.ypp"
                {
		  if ((yyvsp[-1].pl).obj != 0)
		    (yyval.x) = (yyvsp[-1].pl).obj->height();
		  else
		    (yyval.x) = 0.0;
		}
#line 4557 "src/preproc/pic/pic.cpp"
    break;

  case 240: /* expr_not_lower_than: place DOT_WID  */
#line 1620 "../src/preproc/pic/pic.ypp"
                {
		  if ((yyvsp[-1].pl).obj != 0)
		    (yyval.x) = (yyvsp[-1].pl).obj->width();
		  else
		    (yyval.x) = 0.0;
		}
#line 4568 "src/preproc/pic/pic.cpp"
    break;

  case 241: /* expr_not_lower_than: place DOT_RAD  */
#line 1627 "../src/preproc/pic/pic.ypp"
                {
		  if ((yyvsp[-1].pl).obj != 0)
		    (yyval.x) = (yyvsp[-1].pl).obj->radius();
		  else
		    (yyval.x) = 0.0;
		}
#line 4579 "src/preproc/pic/pic.cpp"
    break;

  case 242: /* expr_not_lower_than: expr '+' expr  */
#line 1634 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[-2].x) + (yyvsp[0].x); }
#line 4585 "src/preproc/pic/pic.cpp"
    break;

  case 243: /* expr_not_lower_than: expr '-' expr  */
#line 1636 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[-2].x) - (yyvsp[0].x); }
#line 4591 "src/preproc/pic/pic.cpp"
    break;

  case 244: /* expr_not_lower_than: expr '*' expr  */
#line 1638 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[-2].x) * (yyvsp[0].x); }
#line 4597 "src/preproc/pic/pic.cpp"
    break;

  case 245: /* expr_not_lower_than: expr '/' expr  */
#line 1640 "../src/preproc/pic/pic.ypp"
                {
		  if ((yyvsp[0].x) == 0.0) {
		    lex_error("division by zero");
		    YYABORT;
		  }
		  (yyval.x) = (yyvsp[-2].x)/(yyvsp[0].x);
		}
#line 4609 "src/preproc/pic/pic.cpp"
    break;

  case 246: /* expr_not_lower_than: expr '%' expr  */
#line 1648 "../src/preproc/pic/pic.ypp"
                {
		  if ((yyvsp[0].x) == 0.0) {
		    lex_error("modulus by zero");
		    YYABORT;
		  }
		  (yyval.x) = fmod((yyvsp[-2].x), (yyvsp[0].x));
		}
#line 4621 "src/preproc/pic/pic.cpp"
    break;

  case 247: /* expr_not_lower_than: expr '^' expr  */
#line 1656 "../src/preproc/pic/pic.ypp"
                {
		  errno = 0;
		  (yyval.x) = pow((yyvsp[-2].x), (yyvsp[0].x));
		  if (errno == EDOM) {
		    lex_error("arguments to '^' operator out of domain");
		    YYABORT;
		  }
		  if (errno == ERANGE) {
		    lex_error("result of '^' operator out of range");
		    YYABORT;
		  }
		}
#line 4638 "src/preproc/pic/pic.cpp"
    break;

  case 248: /* expr_not_lower_than: '-' expr  */
#line 1669 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = -(yyvsp[0].x); }
#line 4644 "src/preproc/pic/pic.cpp"
    break;

  case 249: /* expr_not_lower_than: '(' any_expr ')'  */
#line 1671 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[-1].x); }
#line 4650 "src/preproc/pic/pic.cpp"
    break;

  case 250: /* expr_not_lower_than: SIN '(' any_expr ')'  */
#line 1673 "../src/preproc/pic/pic.ypp"
                {
		  errno = 0;
		  (yyval.x) = sin((yyvsp[-1].x));
		  if (errno == ERANGE) {
		    lex_error("sin result out of range");
		    YYABORT;
		  }
		}
#line 4663 "src/preproc/pic/pic.cpp"
    break;

  case 251: /* expr_not_lower_than: COS '(' any_expr ')'  */
#line 1682 "../src/preproc/pic/pic.ypp"
                {
		  errno = 0;
		  (yyval.x) = cos((yyvsp[-1].x));
		  if (errno == ERANGE) {
		    lex_error("cos result out of range");
		    YYABORT;
		  }
		}
#line 4676 "src/preproc/pic/pic.cpp"
    break;

  case 252: /* expr_not_lower_than: ATAN2 '(' any_expr ',' any_expr ')'  */
#line 1691 "../src/preproc/pic/pic.ypp"
                {
		  errno = 0;
		  (yyval.x) = atan2((yyvsp[-3].x), (yyvsp[-1].x));
		  if (errno == EDOM) {
		    lex_error("atan2 argument out of domain");
		    YYABORT;
		  }
		  if (errno == ERANGE) {
		    lex_error("atan2 result out of range");
		    YYABORT;
		  }
		}
#line 4693 "src/preproc/pic/pic.cpp"
    break;

  case 253: /* expr_not_lower_than: LOG '(' any_expr ')'  */
#line 1704 "../src/preproc/pic/pic.ypp"
                {
		  errno = 0;
		  (yyval.x) = log10((yyvsp[-1].x));
		  if (errno == ERANGE) {
		    lex_error("log result out of range");
		    YYABORT;
		  }
		}
#line 4706 "src/preproc/pic/pic.cpp"
    break;

  case 254: /* expr_not_lower_than: EXP '(' any_expr ')'  */
#line 1713 "../src/preproc/pic/pic.ypp"
                {
		  errno = 0;
		  (yyval.x) = pow(10.0, (yyvsp[-1].x));
		  if (errno == ERANGE) {
		    lex_error("exp result out of range");
		    YYABORT;
		  }
		}
#line 4719 "src/preproc/pic/pic.cpp"
    break;

  case 255: /* expr_not_lower_than: SQRT '(' any_expr ')'  */
#line 1722 "../src/preproc/pic/pic.ypp"
                {
		  errno = 0;
		  (yyval.x) = sqrt((yyvsp[-1].x));
		  if (errno == EDOM) {
		    lex_error("sqrt argument out of domain");
		    YYABORT;
		  }
		}
#line 4732 "src/preproc/pic/pic.cpp"
    break;

  case 256: /* expr_not_lower_than: K_MAX '(' any_expr ',' any_expr ')'  */
#line 1731 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[-3].x) > (yyvsp[-1].x) ? (yyvsp[-3].x) : (yyvsp[-1].x); }
#line 4738 "src/preproc/pic/pic.cpp"
    break;

  case 257: /* expr_not_lower_than: K_MIN '(' any_expr ',' any_expr ')'  */
#line 1733 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[-3].x) < (yyvsp[-1].x) ? (yyvsp[-3].x) : (yyvsp[-1].x); }
#line 4744 "src/preproc/pic/pic.cpp"
    break;

  case 258: /* expr_not_lower_than: INT '(' any_expr ')'  */
#line 1735 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = (yyvsp[-1].x) < 0 ? -floor(-(yyvsp[-1].x)) : floor((yyvsp[-1].x)); }
#line 4750 "src/preproc/pic/pic.cpp"
    break;

  case 259: /* expr_not_lower_than: RAND '(' any_expr ')'  */
#line 1737 "../src/preproc/pic/pic.ypp"
                {
		  lex_error("use of 'rand' with an argument is"
			    " deprecated; shift and scale 'rand()' with"
			    " arithmetic instead");
		  (yyval.x) = 1.0 + floor(((rand()&0x7fff)/double(0x7fff))*(yyvsp[-1].x));
		}
#line 4761 "src/preproc/pic/pic.cpp"
    break;

  case 260: /* expr_not_lower_than: RAND '(' ')'  */
#line 1744 "../src/preproc/pic/pic.ypp"
                {
		  /* return a random number in the range [0,1) */
		  /* portable, but not very random */
		  (yyval.x) = (rand() & 0x7fff) / double(0x8000);
		}
#line 4771 "src/preproc/pic/pic.cpp"
    break;

  case 261: /* expr_not_lower_than: SRAND '(' any_expr ')'  */
#line 1750 "../src/preproc/pic/pic.ypp"
                {
		  (yyval.x) = 0;
		  srand((unsigned int)(yyvsp[-1].x));
		}
#line 4780 "src/preproc/pic/pic.cpp"
    break;

  case 262: /* expr_not_lower_than: expr LESSEQUAL expr  */
#line 1755 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) <= (yyvsp[0].x)); }
#line 4786 "src/preproc/pic/pic.cpp"
    break;

  case 263: /* expr_not_lower_than: expr '>' expr  */
#line 1757 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) > (yyvsp[0].x)); }
#line 4792 "src/preproc/pic/pic.cpp"
    break;

  case 264: /* expr_not_lower_than: expr GREATEREQUAL expr  */
#line 1759 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) >= (yyvsp[0].x)); }
#line 4798 "src/preproc/pic/pic.cpp"
    break;

  case 265: /* expr_not_lower_than: expr EQUALEQUAL expr  */
#line 1761 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) == (yyvsp[0].x)); }
#line 4804 "src/preproc/pic/pic.cpp"
    break;

  case 266: /* expr_not_lower_than: expr NOTEQUAL expr  */
#line 1763 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != (yyvsp[0].x)); }
#line 4810 "src/preproc/pic/pic.cpp"
    break;

  case 267: /* expr_not_lower_than: expr ANDAND expr  */
#line 1765 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 && (yyvsp[0].x) != 0.0); }
#line 4816 "src/preproc/pic/pic.cpp"
    break;

  case 268: /* expr_not_lower_than: expr OROR expr  */
#line 1767 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[-2].x) != 0.0 || (yyvsp[0].x) != 0.0); }
#line 4822 "src/preproc/pic/pic.cpp"
    break;

  case 269: /* expr_not_lower_than: '!' expr  */
#line 1769 "../src/preproc/pic/pic.ypp"
                { (yyval.x) = ((yyvsp[0].x) == 0.0); }
#line 4828 "src/preproc/pic/pic.cpp"
    break;


#line 4832 "src/preproc/pic/pic.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1773 "../src/preproc/pic/pic.ypp"


/* bison defines const to be empty unless __STDC__ is defined, which it
isn't under cfront */

#ifdef const
#undef const
#endif

static struct pic_defaults_table {
  const char *name;
  double val;
  int scaled;		// non-zero if val should be multiplied by scale
} defaults_table[] = {
  { "arcrad", .25, 1 },
  { "arrowht", .1, 1 },
  { "arrowwid", .05, 1 },
  { "circlerad", .25, 1 },
  { "boxht", .5, 1 },
  { "boxwid", .75, 1 },
  { "boxrad", 0.0, 1 },
  { "dashwid", .05, 1 },
  { "ellipseht", .5, 1 },
  { "ellipsewid", .75, 1 },
  { "moveht", .5, 1 },
  { "movewid", .5, 1 },
  { "lineht", .5, 1 },
  { "linewid", .5, 1 },
  { "textht", 0.0, 1 },
  { "textwid", 0.0, 1 },
  { "scale", 1.0, 0 },
  { "linethick", -1.0, 0 },		// in points
  { "fillval", .5, 0 },
  { "arrowhead", 1.0, 0 },
  { "maxpswid", 8.5, 0 },
  { "maxpsht", 11.0, 0 },
};

place *lookup_label(const char *label)
{
  saved_state *state = current_saved_state;
  PTABLE(place) *tbl = current_table;
  for (;;) {
    place *pl = tbl->lookup(label);
    if (pl)
      return pl;
    if (!state)
      return 0 /* nullptr */;
    tbl = state->tbl;
    state = state->prev;
  }
}

void define_label(const char *label, const place *pl)
{
  place *p = new place[1];
  *p = *pl;
  current_table->define(label, p);
}

int lookup_variable(const char *name, double *val)
{
  place *pl = lookup_label(name);
  if (pl) {
    *val = pl->x;
    return 1;
  }
  return 0;
}

void define_variable(const char *name, double val)
{
  place *p = new place[1];
  p->obj = 0;
  p->x = val;
  p->y = 0.0;
  current_table->define(name, p);
  if (strcmp(name, "scale") == 0) {
    // When the scale changes, reset all scaled predefined variables to
    // their default values.
    for (size_t i = 0; i < countof(defaults_table); i++)
      if (defaults_table[i].scaled)
	define_variable(defaults_table[i].name,
			(val * defaults_table[i].val));
  }
}

// called once only (not once per parse)

void parse_init()
{
  current_direction = RIGHT_DIRECTION;
  current_position.x = 0.0;
  current_position.y = 0.0;
  // This resets everything to its default value.
  reset_all();
}

void reset(const char *nm)
{
  for (size_t i = 0; i < countof(defaults_table); i++)
    if (strcmp(nm, defaults_table[i].name) == 0) {
      double val = defaults_table[i].val;
      if (defaults_table[i].scaled) {
	double scale;
	lookup_variable("scale", &scale);
	val *= scale;
      }
      define_variable(defaults_table[i].name, val);
      return;
    }
  lex_error("'%1' is not a predefined variable", nm);
}

void reset_all()
{
  // We only have to explicitly reset the predefined variables that
  // aren't scaled because 'scale' is not scaled, and changing the
  // value of 'scale' will reset all the predefined variables that
  // are scaled.
  for (size_t i = 0; i < countof(defaults_table); i++)
    if (!defaults_table[i].scaled)
      define_variable(defaults_table[i].name, defaults_table[i].val);
}

// called after each parse

void parse_cleanup()
{
  while (current_saved_state != 0) {
    delete current_table;
    current_table = current_saved_state->tbl;
    saved_state *tem = current_saved_state;
    current_saved_state = current_saved_state->prev;
    delete tem;
  }
  assert(current_table == &top_table);
  PTABLE_ITERATOR(place) iter(current_table);
  const char *key;
  place *pl;
  while (iter.next(&key, &pl))
    if (pl->obj != 0) {
      position pos = pl->obj->origin();
      pl->obj = 0;
      pl->x = pos.x;
      pl->y = pos.y;
    }
  while (olist.head != 0) {
    object *tem = olist.head;
    olist.head = olist.head->next;
    delete tem;
  }
  olist.tail = 0;
  current_direction = RIGHT_DIRECTION;
  current_position.x = 0.0;
  current_position.y = 0.0;
}

const char *ordinal_postfix(int n)
{
  if (n < 10 || n > 20)
    switch (n % 10) {
    case 1:
      return "st";
    case 2:
      return "nd";
    case 3:
      return "rd";
    }
  return "th";
}

const char *object_type_name(object_type type)
{
  switch (type) {
  case BOX_OBJECT:
    return "box";
  case CIRCLE_OBJECT:
    return "circle";
  case ELLIPSE_OBJECT:
    return "ellipse";
  case ARC_OBJECT:
    return "arc";
  case SPLINE_OBJECT:
    return "spline";
  case LINE_OBJECT:
    return "line";
  case POLYGON_OBJECT:
    return "polygon";
  case ARROW_OBJECT:
    return "arrow";
  case MOVE_OBJECT:
    return "move";
  case TEXT_OBJECT:
    return "\"\"";
  case BLOCK_OBJECT:
    return "[]";
  case OTHER_OBJECT:
  case MARK_OBJECT:
  default:
    break;
  }
  return "object";
}

static char sprintf_buf[1024];

char *format_number(const char *fmt, double n)
{
  if (0 /* nullptr */ == fmt)
    fmt = "%g";
  return do_sprintf(fmt, &n, 1);
}

char *do_sprintf(const char *fmt, const double *v, int nv)
{
  // Define valid conversion specifiers and modifiers.
  static const char spcs[] = "eEfgG%";
  static const char mods[] = "#-+ 0123456789.";
  string result;
  int i = 0;
  string one_format;
  while (*fmt) {
    if ('%' == *fmt) {
      one_format += *fmt++;
      for (; *fmt != '\0' && strchr(mods, *fmt) != 0; fmt++)
	one_format += *fmt;
      if ('\0' == *fmt || strchr(spcs, *fmt) == 0) {
	lex_error("invalid sprintf conversion specifier '%1'", *fmt);
	result += one_format;
	result += fmt;
	break;
      }
      if ('%' == *fmt) {
	fmt++;
	snprintf(sprintf_buf, sizeof sprintf_buf, "%%");
      }
      else {
	if (i >= nv) {
	  lex_error("too few arguments to sprintf");
	  result += one_format;
	  result += fmt;
	  break;
	}
	one_format += *fmt++;
	one_format += '\0';
// We validated the format string above.  Most conversion specifiers are
// rejected, including `n`.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	snprintf(sprintf_buf, sizeof sprintf_buf,
		 one_format.contents(), v[i++]);
#pragma GCC diagnostic pop
      }
      one_format.clear();
      result += sprintf_buf;
    }
    else
      result += *fmt++;
  }
  result += '\0';
  return strsave(result.contents());
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
