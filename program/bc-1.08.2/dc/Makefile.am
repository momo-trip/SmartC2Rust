## Process this file with automake to produce Makefile.in
bin_PROGRAMS = dc

dc_SOURCES = dc.c input.c misc.c eval.c stack.c array.c numeric.c string.c
noinst_HEADERS = dc.h dc-proto.h dc-regdef.h

LIBBC = ../lib/libbc.a
LDADD = $(LIBBC) @READLINELIB@

MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = @CFLAGS@

$(PROGRAMS): $(LIBBC)
