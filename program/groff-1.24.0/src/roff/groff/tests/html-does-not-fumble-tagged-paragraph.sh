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

# A paragraph tag should typeset on one line if it's shorter than the
# line length.

input='.TH foo 1 2024-06-19 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Description
.PP
A character definition can be removed with the
.B rchar
request.
.
.
.TP
.BI .chop\~ name
Remove the last character from the macro,
string,
or diversion
.IR name .'

output=$(echo "$input" | "$groff" -man -T html -Z)
echo "$output"

echo "$output" | grep -qx 't *\.chop'

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
