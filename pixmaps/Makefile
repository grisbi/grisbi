# Makefile.in generated automatically by automake 1.4-p6 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..

prefix = /tmp
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/grisbi
pkglibdir = $(libdir)/grisbi
pkgincludedir = $(includedir)/grisbi

top_builddir = ..

ACLOCAL = aclocal-1.4 -I macros
AUTOCONF = autoconf
AUTOMAKE = automake-1.4
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = 
host_triplet = i686-pc-linux-gnu
BUILD_INCLUDED_LIBINTL = no
CATOBJEXT = .gmo
CC = gcc
CFLAGS = -g -O2 -DORBIT2=1 -pthread -I/usr/include/libgnome-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/orbit-2.0 -I/usr/include/libbonobo-2.0 -I/usr/include/gconf/2 -I/usr/include/gnome-vfs-2.0 -I/usr/lib/gnome-vfs-2.0/include -I/usr/include/linc-1.0 -I/usr/include/bonobo-activation-2.0 -I/usr/include/libgnomeui-2.0 -I/usr/include/libgnomecanvas-2.0 -I/usr/include/gtk-2.0 -I/usr/include/libart-2.0 -I/usr/include/libbonoboui-2.0 -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/X11R6/include -I/usr/include/libxml2 -I/usr/include/gnome-xml  
DATADIRNAME = share
GENCAT = gencat
GLIBC21 = yes
GMSGFMT = /usr/bin/msgfmt
HAVE_LIB = @HAVE_LIB@
INSTOBJEXT = .mo
INTLBISON = bison
INTLLIBS = 
INTLOBJS = 
INTL_LIBTOOL_SUFFIX_PREFIX = 
LDFLAGS =  -pthread -Wl,--export-dynamic -L/usr/X11R6/lib -lgnomeui-2 -lSM -lICE -lbonoboui-2 -lxml2 -lpthread -lgnomecanvas-2 -lgnome-2 -lart_lgpl_2 -lpangoft2-1.0 -lbonobo-2 -lgconf-2 -lgnomevfs-2 -lbonobo-activation -lORBit-2 -llinc -lgthread-2.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoxft-1.0 -lpangox-1.0 -lpango-1.0 -lgdk_pixbuf-2.0 -lm -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0 -lxml -lz  
LIB = @LIB@
LIBICONV = 
LIBINTL = 
LTLIB = @LTLIB@
LTLIBICONV = 
LTLIBINTL = 
MAINT = #
MAKEINFO = makeinfo
MKINSTALLDIRS = ./mkinstalldirs
PACKAGE = grisbi
PKG_CONFIG = /usr/bin/pkg-config
POSUB = po
RANLIB = ranlib
USE_INCLUDED_LIBINTL = no
USE_NLS = yes
VERSION = 0.5.0
prefix = /tmp

grisbi_pixmapsdir = $(datadir)/pixmaps/grisbi

grisbi_pixmaps_DATA =  	addresses.png 	banks.png 	currencies.png 	files.png 	financial-years.png 	fonts.png 	form.png 	payment-methods.png 	reconciliation.png 	transaction-list.png 	warnings.png

mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
DATA =  $(grisbi_pixmaps_DATA)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
all: all-redirect
.SUFFIXES:
$(srcdir)/Makefile.in: # Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu pixmaps/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


install-grisbi_pixmapsDATA: $(grisbi_pixmaps_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(grisbi_pixmapsdir)
	@list='$(grisbi_pixmaps_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(grisbi_pixmapsdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(grisbi_pixmapsdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(grisbi_pixmapsdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(grisbi_pixmapsdir)/$$p; \
	  fi; fi; \
	done

uninstall-grisbi_pixmapsDATA:
	@$(NORMAL_UNINSTALL)
	list='$(grisbi_pixmaps_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(grisbi_pixmapsdir)/$$p; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = pixmaps

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	distdir=`cd $(distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu pixmaps/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am:
install-exec: install-exec-am

install-data-am: install-grisbi_pixmapsDATA
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-grisbi_pixmapsDATA
uninstall: uninstall-am
all-am: Makefile $(DATA)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(grisbi_pixmapsdir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-generic mostlyclean-am

clean: clean-am

distclean-am:  distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: uninstall-grisbi_pixmapsDATA install-grisbi_pixmapsDATA tags \
distdir info-am info dvi-am dvi check check-am installcheck-am \
installcheck install-exec-am install-exec install-data-am install-data \
install-am install uninstall-am uninstall all-redirect all-am all \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
