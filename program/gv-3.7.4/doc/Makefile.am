# Makefile.am for gv/doc/
# 
# Time-stamp: "2004-12-19 20:16:44 jemarch"
#
# Copyright (C) 2004 José E. Marchesi

# File distribution

EXTRA_DIST= gv.1 gv-update-userconfig.1

info_TEXINFOS = gv.texi
gv_TEXINFOS = gpl.texi

man_MANS = gv.1 gv-update-userconfig.1

$(srcdir)/gv.1: $(top_srcdir)/src/versionp.h $(top_srcdir)/src/gv_message.h
	help2man -n 'Postscript and PDF viewer' -o $(srcdir)/gv.1 ../src/gv

# This hack allows building CVS without having to anable maintainer mode
$(srcdir)/version.texi: $(srcdir)/stamp-vti
