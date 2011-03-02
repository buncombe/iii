# Customize to fit your system.

# If an environment variable is available, the affected setting below is either
# skipped or (if the variable is holding flags) appended to the already defined
# string.

# File paths:
PREFIX		?= /usr/local
BINDIR		?= ${PREFIX}/bin
MANDIR		?= ${PREFIX}/share/man
MAN1DIR		?= ${MANDIR}/man1
DOCDIR		?= ${PREFIX}/share/doc/ii
INCDIR		?= ${PREFIX}/include
LIBDIR		?= ${PREFIX}/lib

# Set the following to install to a different root directory:
DESTDIR		?=

# Macro definitions:
VERSION		?= 1.6
# The ii(1) executable that the wrapper(1) program will use: 
II_EXEC		?= ii
# Amount of seconds that it (at most) takes for ii(1) to connect:
CHLD_SLEEP	?= 300
# Amount of seconds before wrapper(1) (re)initiates an ii(1) instance:
LOOP_SLEEP	?= 60

# Includes and libs:
INCLUDES	+= -I. -I${INCDIR} -I/usr/include
LIBS		+= -L${LIBDIR} -L/usr/lib -lc -lssl -lcrypto

# Compiler flags:
CC		?= cc
CFLAGS		+= -g -O0 -W -Wall ${INCLUDES} -DVERSION=\"${VERSION}\" \
    -DII_EXEC=\"${II_EXEC}\" -DCHLD_SLEEP=${CHLD_SLEEP} \
    -DLOOP_SLEEP=${LOOP_SLEEP} -DHAVE_SSL

# Uncomment for compiling on Solaris:
#LIBS		= -L${LIBDIR} -L/usr/lib -lc -lsocket -lnsl -lssl -lcrypto
#CFLAGS		= -g ${INCLUDES} -DVERSION=\"${VERSION}\" \
    -DII_EXEC=\"${II_EXEC}\" -DCHLD_SLEEP=${CHLD_SLEEP} \
    -DLOOP_SLEEP=${LOOP_SLEEP} -DHAVE_SSL

# Uncomment to disable SSL support in ii(1):
#LIBS		= -L${LIBDIR} -L/usr/lib -lc
#CFLAGS		= -g -O0 -W -Wall ${INCLUDES} -DVERSION=\"${VERSION}\" \
    -DII_EXEC=\"${II_EXEC}\" -DCHLD_SLEEP=${CHLD_SLEEP} \
    -DLOOP_SLEEP=${LOOP_SLEEP}

# Linker flags. If static binaries are desired, add "-static" to this
# environment variable.
LDFLAGS		+= ${LIBS}
