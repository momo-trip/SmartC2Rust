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
#

groff="${abs_top_builddir:-.}/test-groff"

# Regression-test Savannah #61386.
#
# Excessively long "extra2" arguments to 'TH' (we recommend using this
# for project name and version information) can overrun other parts of
# the titles, such as a date in the center footer.

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

input='.TH foo 1 2021-10-26 "groff 1.23.0.rc1.1449-84949"
.SH Name
foo \- a command with a very short name'

echo 'testing long inner footer with sufficient space to set it' >&2
output=$(echo "$input" | "$groff" -Tascii -P-cbou -man)
echo "$output"
pattern='groff 1\.23\.0\.rc1\.1449-84949 +2021-10-26 +foo\(1\)'
echo "$output" | grep -Eq "$pattern" || wail

echo 'testing long inner footer with insufficient space to set it' >&2
output=$(echo "$input" | "$groff" -Tascii -P-cbou -man -rLL=60n)
echo "$output"
pattern='groff 1\.23\.0\.rc1\.1449\.\.\. +2021-10-26 +foo\(1\)'
echo "$output" | grep -Eq "$pattern" || wail

# Regression-test Savannah #61408.
#
# Don't spew diagnostics if the page doesn't supply a 3rd .TH argument.
echo 'testing for graceful behavior when TH has no 3rd argument' >&2
input='.TH patch 1 "" GNU'
error=$(echo "$input" | "$groff" -Tascii -P-cbou -man -ww -z 2>&1)
echo "$error"
test -z "$error" || wail

# Regression-test Savannah #65469.
#
# Don't spew diagnostics if the argument being abbreviated contains '@'.
echo 'testing for graceful treatment of argument containing "@"' >&2
input='.TH tic 1M 2023-12-30 "ncurses @NCURSES_MAJOR@.@NCURSES_MINOR@"'
error=$(echo "$input" | "$groff" -Tascii -P-cbou -man -ww -z 2>&1)
echo "$error"
test -z "$error" || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
