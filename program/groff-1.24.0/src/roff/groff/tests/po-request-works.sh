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
.nf
foo
.po
bar
.po 5n
baz
.po
qux
.po 0
.po
jeq
.'

output=$(printf "%s\n" "$input" | "$groff" -T ascii)
echo "$output"

echo "checking that initial page offset is zero in nroff mode"
echo "$output" | grep -Fqx 'foo' || wail

echo "checking that initial previous page offset is zero in nroff mode"
echo "$output" | grep -Fqx 'bar' || wail

echo "checking operation of page offset request"
echo "$output" | grep -Fqx '     baz' || wail # 5 spaces

echo "checking page offset without argument restores previous setting"
echo "$output" | grep -Fqx 'qux' || wail

echo "checking that an explicit page offset argument the same as the" \
    "stored previous value clobbers the previous value"
echo "$output" | grep -Fqx 'jeq' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
