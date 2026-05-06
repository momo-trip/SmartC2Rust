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
.PH @@@@ \" empty
.PF @@@@ \" empty
.if \nZ .PH @Alpha@Bravo@Charlie@
.if \nZ .PF @Delta@Echo@Foxtrot@
.if \nZ=2 \{\
.  OH @Golf@Hotel@Indigo@
.  OF @Juliett@Kilo@Lima@
.  EH @Mike@November@Oscar@
.  EF @Papa@Romeo@Quebec@
.\}
.de pp
.P
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
See below.\*F
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

echo "test case 1: -rF0 -rZ0" #>&2
# GNU coreutils's `nl` command adds trailing tabs to empty lines.
output=$(printf '%s\n' "$input" | "$groff" -m m -T ascii -P -cbou \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *8[	 ]*Sed  *ut  *perspiciatis' || wail

echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *59[	 ]*tempore,  *cum  *soluta' || wail

# Now check _with_ (primary) titles...

echo "test case 2: -rF0 -rZ1" #>&2
output=$(printf '%s\n' "$input" | "$groff" -rZ1 -m m -T ascii -P -cbou \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking for header on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *4[	 ]*Alpha' || wail

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *8[	 ]*Sed  *ut  *perspiciatis' || wail

echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *59[	 ]*tempore,  *cum  *soluta' || wail

echo "checking for footer on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *63[	 ]*Delta' || wail

# ...with (primary) titles and a short footnote...

echo "test case 3: -rF1 -rZ1" #>&2
output=$(printf '%s\n' "$input" | "$groff" -rZ1 -rF1 -m m -T ascii \
    -P -cbou | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking for header on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *4[	 ]*Alpha' || wail

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *8[	 ]*Sed  *ut  *perspiciatis' || wail

echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *55[	 ]*molestias  *excepturi' || wail

echo "checking for footnote separator rule on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *57[	 ]*---' || wail

echo "checking for footnote text on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *59[	 ]*.*This is a short footnote\.' \
    || wail

echo "checking for footer on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *63[	 ]*Delta' || wail

# ...with primary titles and a longer footnote...

echo "test case 4: -rF2 -rZ1" #>&2
output=$(printf '%s\n' "$input" | "$groff" -rZ1 -rF2 -m m -T ascii \
    -P -cbou | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking for header on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *4[	 ]*Alpha' || wail

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *8[	 ]*Sed  *ut  *perspiciatis' || wail

# The long footnote causes the footnote area to retreat up the page,
# spilling body text that formerly fit on one page to a second.
echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *52[	 ]*quo  *voluptas  *nulla' || wail

echo "checking for footnote separator rule on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *54[	 ]*---' || wail

echo "checking for footnote text on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *56[	 ]*.*longer  *footnote' || wail

echo "checking for footer on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *63[	 ]*Delta' || wail

# ...and with primary _and_ secondary titles and a longer footnote.

echo "test case 5: -rF2 -rZ2" #>&2
output=$(printf '%s\n' "$input" | "$groff" -rZ2 -rF2 -m m -T ascii \
    -P -cbou | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking for header on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *4[	 ]*Alpha' || wail

echo "checking that body text begins on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *8[	 ]*Sed  *ut  *perspiciatis' || wail

# The long footnote causes the footnote area to retreat up the page,
# spilling body text that formerly fit on one page to a second.
echo "checking that body text ends on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *52[	 ]*quo  *voluptas  *nulla' || wail

echo "checking for footnote separator rule on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *54[	 ]*---' || wail

echo "checking for footnote text on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *56[	 ]*.*longer  *footnote' || wail

echo "checking for footer on correct output line" >&2
# That's [tab space] on the next line.
echo "$output" | grep -q '^ *63[	 ]*Delta' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
