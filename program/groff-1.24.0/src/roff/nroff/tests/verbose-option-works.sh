#!/bin/sh
#
# Copyright 2020-2025 G. Branden Robinson
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

nroff="${abs_top_builddir:-.}/nroff"

if ! echo foobar | grep -qx foobar >/dev/null 2>&1
then
    echo "$0: grep command does not support -qx options; skipping" >&2
    exit 77 # skip
fi

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Ensure a predictable character encoding.
export LC_ALL=C
export LESSCHARSET=
export GROFF_TYPESETTER=

export GROFF_TEST_GROFF=${abs_top_builddir:-.}/test-groff

# The $PATH used by an installed nroff at runtime does not match what
# we're trying to test, which should be using the groff and runtime
# support from the build tree.  Therefore the $PATH that nroff -V
# reports will _always_ be wrong for test purposes.  Skip over it.
#
# If the build environment has a directory in the $PATH matching
# "test-groff " (with the trailing space), failure may result if sed
# doesn't match greedily.  POSIX says it should.
sedexpr='s/^PATH=.*test-groff /test-groff /'
PATH=${abs_top_builddir:-.}:$PATH

nroff_ver=$("$nroff" -v | awk 'NR == 1 {print $NF}')
groff_ver=$("$nroff" -v | awk 'NR == 2 {print $NF}')

# If this test is skipped, you may need to `make distclean` and rebuild.
echo nroff: $nroff_ver >&2
echo groff: $groff_ver >&2
if [ "$nroff_ver" != "$groff_ver" ]
then
    echo "$0: nroff and groff version numbers mismatch; skipping" >&2
    exit 77 # skip
fi

echo "checking 'nroff -V'" >&2
"$nroff" -V | sed "$sedexpr"
"$nroff" -V | sed "$sedexpr" | grep -qx "test-groff -Tascii -mtty-char" \
    || wail

echo "checking 'nroff -V 1'" >&2
"$nroff" -V 1 | sed "$sedexpr"
"$nroff" -V 1 | sed "$sedexpr" \
    | grep -qx "test-groff -Tascii -mtty-char 1" || wail

echo "checking 'nroff -V \"1a 1b\"'" >&2
"$nroff" -V \"1a 1b\" | sed "$sedexpr"
"$nroff" -V \"1a 1b\" | sed "$sedexpr" \
    | grep -qx "test-groff -Tascii -mtty-char \"1a 1b\"" || wail

echo "checking 'nroff -V \"1a 1b\" 2'" >&2
"$nroff" -V \"1a 1b\" 2 | sed "$sedexpr"
"$nroff" -V \"1a 1b\" 2 | sed "$sedexpr" \
    | grep -qx "test-groff -Tascii -mtty-char \"1a 1b\" 2" || wail

echo "checking 'nroff -V 1a\\\"1b 2'" >&2
"$nroff" -V 1a\"1b 2 | sed "$sedexpr"
"$nroff" -V 1a\"1b 2 | sed "$sedexpr" \
    | grep -qx "test-groff -Tascii -mtty-char 1a\"1b 2" || wail

echo "checking 'nroff -V -d FOO=BAR 1'" >&2
"$nroff" -V -d FOO=BAR 1 | sed "$sedexpr"
"$nroff" -V -d FOO=BAR 1 | sed "$sedexpr" \
    | grep -qx "test-groff -Tascii -mtty-char -d FOO=BAR 1" || wail

echo "checking for argument declustering support" >&2
"$nroff" -V -tz
if [ $? -eq 2 ]
then
    echo "$0: shell is not POSIX-conforming; skipping" >&2
    exit 77 # skip
fi

echo "checking argument declustering: 'nroff -V -tz'" >&2
"$nroff" -V -tz | sed "$sedexpr"
"$nroff" -V -tz | sed "$sedexpr" \
    | grep -qx "test-groff -Tascii -mtty-char -t -z" || wail

echo "checking argument declustering: 'nroff -V -tzms'" >&2
"$nroff" -V -tzms | sed "$sedexpr"
"$nroff" -V -tzms | sed "$sedexpr" \
    | grep -qx "test-groff -Tascii -mtty-char -t -z -ms" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
