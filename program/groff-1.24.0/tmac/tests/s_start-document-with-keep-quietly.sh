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
    echo "...FAILED" >&2
    fail=YES
}

# Starting a document with a keep (that is, before calling `RP`, `LP`,
# or some other initialization-capable macro) should not produce
# spurious diagnostics.  See Savannah #64529.

input1='.
.KS
.KE
.'

echo "checking document starting with empty static keep" >&2
error1=$(printf '%s\n' "$input1" | "$groff" -t -m s -z -T ascii 2>&1)
test -n "$error1" && echo "$error1"
echo "$error1" | grep -q . && wail

input2='.
.KS
foobar
.KE
.'

echo "checking document starting with simply populated static keep" >&2
error2=$(printf '%s\n' "$input2" | "$groff" -t -m s -z -T ascii 2>&1)
test -n "$error2" && echo "$error2"
echo "$error2" | grep -q . && wail

# Starting a document with a floating keep should also work fine.

input3='.
.KF
.KE
.'

echo "checking document starting with empty floating keep" >&2
error3=$(printf '%s\n' "$input3" | "$groff" -t -m s -z -T ascii 2>&1)
test -n "$error3" && echo "$error3"
echo "$error3" | grep -q . && wail

input4='.
.KF
bazqux
.KE
.'

echo "checking document starting with simply populated floating keep" \
    >&2
error4=$(printf '%s\n' "$input4" | "$groff" -t -m s -z -T ascii 2>&1)
test -n "$error4" && echo "$error4"
echo "$error4" | grep -q . && wail

# Starting with a table should also behave well.

input5='.
.KS
.TS
L.
table
.TE
.KE
.LP
Hello!
.'

echo "checking document starting with tbl-populated static keep" >&2
error5=$(printf '%s\n' "$input5" | "$groff" -t -m s -z -T ascii 2>&1)
test -n "$error5" && echo "$error5"
echo "$error5" | grep -q . && wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
