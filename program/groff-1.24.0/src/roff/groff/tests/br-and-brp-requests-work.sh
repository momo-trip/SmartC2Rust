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

input='.
.ll 14n
.c2 @
alpha
.br
bravo
@br
charlie
.brp
delta
@brp
echo
.'

output=$(printf '%s\n' "$input" | "$groff" -T ascii -P -cbou)
echo "$output"

# Expected:
#
# alpha
# bravo  charlie
# delta echo

echo "checking that 'br' with normal control character works" >&2
echo "$output" | grep -qx alpha || wail

echo "checking that 'br' with no-break control character works" >&2
echo "$output" | grep -qx 'bravo.*charlie' || wail

echo "checking that 'brp' with normal control character works" >&2
echo "$output" | grep -q '  charlie$' || wail # 2 spaces

echo "checking that 'brp' with no-break control character works" >&2
echo "$output" | grep -qx 'delta.*echo' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
