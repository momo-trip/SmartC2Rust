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
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <assert.h>
#include "parseopt.h"

struct parseopt_help_format parseopt_help_format =
	PARSEOPT_HELP_FORMAT_INITIALIZER;

#define FMT(s) parseopt_help_format.s

enum usage_var_type
{
  usage_var_column,
  usage_var_bool
};

struct usage_var_def
{
  char *name;
  unsigned *valptr;
  enum usage_var_type type;
};

static struct usage_var_def usage_var[] = {
  {"short-opt-col",   &FMT(short_opt_col),   usage_var_column},
  {"header-col",      &FMT(header_col),      usage_var_column},
  {"opt-doc-col",     &FMT(opt_doc_col),     usage_var_column},
  {"usage-indent",    &FMT(usage_indent),    usage_var_column},
  {"rmargin",         &FMT(rmargin),         usage_var_column},
  {"dup-args",        &FMT(dup_args),        usage_var_bool},
  {"dup-args-note",   &FMT(dup_args_note),   usage_var_bool},
  {"long-opt-col",    &FMT(long_opt_col),    usage_var_column},
  {"doc-opt-col",     &FMT(doc_opt_col),     usage_var_column},
  {"sublist-col",     &FMT(sublist_col),     usage_var_column},
  {"sublist-doc-col", &FMT(sublist_doc_col), usage_var_column},
  {NULL}
};

static void
set_usage_var (struct parseopt *po, char const *text, char **end)
{
  struct usage_var_def *p;
  int boolval = 1;
  size_t len = strcspn (text, ",=");
  char *endp;

  if (len > 3 && memcmp (text, "no-", 3) == 0)
    {
      text += 3;
      len -= 3;
      boolval = 0;
    }

  endp = (char *) text + len;

  for (p = usage_var; p->name; p++)
    {
      if (strlen (p->name) == len && memcmp (p->name, text, len) == 0)
	break;
    }

  if (p->name)
    {
      if (p->type == usage_var_bool)
	{
	  if (*endp == '=')
	    po->po_error (po, PO_MSG_ERR,
			  _("error in ARGP_HELP_FMT:"
			    " improper usage of [no-]%s"), p->name);
	  else
	    *p->valptr = boolval;
	}
      else if (*endp == '=')
	{
	  unsigned long val;

	  errno = 0;
	  val = strtoul (text + len + 1, &endp, 10);
	  if (errno || (*endp && *endp != ','))
	    po->po_error (po, PO_MSG_ERR,
			  _("error in ARGP_HELP_FMT:"
			    " bad value for %s (near %s)"),
			  p->name,
			  endp ? endp : "end");
	  else if (val > UINT_MAX)
	    po->po_error (po, PO_MSG_ERR,
			  _("error in ARGP_HELP_FMT:"
			    " %s value is out of range"), p->name);
	  else
	    *p->valptr = val;
	}
      else
	po->po_error (po, PO_MSG_ERR,
		      _("%s: ARGP_HELP_FMT parameter"
			" requires a value"), p->name);
    }
  else
    {
      po->po_error (po, PO_MSG_ERR,
		    _("%s: unknown ARGP_HELP_FMT parameter"), text);
      if (*endp == '=')
	endp += strcspn (endp, ",");
    }
  *end = endp;
}

static void
init_usage_vars (struct parseopt *po)
{
  char *fmt, *p;

  fmt = getenv ("ARGP_HELP_FMT");
  if (!fmt || !*fmt)
    return;

  while (1)
    {
      set_usage_var (po, fmt, &p);
      if (*p == 0)
	break;
      else if (*p == ',')
	p++;
      else
	{
	  po->po_error (po, PO_MSG_ERR,
			_("ARGP_HELP_FMT: missing delimiter near %s"), p);
	  break;
	}
      fmt = p;
    }
}

static inline void
print_arg (WORDWRAP_FILE wf, struct optdef *opt, int delim, int *argsused)
{
  if (opt->opt_argdoc)
    {
      *argsused = 1;
      if (opt->opt_flags & OPTFLAG_ARG_OPTIONAL)
	wordwrap_printf (wf, "%c[%s]", delim, gettext (opt->opt_argdoc));
      else
	wordwrap_printf (wf, "%c%s", delim, gettext (opt->opt_argdoc));
    }
}

static inline struct optdef *
opt_unalias (struct optdef *opt)
{
  while (opt->opt_flags & OPTFLAG_ALIAS)
    opt--;
  return opt;
}

static void
parseopt_usage_std (struct parseopt *po, WORDWRAP_FILE wf)
{
  int i;
  struct optdef *opt, *orig;

  /* Print short options without arguments */
  for (i = 0; i < po->po_optcount; i++)
    {
      opt = po->po_optptr[po->po_optidx[i]];
      if (opt->opt_name[0] != 0 && opt->opt_name[1] == 0 &&
	  opt->opt_argdoc == NULL &&
	  !optdef_is_hidden (opt))
	break;
    }

  if (i < po->po_optcount)
    {
      wordwrap_puts (wf, "[-");
      wordwrap_puts (wf, opt->opt_name);
      while (++i < po->po_optcount)
	{
	  opt = po->po_optptr[po->po_optidx[i]];
	  if (opt->opt_name &&
	      opt->opt_name[0] != 0 && opt->opt_name[1] == 0 &&
	      opt->opt_argdoc == NULL &&
	      !optdef_is_hidden (opt))
	    wordwrap_puts (wf, opt->opt_name);
	}
      wordwrap_putc (wf, ']');
    }

  /* Print short options with arguments */
  for (i = 0; i < po->po_optcount; i++)
    {
      opt = po->po_optptr[po->po_optidx[i]];
      orig = opt_unalias (opt);
      if (opt->opt_name &&
	  opt->opt_name[0] != 0 && opt->opt_name[1] == 0 &&
	  orig->opt_argdoc != NULL &&
	  !optdef_is_hidden (orig))
	{
	  wordwrap_word_start (wf);
	  wordwrap_puts (wf, " [-");
	  wordwrap_puts (wf, opt->opt_name);
	  wordwrap_putc (wf, ' ');
	  wordwrap_puts (wf, gettext (orig->opt_argdoc));
	  wordwrap_putc (wf, ']');
	  wordwrap_word_end (wf);
	}
    }

  /* Print long options */
  for (i = 0; i < po->po_optcount; i++)
    {
      opt = po->po_optptr[po->po_optidx[i]];
      orig = opt_unalias (opt);
      if (opt->opt_name &&
	  opt->opt_name[0] != 0 && opt->opt_name[1] != 0 &&
	  !optdef_is_hidden (opt))
	{
	  wordwrap_word_start (wf);
	  wordwrap_write (wf, " [--", 4);
	  if (opt->opt_flags & OPTFLAG_BOOL)
	    wordwrap_printf (wf, "[%s]", po->po_negation);
	  wordwrap_puts (wf, opt->opt_name);
	  if (orig->opt_argdoc)
	    {
	      wordwrap_putc (wf, '=');
	      if (opt->opt_flags & OPTFLAG_ARG_OPTIONAL)
		wordwrap_putc (wf, '[');
	      wordwrap_puts (wf, gettext (orig->opt_argdoc));
	      if (opt->opt_flags & OPTFLAG_ARG_OPTIONAL)
		wordwrap_putc (wf, ']');
	    }
	  wordwrap_putc (wf, ']');
	  wordwrap_word_end (wf);
	}
    }
}

static void
parseopt_usage_sdash (struct parseopt *po, WORDWRAP_FILE wf)
{
  size_t i;

  for (i = 0; i < po->po_optcount; i++)
    {
      struct optdef *opt = po->po_optptr[po->po_optidx[i]];
      if (optdef_is_option (opt) && !optdef_is_hidden (opt))
	{
	  struct optdef *orig = opt_unalias (opt);

	  if (i > 0)
	    wordwrap_putc (wf, ' ');
	  wordwrap_word_start (wf);
	  wordwrap_write (wf, "[-", 2);
	  if (opt->opt_flags & OPTFLAG_BOOL)
	    wordwrap_printf (wf, "[%s]", po->po_negation);
	  wordwrap_puts (wf, opt->opt_name);
	  if (orig->opt_argdoc)
	    {
	      wordwrap_putc (wf, ' ');
	      if (opt->opt_flags & OPTFLAG_ARG_OPTIONAL)
		wordwrap_putc (wf, '[');
	      wordwrap_puts (wf, gettext (orig->opt_argdoc));
	      if (opt->opt_flags & OPTFLAG_ARG_OPTIONAL)
		wordwrap_putc (wf, ']');
	    }
	  wordwrap_putc (wf, ']');
	  wordwrap_word_end (wf);
	}
    }
}

void
parseopt_usage_fd (struct parseopt *po, int fd)
{
  WORDWRAP_FILE wf;

  init_usage_vars (po);
  wf = wordwrap_fdopen (fd);
  wordwrap_set_right_margin (wf, FMT(rmargin));
  wordwrap_printf (wf, "%s %s ", _("Usage:"), po->po_program_name);
  wordwrap_next_left_margin (wf, FMT(usage_indent));

  if (po->po_flags & PARSEOPT_SINGLE_DASH)
    parseopt_usage_sdash (po, wf);
  else
    parseopt_usage_std (po, wf);

  if (po->po_argdoc)
    wordwrap_printf (wf, " %s\n", gettext (po->po_argdoc));

  if (po->po_program_args)
    {
      size_t i;

      for (i = 0; po->po_program_args[i]; i++)
	{
	  wordwrap_set_left_margin (wf, 2);
	  wordwrap_next_left_margin (wf, FMT(usage_indent));
	  wordwrap_printf (wf, _("or: %s [OPTIONS...] %s\n"),
			   po->po_program_name,
			   gettext (po->po_program_args[i]));
	}
    }

  wordwrap_close (wf);
}

typedef int (*OPTCMP) (struct optdef const *optv, int *idx, int i, int j);

struct optsort
{
  struct optdef *optv;		/* Option definitions */
  size_t optc;			/* Number of elements in optv. */
  int *idx;			/* Sorted indices. */
  size_t opti;			/* Index of the next option definition. */
  OPTCMP cmp;
};

static inline size_t
min (size_t a, size_t b)
{
  return a < b ? a : b;
}

static void
merge (struct optdef const *optv, int *source, int *work,
       size_t left, size_t right, size_t end,
       OPTCMP cmp)
{
  size_t i = left;
  size_t j = right;
  size_t k;

  for (k = left; k < end; k++)
    {
      if (i < right
	  && (j >= end || cmp (optv, source, i, j) <= 0))
	work[k] = source[i++];
      else
	work[k] = source[j++];
    }
}

static int
optsort (struct optsort *ops, int n)
{
  int *tmp, *a, *b;
  size_t width;

  tmp = calloc (n, sizeof (tmp[0]));
  if (!tmp)
    return -1;

  a = ops->idx + ops->opti;
  b = tmp;
  for (width = 1; width < n; width <<= 1)
    {
      size_t i;
      int *t;

      for (i = 0; i < n; i += 2 * width)
	{
	  merge (ops->optv, a, b,
		 i, min (i + width, n), min (i + 2 * width, n),
		 ops->cmp);
	}
      t = a;
      a = b;
      b = t;
    }
  if (b != tmp)
    memcpy (b, tmp, n * sizeof (ops->idx[0]));
  free (tmp);
  return 0;
}

struct help_context
{
  struct parseopt *po;
  struct optdef *opts;
  int *idx;
  int nidx;
  int argsused;
  int prev_doc;
};

static void
print_option_std (WORDWRAP_FILE wf, struct help_context *ctx,
		  struct optdef *cur_opt,
		  size_t i, size_t next)
{
  struct optdef *opt;
  int delim = 0;
  int w;

  /* Format short options. */
  w = 0;
  for (; i < next; i++)
    {
      opt = &ctx->opts[ctx->idx[i]];
      if (optdef_is_hidden (opt))
	continue;
      if (opt->opt_name[1])
	break;
      if (w)
	wordwrap_write (wf, ", ", 2);
      wordwrap_putc (wf, '-');
      wordwrap_puts (wf, opt->opt_name);
      delim = ' ';
      if (FMT(dup_args))
	print_arg (wf, cur_opt, delim, &ctx->argsused);
      w = 1;
    }

  /* Format long options. */
  for (; i < next; i++)
    {
      opt = &ctx->opts[ctx->idx[i]];
      if (!optdef_is_hidden (opt))
	{
	  if (w)
	    wordwrap_write (wf, ", ", 2);
	  wordwrap_set_left_margin (wf, FMT(long_opt_col));
	  w = 0;
	  break;
	}
    }

  for (; i < next; i++)
    {
      opt = &ctx->opts[ctx->idx[i]];
      if (!optdef_is_hidden (opt))
	{
	  if (w)
	    wordwrap_write (wf, ", ", 2);
	  wordwrap_puts (wf, "--");
	  if (opt->opt_flags & OPTFLAG_BOOL)
	    wordwrap_printf (wf, "[%s]", ctx->po->po_negation);
	  wordwrap_puts (wf, opt->opt_name);
	  delim = '=';
	  if (FMT(dup_args))
	    print_arg (wf, cur_opt, delim, &ctx->argsused);
	  w = 1;
	}
    }
  if (delim && !FMT(dup_args))
    print_arg (wf, cur_opt, delim, &ctx->argsused);
}

static void
print_option_sdash (WORDWRAP_FILE wf, struct help_context *ctx,
		    struct optdef *cur_opt,
		    size_t i, size_t next)
{
  int w = 0;
  int delim = 0;
  for (; i < next; i++)
    {
      struct optdef *opt = &ctx->opts[ctx->idx[i]];
      if (!optdef_is_hidden (opt))
	{
	  if (w)
	    wordwrap_write (wf, ", ", 2);
	  wordwrap_putc (wf, '-');
	  if (opt->opt_flags & OPTFLAG_BOOL)
	    wordwrap_printf (wf, "[%s]", ctx->po->po_negation);
	  wordwrap_puts (wf, opt->opt_name);
	  delim = '=';
	  if (FMT(dup_args))
	    print_arg (wf, cur_opt, delim, &ctx->argsused);
	  w = 1;
	}
    }
  if (delim && !FMT(dup_args))
    print_arg (wf, cur_opt, delim, &ctx->argsused);
}

static int
print_option (WORDWRAP_FILE wf, struct help_context *ctx, int i)
{
  struct optdef *opt = &ctx->opts[ctx->idx[i]], *cur_opt = NULL;
  int next;
  struct optdef *head;

  if (optdef_is_hidden (opt))
    return i + 1;

  /* Mark the doc entry as list if its opt_name is set. */
  if ((opt->opt_flags & OPTFLAG_DOC) && opt->opt_name)
    opt->opt_flags |= OPTFLAG_LIST;
  /* Insert a newline between two contiguous plain doc entries,
     and between two entries of different doc type. */
  if (ctx->prev_doc != -1 &&
      ((ctx->prev_doc != (opt->opt_flags & OPTDOC_MASK)) ||
       (ctx->prev_doc == OPTFLAG_DOC && opt->opt_flags == OPTFLAG_DOC)))
    wordwrap_putc (wf, '\n');
  /* Save doc flags for use in the next call. */
  ctx->prev_doc = opt->opt_flags & OPTDOC_MASK;

  /* Handle a doc entry */
  if (opt->opt_flags & OPTFLAG_DOC)
    {
      wordwrap_set_right_margin (wf, FMT(rmargin));
      if (opt->opt_name)
	{
	  wordwrap_set_left_margin (wf,
				    (opt->opt_flags & OPTFLAG_SUBLIST)
				    ? FMT(sublist_col) : FMT(short_opt_col));
	  wordwrap_puts (wf, opt->opt_name);
	  wordwrap_set_left_margin (wf,
				    (opt->opt_flags & OPTFLAG_SUBLIST)
				    ? FMT(sublist_doc_col) : FMT(opt_doc_col));
	}
      else
	{
	  wordwrap_set_left_margin (wf,
				    (opt->opt_flags & OPTFLAG_SUBLIST)
				    ? FMT(sublist_col) : FMT(doc_opt_col));
	}
      if (opt->opt_doc)
	{
	  wordwrap_puts (wf, gettext (opt->opt_doc));
	  if (ctx->prev_doc)
	    wordwrap_putc (wf, '\n');
	}
      return i + 1;
    }

  /* A regular option. */

  /* Find next index and canonical option. */
  head = opt->opt_head;
  if (!(opt->opt_flags & OPTFLAG_ALIAS))
    cur_opt = opt;
  for (next = i + 1; next < ctx->nidx &&
	 optdef_is_option (opt = &ctx->opts[ctx->idx[next]]) &&
	 opt->opt_head == head; next++)
    {
      if (!(opt->opt_flags & OPTFLAG_ALIAS))
	cur_opt = opt;
    }

  assert(cur_opt != NULL);

  wordwrap_set_left_margin (wf, FMT(short_opt_col));

  if (ctx->po->po_flags & PARSEOPT_SINGLE_DASH)
    print_option_sdash (wf, ctx, cur_opt, i, next);
  else
    print_option_std (wf, ctx, cur_opt, i, next);


  wordwrap_set_left_margin (wf, FMT(opt_doc_col));
  if (cur_opt->opt_doc && cur_opt->opt_doc[0])
    wordwrap_puts (wf, gettext (cur_opt->opt_doc));
  wordwrap_putc (wf, '\n');

  return next;
}

static char *dup_args_note_text[2] = {
  N_("Mandatory or optional arguments to long options are also mandatory or "
     "optional for any corresponding short options."),
  N_("If an option has several forms, mandatory or optional argument is "
     "listed only once, after the last form.")
};

static int
optcmp (struct optdef const *optv, int *idx, int i, int j)
{
  struct optdef const *ap = &optv[idx[i]];
  struct optdef const *bp = &optv[idx[j]];

  if (ap->opt_head == bp->opt_head)
    {
      if (optdef_is_option (ap))
	{
	  if (optdef_is_option (bp))
	    {
	      if (ap->opt_name[1])
		{
		  if (!bp->opt_name[1])
		    return 1;
		}
	      else if (bp->opt_name[1])
		return -1;
	    }
	  else
	    return -1;
	}
      else if (optdef_is_option (bp))
	return 1;
    }
  else
    {
      ap = ap->opt_head;
      bp = bp->opt_head;
    }
  return strcmp (ap->opt_name ? ap->opt_name : "",
		 bp->opt_name ? bp->opt_name : "");
}

static void
sortnames (struct optsort *ops, int i, int j)
{
  i += ops->opti;
  j += ops->opti;
  while (j > i)
    {
      if (optcmp (ops->optv, ops->idx, i, j) <= 0)
	break;
      else
	{
	  int t = ops->idx[j];
	  ops->idx[j] = ops->idx[i];
	  ops->idx[i] = t;
	}
      j--;
    }
}

static inline void
sethead (struct optsort *ops, int i, int n)
{
  struct optdef *head = &ops->optv[ops->idx[ops->opti + i]];
  for (;i < n; i++)
    ops->optv[ops->idx[ops->opti + i]].opt_head = head;
}

static void
sort_group (struct optsort *ops)
{
  size_t i, j;
  struct optdef *opt;

  /* Skip documentation entries */
  while (ops->opti < ops->optc && optdef_is_doc_string (&ops->optv[ops->opti]))
    {
      ops->idx[ops->opti] = ops->opti;
      ops->opti++;
    }

  /* Copy optdefs, sorting each option cluster by option name.  Short
     option variants sort first.  Since clusters normally are very small,
     bubble sort is used. */
  i = j = 0;
  while (ops->opti + i < ops->optc &&
	 (optdef_is_option (opt = &ops->optv[ops->opti + i])
	  || optdef_is_option_doc (opt)))
    {
      ops->idx[ops->opti + i] = ops->opti + i;
      if (optdef_is_option (opt) && !(opt->opt_flags & OPTFLAG_ALIAS))
	{
	  if (i > j)
	    sethead (ops, j, i);
	  /* Start next cluster. */
	  j = i;
	}
      opt->opt_head = NULL;
      sortnames (ops, j, i);
      i++;
    }
  if (i > j)
    sethead (ops, j, i);

  /* Sort collected options */
  optsort (ops, i);

  ops->opti += i;
}

static void
sort_options (struct help_context *ctx)
{
  struct optsort opts;

  opts.cmp = optcmp;
  opts.optv = ctx->opts;
  for (opts.optc = 0; !optdef_is_end (&opts.optv[opts.optc]); opts.optc++);

  opts.idx = calloc (opts.optc, sizeof (opts.idx[0]));
  if (opts.idx == NULL)
    abort ();

  ctx->idx = opts.idx;
  ctx->nidx = opts.optc;

  for (opts.opti = 0; opts.opti < opts.optc;)
    sort_group (&opts);
}

static void
print_option_group (WORDWRAP_FILE wf, struct help_context *ctx)
{
  size_t i;

  sort_options (ctx);

  for (i = 0; i < ctx->nidx;)
    i = print_option (wf, ctx, i);

  free (ctx->idx);
}

void
parseopt_help_fd (struct parseopt *po, int fd)
{
  WORDWRAP_FILE wf;
  struct help_context ctx = {
    .po = po,
    .argsused = 0,
    .prev_doc = -1
  };
  size_t i;

  init_usage_vars (po);

  wf = wordwrap_fdopen (fd);

  wordwrap_printf (wf, _("usage: %s"), po->po_program_name);
  if (po->po_argdoc) {
	  wordwrap_putc (wf, ' ');
	  wordwrap_puts (wf, gettext (po->po_argdoc));
  } else if (po->po_optcount) {
	  wordwrap_putc (wf, ' ');
	  wordwrap_puts (wf, gettext ("[OPTIONS...]"));
  }

  wordwrap_putc (wf, '\n');

  if (po->po_program_args)
    {
      for (i = 0; po->po_program_args[i]; i++)
	{
	  wordwrap_set_left_margin (wf, 3);
	  wordwrap_next_left_margin (wf, FMT(usage_indent));
	  wordwrap_printf (wf, _("or: %s %s\n"), po->po_program_name,
			   gettext (po->po_program_args[i]));
	}
      wordwrap_set_left_margin (wf, 0);
    }

  wordwrap_set_right_margin (wf, FMT(rmargin));
  if (po->po_descr)
    wordwrap_puts (wf, gettext (po->po_descr));
  if (po->po_prog_doc_hook)
    {
      if (po->po_descr)
	wordwrap_para (wf);
      po->po_prog_doc_hook (wf, po);
    }
  wordwrap_para (wf);

  for (i = 0; po->po_optgrp[i]; i++)
    {
      ctx.opts = po->po_optgrp[i];
      print_option_group (wf, &ctx);
    }

  wordwrap_para (wf);
  if (ctx.argsused && !FMT(dup_args) && FMT(dup_args_note))
    {
      wordwrap_set_left_margin (wf, 0);
      wordwrap_set_right_margin (wf, FMT(rmargin));
      wordwrap_puts (wf,
	gettext (dup_args_note_text[!!(po->po_flags & PARSEOPT_SINGLE_DASH)]));
      wordwrap_para (wf);
    }

  if (po->po_help_hook)
    {
      wordwrap_set_left_margin (wf, 0);
      po->po_help_hook (wf, po);
      wordwrap_para (wf);
    }

  wordwrap_set_left_margin (wf, 0);
  wordwrap_set_right_margin (wf, FMT(rmargin));

  if (po->po_bugreport_address)
    wordwrap_printf (wf,
		     _("Report bugs and suggestions to <%s>\n"),
		     po->po_bugreport_address);
  if (po->po_package_name && po->po_package_url)
    wordwrap_printf (wf,
		     _("%s home page: <%s>\n"), po->po_package_name,
		     po->po_package_url);
  if (po->po_general_help)
    wordwrap_puts (wf, gettext (po->po_general_help));

  wordwrap_close (wf);
}

void
parseopt_version_fd (struct parseopt *po, int fd)
{
  WORDWRAP_FILE wf;

  if (!po->po_version_hook)
    return;

  init_usage_vars (po);
  wf = wordwrap_fdopen (fd);
  po->po_version_hook (wf, po);
  wordwrap_close (wf);
}
