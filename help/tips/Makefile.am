## Process this file with automake to produce Makefile.in.

EXTRA_DIST = tips.txt fr.po de.po es.po fr.gmo de.gmo es.gmo
grisbi_helpdir = $(datadir)/$(PACKAGE)
grisbi_help_DATA = tips.txt

#grisbi_potdir = $(datadir)/locale/grisbi-tips
#grisbi_pot_DATA = fr.gmo

CC = gcc
GMOFILES=fr.gmo de.gmo es.gmo
localedir = ${datadir}/locale

all:	$(GMOFILES)

tips.pot:	tips.txt
	$(XGETTEXT) --language=perl --keyword=_ -o tips.pot -s tips.txt

%.pox:	%.po
	$(MAKE) grisbi-tips.pot
	$(MSGMERGE) $< $(srcdir)/grisbi-tips.pot -o $*.pox

%.mo:	%.po
	$(MSGFMT) -o $@ $<

%.gmo:	%.po
	file=$(srcdir)/`echo $* | sed 's,.*/,,'`.gmo \
	  && rm -f $$file && $(GMSGFMT) --statistics -o $$file $<

update-po: Makefile
	$(MAKE) tips.pot
	if test "tips" = "gettext"; then PATH=`pwd`/../src:$$PATH; fi; \
	cd $(srcdir); \
	catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed 's/\.gmo$$//'`; \
	  echo "$$lang:"; \
	  if $(MSGMERGE) $$lang.po tips.pot -o $$lang.new.po; then \
	    mv -f $$lang.new.po $$lang.po; \
	  else \
	    echo "msgmerge for $$cat failed!"; \
	    rm -f $$lang.new.po; \
	  fi; \
	done
	$(MAKE) update-gmo

update-gmo: Makefile $(GMOFILES)
	@:

install: install-exec-am install-data-am
	$(mkinstalldirs) $(DESTDIR)$(datadir)
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed 's/\.gmo$$//'`; \
	  dir=$(localedir)/$$lang/LC_MESSAGES; \
	  $(mkinstalldirs) $(DESTDIR)$$dir; \
	  if test -r $$cat; then \
	    $(INSTALL_DATA) $$cat $(DESTDIR)$$dir/grisbi-tips.mo; \
	    echo "installing $$cat as $(DESTDIR)$$dir/grisbi-tips.mo"; \
	  else \
	    $(INSTALL_DATA) $(srcdir)/$$cat $(DESTDIR)$$dir/grisbi-tips.mo; \
	    echo "installing $(srcdir)/$$cat as" \
		 "$(DESTDIR)$$dir/grisbi-tips.mo"; \
	  fi; \
	done

uninstall:
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	  cat=`basename $$cat`; \
	  lang=`echo $$cat | sed 's/\.gmo$$//'`; \
	  dir=$(localedir)/$$lang/LC_MESSAGES; \
          rm -f $(DESTDIR)$$dir/grisbi-tips.mo; \
	  echo "uninstalling  $(DESTDIR)$$dir/grisbi-tips.mo"; \
	done

