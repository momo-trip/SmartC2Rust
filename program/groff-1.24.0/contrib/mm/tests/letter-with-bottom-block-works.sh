#!/bin/sh
#
# Copyright 2025 G. Branden Robinson
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

# Regression-test Savannah #66439.
#
# The BS/BE bottom block feature should work with LT letters.

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

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

input='.
.BS
.tl "Contact"Tax ID"Bank Account/IBAN"
.tl "me@org"98765"DE12123121231212"
.BE
.ND "24 February 2025"
.WA "Epi G. Netic" "Head of Research"
123 Main Street
Anytown, ST  10101
.WE
.IA "Rufus T. Arbogast" "Autovectorization Guru"
456 Elsewhere Avenue
Nirvana, PA  20406
.IE
.LT
.P
We have a research leak!
The next person I catch embedding engineering samples of our Lightspeed
Overdrive 2048-core processors in cork coasters distributed at trade
shows is going to regret it.
.FC
.SG
.'

echo "checking formatting of LT letter with BS/BE bottom block" >&2
# GNU coreutils's `nl` command adds trailing tabs to empty lines.
output=$(printf "%s\n" "$input" | "$groff" -ww -m m -T ascii -P -cbou \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"
# We _would_ use "cut -d' ' -f1-2" here, but Solaris 10 cksum writes
# tabs between fields instead of spaces, nonconformantly with POSIX
# Issue 4 (1994); see XCU p. 195, PDF p. 217.  Quality!  So fire up big
# old AWK instead.  We're sure to be running on "enterprise" hardware
# with that fancy proprietary OS.
expected=$(cksum "$artifacts_dir"/letter_with_bottom_block \
    | awk '{ print $1, $2 }')
actual=$(echo "$output" | cksum | awk '{ print $1, $2 }')
test "$actual" = "$expected" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
