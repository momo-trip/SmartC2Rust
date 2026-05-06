#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
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
.sp
This is a
.pdfhref M -E raw
.ft I
groff
.ft
document,
dude.
.bp
.sp
Okay,
so this is page two,
and we are going to
.pdfhref L -D raw -A (tm) roller boogie
our booties back to the named destination on page one.
.bp
.sp
It is widely believed that the
.ft I
groff
.ft
home page lives at
.pdfhref W -D https://www.gnu.org/software/groff/ -A .
.'

output=$(echo "$input" | "$groff" -a -T pdf)
echo "$output"
echo "$output" | grep -Fq "www.gnu.org/software/groff"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
