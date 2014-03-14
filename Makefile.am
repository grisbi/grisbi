## Process this file with automake to produce Makefile.in

SUBDIRS = src po pixmaps share
if DOC
SUBDIRS += help
endif
man_MANS = grisbi.1

EXTRA_DIST = \
    grisbi.spec \
    win32 \
    win32-msvc\
    $(man_MANS)


ACLOCAL_AMFLAGS = -I m4
