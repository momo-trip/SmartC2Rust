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
    echo "...FAILED" >&2
    fail=YES
}

input=$(cat <<EOF
.TH foo 1 2024-02-01 "groff test suite"
.TP
tag
ordinary tagged paragraph
.TP
.TP
ill-formed but still renderable
.TP
foobar
EOF
)

output=$(printf "%s" "$input" | "$groff" -ww -Tascii -P-cbou -man)
echo "$output"

echo "checking well-formed tagged paragraph" >&2
echo "$output" | grep -Eq 'tag +ordinary tagged paragraph' || wail

# We don't care about the indentation of the ill-formed input, just that
# the words are present.
echo "checking ill-formed tagged paragraph" >&2
echo "$output" | grep -q 'ill-formed but still renderable' || wail

echo "checking paragraph tag dangling at end of document (-rcR=1)" >&2
echo "$output" | grep -q 'foobar' || wail

output=$(printf "%s" "$input" \
    | "$groff" -ww -Tascii -P-cbou -rcR=0 -man)
echo "$output"

echo "checking paragraph tag dangling at end of document (-rcR=0)" >&2
echo "$output" | grep -q 'foobar' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
