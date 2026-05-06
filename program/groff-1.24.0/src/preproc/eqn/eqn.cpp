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
#line 18 "../src/preproc/eqn/eqn.ypp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib.h"

#include "box.h"
extern int non_empty_flag;
int yylex();
void yyerror(const char *);

#line 88 "src/preproc/eqn/eqn.cpp"

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

#line 274 "src/preproc/eqn/eqn.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SRC_PREPROC_EQN_EQN_HPP_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_OVER = 3,                       /* OVER  */
  YYSYMBOL_SMALLOVER = 4,                  /* SMALLOVER  */
  YYSYMBOL_SQRT = 5,                       /* SQRT  */
  YYSYMBOL_SUB = 6,                        /* SUB  */
  YYSYMBOL_SUP = 7,                        /* SUP  */
  YYSYMBOL_LPILE = 8,                      /* LPILE  */
  YYSYMBOL_RPILE = 9,                      /* RPILE  */
  YYSYMBOL_CPILE = 10,                     /* CPILE  */
  YYSYMBOL_PILE = 11,                      /* PILE  */
  YYSYMBOL_LEFT = 12,                      /* LEFT  */
  YYSYMBOL_RIGHT = 13,                     /* RIGHT  */
  YYSYMBOL_TO = 14,                        /* TO  */
  YYSYMBOL_FROM = 15,                      /* FROM  */
  YYSYMBOL_SIZE = 16,                      /* SIZE  */
  YYSYMBOL_FONT = 17,                      /* FONT  */
  YYSYMBOL_ROMAN = 18,                     /* ROMAN  */
  YYSYMBOL_BOLD = 19,                      /* BOLD  */
  YYSYMBOL_ITALIC = 20,                    /* ITALIC  */
  YYSYMBOL_FAT = 21,                       /* FAT  */
  YYSYMBOL_ACCENT = 22,                    /* ACCENT  */
  YYSYMBOL_BAR = 23,                       /* BAR  */
  YYSYMBOL_UNDER = 24,                     /* UNDER  */
  YYSYMBOL_ABOVE = 25,                     /* ABOVE  */
  YYSYMBOL_TEXT = 26,                      /* TEXT  */
  YYSYMBOL_QUOTED_TEXT = 27,               /* QUOTED_TEXT  */
  YYSYMBOL_FWD = 28,                       /* FWD  */
  YYSYMBOL_BACK = 29,                      /* BACK  */
  YYSYMBOL_DOWN = 30,                      /* DOWN  */
  YYSYMBOL_UP = 31,                        /* UP  */
  YYSYMBOL_MATRIX = 32,                    /* MATRIX  */
  YYSYMBOL_COL = 33,                       /* COL  */
  YYSYMBOL_LCOL = 34,                      /* LCOL  */
  YYSYMBOL_RCOL = 35,                      /* RCOL  */
  YYSYMBOL_CCOL = 36,                      /* CCOL  */
  YYSYMBOL_MARK = 37,                      /* MARK  */
  YYSYMBOL_LINEUP = 38,                    /* LINEUP  */
  YYSYMBOL_TYPE = 39,                      /* TYPE  */
  YYSYMBOL_VCENTER = 40,                   /* VCENTER  */
  YYSYMBOL_PRIME = 41,                     /* PRIME  */
  YYSYMBOL_SPLIT = 42,                     /* SPLIT  */
  YYSYMBOL_NOSPLIT = 43,                   /* NOSPLIT  */
  YYSYMBOL_UACCENT = 44,                   /* UACCENT  */
  YYSYMBOL_SPECIAL = 45,                   /* SPECIAL  */
  YYSYMBOL_SPACE = 46,                     /* SPACE  */
  YYSYMBOL_GFONT = 47,                     /* GFONT  */
  YYSYMBOL_GSIZE = 48,                     /* GSIZE  */
  YYSYMBOL_DEFINE = 49,                    /* DEFINE  */
  YYSYMBOL_NDEFINE = 50,                   /* NDEFINE  */
  YYSYMBOL_TDEFINE = 51,                   /* TDEFINE  */
  YYSYMBOL_SDEFINE = 52,                   /* SDEFINE  */
  YYSYMBOL_UNDEF = 53,                     /* UNDEF  */
  YYSYMBOL_IFDEF = 54,                     /* IFDEF  */
  YYSYMBOL_INCLUDE = 55,                   /* INCLUDE  */
  YYSYMBOL_DELIM = 56,                     /* DELIM  */
  YYSYMBOL_CHARTYPE = 57,                  /* CHARTYPE  */
  YYSYMBOL_SET = 58,                       /* SET  */
  YYSYMBOL_RESET = 59,                     /* RESET  */
  YYSYMBOL_GRFONT = 60,                    /* GRFONT  */
  YYSYMBOL_GBFONT = 61,                    /* GBFONT  */
  YYSYMBOL_GIFONT = 62,                    /* GIFONT  */
  YYSYMBOL_63_ = 63,                       /* '^'  */
  YYSYMBOL_64_ = 64,                       /* '~'  */
  YYSYMBOL_65_t_ = 65,                     /* '\t'  */
  YYSYMBOL_66_ = 66,                       /* '{'  */
  YYSYMBOL_67_ = 67,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 68,                  /* $accept  */
  YYSYMBOL_top = 69,                       /* top  */
  YYSYMBOL_equation = 70,                  /* equation  */
  YYSYMBOL_mark = 71,                      /* mark  */
  YYSYMBOL_from_to = 72,                   /* from_to  */
  YYSYMBOL_sqrt_over = 73,                 /* sqrt_over  */
  YYSYMBOL_script = 74,                    /* script  */
  YYSYMBOL_nonsup = 75,                    /* nonsup  */
  YYSYMBOL_simple = 76,                    /* simple  */
  YYSYMBOL_number = 77,                    /* number  */
  YYSYMBOL_pile_element_list = 78,         /* pile_element_list  */
  YYSYMBOL_pile_arg = 79,                  /* pile_arg  */
  YYSYMBOL_column_list = 80,               /* column_list  */
  YYSYMBOL_column_element_list = 81,       /* column_element_list  */
  YYSYMBOL_column_arg = 82,                /* column_arg  */
  YYSYMBOL_column = 83,                    /* column  */
  YYSYMBOL_text = 84,                      /* text  */
  YYSYMBOL_delim = 85                      /* delim  */
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
typedef yytype_uint8 yy_state_t;

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
#define YYFINAL  72
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   387

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  68
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  18
/* YYNRULES -- Number of rules.  */
#define YYNRULES  75
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  142

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   317


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,    65,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    63,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    66,     2,    67,    64,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   128,   128,   130,   135,   137,   148,   150,   152,   157,
     159,   161,   163,   165,   170,   172,   174,   176,   181,   183,
     188,   190,   192,   197,   199,   201,   203,   205,   207,   209,
     211,   213,   215,   217,   219,   221,   223,   225,   227,   229,
     231,   233,   235,   237,   239,   241,   243,   245,   247,   249,
     251,   253,   255,   257,   259,   261,   266,   276,   278,   283,
     285,   290,   292,   297,   299,   304,   306,   311,   313,   315,
     317,   321,   323,   328,   330,   332
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
  "\"end of file\"", "error", "\"invalid token\"", "OVER", "SMALLOVER",
  "SQRT", "SUB", "SUP", "LPILE", "RPILE", "CPILE", "PILE", "LEFT", "RIGHT",
  "TO", "FROM", "SIZE", "FONT", "ROMAN", "BOLD", "ITALIC", "FAT", "ACCENT",
  "BAR", "UNDER", "ABOVE", "TEXT", "QUOTED_TEXT", "FWD", "BACK", "DOWN",
  "UP", "MATRIX", "COL", "LCOL", "RCOL", "CCOL", "MARK", "LINEUP", "TYPE",
  "VCENTER", "PRIME", "SPLIT", "NOSPLIT", "UACCENT", "SPECIAL", "SPACE",
  "GFONT", "GSIZE", "DEFINE", "NDEFINE", "TDEFINE", "SDEFINE", "UNDEF",
  "IFDEF", "INCLUDE", "DELIM", "CHARTYPE", "SET", "RESET", "GRFONT",
  "GBFONT", "GIFONT", "'^'", "'~'", "'\\t'", "'{'", "'}'", "$accept",
  "top", "equation", "mark", "from_to", "sqrt_over", "script", "nonsup",
  "simple", "number", "pile_element_list", "pile_arg", "column_list",
  "column_element_list", "column_arg", "column", "text", "delim", YY_NULLPTR
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
     315,   316,   317,    94,   126,     9,   123,   125
};
#endif

#define YYPACT_NINF (-76)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     234,   275,   -13,   -13,   -13,   -13,     6,    60,    60,   316,
     316,   316,   316,   -76,   -76,    60,    60,    60,    60,   -50,
     234,   234,    60,   316,    -9,    -2,    60,   -76,   -76,   -76,
     234,    26,   234,   -76,   -76,    79,   -76,   -76,    22,   -76,
     -76,   -76,   234,   -43,   -76,   -76,   -76,   -76,   -76,   -76,
     -76,   -76,   234,   316,   316,    57,    57,    57,    57,   316,
     316,   316,   316,     7,   -76,   -76,   316,    57,   -76,   -76,
     316,   130,   -76,   -76,   275,   275,   275,   275,   316,   316,
     316,   -76,   -76,   -76,   316,   234,    -8,   234,   193,    57,
      57,    57,    57,    57,    57,     8,     8,     8,     8,     3,
     -76,    57,    57,   -76,   -76,   -76,   -76,    81,   -76,   343,
     -76,   -76,   -76,   234,   -76,    -6,     6,   234,   -39,   -76,
     -76,   -76,   -76,   -76,   -76,   275,   275,   316,   234,   -76,
     -76,   234,    -5,   234,   -76,   -76,   -76,   234,   -76,    -3,
     234,   -76
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    23,    24,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    27,    28,    29,
       0,     0,     3,     4,     6,     9,    14,    18,    20,    15,
      71,    72,     0,     0,    32,    56,    33,    34,    31,    74,
      75,    73,     0,     0,     0,    43,    44,    45,    46,     0,
       0,     0,     0,     0,     7,     8,     0,    54,    25,    26,
       0,     0,     1,     5,     0,     0,     0,     0,     0,     0,
       0,    38,    39,    40,     0,    57,     0,     0,    37,    48,
      47,    49,    50,    52,    51,     0,     0,     0,     0,     0,
      61,    53,    55,    30,    16,    17,    10,    11,    21,    20,
      19,    41,    42,     0,    59,     0,     0,     0,     0,    67,
      68,    69,    70,    35,    62,     0,     0,     0,    58,    60,
      36,    63,     0,     0,    12,    13,    22,     0,    65,     0,
      64,    66
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -76,   -76,     0,   -17,   -75,     1,   -67,   -47,    46,    -7,
     -22,    44,   -76,   -66,    23,     4,    -1,   -24
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    31,    85,    33,    34,    35,    36,    37,    38,    43,
      86,    44,    99,   132,   119,   100,    45,    52
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      32,   106,    39,    64,    65,    51,    53,    54,    59,    60,
      61,    62,   110,    40,    41,    73,    63,   113,    68,   113,
     137,    66,   137,    87,    69,    70,    72,   133,    78,    79,
      71,   108,    40,    41,    40,    41,    95,    96,    97,    98,
      95,    96,    97,    98,    80,    81,    82,    46,    47,    48,
     134,   135,    88,    42,    73,    55,    56,    57,    58,   114,
     136,   129,   138,    83,   141,   115,    84,   139,    73,    67,
     123,    73,    49,    50,   117,   104,   105,     0,   107,    80,
      81,    82,    74,    75,    74,    75,    40,    41,   118,   118,
     118,   118,   130,    76,    77,   125,   126,     0,    83,    89,
      90,    84,     0,   124,     0,    91,    92,    93,    94,     0,
       0,    73,   101,   128,    73,    51,   102,   131,     0,   120,
     121,   122,     0,    73,   109,     0,   111,     0,     0,     0,
     112,     0,     0,   131,     0,     1,     0,   140,     2,     3,
       4,     5,     6,     0,     0,     0,     7,     8,     9,    10,
      11,    12,     0,     0,     0,     0,    13,    14,    15,    16,
      17,    18,    19,     0,     0,     0,     0,    20,    21,    22,
      23,     0,    24,    25,     0,    26,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    27,    28,    29,    30,   103,     1,     0,
       0,     2,     3,     4,     5,     6,   116,     0,     0,     7,
       8,     9,    10,    11,    12,     0,     0,     0,     0,    13,
      14,    15,    16,    17,    18,    19,     0,     0,     0,     0,
      20,    21,    22,    23,     0,    24,    25,     0,    26,     1,
       0,     0,     2,     3,     4,     5,     6,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    27,    28,    29,    30,
      13,    14,    15,    16,    17,    18,    19,     0,     0,     0,
       0,    20,    21,    22,    23,     0,    24,    25,     0,    26,
       1,     0,     0,     2,     3,     4,     5,     6,     0,     0,
       0,     7,     8,     9,    10,    11,    12,    27,    28,    29,
      30,    13,    14,    15,    16,    17,    18,    19,     0,     0,
       0,     0,     0,     0,    22,    23,     0,    24,    25,     0,
      26,     0,     0,     0,     2,     3,     4,     5,     6,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    27,    28,
      29,    30,    13,    14,    15,    16,    17,    18,    19,    78,
     127,     0,     0,     0,     0,    22,    23,     0,    24,    25,
       0,    26,     0,     0,     0,    80,    81,    82,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    27,
      28,    29,    30,     0,    83,     0,     0,    84
};

static const yytype_int16 yycheck[] =
{
       0,    76,     1,    20,    21,     6,     7,     8,    15,    16,
      17,    18,    79,    26,    27,    32,    66,    25,    27,    25,
      25,    22,    25,    66,    26,    26,     0,    66,     6,     7,
      30,    78,    26,    27,    26,    27,    33,    34,    35,    36,
      33,    34,    35,    36,    22,    23,    24,     3,     4,     5,
     125,   126,    52,    66,    71,     9,    10,    11,    12,    67,
     127,    67,    67,    41,    67,    87,    44,   133,    85,    23,
      67,    88,    66,    67,    66,    74,    75,    -1,    77,    22,
      23,    24,     3,     4,     3,     4,    26,    27,    95,    96,
      97,    98,   116,    14,    15,    14,    15,    -1,    41,    53,
      54,    44,    -1,    99,    -1,    59,    60,    61,    62,    -1,
      -1,   128,    66,   113,   131,   116,    70,   117,    -1,    96,
      97,    98,    -1,   140,    78,    -1,    80,    -1,    -1,    -1,
      84,    -1,    -1,   133,    -1,     5,    -1,   137,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    16,    17,    18,    19,
      20,    21,    -1,    -1,    -1,    -1,    26,    27,    28,    29,
      30,    31,    32,    -1,    -1,    -1,    -1,    37,    38,    39,
      40,    -1,    42,    43,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    63,    64,    65,    66,    67,     5,    -1,
      -1,     8,     9,    10,    11,    12,    13,    -1,    -1,    16,
      17,    18,    19,    20,    21,    -1,    -1,    -1,    -1,    26,
      27,    28,    29,    30,    31,    32,    -1,    -1,    -1,    -1,
      37,    38,    39,    40,    -1,    42,    43,    -1,    45,     5,
      -1,    -1,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      16,    17,    18,    19,    20,    21,    63,    64,    65,    66,
      26,    27,    28,    29,    30,    31,    32,    -1,    -1,    -1,
      -1,    37,    38,    39,    40,    -1,    42,    43,    -1,    45,
       5,    -1,    -1,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    21,    63,    64,    65,
      66,    26,    27,    28,    29,    30,    31,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    39,    40,    -1,    42,    43,    -1,
      45,    -1,    -1,    -1,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    16,    17,    18,    19,    20,    21,    63,    64,
      65,    66,    26,    27,    28,    29,    30,    31,    32,     6,
       7,    -1,    -1,    -1,    -1,    39,    40,    -1,    42,    43,
      -1,    45,    -1,    -1,    -1,    22,    23,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,
      64,    65,    66,    -1,    41,    -1,    -1,    44
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     5,     8,     9,    10,    11,    12,    16,    17,    18,
      19,    20,    21,    26,    27,    28,    29,    30,    31,    32,
      37,    38,    39,    40,    42,    43,    45,    63,    64,    65,
      66,    69,    70,    71,    72,    73,    74,    75,    76,    73,
      26,    27,    66,    77,    79,    84,    79,    79,    79,    66,
      67,    84,    85,    84,    84,    76,    76,    76,    76,    77,
      77,    77,    77,    66,    71,    71,    84,    76,    27,    26,
      84,    70,     0,    71,     3,     4,    14,    15,     6,     7,
      22,    23,    24,    41,    44,    70,    78,    66,    70,    76,
      76,    76,    76,    76,    76,    33,    34,    35,    36,    80,
      83,    76,    76,    67,    73,    73,    72,    73,    75,    76,
      74,    76,    76,    25,    67,    78,    13,    66,    77,    82,
      82,    82,    82,    67,    83,    14,    15,     7,    70,    67,
      85,    70,    81,    66,    72,    72,    74,    25,    67,    81,
      70,    67
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    68,    69,    69,    70,    70,    71,    71,    71,    72,
      72,    72,    72,    72,    73,    73,    73,    73,    74,    74,
      75,    75,    75,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
      76,    76,    76,    76,    76,    76,    77,    78,    78,    79,
      79,    80,    80,    81,    81,    82,    82,    83,    83,    83,
      83,    84,    84,    85,    85,    85
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     1,     2,     2,     1,
       3,     3,     5,     5,     1,     2,     3,     3,     1,     3,
       1,     3,     5,     1,     1,     2,     2,     1,     1,     1,
       3,     2,     2,     2,     2,     4,     5,     3,     2,     2,
       2,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     2,     3,     1,     1,     3,     3,
       4,     1,     2,     1,     3,     3,     4,     2,     2,     2,
       2,     1,     1,     1,     1,     1
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
  case 3: /* top: equation  */
#line 131 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].b)->top_level(); non_empty_flag = 1; }
#line 1492 "src/preproc/eqn/eqn.cpp"
    break;

  case 4: /* equation: mark  */
#line 136 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[0].b); }
#line 1498 "src/preproc/eqn/eqn.cpp"
    break;

  case 5: /* equation: equation mark  */
#line 138 "../src/preproc/eqn/eqn.ypp"
                {
		  list_box *lb = (yyvsp[-1].b)->to_list_box();
		  if (!lb)
		    lb = new list_box((yyvsp[-1].b));
		  lb->append((yyvsp[0].b));
		  (yyval.b) = lb;
		}
#line 1510 "src/preproc/eqn/eqn.cpp"
    break;

  case 6: /* mark: from_to  */
#line 149 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[0].b); }
#line 1516 "src/preproc/eqn/eqn.cpp"
    break;

  case 7: /* mark: MARK mark  */
#line 151 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_mark_box((yyvsp[0].b)); }
#line 1522 "src/preproc/eqn/eqn.cpp"
    break;

  case 8: /* mark: LINEUP mark  */
#line 153 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_lineup_box((yyvsp[0].b)); }
#line 1528 "src/preproc/eqn/eqn.cpp"
    break;

  case 9: /* from_to: sqrt_over  */
#line 158 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[0].b); }
#line 1534 "src/preproc/eqn/eqn.cpp"
    break;

  case 10: /* from_to: sqrt_over TO from_to  */
#line 160 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_limit_box((yyvsp[-2].b), 0, (yyvsp[0].b)); }
#line 1540 "src/preproc/eqn/eqn.cpp"
    break;

  case 11: /* from_to: sqrt_over FROM sqrt_over  */
#line 162 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_limit_box((yyvsp[-2].b), (yyvsp[0].b), 0); }
#line 1546 "src/preproc/eqn/eqn.cpp"
    break;

  case 12: /* from_to: sqrt_over FROM sqrt_over TO from_to  */
#line 164 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_limit_box((yyvsp[-4].b), (yyvsp[-2].b), (yyvsp[0].b)); }
#line 1552 "src/preproc/eqn/eqn.cpp"
    break;

  case 13: /* from_to: sqrt_over FROM sqrt_over FROM from_to  */
#line 166 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_limit_box((yyvsp[-4].b), make_limit_box((yyvsp[-2].b), (yyvsp[0].b), 0), 0); }
#line 1558 "src/preproc/eqn/eqn.cpp"
    break;

  case 14: /* sqrt_over: script  */
#line 171 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[0].b); }
#line 1564 "src/preproc/eqn/eqn.cpp"
    break;

  case 15: /* sqrt_over: SQRT sqrt_over  */
#line 173 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_sqrt_box((yyvsp[0].b)); }
#line 1570 "src/preproc/eqn/eqn.cpp"
    break;

  case 16: /* sqrt_over: sqrt_over OVER sqrt_over  */
#line 175 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_over_box((yyvsp[-2].b), (yyvsp[0].b)); }
#line 1576 "src/preproc/eqn/eqn.cpp"
    break;

  case 17: /* sqrt_over: sqrt_over SMALLOVER sqrt_over  */
#line 177 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_small_over_box((yyvsp[-2].b), (yyvsp[0].b)); }
#line 1582 "src/preproc/eqn/eqn.cpp"
    break;

  case 18: /* script: nonsup  */
#line 182 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[0].b); }
#line 1588 "src/preproc/eqn/eqn.cpp"
    break;

  case 19: /* script: simple SUP script  */
#line 184 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_script_box((yyvsp[-2].b), 0, (yyvsp[0].b)); }
#line 1594 "src/preproc/eqn/eqn.cpp"
    break;

  case 20: /* nonsup: simple  */
#line 189 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[0].b); }
#line 1600 "src/preproc/eqn/eqn.cpp"
    break;

  case 21: /* nonsup: simple SUB nonsup  */
#line 191 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_script_box((yyvsp[-2].b), (yyvsp[0].b), 0); }
#line 1606 "src/preproc/eqn/eqn.cpp"
    break;

  case 22: /* nonsup: simple SUB simple SUP script  */
#line 193 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_script_box((yyvsp[-4].b), (yyvsp[-2].b), (yyvsp[0].b)); }
#line 1612 "src/preproc/eqn/eqn.cpp"
    break;

  case 23: /* simple: TEXT  */
#line 198 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = split_text((yyvsp[0].str)); }
#line 1618 "src/preproc/eqn/eqn.cpp"
    break;

  case 24: /* simple: QUOTED_TEXT  */
#line 200 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new quoted_text_box((yyvsp[0].str)); }
#line 1624 "src/preproc/eqn/eqn.cpp"
    break;

  case 25: /* simple: SPLIT QUOTED_TEXT  */
#line 202 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = split_text((yyvsp[0].str)); }
#line 1630 "src/preproc/eqn/eqn.cpp"
    break;

  case 26: /* simple: NOSPLIT TEXT  */
#line 204 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new quoted_text_box((yyvsp[0].str)); }
#line 1636 "src/preproc/eqn/eqn.cpp"
    break;

  case 27: /* simple: '^'  */
#line 206 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new half_space_box; }
#line 1642 "src/preproc/eqn/eqn.cpp"
    break;

  case 28: /* simple: '~'  */
#line 208 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new full_space_box; }
#line 1648 "src/preproc/eqn/eqn.cpp"
    break;

  case 29: /* simple: '\t'  */
#line 210 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new tab_box; }
#line 1654 "src/preproc/eqn/eqn.cpp"
    break;

  case 30: /* simple: '{' equation '}'  */
#line 212 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[-1].b); }
#line 1660 "src/preproc/eqn/eqn.cpp"
    break;

  case 31: /* simple: PILE pile_arg  */
#line 214 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].pb)->set_alignment(CENTER_ALIGN); (yyval.b) = (yyvsp[0].pb); }
#line 1666 "src/preproc/eqn/eqn.cpp"
    break;

  case 32: /* simple: LPILE pile_arg  */
#line 216 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].pb)->set_alignment(LEFT_ALIGN); (yyval.b) = (yyvsp[0].pb); }
#line 1672 "src/preproc/eqn/eqn.cpp"
    break;

  case 33: /* simple: RPILE pile_arg  */
#line 218 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].pb)->set_alignment(RIGHT_ALIGN); (yyval.b) = (yyvsp[0].pb); }
#line 1678 "src/preproc/eqn/eqn.cpp"
    break;

  case 34: /* simple: CPILE pile_arg  */
#line 220 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].pb)->set_alignment(CENTER_ALIGN); (yyval.b) = (yyvsp[0].pb); }
#line 1684 "src/preproc/eqn/eqn.cpp"
    break;

  case 35: /* simple: MATRIX '{' column_list '}'  */
#line 222 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = (yyvsp[-1].mb); }
#line 1690 "src/preproc/eqn/eqn.cpp"
    break;

  case 36: /* simple: LEFT delim equation RIGHT delim  */
#line 224 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_delim_box((yyvsp[-3].str), (yyvsp[-2].b), (yyvsp[0].str)); }
#line 1696 "src/preproc/eqn/eqn.cpp"
    break;

  case 37: /* simple: LEFT delim equation  */
#line 226 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_delim_box((yyvsp[-1].str), (yyvsp[0].b), 0); }
#line 1702 "src/preproc/eqn/eqn.cpp"
    break;

  case 38: /* simple: simple BAR  */
#line 228 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_overline_box((yyvsp[-1].b)); }
#line 1708 "src/preproc/eqn/eqn.cpp"
    break;

  case 39: /* simple: simple UNDER  */
#line 230 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_underline_box((yyvsp[-1].b)); }
#line 1714 "src/preproc/eqn/eqn.cpp"
    break;

  case 40: /* simple: simple PRIME  */
#line 232 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_prime_box((yyvsp[-1].b)); }
#line 1720 "src/preproc/eqn/eqn.cpp"
    break;

  case 41: /* simple: simple ACCENT simple  */
#line 234 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_accent_box((yyvsp[-2].b), (yyvsp[0].b)); }
#line 1726 "src/preproc/eqn/eqn.cpp"
    break;

  case 42: /* simple: simple UACCENT simple  */
#line 236 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_uaccent_box((yyvsp[-2].b), (yyvsp[0].b)); }
#line 1732 "src/preproc/eqn/eqn.cpp"
    break;

  case 43: /* simple: ROMAN simple  */
#line 238 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new font_box(strsave(get_grfont()), (yyvsp[0].b)); }
#line 1738 "src/preproc/eqn/eqn.cpp"
    break;

  case 44: /* simple: BOLD simple  */
#line 240 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new font_box(strsave(get_gbfont()), (yyvsp[0].b)); }
#line 1744 "src/preproc/eqn/eqn.cpp"
    break;

  case 45: /* simple: ITALIC simple  */
#line 242 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new font_box(strsave(get_gifont()), (yyvsp[0].b)); }
#line 1750 "src/preproc/eqn/eqn.cpp"
    break;

  case 46: /* simple: FAT simple  */
#line 244 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new fat_box((yyvsp[0].b)); }
#line 1756 "src/preproc/eqn/eqn.cpp"
    break;

  case 47: /* simple: FONT text simple  */
#line 246 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new font_box((yyvsp[-1].str), (yyvsp[0].b)); }
#line 1762 "src/preproc/eqn/eqn.cpp"
    break;

  case 48: /* simple: SIZE text simple  */
#line 248 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new size_box((yyvsp[-1].str), (yyvsp[0].b)); }
#line 1768 "src/preproc/eqn/eqn.cpp"
    break;

  case 49: /* simple: FWD number simple  */
#line 250 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new hmotion_box((yyvsp[-1].n), (yyvsp[0].b)); }
#line 1774 "src/preproc/eqn/eqn.cpp"
    break;

  case 50: /* simple: BACK number simple  */
#line 252 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new hmotion_box(-(yyvsp[-1].n), (yyvsp[0].b)); }
#line 1780 "src/preproc/eqn/eqn.cpp"
    break;

  case 51: /* simple: UP number simple  */
#line 254 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new vmotion_box((yyvsp[-1].n), (yyvsp[0].b)); }
#line 1786 "src/preproc/eqn/eqn.cpp"
    break;

  case 52: /* simple: DOWN number simple  */
#line 256 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new vmotion_box(-(yyvsp[-1].n), (yyvsp[0].b)); }
#line 1792 "src/preproc/eqn/eqn.cpp"
    break;

  case 53: /* simple: TYPE text simple  */
#line 258 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].b)->set_spacing_type((yyvsp[-1].str)); (yyval.b) = (yyvsp[0].b); }
#line 1798 "src/preproc/eqn/eqn.cpp"
    break;

  case 54: /* simple: VCENTER simple  */
#line 260 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = new vcenter_box((yyvsp[0].b)); }
#line 1804 "src/preproc/eqn/eqn.cpp"
    break;

  case 55: /* simple: SPECIAL text simple  */
#line 262 "../src/preproc/eqn/eqn.ypp"
                { (yyval.b) = make_special_box((yyvsp[-1].str), (yyvsp[0].b)); }
#line 1810 "src/preproc/eqn/eqn.cpp"
    break;

  case 56: /* number: text  */
#line 267 "../src/preproc/eqn/eqn.ypp"
                {
		  int n;
		  if (sscanf((yyvsp[0].str), "%d", &n) == 1)
		    (yyval.n) = n;
		  delete[] (yyvsp[0].str);
		}
#line 1821 "src/preproc/eqn/eqn.cpp"
    break;

  case 57: /* pile_element_list: equation  */
#line 277 "../src/preproc/eqn/eqn.ypp"
                { (yyval.pb) = new pile_box((yyvsp[0].b)); }
#line 1827 "src/preproc/eqn/eqn.cpp"
    break;

  case 58: /* pile_element_list: pile_element_list ABOVE equation  */
#line 279 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[-2].pb)->append((yyvsp[0].b)); (yyval.pb) = (yyvsp[-2].pb); }
#line 1833 "src/preproc/eqn/eqn.cpp"
    break;

  case 59: /* pile_arg: '{' pile_element_list '}'  */
#line 284 "../src/preproc/eqn/eqn.ypp"
                { (yyval.pb) = (yyvsp[-1].pb); }
#line 1839 "src/preproc/eqn/eqn.cpp"
    break;

  case 60: /* pile_arg: number '{' pile_element_list '}'  */
#line 286 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[-1].pb)->set_space((yyvsp[-3].n)); (yyval.pb) = (yyvsp[-1].pb); }
#line 1845 "src/preproc/eqn/eqn.cpp"
    break;

  case 61: /* column_list: column  */
#line 291 "../src/preproc/eqn/eqn.ypp"
                { (yyval.mb) = new matrix_box((yyvsp[0].col)); }
#line 1851 "src/preproc/eqn/eqn.cpp"
    break;

  case 62: /* column_list: column_list column  */
#line 293 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[-1].mb)->append((yyvsp[0].col)); (yyval.mb) = (yyvsp[-1].mb); }
#line 1857 "src/preproc/eqn/eqn.cpp"
    break;

  case 63: /* column_element_list: equation  */
#line 298 "../src/preproc/eqn/eqn.ypp"
                { (yyval.col) = new column((yyvsp[0].b)); }
#line 1863 "src/preproc/eqn/eqn.cpp"
    break;

  case 64: /* column_element_list: column_element_list ABOVE equation  */
#line 300 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[-2].col)->append((yyvsp[0].b)); (yyval.col) = (yyvsp[-2].col); }
#line 1869 "src/preproc/eqn/eqn.cpp"
    break;

  case 65: /* column_arg: '{' column_element_list '}'  */
#line 305 "../src/preproc/eqn/eqn.ypp"
                { (yyval.col) = (yyvsp[-1].col); }
#line 1875 "src/preproc/eqn/eqn.cpp"
    break;

  case 66: /* column_arg: number '{' column_element_list '}'  */
#line 307 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[-1].col)->set_space((yyvsp[-3].n)); (yyval.col) = (yyvsp[-1].col); }
#line 1881 "src/preproc/eqn/eqn.cpp"
    break;

  case 67: /* column: COL column_arg  */
#line 312 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].col)->set_alignment(CENTER_ALIGN); (yyval.col) = (yyvsp[0].col); }
#line 1887 "src/preproc/eqn/eqn.cpp"
    break;

  case 68: /* column: LCOL column_arg  */
#line 314 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].col)->set_alignment(LEFT_ALIGN); (yyval.col) = (yyvsp[0].col); }
#line 1893 "src/preproc/eqn/eqn.cpp"
    break;

  case 69: /* column: RCOL column_arg  */
#line 316 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].col)->set_alignment(RIGHT_ALIGN); (yyval.col) = (yyvsp[0].col); }
#line 1899 "src/preproc/eqn/eqn.cpp"
    break;

  case 70: /* column: CCOL column_arg  */
#line 318 "../src/preproc/eqn/eqn.ypp"
                { (yyvsp[0].col)->set_alignment(CENTER_ALIGN); (yyval.col) = (yyvsp[0].col); }
#line 1905 "src/preproc/eqn/eqn.cpp"
    break;

  case 71: /* text: TEXT  */
#line 322 "../src/preproc/eqn/eqn.ypp"
                { (yyval.str) = (yyvsp[0].str); }
#line 1911 "src/preproc/eqn/eqn.cpp"
    break;

  case 72: /* text: QUOTED_TEXT  */
#line 324 "../src/preproc/eqn/eqn.ypp"
                { (yyval.str) = (yyvsp[0].str); }
#line 1917 "src/preproc/eqn/eqn.cpp"
    break;

  case 73: /* delim: text  */
#line 329 "../src/preproc/eqn/eqn.ypp"
                { (yyval.str) = (yyvsp[0].str); }
#line 1923 "src/preproc/eqn/eqn.cpp"
    break;

  case 74: /* delim: '{'  */
#line 331 "../src/preproc/eqn/eqn.ypp"
                { (yyval.str) = strsave("{"); }
#line 1929 "src/preproc/eqn/eqn.cpp"
    break;

  case 75: /* delim: '}'  */
#line 333 "../src/preproc/eqn/eqn.ypp"
                { (yyval.str) = strsave("}"); }
#line 1935 "src/preproc/eqn/eqn.cpp"
    break;


#line 1939 "src/preproc/eqn/eqn.cpp"

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

#line 336 "../src/preproc/eqn/eqn.ypp"

