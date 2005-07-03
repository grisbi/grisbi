/*  Fichier qui s'occupe de l'onglet états */
/*      etats.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2002-2004 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "include.h"


/*START_INCLUDE*/
#include "etats_onglet.h"
#include "etats_calculs.h"
#include "etat_io.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "navigation.h"
#include "menu.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils.h"
#include "etats_config.h"
#include "utils_files.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void change_choix_nouvel_etat ( GtkWidget *menu_item,
				GtkWidget *label_description );
static void export_etat_vers_html ( struct struct_etat *etat );
static GtkWidget *gsb_gui_create_report_toolbar ( void );
/*END_STATIC*/



GSList *liste_struct_etats;
gint no_dernier_etat;
GtkWidget *frame_liste_etats;
/** TODO put that in the state frame above  */
/* GtkWidget *label_etat_courant;        /\* label en haut de la liste des états *\/ */
GtkWidget *vbox_liste_etats;          /* vbox contenant la liste des états */
struct struct_etat *etat_courant;
GtkWidget *bouton_effacer_etat;
GtkWidget *bouton_personnaliser_etat;
GtkWidget *bouton_imprimer_etat;
GtkWidget *bouton_exporter_etat;
GtkWidget *bouton_dupliquer_etat;
GtkWidget *scrolled_window_etat;          /* contient l'état en cours */
gint nb_colonnes;
gint ligne_debut_partie;
GtkWidget *notebook_etats;
GtkWidget *notebook_config_etat;
GtkWidget *notebook_selection;
GtkWidget *notebook_aff_donnees;
GtkWidget *onglet_config_etat;


/*START_EXTERN*/
extern gchar *dernier_chemin_de_travail;
extern struct struct_etat_affichage html_affichage ;
extern GtkItemFactory *item_factory_menu_general;
extern GtkWidget *notebook_general;
extern GtkTreeSelection * selection;
extern GtkWidget *window;
/*END_EXTERN*/



/**
 * Create a toolbar containing all necessary controls on reports tab.
 *
 * \return a newly-allocated hbox
 */
GtkWidget *gsb_gui_create_report_toolbar ( void )
{
    GtkWidget *hbox, *handlebox, *hbox2;

    hbox = gtk_hbox_new ( FALSE, 5 );

    /* HandleBox */
    handlebox = gtk_handle_box_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), handlebox, TRUE, TRUE, 0 );
    /* Hbox2 */
    hbox2 = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox2 );

    /* Add various icons */
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 new_button_with_label_and_image ( etat.display_toolbar,
							   _("New report"),
							   "new-report.png",
							   G_CALLBACK ( ajout_etat ),
							   NULL ),
			 FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX ( hbox2 ), 
			 new_stock_button_with_label ( etat.display_toolbar,
						       GTK_STOCK_OPEN, 
						       _("Import"),
						       G_CALLBACK (importer_etat),
						       NULL ), 
			 FALSE, FALSE, 0 );

    bouton_exporter_etat = new_stock_button_with_label ( etat.display_toolbar,
							 GTK_STOCK_SAVE, 
							 _("Export"),
							 G_CALLBACK (exporter_etat),
							 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_exporter_etat, FALSE, FALSE, 0 );

    bouton_imprimer_etat = new_stock_button_with_label ( etat.display_toolbar,
							 GTK_STOCK_PRINT, 
							 _("Print"),
							 G_CALLBACK (impression_etat_courant),
							 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_imprimer_etat, FALSE, FALSE, 0 );

    bouton_effacer_etat = new_stock_button_with_label ( etat.display_toolbar,
							GTK_STOCK_DELETE, 
							_("Delete"),
							G_CALLBACK ( efface_etat ),
							NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_effacer_etat, FALSE, FALSE, 0 );

    bouton_personnaliser_etat = new_stock_button_with_label ( etat.display_toolbar,
							      GTK_STOCK_PROPERTIES, 
							      _("Properties"),
							      G_CALLBACK (personnalisation_etat),
							      NULL ), 
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_personnaliser_etat, FALSE, FALSE, 0 );

    bouton_dupliquer_etat = new_stock_button_with_label ( etat.display_toolbar,
							  GTK_STOCK_COPY, 
							  _("Clone"),
							  G_CALLBACK (dupliquer_etat),
							  NULL ), 
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_dupliquer_etat, FALSE, FALSE, 0 );

    gtk_widget_show_all ( hbox );

    return ( hbox );
}



/**
 *
 *
 */
GtkWidget *creation_onglet_etats ( void )
{
    GtkWidget *tab, *vbox;

    /*   au départ, aucun état n'est ouvert */
    etat_courant = NULL;

    tab = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( tab ), gsb_gui_create_report_toolbar(), 
			 FALSE, FALSE, 0 );

    /* création du notebook contenant l'état et la config */
    notebook_etats = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook_etats ), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(notebook_etats), FALSE );
    gtk_box_pack_start ( GTK_BOX ( tab ), notebook_etats, TRUE, TRUE, 0 );

    /* création de la partie droite */

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_etats ), vbox, gtk_label_new ( _("Reports")));

    /* On met une scrolled window qui sera remplit par l'état */
    scrolled_window_etat = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_etat), 
					  GTK_SHADOW_NONE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_etat ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( vbox ), scrolled_window_etat, TRUE, TRUE, 0 );

    /* L'onglet de config sera créé que si nécessaire */
    onglet_config_etat = NULL;

    gtk_widget_show_all ( tab );

    return ( tab );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* on propose une liste d'états prémachés et les remplis en fonction du choix */
/* de la personne */
/*****************************************************************************************************/

gboolean ajout_etat ( void )
{
    struct struct_etat *etat;
    struct struct_comparaison_montants_etat *comp_montant;
    GtkWidget *dialog;
    gint resultat;
    GtkWidget *frame;
    GtkWidget *option_menu;
    GtkWidget *menu;
    GtkWidget *menu_item;
    GtkWidget *label_description;
    GtkWidget *scrolled_window;


    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_OK_CANCEL,
				       make_hint ( _("Choose template for new report"),
						   _("You are about to create a new report.  For convenience, you can choose between the following templates.  Reports may be customized later." ) ) );

    frame = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, FALSE,
					_("Report type"));

    /* on commence par créer l'option menu */
    option_menu = gtk_option_menu_new ();
    gtk_box_pack_start ( GTK_BOX(frame), option_menu, FALSE, FALSE, 0 );

    /* On met une ligne blanche entre les paddingboxes */
    /*   gtk_box_pack_start ( GTK_BOX(frame), gtk_label_new(""), FALSE, FALSE, 6 ); */

    /* on ajoute maintenant la frame */
    frame = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, TRUE,
					_("Description"));

    /* on met le label dans une scrolled window */
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX(frame), scrolled_window, TRUE, TRUE, 6 );

    /* on ajoute maintenant le label */
    label_description = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_description ), 0, 0 );
    gtk_label_set_line_wrap ( GTK_LABEL ( label_description ), TRUE );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    label_description );

    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( label_description -> parent ),
				   GTK_SHADOW_NONE );

    /* on crée ici le menu qu'on ajoute à l'option menu */
    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("Last month incomes and outgoings"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 0 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    /* on met le texte du 1er choix */
    change_choix_nouvel_etat ( menu_item, label_description );

    menu_item = gtk_menu_item_new_with_label ( _("Current month incomes and outgoings"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item )
	; gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
				GINT_TO_POINTER ( 1 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Annual budget"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 2 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Cheques deposit"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 4 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Monthly outgoings by payee"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 5 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Search"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 6 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));

    menu_item = gtk_menu_item_new_with_label ( _("Blank report"));
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ), "no_etat",
			  GINT_TO_POINTER ( 3 ));
    gtk_signal_connect ( GTK_OBJECT ( menu_item ), "activate",
			 GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			 GTK_OBJECT ( label_description ));
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ), menu );

    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog)->vbox), 6 );
    gtk_widget_show_all ( dialog );

    /* on attend le choix de l'utilisateur */

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( dialog );
	return FALSE;
    }


    /* on récupère le type d'état voulu */

    resultat = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
						       "no_etat" ));
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));


    /* on crée le nouvel état */

    etat = calloc ( 1, sizeof ( struct struct_etat ));

    etat -> no_etat = ++no_dernier_etat;


    /* on remplit maintenant l'état en fonction de ce qu'on a demandé */

    switch ( resultat )
    {
	case 0:
	    /*  revenus et dépenses du mois précédent  */

	    etat -> nom_etat = g_strdup ( _("Previous month incomes and outgoings") );

	    etat -> separer_revenus_depenses = 1;
	    etat -> no_plage_date = 7;


	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    etat -> type_virement = 2;
	    etat -> utilise_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_pas_de_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;

	    break;

	case 1:
	    /*  revenus et dépenses du mois courant  */

	    etat -> nom_etat = g_strdup ( _("Current month incomes and outgoings") );

	    etat -> separer_revenus_depenses = 1;
	    etat -> no_plage_date = 3;


	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    etat -> type_virement = 2;
	    etat -> utilise_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_pas_de_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;

	    break;


	case 2:

	    /* on ajoute le budget annuel */

	    etat -> nom_etat = g_strdup ( _("Annual budget") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;

	    etat -> separer_revenus_depenses = 1;
	    etat -> no_plage_date = 4;
	    etat -> utilise_categ = 1;
	    etat -> exclure_ope_sans_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> afficher_pas_de_sous_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;
	    etat -> exclure_montants_nuls = 1;

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;



	case 3:

	    /* on ajoute un état vierge appelé nouvel état */

	    etat -> nom_etat = g_strdup ( _("New report") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;

	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_date_ope = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> afficher_categ_ope = 1;
	    etat -> separer_revenus_depenses = 1;
	    etat -> type_virement = 2;
	    etat -> utilise_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;
	    etat -> exclure_montants_nuls = 1;


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 4:

	    /* remise de chèques */

	    etat -> nom_etat = g_strdup ( _("Cheques deposit") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_nb_opes = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> afficher_infobd_ope = 1;
	    etat -> type_virement = 2;
	    etat -> exclure_montants_nuls = 1;
	    etat -> utilise_montant = 1;

	    /* on doit créer une structure de montant qui dit que ça va être positif */

	    comp_montant = calloc ( 1,
				    sizeof ( struct struct_comparaison_montants_etat ));
	    comp_montant -> lien_struct_precedente = -1;
	    comp_montant -> comparateur_1 = 8;
	    comp_montant -> lien_1_2 = 3;

	    etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
									 comp_montant );


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 5:

	    /* dépenses mensuelles par tiers */

	    etat -> nom_etat = g_strdup ( _("Monthly outgoings by payee") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_nb_opes = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> type_classement_ope = 2;
	    etat -> afficher_titre_colonnes = 1;
	    etat -> no_plage_date = 7;
	    etat -> utilise_categ = 1;
	    etat -> affiche_sous_total_categ = 1;
	    etat -> afficher_sous_categ = 1;
	    etat -> afficher_nom_categ = 1;

	    etat -> afficher_date_ope = 1;
	    etat -> afficher_categ_ope = 1;
	    etat -> separer_revenus_depenses = 1;
	    etat -> type_virement = 2;
//	    etat -> exclure_ope_sans_categ = 1;
	    etat -> affiche_sous_total_sous_categ = 1;
//	    etat -> exclure_ope_sans_ib = 1;
	    etat -> exclure_montants_nuls = 1;
	    etat -> utilise_tiers = 1;
	    etat -> affiche_sous_total_tiers = 1;
	    etat -> afficher_nom_tiers = 1;


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 6:

	    /* recherche */

	    etat -> nom_etat = g_strdup ( _("Search") );

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 1 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 2 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 3 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 4 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 5 ));
	    etat -> type_classement = g_list_append ( etat -> type_classement,
						      GINT_TO_POINTER ( 6 ));

	    /*   les devises sont à 1 (euro) */

	    etat -> devise_de_calcul_general = 1;
	    etat -> devise_de_calcul_categ = 1;
	    etat -> devise_de_calcul_ib = 1;
	    etat -> devise_de_calcul_tiers = 1;
	    etat -> choix_devise_montant = 1;


	    etat -> afficher_opes = 1;
	    etat -> afficher_date_ope = 1;
	    etat -> afficher_tiers_ope = 1;
	    etat -> afficher_categ_ope = 1;
	    etat -> afficher_sous_categ_ope = 1;
	    etat -> afficher_type_ope = 1;
	    etat -> afficher_ib_ope = 1;
	    etat -> afficher_sous_ib_ope = 1;
	    etat -> afficher_cheque_ope = 1;
	    etat -> afficher_notes_ope = 1;
	    etat -> afficher_pc_ope = 1;
	    etat -> afficher_rappr_ope = 1;
	    etat -> afficher_infobd_ope = 1;
	    etat -> afficher_exo_ope = 1;

	    etat -> ope_clickables = 1;
	    etat -> no_plage_date = 4;
	    etat -> separation_par_plage = 1;
	    etat -> type_separation_plage = 2;
	    etat -> type_virement = 2;

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;


	default :
	    dialogue_error ( _("Unknown report type, creation cancelled"));
	    return FALSE;
    }

    /* on l'ajoute à la liste */
    liste_struct_etats = g_slist_append ( liste_struct_etats, etat );

    /* on réaffiche la liste des états */
    etat_courant = etat;

    /* TODO, update with navigation list */
/*     remplissage_liste_etats (); */

    gtk_widget_set_sensitive ( bouton_personnaliser_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

/*     gtk_label_set_text ( GTK_LABEL ( label_etat_courant ), */
/* 			 etat_courant -> nom_etat ); */

    /* Add an entry in navigation pane. */
    gsb_gui_navigation_add_report ( etat );

    personnalisation_etat ();
    modification_fichier ( TRUE );

    return FALSE;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void change_choix_nouvel_etat ( GtkWidget *menu_item,
				GtkWidget *label_description )
{
    gchar *description;

    switch ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( menu_item ),
						     "no_etat" )))
    {
	case 0:
	    /* revenus et dépenses du mois dernier  */

	    description = _("This report displays totals for last month's transactions sorted by categories and sub-categories. You just need to select the account(s). By default, all accounts are selected.");
	    break;

	case 1:
	    /* revenus et dépenses du mois en cours  */

	    description = _("This report displays totals of current month's transactions sorted by categories and sub-categories. You just need to select the account(s). By default, all accounts are selected.");
	    break;

	case 2:
	    /* budget annuel  */

	    description = _("This report displays annual budget. You just need to select the account(s). By default all accounts are selected.");
	    break;

	case 3:
	    /* etat vierge  */

	    description = _("This report is an empty one. You need to customise it entirely.");
	    break;

	case 4:
	    /* remise de chèques  */

	    description = _("This report displays the cheques deposit. You just need to select the account(s). By default all accounts are selected.");
	    break;

	case 5:
	    /* dépenses mensuelles par tiers  */

	    description = _("This report displays current month's outgoings sorted by payees. You just need to select the account(s). By default all accounts areselected.");
	    break;

	case 6:
	    /* recherche  */

	    description = _("This report displays all the information for all transactions of all accounts for the current year. You just have to add the amount, date, payees etc. criteria thant you want. By default the transactions are clickables.");
	    break;

	default:

	    description = _("???? should not be displayed...");
    }

    gtk_label_set_text ( GTK_LABEL ( label_description ),
			 description );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
void efface_etat ( void )
{
    if ( !liste_struct_etats || !etat_courant )
	return;

     if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );

   if ( !question_yes_no_hint ( g_strdup_printf (_("Delete report \"%s\"?"),
						  etat_courant -> nom_etat ),
				 _("This will irreversibly remove this report.  There is no undo for this.") ))
	return;

    /*   on met l'état courant à -1 et */
    /* le bouton à null, et le label de l'état en cours à rien */
    liste_struct_etats = g_slist_remove ( liste_struct_etats, etat_courant );

    /* Update reports list in navigation. */
    gsb_gui_navigation_remove_report ( etat_courant );

    etat_courant = NULL;
    gtk_widget_set_sensitive ( bouton_personnaliser_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Clone report"), NULL)),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Print report..."), NULL)),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Export report..."), NULL)),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Export report as HTML..."), NULL)),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Remove report"), NULL)),
			       FALSE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Edit report..."), NULL)),
			       FALSE );


    if ( GTK_BIN ( scrolled_window_etat ) -> child )
	gtk_widget_hide ( GTK_BIN ( scrolled_window_etat ) -> child );

/*     remplissage_liste_etats (); */
    modification_fichier ( TRUE );

}
/*****************************************************************************************************/


/*****************************************************************************************************/
void changement_etat ( struct struct_etat *etat )
{
    etat_courant = etat;
    gtk_widget_set_sensitive ( bouton_personnaliser_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Clone report"), NULL)),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Print report..."), NULL)),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Export report..."), NULL)),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Export report as HTML..."), NULL)),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Remove report"), NULL)),
			       TRUE );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Reports"), _("Edit report..."), NULL)),
			       TRUE );

/*     gtk_label_set_text ( GTK_LABEL ( label_etat_courant ), etat -> nom_etat ); */
/*     gtk_label_set_line_wrap ( GTK_LABEL ( label_etat_courant ), TRUE ); */

    /* on affiche l'état */
    rafraichissement_etat ( etat );
}



/**
 *
 *
 */
void export_etat_vers_html ( struct struct_etat *etat )
{
    affichage_etat ( etat, &html_affichage );
}



/**
 *
 *
 */
void export_etat_courant_vers_html ( )
{
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );

    export_etat_vers_html ( NULL );
}



void exporter_etat ( void )
{
    GtkWidget *fenetre_nom;
    gint resultat;
    gchar *nom_etat;

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );

    fenetre_nom = file_selection_new (_("Export report"),FILE_SELECTION_IS_SAVE_DIALOG );
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				  dernier_chemin_de_travail );
    file_selection_set_entry (  GTK_FILE_SELECTION ( fenetre_nom ),
				g_strconcat ( safe_file_name ( etat_courant -> nom_etat ),
					      ".egsb",
					      NULL ));
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_etat = file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* la vérification que c'est possible a �t� faite par la boite de selection*/

	    if ( !enregistre_etat ( nom_etat ))
	    {
		return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return;
    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void importer_etat ( void )
{
    GtkWidget *fenetre_nom;
    gint resultat;
    gchar *nom_etat;

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );

    fenetre_nom = file_selection_new ( _("Import a report") , FILE_SELECTION_MUST_EXIST);
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				      dernier_chemin_de_travail );
    file_selection_set_entry (  GTK_FILE_SELECTION ( fenetre_nom ),
			 g_strconcat ( dernier_chemin_de_travail,
				       ".egsb",
				       NULL ));

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_etat =file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* la vérification que c'est possible a �t� faite par la boite de selection*/


	    if ( !charge_etat ( nom_etat ))
	    {
		return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return;
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* cette fonction crée une copie de l'état courant */
/*****************************************************************************************************/

void dupliquer_etat ( void )
{
    struct struct_etat *etat;
    GSList *liste_tmp;

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );

    etat = calloc ( 1,
		    sizeof ( struct struct_etat ));


    /* on recopie les données de l'état courant */

    memcpy ( etat,
	     etat_courant,
	     sizeof ( struct struct_etat ));

    /* il reste juste à faire une copie des listes et des chaines pour terminer */

    etat -> no_etat = ++no_dernier_etat;

    etat -> nom_etat = g_strdup ( etat_courant -> nom_etat );
    etat -> no_exercices = g_slist_copy ( etat_courant -> no_exercices );

    if ( etat_courant -> date_perso_debut )
	etat -> date_perso_debut = g_date_new_dmy ( g_date_day ( etat_courant -> date_perso_debut ),
						    g_date_month ( etat_courant -> date_perso_debut ),
						    g_date_year ( etat_courant -> date_perso_debut ));

    if ( etat_courant -> date_perso_fin )
	etat -> date_perso_fin = g_date_new_dmy ( g_date_day ( etat_courant -> date_perso_fin ),
						  g_date_month ( etat_courant -> date_perso_fin ),
						  g_date_year ( etat_courant -> date_perso_fin ));

    etat -> type_classement = g_list_copy ( etat_courant -> type_classement );
    etat -> no_comptes = g_slist_copy ( etat_courant -> no_comptes );
    etat -> no_comptes_virements = g_slist_copy ( etat_courant -> no_comptes_virements );
    etat -> no_categ = g_slist_copy ( etat_courant -> no_categ );
    etat -> no_ib = g_slist_copy ( etat_courant -> no_ib );
    etat -> no_tiers = g_slist_copy ( etat_courant -> no_tiers );


    /* on fait une copie de la liste des textes */

    if ( etat -> liste_struct_comparaison_textes )
    {
	etat -> liste_struct_comparaison_textes = NULL;

	liste_tmp = etat_courant -> liste_struct_comparaison_textes;

	while ( liste_tmp )
	{
	    struct struct_comparaison_textes_etat *ancien_comp_textes;
	    struct struct_comparaison_textes_etat *comp_textes;

	    ancien_comp_textes = liste_tmp -> data;

	    comp_textes = calloc ( 1,
				   sizeof ( struct struct_comparaison_textes_etat ));

	    memcpy ( comp_textes,
		     ancien_comp_textes,
		     sizeof ( struct struct_comparaison_textes_etat ));

	    comp_textes -> texte = g_strdup ( comp_textes -> texte );

	    etat -> liste_struct_comparaison_textes = g_slist_append ( etat -> liste_struct_comparaison_textes,
								       comp_textes );
	    liste_tmp = liste_tmp -> next;
	}
    }

    /* on fait une copie de la liste des montants */

    if ( etat -> liste_struct_comparaison_montants )
    {
	etat -> liste_struct_comparaison_montants = NULL;

	liste_tmp = etat_courant -> liste_struct_comparaison_montants;

	while ( liste_tmp )
	{
	    struct struct_comparaison_montants_etat *ancien_comp_montants;
	    struct struct_comparaison_montants_etat *comp_montants;

	    ancien_comp_montants = liste_tmp -> data;

	    comp_montants = calloc ( 1,
				     sizeof ( struct struct_comparaison_montants_etat ));

	    memcpy ( comp_montants,
		     ancien_comp_montants,
		     sizeof ( struct struct_comparaison_montants_etat ));

	    etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
									 comp_montants );
	    liste_tmp = liste_tmp -> next;
	}
    }



    /* on l'ajoute à la liste */

    liste_struct_etats = g_slist_append ( liste_struct_etats,
					  etat );

    /* on réaffiche la liste des états */

    etat_courant = etat;

    /* TODO, update with navigation list */
/*     remplissage_liste_etats (); */

    gtk_widget_set_sensitive ( bouton_personnaliser_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

/*     gtk_label_set_text ( GTK_LABEL ( label_etat_courant ), etat_courant -> nom_etat ); */

    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

    personnalisation_etat ();
    modification_fichier ( TRUE );
}
/*****************************************************************************************************/


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
