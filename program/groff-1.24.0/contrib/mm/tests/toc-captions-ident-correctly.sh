#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
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

# Regression-test Savannah #67903.  Input adapted from a minimal
# reproducer by Alexis Hildebrant.

input='.
.ds Rule \l#\\n[.l]u-1n#
.ds Captc A\*[Rule]
.ds Le B\*[Rule]
.ds Lf C\*[Rule]
.ds Lt D\*[Rule]
.ds Lx E\*[Rule]
.
.ND 2026-01-11
.TL
It Beats a PowerPoint Deck
.AU "W.\& G.\& Snuffy"
.MT
.H 1 "Section heading"
.EC "My equation"
.EX "My exhibit"
.FG "My figure"
.TB "My table"
.TC
.'

output=$(printf '%s\n' "$input" | "$groff" -rO0 -mm -T ascii -P -cbou \
    | sed '/^$/d')
echo "$output"

echo "checking indentation of 'Captc' string" >&2
echo "$output" | grep -q '^A___' || wail

echo "checking indentation of 'Lf' string" >&2
echo "$output" | grep -q '^C___' || wail

echo "checking indentation of 'Lt' string" >&2
echo "$output" | grep -q '^D___' || wail

echo "checking indentation of 'Le' string" >&2
echo "$output" | grep -q '^B___' || wail

echo "checking indentation of 'Lx' string" >&2
echo "$output" | grep -q '^E___' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
