# Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir) -I$(includedir) -I../intl -DLOCALEDIR=\"$(localedir)\" -DPIXMAPS_DIR="\"$(datadir)/pixmaps/grisbi\"" -DLOGO_PATH="\"$(datadir)/pixmaps/grisbi/grisbi-logo.png\"" -DANIM_PATH="\"$(datadir)/pixmaps/grisbi/euro.gif\"" -DHELP_PATH="\"$(datadir)/grisbi/help\"" -Wall @g2banking_includes@
bin_PROGRAMS = grisbi

config_dir = config

sources_config = 			\
	configuration.c

includes_config= \
	configuration.h  \
	xmlnames.h      

sources = \
	accueil.c		\
	affichage.c		\
	affichage_liste.c	\
	affichage_formulaire.c	\
	banque.c		\
	barre_outils.c		\
	calendar.c		\
	categories_onglet.c	\
	classement_operations.c	\
	classement_echeances.c	\
	comptes_gestion.c	\
	comptes_onglet.c	\
	comptes_traitements.c	\
	import_csv.c	\
	devises.c		\
	dialog.c		\
	echeancier_onglet.c	\
	echeancier_formulaire.c	\
	echeancier_liste.c	\
	echeancier_infos.c	\
	echeancier_ventilation.c\
	equilibrage.c		\
	erreur.c		\
	etat_io.c		\
	etats_affiche.c		\
	etats_calculs.c		\
	etats_config.c		\
	etats_gtktable.c	\
	etats_latex.c		\
	etats_onglet.c		\
	etats_support.c		\
	exercice.c		\
	fenetre_principale.c	\
	fichier_configuration.c	\
	fichiers_gestion.c	\
	fichiers_io.c		\
	format.c		\
	gnucash.c               \
	gtk_combofix.c		\
	gtk_list_button.c	\
	help.c			\
	imputation_budgetaire.c	\
	main.c			\
	menu.c			\
	meta_categories.c	\
	meta_budgetary.c	\
	metatree.c		\
	ofx.c                   \
	operations_comptes.c	\
	operations_formulaire.c \
	operations_liste.c	\
	operations_onglet.c	\
	parametres.c		\
	parse_cmdline.c		\
	patienter.c		\
	print_config.c		\
	qif.c			\
	import.c		\
	html.c			\
	tiers_onglet.c		\
	tip.c			\
	traitement_variables.c	\
	type_operations.c	\
	ventilation.c		\
	utils.c			\
	utils_buttons.c		\
	utils_categories.c	\
	utils_comptes.c		\
	utils_dates.c		\
	utils_devises.c		\
	utils_echeances.c	\
	utils_editables.c	\
	utils_exercices.c	\
	utils_file_selection.c \
	utils_files.c		\
	utils_ib.c		\
	utils_montants.c	\
	utils_operations.c	\
	utils_rapprochements.c	\
	utils_str.c		\
	utils_tiers.c		\
	utils_types.c		\
	utils_xml.c		\
	search_glist.c		\
	gbanking.c

grisbi_SOURCES = $(sources) $(sources_config)

grisbi_INCLUDES =		\
	$(include_config)       \
	accueil.h		\
	affichage.h		\
	affichage_liste.h	\
	affichage_formulaire.h	\
	banque.h		\
	barre_outils.h		\
	calendar.h		\
	categories_onglet.h	\
	classement_operations.h	\
	classement_echeances.h	\
	comptes_gestion.h	\
	comptes_onglet.h	\
	comptes_traitements.h	\
	constants.h		\
	import_csv.h \
	devises.h		\
	dialog.h		\
	echeancier_onglet.h	\
	echeancier_formulaire.h	\
	echeancier_liste.h	\
	echeancier_infos.h	\
	echeancier_ventilation.h\
	equilibrage.h		\
	erreur.h		\
	etat_io.h		\
	etats_calculs.h		\
	etats_config.h		\
	etats_gtktable.h	\
	etats.h			\
	etats_latex.h		\
	etats_onglet.h		\
	etats_support.h		\
	exercice.h		\
	fenetre_principale.h	\
	fichier_configuration.h	\
	fichiers_gestion.h	\
	fichiers_io.h		\
	format.h		\
	gtk_combofix.h		\
	gtk_list_button.h	\
	help.h			\
	imputation_budgetaire.h	\
	include.h		\
	main.h			\
	menu.h			\
	operations_comptes.h	\
	operations_formulaire.h	\
	operations_liste.h	\
	operations_onglet.h	\
	parametres.h		\
	parse_cmdline.h		\
	patienter.h		\
	print_config.h		\
	qif.h			\
	import.h		\
	html.h			\
	tip.h			\
	structures.h		\
	tiers_onglet.h		\
	traitement_variables.h	\
	type_operations.h	\
	ventilation.h		\
	utils.h			\
	utils_c.h	\
	utils_buttons.h		\
	utils_categories.h	\
	utils_comptes.h		\
	utils_dates.h		\
	utils_devises.h		\
	utils_echeances.h	\
	utils_editables.h	\
	utils_exercices.h	\
	utils_file_selection.h \
	utils_files.h		\
	utils_ib.h		\
	utils_montants.h	\
	utils_operations.h	\
	utils_rapprochements.h	\
	utils_str.h		\
	utils_tiers.h		\
	utils_types.h		\
	utils_xml.h		\
	search_glist.h		\
	gbanking.h


EXTRA_DIST = grisbi.desktop grisbi.keys grisbi.mime $(grisbi_INCLUDES)

mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime
