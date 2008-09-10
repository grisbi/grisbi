grisbi_sharedir = $(datadir)/$(PACKAGE)

grisbi_share_dirs = categories

install-data-hook:
	for i in `find ${grisbi_share_dirs} -type f` ; do \
		mkdir -p ${grisbi_sharedir}/`dirname $$i` ; \
		cp $$i ${grisbi_sharedir}/`dirname $$i` ; \
	done

EXTRA_DIST=${grisbi_share_dirs}