#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
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

# Regression-test Savannah #68059.

input='.
.P
This is an
.I mm
document using an
.B ML
marked list that attempts to suppress vertical spacing
before each list item
by specifying a third argument to the
.B ML
call.
.ML * "" 1
.LI
alpha
.LI
beta
.LI
gamma
.LE
.'

output=$(echo "$input" | "$groff" -m m -T ascii -P -cbou \
    | head -n 18 | nl -ba)
echo "$output"

echo "$output" \
    | grep -Eq '^ +13[ 	]+\* *beta' # 1 space, 1 tab in brackets

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
