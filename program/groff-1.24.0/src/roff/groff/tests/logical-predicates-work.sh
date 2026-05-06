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

input='.
.ec @
.@" Construct cases expecting "true"; invalid syntax is "false".
.ie t  .tm case "t"  true
.el    .tm case "t"  FALSE
.ie !n .tm case "!n" true
.el    .tm case "!n" FALSE
@c
.ie o  .tm case "o"  true
.el    .tm case "o"  FALSE
.ie !e .tm case "!e" true
.el    .tm case "!e" FALSE
.
.@" GNU troff extensions
.ie c a       .tm case "c"  true
.el           .tm case "c"  FALSE
.ie d .T      .tm case "d"  true
.el           .tm case "d"  FALSE
.ie F TR      .tm case "F"  true
.el           .tm case "F"  FALSE
.ie m default .tm case "m"  true
.el           .tm case "m"  FALSE
.ie r .T      .tm case "r"  true
.el           .tm case "r"  FALSE
.ie S R       .tm case "S"  true
.el           .tm case "S"  FALSE
.ie !v        .tm case "!v" true
.el           .tm case "!v" FALSE
.
.@" numeric expressions
.ie 1      .tm case "1"  true
.el        .tm case "1"  FALSE
.ie !0     .tm case "!0" true
.el        .tm case "!0" FALSE
.ie @n(.T  .tm register interpolation case true
.el        .tm register interpolation case FALSE
.ie !@n(.k .tm negated register interpolation case true
.el        .tm negated register interpolation case FALSE
.
.@" output comparison
.if @n(.C .tm expect warnings
.ie /foo/foo/  .tm output comparison case true
.el            .tm output comparison case FALSE
.ie !/foo/bar/ .tm negated output comparison case true
.el            .tm negated output comparison case FALSE
.'

error=$(printf "%s\n" "$input" | "$groff" -ww -T ps 2>&1 >/dev/null)
echo "$error"

echo "$error" | grep -Fqx 'case "t"  true' || wail
echo "$error" | grep -Fqx 'case "!n" true' || wail
echo "$error" | grep -Fqx 'case "o"  true' || wail
echo "$error" | grep -Fqx 'case "!e" true' || wail
echo "$error" | grep -Fqx 'case "c"  true' || wail
echo "$error" | grep -Fqx 'case "d"  true' || wail
echo "$error" | grep -Fqx 'case "m"  true' || wail
echo "$error" | grep -Fqx 'case "r"  true' || wail
echo "$error" | grep -Fqx 'case "S"  true' || wail
echo "$error" | grep -Fqx 'case "!v" true' || wail
echo "$error" | grep -Fqx 'case "1"  true' || wail
echo "$error" | grep -Fqx 'case "!0" true' || wail
echo "$error" | grep -Fqx 'register interpolation case true' \
    || wail
echo "$error" | grep -Fqx 'negated register interpolation case true' \
    || wail
echo "$error" | grep -Fqx 'output comparison case true' \
    || wail
echo "$error" | grep -Fqx 'negated output comparison case true' \
    || wail

echo '********** checking compatibility mode behavior' >&2

error=$(printf "%s\n" "$input" | "$groff" -C -ww -T ps 2>&1 >/dev/null)
echo "$error"

echo "$error" | grep -Fqx 'case "t"  true' || wail
echo "$error" | grep -Fqx 'case "!n" true' || wail
echo "$error" | grep -Fqx 'case "o"  true' || wail
echo "$error" | grep -Fqx 'case "!e" true' || wail
echo "$error" | grep -Fqx 'case "c"  true' || wail
echo "$error" | grep -Fqx 'case "d"  true' || wail
echo "$error" | grep -Fqx 'case "m"  true' || wail
echo "$error" | grep -Fqx 'case "r"  true' || wail
echo "$error" | grep -Fqx 'case "S"  true' || wail
echo "$error" | grep -Fqx 'case "!v" true' || wail
echo "$error" | grep -Fqx 'case "1"  true' || wail
echo "$error" | grep -Fqx 'case "!0" true' || wail
echo "$error" | grep -Fqx 'register interpolation case true' \
    || wail
echo "$error" | grep -Fqx 'negated register interpolation case true' \
    || wail
echo "$error" | grep -Fqx 'output comparison case FALSE' \
    || wail
echo "$error" | grep -Fqx 'negated output comparison case FALSE' \
    || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
