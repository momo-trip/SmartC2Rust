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

pdfmom="${abs_top_builddir:-.}/pdfmom"
groff="${abs_top_builddir:-.}/test-groff"

fail=

wail () {
    echo ...FAILED >&2
    fail=yes
}

# Test cross-package internal hyperlinks.
#
# We test backward links first because that is the more robust
# operation; it doesn't require two-pass formatting.

# First, arrange a man(7) document before an mdoc(7) one.

input1='.
.TH foo 1 2025-10-21 "groff test suite"
.SH Name
foo \\- a command with a very short name
.SH Description
The real work is done by
.MR bar 1 .
.Dd 2025-10-21
.Dt bar 1
.Os "groff test suite"
.Sh Name
.Nm bar
.Nd a command that is difficult to use
.Sh Description
Novice users should avoid this command in favor of its wrapper,
.Xr foo 1 .
An even more esoteric interface is
.Xr baz 1 .
.'

output1=$(echo "$input1" \
  | GROFF_COMMAND="$groff" "$pdfmom" --roff -rU1 -mandoc -Z | nl -ba)
echo "$output1"

# Expected (lines truncated):
#  109	x X pdf: markstart 6830 -1770 2000 /Subtype /Link /Dest /bar(1)
#  244	x X pdf: markstart 6830 -1770 2000 /Subtype /Link /Dest /foo(1)
#  278	x X pdf: markstart 6830 -140 2000 /Subtype /Link /Action << /Subtype /URI /URI (man:baz(1))

echo "checking that backward internal link from mdoc(7) document" \
  "to man(7) document works" >&2
echo "$output1" \
  | grep -Eq '^ *244[[:space:]]+x X pdf: markstart .*/Dest /foo\(1\)' \
  || wail

echo "checking that forward internal link from man(7) document" \
  "to mdoc(7) document works" >&2
echo "$output1" \
  | grep -Eq '^ *109[[:space:]]+x X pdf: markstart .*/Dest /bar\(1\)' \
  || wail

echo "checking that external links are not rewritten" >&2
echo "$output1" \
  | grep -Eq '^ *278[[:space:]]+x X pdf: markstart .*/URI \(man:baz\(1\)\)' \
  || wail

# Second, arrange an mdoc(7) document before a man(7) one.

input2='.
.Dd 2025-10-21
.Dt foo 1
.Os "groff test suite"
.Sh Name
.Nm foo
.Nd a command with a very short name
.Sh Description
The real work is done by
.Xr bar 1 .
.TH bar 1 2025-10-21 "groff test suite"
.SH Name
bar \\- a command that is difficult to use
.SH Description
Novice users should avoid this command in favor of its wrapper,
.MR foo 1 .
An even more esoteric interface is
.MR baz 1 .
.'

output2=$(echo "$input1" \
  | GROFF_COMMAND="$groff" "$pdfmom" --roff -rU1 -mandoc -Z | nl -ba)
echo "$output2"

# Expected (lines truncated):
#  109	x X pdf: markstart 6830 -1770 2000 /Subtype /Link /Dest /bar(1)
#  244	x X pdf: markstart 6830 -1770 2000 /Subtype /Link /Dest /foo(1)
#  278	x X pdf: markstart 6830 -140 2000 /Subtype /Link /Action << /Subtype /URI /URI (man:baz(1))

echo "checking that backward internal link from man(7) document" \
  "to mdoc(7) document works" >&2
echo "$output2" \
  | grep -Eq '^ *244[[:space:]]+x X pdf: markstart .*/Dest /foo\(1\)' \
  || wail

echo "checking that forward internal link from mdoc(7) document" \
  "to man(7) document works" >&2
echo "$output2" \
  | grep -Eq '^ *109[[:space:]]+x X pdf: markstart .*/Dest /bar\(1\)' \
  || wail

echo "checking that external links are not rewritten" >&2
echo "$output2" \
  | grep -Eq '^ *278[[:space:]]+x X pdf: markstart .*/URI \(man:baz\(1\)\)' \
  || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
