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

# Regression-test Savannah #66380.
#
# Blocked letters should put a blank line before the date.

input='.
.WA
123 Main Street
Anytown, ST  10101
.WE
.ND "26 October 2024"
.IA
456 Elsewhere Avenue
Nirvana, PA  20406
.IE
.LT
.P
We have a research leak!
The next person I catch embedding engineering samples of our Lightspeed
Overdrive 2048-core processors in cork coasters distributed at trade
shows is going to regret it.
.FC
.SG
.NS
sundry careless people
.NE
.'

output=$(echo "$input" | "$groff" -m m -T ascii -P -cbou)
echo "$output"

echo "$output" | sed -n 15p | grep -q '26 October 2024'

# vim:set ai et sw=4 ts=4 tw=72:
