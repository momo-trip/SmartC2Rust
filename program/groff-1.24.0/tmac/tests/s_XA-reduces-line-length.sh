#!/bin/sh
#
# Copyright 2024 G. Branden Robinson
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

fail=

wail () {
    echo "...FAILED" >&2
    fail=YES
}

# Regression-test Savannah #66328.

input='.
.ds C1 Chapter the First, \
in which we Ramble On at Excessive Length for the Purpose of \
Contriving a Table of Contents Entry So Garrulous That It Will Surely \
Overrun the Line in Any Sensible Page Configuration\"
.ds C2 Chapter the Second, \
in which we Do Much the Same, \
but Apply an Added Twist\"
.ds C3 Of a Supplement for the Table of Contents, \
Which in a Traditional Work Might Be Something Like \
an Em-Dash-Separated List of Subsections or Sundry Other Topics \
Covered Therein
.NH 1
\*(C1
.XS
\*(C1
.XE
.LP
This is an
.I ms
document.
.NH 1
\*(C2
.XS
\*(C2
.XA no
.in 8n
\*(C3
.in
.XE
.LP
It is part of a regression test.
.TC
.'

# Expected output (page 2 only):
#                                -i-
#
#
#
#                         Table of Contents
#
#
# Chapter the First, in which we Ramble On at Excessive
# Length for the Purpose of Contriving a Table of Contents
# Entry So Garrulous That It Will Surely Overrun the Line
# in Any Sensible Page Configuration  . . . . . . . . . . . . .   1
# Chapter the Second, in which we Do Much the Same, but Ap-
# ply an Added Twist  . . . . . . . . . . . . . . . . . . . . .   1
#         Of a Supplement for the Table of Contents, Which
#         in a Traditional Work Might Be Something Like an
#         Em-Dash-Separated List of Subsections or Sundry
#         Other Topics Covered Therein

output=$(printf '%s\n' "$input" | "$groff" -Tascii -P-cbou -ms)
echo "$output"

echo "testing line length of primary TOC entry" >&2
echo "$output" | grep -qx 'Chapter the First.*Excessive' || wail

echo "testing line length of supplemental TOC entry" >&2
echo "$output" | grep -qx ' *Of a Supplement.*, Which' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
