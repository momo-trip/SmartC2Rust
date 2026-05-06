# Makefile.am for gv/src
#
# Copyright (C) 2004 JosÃ© E. Marchesi

# Process this file with automake to get Makefile.in

# File distribution
bin_PROGRAMS = gv
bin_SCRIPTS = gv-update-userconfig
pkgdata_DATA = gv_system.ad
if !ENABLE_FALLBACK_STYLES
pkgdata_DATA += gv_spartan.dat gv_widgetless.dat
endif

appdefaultsdir = $(pkgdatadir)

appdefaults_DATA = ../GV

EXTRA_DIST=ad2c gv_font_res.dat gv_font_res-I18N_mb.dat gv_layout_res.dat gv-i18n.pl gv_misc_res.dat Aaa_bison.yacc \
           gv_current.xbm gv_doc.xbm gv_empty.xbm gv_even.xbm gv_icon.xbm gv_odd.xbm \
           gv_selected.xbm gv_unmark.xbm $(srcdir)/nls/* \
	   gv_spartan.dat gv_widgetless.dat gv_user_res.dat gv_copyright.dat

# Clean files
CLEANFILES = ../GV \
             GV.ad \
             gv_system.ad \
             gv_user.ad \
             gv_spartan.h \
             gv_widgetless.h \
             gv_intern_res_unix.h \
             gv_class.ad \
             gv_class.h \
             gv_make_res.dat \
	     gv_intern_res_unix.dat \
             gv-update-userconfig
MAINTAINERCLEANFILES = gv-i18n.h

# Compilation flags
AM_CPPFLAGS = -DNON_BLOCKING_IO -DGV_CODE -DGV_LIBDIR=\"$(pkgdatadir)\" \
              -I$(top_srcdir)/lib -I$(top_builddir)/lib -g $(X_CFLAGS)

AM_CFLAGS = 

LDADD = ../lib/libgnu.a

gv_LDFLAGS = @X_LIBS@

gv_SOURCES = Aaa.c \
             Aaa_bison.c \
             Aaa_bison.h \
             Aaa_lex.c \
             Button.c \
             Button.h \
             ButtonP.h \
             Clip.c \
             Clip.h \
             ClipP.h \
             FileSel.c \
             FileSel.h \
             FileSelP.h \
             Frame.c \
             Frame.h \
             FrameP.h \
             Ghostview.c \
             Ghostview.h \
             GhostviewP.h \
             MButton.c \
             MButton.h \
             MButtonP.h \
             Switch.c \
             Switch.h \
             SwitchP.h \
             Vlist.c \
             Vlist.h \
             VlistP.h \
             actions.c \
             actions.h \
             callbacks.c \
             callbacks.h \
             confirm.c \
             confirm.h \
             config.h \
             dialog.c \
             dialog.h \
             doc_misc.c \
             doc_misc.h \
             error.c \
             error.h \
             file.c \
             file.h \
             info.c \
             info.h \
             magmenu.c \
             magmenu.h \
             main.c \
             main_globals.h \
             main_resources.h \
             media.c \
             media.h \
             message.h \
             misc.c \
             misc.h \
             miscmenu.c \
             miscmenu.h \
             misc_private.c \
             misc_private.h \
             note.c \
             note.h \
             options.c \
             options.h \
             options_fs.c \
             options_fs.h \
             options_gs.c \
             options_gs.h \
             options_gv.c \
             options_gv.h \
             options_setup.c \
             options_setup.h \
             paths.h \
             process.c \
             process.h \
             popup.c \
             popup.h \
             ps.c \
             ps.h \
             resource.c \
             resource.h \
             save.c \
             save.h \
             scale.c \
             scale.h \
             secscanf.c \
             secscanf.h \
             gv_signal.c \
             gv_signal.h \
             version.c \
             version.h \
             versionp.h \
             widgets_misc.c \
             widgets_misc.h \
             zoom.c \
             zoom.h \
             message.h \
             Aaa_intern.h \
             Aaa.h \
             AaaP.h \
             gv_message.h \
             types.h \
             gv-i18n.h \
             gv-update-userconfig.in

BUILT_SOURCES = gv_spartan.h gv_widgetless.h gv_intern_res_unix.h gv_class.h gv-i18n.h


# Additional rules for generating styles

../GV : GV.ad
	@echo "creating GV"
	sed -e 's/^[^!]/!&/' GV.ad > ../GV

GV.ad : gv_system.ad
	@echo "creating GV.ad"
	@cp gv_system.ad GV.ad

gv_system.ad : gv_user.ad
	@echo "creating gv_system.ad"
	@cp gv_user.ad gv_system.ad

gv_user.ad : $(srcdir)/gv_user_res.dat gv_intern_res_unix.dat gv_make_res.dat
	@echo "creating gv_user.ad"
	@echo "!"						>  gv_user.ad
	@echo "!  gv_user.ad"				>> gv_user.ad
	@echo "!  User specific application defaults for gv"	>> gv_user.ad
	@echo "!  Copyright (C) 1995, 1996, 1997  Johannes Plass"	>> gv_user.ad
	@echo "!  Copyright (C) 2004,2005,2006,2007 José E. Marchesi"	>> gv_user.ad
	@echo "!"						>> gv_user.ad
	@cat $(srcdir)/gv_user_res.dat					>> gv_user.ad
	@cat gv_intern_res_unix.dat				>> gv_user.ad
	@cat gv_make_res.dat					>> gv_user.ad



gv_spartan.h : $(srcdir)/gv_spartan.dat
	@sh $(srcdir)/ad2c $(srcdir)/gv_spartan.dat > gv_spartan.h

gv_widgetless.h : $(srcdir)/gv_widgetless.dat
	@sh $(srcdir)/ad2c $(srcdir)/gv_widgetless.dat > gv_widgetless.h

gv_intern_res_unix.h : gv_intern_res_unix.dat
	@echo "creating gv_intern_res_unix.h"
	@sh $(srcdir)/ad2c gv_intern_res_unix.dat > gv_intern_res_unix.h

gv_intern_res_unix.dat :
	@echo "creating gv_intern_res_unix.dat"
	@echo "" 							 >  gv_intern_res_unix.dat
	@echo "!########## gv_intern_res.dat (generated by makefile)" >> gv_intern_res_unix.dat
	@echo ""							>> gv_intern_res_unix.dat
	@echo "GV.gsInterpreter:	gs" >> gv_intern_res_unix.dat
	@echo "GV.gsCmdScanPDF:	gs -dNODISPLAY -dQUIET -sPDFname=%s -sDSCname=%s %s pdf2dsc.ps -c quit" >> gv_intern_res_unix.dat
	@echo "GV.gsCmdConvPDF:	gs -dNOPAUSE -dQUIET -dBATCH -sDEVICE=pswrite -sOutputFile=%s -f %s -c save pop quit" >> gv_intern_res_unix.dat
	@echo "GV.gsX11Device:		-sDEVICE=x11" >> gv_intern_res_unix.dat
	@echo "GV.gsX11AlphaDevice:	-sDEVICE=x11 -dTextAlphaBits=4 -dGraphicsAlphaBits=2 -dMaxBitmap=10000000 -dNOPLATFONTS" >> gv_intern_res_unix.dat
	@echo "GV.gsSafeDir:		True" >> gv_intern_res_unix.dat
	@echo "GV.gsSafer:		True" >> gv_intern_res_unix.dat
	@echo "GV.gsQuiet:		True" >> gv_intern_res_unix.dat
	@echo "GV.infoVerbose:		Warnings" >> gv_intern_res_unix.dat
	@echo "GV.xinerama:		Auto" >> gv_intern_res_unix.dat
	@echo "GV.gsArguments:		-dFIXEDMEDIA" >> gv_intern_res_unix.dat
	@echo "GV.uncompressCommand:	gzip -d -c %s > %s" >> gv_intern_res_unix.dat
	@echo "GV.printCommand:	$(PRINT_COMMAND)" >> gv_intern_res_unix.dat


gv_class.h : gv_class.ad
	@echo "creating gv_class.h"
	@sh $(srcdir)/ad2c gv_class.ad > gv_class.h

gv-i18n.h: $(srcdir)/nls/LANGUAGES $(srcdir)/nls/*.dat
	perl $(srcdir)/gv-i18n.pl $(srcdir) > gv-i18n.h || { rm gv-i18n.h ; exit 1 ; }

gv_class.ad : $(srcdir)/gv_font_res.dat $(srcdir)/gv_font_res-I18N_mb.dat $(srcdir)/gv_layout_res.dat gv_make_res.dat $(srcdir)/gv_misc_res.dat $(srcdir)/gv_user_res.dat gv_intern_res_unix.dat
	@echo "creating gv_class.ad"
	cp $(srcdir)/gv_copyright.dat gv_class.ad
	chmod u+w gv_class.ad
	@echo "!" >> gv_class.ad
	@echo "!  gv_class.ad" >> gv_class.ad
	@echo "!  Application class defaults for gv" >> gv_class.ad
	@echo "!  Copyright (C) 1995, 1996, 1997  Johannes Plass" >> gv_class.ad
	@echo "!  Copyright (C) 2004,2005,2006,2007 José E. Marchesi" >> gv_class.ad
	@echo "!" >> gv_class.ad
	@echo ""  >> gv_class.ad
	@cat $(srcdir)/gv_user_res.dat >> gv_class.ad
	@cat gv_intern_res_unix.dat >> gv_class.ad
	@cat gv_make_res.dat >> gv_class.ad
if ENABLE_INTERNATIONAL
	echo "GV*international: TRUE" >> gv_class.ad
endif
	@cat $(srcdir)/gv_font_res-I18N_mb.dat >> gv_class.ad
	@cat $(srcdir)/gv_font_res.dat >> gv_class.ad
	@cat $(srcdir)/gv_misc_res.dat >> gv_class.ad
	@cat $(srcdir)/gv_layout_res.dat >> gv_class.ad

sysdocdir = /usr/doc
gv_make_res.dat :
	@echo "creating gv_make_res.dat"
	@echo ""							>  gv_make_res.dat
	@echo  "!########## gv_make_res.dat (generated by makefile)"	>> gv_make_res.dat
	@echo ""							>> gv_make_res.dat
	@echo "GV.scratchDir:		$(SCRATCH_DIR)"			>> gv_make_res.dat
	@echo "GV.defaultSaveDir:	$(SAVE_DIR)"			>> gv_make_res.dat
	@echo "GV.fallbackPageMedia:	$(PAPERSIZE)"			>> gv_make_res.dat
	@echo "GV.useBackingPixmap:	$(USE_BACKING_PIXMAP)"		>> gv_make_res.dat
	@echo "GV*dirs:		Home\n\\"				>> gv_make_res.dat
	@echo "			Tmp\n\\"				>> gv_make_res.dat
	@echo "			$(sysdocdir)\n\\"			>> gv_make_res.dat
	@echo "			/usr/local/doc"				>> gv_make_res.dat
	@echo "GV*filter:		no .*"				>> gv_make_res.dat
	@echo "GV*filters:		None\n\\"			>> gv_make_res.dat
	@echo "			*.*ps* *.pdf*  no  .*\n\\"		>> gv_make_res.dat
	@echo "			*.*ps*   no  .*\n\\"			>> gv_make_res.dat
	@echo "			*.pdf*  no  .*\n\\"			>> gv_make_res.dat
	@echo "			no  .*"					>> gv_make_res.dat
	@echo ""							>> gv_make_res.dat

gv-update-userconfig: $(srcdir)/gv-update-userconfig.in GV.ad
	cat $(srcdir)/gv-update-userconfig.in > gv-update-userconfig
	echo "__END__" >> gv-update-userconfig
	cat GV.ad >> gv-update-userconfig
	cat $(srcdir)/gv_misc_res.dat >> gv-update-userconfig

# Conditional source files

if USE_SCROLLBAR_CODE
   gv_SOURCES += Scrollbar.c Scrollbar.h ScrollbarP.h
endif

if USE_SETENV_CODE
   gv_SOURCES += setenv.c setenv.h
   AM_CPPFLAGS += -DUSE_SETENV_CODE
endif

install-data-local:
	$(MKDIR_P) "$(DESTDIR)$(pkgdatadir)/safe-gs-workdir"
	chmod 755 "$(DESTDIR)$(pkgdatadir)/safe-gs-workdir"
