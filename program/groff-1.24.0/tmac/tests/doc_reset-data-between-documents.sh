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
    fail=yes
}

# Regression-test Savannah #65480.
#
# _Any_ document transition should wipe the page header/footer data,
# reset interpolation behavior of the `Nm` macro, and so forth.

input='.Dd 2024-03-18
.Dt foo 1
.Os
.Sh Name
.Nm foo
.Nd frobnicate a bar
.Sh Description
.Nm
is a program.
.Dd 2024-03-18
.Dt bar 1
.Os
.Sh Name
.Nm bar
.Nd format documents rebarbatively
.Sh Description
.Nm
is also a program.'

# Expected (with -rLL=70n argument added and consecutive blank lines
# condensed):
#
# foo(1)                  General Commands Manual                 foo(1)
#
# Name
#      foo -- frobnicate a bar
#
# Description
#      foo is a program.
#
# GNU                           2024-03-18                             1
#
# bar(1)                  General Commands Manual                 bar(1)
#
# Name
#      bar -- format documents rebarbatively
#
# Description
#      bar is also a program.
#
# GNU                           2024-03-18                             1
#

echo "checking text interpolated by Nm macro with continuous" \
    "rendering" >&2
output=$(printf "%s\n" "$input" | "$groff" -rcR=1 -mdoc -Tascii -P-cbou)
echo "$output"
echo "$output" | grep -q 'bar is also a program\.' || wail

echo "checking text interpolated by Nm macro without continuous" \
    "rendering" >&2
output=$(printf "%s\n" "$input" | "$groff" -rcR=0 -mdoc -Tascii -P-cbou)
echo "$output"
echo "$output" | grep -q 'bar is also a program\.' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
