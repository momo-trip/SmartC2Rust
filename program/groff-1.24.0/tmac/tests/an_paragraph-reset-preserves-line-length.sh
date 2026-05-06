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
'"'"'\" t
.TH foo 1 "2025-07-23" "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Description
Here is a table.
.IP
.TS
tab(@);
Lf(BI) Lb
L Lx.
option@Effect
AT@T{
Attention;
put contents of string
.B LetAT
and
.I value
left-aligned after the inside address.
T}
CN@T{
Confidential;
put
.I value,
or contents of string
.BR LetCN ,
left-aligned after the date.
T}
RN@T{
Reference;
put contents of string
.B LetRN
and
.I value
after the confidential notation
(if any)
and the date,
aligned with the latter.
T}
SA@T{
Salutation;
put
.I value,
or contents of string
.BR LetSA ,
left-aligned after the inside address
and the confidential notation
(if any).
T}
SJ@T{
Subject;
put contents of string
.B LetSJ
and
.I value
left-aligned after the inside address
and the attention and salutation notations
(if any).
.
In letter type \[lq]SP\[rq],
.B LetSJ
is ignored and
.I value
is set in full capitals.
T}
.TE
.'

output=$(printf "%s\n" "$input" | "$groff" -t -man -Tascii -P -cbou)
echo "$output"
echo "$output" \
    | grep -Eq 'confidential notation \(if any\) and the date, +aligned +with$'

# vim:set ai et sw=4 ts=4 tw=72:
