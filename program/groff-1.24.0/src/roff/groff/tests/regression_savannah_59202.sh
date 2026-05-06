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

# troff should not segfault when its standard output is closed.
# Savannah #59202.

if [ -e core ]
then
  echo "$0: 'core' file already exists; skipping" >&2
  exit 77 # skip
fi

echo | "$groff" >&-
! test -e core

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
