#!/bin/sh
#
# Copyright 2024 G. Branden Robinson
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

# Unit-test ML list mark application, replacement, and prefixing.

input='.SA 0
.P
This is an
.I mm
document with an
.B ML
marked list.
.ML *
.LI
default mark, no prefix
.LI +
mark replaced
.LI + 1
mark prefixed (with a padding space)
.LE'

output=$(echo "$input" | "$groff" -mm -Tascii -P -cbou)
echo "$output"

echo "checking ML list with 0-argument LI call" >&2
echo "$output" \
    | grep -qx ' *\* default mark, no prefix' || wail

echo "checking ML list with 1-argument LI call" >&2
echo "$output" \
    | grep -qx ' *+ mark replaced' || wail

echo "checking ML list with 2-argument LI call" >&2
echo "$output" \
    | grep -qx ' *+ \* mark prefixed (with a padding space)' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
