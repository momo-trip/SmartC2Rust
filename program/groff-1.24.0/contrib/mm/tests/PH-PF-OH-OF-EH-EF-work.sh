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
    echo ...FAILED >&2
    fail=YES
}

# Unit-test header/footer macros.

input='.
.PH @Alpha@Bravo@Charlie@
.PF @Delta@Echo@Foxtrot@
.OH @Golf@Hotel@Indigo@
.OF @Juliett@Kilo@Lima@
.EH @Mike@November@Oscar@
.EF @Papa@Romeo@Quebec@
.P
This is my
.I mm
document.
.bp
.P
It had a lot of blank space on the first page.
.'

output=$(echo "$input" | "$groff" -m m -T ascii -P -cbou)
echo "$output"

echo "checking presence of primary header" >&2
echo "$output" | grep -q 'Alpha  *Bravo  *Charlie' || wail

echo "checking presence of primary footer" >&2
echo "$output" | grep -q 'Delta  *Echo  *Foxtrot' || wail

echo "checking presence of odd-page secondary header" >&2
echo "$output" | grep -q 'Golf  *Hotel  *Indigo' || wail

echo "checking presence of odd-page secondary footer" >&2
echo "$output" | grep -q 'Juliett  *Kilo  *Lima' || wail

echo "checking presence of even-page secondary header" >&2
echo "$output" | grep -q 'Mike  *November  *Oscar' || wail

echo "checking presence of even-page secondary footer" >&2
echo "$output" | grep -q 'Papa  *Romeo  *Quebec' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
