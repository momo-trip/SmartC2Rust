#!/bin/sh
#
# Copyright 2019-2024 G. Branden Robinson
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

input='.
.TH sample 1 2020-10-31 "groff test suite"
.SH Name
sample \- test subject for groff
.'

output=$(printf "%s" "$input" | "$groff" -Tascii -P -cbou -man)
echo "$output"

echo "testing package default 'CT' register setting" >&2
echo "$output" | grep -q 'sample(1).*sample(1)' || fail

output=$(printf "%s" "$input" | "$groff" -Tascii -P -cbou -rCT=0 -man)
echo "$output"

echo "testing '-rCT=0' argument " >&2
echo "$output" | grep -q 'sample(1).*sample(1)' || fail

output=$(printf "%s" "$input" | "$groff" -Tascii -P -cbou -rCT=1 -man)
echo "$output"

echo "testing '-rCT=1' argument " >&2
echo "$output" | grep -q 'SAMPLE(1).*SAMPLE(1)' || fail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
