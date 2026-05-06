/*
 * implement the "dc" Desk Calculator language.
 *
 * Copyright (C) 1994, 1997, 1998, 2000, 2003, 2006, 2008, 2013, 2017, 2022, 2024
 * Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* Written with strong hiding of implementation details
 * in their own specialized modules.
 */
/* This module contains the argument processing/main functions.
 */

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_FSTAT
# include <sys/types.h>
# include <sys/stat.h>
#endif
#include "../h/getopt.h"
#include "dc.h"

const char *progname;	/* basename of program invocation */
unsigned int max_recursion_depth=0;

static void
bug_report_info(void)
{
	printf("Email bug reports to:  bug-dc@gnu.org .\n");
}

static void
show_version(void)
{
	printf("dc %s (GNU %s %s)\n", DC_VERSION, PACKAGE, VERSION);
	printf("\n%s\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE,\n\
to the extent permitted by law.\n", DC_COPYRIGHT);
}

/* your generic usage function */
static void
usage(FILE *f)
{
	fprintf(f, "\
Usage: %s [OPTION] [file ...]\n\
  -e, --expression=EXPR    evaluate expression\n\
  -f, --file=FILE          evaluate contents of file\n\
  -i, --interactive        force interactive mode\n\
  --max-recursion=depth    limit recursion depth\n\
  -h, --help               display this help and exit\n\
  -V, --version            output version information and exit\n\
\n\
", progname);
	bug_report_info();
}

/* returns a pointer to one past the last occurrence of c in s,
 * or s if c does not occur in s.
 */
static char *
r1bindex(char *s, int  c)
{
	char *p = strrchr(s, c);
	return p ? p+1 : s;
}

static void
try_file(const char *filename)
{
	FILE *input;

	if (!filename || strcmp(filename, "-") == 0) {
		input = stdin;
		filename = "(stdin)";
	} else if ( ! (input=fopen(filename, "r")) ) {
		fprintf(stderr, "%s: Could not open file %s\n", progname, filename);
		return;
	}
	{
		/* Several complaints have been filed about dc's silence
		 * when a "cd" typo is made.  I really wanted to avoid
		 * this mess, but I guess it really should be added...
		 */
#ifndef HAVE_FSTAT
		/* non-POSIXish system; this code _might_ notice a directory */
		int c = getc(input);
		if (c == EOF && ferror(input)) {
			perror(filename);
			goto close;
		}
		ungetc(c, input);

#else /* HAVE_FSTAT */
  /* If HAVE_FSTAT and no S_IS*() macros, it must be a pre-POSIX
   * Unix-ish system?
   */
# ifndef S_ISREG
#  ifdef S_IFREG
#    define S_ISREG(m)	(((m)&S_IFMT)==S_IFREG)
#   else
#    define S_ISREG(m)	0
#  endif
# endif
# ifndef S_ISCHR
#  ifdef S_IFCHR
#   define S_ISCHR(m)	(((m)&S_IFMT)==S_IFCHR)
#  endif
# endif
# ifndef S_ISFIFO
#  ifdef S_IFIFO
#   define S_ISFIFO(m)	(((m)&S_IFMT)==S_IFIFO)
#  endif
# endif
# ifndef S_ISSOCK
#  ifdef S_IFSOCK
#   define S_ISSOCK(m)	(((m)&S_IFMT)==S_IFSOCK)
#  endif
# endif
# ifndef S_ISDIR
#  ifdef S_IFDIR
#   define S_ISDIR(m)	(((m)&S_IFMT)==S_IFDIR)
#  endif
# endif
# ifndef S_ISBLK
#  ifdef S_IFBLK
#   define S_ISBLK(m)	(((m)&S_IFMT)==S_IFBLK)
#  endif
# endif
		struct stat s;
		if (fstat(fileno(input), &s) == -1) {
			/* "can't happen" */
			fprintf(stderr, "%s: Could not fstat file ", progname);
			perror(filename);
			goto close;
		}

#ifdef S_ISDIR
		if (S_ISDIR(s.st_mode)) {
			fprintf(stderr,
				"%s: Will not attempt to process directory %s\n",
				progname, filename);
			goto close;
		} else
#endif
#ifdef S_ISBLK
		if (S_ISBLK(s.st_mode)) {
			fprintf(stderr,
				"%s: Will not attempt to process block-special file %s\n",
				progname, filename);
			goto close;
		} else
#endif
		if (!S_ISREG(s.st_mode)
# ifdef S_ISCHR
			/* typically will be /dev/null or some sort of tty */
			&& !S_ISCHR(s.st_mode)
# endif
# ifdef S_ISFIFO
			&& !S_ISFIFO(s.st_mode)
# endif
# ifdef S_ISSOCK
			&& !S_ISSOCK(s.st_mode)
# endif
				) {
			fprintf(stderr,
				"%s: Will not attempt to process file of unusual type: %s\n",
				progname, filename);
			goto close;
		}
#endif /* HAVE_FSTAT */
	}
	switch (dc_evalfile(input)) {
	case DC_SUCCESS:
		break;
	case DC_EXIT_REQUEST:
		exit(EXIT_SUCCESS);
	default:
		exit(EXIT_FAILURE);
	}
close:
	if (input == stdin)
		input_cleanup();
	else
		fclose(input);
}


int
main(int argc, char **argv)
{
	static struct option const long_opts[] = {
		{"expression", required_argument, NULL, 'e'},
		{"file", required_argument, NULL, 'f'},
		{"help", no_argument, NULL, 'h'},
		{"version", no_argument, NULL, 'V'},
		{"max-recursion", required_argument, NULL, 256},
		{NULL, 0, NULL, 0}
	};
	int interactive = -1; // < 0 means to attempt autodetection
	int did_eval = 0;
	int c;

	progname = r1bindex(*argv, '/');
	dc_math_init();
	dc_string_init();
	dc_register_init();
	dc_array_init();

	while ((c = getopt_long(argc, argv, "hVe:f:i", long_opts, (int *)0)) != EOF) {
		switch (c) {
		case 'e':
			c = dc_evalstr(optarg);
			if (c != DC_SUCCESS) {
				dc_flush_stdout(0);
				return c == DC_EXIT_REQUEST ? EXIT_SUCCESS : EXIT_FAILURE;
			}
			did_eval = 1;
			break;
		case 'f':
			try_file(optarg);
			did_eval = 1;
			break;
		case 'h':
			usage(stdout);
			dc_flush_stdout(1);
			return EXIT_SUCCESS;
		case 'i':
			interactive = 1;
			break;
		case 'V':
			show_version();
			dc_flush_stdout(1);
			return EXIT_SUCCESS;
		case 256:
			max_recursion_depth = strtol(optarg, NULL, 0);
			break;
		default:
			usage(stderr);
			return EXIT_FAILURE;
		}
	}

	dc_set_interactive_flag(interactive);
	if (optind < argc)
		// command-files without -f trigger "then read stdin" behavior
		did_eval = 0;
	for (; optind < argc; ++optind)
		try_file(argv[optind]);
	if (did_eval == 0)
		try_file(NULL); //finish with reading stdin, if not otherwise suppressed
	dc_flush_stdout(1);
	if (ferror(stderr) || fclose(stderr))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
