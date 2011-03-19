# Customize to fit your system.

# If an environment variable is available, the affected setting below is either
# skipped or (if the variable is holding flags) appended to the already defined
# string.

# File paths:
PREFIX		?= /usr/local
BINDIR		?= $(PREFIX)/bin
MANDIR		?= $(PREFIX)/share/man
MAN1DIR		?= $(MANDIR)/man1
DOCDIR		?= $(PREFIX)/share/doc/ii
INCDIR		?= $(PREFIX)/include
LIBDIR		?= $(PREFIX)/lib

# Set the following to install to a different root directory:
DESTDIR		?=

# The version of ii(1):
VERSION		?= 1.6

# Includes and libs:
INCLUDES	+= -I. -I$(INCDIR) -I/usr/include
LIBS		+= -L$(LIBDIR) -L/usr/lib -lc -lssl -lcrypto

# Compiler flags:
CC		?= cc
CFLAGS		+= -g -O0 -W -Wall $(INCLUDES) -DVERSION=\"$(VERSION)\" \
    -DIIEXEC=\"$(IIEXEC)\" -DCHLDSLEEP=$(CHLDSLEEP) \
    -DLOOPSLEEP=$(LOOPSLEEP) -DUSESSL

# Uncomment for compiling on Solaris:
#LIBS		= -L$(LIBDIR) -L/usr/lib -lc -lsocket -lnsl -lssl -lcrypto
#CFLAGS		= -g $(INCLUDES) -DVERSION=\"$(VERSION)\" \
    -DIIEXEC=\"$(IIEXEC)\" -DCHLDSLEEP=$(CHLDSLEEP) \
    -DLOOPSLEEP=$(LOOPSLEEP) -DUSESSL

# Uncomment to disable SSL and SASL support in ii(1):
#LIBS		= -L$(LIBDIR) -L/usr/lib -lc
#CFLAGS		= -g -O0 -W -Wall $(INCLUDES) -DVERSION=\"$(VERSION)\" \
    -DIIEXEC=\"$(IIEXEC)\" -DCHLDSLEEP=$(CHLDSLEEP) \
    -DLOOPSLEEP=$(LOOPSLEEP)

# Linker flags. If a static binary is desired, add "-static" to this
# environment variable.
LDFLAGS		+= $(LIBS)
