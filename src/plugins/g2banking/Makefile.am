# g2banking (HBCI) plugin

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src

plugindir = $(libdir)/@PACKAGE@

if HAVE_G2BANKING

plugin_LTLIBRARIES = g2banking.la
g2banking_la_SOURCES = g2banking.c g2banking.h
g2banking_la_LDFLAGS = -module -avoid-version @g2banking_libs@ 

# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.
g2banking_la_LIBS = @g2banking_libs@ @GTK_LIBS@
g2banking_la_CFLAGS = @g2banking_includes@ -DHAVE_G2BANKING @GTK_CFLAGS@

endif

CLEANFILES = \
	*~

MAINTAINERCLEANFILES = \
	Makefile.in
