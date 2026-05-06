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

grotty="${abs_top_builddir:-.}/grotty"

fail=

wail () {
    echo "...FAILED" >&2
    fail=yes
}

input='#
x T ascii
x res 240 24 40
x init
p 1
x font 1 R
f 1
s 10
V 40
H 0
m d
D F d
t 1234567890123456
n 40 0
V 80
H 0
t abcdef
w
h 48
t ijklmnop
n 40 0
V 120
H 0
t abcdefg
w
h 24
t ijklmnop
n 40 0
x trailer
V 120
x stop
#'

output=$(printf '%s\n' "$input" | "$grotty" -F font -F build/font -h)
echo "$output"

tab=$(printf '\t') # Some shells don't support $'\t' yet.

echo "checking that tab used to move > 1 cell to next tab stop" >&2
echo "$output" | grep -q "abcdef${tab}ijklmnop" || wail

echo "checking that space used to move 1 cell to next tab stop" >&2
echo "$output" | grep -q "abcdefg ijklmnop" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
