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
.TH foo 3 2025-07-24 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Synopsis
.ad l
.br
.B void *\c
.SY bsearch (
.BI int\~ one ,
.BI int\~ two ,
.BI int\~ three ,
.BI int\~ four ,
.BI int\~ five ,
.BI int\~ six ,
.BI int\~ seven ,
.BI int\~ eight ,
.BI int\~ nine ,
.BI int\~ ten ,
.BI int\~ eleven );
.YS
.'

# Expected output:
#
# foo(3)                 Library Functions Manual                 foo(3)
#
# Name
#      foo - frobnicate a bar
#
# Synopsis
#      void *bsearch(int one, int two, int three, int four, int five,
#                    int six, int seven, int eight, int nine, int ten,
#                    int eleven);
#
# groff test suite              2025-07-24                        foo(3)

output=$(printf "%s\n" "$input" \
    | "$groff" -rLL=70n -man -Tascii -P-cbou)
echo "$output"
echo "$output" \
    | grep -Fq \
      'void *bsearch(int one, int two, int three, int four, int five,'
exit

# vim:set ai et sw=4 ts=4 tw=72:
