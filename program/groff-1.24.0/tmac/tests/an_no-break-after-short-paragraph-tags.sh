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

# Regression-test Savannah #60624.
#
# A page break should not follow the tag in a tagged paragraph when the
# tag can fit within the indentation.

input='.TH foo 1 2021-05-18 "groff test suite"
.SH Name
foo \- frobnicate a thingamajig
.rs \" force spacing on
.sp 50
.TP
3
7th edition'

output=$(echo "$input" | "$groff" -Tascii -P-cbou -man -rcR=0)
echo "$output"

echo "$output" | grep -Eqx ' +3 +7th edition'

# vim:set ai et sw=4 ts=4 tw=72:
