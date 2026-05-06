#!/bin/sh
#
# Copyright 2024-2025 G. Branden Robinson
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

for c in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z \
         a b c d e f g h i j k l m n o p q r s t u v w x y z \
         '!' '"' '#' '$' "'" ',' ';' '?' \
         '@' '[' ']' '^' '_' \
         '`' '{' '}' '~'
do
    echo "checking validity of '$c' as escape sequence delimiter" \
        "when not in compatbility mode" >&2
    output=$(printf '\\l%c1n+2n\\&0%c\n' "$c" "$c" \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Fqx 000 || wail
done

for octal in 001 002 003 004 005 006 007 010 011 014 177
do
    echo "checking validity of control character $octal (octal)" \
        "as escape sequence delimiter when not in compatibility mode" \
        >&2
    output=$(printf '\\l\'$octal'1n+2n\\&_\'$octal'\n' \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Fqx ___ || wail
done

for c in 0 1 2 3 4 5 6 7 8 9 + - '(' . '|'
do
    echo "checking invalidity of '$c' as escape sequence delimiter" \
        "when not in compatbility mode" >&2
    output=$(printf '\\l%c1n+2n\\&_%c\n' "$c" "$c" \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -qx 1n+2n_. || wail
done

for c in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z \
         a b c d e f g h i j k l m n o p q r s t u v w x y z \
         '!' '"' '#' '$' "'" ',' ';' '?' \
         '@' '[' ']' '^' '_' \
         '`' '{' '}' '~'
do
    echo "checking validity of '$c' as title request delimiter" \
        "when not in compatbility mode" >&2
    output=$(printf '.tl %c1%c2%c3%c\n' "$c" "$c" "$c" "$c" \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Eqx '1 +2 +3' || wail
done

for octal in 001 002 003 004 005 006 007 010 011 014 177
do
    echo "checking validity of control character $octal (octal)" \
        "as title request delimiter when not in compatibility mode" \
        >&2
    output=$(printf '.tl \'$octal'1\'$octal'2\'$octal'3\'$octal'\n' \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Eqx '1 +2 +3' || wail
done

# Test unparameterized and invalid escape sequences.  The latter degrade
# to ordinary characters.  See below regarding `\0`, `\^`, and `\|`.
for c in         E   G   I J K         P       T U   W       \
         a   c d e       i j           p q r   t u       y   \
                                 /     % '<' '>' = '&' :     ')' \
                         "'" ',' ';'     \
         '@'     ']' '^' '_' \
         '`' '{' '}' '~'
do
    echo 'checking validity of backslash-"'$c'"' \
        "as escape sequence delimiter when not in compatibility mode" \
        >&2
    output=$(printf '\\l%c1n+2n\\&0%c\n' "$c" "$c" \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Fqx 000 || wail
done

for c in 0 1 2 3 4 5 6 7 8 9 + - '(' . '|'
do
    echo "checking invalidity of '$c' as escape sequence delimiter" \
        "when not in compatbility mode" >&2
    output=$(printf '\\l%c1n+2n\\&_%c\n' "$c" "$c" \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -qx 1n+2n_. || wail
done

# TODO: Future: Savannah #66009
#for c in 0 1 2 3 4 5 6 7 8 9 + - '(' . '|'
#do
#    echo "checking invalidity of '$c' as title request delimiter" \
#        "when not in compatbility mode" >&2
#    output=$(printf '.tl %cA%cB%cC%c\n' "$c" "$c" "$c" "$c" \
#      | "$groff" -w delim -T ascii | sed '/^$/d')
#    echo "$output" | grep -Eqx 'A +B +C' && wail
#done

# excluded: conditional expression operators
for c in A B C D E   G H I J K L M N O P Q R   T U V W X Y Z \
         a b       f g h i j k l       p q   s   u   w x y z \
             '"' '#' '$' "'" ',' ';' '?' \
         '@' '[' ']' '^' '_' \
         '`' '{' '}' '~'
do
    echo "checking validity of '$c' as output comparison delimiter" \
         "when not in compatibility mode" >&2
    output=$(printf '.if %c***%c***%c ***\n' "$c" "$c" "$c" \
      | "$groff" -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Fqx '***' || wail
done

# Regression-test Savannah #67744.
echo "checking invalidity of \h as escape sequence delimiter" \
    "when not in compatibility mode" >&2
output=$(printf 'foo\\C\\h"1m"em\\h"1m"bar\n' | "$groff" -T ascii -a)
echo "$output" | grep -Fqx "foo--bar" && wail

# Check invalidity of \C as escape sequence delimiter when not in
# compatibility mode.  See Savannah #67842.
echo "checking invalidity of \C as escape sequence delimiter" \
    "to delimited escape sequence" >&2
output=$(printf 'foo\\h\\C"em"2m\\C"em"bar\n.pl \\n(nlu' \
    | "$groff" -T ascii)
echo "$output" | grep -Fqx "foo  bar" && wail # 2 spaces

# Now test the context-dependent sets of delimiters of AT&T troff.

# not tested: '_' (because it's part of our delimited expression)
for c in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z \
         a b c d e f g h i j k l m n o p q r s t u v w x y z \
         0 1 2 3 4 5 6 7 8 9 + - / '*' % '<' '>' = '&' : '(' ')' . '|' \
         '!' '"' '#' '$' "'" ',' ';' '?' \
         '@' '[' ']' '^' \
         '`' '{' '}' '~'
do
    echo "checking validity of '$c' as string expression delimiter" \
         "in compatibility mode" >&2
    output=$(printf '\\o%c__%c__\n' "$c" "$c" \
      | "$groff" -C -w delim -T ascii -P -cbou | sed '/^$/d')
    echo "$output" | grep -Fqx ___ || wail
done

# Check the leftover '_'.
echo "checking validity of '_' as string expression delimiter" \
    "in compatibility mode" >&2
output=$(printf '\\o_**_**\n' \
  | "$groff" -C -w delim -T ascii -P -cbou | sed '/^$/d')
echo "$output" | grep -Fqx '***' || wail

for octal in 002 003 005 006 007 177
do
    echo "checking validity of control character $octal (octal)" \
         "as string expression delimiter in compatibility mode" >&2
    output=$(printf '\\o\'$octal'__\'$octal'__\n' \
      | "$groff" -C -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -qx _.___ || wail
done

for c in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z \
         a b c d e f g h i j k l m n o p q r s t u v w x y z \
         0 1 2 3 4 5 6 7 8 9 . '|' \
         '!' '"' '#' '$' "'" ',' ';' '?' \
         '@' '[' ']' '^' '`' '{' '}' '~'
do
    echo "checking validity of '$c' as numeric expression delimiter" \
         "in compatibility mode" >&2
    output=$(printf '_\\h%c1n+2n%c_\n' "$c" "$c" \
      | "$groff" -C -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Fqx '_   _' || wail
done

for c in + - / '*' % '<' '>' = '&' : '(' ')'
do
    echo "checking invalidity of '$c' as numeric expression delimiter" \
         "in compatibility mode" >&2
    output=$(printf '_\\h%c1n+2n%c_\n' "$c" "$c" \
      | "$groff" -C -w delim -T ascii -P -cbou | sed '/^$/d')
    echo "$output" | grep -Fqx '_   _' && wail
done

for octal in 002 003 005 006 007 177
do
    echo "checking validity of control character $octal (octal)" \
         "as numeric expression delimiter in compatibility mode" >&2
    output=$(printf '\\o\'$octal'__\'$octal'__\n' \
      | "$groff" -C -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -qx _.___ || wail
done

# 'v' as a conditional expression operator is a vtroff extension, not a
# GNU one.  vtroff is also unobtainium in the 21st century.  DWB 3.3,
# Plan 9, and Solaris troffs don't treat 'v' specially.
#
# TODO: Ideally, we should permit only AT&T-compatible (plus 'v'?)
# operators when in compatibility mode.  (If you want the extensions,
# use the `do` request.)  But that will demand hitting
# `is_conditional_expression_true()` in "input.cpp" with a hammer, and
# announcement of a behavior change in the "NEWS" file.
#
# If/when we do, change the following loop as shown.  And decide whether
# to treat 'v' as an AT&Tism or a GNUism.
#for c in A B C D E F G H I J K L M N O P Q R S T U V W X Y Z \
#         a b c d   f g h i j k l m     p q r s   u v w x y z
for c in A B C D E   G H I J K L M N O P Q R   T U V W X Y Z \
         a b       f g h i j k l       p q   s   u   w x y z \
             '"' '#' '$' "'" ',' ';' '?' \
         '@' '[' ']' '^' '_' \
         '`' '{' '}' '~'
do
    echo "checking validity of '$c' as output comparison delimiter" \
         "in compatibility mode" >&2
    output=$(printf '.if %c***%c***%c ___\n' "$c" "$c" "$c" \
      | "$groff" -C -w delim -T ascii | sed '/^$/d')
    echo "$output" | grep -Fqx ___ || wail
done

for octal in 002 003 005 006 007 177
do
    echo "checking validity of control character $octal (octal)" \
         "as output comparison delimiter in compatibility mode" >&2
    output=$(printf '.if \'$octal'@@@\'$octal'@@@\'$octal' ___\n' \
      | "$groff" -C -w delim -T ascii -P -cbou | sed '/^$/d')
    echo "$output" | grep -Fqx ___ || wail
done

for c in e n o t \
         0 1 2 3 4 5 6 7 8 9 + - / '*' % '<' '>' = '&' : '(' ')' . '|' \
         '!'
do
    echo "checking invalidity of '$c' as output comparison delimiter" \
        "in compatibility mode" >&2
    output=$(printf '.if %c@@@%c@@@%c ___\n' "$c" "$c" "$c" \
        | "$groff" -C -w delim -T ascii -P -cbou | sed '/^$/d')
    echo "$output" | grep -Fqx ___ && wail
done

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
