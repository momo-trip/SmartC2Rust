/*
 * interface dc to the bc numeric routines
 *
 * Copyright (C) 1994, 1997, 1998, 2000, 2005, 2008, 2013, 2017
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

/* This should be the only module that knows the internals of type dc_num */
/* In this particular implementation we just slather out some glue and
 * make use of bc's numeric routines.
 */

/* make all the header files see that these are really the same thing;
 * this is acceptable because everywhere else dc_number is just referenced
 * as a pointer-to-incomplete-structure type
 */
#define dc_number bc_struct

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

#include "../h/number.h"
#include "dc.h"

#ifdef __GNUC__
# if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__-0 >= 7)
#  define ATTRIB(x) __attribute__(x)
# endif
#endif
#ifndef ATTRIB
# define ATTRIB(x)
#endif

/* Forward prototype */
static void out_char (int);

/* there is no POSIX standard for dc, so we'll take the GNU definitions */
int std_only = FALSE;

/* convert an opaque dc_num into a real bc_num */
/* by a freak accident, these are now no-op mappings,
 * but leave the notation here in case that changes later
 * */
#define CastNum(x)		(x)
#define CastNumPtr(x)	(x)

/* add two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_add(dc_num a, dc_num b, int kscale ATTRIB((unused)), dc_num *result)
{
	bc_init_num(CastNumPtr(result));
	bc_add(CastNum(a), CastNum(b), CastNumPtr(result), 0);
	return DC_SUCCESS;
}

/* subtract two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_sub(dc_num a, dc_num b, int kscale ATTRIB((unused)), dc_num *result)
{
	bc_init_num(CastNumPtr(result));
	bc_sub(CastNum(a), CastNum(b), CastNumPtr(result), 0);
	return DC_SUCCESS;
}

/* multiply two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_mul(dc_num a, dc_num b, int kscale, dc_num *result)
{
	bc_init_num(CastNumPtr(result));
	bc_multiply(CastNum(a), CastNum(b), CastNumPtr(result), kscale);
	return DC_SUCCESS;
}

/* divide two dc_nums, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_div(dc_num a, dc_num b, int kscale, dc_num *result)
{
	bc_init_num(CastNumPtr(result));
	if (bc_divide(CastNum(a), CastNum(b), CastNumPtr(result), kscale)){
		dc_warn("divide by zero");
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* divide two dc_nums, place quotient into *quotient and remainder
 * into *remainder;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_divrem(dc_num a, dc_num b, int kscale, dc_num *quotient, dc_num *remainder)
{
	bc_init_num(CastNumPtr(quotient));
	bc_init_num(CastNumPtr(remainder));
	if (bc_divmod(CastNum(a), CastNum(b),
						CastNumPtr(quotient), CastNumPtr(remainder), kscale)){
		dc_warn("divide by zero");
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* place the reminder of dividing a by b into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_rem(dc_num a, dc_num b, int kscale, dc_num *result)
{
	bc_init_num(CastNumPtr(result));
	if (bc_modulo(CastNum(a), CastNum(b), CastNumPtr(result), kscale)){
		dc_warn("remainder by zero");
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

int
dc_modexp(dc_num base, dc_num expo, dc_num mod, int kscale, dc_num *result)
{
	bc_init_num(CastNumPtr(result));
	if (bc_raisemod(CastNum(base), CastNum(expo), CastNum(mod),
					CastNumPtr(result), kscale)){
		if (bc_is_zero(CastNum(mod)))
			dc_warn("remainder by zero");
		else if (bc_is_neg(CastNum(expo)))
			dc_warn("exponent must be non-negative");
		else
			dc_warn("domain error");
		return DC_DOMAIN_ERROR;
	}
	return DC_SUCCESS;
}

/* place the result of exponentiation a by b into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_exp(dc_num a, dc_num b, int kscale, dc_num *result)
{
	/* this test is lifted out from bc_raise() so that we can handle
	   the error consistently with other dc domain errors: */
	if ( ! bc_is_zero(CastNum(b)) && bc_num2long(CastNum(b)) == 0 ) {
		dc_warn("exponent too large in raise");
		return DC_DOMAIN_ERROR;
	}

	bc_init_num(CastNumPtr(result));
	bc_raise(CastNum(a), CastNum(b), CastNumPtr(result), kscale);
	return DC_SUCCESS;
}

/* take the square root of the value, place into *result;
 * return DC_SUCCESS on success, DC_DOMAIN_ERROR on domain error
 */
int
dc_sqrt(dc_num value, int kscale, dc_num *result)
{
	bc_num tmp;

	tmp = bc_copy_num(CastNum(value));
	if (!bc_sqrt(&tmp, kscale)){
		dc_warn("square root of negative number");
		bc_free_num(&tmp);
		return DC_DOMAIN_ERROR;
	}
	*(CastNumPtr(result)) = tmp;
	return DC_SUCCESS;
}

/* compare dc_nums a and b;
 *  return a negative value if a < b;
 *  return a positive value if a > b;
 *  return zero value if a == b
 */
int
dc_compare(dc_num a, dc_num b)
{
	return bc_compare(CastNum(a), CastNum(b));
}

/* attempt to convert a dc_num to its corresponding int value;
 * if value cannot fit in an (int), return the provided error_mark_result.
 */
int
dc_num2int(dc_num value, int error_mark_result)
{
	long long_result = bc_num2long(CastNum(value));
	int  result = (int)long_result;

	if (result!=long_result || (result==0 && !bc_is_zero(CastNum(value)))){
		dc_warn("value overflows simple integer; punting...");
		return error_mark_result;
	}
	return result;
}

/* convert a C integer value into a dc_num */
/* For convenience of the caller, package the dc_num
 * into a dc_data result.
 */
dc_data
dc_int2data(int value)
{
	dc_data result;

	bc_init_num(CastNumPtr(&result.v.number));
	bc_int2num(CastNumPtr(&result.v.number), value);
	result.dc_type = DC_NUMBER;
	return result;
}

/* get a dc_num from some input stream;
 *  input is a function which knows how to read the desired input stream
 *  ibase is the input base (2<=ibase<=DC_IBASE_MAX)
 *  *readahead will be set to the readahead character consumed while
 *   looking for the end-of-number
 */
/* For convenience of the caller, package the dc_num
 * into a dc_data result.
 */
dc_data
dc_getnum(int (*input)(void), int ibase, int *readahead)
{
	bc_num	base;
	bc_num	result;
	bc_num	build;
	bc_num	tmp;
	bc_num	divisor;
	dc_data	full_result;
	int		negative = 0;
	int		digit;
	int		decimal;
	int		c;

	bc_init_num(&tmp);
	bc_init_num(&build);
	bc_init_num(&base);
	result = bc_copy_num(_zero_);
	bc_int2num(&base, ibase);
	c = (*input)();
	while (isspace(c))
		c = (*input)();
	if (c == '_' || c == '-'){
		negative = c;
		c = (*input)();
	}else if (c == '+'){
		c = (*input)();
	}
	while (isspace(c))
		c = (*input)();
	for (;;){
		if (isdigit(c))
			digit = c - '0';
		else if ('A' <= c && c <= 'F')
			digit = 10 + c - 'A';
		else
			break;
		c = (*input)();
		bc_int2num(&tmp, digit);
		bc_multiply(result, base, &result, 0);
		bc_add(result, tmp, &result, 0);
	}
	if (c == '.'){
		bc_free_num(&build);
		bc_free_num(&tmp);
		divisor = bc_copy_num(_one_);
		build = bc_copy_num(_zero_);
		decimal = 0;
		for (;;){
			c = (*input)();
			if (isdigit(c))
				digit = c - '0';
			else if ('A' <= c && c <= 'F')
				digit = 10 + c - 'A';
			else
				break;
			bc_int2num(&tmp, digit);
			bc_multiply(build, base, &build, 0);
			bc_add(build, tmp, &build, 0);
			bc_multiply(divisor, base, &divisor, 0);
			++decimal;
		}
		bc_divide(build, divisor, &build, decimal);
		bc_add(result, build, &result, 0);
		bc_free_num(&divisor);
	}
	/* Final work. */
	if (negative)
		bc_sub(_zero_, result, &result, 0);

	bc_free_num(&tmp);
	bc_free_num(&build);
	bc_free_num(&base);
	if (readahead)
		*readahead = c;
	*CastNumPtr(&full_result.v.number) = result;
	full_result.dc_type = DC_NUMBER;
	return full_result;
}


/* Return the "length" of the number, ignoring *all* leading zeros,
 * (including those to the right of the radix point!)
 */
int
dc_numlen(dc_num value)
{
	/* XXX warning: unholy coziness with the internals of a bc_num! */
	bc_num num = CastNum(value);
	char *p = num->n_value;
	int i = num->n_len + num->n_scale;

	while (1<i && *p=='\0')
		--i, ++p;
	return i;
}

/* return the scale factor of the passed dc_num;
 * the value is deallocated after use.
 */
int
dc_tell_scale(dc_num value)
{
	int kscale = CastNum(value)->n_scale;
	dc_free_num(&value);
	return kscale;
}


/* initialize the math subsystem */
void
dc_math_init(void)
{
	bc_init_numbers();
}

/* print out a dc_num in output base obase to stdout;
 * if discard_p is DC_TOSS then deallocate the value after use
 */
void
dc_out_num(dc_num value, int obase, dc_discard discard_p)
{
	out_char('\0'); /* clear the column counter */
	bc_out_num(CastNum(value), obase, out_char, 0);
	if (discard_p == DC_TOSS)
		dc_free_num(&value);
}

/* dump out the absolute value of the integer part of a
 * dc_num as a byte stream, without any line wrapping;
 * finally, deallocate the value after use
 */
void
dc_dump_num(dc_num dcvalue)
{
	struct digit_stack { int digit; struct digit_stack *link;};
	struct digit_stack *top_of_stack = NULL;
	struct digit_stack *cur;
	struct digit_stack *next;
	bc_num value;
	bc_num obase;
	bc_num digit;

	bc_init_num(&value);
	bc_init_num(&obase);
	bc_init_num(&digit);

	/* we only handle the integer portion: */
	bc_divide(CastNum(dcvalue), _one_, &value, 0);
	/* we only handle the absolute value: */
	value->n_sign = PLUS;
	/* we're done with the dcvalue parameter: */
	dc_free_num(&dcvalue);

	bc_int2num(&obase, 1+UCHAR_MAX);
	do {
		(void) bc_divmod(value, obase, &value, &digit, 0);
		cur = dc_malloc(sizeof *cur);
		cur->digit = (int)bc_num2long(digit);
		cur->link = top_of_stack;
		top_of_stack = cur;
	} while (!bc_is_zero(value));

	for (cur=top_of_stack; cur; cur=next) {
		putchar(cur->digit);
		next = cur->link;
		free(cur);
	}

	bc_free_num(&digit);
	bc_free_num(&obase);
	bc_free_num(&value);
}

/* deallocate an instance of a dc_num */
void
dc_free_num(dc_num *value)
{
	bc_free_num(CastNumPtr(value));
}

/* return a duplicate of the number in the passed value */
/* The mismatched data types forces the caller to deal with
 * bad dc_type'd dc_data values, and makes it more convenient
 * for the caller to not have to do the grunge work of setting
 * up a dc_type result.
 */
dc_data
dc_dup_num(dc_num value)
{
	dc_data result;

	++CastNum(value)->n_refs;
	result.v.number = value;
	result.dc_type = DC_NUMBER;
	return result;
}



/*---------------------------------------------------------------------------\
| The rest of this file consists of stubs for bc routines called by numeric.c|
| so as to minimize the amount of bc code needed to build dc.                |
| The bulk of the code was just lifted straight out of the bc source.        |
\---------------------------------------------------------------------------*/

#include <stdarg.h>

static int out_col = 0;
static int line_max = -1;	/* negative means "need to check environment" */
#define DEFAULT_LINE_MAX 70

static void
set_line_max_from_environment(void)
{
	const char *env_line_len = getenv("DC_LINE_LENGTH");
	line_max = DEFAULT_LINE_MAX;
	errno = 0;
	if (env_line_len) {
		char *endptr;
		long proposed_line_len = strtol(env_line_len, &endptr, 0);
		line_max = (int)proposed_line_len;

		/* silently enforce sanity */
		while (isspace(*(const unsigned char *)endptr))
			++endptr;
		if (*endptr || errno || line_max != proposed_line_len
					|| line_max < 0 || line_max == 1)
			line_max = DEFAULT_LINE_MAX;
	}
}

/* Output routines: Write a character CH to the standard output.
   It keeps track of the number of characters output and may
   break the output with a "\<cr>". */

static void
out_char (int ch)
{
	if (ch == '\0') {
		out_col = 0;
	} else {
		if (line_max < 0)
			set_line_max_from_environment();
		if (++out_col >= line_max && line_max != 0) {
			putchar('\\');
			putchar('\n');
			out_col = 1;
		}
		putchar(ch);
	}
}

/* Malloc could not get enough memory. */

void
out_of_memory(void)
{
	dc_memfail();
}

/* Runtime error --- will print a message and stop the machine. */

void
rt_error(const char *mesg, ...)
{
	va_list args;

	fprintf(stderr, "Runtime error: ");
	va_start(args, mesg);
	vfprintf(stderr, mesg, args);
	va_end(args);
	fprintf(stderr, "\n");
}


/* A runtime warning tells of some action taken by the processor that
   may change the program execution but was not enough of a problem
   to stop the execution. */

void
rt_warn(const char *mesg, ...)
{
	va_list args;

	fprintf(stderr, "Runtime warning: ");
	va_start(args, mesg);
	vfprintf(stderr, mesg, args);
	va_end(args);
	fprintf(stderr, "\n");
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
