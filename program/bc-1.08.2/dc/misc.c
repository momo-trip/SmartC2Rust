/*
 * misc. functions for the "dc" Desk Calculator language.
 *
 * Copyright (C) 1994, 1997, 1998, 2000, 2006, 2008, 2013, 2017
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

/* This module contains miscellaneous functions that have no
 * special knowledge of any private data structures.
 * They could each be moved to their own separate modules,
 * but are aggregated here as a matter of convenience.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dc.h"
#include "../config.h"

/* print a simple warning message */
void
dc_warn(const char *msg)
{
	fprintf(stderr, "%s: %s\n", progname, msg);
}

/* print an "out of memory" diagnostic and exit program */
void
dc_memfail(void)
{
	dc_warn("out of memory");
	exit(EXIT_FAILURE);
}

/* malloc or die */
void *
dc_malloc(size_t len)
{
	void *result = malloc(len);

	if ( ! result )
		dc_memfail();
	return result;
}


/* print the id in a human-understandable form
 *  fp is the output stream to place the output on
 *  id is the name of the register (or command) to be printed
 *  suffix is a modifier (such as "stack") to be printed
 */
void
dc_show_id(FILE *fp, int id, const char *suffix)
{
	if (isgraph(id))
		fprintf(fp, "'%c' (%#o)%s", (unsigned int) id, id, suffix);
	else
		fprintf(fp, "%#o%s", (unsigned int) id, suffix);
}


/* report that corrupt data has been detected;
 * use the msg and regid (if nonnegative) to give information
 * about where the garbage was found,
 *
 * will abort() so that a debugger might be used to help find
 * the bug
 */
/* If this routine is called, then there is a bug in the code;
 * i.e. it is _not_ a data or user error
 */
void
dc_garbage(const char *msg, int regid)
{
	if (regid < 0) {
		fprintf(stderr, "%s: garbage %s\n", progname, msg);
	} else {
		fprintf(stderr, "%s:%s register ", progname, msg);
		dc_show_id(stderr, regid, " is garbage\n");
	}
	abort();
}


/* call system() with the passed string;
 * if the string contains a newline, terminate the string
 * there before calling system.
 * Return a pointer to the first unused character in the string
 * (i.e. past the '\n' if there was one, to the '\0' otherwise).
 */
static void maybe_system(const char *cmd) {
#ifdef ENABLE_DC_BANG_SHELL
	(void)system(cmd);
#else
	fprintf(stderr, "The `!' command has been disabled.  Try job control?\n");
#endif
}

const char *
dc_system(const char *s)
{
	const char *p;
	char *tmpstr;
	size_t len;

	p = strchr(s, '\n');
	if (p) {
		len = (size_t) (p - s);
		tmpstr = dc_malloc(len + 1);
		strncpy(tmpstr, s, len);
		tmpstr[len] = '\0';
		maybe_system(tmpstr);
		free(tmpstr);
		return p + 1;
	}
	maybe_system(s);
	return s + strlen(s);
}



/* Check to see if there were any output errors; if so, then give
 * an error message (if stderr is not known to be unhappy), and
 * ensure that the program exits with an error indication.
 */
void
dc_flush_stdout(int close_stdout)
{
	const char *errmsg = NULL;

	if (ferror(stdout))
		errmsg = "error writing to stdout";
	else if (fflush(stdout))
		errmsg = "error flushing stdout";
	else if (close_stdout && fclose(stdout))
		errmsg = "error closing stdout";

	if (errmsg) {
		fprintf(stderr, "%s: ", progname);
		perror(errmsg);
		exit(EXIT_FAILURE);
	}
}

/* print out the indicated value */
void
dc_print(dc_data value, int obase, dc_newline newline_p, dc_discard discard_p)
{
	if (value.dc_type == DC_NUMBER) {
		dc_out_num(value.v.number, obase, discard_p);
	} else if (value.dc_type == DC_STRING) {
		dc_out_str(value.v.string, discard_p);
	} else {
		dc_garbage("in data being printed", -1);
	}
	if (newline_p == DC_WITHNL)
		putchar ('\n');
	dc_flush_stdout(0);
}

/* return a duplicate of the passed value, regardless of type */
dc_data
dc_dup(dc_data value)
{
	if (value.dc_type!=DC_NUMBER && value.dc_type!=DC_STRING)
		dc_garbage("in value being duplicated", -1);
	if (value.dc_type == DC_NUMBER)
		return dc_dup_num(value.v.number);
	/*else*/
	return dc_dup_str(value.v.string);
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
