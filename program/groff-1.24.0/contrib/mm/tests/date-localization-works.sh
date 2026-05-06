#!/bin/sh
#
# Copyright 2024 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
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
    echo ...FAILED >&2
    fail=YES
}

input='.
.nr mo 10
.nr dy 6
.nr year 2024
.ND
.LT
.P
This is an
.I mm
letter.
.'

for lang in cs de en es fr it ru sv
do
    output=$(printf "%s\n" "$input" \
             | "$groff" -m m -m $lang -T utf8 -P -cbou | sed '/^$/d')
    echo "$output"
    case $lang in
        cs) pattern='6 .+jen 2024' ;;
        de) pattern='6\. Oktober 2024' ;;
        en) pattern='October 6, 2024' ;;
        es) pattern='6 de octubre de 2024' ;;
        fr) pattern='6 Octobre 2024' ;;
        it) pattern='6 Ottobre 2024' ;;
        ru) pattern='6 .+ 2024' ;; # a bit weak
        sv) pattern='6 oktober 2024' ;;
    esac
    echo "checking date localization in language $lang" >&2
    echo "$output" | grep -Eq "$pattern" || wail
done

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
