# ii - irc it - simple but flexible IRC client
#   (C)opyright MMV-MMVI Anselm R. Garbe
#   (C)opyright MMV-MMVII Anselm R. Garbe, Nico Golde

include config.mk

SRC      = ii.c
OBJ      = ${SRC:.c=.o}

all: options ii wrapper
	@echo built ii and wrapper

options:
	@echo ii build options:
	@echo "LIBS     = ${LIBS}"
	@echo "INCLUDES = ${INCLUDES}"
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

dist: clean
	@mkdir -p ii-${VERSION}
	@cp -R query.sh Makefile CHANGES README FAQ LICENSE config.mk ii.c ii.1 wrapper.c ii-${VERSION}
	@tar -cf ii-${VERSION}.tar ii-${VERSION}
	@gzip ii-${VERSION}.tar
	@rm -rf ii-${VERSION}
	@echo created distribution ii-${VERSION}.tar.gz

ii: ${OBJ}
	@echo LD $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

install: all
	@mkdir -p ${DESTDIR}/${DOCDIR}
	@mkdir -p ${DESTDIR}/${BINDIR}
	@mkdir -p ${DESTDIR}/${MAN1DIR}

	@install -d ${DESTDIR}/${BINDIR} ${DESTDIR}/${MAN1DIR}
	@install -m 644 CHANGES README query.sh FAQ LICENSE ${DESTDIR}/${DOCDIR}
	@install -m 775 ii wrapper ${DESTDIR}/${BINDIR}
	@install -m 444 ii.1 ${DESTDIR}/${MAN1DIR}
	@echo "installed ii and wrapper"

uninstall: all
	@rm -f ${DESTDIR}/${MAN1DIR}/ii.1
	@rm -rf ${DESTDIR}/${DOCDIR}
	@rm -f ${DESTDIR}/${BINDIR}/ii
	@rm -f ${DESTDIR}/${BINDIR}/wrapper
	@echo "uninstalled ii and wrapper"

clean:
	rm -f ii wrapper *~ *.o *core *.tar.gz

wrapper: wrapper.o
	@echo LD $@
	@${CC} -o $@ $@.o ${LDFLAGS}
