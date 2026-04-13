/* This file is part of GNU cflow
   Copyright (C) 1997-2025 Sergey Poznyakoff

   GNU cflow is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   GNU cflow is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <cflow.h>
#include <progname.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <wordsplit.h>
#include <parseopt.h>
#include <parser.h>

/* Structure representing various arguments of command line options */
struct option_type {
    char *str;           /* optarg value */
    int min_match;       /* minimal number of characters to match */
    int type;            /* data associated with the arg */
};

int debug;              /* debug level */
char *outname = "-";    /* default output file name */
int print_option = 0;   /* what to print. */
int verbose;            /* be verbose on output */
int use_indentation;    /* Rely on indentation,
			 * i.e. suppose the function body
			 * is necessarily surrounded by the curly braces
			 * in the first column
			 */
int strict_ansi;        /* Assume sources to be written in ANSI C */
int print_line_numbers; /* Print line numbers */
int print_levels;       /* Print level number near every branch */
int print_as_tree;      /* Print as tree */
int brief_listing;      /* Produce short listing */
int reverse_tree;       /* Generate reverse tree */
unsigned max_depth;     /* The depth at which the flowgraph is cut off */
int emacs_option;       /* Format and check for use with Emacs cflow-mode */
int omit_arguments_option;    /* Omit arguments from function declaration string */
int omit_symbol_names_option; /* Omit symbol name from symbol declaration string */

static int no_main_option; /* Disable start symbols */

static char *default_optfilepath =
     "~/.cflow.d:"
     PKGDATADIR "/site-init:"
     PKGDATADIR "/" PACKAGE_VERSION;

static char *optfilepath;
static char *optfilesuf = ".cfo";
static char *default_profile_name = "default";

#define SM_FUNCTIONS   0x0001
#define SM_DATA        0x0002
#define SM_STATIC      0x0004
#define SM_UNDERSCORE  0x0008
#define SM_TYPEDEF     0x0010
#define SM_UNDEFINED   0x0020

static inline int
CHAR_TO_SM(int c)
{
     switch (c) {
     case 'x': return SM_DATA;
     case '_': return SM_UNDERSCORE;
     case 's': return SM_STATIC;
     case 't': return SM_TYPEDEF;
     case 'u': return SM_UNDEFINED;
     }
     return 0;
}

#define SYMBOL_INCLUDE(c) (symbol_map |= CHAR_TO_SM(c))
#define SYMBOL_EXCLUDE(c) (symbol_map &= ~CHAR_TO_SM(c))
static int symbol_map;  /* A bitmap of symbols included in the graph. */

int output_visible;

char *level_indent[] = { NULL, NULL };
char *level_end[] = { "", "" };
char *level_begin = "";

int preprocess_option = 0; /* Do they want to preprocess sources? */

int all_functions;

/* Given the option_type array and (possibly abbreviated) option argument
 * find the type corresponding to that argument.
 * Return 0 if the argument does not match any one of OPTYPE entries
 */
static int
find_option_type(struct option_type *optype, const char *str, int len)
{
     if (len == 0)
	  len = strlen(str);
     for ( ; optype->str; optype++) {
	  if (len >= optype->min_match &&
	      len <= strlen(optype->str) &&
	      memcmp(str, optype->str, len) == 0) {
	       return optype->type;
	  }
     }
     return 0;
}

/* Args for --symbol option */
static struct option_type symbol_optype[] = {
     { "keyword", 2, WORD },
     { "kw", 2, WORD },
     { "modifier", 1, MODIFIER },
     { "identifier", 1, IDENTIFIER },
     { "type", 1, TYPE },
     { "wrapper", 1, PARM_WRAPPER },
     { "qualifier", 1, QUALIFIER },
     { "declaration", 1, DECLARATION },
     { 0 },
};

/* Parse the string STR and store the symbol in the temporary symbol table.
 * STR is the string of form: NAME:TYPE
 * NAME means symbol name, TYPE means symbol type (possibly abbreviated)
 */
static void
symbol_override(const char *str)
{
     const char *ptr;
     char *name;
     Symbol *sp;

     ptr = strchr(str, ':');
     if (!ptr)
	  error(EX_USAGE, 0, _("%s: no symbol type supplied"), str);
     else {
	  name = strndup(str, ptr - str);
	  if (!name)
	       xalloc_die();
	  if (ptr[1] == '=') {
	       Symbol *alias;

	       ptr += 2;
	       if (strcmp(name, ptr) == 0) {
		    error(EX_USAGE, 0, _("cyclic alias: %s -> %s"), name, ptr);
	       }

	       alias = lookup(ptr);
	       if (alias) {
		    if (strcmp(alias->name, name) == 0) {
			 error(EX_USAGE, 0, _("cyclic alias: %s -> %s -> %s"),
			       name, ptr, alias->name);
		    }
	       } else {
		    alias = install(xstrdup(ptr), INSTALL_OVERWRITE);
		    alias->type = SymToken;
		    alias->token_type = 0;
		    alias->source = NULL;
		    alias->def_line = -1;
		    alias->ref_line = NULL;
	       }
	       sp = install(name, INSTALL_OVERWRITE);
	       sp->type = SymToken;
	       sp->alias = alias;
	       sp->flag = symbol_alias;
	  } else {
	       int type = find_option_type(symbol_optype, ptr+1, 0);
	       if (type == 0)
		    error(EX_USAGE, 0, _("unknown symbol type: %s"), ptr+1);
	       sp = install(name, INSTALL_OVERWRITE);
	       sp->type = SymToken;
	       sp->token_type = type;
	  }
	  sp->source = NULL;
	  sp->def_line = -1;
	  sp->ref_line = NULL;
     }
}

/* Convert first COUNT bytes of the string pointed to by STR_PTR
 * to integer using BASE. Move STR_PTR to the point where the
 * conversion stopped.
 * Return the number obtained.
 */
static int
number(const char **str_ptr, int base, int count)
{
     int  c, n;
     unsigned i;
     const char *str = *str_ptr;

     for (n = 0; *str && count; count--) {
	  c = *str++;
	  if (isdigit(c))
	       i = c - '0';
	  else
	       i = toupper(c) - 'A' + 10;
	  if (i > base) {
	       break;
	  }
	  n = n * base + i;
     }
     *str_ptr = str - 1;
     return n;
}

/* Processing for --level option
 * The option syntax is
 *    --level NUMBER
 * or
 *    --level KEYWORD=STR
 * where
 *    KEYWORD is one of "begin", "0", ", "1", "end0", "end1",
 *    or an abbreviation thereof,
 *    STR is the value to be assigned to the parameter.
 *
 * STR can contain usual C escape sequences plus \e meaning '\033'.
 * Apart from this any character followed by xN suffix (where N is
 * a decimal number) is expanded to the sequence of N such characters.
 * 'x' looses its special meaning at the start of the string.
 */
#define MAXLEVELINDENT 216
#define LEVEL_BEGIN 1
#define LEVEL_INDENT0 2
#define LEVEL_INDENT1 3
#define LEVEL_END0 4
#define LEVEL_END1 5

static struct option_type level_indent_optype[] = {
     { "begin", 1, LEVEL_BEGIN },
     { "start", 1, LEVEL_BEGIN },
     { "0", 1, LEVEL_INDENT0 },
     { "1", 1, LEVEL_INDENT1 },
     { "end0", 4, LEVEL_END0 },
     { "end1", 4, LEVEL_END1 },
};

static void
parse_level_string(const char *str, char **return_ptr)
{
     static char text[MAXLEVELINDENT];
     char *p;
     int i, c, num;

     p = text;
     memset(text, ' ', sizeof(text));
     text[sizeof(text)-1] = 0;

     while (*str) {
	  switch (*str) {
	  case '\\':
	       switch (*++str) {
	       case 'a':
		    *p++ = '\a';
		    break;
	       case 'b':
		    *p++ = '\b';
		    break;
	       case 'e':
		    *p++ = '\033';
		    break;
	       case 'f':
		    *p++ = '\f';
		    break;
	       case 'n':
		    *p++ = '\n';
		    break;
	       case 'r':
		    *p++ = '\r';
		    break;
	       case 't':
		    *p++ = '\t';
		    break;
	       case 'x':
	       case 'X':
		    ++str;
		    *p++ = number(&str,16,2);
		    break;
	       case '0':
		    ++str;
		    *p++ = number(&str,8,3);
		    break;
	       default:
		    *p++ = *str;
	       }
	       ++str;
	       break;
	  case 'x':
	       if (p == text) {
		    goto copy;
	       }
	       num = strtol(str+1, (char**)&str, 10);
	       c = p[-1];
	       for (i = 1; i < num; i++) {
		    *p++ = c;
		    if (*p == 0)
			 error(EX_USAGE, 0,
			       _("level indent string is too long"));
	       }
	       break;
	  default:
	  copy:
	       *p++ = *str++;
	       if (*p == 0)
		    error(EX_USAGE, 0, _("level indent string is too long"));
	  }
     }
     *p = 0;
     *return_ptr = strdup(text);
}

static void
set_level_indent(const char *str)
{
     long n;
     const char *p;
     char *q;

     n = strtol(str, &q, 0);
     if (*q == 0 && n > 0) {
	  char *s = xmalloc(n+1);
	  memset(s, ' ', n-1);
	  s[n-1] = 0;
	  level_indent[0] = level_indent[1] = s;
	  return;
     }

     p = str;
     while (*p != '=') {
	  if (*p == 0)
	       error(EX_USAGE, 0, _("level-indent syntax"));
	  p++;
     }
     ++p;

     switch (find_option_type(level_indent_optype, str, p - str - 1)) {
     case LEVEL_BEGIN:
	  parse_level_string(p, &level_begin);
	  break;
     case LEVEL_INDENT0:
	  parse_level_string(p, &level_indent[0]);
	  break;
     case LEVEL_INDENT1:
	  parse_level_string(p, &level_indent[1]);
	  break;
     case LEVEL_END0:
	  parse_level_string(p, &level_end[0]);
	  break;
     case LEVEL_END1:
	  parse_level_string(p, &level_end[1]);
	  break;
     default:
	  error(EX_USAGE, 0, _("unknown level indent option: %s"), str);
     }
}

static int
optset_include_classes(struct parseopt *po, struct optdef *opt, char *arg)
{
     int n;

     n = 1;
     for (; *arg; arg++) {
	  switch (*arg) {
	  case '-':
	  case '^':
	       n = 0;
	       break;
	  case '+':
	       n = 1;
	       break;
	  case 'x':
	  case '_':
	  case 's':
	  case 't':
	  case 'u':
	       if (n)
		    SYMBOL_INCLUDE(*arg);
	       else
		    SYMBOL_EXCLUDE(*arg);
	       break;
	  default:
	       po->po_error(po, PO_MSG_ERR, _("Unknown symbol class: %c"),
			    *arg);
	       exit(po->po_ex_usage);
	  }
     }
     return 0;
}

static int
optset_output_driver(struct parseopt *po, struct optdef *opt, char *arg)
{
     if (select_output_driver(arg)) {
	  po->po_error(po, PO_MSG_ERR, _("%s: No such output driver"), arg);
	  exit(po->po_ex_usage);
     }
     output_init();
     return 0;
}

static int
optset_xref(struct parseopt *po, struct optdef *opt, char *arg)
{
     print_option = PRINT_XREF;
     SYMBOL_EXCLUDE('s'); /* Exclude static symbols by default */
     return 0;
}

static int
optset_symbol(struct parseopt *po, struct optdef *opt, char *arg)
{
     symbol_override(arg);
     return 0;
}

static int
optset_preproc_option(struct parseopt *po, struct optdef *opt, char *arg)
{
     while (opt->opt_name[1]) {
	  ++opt;
	  if (!(opt->opt_flags & OPTFLAG_ALIAS)) {
	       po->po_error(po, PO_MSG_ERR,
			    "INTERNAL ERROR at %s:%d, please report",
			    __FILE__, __LINE__);
	       exit(EX_SOFTWARE);
	  }
     }
     pp_option(opt->opt_name[0], arg);
     preprocess_option = 1;
     return 0;
}

static int
optset_preprocess(struct parseopt *po, struct optdef *opt, char *arg)
{
     preprocess_option = 1;
     set_preprocessor(arg ? arg : CFLOW_PREPROC);
     return 0;
}

static int
optset_level_indent(struct parseopt *po, struct optdef *opt, char *arg)
{
     set_level_indent(arg);
     return 0;
}

static int
optset_main_symbol(struct parseopt *po, struct optdef *opt, char *arg)
{
     install_starter(arg);
     return 0;
}

static int
optset_clear_main_symbol(struct parseopt *po, struct optdef *opt, char *arg)
{
     clear_starters();
     no_main_option = 1;
     return 0;
}

static int
optset_install_target(struct parseopt *po, struct optdef *opt, char *arg)
{
     install_target(arg);
     return 0;
}

static int
optset_int_1(struct parseopt *po, struct optdef *opt, char *arg)
{
     if (arg)
	  return optset_int(po, opt, arg);
     *(int*) opt->opt_ptr = 1;
     return 0;
}

static int
optset_prepend_path(struct parseopt *po, struct optdef *opt, char *arg)
{
     size_t arglen = strlen(arg);
     size_t pathlen = strlen(optfilepath);

     optfilepath = xrealloc(optfilepath, arglen + pathlen + 2);
     memmove(optfilepath + arglen + 1, optfilepath, pathlen + 1);
     memcpy(optfilepath, arg, arglen);
     optfilepath[arglen] = ':';

     return 0;
}

static int optset_profile(struct parseopt *po, struct optdef *opt, char *arg);

static struct optdef options[] = {
     {
	  .opt_flags = OPTFLAG_DOC,
	  .opt_doc = N_("General options:")
     },
     {
	  .opt_name = "depth",
	  .opt_argdoc = N_("NUMBER"),
	  .opt_doc = N_("set the depth at which the flowgraph is cut off"),
	  .opt_set = optset_uint,
	  .opt_ptr = &max_depth
     },
     {
	  .opt_name = "d",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "include",
	  .opt_argdoc = N_("CLASSES"),
	  .opt_doc = N_("include specified classes of symbols (see below);"
			" prepend CLASSES with ^ or - to exclude them from"
			" the output"),
	  .opt_set = optset_include_classes
     },
     {
	  .opt_name = "i",
	  .opt_flags = OPTFLAG_ALIAS
     },
     {
	  .opt_flags = OPTFLAG_DOC|OPTFLAG_SUBLIST,
	  .opt_name = "x",
	  .opt_doc = N_("all data symbols, both external and static")
     },
     {
	  .opt_flags = OPTFLAG_DOC|OPTFLAG_SUBLIST,
	  .opt_name = "_",
	  .opt_doc = N_("symbols whose names begin with an underscore")
     },
     {
	  .opt_flags = OPTFLAG_DOC|OPTFLAG_SUBLIST,
	  .opt_name = "s",
	  .opt_doc = N_("static symbols")
     },
     {
	  .opt_flags = OPTFLAG_DOC|OPTFLAG_SUBLIST,
	  .opt_name = "t",
	  .opt_doc = N_("typedefs (for cross-references only)")
     },

     {
	  .opt_name = "format",
	  .opt_argdoc = N_("NAME"),
	  /* TRANSLATORS: Don't translate quoted names. */
	  .opt_doc = N_("use given output format NAME; valid names are"
			" `gnu' (default), `posix', and `dot'"),
	  .opt_set = optset_output_driver
     },
     {
	  .opt_name = "f",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "reverse",
	  .opt_doc = N_("print reverse call tree"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &reverse_tree
     },
     {
	  .opt_name = "r",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "xref",
	  .opt_doc = N_("produce cross-reference listing only"),
	  .opt_set = optset_xref
     },
     {
	  .opt_name = "x",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "output",
	  .opt_argdoc = N_("FILE"),
	  .opt_doc = N_("set output file name (default -, meaning stdout)"),
	  .opt_set = optset_string_copy,
	  .opt_ptr = &outname
     },
     {
	  .opt_name = "o",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_flags = OPTFLAG_DOC,
	  .opt_doc = N_("Program initialization:")
     },
     {
	  .opt_name = "profile",
	  .opt_argdoc = N_("FILE"),
	  .opt_doc = N_("read options from FILE"),
	  .opt_set = optset_profile,
     },
     {
	  .opt_name = "prepend-path",
	  .opt_argdoc = N_("DIR"),
	  .opt_doc = N_("prepend DIR to the profile search path"),
	  .opt_flags = OPTFLAG_EARLY,
	  .opt_set = optset_prepend_path
     },
     {
	  .opt_name = "P",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "q",
	  .opt_doc = N_("don't load default profile"),
	  .opt_flags = OPTFLAG_EARLY,
	  .opt_set = optset_string_copy,
	  .opt_ptr = &default_profile_name
     },

     {
	  .opt_flags = OPTFLAG_DOC,
	  .opt_doc = N_("Parser control:")
     },
     {
	  .opt_name = "use-indentation",
	  .opt_doc = N_("rely on indentation"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &use_indentation
     },
     {
	  .opt_name = "S",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "ansi",
	  .opt_doc = N_("accept only sources in ANSI C"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &strict_ansi
     },
     {
	  .opt_name = "a",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "pushdown",
	  .opt_argdoc = N_("NUMBER"),
	  .opt_doc = N_("set initial token stack size to NUMBER"),
	  .opt_set = optset_uint,
	  .opt_ptr = &token_stack_length
     },
     {
	  .opt_name = "p",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "symbol",
	  .opt_argdoc = N_("SYMBOL:[=]TYPE"),
	  .opt_doc =
	  /* TRANSLATORS: Don't translate type names. */
	  N_("register SYMBOL with given TYPE, or define an alias"
	     " (if := is used); valid types are: keyword (or kw),"
	     " modifier, qualifier, identifier, type, wrapper,"
	     " or any unambiguous abbreviation thereof"),
	  .opt_set = optset_symbol,
     },
     {
	  .opt_name = "s",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "define",
	  .opt_argdoc = N_("NAME[=DEFN]"),
	  .opt_doc = N_("predefine NAME as a macro"),
	  .opt_set = optset_preproc_option,
     },
     {
	  .opt_name = "D",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "undefine",
	  .opt_argdoc = N_("NAME"),
	  .opt_doc = N_("cancel any previous definition of NAME"),
	  .opt_set = optset_preproc_option,
     },
     {
	  .opt_name = "U",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "include-dir",
	  .opt_argdoc = N_("DIR"),
	  .opt_doc = N_("add the directory DIR to the list of directories"
			" to be searched for header files"),
	  .opt_set = optset_preproc_option,
     },
     {
	  .opt_name = "I",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "preprocess",
	  .opt_argdoc = N_("COMMAND"),
	  .opt_flags = OPTFLAG_ARG_OPTIONAL,
	  .opt_doc = N_("run the specified preprocessor command"),
	  .opt_set = optset_preprocess,
     },
     {
	  .opt_name = "cpp",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "no-preprocess",
	  .opt_doc = N_("disable preprocessor"),
	  .opt_set = optset_false,
	  .opt_ptr = &preprocess_option
     },
     {
	  .opt_name = "no-cpp",
	  .opt_flags = OPTFLAG_ALIAS,
     },

     {
	  .opt_flags = OPTFLAG_DOC,
	  .opt_doc = N_("Output control:")
     },
     {
	  .opt_name = "all",
	  .opt_doc = N_("show all functions, not only those reachable"
			" from main"),
	  .opt_set = optset_incr,
	  .opt_ptr = &all_functions
     },
     {
	  .opt_name = "A",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "number",
	  .opt_doc = N_("print line numbers"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &print_line_numbers
     },
     {
	  .opt_name = "n",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "print-level",
	  .opt_argdoc = N_("print nesting level along with the call tree"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &print_levels
     },
     {
	  .opt_name = "l",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "level-indent",
	  .opt_argdoc = N_("ELEMENT"),
	  .opt_doc = N_("control graph appearance; see [1] for details"),//FIXME
	  .opt_set = optset_level_indent,
     },

     {
	  .opt_name = "tree",
	  .opt_doc = N_("draw ASCII art tree"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &print_as_tree
     },
     {
	  .opt_name = "T",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "brief",
	  .opt_doc = N_("brief output"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &brief_listing
     },
     {
	  .opt_name = "b",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "emacs",
	  .opt_doc = N_("additionally format output for use with GNU Emacs"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &emacs_option
     },

     {
	  .opt_name = "omit-arguments",
	  .opt_doc = N_("don't print argument lists in function declarations"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &omit_arguments_option
     },

     {
	  .opt_name = "omit-symbol-names",
	  .opt_doc = N_("don't print symbol names in declaration strings"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &omit_symbol_names_option
     },

     {
	  .opt_name = "main",
	  .opt_argdoc = N_("NAME"),
	  .opt_doc = N_("start graph at this function; multiple options"
			" are allowed"),
	  .opt_set = optset_main_symbol,
     },
     {
	  .opt_name = "m",
	  .opt_flags = OPTFLAG_ALIAS
     },
     {
	  .opt_name = "start",
	  .opt_flags = OPTFLAG_ALIAS,
     },

     {
	  .opt_name = "no-main",
	  .opt_doc = N_("there's no main function; print graphs for all"
			" functions in the program"),
	  .opt_set = optset_clear_main_symbol,
     },

     {
	  .opt_name = "target",
	  .opt_argdoc = N_("NAME"),
	  .opt_doc = N_("show only graphs leading from start symbols to"
			" this function; multiple options are allowed"),
	  .opt_set = optset_install_target,
     },

     {
	  .opt_flags = OPTFLAG_DOC,
	  .opt_doc = N_("Informational options:")
     },
     {
	  .opt_name = "verbose",
	  .opt_doc = N_("verbose error diagnostics"),
	  .opt_flags = OPTFLAG_BOOL,
	  .opt_ptr = &verbose
     },
     {
	  .opt_name = "v",
	  .opt_flags = OPTFLAG_ALIAS
     },

     {
	  .opt_name = "debug",
	  .opt_argdoc = N_("NUMBER"),
	  .opt_flags = OPTFLAG_ARG_OPTIONAL,
	  .opt_doc = N_("set debugging level"),
	  .opt_set = optset_int_1,
	  .opt_ptr = &debug
     },

     { NULL }
}, *optdef[] = { options, NULL };

static int copyright_year = 2025;
static char gplv3[] = N_("\
\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
\n\
");

void
version_hook(WORDWRAP_FILE wf, struct parseopt *po)
{
     wordwrap_printf(wf, "%s (%s) %s\n",
		     po->po_program_name, PACKAGE_NAME, PACKAGE_VERSION);
     wordwrap_printf(wf, "Copyright %s 2005-%d Sergey Poznyakoff\n",
		     _("(C)"),
		     copyright_year);
     wordwrap_puts(wf, gplv3);
     wordwrap_printf(wf, _("Written by %s."), "Sergey Poznyakoff");

}

static void
help_hook(WORDWRAP_FILE wf, struct parseopt *po)
{
     wordwrap_printf (wf, "%s\n", _("References:"));
     wordwrap_puts (wf, " [1]   https://www.gnu.org/software/cflow/manual/html_section/ASCII-Tree.html\n");
     wordwrap_para (wf);
     wordwrap_printf (wf, _("Profile search path: %s\n"), default_optfilepath);
}

static void init_hook (struct parseopt *po);

static struct parseopt po = {
     .po_descr = N_("generate program flowgraph"),
     .po_argdoc = "[OPTIONS] [FILE]...",
     .po_optdef = optdef,
     .po_package_name = PACKAGE_NAME,
     .po_package_url = PACKAGE_URL,
     .po_bugreport_address = PACKAGE_BUGREPORT,
     .po_general_help = N_("General help using GNU software: <https://www.gnu.org/gethelp/>"),
     .po_init_hook = init_hook,
     .po_help_hook = help_hook,
     .po_version_hook = version_hook,
     .po_ex_usage = EX_USAGE,
};

static void
po_env_error (struct parseopt *po, int pri, char const *fmt, ...)
{
     va_list ap;

     if (po->po_flags & PARSEOPT_IGNORE_ERRORS)
	  return;

     if (pri == PO_MSG_ERR) {
	  if (po->po_program_name)
	       fprintf(stderr, "%s: ", po->po_program_name);
	  fprintf(stderr, "CFLOW_OPTIONS: ");
     }
     va_start (ap, fmt);
     vfprintf (stderr, fmt, ap);
     va_end (ap);
     fputc ('\n', stderr);
}

static void
parseopt_from_env(void)
{
     char *env;

     env = getenv("CFLOW_OPTIONS");
     if (env && *env) {
	  struct wordsplit ws;
	  struct parseopt p2;

	  if (wordsplit(env, &ws, WRDSF_DEFFLAGS))
	       error(EX_FATAL, 0, "failed to parse CFLOW_OPTIONS: %s",
		     wordsplit_strerror(&ws));

	  p2 = po;
	  p2.po_flags |= PARSEOPT_ARGV0 | PARSEOPT_NO_ERREXIT;
	  p2.po_error = po_env_error;

	  if (parseopt_getopt(&p2, ws.ws_wordc, ws.ws_wordv) == OPT_ERR)
	       error(EX_SOFTWARE, 0, _("parseopt_getopt failed"));

	  wordsplit_free(&ws);
	  parseopt_free(&p2);
     }
}

struct parseopt_file {
     struct parseopt po;
     char *dir_name;
     char *file_name;
     int line;
     FILE *file;
};

static void
fromfile_error(struct parseopt *ptr, int pri, char const *fmt, ...)
{
     struct parseopt_file *pf = (struct parseopt_file *) ptr;
     va_list ap;

     if (pf->po.po_flags & PARSEOPT_IGNORE_ERRORS)
	  return;

     if (pf->po.po_program_name && pri == PO_MSG_ERR) {
	  fprintf(stderr, "%s: ", pf->po.po_program_name);
	  if (pf->dir_name)
	       fprintf(stderr, "%s/%s:%d: ",
		       pf->dir_name, pf->file_name, pf->line);
	  else
	       fprintf(stderr, "%s:%d: ", pf->file_name, pf->line);
     }
     va_start(ap, fmt);
     vfprintf(stderr, fmt, ap);
     va_end(ap);
     fputc('\n', stderr);
}

static int
fromfile(struct parseopt_file *pf)
{
     char *buf = NULL;
     size_t size = 0;
     ssize_t n;
     struct wordsplit ws;
     int wsflags = 0;
     int res = 0;

     ws.ws_comment = "#";
     wsflags = WRDSF_DEFFLAGS | WRDSF_COMMENT;

     while ((n = getline(&buf, &size, pf->file)) >= 0) {
	  char *p;

	  pf->line++;
	  for (p = buf; n > 0 && isspace (*p); p++, n--)
	       ;
	  if (n == 0 || *p == '#')
	       continue;
	  while (n > 0 && isspace(p[n-1]))
	       n--;
	  if (n == 0)
	       continue;
	  p[n] = 0;

	  if (wordsplit(p, &ws, wsflags)) {
	       pf->po.po_error((struct parseopt*) pf, PO_MSG_ERR,
			       _("error splitting line: %s"),
			       wordsplit_strerror(&ws));
	       res = 1;
	       break;
	  }

	  if (parseopt_getopt((struct parseopt *) pf,
			      ws.ws_wordc, ws.ws_wordv)) {
	       res = 1;
	       break;
	  }

	  wsflags |= WRDSF_REUSE;
     }

     if (wsflags & WRDSF_REUSE)
	  wordsplit_free(&ws);
     free (buf);

     if (res == 0 && ferror(pf->file)) {
	  pf->po.po_error ((struct parseopt*) pf, PO_MSG_ERR,
			   _("file read error: %s"), strerror(errno));
	  res = 1;
     }

     return res;
}

struct optfileid {
     dev_t dev;
     ino_t ino;
};

struct linked_list *optfiles;

static int
optfile_register(dev_t dev, ino_t ino)
{
     struct linked_list_entry *p;
     struct optfileid *id;

     if (!optfiles)
	  optfiles = linked_list_create(free);
     for (p = linked_list_head(optfiles); p; p = p->next) {
	  id = p->data;
	  if (id->dev == dev && id->ino == ino)
	       return 1;
     }

     id = xmalloc(sizeof(*id));
     id->dev = dev;
     id->ino = ino;

     linked_list_append(&optfiles, id);

     return 0;
}

static char *
tildexpand(char const *str)
{
     char *ret;

     if (str[0] == '~') {
	  struct passwd *pwd;
	  size_t n = strcspn(str, "/");
	  size_t len;

	  if (n == 1)
	       pwd = getpwuid (getuid ());
	  else {
	       char *user = xmalloc(n);
	       memcpy(user, str+1, n-1);
	       user[n-1] = 0;
	       pwd = getpwnam(user);
	       free(user);
	  }

	  if (!pwd)
	       return NULL;

	  len = strlen(str) - n;

	  ret = xmalloc(strlen(pwd->pw_dir) + len + 1);
	  strcpy(ret, pwd->pw_dir);
	  if (len > 0)
	       strcat(ret, str + n);
     } else
	  ret = xstrdup(str);
     return ret;
}

enum {
     OPTFILE_OK,
     OPTFILE_DUP,
     OPTFILE_NOENT
};

static int
optfile_lookup(char const *name, struct parseopt_file *pf)
{
     struct stat st;
     struct wordsplit ws = { .ws_delim = ":" };
     int dirfd, fd = -1;
     size_t i;
     char *file_name;
     char *dirname;

     if (stat(name, &st) == 0) {
	  FILE *fp;

	  if (optfile_register(st.st_dev, st.st_ino))
	       return OPTFILE_DUP;

	  if ((fp = fopen(name, "r")) == NULL)
	       error(EX_FATAL, errno, _("can't open option file %s: %s"),
		     name, strerror(errno));

	  pf->dir_name = NULL;
	  pf->file_name = xstrdup(name);
	  pf->file = fp;
	  return OPTFILE_OK;
     }

     file_name = xmalloc(strlen(name) + strlen(optfilesuf) + 1);
     strcat(strcpy(file_name, name), optfilesuf);

     if (wordsplit(optfilepath, &ws, WRDSF_NOVAR | WRDSF_NOCMD | WRDSF_DELIM))
	  error(EX_SOFTWARE, 0, "%s:%d: INTERNAL ERROR: wordsplit: %s",
		__FILE__, __LINE__, wordsplit_strerror(&ws));
     for (i = 0; i < ws.ws_wordc; i++) {
	  dirname = tildexpand(ws.ws_wordv[i]);
	  if (!dirname)
	       continue;
	  dirfd = open(dirname, O_RDONLY | O_NONBLOCK | O_DIRECTORY);
	  if (dirfd == -1) {
	       if (errno != ENOENT)
		    error(0, errno, _("can't open directory %s"), dirname);
	  } else {
	       fd = openat(dirfd, file_name, O_RDONLY);
	       close(dirfd);
	       if (fd == -1) {
		    if (errno != ENOENT)
			 error(EX_FATAL, errno, _("can't open file %s/%s"),
			       dirname, file_name);
	       } else {
		    break;
	       }
	  }
	  free(dirname);
     }
     wordsplit_free(&ws);

     if (fd == -1) {
	  return OPTFILE_NOENT;
     } else {
	  struct stat st;

	  if (fstat(fd, &st))
	       error(EX_FATAL, errno, _("can't stat %s/%s"),
		     dirname, file_name);

	  if (optfile_register(st.st_dev, st.st_ino)) {
	       free(dirname);
	       free(file_name);
	       close(fd);
	       return OPTFILE_DUP;
	  } else {
	       pf->dir_name = dirname;
	       pf->file_name = file_name;
	       if ((pf->file = fdopen(fd, "r")) == NULL)
		    error(EX_FATAL, errno, "fdopen");
	       return OPTFILE_OK;
	  }
     }
}

int
optset_profile(struct parseopt *po, struct optdef *opt, char *arg)
{
     struct parseopt_file pf;
     int rc;

     switch (optfile_lookup(arg, &pf)) {
     case OPTFILE_OK:
	  break;

     case OPTFILE_DUP:
	  return 0;

     case OPTFILE_NOENT:
	  po->po_error(po, PO_MSG_ERR, _("no such profile: %s"), arg);
	  exit(po->po_ex_usage);
     }

     pf.line = 0;
     pf.po = *po;
     pf.po.po_error = fromfile_error;
     pf.po.po_flags |= PARSEOPT_ARGV0 | PARSEOPT_NO_ERREXIT;
     rc = fromfile(&pf);

     fclose(pf.file);
     free(pf.file_name);
     free(pf.dir_name);

     if (rc)
	  exit(po->po_ex_usage);
     return 0;
}

static int
parseopt_from_rc(char const *file_name)
{
     struct parseopt_file pf;
     FILE *fp;
     int rc;

     if ((fp = fopen(file_name, "r")) == NULL) {
	  if (errno == ENOENT)
	       return 0;
	  po.po_error(&po, PO_MSG_ERR,
		      _("can't open option file %s: %s"),
		      file_name, strerror (errno));
	  return 1;
     }

     pf.dir_name = NULL;
     pf.file_name = (char*) file_name;
     pf.line = 0;
     pf.file = fp;

     pf.po = po;
     pf.po.po_error = fromfile_error;
     pf.po.po_flags |= PARSEOPT_ARGV0 | PARSEOPT_NO_ERREXIT;

     rc = fromfile(&pf);

     fclose(fp);

     return rc;
}

#ifndef LOCAL_RC
# define LOCAL_RC "~/.cflowrc"
#endif

int
parse_rc(void)
{
     char *name;
     int rc = 0;

     name = getenv("CFLOWRC");
     if (name) {
	  rc = (*name == 0) ? 0 : parseopt_from_rc(name);
     } else {
	  name = tildexpand(LOCAL_RC);
	  if (access(name, F_OK) == 0) {
	       int n = strcspn(optfilepath, ":");
	       error(0, 0, _("warning: processing obsolete %s file"),
		     LOCAL_RC);
	       if (n > 0) {
		    if (optfilepath[n-1] == '/')
			 n--;
		    error(0, 0,
			  _("warning: consider renaming it to %*.*s/%s%s"),
			  n, n, optfilepath,
			  default_profile_name, optfilesuf);
	       }
	       rc = parseopt_from_rc(name);
	  }
	  free(name);
     }
     return rc;
}

static void
init_hook (struct parseopt *po)
{
     struct parseopt_file pf;

     if (!default_profile_name)
	  return;

     if (optfile_lookup(default_profile_name, &pf) == OPTFILE_OK) {
	  pf.line = 0;
	  pf.po = *po;
	  pf.po.po_error = fromfile_error;
	  pf.po.po_flags |= PARSEOPT_ARGV0 | PARSEOPT_NO_ERREXIT;
	  fromfile(&pf);
	  fclose(pf.file);
	  free(pf.file_name);
	  free(pf.dir_name);
     }
}


int
globals_only()
{
     return !(symbol_map & SM_STATIC);
}

int
include_symbol(Symbol *sym)
{
     int type = 0;

     if (!sym)
	  return 0;

     if (sym->visible != output_visible)
	  return 0;

     if (sym->type == SymIdentifier) {
	  if (sym->name[0] == '_' && !(symbol_map & SM_UNDERSCORE))
	       return 0;

	  if (sym->storage == StaticStorage)
	       type |= SM_STATIC;
	  if (sym->arity == -1 && sym->storage != AutoStorage)
	       type |= SM_DATA;
	  else if (sym->arity >= 0)
	       type |= SM_FUNCTIONS;

	  if (!sym->source)
	       type |= SM_UNDEFINED;

     } else if (sym->type == SymToken) {
	  if (sym->token_type == TYPE && sym->source)
	       type |= SM_TYPEDEF;
	  else
	       return 0;
     }
     return (symbol_map & type) == type;
}

void
xalloc_die(void)
{
     error(EX_FATAL, ENOMEM, _("Exiting"));
     abort();
}

void
init()
{
     if (level_indent[0] == NULL)
	  level_indent[0] = "    "; /* 4 spaces */
     if (level_indent[1] == NULL)
	  level_indent[1] = level_indent[0];
     if (level_end[0] == NULL)
	  level_end[0] = "";
     if (level_end[1] == NULL)
	  level_end[1] = "";

     init_lex(debug > 2);
     init_parse();
}

int
main(int argc, char **argv)
{
     int status = EX_OK;

     set_program_name(argv[0]);
     po.po_program_name = (char*) program_name;

     setlocale(LC_ALL, "");
     bindtextdomain(PACKAGE, LOCALEDIR);
     textdomain(PACKAGE);

     register_output("gnu", gnu_output_handler, NULL);
     register_output("posix", posix_output_handler, NULL);
     register_output("dot", dot_output_handler, NULL);

     symbol_map = SM_FUNCTIONS|SM_STATIC|SM_UNDEFINED;

     if (getenv("POSIXLY_CORRECT")) {
	  if (select_output_driver("posix")) {
	       error(0, 0, _("INTERNAL ERROR: %s: No such output driver"),
		     "posix");
	       abort();
	  }
	  output_init();
     }

     optfilepath = xstrdup(default_optfilepath);
     parseopt_from_env();
     parse_rc();
     if (parseopt_getopt (&po, argc, argv) == OPT_ERR)
	  error(EX_SOFTWARE, 0, _("parseopt_getopt failed"));
     parseopt_argv(&po, &argc, &argv);

     if (token_stack_length == 0)
	  error(EX_USAGE, 0, _("argument to -p cannot be 0"));

     if (print_as_tree) {
	  set_level_indent("0=  "); /* two spaces */
	  set_level_indent("1=| ");
	  set_level_indent("end0=+-");
	  set_level_indent("end1=\\\\-");
     } else {
	  level_indent[0] = level_indent[1] = NULL;
	  level_end[0] = level_end[1] = NULL;
     }

     if (!no_main_option)
	  set_default_starter();

     if (print_option == 0)
	  print_option = PRINT_TREE;

     init();

     while (argc--) {
	  if (source(*argv++) == 0)
	       yyparse();
	  else
	       status = EX_SOFT;
     }

     if (input_file_count == 0)
	     error(EX_USAGE, 0, _("no input files"));

     output();
     return status;
}
