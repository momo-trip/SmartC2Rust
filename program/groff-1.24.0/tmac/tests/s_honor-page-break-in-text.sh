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

# Regression-test Savannah #64005 redux.

input='.pl 18v
.LP
We schedule a page break at the next line break.
'"'"'bp
This lengthy rambling sentence
with some lorem ipsum voluptatem filler
should start and end on page 2 (\n%).
.bp
This sentence should start on page 3 (\n%).
.pl \n(nlu'

output=$(printf '%s\n' "$input" | "$groff" -Tascii -P-cbou -ms)
echo "$output"

echo "checking use of 'bp' with no-break control character" >&2
echo "$output" | grep -Fqx 'start and end on page 2 (2).' || wail

echo "checking use of 'bp' with normal control character" >&2
echo "$output" | grep -Fqx 'This sentence should start on page 3 (3).' \
    || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
