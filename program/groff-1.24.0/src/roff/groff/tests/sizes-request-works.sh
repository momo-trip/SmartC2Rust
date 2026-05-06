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

input='.
.pev
.sizes 2-20 36
.pev
.'

echo "checking that 'sizes' request works" >&2
output=$(printf '%s\n' "$input" | "$groff" -T ps 2>&1)
echo "$output" | grep 'valid type size list'
echo "$output" | grep -Fq '2s-20s, 36s'

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
