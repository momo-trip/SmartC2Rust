#!/bin/sh
#
# Copyright 2024 G. Branden Robinson
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
    echo ...FAILED >&2
    fail=YES
}

input='.
.H 1 Opus
.H 2 "De Redrum Unnatura"
.nP
Sed ut perspiciatis,
unde omnis iste natus error sit voluptatem accusantium doloremque
laudantium,
totam rem aperiam eaque ipsa,
quae ab illo inventore veritatis et quasi architecto beatae vitae dicta
sunt,
explicabo.
.nP
Nemo enim ipsam voluptatem,
quia voluptas sit,
aspernatur aut odit aut fugit,
sed quia consequuntur magni dolores eos,
qui ratione voluptatem sequi nesciunt,
neque porro quisquam est,
qui dolorem ipsum,
quia dolor sit amet consectetur adipiscivelit,
sed quia non-numquam eius modi tempora incidunt,
ut labore et dolore magnam aliquam quaerat voluptatem.
.H 3 "Siegesbeckia orientalis"
.nP
Quis autem vel eum iure reprehenderit,
qui inea voluptate velit esse,
quam nihil molestiae consequatur,
vel illum,
qui dolorem eum fugiat,
quo voluptas nulla pariatur?
.H 2 "Eternalii famishiis"
.nP
At vero eos et accusamus et iusto odio dignissimos ducimus,
qui blanditiis praesentium voluptatum deleniti atque corrupti,
quos dolores et quas molestias excepturi sint,
obcaecati cupiditate non-provident,
similique sunt in culpa,
qui officia deserunt mollitia animi,
id est laborum et dolorum fuga.
.H 2 [redacted]
.H 2 [redacted]
.H 2 [redacted]
.H 2 [redacted]
.H 2 [redacted]
.H 2 [redacted]
.H 2 [redacted]
.H 2 "Malleus Maleficarum"
.nP
Ut enim ad minima veniam,
quis nostrum exercitationem ullam corporis suscipitlaboriosam,
nisi ut aliquid ex ea commodi consequatur?
.'

output=$(printf "%s\n" "$input" | "$groff" -mm -Tascii -P-cbou)
echo "$output"

# Expected output (consecutive blank lines condensed):
#
#                                    - 1 -
#
#        1.  Opus
#
#        1.1  De Redrum Unnatura
#
#        1.01  Sed  ut  perspiciatis, unde omnis iste natus error sit
#              voluptatem accusantium  doloremque  laudantium,  totam
#        rem  aperiam eaque ipsa, quae ab illo inventore veritatis et
#        quasi architecto beatae vitae dicta sunt, explicabo.
#
#        1.02  Nemo  enim  ipsam  voluptatem,  quia   voluptas   sit,
#              aspernatur  aut  odit aut fugit, sed quia consequuntur
#        magni dolores eos, qui ratione  voluptatem  sequi  nesciunt,
#        neque  porro quisquam est, qui dolorem ipsum, quia dolor sit
#        amet consectetur adipiscivelit, sed  quia  non-numquam  eius
#        modi  tempora  incidunt,  ut labore et dolore magnam aliquam
#        quaerat voluptatem.
#
#        1.1.1  Siegesbeckia orientalis
#
#        1.03  Quis  autem  vel  eum  iure  reprehenderit,  qui  inea
#              voluptate    velit    esse,   quam   nihil   molestiae
#        consequatur, vel illum, qui dolorem eum fugiat, quo voluptas
#        nulla pariatur?
#
#        1.2  Eternalii famishiis
#
#        2.01  At vero eos et accusamus  et  iusto  odio  dignissimos
#              ducimus,   qui   blanditiis   praesentium   voluptatum
#        deleniti atque corrupti,  quos  dolores  et  quas  molestias
#        excepturi    sint,   obcaecati   cupiditate   non-provident,
#        similique sunt  in  culpa,  qui  officia  deserunt  mollitia
#        animi, id est laborum et dolorum fuga.
#
#        1.3  [redacted]
#
#        1.4  [redacted]
#
#        1.5  [redacted]
#
#        1.6  [redacted]
#
#        1.7  [redacted]
#
#        1.8  [redacted]
#
#        1.9  [redacted]
#
#        1.10  Malleus Maleficarum
#
#        10.01  Ut enim ad minima veniam, quis nostrum exercitationem
#               ullam corporis suscipitlaboriosam, nisi ut aliquid ex
#        ea commodi consequatur?

echo "checking label of first paragraph" >&2
echo "$output" | grep -Eq '1\.01 {2}Sed *ut *perspiciatis' || wail

echo "checking indentation of first paragraph, second line" >&2
echo "$output" | grep -Eq '^ {13}voluptatem *accusantium' || wail

echo "checking label of second paragraph" >&2
echo "$output" | grep -Eq '1\.02 {2}Nemo *enim *ipsam' || wail

echo "checking indentation of second paragraph, second line" >&2
echo "$output" | grep -Eq '^ {13}aspernatur *aut *odit' || wail

echo "checking label of third paragraph" >&2
echo "$output" | grep -Eq '1\.03 {2}Quis *autem *vel' || wail

echo "checking indentation of third paragraph, second line" >&2
echo "$output" | grep -Eq '^ {13}voluptate *velit *esse' || wail

echo "checking label of fourth paragraph" >&2
echo "$output" | grep -Eq '2\.01 {2}At *vero *eos *et' || wail

echo "checking indentation of fourth paragraph, second line" >&2
echo "$output" | grep -Eq '^ {13}ducimus, *qui *blanditiis' || wail

echo "checking label of fifth paragraph" >&2
echo "$output" | grep -Eq '10\.01 {2}Ut *enim *ad *minima' || wail

echo "checking indentation of fifth paragraph, second line" >&2
echo "$output" | grep -Eq '^ {14}ullam *corporis' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
