## Run automake on this file to produce Makefile.in

# FIG : Facility for Interactive Generation of figures
# Copyright (c) 1985-1988 by Supoj Sutanthavibul
# Parts Copyright (c) 1989-2015 by Brian V. Smith
# Parts Copyright (c) 1991 by Paul King
# Parts Copyright (c) 2016-2024 by Thomas Loimer
#
# Any party obtaining a copy of these files is granted, free of charge, a
# full and unrestricted irrevocable, world-wide, paid up, royalty-free,
# nonexclusive right and license to deal in this software and documentation
# files (the "Software"), including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense and/or sell copies
# of the Software, and to permit persons who receive copies from any such
# party to do so, with the only requirement being that the above copyright
# and this permission notice remain intact.
#
#
# Makefile.am
# Author: Thomas Loimer, 2016

# The default ARFLAGS are "cru". However, ar now outputs the warning
# "`u' modifier ignored since `D' is the default...".
ARFLAGS = cr

AM_CPPFLAGS = $(fontconfig_CFLAGS) $(XCPPFLAGS) -DNDEBUG	\
	-DXFIGLIBDIR="\"$(compkeydbdir)\""			\
	-DOBJLIBDIR="\"$(pkgdatadir)/Libraries\"" -DXFIGDOCDIR="\"$(docdir)\""

AM_LDFLAGS = $(XLDFLAGS)

# LIBOBJS must be placed in LDADD or xfig_LDADD - otherwise, the
# lib-dir is not distributed
# LIBOBJS may contain the replacement functions from AC_REPLACE_FUNCS.
xfig_LDADD = $(LIBOBJS) $(fontconfig_LIBS) $(XLIBS)

bin_PROGRAMS = xfig

xfig_files = d_arcbox.c d_arcbox.h d_arc.c d_arc.h d_box.c d_box.h \
	d_ellipse.c d_ellipse.h d_line.c d_line.h d_picobj.c d_picobj.h \
	d_regpoly.c d_regpoly.h d_spline.c d_spline.h d_subspline.c \
	d_subspline.h d_text.c d_text.h dirstruct.h e_addpt.c e_addpt.h \
	e_align.c e_align.h e_arrow.c e_arrow.h e_break.c e_break.h e_chop.c \
	e_chop.h e_compound.c e_compound.h e_convert.c e_convert.h e_copy.c \
	e_copy.h e_delete.c e_delete.h e_deletept.c e_deletept.h e_edit.c \
	e_edit.h e_flip.c e_flip.h e_glue.c e_glue.h e_joinsplit.c \
	e_joinsplit.h e_measure.c e_measure.h e_move.c e_move.h e_movept.c \
	e_movept.h e_placelib.c e_placelib.h e_rotate.c e_rotate.h e_scale.c \
	e_scale.h e_tangent.c e_tangent.h e_update.c e_update.h figx.h \
	f_load.c f_load.h f_neuclrtab.c f_neuclrtab.h f_picobj.c f_picobj.h \
	f_read.c f_readeps.c f_readgif.c f_read.h f_readold.c f_readpcx.c \
	f_readpcx.h f_readppm.c f_readxbm.c f_save.c f_save.h f_util.c \
	f_util.h f_wrpcx.c main.h mode.c mode.h object.c object.h \
	paintop.h resources.c resources.h u_bound.c u_bound.h u_colors.c \
	u_colors.h u_convert.c u_convert.h u_create.c u_create.h \
	u_drag.c u_drag.h u_draw.c \
	u_draw.h u_elastic.c u_elastic.h u_error.c u_error.h u_fonts.c \
	u_fonts.h u_free.c u_free.h u_geom.c u_geom.h u_ghostscript.c u_list.c \
	u_list.h u_markers.c u_markers.h u_pan.c u_pan.h u_print.c u_print.h \
	u_quartic.c u_quartic.h u_redraw.c u_redraw.h u_scale.c u_scale.h \
	u_search.c u_search.h u_smartsearch.c u_smartsearch.h \
	u_spawn.c u_spawn.h u_translate.c \
	u_translate.h u_undo.c u_undo.h w_browse.c w_browse.h w_canvas.c \
	w_canvas.h w_capture.c w_capture.h w_cmdpanel.c w_cmdpanel.h w_color.c \
	w_color.h w_cursor.c w_cursor.h w_digitize.c w_digitize.h w_dir.c \
	w_dir.h w_drawprim.c w_drawprim.h w_export.c w_export.h w_file.c \
	w_file.h w_fontbits.c w_fontbits.h w_fontpanel.c w_fontpanel.h \
	w_grid.c w_grid.h w_help.c w_help.h w_icons.c w_icons.h w_indpanel.c \
	w_indpanel.h w_intersect.c w_intersect.h w_keyboard.c w_keyboard.h \
	w_layers.c w_layers.h w_library.c w_library.h w_listwidget.c \
	w_listwidget.h w_modepanel.c w_modepanel.h \
	w_mousefun.c w_mousefun.h w_msgpanel.c w_msgpanel.h w_print.c \
	w_print.h w_rottext.c w_rottext.h w_rulers.c w_rulers.h w_setup.c \
	w_setup.h w_snap.c w_snap.h w_srchrepl.c w_srchrepl.h w_style.c \
	w_style.h w_util.c w_util.h w_zoom.c w_zoom.h splash.xbm version.xbm \
	xfig_math.h w_i18n.c

if HAVE_JPEG
xfig_files += f_readjpg.c
endif

if HAVE_PNG
xfig_files += f_readpng.c f_wrpng.c
endif

if HAVE_TIFF
xfig_files += f_readtif.c
endif

if USE_XPM
xfig_files += f_readxpm.c
endif

if USE_SPLASH
xfig_files += splash.xpm
endif

xfig_SOURCES = $(xfig_files) main.c
# fallback.c is a generated file, do not distribute it
nodist_xfig_SOURCES = fallback.c

EXTRA_DIST = u_draw_spline.c

# Create a convenience library for the unit tests.
# Unit tests provide a main() function and are linked to this convenience
# library. Replace the original main() for the xfig binary with xfigmain().
xfigmain.c: main.c
	$(SED) '/^main(/ s/main/xfigmain/' "$(srcdir)/main.c" >$@

check_LIBRARIES = libxfig.a
nodist_libxfig_a_SOURCES = $(nodist_xfig_SOURCES)
libxfig_a_SOURCES = $(xfig_files) xfigmain.c
libxfig_a_LIBADD = $(LIBOBJS)

# These files depend on PACKAGE_VERSION, which is
# defined in config.h ( = $(CONFIG_HEADER))
f_save.$(OBJEXT) main.$(OBJEXT) u_error.$(OBJEXT): $(CONFIG_HEADER)

if USE_SPLASH
w_icons.$(OBJEXT): version.xbm splash.xbm splash.xpm
else
w_icons.$(OBJEXT): version.xbm splash.xbm
endif

MOSTLYCLEANFILES = fallback.c
DISTCLEANFILES = splash.xbm splash.xpm version.xbm
MAINTAINERCLEANFILES = Makefile.in

# pgmtopbm might fail and produce an all-white image, see
# https://sourceforge.net/p/mcj/tickets/70
# The -threshold option to pgmtopbm should fix this issue. Nevertheless, check
# and try again (the line with $(FGREP)), using ImageMagick's convert-program.
version.xbm: version.fig.in $(top_srcdir)/version.m4
	-fig2dev -L ppm version.fig | ppmtopgm | pgmtopbm -threshold | pnmcrop \
	| pbmtoxbm | $(SED) '1,3 s/noname_/letters_/; 3 s/char/unsigned &/' >$@
	$(FGREP) -q 0xff $@ || fig2dev -L ppm version.fig | \
		convert ppm:- -trim xbm:- | \
		$(SED) '1,3 s/-_/letters_/; 3 s/char/unsigned &/' >$@

splash.xpm: $(top_srcdir)/splash-image/spl_bckgnd.fig.in \
		$(top_srcdir)/version.m4
	fig2dev -L xpm ../splash-image/spl_bckgnd.fig \
	| $(SED) '2 {s/static //; s/noname/spl_bckgnd_xpm/;}' > $@

splash.xbm: $(top_srcdir)/splash-image/spl_bckgnd.fig.in \
		$(top_srcdir)/version.m4
	fig2dev -L ppm -S 4 ../splash-image/spl_bckgnd.fig \
	| ppmtopgm | pgmtopbm -d8 | pbmtoxbm \
	| $(SED) '1,3 s/noname/spl_bckgnd/; 3 s/char/unsigned &/' > $@

# The sed script below transforms a X resource file to an array of strings.
# Note on quoting: to match one backslash, sed '/\\/' but sed "/\\\\/"
# sed on macOS needs a literal newline, not \n in the replacement string.
# To embed a newline, see info autoconf > Portable Make > Newlines in Make Rules
# The script is therefore split into a single-quoted and a double-quoted part.
# Comments to the script:
#	# remove comment lines and blank lines
#	# compress any whitespace (= space or tab) to single blank
#	# remove blank at beginning or end of line;
#	# colon: remove whitespace before colon, put one tab after colon
#	#	 except if followed by newline or \n; blank before \
#	# \n must become \\n in C string, suppress a blank before
#	# concatenate lines ending with \, until one without \
#		# accumulate lines
#		# restart cycle for lines ending with \
#		# exchange hold and pattern space on the last matched line
#		# get rid of the newline starting now the pattern space
#	# backslash - newline becomes newline - tab - tab
#	# but add a blank after #augment, #override or #replace
#	# and surround the text with quotes, writing C strings
fallbackscript1 = '				\
	/^[[:blank:]]*!/ d;			\
	/^[[:blank:]]*$$/ d;			\
	s/[[:blank:]][[:blank:]]*/ /g;		\
	s/^ //;					\
	s/ $$//;				\
	/:/ {	s/[ ]*:/:/;			\
		s/:[ ]*$$/:/;			\
		s/:[ ]*\\n/:\\n/;		\
		s/:[ ]*\\/: \\/;		\
		s/: /:	/;	};		\
	/ *\\n/ s//\\\\n/;			\
	/\\$$/,/[^\]$$/ {			\
		H;				\
		/\\$$/ d;			\
		s/.*//;				\
		x;				\
		s/^\n//;	}'
fallbackscript2 = "					\
	/\\\\\n/ {					\
		s/ *\\\\\n/\"\\$${nl}		\"/g;	\
		/augment\"/ s//augment \"/;		\
		/override\"/ s//override \"/;	\
		/replace\"/ s//replace \"/; };	\
	s/^/	\"/;					\
	s/$$/\",/"

fallback.c: $(top_srcdir)/app-defaults/Fig
	echo "/* generated by make from $(top_srcdir)/app-defaults/Fig */" >$@
	echo "#include <stddef.h>" >>$@
	echo "#include <X11/Intrinsic.h>" >>$@
	echo "String	fallback_resources[] = {" >>$@
	nlinit=`echo 'nl="'; echo '"'`; eval "$$nlinit";	\
	$(SED) -e $(fallbackscript1) -e $(fallbackscript2)	\
	$(top_srcdir)/app-defaults/Fig >>$@
	echo "	NULL" >>$@
	echo "};" >>$@
