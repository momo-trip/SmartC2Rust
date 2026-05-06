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
#0000060   0   V esc   *   p   9   0   0   x   2   0   0   Y   H   e   l
#0000100   l   o esc   *   p   -   9   X   , esc   *   p   +   4   9   X
#0000120   w esc   *   p   -   9   X   o   r   l   d   !  ff esc   E

output=$(echo "Hello, world!" | "$groff" -T lj4 -P -pletter | od -t a)
echo "$output" | grep '^0000060'
echo "$output" | grep '^00001[02]0'
echo "$output" | grep -Eq '^0000060.*H +e +l'            || exit 1
echo "$output" | grep -Eq '^0000100.*l +o.* +,'          || exit 1
echo "$output" | grep -Eq '^0000120.*w.* +o +r +l +d +!' || exit 1

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
