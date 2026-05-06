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

#ifndef YY_YY_SRC_PREPROC_EQN_EQN_HPP_INCLUDED
# define YY_YY_SRC_PREPROC_EQN_EQN_HPP_INCLUDED
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
    OVER = 258,                    /* OVER  */
    SMALLOVER = 259,               /* SMALLOVER  */
    SQRT = 260,                    /* SQRT  */
    SUB = 261,                     /* SUB  */
    SUP = 262,                     /* SUP  */
    LPILE = 263,                   /* LPILE  */
    RPILE = 264,                   /* RPILE  */
    CPILE = 265,                   /* CPILE  */
    PILE = 266,                    /* PILE  */
    LEFT = 267,                    /* LEFT  */
    RIGHT = 268,                   /* RIGHT  */
    TO = 269,                      /* TO  */
    FROM = 270,                    /* FROM  */
    SIZE = 271,                    /* SIZE  */
    FONT = 272,                    /* FONT  */
    ROMAN = 273,                   /* ROMAN  */
    BOLD = 274,                    /* BOLD  */
    ITALIC = 275,                  /* ITALIC  */
    FAT = 276,                     /* FAT  */
    ACCENT = 277,                  /* ACCENT  */
    BAR = 278,                     /* BAR  */
    UNDER = 279,                   /* UNDER  */
    ABOVE = 280,                   /* ABOVE  */
    TEXT = 281,                    /* TEXT  */
    QUOTED_TEXT = 282,             /* QUOTED_TEXT  */
    FWD = 283,                     /* FWD  */
    BACK = 284,                    /* BACK  */
    DOWN = 285,                    /* DOWN  */
    UP = 286,                      /* UP  */
    MATRIX = 287,                  /* MATRIX  */
    COL = 288,                     /* COL  */
    LCOL = 289,                    /* LCOL  */
    RCOL = 290,                    /* RCOL  */
    CCOL = 291,                    /* CCOL  */
    MARK = 292,                    /* MARK  */
    LINEUP = 293,                  /* LINEUP  */
    TYPE = 294,                    /* TYPE  */
    VCENTER = 295,                 /* VCENTER  */
    PRIME = 296,                   /* PRIME  */
    SPLIT = 297,                   /* SPLIT  */
    NOSPLIT = 298,                 /* NOSPLIT  */
    UACCENT = 299,                 /* UACCENT  */
    SPECIAL = 300,                 /* SPECIAL  */
    SPACE = 301,                   /* SPACE  */
    GFONT = 302,                   /* GFONT  */
    GSIZE = 303,                   /* GSIZE  */
    DEFINE = 304,                  /* DEFINE  */
    NDEFINE = 305,                 /* NDEFINE  */
    TDEFINE = 306,                 /* TDEFINE  */
    SDEFINE = 307,                 /* SDEFINE  */
    UNDEF = 308,                   /* UNDEF  */
    IFDEF = 309,                   /* IFDEF  */
    INCLUDE = 310,                 /* INCLUDE  */
    DELIM = 311,                   /* DELIM  */
    CHARTYPE = 312,                /* CHARTYPE  */
    SET = 313,                     /* SET  */
    RESET = 314,                   /* RESET  */
    GRFONT = 315,                  /* GRFONT  */
    GBFONT = 316,                  /* GBFONT  */
    GIFONT = 317                   /* GIFONT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define OVER 258
#define SMALLOVER 259
#define SQRT 260
#define SUB 261
#define SUP 262
#define LPILE 263
#define RPILE 264
#define CPILE 265
#define PILE 266
#define LEFT 267
#define RIGHT 268
#define TO 269
#define FROM 270
#define SIZE 271
#define FONT 272
#define ROMAN 273
#define BOLD 274
#define ITALIC 275
#define FAT 276
#define ACCENT 277
#define BAR 278
#define UNDER 279
#define ABOVE 280
#define TEXT 281
#define QUOTED_TEXT 282
#define FWD 283
#define BACK 284
#define DOWN 285
#define UP 286
#define MATRIX 287
#define COL 288
#define LCOL 289
#define RCOL 290
#define CCOL 291
#define MARK 292
#define LINEUP 293
#define TYPE 294
#define VCENTER 295
#define PRIME 296
#define SPLIT 297
#define NOSPLIT 298
#define UACCENT 299
#define SPECIAL 300
#define SPACE 301
#define GFONT 302
#define GSIZE 303
#define DEFINE 304
#define NDEFINE 305
#define TDEFINE 306
#define SDEFINE 307
#define UNDEF 308
#define IFDEF 309
#define INCLUDE 310
#define DELIM 311
#define CHARTYPE 312
#define SET 313
#define RESET 314
#define GRFONT 315
#define GBFONT 316
#define GIFONT 317

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 35 "../src/preproc/eqn/eqn.ypp"

	char *str;
	box *b;
	pile_box *pb;
	matrix_box *mb;
	int n;
	column *col;

#line 200 "src/preproc/eqn/eqn.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_PREPROC_EQN_EQN_HPP_INCLUDED  */
