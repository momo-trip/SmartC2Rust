/* A Bison parser, made by GNU Bison 3.8.2.  */

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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 32 "bc.y"


#include "bcdefs.h"
#include "global.h"
#include "proto.h"

/* current function number. */
int cur_func = -1;

/* Expression encoded information -- See comment at expression rules. */
#define EX_ASSGN 0 
#define EX_REG   1
#define EX_COMP  2
#define EX_PAREN 4
#define EX_VOID  8 
#define EX_EMPTY 16


#line 90 "bc.c"

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
#ifndef YY_YY_BC_H_INCLUDED
# define YY_YY_BC_H_INCLUDED
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
    ENDOFLINE = 258,               /* ENDOFLINE  */
    AND = 259,                     /* AND  */
    OR = 260,                      /* OR  */
    NOT = 261,                     /* NOT  */
    STRING = 262,                  /* STRING  */
    NAME = 263,                    /* NAME  */
    NUMBER = 264,                  /* NUMBER  */
    ASSIGN_OP = 265,               /* ASSIGN_OP  */
    REL_OP = 266,                  /* REL_OP  */
    INCR_DECR = 267,               /* INCR_DECR  */
    Define = 268,                  /* Define  */
    Break = 269,                   /* Break  */
    Quit = 270,                    /* Quit  */
    Length = 271,                  /* Length  */
    Return = 272,                  /* Return  */
    For = 273,                     /* For  */
    If = 274,                      /* If  */
    While = 275,                   /* While  */
    Sqrt = 276,                    /* Sqrt  */
    Else = 277,                    /* Else  */
    Scale = 278,                   /* Scale  */
    Ibase = 279,                   /* Ibase  */
    Obase = 280,                   /* Obase  */
    Auto = 281,                    /* Auto  */
    Read = 282,                    /* Read  */
    Random = 283,                  /* Random  */
    Warranty = 284,                /* Warranty  */
    Halt = 285,                    /* Halt  */
    Last = 286,                    /* Last  */
    Continue = 287,                /* Continue  */
    Print = 288,                   /* Print  */
    Limits = 289,                  /* Limits  */
    UNARY_MINUS = 290,             /* UNARY_MINUS  */
    HistoryVar = 291,              /* HistoryVar  */
    Void = 292                     /* Void  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define ENDOFLINE 258
#define AND 259
#define OR 260
#define NOT 261
#define STRING 262
#define NAME 263
#define NUMBER 264
#define ASSIGN_OP 265
#define REL_OP 266
#define INCR_DECR 267
#define Define 268
#define Break 269
#define Quit 270
#define Length 271
#define Return 272
#define For 273
#define If 274
#define While 275
#define Sqrt 276
#define Else 277
#define Scale 278
#define Ibase 279
#define Obase 280
#define Auto 281
#define Read 282
#define Random 283
#define Warranty 284
#define Halt 285
#define Last 286
#define Continue 287
#define Print 288
#define Limits 289
#define UNARY_MINUS 290
#define HistoryVar 291
#define Void 292

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 53 "bc.y"

	char	 *s_value;
	char	  c_value;
	int	  i_value;
	arg_list *a_value;
       

#line 225 "bc.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_BC_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ENDOFLINE = 3,                  /* ENDOFLINE  */
  YYSYMBOL_AND = 4,                        /* AND  */
  YYSYMBOL_OR = 5,                         /* OR  */
  YYSYMBOL_NOT = 6,                        /* NOT  */
  YYSYMBOL_STRING = 7,                     /* STRING  */
  YYSYMBOL_NAME = 8,                       /* NAME  */
  YYSYMBOL_NUMBER = 9,                     /* NUMBER  */
  YYSYMBOL_ASSIGN_OP = 10,                 /* ASSIGN_OP  */
  YYSYMBOL_REL_OP = 11,                    /* REL_OP  */
  YYSYMBOL_INCR_DECR = 12,                 /* INCR_DECR  */
  YYSYMBOL_Define = 13,                    /* Define  */
  YYSYMBOL_Break = 14,                     /* Break  */
  YYSYMBOL_Quit = 15,                      /* Quit  */
  YYSYMBOL_Length = 16,                    /* Length  */
  YYSYMBOL_Return = 17,                    /* Return  */
  YYSYMBOL_For = 18,                       /* For  */
  YYSYMBOL_If = 19,                        /* If  */
  YYSYMBOL_While = 20,                     /* While  */
  YYSYMBOL_Sqrt = 21,                      /* Sqrt  */
  YYSYMBOL_Else = 22,                      /* Else  */
  YYSYMBOL_Scale = 23,                     /* Scale  */
  YYSYMBOL_Ibase = 24,                     /* Ibase  */
  YYSYMBOL_Obase = 25,                     /* Obase  */
  YYSYMBOL_Auto = 26,                      /* Auto  */
  YYSYMBOL_Read = 27,                      /* Read  */
  YYSYMBOL_Random = 28,                    /* Random  */
  YYSYMBOL_Warranty = 29,                  /* Warranty  */
  YYSYMBOL_Halt = 30,                      /* Halt  */
  YYSYMBOL_Last = 31,                      /* Last  */
  YYSYMBOL_Continue = 32,                  /* Continue  */
  YYSYMBOL_Print = 33,                     /* Print  */
  YYSYMBOL_Limits = 34,                    /* Limits  */
  YYSYMBOL_UNARY_MINUS = 35,               /* UNARY_MINUS  */
  YYSYMBOL_HistoryVar = 36,                /* HistoryVar  */
  YYSYMBOL_Void = 37,                      /* Void  */
  YYSYMBOL_38_ = 38,                       /* '+'  */
  YYSYMBOL_39_ = 39,                       /* '-'  */
  YYSYMBOL_40_ = 40,                       /* '*'  */
  YYSYMBOL_41_ = 41,                       /* '/'  */
  YYSYMBOL_42_ = 42,                       /* '%'  */
  YYSYMBOL_43_ = 43,                       /* '^'  */
  YYSYMBOL_44_ = 44,                       /* ';'  */
  YYSYMBOL_45_ = 45,                       /* '('  */
  YYSYMBOL_46_ = 46,                       /* ')'  */
  YYSYMBOL_47_ = 47,                       /* '{'  */
  YYSYMBOL_48_ = 48,                       /* '}'  */
  YYSYMBOL_49_ = 49,                       /* ','  */
  YYSYMBOL_50_ = 50,                       /* '['  */
  YYSYMBOL_51_ = 51,                       /* ']'  */
  YYSYMBOL_52_ = 52,                       /* '&'  */
  YYSYMBOL_YYACCEPT = 53,                  /* $accept  */
  YYSYMBOL_program = 54,                   /* program  */
  YYSYMBOL_input_item = 55,                /* input_item  */
  YYSYMBOL_opt_newline = 56,               /* opt_newline  */
  YYSYMBOL_semicolon_list = 57,            /* semicolon_list  */
  YYSYMBOL_statement_list = 58,            /* statement_list  */
  YYSYMBOL_statement_or_error = 59,        /* statement_or_error  */
  YYSYMBOL_statement = 60,                 /* statement  */
  YYSYMBOL_61_1 = 61,                      /* $@1  */
  YYSYMBOL_62_2 = 62,                      /* $@2  */
  YYSYMBOL_63_3 = 63,                      /* @3  */
  YYSYMBOL_64_4 = 64,                      /* $@4  */
  YYSYMBOL_65_5 = 65,                      /* $@5  */
  YYSYMBOL_66_6 = 66,                      /* $@6  */
  YYSYMBOL_67_7 = 67,                      /* $@7  */
  YYSYMBOL_68_8 = 68,                      /* $@8  */
  YYSYMBOL_print_list = 69,                /* print_list  */
  YYSYMBOL_print_element = 70,             /* print_element  */
  YYSYMBOL_opt_else = 71,                  /* opt_else  */
  YYSYMBOL_72_9 = 72,                      /* $@9  */
  YYSYMBOL_function = 73,                  /* function  */
  YYSYMBOL_74_10 = 74,                     /* $@10  */
  YYSYMBOL_opt_void = 75,                  /* opt_void  */
  YYSYMBOL_opt_parameter_list = 76,        /* opt_parameter_list  */
  YYSYMBOL_opt_auto_define_list = 77,      /* opt_auto_define_list  */
  YYSYMBOL_define_list = 78,               /* define_list  */
  YYSYMBOL_opt_argument_list = 79,         /* opt_argument_list  */
  YYSYMBOL_argument_list = 80,             /* argument_list  */
  YYSYMBOL_opt_expression = 81,            /* opt_expression  */
  YYSYMBOL_return_expression = 82,         /* return_expression  */
  YYSYMBOL_expression = 83,                /* expression  */
  YYSYMBOL_84_11 = 84,                     /* $@11  */
  YYSYMBOL_85_12 = 85,                     /* $@12  */
  YYSYMBOL_86_13 = 86,                     /* $@13  */
  YYSYMBOL_named_expression = 87,          /* named_expression  */
  YYSYMBOL_required_eol = 88               /* required_eol  */
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

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   719

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  36
/* YYNRULES -- Number of rules.  */
#define YYNRULES  112
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  198

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   292


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
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    42,    52,     2,
      45,    46,    40,    38,    49,    39,     2,    41,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    50,     2,    51,    43,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    47,     2,    48,     2,     2,     2,     2,
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
      35,    36,    37
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   124,   124,   132,   134,   136,   138,   144,   145,   149,
     150,   151,   152,   155,   156,   157,   158,   159,   160,   162,
     163,   166,   168,   170,   179,   186,   197,   209,   211,   213,
     216,   221,   233,   248,   215,   268,   267,   283,   291,   282,
     307,   310,   309,   313,   314,   316,   322,   329,   331,   330,
     343,   341,   368,   369,   376,   377,   380,   381,   383,   386,
     388,   390,   394,   398,   400,   402,   406,   412,   413,   415,
     423,   430,   438,   457,   461,   464,   470,   485,   484,   514,
     513,   529,   528,   546,   554,   585,   592,   599,   606,   613,
     620,   627,   634,   643,   659,   665,   685,   704,   727,   734,
     741,   748,   754,   761,   763,   771,   773,   775,   777,   781,
     788,   789,   790
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
  "\"end of file\"", "error", "\"invalid token\"", "ENDOFLINE", "AND",
  "OR", "NOT", "STRING", "NAME", "NUMBER", "ASSIGN_OP", "REL_OP",
  "INCR_DECR", "Define", "Break", "Quit", "Length", "Return", "For", "If",
  "While", "Sqrt", "Else", "Scale", "Ibase", "Obase", "Auto", "Read",
  "Random", "Warranty", "Halt", "Last", "Continue", "Print", "Limits",
  "UNARY_MINUS", "HistoryVar", "Void", "'+'", "'-'", "'*'", "'/'", "'%'",
  "'^'", "';'", "'('", "')'", "'{'", "'}'", "','", "'['", "']'", "'&'",
  "$accept", "program", "input_item", "opt_newline", "semicolon_list",
  "statement_list", "statement_or_error", "statement", "$@1", "$@2", "@3",
  "$@4", "$@5", "$@6", "$@7", "$@8", "print_list", "print_element",
  "opt_else", "$@9", "function", "$@10", "opt_void", "opt_parameter_list",
  "opt_auto_define_list", "define_list", "opt_argument_list",
  "argument_list", "opt_expression", "return_expression", "expression",
  "$@11", "$@12", "$@13", "named_expression", "required_eol", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-144)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-16)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -144,   188,  -144,   392,   595,  -144,   -36,  -144,   484,   -31,
    -144,  -144,   -32,   595,  -144,   -11,  -144,   -10,    -7,  -144,
    -144,    -6,    -5,  -144,  -144,  -144,  -144,  -144,  -144,  -144,
     595,   595,   222,  -144,     2,  -144,  -144,  -144,   676,    14,
    -144,  -144,   131,   621,   595,   -27,  -144,  -144,  -144,    54,
     595,  -144,   676,    20,   595,    21,   595,   595,    13,    37,
     569,  -144,   425,   535,     1,  -144,  -144,   318,  -144,  -144,
     595,   595,   595,   595,   595,   595,   595,  -144,  -144,   -23,
      41,    36,   676,    39,    43,   436,   595,   445,   595,   485,
     494,  -144,  -144,  -144,  -144,    40,   676,  -144,   270,   535,
    -144,  -144,   595,   595,   -22,    51,    51,     4,     4,     4,
       4,   595,   105,  -144,   647,  -144,    23,  -144,    53,   676,
    -144,   676,  -144,  -144,   569,  -144,  -144,   131,   123,   -22,
    -144,   -20,   676,    45,    91,    94,    57,    55,  -144,   102,
      60,  -144,   352,    56,    58,    65,   102,    24,   595,  -144,
     535,   102,  -144,  -144,    67,    68,    69,    70,   115,   116,
      81,   109,   535,  -144,  -144,   132,    86,    88,    89,  -144,
    -144,  -144,  -144,  -144,     7,  -144,    92,    97,   595,   102,
    -144,    23,  -144,  -144,  -144,    99,   535,    12,   222,  -144,
    -144,  -144,  -144,     9,   102,  -144,   535,  -144
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     0,    24,   103,    93,     0,    52,
      25,    27,     0,    75,    30,     0,    37,     0,   107,   105,
     106,     0,     0,    21,    28,   109,    26,    41,    22,   108,
       0,     0,     0,     3,     0,    10,    19,     5,    23,    92,
       6,    20,    83,    67,     0,   103,   107,    96,    53,     0,
       0,    29,    76,     0,     0,     0,     0,     0,     0,     0,
       0,    91,     0,     0,     0,    14,     4,     0,    79,    81,
       0,     0,     0,     0,     0,     0,     0,    77,    97,   103,
       0,    68,    69,     0,     0,     0,    73,     0,     0,     0,
       0,   101,   102,    45,    42,    43,    46,    94,     0,    17,
      40,    11,     0,     0,    84,    85,    86,    87,    88,    89,
      90,     0,     0,    95,     0,   104,    54,    98,     0,    74,
      35,    38,    99,   100,     0,    16,    18,    80,    82,    78,
      70,   103,    71,    59,     0,     0,     0,    55,    31,     7,
       0,    44,     0,     0,     0,     0,     7,     0,    73,     8,
       0,     7,    72,    60,     0,     0,     0,    63,     0,     0,
       0,    47,     0,    61,    62,   110,     0,     0,     0,    32,
      48,    36,    39,   111,    56,    64,     0,     0,    73,     7,
     112,     0,    50,    65,    66,     0,     0,     0,     0,    33,
      49,    57,    58,     0,     7,    51,     0,    34
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -144,  -144,  -144,  -143,  -144,   -39,     0,    -3,  -144,  -144,
    -144,  -144,  -144,  -144,  -144,  -144,    27,  -144,  -144,  -144,
    -144,  -144,  -144,  -144,  -144,   -29,  -144,  -144,  -141,  -144,
      -2,  -144,  -144,  -144,   145,  -144
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,    33,   150,    34,    64,    65,    36,    53,   148,
     178,   194,   139,    55,   140,    60,    94,    95,   171,   179,
      37,   188,    49,   136,   182,   137,    80,    81,   118,    51,
      38,   111,   102,   103,    39,   174
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      41,    35,    42,   156,    98,    66,    48,   160,   162,    43,
     180,    52,    98,    50,    44,   191,    71,    72,    73,    74,
      75,    76,    43,    44,    77,    43,    78,   112,    61,    62,
     142,   133,   157,   181,    54,    56,   186,   185,    57,    58,
      59,    82,    83,    68,    69,    99,    67,    76,    85,   100,
      70,   196,    87,    99,    89,    90,   192,   195,    96,    91,
      41,   147,    84,   134,   158,    86,    88,   101,   104,   105,
     106,   107,   108,   109,   110,   135,   159,    71,    72,    73,
      74,    75,    76,    92,   119,   114,   121,   113,   116,   124,
     115,    73,    74,    75,    76,   143,   126,   138,   125,   144,
     127,   128,   145,   146,   147,   149,   151,   153,   154,   129,
      83,     4,   132,     6,     7,   155,   165,     8,   163,   164,
     166,    12,    96,   167,   168,   169,    17,    68,    18,    19,
      20,   170,    21,    22,    70,   173,    25,   175,   176,   177,
      83,    29,    70,   183,    30,   189,   119,   161,   184,   193,
      31,   141,   187,    47,     0,     0,   130,     0,     0,   172,
       0,    71,    72,    73,    74,    75,    76,     0,     0,    71,
      72,    73,    74,    75,    76,     0,   119,     0,     0,     0,
       0,     0,     0,   190,     0,     0,     0,     0,     2,     3,
       0,    -9,     0,   197,     4,     5,     6,     7,     0,     0,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
       0,    18,    19,    20,     0,    21,    22,    23,    24,    25,
      26,    27,    28,    63,    29,   -13,     0,    30,     4,     5,
       6,     7,    -9,    31,     8,    32,    10,    11,    12,    13,
      14,    15,    16,    17,     0,    18,    19,    20,     0,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,     0,
       0,    30,     0,     0,     0,     0,   -13,    31,     0,    32,
     -13,    63,     0,   -15,     0,     0,     4,     5,     6,     7,
       0,     0,     8,     0,    10,    11,    12,    13,    14,    15,
      16,    17,     0,    18,    19,    20,     0,    21,    22,    23,
      24,    25,    26,    27,    28,     0,    29,     0,     0,    30,
       0,     0,     0,     0,   -15,    31,     0,    32,   -15,    63,
       0,   -12,     0,     0,     4,     5,     6,     7,     0,     0,
       8,     0,    10,    11,    12,    13,    14,    15,    16,    17,
       0,    18,    19,    20,     0,    21,    22,    23,    24,    25,
      26,    27,    28,     0,    29,     0,     0,    30,     4,     0,
       6,     7,   -12,    31,     8,    32,     0,     0,    12,     0,
       0,     0,     0,    17,     0,    18,    19,    20,     0,    21,
      22,     0,     0,    25,     0,     0,     0,     0,    29,     0,
       0,    30,     0,     0,     0,    40,     0,    31,     4,     5,
       6,     7,     0,   152,     8,     0,    10,    11,    12,    13,
      14,    15,    16,    17,     0,    18,    19,    20,     0,    21,
      22,    23,    24,    25,    26,    27,    28,     0,    29,    68,
      69,    30,     0,     0,     0,     0,    70,    31,     0,    32,
      68,    69,     0,     0,     0,     0,     0,    70,     0,    68,
      69,     0,     0,     0,     0,     0,    70,     0,     0,     0,
       0,     0,     0,    71,    72,    73,    74,    75,    76,     0,
       0,    97,     0,     0,    71,    72,    73,    74,    75,    76,
       0,     0,   117,    71,    72,    73,    74,    75,    76,    68,
      69,   120,    45,     0,     0,     0,    70,     0,    68,    69,
       0,     0,     0,     0,     0,    70,     0,    46,    19,    20,
       0,     0,     0,     0,     0,    25,     0,     0,     0,     0,
      29,     0,     0,    71,    72,    73,    74,    75,    76,     0,
       0,   122,    71,    72,    73,    74,    75,    76,     0,     0,
     123,     4,     5,     6,     7,     0,     0,     8,     0,    10,
      11,    12,    13,    14,    15,    16,    17,     0,    18,    19,
      20,     0,    21,    22,    23,    24,    25,    26,    27,    28,
       0,    29,     0,     0,    30,     4,    93,     6,     7,     0,
      31,     8,    32,     0,     0,    12,     0,     0,     0,     0,
      17,     0,    18,    19,    20,     0,    21,    22,     0,     0,
      25,     4,     0,     6,     7,    29,     0,     8,    30,     0,
       0,    12,     0,     0,    31,     0,    17,     0,    18,    19,
      20,     0,    21,    22,     0,     0,    25,     4,     0,    79,
       7,    29,     0,     8,    30,     0,     0,    12,     0,     0,
      31,     0,    17,     0,    18,    19,    20,     0,    21,    22,
       0,     0,    25,     4,     0,   131,     7,    29,     0,     8,
      30,     0,     0,    12,     0,     0,    31,     0,    17,     0,
      18,    19,    20,     0,    21,    22,     0,     0,    25,     0,
      68,    69,     0,    29,     0,     0,    30,    70,     0,     0,
       0,     0,    31,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    71,    72,    73,    74,    75,    76
};

static const yytype_int16 yycheck[] =
{
       3,     1,     4,   146,     3,     3,    37,   148,   151,    45,
       3,    13,     3,    45,    50,     3,    38,    39,    40,    41,
      42,    43,    45,    50,    10,    45,    12,    50,    30,    31,
      50,     8,     8,    26,    45,    45,   179,   178,    45,    45,
      45,    43,    44,     4,     5,    44,    44,    43,    50,    48,
      11,   194,    54,    44,    56,    57,    44,    48,    60,    46,
      63,    49,     8,    40,    40,    45,    45,    67,    70,    71,
      72,    73,    74,    75,    76,    52,    52,    38,    39,    40,
      41,    42,    43,    46,    86,    49,    88,    46,    45,    49,
      51,    40,    41,    42,    43,    50,    99,    44,    98,     8,
     102,   103,     8,    46,    49,     3,    46,    51,    50,   111,
     112,     6,   114,     8,     9,    50,    47,    12,    51,    51,
      50,    16,   124,     8,     8,    44,    21,     4,    23,    24,
      25,    22,    27,    28,    11,     3,    31,    51,    50,    50,
     142,    36,    11,    51,    39,    46,   148,   150,    51,   188,
      45,   124,   181,     8,    -1,    -1,    51,    -1,    -1,   162,
      -1,    38,    39,    40,    41,    42,    43,    -1,    -1,    38,
      39,    40,    41,    42,    43,    -1,   178,    -1,    -1,    -1,
      -1,    -1,    -1,   186,    -1,    -1,    -1,    -1,     0,     1,
      -1,     3,    -1,   196,     6,     7,     8,     9,    -1,    -1,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,     1,    36,     3,    -1,    39,     6,     7,
       8,     9,    44,    45,    12,    47,    14,    15,    16,    17,
      18,    19,    20,    21,    -1,    23,    24,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    -1,
      -1,    39,    -1,    -1,    -1,    -1,    44,    45,    -1,    47,
      48,     1,    -1,     3,    -1,    -1,     6,     7,     8,     9,
      -1,    -1,    12,    -1,    14,    15,    16,    17,    18,    19,
      20,    21,    -1,    23,    24,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    -1,    -1,    39,
      -1,    -1,    -1,    -1,    44,    45,    -1,    47,    48,     1,
      -1,     3,    -1,    -1,     6,     7,     8,     9,    -1,    -1,
      12,    -1,    14,    15,    16,    17,    18,    19,    20,    21,
      -1,    23,    24,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    -1,    36,    -1,    -1,    39,     6,    -1,
       8,     9,    44,    45,    12,    47,    -1,    -1,    16,    -1,
      -1,    -1,    -1,    21,    -1,    23,    24,    25,    -1,    27,
      28,    -1,    -1,    31,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    39,    -1,    -1,    -1,     3,    -1,    45,     6,     7,
       8,     9,    -1,    51,    12,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    -1,    23,    24,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,     4,
       5,    39,    -1,    -1,    -1,    -1,    11,    45,    -1,    47,
       4,     5,    -1,    -1,    -1,    -1,    -1,    11,    -1,     4,
       5,    -1,    -1,    -1,    -1,    -1,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    43,    -1,
      -1,    46,    -1,    -1,    38,    39,    40,    41,    42,    43,
      -1,    -1,    46,    38,    39,    40,    41,    42,    43,     4,
       5,    46,     8,    -1,    -1,    -1,    11,    -1,     4,     5,
      -1,    -1,    -1,    -1,    -1,    11,    -1,    23,    24,    25,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    38,    39,    40,    41,    42,    43,    -1,
      -1,    46,    38,    39,    40,    41,    42,    43,    -1,    -1,
      46,     6,     7,     8,     9,    -1,    -1,    12,    -1,    14,
      15,    16,    17,    18,    19,    20,    21,    -1,    23,    24,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      -1,    36,    -1,    -1,    39,     6,     7,     8,     9,    -1,
      45,    12,    47,    -1,    -1,    16,    -1,    -1,    -1,    -1,
      21,    -1,    23,    24,    25,    -1,    27,    28,    -1,    -1,
      31,     6,    -1,     8,     9,    36,    -1,    12,    39,    -1,
      -1,    16,    -1,    -1,    45,    -1,    21,    -1,    23,    24,
      25,    -1,    27,    28,    -1,    -1,    31,     6,    -1,     8,
       9,    36,    -1,    12,    39,    -1,    -1,    16,    -1,    -1,
      45,    -1,    21,    -1,    23,    24,    25,    -1,    27,    28,
      -1,    -1,    31,     6,    -1,     8,     9,    36,    -1,    12,
      39,    -1,    -1,    16,    -1,    -1,    45,    -1,    21,    -1,
      23,    24,    25,    -1,    27,    28,    -1,    -1,    31,    -1,
       4,     5,    -1,    36,    -1,    -1,    39,    11,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    38,    39,    40,    41,    42,    43
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    54,     0,     1,     6,     7,     8,     9,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    23,    24,
      25,    27,    28,    29,    30,    31,    32,    33,    34,    36,
      39,    45,    47,    55,    57,    59,    60,    73,    83,    87,
       3,    60,    83,    45,    50,     8,    23,    87,    37,    75,
      45,    82,    83,    61,    45,    66,    45,    45,    45,    45,
      68,    83,    83,     1,    58,    59,     3,    44,     4,     5,
      11,    38,    39,    40,    41,    42,    43,    10,    12,     8,
      79,    80,    83,    83,     8,    83,    45,    83,    45,    83,
      83,    46,    46,     7,    69,    70,    83,    46,     3,    44,
      48,    59,    85,    86,    83,    83,    83,    83,    83,    83,
      83,    84,    50,    46,    49,    51,    45,    46,    81,    83,
      46,    83,    46,    46,    49,    59,    60,    83,    83,    83,
      51,     8,    83,     8,    40,    52,    76,    78,    44,    65,
      67,    69,    50,    50,     8,     8,    46,    49,    62,     3,
      56,    46,    51,    51,    50,    50,    56,     8,    40,    52,
      81,    60,    56,    51,    51,    47,    50,     8,     8,    44,
      22,    71,    60,     3,    88,    51,    50,    50,    63,    72,
       3,    26,    77,    51,    51,    81,    56,    78,    74,    46,
      60,     3,    44,    58,    64,    48,    56,    60
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    53,    54,    54,    55,    55,    55,    56,    56,    57,
      57,    57,    57,    58,    58,    58,    58,    58,    58,    59,
      59,    60,    60,    60,    60,    60,    60,    60,    60,    60,
      61,    62,    63,    64,    60,    65,    60,    66,    67,    60,
      60,    68,    60,    69,    69,    70,    70,    71,    72,    71,
      74,    73,    75,    75,    76,    76,    77,    77,    77,    78,
      78,    78,    78,    78,    78,    78,    78,    79,    79,    80,
      80,    80,    80,    81,    81,    82,    82,    84,    83,    85,
      83,    86,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    87,    87,    87,    87,    87,    87,    87,
      88,    88,    88
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     2,     1,     2,     0,     1,     0,
       1,     3,     2,     0,     1,     2,     3,     2,     3,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       0,     0,     0,     0,    14,     0,     8,     0,     0,     8,
       3,     0,     3,     1,     3,     1,     1,     0,     0,     4,
       0,    13,     0,     1,     0,     1,     0,     3,     3,     1,
       3,     4,     4,     3,     5,     6,     6,     0,     1,     1,
       3,     3,     5,     0,     1,     0,     1,     0,     4,     0,
       4,     0,     4,     2,     3,     3,     3,     3,     3,     3,
       3,     2,     1,     1,     3,     4,     2,     2,     4,     4,
       4,     3,     3,     1,     4,     1,     1,     1,     1,     1,
       0,     1,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
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
  case 2: /* program: %empty  */
#line 124 "bc.y"
                            {
			      (yyval.i_value) = 0;
			      if (interactive && !quiet)
				{
				  show_bc_version ();
				  welcome ();
				}
			    }
#line 1539 "bc.c"
    break;

  case 4: /* input_item: semicolon_list ENDOFLINE  */
#line 135 "bc.y"
                            { run_code (); }
#line 1545 "bc.c"
    break;

  case 5: /* input_item: function  */
#line 137 "bc.y"
                            { run_code (); }
#line 1551 "bc.c"
    break;

  case 6: /* input_item: error ENDOFLINE  */
#line 139 "bc.y"
                            {
			      yyerrok;
			      init_gen ();
			    }
#line 1560 "bc.c"
    break;

  case 8: /* opt_newline: ENDOFLINE  */
#line 146 "bc.y"
                            { ct_warn ("newline not allowed"); }
#line 1566 "bc.c"
    break;

  case 9: /* semicolon_list: %empty  */
#line 149 "bc.y"
                            { (yyval.i_value) = 0; }
#line 1572 "bc.c"
    break;

  case 13: /* statement_list: %empty  */
#line 155 "bc.y"
                            { (yyval.i_value) = 0; }
#line 1578 "bc.c"
    break;

  case 20: /* statement_or_error: error statement  */
#line 164 "bc.y"
                            { (yyval.i_value) = (yyvsp[0].i_value); }
#line 1584 "bc.c"
    break;

  case 21: /* statement: Warranty  */
#line 167 "bc.y"
                            { warranty (""); }
#line 1590 "bc.c"
    break;

  case 22: /* statement: Limits  */
#line 169 "bc.y"
                            { limits (); }
#line 1596 "bc.c"
    break;

  case 23: /* statement: expression  */
#line 171 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_COMP)
				ct_warn ("comparison in expression");
			      if ((yyvsp[0].i_value) & EX_REG)
				generate ("W");
			      else 
				generate ("p");
			    }
#line 1609 "bc.c"
    break;

  case 24: /* statement: STRING  */
#line 180 "bc.y"
                            {
			      (yyval.i_value) = 0;
			      generate ("w");
			      generate ((yyvsp[0].s_value));
			      free ((yyvsp[0].s_value));
			    }
#line 1620 "bc.c"
    break;

  case 25: /* statement: Break  */
#line 187 "bc.y"
                            {
			      if (break_label == 0)
				yyerror ("Break outside a for/while");
			      else
				{
				  snprintf (genstr, genlen, "J%1d:",
				  	    break_label);
				  generate (genstr);
				}
			    }
#line 1635 "bc.c"
    break;

  case 26: /* statement: Continue  */
#line 198 "bc.y"
                            {
			      ct_warn ("Continue statement");
			      if (continue_label == 0)
				yyerror ("Continue outside a for");
			      else
				{
				  snprintf (genstr, genlen, "J%1d:", 
					    continue_label);
				  generate (genstr);
				}
			    }
#line 1651 "bc.c"
    break;

  case 27: /* statement: Quit  */
#line 210 "bc.y"
                            { bc_exit (0); }
#line 1657 "bc.c"
    break;

  case 28: /* statement: Halt  */
#line 212 "bc.y"
                            { generate ("h"); }
#line 1663 "bc.c"
    break;

  case 29: /* statement: Return return_expression  */
#line 214 "bc.y"
                            { generate ("R"); }
#line 1669 "bc.c"
    break;

  case 30: /* $@1: %empty  */
#line 216 "bc.y"
                            {
			      (yyvsp[0].i_value) = break_label; 
			      break_label = next_label++;
			    }
#line 1678 "bc.c"
    break;

  case 31: /* $@2: %empty  */
#line 221 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_COMP)
				ct_warn ("Comparison in first for expression");
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("first expression is void");
			      if (!((yyvsp[-1].i_value) & EX_EMPTY))
				generate ("p");
			      (yyvsp[-1].i_value) = next_label++;
			      snprintf (genstr, genlen, "N%1d:", (yyvsp[-1].i_value));
			      generate (genstr);
			    }
#line 1694 "bc.c"
    break;

  case 32: /* @3: %empty  */
#line 233 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("second expression is void");
			      if ((yyvsp[-1].i_value) & EX_EMPTY ) generate ("1");
			      (yyvsp[-1].i_value) = next_label++;
			      snprintf (genstr, genlen, "B%1d:J%1d:", (yyvsp[-1].i_value),
			      		break_label);
			      generate (genstr);
			      (yyval.i_value) = continue_label;
			      continue_label = next_label++;
			      snprintf (genstr, genlen, "N%1d:", 
			      		continue_label);
			      generate (genstr);
			    }
#line 1713 "bc.c"
    break;

  case 33: /* $@4: %empty  */
#line 248 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_COMP)
				ct_warn ("Comparison in third for expression");
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("third expression is void");
			      if ((yyvsp[-1].i_value) & EX_EMPTY)
				snprintf (genstr, genlen, "J%1d:N%1d:", (yyvsp[-7].i_value), (yyvsp[-4].i_value));
			      else
				snprintf (genstr, genlen, "pJ%1d:N%1d:", (yyvsp[-7].i_value), (yyvsp[-4].i_value));
			      generate (genstr);
			    }
#line 1729 "bc.c"
    break;

  case 34: /* statement: For $@1 '(' opt_expression ';' $@2 opt_expression ';' @3 opt_expression ')' $@4 opt_newline statement  */
#line 260 "bc.y"
                            {
			      snprintf (genstr, genlen, "J%1d:N%1d:",
				       continue_label, break_label);
			      generate (genstr);
			      break_label = (yyvsp[-13].i_value);
			      continue_label = (yyvsp[-5].i_value);
			    }
#line 1741 "bc.c"
    break;

  case 35: /* $@5: %empty  */
#line 268 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("void expression");
			      (yyvsp[-1].i_value) = if_label;
			      if_label = next_label++;
			      snprintf (genstr, genlen, "Z%1d:", if_label);
			      generate (genstr);
			    }
#line 1754 "bc.c"
    break;

  case 36: /* statement: If '(' expression ')' $@5 opt_newline statement opt_else  */
#line 277 "bc.y"
                            {
			      snprintf (genstr, genlen, "N%1d:", if_label); 
			      generate (genstr);
			      if_label = (yyvsp[-5].i_value);
			    }
#line 1764 "bc.c"
    break;

  case 37: /* $@6: %empty  */
#line 283 "bc.y"
                            {
			      (yyvsp[0].i_value) = continue_label;
			      continue_label = next_label++;
			      snprintf (genstr, genlen, "N%1d:", 
					continue_label);
			      generate (genstr);
			    }
#line 1776 "bc.c"
    break;

  case 38: /* $@7: %empty  */
#line 291 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror ("void expression");
			      (yyvsp[0].i_value) = break_label; 
			      break_label = next_label++;
			      snprintf (genstr, genlen, "Z%1d:", break_label);
			      generate (genstr);
			    }
#line 1789 "bc.c"
    break;

  case 39: /* statement: While $@6 '(' expression $@7 ')' opt_newline statement  */
#line 300 "bc.y"
                            {
			      snprintf (genstr, genlen, "J%1d:N%1d:", 
					continue_label, break_label);
			      generate (genstr);
			      break_label = (yyvsp[-4].i_value);
			      continue_label = (yyvsp[-7].i_value);
			    }
#line 1801 "bc.c"
    break;

  case 40: /* statement: '{' statement_list '}'  */
#line 308 "bc.y"
                            { (yyval.i_value) = 0; }
#line 1807 "bc.c"
    break;

  case 41: /* $@8: %empty  */
#line 310 "bc.y"
                            {  ct_warn ("print statement"); }
#line 1813 "bc.c"
    break;

  case 45: /* print_element: STRING  */
#line 317 "bc.y"
                            {
			      generate ("O");
			      generate ((yyvsp[0].s_value));
			      free ((yyvsp[0].s_value));
			    }
#line 1823 "bc.c"
    break;

  case 46: /* print_element: expression  */
#line 323 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror ("void expression in print");
			      generate ("P");
			    }
#line 1833 "bc.c"
    break;

  case 48: /* $@9: %empty  */
#line 331 "bc.y"
                            {
			      ct_warn ("else clause in if statement");
			      (yyvsp[0].i_value) = next_label++;
			      snprintf (genstr, genlen, "J%d:N%1d:", (yyvsp[0].i_value),
					if_label); 
			      generate (genstr);
			      if_label = (yyvsp[0].i_value);
			    }
#line 1846 "bc.c"
    break;

  case 50: /* $@10: %empty  */
#line 343 "bc.y"
                            { char *params, *autos;
			      /* Check auto list against parameter list? */
			      check_params ((yyvsp[-5].a_value),(yyvsp[0].a_value));
			      params = arg_str ((yyvsp[-5].a_value));
			      autos  = arg_str ((yyvsp[0].a_value));
			      set_genstr_size (30 + strlen (params)
					       + strlen (autos));
			      cur_func = lookup((yyvsp[-7].s_value),FUNCTDEF);
			      snprintf (genstr, genlen, "F%d,%s.%s[", cur_func,
					params, autos); 
			      generate (genstr);
			      functions[cur_func].f_void = (yyvsp[-8].i_value);
			      free_args ((yyvsp[-5].a_value));
			      free_args ((yyvsp[0].a_value));
			      (yyvsp[-9].i_value) = next_label;
			      next_label = 1;
			    }
#line 1868 "bc.c"
    break;

  case 51: /* function: Define opt_void NAME '(' opt_parameter_list ')' opt_newline '{' required_eol opt_auto_define_list $@10 statement_list '}'  */
#line 361 "bc.y"
                            {
			      generate ("0R]");
			      next_label = (yyvsp[-12].i_value);
			      cur_func = -1;
			    }
#line 1878 "bc.c"
    break;

  case 52: /* opt_void: %empty  */
#line 368 "bc.y"
                            { (yyval.i_value) = 0; }
#line 1884 "bc.c"
    break;

  case 53: /* opt_void: Void  */
#line 370 "bc.y"
                            {
			      (yyval.i_value) = 1;
			      ct_warn ("void functions");
			    }
#line 1893 "bc.c"
    break;

  case 54: /* opt_parameter_list: %empty  */
#line 376 "bc.y"
                            { (yyval.a_value) = NULL; }
#line 1899 "bc.c"
    break;

  case 56: /* opt_auto_define_list: %empty  */
#line 380 "bc.y"
                            { (yyval.a_value) = NULL; }
#line 1905 "bc.c"
    break;

  case 57: /* opt_auto_define_list: Auto define_list ENDOFLINE  */
#line 382 "bc.y"
                            { (yyval.a_value) = (yyvsp[-1].a_value); }
#line 1911 "bc.c"
    break;

  case 58: /* opt_auto_define_list: Auto define_list ';'  */
#line 384 "bc.y"
                            { (yyval.a_value) = (yyvsp[-1].a_value); }
#line 1917 "bc.c"
    break;

  case 59: /* define_list: NAME  */
#line 387 "bc.y"
                            { (yyval.a_value) = nextarg (NULL, lookup ((yyvsp[0].s_value),SIMPLE), FALSE);}
#line 1923 "bc.c"
    break;

  case 60: /* define_list: NAME '[' ']'  */
#line 389 "bc.y"
                            { (yyval.a_value) = nextarg (NULL, lookup ((yyvsp[-2].s_value),ARRAY), FALSE); }
#line 1929 "bc.c"
    break;

  case 61: /* define_list: '*' NAME '[' ']'  */
#line 391 "bc.y"
                            { (yyval.a_value) = nextarg (NULL, lookup ((yyvsp[-2].s_value),ARRAY), TRUE);
			      ct_warn ("Call by variable arrays");
			    }
#line 1937 "bc.c"
    break;

  case 62: /* define_list: '&' NAME '[' ']'  */
#line 395 "bc.y"
                            { (yyval.a_value) = nextarg (NULL, lookup ((yyvsp[-2].s_value),ARRAY), TRUE);
			      ct_warn ("Call by variable arrays");
			    }
#line 1945 "bc.c"
    break;

  case 63: /* define_list: define_list ',' NAME  */
#line 399 "bc.y"
                            { (yyval.a_value) = nextarg ((yyvsp[-2].a_value), lookup ((yyvsp[0].s_value),SIMPLE), FALSE); }
#line 1951 "bc.c"
    break;

  case 64: /* define_list: define_list ',' NAME '[' ']'  */
#line 401 "bc.y"
                            { (yyval.a_value) = nextarg ((yyvsp[-4].a_value), lookup ((yyvsp[-2].s_value),ARRAY), FALSE); }
#line 1957 "bc.c"
    break;

  case 65: /* define_list: define_list ',' '*' NAME '[' ']'  */
#line 403 "bc.y"
                            { (yyval.a_value) = nextarg ((yyvsp[-5].a_value), lookup ((yyvsp[-2].s_value),ARRAY), TRUE);
			      ct_warn ("Call by variable arrays");
			    }
#line 1965 "bc.c"
    break;

  case 66: /* define_list: define_list ',' '&' NAME '[' ']'  */
#line 407 "bc.y"
                            { (yyval.a_value) = nextarg ((yyvsp[-5].a_value), lookup ((yyvsp[-2].s_value),ARRAY), TRUE);
			      ct_warn ("Call by variable arrays");
			    }
#line 1973 "bc.c"
    break;

  case 67: /* opt_argument_list: %empty  */
#line 412 "bc.y"
                            { (yyval.a_value) = NULL; }
#line 1979 "bc.c"
    break;

  case 69: /* argument_list: expression  */
#line 416 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_COMP)
				ct_warn ("comparison in argument");
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror ("void argument");
			      (yyval.a_value) = nextarg (NULL,0,FALSE);
			    }
#line 1991 "bc.c"
    break;

  case 70: /* argument_list: NAME '[' ']'  */
#line 424 "bc.y"
                            {
			      snprintf (genstr, genlen, "K%d:",
					-lookup ((yyvsp[-2].s_value),ARRAY));
			      generate (genstr);
			      (yyval.a_value) = nextarg (NULL,1,FALSE);
			    }
#line 2002 "bc.c"
    break;

  case 71: /* argument_list: argument_list ',' expression  */
#line 431 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_COMP)
				ct_warn ("comparison in argument");
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror ("void argument");
			      (yyval.a_value) = nextarg ((yyvsp[-2].a_value),0,FALSE);
			    }
#line 2014 "bc.c"
    break;

  case 72: /* argument_list: argument_list ',' NAME '[' ']'  */
#line 439 "bc.y"
                            {
			      snprintf (genstr, genlen, "K%d:", 
					-lookup ((yyvsp[-2].s_value),ARRAY));
			      generate (genstr);
			      (yyval.a_value) = nextarg ((yyvsp[-4].a_value),1,FALSE);
			    }
#line 2025 "bc.c"
    break;

  case 73: /* opt_expression: %empty  */
#line 457 "bc.y"
                            {
			      (yyval.i_value) = EX_EMPTY;
			      ct_warn ("Missing expression in for statement");
			    }
#line 2034 "bc.c"
    break;

  case 75: /* return_expression: %empty  */
#line 464 "bc.y"
                            {
			      (yyval.i_value) = 0;
			      generate ("0");
			      if (cur_func == -1)
				yyerror("Return outside of a function.");
			    }
#line 2045 "bc.c"
    break;

  case 76: /* return_expression: expression  */
#line 471 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_COMP)
				ct_warn ("comparison in return expression");
			      if (!((yyvsp[0].i_value) & EX_PAREN))
				ct_warn ("return expression requires parenthesis");
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror("return requires non-void expression");
			      if (cur_func == -1)
				yyerror("Return outside of a function.");
			      else if (functions[cur_func].f_void)
				yyerror("Return expression in a void function.");
			    }
#line 2062 "bc.c"
    break;

  case 77: /* $@11: %empty  */
#line 485 "bc.y"
                            {
			      if ((yyvsp[0].c_value) != '=')
				{
				  if ((yyvsp[-1].i_value) < 0)
				    snprintf (genstr, genlen, "DL%d:", -(yyvsp[-1].i_value));
				  else
				    snprintf (genstr, genlen, "l%d:", (yyvsp[-1].i_value));
				  generate (genstr);
				}
			    }
#line 2077 "bc.c"
    break;

  case 78: /* expression: named_expression ASSIGN_OP $@11 expression  */
#line 496 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_ASSGN)
				ct_warn("comparison in assignment");
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror("Assignment of a void expression");
			      if ((yyvsp[-2].c_value) != '=')
				{
				  snprintf (genstr, genlen, "%c", (yyvsp[-2].c_value));
				  generate (genstr);
				}
			      if ((yyvsp[-3].i_value) < 0)
				snprintf (genstr, genlen, "S%d:", -(yyvsp[-3].i_value));
			      else
				snprintf (genstr, genlen, "s%d:", (yyvsp[-3].i_value));
			      generate (genstr);
			      (yyval.i_value) = EX_ASSGN;
			    }
#line 2099 "bc.c"
    break;

  case 79: /* $@12: %empty  */
#line 514 "bc.y"
                            {
			      ct_warn("&& operator");
			      (yyvsp[0].i_value) = next_label++;
			      snprintf (genstr, genlen, "DZ%d:p", (yyvsp[0].i_value));
			      generate (genstr);
			    }
#line 2110 "bc.c"
    break;

  case 80: /* expression: expression AND $@12 expression  */
#line 521 "bc.y"
                            {
			      if (((yyvsp[-3].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with &&");
			      snprintf (genstr, genlen, "DZ%d:p1N%d:", (yyvsp[-2].i_value), (yyvsp[-2].i_value));
			      generate (genstr);
			      (yyval.i_value) = ((yyvsp[-3].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2122 "bc.c"
    break;

  case 81: /* $@13: %empty  */
#line 529 "bc.y"
                            {
			      ct_warn("|| operator");
			      (yyvsp[0].i_value) = next_label++;
			      snprintf (genstr, genlen, "B%d:", (yyvsp[0].i_value));
			      generate (genstr);
			    }
#line 2133 "bc.c"
    break;

  case 82: /* expression: expression OR $@13 expression  */
#line 536 "bc.y"
                            {
			      int tmplab;
			      if (((yyvsp[-3].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with ||");
			      tmplab = next_label++;
			      snprintf (genstr, genlen, "B%d:0J%d:N%d:1N%d:",
				       (yyvsp[-2].i_value), tmplab, (yyvsp[-2].i_value), tmplab);
			      generate (genstr);
			      (yyval.i_value) = ((yyvsp[-3].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2148 "bc.c"
    break;

  case 83: /* expression: NOT expression  */
#line 547 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror ("void expression with !");
			      (yyval.i_value) = (yyvsp[0].i_value) & ~EX_PAREN;
			      ct_warn("! operator");
			      generate ("!");
			    }
#line 2160 "bc.c"
    break;

  case 84: /* expression: expression REL_OP expression  */
#line 555 "bc.y"
                            {
			      if (((yyvsp[-2].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with comparison");
			      (yyval.i_value) = EX_REG | EX_COMP;
			      switch (*((yyvsp[-1].s_value)))
				{
				case '=':
				  generate ("=");
				  break;

				case '!':
				  generate ("#");
				  break;

				case '<':
				  if ((yyvsp[-1].s_value)[1] == '=')
				    generate ("{");
				  else
				    generate ("<");
				  break;

				case '>':
				  if ((yyvsp[-1].s_value)[1] == '=')
				    generate ("}");
				  else
				    generate (">");
				  break;
				}
                              free((yyvsp[-1].s_value));
			    }
#line 2195 "bc.c"
    break;

  case 85: /* expression: expression '+' expression  */
#line 586 "bc.y"
                            {
			      if (((yyvsp[-2].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with +");
			      generate ("+");
			      (yyval.i_value) = ((yyvsp[-2].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2206 "bc.c"
    break;

  case 86: /* expression: expression '-' expression  */
#line 593 "bc.y"
                            {
			      if (((yyvsp[-2].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with -");
			      generate ("-");
			      (yyval.i_value) = ((yyvsp[-2].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2217 "bc.c"
    break;

  case 87: /* expression: expression '*' expression  */
#line 600 "bc.y"
                            {
			      if (((yyvsp[-2].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with *");
			      generate ("*");
			      (yyval.i_value) = ((yyvsp[-2].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2228 "bc.c"
    break;

  case 88: /* expression: expression '/' expression  */
#line 607 "bc.y"
                            {
			      if (((yyvsp[-2].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with /");
			      generate ("/");
			      (yyval.i_value) = ((yyvsp[-2].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2239 "bc.c"
    break;

  case 89: /* expression: expression '%' expression  */
#line 614 "bc.y"
                            {
			      if (((yyvsp[-2].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with %");
			      generate ("%");
			      (yyval.i_value) = ((yyvsp[-2].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2250 "bc.c"
    break;

  case 90: /* expression: expression '^' expression  */
#line 621 "bc.y"
                            {
			      if (((yyvsp[-2].i_value) & EX_VOID) || ((yyvsp[0].i_value) & EX_VOID))
				yyerror ("void expression with ^");
			      generate ("^");
			      (yyval.i_value) = ((yyvsp[-2].i_value) | (yyvsp[0].i_value)) & ~EX_PAREN;
			    }
#line 2261 "bc.c"
    break;

  case 91: /* expression: '-' expression  */
#line 628 "bc.y"
                            {
			      if ((yyvsp[0].i_value) & EX_VOID)
				yyerror ("void expression with unary -");
			      generate ("n");
			      (yyval.i_value) = (yyvsp[0].i_value) & ~EX_PAREN;
			    }
#line 2272 "bc.c"
    break;

  case 92: /* expression: named_expression  */
#line 635 "bc.y"
                            {
			      (yyval.i_value) = EX_REG;
			      if ((yyvsp[0].i_value) < 0)
				snprintf (genstr, genlen, "L%d:", -(yyvsp[0].i_value));
			      else
				snprintf (genstr, genlen, "l%d:", (yyvsp[0].i_value));
			      generate (genstr);
			    }
#line 2285 "bc.c"
    break;

  case 93: /* expression: NUMBER  */
#line 644 "bc.y"
                            {
			      int len = strlen((yyvsp[0].s_value));
			      (yyval.i_value) = EX_REG;
			      if (len == 1 && *(yyvsp[0].s_value) == '0')
				generate ("0");
			      else if (len == 1 && *(yyvsp[0].s_value) == '1')
				generate ("1");
			      else
				{
				  generate ("K");
				  generate ((yyvsp[0].s_value));
				  generate (":");
				}
			      free ((yyvsp[0].s_value));
			    }
#line 2305 "bc.c"
    break;

  case 94: /* expression: '(' expression ')'  */
#line 660 "bc.y"
                            { 
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("void expression in parenthesis");
			      (yyval.i_value) = (yyvsp[-1].i_value) | EX_REG | EX_PAREN;
			    }
#line 2315 "bc.c"
    break;

  case 95: /* expression: NAME '(' opt_argument_list ')'  */
#line 666 "bc.y"
                            { int fn;
			      fn = lookup ((yyvsp[-3].s_value),FUNCT);
			      if (functions[fn].f_void)
				(yyval.i_value) = EX_VOID;
			      else
				(yyval.i_value) = EX_REG;
			      if ((yyvsp[-1].a_value) != NULL)
				{ char *params = call_str ((yyvsp[-1].a_value));
				  set_genstr_size (20 + strlen (params));
				  snprintf (genstr, genlen, "C%d,%s:", fn,
				  	    params);
				  free_args ((yyvsp[-1].a_value));
				}
			      else
				{
				  snprintf (genstr, genlen, "C%d:", fn);
				}
			      generate (genstr);
			    }
#line 2339 "bc.c"
    break;

  case 96: /* expression: INCR_DECR named_expression  */
#line 686 "bc.y"
                            {
			      (yyval.i_value) = EX_REG;
			      if ((yyvsp[0].i_value) < 0)
				{
				  if ((yyvsp[-1].c_value) == '+')
				    snprintf (genstr, genlen, "DA%d:L%d:", -(yyvsp[0].i_value), -(yyvsp[0].i_value));
				  else
				    snprintf (genstr, genlen, "DM%d:L%d:", -(yyvsp[0].i_value), -(yyvsp[0].i_value));
				}
			      else
				{
				  if ((yyvsp[-1].c_value) == '+')
				    snprintf (genstr, genlen, "i%d:l%d:", (yyvsp[0].i_value), (yyvsp[0].i_value));
				  else
				    snprintf (genstr, genlen, "d%d:l%d:", (yyvsp[0].i_value), (yyvsp[0].i_value));
				}
			      generate (genstr);
			    }
#line 2362 "bc.c"
    break;

  case 97: /* expression: named_expression INCR_DECR  */
#line 705 "bc.y"
                            {
			      (yyval.i_value) = EX_REG;
			      if ((yyvsp[-1].i_value) < 0)
				{
				  snprintf (genstr, genlen, "DL%d:x", -(yyvsp[-1].i_value));
				  generate (genstr); 
				  if ((yyvsp[0].c_value) == '+')
				    snprintf (genstr, genlen, "A%d:", -(yyvsp[-1].i_value));
				  else
				      snprintf (genstr, genlen, "M%d:", -(yyvsp[-1].i_value));
				}
			      else
				{
				  snprintf (genstr, genlen, "l%d:", (yyvsp[-1].i_value));
				  generate (genstr);
				  if ((yyvsp[0].c_value) == '+')
				    snprintf (genstr, genlen, "i%d:", (yyvsp[-1].i_value));
				  else
				    snprintf (genstr, genlen, "d%d:", (yyvsp[-1].i_value));
				}
			      generate (genstr);
			    }
#line 2389 "bc.c"
    break;

  case 98: /* expression: Length '(' expression ')'  */
#line 728 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("void expression in length()");
			      generate ("cL");
			      (yyval.i_value) = EX_REG;
			    }
#line 2400 "bc.c"
    break;

  case 99: /* expression: Sqrt '(' expression ')'  */
#line 735 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("void expression in sqrt()");
			      generate ("cR");
			      (yyval.i_value) = EX_REG;
			    }
#line 2411 "bc.c"
    break;

  case 100: /* expression: Scale '(' expression ')'  */
#line 742 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror ("void expression in scale()");
			      generate ("cS");
			      (yyval.i_value) = EX_REG;
			    }
#line 2422 "bc.c"
    break;

  case 101: /* expression: Read '(' ')'  */
#line 749 "bc.y"
                            {
			      ct_warn ("read function");
			      generate ("cI");
			      (yyval.i_value) = EX_REG;
			    }
#line 2432 "bc.c"
    break;

  case 102: /* expression: Random '(' ')'  */
#line 755 "bc.y"
                            {
			      ct_warn ("random function");
			      generate ("cX");
			      (yyval.i_value) = EX_REG;
			    }
#line 2442 "bc.c"
    break;

  case 103: /* named_expression: NAME  */
#line 762 "bc.y"
                            { (yyval.i_value) = lookup((yyvsp[0].s_value),SIMPLE); }
#line 2448 "bc.c"
    break;

  case 104: /* named_expression: NAME '[' expression ']'  */
#line 764 "bc.y"
                            {
			      if ((yyvsp[-1].i_value) & EX_VOID)
				yyerror("void expression as subscript");
			      if ((yyvsp[-1].i_value) & EX_COMP)
				ct_warn("comparison in subscript");
			      (yyval.i_value) = lookup((yyvsp[-3].s_value),ARRAY);
			    }
#line 2460 "bc.c"
    break;

  case 105: /* named_expression: Ibase  */
#line 772 "bc.y"
                            { (yyval.i_value) = 0; }
#line 2466 "bc.c"
    break;

  case 106: /* named_expression: Obase  */
#line 774 "bc.y"
                            { (yyval.i_value) = 1; }
#line 2472 "bc.c"
    break;

  case 107: /* named_expression: Scale  */
#line 776 "bc.y"
                            { (yyval.i_value) = 2; }
#line 2478 "bc.c"
    break;

  case 108: /* named_expression: HistoryVar  */
#line 778 "bc.y"
                            { (yyval.i_value) = 3;
			      ct_warn ("History variable");
			    }
#line 2486 "bc.c"
    break;

  case 109: /* named_expression: Last  */
#line 782 "bc.y"
                            { (yyval.i_value) = 4;
			      ct_warn ("Last variable");
			    }
#line 2494 "bc.c"
    break;

  case 110: /* required_eol: %empty  */
#line 788 "bc.y"
                          { ct_warn ("End of line required"); }
#line 2500 "bc.c"
    break;

  case 112: /* required_eol: required_eol ENDOFLINE  */
#line 791 "bc.y"
                          { ct_warn ("Too many end of lines"); }
#line 2506 "bc.c"
    break;


#line 2510 "bc.c"

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
  ++yynerrs;

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
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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

#line 794 "bc.y"

