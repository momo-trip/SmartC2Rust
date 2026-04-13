VERSION=	1.3.0

YANKCMD=	xsel

PREFIX=		/usr/local
MANPREFIX=	${PREFIX}/share/man

PROG=	yank
OBJS=	yank.o

KNFMT+=	yank.c

CPPCHECK+=	yank.c

CPPCHECKFLAGS+=	--quiet
CPPCHECKFLAGS+=	--check-level=exhaustive
CPPCHECKFLAGS+=	--enable=all
CPPCHECKFLAGS+=	--error-exitcode=1
CPPCHECKFLAGS+=	--max-configs=2
CPPCHECKFLAGS+=	--suppress-xml=cppcheck-suppressions.xml
CPPCHECKFLAGS+=	${CPPFLAGS}

IWYUFLAGS+=	${CPPFLAGS}

INSTALL_PROGRAM=	install -s -m 0755
INSTALL_MAN=		install -m 0644

CPPFLAGS+=	-DVERSION=\"${VERSION}\" -DYANKCMD=\"${YANKCMD}\"
CFLAGS+=	-pedantic -Wall -Wextra

DISTFILES=	CHANGELOG.md \
		LICENSE \
		Makefile \
		README.md \
		yank.1 \
		yank.c

all: ${PROG}

${PROG}: ${OBJS}
	${CC} -o ${PROG} ${OBJS} ${LDFLAGS}

clean:
	rm -f ${PROG} ${OBJS}
.PHONY: clean

dist:
	set -e; \
	d=yank-${VERSION}; \
	mkdir $$d; \
	for f in ${DISTFILES}; do \
		mkdir -p $$d/`dirname $$f`; \
		cp -p ${.CURDIR}/$$f $$d/$$f; \
	done; \
	find $$d -type d -exec touch -r ${.CURDIR}/Makefile {} \;; \
	tar czvf ${.CURDIR}/$$d.tar.gz $$d; \
	(cd ${.CURDIR}; sha256 $$d.tar.gz >$$d.sha256); \
	rm -r $$d
.PHONY: dist

format:
	cd ${.CURDIR} && knfmt -is ${KNFMT}
.PHONY: format

install: ${PROG}
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	${INSTALL_PROGRAM} ${PROG} ${DESTDIR}${PREFIX}/bin
	${INSTALL_MAN} yank.1 ${DESTDIR}${MANPREFIX}/man1
.PHONY: install

lint:
	cd ${.CURDIR} && mandoc -Tlint -Wstyle yank.1
	cd ${.CURDIR} && knfmt -ds ${KNFMT}
.PHONY: lint

lint-cppcheck:
	cd ${.CURDIR} && cppcheck ${CPPCHECKFLAGS} ${CPPCHECK}
.PHONY: lint-cppcheck

lint-include-what-you-use:
	cd ${.CURDIR} && iwyu-filter ${IWYUFLAGS} -- ${CPPCHECK}
.PHONY: lint-include-what-you-use

-include config.mk
