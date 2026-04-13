# This file is part of GNU cflow                           -*- autoconf -*-
# Copyright (C) 2005-2025 Free Software Foundation, Inc.
#
# Written by Sergey Poznyakoff
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.71])
AC_INIT([GNU cflow],[1.8],[bug-cflow@gnu.org],[cflow],
	[http://www.gnu.org/software/cflow])
AC_CONFIG_SRCDIR([src/cflow.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4 doc/imprimatur])
AM_INIT_AUTOMAKE([1.16.5 gnits tar-ustar dist-bzip2 dist-xz std-options silent-rules subdir-objects])
AC_CONFIG_HEADERS([config.h])

# Enable silent rules by default:
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
gl_EARLY
AC_PROG_LEX([noyywrap])
AC_PROG_RANLIB

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

gl_INIT
MU_DEBUG_MODE

### Check for Emacs site-lisp directory
AM_PATH_LISPDIR

# Gettext.
AM_ICONV
AM_GNU_GETTEXT([external], [need-formatstring-macros])
AM_GNU_GETTEXT_VERSION(0.19)

IMPRIMATUR_INIT(doc/imprimatur)

### Initialize the test suite.
AC_CONFIG_TESTDIR(tests)
AC_CONFIG_FILES([tests/Makefile tests/atlocal])
AM_MISSING_PROG([AUTOM4TE], [autom4te])

AC_CONFIG_FILES([
 Makefile
 gnu/Makefile
 src/Makefile
 elisp/Makefile
 po/Makefile.in
 doc/Makefile])
AC_OUTPUT
