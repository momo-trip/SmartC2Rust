#!/bin/sh
#
# Copyright 2021 G. Branden Robinson
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

input='\*[locale]'

echo "testing default localization (English)" >&2
output=$(echo "$input" | "$groff" -bww -Tascii)
echo "$output" | grep -qx english || wail

echo "testing Czech localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m cs)
echo "$output" | grep -qx czech || wail

echo "testing German localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m de)
echo "$output" | grep -qx german || wail

echo "testing English localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m en)
echo "$output" | grep -qx english || wail

echo "testing Spanish localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m es)
echo "$output" | grep -qx spanish || wail

echo "testing French localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m fr)
echo "$output" | grep -qx french || wail

echo "testing Italian localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m it)
echo "$output" | grep -qx italian || wail

echo "testing Japanese localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m ja)
echo "$output" | grep -qx japanese || wail

echo "testing Russian localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m ru)
echo "$output" | grep -qx russian || wail

echo "testing Swedish localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m sv)
echo "$output" | grep -qx swedish || wail

echo "testing Chinese localization" >&2
output=$(echo "$input" | "$groff" -bww -Tascii -m zh)
echo "$output" | grep -qx chinese || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
