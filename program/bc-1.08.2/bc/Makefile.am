## Process this file with automake to produce Makefile.in
bin_PROGRAMS = bc

bc_SOURCES = main.c bc.y scan.l execute.c load.c storage.c util.c global.c \
	     warranty.c

EXTRA_DIST = bc.h bcdefs.h const.h global.h libmath.b libmath.h proto.h \
             sbc.y fix-libmath.sed
noinst_HEADERS = libmath.h

CLEANFILES = fbc-bootstrap global_bootstrap.o libmath.tmp

DISTCLEANFILES = sbc sbc.c sbc.h

MAINTAINERCLEANFILES = Makefile.in bc.c bc.h scan.c \
	bc.y bcdefs.h const.h execute.c \
	global.c global.h libmath.b libmath.h load.c main.c \
	proto.h scan.l storage.c util.c

AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/../h
LIBBC = ../lib/libbc.a
LIBL = @LEXLIB@
LDADD = $(LIBBC) $(LIBL) @READLINELIB@

AM_YFLAGS = -d

AM_CFLAGS = @CFLAGS@

$(PROGRAMS): libmath.h $(LIBBC)

scan.o: bc.h
global.o: libmath.h

fbcOBJ = main.o bc.o scan.o execute.o load.o storage.o util.o warranty.o
fbc-bootstrap: $(fbcOBJ) $(LIBBC)
	$(COMPILE) -DBOOTSTRAP -c -o global_bootstrap.o global.c
	$(LINK) -o $@ $(fbcOBJ) global_bootstrap.o $(LIBBC) $(LIBL) $(READLINELIB) $(LIBS)

libmath.h: libmath.b
	$(MAKE) fbc-bootstrap
	./fbc-bootstrap -c $(srcdir)/libmath.b </dev/null | \
		sed -f fix-libmath.sed \
		> libmath.tmp
	mv libmath.tmp libmath.h

sbcOBJ = main.o sbc.o scan.o execute.o global.o load.o storage.o util.o \
         warranty.o
sbc.o: sbc.c
sbc: $(sbcOBJ) $(LIBBC)
	$(LINK) $(sbcOBJ) $(LIBBC) $(LIBL) $(READLINELIB) $(LIBS)
