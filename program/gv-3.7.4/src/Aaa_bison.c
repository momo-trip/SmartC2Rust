
/*  A Bison parser, made from layout_bison.y with Bison version GNU Bison version 1.24
  */

#include "ac_config.h"

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse layout_parse
#define yylex layout_lex
#define yyerror layout_error
#define yylval layout_lval
#define yychar layout_char
#define yydebug layout_debug
#define yynerrs layout_nerrs
#define	OC	258
#define	CC	259
#define	OA	260
#define	CA	261
#define	OP	262
#define	CP	263
#define	NAME	264
#define	NUMBER	265
#define	INFINITY	266
#define	VERTICAL	267
#define	HORIZONTAL	268
#define	EQUAL	269
#define	DOLLAR	270
#define	PLUS	271
#define	MINUS	272
#define	TIMES	273
#define	DIVIDE	274
#define	PERCENTOF	275
#define	PERCENT	276
#define	WIDTH	277
#define	HEIGHT	278
#define	UMINUS	279
#define	UPLUS	280

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>
#include <stdlib.h>
#include "Aaa_intern.h"

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		68
#define	YYFLAG		-32768
#define	YYNTBASE	26

#define YYTRANSLATE(x) ((unsigned)(x) <= 280 ? yytranslate[x] : 39)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     8,    12,    14,    19,    22,    24,    32,
    33,    38,    39,    42,    43,    46,    47,    50,    52,    54,
    57,    60,    62,    65,    68,    72,    75,    77,    80,    84,
    88,    92,    96,   100,   103,   106,   108,   110
};

static const short yyrhs[] = {    28,
     0,     9,    30,     0,    35,    31,     0,     9,    14,    35,
     0,    28,     0,    38,     3,    29,     4,     0,    27,    29,
     0,    27,     0,     5,    32,    33,    18,    32,    33,     6,
     0,     0,     5,    32,    33,     6,     0,     0,    16,    34,
     0,     0,    17,    34,     0,     0,    36,    11,     0,    36,
     0,    11,     0,    17,    36,     0,    16,    36,     0,    36,
     0,    22,     9,     0,    23,     9,     0,     7,    37,     8,
     0,    36,    21,     0,    10,     0,    15,     9,     0,    37,
    16,    37,     0,    37,    17,    37,     0,    37,    18,    37,
     0,    37,    19,    37,     0,    37,    20,    37,     0,    17,
    37,     0,    16,    37,     0,    36,     0,    12,     0,    13,
     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    29,    32,    43,    54,    68,    73,   105,   110,   113,   124,
   136,   147,   159,   161,   164,   166,   169,   171,   173,   176,
   184,   186,   188,   194,   200,   202,   209,   215,   222,   231,
   233,   235,   237,   239,   247,   249,   251,   253
};

static const char * const yytname[] = {   "$","error","$undefined.","OC","CC",
"OA","CA","OP","CP","NAME","NUMBER","INFINITY","VERTICAL","HORIZONTAL","EQUAL",
"DOLLAR","PLUS","MINUS","TIMES","DIVIDE","PERCENTOF","PERCENT","WIDTH","HEIGHT",
"UMINUS","UPLUS","layout","box","compositebox","boxes","bothparams","oneparams",
"opStretch","opShrink","glue","signedExpr","simpleExpr","expr","orientation",
""
};
#endif

static const short yyr1[] = {     0,
    26,    27,    27,    27,    27,    28,    29,    29,    30,    30,
    31,    31,    32,    32,    33,    33,    34,    34,    34,    35,
    35,    35,    36,    36,    36,    36,    36,    36,    37,    37,
    37,    37,    37,    37,    37,    37,    38,    38
};

static const short yyr2[] = {     0,
     1,     2,     2,     3,     1,     4,     2,     1,     7,     0,
     4,     0,     2,     0,     2,     0,     2,     1,     1,     2,
     2,     1,     2,     2,     3,     2,     1,     2,     3,     3,
     3,     3,     3,     2,     2,     1,     1,     1
};

static const short yydefact[] = {     0,
    37,    38,     1,     0,     0,     0,    10,    27,     0,     0,
     0,     0,     0,     8,     5,     0,    12,    22,     0,     0,
    36,     0,    14,     0,     2,    28,    21,    20,    23,    24,
     7,     6,    14,     3,    26,    35,    34,    25,     0,     0,
     0,     0,     0,     0,    16,     4,    16,    29,    30,    31,
    32,    33,    19,    13,    18,     0,     0,     0,    17,    15,
    14,    11,    16,     0,     9,     0,     0,     0
};

static const short yydefgoto[] = {    66,
    14,    15,    16,    25,    34,    45,    57,    54,    17,    21,
    22,     4
};

static const short yypact[] = {    -2,
-32768,-32768,-32768,     5,    21,    25,     7,-32768,     4,    43,
    43,     6,     9,    21,-32768,    10,    11,    24,    25,    25,
    24,    61,    36,    39,-32768,-32768,    24,    24,-32768,-32768,
-32768,-32768,    36,-32768,-32768,-32768,-32768,-32768,    25,    25,
    25,    25,    25,    53,    40,-32768,    40,    52,    52,-32768,
-32768,-32768,-32768,-32768,    -4,    53,    41,    67,-32768,-32768,
    36,-32768,    40,    68,-32768,    27,    82,-32768
};

static const short yypgoto[] = {-32768,
-32768,    83,    70,-32768,-32768,   -32,   -43,    29,    62,    -5,
   -17,-32768
};


#define	YYLAST		86


static const short yytable[] = {    18,
    47,    36,    37,    58,    27,    28,    59,     5,    18,     1,
     2,    23,    26,    32,    29,    33,    35,    30,    18,    64,
    24,    48,    49,    50,    51,    52,    67,     6,    63,     7,
     8,     6,     1,     2,     8,     9,    10,    11,    55,     9,
    19,    20,    12,    13,    35,     6,    12,    13,     8,     6,
    55,    44,     8,     9,    10,    11,    56,     9,    61,     6,
    12,    13,     8,    53,    12,    13,     0,     9,    38,    41,
    42,    43,    62,    65,    12,    13,    39,    40,    41,    42,
    43,    68,     3,    31,    60,    46
};

static const short yycheck[] = {     5,
    33,    19,    20,    47,    10,    11,    11,     3,    14,    12,
    13,     5,     9,     4,     9,     5,    21,     9,    24,    63,
    14,    39,    40,    41,    42,    43,     0,     7,    61,     9,
    10,     7,    12,    13,    10,    15,    16,    17,    44,    15,
    16,    17,    22,    23,    21,     7,    22,    23,    10,     7,
    56,    16,    10,    15,    16,    17,    17,    15,    18,     7,
    22,    23,    10,    11,    22,    23,    -1,    15,     8,    18,
    19,    20,     6,     6,    22,    23,    16,    17,    18,    19,
    20,     0,     0,    14,    56,    24
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */


/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 3 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
/*###jp###*/
int yyparse (void*);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif



/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;
/*
  printf("---%d\n",(yyssp - yyss + 1));
*/
  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
{ *(((LayoutConverterArg)layoutConverterArg)->to) = (LayoutPtr) yyvsp[0].bval; ;
    break;}
case 2:
{
			BoxPtr	box = New(LBoxRec);
                        MEMMESSAGE(WidgetBox)
			box->nextSibling = 0;
			box->type = WidgetBox;
			box->params = *yyvsp[0].pval;
			Dispose (yyvsp[0].pval);
			box->u.widget.quark = yyvsp[-1].qval;
			yyval.bval = box;
		    ;
    break;}
case 3:
{
			BoxPtr	box = New(LBoxRec);
                        MEMMESSAGE(GlueBox)
			box->nextSibling = 0;
			box->type = GlueBox;
			box->params = *yyvsp[0].pval;
			Dispose (yyvsp[0].pval);
			box->u.glue.expr = yyvsp[-1].eval;
			yyval.bval = box;
		    ;
    break;}
case 4:
{
			BoxPtr	box = New(LBoxRec);
                        MEMMESSAGE(VariableBox)
			box->nextSibling = 0;
			box->type = VariableBox;
			box->u.variable.quark = yyvsp[-2].qval;
			box->u.variable.expr = yyvsp[0].eval;
			ZeroGlue (box->params.stretch[LayoutHorizontal]); /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutHorizontal]);  /*###jp###*/
			ZeroGlue (box->params.stretch[LayoutVertical]);   /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutVertical]);    /*###jp###*/
			yyval.bval = box;
		    ;
    break;}
case 5:
{
			yyval.bval = yyvsp[0].bval;
		    ;
    break;}
case 6:
{
			BoxPtr	box = New(LBoxRec);
			BoxPtr	child;

                        MEMMESSAGE(BoxBox)
			box->nextSibling = 0;
			box->parent = 0;
			box->type = BoxBox;
			box->u.box.dir = yyvsp[-3].lval;
			box->u.box.firstChild = yyvsp[-1].bval;
			ZeroGlue (box->params.stretch[LayoutHorizontal]); /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutHorizontal]);  /*###jp###*/
			ZeroGlue (box->params.stretch[LayoutVertical]);   /*###jp###*/
			ZeroGlue (box->params.shrink[LayoutVertical]);    /*###jp###*/

			for (child = yyvsp[-1].bval; child; child = child->nextSibling) 
			{
			    if (child->type == GlueBox) 
			    {
				child->params.stretch[!yyvsp[-3].lval].expr = 0;
				child->params.shrink[!yyvsp[-3].lval].expr = 0;
				child->params.stretch[!yyvsp[-3].lval].order = 100000;
				child->params.shrink[!yyvsp[-3].lval].order = 100000;
				child->params.stretch[!yyvsp[-3].lval].value = 1;
				child->params.shrink[!yyvsp[-3].lval].value = 1;
			    }
			    child->parent = box;
			}
			yyval.bval = box;
		    ;
    break;}
case 7:
{ 
			yyvsp[-1].bval->nextSibling = yyvsp[0].bval;
			yyval.bval = yyvsp[-1].bval;
		    ;
    break;}
case 8:
{	yyval.bval = yyvsp[0].bval; ;
    break;}
case 9:
{	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			p->stretch[LayoutHorizontal] = yyvsp[-5].gval;
			p->shrink[LayoutHorizontal] = yyvsp[-4].gval;
			p->stretch[LayoutVertical] = yyvsp[-2].gval;
			p->shrink[LayoutVertical] = yyvsp[-1].gval;
			yyval.pval = p;
		    ;
    break;}
case 10:
{	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			ZeroGlue (p->stretch[LayoutHorizontal]);
			ZeroGlue (p->shrink[LayoutHorizontal]);
			ZeroGlue (p->stretch[LayoutVertical]);
			ZeroGlue (p->shrink[LayoutVertical]);
			yyval.pval = p;
		    ;
    break;}
case 11:
{	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			p->stretch[LayoutHorizontal] = yyvsp[-2].gval;
			p->shrink[LayoutHorizontal] = yyvsp[-1].gval;
			p->stretch[LayoutVertical] = yyvsp[-2].gval;
			p->shrink[LayoutVertical] = yyvsp[-1].gval;
			yyval.pval = p;
		    ;
    break;}
case 12:
{	
			BoxParamsPtr	p = New(BoxParamsRec);
			
                        MEMMESSAGE(BoxParamsRec)
			ZeroGlue (p->stretch[LayoutHorizontal]);
			ZeroGlue (p->shrink[LayoutHorizontal]);
			ZeroGlue (p->stretch[LayoutVertical]);
			ZeroGlue (p->shrink[LayoutVertical]);
			yyval.pval = p;
		    ;
    break;}
case 13:
{ yyval.gval = yyvsp[0].gval; ;
    break;}
case 14:
{ ZeroGlue (yyval.gval); ;
    break;}
case 15:
{ yyval.gval = yyvsp[0].gval; ;
    break;}
case 16:
{ ZeroGlue (yyval.gval); ;
    break;}
case 17:
{ yyval.gval.order = yyvsp[0].ival; yyval.gval.expr = yyvsp[-1].eval; ;
    break;}
case 18:
{ yyval.gval.order = 0; yyval.gval.expr = yyvsp[0].eval; ;
    break;}
case 19:
{ yyval.gval.order = yyvsp[0].ival; yyval.gval.expr = 0; yyval.gval.value = 1; ;
    break;}
case 20:
{
			yyval.eval = New(ExprRec);
                        MEMMESSAGE(ExprRec)
			yyval.eval->type = Unary;
			yyval.eval->u.unary.op = yyvsp[-1].oval;
			yyval.eval->u.unary.down = yyvsp[0].eval;
		    ;
    break;}
case 21:
{ yyval.eval = yyvsp[0].eval; ;
    break;}
case 23:
{	yyval.eval = New(ExprRec);
                        MEMMESSAGE(Width)
			yyval.eval->type = Width;
			yyval.eval->u.width = yyvsp[0].qval;
		    ;
    break;}
case 24:
{	yyval.eval = New(ExprRec);
                        MEMMESSAGE(Height)
			yyval.eval->type = Height;
			yyval.eval->u.height = yyvsp[0].qval;
		    ;
    break;}
case 25:
{ yyval.eval = yyvsp[-1].eval; ;
    break;}
case 26:
{
			yyval.eval = New(ExprRec);
			yyval.eval->type = Unary;
			yyval.eval->u.unary.op = yyvsp[0].oval;
			yyval.eval->u.unary.down = yyvsp[-1].eval;
		    ;
    break;}
case 27:
{	yyval.eval = New(ExprRec);
                        MEMMESSAGE(Constant)
			yyval.eval->type = Constant;
			yyval.eval->u.constant = yyvsp[0].ival;
		    ;
    break;}
case 28:
{	yyval.eval = New(ExprRec);
                        MEMMESSAGE(Variable)
			yyval.eval->type = Variable;
			yyval.eval->u.variable = yyvsp[0].qval;
		    ;
    break;}
case 29:
{ binary: ;
			yyval.eval = New(ExprRec);
                        MEMMESSAGE(Binary)
			yyval.eval->type = Binary;
			yyval.eval->u.binary.op = yyvsp[-1].oval;
			yyval.eval->u.binary.left = yyvsp[-2].eval;
			yyval.eval->u.binary.right = yyvsp[0].eval;
		    ;
    break;}
case 30:
{ goto binary; ;
    break;}
case 31:
{ goto binary; ;
    break;}
case 32:
{ goto binary; ;
    break;}
case 33:
{ goto binary; ;
    break;}
case 34:
{ /* unary: ; ###jp###*/
			yyval.eval = New(ExprRec);
                        MEMMESSAGE(Unary)
			yyval.eval->type = Unary;
			yyval.eval->u.unary.op = yyvsp[-1].oval;
			yyval.eval->u.unary.down = yyvsp[0].eval;
		    ;
    break;}
case 35:
{ yyval.eval = yyvsp[0].eval; ;
    break;}
case 37:
{   yyval.lval = LayoutVertical; ;
    break;}
case 38:
{   yyval.lval = LayoutHorizontal; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}

