# Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir) -I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = grisbi_applet

grisbi_applet_SOURCES = 	\
	grisbi_applet.c


grisbi_applet_LDADD = 		\
	$(GNOME_LIBDIR)			\
	$(GNOMEUI_LIBS) 		\
	$(EXTRA_GNOME_LIBS)

EXTRA_DIST =			\
	grisbi_applet.gnorba	\
	grisbi_applet.desktop

sysdir = $(datadir)/applets/Utility
sys_DATA = grisbi_applet.desktop

gnorbadir = $(sysconfdir)/CORBA/servers
gnorba_DATA = grisbi_applet.gnorba
