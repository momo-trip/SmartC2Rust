/*
 * implement string functions for dc
 *
 * Copyright (C) 1994, 1997, 1998, 2006, 2008, 2013, 2017
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

/* This should be the only module that knows the internals of type dc_string */

#include <stdio.h>
#include <stddef.h>	/* ptrdiff_t */
#include <stdlib.h>
#include <string.h>	/* memcpy */
#include "dc.h"

/* here is the completion of the dc_string type: */
struct dc_string {
	char *s_ptr;  /* pointer to base of string */
	size_t s_len; /* length of counted string */
	int  s_refs;  /* reference count to cut down on memory use by duplicates */
};


/* return a duplicate of the string in the passed value */
/* The mismatched data types forces the caller to deal with
 * bad dc_type'd dc_data values, and makes it more convenient
 * for the caller to not have to do the grunge work of setting
 * up a dc_type result.
 */
dc_data
dc_dup_str(dc_str value)
{
	dc_data result;

	++value->s_refs;
	result.v.string = value;
	result.dc_type = DC_STRING;
	return result;
}

/* free an instance of a dc_str value */
void
dc_free_str(dc_str *value)
{
	struct dc_string *string = *value;

	if (--string->s_refs < 1){
		free(string->s_ptr);
		free(string);
	}
}

/* Output a dc_str value.
 * Free the value after use if discard_flag is DC_TOSS.
 */
void
dc_out_str(dc_str value, dc_discard discard_flag)
{
	fwrite(value->s_ptr, value->s_len, sizeof *value->s_ptr, stdout);
	if (discard_flag == DC_TOSS)
		dc_free_str(&value);
}

/* Make a copy of a string (base s, length len)
 * into a dc_str value; if "augment" is non-NULL,
 * then additionally append it (as a C string)
 * to the end of s in the constructed string.
 * Return a dc_data result with this the copied value;
 * if s==NULL then the result will be of dc_type DC_UNINTIALIZED,
 * otherwise it will be DC_STRING.
 */
dc_data
dc_makestring(const char *s, size_t len, const char *augment)
{
	dc_data result;
	struct dc_string *string;
	size_t alen = augment ? strlen(augment) : 0;

	if (!s) {
		memset(&result, 0, sizeof result);
		result.dc_type = DC_UNINITIALIZED;
		return result;
	}
	string = dc_malloc(sizeof *string);
	string->s_ptr = dc_malloc(len+alen+1);
	memcpy(string->s_ptr, s, len);
	if (alen)
		memcpy(string->s_ptr+len, augment, alen);
	string->s_ptr[len+alen] = '\0';	/* nul terminated for those who need it */
	string->s_len = len+alen;
	string->s_refs = 1;
	result.v.string = string;
	result.dc_type = DC_STRING;
	return result;
}

/* read a dc_str value from FILE *fp;
 * if ldelim == rdelim, then read until a ldelim char or EOF is reached;
 * if ldelim != rdelim, then read until a matching rdelim for the
 * (already eaten) first ldelim is read.
 * Return a dc_data result with the dc_str value as its contents;
 * this will either be of .dc_type==DC_STRING (in the normal case)
 * or of .dc_type==DC_UNINITIALIZED (to indicate an EOF condition).
 */
dc_data
dc_readstring(int (*input)(void), int ldelim, int rdelim, const char *augment)
{
	static char *line_buf = NULL;	/* a buffer to build the string in */
	static size_t buflen = 0;		/* the current size of line_buf */
	int depth=1;
	int c;
	char *p;
	const char *end;

	if (!line_buf){
		/* initial buflen should be large enough to handle most cases */
		buflen = (size_t) 2016;
		line_buf = dc_malloc(buflen);
	}
	p = line_buf;
	end = line_buf + buflen;
	for (;;){
		c = (*input)();
		if (c == EOF)
			break;
		else if (c == rdelim && --depth < 1)
			break;
		else if (c == ldelim)
			++depth;
		if (p >= end){
			ptrdiff_t offset = p - line_buf;
			/* buflen increment should be big enough
			 * to avoid execessive reallocs:
			 */
			buflen += 2048;
			line_buf = realloc(line_buf, buflen);
			if (!line_buf)
				dc_memfail();
			p = line_buf + offset;
			end = line_buf + buflen;
		}
		*p++ = c;
	}
	if (c==EOF && p==line_buf)
		return dc_makestring(NULL, 0, NULL);
	return dc_makestring(line_buf, (size_t)(p-line_buf), augment);
}

/* return the base pointer of the dc_str value;
 * This function is needed because no one else knows what dc_str
 * looks like.
 */
const char *
dc_str2charp(dc_str value)
{
	return value->s_ptr;
}

/* return the length of the dc_str value;
 * This function is needed because no one else knows what dc_str
 * looks like, and strlen(dc_str2charp(value)) won't work
 * if there's an embedded '\0'.
 */
size_t
dc_strlen(dc_str value)
{
	return value->s_len;
}


/* initialize the strings subsystem */
void
dc_string_init(void)
{
	/* nothing to do for this implementation */
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
