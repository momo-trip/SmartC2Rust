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
#

groff="${abs_top_builddir:-.}/test-groff"

# Keep preconv from being run.
unset GROFF_ENCODING

fail=

wail () {
    echo ...FAILED >&2
    fail=yes
}

input='.TH foo 1 2021-10-06 "groff test suite"
.SH Name
foo \\- a command with a very short name
.SH Description
The real work is done by
.MR bar 1 .'

output=$(echo "$input" | "$groff" -Tascii -rU1 -man -Z | nl)
echo "$output"

# Expected:
#   91  x X tty: link man:bar(1)
#   92  f2
#   93  tbar
#   94  f1
#   95  t(1)
#   96  V280
#   97  H912
#   98  x X tty: link

echo "checking for opening 'link' device extension command" >&2
echo "$output" | grep -Eq '91[[:space:]]+x X tty: link man:bar\(1\)$' \
    || wail

echo "checking for correct man page title font style" >&2
echo "$output" | grep -Eq '92[[:space:]]+f2' \
    || wail
echo "$output" | grep -Eq '93[[:space:]]+tbar' \
    || wail

echo "checking for correct man page section font style" >&2
echo "$output" | grep -Eq '94[[:space:]]+f1' \
    || wail
echo "$output" | grep -Eq '95[[:space:]]+t\(1\)' \
    || wail

echo "checking for closing 'link' device extension command" >&2
echo "$output" | grep -Eq '98[[:space:]]+x X tty: link$' \
    || wail

output=$(echo "$input" | "$groff" -man -Thtml)
echo "$output"

echo "checking for correctly formatted man URI in HTML output" >&2
echo "$output" | grep -Fq '<a href="man:bar(1)"><i>bar</i>(1)</a>.' \
    || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
