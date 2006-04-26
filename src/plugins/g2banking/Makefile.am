# g2banking (HBCI) plugin

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src

plugindir = $(libdir)/grisbi

if HAVE_G2BANKING

plugin_LTLIBRARIES = g2banking.la
g2banking_la_SOURCES = g2banking.c g2banking.h
g2banking_la_LDFLAGS = -module -avoid-version

# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.
LIBS = -lxxx

endif

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
