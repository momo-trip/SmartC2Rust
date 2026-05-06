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

eqn="${abs_top_builddir:-.}/eqn"

# Regression-test Savannah #67285.  eqn should not strip "invalid"
# characters from parts of a document it does not interpret.

LC_ALL=C
export LC_ALL

output=$(printf '.\\" degree sign: \313\232\n' | "$eqn" -R)
printf "%s\n" "$output"
printf "%s\n" "$output" | od -c
printf "%s\n" "$output" | od -c | grep -q ' 232'

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
