#!/bin/sh
#
# Copyright 2020-2025 G. Branden Robinson
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

# Keep this list of programs in sync with GROFF_CHECK_GROHTML_PROGRAMS
# in m4/groff.m4.
for cmd in pnmcrop pamcut pnmtopng pnmtops ps2ps
do
    if ! command -v $cmd >/dev/null
    then
        echo "$0: cannot locate '$cmd' command; skipping" >&2
        exit 77 # skip
    fi
done

# Commit c71b4ef4aa provoked an infinite loop in post-grohtml with these
# preprocessors.

input='.
.pp
Hello, world!
.EQ
gsize 12
delim $$
.EN
.pp
.pp
The faster clocks are $ PN $
.'

output=$(printf "%s\n" | "$groff" -b -ww -Thtml -eg -me)
echo "$output"
echo "$output" | grep -q 'faster clocks'

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
