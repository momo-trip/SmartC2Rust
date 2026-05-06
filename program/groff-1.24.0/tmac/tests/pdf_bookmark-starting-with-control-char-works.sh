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

# Regression-test Savannah #65320.
#
# Ensure that a bookmark starting with a roff control character makes it
# into the PDF bookmark.
input='Intro.
.sp
.ft B
.pdfbookmark 1 Attack...
Attack\|.\|.\|.
.sp
.pdfbookmark 1 ...Decay...
\&.\|.\|.\|Decay\|.\|.\|.
.sp
.pdfbookmark 1 \&...Sustain...
\&.\|.\|.\|Sustain\|.\|.\|.
.sp
.pdfbookmark 1 Release
Release'

# Expected:
# x X ps:exec [/Dest /pdf:bm1 /Title (Attack...) /Level 1 /OUT pdfmark
# x X ps:exec [/Dest /pdf:bm2 /Title (...Decay...) /Level 1 /OUT pdfmark
# x X ps:exec [/Dest /pdf:bm3 /Title (...Sustain...) /Level 1 /OUT pdfmark
# x X ps:exec [/Dest /pdf:bm4 /Title (Release) /Level 1 /OUT pdfmark

output=$(printf "%s\n" "$input" | "$groff" -b -ww -Tpdf -Z \
    | grep '^x X')
echo "$output"

echo "checking bookmark with trailing ellipsis" >&2
echo "$output" \
    | grep -qx 'x X ps:exec .*pdf:bm1 /Title (Attack\.\.\.).*pdfmark' \
    || wail

echo "checking bookmark with leading and trailing ellipsis" >&2
echo "$output" \
    | grep -qx 'x X ps:exec .*pdf:bm2 /Title (\.\.\.Decay\.\.\.).*pdfmark' \
    || wail

echo "checking bookmark with leading and trailing ellipsis" \
    "(and dummy character workaround)" >&2
echo "$output" \
    | grep -qx 'x X ps:exec .*pdf:bm3 /Title (\.\.\.Sustain\.\.\.).*pdfmark' \
    || wail

echo "checking bookmark with no ellipsis" >&2
echo "$output" \
    | grep -qx 'x X ps:exec .*pdf:bm4 /Title (Release).*pdfmark' \
    || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
