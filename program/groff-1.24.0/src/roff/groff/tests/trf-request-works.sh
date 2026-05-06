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

fail=

wail () {
  echo ...FAILED >&2
  fail=YES
}

# Locate directory containing our test artifacts.
artifact_dir=
base=src/roff/groff/tests
dir=artifacts

for buildroot in . .. ../..
do
    d=$buildroot/$base/$dir
    if [ -d "$d" ]
    then
        artifact_dir=$d
        break
    fi
done

# If we can't find it, we can't test.
test -z "$artifact_dir" && exit 77 # skip

comment='# this is a trout/grout comment'

input1=".
hello
.trf $artifact_dir/throughput-file
world
."

output=$(printf "%s" "$input1" | "$groff" -b -ww -Z)
echo "$output"
echo "checking that a transparent throughput file works" >&2
echo "$output" | grep -Fqx "$comment" || wail

# Regression-test Savannah #67532.
#
# A diverted throughput file's contents should not be formatted as text.

input2=".
.box DIV
hello
.trf $artifact_dir/throughput-file
world
.br
.box
.DIV
."

output=$(printf "%s" "$input2" | "$groff" -b -ww -Z)
echo "$output"
echo "checking that a diverted transparent throughput file works" >&2
echo "$output" | grep -Fqx "$comment" || wail

# Regression-test Savannah #66863.
#
# The `trf` request should not excessively advance the input stream
# pointer.

output=$(printf "%s" "$input1" | "$groff" -b -ww -a)
echo "$output"
echo "checking that 'trf' request advances input stream pointer" \
  "correctly" >&2
echo "$output" | grep -qx world || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
