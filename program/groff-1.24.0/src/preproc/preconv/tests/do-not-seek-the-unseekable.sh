#!/bin/sh
#
# Copyright 2022 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# groff is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

set -e

preconv="${abs_top_builddir:-.}/preconv"

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Scrape debugging output to see if we're skipping unseekable streams.
# This is fragile, but we don't want to lock the language of diagnostic
# messages (especially debugging ones).  If this test fails, check the
# text of the command's debugging output for a mismatch before
# investigating deeper problems.

echo "testing seekability of file operand '-'" >&2
output=$(printf '' | "$preconv" -d - 2>&1)
echo "$output" | grep -q "stream is not seekable" || wail

test -z "$fail" || exit

# /dev/stdin might not exist in a chroot.  Or, if it's not (a symbolic
# link to) a character special device, the next test will not be valid,
# as when using GNU Make's `-j` option.
#
# Similarly, we must have a controlling terminal.
if ! [ -c /dev/stdin ]
then
    echo "$0: /dev/stdin is not a character device; skipping" >&2
    exit 77 # skip
fi

if [ "$(tty)" = "not a tty" ]
then
    echo "$0: there is no controlling terminal; skipping" >&2
    exit 77 # skip
fi

echo "testing seekability of standard input stream" >&2
output=$(printf '' | "$preconv" -d /dev/stdin 2>&1)
echo "$output" | grep -q "stream is not seekable" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
