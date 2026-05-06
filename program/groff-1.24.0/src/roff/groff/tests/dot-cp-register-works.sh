#!/bin/sh
#
# Copyright 2020-2024 G. Branden Robinson
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

input='.
.pl 1v
A
.do if 1 \n[.cp] \" Get initial compatibility state (depends on -C).
B
.do if 1 \n[.cp] \" Did observing the state change it?
.cp 1
C
.do if 1 \n[.cp] \" Saved compatibility state should be 1 now.
.cp 0
D
.do if 1 \n[.cp] \" Verify 1->0 transition.
.cp 1
E
.do if 1 \n[.cp] \" Verify 0->1 transition.
.cp 0
F
.if !\n[.C] \n[.cp] \" Outside of .do context, should return -1.
.'

# Expected:
#
# A 0 B 0 C 1 D 0 E 1 F 0
# A 1 B 1 C 1 D 0 E 1 F 0

output=$(printf "%s" "$input" | "$groff" -T ascii)
echo "$output"

echo "checking value of '.cp' when not started in compatibility mode" \
  >&2
echo "$output" | grep -Fqx "A 0 B 0 C 1 D 0 E 1 F 0" || wail

output=$(printf "%s" "$input" | "$groff" -C -T ascii)
echo "$output"

echo "checking value of '.cp' when started in compatibility mode" \
  >&2
echo "$output" | grep -Fqx "A 1 B 1 C 1 D 0 E 1 F 0" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
