#!/bin/sh
#
# Copyright 2022 Free Software Foundation, Inc.
#
# contributed by TANAKA Takuji
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
  echo ...FAILED >&2
  fail=YES
}

# test for PostScript with Japanese, grout output
#   "さざ波" -> \343\201\225\343\201\226\346\263\242
jstr='\343\201\225\343\201\226\346\263\242'
echo "checking 'groff -Kutf8 -Tps -Z'" >&2
printf ".ft JPM\n$jstr\n" | "$groff" -Kutf8 -Tps -Z | tr '\n' ';' \
  | grep -q \
    ';C *u3055;h *10000;C *u3055_3099;h *10000;C *u6CE2;h *10000;' \
  || wail

# test for PostScript with Japanese, PS output using UTF-16BE encoding
echo "checking 'groff -Kutf8 -Tps'" >&2
printf ".ft JPM\n$jstr\n" | "$groff" -Kutf8 -Tps \
  | grep -q '/Ryumin-Light-UniJIS-UTF16-H SF<305530566CE2>' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
