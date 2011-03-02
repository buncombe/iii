# ii - irc it - simple but flexible IRC client
#   (C)opyright MMV-MMVI Anselm R. Garbe
#   (C)opyright MMV-MMVII Anselm R. Garbe, Nico Golde

include config.mk

# Configuration:
.SUFFIXES: .in.1 .1 .1.html .1.txt
PROJECT	= ii
DOCSRC	= doc/ # Should end with a slash.
CSRCS	= ii.c wrapper.c
MANSRCS	= ${DOCSRC}ii.in.1 ${DOCSRC}wrapper.in.1

all: options ${CSRCS:.c=} ${MANSRCS:.in.1=.1}
	@echo Built the sources and formatted the manpages.

options:
	@echo Build options:
	@echo "LIBS     = ${LIBS}"
	@echo "INCLUDES = ${INCLUDES}"
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

# Building the sources:
.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

.o:
	@echo LD $<
	@${CC} -o $@ $< ${LDFLAGS}

${CSRCS:.c=}: $@.o $@

# Formatting the manpages:
.in.1.1:
	@echo MDOC TERM $<
	@mandoc -Tlint $<
	@cp -f $< $@

.1.1.html:
	@echo MDOC HTML $<
	@mandoc -Thtml -Wall -fstrict $< >$@

.1.1.txt:
	@echo MDOC TXT $<
	@mandoc -Wall -fstrict $< | col -b >$@

format: ${MANSRCS:.in.1=.1.txt} ${MANSRCS:.in.1=.1.html}
	@echo Created TXT and HTML versions of the manpages.

dist: all format
	@mkdir -p ${PROJECT}-${VERSION}/${DOCSRC}
	@cp -R query.sh Makefile CHANGES README.md LICENSE config.mk ${CSRCS} \
	    ${PROJECT}-${VERSION}
	@cp -R ${DOCSRC}FAQ.md ${MANSRCS} ${MANSRCS:.in.1=.1.txt} \
	    ${MANSRCS:.in.1=.1.html} ${MANSRCS:.in.1=.1} \
	    ${PROJECT}-${VERSION}/${DOCSRC}
	@cp -R patches/ ${PROJECT}-${VERSION}/patches
	@tar -cf ${PROJECT}-${VERSION}.tar ${PROJECT}-${VERSION}
	@gzip ${PROJECT}-${VERSION}.tar
	@rm -rf ${PROJECT}-${VERSION}
	@echo Created distribution ${PROJECT}-${VERSION}.tar.gz.

install: all
	@mkdir -p ${DESTDIR}/${DOCDIR}
	@mkdir -p ${DESTDIR}/${BINDIR}
	@mkdir -p ${DESTDIR}/${MAN1DIR}

	@install -d ${DESTDIR}/${BINDIR} ${DESTDIR}/${MAN1DIR}
	@install -m 644 CHANGES README.md query.sh ${DOCSRC}FAQ.md LICENSE \
	    ${DESTDIR}/${DOCDIR}
	@install -m 775 ${CSRCS:.c=} ${DESTDIR}/${BINDIR}
	@install -m 444 ${MANSRCS:.in.1=.1} ${DESTDIR}/${MAN1DIR}
	@echo Installed everything.

uninstall: all
.for mansrc in ${MANSRCS:.in.1=.1}
	@rm -f ${DESTDIR}/${MAN1DIR}/`basename ${mansrc}`
.endfor
	@rm -rf ${DESTDIR}/${DOCDIR}
.for exec in ${CSRCS:.c=}
	@rm -f ${DESTDIR}/${BINDIR}/${exec}
.endfor
	@echo Uninstalled everything.

clean:
	rm -f ${CSRCS:.c=} */*~ *.o *core *.tar.gz ${MANSRCS:.in.1=.1} \
	    ${MANSRCS:.in.1=.1.txt} ${MANSRCS:.in.1=.1.html}
