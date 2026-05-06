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

groff="${abs_top_builddir:-.}/test-groff"

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Regression-test Savannah #67420.
#
# Correct box-drawing glyphs should appear between the heading and body
# of boxed `TH` tables on the "utf8" output device.

input='.
.LP
This is an
.I ms
document.
.TS H
allbox;
C
L.
Heading
.TH
some data
.TE
.pl \n(nlu
.'

output=$(printf "%s\n" "$input" | "$groff" -t -m s -T utf8)
echo "$output"
output=$(echo "$output" | sed -n '11p' | od -t x1)
echo "$output"

# Expected output:
# 0000000 e2 94 9c e2 94 80 e2 94 80 e2 94 80 e2 94 80 e2
# 0000020 94 80 e2 94 80 e2 94 80 e2 94 80 e2 94 80 e2 94
# 0000040 80 e2 94 80 e2 94 a4 0a
# 0000050

echo "checking that left edge of table after heading looks like |-" >&2
echo "$output" | grep -q '0000000  *e2  *94  *9c' || wail

echo "checking that right edge of table after heading looks like -|" >&2
echo "$output" | grep -q '0000040 .*  *e2  *94  *a4  *0. *$' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
