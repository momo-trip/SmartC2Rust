#!/bin/sh
#
# Copyright 2021-2025 G. Branden Robinson
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

# Regression-test Savannah #43532.
#
# Excessively long man page titles can overrun other parts of the titles
# (headers and footers).  Verify abbreviation of ones that would.

fail=

wail () {
    echo "...FAILED" >&2
    fail=YES
}

input='.TH foo 1 2021-05-31 "groff test suite"
.SH Name
foo \- a command with a very short name'

output=$(echo "$input" | "$groff" -Tascii -P-cbou -man)
echo "$output"

echo "checking that short man page title is set acceptably" >&2
echo "$output" \
    | grep -Eq 'foo\(1\) +General Commands Manual +foo\(1\)' || wail

input='.TH CosNotifyChannelAdmin_StructuredProxyPushSupplier 3erl \
2021-05-31 "groff test suite" "Erlang Module Definition"
.SH Name
CosNotifyChannelAdmin_StructuredProxyPushSupplier \- OMFG'

output=$(echo "$input" | "$groff" -Tascii -P-cbou -man)
echo "$output"

echo "checking that ultra-long man page title is abbreviated" >&2
title_abbv="CosNotif...hSupplier(3erl)"
# 2 spaces each before "Erlang" and after "Definition"
pattern="$title_abbv  Erlang Module Definition  $title_abbv"
echo "$output" | grep -Fq "$pattern" || wail

input='.
.TH SSL_CTX_sess_set_remove_cb 3 2025-01-09 "groff test suite"
.SH Name
SSL_CTX_sess_set_remove_cb \- use a brilliantly designed API
.'

# Check every line length between 64 and 68 inclusive.  Some of these
# values elicit crowding in groff 1.23.

echo "checking that known troublesome man page title abbreviates well" \
    >&2
for width in 64 65 66 67 68
do
    echo "...at width ${width}n" >&2
    output=$(echo "$input" \
        | "$groff" -r LL=${width}n -m an -T ascii -P -cbou)
    echo "$output"
    if ! echo "$output" | sed -n 1p | grep ' Library Functions Manual '
    then
        wail
        break
    fi
done

test -z "$fail"

# vim:set ai et sw=4 ts=4 tw=72:
