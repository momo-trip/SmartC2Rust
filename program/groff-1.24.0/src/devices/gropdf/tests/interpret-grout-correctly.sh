#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
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

gropdf="${abs_top_builddir:-.}/gropdf"
fontdir="${abs_top_builddir:-.}/font"

# Regression-test Savannah #68077.

input='#
x T pdf
x res 72000 1 1
x init
p1
x X ps:exec [/Dest /foo(1) /View [/FitH -26000 u] /DEST pdfmark
x X ps:exec [/Dest /foo(1) /Title (foo(1)) /Level 1 /OUT pdfmark
x font 39 TI
f39
s10000
md
DFd
V43000
H73970
tfoo
x font 5 TR
f5
h180
t(1)
h151030
tGeneral
wh2500
tCommands
wh2500
tManual
f39
h153000
tfoo
f5
h180
t(1)
n12000 0
x X pdf: markrestart
V91000
H72000
tdigit
wx font 37 TB
f37
h2500
t0
f5
t.
n12000 0
x X pdf: marksuspend
V756000
H72000
tgrof
H88410
tf
wh2500
ttest
wh2500
tsuite
h153150
t2026-02-20
h205670
t1
n12000 0
x trailer
V792000
x stop
#'

if ! command -v pdftotext >/dev/null
then
    echo "$0: cannot locate 'pdftotext' command; skipping" >&2
    exit 77 # skip
fi

output=$(echo "$input" | "$gropdf" -F "$fontdir" | pdftotext - -)
echo "$output"
echo "$output" | grep -Fq 'digit 0.'

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
