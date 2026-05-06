#!/bin/sh
#
# Copyright 2021-2025 G. Branden Robinson
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
    echo "...FAILED" >&2
    fail=YES
}

input='.tm .hy=\n[.hy]'

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mcs 2>&1)
echo 'checking raw troff with -mcs' >&2
echo "$output" | grep -Fqx '.hy=1' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mde 2>&1)
echo 'checking raw troff with -mde' >&2
echo "$output" | grep -Fqx '.hy=1' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -men 2>&1)
echo 'checking raw troff with -men' >&2
echo "$output" | grep -Fqx '.hy=4' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mes 2>&1)
echo 'checking raw troff with -mes' >&2
echo "$output" | grep -Fqx '.hy=1' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mfr 2>&1)
echo 'checking raw troff with -mfr' >&2
echo "$output" | grep -Fqx '.hy=4' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mit 2>&1)
echo 'checking raw troff with -mit' >&2
echo "$output" | grep -Fqx '.hy=1' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mpl 2>&1)
echo 'checking raw troff with -mpl' >&2
echo "$output" | grep -Fqx '.hy=1' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mru 2>&1)
echo 'checking raw troff with -mru' >&2
echo "$output" | grep -Fqx '.hy=1' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -msv 2>&1)
echo 'checking raw troff with -msv' >&2
echo "$output" | grep -Fqx '.hy=32' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -mcs 2>&1)
echo 'checking -me with -mcs' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -mde 2>&1)
echo 'checking -me with -mde' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -men 2>&1)
echo 'checking -me with -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -mes 2>&1)
echo 'checking -me with -mes' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -mfr 2>&1)
echo 'checking -me with -mfr' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -mit 2>&1)
echo 'checking -me with -mit' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -mpl 2>&1)
echo 'checking -me with -mpl' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -mru 2>&1)
echo 'checking -me with -mru' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -me -msv 2>&1)
echo 'checking -me with -msv' >&2
echo "$output" | grep -Fqx '.hy=34' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -mcs 2>&1)
echo 'checking -ms with -mcs' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -mde 2>&1)
echo 'checking -ms with -mde' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -men 2>&1)
echo 'checking -ms with -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -mes 2>&1)
echo 'checking -ms with -mes' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -mfr 2>&1)
echo 'checking -ms with -mfr' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -mit 2>&1)
echo 'checking -ms with -mit' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -mpl 2>&1)
echo 'checking -ms with -mpl' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -mru 2>&1)
echo 'checking -ms with -mru' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -ms -msv 2>&1)
echo 'checking -ms with -msv' >&2
echo "$output" | grep -Fqx '.hy=34' || wail

input='.TH foo 1 2022-01-06 "groff test suite"
.tm .hy=\n[.hy]'

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -mcs 2>&1)
echo 'checking -man with -mcs' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -mde 2>&1)
echo 'checking -man with -mde' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -men 2>&1)
echo 'checking -man with -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -mes 2>&1)
echo 'checking -man with -mes' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -mfr 2>&1)
echo 'checking -man with -mfr' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -mit 2>&1)
echo 'checking -man with -mit' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -mpl 2>&1)
echo 'checking -man with -mpl' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -mru 2>&1)
echo 'checking -man with -mru' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -man -msv 2>&1)
echo 'checking -man with -msv' >&2
echo "$output" | grep -Fqx '.hy=34' || wail

# Ensure that the 'trap bit' (hyphenation value 2, which has nothing to
# do with any language) is preserved when switching locales back from a
# CJK language, since those languages' modes unconditionally clear it.

input='.
.TH foo 1 2022-04-09 "groff test suite"
.SH 名前
foo \- APT 用選択制御ファイル
.mso en.tmac
.TH bar 1 2022-04-09 "groff test suite"
.SH Name
bar \- three subjects walk into this
.tm .hy=\n[.hy]
.'

output=$(printf "%s\n" "$input" | "$groff" -K utf8 -Tutf8 -P-cbou \
  -man -mja -men 2>&1)
echo 'checking -man with -mja -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

input='.
.TH foo 1 2022-04-09 "groff test suite"
.SH 名称
foo \- 解析 man 手册页的头部信息
.mso en.tmac
.TH bar 1 2022-04-09 "groff test suite"
.SH Name
bar \- three subjects walk into this
.tm .hy=\n[.hy]
.'

output=$(printf "%s\n" "$input" | "$groff" -K utf8 -Tutf8 -P-cbou \
  -man -mzh -men 2>&1)
echo 'checking -man with -mzh -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

input='.
.Dd 2025-07-16
.Dt foo 1
.Os "groff test suite"
.tm .hy=\n[.hy]
.'

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -mcs 2>&1)
echo 'checking -mdoc with -mcs' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -mde 2>&1)
echo 'checking -mdoc with -mde' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -men 2>&1)
echo 'checking -mdoc with -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -mes 2>&1)
echo 'checking -mdoc with -mes' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -mfr 2>&1)
echo 'checking -mdoc with -mfr' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -mit 2>&1)
echo 'checking -mdoc with -mit' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -mpl 2>&1)
echo 'checking -mdoc with -mpl' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -mru 2>&1)
echo 'checking -mdoc with -mru' >&2
echo "$output" | grep -Fqx '.hy=2' || wail

output=$(printf "%s\n" "$input" | "$groff" -Tascii -P-cbou -mdoc -msv 2>&1)
echo 'checking -mdoc with -msv' >&2
echo "$output" | grep -Fqx '.hy=34' || wail

# Ensure that the 'trap bit' (hyphenation value 2, which has nothing to
# do with any language) is preserved when switching locales back from a
# CJK language, since those languages' modes unconditionally clear it.

input='.
.Dd 2025-07-16
.Dt foo 1
.Os "groff test suite"
.Sh 名前
.Nm foo
.Nd APT 用選択制御ファイル
.mso en.tmac
.Dd 2025-07-16
.Dt bar 1
.Os "groff test suite"
.Sh Name
.Nm bar
.Nd three subjects walk into this
.\".Sh Description
.\"Stuff.
.tm .hy=\n[.hy]
.'

output=$(printf "%s\n" "$input" | "$groff" -K utf8 -Tutf8 -P-cbou \
  -mdoc -mja -men 2>&1)
echo 'checking -mdoc with -mja -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

input='.
.Dd 2025-07-16
.Dt foo 1
.Os "groff test suite"
.Sh 名称
.Nm foo
.Nd 解析 mdoc 手册页的头部信息
.mso en.tmac
.Dd 2025-07-16
.Dt bar 1
.Os "groff test suite"
.Sh Name
.Nm bar
.Nd three subjects walk into this
.\".Sh Description
.\"Stuff.
.tm .hy=\n[.hy]
.'

output=$(printf "%s\n" "$input" | "$groff" -K utf8 -Tutf8 -P-cbou \
  -mdoc -mzh -men 2>&1)
echo 'checking -mdoc with -mzh -men' >&2
echo "$output" | grep -Fqx '.hy=6' || wail

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
