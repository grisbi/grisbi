# Process this file with automake to produce Makefile.in

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir) -I$(includedir) -I../intl -DLOCALEDIR=\"$(localedir)\" -DPIXMAPS_DIR="\"$(datadir)/pixmaps/grisbi\"" -DCHEMIN_LOGO="\"$(datadir)/pixmaps/grisbi/grisbi-logo.png\"" -Wall 
bin_PROGRAMS = grisbi

sources = 			\
	accueil.c		\
	affichage.c		\
	affichage_liste.c	\
	banque.c		\
	barre_outils.c		\
	calendar.c		\
	categories_onglet.c	\
	comptes_gestion.c	\
	comptes_onglet.c	\
	comptes_traitements.c	\
	devises.c		\
	dialog.c		\
	echeancier_onglet.c	\
	echeancier_formulaire.c	\
	echeancier_liste.c	\
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
	gtk_combofix.c		\
	gtk_list_button.c	\
	help.c			\
	imputation_budgetaire.c	\
	main.c			\
	menu.c			\
	operations_classement.c	\
	operations_comptes.c	\
	operations_formulaire.c \
	operations_liste.c	\
	operations_onglet.c	\
	parametres.c		\
	patienter.c		\
	print_config.c		\
	qif.c			\
	import.c		\
	html.c			\
	tiers_onglet.c		\
	traitement_variables.c	\
	type_operations.c	\
	ventilation.c		\
	utils.c			\
	search_glist.c

if NOOFX
grisbi_SOURCES = $(sources)
else
grisbi_SOURCES = $(sources) ofx.c
endif

grisbi_INCLUDES =		\
	accueil.h		\
	affichage.h		\
	affichage_liste.h	\
	banque.h		\
	barre_outils.h		\
	calendar.h		\
	categories_onglet.h	\
	comptes_gestion.h	\
	comptes_onglet.h	\
	comptes_traitements.h	\
	constants.h		\
	devises.h		\
	dialog.h		\
	echeancier_onglet.h	\
	echeancier_formulaire.h	\
	echeancier_liste.h	\
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
	gtkcombofix.h		\
	gtk_list_button.h	\
	help.h			\
	imputation_budgetaire.h	\
	include.h		\
	main.h			\
	menu.h			\
	operations_classement.h	\
	operations_comptes.h	\
	operations_formulaire.h	\
	operations_liste.h	\
	operations_onglet.h	\
	parametres.h		\
	patienter.h		\
	print_config.h		\
	qif.h			\
	import.h		\
	html.h			\
	structures.h		\
	tiers_onglet.h		\
	traitement_variables.h	\
	type_operations.h	\
	ventilation.h		\
	utils.h			\
	search_glist.h


EXTRA_DIST = grisbi.desktop grisbi.keys grisbi.mime $(grisbi_INCLUDES)

mimedir = $(datadir)/mime-info
mime_DATA = grisbi.keys grisbi.mime


