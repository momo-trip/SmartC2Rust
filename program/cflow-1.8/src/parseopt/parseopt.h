/*
 * parseopt - generic option parser library
 * Copyright (C) 2023-2025 Sergey Poznyakoff
 *
 * Parseopt is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * Parseopt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with parseopt. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PARSEOPT_H
# define _PARSEOPT_H

#include "stddef.h"
#include "wordwrap.h"

#define OPTFLAG_DEFAULT      OPTFLAG_OPTION
#define OPTFLAG_OPTION       0x0000
#define OPTFLAG_ALIAS        0x0001
#define OPTFLAG_DOC          0x0002
#define OPTFLAG_HIDDEN       0x0004
#define OPTFLAG_ARG_OPTIONAL 0x0008
#define OPTFLAG_LIST         0x0010
#define OPTFLAG_SUBLIST      0x0020
#define OPTFLAG_BOOL         0x0040
#define OPTFLAG_EARLY        0x0080

#define OPTFLAG_IS_SET       0x8000

#define OPTDOC_MASK (OPTFLAG_DOC|OPTFLAG_LIST|OPTFLAG_SUBLIST)

struct parseopt;

struct optdef
{
  char *opt_name;        /* Option name */
  char *opt_argdoc;      /* Argument description. */
  int opt_flags;         /* Flags (see above). */
  int opt_code;          /* Option code.  If 0, the ASCII code of the short
			    option version will be used. */
  char *opt_doc;         /* Option documentation string. */
  void *opt_ptr;         /* Data pointer. */
  int (*opt_set) (struct parseopt *, struct optdef *, char *);
			 /* Function to set the option */

  /* Used internally: */
  struct optdef *opt_head; /* Index of option cluster head.
			      Used when sorting for help output. */
};

static inline int
optdef_is_end (struct optdef const *opt)
{
  return opt->opt_name == NULL && opt->opt_flags == 0;
}

static inline int
optdef_is_doc_string (struct optdef const *opt)
{
  return opt->opt_flags & OPTFLAG_DOC;
}

static inline int
optdef_is_group_header (struct optdef const *opt)
{
  return optdef_is_doc_string (opt) && !(opt->opt_flags & OPTFLAG_SUBLIST);
}

static inline int
optdef_is_option_doc (struct optdef const *opt)
{
  return optdef_is_doc_string (opt) && (opt->opt_flags & OPTFLAG_SUBLIST);
}

static inline int
optdef_is_option (struct optdef const *opt)
{
  return !optdef_is_doc_string (opt) && opt->opt_name != NULL;
}

static inline int
optdef_is_hidden (struct optdef const *opt)
{
  return opt->opt_flags & OPTFLAG_HIDDEN;
}

static inline int
optdef_is_short_option (struct optdef const *opt)
{
  return optdef_is_option (opt) && opt->opt_name[1] == 0;
}

static inline int
optdef_is_long_option (struct optdef const *opt)
{
  return optdef_is_option (opt) && opt->opt_name[1] != 0;
}

#define PARSEOPT_DEFAULT        0
/* Don't ignore the first element of ARGV.  By default it is the program
   name */
#define PARSEOPT_ARGV0          0x0001
/* Ignore command line errors. */
#define PARSEOPT_IGNORE_ERRORS  0x0002
/* Don't order arguments so that options come first. */
#define PARSEOPT_IN_ORDER       0x0004
/* Don't provide standard options: -?, --help, --usage */
#define PARSEOPT_NO_STDOPT      0x0008
/* Don't exit on errors */
#define PARSEOPT_NO_ERREXIT     0x0010
/* Return non-option arguments as well. Implies PARSEOPT_IN_ORDER. */
#define PARSEOPT_NONOPT_ARG     0x0020
/* Single-dash (find-style) options. */
#define PARSEOPT_SINGLE_DASH    0x0040

#define PARSEOPT_USER_MASK      0x0fff
/* Used internally.  Never set this! */
#define PARSEOPT_EARLY          0x1000
#define PARSEOPT_INITIALIZED    0x8000

/* Message priorities for use as second argument to po_error. */
enum
  {
    PO_MSG_ERR,  /* Error message. */
    PO_MSG_INFO  /* Informational message.  Currently used when listing
		    ambiguous option possibilities. */
  };

struct parseopt
{
  /* User-supplied data: */
  char *po_descr;		/* Program description. */
  char *po_argdoc;		/* Argument docstring. */
  struct optdef **po_optdef;	/* Option definitions. */
  int po_flags;                 /* Parseopt flags. */

  char *po_program_name;
  char **po_program_args;
  char *po_package_name;
  char *po_package_url;
  char *po_bugreport_address;
  char *po_general_help;

  void (*po_init_hook) (struct parseopt *);
  void (*po_help_hook) (WORDWRAP_FILE, struct parseopt *);
  void (*po_version_hook) (WORDWRAP_FILE, struct parseopt *);
  void (*po_prog_doc_hook) (WORDWRAP_FILE, struct parseopt *);

  void *po_hook_data;           /* Data to use by hooks and po_setopt. */

  int po_ex_usage;		/* Exit code in case of usage error. */
  void (*po_error) (struct parseopt *, int, char const *, ...);
				/* Error reporting function. */

  void (*po_setopt) (struct parseopt *, int, char *);

  char *po_negation;            /* Negation prefix for boolean options.
				   Defaults to "no-". */

  /* Internal fields: */
  struct optdef **po_optgrp;    /* Option groups. */
  struct optdef **po_optptr;    /* Pointers to proper option elements in
				   po_optdef. */
  int *po_optidx;		/* Options indexed in lexicographical order. */
  size_t po_optcount;		/* Number of elements in optidx
				   (actual options). */

  /* Fields initialized by parseopt_init: */
  int po_argc;			/* Number of arguments. */
  char **po_argv;		/* Argument vector. */
  struct optdef *po_arg_set;    /* A pointer to an entry in po_optdef, such
				   that it has opt_code == OPT_ARG and
				   set_opt != NULL, or NULL, if no such entry
				   is found. */

  /* Fields initialized by parseopt_init and maintained by
     parseopt_next: */
  int po_argi;			/* Index of next argument in argv[]. */
  int po_eopt;			/* End of options: if true, treat remaining
				   argv as positional arguments. */
  char const *po_optname;	/* Current option. */
  int po_dash_count;		/* Is it a long option? */
  char po_opt_buf[2];

  /* The following two keep the position of the first non-optional
     argument and the number of contiguous non-optional arguments
     after it. Obviously, the following holds true:

     po_arg_start + po_arg_count == po_argi
   */
  int po_arg_start;
  int po_arg_count;
  int po_permuted;		/* Whether the arguments were permuted */

};

int parseopt_init (struct parseopt *po, int argc, char **argv);
void parseopt_free (struct parseopt *po);

static inline int parseopt_eopt (struct parseopt *po) { return po->po_eopt; }

/* Special return values for parseopt_next. */
enum
  {
    OPT_ARG  = 1,  /* Non-option argument */
    OPT_END  = 0,  /* End opt options reached. */
    OPT_ERR  = -1, /* Error. Returned only if PARSEOPT_NO_ERREXIT bit is set.
		      The appropriate diagnostic has been issued via the
		      po_error callback. */
    OPT_NEXT = -2, /* Used internally. */
  };

int parseopt_next (struct parseopt *po, char **ret_arg);
void parseopt_argv (struct parseopt *po, int *argc, char ***argv);
int parseopt_getopt (struct parseopt *po, int argc, char **argv);
char const *parseopt_lookahead (struct parseopt *po);
void parseopt_skip (struct parseopt *po);
int parseopt_env_enable (struct parseopt *po, char **nv, size_t nc, int *used);
void parseopt_help_fd (struct parseopt *po, int fd);
static inline void parseopt_help (struct parseopt *po) {
	parseopt_help_fd (po, 1);
}
void parseopt_usage_fd (struct parseopt *po, int fd);
static inline void parseopt_usage (struct parseopt *po) {
	parseopt_usage_fd (po, 1);
}
void parseopt_version_fd (struct parseopt *po, int fd);
static inline void parseopt_version (struct parseopt *po) {
	parseopt_version_fd (po, 1);
}

struct optdef *parseopt_optdef_by_code (struct parseopt *po, int code);
struct optdef *parseopt_optdef_by_name (struct parseopt *po, char const *name);
int parseopt_is_set (struct parseopt *po, int code);

char const *option_dash (struct parseopt *po, struct optdef *opt);

int optset_int (struct parseopt *po, struct optdef *opt, char *arg);
int optset_uint (struct parseopt *po, struct optdef *opt, char *arg);
int optset_long (struct parseopt *po, struct optdef *opt, char *arg);
int optset_ulong (struct parseopt *po, struct optdef *opt, char *arg);
int optset_llong (struct parseopt *po, struct optdef *opt, char *arg);
int optset_ullong (struct parseopt *po, struct optdef *opt, char *arg);
int optset_size_t (struct parseopt *po, struct optdef *opt, char *arg);

int optset_incr (struct parseopt *po, struct optdef *opt, char *arg);
int optset_true (struct parseopt *po, struct optdef *opt, char *arg);
int optset_false (struct parseopt *po, struct optdef *opt, char *arg);

int optset_string (struct parseopt *po, struct optdef *opt, char *arg);
int optset_string_copy (struct parseopt *po, struct optdef *opt, char *arg);
int optset_string_alloc (struct parseopt *po, struct optdef *opt, char *arg);
int optset_bool (struct parseopt *po, struct optdef *opt, char *arg);

struct parseopt_help_format
{
  unsigned short_opt_col;
  unsigned long_opt_col;
  unsigned doc_opt_col;
  unsigned header_col;
  unsigned opt_doc_col;
  unsigned sublist_col;
  unsigned sublist_doc_col;
  unsigned usage_indent;
  unsigned rmargin;
  unsigned dup_args;
  unsigned dup_args_note;
};

#define PARSEOPT_HELP_FORMAT_INITIALIZER { \
    .short_opt_col   = 2,			   \
    .long_opt_col    = 6,			   \
    .doc_opt_col     = 2,			   \
    .header_col      = 1,			   \
    .opt_doc_col     = 29,			   \
    .sublist_col     = 29,			   \
    .sublist_doc_col = 36,			   \
    .usage_indent    = 12,			   \
    .rmargin         = 79,			   \
    .dup_args        = 0,			   \
    .dup_args_note   = 1			   \
 }

extern struct parseopt_help_format parseopt_help_format;

#endif
