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
 * copy, modify, merge, publish, distribute, sublicense and/or sell copies of
 * the Software, and to permit persons who receive copies from any such
 * party to do so, with the only requirement being that the above copyright
 * and this permission notice remain intact.
 *
 */

/*
 *	test1.c: Test {floor,ceil}_coords() for overflow.
 *	Author: Thomas Loimer, 2017-2022
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <limits.h>
#include <stdio.h>

#include "object.h"
#include "mode.h"
#include "w_canvas.h"

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	int	x, y;
	int	spacing;
	int	err = 0;

	appres.userscale = 1.0;
	anypointposn = 0;
	cur_gridunit = FRACT_UNIT;
	cur_gridtype = GRID_SQUARE;
	cur_pointposn = P_GRID1;

	spacing = point_spacing();

#define	BIGGER(X, VAL)		X > VAL + spacing
#define SMALLER(X, VAL)		X < VAL - spacing
#define	TEST_BOUND(FUNC, XVAL, YVAL, TEST, NUM)		\
		do {					\
			x = XVAL; y = YVAL;		\
			FUNC(&x, &y);			\
			if (TEST)			\
				err |= 1 << NUM;	\
		}	while (0)

	TEST_BOUND(floor_coords, INT_MIN, 0, BIGGER(x, INT_MIN), 0);
	TEST_BOUND(floor_coords, 0, INT_MIN, BIGGER(y, INT_MIN), 1);
	TEST_BOUND(ceil_coords, INT_MAX, 0, SMALLER(x, INT_MAX), 2);
	TEST_BOUND(ceil_coords, 0, INT_MAX, SMALLER(y, INT_MAX), 3);

	cur_gridtype = GRID_ISO;
	cur_pointposn = P_GRID2;
	spacing = point_spacing();

	TEST_BOUND(floor_coords, INT_MIN, 0, BIGGER(x, INT_MIN), 4);
	TEST_BOUND(floor_coords, 0, INT_MIN, BIGGER(y, INT_MIN), 5);
	TEST_BOUND(ceil_coords, INT_MAX, 0, SMALLER(x, INT_MAX), 6);
	TEST_BOUND(ceil_coords, 0, INT_MAX, SMALLER(y, INT_MAX), 7);

	if (err)
		fprintf(stderr, "error code: %d\n", err);
	return err;
}
