#!/usr/bin/env perl
#
# Copyright (C) 1991-2024 Free Software Foundation, Inc.
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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Print the modification date of $1 in ISO 8601 format.

use warnings;
use strict;
use POSIX qw(LC_ALL setlocale strftime);

# Don't want localized dates.
setlocale(LC_ALL, "C");

my @mtime = gmtime($ENV{SOURCE_DATE_EPOCH} || (stat $ARGV[0])[9]);
my $mdate = strftime("%Y-%m-%d", @mtime);
$mdate =~ s/^ //;
print "$mdate\n";

# Local Variables:
# fill-column: 72
# mode: CPerl
# End:
# vim: set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
