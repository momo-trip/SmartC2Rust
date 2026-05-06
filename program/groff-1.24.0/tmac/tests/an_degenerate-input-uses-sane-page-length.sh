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

input='.
.P
This is my paragraph.
.'

output=$(echo "$input" | "$groff" -rcR=1 -man -T ascii -P -cbou)
echo "$output"

# We might have 2 blank lines for the header and footer, and up to one
# for inter-paragraph spacing.
test $(echo "$output" | wc -l) -lt 5

# vim:set ai et sw=4 ts=4 tw=72:
