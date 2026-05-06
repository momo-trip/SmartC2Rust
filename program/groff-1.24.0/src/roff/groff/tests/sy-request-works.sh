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

fail=

wail () {
  echo ...FAILED >&2
  fail=YES
}

echo "checking that 'sy' request works" >&2
output=$(printf '.sy echo hello\n' | "$groff" -Uz)
echo $output
echo $output | grep -Fqx hello || wail

echo "checking that parser returns to correct state after 'sy'" >&2
output=$(printf '.sy echo hello\n.tm goodbye\n' \
  | "$groff" -Uz 2>&1 > /dev/null)
echo $output
echo $output | grep -Fqx goodbye || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
