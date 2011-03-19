# Customize to fit your system.

# If an environment variable is available, the affected setting below is either
# skipped or (if the variable is holding flags) appended to the already defined
# string.

# File paths:
PREFIX		?= /usr/local
BINDIR		?= $(PREFIX)/bin
MANDIR		?= $(PREFIX)/share/man
MAN1DIR		?= $(MANDIR)/man1
DOCDIR		?= $(PREFIX)/share/doc/wrapper
INCDIR		?= $(PREFIX)/include
LIBDIR		?= $(PREFIX)/lib

# Set the following to install to a different root directory:
DESTDIR		?=

# Macro definitions:
# The ii(1) executable that the wrapper(1) program will use:
IIEXEC		?= ii
# Amount of seconds that it (at most) takes for ii(1) to connect:
CHLDSLEEP	?= 300
# Amount of seconds before wrapper(1) (re)initiates an ii(1) instance:
LOOPSLEEP	?= 60

# Includes and libs:
INCLUDES	+= -I. -I$(INCDIR) -I/usr/include
LIBS		+= -L$(LIBDIR) -L/usr/lib -lc

# Compiler flags:
CC		?= cc
CFLAGS		+= -g -O0 -W -Wall $(INCLUDES) -DIIEXEC=\"$(IIEXEC)\" \
    -DCHLDSLEEP=$(CHLDSLEEP) -DLOOPSLEEP=$(LOOPSLEEP)

# Linker flags. If a static binary is desired, add "-static" to this
# environment variable.
LDFLAGS		+= $(LIBS)
