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

fail=

wail () {
  echo "...FAILED" >&2
  fail=yes
}

# Regression-test Savannah #66255.

tmpfile=savannah-66255.txt

cleanup () {
  rm -f $tmpfile
}

# A process handling a fatal signal should:
#   1.  Mask all fatal signals of interest.  (GBR often excludes ABRT.)
#   2.  Perform cleanup operations.
#   3.  Unmask the signal (removing the handler).
#   4.  Signal its own process group with the signal caught so that the
#       the children exit and shell accurately reports how the process
#       died.
fatals="HUP INT QUIT TERM"
for s in $fatals
do
  trap "trap '' $fatals; cleanup; trap - $fatals; kill -$s -$$" $s
done

input='.
.open mystream '$tmpfile'
.write mystream a
.write mystream b
.write mystream
.write mystream c
.close mystream
.'

output=$(echo "$input" | "$groff" -U)
od -c $tmpfile

echo "verifying presence of desired text in output file" >&2
grep -qx 'c' $tmpfile || wail

echo "verifying absence of garbage text in output file" >&2
grep -q 'write' $tmpfile && wail

cleanup
test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
