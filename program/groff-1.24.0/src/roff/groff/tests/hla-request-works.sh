#!/bin/sh
#
# Copyright 2023 G. Branden Robinson
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

fail=

wail () {
  echo ...FAILED >&2
  fail=yes
}

input='.ec @
.ll 30n
.tm before=@n[.hla]
.de Madison
Religious bondage shackles and debilitates the mind and unfits it for
every noble enterprise.
.br
..
.Madison
.hla
.tm after=@n[.hla]
.Madison'

error=$(printf "%s\n" "$input" | "$groff" -T ascii 2>&1 > /dev/null)
echo "$error"

output=$(printf "%s\n" "$input" | "$groff" -T ascii 2> /dev/null | nl)
echo "$output"

echo "checking initial language reported by .hla register" >&2
echo "$error" | grep -Fqx 'before=en' || wail

echo "checking that words get automatically hyphenated" >&2
echo "$output" | grep -Eqx ' +2.*debilitates +the +mind +and +un-' \
	|| wail

echo "checking final language reported by .hla register" >&2
echo "$error" | grep -Fqx 'after=' || wail

echo "checking that words don't get automatically hyphenated" >&2
echo "$output" | grep -Eqx ' +6.*debilitates +the +mind +and' \
	|| wail

test -z "$fail"
