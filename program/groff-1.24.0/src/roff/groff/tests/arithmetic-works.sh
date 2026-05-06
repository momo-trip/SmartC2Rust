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

# Unit-test arithmetic.

# Start with a couple of cases that proved fragile when developing it.
#
# Incrementing a formatter internal via a request should produce the
# expected changes in magnitude (not repeat or square them).
#
# Vertical spacing exercises `vunits` and line length, `hunits`.

input='.
.ec @
.nf
.vs +40u
1: @&.v=@n(.v
.ll -2
2: @&.l=@n(.l
.ll 2147483647u
3: @&.l=@n(.l, .H=@n(.H
.ll
.pl 2147483647u
4: @&.p=@n(.p, .V=@n(.V
.pl
.'

# Expected:
#
# 1: .v=80
#
# 2: .l=1512
#
# 3: .l=2147483640, .H=24
#
# 4: .p=2147483640, .V=40
#
# The blank lines are due to the `vs` increase.

output=$(echo "$input" | "$groff" -w range -T ascii)
echo "$output"

echo "checking that vertical spacing is correctly incremented" >&2
echo "$output" | grep -Fqx '1: .v=80' || wail

echo "checking that line length is correctly incremented" >&2
echo "$output" | grep -Fqx '2: .l=1512' || wail

echo "checking that setting huge line length saturates" >&2
echo "$output" | grep -Fqx '3: .l=2147483640, .H=24' || wail

echo "checking that setting huge page length saturates" >&2
echo "$output" | grep -Fqx '4: .p=2147483640, .V=40' || wail

input='.
.ec @
.nf
.nr a 99999999999999999999n
.nr b (-99999999999999999999n)
.nr c 99999999999999999999v
.nr d (-99999999999999999999v)
a: @na
b: @nb
c: @nc
d: @nd
.'

output=$(echo "$input" | "$groff" -w range -T ascii)
echo "$output"

# The vunits and hunits constructors in src/roff/troff/number.cpp don't
# permit INT_MIN to be assigned, as a side effect of making rounding
# behavior sign-independent.

echo "checking that assignment of large positive horizontal" \
  "measurement saturates" >&2
echo "$output" | grep -Fqx 'a: 2147483647' || wail

echo "checking assignment of large negative horizontal" \
  "measurement saturates" >&2
echo "$output" | grep -Fqx 'b: -2147483647' || wail

echo "checking assignment of large positive vertical" \
  "measurement saturates" >&2
echo "$output" | grep -Fqx 'c: 2147483647' || wail

echo "checking assignment of large negative vertical" \
  "measurement saturates" >&2
echo "$output" | grep -Fqx 'd: -2147483647' || wail

# Exercise boundary values.

input='.
.ec @
.nf
.nr i 99999999999999999999
1: 99999999999999999999 -> @ni
.nr i (-99999999999999999999)
2: (-99999999999999999999) -> @ni
.nr i 2147483647
3: assign 2147483647 -> @ni
.nr i +1
4: incr 2147483647 -> @ni
.nr i 2147483647
.nr i (-1)*@ni
5: (-1)*2147483647 -> @ni
.nr i -1
6: decr -2147483648 -> @ni
.'

output=$(echo "$input" | "$groff" -w range -T ascii)
echo "$output"

echo "checking assignment of huge value to register" >&2
echo "$output" \
  | grep -Fqx '1: 99999999999999999999 -> 999999999' || wail

echo "checking assignment of huge negative value to register" >&2
echo "$output" \
  | grep -Fqx '2: (-99999999999999999999) -> -999999999' || wail

echo "checking assignment of 2^31 - 1 to register" >&2
echo "$output" | grep -Fqx '3: assign 2147483647 -> 2147483647' || wail

echo "checking incrementation of register" >&2
echo "$output" | grep -Fqx '4: incr 2147483647 -> -2147483648' || wail

echo "checking negation of positive register" >&2
echo "$output" | grep -Fqx '5: (-1)*2147483647 -> -2147483647' || wail

echo "checking decrementation of register" >&2
echo "$output" | grep -Fqx '6: decr -2147483648 -> -2147483648' || wail

# A quirk that GNU troff arithmetic has is that one cannot directly
# assign INT_MIN to a register, because the negative sign is parsed
# separately from the digit sequence.  (This is true even when the
# negative sign is not interpreted as a decrementation operator.)
#
# .nr a 2147483647
# .tm a=\na
#   a=2147483647
# .nr a \na*-1
# .tm a=\na
#   a=-2147483647
# .nr a -1
# .tm a=\na
#   a=-2147483648
# .nr a \na*-1
# troff:<standard input>:7: warning: integer value saturated
#
# This seems okay to GBR.

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
