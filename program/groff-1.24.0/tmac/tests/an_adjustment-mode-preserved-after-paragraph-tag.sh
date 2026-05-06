#!/bin/sh
#
# Copyright 2024-2025 G. Branden Robinson
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

# Regression-test Savannah #65462.

input='.
.TH foo 1 2024-03-15 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Description
.\" The seemingly casual text below is crafted so that one can visually
.\" verify the presence or absence of adjustment.
.\"   groff <= 1.23 defaults: BP=7n, LL=78n (BP didn'"'"'t exist yet)
.\"   groff >  1.23 defaults: BP=5n, LL=80n
Let us measure our degree of gruntlement.
.P
.ad l
First, align output to the left.
A dubious practice,
to be sure,
but the house style of Perl man pages to date.
.IR pod2man (1)
from podlators\~6.0
employs
.I groff
1.23'"'"'s
.B AD
string.
.TP
Seventh Edition Unix
had the same preference in
.I nroff
mode in 1979,
and did not adjust the text to both margins on terminals
(using the
.B na
request),
though it did when using a typesetter.
.TP
SunOS
commented out the disablement of adjustment as early as its 2.0 release
(1982),
and appears to have retained that all the way through Solaris 10.
.P
As of
.I groff
1.24,
paragraphing macros reset the adjustment mode to the default configured
by the user,
or by the document if the user did not configure one.
.P
Due to differences between implementations,
between
.I nroff
and
.I troff
modes,
and to the surprising behavior of the request sequence
.IP
.EX
\&.ad l
\&.na
\&.ad
.EE
.P
no useful idioms for use of the
.B ad
request
in man pages have arisen.
.'

output=$(printf "%s\n" "$input" | "$groff" -man -Tascii -P-cbou)
echo "$output"
# Assume adjustment starting from the left on this output line.
echo "$output" \
    | grep -Eq 'had  +the  +same  +preference in nroff mode in 1979'

exit

# vim:set ai et sw=4 ts=4 tw=72:
