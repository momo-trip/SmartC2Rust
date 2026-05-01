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
#include <inttypes.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "parseopt.h"

static int
get_signed_int (char *arg, intmax_t min, intmax_t max, intmax_t *ret_val)
{
  intmax_t n;
  char *p;

  errno = 0;
  n = strtoimax (arg, &p, 10);
  if (errno || *p)
    return -1;
  if (n < min || n > max)
    return -1;
  *ret_val = n;
  return 0;
}

static int
get_unsigned_int (char *arg, uintmax_t max, uintmax_t *ret_val)
{
  uintmax_t n;
  char *p;

  errno = 0;
  n = strtoumax (arg, &p, 10);
  if (errno || *p)
    return -1;
  if (n > max)
    return -1;
  *ret_val = n;
  return 0;
}

#define __cat__(a, b) a ## b
#define DCL_SGET(typename, type, min, max)				\
  int __cat__(optset_, typename) (struct parseopt *po, struct optdef *opt, \
				  char *arg)			\
  {									\
    intmax_t n;								\
    assert (opt->opt_ptr != NULL);					\
    if (arg == NULL) return -1;						\
    if (get_signed_int (arg, min, max, &n)) return -1;			\
    *(type*) opt->opt_ptr = (type) n;					\
    return 0;								\
  }

#define __cat__(a, b) a ## b
#define DCL_UGET(typename, type, max)					\
  int __cat__(optset_, typename) (struct parseopt *po, struct optdef *opt, \
				  char *arg)			\
  {									\
    uintmax_t n;							\
    assert (opt->opt_ptr != NULL);					\
    if (arg == NULL) return -1;						\
    if (get_unsigned_int (arg, max, &n)) return -1;			\
    *(type*) opt->opt_ptr = (type) n;					\
    return 0;								\
  }

DCL_SGET(int, int, INT_MIN, INT_MAX)
DCL_SGET(long, long, LONG_MIN, LONG_MAX)
DCL_SGET(llong, long long, LLONG_MIN, LLONG_MAX)

DCL_UGET(uint, unsigned int, UINT_MAX)
DCL_UGET(ulong, unsigned long, ULONG_MAX)
DCL_UGET(ullong, unsigned long long, ULLONG_MAX)
DCL_UGET(size_t, size_t, ((size_t)-1))

int
optset_incr (struct parseopt *po, struct optdef *opt, char *arg)
{
  assert (opt->opt_ptr != NULL);
  assert (arg == NULL);
  (*(int*) opt->opt_ptr) ++;
  return 0;
}

int
optset_string_copy (struct parseopt *po, struct optdef *opt, char *arg)
{
  assert (opt->opt_ptr != NULL);
  *(char **) opt->opt_ptr = arg;
  return 0;
}

int
optset_string (struct parseopt *po, struct optdef *opt, char *arg)
{
  char *p;
  assert (opt->opt_ptr != NULL);
  if (arg == NULL)
    return -1;
  if ((p = strdup (arg)) == NULL)
    {
      po->po_error (po, PO_MSG_ERR, _("memory allocation error"));
      return 0;
    }
  *(char **) opt->opt_ptr = p;
  return 0;
}

int
optset_string_alloc (struct parseopt *po, struct optdef *opt, char *arg)
{
  char *p;
  assert (opt->opt_ptr != NULL);
  if (arg == NULL)
    return -1;
  if ((p = strdup (arg)) == NULL)
    {
      po->po_error (po, PO_MSG_ERR, _("memory allocation error"));
      return 0;
    }
  free (*(char **) opt->opt_ptr);
  *(char **) opt->opt_ptr = p;
  return 0;
}

int
optset_true (struct parseopt *po, struct optdef *opt, char *arg)
{
  *(int *) opt->opt_ptr = 1;
  return 0;
}

int
optset_false (struct parseopt *po, struct optdef *opt, char *arg)
{
  *(int *) opt->opt_ptr = 0;
  return 0;
}

int
optset_bool (struct parseopt *po, struct optdef *opt, char *arg)
{
  *(int *) opt->opt_ptr = arg[0] == '1';
  return 0;
}
