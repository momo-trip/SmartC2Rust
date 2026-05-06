## Run automake on this file to produce Makefile.in

# FIG : Facility for Interactive Generation of figures
# Copyright (c) 1985-1988 by Supoj Sutanthavibul
# Parts Copyright (c) 1989-2015 by Brian V. Smith
# Parts Copyright (c) 1991 by Paul King
# Parts Copyright (c) 2016-2023 by Thomas Loimer
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
# doc/Makefile.am
# Author: Thomas Loimer

dist_doc_DATA = FORMAT1.3 FORMAT1.4 FORMAT2.0 FORMAT2.1 FORMAT3.0 \
	FORMAT3.1 FORMAT3.2 xfig-howto.pdf xfig_ref_en.pdf

doc_DATA = xfig_man.html

man1_MANS = xfig.man

EXTRA_DIST = html MAKEPS

CLEANFILES = html/images/xfig-title.fig

MAINTAINERCLEANFILES = Makefile.in

.PHONY: documentation

# Manually generate $(srcdir)/xfig_ref_en.pdf, so users do not require htmldoc
documentation: $(top_srcdir)/version.m4
	fig2dev -L png -S4 html/images/xfig-title.fig			\
			$(srcdir)/html/images/xfig-title.png
	cd $(srcdir)/html && htmldoc -f ../xfig_ref_en.pdf --no-title	\
		--webpage --header ..c -t pdf14 --size a4 contents.html	\
		$$($(FGREP) '<a href=' contents.html |			\
			$(SED) 's/.*a href="//; s/["\#].*//;' | uniq |	\
			$(FGREP) -v japanese)

# To create xfig-title.png, probably test:
#   { test -r html/images/xfig-title.fig ||
#      $(MAKE) -C .. doc/html/images/xfig-title.fig; } &&
#   fig2dev -L png -S4 #    html/images/xfig-title.fig "$@"

# man is sensitive to spaces after the options.
# Probably, groff -Thtml -mman xfig.man > $@ is more portable?

# This file remains in the src tree and is updated if necessary.
$(srcdir)/xfig_man.html.in: $(srcdir)/xfig.man.in
	man -Thtml -l "$(srcdir)/xfig.man.in" >"$@"

install-data-hook:
	cp -Rp "$(srcdir)/html" "$(DESTDIR)$(htmldir)"		\
		&& chmod -R u+rwX "$(DESTDIR)$(htmldir)"
	cd "$(DESTDIR)$(htmldir)/html"				\
		&& rm -f ja_JP.ujis && $(LN_S) japanese ja_JP.ujis
	cd "$(DESTDIR)$(htmldir)/html/images"			\
		&& rm -f xfig-title.fig xfig-title.fig.in

uninstall-local:
	rm -rf "$(DESTDIR)$(htmldir)/html"
