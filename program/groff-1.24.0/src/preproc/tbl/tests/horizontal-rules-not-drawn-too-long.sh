#!/bin/sh
#
# Copyright 2023 G. Branden Robinson
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

# Regression-test Savannah #62471.
#
# Horizontal rules should not be drawn too long in nroff mode.

input='.
.ll 65n
.ec @
.nf
1234567890@
1234567890@
1234567890@
1234567890@
1234567890@
1234567890@
12345
.fi
@l?65n@&*?
.ec
.TS
L.
table with no boxes or horizontal rules
.TE
.sp
.TS
box;
Lx.
boxed table
.TE
.sp
.TS
Lx.
foo
_
bar
.TE
.sp
.TS
box;
Lx.
boxed table with
_
horizontal rule in data
.TE'

output=$(printf "%s\n" "$input" | "$groff" -t -Tascii)
echo "$output"
test $(echo "$output" | grep -Ecx '[-]{65}') -eq 1

# vim:set ai et sw=4 ts=4 tw=72:
