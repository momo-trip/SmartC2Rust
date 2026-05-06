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

wail () {
  echo "...FAILED" >&2
  fail=yes
}

input='.pl 1v
.ll 9n
abc def\p
.na
ghi jkl\p
.ad
.ad l
mno pqr\p
.na
stu vwx\p
.ad
yza bcd\p
.ad l
.ad b
efg hij\p
.na
klm nop\p
.ad c
qrs tuv\p
.na
wxy zab\p
.ad
cde fgh\p
.ad r
ijk lmn\p
.na
opq rst\p
.ad
uvw xyz\p
.ad b
.ad 100
ABC DEF\p'

output=$(echo "$input" | "$groff" -T ascii)
echo "$output"

# Expected output:
#
#   abc   def
#   ghi jkl
#   mno pqr
#   stu vwx
#   yza   bcd
#   efg   hij
#   klm nop
#    qrs tuv
#   wxy zab
#    cde fgh
#     ijk lmn
#   opq rst
#     uvw xyz
#   ABC   DEF

echo "verifying that adjustment is enabled by default" >&2
echo "$output" | grep -Fqx "abc   def" || wail # 3 spaces

echo "verifying that '.na' disables adjustment" >&2
echo "$output" | grep -Fqx "ghi jkl" || wail

echo "verifying that '.ad l' aligns left" >&2
echo "$output" | grep -Fqx "mno pqr" || wail

echo "verifying that '.na' is a no-op after '.ad l'" >&2
echo "$output" | grep -Fqx "stu vwx" || wail

echo "verifying that '.ad' enables adjustment after '.ad l'" >&2
echo "$output" | grep -Fqx "yza   bcd" || wail # 3 spaces

echo "verifying that '.ad b' enables adjustment after '.ad l'" >&2
echo "$output" | grep -Fqx "efg   hij" || wail # 3 spaces

echo "verifying that '.na' disables adjustment after '.ad b'" >&2
echo "$output" | grep -Fqx "klm nop" || wail

echo "verifying that '.ad c' center-aligns" >&2
echo "$output" | grep -Fqx " qrs tuv" || wail

echo "verifying that '.na' left-aligns after '.ad c'" >&2
echo "$output" | grep -Fqx "wxy zab" || wail

echo "verifying that '.ad' center-aligns after '.na' after '.ad c'" >&2
echo "$output" | grep -Fqx " cde fgh" || wail

echo "verifying that '.ad r' right-aligns" >&2
echo "$output" | grep -Fqx "  ijk lmn" || wail # 2 leading spaces

echo "verifying that '.na' left-aligns after '.ad r'" >&2
echo "$output" | grep -Fqx "opq rst" || wail

echo "verifying that '.ad' right-aligns after '.na' after '.ad r'" >&2
echo "$output" | grep -Fqx "  uvw xyz" || wail # 2 leading spaces

echo "verifying that out-of-range numeric mode is a no-op" >&2
echo "$output" | grep -Fqx "ABC   DEF" || wail # 3 spaces

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
