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

pic="${abs_top_builddir:-.}/pic"

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Smoke-test GNU pic `polygon` keyword extension.

input='.
.sp 2i
.PS
polygon up 1 then right 1 down 0.5 fill 0.5
.PE
.'

output=$(echo "$input" | "$pic")
printf "%s\n" "$output"

echo "checking that polygon command draws stroked polygon" >&2
printf "%s\n" "$output" \
    | grep -Eq '\\D.p 0(\.0*)i -1(\.0*)i 1(\.0*)i 0.50*i.' || wail

echo "checking that fill modifier draws filled polygon" >&2
printf "%s\n" "$output" \
    | grep -Eq '\\D.P 0(\.0*)i -1(\.0*)i 1(\.0*)i 0.50*i.' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
