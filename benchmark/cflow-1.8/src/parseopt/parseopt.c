/* parseopt - generic option parser library
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
#include "parseconf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <assert.h>
#include "parseopt.h"

static char const *dash_str[] = { "", "-", "--" };

char const *
option_dash (struct parseopt *po, struct optdef *opt)
{
  return dash_str[((po->po_flags & PARSEOPT_SINGLE_DASH) ||
		   strlen (opt->opt_name) == 1) ? 1 : 2];
}

static struct optdef *
option_find_short (struct parseopt *po, struct optdef **orig_opt,
		   char **argptr, int *exparg)
{
  int i;

  for (i = 0; i < po->po_optcount; i++)
    {
      struct optdef *opt = po->po_optptr[po->po_optidx[i]];
      size_t namelen = strlen (opt->opt_name);

      if (namelen == 1 && opt->opt_name[0] == po->po_optname[0])
	{
	  *orig_opt = opt;
	  while (opt->opt_flags & OPTFLAG_ALIAS)
	    opt--;
	  if (opt->opt_argdoc && po->po_optname[1])
	    {
	      *exparg = 1;
	      *argptr = (char *) po->po_optname + 1;
	      po->po_opt_buf[0] = po->po_optname[0];
	      po->po_opt_buf[1] = 0;
	      po->po_optname = po->po_opt_buf;
	    }
	  else
	    {
	      *exparg = 0;
	      if (opt->opt_flags & OPTFLAG_BOOL)
		*argptr = "1";
	      else
		*argptr = NULL;
	    }
	  return opt;
	}
    }

  po->po_error (po, PO_MSG_ERR,
		_("unrecognized option '-%c'"), po->po_optname[0]);

  return NULL;
}

enum neg_match
  {
    neg_not_tried,
    neg_nomatch,
    neg_match_inexact,
    neg_match_exact
  };

static enum neg_match
negmatch (struct parseopt *po, struct optdef const *opt,
	  char const *optstr, size_t optlen)
{
  if (opt->opt_flags & OPTFLAG_BOOL)
    {
      size_t neglen = strlen (po->po_negation);
      size_t len = strlen (opt->opt_name);
      if (optlen <= neglen + len
	  && memcmp (optstr, po->po_negation, neglen) == 0
	  && memcmp (optstr + neglen, opt->opt_name,
		     optlen - neglen) == 0)
	{
	  return (optlen == neglen + len) ? neg_match_exact : neg_match_inexact;        }
    }
  return neg_nomatch;
}

static struct optdef *
option_find_long (struct parseopt *po, struct optdef **orig_opt, char **argptr,
		  int *exparg)
{
  size_t optlen = strcspn (po->po_optname, "=");
  int i;
  int found = 0;
  struct optdef *found_opt = NULL;
  int negated = 0;

  for (i = 0; i < po->po_optcount; i++)
    {
      struct optdef *opt = po->po_optptr[po->po_optidx[i]];
      size_t namelen = strlen (opt->opt_name);
      enum neg_match neg = neg_not_tried;

      /* Ignore single-letter options when used with -- */
      if (namelen == 1 && !(po->po_flags & PARSEOPT_SINGLE_DASH))
	continue;

      if ((optlen <= namelen &&
	   memcmp (opt->opt_name, po->po_optname, optlen) == 0)
	  || (neg = negmatch (po, opt, po->po_optname, optlen)) != neg_nomatch)
	{
	  switch (found)
	    {
	    case 0:
	      found_opt = opt;
	      found++;
	      negated = neg > neg_nomatch;
	      if (optlen == namelen || neg == neg_match_exact)
		{
		  /* Exact match */
		  goto end;
		}
	      break;

	    case 1:
	      found++;
	      po->po_error (po, PO_MSG_ERR,
			    _("option '%s%*.*s' is ambiguous; possibilities:"),
			    dash_str[po->po_dash_count],
			    optlen, optlen, po->po_optname);
	      po->po_error (po, PO_MSG_INFO, "%s%s%s",
			    dash_str[po->po_dash_count],
			    negated ? po->po_negation : "",
			    found_opt->opt_name);
	      if (!negated &&
		  negmatch (po, found_opt, po->po_optname, optlen) != neg_nomatch)
		po->po_error (po, PO_MSG_INFO, "%s%s%s",
			      dash_str[po->po_dash_count],
			      po->po_negation,
			      found_opt->opt_name);
	      /* fall through */
	    case 2:
	      po->po_error (po, PO_MSG_INFO, "%s%s%s",
		       dash_str[po->po_dash_count],
		       neg > neg_nomatch ? po->po_negation : "",
		       opt->opt_name);
	      if (neg == neg_not_tried &&
		  negmatch (po, opt, po->po_optname, optlen) != neg_nomatch)
		po->po_error (po, PO_MSG_INFO, "%s%s%s",
			 dash_str[po->po_dash_count],
			 po->po_negation,
			 found_opt->opt_name);
	    }
	}
    }
end:
  switch (found)
    {
    case 0:
      po->po_error (po, PO_MSG_ERR, _("unrecognized option '%s%*.*s'"),
		    dash_str[po->po_dash_count], optlen, optlen,
		    po->po_optname);
      break;

    case 1:
      *orig_opt = found_opt;
      while (found_opt->opt_flags & OPTFLAG_ALIAS)
	found_opt--;
      if (po->po_optname[optlen] == '=')
	{
	  *argptr = (char *) po->po_optname + optlen + 1;
	  *exparg = 1;
	}
      else
	{
	  *exparg = 0;
	  if (found_opt->opt_flags & OPTFLAG_BOOL)
	    *argptr = negated ? "0" : "1";
	  else
	    *argptr = NULL;
	}
      return found_opt;

    case 2:
      break;
    }
  return NULL;
}

static void
permute (struct parseopt *po)
{
  if (!(po->po_flags & PARSEOPT_IN_ORDER) && po->po_arg_count)
    {
      /* Array to save arguments in */
      char *save[2];
      /* Number of arguments processed (at most two) */
      int n = po->po_argi - (po->po_arg_start + po->po_arg_count);
      assert (n <= 2);

      /* Store the processed elements away */
      save[0] = po->po_argv[po->po_arg_start + po->po_arg_count];
      if (n == 2)
	save[1] = po->po_argv[po->po_arg_start + po->po_arg_count + 1];

      /* Shift the array */
      memmove (po->po_argv + po->po_arg_start + n,
	       po->po_argv + po->po_arg_start,
	       po->po_arg_count * sizeof (po->po_argv[0]));

      /* Place saved elements in the vacating slots */
      po->po_argv[po->po_arg_start] = save[0];
      if (n == 2)
	po->po_argv[po->po_arg_start + 1] = save[1];

      /* Fix up start index */
      po->po_arg_start += n;
      po->po_permuted = 1;
    }
}

char const *
parseopt_lookahead (struct parseopt *po)
{
  if (!((po->po_flags & PARSEOPT_SINGLE_DASH)) &&
      po->po_dash_count == 1 && po->po_optname && po->po_optname[1])
    return po->po_optname;
  else
    return po->po_argv[po->po_argi];
}

void
parseopt_skip (struct parseopt *po)
{
  if (po->po_argi == po->po_argc || po->po_eopt)
    return;
  po->po_argi++;
}

static int
parseopt_next_internal (struct parseopt *po, char **ret_arg)
{
  char *opt_arg;

  if (!((po->po_flags & PARSEOPT_SINGLE_DASH)) &&
      po->po_dash_count == 1 && po->po_optname && po->po_optname[1])
    po->po_optname++;
  else
    {
      permute (po);

      for (;;)
	{
	  char *arg;

	  po->po_dash_count = 0;

	  if (po->po_argi == po->po_argc || po->po_eopt)
	    break;

	  arg = po->po_argv[po->po_argi++];

	  if (*arg == '-')
	    {
	      arg++;
	      po->po_dash_count++;
	      if (*arg == '-')
		{
		  arg++;
		  po->po_dash_count++;
		  if (*arg == 0)
		    {
		      po->po_dash_count = 0;
		      permute (po);
		      po->po_eopt = 1;
		      break;
		    }
		  if (po->po_flags & PARSEOPT_SINGLE_DASH)
		    {
		      arg--;
		      po->po_dash_count--;
		    }
		}
	      po->po_optname = arg;
	      break;
	    }
	  else if (po->po_flags & PARSEOPT_IN_ORDER)
	    {
	      po->po_argi--;
	      break;
	    }
	  else
	    {
	      if (!po->po_permuted && po->po_arg_count == 0)
		po->po_arg_start = po->po_argi - 1;
	      po->po_arg_count++;
	    }
	}
    }

  if (po->po_dash_count)
    {
      struct optdef *opt, *orig_opt;
      int has_arg;
      opt = ((po->po_dash_count == 2 || po->po_flags & PARSEOPT_SINGLE_DASH)
	     ? option_find_long : option_find_short) (po, &orig_opt, &opt_arg,
						      &has_arg);
      if (!opt)
	{
	  if (po->po_flags & PARSEOPT_NO_ERREXIT)
	    return OPT_ERR;
	  exit (po->po_ex_usage);
	}
      if (opt->opt_argdoc)
	{
	  if (opt_arg)
	    /* nothing */;
	  else if (opt->opt_flags & OPTFLAG_ARG_OPTIONAL)
	    opt_arg = NULL;
	  else if (po->po_argi == po->po_argc)
	    {
	      po->po_error (po, PO_MSG_ERR,
			    _("option '%s%s' requires argument"),
			    dash_str[po->po_dash_count], orig_opt->opt_name);
	      if (po->po_flags & PARSEOPT_NO_ERREXIT)
		return OPT_ERR;
	      exit (po->po_ex_usage);
	    }
	  else
	    opt_arg = po->po_argv[po->po_argi++];
	  if (!opt->opt_set)
	    *ret_arg = opt_arg;
	}
      else if (has_arg)
	{
	  po->po_error (po, PO_MSG_ERR,
			_("option '%s%s' does not take argument"),
			dash_str[po->po_dash_count], orig_opt->opt_name);
	  if (po->po_flags & PARSEOPT_NO_ERREXIT)
	    return OPT_ERR;
	  exit (po->po_ex_usage);
	}

      if ((!!(po->po_flags & PARSEOPT_EARLY)) ^
	  (!!(opt->opt_flags & OPTFLAG_EARLY)))
	return OPT_NEXT;

      opt->opt_flags |= OPTFLAG_IS_SET;

      if (opt->opt_set)
	{
	  if (opt->opt_set (po, opt, opt_arg))
	    {
	      po->po_error (po, PO_MSG_ERR,
			    _("bad value for option '%s%s'"),
			    dash_str[po->po_dash_count], orig_opt->opt_name);
	      if (po->po_flags & PARSEOPT_NO_ERREXIT)
		return OPT_ERR;
	      exit (po->po_ex_usage);
	    }
	  return OPT_NEXT;
	}
      return opt->opt_code;
    }

  if (!po->po_permuted)
    po->po_arg_start = po->po_argi - po->po_arg_count;

  opt_arg = po->po_argv[po->po_arg_start];

  if (opt_arg && (po->po_flags & PARSEOPT_NONOPT_ARG))
    {
      po->po_argi++;
      if (po->po_flags & PARSEOPT_EARLY)
	return OPT_NEXT;
      if (po->po_arg_set)
	{
	  if (po->po_arg_set->opt_set (po, po->po_arg_set, opt_arg))
	    {
	      po->po_error (po, PO_MSG_ERR,
			    _("invalid argument '%s'"), opt_arg);
	      if (po->po_flags & PARSEOPT_NO_ERREXIT)
		return OPT_ERR;
	      exit (po->po_ex_usage);
	    }
	  return OPT_NEXT;
	}
      else
	{
	  *ret_arg = opt_arg;
	  return OPT_ARG;
	}
    }

  *ret_arg = opt_arg;

  return OPT_END;
}

int
parseopt_next (struct parseopt *po, char **ret_arg)
{
  int rc;
  do
    rc = parseopt_next_internal (po, ret_arg);
  while (rc == OPT_NEXT);
  return rc;
}

void
parseopt_argv (struct parseopt *po, int *argc, char ***argv)
{
  *argc = po->po_argc - po->po_arg_start;
  *argv = po->po_argv + po->po_arg_start;
}

void
parseopt_error (struct parseopt *po, int pri, char const *fmt, ...)
{
  va_list ap;

  if (po->po_flags & PARSEOPT_IGNORE_ERRORS)
    return;

  if (po->po_program_name && pri == PO_MSG_ERR)
    fprintf (stderr, "%s: ", po->po_program_name);
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);
  fputc ('\n', stderr);
}

static int
find_short_name (struct optdef *opt)
{
  do
    {
      if (optdef_is_short_option (opt))
	return opt->opt_name[0];
    }
  while (!optdef_is_end (++opt) && (opt->opt_flags & OPTFLAG_ALIAS));
  return 0;
}

static int
optidx_slot (struct parseopt *po, int n, struct optdef *opt)
{
  int i, j, m;

  if (n == 0)
    return 0;
  i = 0;
  j = n;
  while (i < j)
    {
      m = i + ((j - i) / 2);
      if (strcmp (po->po_optptr[po->po_optidx[m]]->opt_name,
		  opt->opt_name) <= 0)
	i = m + 1;
      else
	j = m;
    }
  return i;
}

static size_t
collect_optdef (struct parseopt *po, struct optdef *opt, size_t n)
{
  size_t i, j;

  for (i = 0; !optdef_is_end (&opt[i]); i++)
    {
      if (optdef_is_option (&opt[i]))
	{
	  po->po_optptr[n] = &opt[i];
	  j = optidx_slot (po, n, &opt[i]);
	  if (j < n)
	    memmove (po->po_optidx + j + 1, po->po_optidx + j,
		     (n - j) * sizeof (po->po_optidx[0]));
	  po->po_optidx[j] = n;
	  n++;
	}
    }
  return n;
}

char *parseopt_negation_prefix = "no-";

/* Scan flags */
#define SCANF_V 0x1   /* Single-letter V option present. */

static void
prepare_optdef (struct parseopt *po, struct optdef *opt, int *scan_flags)
{
  int i;

  /* Ensure sane start of options. */
  opt[0].opt_flags &= ~(OPTFLAG_ALIAS|OPTFLAG_SUBLIST);

  /* Count actual options.  Check each of them for consistency and set
     opt_code field where needed. */
  for (i = 0; !optdef_is_end (&opt[i]); i++)
    {
      if (optdef_is_option (&opt[i]))
	{
	  po->po_optcount++;

	  if (opt[i].opt_code == OPT_ARG)
	    {
	      opt[i].opt_flags = OPTFLAG_HIDDEN;
	      po->po_flags |= PARSEOPT_NONOPT_ARG;
	      if (opt[i].opt_set)
		po->po_arg_set = &opt[i];
	      continue;
	    }

	  if (po->po_version_hook && strcmp (opt[i].opt_name, "V") == 0)
	    {
	      *scan_flags |= SCANF_V;
	    }

	  if (!(opt[i].opt_flags & OPTFLAG_ALIAS))
	    {
	      opt[i].opt_flags &= ~OPTFLAG_IS_SET;
	      if (opt[i].opt_code == 0)
		opt[i].opt_code = find_short_name (&opt[i]);
	      if (opt[i].opt_flags & OPTFLAG_BOOL)
		{
		  opt[i].opt_flags &= ~OPTFLAG_ARG_OPTIONAL;
		  opt[i].opt_argdoc = NULL;
		  if (opt[i].opt_ptr)
		    opt[i].opt_set = optset_bool;
		  if (!po->po_negation)
		    po->po_negation = parseopt_negation_prefix;
		}
	      if (opt[i].opt_flags & OPTFLAG_EARLY)
		   po->po_flags |= PARSEOPT_EARLY;
	    }
	}
    }
}

static int
set_help (struct parseopt *po, struct optdef *opt, char *arg)
{
  parseopt_help (po);
  exit (0);
}

static int
set_usage (struct parseopt *po, struct optdef *opt, char *arg)
{
  parseopt_usage (po);
  exit (0);
}

static int
set_version (struct parseopt *po, struct optdef *opt, char *arg)
{
  parseopt_version (po);
  exit (0);
}

static struct optdef stdopt[] = {
  {
    .opt_doc = N_("Informative options"),
    .opt_flags = OPTFLAG_DOC
  },
  {
    .opt_name = "help",
    .opt_doc = N_("produce this help text"),
    .opt_set = set_help
  },
  {
    .opt_name = "?",
    .opt_flags = OPTFLAG_ALIAS
  },
  {
    .opt_name = "usage",
    .opt_doc = N_("give a short usage message"),
    .opt_set = set_usage
  },

  { NULL }
};

static struct optdef veropt[] = {
  {
    .opt_name = "version",
    .opt_doc = N_("print program version"),
    .opt_set = set_version
  },
  { NULL }
};

static struct optdef veropt2[] = {
  {
    .opt_name = "version",
    .opt_doc = N_("print program version"),
    .opt_set = set_version
  },
  {
    .opt_name = "V",
    .opt_flags = OPTFLAG_ALIAS
  },
  { NULL }
};

static inline struct optdef *
_parseopt_optgroup (struct parseopt *po, int i)
{
  if (po->po_optdef)
    return po->po_optdef[i];
  return NULL;
}

int
parseopt_init0 (struct parseopt *po)
{
  size_t i, j, n;
  struct optdef *opt;
  int sf = (po->po_flags & PARSEOPT_SINGLE_DASH) ? SCANF_V : 0;

  po->po_flags &= PARSEOPT_USER_MASK;
  if (po->po_ex_usage == 0)
    po->po_ex_usage = EX_USAGE;
  if (!po->po_error)
    po->po_error = parseopt_error;

  /* Prepare options. */
  for (n = 0; _parseopt_optgroup (po, n) != NULL; n++)
    ;
  if (!(po->po_flags & PARSEOPT_NO_STDOPT))
    n++;
  if (po->po_version_hook)
    n++;

  po->po_optgrp = calloc (n + 1, sizeof (po->po_optgrp[0]));
  if (po->po_optgrp == NULL)
    return -1;

  po->po_optcount = 0;
  for (i = 0; (opt = _parseopt_optgroup (po, i)) != NULL; i++)
    prepare_optdef (po, po->po_optgrp[i] = opt, &sf);
  if (!(po->po_flags & PARSEOPT_NO_STDOPT))
    prepare_optdef (po, po->po_optgrp[i++] = stdopt, &sf);
  if (po->po_version_hook)
    prepare_optdef (po, po->po_optgrp[i++] = (sf & SCANF_V) ? veropt : veropt2,
		    &sf);

  po->po_optptr = calloc (po->po_optcount, sizeof (po->po_optptr[0]));
  if (po->po_optptr == NULL)
    {
      free (po->po_optgrp);
      return -1;
    }
  po->po_optidx = calloc (po->po_optcount, sizeof (po->po_optidx[0]));
  if (po->po_optidx == NULL)
    {
      free (po->po_optgrp);
      free (po->po_optptr);
      return -1;
    }

  /* Collect and index options. */
  j = 0;
  for (i = 0; i < n; i++)
    j = collect_optdef (po, po->po_optgrp[i], j);

  assert (j == po->po_optcount);

  if (po->po_flags & PARSEOPT_NONOPT_ARG)
    po->po_flags |= PARSEOPT_IN_ORDER;

  po->po_flags |= PARSEOPT_INITIALIZED;

  return 0;
}

int
parseopt_init (struct parseopt *po, int argc, char **argv)
{
  if (!(po->po_flags & PARSEOPT_INITIALIZED))
    if (parseopt_init0 (po))
      return -1;

  po->po_argc = argc;
  po->po_argv = argv;
  po->po_argi = 0;

  if (!(po->po_flags & PARSEOPT_ARGV0))
    {
      po->po_argi++;
      if (!po->po_program_name)
	{
	  char *p = strrchr (po->po_argv[0], '/');
	  if (p)
	    p++;
	  else
	    p = (char*) po->po_argv[0];
	  if (strlen (p) > 3 && memcmp (p, "lt-", 3) == 0)
	    p += 3;
	  po->po_program_name = p;
	}
    }

  po->po_eopt = 0;
  po->po_arg_start = po->po_argi;
  po->po_arg_count = 0;
  po->po_permuted = 0;

  return 0;
}

void
parseopt_free (struct parseopt *po)
{
  if (po->po_flags & PARSEOPT_INITIALIZED)
    {
      free (po->po_optgrp);
      free (po->po_optptr);
      free (po->po_optidx);
      po->po_flags &= ~PARSEOPT_INITIALIZED;
    }
}

int
parseopt_parse (struct parseopt *po)
{
  int c;
  char *p;

  if ((c = parseopt_next (po, &p)) > 0)
    {
      assert (po->po_setopt != NULL);
      do
	po->po_setopt (po, c, p);
      while ((c = parseopt_next (po, &p)) > 0);
    }
  return c;
}

int
parseopt_getopt (struct parseopt *po, int argc, char **argv)
{
  int c;

  if (parseopt_init (po, argc, argv))
    return OPT_ERR;

  if (po->po_flags & PARSEOPT_EARLY)
    {
      c = parseopt_parse (po);
      if (c != OPT_END)
	return c;
      po->po_flags &= ~PARSEOPT_EARLY;

      po->po_argi = (po->po_flags & PARSEOPT_ARGV0) ? 0 : 1;
      po->po_eopt = 0;
      po->po_arg_start = po->po_argi;
      po->po_arg_count = 0;
      po->po_permuted = 0;
    }
  if (po->po_init_hook)
    po->po_init_hook (po);
  return parseopt_parse (po);
}

struct optdef *
parseopt_optdef_by_code (struct parseopt *po, int code)
{
  size_t i;

  for (i = 0; i < po->po_optcount; i++)
    {
      struct optdef *opt = po->po_optptr[po->po_optidx[i]];
      if (opt->opt_code == code)
	return opt;
    }
  return NULL;
}

struct optdef *
parseopt_optdef_by_name (struct parseopt *po, char const *name)
{
  size_t i;

  for (i = 0; i < po->po_optcount; i++)
    {
      struct optdef *opt = po->po_optptr[po->po_optidx[i]];
      if (strcmp (opt->opt_name, name) == 0)
	{
	  while (opt->opt_flags & OPTFLAG_ALIAS)
	    opt--;
	  return opt;
	}
    }
  return NULL;
}

int
parseopt_is_set (struct parseopt *po, int code)
{
  struct optdef *opt = parseopt_optdef_by_code (po, code);
  if (opt)
    return !!(opt->opt_flags & OPTFLAG_IS_SET);
  return -1;
}
