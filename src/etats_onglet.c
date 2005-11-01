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
#include "dialog.h"
#include "utils_file_selection.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_file_others.h"
#include "navigation.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "utils.h"
#include "etats_config.h"
#include "print_config.h"
#include "utils_files.h"
#include "structures.h"
#include "include.h"
#include "etats_csv.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean ajout_etat ( void );
static void change_choix_nouvel_etat ( GtkWidget *menu_item, GtkWidget *label_description );
static void dupliquer_etat ( void );
static void efface_etat ( void );
static void export_etat_courant_vers_csv ( gchar * filename );
static void export_etat_courant_vers_html ( gchar * filename );
static void export_etat_vers_html ( gint report_number, gchar * filename );
static void exporter_etat ( void );
static GtkWidget *gsb_gui_create_report_toolbar ( void );
static gboolean gsb_report_export_change_format ( GtkWidget * combo, GtkWidget * selector );
static void importer_etat ( void );
/*END_STATIC*/


GtkWidget *frame_liste_etats;
/** TODO put that in the state frame above  */
/* GtkWidget *label_etat_courant;        /\* label en haut de la liste des états *\/ */
GtkWidget *vbox_liste_etats;          /* vbox contenant la liste des états */
GtkWidget *bouton_effacer_etat;
GtkWidget *bouton_personnaliser_etat;
GtkWidget *bouton_imprimer_etat;
GtkWidget *bouton_exporter_etat;
GtkWidget *bouton_dupliquer_etat;
GtkWidget *scrolled_window_etat = NULL;          /* contient l'état en cours */
gint nb_colonnes;
gint ligne_debut_partie;
GtkWidget *notebook_etats;
GtkWidget *notebook_config_etat;
GtkWidget *notebook_selection;
GtkWidget *notebook_aff_donnees;
GtkWidget *onglet_config_etat;


/*START_EXTERN*/
extern struct struct_etat_affichage csv_affichage ;
extern gchar *dernier_chemin_de_travail;
extern struct struct_etat_affichage html_affichage ;
extern struct struct_etat_affichage latex_affichage ;
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

    gsb_gui_unsensitive_report_widgets ();
    gtk_widget_show_all ( hbox );

    return ( hbox );
}



/**
 * Create the report tab widgets.
 *
 * \return	 A newly allocated vbox.
 */
GtkWidget *creation_onglet_etats ( void )
{
    GtkWidget *tab, *vbox;

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


/**
 * Ask for a report type for a template list, create the report and
 * update user interface.
 *
 * \return	FALSE
 */
gboolean ajout_etat ( void )
{
    gint report_number, amount_comparison_number, resultat;
    GtkWidget *dialog, *frame, *option_menu, *menu, *menu_item, *label_description;
    GtkWidget *scrolled_window;


    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general), 7 );

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

    /* get the wanted report */

    resultat = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
						       "no_etat" ));
    gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    /* create and fill the new report */

    switch ( resultat )
    {
	case 0:
	    /*  Previous month incomes and outgoings  */

	    report_number = gsb_data_report_new (_("Previous month incomes and outgoings"));

	    gsb_data_report_set_split_credit_debit ( report_number,
						     1 );
	    gsb_data_report_set_date_type ( report_number,
					    7 );

	    /*   le classement de base est 1-2-3-4-5-6 */

	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 1 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 2 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 3 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 4 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 5 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 6 )));

	    gsb_data_report_set_transfer_choice ( report_number,
						  2 );
	    gsb_data_report_set_category_used ( report_number,
						1 );
	    gsb_data_report_set_category_show_sub_category ( report_number,
							     1 );
	    gsb_data_report_set_category_show_category_amount ( report_number,
								1 );
	    gsb_data_report_set_category_show_sub_category_amount ( report_number,
								    1 );
	    gsb_data_report_set_category_show_without_category ( report_number,
								 1 );
	    gsb_data_report_set_category_show_name ( report_number,
						     1 );

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general ( report_number,
						   1 );
	    gsb_data_report_set_category_currency ( report_number,
						    1 );
	    gsb_data_report_set_budget_currency ( report_number,
						  1 );
	    gsb_data_report_set_payee_currency ( report_number,
						 1 );
	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     1 );

	    break;

	case 1:
	    /*  Current month incomes and outgoings */

	    report_number = gsb_data_report_new (_("Current month incomes and outgoings"));

	    gsb_data_report_set_split_credit_debit ( report_number,
						     1 );
	    gsb_data_report_set_date_type ( report_number,
					    3 );


	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 1 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 2 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 3 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 4 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 5 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 6 )));


	    gsb_data_report_set_transfer_choice ( report_number,
						  2 );
	    gsb_data_report_set_category_used ( report_number,
						1 );
	    gsb_data_report_set_category_show_sub_category ( report_number,
							     1 );
	    gsb_data_report_set_category_show_category_amount ( report_number,
								1 );
	    gsb_data_report_set_category_show_sub_category_amount ( report_number,
								    1 );
	    gsb_data_report_set_category_show_without_category ( report_number,
								 1 );
	    gsb_data_report_set_category_show_name ( report_number,
						     1 );

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general ( report_number,
						   1 );
	    gsb_data_report_set_category_currency ( report_number,
						    1 );
	    gsb_data_report_set_budget_currency ( report_number,
						  1 );
	    gsb_data_report_set_payee_currency ( report_number,
						 1 );
	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     1 );

	    break;


	case 2:

	    /* Annual budget */

	    report_number = gsb_data_report_new (_("Annual budget"));

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 1 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 2 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 3 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 4 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 5 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 6 )));


	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general ( report_number,
						   1 );
	    gsb_data_report_set_category_currency ( report_number,
						    1 );
	    gsb_data_report_set_budget_currency ( report_number,
						  1 );
	    gsb_data_report_set_payee_currency ( report_number,
						 1 );
	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     1 );

	    gsb_data_report_set_split_credit_debit ( report_number,
						     1 );
	    gsb_data_report_set_date_type ( report_number,
					    4 );
	    gsb_data_report_set_category_used ( report_number,
						1 );
	    gsb_data_report_set_category_only_report_with_category ( report_number,
								     1 );
	    gsb_data_report_set_category_show_category_amount ( report_number,
								1 );
	    gsb_data_report_set_category_show_sub_category ( report_number,
							     1 );
	    gsb_data_report_set_category_show_without_category ( report_number,
								 1 );
	    gsb_data_report_set_category_show_sub_category_amount ( report_number,
								    1 );
	    gsb_data_report_set_category_show_name ( report_number,
						     1 );
	    gsb_data_report_set_amount_comparison_only_report_non_null ( report_number,
									 1 );

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;



	case 3:

	    /* New report */

	    report_number = gsb_data_report_new (_("New report"));

	    /*   le classement de base est 1-2-3-4-5-6  */

	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 1 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 2 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 3 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 4 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 5 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 6 )));

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general ( report_number,
						   1 );
	    gsb_data_report_set_category_currency ( report_number,
						    1 );
	    gsb_data_report_set_budget_currency ( report_number,
						  1 );
	    gsb_data_report_set_payee_currency ( report_number,
						 1 );

	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     1 );


	    gsb_data_report_set_show_report_transactions ( report_number,
							   1 );
	    gsb_data_report_set_show_report_date ( report_number,
						   1 );
	    gsb_data_report_set_show_report_payee ( report_number,
						    1 );
	    gsb_data_report_set_show_report_category ( report_number,
						       1 );
	    gsb_data_report_set_split_credit_debit ( report_number,
						     1 );
	    gsb_data_report_set_transfer_choice ( report_number,
						  2 );
	    gsb_data_report_set_category_used ( report_number,
						1 );
	    gsb_data_report_set_category_show_category_amount ( report_number,
								1 );
	    gsb_data_report_set_category_show_sub_category ( report_number,
							     1 );
	    gsb_data_report_set_category_show_sub_category_amount ( report_number,
								    1 );
	    gsb_data_report_set_category_show_name ( report_number,
						     1 );
	    gsb_data_report_set_amount_comparison_only_report_non_null ( report_number,
									 1 );


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 4:

	    /* Cheques deposit */

	    report_number = gsb_data_report_new (_("Cheques deposit"));

	    /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 1 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 2 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 3 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 4 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 5 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 6 )));

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general ( report_number,
						   1 );
	    gsb_data_report_set_category_currency ( report_number,
						    1 );
	    gsb_data_report_set_budget_currency ( report_number,
						  1 );
	    gsb_data_report_set_payee_currency ( report_number,
						 1 );
	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     1 );


	    gsb_data_report_set_show_report_transactions ( report_number,
							   1 );
	    gsb_data_report_set_show_report_transaction_amount ( report_number,
								 1 );
	    gsb_data_report_set_show_report_payee ( report_number,
						    1 );
	    gsb_data_report_set_show_report_bank_references ( report_number,
							      1 );
	    gsb_data_report_set_transfer_choice ( report_number,
						  2 );
	    gsb_data_report_set_amount_comparison_only_report_non_null ( report_number,
									 1 );
	    gsb_data_report_set_amount_comparison_used ( report_number,
							 1 );

	    /* on doit créer une structure de montant qui dit que ça va être positif */

	    amount_comparison_number = gsb_data_report_amount_comparison_new (0);
	    gsb_data_report_amount_comparison_set_report_number ( amount_comparison_number,
								  report_number);
	    gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( amount_comparison_number,
										   -1 );
	    gsb_data_report_amount_comparison_set_first_comparison ( amount_comparison_number,
								     8 );
	    gsb_data_report_amount_comparison_set_link_first_to_second_part ( amount_comparison_number,
									      3 );

	    gsb_data_report_set_amount_comparison_list ( report_number,
							 g_slist_append ( gsb_data_report_get_amount_comparison_list (report_number),
									  GINT_TO_POINTER (amount_comparison_number)));

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 5:

	    /* Monthly outgoings by payee */

	    report_number = gsb_data_report_new (_("Monthly outgoings by payee"));

	    /*   le classement de base est 1-2-3-4-5-6  */

	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 6 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 1 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 2 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 3 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 4 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 5 )));


	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general ( report_number,
						   1 );
	    gsb_data_report_set_category_currency ( report_number,
						    1 );
	    gsb_data_report_set_budget_currency ( report_number,
						  1 );
	    gsb_data_report_set_payee_currency ( report_number,
						 1 );
	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     1 );


	    gsb_data_report_set_show_report_transactions ( report_number,
							   1 );
	    gsb_data_report_set_show_report_transaction_amount ( report_number,
								 1 );
	    gsb_data_report_set_show_report_payee ( report_number,
						    1 );
	    gsb_data_report_set_sorting_report ( report_number,
						 2 );
	    gsb_data_report_set_column_title_show ( report_number,
						    1 );
	    gsb_data_report_set_date_type ( report_number,
					    7 );
	    gsb_data_report_set_category_used ( report_number,
						1 );
	    gsb_data_report_set_category_show_category_amount ( report_number,
								1 );
	    gsb_data_report_set_category_show_sub_category ( report_number,
							     1 );
	    gsb_data_report_set_category_show_name ( report_number,
						     1 );

	    gsb_data_report_set_show_report_date ( report_number,
						   1 );
	    gsb_data_report_set_show_report_category ( report_number,
						       1 );
	    gsb_data_report_set_split_credit_debit ( report_number,
						     1 );
	    gsb_data_report_set_transfer_choice ( report_number,
						  2 );
	    gsb_data_report_set_category_show_sub_category_amount ( report_number,
								    1 );
	    gsb_data_report_set_amount_comparison_only_report_non_null ( report_number,
									 1 );
	    gsb_data_report_set_payee_used ( report_number,
					     1 );
	    gsb_data_report_set_payee_show_payee_amount ( report_number,
							  1 );
	    gsb_data_report_set_payee_show_name ( report_number,
						  1 );


	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;

	case 6:

	    /* Search */

	    report_number = gsb_data_report_new (_("Search"));

	    /*   le classement de base est 1-2-3-4-5-6  */

	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 1 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 2 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 3 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 4 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 5 )));
	    gsb_data_report_set_sorting_type ( report_number,
					       g_slist_append ( gsb_data_report_get_sorting_type (report_number),
								GINT_TO_POINTER ( 6 )));

	    /*   les devises sont à 1 (euro) */

	    gsb_data_report_set_currency_general ( report_number,
						   1 );
	    gsb_data_report_set_category_currency ( report_number,
						    1 );
	    gsb_data_report_set_budget_currency ( report_number,
						  1 );
	    gsb_data_report_set_payee_currency ( report_number,
						 1 );
	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     1 );


	    gsb_data_report_set_show_report_transactions ( report_number,
							   1 );
	    gsb_data_report_set_show_report_date ( report_number,
						   1 );
	    gsb_data_report_set_show_report_payee ( report_number,
						    1 );
	    gsb_data_report_set_show_report_category ( report_number,
						       1 );
	    gsb_data_report_set_show_report_sub_category ( report_number,
							   1 );
	    gsb_data_report_set_show_report_method_of_payment ( report_number,
								1 );
	    gsb_data_report_set_show_report_budget ( report_number,
						     1 );
	    gsb_data_report_set_show_report_sub_budget ( report_number,
							 1 );
	    gsb_data_report_set_show_report_method_of_payment_content ( report_number,
									1 );
	    gsb_data_report_set_show_report_note ( report_number,
						   1 );
	    gsb_data_report_set_show_report_voucher ( report_number,
						      1 );
	    gsb_data_report_set_show_report_marked ( report_number,
						     1 );
	    gsb_data_report_set_show_report_bank_references ( report_number,
							      1 );
	    gsb_data_report_set_show_report_financial_year ( report_number,
							     1 );

	    gsb_data_report_set_report_can_click ( report_number,
						   1 );
	    gsb_data_report_set_date_type ( report_number,
					    4 );
	    gsb_data_report_set_period_split ( report_number,
					       1 );
	    gsb_data_report_set_period_split_type ( report_number,
						    2 );
	    gsb_data_report_set_transfer_choice ( report_number,
						  2 );

	    /*   tout le reste est à NULL, ce qui est très bien */

	    break;


	default :
	    dialogue_error ( _("Unknown report type, creation cancelled"));
	    return FALSE;
    }

    /* Add an entry in navigation pane. */
    gsb_gui_navigation_add_report ( report_number );
    gsb_gui_update_gui_to_report ( report_number );

    personnalisation_etat ();
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * Callback triggered when user change the menu of template reports in
 * the "new report" dialog,
 *
 * \param menu_item		Menu item that triggered signal.
 * \param label_description	A GtkLabel to fill with the long
 *				description of template report.
 */
void change_choix_nouvel_etat ( GtkWidget *menu_item, GtkWidget *label_description )
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



/**
 * Delete current report, after a confirmation dialog.
 */
void efface_etat ( void )
{
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( !current_report_number )
	return;

     if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general),
				7 );

   if ( !question_yes_no_hint ( g_strdup_printf (_("Delete report \"%s\"?"),
						  gsb_data_report_get_report_name (current_report_number) ),
				 _("This will irreversibly remove this report.  There is no undo for this.") ))
	return;

   /* remove the report */

    gsb_data_report_remove ( current_report_number );

    /* Update reports list in navigation. */
    gsb_gui_navigation_remove_report ( current_report_number);

    gsb_gui_unsensitive_report_widgets ();

    modification_fichier ( TRUE );

}



/**
 * Set widgets associated to active report unsensitive.  For instance
 * when there is no selected report.
 */
void gsb_gui_unsensitive_report_widgets ()
{
    if ( scrolled_window_etat && GTK_IS_WIDGET ( scrolled_window_etat ) && 
	 GTK_BIN ( scrolled_window_etat ) -> child )
	gtk_widget_hide ( GTK_BIN ( scrolled_window_etat ) -> child );

    gtk_widget_set_sensitive ( bouton_personnaliser_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, FALSE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, FALSE );
}


/**
 * Set widgets associated to active report sensitive.  For instance
 * when a report has just been selected.
 *
 * \param report_number		Report to display.
 */
void gsb_gui_update_gui_to_report ( gint report_number )
{
    gtk_widget_set_sensitive ( bouton_personnaliser_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_imprimer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_exporter_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_dupliquer_etat, TRUE );
    gtk_widget_set_sensitive ( bouton_effacer_etat, TRUE );

    rafraichissement_etat ( report_number );
}



/**
 * Export a report as a HTML file.  It uses a "benj's meta structure"
 * affichage_etat structure as a backend.
 *
 * \param report_number		Report to export as HTML.
 * \param filename		Filename to save report into.
 */
void export_etat_vers_html ( gint report_number, gchar * filename )
{
    affichage_etat ( report_number, &html_affichage, filename );
}



/**
 * Export current report as a HTML file.  It uses a "benj's meta
 * structure" affichage_etat structure as a backend.
 *
 * \param filename		Filename to save report into.
 */
void export_etat_courant_vers_html ( gchar * filename )
{
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general), 7 );

    export_etat_vers_html ( gsb_gui_navigation_get_current_report (), filename );
}



/**
 * Export current report as a CSV file.  It uses a "benj's meta
 * structure" affichage_etat structure as a backend.
 *
 * \param filename		Filename to save report into.
 */
void export_etat_courant_vers_csv ( gchar * filename )
{
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general), 7 );

    affichage_etat ( 0, &csv_affichage, filename );
}



/**
 * Callback of the file format menu in the export report dialog.  It
 * is responsible to change the "format" object property of the
 * dialog, which is used when it is closed to determine format.  
 * 
 * It is also responsible to change the default value of filename in
 * the selector.  For that, it uses the "basename" property set in the
 * chosser creation.  The "basename" property is the report name.
 * 
 * \param combo		Combo box that triggered event.
 * \param selector	GtkFileChooser containing combo.
 * 
 * \return FALSE
 */
gboolean gsb_report_export_change_format ( GtkWidget * combo, GtkWidget * selector )
{
    gchar * name, * extension;

    g_object_set_data ( G_OBJECT(selector), "format", 
			(gpointer) gtk_combo_box_get_active ( GTK_COMBO_BOX(combo) ) );

    name = safe_file_name ( g_object_get_data ( selector, "basename" ) );
    switch ( gtk_combo_box_get_active ( GTK_COMBO_BOX(combo) ) )
    {
	    case 0:		/* EGSB */
		extension = "egsb";
		break;

	    case 1:		/* HTML */
		extension = "html";
		break;

	    case 2:		/* CSV */
		extension = "csv";
		break;

	    case 3:		/* Postscript */
		extension = "ps";
		break;

		extension = "tex";
	    case 4:		/* Latex */
		break;

	    default :
		extension = NULL;
		break;
    }

    gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER(selector), 
					g_strconcat ( name, ".", extension, NULL ) );
    return FALSE;
}



/**
 * This is responsible of exporting current report.  This function
 * will ask a filename, a format, and call the appropriate benj's meta
 * backend to do the job.
 */
void exporter_etat ( void )
{
    GtkWidget * fenetre_nom, *hbox, * combo;
    gint resultat, current_report_number;
    gchar * nom_etat;
    struct print_config * print_config_backup;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general), 7 );
    
    fenetre_nom = file_selection_new ( _("Export report"), FILE_SELECTION_IS_SAVE_DIALOG );
    g_object_set_data ( G_OBJECT(fenetre_nom), "basename", 
			gsb_data_report_get_report_name ( gsb_gui_navigation_get_current_report () ) );
    
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new ( COLON(_("File format")) ), 
			 FALSE, FALSE, 0 );
    
    combo = gtk_combo_box_new_text();
    gtk_box_pack_start ( GTK_BOX(hbox), combo, TRUE, TRUE, 0 );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("Grisbi report file (egsb file)" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("HTML file" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("CSV file" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("Postscript file" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("Latex file" ) );
    gtk_combo_box_set_active ( GTK_COMBO_BOX(combo), 0 );
    g_signal_connect ( G_OBJECT(combo), "changed", 
		       G_CALLBACK ( gsb_report_export_change_format ), 
		       (gpointer) fenetre_nom );
    gtk_widget_show_all ( hbox );
    gtk_file_chooser_set_extra_widget ( GTK_FILE_CHOOSER(fenetre_nom), hbox );
    
    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));
    if ( resultat == GTK_RESPONSE_OK )
    {
	nom_etat = file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));
	
	switch ( (gint) g_object_get_data ( G_OBJECT(fenetre_nom), "format" ) )
	{
	    case 0:		/* EGSB */
		gsb_file_others_save_report ( nom_etat );
		break;
		
	    case 1:		/* HTML */
		export_etat_courant_vers_html ( nom_etat );
		break;

	    case 2:		/* CSV */
		export_etat_courant_vers_csv ( nom_etat );
		break;

	    case 3:		/* Postscript */
		print_config_backup = print_config_dup ( );
		etat.print_config.printer = FALSE;
		etat.print_config.filetype = POSTSCRIPT_FILE;
		etat.print_config.printer_filename = nom_etat;
		affichage_etat ( gsb_gui_navigation_get_current_report (),
				 &latex_affichage, nom_etat );
		print_config_set ( print_config_backup );
		break;

	    case 4:		/* Latex */
		print_config_backup = print_config_dup ( );
		etat.print_config.printer = FALSE;
		etat.print_config.filetype = LATEX_FILE;
		etat.print_config.printer_filename = nom_etat;
		affichage_etat ( gsb_gui_navigation_get_current_report (),
				 &latex_affichage, nom_etat );
		print_config_set ( print_config_backup );
		break;

	    default :
		break;
	}
    }
    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
}



/**
 * Import a report.
 */
void importer_etat ( void )
{
    GtkWidget *fenetre_nom;
    gint resultat;
    gchar *nom_etat;

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general), 7 );

    fenetre_nom = file_selection_new ( _("Import a report") , FILE_SELECTION_MUST_EXIST);
    file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				  dernier_chemin_de_travail );
    file_selection_set_entry (  GTK_FILE_SELECTION ( fenetre_nom ),
				g_strconcat ( dernier_chemin_de_travail, ".egsb", NULL ));

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_etat =file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom ));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* la vérification que c'est possible a �t� faite par la boite de selection*/
	    if ( !gsb_file_others_load_report ( nom_etat ))
	    {
		return;
	    }
	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return;
    }
}



/**
 * Make a copy of current report and update user interface.
 */
void dupliquer_etat ( void )
{
    gint report_number, current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != 7 )
	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general), 7 );

    report_number = gsb_data_report_dup (current_report_number);

    gsb_gui_update_gui_to_report ( report_number );

    personnalisation_etat ();
    modification_fichier ( TRUE );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
