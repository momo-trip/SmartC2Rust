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

wail() {
    echo ...FAILED >&2
    fail=yes
}

# Verify that references format correctly.

input='.
.Dd 2024-08-25
.Dt foo 1
.Os "groff test suite"
.Sh Name
.Nm foo
.Nd frobnicate a bar
.Sh Description
Who needs documentation anyway?
.Sh "See also"
.Rs
.%A "I.\\& M.\\& Cl\[e aa]ver"
.%T "A Na\[i ad]ve Topological Sort"
.%Q "Harry Mudd University Press"
.%C "Claremont, California"
.%D "2017-07-10"
.Re
.'

# Expected:
#
# ...
# See also
#      I.    M.    Clever,   A Naive Topological Sort,   Harry Mudd
#      University Press, Claremont, California, 2017-07-10.
# ...

echo "checking formatted output of 'Rs'/'Re' macros" >&2
output=$(echo "$input" | "$groff" -r LL=65n -m doc -T ascii -P -cbou)
echo "$output"
echo "$output" | grep -Eq 'I\. +M\. +Clever, +A +Naive +Topological' \
  || wail

echo "checking that 'Rs'/'Re' macros produce no diagnostics" >&2
errors=$(echo "$input" | "$groff" -r LL=65n -m doc -T ascii -z 2>&1)
echo "$errors"
test -z "$errors" || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
