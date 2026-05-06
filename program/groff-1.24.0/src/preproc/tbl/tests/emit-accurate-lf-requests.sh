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
#

tbl="${abs_top_builddir:-.}/tbl"

# Regression-test Savannah #66931.
#
# When writing out a file name argument to the `lf` request, ensure that
# the line number isn't always set to "1".

input='.
.TS
L.
foo
.TE
.'

echo "$input" | "$tbl" | grep '^\.lf [2-9][0-9]* "..*'

# vim:set ai et sw=4 ts=4 tw=72:
