# The author of this work has dedicated it to the public by waiving all of his
# or her rights to the work under copyright law and all related or neighboring
# legal rights he or she had in the work, to the extent allowable by law.

include config.mk
.PHONY: all clean convert dist format install uninstall
.SUFFIXES: .in.1 .1 .1.html .1.txt

default: ii format
all: ii format convert
convert: ii.1.txt ii.1.html
format: ii.1

# Building ii(1):
ii: ii.o
	$(CC) -o $@ $@.o $(LDFLAGS)

# Formatting the manpages:
.in.1.1:
	mandoc -Tlint $<
	cp -f $< $@

# Converting the manpages:
.1.1.html:
	mandoc -Thtml -Wall -fstrict $< >$@
.1.1.txt:
	mandoc -Wall -fstrict $< | col -b >$@

dist: all
	mkdir ii-$(VERSION)
	cp Makefile CHANGES README.md LICENSE config.mk ii.c ii-$(VERSION)
	tar -czf ii-$(VERSION).tar.gz ii-$(VERSION)
	rm -rf ii-$(VERSION)

install: default
	mkdir -p $(DESTDIR)/$(DOCDIR)
	mkdir -p $(DESTDIR)/$(BINDIR)
	mkdir -p $(DESTDIR)/$(MAN1DIR)
	install -d $(DESTDIR)/$(BINDIR) $(DESTDIR)/$(MAN1DIR)
	install -m 644 CHANGES README.md LICENSE $(DESTDIR)/$(DOCDIR)
	install -m 775 ii $(DESTDIR)/$(BINDIR)
	install -m 444 ii.1 $(DESTDIR)/$(MAN1DIR)

uninstall:
	rm -f $(DESTDIR)/$(MAN1DIR)/ii.1
	rm -rf $(DESTDIR)/$(DOCDIR)
	rm -f $(DESTDIR)/$(BINDIR)/ii

clean:
	rm -f ii *~ *.o *.core *.tar.gz ii.1*
