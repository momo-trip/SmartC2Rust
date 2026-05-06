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

input='.
.TH foo 1 2025-07-24 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Synopsis
.SY foo
.RI [ antidisestablishmentarianism
.RI [ pneumonoultramicroscopicsilicovolcanoconiosis ]]
.YS
.'

# Expected output:
#
# foo(1)                  General Commands Manual                 foo(1)
#
# Name
#      foo - frobnicate a bar
#
# Synopsis
#      foo [antidisestablishmentarianism
#          [pneumonoultramicroscopicsilicovolcanoconiosis]]
#
# groff test suite              2025-07-24                        foo(1)

output=$(printf "%s\n" "$input" \
    | "$groff" -rLL=70n -man -Tascii -P-cbou)
echo "$output"
echo "$output" | grep -q antidisestablishmentarianism \
  && echo "$output" | grep -q \
    pneumonoultramicroscopicsilicovolcanoconiosis

exit

# vim:set ai et sw=4 ts=4 tw=72:
