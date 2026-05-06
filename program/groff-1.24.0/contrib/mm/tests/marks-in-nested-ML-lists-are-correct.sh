#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
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
    echo ...FAILED >&2
    fail=YES
}

# Regression-test Savannah #68052.  Nesting `ML` lists should restore
# the correct list item marks as un-nesting occurs.  This input is based
# on a reproducer by Alexis Hildebrandt.

input='.SA 0
.P
This is an
.I mm
document with nested
.B ML
marked lists.
.ML !
.LI
one
.ML #
.LI
two
.ML $
.LI
three
.LE
.LI
four
.LE
.LI
five
.LE'

output=$(echo "$input" | "$groff" -mm -Tascii -P -cbou)
echo "$output"

echo "checking mark on first item" >&2
echo "$output" \
    | grep -qx ' *! one' || wail

echo "checking mark on second item" >&2
echo "$output" \
    | grep -qx ' *# two' || wail

echo "checking mark on third item" >&2
echo "$output" \
    | grep -qx ' *$ three' || wail

echo "checking mark on fourth item" >&2
echo "$output" \
    | grep -qx ' *# four' || wail

echo "checking mark on fifth item" >&2
echo "$output" \
    | grep -qx ' *! five' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
