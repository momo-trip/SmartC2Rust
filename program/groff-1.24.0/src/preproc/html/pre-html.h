/* Copyright (C) 2000-2020 Free Software Foundation, Inc.
 * Written by Gaius Mulley (gaius@glam.ac.uk).
 *
 * This file is part of groff, the GNU roff typesetting system.
 *
 * groff is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * groff is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *  defines functions implemented within pre-html.cpp
 */

#if !defined(PREHTMLH)
#  define PREHTMLH
#  if defined(PREHTMLC)
#     define EXTERN
#  else
#     define EXTERN extern
#  endif


extern void sys_fatal (const char *s);

#undef EXTERN
#endif

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
