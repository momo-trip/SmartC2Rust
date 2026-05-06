#!/bin/sh
#
# Copyright 2019-2020 G. Branden Robinson
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

# https://lists.gnu.org/archive/html/bug-bash/2017-02/msg00024.html

expected="' = '"
actual=$(printf '.pl 1v\n\\[oq] = '"'"'\n' | "$groff" -Tlatin1)
test "$actual" = "$expected"
