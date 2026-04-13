# This file is part of GNU cflow
# Copyright (C) 2005-2022 Free Software Foundation, Inc.
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

bin_PROGRAMS = cflow
cflow_SOURCES = \
 c.l\
 cflow.h\
 depmap.c\
 dot.c\
 gnu.c\
 linked-list.c\
 main.c\
 output.c\
 parser.c\
 parser.h\
 posix.c\
 symbol.c\
 wordsplit/wordsplit.c\
 wordsplit/wordsplit.h\
 parseopt/parseconf.h\
 parseopt/parseopt.c\
 parseopt/parseopt.h\
 parseopt/optset.c\
 parseopt/help.c\
 parseopt/wordwrap.c\
 parseopt/wordwrap.h

localedir = $(datadir)/locale

LDADD=../gnu/libgnu.a @LIBINTL@
AM_CPPFLAGS=\
 -I$(top_srcdir)/gnu -I../ -I../gnu\
 -I$(srcdir)/wordsplit\
 -I$(srcdir)/parseopt\
 -DLOCALEDIR=\"$(localedir)\"\
 -DPKGDATADIR=\"$(pkgdatadir)\"

AM_LFLAGS=-dvp
EXTRA_DIST = init/c11.cfo init/gcc.cfo

initdir=$(pkgdatadir)/$(VERSION)
init_DATA = init/c11.cfo init/gcc.cfo

CFLOW=$(abs_builddir)/cflow
CFLOW_FLAGS=-i^s --brief
cflow_CFLOW_INPUT=$(cflow_OBJECTS:.@OBJEXT@=.c)
cflow.cflow: $(cflow_CFLOW_INPUT) Makefile
	$(AM_V_GEN)\
	 $(CFLOW) -ocflow.cflow -P$(srcdir)/init --profile=gcc \
                  --cpp="$(CC) -E" $(CFLOW_FLAGS) $(DEFS) \
                  $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	          $(CPPFLAGS) \
         $(cflow_CFLOW_INPUT)
