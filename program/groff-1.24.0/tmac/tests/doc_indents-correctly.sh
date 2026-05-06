#!/bin/sh
#
# Copyright 2022-2025 G. Branden Robinson
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

# Regression-test Debian #1022179.
#
# Ensure that subsection headings are indented correctly even if they
# break across output lines.

input='.Dd 2022-10-28
.Dt foo 1
.Os
.Sh "A long section heading that wraps to illustrate the fact that the\
 indentation of said title is consistent even if it breaks across lines"
Discussion should be indented as ordinary paragraph.
.Ss "A long subsection heading that wraps to illustrate the fact that\
 the indentation of said title is consistent even if it breaks across\
 lines"
Further discussion should be indented as ordinary paragraph.'

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc)
echo "$output"

# Verify that default `Sh` indentation is zero.
echo "checking default 'Sh' indentation on 1st line" >&2
echo "$output" | grep -Eq '^A +long +section +heading' || wail

echo "cehcking default 'Sh' indentation on 2nd line" >&2
echo "$output" | grep -Eq '^said title is consistent' || wail

# Verify that paragraph indentation after section heading is correct.
echo "checking 'Pp' indentation after 'Sh'" >&2
# 5 spaces in string literal.
echo "$output" | grep -Eq '^     Discussion should be indented' || wail

# Verify that default `Ss` indentation is three ens.
echo "checking default 'Ss' indentation on 1st line" >&2
# 3 spaces in string literal.
echo "$output" | grep -Eq '^   A +long +subsection +heading' || wail

echo "checking default 'Ss' indentation on 2nd line" >&2
# 3 spaces in string literal.
echo "$output" | grep -Eq '^   tion of said title is' || wail

# Verify that paragraph indentation after subsection heading is correct.
echo "checking 'Pp' indentation after 'Ss'" >&2
# 5 spaces in string literal.
echo "$output" | grep -Eq '^     Further discussion should be' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
