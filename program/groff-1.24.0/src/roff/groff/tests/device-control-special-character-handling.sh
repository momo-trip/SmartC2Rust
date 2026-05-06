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

input='.
.nf
\X#bogus1: esc \%to-do\[u1F63C]\\[u1F00]\-\[`a]#
.device bogus1: req \%to-do\[u1F63C]\\[u1F00]\-\[`a]
.ec @
@X#bogus2: esc @%to-do@[u1F63C]@@[u1F00]@-@[`a]#
.device bogus2: req @%to-do@[u1F63C]@@[u1F00]@-@[`a]
.'

output=$(printf '%s\n' "$input" | "$groff" -T ps -Z 2> /dev/null \
  | grep '^x X')
echo "$output"

# Expected:
#
# x X bogus1: esc to-do\[u1F63C]\[u1F00]-\[u00E0]
# x X bogus1: req @%to-do\[u1F63C]\[u1F00]@-\[`a]
# x X bogus2: esc to-do\[u1F63C]\[u1F00]-\[u00E0]
# x X bogus2: req @%to-do@[u1F63C]@[u1F00]@-@[`a]

echo "checking X escape sequence, default escape character" >&2
echo "$output" \
  | grep -Fqx 'x X bogus1: esc to-do\[u1F63C]\[u1F00]-\[u00E0]' || wail

#echo "checking device request, default escape character" >&2
#echo "$output" \
#  | grep -qx 'x X bogus1: req to-do\\\[u1F00] -'"'"'"`^\\~' \
#  || wail

echo "checking X escape sequence, alternate escape character" >&2
echo "$output" \
  | grep -Fqx 'x X bogus2: esc to-do\[u1F63C]\[u1F00]-\[u00E0]' || wail

#echo "checking device request, alternate escape character" >&2
#echo "$output" \
#  | grep -qx 'x X bogus2: req to-do\\\[u1F00] -'"'"'"`^\\~' \
#  || wail

input='.
.nf
\X#bogus3: \[dq]\[sh]\[Do]\[aq]\[sl]\[at]#
\X#bogus4: \[lB]\[rs]\[rB]\[ha]#
\X#bogus5: \[lC]\[ba]\[or]\[rC]\[ti]#
.\"device bogus3: \[dq]\[sh]\[Do]\[aq]\[sl]\[at]
.\"device bogus4: \[lB]\[rs]\[rB]\[ha]
.\"device bogus5: \[lC]\[ba]\[or]\[rC]\[ti]
.'

# Expected:
#
# x X bogus3: "#$'/@
# x X bogus4: [\]^
# x X bogus5: {||}~

output=$(printf '%s\n' "$input" | "$groff" -T ps -Z 2> /dev/null \
  | grep '^x X')
echo "$output"

echo "checking X escape sequence, conversions to basic Latin (1/3)" >&2
echo "$output" | grep -Fqx 'x X bogus3: "#$'"'"'/@' || wail

echo "checking X escape sequence, conversions to basic Latin (2/3)" >&2
echo "$output" | grep -Fqx 'x X bogus4: [\]^' || wail

echo "checking X escape sequence, conversions to basic Latin (3/3)" >&2
echo "$output" | grep -Fqx 'x X bogus5: {||}~' || wail

# A more practical case, suggested by Deri James.

input='.
.ds h Caf\['"'"'e] Hyphen-Minus and \[rs]\[u2010]
\X"ps:exec 1:\\X     [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark"
\!x X ps:exec 2:\!     [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark
.device ps:exec 3:device [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark
.output x X ps:exec 4:output [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark
.'

echo INPUT:
printf '%s\n' "$input" | nl

output=$(printf '%s\n' "$input" | "$groff" -T pdf -Z 2> /dev/null \
  | grep '^x X')
echo OUTPUT:
printf "%s\n" "$output"

# Expected:
#
# x X ps:exec 2:\!     [/Dest /pdf:bm1 /Title (Caf\['e] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark
# x X ps:exec 4:output [/Dest /pdf:bm1 /Title (Caf\['e] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark
# x X ps:exec 1:\X     [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \\[u2010]) /Level 1 /OUT pdfmark
# x X ps:exec 3:device [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \\[u2010]) /Level 1 /OUT pdfmark

echo "checking practical bookmarking with \X escape sequence" >&2
# 5 spaces in the pattern below
printf "%s\n" "$output" \
  | grep -Fqx 'x X ps:exec 1:\X     [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \\[u2010]) /Level 1 /OUT pdfmark' \
  || wail

echo "checking practical bookmarking with \! escape sequence" >&2
# 5 spaces in the pattern below
printf "%s\n" "$output" \
  | grep -Fqx 'x X ps:exec 2:\!     [/Dest /pdf:bm1 /Title (Caf\['"'"'e] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark' \
  || wail

echo "checking practical bookmarking with device request" >&2
printf "%s\n" "$output" \
  | grep -Fqx 'x X ps:exec 3:device [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \\[u2010]) /Level 1 /OUT pdfmark' \
  || wail

echo "checking practical bookmarking with output request" >&2
printf "%s\n" "$output" \
  | grep -Fqx 'x X ps:exec 4:output [/Dest /pdf:bm1 /Title (Caf\['"'"'e] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark' \
  || wail

# Test the same thing, but with a composite special character escape
# sequence.

input='.
.ds h Caf\[e aa] Hyphen-Minus and \[rs]\[u2010]
\X"ps:exec 5:\\X     [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark"
\!x X ps:exec 6:\!     [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark
.device ps:exec 7:device [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark
.output x X ps:exec 8:output [/Dest /pdf:bm1 /Title (\*[h]) /Level 1 /OUT pdfmark
.'

echo INPUT:
printf '%s\n' "$input" | nl

output=$(printf '%s\n' "$input" | "$groff" -T pdf -Z 2> /dev/null \
  | grep '^x X')
echo OUTPUT:
printf "%s\n" "$output"

# Expected:
#
# x X ps:exec 6:\! [/Dest /pdf:bm1 /Title (Caf\['e] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark
# x X ps:exec 8:output [/Dest /pdf:bm1 /Title (Caf\['e] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark
# x X ps:exec 5:\X [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \[u2010]) /Level 1 /OUT pdfmark
# x X ps:exec 7:device [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark

echo "checking practical bookmarking with \X escape sequence" >&2
printf "%s\n" "$output" \
  | grep -Fqx 'x X ps:exec 5:\X     [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \\[u2010]) /Level 1 /OUT pdfmark' \
  || wail

echo "checking practical bookmarking with \! escape sequence" >&2
printf "%s\n" "$output" \
  | grep -Fqx 'x X ps:exec 6:\!     [/Dest /pdf:bm1 /Title (Caf\[e aa] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark' \
  || wail

#echo "checking practical bookmarking with device request" >&2
#printf "%s\n" "$output" \
#  | grep -Fqx 'x X ps:exec 7:device [/Dest /pdf:bm1 /Title (Caf\[u00E9] Hyphen-Minus and \\[u2010]) /Level 1 /OUT pdfmark' \
#  || wail

echo "checking practical bookmarking with output request" >&2
printf "%s\n" "$output" \
  | grep -Fqx 'x X ps:exec 8:output [/Dest /pdf:bm1 /Title (Caf\[e aa] Hyphen-Minus and \[rs]\[u2010]) /Level 1 /OUT pdfmark' \
  || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
