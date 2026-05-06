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
#

groff="${abs_top_builddir:-.}/test-groff"

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Ensure the S register takes effect, configuring alternative type sizes
# and vertical spacings.
#
# Except in the paragraph text, we must use extra escape characters to
# guard the register interpolations, because they appear in macro
# arguments.

input='.
.TH foo 1 2025-07-25 "groff test suite ps=\\n[.s]z, vs=\\n[.v]u"
.SH Section ps=\\n[.s]z, vs=\\n[.v]u
.SS Subsection ps=\\n[.s]z, vs=\\n[.v]u
.P
Paragraph ps=\n[.s]z, vs=\n[.v]u.
.'

# Expected output:
#
# foo(1) General Commands Manual foo(1)
# Section ps=10.95z, vs=12000u
#  Subsection ps=10z, vs=12000u
#  Paragraph ps=10z, vs=12000u.
# groff test suite ps=10z, vs=12000u 2025-07-25 1

output=$(printf "%s\n" "$input" | "$groff" -a -rS10 -man -Tps)
echo "$output"
echo "checking header and footer parameters when given -rS10" >&2
echo "$output" | grep -q 'suite ps=10z, vs=12000u' || wail
echo "checking section heading parameters when given -rS10" >&2
echo "$output" | grep -q 'Section ps=10.95z, vs=12000u' || wail
echo "checking subsection heading parameters when given -rS10" >&2
echo "$output" | grep -q 'Subsection ps=10z, vs=12000u' || wail
echo "checking paragraph parameters when given -rS10" >&2
echo "$output" | grep -q 'Paragraph ps=10z, vs=12000u' || wail

# Expected output:
#
# foo(1) General Commands Manual foo(1)
# Section ps=12z, vs=13600u
#  Subsection ps=10.95z, vs=13600u
#  Paragraph ps=10.95z, vs=13600u.
# groff test suite ps=10.95z, vs=13600u 2025-07-25 1

output=$(printf "%s\n" "$input" | "$groff" -a -rS11 -man -Tps)
echo "$output"
echo "checking header and footer parameters when given -rS11" >&2
echo "$output" | grep -q 'suite ps=10.95z, vs=13600u' || wail
echo "checking section heading parameters when given -rS11" >&2
echo "$output" | grep -q 'Section ps=12z, vs=13600u' || wail
echo "checking subsection heading parameters when given -rS11" >&2
echo "$output" | grep -q 'Subsection ps=10.95z, vs=13600u' || wail
echo "checking paragraph parameters when given -rS11" >&2
echo "$output" | grep -q 'Paragraph ps=10.95z, vs=13600u' || wail

# Expected output:
#
# foo(1) General Commands Manual foo(1)
# Section ps=14.4z, vs=14500u
#  Subsection ps=12z, vs=14500u
#  Paragraph ps=12z, vs=14500u.
# groff test suite ps=12z, vs=14500u 2025-07-25 1

output=$(printf "%s\n" "$input" | "$groff" -a -rS12 -man -Tps)
echo "$output"
echo "checking header and footer parameters when given -rS12" >&2
echo "$output" | grep -q 'suite ps=12z, vs=14500u' || wail
echo "checking section heading parameters when given -rS12" >&2
echo "$output" | grep -q 'Section ps=14.4z, vs=14500u' || wail
echo "checking subsection heading parameters when given -rS12" >&2
echo "$output" | grep -q 'Subsection ps=12z, vs=14500u' || wail
echo "checking paragraph parameters when given -rS12" >&2
echo "$output" | grep -q 'Paragraph ps=12z, vs=14500u' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
