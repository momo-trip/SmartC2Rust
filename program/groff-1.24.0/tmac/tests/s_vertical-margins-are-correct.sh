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

wail () {
    echo "...FAILED" >&2
    fail=YES
}

# Unit-test the vertical page margins (before and after titles).

input='.
.ds FR 1\"
.P1
.if \nT .ds CH Header
.if \nT .ds CF Footer
.de pp
.LP
Sed ut perspiciatis, unde omnis iste natus error sit voluptatem
accusantium doloremque laudantium, totam rem aperiam eaque ipsa, quae ab
illo inventore veritatis et quasi architecto beatae vitae dicta sunt,
explicabo.  Nemo enim ipsam voluptatem, quia voluptas sit, aspernatur
aut odit aut fugit, sed quia consequuntur magni dolores eos, qui ratione
voluptatem sequi nesciunt, neque porro quisquam est, qui dolorem ipsum,
quia dolor sit amet consectetur adipiscivelit, sed quia non-numquam eius
modi tempora incidunt, ut labore et dolore magnam aliquam quaerat
voluptatem.  Ut enim ad minima veniam, quis nostrum exercitationem ullam
corporis suscipitlaboriosam, nisi ut aliquid ex ea commodi consequatur?
Quis autem vel eum iure reprehenderit, qui inea voluptate velit esse,
quam nihil molestiae consequatur, vel illum, qui dolorem eum fugiat, quo
voluptas nulla pariatur?  At vero eos et accusamus et iusto odio
dignissimos ducimus, qui blanditiis praesentium voluptatum deleniti
atque corrupti, quos dolores et quas molestias excepturi sint, obcaecati
cupiditate non-provident, similique sunt in culpa, qui officia deserunt
mollitia animi, id est laborum et dolorum fuga.  Et harum quidem rerum
facilis est et expedita distinctio.  Nam libero tempore, cum soluta
nobis est eligendi optio, cumque nihil impedit, quo minus id, quod
maxime placeat, facere possimus, omnis voluptas assumenda est, omnis
dolor repellendus.  Temporibus autem quibusdam et aut officiis debitis
aut rerum necessitatibus saepe eveniet, ut et voluptates repudiandae
sint et molestiae non-recusandae.
..
.pp
.if \nF \{\
See below.\**
.  FS
.  if \nF=1 This is a short footnote.
.  if \nF=2 \{\
This is a longer footnote.
Ne forte ex homine et veterino semine equorum
confieri credas Centauros posse neque esse,
aut rabidis canibus succinctas semimarinis
corporibus Scyllas et cetera de genere horum.
.  \}
.  FE
.\}
.pp
.'

# Check that body text stops and starts in the correct places even
# without titles.

# GNU coreutils's `nl` command adds trailing tabs to empty lines.
output=$(printf '%s\n' "$input" | "$groff" -Tascii -P-cbou -ms \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *7[	 ]*Sed  *ut  *perspiciatis' || wail

echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *57[	 ]*sint et molestiae' || wail

# Now check _with_ titles...

output=$(printf '%s\n' "$input" | "$groff" -Tascii -P-cbou -ms -rT1 \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking for header on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -qx ' *4[	 ]*Header' || wail

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *7[	 ]*Sed  *ut  *perspiciatis' || wail

echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *57[	 ]*sint et molestiae' || wail

echo "checking for footer on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -qx ' *63[	 ]*Footer' || wail

# ...with titles and a short footnote...

output=$(printf '%s\n' "$input" \
    | "$groff" -Tascii -P-cbou -ms -rT1 -rF1 \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking for header on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -qx ' *4[	 ]*Header' || wail

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *7[	 ]*Sed  *ut  *perspiciatis' || wail

echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *57[	 ]*sint et molestiae' || wail

echo "checking for footnote separator rule on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q ' *59[	 ]*---' || wail

echo "checking for footnote text on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q ' *60[	 ]*.*short footnote' || wail

echo "checking for footer on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -qx ' *63[	 ]*Footer' || wail

# ...and with titles and a longer footnote.

output=$(printf '%s\n' "$input" \
    | "$groff" -Tascii -P-cbou -ms -rT1 -rF2 \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking for header on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -qx ' *4[	 ]*Header' || wail

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *7[	 ]*Sed  *ut  *perspiciatis' || wail

# The long footnote causes the footnote area to retreat up the page,
# spilling body text that formerly fit on one page to a second.
echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *74[	 ]*sint et molestiae' || wail

echo "checking for footnote separator rule on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q ' *56[	 ]*---' || wail

echo "checking for footnote text on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q ' *57[	 ]*.*longer footnote' || wail

echo "checking for footer on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -qx ' *63[	 ]*Footer' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
