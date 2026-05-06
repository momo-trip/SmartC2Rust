/* A Bison parser, made by GNU Bison 3.7.5.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 341 "src/preproc/pic/pic.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_PREPROC_PIC_PIC_HPP_INCLUDED  */
