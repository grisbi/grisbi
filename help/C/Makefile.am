## Process this file with automake to produce Makefile.in.

HTML_FILES =			\
	index.html		\
	grisbi001.html		\
	grisbi002.html		\
	grisbi003.html		\
	grisbi004.html		\
	grisbi005.html		\
	grisbi006.html		\
	grisbi007.html		\
	grisbi008.html		\
	grisbi009.html		\
	grisbi010.html		\
	grisbi011.html		\
	grisbi012.html		\
	grisbi013.html		\
	grisbi014.html		\
	grisbi015.html		\
	grisbi016.html		\
	grisbi017.html		\
	grisbi018.html		\
	grisbi019.html		\
	quickstart.html

grisbi_helpdir = $(datadir)/gnome/help/grisbi/C

grisbi_help_DATA = \
	topic.dat     \
	contents_motif.gif		\
	previous_motif.gif	\
	next_motif.gif	\
	$(HTML_FILES)

EXTRA_DIST = 		\
	contents_motif.gif		\
	previous_motif.gif	\
	next_motif.gif	\
	topic.dat

$(HTML_FILES) : grisbi/index.html
	-cp $(srcdir)/grisbi/*.html .

#grisbi/grisbi.html: gphoneman.sgml
#	-(cd $(srcdir); db2html gphoneman.sgml)

dist-hook:
	mkdir $(distdir)/grisbi
	mkdir $(distdir)/images
	-cp grisbi/*.html $(distdir)/grisbi
	-cp $(HTML_FILES) $(distdir)
	-cp images/*.png $(distdir)/images

install-data-local: index.html
	$(mkinstalldirs) $(grisbi_helpdir)/images
	-for file in $(srcdir)/grisbi/*.html; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $(srcdir)/$$file $(grisbi_helpdir)/$$basefile; \
	done
	-for file in $(srcdir)/images/*.png; do \
	  basefile=`basename $$file`; \
	  $(INSTALL_DATA) $(srcdir)/$$file \
				$(grisbi_helpdir)/images/$$basefile; \
	done
