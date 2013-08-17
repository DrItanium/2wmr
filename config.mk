# 2wm version
VERSION = 0.2

# Customize below to fit your system

# paths
PREFIX = $(ElectronFSRoot)/sys
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} -I$(ElectronFSRoot)/sys/include
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 $(ElectronFSRoot)/sys/lib/libelectron.a -lm -lrt

# flags
CFLAGS = -Os ${INCS} -DVERSION=\"${VERSION}\"
LDFLAGS = ${LIBS}
#CFLAGS = -g -Wall -O2 ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = -g ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}
#CFLAGS += -xtarget=ultra

# compiler and linker
CC = cc
