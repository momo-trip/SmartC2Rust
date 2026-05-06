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

input='.
.TH foo 1 2025-10-03 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Description
Sed ut perspiciatis, unde omnis isteXXX
The latest version is always available from
.UR ftp://\:ftp.gnu.org/\:pub/\:gnu/\:bash/
.UE
and
.UR http://\:git\:.savannah\:.gnu\:.org/\:cgit/\:bash\:.git/\:snapshot/\
\:bash\-master\:.tar.\:gz
.UE .
.'

# Use a short line length to provoke a break within the URL.
output=$(printf "%s\n" "$input" \
    | "$groff" -rLL=65n -man -T ascii -P -cbou)
echo "$output"
echo "$output" | grep -q 'bash-master'

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
