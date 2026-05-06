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
#include <stdio.h> // EOF, FILE, fflush(), fgets(), fileno(), fprintf(),
		   // printf(), putchar(), setbuf(), stderr, stdin,
		   // stdout
#include <stdlib.h> // exit(), EXIT_SUCCESS, strtol()
#include <string.h> // strerror()

#include <getopt.h> // getopt_long()

#include "lib.h"

#include "errarg.h"
#include "error.h"
#include "cset.h"

#include "refid.h"
#include "search.h"

// needed for isatty()
#include "posix.h"
#include "nonposix.h"

extern "C" const char *Version_string;

static void usage(FILE *stream)
{
  fprintf(stream,
	  "usage: %s [-i XYZ] [-t N] database ...\n"
	  "usage: %s {-v | --version}\n"
	  "usage: %s --help\n",
	  program_name, program_name, program_name);
  if (stdout == stream)
    fputs("\n"
"GNU lookbib writes a prompt to the standard error stream (unless the\n"
"standard input stream is not a terminal), reads from the standard\n"
"input a line containing a set of keywords, searches each\n"
"bibliographic \"database\" for references containing those keywords,\n"
"writes any references found to the standard output stream, and\n"
"repeats this process until the end of input.  See the lookbib(1)\n"
"manual page.\n",
	  stream);
}

int main(int argc, char **argv)
{
  program_name = argv[0];
  static char stderr_buf[BUFSIZ];
  setbuf(stderr, stderr_buf);
  int opt;
  static const struct option long_options[] = {
    { "help", no_argument, 0 /* nullptr */, CHAR_MAX + 1 },
    { "version", no_argument, 0 /* nullptr */, 'v' },
    { 0 /* nullptr */, 0, 0 /* nullptr */, 0 }
  };
  while ((opt = getopt_long(argc, argv, ":vVi:t:", long_options,
			    0 /* nullptr */))
	 != EOF)
    switch (opt) {
    case 'V':
      do_verify = true;
      break;
    case 'i':
      linear_ignore_fields = optarg;
      break;
    case 't':
      {
	char *ptr;
	long n = strtol(optarg, &ptr, 10);
	if (ptr == optarg) {
	  error("invalid integer '%1' in argument to command-line 't'"
		" option; ignoring", optarg);
	  break;
	}
	if (n < 1)
	  n = 1;
	linear_truncate_len = int(n);
	break;
      }
    case 'v':
      {
	printf("GNU lookbib (groff) version %s\n", Version_string);
	exit(EXIT_SUCCESS);
	break;
      }
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
    error("no database file operands");
    usage(stderr);
    exit(2);
  }
  search_list list;
  for (int i = optind; i < argc; i++)
    list.add_file(argv[i]);
  if (list.nfiles() == 0)
    fatal("no databases");
  char line[1024];
  int interactive = isatty(fileno(stdin));
  for (;;) {
    if (interactive) {
      fputs("> ", stderr);
      fflush(stderr);
    }
    if (!fgets(line, sizeof line, stdin))
      break;
    char *ptr = line;
    while (csspace(*ptr))
      ptr++;
    if (*ptr == '\0')
      continue;
    search_list_iterator iter(&list, line);
    const char *start;
    int len;
    int count;
    for (count = 0; iter.next(&start, &len); count++) {
      if (fwrite(start, 1, len, stdout) != (size_t)len)
	fatal("cannot write to standard output stream: %1",
	      strerror(errno));
      // Can happen for last reference in file.
      if (start[len - 1] != '\n')
	putchar('\n');
      putchar('\n');
    }
    fflush(stdout);
    if (interactive) {
      fprintf(stderr, "%d found\n", count);
      fflush(stderr);
    }
  }
  if (interactive)
    putc('\n', stderr);
  return 0;
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
