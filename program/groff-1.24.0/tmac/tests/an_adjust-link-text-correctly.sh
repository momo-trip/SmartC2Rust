#!/bin/sh
#
# Copyright 2022 G. Branden Robinson
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

fail=

wail() {
    echo ...FAILED >&2
    fail=yes
}

# Ensure that link text (when the hyperlink itself is not formatted
# because the device supports hyperlinking) uses the correct line length
# and is adjusted.

# The 'XX' provokes hyphenation interior to the line with certain
# incorrect approaches to link text handling (forgetting to copy
# environment 0 to the diversion's environment).
input='.TH foo 1 2022-11-08 "groff test suite"
.SH "See also"
.
.UR http://\:www\:.hp\:.com/\:ctg/\:Manual/\:bpl13210\:.pdf
.I HP PCL/PJL Reference:
.I PCL\~5XX Printer Language Technical Reference Manual,
.I Part I
.UE'

# Expected:
#   PCL 5XX Printer Language Technical Reference  Manual,

echo "checking formatting of whole-line link text" >&2
output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -rU1)
echo "$output"
echo "$output" \
    | grep -q 'Printer Language Technical Reference  Manual,$' || wail

input='.TH foo 1 2022-11-08 "groff test suite"
.SH "See also"
.
Consult
.UR http://\:www\:.hp\:.com/\:ctg/\:Manual/\:bpl13210\:.pdf
.I HP PCL/PJL Reference:
.I PCL\~5 Printer Language Technical Reference Manual,
.I Part I
.UE .'

# Expected:
#   PCL 5 Printer  Language  Technical  Reference

echo "checking formatting of partial-line link text" >&2
output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -rU1)
echo "$output"
# 2 spaces each
echo "$output" \
    | grep -q 'Printer  Language  Technical  Reference$' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
