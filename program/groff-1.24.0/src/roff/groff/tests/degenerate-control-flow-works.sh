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
  echo ...FAILED >&2
  fail=YES
}

input='.nm 1
@
.if 0
A
.if !0
B
.ie 0
C
.el
D
E'

output=$(echo "$input" | "$groff" -W el -T ascii)
echo "$output"

# Expected:
#   1 @ A
#
#   2 B C
#
#   3 D E

echo "checking truncated control flow request behavior (1/3)" >&2
echo "$output" | grep -q '1 @ A$' || wail

echo "checking truncated control flow request behavior (2/3)" >&2
echo "$output" | grep -q '2 B C$' || wail

echo "checking truncated control flow request behavior (3/3)" >&2
echo "$output" | grep -q '3 D E$' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
