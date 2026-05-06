#!/bin/sh
#
# Copyright 2022 G. Branden Robinson
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
# Give the output a name that won't collide with another test.
gnu_pdf="${abs_top_builddir:-.}/doc/gnu-no-choke-on-pdfinfo.pdf"

# Regression-test Savannah #58206.

# We need gs(1) and pdfpic.tmac needs pdfinfo(1).
# TODO: Generate this test script from an '.in' file and search for the
# `GHOSTSCRIPT` command name detected by our "configure" script.
# See "contrib/hdtbl/examples/test-hdtbl.sh.in".
for cmd in gs pdfinfo
do
    if ! command -v $cmd >/dev/null
    then
        echo "$0: cannot locate '$cmd' command; skipping" >&2
        exit 77 # skip
    fi
done

# Locate directory containing our test artifacts.
artifact_dir=

for buildroot in . .. ../..
do
    d=$buildroot/doc
    if [ -f $d/gnu.eps ]
    then
        artifact_dir=$d
        gnu_eps=$artifact_dir/gnu.eps
        break
    fi
done

# If we can't find it, we can't test.
if [ -z "$artifact_dir" ]
then
    echo "$0: cannot locate test artifact directory; skipping" >&2
    exit 77 # skip
fi

if [ -e "$gnu_pdf" ]
then
    echo "$0: temporary output file '$gnu_pdf' already exists;" \
        "skipping" >&2
    exit 77 # skip
fi

fail=

input='.am pdfpic@error
.  ab
..
Here is a picture of a wildebeest.
.PDFPIC '"$gnu_pdf"

if ! gs -q -o - -sDEVICE=pdfwrite -f "$gnu_eps" \
    -c "[ /Title (\000B\000U\000S\000T\000E\000D) /DOCINFO pdfmark" \
    > "$gnu_pdf"
then
    echo "gs command failed" >&2
    rm -f "$gnu_pdf"
    exit 77 # skip
fi

test -z "$fail" \
    && printf '%s\n' "$input" | "$groff" -Tpdf -U -z || fail=YES

rm -f "$gnu_pdf"
test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
