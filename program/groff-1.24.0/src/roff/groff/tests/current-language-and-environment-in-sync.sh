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

# Unit-test synchronization between the formatter's "current language"
# (global) and the hyphenation language code in the current environment.
#
# See Savannah #66387 and #66392.

input='.
.tm 1 en=\n[.hla]
.ev french
.tm 2 (empty)=\n[.hla]
.hla fr
.tm 3 fr=\n[.hla]
.ev
.tm 4 en=\n[.hla]
.ev french
.tm 5 fr=\n[.hla]
.ev german
.tm 6 (empty)=\n[.hla]
.hla de
.tm 7 de=\n[.hla]
.ev no-language
.tm 8 (empty)=\n[.hla]
.ev
.tm 9 de=\n[.hla]
.ev
.tm A fr=\n[.hla]
.ev
.tm B en=\n[.hla]
.ev copy
.tm C (empty)=\n[.hla]
.evc french
.tm D fr=\n[.hla]
.ev no-language
.tm E (empty)=\n[.hla]
.hla en
.tm F en=\n[.hla]
.'

# Expected output:
#
# 1 en=en
# 2 (empty)=
# 3 fr=fr
# 4 en=en
# 5 fr=fr
# 6 (empty)=
# 7 de=de
# 8 (empty)=
# 9 de=de
# A fr=fr
# B en=en
# C (empty)=
# D fr=fr
# E (empty)=
# F en=en

output=$(printf "%s\n" "$input" | "$groff" -z 2>&1)
echo "$output"

echo "verifying that language code on startup is 'en'" >&2
echo "$output" | grep -Fqx '1 en=en' || wail

echo "verifying that language code in new environment is empty (1)" >&2
echo "$output" | grep -Fqx '2 (empty)=' || wail

echo "verifying that language code can be set to 'fr'" >&2
echo "$output" | grep -Fqx '3 fr=fr' || wail

echo "verifying that language code reverts on environment pop (1)" >&2
echo "$output" | grep -Fqx '4 en=en' || wail

echo "verifying that language code restored on environment push (1)" >&2
echo "$output" | grep -Fqx '5 fr=fr' || wail

echo "verifying that language code in new environment is empty (2)" >&2
echo "$output" | grep -Fqx '6 (empty)=' || wail

echo "verifying that language code can be set to 'de'" >&2
echo "$output" | grep -Fqx '7 de=de' || wail

echo "verifying that language code in new environment is empty (3)" >&2
echo "$output" | grep -Fqx '8 (empty)=' || wail

echo "verifying that language code restored on environment pop (2)" >&2
echo "$output" | grep -Fqx '9 de=de' || wail

echo "verifying that language code restored on environment pop (3)" >&2
echo "$output" | grep -Fqx 'A fr=fr' || wail

echo "verifying that language code restored on environment pop (4)" >&2
echo "$output" | grep -Fqx 'B en=en' || wail

echo "verifying that language code in new environment is empty (4)" >&2
echo "$output" | grep -Fqx 'C (empty)=' || wail

echo 'verifying that `evc` request copies language code' >&2
echo "$output" | grep -Fqx 'D fr=fr' || wail

echo "verifying that empty language code restored on environment push" \
  >&2
echo "$output" | grep -Fqx 'E (empty)=' || wail

echo 'verifying that assertion holds when using `hla` request' >&2
echo "$output" | grep -Fqx 'F en=en' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
