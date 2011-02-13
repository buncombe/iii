# Customize to fit your system

# If an environment variable is available, the affected setting below is either
# skipped or (if the variable is holding flags) appended to the already defined
# string.

# paths
PREFIX		?= /usr/local
BINDIR		?= ${PREFIX}/bin
MANDIR		?= ${PREFIX}/share/man
MAN1DIR		?= ${MANDIR}/man1
DOCDIR		?= ${PREFIX}/share/doc/ii

# Set the following to install to a different root
DESTDIR		?=

INCDIR		?= ${PREFIX}/include
LIBDIR		?= ${PREFIX}/lib
VERSION		?= 1.6
# The ii(1) executable that the wrapper program will use: 
EXECUTABLE	?= ii
# Amount of seconds that it (generally) takes for ii(1) to connect:
SSLEEP		?= 60

# includes and libs
INCLUDES	+= -I. -I${INCDIR} -I/usr/include
LIBS		+= -L${LIBDIR} -L/usr/lib -lc
# uncomment and comment other variables for compiling on Solaris
#LIBS		+= -L${LIBDIR} -L/usr/lib -lc -lsocket -lnsl
#CFLAGS		+= -g ${INCLUDES} -DVERSION=\"${VERSION}\" -DEXECUTABLE=\"${EXECUTABLE}\"

# compiler
CC		?= cc
CFLAGS		+= -g -O0 -W -Wall ${INCLUDES} -DVERSION=\"${VERSION}\" -DEXECUTABLE=\"${EXECUTABLE}\" -DSSLEEP=${SSLEEP}
LDFLAGS		+= ${LIBS}
