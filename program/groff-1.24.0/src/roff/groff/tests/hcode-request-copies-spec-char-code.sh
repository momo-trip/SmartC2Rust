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

# Test use of special characters as sources for hyphenation codes.
# See Savannah #66054.  Thanks to Dave Kemper.

input=".
.ec @
.ll 1n
r@['e]sum@['e]
.hcode @['e] e
r@['e]sum@['e]
.hcode @['E] @['e]
R@['E]SUM@['E]
.pl @n[nl]u
."

output=$(echo "$input" | "$groff" -a -ww -Wbreak)
echo "$output"

# Expected output:
#
# <beginning of page>
# r<'e>sum<'e>
# r<'e><hy>
# sum<'e>
# R<'E><hy>
# SUM<'E>

echo "$output" | grep -Fqx "R<'E><hy>"

# vim:set ai et sw=4 ts=4 tw=72:
