#!/bin/sh
#
# Copyright 2022-2024 G. Branden Robinson
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

if ! echo foobar | grep -Fqx foobar >/dev/null 2>&1
then
    echo "$0: grep command does not support -Fqx options; skipping" >&2
    exit 77 # skip
fi

fail=

wail() {
    echo ...FAILED >&2
    fail=yes
}

input='.
.TH foo 1 2022-11-22 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH Description
See
.UR http://\:foo\:.example\:.com
figure 1
.UE .
.
Or
.UR http://\:bar\:.example\:.com
.UE .
.
.
.TP
.UR http://\:baz\:.example\:.com
.UE
Frumious!
.'

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -rU0)
echo "$output"

echo "checking formatting of web URI with link text" \
    "(ascii device; hyperlinks disabled)" >&2
echo "$output" | grep -Fq 'See figure 1 <http://foo.example.com>.' \
    || wail

echo "checking formatting of web URI with no link text" \
    "(ascii device; hyperlinks disabled)" >&2
echo "$output" | grep -Fq 'Or <http://bar.example.com>.' || wail

echo "checking formatting of web URI with no link text as paragraph" \
     "tag (ascii device; hyperlinks disabled)" >&2
echo "$output" | grep -Fq ' <http://baz.example.com>' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -rU1)
echo "$output"

echo "checking formatting of web URI with link text" \
    "(ascii device; hyperlinks enabled)" >&2
echo "$output" | grep -Fq 'See figure 1.' || wail

echo "checking formatting of web URI with no link text" \
    "(ascii device; hyperlinks enabled)" >&2
echo "$output" | grep -Fq 'Or http://bar.example.com.' || wail

echo "checking formatting of web URI with no link text as paragraph" \
     "tag (ascii device; hyperlinks enabled)" >&2
echo "$output" | grep -Fq ' http://baz.example.com' || wail

html_input='.
.TH foo 1 2022-12-04 "groff test suite"
.SH Name
foo \- frobnicate a bar
.SH "See also"
For our SOSP presentation,
check our
.UR https://\:example\:.com
website
.UE .
.'

output=$(printf "%s\n" "$html_input" | "$groff" -man -Thtml)
echo "$output"

echo "checking HTML output of web URI" >&2
echo "$output" \
    | grep -Fqx '<a href="https://example.com">website</a>.</p>' \
    || wail

# We want to report failure (if it happened) before we attempt checks we
# might have to skip.
test -z "$fail" || exit

if ! command -v pdftotext >/dev/null
then
    echo "$0: cannot locate 'pdftotext' command; skipping" >&2
    exit 77 # skip
fi

# At least two different implementations of pdftotext exist; if we don't
# have the poppler version, assume it's xpdfreader.com's.
have_poppler_pdftotext=yes
if pdftotext -v 2>&1 | grep -q xpdfreader
then
  have_poppler_pdftotext=
fi

if [ -n "$have_poppler_pdftotext" ]
then
  output=$(printf "%s\n" "$input" | "$groff" -Tpdf -P-d -man -rU0 \
    | pdftotext - -)
else
  tmp_file=$(mktemp)
  printf "%s\n" "$input" | "$groff" -Tpdf -P-d -man -rU0 > "$tmp_file"
  output=$(pdftotext "$tmp_file" -)
  rm "$tmp_file"
fi
echo "$output"

# The version of pdftotext on Solaris 10 writes 'figure' with an 'fi'
# ligature.
echo "checking formatting of web URI with link text" \
    "(pdf device; hyperlinks disabled)" >&2
# expected: See figure 1 〈http://foo.example.com〉.
echo "$output" | grep -q 'See .*gure 1 .*http://foo.example.com.*\.' \
    || wail

echo "checking formatting of web URI with no link text" \
    "(pdf device; hyperlinks disabled)" >&2
# expected: Or 〈http://bar.example.com〉.
echo "$output" | grep -q 'Or .*http://bar.example.com.*\.' || wail

if [ -n "$have_poppler_pdftotext" ]
then
  output=$(printf "%s\n" "$input" | "$groff" -Tpdf -P-d -man -rU1 \
    | pdftotext - -)
else
  tmp_file=$(mktemp)
  printf "%s\n" "$input" | "$groff" -Tpdf -P-d -man -rU1 > "$tmp_file"
  output=$(pdftotext "$tmp_file" -)
  rm "$tmp_file"
fi
echo "$output"

# The version of pdftotext on Solaris 10 writes 'figure' with an 'fi'
# ligature.
echo "checking formatting of web URI with link text" \
    "(pdf device; hyperlinks enabled)" >&2
# expected: See figure 1. Or http://bar.example.com.
echo "$output" | grep -q 'See .*gure 1. Or' || wail

echo "checking formatting of web URI with no link text" \
    "(pdf device; hyperlinks enabled)" >&2
# expected: See figure 1. Or http://bar.example.com.
echo "$output" | grep -Fq 'Or http://bar.example.com.' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
