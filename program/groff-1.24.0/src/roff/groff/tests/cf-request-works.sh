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

# Regression-test Savannah #66863.
#
# The `cf` request should not excessively advance the input stream
# pointer.

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

input=".
hello
.cf $artifact_dir/throughput-file
world
."

output=$(printf "%s" "$input" | "$groff" -b -ww -Ua)
echo "$output"
echo "$output" | grep -Fqx world

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
