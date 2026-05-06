/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-2015 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 * Parts Copyright (c) 2016-2022 by Thomas Loimer
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and documentation
 * files (the "Software"), including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense and/or sell copies
 * of the Software, and to permit persons who receive copies from any such
 * party to do so, with the only requirement being that the above copyright
 * and this permission notice remain intact.
 *
 */

/*
 *	test4.c: Test whether point_spacing() returning 0 causes an exception,
 *		ticket #144.
 *	Author: Thomas Loimer, 2022-07-20
 *
 * Make appres.userscale (xfig -userscale 100) large enough such that
 * point_spacing() returns 0 and round_coords() calls round_square(), which
 * eventually tries  x % spacing  , causing SIGFPE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <limits.h>
#include <math.h>

#include "resources.h"	/* must come before mode.h; struct appres */
#include "mode.h"

extern void round_coords(int *x, int *y);		/* w_canvas.h */

/*
 * defined in mode.h:
 *	cur_gridunit		MM_UNIT, FRACT_UNIT, TENTH_UNIT
 *	cur_pointposn		P_ANY, P_MAGNET, P_GRID1, ..., P_GRID4
 *	anypointposn		True or False
 */


int
main(int argc, char *argv[])
{
	(void)	argc;
	(void)	argv;
	int	x = 1;
	int	y = 1;
	appres.userscale = 100.0f;
	anypointposn = 0;
	cur_pointposn = P_GRID1;
	round_coords(&x, &y);
	return 0;
}
