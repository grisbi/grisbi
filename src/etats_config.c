/* ************************************************************************** */
/* Ce fichier s'occupe de la configuration des états                          */
/* 			etats_config.c                                        */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (dionysos@grisbi.org) 	      */
/*			http://www.grisbi.org   			      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "etats_config.h"

#include "calendar.h"
#include "devises.h"
#include "dialog.h"
#include "etats_calculs.h"
#include "etats_onglet.h"
#include "operations_classement.h"
#include "search_glist.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "utils.h"


gchar *liste_plages_dates[] = {
    N_("All"),
    N_("Custom"),
    N_("Total to now"),
    N_("Current month"),
    N_("Current year"),
    N_("Current month to now"),
    N_("Current year to now"),
    N_("Previous month"),
    N_("Previous year"),
    N_("Last 30 days"),
    N_("Last 3 months"),
    N_("Last 6 months"),
    N_("Last 12 months"),
    NULL };

gchar *jours_semaine[] = {
    N_("Monday"),
    N_("Tuesday"),
    N_("Wednesday"),
    N_("Thursday"),
    N_("Friday"),
    N_("Saturday"),
    N_("Sunday"),
    NULL };

/******************************************************************************/
/* Fontion personnalistation_etat */
/* affiche la fenetre de personnalisation */
/******************************************************************************/
void personnalisation_etat (void)
{
    GtkWidget *separateur;
    GtkWidget *bouton;
    GtkWidget *hbox;
    GtkCTreeNode *parent;
    GList *pointeur_liste;

    if ( !etat_courant )
	return;

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );


    if ( !onglet_config_etat )
    {
	onglet_config_etat = gtk_vbox_new ( FALSE,
					    5 );
	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_etats ),
				   onglet_config_etat,
				   gtk_label_new ( _("Reports setup")));
	gtk_widget_show ( onglet_config_etat );

	/* on commence par créer le notebook parent de la config des états */

	notebook_config_etat = gtk_notebook_new ();
	gtk_notebook_set_scrollable ( GTK_NOTEBOOK ( notebook_config_etat ),
				      TRUE );
	gtk_box_pack_start ( GTK_BOX ( onglet_config_etat ),
			     notebook_config_etat,
			     TRUE,
			     TRUE,
			     0 );
	gtk_widget_show ( notebook_config_etat );


	/* on ajoute les onglets dans l'onglet de base correspondant */

	/* remplissage de l'onglet de sélection */

	notebook_selection = gtk_notebook_new ();
	gtk_container_set_border_width ( GTK_CONTAINER ( notebook_selection ),
					 5 );
	gtk_notebook_set_scrollable ( GTK_NOTEBOOK ( notebook_selection ),
				      TRUE );
	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
				   notebook_selection,
				   gtk_label_new (_("Data selection")) );
	gtk_widget_show ( notebook_selection );


	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_dates (),
				   gtk_label_new (SPACIFY(_("Dates"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_virements (),
				   gtk_label_new (SPACIFY(_("Transfers"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_comptes (),
				   gtk_label_new (SPACIFY(_("Accounts"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_tiers (),
				   gtk_label_new (SPACIFY(_("Third party"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_categories (),
				   gtk_label_new (SPACIFY(_("Categories"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_ib (),
				   gtk_label_new (SPACIFY(_("Budgetary lines"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_texte (),
				   gtk_label_new (SPACIFY(_("Texts"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_montant (),
				   gtk_label_new (SPACIFY(_("Amounts"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_mode_paiement (),
				   gtk_label_new (SPACIFY(_("Payment methods"))) );

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_selection ),
				   onglet_etat_divers (),
				   gtk_label_new (SPACIFY(_("Misc."))) );

	/* remplissage de l'onglet d'organisation */

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
				   page_organisation_donnees (),
				   gtk_label_new (_("Data organisation")) );


	/* remplissage de l'onglet d'affichage */

	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_config_etat ),
				   page_affichage_donnees (),
				   gtk_label_new (_("Data display")) );


	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_config_etat),
				0 );

	/* mise en place des boutons appliquer et annuler */

	separateur = gtk_hseparator_new ();
	gtk_box_pack_start ( GTK_BOX ( onglet_config_etat ),
			     separateur,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( separateur );

	hbox = gtk_hbox_new ( FALSE,
			      5 );
	gtk_box_pack_start ( GTK_BOX ( onglet_config_etat ),
			     hbox,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( hbox );

	bouton = gtk_button_new_from_stock (GTK_STOCK_APPLY);
	gtk_button_set_relief ( GTK_BUTTON ( bouton ),
				GTK_RELIEF_NONE );
	gtk_signal_connect ( GTK_OBJECT ( bouton ),
			     "clicked",
			     GTK_SIGNAL_FUNC ( recuperation_info_perso_etat ),
			     NULL );
	gtk_box_pack_end ( GTK_BOX ( hbox ),
			   bouton,
			   FALSE,
			   FALSE,
			   0 );
	gtk_widget_show ( bouton );

	bouton = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_button_set_relief ( GTK_BUTTON ( bouton ),
				GTK_RELIEF_NONE );
	gtk_signal_connect ( GTK_OBJECT ( bouton ),
			     "clicked",
			     GTK_SIGNAL_FUNC ( annule_modif_config ),
			     NULL );
	gtk_box_pack_end ( GTK_BOX ( hbox ),
			   bouton,
			   FALSE,
			   FALSE,
			   0 );
	gtk_widget_show ( bouton);

    }


    /* on va maintenant remplir toutes les infos de l'état */


    /* onglet généralités */


    /* on met le nom de l'état */

    gtk_entry_set_text ( GTK_ENTRY ( entree_nom_etat ),
			 etat_courant -> nom_etat );

    /* on remplit le ctree en fonction du classement courant */

    pointeur_liste = etat_courant -> type_classement;
    parent = NULL;
    gtk_clist_clear ( GTK_CLIST ( liste_type_classement_etat ));

    while ( pointeur_liste )
    {
	gchar *text[1];

	text[0] = NULL;

	switch ( GPOINTER_TO_INT ( pointeur_liste -> data ))
	{
	    case 1:
		text[0] = _("Category");
		break;

	    case 3:
		text[0] = _("Budgetary line");
		break;

	    case 5:
		text[0] = _("Account");
		break;

	    case 6:
		text[0] = _("Third party");
		break;

	}

	if ( text[0] )
	{
	    parent = gtk_ctree_insert_node ( GTK_CTREE ( liste_type_classement_etat ),
					     parent,
					     NULL,
					     text,
					     5,
					     NULL,
					     NULL,
					     NULL,
					     NULL,
					     FALSE,
					     TRUE );
	    gtk_ctree_node_set_row_data ( GTK_CTREE ( liste_type_classement_etat ),
					  GTK_CTREE_NODE ( parent ),
					  pointeur_liste -> data );
	}

	pointeur_liste = pointeur_liste -> next;
    }

    gtk_clist_select_row ( GTK_CLIST ( liste_type_classement_etat ),
			   0,
			   0 );

    if ( etat_courant -> afficher_r )
    {
	if ( etat_courant -> afficher_r == 1 )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_non_r_etat ),
					   TRUE );
	else
	    if ( etat_courant -> afficher_r )
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_etat ),
					       TRUE );
    }
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat ),
				       TRUE );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ),
				   etat_courant -> afficher_opes );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_opes ),
				   etat_courant -> afficher_nb_opes );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_ope ),
				   etat_courant -> afficher_no_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ),
				   etat_courant -> afficher_date_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ),
				   etat_courant -> afficher_tiers_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ),
				   etat_courant -> afficher_categ_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ),
				   etat_courant -> afficher_sous_categ_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_type_ope ),
				   etat_courant -> afficher_type_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ),
				   etat_courant -> afficher_ib_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ),
				   etat_courant -> afficher_sous_ib_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_cheque ),
				   etat_courant -> afficher_cheque_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ),
				   etat_courant -> afficher_notes_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ),
				   etat_courant -> afficher_pc_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_rappr ),
				   etat_courant -> afficher_rappr_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ),
				   etat_courant -> afficher_infobd_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation ),
				   etat_courant -> pas_detailler_ventilation );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separer_revenus_depenses ),
				   etat_courant -> separer_revenus_depenses );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_exo_opes ),
				   etat_courant -> afficher_exo_ope );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_titres_colonnes ),
				   etat_courant -> afficher_titre_colonnes );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_classement_ope_etat ),
				  etat_courant -> type_classement_ope );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_rendre_ope_clickables ),
				   etat_courant -> ope_clickables );

    if ( !etat_courant -> type_affichage_titres )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_titre_en_haut ),
				       TRUE );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclure_dans_tiers ),
				   etat_courant -> inclure_dans_tiers );

    /* on rend insensitif les sous qque choses si nécessaire */

    sens_desensitive_pointeur ( bouton_afficher_opes,
				frame_onglet_generalites );
    sens_desensitive_pointeur ( bouton_afficher_categ_opes,
				bouton_afficher_sous_categ_opes );
    sens_desensitive_pointeur ( bouton_afficher_ib_opes,
				bouton_afficher_sous_ib_opes );
    sens_desensitive_pointeur ( bouton_afficher_titres_colonnes,
				bouton_titre_changement );
    sens_desensitive_pointeur ( bouton_afficher_titres_colonnes,
				bouton_titre_en_haut );

    /* mise en forme de la devise */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_general_etat ),
				  g_slist_position ( liste_struct_devises,
						     g_slist_find_custom ( liste_struct_devises,
									   GINT_TO_POINTER ( etat_courant -> devise_de_calcul_general ),
									   ( GCompareFunc ) recherche_devise_par_no )));

    /* onglet dates */


    if ( etat_courant -> exo_date )
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ),
				       TRUE );
	gtk_widget_set_sensitive ( vbox_utilisation_date,
				   FALSE );
    }
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_dates ),
				       TRUE );

    sens_desensitive_pointeur ( radio_button_utilise_exo,
				bouton_separe_exo_etat );

    if ( etat_courant -> utilise_detail_exo == 3 )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat ),
				       TRUE );
    else
    {
	gtk_widget_set_sensitive ( vbox_generale_exo_etat,
				   FALSE );

	if ( etat_courant -> utilise_detail_exo )
	{
	    if ( etat_courant -> utilise_detail_exo == 1 )
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_courant ),
					       TRUE );
	    else
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_precedent ),
					       TRUE );
	}
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_tous ),
					   TRUE );
    }


    /* on sélectionne les exercices */

    selectionne_liste_exo_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ),
				   etat_courant -> separation_par_exo );

    /* on sélectionne la plage de date */

    gtk_clist_select_row ( GTK_CLIST ( liste_plages_dates_etat ),
			   etat_courant -> no_plage_date,
			   0 );

    if ( etat_courant -> no_plage_date != 1 )
    {
	gtk_widget_set_sensitive ( entree_date_init_etat,
				   FALSE );
	gtk_widget_set_sensitive ( entree_date_finale_etat,
				   FALSE );
    }
    else
    {
	gtk_widget_set_sensitive ( entree_date_init_etat,
				   TRUE );
	gtk_widget_set_sensitive ( entree_date_finale_etat,
				   TRUE );
    }

    /* on remplit les dates perso si elles existent */

    if ( etat_courant -> date_perso_debut )
	gtk_entry_set_text ( GTK_ENTRY ( entree_date_init_etat ),
			     g_strdup_printf ( "%02d/%02d/%04d",
					       g_date_day ( etat_courant -> date_perso_debut ),
					       g_date_month ( etat_courant -> date_perso_debut ),
					       g_date_year ( etat_courant -> date_perso_debut )));

    if ( etat_courant -> date_perso_fin )
	gtk_entry_set_text ( GTK_ENTRY ( entree_date_finale_etat ),
			     g_strdup_printf ( "%02d/%02d/%04d",
					       g_date_day ( etat_courant -> date_perso_fin ),
					       g_date_month ( etat_courant -> date_perso_fin ),
					       g_date_year ( etat_courant -> date_perso_fin )));

    /* on remplit les détails de la séparation des dates */

    if ( etat_courant -> separation_par_plage )
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat ),
				       TRUE );
	gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				   TRUE );
	modif_type_separation_dates ( GINT_TO_POINTER ( etat_courant -> type_separation_plage ));
    }
    else
    {
	gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				   FALSE );
	gtk_widget_set_sensitive ( bouton_debut_semaine,
				   FALSE );
    }

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ),
				  etat_courant -> type_separation_plage );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_debut_semaine ),
				  etat_courant -> jour_debut_semaine );


    /* onglet comptes */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat ),
				   etat_courant -> utilise_detail_comptes );

    sens_desensitive_pointeur ( bouton_detaille_comptes_etat,
				vbox_generale_comptes_etat );


    selectionne_liste_comptes_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat ),
				   etat_courant -> regroupe_ope_par_compte );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte ),
				   etat_courant -> affiche_sous_total_compte );

    sens_desensitive_pointeur ( bouton_regroupe_ope_compte_etat,
				bouton_affiche_sous_total_compte );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_comptes ),
				   etat_courant -> afficher_nom_compte );


    /* onglet virements */


    if ( etat_courant -> type_virement )
    {
	if ( etat_courant -> type_virement == 1 )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat ),
					   TRUE );
	else
	{
	    if ( etat_courant -> type_virement == 2 )
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat ),
					       TRUE );
	    else
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_perso ),
					       TRUE );
	}
    }
    else
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements ),
				       TRUE );
	gtk_widget_set_sensitive ( bouton_exclure_non_virements_etat,
				   FALSE );
    }

    sens_desensitive_pointeur ( bouton_inclusion_virements_perso,
				hbox_liste_comptes_virements );

    selectionne_liste_virements_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_non_virements_etat ),
				   etat_courant -> exclure_ope_non_virement );


    /* onglet catégories */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_categ_etat ),
				   etat_courant -> utilise_categ );

    sens_desensitive_pointeur ( bouton_utilise_categ_etat,
				bouton_affiche_sous_total_categ );
    sens_desensitive_pointeur ( bouton_utilise_categ_etat,
				bouton_afficher_sous_categ );
    sens_desensitive_pointeur ( bouton_utilise_categ_etat,
				bouton_affiche_sous_total_sous_categ );
    sens_desensitive_pointeur ( bouton_utilise_categ_etat,
				bouton_afficher_pas_de_sous_categ );
    sens_desensitive_pointeur ( bouton_utilise_categ_etat,
				bouton_afficher_noms_categ );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat ),
				   etat_courant -> utilise_detail_categ );

    sens_desensitive_pointeur ( bouton_detaille_categ_etat,
				hbox_detaille_categ_etat );


    /* on sélectionne les catégories choisies */

    selectionne_liste_categ_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_categ ),
				   etat_courant -> exclure_ope_sans_categ );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ ),
				   etat_courant -> affiche_sous_total_categ );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ ),
				   etat_courant -> afficher_sous_categ );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ ),
				   etat_courant -> affiche_sous_total_sous_categ );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_categ ),
				   etat_courant -> afficher_pas_de_sous_categ );


    /* mise en forme de la devise */

    selectionne_devise_categ_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_categ ),
				   etat_courant -> afficher_nom_categ );


    /* onglet ib */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat ),
				   etat_courant -> utilise_ib );

    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_affiche_sous_total_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_afficher_sous_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_affiche_sous_total_sous_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_afficher_pas_de_sous_ib );
    sens_desensitive_pointeur ( bouton_utilise_ib_etat,
				bouton_afficher_noms_ib );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ),
				   etat_courant -> utilise_detail_ib );

    sens_desensitive_pointeur ( bouton_detaille_ib_etat,
				hbox_detaille_ib_etat );


    /* on sélectionne les ib choisies */

    selectionne_liste_ib_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_ib ),
				   etat_courant -> exclure_ope_sans_ib );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ),
				   etat_courant -> affiche_sous_total_ib );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib ),
				   etat_courant -> afficher_sous_ib );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ),
				   etat_courant -> affiche_sous_total_sous_ib );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_ib ),
				   etat_courant -> afficher_pas_de_sous_ib );

    /* mise en forme de la devise */

    selectionne_devise_ib_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_ib ),
				   etat_courant -> afficher_nom_ib );

    /* onglet tiers */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat ),
				   etat_courant -> utilise_tiers );

    sens_desensitive_pointeur ( bouton_utilise_tiers_etat,
				bouton_afficher_noms_tiers );
    sens_desensitive_pointeur ( bouton_utilise_tiers_etat,
				bouton_affiche_sous_total_tiers );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat ),
				   etat_courant -> utilise_detail_tiers );

    sens_desensitive_pointeur ( bouton_detaille_tiers_etat,
				hbox_detaille_tiers_etat );


    /* on sélectionne les tiers choisies */

    selectionne_liste_tiers_etat_courant();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers ),
				   etat_courant -> affiche_sous_total_tiers );

    /* mise en forme de la devise */

    selectionne_devise_tiers_etat_courant ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_tiers ),
				   etat_courant -> afficher_nom_tiers );

    /*  onglet texte */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_texte ),
				   etat_courant -> utilise_texte );
    sens_desensitive_pointeur ( bouton_utilise_texte,
				vbox_generale_textes_etat );
    remplit_liste_comparaisons_textes_etat ();


    /* onglet montant */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_montant ),
				   etat_courant -> utilise_montant );
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_montant_etat ),
				  g_slist_position ( liste_struct_devises,
						     g_slist_find_custom ( liste_struct_devises,
									   GINT_TO_POINTER ( etat_courant -> choix_devise_montant ),
									   ( GCompareFunc ) recherche_devise_par_no )));
    sens_desensitive_pointeur ( bouton_utilise_montant,
				vbox_generale_montants_etat );
    remplit_liste_comparaisons_montants_etat ();

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_nulles_etat ),
				   etat_courant -> exclure_montants_nuls );

    /* onglet modes de paiement */

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_mode_paiement_etat ),
				   etat_courant -> utilise_mode_paiement );

    sens_desensitive_pointeur ( bouton_detaille_mode_paiement_etat,
				vbox_mode_paiement_etat );

    selectionne_liste_modes_paiement_etat_courant ();


    /* on se met sur la bonne page */

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_etats ),
			    1 );

    /* on empêche le changement d'état */

    gtk_widget_set_sensitive ( frame_liste_etats,
			       FALSE );
}
/******************************************************************************/

/******************************************************************************/
void annule_modif_config ( void )
{
    gtk_widget_set_sensitive ( frame_liste_etats,
			       TRUE );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_etats ),
			    0 );
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_exo_etat_courant ( void )
{
    GSList *pointeur_sliste;

    if ( !etat_courant )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_exo_etat ));

    pointeur_sliste = etat_courant -> no_exercices;

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_exo_etat ),
			       g_slist_position ( liste_struct_exercices,
						  g_slist_find_custom ( liste_struct_exercices,
									pointeur_sliste -> data,
									(GCompareFunc) recherche_exercice_par_no )),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_comptes_etat_courant ( void )
{
    GSList *pointeur_sliste;

    if ( !etat_courant )
	return;
    if ( !liste_comptes_etat )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_etat ));

    pointeur_sliste = etat_courant -> no_comptes;

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_comptes_etat ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_comptes_etat ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_virements_etat_courant ( void )
{
    GSList *pointeur_sliste;

    if ( !etat_courant )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_virements ));

    pointeur_sliste = etat_courant -> no_comptes_virements;

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_comptes_virements ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_comptes_virements ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_categ_etat_courant ( void )
{
    GSList *pointeur_sliste;

    if ( !etat_courant )
	return;
    if ( !liste_categ_etat )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_categ_etat ));

    pointeur_sliste = etat_courant -> no_categ;

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_categ_etat ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_categ_etat ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_devise_categ_etat_courant ( void )
{
    if ( !etat_courant )
	return;

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
				  g_slist_position ( liste_struct_devises,
						     g_slist_find_custom ( liste_struct_devises,
									   GINT_TO_POINTER ( etat_courant -> devise_de_calcul_categ ),
									   ( GCompareFunc ) recherche_devise_par_no )));
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_ib_etat_courant ( void )
{
    GSList *pointeur_sliste;

    if ( !etat_courant )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_ib_etat ));

    pointeur_sliste = etat_courant -> no_ib;

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_ib_etat ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_ib_etat ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_devise_ib_etat_courant ( void )
{
    if ( !etat_courant )
	return;

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
				  g_slist_position ( liste_struct_devises,
						     g_slist_find_custom ( liste_struct_devises,
									   GINT_TO_POINTER ( etat_courant -> devise_de_calcul_ib ),
									   ( GCompareFunc ) recherche_devise_par_no )));
}
/******************************************************************************/

/******************************************************************************/
void selectionne_liste_tiers_etat_courant ( void )
{
    GSList *pointeur_sliste;

    if ( !etat_courant )
	return;

    if ( !onglet_config_etat )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_tiers_etat ));

    pointeur_sliste = etat_courant -> no_tiers;

    while ( pointeur_sliste )
    {
	gtk_clist_select_row ( GTK_CLIST ( liste_tiers_etat ),
			       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_tiers_etat ),
							      pointeur_sliste -> data ),
			       0 );
	pointeur_sliste = pointeur_sliste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_devise_tiers_etat_courant ( void )
{
    if ( !etat_courant )
	return;

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
				  g_slist_position ( liste_struct_devises,
						     g_slist_find_custom ( liste_struct_devises,
									   GINT_TO_POINTER ( etat_courant -> devise_de_calcul_tiers ),
									   ( GCompareFunc ) recherche_devise_par_no )));
}
/******************************************************************************/

/******************************************************************************/
void recuperation_info_perso_etat ( void )
{
    GList *pointeur_liste;
    gchar *pointeur_char;
    gint i;
    struct struct_comparaison_montants_etat *comp_montants;
    struct struct_comparaison_textes_etat *comp_textes;


    /* vérification que les dates init et finales sont correctes */

    if ( strlen ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_date_init_etat )))
	 &&
	 !format_date ( entree_date_init_etat ))
    {
	dialogue_error ( _("Invalid custom initial date") );
	return;
    }

    if ( strlen ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_date_finale_etat )))
	 &&
	 !format_date ( entree_date_finale_etat )) {
	dialogue_error ( _("Invalid custom final date") );
	return;
    }


    /* on récupère maintenant toutes les données */
    /* récupération du nom du rapport */

    pointeur_char = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_nom_etat )));

    if ( strlen ( pointeur_char )
	 &&
	 strcmp ( pointeur_char,
		  etat_courant -> nom_etat ))
    {
	etat_courant -> nom_etat = g_strdup ( pointeur_char );

	gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
			     etat_courant -> nom_etat );

	/* on réaffiche la liste des états */

	remplissage_liste_etats ();
    }

    /* récupération du type de classement */

    g_list_free ( etat_courant -> type_classement );

    etat_courant -> type_classement = NULL;

    for ( i=0 ; i<GTK_CLIST ( liste_type_classement_etat ) -> rows ; i++ )
    {
	gint no;

	no = GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( liste_type_classement_etat ),
							     gtk_ctree_node_nth ( GTK_CTREE ( liste_type_classement_etat ),
										  i )));

	etat_courant -> type_classement = g_list_append ( etat_courant -> type_classement,
							  GINT_TO_POINTER ( no ));

	/* rajoute les ss categ et ss ib */

	if ( no == 1 )
	    etat_courant -> type_classement = g_list_append ( etat_courant -> type_classement,
							      GINT_TO_POINTER ( 2 ));
	if ( no == 3 )
	    etat_courant -> type_classement = g_list_append ( etat_courant -> type_classement,
							      GINT_TO_POINTER ( 4 ));
    }

    /* récupération de l'affichage ou non des R */

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat )) )
	etat_courant -> afficher_r = 0;
    else
    {
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_non_r_etat )) )
	    etat_courant -> afficher_r = 1;
	else
	    etat_courant -> afficher_r = 2;
    }

    /* récupération de l'affichage des opés */

    etat_courant -> afficher_opes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ));
    etat_courant -> afficher_nb_opes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_opes ));

    etat_courant -> afficher_no_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_ope ));
    etat_courant -> afficher_date_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ));
    etat_courant -> afficher_tiers_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ));
    etat_courant -> afficher_categ_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ));
    etat_courant -> afficher_sous_categ_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ));
    etat_courant -> afficher_type_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_type_ope ));
    etat_courant -> afficher_ib_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ));
    etat_courant -> afficher_sous_ib_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ));
    etat_courant -> afficher_cheque_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_cheque ));
    etat_courant -> afficher_notes_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ));
    etat_courant -> afficher_pc_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ));
    etat_courant -> afficher_rappr_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_rappr ));
    etat_courant -> afficher_infobd_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ));
    etat_courant -> pas_detailler_ventilation = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation ));
    etat_courant -> separer_revenus_depenses = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separer_revenus_depenses ));
    etat_courant -> afficher_exo_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_exo_opes ));
    etat_courant -> afficher_titre_colonnes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_titres_colonnes ));
    etat_courant -> type_affichage_titres = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_titre_changement ));

    etat_courant -> type_classement_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_classement_ope_etat ) -> menu_item ),
										  "no_classement" ));

    etat_courant -> ope_clickables = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_rendre_ope_clickables ));

    etat_courant -> devise_de_calcul_general = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_general_etat ) -> menu_item ),
										       "no_devise" ));
    etat_courant -> inclure_dans_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclure_dans_tiers ));


    /* récupération des dates */

    etat_courant -> exo_date = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ));

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_tous )) )
	etat_courant -> utilise_detail_exo = 0;
    else
    {
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_courant )) )
	    etat_courant -> utilise_detail_exo = 1;
	else
	{
	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_precedent )) )
		etat_courant -> utilise_detail_exo = 2;
	    else
		etat_courant -> utilise_detail_exo = 3;
	}
    }

    if ( etat_courant -> no_exercices )
    {
	g_slist_free ( etat_courant -> no_exercices );
	etat_courant -> no_exercices = NULL;
    }

    pointeur_liste = GTK_CLIST ( liste_exo_etat ) -> selection;

    while ( pointeur_liste )
    {
	etat_courant -> no_exercices = g_slist_append ( etat_courant -> no_exercices,
							gtk_clist_get_row_data ( GTK_CLIST ( liste_exo_etat ),
										 GPOINTER_TO_INT ( pointeur_liste -> data )));
	pointeur_liste = pointeur_liste -> next;
    }

    /*   si tous les exos ont été sélectionnés, on met bouton_detaille_exo_etat à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_exo_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_exo_etat ) -> rows )
	 &&
	 etat_courant -> utilise_detail_exo == 3 )
    {
	dialogue ( _("All the financial years have been selected. Grisbi will run faster\nwithout the \"Detail financial years\" option") );
	etat_courant -> utilise_detail_exo = FALSE;
    }


    etat_courant -> separation_par_exo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ));

    etat_courant -> no_plage_date = GPOINTER_TO_INT ( GTK_CLIST ( liste_plages_dates_etat ) -> selection -> data );

    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_date_init_etat ))))
	 &&
	 format_date ( entree_date_init_etat ))
    {
	gint jour, mois, annee;

	sscanf ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_date_init_etat ))),
		 "%d/%d/%d",
		 &jour,
		 &mois,
		 &annee );

	etat_courant -> date_perso_debut = g_date_new_dmy ( jour,
							    mois,
							    annee );
    }

    if ( strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_date_finale_etat ))))
	 &&
	 format_date ( entree_date_finale_etat ))
    {
	gint jour, mois, annee;

	sscanf ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_date_finale_etat ))),
		 "%d/%d/%d",
		 &jour,
		 &mois,
		 &annee );

	etat_courant -> date_perso_fin = g_date_new_dmy ( jour,
							  mois,
							  annee );
    }

    etat_courant -> separation_par_plage = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat ));
    etat_courant -> type_separation_plage = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ) -> menu_item ),
										    "type" ));
    etat_courant -> jour_debut_semaine = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_debut_semaine ) -> menu_item ),
										 "jour" ));

    /* récupération des comptes */

    etat_courant -> utilise_detail_comptes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat ));

    if ( etat_courant -> no_comptes )
    {
	g_slist_free ( etat_courant -> no_comptes );
	etat_courant -> no_comptes = NULL;
    }

    pointeur_liste = GTK_CLIST ( liste_comptes_etat ) -> selection;

    while ( pointeur_liste )
    {
	etat_courant -> no_comptes = g_slist_append ( etat_courant -> no_comptes,
						      gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_etat ),
									       GPOINTER_TO_INT ( pointeur_liste -> data )));
	pointeur_liste = pointeur_liste -> next;
    }

    /*   si tous les comptes ont été sélectionnés, on met utilise_detail_comptes à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_comptes_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_comptes_etat ) -> rows )
	 &&
	 etat_courant -> utilise_detail_comptes )
    {
	dialogue ( _("All the accounts have been selected. Grisbi will run faster\nwithout the \"Detail accounts used\" option") );
	etat_courant -> utilise_detail_comptes = FALSE;
    }

    etat_courant -> regroupe_ope_par_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat ));
    etat_courant -> affiche_sous_total_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte ));
    etat_courant -> afficher_nom_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_comptes ));


    /* récupération des virements */

    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat )))
	etat_courant -> type_virement = 1;
    else
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat )))
	    etat_courant -> type_virement = 2;
	else
	{
	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements )))
		etat_courant -> type_virement = 0;
	    else
		etat_courant -> type_virement = 3;
	}

    if ( etat_courant -> no_comptes_virements )
    {
	g_slist_free ( etat_courant -> no_comptes_virements );
	etat_courant -> no_comptes_virements = NULL;
    }

    pointeur_liste = GTK_CLIST ( liste_comptes_virements ) -> selection;

    while ( pointeur_liste )
    {
	etat_courant -> no_comptes_virements = g_slist_append ( etat_courant -> no_comptes_virements,
								gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_virements ),
											 GPOINTER_TO_INT ( pointeur_liste -> data )));
	pointeur_liste = pointeur_liste -> next;
    }

    etat_courant -> exclure_ope_non_virement = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_non_virements_etat ));



    /*   récupération des catégories */

    etat_courant -> utilise_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_categ_etat ));

    etat_courant -> utilise_detail_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat ));

    if ( etat_courant -> no_categ )
    {
	g_slist_free ( etat_courant -> no_categ );
	etat_courant -> no_categ = NULL;
    }

    pointeur_liste = GTK_CLIST ( liste_categ_etat ) -> selection;

    while ( pointeur_liste )
    {
	etat_courant -> no_categ = g_slist_append ( etat_courant -> no_categ,
						    gtk_clist_get_row_data ( GTK_CLIST ( liste_categ_etat ),
									     GPOINTER_TO_INT ( pointeur_liste -> data )));
	pointeur_liste = pointeur_liste -> next;
    }

    /*   si tous les categ ont été sélectionnés, on met utilise_detail_categ à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_categ_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_categ_etat ) -> rows )
	 &&
	 etat_courant -> utilise_detail_categ )
    {
	dialogue ( _("All the categories have been selected. Grisbi will run faster\nwithout  the \"Detail categories used \" option") );
	etat_courant -> utilise_detail_categ = FALSE;
    }

    etat_courant -> exclure_ope_sans_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_categ ));
    etat_courant -> affiche_sous_total_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ ));
    etat_courant -> afficher_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ ));
    etat_courant -> affiche_sous_total_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ ));
    etat_courant -> afficher_pas_de_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_categ ));

    etat_courant -> devise_de_calcul_categ = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_categ_etat ) -> menu_item ),
										     "no_devise" ));
    etat_courant -> afficher_nom_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_categ ));


    /*   récupération des ib */

    etat_courant -> utilise_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat ));

    etat_courant -> utilise_detail_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ));

    if ( etat_courant -> no_ib )
    {
	g_slist_free ( etat_courant -> no_ib );
	etat_courant -> no_ib = NULL;
    }

    pointeur_liste = GTK_CLIST ( liste_ib_etat ) -> selection;

    while ( pointeur_liste )
    {
	etat_courant -> no_ib = g_slist_append ( etat_courant -> no_ib,
						 gtk_clist_get_row_data ( GTK_CLIST ( liste_ib_etat ),
									  GPOINTER_TO_INT ( pointeur_liste -> data )));
	pointeur_liste = pointeur_liste -> next;
    }

    etat_courant -> afficher_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib ));

    /*   si toutes les ib ont été sélectionnés, on met utilise_detail_ib à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_ib_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_ib_etat ) -> rows )
	 &&
	 etat_courant -> utilise_detail_ib )
    {
	dialogue ( _("All budgetary lines have been selected.  Grisbi will run faster\nwithout \"Detail the budgetary lines used\" option") );
	etat_courant -> utilise_detail_ib = FALSE;
    }

    etat_courant -> exclure_ope_sans_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_ib ));
    etat_courant -> affiche_sous_total_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ));
    etat_courant -> affiche_sous_total_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ));
    etat_courant -> afficher_pas_de_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_ib ));

    etat_courant -> devise_de_calcul_ib = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_ib_etat ) -> menu_item ),
										  "no_devise" ));
    etat_courant -> afficher_nom_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_ib ));



    /*   récupération des tiers */

    etat_courant -> utilise_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat ));

    etat_courant -> utilise_detail_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat ));

    if ( etat_courant -> no_tiers )
    {
	g_slist_free ( etat_courant -> no_tiers );
	etat_courant -> no_tiers = NULL;
    }

    pointeur_liste = GTK_CLIST ( liste_tiers_etat ) -> selection;

    while ( pointeur_liste )
    {
	etat_courant -> no_tiers = g_slist_append ( etat_courant -> no_tiers,
						    gtk_clist_get_row_data ( GTK_CLIST ( liste_tiers_etat ),
									     GPOINTER_TO_INT ( pointeur_liste -> data )));
	pointeur_liste = pointeur_liste -> next;
    }

    /*   si tous les tiers ont été sélectionnés, on met utilise_detail_tiers à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_tiers_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_tiers_etat ) -> rows )
	 &&
	 etat_courant -> utilise_detail_tiers )
    {
	dialogue ( _("All the third parts have been selected. Grisbi will run faster\nwithout the \"Detail the third parties\" option") );
	etat_courant -> utilise_detail_tiers = FALSE;
    }

    etat_courant -> affiche_sous_total_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers ));

    etat_courant -> devise_de_calcul_tiers = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ) -> menu_item ),
										     "no_devise" ));
    etat_courant -> afficher_nom_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_tiers ));



    /* récupération du texte */

    etat_courant -> utilise_texte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_texte ));

    /* récupération de la liste des comparaisons de texte */
    /*   il y a au moins une structure de créée, si celle si a l'entrée txt et 2 montants vides, */
    /* c'est qu'il n'y a aucune liste */

    comp_textes = etat_courant -> liste_struct_comparaison_textes -> data;

    if ( g_slist_length ( etat_courant -> liste_struct_comparaison_textes ) == 1
	 &&
	 !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_textes -> entree_txt ))))
	 &&
	 !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_textes -> entree_montant_1 ))))
	 &&
	 !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_textes -> entree_montant_2 ))))
	 &&
	 GTK_WIDGET_SENSITIVE ( comp_textes -> entree_montant_1 )
	 &&
	 GTK_WIDGET_SENSITIVE ( comp_textes -> entree_montant_2 ))
    {
	g_slist_free ( etat_courant -> liste_struct_comparaison_textes );
	etat_courant -> liste_struct_comparaison_textes = NULL;
    }
    else
    {
	/* on a rentré au moins une comparaison */
	/* on rempli les champs de la structure */

	GSList *liste_tmp;

	liste_tmp = etat_courant -> liste_struct_comparaison_textes;

	while ( liste_tmp )
	{
	    comp_textes = liste_tmp -> data;

	    if ( comp_textes -> bouton_lien )
		comp_textes -> lien_struct_precedente = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_textes -> bouton_lien ) -> menu_item ),
												"no_lien" ));
	    else
		comp_textes -> lien_struct_precedente = -1;


	    comp_textes -> champ = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_textes -> bouton_champ ) -> menu_item ),
									   "no_champ" ));
	    comp_textes -> utilise_txt = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( comp_textes -> bouton_utilise_txt ));
	    comp_textes -> operateur = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_textes -> bouton_operateur ) -> menu_item ),
									       "no_operateur" ));
	    comp_textes -> texte = g_strstrip ( g_strdup ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_textes -> entree_txt ))));
	    if ( !strlen ( comp_textes -> texte ))
		comp_textes -> texte = NULL;

	    comp_textes -> comparateur_1 = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_textes -> bouton_comparateur_1 ) -> menu_item ),
										   "no_comparateur" ));
	    comp_textes -> lien_1_2 = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_textes -> bouton_lien_1_2 ) -> menu_item ),
									      "no_lien" ));
	    comp_textes -> comparateur_2 = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_textes -> bouton_comparateur_2 ) -> menu_item ),
										   "no_comparateur" ));
	    comp_textes -> montant_1 = my_atoi ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_textes -> entree_montant_1 )));
	    comp_textes -> montant_2 = my_atoi ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_textes -> entree_montant_2 )));

	    liste_tmp = liste_tmp -> next;
	}
    }



    /* récupération du montant */

    etat_courant -> utilise_montant = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_montant ));
    etat_courant -> choix_devise_montant = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_montant_etat ) -> menu_item ),
										   "no_devise" ));

    /* récupération de la liste des comparaisons de montant */
    /*   il y a au moins une structure de créée, si celle si a les 2 montants vides, */
    /* c'est qu'il n'y a aucune liste */

    comp_montants = etat_courant -> liste_struct_comparaison_montants -> data;

    if ( g_slist_length ( etat_courant -> liste_struct_comparaison_montants ) == 1
	 &&
	 !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_montants -> entree_montant_1 ))))
	 &&
	 !strlen ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_montants -> entree_montant_2 ))))
	 &&
	 GTK_WIDGET_SENSITIVE ( comp_montants -> entree_montant_1 )
	 &&
	 GTK_WIDGET_SENSITIVE ( comp_montants -> entree_montant_2 ))
    {
	g_slist_free ( etat_courant -> liste_struct_comparaison_montants );
	etat_courant -> liste_struct_comparaison_montants = NULL;
    }
    else
    {
	/* on a rentré au moins une comparaison */
	/* on rempli les champs de la structure */

	GSList *liste_tmp;

	liste_tmp = etat_courant -> liste_struct_comparaison_montants;

	while ( liste_tmp )
	{
	    comp_montants = liste_tmp -> data;

	    if ( comp_montants -> bouton_lien )
		comp_montants -> lien_struct_precedente = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_montants -> bouton_lien ) -> menu_item ),
												  "no_lien" ));
	    else
		comp_montants -> lien_struct_precedente = -1;

	    comp_montants -> comparateur_1 = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_montants -> bouton_comparateur_1 ) -> menu_item ),
										     "no_comparateur" ));
	    comp_montants -> lien_1_2 = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_montants -> bouton_lien_1_2 ) -> menu_item ),
										"no_lien" ));
	    comp_montants -> comparateur_2 = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( comp_montants -> bouton_comparateur_2 ) -> menu_item ),
										     "no_comparateur" ));
	    comp_montants -> montant_1 = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_montants -> entree_montant_1 )),
						     NULL );
	    comp_montants -> montant_2 = my_strtod ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( comp_montants -> entree_montant_2 )),
						     NULL );

	    liste_tmp = liste_tmp -> next;
	}
    }

    etat_courant -> exclure_montants_nuls = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_nulles_etat ));

    /* récupération des modes de paiement */

    etat_courant -> utilise_mode_paiement = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_mode_paiement_etat ));

    if ( etat_courant -> noms_modes_paiement )
    {
	g_slist_free ( etat_courant -> noms_modes_paiement );
	etat_courant -> noms_modes_paiement = NULL;
    }

    pointeur_liste = GTK_CLIST ( liste_mode_paiement_etat ) -> selection;

    while ( pointeur_liste )
    {
	etat_courant -> noms_modes_paiement = g_slist_append ( etat_courant -> noms_modes_paiement,
							       gtk_clist_get_row_data ( GTK_CLIST ( liste_mode_paiement_etat ),
											GPOINTER_TO_INT ( pointeur_liste -> data )));
	pointeur_liste = pointeur_liste -> next;
    }

    /*   si tous les modes de paiement ont été sélectionnés, on met utilise_mode_paiement à 0 (plus rapide) */

    if ( ( g_list_length ( GTK_CLIST ( liste_mode_paiement_etat ) -> selection )
	   ==
	   GTK_CLIST ( liste_mode_paiement_etat ) -> rows )
	 &&
	 etat_courant -> utilise_mode_paiement )
    {
	dialogue ( _("All methods of payment have been selected. Grisbi will run faster without the \"Select the transactions based on payment method\" option") );
	etat_courant -> utilise_mode_paiement = FALSE;
    }


    modification_fichier ( TRUE );

    /* on fait une mise à jour des tiers pour afficher l'état dans la liste des tiers si nécessaire */

    mise_a_jour_tiers ();

    /* on réaffiche l'état */

    rafraichissement_etat ( etat_courant );

    /* on repasse à la 1ère page du notebook */

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_etats ),
			    0 );
    gtk_widget_set_sensitive ( frame_liste_etats,
			       TRUE );
}
/******************************************************************************/



/******************************************************************************/
/* vérifie pour la page donnée en argument si le label du notebook doit */
/* être mis d'une couleur différente ou non */
/******************************************************************************/
void stylise_tab_label_etat ( gint *no_page )
{
    GtkStyle *style;

    /* par défaut le style est normal */

    style = style_label_nom_compte;

    switch ( GPOINTER_TO_INT ( no_page ))
    {
	case 0:
	    /* page des dates */
	    /* rien pour l'instant */

	    break;

	case 1:
	    /* page des virements */

	    if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements )))
		style = style_label;
	    break;

	case 2:
	    /* page des comptes */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat )))
		style = style_label;
	    break;

	case 3:
	    /* page des tiers */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat )))
		style = style_label;
	    break;

	case 4:
	    /* page des catégories */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat ))
		 ||
		 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_categ )))
		style = style_label;
	    break;

	case 5:
	    /* page des ib */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ))
		 ||
		 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_ib )))
		style = style_label;
	    break;

	case 6:
	    /* page des textes */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_texte )))
		style = style_label;
	    break;

	case 7:
	    /* page des montants */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_montant ))
		 ||
		 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_nulles_etat )))
		style = style_label;
	    break;

	case 8:
	    /* page des modes de paiement */

	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_mode_paiement_etat )))
		style = style_label;
	    break;

	case 9:
	    /* page des divers */

	    if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat ))
		 ||
		 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation )))
		style = style_label;
	    break;
    }

    gtk_widget_set_style ( gtk_notebook_get_tab_label ( GTK_NOTEBOOK ( notebook_selection ),
							gtk_notebook_get_nth_page ( GTK_NOTEBOOK ( notebook_selection ),
										    GPOINTER_TO_INT ( no_page ))),
			   style );

}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_dates ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *separateur;
    GtkWidget *vbox;
    GtkWidget *scrolled_window;
    gchar **plages_dates;
    gint i;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *frame;
    GtkWidget *hbox_onglet;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    hbox_onglet = gtk_hbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( hbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    hbox_onglet );
    gtk_widget_show ( hbox_onglet );


    /* mise en place de la plage de dates */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    radio_button_utilise_dates = gtk_radio_button_new_with_label ( NULL,
								   _("Use dates ranges") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 radio_button_utilise_dates,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( radio_button_utilise_dates );

    /* on met en dessous une liste avec les plages de date proposées */

    frame = gtk_frame_new (NULL);
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( frame );

    vbox_utilisation_date = gtk_vbox_new ( FALSE,
					   5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_utilisation_date ),
				     10 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox_utilisation_date );
    gtk_widget_show ( vbox_utilisation_date );

    /* on met la connection pour rendre sensitif cette frame */

    gtk_signal_connect ( GTK_OBJECT ( radio_button_utilise_dates ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 vbox_utilisation_date );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );


    liste_plages_dates_etat = gtk_clist_new ( 1 );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_plages_dates_etat ),
				       0,
				       TRUE );
    gtk_signal_connect ( GTK_OBJECT ( liste_plages_dates_etat ),
			 "button_press_event",
			 GTK_SIGNAL_FUNC ( click_liste_etat ),
			 NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_plages_dates_etat );
    gtk_widget_show ( liste_plages_dates_etat );

    /* on remplit la liste des dates */

    plages_dates = liste_plages_dates;

    i = 0;

    while ( plages_dates[i] )
    {
	gint ligne;
	gchar * plage = _(plages_dates[i]);

	ligne = gtk_clist_append ( GTK_CLIST ( liste_plages_dates_etat ),
				   &plage );
	i++;
    }

    /* séparation gauche-droite */

    separateur = gtk_vseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
			 separateur,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( separateur );


    /*   on met en forme la partie de droite : utilisation des exercices */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    radio_button_utilise_exo = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( radio_button_utilise_dates )),
								 _("Use financial years") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 radio_button_utilise_exo,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( radio_button_utilise_exo );

    /* on met la liste des exos sous ce radio button */

    frame = gtk_frame_new ( FALSE );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 frame,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( frame );


    /* on met la connection pour rendre sensitif cette frame */

    gtk_signal_connect ( GTK_OBJECT (radio_button_utilise_exo ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 frame );

    vbox_utilisation_exo = gtk_vbox_new ( FALSE,
					  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_utilisation_exo ),
				     10 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox_utilisation_exo );
    gtk_widget_show ( vbox_utilisation_exo );


    /*   on met le détail, exo courant ou précédent */

    bouton_exo_tous = gtk_radio_button_new_with_label ( NULL,
							_("All financial years"));
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_exo_tous,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exo_tous );

    bouton_exo_courant = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
							   _("Current financial year"));
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_exo_courant,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exo_courant );

    bouton_exo_precedent = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
							     _("Former financial year"));
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_exo_precedent,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exo_precedent );



    bouton_detaille_exo_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
								 _("Detail financial years") );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 bouton_detaille_exo_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_exo_etat );

    vbox_generale_exo_etat = gtk_vbox_new ( FALSE,
					    5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
			 vbox_generale_exo_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_exo_etat );

    gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_exo_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 vbox_generale_exo_etat );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_exo_etat ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_exo_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_exo_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_exo_etat ),
				       0,
				       TRUE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_exo_etat );
    gtk_widget_show ( liste_exo_etat );

    /* on remplit la liste des exercices */

    remplissage_liste_exo_etats ();


    /* on met ensuite la date perso de début */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Initial date")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    entree_date_init_etat = gtk_entry_new_with_max_length ( 10 );
    gtk_widget_set_usize ( entree_date_init_etat,
			   100,
			   FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_date_init_etat ),
			 "button_press_event",
			 GTK_SIGNAL_FUNC ( clique_sur_entree_date_etat ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( entree_date_init_etat ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( pression_touche_date_etat ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( entree_date_init_etat ),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC ( sortie_entree_date_etat ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_date_init_etat,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( entree_date_init_etat );

    /* on met ensuite la date perso de fin */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Final date")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    entree_date_finale_etat = gtk_entry_new_with_max_length ( 10 );
    gtk_widget_set_usize ( entree_date_finale_etat,
			   100,
			   FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_date_finale_etat ),
			 "button_press_event",
			 GTK_SIGNAL_FUNC ( clique_sur_entree_date_etat ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( entree_date_finale_etat ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( pression_touche_date_etat ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( entree_date_finale_etat ),
			 "focus_out_event",
			 GTK_SIGNAL_FUNC ( sortie_entree_date_etat ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_date_finale_etat,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( entree_date_finale_etat );


    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
/* vérifie où l'on clique et empèche la désélection de ligne */
/******************************************************************************/
void click_liste_etat ( GtkCList *liste,
			GdkEventButton *evenement,
			gint origine )
{
    gint colonne, x, y;
    gint ligne;

    /*   origine = 0 si ça vient des dates, dans ce cas on sensitive les entrées init et fin */
    /* origine = 1 si ça vient du choix de type de classement */


    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste ),
				   "button_press_event");

    /* Récupération des coordonnées de la souris */

    gdk_window_get_pointer ( GTK_CLIST ( liste ) -> clist_window,
			     &x,
			     &y,
			     FALSE );

    gtk_clist_get_selection_info ( GTK_CLIST ( liste ),
				   x,
				   y,
				   &ligne,
				   &colonne);

    if ( GTK_CLIST ( liste ) -> selection
	 &&
	 GPOINTER_TO_INT ( GTK_CLIST ( liste ) -> selection -> data ) == ligne )
	return;

    gtk_clist_select_row ( GTK_CLIST ( liste ),
			   ligne,
			   0 );

    if ( !origine )
    {
	if ( ligne == 1 )
	{
	    gtk_widget_set_sensitive ( entree_date_init_etat,
				       TRUE );
	    gtk_widget_set_sensitive ( entree_date_finale_etat,
				       TRUE );
	}
	else
	{
	    gtk_widget_set_sensitive ( entree_date_init_etat,
				       FALSE );
	    gtk_widget_set_sensitive ( entree_date_finale_etat,
				       FALSE );
	}
    }
}
/******************************************************************************/

/******************************************************************************/
gboolean clique_sur_entree_date_etat ( GtkWidget *entree, GdkEventButton *ev )
{
    GtkWidget *popup_cal;

    if ( ev -> type == GDK_2BUTTON_PRESS )
    {
	gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
				       "button-press-event");
	popup_cal = gsb_calendar_new ( entree );
	gtk_signal_connect_object ( GTK_OBJECT ( popup_cal ),
				    "destroy",
				    GTK_SIGNAL_FUNC ( ferme_calendrier ),
				    GTK_OBJECT ( entree ) );
	gtk_widget_grab_focus ( GTK_WIDGET ( popup_cal ) );
    }
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean pression_touche_date_etat ( GtkWidget *widget,
				     GdkEventKey *ev )
{
    GtkWidget *popup_cal;

    switch ( ev -> keyval )
    {
	case GDK_Return :		/* touches entrée */
	case GDK_KP_Enter :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key_press_event");

	    /* si la touche CTRL est elle aussi active, alors c'est que l'on est
	       probablement sur un champ de date et que l'on souhaite ouvrir
	       un calendrier */
	    if ( ( ev -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
	    {
		popup_cal = gsb_calendar_new ( widget );
		gtk_signal_connect_object ( GTK_OBJECT ( popup_cal ),
					    "destroy",
					    GTK_SIGNAL_FUNC ( ferme_calendrier ),
					    GTK_OBJECT ( widget ) );
		gtk_widget_grab_focus ( GTK_WIDGET ( popup_cal ) );
	    }
	    /* si la touche CTRL n'est pas active, alors on valide simplement
	       la saisie de l'échéance */
	    else
	    {
		/* ALAIN-FIXME */
/* 		dialogue("prévoir autre chose"); */
	      return FALSE;
	    }
	    return TRUE;

	case GDK_plus :		/* touches + */
	case GDK_KP_Add :

	    /* si on est dans une entree de date, on augmente d'un jour
	       (d'une semaine) la date */

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		inc_dec_date ( widget, ONE_DAY );
	    else
		inc_dec_date ( widget, ONE_WEEK );
	    return TRUE;

	case GDK_minus :		/* touches - */
	case GDK_KP_Subtract :

	    /* si on est dans une entree de date, on diminue d'un jour
	       (d'une semaine) la date */

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		inc_dec_date ( widget, - ONE_DAY );
	    else
		inc_dec_date ( widget, - ONE_WEEK );
	    return TRUE;

	case GDK_Page_Up :		/* touche PgUp */
	case GDK_KP_Page_Up :

	    /* si on est dans une entree de date,
	       on augmente d'un mois (d'un an) la date */

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		inc_dec_date ( widget, ONE_MONTH );
	    else
		inc_dec_date ( widget, ONE_YEAR );
	    return TRUE;
	    break;

	case GDK_Page_Down :		/* touche PgDown */
	case GDK_KP_Page_Down :

	    /* si on est dans une entree de date,
	       on augmente d'un mois (d'un an) la date */

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( ev -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		inc_dec_date ( widget, - ONE_MONTH );
	    else
		inc_dec_date ( widget, - ONE_YEAR );
	    return TRUE;

	default :
	    /* Reverting to default handler */
	    return FALSE;
    }

    //  return TRUE; c'est le cas des opérations
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean sortie_entree_date_etat ( GtkWidget *entree )
{
    /* si l'entrée contenant la date est vide, alors on met la date du jour */

    if ( strlen ( g_strstrip ( (gchar*) gtk_entry_get_text ( GTK_ENTRY ( entree )))) == 0  )
	gtk_entry_set_text ( GTK_ENTRY ( entree ), gsb_today() );

    format_date ( entree );
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
void change_separation_result_periode ( void )
{
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat )))
    {
	gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				   TRUE );
	modif_type_separation_dates ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_separe_plages_etat ),
							    "type" ));
    }
    else
    {
	gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				   FALSE );
	gtk_widget_set_sensitive ( bouton_debut_semaine,
				   FALSE );
	/*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
	/* 				 FALSE ); */
	/*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
	/* 				 FALSE ); */
    }
}
/******************************************************************************/

/******************************************************************************/
void modif_type_separation_dates ( gint *origine )
{
    switch ( GPOINTER_TO_INT ( origine ))
    {
	case 0:
	    gtk_widget_set_sensitive ( bouton_debut_semaine,
				       TRUE );
	    /*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
	    /* 				 FALSE ); */
	    /*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
	    /* 				 FALSE ); */
	    break;

	case 1:
	case 2:
	    gtk_widget_set_sensitive ( bouton_debut_semaine,
				       FALSE );
	    /*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
	    /* 				 FALSE ); */
	    /*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
	    /* 				 FALSE ); */
	    break;

	case 3:
	    gtk_widget_set_sensitive ( bouton_debut_semaine,
				       FALSE );
	    /*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
	    /* 				 TRUE ); */
	    /*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
	    /* 				 TRUE ); */
    }
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_exo_etats ( void )
{
    GSList *pointeur_liste;

    if ( !onglet_config_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_exo_etat ) );

    pointeur_liste = liste_struct_exercices;

    while ( pointeur_liste )
    {
	struct struct_exercice *exercice;
	gchar *nom[1];
	gint ligne;

	exercice = pointeur_liste -> data;

	nom[0] = exercice -> nom_exercice;

	ligne = gtk_clist_append ( GTK_CLIST ( liste_exo_etat ),
				   nom );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_exo_etat ),
				 ligne,
				 GINT_TO_POINTER ( exercice -> no_exercice ));

	pointeur_liste = pointeur_liste -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_comptes ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    /* on met dans la partie de gauche une liste contenant les comptes à */
    /* sélectionner */

    bouton_detaille_comptes_etat = gtk_check_button_new_with_label ( _("Select transactions only for given accounts"));
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_detaille_comptes_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 2 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_detaille_comptes_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_comptes_etat );

    vbox_generale_comptes_etat = gtk_vbox_new ( FALSE,
						5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_generale_comptes_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_comptes_etat );


    gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_comptes_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 vbox_generale_comptes_etat );

    label = gtk_label_new ( COLON(_("Select the accounts included in the report")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0.1,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_comptes_etat ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_comptes_etat ),
			 hbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_widget_set_usize ( scrolled_window,
			   300,
			   FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 scrolled_window,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_comptes_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_comptes_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_comptes_etat ),
				       0,
				       TRUE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_comptes_etat );
    gtk_widget_show ( liste_comptes_etat );

    /* on remplit la liste des comptes */

    remplissage_liste_comptes_etats ();


    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
				GTK_OBJECT  ( liste_comptes_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				GTK_OBJECT  ( liste_comptes_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select bank accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 0 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select cash accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select liabilities accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 2 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select assets accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_etat ),
				GINT_TO_POINTER ( 3 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_comptes_etats ( void )
{
    gint i;

    if ( !liste_comptes_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_comptes_etat ) );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	gchar *nom[1];
	gint ligne;

	nom[0] = NOM_DU_COMPTE;

	ligne = gtk_clist_append ( GTK_CLIST ( liste_comptes_etat ),
				   nom );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_etat ),
				 ligne,
				 GINT_TO_POINTER ( NO_COMPTE ));
	p_tab_nom_de_compte_variable++;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_partie_liste_compte_etat ( gint *type_compte )
{
    gint i;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_etat ));

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	gint no_compte;

	no_compte = GPOINTER_TO_INT ( gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_etat ),
							       i ));

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

	if ( TYPE_DE_COMPTE == GPOINTER_TO_INT ( type_compte ))
	    gtk_clist_select_row ( GTK_CLIST ( liste_comptes_etat ),
				   i,
				   0 );
    }
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_virements ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *scrolled_window;
    GtkWidget *vbox_onglet;
    GtkWidget *vbox;
    GtkWidget *bouton;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    /*   on met les boutons d'inclusion ou non des virements */

    bouton_non_inclusion_virements = gtk_radio_button_new_with_label ( NULL,
								       _("Do not include transfers") );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_non_inclusion_virements,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_non_inclusion_virements );

    bouton_inclusion_virements_actifs_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									       _("Include transfers from or to assets or liabilities accounts") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_inclusion_virements_actifs_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_inclusion_virements_actifs_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclusion_virements_actifs_etat );

    bouton_inclusion_virements_hors_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									     _("Include transfers from or to accounts not in this report") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_inclusion_virements_hors_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_inclusion_virements_hors_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclusion_virements_hors_etat );

    bouton_inclusion_virements_perso = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									 COLON(POSTSPACIFY(_("Include transfers from or to these accounts"))) );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_inclusion_virements_perso ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_inclusion_virements_perso,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclusion_virements_perso );

    hbox_liste_comptes_virements = gtk_hbox_new ( FALSE,
						  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_liste_comptes_virements,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_liste_comptes_virements );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( hbox_liste_comptes_virements ),
			 scrolled_window,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_comptes_virements = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_comptes_virements ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_comptes_virements ),
				       0,
				       TRUE );
    gtk_widget_set_usize ( liste_comptes_virements,
			   300,
			   FALSE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_comptes_virements );
    gtk_widget_show ( liste_comptes_virements );

    /* on remplit la liste des comptes */

    remplissage_liste_comptes_virements ();


    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_liste_comptes_virements ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
				GTK_OBJECT  ( liste_comptes_virements ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				GTK_OBJECT  ( liste_comptes_virements ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select bank accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 0 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select cash accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 1 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select liabilities accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 2 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Select assets accounts") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( selectionne_partie_liste_compte_vir_etat ),
				GINT_TO_POINTER ( 3 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_perso ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 hbox_liste_comptes_virements );


    /* on rajoute le bouton exclure les opé non virement */

    bouton_exclure_non_virements_etat = gtk_check_button_new_with_label ( _("Exclude the transactions which are not transfers") );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_exclure_non_virements_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exclure_non_virements_etat );

    gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_perso ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_exclure_non_virements_etat );
    gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_hors_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_exclure_non_virements_etat );
    gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_actifs_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_exclure_non_virements_etat );


    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_comptes_virements ( void )
{
    gint i;

    if ( !onglet_config_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_comptes_virements ) );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	gchar *nom[1];
	gint ligne;

	nom[0] = NOM_DU_COMPTE;

	ligne = gtk_clist_append ( GTK_CLIST ( liste_comptes_virements ),
				   nom );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_virements ),
				 ligne,
				 GINT_TO_POINTER ( NO_COMPTE ));
	p_tab_nom_de_compte_variable++;
    }
}
/******************************************************************************/

/******************************************************************************/
void selectionne_partie_liste_compte_vir_etat ( gint *type_compte )
{
    gint i;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_virements ));

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	gint no_compte;

	no_compte = GPOINTER_TO_INT ( gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_virements ),
							       i ));

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

	if ( TYPE_DE_COMPTE == GPOINTER_TO_INT ( type_compte ))
	    gtk_clist_select_row ( GTK_CLIST ( liste_comptes_virements ),
				   i,
				   0 );
    }
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_categories ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );

    /* on propose de détailler les categ utilisées */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_detaille_categ_etat = gtk_check_button_new_with_label ( _("Detail categories") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_detaille_categ_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 4 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_detaille_categ_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_categ_etat );


    /* mise en place de la liste et des boutons de détaillage */

    hbox_detaille_categ_etat = gtk_hbox_new ( FALSE,
					      5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_detaille_categ_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_detaille_categ_etat );

    gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_categ_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 hbox_detaille_categ_etat );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( COLON(_("Select categories to include")) );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_categ_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_categ_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_categ_etat ),
				       0,
				       TRUE );
    gtk_clist_set_compare_func ( GTK_CLIST ( liste_categ_etat ),
				 (GtkCListCompareFunc) classement_alphabetique_tree );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_categ_etat );
    gtk_widget_show ( liste_categ_etat );

    /* on va remplir la liste avec les catégories */

    remplissage_liste_categ_etats ();

    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
				GTK_OBJECT  ( liste_categ_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				GTK_OBJECT  ( liste_categ_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Income categories") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( click_type_categ_etat ),
				NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Outgoing categories") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( click_type_categ_etat ),
				GINT_TO_POINTER (1));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    /* on propose d'exclure les opés sans catégories */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_exclure_ope_sans_categ = gtk_check_button_new_with_label ( _("Exclude transactions with no category") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_exclure_ope_sans_categ ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 4 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_exclure_ope_sans_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exclure_ope_sans_categ );


    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void click_type_categ_etat ( gint type )
{
    /* type est 0 pour les revenus et 1 pour les dépenses */
    /* fait le tour des catég dans la liste et sélectionne celles */
    /* qui correspondent au type recherché */

    gint i;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_categ_etat ));

    for ( i=0 ; i<GTK_CLIST ( liste_categ_etat ) -> rows ; i++ )
    {
	struct struct_categ *categ;

	categ = g_slist_find_custom ( liste_struct_categories,
				      gtk_clist_get_row_data ( GTK_CLIST ( liste_categ_etat ),
							       i ),
				      (GCompareFunc) recherche_categorie_par_no ) -> data;

	if ( categ -> type_categ == type )
	    gtk_clist_select_row ( GTK_CLIST ( liste_categ_etat ),
				   i,
				   0 );
    }
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_categ_etats ( void )
{
    GSList *pointeur_liste;

    if ( !liste_categ_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_categ_etat ) );

    pointeur_liste = liste_struct_categories;

    while ( pointeur_liste )
    {
	struct struct_categ *categ;
	gchar *nom[1];
	gint ligne;

	categ = pointeur_liste -> data;

	nom[0] = categ -> nom_categ;

	ligne = gtk_clist_append ( GTK_CLIST ( liste_categ_etat ),
				   nom );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_categ_etat ),
				 ligne,
				 GINT_TO_POINTER ( categ -> no_categ ));

	pointeur_liste = pointeur_liste -> next;
    }

    gtk_clist_sort ( GTK_CLIST ( liste_categ_etat ));

}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_ib ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    bouton_detaille_ib_etat = gtk_check_button_new_with_label ( _("Detail budgetary lines") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_detaille_ib_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 5 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_detaille_ib_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_ib_etat );


    /* mise en place de la liste et des boutons de détaillage */

    hbox_detaille_ib_etat = gtk_hbox_new ( FALSE,
					   5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_detaille_ib_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_detaille_ib_etat );

    gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_ib_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 hbox_detaille_ib_etat );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( COLON(_("Select the budgetary lines to include in the report")) );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_ib_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_ib_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_ib_etat ),
				       0,
				       TRUE );
    gtk_clist_set_compare_func ( GTK_CLIST ( liste_ib_etat ),
				 (GtkCListCompareFunc) classement_alphabetique_tree );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_ib_etat );
    gtk_widget_show ( liste_ib_etat );

    /* on va remplir la liste avec les ib */

    remplissage_liste_ib_etats ();

    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
				GTK_OBJECT  ( liste_ib_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				GTK_OBJECT  ( liste_ib_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Income budget lines") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( click_type_ib_etat ),
				NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Outgoing budget lines") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( click_type_ib_etat ),
				GINT_TO_POINTER (1));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_exclure_ope_sans_ib = gtk_check_button_new_with_label ( _("Exclude transactions without budgetary line") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_exclure_ope_sans_ib ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 5 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_exclure_ope_sans_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exclure_ope_sans_ib );



    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void click_type_ib_etat ( gint type )
{
    /* type est 0 pour les revenus et 1 pour les dépenses */
    /* fait le tour des catég dans la liste et sélectionne celles */
    /* qui correspondent au type recherché */

    gint i;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_ib_etat ));

    for ( i=0 ; i<GTK_CLIST ( liste_ib_etat ) -> rows ; i++ )
    {
	struct struct_imputation *imputation;

	imputation = g_slist_find_custom ( liste_struct_imputation,
					   gtk_clist_get_row_data ( GTK_CLIST ( liste_ib_etat ),
								    i ),
					   (GCompareFunc) recherche_imputation_par_no ) -> data;

	if ( imputation -> type_imputation == type )
	    gtk_clist_select_row ( GTK_CLIST ( liste_ib_etat ),
				   i,
				   0 );
    }
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_ib_etats ( void )
{
    GSList *pointeur_liste;

    if ( !onglet_config_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_ib_etat ) );

    pointeur_liste = liste_struct_imputation;

    while ( pointeur_liste )
    {
	struct struct_imputation *imputation;
	gchar *nom[1];
	gint ligne;

	imputation = pointeur_liste -> data;

	nom[0] = imputation -> nom_imputation;

	ligne = gtk_clist_append ( GTK_CLIST ( liste_ib_etat ),
				   nom );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_ib_etat ),
				 ligne,
				 GINT_TO_POINTER ( imputation -> no_imputation ));

	pointeur_liste = pointeur_liste -> next;
    }

    gtk_clist_sort ( GTK_CLIST ( liste_ib_etat ));
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_tiers ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *vbox_onglet;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_detaille_tiers_etat = gtk_check_button_new_with_label ( _("Detail third parties") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_detaille_tiers_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 3 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_detaille_tiers_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_tiers_etat );


    /* mise en place de la liste et des boutons de détaillage */

    hbox_detaille_tiers_etat = gtk_hbox_new ( FALSE,
					      5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_detaille_tiers_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox_detaille_tiers_etat );

    gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_tiers_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 hbox_detaille_tiers_etat );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
			 vbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( COLON(_("Select third parties to include in this report")) );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_tiers_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_tiers_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_tiers_etat ),
				       0,
				       TRUE );
    gtk_clist_set_compare_func ( GTK_CLIST ( liste_tiers_etat ),
				 (GtkCListCompareFunc) classement_alphabetique_tree );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_tiers_etat );
    gtk_widget_show ( liste_tiers_etat );

    /* on va remplir la liste avec les tiers */

    remplissage_liste_tiers_etats ();

    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
				GTK_OBJECT  ( liste_tiers_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				GTK_OBJECT  ( liste_tiers_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_tiers_etats ( void )
{
    GSList *pointeur_liste;

    if ( !onglet_config_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_tiers_etat ) );

    pointeur_liste = liste_struct_tiers;

    while ( pointeur_liste )
    {
	struct struct_tiers *tiers;
	gchar *nom[1];
	gint ligne;

	tiers = pointeur_liste -> data;

	nom[0] = tiers -> nom_tiers;

	ligne = gtk_clist_append ( GTK_CLIST ( liste_tiers_etat ),
				   nom );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_tiers_etat ),
				 ligne,
				 GINT_TO_POINTER ( tiers -> no_tiers ));

	pointeur_liste = pointeur_liste -> next;
    }

    gtk_clist_sort ( GTK_CLIST ( liste_tiers_etat ));
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_texte ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;
    GtkWidget *scrolled_window;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );



    /* on commence par créer le choix d'utiliser le montant */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_utilise_texte = gtk_check_button_new_with_label ( _("Select transactions according to the text") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_utilise_texte ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 6 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_utilise_texte,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_utilise_texte );

    vbox_generale_textes_etat = gtk_vbox_new ( FALSE,
					       5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_generale_textes_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_textes_etat );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_texte ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 vbox_generale_textes_etat );


    /* on va ensuite créer la liste qui contiendra les critères */
    /* le remplissage ou la ligne vide se mettent plus tard */

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_textes_etat ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_textes_etat = gtk_vbox_new ( FALSE,
				       5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    liste_textes_etat );
    gtk_widget_show ( liste_textes_etat );

    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void remplit_liste_comparaisons_textes_etat ( void )
{
    GSList *liste_tmp;

    liste_tmp = etat_courant -> liste_struct_comparaison_textes;


    /* commence par effacer l'ancienne liste */

    while ( GTK_BOX ( liste_textes_etat ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( liste_textes_etat ),
			       (( GtkBoxChild *) ( GTK_BOX ( liste_textes_etat ) -> children -> data )) -> widget );


    /*   s'il n'y a rien dans la liste, on met juste une ligne vide */

    if ( !liste_tmp )
    {
	ajoute_ligne_liste_comparaisons_textes_etat ( NULL );
	return;
    }

    /*   on fait le tour de la liste des comparaisons de texte, ajoute une ligne */
    /* et la remplit à chaque fois */

    while ( liste_tmp )
    {
	struct struct_comparaison_textes_etat *comp_textes;

	comp_textes = liste_tmp -> data;

	/* on crée la ligne et remplit les widget de la structure */

	comp_textes -> vbox_ligne = cree_ligne_comparaison_texte ( comp_textes );
	gtk_box_pack_start ( GTK_BOX ( liste_textes_etat ),
			     comp_textes -> vbox_ligne,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( comp_textes -> vbox_ligne );

	/* on remplit maintenant les widget avec les valeurs de la stucture */

	/*       s'il n'y a pas de lien avec la struct précédente, on le vire */
	/* on rajoute le && car parfois le bouton de lien se met quand même en 1ère ligne */

	if ( comp_textes -> lien_struct_precedente != -1
	     &&
	     liste_tmp != etat_courant -> liste_struct_comparaison_textes )
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_textes -> bouton_lien ),
					  comp_textes -> lien_struct_precedente );
	else
	{
	    gtk_widget_destroy ( comp_textes -> bouton_lien );
	    comp_textes -> bouton_lien = NULL;
	}


	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_textes -> bouton_champ ),
				      comp_textes -> champ );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_textes -> bouton_operateur ),
				      comp_textes -> operateur );
	if ( comp_textes -> texte )
	    gtk_entry_set_text ( GTK_ENTRY ( comp_textes -> entree_txt ),
				 comp_textes -> texte );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_textes -> bouton_comparateur_1 ),
				      comp_textes -> comparateur_1 );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_textes -> bouton_lien_1_2 ),
				      comp_textes -> lien_1_2 );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_textes -> bouton_comparateur_2 ),
				      comp_textes -> comparateur_2 );
	gtk_entry_set_text ( GTK_ENTRY ( comp_textes -> entree_montant_1 ),
			     itoa ( comp_textes -> montant_1 ));
	gtk_entry_set_text ( GTK_ENTRY ( comp_textes -> entree_montant_2 ),
			     itoa ( comp_textes -> montant_2 ));

	if ( comp_textes -> utilise_txt )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( comp_textes -> bouton_utilise_txt ),
					   TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( comp_textes -> bouton_utilise_no ),
					   TRUE );


	/* on désensitive tous ce qui est nécessaire */

	if ( comp_textes -> champ == 8
	     ||
	     comp_textes -> champ == 9
	     ||
	     comp_textes -> champ == 10 )
	{
	    /* 	  on est sur un chq ou une pc */
	    /* on rend sensitif les check button et la hbox correspondante */

	    sensitive_widget ( comp_textes -> bouton_utilise_txt );
	    sensitive_widget ( comp_textes -> bouton_utilise_no );
	    sensitive_hbox_fonction_bouton_txt ( comp_textes );
	}
	else
	{
	    desensitive_widget ( comp_textes -> bouton_utilise_txt );
	    desensitive_widget ( comp_textes -> bouton_utilise_no );
	    desensitive_widget ( comp_textes -> hbox_chq );
	    sensitive_widget ( comp_textes -> hbox_txt );
	}
	/* on sensitive/désensitive l'entrée txt*/

	if ( comp_textes -> operateur >= 4 )
	    desensitive_widget ( comp_textes -> entree_txt );

	/* on sensitive/désensitive les entrées de montant si nécessaire */

	if ( comp_textes -> comparateur_1 == 6 )
	    desensitive_widget ( comp_textes -> entree_montant_1 );

	if ( comp_textes -> comparateur_2 == 6 )
	    desensitive_widget ( comp_textes -> entree_montant_2 );

	/* on sensitive/désensitive la hbox_2 si nécessaire */

	if ( comp_textes -> lien_1_2 == 3 )
	    desensitive_widget ( comp_textes -> hbox_partie_2 );

	liste_tmp = liste_tmp -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction ajoute une ligne vierge */
/* si ancien_comp_textes n'est pas nul, la ligne est insérée juste après celle de l'argument */
/******************************************************************************/

void ajoute_ligne_liste_comparaisons_textes_etat ( struct struct_comparaison_textes_etat *ancien_comp_textes )
{
    struct struct_comparaison_textes_etat *comp_textes;
    gint position;

    /* on récupère tout de suite la position à laquelle il faut insérer la ligne */

    if ( ancien_comp_textes )
	position = g_slist_index ( etat_courant -> liste_struct_comparaison_textes,
				   ancien_comp_textes ) + 1;
    else
	position = 0;

    /* on commence par créer une structure vide */

    comp_textes = calloc ( 1,
			   sizeof ( struct struct_comparaison_textes_etat ));


    /* on crée la ligne et remplit les widget de la structure */

    comp_textes -> vbox_ligne = cree_ligne_comparaison_texte ( comp_textes );
    gtk_box_pack_start ( GTK_BOX ( liste_textes_etat ),
			 comp_textes -> vbox_ligne,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> vbox_ligne );

    /* on vire le lien de la ligne s'il n'y a pas encore de liste */
    /*   (cad si c'est la 1ère ligne) */

    if ( !etat_courant -> liste_struct_comparaison_textes )
    {
	gtk_widget_destroy ( comp_textes -> bouton_lien );
	comp_textes -> bouton_lien = NULL;
    }

    /*   par défaut, le bouton bouton_lien_1_2 est sur stop */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_textes -> bouton_lien_1_2 ),
				  3 );
    comp_textes -> lien_1_2 = 3;
    gtk_widget_set_sensitive ( comp_textes -> hbox_partie_2,
			       FALSE );


    /*   par défaut, la ligne de chq est non sensitive */

    gtk_widget_set_sensitive ( comp_textes -> bouton_utilise_txt,
			       FALSE );
    gtk_widget_set_sensitive ( comp_textes -> bouton_utilise_no,
			       FALSE );
    gtk_widget_set_sensitive ( comp_textes -> hbox_chq,
			       FALSE );


    /* on met la structure dans la liste à la position demandée */

    etat_courant -> liste_struct_comparaison_textes = g_slist_insert ( etat_courant -> liste_struct_comparaison_textes,
								       comp_textes,
								       position );


    /* on met la ligne à sa place dans la liste */

    gtk_box_reorder_child ( GTK_BOX ( liste_textes_etat ),
			    comp_textes -> vbox_ligne,
			    position );
}
/******************************************************************************/

/******************************************************************************/
/* crée la hbox de la ligne et la renvoie */
/* remplie en même temps les widget de la struct envoyée en argument, sauf hbox_ligne */
/******************************************************************************/
GtkWidget *cree_ligne_comparaison_texte ( struct struct_comparaison_textes_etat *comp_textes )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *bouton;
    GtkWidget *hbox;

    /*   on laisse les infos vides, on met juste les boutons et les labels */

    vbox = gtk_vbox_new ( FALSE,
			  5 );


    /* création de la 1ère ligne */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    comp_textes -> bouton_lien = cree_bouton_lien_lignes_comparaison ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_textes -> bouton_lien,
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( POSTSPACIFY(_("Transactions whose")));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    /*   avant de créer le bouton des champs, on doit créer hbox_txt, hbox_chq et les 2 check button */

    comp_textes -> hbox_txt = gtk_hbox_new ( FALSE,
					     5 );
    comp_textes -> hbox_chq = gtk_hbox_new ( FALSE,
					     5 );
    comp_textes -> bouton_utilise_txt = gtk_radio_button_new ( NULL );
    comp_textes -> bouton_utilise_no = gtk_radio_button_new ( gtk_radio_button_group ( GTK_RADIO_BUTTON( comp_textes -> bouton_utilise_txt )));

    comp_textes -> bouton_champ = cree_bouton_champ ( comp_textes );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_textes -> bouton_champ,
			 FALSE,
			 FALSE,
			 0 );

    /* la suite se met dans hbox_txt */
    /* en 2ème ligne */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( "" );
    gtk_widget_set_usize ( label,
			   150,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    /* on met le check button utilisé en cas de champ à no */

    gtk_signal_connect ( GTK_OBJECT ( comp_textes -> bouton_utilise_txt ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 comp_textes -> hbox_txt );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_textes -> bouton_utilise_txt,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> bouton_utilise_txt );


    /* on met maintenant le comparateur txt */

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_textes -> hbox_txt,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> hbox_txt );

    /* avant de créer l'opérateur, on doit créer l'entrée de txt */

    comp_textes -> entree_txt = gtk_entry_new ();

    comp_textes -> bouton_operateur = cree_bouton_operateur_txt ( comp_textes );
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_txt ),
			 comp_textes -> bouton_operateur,
			 FALSE,
			 FALSE,
			 0 );

    /* on peut maintenant mettre l'entrée de txt */

    gtk_widget_set_usize ( comp_textes -> entree_txt,
			   150,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_txt ),
			 comp_textes -> entree_txt,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> entree_txt );


    /* on crée maintenant la 2ème ligne qui concerne les tests de chq */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( "" );
    gtk_widget_set_usize ( label,
			   150,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    /* on met le check button utilisé en cas de champ à no */

    gtk_signal_connect ( GTK_OBJECT ( comp_textes -> bouton_utilise_no ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 comp_textes -> hbox_chq );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_textes -> bouton_utilise_no,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> bouton_utilise_no );


    /* mise en place de la hbox des montants de chq */

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_textes -> hbox_chq,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> hbox_chq );


    label = gtk_label_new ( POSTSPACIFY(_("is")));
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_chq ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    comp_textes -> bouton_comparateur_1 = cree_bouton_comparateur_texte ( comp_textes );
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_chq ),
			 comp_textes -> bouton_comparateur_1,
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( SPACIFY(_("to")));
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_chq ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    comp_textes -> entree_montant_1 = gtk_entry_new ();
    gtk_widget_set_usize ( comp_textes -> entree_montant_1,
			   100,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_chq ),
			 comp_textes -> entree_montant_1,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> entree_montant_1 );


    /* la fonction cree_bouton_lien_montant va se servir de comp_montants -> hbox_partie_2 */
    /* il faut donc créer celle ci avant l'appel de la fonction */

    comp_textes -> hbox_partie_2 = gtk_hbox_new ( FALSE,
						  5 );

    /* on crée alors le bouton de lien */

    comp_textes -> bouton_lien_1_2 = cree_bouton_lien ( comp_textes -> hbox_partie_2 );
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_chq ),
			 comp_textes -> bouton_lien_1_2,
			 FALSE,
			 FALSE,
			 0 );

    /* on ajoute la hbox2 */

    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_chq ),
			 comp_textes -> hbox_partie_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> hbox_partie_2 );

    /* on peut maintenant ajouter dans comp_montants -> hbox_partie_2 */

    comp_textes -> bouton_comparateur_2 = cree_bouton_comparateur_texte ( comp_textes );
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_partie_2 ),
			 comp_textes -> bouton_comparateur_2,
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( SPACIFY(_("to")));
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_partie_2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    comp_textes -> entree_montant_2 = gtk_entry_new ();
    gtk_widget_set_usize ( comp_textes -> entree_montant_2,
			   100,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( comp_textes -> hbox_partie_2 ),
			 comp_textes -> entree_montant_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_textes -> entree_montant_2 );

    /* on met les bouton ajouter et supprimer */

    bouton = gtk_button_new_with_label ( _("Add"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( ajoute_ligne_liste_comparaisons_textes_etat ),
				(GtkObject *) comp_textes );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Remove"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( retire_ligne_liste_comparaisons_textes_etat ),
				(GtkObject *) comp_textes );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( vbox );
}
/******************************************************************************/

/******************************************************************************/
void retire_ligne_liste_comparaisons_textes_etat ( struct struct_comparaison_textes_etat *ancien_comp_textes )
{
    /* il faut qu'il y ai plus d'une ligne affichée */

    if ( g_slist_length ( etat_courant -> liste_struct_comparaison_textes ) < 2 )
	return;

    /* on commence par supprimer la ligne dans la liste */

    gtk_widget_destroy ( ancien_comp_textes -> vbox_ligne );

    /* si la structure qu'on retire est la 1ère, on vire le widget de lient */

    if ( !g_slist_index ( etat_courant -> liste_struct_comparaison_textes,
			  ancien_comp_textes ))
    {
	struct struct_comparaison_textes_etat *comp_textes;

	comp_textes = etat_courant -> liste_struct_comparaison_textes -> next -> data;
	gtk_widget_destroy ( comp_textes -> bouton_lien );
	comp_textes -> bouton_lien = NULL;
    }

    /* et on retire la struct de la sliste */

    etat_courant -> liste_struct_comparaison_textes = g_slist_remove ( etat_courant -> liste_struct_comparaison_textes,
								       ancien_comp_textes );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec les noms de champs de txt, et rend (non-)sensitif */
/* la hbox correspondante ( txt ou chq ) */
/* il faut donc que hbox_txt,  hbox_chq et les 2 check button soient déjà créées */
/******************************************************************************/
GtkWidget *cree_bouton_champ ( struct struct_comparaison_textes_etat *comp_textes )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    /*   pour chaque item, on désensitive les check button et la ligne des tests de no, et */
    /* on rend sensitif la ligne des test en txt */
    /*     sauf pour les items à no (chq et pc) où on rend sensitif les check button et la */
    /*     ligne correspondans au check button, et désensitive celle où le check button n'est pas */
    /* mis */

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("the third party"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 0 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the third party information"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 1 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the category"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 2 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the sub-category"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 3 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the budgetary line"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 4 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the sub-budgetary line"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 5 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the note"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 6 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the bank reference"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 7 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( gtk_toggle_button_set_active ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> hbox_chq ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the voucher"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 8 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_hbox_fonction_bouton_txt ),
				(GtkObject *) comp_textes );
    gtk_widget_show ( menu_item );


    menu_item = gtk_menu_item_new_with_label ( _("the cheque number"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 9 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_hbox_fonction_bouton_txt ),
				(GtkObject *) comp_textes );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the reconciliation reference"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_champ",
			  GINT_TO_POINTER ( 10 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_txt ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> bouton_utilise_no ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_hbox_fonction_bouton_txt ),
				(GtkObject *) comp_textes );
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction est appellée lorsqu'on sélectionne un champ de texte à no */
/* elle rend sensitif la hbox correspondant au check button */
/******************************************************************************/
void sensitive_hbox_fonction_bouton_txt ( struct struct_comparaison_textes_etat *comp_textes )
{
    sens_desensitive_pointeur ( comp_textes -> bouton_utilise_txt,
				comp_textes -> hbox_txt );
    sens_desensitive_pointeur ( comp_textes -> bouton_utilise_no,
				comp_textes -> hbox_chq );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec contient, ne contient pas ... */
/******************************************************************************/
GtkWidget *cree_bouton_operateur_txt ( struct struct_comparaison_textes_etat *comp_textes )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("contains"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 0 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> entree_txt ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("doesn't contain"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 1 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> entree_txt ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("begins with"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 2 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> entree_txt ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("ends with"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 3 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( comp_textes -> entree_txt ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("is empty"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 4 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> entree_txt ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("isn't empty"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_operateur",
			  GINT_TO_POINTER ( 5 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( comp_textes -> entree_txt ));
    gtk_widget_show ( menu_item );


    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_montant ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;
    GtkWidget *scrolled_window;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );


    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );



    /* on commence par créer le choix d'utiliser le montant */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_utilise_montant = gtk_check_button_new_with_label ( _("Select the transactions by amount") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_utilise_montant ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 7 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_utilise_montant,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_utilise_montant );

    vbox_generale_montants_etat = gtk_vbox_new ( FALSE,
						 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_generale_montants_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_generale_montants_etat );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_montant ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 vbox_generale_montants_etat );



    /* on va ensuite créer la liste qui contiendra les critères */
    /* le remplissage ou la ligne vide se mettent plus tard */

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox_generale_montants_etat ),
			 scrolled_window,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_montants_etat = gtk_vbox_new ( FALSE,
					 5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    liste_montants_etat );
    gtk_widget_show ( liste_montants_etat );


    /* on rajoute le bouton exclure les opé dont le montant est nul */

    bouton_exclure_ope_nulles_etat = gtk_check_button_new_with_label ( _("Exclude the transactions with a null amount") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_exclure_ope_nulles_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 7 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_exclure_ope_nulles_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_exclure_ope_nulles_etat );


    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void remplit_liste_comparaisons_montants_etat ( void )
{
    GSList *liste_tmp;

    liste_tmp = etat_courant -> liste_struct_comparaison_montants;


    /* commence par effacer l'ancienne liste */

    while ( GTK_BOX ( liste_montants_etat ) -> children )
	gtk_container_remove ( GTK_CONTAINER ( liste_montants_etat ),
			       (( GtkBoxChild *) ( GTK_BOX ( liste_montants_etat ) -> children -> data )) -> widget );


    /*   s'il n'y a rien dans la liste, on met juste une ligne vide */

    if ( !liste_tmp )
    {
	ajoute_ligne_liste_comparaisons_montants_etat ( NULL );
	return;
    }

    /*   on fait le tour de la liste des comparaisons de montant, ajoute une ligne */
    /* et la remplit à chaque fois */

    while ( liste_tmp )
    {
	struct struct_comparaison_montants_etat *comp_montants;

	comp_montants = liste_tmp -> data;

	/* on crée la ligne et remplit les widget de la structure */

	comp_montants -> hbox_ligne = cree_ligne_comparaison_montant ( comp_montants );
	gtk_box_pack_start ( GTK_BOX ( liste_montants_etat ),
			     comp_montants -> hbox_ligne,
			     FALSE,
			     FALSE,
			     0 );
	gtk_widget_show ( comp_montants -> hbox_ligne );

	/* on remplit maintenant les widget avec les valeurs de la stucture */

	/*       s'il n'y a pas de lien avec la struct précédente, on le vire */
	/* on rajoute le && car parfois le bouton de lien se met quand même en 1ère ligne */

	if ( comp_montants -> lien_struct_precedente != -1
	     &&
	     liste_tmp != etat_courant -> liste_struct_comparaison_montants )
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_montants -> bouton_lien ),
					  comp_montants -> lien_struct_precedente );
	else
	{
	    gtk_widget_destroy ( comp_montants -> bouton_lien );
	    comp_montants -> bouton_lien = NULL;
	}


	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_montants -> bouton_comparateur_1 ),
				      comp_montants -> comparateur_1 );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_montants -> bouton_lien_1_2 ),
				      comp_montants -> lien_1_2 );
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_montants -> bouton_comparateur_2 ),
				      comp_montants -> comparateur_2 );
	gtk_entry_set_text ( GTK_ENTRY ( comp_montants -> entree_montant_1 ),
			     g_strdup_printf ( "%4.2f",
					       comp_montants -> montant_1 ));
	gtk_entry_set_text ( GTK_ENTRY ( comp_montants -> entree_montant_2 ),
			     g_strdup_printf ( "%4.2f",
					       comp_montants -> montant_2 ));

	/* on sensitive/désensitive les entrées si nécessaire */

	if ( comp_montants -> comparateur_1 > 5 )
	    desensitive_widget ( comp_montants -> entree_montant_1 );
	else
	    sensitive_widget ( comp_montants -> entree_montant_1 );

	if ( comp_montants -> comparateur_2 > 5 )
	    desensitive_widget ( comp_montants -> entree_montant_2 );
	else
	    sensitive_widget ( comp_montants -> entree_montant_2 );


	/* on sensitive/désensitive la hbox_2 si nécessaire */

	if ( comp_montants -> lien_1_2 == 3 )
	    desensitive_widget ( comp_montants -> hbox_partie_2 );
	else
	    sensitive_widget ( comp_montants -> hbox_partie_2 );

	liste_tmp = liste_tmp -> next;
    }
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction ajoute une ligne vierge */
/* si ancien_comp_montants n'est pas nul, la ligne est insérée juste après celle de l'argument */
/******************************************************************************/

void ajoute_ligne_liste_comparaisons_montants_etat ( struct struct_comparaison_montants_etat *ancien_comp_montants )
{
    struct struct_comparaison_montants_etat *comp_montants;
    gint position;

    /* on récupère tout de suite la position à laquelle il faut insérer la ligne */

    if ( ancien_comp_montants )
	position = g_slist_index ( etat_courant -> liste_struct_comparaison_montants,
				   ancien_comp_montants ) + 1;
    else
	position = 0;

    /* on commence par créer une structure vide */

    comp_montants = calloc ( 1,
			     sizeof ( struct struct_comparaison_montants_etat ));


    /* on crée la ligne et remplit les widget de la structure */

    comp_montants -> hbox_ligne = cree_ligne_comparaison_montant ( comp_montants );
    gtk_box_pack_start ( GTK_BOX ( liste_montants_etat ),
			 comp_montants -> hbox_ligne,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_montants -> hbox_ligne );

    /* on vire le lien de la ligne s'il n'y a pas encore de liste */
    /*   (cad si c'est la 1ère ligne) */

    if ( !etat_courant -> liste_struct_comparaison_montants )
    {
	gtk_widget_destroy ( comp_montants -> bouton_lien );
	comp_montants -> bouton_lien = NULL;
    }

    /*   par défaut, le bouton bouton_lien_1_2 est sur stop */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( comp_montants -> bouton_lien_1_2 ),
				  3 );
    comp_montants -> lien_1_2 = 3;
    gtk_widget_set_sensitive ( comp_montants -> hbox_partie_2,
			       FALSE );

    /* on met la structure dans la liste à la position demandée */

    etat_courant -> liste_struct_comparaison_montants = g_slist_insert ( etat_courant -> liste_struct_comparaison_montants,
									 comp_montants,
									 position );


    /* on met la ligne à sa place dans la liste */

    gtk_box_reorder_child ( GTK_BOX ( liste_montants_etat ),
			    comp_montants -> hbox_ligne,
			    position );
}
/******************************************************************************/

/******************************************************************************/
/* crée la hbox de la ligne et la renvoie */
/* remplie en même temps les widget de la struct envoyée en argument, sauf hbox_ligne */
/******************************************************************************/
GtkWidget *cree_ligne_comparaison_montant ( struct struct_comparaison_montants_etat *comp_montants )
{
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *bouton;

    /*   on laisse les infos vides, on met juste les boutons et les labels */

    hbox = gtk_hbox_new ( FALSE,
			  5 );

    comp_montants -> bouton_lien = cree_bouton_lien_lignes_comparaison ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_montants -> bouton_lien,
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( POSTSPACIFY(_("Transactions with an amount of")));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    comp_montants -> bouton_comparateur_1 = cree_bouton_comparateur_montant ( comp_montants );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_montants -> bouton_comparateur_1,
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( SPACIFY(_("to")));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    comp_montants -> entree_montant_1 = gtk_entry_new ();
    gtk_widget_set_usize ( comp_montants -> entree_montant_1,
			   50,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_montants -> entree_montant_1,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_montants -> entree_montant_1 );


    /* la fonction cree_bouton_lien va se servir de comp_montants -> hbox_partie_2 */
    /* il faut donc créer celle ci avant l'appel de la fonction */

    comp_montants -> hbox_partie_2 = gtk_hbox_new ( FALSE,
						    5 );

    /* on crée alors le bouton de lien */

    comp_montants -> bouton_lien_1_2 = cree_bouton_lien ( comp_montants -> hbox_partie_2 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_montants -> bouton_lien_1_2,
			 FALSE,
			 FALSE,
			 0 );

    /* on ajoute la hbox2 */

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 comp_montants -> hbox_partie_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_montants -> hbox_partie_2 );

    /* on peut maintenant ajouter dans comp_montants -> hbox_partie_2 */

    comp_montants -> bouton_comparateur_2 = cree_bouton_comparateur_montant ( comp_montants );
    gtk_box_pack_start ( GTK_BOX ( comp_montants -> hbox_partie_2 ),
			 comp_montants -> bouton_comparateur_2,
			 FALSE,
			 FALSE,
			 0 );

    label = gtk_label_new ( SPACIFY(_("to")));
    gtk_box_pack_start ( GTK_BOX ( comp_montants -> hbox_partie_2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    comp_montants -> entree_montant_2 = gtk_entry_new ();
    gtk_widget_set_usize ( comp_montants -> entree_montant_2,
			   50,
			   FALSE );
    gtk_box_pack_start ( GTK_BOX ( comp_montants -> hbox_partie_2 ),
			 comp_montants -> entree_montant_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( comp_montants -> entree_montant_2 );

    /* on met les bouton ajouter et supprimer */

    bouton = gtk_button_new_with_label ( _("Add"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( ajoute_ligne_liste_comparaisons_montants_etat ),
				(GtkObject *) comp_montants );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Remove"));
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( retire_ligne_liste_comparaisons_montants_etat ),
				(GtkObject *) comp_montants );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    return ( hbox );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec et/ou/sauf */
/******************************************************************************/
GtkWidget *cree_bouton_lien_lignes_comparaison ( void )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("and"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 0 ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("or"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 1 ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("except"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 2 ));
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec =,<,<=,>,>=, nul, non nul, positif, nég */
/******************************************************************************/
GtkWidget *cree_bouton_comparateur_montant ( struct struct_comparaison_montants_etat *comp_montants )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("equal"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 0 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 1 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than or equal"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 2 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 3 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than or equal"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 4 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("different from"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 5 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("null"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 6 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("not null"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 7 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("positive"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 8 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("negative"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 9 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_montant ),
			 comp_montants );
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction rend sensitive ou non l'entrée après le label après l'option menu du menu_item en argument */
/* donc si on choisit nul, non nul, pos ou nég, on grise l'entrée */
/******************************************************************************/
void change_comparaison_montant ( GtkWidget *menu_item,
				  struct struct_comparaison_montants_etat *comp_montants )
{
    gint sensitif;

    switch ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( menu_item ),
						     "no_comparateur" )))
    {
	case 6:
	case 7:
	case 8:
	case 9:

	    sensitif = 0;
	    break;

	default:

	    sensitif = 1;
    }

    if ( gtk_menu_get_attach_widget ( GTK_MENU ( menu_item -> parent )) == comp_montants -> bouton_comparateur_1 )
	gtk_widget_set_sensitive ( comp_montants -> entree_montant_1,
				   sensitif );
    else
	gtk_widget_set_sensitive ( comp_montants -> entree_montant_2,
				   sensitif );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec =,<,<=,>,>= */
/******************************************************************************/
GtkWidget *cree_bouton_comparateur_texte ( struct struct_comparaison_textes_etat *comp_textes )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("equal"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 0 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_texte ),
			 comp_textes );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 1 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_texte ),
			 comp_textes );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("less than or equal"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 2 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_texte ),
			 comp_textes );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 3 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_texte ),
			 comp_textes );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("greater than or equal"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 4 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_texte ),
			 comp_textes );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("different from"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 5 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_texte ),
			 comp_textes );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("the biggest"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_comparateur",
			  GINT_TO_POINTER ( 6 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			 "activate",
			 GTK_SIGNAL_FUNC ( change_comparaison_texte ),
			 comp_textes );
    gtk_widget_show ( menu_item );


    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    return ( bouton );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction rend sensitive ou non l'entrée après le label après l'option menu du menu_item en argument */
/* donc si on choisit nul, non nul, pos ou nég, on grise l'entrée */
/******************************************************************************/
void change_comparaison_texte ( GtkWidget *menu_item,
				struct struct_comparaison_textes_etat *comp_textes )
{
    gint sensitif;

    if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( menu_item ),
						 "no_comparateur" )) == 6 )
	sensitif = 0;
    else
	sensitif = 1;


    if ( gtk_menu_get_attach_widget ( GTK_MENU ( menu_item -> parent )) == comp_textes -> bouton_comparateur_1 )
	gtk_widget_set_sensitive ( comp_textes -> entree_montant_1,
				   sensitif );
    else
	gtk_widget_set_sensitive ( comp_textes -> entree_montant_2,
				   sensitif );
}
/******************************************************************************/

/******************************************************************************/
/* cette fonction crée un option_menu avec et/ou/aucun */
/******************************************************************************/

GtkWidget *cree_bouton_lien ( GtkWidget *hbox )
{
    GtkWidget *bouton;
    GtkWidget *menu;
    GtkWidget *menu_item;

    bouton = gtk_option_menu_new ();
    gtk_widget_show ( bouton );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("and"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 0 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("or"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 1 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("except"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 2 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( sensitive_widget ),
				GTK_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("stop"));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_lien",
			  GINT_TO_POINTER ( 3 ));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( desensitive_widget ),
				GTK_OBJECT ( hbox ));
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       menu );
    gtk_widget_show ( menu );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton ),
				  3 );
    return ( bouton );
}
/******************************************************************************/



/******************************************************************************/
void retire_ligne_liste_comparaisons_montants_etat ( struct struct_comparaison_montants_etat *ancien_comp_montants )
{
    /* il faut qu'il y ai plus d'une ligne affichée */

    if ( g_slist_length ( etat_courant -> liste_struct_comparaison_montants ) < 2 )
	return;

    /* on commence par supprimer la ligne dans la liste */

    gtk_widget_destroy ( ancien_comp_montants -> hbox_ligne );

    /* si la structure qu'on retire est la 1ère, on vire le widget de lient */

    if ( !g_slist_index ( etat_courant -> liste_struct_comparaison_montants,
			  ancien_comp_montants ))
    {
	struct struct_comparaison_montants_etat *comp_montants;

	comp_montants = etat_courant -> liste_struct_comparaison_montants -> next -> data;
	gtk_widget_destroy ( comp_montants -> bouton_lien );
	comp_montants -> bouton_lien = NULL;
    }

    /* et on retire la struct de la sliste */

    etat_courant -> liste_struct_comparaison_montants = g_slist_remove ( etat_courant -> liste_struct_comparaison_montants,
									 ancien_comp_montants );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_divers ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *vbox_onglet;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkWidget *label;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );

    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    /* on peut sélectionner les opé R ou non R */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    frame = gtk_frame_new ( _("Reconciled transactions"));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( _("Select") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    bouton_opes_r_et_non_r_etat = gtk_radio_button_new_with_label ( NULL,
								    _("All transactions") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_opes_r_et_non_r_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_opes_r_et_non_r_etat );

    bouton_opes_non_r_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_opes_r_et_non_r_etat )),
							       _("Non reconciled transactions") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_opes_non_r_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 9 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_opes_non_r_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_opes_non_r_etat );

    bouton_opes_r_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_opes_r_et_non_r_etat )),
							   _("The reconciled transactions") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_opes_r_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 9 ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_opes_r_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_opes_r_etat );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_pas_detailler_ventilation = gtk_check_button_new_with_label ( _("Do not detail reconciled transactions") );
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_pas_detailler_ventilation ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 9 ));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_pas_detailler_ventilation,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_pas_detailler_ventilation );



    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *page_organisation_donnees ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *scrolled_window;
    GtkWidget *frame;
    GtkWidget *vbox;
    GtkWidget *fleche;
    GtkWidget *vbox_onglet;
    GtkWidget *hbox_haut;
    GtkWidget *menu;
    GtkWidget *menu_item;
    gint i;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );

    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    /*   hbox du haut de la fenetre, le type de classement et ce qu'on utilise dans le classement */

    hbox_haut = gtk_hbox_new ( FALSE,
			       5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox_haut,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( hbox_haut );

    /* choix de ce qu'on utilise dans le classement */

    frame = gtk_frame_new ( COLON(_("Informations used for the group")) );
    gtk_box_pack_start ( GTK_BOX ( hbox_haut ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );


    /* on permet de regrouper les opérations par compte */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_regroupe_ope_compte_etat = gtk_check_button_new_with_label ( _("Group transactions by account") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_regroupe_ope_compte_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_regroupe_ope_compte_etat );


    /* on permet de regrouper les opérations par tiers */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_utilise_tiers_etat = gtk_check_button_new_with_label ( _("Group transactions by third party") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_utilise_tiers_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_utilise_tiers_etat );



    /* on permet de regrouper les opérations par categ */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    bouton_utilise_categ_etat = gtk_check_button_new_with_label ( _("Group transactions by category") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_utilise_categ_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_utilise_categ_etat );

    /* on permet de regrouper les opérations par ib */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    bouton_utilise_ib_etat = gtk_check_button_new_with_label ( _("Group transactions by budgetary line") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_utilise_ib_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_utilise_ib_etat );


    /* choix du type de classement */

    frame = gtk_frame_new ( COLON(_("Group level organisation")) );
    gtk_box_pack_start ( GTK_BOX ( hbox_haut ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( hbox ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			hbox );
    gtk_widget_show ( hbox );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_set_usize ( scrolled_window,
			   200,
			   100 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 scrolled_window,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( scrolled_window );


    liste_type_classement_etat = gtk_ctree_new ( 1,
						 0 );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_type_classement_etat ),
				       0,
				       TRUE );
    gtk_ctree_set_line_style ( GTK_CTREE ( liste_type_classement_etat ),
			       GTK_CTREE_LINES_NONE );
    gtk_ctree_set_expander_style ( GTK_CTREE ( liste_type_classement_etat ),
				   GTK_CTREE_EXPANDER_NONE );

    gtk_signal_connect ( GTK_OBJECT ( liste_type_classement_etat ),
			 "button_press_event",
			 GTK_SIGNAL_FUNC ( click_liste_etat ),
			 GINT_TO_POINTER (1) );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_type_classement_etat );
    gtk_widget_show ( liste_type_classement_etat );

    /* on place ici les flèches sur le côté de la liste */

    vbox = gtk_vbutton_box_new ();

    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 vbox,
			 FALSE,
			 FALSE,
			 0);

    /* FIXME */
    fleche = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
    /*   fleche = GTK_STOCK ( GTK_STOCK_UP ); */
    gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( fleche ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( click_haut_classement_etat ),
			 NULL );
    gtk_container_add ( GTK_CONTAINER ( vbox ),
			fleche  );

    /* FIXME */
    fleche = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
    /*   fleche = GTK_STOCK ( GTK_STOCK_DOWN ); */
    gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect ( GTK_OBJECT ( fleche ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( click_bas_classement_etat ),
			 NULL);
    gtk_container_add ( GTK_CONTAINER ( vbox ),
			fleche  );
    gtk_widget_show_all ( vbox );


    /* on permet la séparation des revenus et des dépenses */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_separer_revenus_depenses = gtk_check_button_new_with_label ( _("Split incomes and outgoings") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_separer_revenus_depenses,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_separer_revenus_depenses );


    /* on permet la séparation par exercice */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_separe_exo_etat = gtk_check_button_new_with_label ( _("Split by financial year") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_separe_exo_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_separe_exo_etat );

    /* ce bouton est grisé ou non en fonction de l'utilisation ou non de l'exercice */

    gtk_signal_connect ( GTK_OBJECT ( radio_button_utilise_exo ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_separe_exo_etat );

    /* on permet ensuite la séparation des résultats par période */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_separe_plages_etat = gtk_check_button_new_with_label ( _("Split by period") );
    gtk_signal_connect ( GTK_OBJECT ( bouton_separe_plages_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( change_separation_result_periode ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_separe_plages_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_separe_plages_etat );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( "        " );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    frame = gtk_frame_new ( NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     10 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );

    /* mise en place de la ligne type - choix perso */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_type_separe_plages_etat = gtk_option_menu_new ();

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("Week") );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "type",
			  NULL );
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( modif_type_separation_dates ),
				NULL );
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Months") );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "type",
			  GINT_TO_POINTER (1) );
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( modif_type_separation_dates ),
				GINT_TO_POINTER (1) );
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("Year") );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "type",
			  GINT_TO_POINTER (2));
    gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
				"activate",
				GTK_SIGNAL_FUNC ( modif_type_separation_dates ),
				GINT_TO_POINTER (2));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ),
			       menu );
    gtk_widget_show ( menu );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_type_separe_plages_etat,
			 FALSE, 
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_type_separe_plages_etat );


    /* mise en place de la ligne de début de semaine */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( POSTSPACIFY(_("The week begins on")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE, 
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    bouton_debut_semaine = gtk_option_menu_new ();

    menu = gtk_menu_new ();

    i = 0;

    while ( jours_semaine[i] )
    {
	menu_item = gtk_menu_item_new_with_label ( _(jours_semaine[i]) );
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      _("day"),
			      GINT_TO_POINTER (i));
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_widget_show ( menu_item );
	i++;
    }

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_debut_semaine ),
			       menu );
    gtk_widget_show ( menu );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_debut_semaine,
			 FALSE, 
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_debut_semaine );

    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void click_haut_classement_etat ( void )
{
    GtkCTreeNode *node_parent;
    GtkCTreeNode *node;
    GtkCTreeNode *node_enfant;
    GtkCTreeNode *nouveau_parent;

    node = GTK_CLIST ( liste_type_classement_etat )->selection -> data;

    /*   si on est au niveau 1, peut pas plus haut */

    if ( GTK_CTREE_ROW ( node ) -> level == 1 )
	return;

    node_parent = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->parent;
    node_enfant = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->children;

    nouveau_parent = GTK_CTREE_ROW ( node_parent )->parent;

    /* on remonte le node */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node,
		     nouveau_parent,
		     NULL );

    /* on descend celui du dessus */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node_parent,
		     node,
		     NULL );

    /* on attache l'enfant à son nouveau parent */

    if ( node_enfant )
	gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
			 node_enfant,
			 node_parent,
			 NULL );

    gtk_ctree_expand_recursive ( GTK_CTREE ( liste_type_classement_etat ),
				 node );
}
/******************************************************************************/

/******************************************************************************/
void click_bas_classement_etat ( void )
{
    GtkCTreeNode *node_parent;
    GtkCTreeNode *node;
    GtkCTreeNode *node_enfant;
    GtkCTreeNode *node_enfant_enfant;

    node = GTK_CLIST ( liste_type_classement_etat )->selection -> data;

    /*   si on est au niveau 4, peut pas plus bas */

    if ( GTK_CTREE_ROW ( node ) -> level == 4 )
	return;

    node_parent = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->parent;
    node_enfant = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->children;

    node_enfant_enfant = GTK_CTREE_ROW ( node_enfant )->children;

    /* on remonte le node enfant */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node_enfant,
		     node_parent,
		     NULL );

    /* on descend le node */

    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node,
		     node_enfant,
		     NULL );

    /* on attache l'enfant de l'enfant à son nouveau parent */

    if ( node_enfant_enfant )
	gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
			 node_enfant_enfant,
			 node,
			 NULL );

    gtk_ctree_expand_recursive ( GTK_CTREE ( liste_type_classement_etat ),
				 node );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *page_affichage_donnees ( void )
{
    /* on crée un notebook */

    notebook_aff_donnees = gtk_notebook_new ();
    gtk_notebook_set_scrollable ( GTK_NOTEBOOK (notebook_aff_donnees  ),
				  TRUE );
    gtk_container_set_border_width ( GTK_CONTAINER ( notebook_aff_donnees ),
				     5 );
    gtk_widget_show ( notebook_aff_donnees );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_aff_donnees ),
			       onglet_affichage_etat_generalites (),
			       gtk_label_new (SPACIFY(_("Generalities"))) );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_aff_donnees ),
			       onglet_affichage_etat_divers (),
			       gtk_label_new (SPACIFY(_("Titles"))) );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_aff_donnees ),
			       onglet_affichage_etat_operations (),
			       gtk_label_new (SPACIFY(_("Transactions"))) );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_aff_donnees ),
			       onglet_affichage_etat_devises (),
			       gtk_label_new (SPACIFY(_("Currencies"))) );

    return ( notebook_aff_donnees );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_affichage_etat_generalites ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );

    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    /* choix du nom du rapport */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Report name")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    entree_nom_etat = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 entree_nom_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( entree_nom_etat );


    /* afficher ou non le nb d'opés avec les totaux */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_afficher_nb_opes = gtk_check_button_new_with_label ( _("Diplay the number of transactions with the totals") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_nb_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_nb_opes );

    /* mise en place du bouton pour faire apparaitre l'état dans la liste des tiers */

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    bouton_inclure_dans_tiers = gtk_check_button_new_with_label ( _("Consider the third parties of this report as a multiple third party.") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_inclure_dans_tiers,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_inclure_dans_tiers );

    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_affichage_etat_operations ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *separateur;
    GtkWidget *table;
    GtkWidget *vbox_onglet;
    GtkWidget *menu;
    GtkWidget *menu_item;


    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );

    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    /* afficher ou non les opés */

    bouton_afficher_opes = gtk_check_button_new_with_label ( _("Display transactions") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_opes );


    /* demande les détails affichés dans les opérations */

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    frame_onglet_generalites = gtk_frame_new ( _("Display the information about") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 frame_onglet_generalites,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame_onglet_generalites );


    /* connection pour rendre sensitif la frame */

    gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_opes ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 frame_onglet_generalites );


    table = gtk_table_new ( 12,
			    3,
			    FALSE );
    gtk_container_add ( GTK_CONTAINER ( frame_onglet_generalites ),
			table );
    gtk_widget_show ( table );


    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 1,
				0, 1 );
    gtk_widget_show ( hbox );

    bouton_afficher_no_ope = gtk_check_button_new_with_label ( _("the transaction number") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_no_ope,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_no_ope );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				1, 2,
				0, 1 );
    gtk_widget_show ( hbox );

    bouton_afficher_date_opes = gtk_check_button_new_with_label ( _("the date") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_date_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_date_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				2, 3,
				0, 1 );
    gtk_widget_show ( hbox );

    bouton_afficher_exo_opes = gtk_check_button_new_with_label ( _("the financial year") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_exo_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_exo_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 1,
				1, 2 );
    gtk_widget_show ( hbox );

    bouton_afficher_tiers_opes = gtk_check_button_new_with_label ( _("the third party") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_tiers_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_tiers_opes );


    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				1, 2,
				1, 2 );
    gtk_widget_show ( hbox );

    bouton_afficher_categ_opes = gtk_check_button_new_with_label ( _("the category") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_categ_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_categ_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				2, 3,
				1, 2 );
    gtk_widget_show ( hbox );

    bouton_afficher_sous_categ_opes = gtk_check_button_new_with_label ( _("the sub-category") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_sous_categ_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_sous_categ_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 1,
				2, 3 );
    gtk_widget_show ( hbox );

    bouton_afficher_ib_opes = gtk_check_button_new_with_label ( _("the budgetary line") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_ib_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_ib_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				1, 2,
				2, 3 );
    gtk_widget_show ( hbox );

    bouton_afficher_sous_ib_opes = gtk_check_button_new_with_label ( _("the sub-budgetary line") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_sous_ib_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_sous_ib_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				2, 3,
				2, 3 );
    gtk_widget_show ( hbox );

    bouton_afficher_notes_opes = gtk_check_button_new_with_label ( _("the notes") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_notes_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_notes_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 1,
				3, 4 );
    gtk_widget_show ( hbox );

    bouton_afficher_type_ope = gtk_check_button_new_with_label ( _("the method of payment") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_type_ope,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_type_ope );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				1, 2,
				3, 4 );
    gtk_widget_show ( hbox );

    bouton_afficher_no_cheque = gtk_check_button_new_with_label ( _("the cheque/transfer number") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_no_cheque,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_no_cheque );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				2, 3,
				3, 4 );
    gtk_widget_show ( hbox );

    bouton_afficher_pc_opes = gtk_check_button_new_with_label ( _("the voucher") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_pc_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_pc_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 1,
				4, 5 );
    gtk_widget_show ( hbox );

    bouton_afficher_infobd_opes = gtk_check_button_new_with_label ( _("the bank reference") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_infobd_opes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_infobd_opes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				1, 2,
				4, 5  );
    gtk_widget_show ( hbox );

    bouton_afficher_no_rappr = gtk_check_button_new_with_label ( _("the reconciliation reference") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_no_rappr,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_no_rappr );


    /* on propose d'afficher le titre des colonnes */

    separateur = gtk_hseparator_new ();
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				separateur,
				0, 3,
				5, 6 );
    gtk_widget_show ( separateur );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 3,
				6, 7 );
    gtk_widget_show ( hbox );

    bouton_afficher_titres_colonnes = gtk_check_button_new_with_label ( _("Display the column titles") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_afficher_titres_colonnes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_titres_colonnes );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 3,
				7, 8 );
    gtk_widget_show ( hbox );

    bouton_titre_changement = gtk_radio_button_new_with_label ( NULL,
								_("every section") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_titre_changement,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_titre_changement );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 3,
				8, 9 );
    gtk_widget_show ( hbox );

    bouton_titre_en_haut = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_titre_changement )),
							     _("at the top of the report") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_titre_en_haut,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_titre_en_haut );

    /* mise en place du type de classement des opés */

    separateur = gtk_hseparator_new ();
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				separateur,
				0, 3,
				9,10 );
    gtk_widget_show ( separateur );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 1,
				10, 11 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( POSTSPACIFY(_("Sort the transactions by")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				1, 3,
				10, 11 );
    gtk_widget_show ( hbox );

    bouton_choix_classement_ope_etat = gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_choix_classement_ope_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_choix_classement_ope_etat );

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("date"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 0 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("transaction number"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 1 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("third party"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 2 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("category"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 3 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("budgetary line"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 4 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("note"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 5 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("method of payment"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 6 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("cheque/transfer number"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 7 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("voucher"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 8 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("bank reference"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 9 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    menu_item = gtk_menu_item_new_with_label ( _("reconciliation reference"));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_classement",
			  GINT_TO_POINTER ( 10 ));
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_widget_show ( menu_item );

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_classement_ope_etat ),
			       menu );
    gtk_widget_show ( menu );


    /* on propose de rendre clickable le détails des opérations */

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				hbox,
				0, 3,
				11, 12 );
    gtk_widget_show ( hbox );

    bouton_rendre_ope_clickables = gtk_check_button_new_with_label ( _("Make transactions clickable"));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_rendre_ope_clickables,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_rendre_ope_clickables );


    /* on met les connexions */

    gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_categ_opes ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_categ_opes );
    gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_ib_opes ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_ib_opes );
    gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_titres_colonnes ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_titre_changement );
    gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_titres_colonnes ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_titre_en_haut );

    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_affichage_etat_devises ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *hbox_2;
    GtkWidget *vbox_onglet;


    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );

    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );



    hbox = gtk_hbox_new ( TRUE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("General totals currency")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    hbox_2 = gtk_hbox_new ( TRUE,
			    5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 hbox_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox_2 );

    bouton_devise_general_etat = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_general_etat ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 bouton_devise_general_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_general_etat );

    label = gtk_label_new ( "" );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( TRUE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    label = gtk_label_new ( COLON(_("Third parties currency")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    hbox_2 = gtk_hbox_new ( TRUE,
			    5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 hbox_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox_2 );

    bouton_devise_tiers_etat = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 bouton_devise_tiers_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_tiers_etat );

    label = gtk_label_new ( "" );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( TRUE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Categories currency")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    hbox_2 = gtk_hbox_new ( TRUE,
			    5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 hbox_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox_2 );

    bouton_devise_categ_etat = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 bouton_devise_categ_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_categ_etat );

    label = gtk_label_new ( "" );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );


    hbox = gtk_hbox_new ( TRUE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Budgetary lines currency")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    hbox_2 = gtk_hbox_new ( TRUE,
			    5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 hbox_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox_2 );

    bouton_devise_ib_etat = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 bouton_devise_ib_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_ib_etat );

    label = gtk_label_new ( "" );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( TRUE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( POSTSPACIFY(_("Amount comparison currency")));
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( label );

    hbox_2 = gtk_hbox_new ( TRUE,
			    5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 hbox_2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox_2 );

    bouton_devise_montant_etat = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_montant_etat ),
			       creation_option_menu_devises ( 0,
							      liste_struct_devises ));
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 bouton_devise_montant_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_devise_montant_etat );

    label = gtk_label_new ( "" );
    gtk_box_pack_start ( GTK_BOX ( hbox_2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );



    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_affichage_etat_divers ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *vbox_onglet;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *vbox;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );

    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    /* mise en place de la frame des comptes */

    frame = gtk_frame_new ( COLON(_("Accounts")));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );


    /* on permet d'afficher le nom du compte */

    bouton_afficher_noms_comptes = gtk_check_button_new_with_label ( _("Display account name") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_noms_comptes,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_comptes );


    /* on permet d'afficher un ss total lors de chgt de compte */
    /* activé uniquement si on a regroupé les opés par compte */

    bouton_affiche_sous_total_compte = gtk_check_button_new_with_label ( _("Display a sub-total for each account") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_affiche_sous_total_compte,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_compte );

    gtk_signal_connect ( GTK_OBJECT ( bouton_regroupe_ope_compte_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_compte );

    /* mise en place de la frame des tiers */

    frame = gtk_frame_new ( COLON(_("Third party")));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );

    /* permet d'afficher le nom du tiers, activé uniquement si on utilise les tiers */

    bouton_afficher_noms_tiers = gtk_check_button_new_with_label ( _("Display third party's name") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_noms_tiers,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_tiers );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_tiers_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_noms_tiers );

    /* permet d'afficher un ss total lors de chgt de tiers */
    /* activé uniquement si on utilise les tiers */

    bouton_affiche_sous_total_tiers = gtk_check_button_new_with_label ( _("Dsplay a sub-total for each third party") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_affiche_sous_total_tiers,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_tiers );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_tiers_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_tiers );


    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    /* mise en place de la frame des catégories */

    frame = gtk_frame_new ( COLON(_("Categories")));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );


    /* affichage possible du nom de la categ */

    bouton_afficher_noms_categ = gtk_check_button_new_with_label ( _("Display the (sub)category's name") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_noms_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_categ );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_categ_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_noms_categ );

    /* permet d'afficher un sous total lors de chgt de categ */
    /* activé uniquement si on utilise les categ */

    bouton_affiche_sous_total_categ = gtk_check_button_new_with_label ( _("Display a sub-total for each category") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_affiche_sous_total_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_categ );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_categ_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_categ );

    /* mise en place du bouton pour afficher les sous categ */

    bouton_afficher_sous_categ = gtk_check_button_new_with_label ( _("Display sub-categories") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_sous_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_sous_categ );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_categ_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_categ );


    bouton_affiche_sous_total_sous_categ = gtk_check_button_new_with_label ( _("Display a sub-total for each sub-category") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_affiche_sous_total_sous_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_sous_categ );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_categ_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_sous_categ );


    bouton_afficher_pas_de_sous_categ = gtk_check_button_new_with_label ( _("Display \" No sub-category\" if none") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_pas_de_sous_categ,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_pas_de_sous_categ );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_categ_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_pas_de_sous_categ );


    /* mise en place de la frame des ib */

    frame = gtk_frame_new ( COLON(_("Budgetary lines")));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 frame,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( frame );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				     5 );
    gtk_container_add ( GTK_CONTAINER ( frame ),
			vbox );
    gtk_widget_show ( vbox );


    bouton_afficher_noms_ib = gtk_check_button_new_with_label ( _("Display the (sub-)budget line name") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_noms_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_noms_ib );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_noms_ib );

    /* permet d'afficher un sous total lors de chgt d'ib */
    /* activé uniquement si on utilise les ib */

    bouton_affiche_sous_total_ib = gtk_check_button_new_with_label ( _("Display a sub-total for each budgetary line") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_affiche_sous_total_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_ib );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_ib );

    /* mise en place du bouton pour afficher les sous categ */

    bouton_afficher_sous_ib = gtk_check_button_new_with_label ( _("Display sub-budget lines") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_sous_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_sous_ib );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_sous_ib );

    bouton_affiche_sous_total_sous_ib = gtk_check_button_new_with_label ( _("Display a sub-total for each sub-budget line") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_affiche_sous_total_sous_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_affiche_sous_total_sous_ib );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_affiche_sous_total_sous_ib );


    bouton_afficher_pas_de_sous_ib = gtk_check_button_new_with_label ( _("Display \"No sub-budgetary line\" if none") );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton_afficher_pas_de_sous_ib,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_afficher_pas_de_sous_ib );

    gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_ib_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 bouton_afficher_pas_de_sous_ib );

    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
GtkWidget *onglet_etat_mode_paiement ( void )
{
    GtkWidget *widget_retour;
    GtkWidget *scrolled_window;
    GtkWidget *bouton;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *vbox_onglet;

    widget_retour = gtk_scrolled_window_new ( FALSE,
					      FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_show ( widget_retour );

    vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				     10 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					    vbox_onglet );
    gtk_widget_show ( vbox_onglet );


    /* on met dans la partie de gauche une liste contenant les modes de paiement à */
    /* sélectionner */

    bouton_detaille_mode_paiement_etat = gtk_check_button_new_with_label ( _("Select the transactions by method of payment"));
    gtk_signal_connect_object ( GTK_OBJECT ( bouton_detaille_mode_paiement_etat ),
				"toggled",
				GTK_SIGNAL_FUNC ( stylise_tab_label_etat ),
				GINT_TO_POINTER ( 8 ));
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 bouton_detaille_mode_paiement_etat,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton_detaille_mode_paiement_etat );

    vbox_mode_paiement_etat = gtk_vbox_new ( FALSE,
					     5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
			 vbox_mode_paiement_etat,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( vbox_mode_paiement_etat );


    gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_mode_paiement_etat ),
			 "toggled",
			 GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
			 vbox_mode_paiement_etat );

    label = gtk_label_new ( COLON(_("Select methods of payment to include")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0.1,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_mode_paiement_etat ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    hbox = gtk_hbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( vbox_mode_paiement_etat ),
			 hbox,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( hbox );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_widget_set_usize ( scrolled_window,
			   300,
			   FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 scrolled_window,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( scrolled_window );

    liste_mode_paiement_etat = gtk_clist_new ( 1 );
    gtk_clist_set_selection_mode ( GTK_CLIST ( liste_mode_paiement_etat ),
				   GTK_SELECTION_MULTIPLE );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_mode_paiement_etat ),
				       0,
				       TRUE );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			liste_mode_paiement_etat );
    gtk_widget_show ( liste_mode_paiement_etat );

    /* on remplit la liste des comptes */

    remplissage_liste_modes_paiement_etats ();



    /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( vbox );

    bouton = gtk_button_new_with_label ( _("Select all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
				GTK_OBJECT  ( liste_mode_paiement_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );

    bouton = gtk_button_new_with_label ( _("Unselect all") );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				GTK_OBJECT  ( liste_mode_paiement_etat ));
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 bouton,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( bouton );


    return ( widget_retour );
}
/******************************************************************************/

/******************************************************************************/
void remplissage_liste_modes_paiement_etats ( void )
{
    gint i;
    GSList *liste_nom_types;
    GSList *liste_tmp;


    if ( !liste_comptes_etat )
	return;

    gtk_clist_clear ( GTK_CLIST ( liste_mode_paiement_etat ) );

    /* on va commencer par créer une liste de textes contenant les noms */
    /* des modes de paiement sans doublon */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
    liste_nom_types = NULL;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	liste_tmp = TYPES_OPES;

	while ( liste_tmp )
	{
	    struct struct_type_ope *type_ope;

	    type_ope = liste_tmp -> data;

	    if ( !g_slist_find_custom ( liste_nom_types,
					type_ope -> nom_type,
					(GCompareFunc) recherche_nom_dans_liste ))
		liste_nom_types = g_slist_append ( liste_nom_types,
						   g_strdup ( type_ope -> nom_type ));

	    liste_tmp = liste_tmp -> next;
	}
	p_tab_nom_de_compte_variable++;
    }

    /* on a donc une liste de noms de types d'opé non redondant, on classe */
    /* par ordre alphabétique et on affiche */

    liste_nom_types = g_slist_sort ( liste_nom_types,
				     (GCompareFunc) classe_liste_alphabetique );

    liste_tmp = liste_nom_types;

    while ( liste_tmp )
    {
	gint ligne;

	ligne = gtk_clist_append ( GTK_CLIST ( liste_mode_paiement_etat ),
				   (gchar **) &liste_tmp -> data );

	gtk_clist_set_row_data ( GTK_CLIST ( liste_mode_paiement_etat ),
				 ligne,
				 liste_tmp -> data );

	liste_tmp = liste_tmp -> next;
    }
}
/******************************************************************************/



/******************************************************************************/
void selectionne_liste_modes_paiement_etat_courant ( void )
{
    gint i;

    if ( !etat_courant )
	return;
    if ( !liste_comptes_etat )
	return;

    gtk_clist_unselect_all ( GTK_CLIST ( liste_mode_paiement_etat ));

    if ( !etat_courant -> noms_modes_paiement )
	return;

    /* on fait le tour de la liste pour voir s'il y a un état sélectionné */

    for ( i=0 ; i < GTK_CLIST ( liste_mode_paiement_etat ) -> rows ; i++ )
    {
	if ( g_slist_find_custom ( etat_courant -> noms_modes_paiement,
				   gtk_clist_get_row_data ( GTK_CLIST ( liste_mode_paiement_etat ),
							    i ),
				   (GCompareFunc) recherche_nom_dans_liste ))
	    gtk_clist_select_row ( GTK_CLIST ( liste_mode_paiement_etat ),
				   i,
				   0 );
    }
}
/******************************************************************************/



