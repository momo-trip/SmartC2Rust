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

# Test placement of footnote area and recovery of page bottom margin.

input='.
.PH @Alpha@Bravo@Charlie@
.PF @Delta@Echo@Foxtrot@
.OH @Golf@Hotel@Indigo@
.OF @Juliett@Kilo@Lima@
.EH @Mike@November@Oscar@
.EF @Papa@Romeo@Quebec@
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
.pp
Footnote mark here.\*F
.FS
Ne forte ex homine et veterino semine equorum
confieri credas Centauros posse neque esse,
aut rabidis canibus succinctas semimarinis
corporibus Scyllas et cetera de genere horum.
.FE
.\" Repeat the paragraph enough times that we can see if the bottom of
.\" the text area on a subsequent page returns to where it should be
.\" when footnotes are eabsent.
.pp
.pp
.pp
.pp
.'

# GNU coreutils's `nl` command adds trailing tabs to empty lines.
output=$(printf '%s\n' "$input" | "$groff" -m m -T ascii -P -cbou \
    | nl -ba | sed 's/[	 ]*$//') # That's [tab space].
echo "$output"

echo "checking that body text ends at correct line" \
    "when footnote absent (page 1)" >&2
echo "$output" | grep -q ' *59[	 ]*tempore,  *cum  *soluta' || wail

echo "checking that secondary footer is on correct line" \
    "when footnote absent (page 1)" >&2
echo "$output" | grep -q ' *62[	 ]*Juliett' || wail

echo "checking that primary footer is on correct line" \
    "when footnote absent (page 1)" >&2
echo "$output" | grep -q ' *63[	 ]*Delta' || wail

echo "checking that body text ends at correct line" \
    "when footnote present (page 2)" >&2
echo "$output" | grep -q ' *118[	 ]*incidunt,  *ut  *labore' || wail

echo "checking that footnote area separator is on correct line" \
    "when footnote present (page 2)" >&2
echo "$output" | grep -q ' *120[	 ]*---' || wail

echo "checking that footnote starts on correct line" \
    "(page 2)" >&2
echo "$output" | grep -q ' *122[	 ]*1\. Ne  *forte  *ex' || wail

echo "checking that footnote ends on correct line" \
    "(page 2)" >&2
echo "$output" | grep -q ' *125[	 ]*genere  *horum\.' || wail

echo "checking that secondary footer is on correct line" \
    "when footnote present (page 2)" >&2
echo "$output" | grep -q ' *128[	 ]*Papa' || wail

echo "checking that primary footer is on correct line" \
    "when footnote present (page 2)" >&2
echo "$output" | grep -q ' *129[	 ]*Delta' || wail

echo "checking that body text ends at correct line" \
    "when footnote absent (page 3)" >&2
echo "$output" | grep -q ' *191[	 ]*quasi  *architecto' || wail

echo "checking that secondary footer is on correct line" \
    "when footnote absent (page 3)" >&2
echo "$output" | grep -q ' *194[	 ]*Juliett' || wail

echo "checking that primary footer is on correct line" \
    "when footnote absent (page 3)" >&2
echo "$output" | grep -q ' *195[	 ]*Delta' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
