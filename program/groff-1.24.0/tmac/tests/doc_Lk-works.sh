#!/bin/sh
#
# Copyright 2024 G. Branden Robinson
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

wail() {
    echo ...FAILED >&2
    fail=yes
}

# Ensure .Lk renders correctly.

input='.Dd 2024-02-22
.Dt foo 1
.Os groff test suite
.Sh Name
.Nm foo
.Nd frobnicate a bar
.Sh Description
Sometimes you
.Em click Lk http://example.com one link
and you get
.Lk http://another.example.com .
.Pp
Follow instructions
.Pf ( Lk http://\:hidden\:.example\:.com elsewhere ) .'

output=$(echo "$input" | "$groff" -rU0 -mdoc -Tascii -P-cbou)
echo "$output"

# Expected:
#     Sometimes   you   click   one   link   <http://example.com>   and  you  get
#     http://another.example.com.
#
#     Follow instructions (elsewhere <http://hidden.example.com>).

echo "checking that conventional Lk macro call works (-rU0)" >&2
echo "$output" \
    | grep -Eq '^ +http://another\.example\.com' || wail

echo "checking that inline Lk macro call works (-rU0)" >&2
echo "$output" \
    | grep -Eq 'one +link +<http://example\.com>' || wail

echo "checking that prefixed Lk macro call works (-rU0)" >&2
echo "$output" \
    | grep -Fq 'instructions (elsewhere <http://hidden.example.com>).' \
    || wail

output=$(echo "$input" | "$groff" -rU1 -mdoc -Tascii -P-cbou)
echo "$output"

# Expected:
#     Sometimes you click one link and you get http://another.example.com.
#
#     Follow instructions (elsewhere).

echo "checking that conventional Lk macro call works (-rU1)" >&2
echo "$output" \
    | grep -q 'Sometimes you click one link and' || wail

echo "checking that inline Lk macro call works (-rU1)" >&2
echo "$output" \
    | grep -Fq 'you get http://another.example.com.' || wail

echo "checking that prefixed Lk macro call works (-rU1)" >&2
echo "$output" \
    | grep -Fq 'Follow instructions (elsewhere).' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
