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

fail=

wail () {
  echo ...FAILED >&2
  fail=YES
}

input='.
.lf 900
.ec @
.tm @n[.c]
.'

echo "checking 'lf' request behavior with one argument" >&2
output=$(printf "%s\n" "$input" | "$groff" 2>&1)
echo "$output"
echo "$output" | grep -Fqx '901' || wail

input='.
.lf 900 myfile
.ec @
.tm @n[.c] @n[.F]
.'

echo "checking 'lf' request behavior with two arguments" >&2
output=$(printf "%s\n" "$input" | "$groff" 2>&1)
echo "$output"
echo "$output" | grep -Fqx '901 myfile' || wail

input='.
.lf 900 my file
.ec @
.tm @n[.c] @n[.F]
.'

echo "checking 'lf' request behavior with spaceful second argument" >&2
output=$(printf "%s\n" "$input" | "$groff" 2>&1)
echo "$output"
echo "$output" | grep -Fqx '901 my file' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
