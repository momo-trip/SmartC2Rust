#!/bin/sh
#
# Copyright 2020-2024 G. Branden Robinson
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

# Regression-test Savannah #58992.
#
# Our man macros should no longer attempt to read the .l register on
# nroff devices to set the line length.  That register may or may not
# have been set by a user .ll request; we can't tell whether a value of
# 65n came from nroff or the user.
#
# Instead, the LL register must be used to set the line length.
#
# In this test we _expect_ the .ll request to be ignored and overridden.
# We choose a value that is not nroff's default nor man's default.

input='.ll 70n
.TH ll\-hell 1 2020-08-22 "groff test suite"
.SH Name
ll\-hell \- see how long the lines are
.SH Description
.nr cells (\n[LL]u / \n[.H]u)
LL=\n[LL]u, .H=\n[.H]u, length=\n[cells]n
.PP
.nr cells (\n[.l]u / \n[.H]u)
\&.l=\n[.l]u, .H=\n[.H]u, length=\n[cells]n'

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man)
echo "$output"
echo "$output" | grep -q 'LL=1920u'

# vim:set ai et sw=4 ts=4 tw=72:
