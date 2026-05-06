#!/bin/sh
#
# Copyright 2025 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it over
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
  fail=yes
}

input='.
.nf
.bd 1 10
+W
.bd 1
+X
.bd S TB 20
.ft TB
\(plY
.bd S TB
\(plZ
.'

output=$(printf '%s\n' "$input" | "$groff" -Z)
echo "$output"

output=$(echo "$output" | tr '\n' ' ')

# Expected (with line breaks added for comment readability):
# x T ps x res 72000 1 1 x init p1 x font 5 TR f5 s10000 V12000 H72000
# md DFd c+h9 t+ cWh9 tW n12000 0 V24000 H72000 t+X n12000 0 x font 11
# S f11 V36000 H72000 Cpl h19 Cpl x font 38 TB f38 h5490 tY n12000 0 f11
# V48000 H72000 Cpl f38 h5490 tZ n12000 0 x trailer V792000 x stop

echo "checking that unconditional emboldening works" >&2
echo "$output" | grep -Eq 'c *\+ *h *9 [ct] *\+ cW *h *9' || wail

echo "checking that unconditional un-emboldening works" >&2
echo "$output" | grep -Eq 't *\+X n' || wail

echo "checking that unconditional emboldening works" >&2
echo "$output" | grep -Eq 'C *pl +h *19 +C *pl +x font [0-9]+ +TB +f *[0-9]+ +h *[0-9]+ +[ct] *Y' || wail

echo "checking that unconditional un-emboldening works" >&2
echo "$output" | grep -Eq 'C *pl +f *[0-9]+ +h *[0-9]+ +[ct] *Z' || wail

# Deal with the ambiguous case.

input='.
.bd 1 2 3
foo
.'

output=$(printf '%s\n' "$input" | "$groff" -Z)
echo "$output"

output=$(echo "$output" | tr '\n' ' ')

# Expected (with line breaks added for comment readability):
# x T ps x res 72000 1 1 x init p1 x font 5 TR f5 s10000 V12000 H72000
# md DFd cfh1 tf coh1 to coh1 to n12000 0 x trailer V792000 x stop

echo "checking that two numeric arguments cause third to be ignored" >&2
echo "$output" | grep -Eq 'c *f *h *1 [ct] *f c *o *h *1' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
