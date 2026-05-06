/*
 * Header file for dc routines
 *
 * Copyright (C) 1994, 1997, 1998, 2008, 2017, 2022
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

#ifndef DC_DEFS_H
#define DC_DEFS_H

/* 'I' is a command, and bases 17 and 18 are quite
 * unusual, so we limit ourselves to bases 2 to 16
 */
#define DC_IBASE_MAX	16

#define DC_SUCCESS		0
#define DC_DOMAIN_ERROR	1
#define DC_FAIL			2	/* generic failure */
#define DC_EXIT_REQUEST	3


typedef enum {DC_TOSS, DC_KEEP}   dc_discard;
typedef enum {DC_NONL, DC_WITHNL} dc_newline;


/* type discriminant for dc_data */
typedef enum {DC_UNINITIALIZED, DC_NUMBER, DC_STRING} dc_value_type;

/* only numeric.c knows what dc_num's *really* look like */
typedef struct dc_number *dc_num;

/* only string.c knows what dc_str's *really* look like */
typedef struct dc_string *dc_str;


/* except for the two implementation-specific modules, all
 * dc functions only know of this one generic type of object
 */
typedef struct {
	dc_value_type dc_type;	/* discriminant for union */
	union {
		dc_num number;
		dc_str string;
	} v;
} dc_data;


/* Global variables: */
extern const char *progname;	/* basename of program invocation */
extern unsigned int max_recursion_depth;

/* everyone like to see prototypes... */
#include "dc-proto.h"

#endif /* not DC_DEFS_H */


/*
 * Local Variables:
 * mode: C
 * tab-width: 4
 * End:
 * vi: set ts=4 :
 */
