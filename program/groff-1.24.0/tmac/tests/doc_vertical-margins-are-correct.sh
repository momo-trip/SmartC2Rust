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
#

groff="${abs_top_builddir:-.}/test-groff"

if ! echo foobar | grep -Eqx foobar >/dev/null 2>&1
then
    echo "$0: grep command does not support -Eqx options; skipping" >&2
    exit 77 # skip
fi

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

input='.
.Dd 2024-11-08
.Dt foo 1
.Os "groff test suite"
.nf
1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52'

output=$(printf "%s"'\\c'"\n'fi\n" "$input" \
    | "$groff" -r cR=0 -m doc -T ascii -P -cbou | nl -ba)
echo "$output"

echo "checking page length" >&2
res=$(echo "$output" | wc -l)
# macOS `wc` prefixes the line count with spaces.  Get rid of them.
res=$(( res + 0 )) || exit 99
test "$res" = 66 || wail

echo "checking placement of page header" >&2
echo "$output" \
    | grep -Eqx ' +4[[:space:]]+foo\(1\) +[A-Za-z ]+ +foo\(1\)' || wail

echo "checking placement of first body text line" >&2
echo "$output" | grep -Eqx ' +8[[:space:]]+1' || wail

echo "checking placement of last body text line" >&2
echo "$output" | grep -Eqx ' +59[[:space:]]+52' || wail

echo "checking placement of page footer" >&2
echo "$output" \
    | grep -Eqx ' +63[[:space:]]+groff test suite +2024-11-08 +1' \
    || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
