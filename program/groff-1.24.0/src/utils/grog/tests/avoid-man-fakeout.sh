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

grog="${abs_top_builddir:-.}/grog"

# Regression-test Savannah #66006.
#
# Don't confuse input that uses `TH` inside tbl(1) regions for man(7)
# documents.

input='.
.TS
tab(@)
L L.
Heading 1@Heading 2
.TH
datum 1@datum 2
datum 3@datum 4
.TE
.'

output=$(echo "$input" | "$grog")
echo "$output"
echo "$output" | grep -Fqx 'groff -t -'

# vim:set ai et sw=4 ts=4 tw=72:
