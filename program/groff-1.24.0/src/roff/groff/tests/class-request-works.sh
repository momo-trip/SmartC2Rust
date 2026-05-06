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

fail=

wail () {
  echo ...FAILED >&2
  fail=YES
}

# Verify that `class` doesn't crash the formatter.

echo "checking creation of class containing ordinary character" >&2
output=$(printf '.class [ord] A\n' | "$groff" 2>&1)
test $? -eq 0 || wail
echo "$output"

echo "checking creation of class containing special character" >&2
output=$(printf '.class [spc] \\[-]\n' | "$groff" 2>&1)
test $? -eq 0 || wail
echo "$output"

echo "checking creation of class containing indexed character" >&2
output=$(printf '.class [ind] \\N"45"\n' | "$groff" 2>&1)
test $? -eq 0 || wail
echo "$output"

echo "checking creation of class containing code point range" >&2
output=$(printf '.class [range] A-Z\n' | "$groff" 2>&1)
test $? -eq 0 || wail
echo "$output"

# We don't test class nesting.  It's never worked.  See Savannah #67770.

# Regression-test a goof made during the request's development.

echo "checking that 'class' request handles trailing space" >&2
output=$(printf '.class [class1] A B \n' | "$groff" -w missing 2>&1)
echo "$output"
echo "$output" | grep 'warning' && wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
