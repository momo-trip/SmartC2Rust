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

groff="${abs_top_builddir:-.}/test-groff"

input='.
.mso rfc1345.tmac
\[!I]Dios mio!
.pl \n[nl]u
.'

# More than 12 characters of output could mean junk spaces crept in.
output=$(printf "%s\n" "$input" | "$groff" -T utf8 2>&1)
echo "$output"
test $(echo "$output" | wc -c) -eq 12

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
