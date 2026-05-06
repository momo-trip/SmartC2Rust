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
.Dd 2025-10-08
.Dt foo 1
.Os "groff test suite"
.Sh Name
.Nm foo
.Nd frobnicate a bar
.
.Dd 2025-10-08
.Dt bar 1
.Os "groff test suite"
.Sh Name
.Nm bar
.Nd barnicate a foo
.Sh Description
#############################################\
#############################################
.'

error=$(printf "%s\n" "$input" \
    | "$groff" -mdoc -T ascii -P -cbou -z 2>&1)
output=$(printf "%s\n" "$input" \
    | "$groff" -mdoc -T ascii -P -cbou 2> /dev/null)
echo "$error"
echo "$output"
echo "$error" | grep -Fq 'line 14'

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
