/*
 * Secure sscanf - sscanf with an additional size argument for string
 * arguments. All format specifiers should work as in the standard
 * scanf - except for those writing to a string buffer provided by the
 * caller. These specifiers take an additional argument of type size_t
 * that specifies the size of the buffer.
 *
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with GNU gv; see the file COPYING.  If not, write to
** the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2002, Olaf Kirch <okir@suse.de>
 */

#define _GNU_SOURCE
#include <ac_config.h>

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "secscanf.h"

static size_t GNU_strnlen(const char *s, size_t len)
{
   size_t i;
   for(i=0; i<len && *(s+i); i++);
   return i;
}

static char* GNU_strndup (char const *s, size_t n)
{
   size_t len = GNU_strnlen (s, n);
   char *new = malloc (len + 1);

   if (new == NULL)
   return NULL;

   new[len] = '\0';
   return memcpy (new, s, len);
}

enum {
	CONV_ANY,
	CONV_STR,
	CONV_NUM,
	CONV_INTEGER,
	CONV_FLOAT,
	CONV_POINTER,
};

enum {
	SIZE_ANY,
	SIZE_SHORT,
	SIZE_LONG,
	SIZE_QUAD,
};

union scan_value {
	const char *		v_string;
	long long		v_signed;
	unsigned long long	v_integer;
	long double		v_double;
	void *			v_pointer;
};


static int	process_number(union scan_value *vp, const char **sp, char fmt);
static int	process_char_class(const char **, const char **, int);

static inline int
set_conv_type(int *type, int new_type)
{
	switch (*type) {
	case CONV_ANY:
		break;
	case CONV_NUM:
		if (new_type == CONV_INTEGER
		 || new_type == CONV_FLOAT
		 || new_type == CONV_POINTER)
		 	break;
		/* fallthru */
	default:
		if (*type != new_type)
			return 0;
		break;
	}

	*type = new_type;
	return 1;
}

int
sec_sscanf(const char *s, const char *fmt, ...)
{
	const char	*begin = s;
	int		num_fields = 0, fmt_empty = 1;
	va_list		ap;

	va_start(ap, fmt);
	while (*fmt) {
		union scan_value value;
		const char	*pre_space_skip,
				*value_begin;
		int		assign = 1, allocate = 0,
				conv_type = CONV_ANY,
				conv_size = SIZE_ANY,
				field_width = -1,
				nul_terminated = 1;
		char		c;

		c = *fmt++;
		if (isspace(c)) {
			while (isspace(*s))
				s++;
			continue;
		}

		fmt_empty = 0;
		if (c != '%') {
			if (c != *s)
				goto stop;
			s++;
			continue;
		}

		/* Each % directive implicitly skips white space
		 * except for the %c case */
		pre_space_skip = s;
		while (isspace(*s))
			s++;

		while (1) {
			int	type = CONV_ANY, size = SIZE_ANY;

			switch (*fmt) {
			case '*':
				assign = 0;
				break;
			case 'a':
				type = CONV_STR;
				allocate = 1;
				break;
			case 'h':
				type = CONV_INTEGER;
				size = SIZE_SHORT;
				break;
			case 'l':
				type = CONV_NUM;
				size = SIZE_LONG;
				break;
			case 'L':
			case 'q':
				type = CONV_NUM;
				size = SIZE_QUAD;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				field_width = strtol(fmt, (char **) &fmt, 10);
				fmt--;
				break;
			default:
				goto flags_done;
			}

			if (!set_conv_type(&conv_type, type))
				goto stop;

			if (size != SIZE_ANY) {
				if (size == SIZE_LONG && conv_size == SIZE_LONG)
					conv_size = SIZE_QUAD;
				else
					conv_size = size;
			}

			fmt++;
		}

	flags_done:
		value_begin = s;

		switch (*fmt++) {
		case '%':
			if (*s == '\0')
				goto eof;
			if (*s != '%')
				goto stop;
			continue;
		case '[':
			value.v_string = s;
			if (!set_conv_type(&conv_type, CONV_STR)
			 || !process_char_class(&fmt, &s, field_width))
				goto stop;
			break;
		case 's':
			value.v_string = s;
			if (!set_conv_type(&conv_type, CONV_STR))
				goto stop;
			while (*s && !isspace(*s) && field_width-- != 0)
				s++;
			break;
		case 'c':
			if (!set_conv_type(&conv_type, CONV_STR))
				goto stop;
			value.v_string = s = value_begin = pre_space_skip;

			if (field_width < 0)
				s++;
			else while (*s && field_width--)
				s++;
			nul_terminated = 0;
			break;
		case 'd':
		case 'i':
		case 'o':
		case 'u':
		case 'x':
		case 'X':
			if (!set_conv_type(&conv_type, CONV_INTEGER)
			 || !process_number(&value, &s, fmt[-1]))
				goto stop;
			break;
		case 'p':
			if (!set_conv_type(&conv_type, CONV_POINTER)
			 || !process_number(&value, &s, fmt[-1]))
				goto stop;
			break;
		case 'f':
		case 'g':
		case 'e':
		case 'E':
			if (!set_conv_type(&conv_type, CONV_FLOAT)
			 || !process_number(&value, &s, fmt[-1]))
				goto stop;
			break;
		case 'n':
			if (!set_conv_type(&conv_type, CONV_INTEGER))
				goto stop;
			value.v_signed = (s - begin);
			break;
		default:
			goto stop;
		}

		/* We've consumed what we need to consume. Now copy */
		if (!assign)
			continue;

		/* Make sure we've consumed at least *something* */
		if (s == value_begin)
			goto eof;

		/* Deal with a conversion flag */
		if (conv_type == CONV_STR && allocate) {
			value.v_pointer = GNU_strndup(value.v_string, s - value.v_string);
			conv_type = CONV_POINTER;
			allocate = 0;
		}

		switch (conv_type) {
		case CONV_STR:
			{
				const char *string = value.v_string;
				char	*buf;
				size_t	size;

				if (string == NULL)
					goto stop;
				buf = va_arg(ap, char *);
				size = va_arg(ap, size_t) - nul_terminated;
				if (size > (size_t)(s - string))
					size = s - string;
				strncpy(buf, string, size);
				if (nul_terminated)
					buf[size] = '\0';
			}
			break;

		case CONV_POINTER:
			{
				void	**ptr;

				ptr = va_arg(ap, void **);
				*ptr = value.v_pointer;
			}
			break;
		case CONV_INTEGER:
			{
				void	*ptr;

				ptr = va_arg(ap, void *);
				switch (conv_size) {
				case SIZE_SHORT:
					*(short *) ptr = value.v_integer;
					break;
				case SIZE_ANY:
					*(int *) ptr = value.v_integer;
					break;
				case SIZE_LONG:
					*(long *) ptr = value.v_integer;
					break;
				case SIZE_QUAD:
					*(long long *) ptr = value.v_integer;
					break;
				default:
					goto stop;
				}
			}
			break;
		case CONV_FLOAT:
			{
				void	*ptr;

				ptr = va_arg(ap, void *);
				switch (conv_size) {
				case SIZE_ANY:
					*(float *) ptr = value.v_double;
					break;
				case SIZE_LONG:
					*(double *) ptr = value.v_double;
					break;
				case SIZE_QUAD:
					*(long double *) ptr = value.v_double;
					break;
				default:
					goto stop;
				}
			}
			break;
		default:
			goto stop;
		}

		num_fields++;
	}

stop:	return num_fields;

eof:	if (num_fields)
		return num_fields;
	return EOF;
}

static int
process_number(union scan_value *vp, const char **sp, char fmt)
{
	const char	*s = *sp;

	switch (fmt) {
	case 'd':
		vp->v_signed = strtoll(s, (char **) sp, 10);
		break;
	case 'i':
		vp->v_signed = strtoll(s, (char **) sp, 0);
		break;
	case 'o':
		vp->v_integer = strtoull(s, (char **) sp, 8);
		break;
	case 'u':
		vp->v_integer = strtoull(s, (char **) sp, 10);
		break;
	case 'x':
	case 'X':
		vp->v_integer = strtoull(s, (char **) sp, 16);
		break;
	case 'p':
		vp->v_pointer = (void *)(intptr_t) strtoull(s, (char **) sp, 0);
		break;
	case 'f':
	case 'g':
	case 'e':
	case 'E':
		vp->v_double = strtold(s, (char **) sp);
		break;
	default:
		return 0;
	}

	return 1;
}

static int
process_char_class(const char **fmt, const char **sp, int width)
{
	unsigned char	*s, c, prev_char = 0;
	unsigned char	table[255];
	int		val = 1;

	s = (unsigned char *) *fmt;
	if (*s == '^') {
		memset(table, 1, sizeof(table));
		val = 0;
		s++;
	} else {
		memset(table, 0, sizeof(table));
		val = 1;
	}
	/* First character in set is closing bracket means add it to the
	 * set of characters */
	if ((c = *s) == ']') {
		table[c] = val;
		prev_char = c;
		s++;
	}

	/* Any other closing bracket finishes off the set */
	while ((c = *s++) != ']') {
		if (prev_char) {
			if (c == '-' && *s != '\0' && *s != ']') {
				c = *s++;
			} else {
				//table[prev_char] = val;
				prev_char = '\0';
			}
		}

		if (c == '\0')
			return 0;

		if (prev_char) {
			while (prev_char < c)
				table[prev_char++] = val;
		}
		table[c] = val;
		prev_char = c;
	}
	*fmt = (char *) s;

#if 0
	{
		int	n;

		printf("char class=");
		for (n = 0; n < 255; n++)
			if (table[n])
				printf(isprint(n)? "%c" : "\\%03o", n);
		printf("\n");
	}
#endif

	s = (unsigned char *) *sp;
	while ((c = *s) != '\0' && table[c] && width--)
		s++;

	*sp = (char *) s;
	return 1;
}

#ifdef TEST
static int	verify(const char *fmt, const char *s);
static int	verify_s(const char *fmt, const char *s);

enum { S, I, L, F, D, P };

int
main(int argc, char **argv)
{
	verify("%d %d", "12 13");
	verify("%d-%d", "12 13");
	verify("%d-%d", "12-13");
	verify("%u %u", "12 13");
	verify("%o %o", "12 13");
	verify("%x %x", "12 13");
	verify("%X %X", "12 13");
	verify("%hd %hd", "12 13");
	verify("%ld %ld", "12 13");
	verify("%lld %lld", "12 13");
	verify("%Ld %Ld", "12 13");
	verify("%qd %qd", "12 13");
	verify("%f %f", "12 13");
	verify("%lf %lf", "12 13");
	verify("%Lf %Lf", "12 13");
	verify("%qf %qf", "12 13");
	verify("%*d-%d", "12-13");
	verify("%*s %d", "12 13");
	verify("%p", "0xdeadbeef");
	verify("%*[a-e] %x", "deadbeef feeb");
	verify("%*[a-f] %x", "deadbeef feeb");
	verify("%*[^g-z] %x", "deadbeef feeb");
	verify("%*[^ g-z] %x", "deadbeef feeb");
	verify("%*[^ g-z-] %x", "dead-beef feeb");
	verify("%*5s %d", "toast123 456");
	verify("", "lalla");
	verify("%u", "");

	verify_s("%s", "aa bb");
	verify_s("%s %s", "aa bb");
	verify_s("%[a-z] %s", "aa bb");
	verify_s("%c %s", "aa bb");
	verify_s("%2c %s", " aa bb");
	verify_s("%20c %s", " aa bb");

	return 0;
}

static int
verify(const char *fmt, const char *s)
{
	union scan_value vals[5], vals_ref[5], *v;
	int	n, m;

	memset(vals, 0xe5, sizeof(vals));
	memset(vals_ref, 0xe5, sizeof(vals_ref));

	v = vals;
	n = sec_sscanf(s, fmt, v + 0, v + 1, v + 2, v + 3, v + 4);

	v = vals_ref;
	m = sscanf(s, fmt, v + 0, v + 1, v + 2, v + 3, v + 4);

	if (m != n) {
		printf("FAILED: fmt=\"%s\"\n"
		       "        str=\"%s\"\n"
		       "        sec_scanf returns %d, sscanf returns %d\n",
		       fmt, s, n, m);
		return 0;
	}

	if (memcmp(vals, vals_ref, sizeof(vals))) {
		printf("FAILED: fmt=\"%s\"\n"
		       "        str=\"%s\"\n"
		       "        data differs!\n",
		       fmt, s);
		printf("0x%Lx != 0x%Lx\n", vals[0].v_integer, vals_ref[0].v_integer);
		return 0;
	}

	return 1;
}

static int
verify_s(const char *fmt, const char *s)
{
	char	buf[3][256], buf_ref[3][256];
	int	n, m;

	memset(buf, 0xe5, sizeof(buf));
	memset(buf_ref, 0xe5, sizeof(buf_ref));

	n = sec_sscanf(s, fmt, buf, sizeof(buf[0]), buf + 1, sizeof(buf[1]), buf + 2, sizeof(buf[2]));

	m = sscanf(s, fmt, buf_ref, buf_ref + 1, buf_ref + 2);

	if (m != n) {
		printf("FAILED: fmt=\"%s\"\n"
		       "        str=\"%s\"\n"
		       "        sec_scanf returns %d, sscanf returns %d\n",
		       fmt, s, n, m);
		return 0;
	}

	if (memcmp(buf, buf_ref, sizeof(buf))) {
		printf("FAILED: fmt=\"%s\"\n"
		       "        str=\"%s\"\n"
		       "        data differs!\n",
		       fmt, s);
		printf("%s != %s\n", buf[0], buf_ref[0]);
		return 0;
	}

	return 1;
}
#endif
