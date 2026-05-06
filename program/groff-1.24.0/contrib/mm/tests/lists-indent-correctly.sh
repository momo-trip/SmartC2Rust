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

# Unit-test list indentation.

input='.SA 0
.P
This is an
.I mm
document.
Sed ut perspiciatis, unde omnis xxx iste natus error sit voluptatem
accusantium doloremque.
.P 1
This is an indented paragraph.
.AL
.LI
a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15 a16 a17 a18 a19 a20
a21 a22 a23 a24 a25 a26 a27 a28 a29 a30
.LE
.BL
.LI
b1 b2 b3 b4 b5 b6 b7 b8 b9 b10 b11 b12 b13 b14 b15 b16 b17 b18 b19 b20
b21 b22 b23 b24 b25 b26 b27 b28 b29 b30
.LE
.DL
.LI
c1 c2 c3 c4 c5 c6 c7 c8 c9 c10 c11 c12 c13 c14 c15 c16 c17 c18 c19 c20
c21 c22 c23 c24 c25 c26 c27 c28 c29 c30
.LE
.ML !
.LI
d1 d2 d3 d4 d5 d6 d7 d8 d9 d10 d11 d12 d13 d14 d15 d16 d17 d18 d19 d20
d21 d22 d23 d24 d25 d26 d27 d28 d29 d30
.LE
.ML @ \\n(Pi
.LI
e1 e2 e3 e4 e5 e6 e7 e8 e9 e10 e11 e12 e13 e14 e15 e16 e17 e18 e19 e20
e21 e22 e23 e24 e25 e26 e27 e28 e29 e30
.LE
.RL
.LI
f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 f16 f17 f18 f19 f20
f21 f22 f23 f24 f25 f26 f27 f28 f29 f30
.LE
.VL \\n(Pi
.LI tag
g1 g2 g3 g4 g5 g6 g7 g8 g9 g10 g11 g12 g13 g14 g15 g16 g17 g18 g19 g20
g21 g22 g23 g24 g25 g26 g27 g28 g29 g30
.LE
.P 1
This is another indented paragraph.
Now we will see how these lists handle item prefixes
(inapplicable to
.B ML
and
.BR VL ).
.ec @
.if @n[.mgm] @{@
.ec
.I "groff mm"
warns us of an overlong mark in the final case,
prompting us to consider increasing the value of the
.B Li
register.
.ec @
.@}
.ec
.AL
.LI * 1
h1 h2 h3 h4 h5 h6 h7 h8 h9 h10 h11 h12 h13 h14 h15 h16 h17 h18 h19 h20
h21 h22 h23 h24 h25 h26 h27 h28 h29 h30
.LE
.BL
.LI * 1
i1 i2 i3 i4 i5 i6 i7 i8 i9 i10 i11 i12 i13 i14 i15 i16 i17 i18 i19 i20
i21 i22 i23 i24 i25 i26 i27 i28 i29 i30
.LE
.DL
.LI * 1
j1 j2 j3 j4 j5 j6 j7 j8 j9 j10 j11 j12 j13 j14 j15 j16 j17 j18 j19 j20
j21 j22 j23 j24 j25 j26 j27 j28 j29 j30
.LE
.RL
.LI * 1
k1 k2 k3 k4 k5 k6 k7 k8 k9 k10 k11 k12 k13 k14 k15 k16 k17 k18 k19 k20
k21 k22 k23 k24 k25 k26 k27 k28 k29 k30
.LE
.ec @
.if @n[.mgm] @{@
.ec
.P 1
In
.I "groff mm,"
we can eliminate the padding between the prefix and the mark.
Thus we can make the
.B RL
reference list mark fit again.
.AL
.LI * 0
l1 l2 l3 l4 l5 l6 l7 l8 l9 l10 l11 l12 l13 l14 l15 l16 l17 l18 l19 l20
l21 l22 l23 l24 l25 l26 l27 l28 l29 l30
.LE
.BL
.LI * 0
m1 m2 m3 m4 m5 m6 m7 m8 m9 m10 m11 m12 m13 m14 m15 m16 m17 m18 m19 m20
m21 m22 m23 m24 m25 m26 m27 m28 m29 m30
.LE
.DL
.LI * 0
n1 n2 n3 n4 n5 n6 n7 n8 n9 n10 n11 n12 n13 n14 n15 n16 n17 n18 n19 n20
n21 n22 n23 n24 n25 n26 n27 n28 n29 n30
.LE
.RL
.LI * 0
o1 o2 o3 o4 o5 o6 o7 o8 o9 o10 o11 o12 o13 o14 o15 o16 o17 o18 o19 o20
o21 o22 o23 o24 o25 o26 o27 o28 o29 o30
.LE
.ec @
.@}
.ec'

#
#
#
#                                   - 1 -
#
#
#
#       This is an mm document.  Sed ut perspiciatis, unde omnis xxx
#       iste natus error sit voluptatem accusantium doloremque.
#
#            This is an indented paragraph.
#
#         1. a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15 a16
#            a17 a18 a19 a20 a21 a22 a23 a24 a25 a26 a27 a28 a29 a30
#
#          * b1 b2 b3 b4 b5 b6 b7 b8 b9 b10 b11 b12 b13 b14 b15 b16
#            b17 b18 b19 b20 b21 b22 b23 b24 b25 b26 b27 b28 b29 b30
#
#         -- c1 c2 c3 c4 c5 c6 c7 c8 c9 c10 c11 c12 c13 c14 c15 c16
#            c17 c18 c19 c20 c21 c22 c23 c24 c25 c26 c27 c28 c29 c30
#
#       ! d1 d2 d3 d4 d5 d6 d7 d8 d9 d10 d11 d12 d13 d14 d15 d16 d17
#         d18 d19 d20 d21 d22 d23 d24 d25 d26 d27 d28 d29 d30
#
#          @ e1 e2 e3 e4 e5 e6 e7 e8 e9 e10 e11 e12 e13 e14 e15 e16
#            e17 e18 e19 e20 e21 e22 e23 e24 e25 e26 e27 e28 e29 e30
#
#        [1] f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 f16
#            f17 f18 f19 f20 f21 f22 f23 f24 f25 f26 f27 f28 f29 f30
#
#       tag  g1 g2 g3 g4 g5 g6 g7 g8 g9 g10 g11 g12 g13 g14 g15 g16
#            g17 g18 g19 g20 g21 g22 g23 g24 g25 g26 g27 g28 g29 g30
#
#            This is another indented paragraph.  Now we will see
#       how these lists handle item prefixes (inapplicable to ML and
#       VL).  groff mm warns us of an overlong mark in the final
#       case, prompting us to consider increasing the value of the
#       Li register.
#
#       * 1. h1 h2 h3 h4 h5 h6 h7 h8 h9 h10 h11 h12 h13 h14 h15 h16
#            h17 h18 h19 h20 h21 h22 h23 h24 h25 h26 h27 h28 h29 h30
#
#        * * i1 i2 i3 i4 i5 i6 i7 i8 i9 i10 i11 i12 i13 i14 i15 i16
#            i17 i18 i19 i20 i21 i22 i23 i24 i25 i26 i27 i28 i29 i30
#
#       * -- j1 j2 j3 j4 j5 j6 j7 j8 j9 j10 j11 j12 j13 j14 j15 j16
#            j17 j18 j19 j20 j21 j22 j23 j24 j25 j26 j27 j28 j29 j30
#
#       * [1] k1 k2 k3 k4 k5 k6 k7 k8 k9 k10 k11 k12 k13 k14 k15 k16
#            k17 k18 k19 k20 k21 k22 k23 k24 k25 k26 k27 k28 k29 k30
#
#            In groff mm, we can eliminate the padding between the
#       prefix and the mark.  Thus we can make the RL reference list
#       mark fit again.
#
#        *1. l1 l2 l3 l4 l5 l6 l7 l8 l9 l10 l11 l12 l13 l14 l15 l16
#            l17 l18 l19 l20 l21 l22 l23 l24 l25 l26 l27 l28 l29 l30
#
#         ** m1 m2 m3 m4 m5 m6 m7 m8 m9 m10 m11 m12 m13 m14 m15 m16
#            m17 m18 m19 m20 m21 m22 m23 m24 m25 m26 m27 m28 m29 m30
#
#
#
#
#
#
#
#
#
#                                   - 2 -
#
#
#
#        *-- n1 n2 n3 n4 n5 n6 n7 n8 n9 n10 n11 n12 n13 n14 n15 n16
#            n17 n18 n19 n20 n21 n22 n23 n24 n25 n26 n27 n28 n29 n30
#
#       *[1] o1 o2 o3 o4 o5 o6 o7 o8 o9 o10 o11 o12 o13 o14 o15 o16
#            o17 o18 o19 o20 o21 o22 o23 o24 o25 o26 o27 o28 o29 o30

output=$(printf "%s\n" "$input" \
    | "$groff" -mm -Tascii -P-cbou 2> /dev/null)
echo "$output"

echo "checking indentation of AL list, first line" >&2
echo "$output" | grep -Eq "^ {9}1\. a1" || wail

echo "checking indentation of AL list, second line" >&2
echo "$output" | grep -Eq "^ {12}a17" || wail

echo "checking indentation of BL list, first line" >&2
echo "$output" | grep -Eq "^ {10}\* b1" || wail

echo "checking indentation of BL list, second line" >&2
echo "$output" | grep -Eq "^ {12}b17" || wail

echo "checking indentation of DL list, first line" >&2
echo "$output" | grep -Eq "^ {9}-- c1" || wail

echo "checking indentation of DL list, second line" >&2
echo "$output" | grep -Eq "^ {12}c17" || wail

echo "checking indentation of ML list, first line" >&2
echo "$output" | grep -Eq "^ {7}! d1" || wail

echo "checking indentation of ML list, second line" >&2
echo "$output" | grep -Eq "^ {9}d18" || wail

echo "checking indentation of ML list with text indent, first line" >&2
echo "$output" | grep -Eq "^ {10}@ {1}e1" || wail

echo "checking indentation of ML list with text indent, second line" >&2
echo "$output" | grep -Eq "^ {12}e17" || wail

echo "checking indentation of RL list, first line" >&2
echo "$output" | grep -Eq "^ {8}\[1] f1" || wail

echo "checking indentation of RL list, second line" >&2
echo "$output" | grep -Eq "^ {12}f17" || wail

echo "checking indentation of VL list, first line" >&2
echo "$output" | grep -Eq "^ {7}tag {2}g1" || wail

echo "checking indentation of VL list, second line" >&2
echo "$output" | grep -Eq "^ {12}g17" || wail

echo "checking indentation of prefixed AL list, first line" >&2
echo "$output" | grep -Eq "^ {7}\* 1\. h1" || wail

echo "checking indentation of prefixed AL list, second line" >&2
echo "$output" | grep -Eq "^ {12}h17" || wail

echo "checking indentation of prefixed BL list, first line" >&2
echo "$output" | grep -Eq "^ {8}\* \* i1" || wail

echo "checking indentation of prefixed BL list, second line" >&2
echo "$output" | grep -Eq "^ {12}i17" || wail

echo "checking indentation of prefixed DL list, first line" >&2
echo "$output" | grep -Eq "^ {7}\* -- j1" || wail

echo "checking indentation of prefixed DL list, second line" >&2
echo "$output" | grep -Eq "^ {12}j17" || wail

echo "checking indentation of prefixed RL list, first line" >&2
echo "$output" | grep -Eq "^ {7}\* \[1] k1" || wail

echo "checking indentation of prefixed RL list, second line" >&2
echo "$output" | grep -Eq "^ {12}k17" || wail

echo "checking indentation of padless prefixed AL list, first line" >&2
echo "$output" | grep -Eq "^ {8}\*1\. l1" || wail

echo "checking indentation of padless prefixed AL list, second line" >&2
echo "$output" | grep -Eq "^ {12}l17" || wail

echo "checking indentation of padless prefixed BL list, first line" >&2
echo "$output" | grep -Eq "^ {9}\*\* m1" || wail

echo "checking indentation of padless prefixed BL list, second line" >&2
echo "$output" | grep -Eq "^ {12}m17" || wail

echo "checking indentation of padless prefixed DL list, first line" >&2
echo "$output" | grep -Eq "^ {8}\*-- n1" || wail

echo "checking indentation of padless prefixed DL list, second line" >&2
echo "$output" | grep -Eq "^ {12}n17" || wail

echo "checking indentation of padless prefixed RL list, first line" >&2
echo "$output" | grep -Eq "^ {7}\*\[1] o1" || wail

echo "checking indentation of padless prefixed RL list, second line" >&2
echo "$output" | grep -Eq "^ {12}o17" || wail

error=$(printf "%s\n" "$input" | "$groff" -z -mm -Tascii -P-cbou 2>&1)
echo "$error"

echo "checking that input produced only 1 diagnostic message" >&2
test $(echo "$error" | wc -l) -eq 1 || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
