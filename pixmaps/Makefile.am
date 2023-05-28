SUBDIRS = flags

grisbi_pixmapsdir = $(datadir)/pixmaps/$(PACKAGE)

grisbi_pixmaps_DATA = \
	grisbi.svg \
	gsb-amount-32.png \
	gsb-ac-asset-32.png \
	gsb-ac-bank-16.png \
	gsb-ac-bank-32.png \
	gsb-ac-cash-32.png \
	gsb-ac-home-32.png \
	gsb-ac-liability-16.png \
	gsb-ac-liability-24.png \
	gsb-ac-liability-32.png \
	gsb-addresses-32.png \
	gsb-archive-16.png \
	gsb-archive-24.png \
	gsb-archive-32.png \
	gsb-arrow-down-16.png \
	gsb-balance_estimate-32.png \
	gsb-banks-32.png \
	gsb-budgetary_lines-32.png \
	gsb-bug-16.png \
	gsb-bug-32.png \
	gsb-categories-32.png \
	gsb-comments-16.png \
	gsb-comments-24.png \
	gsb-convert-16.png \
	gsb-convert-24.png \
	gsb-currencies-32.png \
	gsb-display-gui-32.png \
	gsb-export-16.png \
	gsb-export-24.png \
	gsb-export-32.png \
	gsb-export-archive-16.png \
	gsb-export-archive-32.png \
	gsb-files-32.png \
	gsb-financial-years-32.png \
	gsb-fonts-32.png \
	gsb-form-32.png \
	gsb-generalities-32.png \
	gsb-graph-histo-24.png \
	gsb-graph-line-24.png \
	gsb-graph-sectors-24.png \
	gsb-grille-16.png \
	gsb-help-16.png \
	gsb-import-16.png \
	gsb-import-32.png \
	gsb-jump-16.png \
	gsb-new-categ-16.png \
	gsb-new-categ-24.png \
	gsb-new-file-16.png \
	gsb-new-file-24.png \
	gsb-new-file-32.png \
	gsb-new-ib-16.png \
	gsb-new-ib-24.png \
	gsb-new-payee-16.png \
	gsb-new-payee-24.png \
	gsb-new-report-16.png \
	gsb-new-report-24.png \
	gsb-new-scheduled-24.png \
	gsb-new-sub-categ-24.png \
	gsb-new-sub-ib-24.png \
	gsb-new-transaction-16.png \
	gsb-new-transaction-24.png \
	gsb-organization-32.png \
	gsb-payees-32.png \
	gsb-payees-manage-16.png \
	gsb-payees-manage-24.png \
	gsb-payment-32.png \
	gsb-pdf-24.png \
	gsb-preferences-16.png \
	gsb-reconat-32.png \
	gsb-reconciliation-24.png \
	gsb-reconciliation-32.png \
	gsb-reports-32.png \
	gsb-scheduler-32.png \
	gsb-text-32.png \
	gsb-title-32.png \
	gsb-toolbar-32.png \
	gsb-transaction-list-32.png \
	gsb-transdisplay-32.png \
	gsb-transfer-32.png \
	gsb-warnings-32.png \
	gsb-web-browser-16.png \
	gtk-about-16.png \
	gtk-close-16.png \
	gtk-copy-16.png \
	gtk-copy-24.png \
	gtk-corbeille-32.png \
	gtk-delete-16.png \
	gtk-delete-24.png \
	gtk-dialog-warning-16.png \
	gtk-edit-16.png \
	gtk-edit-24.png \
	gtk-execute-16.png \
	gtk-execute-24.png \
	gtk-open-16.png \
	gtk-open-24.png \
	gtk-print-16.png \
	gtk-print-24.png \
	gtk-properties-16.png \
	gtk-properties-24.png \
	gtk-quit-16.png \
	gtk-refresh-16.png \
	gtk-remove-16.png \
	gtk-save-as-16.png \
	gtk-save-16.png \
	gtk-search-16.png \
	gtk-select-color-24.png \
	gtk-window-new-16.png

grisbi_svgiconsdir = $(datadir)/icons/hicolor/scalable/apps
grisbi_svgicons_DATA = grisbi.svg

grisbi_mimesvgicondir = $(datadir)/icons/hicolor/scalable/mimetypes
grisbi_mimesvgicon_DATA = application-x-grisbi.svg

EXTRA_DIST = $(grisbi_pixmaps_DATA) $(grisbi_icons_DATA) $(grisbi_mimesvgicon_DATA)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

