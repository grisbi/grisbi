# OFX plugin

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src $(GRISBI_CFLAGS) $(LIBOFX_CFLAGS)

if HAVE_OFX

noinst_LTLIBRARIES = ofx.la
ofx_la_SOURCES = ofx.c ofx.h
ofx_la_LDFLAGS = -module -avoid-version
ofx_la_LIBADD = $(LIBOFX_LIBS)

endif

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
