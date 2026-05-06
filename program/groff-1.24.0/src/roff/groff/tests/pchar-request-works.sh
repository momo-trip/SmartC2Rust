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

# Verify that `pchar` doesn't crash the formatter.  We have no
# specification for its output to the standard error stream.

echo "checking that 'pchar' request works on ordinary character" >&2
output=$(printf '.pchar -\n' | "$groff" 2>&1)
test $? -eq 0 || wail
echo "$output"

echo "checking that 'pchar' request works on special character" >&2
output=$(printf '.pchar \\[-]\n' | "$groff" 2>&1)
test $? -eq 0 || wail
echo "$output"

echo "checking that 'pchar' request works on indexed character" >&2
output=$(printf '.pchar \\N"45"\n' | "$groff" 2>&1)
test $? -eq 0 || wail
echo "$output"

echo "checking that 'pchar' request works on character class" >&2
output=$(printf '.pchar \\C"[CJKprepunct]"\n' \
    | "$groff" -m ja -T utf8 2>&1)
test $? -eq 0 || wail
echo "$output"

# Regression-test a blunder made during the request's development.

echo "checking that 'pchar' request handles trailing space" >&2
output=$(printf '.pchar - \n' | "$groff" 2>&1)
echo "$output"
echo "$output" | grep 'error' && wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
