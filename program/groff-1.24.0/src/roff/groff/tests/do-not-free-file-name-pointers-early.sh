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

# Regression-test Savannah #66981.
#
# File name strings in GNU troff tend to be dynamically allocated and to
# have highly variable lifetimes.  Aggressively freeing them can lead to
# undefined behavior (referencing deallocated memory).
#
# Because we're talking about dynamic memory UB, the following input may
# not reproduce bad behavior in all environments.  On GBR's system, the
# following is evident prior to the bug fix.
#
# {"name": "CE", "file name": "a", "starting line number": 2, ... }
# {"name": "CE", "file name": "\u0090\u0092\u009B\u00CE6V", ... }
#
# ...where the garbage in the file name varies with every run.

input='.
.lf 2 a
.ds CE \" empty
.TS H
l
l.
\&
.TH
\&
.pm CE
.TE
.pm CE
.'

output=$(printf '%s\n' "$input" | "$groff" -zt -ms 2>&1 | nl -ba)
echo "$output"
echo "$output" | sed -n 2p | grep -Fq '"file name": "a",'

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
