#!/bin/sh
#
# Copyright 2021-2024 G. Branden Robinson
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

input=$(cat <<EOF
.TH foo 1 2021-11-05 "groff test suite"
.TP 9n
.UR https://\:github.com/\:Alhadis/\:Roff\:.js/
.I Roff.js
.UE
is a viewer for intermediate output written in Java\%Script.
EOF
)

fail=

wail () {
    echo "...FAILED" >&2
    fail=YES
}

# Check for regressions when OSC 8 disabled
uflag=-rU0

output=$(printf "%s" "$input" \
    | "$groff" -bww -rLL=67n $uflag -man -Tascii -P-cbou)
echo "$output"

# Expected output:
# foo(1)                General Commands Manual                foo(1)
#
#      Roff.js  <https://github.com/Alhadis/Roff.js/> is a viewer for
#               intermediate output written in JavaScript.
#
# groff test suite             2021-11-05                      foo(1)

echo "checking for tag on same line as body ($uflag)" >&2
echo "$output" | grep -Eq '^     Roff\.js  +.*is a' || wail # 5 spaces

echo "checking for presence of typeset URI ($uflag)" >&2
echo "$output" | grep -Fq '<https://github.com/Alhadis/Roff.js/>' \
    || wail

# Sloppy handling of UE, ME macro arguments can cause unwanted space.
echo "checking for normative (no extra) spacing after URI ($uflag)" >&2
echo "$output" | grep -q '> is a viewer for' || wail

# Now check for good formatting when URIs are hyperlinked.
# Unfortunately we have to abandon `-cbou` or groff will (correctly)
# detect that hyperlinks are impossible, and render them as text.  That
# makes pattern matching harder because any old crazy,
# terminal-dependent escape sequences could be present.
uflag=-rU1

output=$(printf "%s" "$input" \
    | "$groff" -bww -rLL=66n $uflag -man -Tascii -P-cbou)
echo "$output"

# Expected output:
# foo(1)                General Commands Manual               foo(1)
#
#      Roff.js  is a viewer for intermediate output written in Java-
#               Script.
#
# groff test suite            2021-11-05                      foo(1)

echo "checking for tag on same line as body ($uflag)" >&2
echo "$output" | grep -Eq 'Roff\.js +is +a +viewer' || wail

text='is a viewer for intermediate output written in Java-'
echo "checking for normative (no extra) spacing after URI ($uflag)" >&2
echo "$output" | grep -Fq "$text" || wail

output=$(printf "%s" "$input" | "$groff" -bww -Tascii $uflag -man -Z \
    | grep '^t')
echo "$output"

echo "checking for absence of typeset URI" >&2
! echo "$output" | grep https || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
