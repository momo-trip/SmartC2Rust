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

# Unit-test LB macro mark-or-format argument.

input='.
.P
This is an
.I mm
document.
.LB 4n 0 1 1 a 0 1
.LI
alpha
.LE
.LB 4n 0 1 1 A 0 1
.LI
bravo
.LE
.LB 4n 0 1 1 i 0 1
.LI
charlie
.LE
.LB 4n 0 1 1 I 0 1
.LI
delta
.LE
.LB 4n 0 1 1 0 0 1
.LI
echo
.LE
.LB 4n 0 1 1 99 0 1
.LI
foxtrot
.LE
.'

output=$(echo "$input" | "$groff" -mm -Tascii -P-cbou)
echo "$output"

echo "checking rendering of 'a' format" >&2
echo "$output" | grep -Fq 'a. alpha' || wail

echo "checking rendering of 'A' format" >&2
echo "$output" | grep -Fq 'A. bravo' || wail

echo "checking rendering of 'i' format" >&2
echo "$output" | grep -Fq 'i. charlie' || wail

echo "checking rendering of 'I' format" >&2
echo "$output" | grep -Fq 'I. delta' || wail

echo "checking rendering of '0' format" >&2
echo "$output" | grep -Fq '1. echo' || wail

echo "checking rendering of '99' format" >&2
echo "$output" | grep -Fq '01. foxtrot' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
