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

# Unit test the Xr maco.

input='.Dd 2024-02-22
.Dt foo 1
.Os "groff test suite"
.Sh Name
.Nm foo
.Nd frobnicate a bar
.Sh Description
Degenerate macro call here.
.Xr
.Pp
Read the
.Xr groff 1
man page.
.Pp
Read the man page
.Xr groff 1 ,
reflect,
and find enlightenment.
.Pf ( Xr groff_font 5
is also worth a look.)
.Pp
The
.Xr groff ,
.Xr groff_diff ,
and
.Xr roff
pages in section 7 are also illuminating.
.Pp
You could also read about
.Xr groff Xr mdoc .
.Pp
Or
about
.Xr groff Pf - Xr man
and
.Xr groff Ns - Ns Xr ms .'

output=$(echo "$input" | "$groff" -rU0 -mdoc -Tascii -P-cbou \
    2>/dev/null)
error=$(echo "$input" | "$groff" -rU0 -mdoc -Tascii -P-cbou \
    2>&1 >/dev/null)
echo "$output"
echo "$error"

echo "checking for error diagnostic on argumentless Xr call (-rU0)" >&2
echo "$error" | grep -q '9' || wail

echo "checking two-argument Xr call with full cross reference (-rU0)" \
    >&2
echo "$output" \
    | grep -Fq 'Read the groff(1) man page.' || wail

echo "checking two-argument Xr call with partial reference and" \
    "trailing punctuation (-rU0)" >&2
echo "$output" \
    | grep -Eq 'Read +the +man +page +groff\(1\), +reflect,' || wail

echo "checking a series of partial references, some punctuated (-rU0)" \
    >&2
echo "$output" \
    | grep -Fq 'The groff, groff_diff, and roff pages' || wail

echo "checking that prefixed Xr macro call works (-rU0)" >&2
echo "$output" \
    | grep -Fq '(groff_font(5) is also worth a look.)' || wail

# We don't expect an inline Pf call to prevent a word break before it.
echo "checking that inline Pf after Xr macro call works (-rU0)" >&2
echo "$output" | grep -Fq 'read about groff mdoc' || wail

# But we do expect Ns to do so.
echo "checking that inline Ns after Xr macro call works (-rU0)" >&2
echo "$output" | grep -Fq 'Or about groff -man and groff-ms.' || wail

output=$(echo "$input" | "$groff" -rU1 -mdoc -Tascii -P-cbou \
    2>/dev/null)
error=$(echo "$input" | "$groff" -rU1 -mdoc -Tascii -P-cbou \
    2>&1 >/dev/null)
echo "$output"
echo "$error"

echo "checking for error diagnostic on argumentless Xr call (-rU1)" >&2
echo "$error" | grep -q '9' || wail

echo "checking two-argument Xr call with full cross reference (-rU1)" \
    >&2
echo "$output" \
    | grep -Fq 'Read the groff(1) man page.' || wail

echo "checking two-argument Xr call with partial reference and" \
    "trailing punctuation (-rU1)" >&2
echo "$output" \
    | grep -Eq 'Read +the +man +page +groff\(1\), +reflect,' || wail

echo "checking a series of partial references, some punctuated (-rU1)" \
    >&2
echo "$output" \
    | grep -Fq 'The groff, groff_diff, and roff pages' || wail

echo "checking that prefixed Xr macro call works (-rU1)" >&2
echo "$output" \
    | grep -Fq '(groff_font(5) is also worth a look.)' || wail

# We don't expect an inline Pf call to prevent a word break before it.
echo "checking that inline Pf after Xr macro call works (-rU1)" >&2
echo "$output" | grep -Fq 'read about groff mdoc' || wail

# But we do expect Ns to do so.
echo "checking that inline Ns after Xr macro call works (-rU1)" >&2
echo "$output" | grep -Fq 'Or about groff -man and groff-ms.' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
