# 2wm - stereo window manager
#   Original Code (C)opyright MMVII Anselm R. Garbe
#   Changes (C) MMXIV Joshua W. Scoggins

include config.mk

SRC = client.cc event.cc main.cc util.cc view.cc
OBJ = ${SRC:.cc=.o}

all: options 2wm

options:
	@echo 2wmrxx build options:
	@echo "CXXFLAGS   = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CXX       = ${CXX}"

.cc.o:
	@echo CXX $<
	@${CXX} -c ${CXXFLAGS} $<

${OBJ}: 2wm.h config.h config.mk

config.h:
	@echo creating $@ from config.default.h
	@cp config.default.h $@

2wm: ${OBJ}
	@echo CXX -o $@
	@${CXX} -o $@ ${OBJ} ${LDFLAGS}
	@strip $@

clean:
	@echo cleaning
	@rm -f 2wm ${OBJ} 2wm-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p 2wm-${VERSION}
	@cp -R LICENSE Makefile README config.*.h config.mk \
		2wm.1 2wm.h ${SRC} 2wm-${VERSION}
	@tar -cf 2wm-${VERSION}.tar 2wm-${VERSION}
	@gzip 2wm-${VERSION}.tar
	@rm -rf 2wm-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f 2wm ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/2wm
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed 's/VERSION/${VERSION}/g' < 2wm.1 > ${DESTDIR}${MANPREFIX}/man1/2wm.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/2wm.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/2wm
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/2wm.1

.PHONY: all options clean dist install uninstall
