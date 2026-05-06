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
  echo ...FAILED >&2
  fail=YES
}

# Omit the trailing newlines from input; they are supplied below.
# 4 backslashes
input_plain=$(printf "\\\\X'ps: nop'")

# 4, 8 backslashes
input_special=$(printf "\\\\X'pdf: \\\\\\\\[u007E]'\n")

for device in ascii dvi html xhtml latin1 lbp lj4 pdf ps utf8 \
              X75 X75-12 X100 X100-12
do
  echo "checking device command escape operation on $device device" >&2
  output=$(printf "%s\n" "$input_plain" | "$groff" -T $device -Z) \
    || wail
  echo "$output"
  echo "$output" | grep -q 'x * X *ps: nop' || wail

  echo "checking that Unicode escape sequence is preserved on $device" \
       "device" >&2
  output=$(printf "%s\n" "$input_special" | "$groff" -T $device -Z) \
    || wail
  echo "$output"
  # 3 backslashes
  echo "$output" | grep -q 'x * X *pdf: \\\[u007E]' || wail
done

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
