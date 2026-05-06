#!/bin/sh
#
# Copyright 2024-2025 G. Branden Robinson
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

groff="${abs_top_builddir:-.}/test-groff"

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Locate directory containing our examples.
examples_dir=

for buildroot in . .. ../..
do
    e=$buildroot/contrib/mm/examples
    if [ -d "$e" ]
    then
        examples_dir=$e
        break
    fi
done

# If we can't find it, we can't test.
if [ -z "$examples_dir" ]
then
    echo "$0: cannot locate examples directory; skipping" >&2
    exit 77 # skip
fi

# Locate directory containing our test artifacts.
artifacts_dir=

for buildroot in . .. ../..
do
    a=$buildroot/contrib/mm/tests/artifacts
    if [ -d "$a" ]
    then
        artifacts_dir=$a
        break
    fi
done

# If we can't find it, we can't test.
if [ -z "$artifacts_dir" ]
then
    echo "$0: cannot locate test artifact directory; skipping" >&2
    exit 77 # skip
fi

# Regression-test Savannah #65865 and other things that could go wrong.

input="$examples_dir"/memorandum.mm

for t in 0 1 2 3 4 5 custom
do
    echo "checking formatting of MT type '$t'" >&2
    output=$("$groff" -ww -m m -d mT=$t -T ascii -P -cbou "$input" \
        | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
    echo "$output"
    # We _would_ use "cut -d' ' -f1-2" here, but Solaris 10 cksum writes
    # tabs between fields instead of spaces, nonconformantly with POSIX
    # Issue 4 (1994); see XCU p. 195, PDF p. 217.  Quality!  So fire up
    # big old AWK instead.  We're sure to be running on "enterprise"
    # hardware with that fancy proprietary OS.
    expected=$(cksum "$artifacts_dir"/memorandum.$t \
        | awk '{ print $1, $2 }')
    actual=$(echo "$output" | cksum | awk '{ print $1, $2 }')
    test "$actual" = "$expected" || wail
done

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
