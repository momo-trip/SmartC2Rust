#!/bin/sh
#
# Copyright 2025 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# groff is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

pic="${abs_top_builddir:-.}/pic"

# Regression-test Savannah #67899.  Thanks to John de Armas for the
# reproducer (based on an example from "doc/pic.ms").

if [ -e core ]
then
    echo "$0: 'core' file already exists; skipping" >&2
    exit 77 # skip
fi


input='
.PS
# Plot a single jumper in a $1 by $2 box, $3 is the on-off state
    define jumper { [
    shrinkfactor = 0.8;
    Outer: box invis wid 0.5 ht 1;

    # Count on end ] to reset these
    boxwid = Outer.wid * shrinkfactor / 2;
    boxht = Outer.ht * shrinkfactor / 2;

    box fill (!$1) with .s at center of Outer;
    box fill ($1) with .n at center of Outer;
] }

# Plot a block of six jumpers
define jumperblock {
    jumper($1);
    jumper($2);
    jumper($3);
    jumper($4);
    jumper($5);
    jumper($6);

    jwidth = last [].Outer.wid;
    jheight = last [].Outer.ht;

    box with .nw at 6th last [].nw wid 6*jwidth ht jheight;

    # Use {} to avoid changing position from last box draw.
    # This is necessary so move in any direction will work as expected
    {"Jumpers in state $1$2$2$3$4$5$6" at last box .s + (0, -0.2);}
}
# Sample macro invocations
jumperblock(1,1,0,0,1,0);
move;
jumperblock(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,\
            23,24,25,26,27,28,29,30,31,,0AAAA,1AAAB,,,,,,7AAAH,,9AAAJA,\
            10AAAK,11AAAL,12AAAM,13AAAN,14AAAO,15AAAP,16AAAQ,17AAAR,\
            18AAAS,19AAAT,20AAAU,21AAAAAAV);
.PE
'

echo "$input" | "$pic"
! test -f core

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
