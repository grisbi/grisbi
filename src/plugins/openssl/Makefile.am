# OpenSSL plugin

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src

plugindir = $(libdir)/@PACKAGE@

if HAVE_SSL

plugin_LTLIBRARIES = openssl.la
openssl_la_SOURCES = openssl.c openssl.h
openssl_la_LDFLAGS = -module -avoid-version $(OpenSSL_LIBS)
openssl_la_CFLAGS = $(OpenSSL_CFLAGS)

# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.
openssl_la_LIBS = 

endif

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
