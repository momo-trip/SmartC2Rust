/* Copyright (C) 1992-2025 Free Software Foundation, Inc.
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

/* This translates ps fonts in .pfb format to ASCII ps files. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h> // assert()
#include <errno.h> // errno
#include <stdio.h>
#include <stdlib.h> // exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> // strerror()
#include <limits.h>

#include <getopt.h>

#include "nonposix.h"

/* Binary bytes per output line. */
#define BYTES_PER_LINE (64/2)
#define MAX_LINE_LENGTH 78
#define HEX_DIGITS "0123456789abcdef"

extern const char *Version_string;

static char *program_name;

static void error(const char *s)
{
  (void) fprintf(stderr, "%s: error: %s\n", program_name, s);
}

static void die(const char *s)
{
  error(s);
  exit(EXIT_FAILURE);
}

static void usage(FILE *stream)
{
  (void) fprintf(stream, "usage: %s [pfb-file]\n"
		 "usage: %s {-v | --version}\n"
		 "usage: %s --help\n",
	  program_name, program_name, program_name);
  if (stdout == stream)
    fputs("\n"
"Translate a PostScript Type 1 font in Printer Font Binary (PFB) format"
"\n"
"to Printer Font ASCII (PFA) format.  See the pfbtops(1) manual page."
"\n",
	  stream);
}

static void get_text(int n)
{
  int c = 0, c1;
  int in_string = 0;
  int is_comment = 0;
  int count = 0;

  while (--n >= 0) {
    c = getchar();
    if (c == '(' && !is_comment)
      in_string++;
    else if (c == ')' && !is_comment)
      in_string--;
    else if (c == '%' && !in_string)
      is_comment = 1;
    else if (c == '\\' && in_string) {
      count++;
      putchar(c);
      if (n-- == 0)
	break;
      c = getchar();
      /* don't split octal character representations */
      if (c >= '0' && c <= '7') {
	count++;
	putchar(c);
	if (n-- == 0)
	  break;
	c = getchar();
	if (c >= '0' && c <= '7') {
	  count++;
	  putchar(c);
	  if (n-- == 0)
	    break;
	  c = getchar();
	  if (c >= '0' && c <= '7') {
	    count++;
	    putchar(c);
	    if (n-- == 0)
	      break;
	    c = getchar();
	  }
	}
      }
    }
    if (c == EOF)
      die("end of file in text packet");
    else if (c == '\r') {
      if (n-- == 0)
	break;
      c1 = getchar();
      if (c1 != '\n') {
	ungetc(c1, stdin);
	n++;
      }
      c = '\n';
    }
    if (c == '\n') {
      count = 0;
      is_comment = 0;
    }
    else if (count >= MAX_LINE_LENGTH) {
      if (in_string > 0) {
	count = 1;
	putchar('\\');
	putchar('\n');
      }
      else if (is_comment) {
	count = 2;
	putchar('\n');
	putchar('%');
      }
      else {
	/* split at the next whitespace character */
	while (c != ' ' && c != '\t' && c != '\f') {
	  putchar(c);
	  if (n-- == 0)
	    break;  
	  c = getchar();
	}
	count = 0;
	putchar('\n');
	continue;
      }
    }
    count++;
    putchar(c);
  }
  if (c != '\n')
    putchar('\n');
}

static void get_binary(int n)
{
  int c;
  int count = 0;

  while (--n >= 0) {
    c = getchar();
    if (c == EOF)
      die("end of file in binary packet");
    if (count >= BYTES_PER_LINE) {
      putchar('\n');
      count = 0;
    }
    count++;
    putchar(HEX_DIGITS[(c >> 4) & 0xf]);
    putchar(HEX_DIGITS[c & 0xf]);
  }
  putchar('\n');
}

int main(int argc, char **argv)
{
  int opt;
  static const struct option long_options[] = {
    { "help", no_argument, NULL, CHAR_MAX + 1 },
    { "version", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
  };

  program_name = argv[0];

  while ((opt = getopt_long(argc, argv, ":v", long_options, NULL))
	 != EOF) {
    switch (opt) {
    case 'v':
      printf("GNU pfbtops (groff) version %s\n", Version_string);
      exit(EXIT_SUCCESS);
      break;
    case CHAR_MAX + 1: /* --help */
      usage(stdout);
      exit(EXIT_SUCCESS);
      break;
    case '?':
      if (optopt != 0) {
	char errbuf[] = "unrecognized command-line option 'X'";
	errbuf[(strchr(errbuf, 'X') - errbuf)] = optopt;
	error(errbuf);
      }
      else
	// We can't use `error()` because we have no idea how big the
	// user-specified long option is.
	(void) fprintf(stderr, "%s: error: unrecognized command-line"
		       " option '%s'\n", program_name,
		       argv[(optind - 1)]);
      usage(stderr);
      exit(2);
      break;
    // in case we ever accept options that take arguments
    case ':':
      fprintf(stderr, "%s: error: command-line option '%c' requires an"
	      " argument\n", program_name, (char) optopt);
      usage(stderr);
      exit(2);
      break;
    default:
      assert(0 == "unhandled case of command-line option");
    }
  }

  if (argc - optind > 1) {
    usage(stderr);
    exit(2);
  }
  const char *file = argv[optind];
  if (argc > optind && !freopen(file, "r", stdin)) {
    fprintf(stderr, "%s: error: unable to open file '%s': %s\n",
	    program_name, file, strerror(errno));
    exit(EXIT_FAILURE);
  }
  SET_BINARY(fileno(stdin));
  for (;;) {
    int type, c, i;
    long n;

    c = getchar();
    if (c != 0x80)
      die("first byte of packet not 0x80");
    type = getchar();
    if (type == 3)
      break;
    if (type != 1 && type != 2)
      die("bad packet type");
    n = 0;
    for (i = 0; i < 4; i++) {
      c = getchar();
      if (c == EOF)
	die("end of file in packet header");
      n |= (long)c << (i << 3);
    }
    if (n < 0)
      die("negative packet length");
    if (type == 1)
      get_text(n);
    else
      get_binary(n);
  }
  exit(EXIT_SUCCESS);
}

// Local Variables:
// fill-column: 72
// mode: C
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
