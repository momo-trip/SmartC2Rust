#!/bin/sh
#
# Copyright 2021-2024 G. Branden Robinson
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

# Regression-test Savannah #60025.
#
# Ensure .Mt renders correctly.

input='.Dd 2021-02-10
.Dt foo 1
.Os groff test suite
.Sh Name
.Nm foo
.Nd frobnicate a bar
.Sh Authors
.An -nosplit
The
.Nm mandoc
utility was written by
.An Kristaps Dzonsons ,
reachable at
.Mt kristaps@bsd.lv ,
and maintained by
.An Ingo Schwarze Aq Mt schwarze@openbsd.org .
Certainly that scoundrel Bob Bogus
.Pf ( Mt bogus@example.com )
played no role.'

output=$(echo "$input" | "$groff" -rU0 -mdoc -Tascii -P-cbou)
echo "$output"

echo "checking that conventional Mt macro call works (-rU0)" >&2
echo "$output" \
    | grep -Eq '^ +kristaps@bsd\.lv,' || wail

echo "checking that inline Mt macro call works (-rU0)" >&2
echo "$output" \
    | grep -Eq 'Schwarze +<schwarze@openbsd\.org>' || wail

echo "checking that prefixed Mt macro call works (-rU0)" >&2
echo "$output" \
    | grep -Fq 'Bogus (bogus@example.com) played' || wail

output=$(echo "$input" | "$groff" -rU1 -mdoc -Tascii -P-cbou)
echo "$output"

echo "checking that conventional Mt macro call works (-rU1)" >&2
echo "$output" \
    | grep -Eq '^ +kristaps@bsd\.lv,' || wail

echo "checking that inline Mt macro call works (-rU1)" >&2
echo "$output" \
    | grep -Eq 'Schwarze +<schwarze@openbsd\.org>' || wail

echo "checking that prefixed Mt macro call works (-rU1)" >&2
echo "$output" \
    | grep -Fq 'Bogus (bogus@example.com) played' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
