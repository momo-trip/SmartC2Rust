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
#

groff="${abs_top_builddir:-.}/test-groff"

# Regression-test Savannah #64267.

input='.
.TH foo 1 2024-03-15 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Monsters
In this game reminiscent of
.IR rogue (6),
you can expect to encounter the following opponents in increasing order
of menace.
.
.PD 0
.IP B
bat
.IP T
troll
.P
.PD \" inexpert page author did not put this _before_ the `P` call
.SH Treasure
There is a lot of it.
.'

# Expected output:
#
# foo(1)                  General Commands Manual                 foo(1)
#
# Name
#      foo - frobnicate a bar
#
# Monsters
#      In this game reminiscent of rogue(6), you can expect to encounter
#      the following opponents in increasing order of menace.
#      B      bat
#      T      troll
#
# Treasure
#      There is a lot of it.
#
# groff test suite              2024-03-15                        foo(1)

output=$(printf "%s\n" "$input" \
    | "$groff" -rLL=70n -man -Tascii -P-cbou)
echo "$output"
echo "$output" | sed -n -e '/troll/{' \
    -e 'n;/^$/{' \
    -e 'n;/Treasure/p;' \
    -e '}' \
    -e '}' | grep -q .

exit

# vim:set ai et sw=4 ts=4 tw=72:
