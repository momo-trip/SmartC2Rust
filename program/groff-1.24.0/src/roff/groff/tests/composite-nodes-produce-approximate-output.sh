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

groff="${abs_top_builddir:-.}/test-groff"

# Regression-test Savannah #55799.
#
# Verify that the formatter traverses into the contents of composite
# nodes when formatting them for approximate output.

input='.
.char a xyz
Men plan,
God laughs.
.'

output=$(printf "%s\n" "$input" | "$groff" -T ps -a)
echo "$output"
echo "$output" | grep -q 'Men plxyzn, God lxyzughs.'

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
