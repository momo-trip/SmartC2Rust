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

groff="${abs_top_builddir:-.}/test-groff"

fail=

wail() {
    echo ...FAILED >&2
    fail=yes
}

input='.
.Dd 2025-07-16
.Dt foo 1
.Os "groff test suite"
.ds AD l
.Sh Name
.Nm foo
.Nd a command with a very short name
.Sh Description
This paragraph should not be adjusted.
Sed ut perspiciatis, unde omnis iste natus error sit voluptatem
accusantium doloremque laudantium, totam rem aperiam eaque ipsa, quae ab
illo inventore veritatis et quasi architecto beatae vitae dicta sunt,
explicabo.
.Pp
This paragraph should also not be adjusted.
Ut enim ad minima veniam, quis nostrum exercitationem ullam
corporis suscipitlaboriosam suscipitlaboriosam suscipitlaboriosam,
nisi ut aliquid ex ea commodi consequatur?
.Dd 2025-07-16
.Dt bar 1
.Os "groff test suite"
.Sh Name
.Nm bar
.Nd another command with a very short name
.Sh Description
This paragraph should be adjusted.
Nemo enim ipsam voluptatem, quia voluptas sit, aspernatur
aut odit aut fugit, sed quia consequuntur magni dolores eos, qui ratione
voluptatem sequi nesciunt, neque porro quisquam est, qui dolorem ipsum,
quia dolor sit amet consectetur adipiscivelit, sed quia non-numquam eius
modi tempora incidunt, ut labore et dolore magnam aliquam quaerat
voluptatem.
.ds AD b \" attempt to meddle with next document
.Dd 2025-07-27
.Dt baz 1
.Os "groff test suite"
.Sh Name
.Nm baz
.Nd you guessed it
.Sh Description
This paragraph should not be adjusted.
Quis autem vel eum iure reprehenderit, qui inea voluptate velit esse,
quam nihil molestiae consequatur, vel illum, qui dolorem eum fugiat, quo
voluptas nulla pariatur?
.'

# A line length of 70 makes adjustment obvious on the first line of
# paragraph text in each "Description".
output=$(printf "%s\n" "$input" \
    | "$groff" -rLL=70n -mdoc -T ascii -P -cbou)
echo "$output"

echo "checking that the first document's first paragraph is not" \
    "adjusted" >&2
echo "$output" | grep -q 'totam rem aperiam eaque ipsa' || wail

echo "checking that the first document's second paragraph is not" \
    "adjusted" >&2
echo "$output" | grep -q 'ullam corporis suscipitlabo-' || wail

echo "checking that the second document's paragraph is adjusted" >&2
# Be robust to adjustment parity value.
if ! echo "$output" | grep -Eq 'Nemo enim  +ipsam  +voluptatem,' \
    && ! echo "$output" | grep -Eq 'This  +paragraph  +should +be'
then
    wail
fi

output=$(printf "%s\n" "$input" \
    | "$groff" -dAD=l -rLL=78n -mdoc -T ascii -P -cbou)
echo "$output"

echo "checking that the third document's paragraph is not adjusted" \
    "when disabled by user" >&2
# Be robust to adjustment parity value.
if ! echo "$output" | grep -q 'vel illum,' \
    && ! echo "$output" | grep -q 'nulla pariatur\?'
then
    wail
fi

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
