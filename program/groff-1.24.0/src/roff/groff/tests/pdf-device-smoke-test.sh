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

# Expected output:
#0000460   H   e   l   l   o   ,  nl   q  sp   B   T  nl   1  sp   0  sp
#0001020   H   1   0   6   3   4   0  nl   %  sp   t   o   r   l   d   !

# '-P -d' because we want the PDF uncompressed for pattern matching.
output=$(echo "Hello, world!" | "$groff" -T pdf -P -d | od -t a)
echo "$output" | grep '^0000460'
echo "$output" | grep '^0001020'
echo "$output" | grep -Eq '^0000460.*H +e +l +l +o +,' || exit 1
echo "$output" | grep -Eq '^0001020.*o +r +l +d +!'    || exit 1

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
