# The author of this work has dedicated it to the public by waiving all of his
# or her rights to the work under copyright law and all related or neighboring
# legal rights he or she had in the work, to the extent allowable by law.

include config.mk
FORMAT = wrapper.1
CONVERT = wrapper.1.txt wrapper.1.html
.PHONY: all clean convert format install uninstall
.SUFFIXES: .in.1 .1 .1.html .1.txt

default: wrapper format
all: wrapper format convert
convert: $(CONVERT)
format: $(FORMAT)

# Building wrapper(1):
wrapper: wrapper.o val.o
	$(CC) -o $@ $@.o val.o $(LDFLAGS)
wrapper.o val.o: val.h

# Formatting the manpages:
.in.1.1:
	mandoc -Tlint $<
	cp -f $< $@

# Converting the manpages:
.1.1.html:
	mandoc -Thtml -Wall -fstrict $< >$@
.1.1.txt:
	mandoc -Wall -fstrict $< | col -b >$@

install: default
	mkdir -p $(DESTDIR)/$(DOCDIR)
	mkdir -p $(DESTDIR)/$(BINDIR)
	mkdir -p $(DESTDIR)/$(MAN1DIR)
	install -d $(DESTDIR)/$(BINDIR) $(DESTDIR)/$(MAN1DIR)
	install -m 644 README.md $(DESTDIR)/$(DOCDIR)
	install -m 775 wrapper $(DESTDIR)/$(BINDIR)
	install -m 444 $(FORMAT) $(DESTDIR)/$(MAN1DIR)

uninstall:
.for manpage in $(FORMAT)
	rm -f $(DESTDIR)/$(MAN1DIR)/`basename $(manpage)`
.endfor
	rm -rf $(DESTDIR)/$(DOCDIR)
	rm -f $(DESTDIR)/$(BINDIR)/wrapper

clean:
	rm -f wrapper */*~ *~ *.o *.core $(FORMAT) $(CONVERT)
