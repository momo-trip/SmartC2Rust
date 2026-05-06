#!/bin/sh
#
# Copyright 2021-2025 G. Branden Robinson
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

tbl="${abs_top_builddir:-.}/tbl"

# Regression-test Savannah #61417.
#
# Don't segfault because we tried to span down from an invalid span that
# tbl neglected to replace with an empty table entry.

if [ -e core ]
then
    echo "$0: 'core' file already exists; skipping" >&2
    exit 77 # skip
fi

input=$(cat <<EOF
.TS
l.
\^
\^
.TE
EOF
)
output=$(printf "%s" "$input" | "$tbl")
! test -f core

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
