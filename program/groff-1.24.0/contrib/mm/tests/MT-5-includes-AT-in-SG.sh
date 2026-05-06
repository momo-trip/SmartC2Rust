#!/bin/sh
#
# Copyright 2021 G. Branden Robinson
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

# Regression-test Savannah #57034.
#
# Ensure that an author's title (if any) is written in the signature.
#
# Thanks to Ken Mandelberg for the reproducer.

input='.TL
Inquiry
.AU "John SMITH"
.AT "Director"
.MT 5
.P
sentence
.FC Sincerely,
.SG'

output=$(echo "$input" | "$groff" -mm -Tascii -P-cbou)
echo "$output"

echo "$output" | grep -Eqx '[[:space:]]+Director[[:space:]]*'

# vim:set ai et sw=4 ts=4 tw=72:
