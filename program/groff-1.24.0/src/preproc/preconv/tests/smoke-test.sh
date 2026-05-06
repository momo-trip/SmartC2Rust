#!/bin/sh
#
# Copyright 2020-2024 G. Branden Robinson
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

preconv="${abs_top_builddir:-.}/preconv"

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Ensure a predictable character encoding.
export LC_ALL=C

echo "testing -e flag override of BOM detection" >&2
printf '\376\377\0\100\0\n' \
    | "$preconv" -d -e euc-kr 2>&1 > /dev/null \
    | grep -q "no search for coding tag" || wail

echo "testing detection of UTF-32BE BOM" >&2
printf '\0\0\376\377\0\0\0\100\0\0\0\n' \
    | "$preconv" -d 2>&1 > /dev/null \
    | grep -q "found BOM" || wail

echo "testing detection of UTF-32LE BOM" >&2
printf '\377\376\0\0\100\0\0\0\n\0\0\0' \
    | "$preconv" -d 2>&1 > /dev/null \
    | grep -q "found BOM" || wail

echo "testing detection of UTF-16BE BOM" >&2
printf '\376\377\0\100\0\n' \
    | "$preconv" -d 2>&1 > /dev/null \
    | grep -q "found BOM" || wail

echo "testing detection of UTF-16LE BOM" >&2
printf '\377\376\100\0\n\0' \
    | "$preconv" -d 2>&1 > /dev/null \
    | grep -q "found BOM" || wail

echo "testing detection of UTF-8 BOM" >&2
printf '\357\273\277@\n' \
    | "$preconv" -d 2>&1 > /dev/null \
    | grep -q "found BOM" || wail

# We do not find a coding tag on piped input because it isn't seekable.
echo "testing detection of Emacs coding tag in piped input" >&2
printf '.\\" -*- coding: euc-kr; -*-\\n' \
    | "$preconv" -d 2>&1 >/dev/null \
    | grep -q "no coding tag" || wail

test -z "$fail" || exit

# We need uchardet to work to get past this point.
if ! "$preconv" -v | grep -q 'with uchardet support'
then
    echo "$0: preconv lacks uchardet support; skipping" >&2
    exit 77 # skip
fi

# Instead of using temporary files, which in all fastidiousness means
# cleaning them up even if we're interrupted, which in turn means
# setting up signal handlers, we use files in the build tree.

# TODO: groff_mmse(7) is no longer UTF-8-encoded; find another.
#doc=contrib/mm/groff_mmse.7
#echo "testing uchardet detection on UTF-8 document $doc" >&2
#"$preconv" -d -D us-ascii 2>&1 >/dev/null $doc \
#    | grep -q 'charset: UTF-8' || wail

# uchardet can't seek on a pipe either.
echo "testing uchardet detection on pipe (expect fallback to -D)" >&2
printf 'Eat at the caf\351.\n' \
    | "$preconv" -d -D euc-kr 2>&1 > /dev/null \
    | grep -q "encoding used: 'EUC-KR'" || wail

test -z "$fail" || exit

# Fall back to the locale.
#
# It's hard to determine the character encoding of the 'C' locale
# because the only POSIX-standard way to do so is to build a C program
# to call `nl_langinfo(CODESET)`.  There's also no POSIX-standard way
# to ask a system to report the byte sequence it uses to encode, say,
# "lowercase e with acute accent".
#
# (I think Perl can do that, though.)
#
# We're just a shell script, so on non-glibc systems, we guess at it.
#
# On glibc systems, the 'C' locale uses "ANSI_X3.4-1968" for the
# character set, and `locale charmap` tells us as much, but preconv
# assumes Latin-1 instead of US-ASCII, so we override that.
#
# On Darwin (macOS) systems, we do the same.  See
# <https://lists.gnu.org/archive/html/groff/2026-02/msg00129.html>.
#
# For everything else, we assume UTF-8.

libc_vendor=

if command -v locale > /dev/null
then
    libc_vendor=gnu
    charset=ISO-8859-1
elif [ "$(uname -s)" = "Darwin" ]
then
    libc_vendor=apple
    charset=ISO-8859-1
else
    libc_vendor=unknown
    charset=UTF-8
fi

printf "standard C library vendor: %s;" $libc_vendor >&2
printf " expecting preconv character encoding %s\n" $charset >&2

echo "testing fallback to locale setting in environment" >&2
printf 'Eat at the caf\351.\n' \
    | "$preconv" -d 2>&1 > /dev/null \
    | grep -q "encoding used: '$charset'" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
