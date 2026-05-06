/* Copyright (C) 1989-2024 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com)

This file is part of groff, the GNU roff typesetting system.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or
(at your option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h> // exit(), EXIT_FAILURE, EXIT_SUCCESS, getenv(),
		    // strtol()

#include <getopt.h> // getopt_long()

#include "lib.h"

#include "errarg.h"
#include "error.h"

#include "defs.h"
#include "refid.h"
#include "search.h"

extern "C" const char *Version_string;

static void usage(FILE *stream)
{
  fprintf(stream,
          "usage: %s [-n] [-i XYZ] [-p database] ... [-t N] key ...\n"
          "usage: %s {-v | --version}\n"
          "usage: %s --help\n",
	  program_name, program_name, program_name);
  if (stdout == stream)
    fputs("\n"
"GNU lkbib searches bibliographic databases for references matching\n"
"all keywords \"key\" and writes any references found to the standard\n"
"output stream.  It reads databases given by -p options and then\n"
"(unless -n is given) a default database.  See the lkbib(1) manual\n"
"page.\n",
	  stream);
}

int main(int argc, char **argv)
{
  program_name = argv[0];
  static char stderr_buf[BUFSIZ];
  setbuf(stderr, stderr_buf);
  int search_default = 1;
  search_list list;
  int opt;
  static const struct option long_options[] = {
    { "help", no_argument, 0 /* nullptr */, CHAR_MAX + 1 },
    { "version", no_argument, 0 /* nullptr */, 'v' },
    { 0 /* nullptr */, 0, 0 /* nullptr */, 0 }
  };
  while ((opt = getopt_long(argc, argv, ":nvVi:t:p:", long_options,
			    0 /* nullptr */))
	 != EOF)
    switch (opt) {
    case 'V':
      do_verify = true;
      break;
    case 'n':
      search_default = 0;
      break;
    case 'i':
      linear_ignore_fields = optarg;
      break;
    case 't':
      {
	char *ptr;
	long n = strtol(optarg, &ptr, 10);
	if (ptr == optarg) {
	  error("bad integer '%1' in 't' option", optarg);
	  break;
	}
	if (n < 1)
	  n = 1;
	linear_truncate_len = int(n);
	break;
      }
    case 'v':
      {
	printf("GNU lkbib (groff) version %s\n", Version_string);
	exit(EXIT_SUCCESS);
	break;
      }
    case 'p':
      list.add_file(optarg);
      break;
    case CHAR_MAX + 1: // --help
      usage(stdout);
      exit(EXIT_SUCCESS);
      break;
    case '?':
      if (optopt != 0)
	error("unrecognized command-line option '%1'", char(optopt));
      else
	error("unrecognized command-line option '%1'",
	      argv[(optind - 1)]);
      usage(stderr);
      exit(2);
      break;
    case ':':
      error("command-line option '%1' requires an argument",
           char(optopt));
      usage(stderr);
      exit(2);
      break;
    default:
      assert(0 == "unhandled case of command-line option");
    }
  if (optind >= argc) {
    usage(stderr);
    exit(EXIT_FAILURE);
  }
  char *filename = getenv("REFER");
  if (filename)
    list.add_file(filename);
  else if (search_default)
    list.add_file(DEFAULT_INDEX, 1);
  if (list.nfiles() == 0)
    fatal("no databases");
  int total_len = 0;
  int i;
  for (i = optind; i < argc; i++)
    total_len += strlen(argv[i]);
  total_len += argc - optind - 1 + 1; // for spaces and '\0'
  char *buffer = new char[total_len];
  char *ptr = buffer;
  for (i = optind; i < argc; i++) {
    if (i > optind)
      *ptr++ = ' ';
    strcpy(ptr, argv[i]);
    ptr = strchr(ptr, '\0');
  }
  search_list_iterator iter(&list, buffer);
  const char *start;
  int len;
  int count;
  for (count = 0; iter.next(&start, &len); count++) {
    if (fwrite(start, 1, len, stdout) != (size_t)len)
      fatal("write error on stdout: %1", strerror(errno));
    // Can happen for last reference in file.
    if (start[len - 1] != '\n')
      putchar('\n');
    putchar('\n');
  }
  return !count;
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
