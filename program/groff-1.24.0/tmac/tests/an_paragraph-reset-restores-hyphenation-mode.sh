#!/bin/sh
#
# Copyright 2025 G. Branden Robinson
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
    echo "...FAILED" >&2
    fail=YES
}

# Ensure that the document's configured hyphenation mode is restored
# upon a "paragraph reset".

input1='.
.TH foo 1 2025-08-10 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Description
aardvarks
abandoned
abasement
abatement
abattoirs
abdicated
.nh \" should prevent hyphenation of next word
abdicates
.P
abdominal
abductees
abducting
abduction
abductors
abhorrent
.hy \" should be a no-op
abhorring
.P
abilities
ablatives
ablutions
abnegated
abnegates
abolished
abolishes
.'

output1=$(printf "%s\n" "$input1" \
    | "$groff" -r LL=72n -m an -T ascii -P -cbou)
echo "$output1"

echo "checking that last word in first paragraph is not hyphenated" >&2
echo "$output1" | grep -q ' abdicated$' || wail

echo "checking that last word in second paragraph is hyphenated" >&2
echo "$output1" | grep -q ' abhor-$' || wail

echo "checking that last word in third paragraph is hyphenated" >&2
echo "$output1" | grep -q ' abol-$' || wail

input2='.
.TH bar 1 2025-08-10 "groff test suite"
.nr HY 0 \" page author hates automatic hyphenation
.SH Name
bar \- consume the output of foo(1)
.SH Description
abolition
abominate
aborigine
abortions
abounding
abrasions
.nh \" should be a no-op
abrasives
.P
abridging
abrogated
abrogates
abruptest
abscessed
abscesses
.hy \" should permit hyphenation of next word
abscissae
.P
abscissas
absconded
absentees
absenting
absolutes
absolving
absorbent
.'

output2=$(printf "%s\n" "$input2" \
    | "$groff" -r LL=72n -m an -T ascii -P -cbou)
echo "$output2"

echo "checking that last word in first paragraph is not hyphenated" \
    "(.nr HY 0)" >&2
echo "$output2" | grep -q ' abrasives$' || wail

echo "checking that last word in second paragraph is hyphenated" \
    "(.nr HY 0)" >&2
echo "$output2" | grep -q ' abscis-$' || wail

echo "checking that last word in third paragraph is not hyphenated" \
    "(.nr HY 0)" >&2
echo "$output2" | grep -q ' absorbent$' || wail

input3='.
.TH baz 1 2025-08-10 "groff test suite"
.nr HY 1 \" page author is virtuous aspiring typographer
.SH Name
baz \- reverse the transformation performed by bar(1)
.SH Description
absorbing
abstained
abstainer
abstinent
abstracts
absurdest
.nh \" should prevent hyphenation of next word
absurdity
.P
abundance
abusively
abutments
abysmally
academics
academies
.hy \" should be a no-op
accenting
.P
accepting
accessing
accession
accessory
accidents
acclaimed
acclimate
.'

output3=$(printf "%s\n" "$input3" \
    | "$groff" -r LL=72n -m an -T ascii -P -cbou)
echo "$output3"

echo "checking that last word in first paragraph is not hyphenated" \
    "(.nr HY 1)" >&2
echo "$output3" | grep -q ' absurdity$' || wail

echo "checking that last word in second paragraph is hyphenated" \
    "(.nr HY 1)" >&2
echo "$output3" | grep -q ' accent-$' || wail

echo "checking that last word in third paragraph is hyphenated" \
    "(.nr HY 1)" >&2
echo "$output3" | grep -q ' accli-$' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
