/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cedric Auger (cedric@grisbi.org)            */
/*          2002-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
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

/**
 * \file etats_onglet.c
 * Handle all UI actions for the reports.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "etats_onglet.h"
#include "etats_calculs.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "gsb_automem.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "gsb_file_others.h"
#include "navigation.h"
#include "gsb_report.h"
#include "gsb_status.h"
#include "traitement_variables.h"
#include "utils.h"
#include "etats_config.h"
#include "print_config.h"
#include "print_report.h"
#include "utils_files.h"
#include "structures.h"
#include "fenetre_principale.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean ajout_etat ( void );
static void change_choix_nouvel_etat ( GtkWidget *combobox, GtkWidget *label_description );
static void dupliquer_etat ( void );
static void efface_etat ( void );
static void export_etat_courant_vers_csv ( gchar * filename );
static void export_etat_courant_vers_html ( gchar * filename );
static void exporter_etat ( void );
static GtkWidget *gsb_gui_create_report_toolbar ( void );
static gboolean gsb_report_export_change_format ( GtkWidget * combo, GtkWidget * selector );
static void importer_etat ( void );
/*END_STATIC*/


static GtkWidget *bouton_effacer_etat = NULL;
static GtkWidget *bouton_personnaliser_etat = NULL;
static GtkWidget *bouton_imprimer_etat = NULL;
static GtkWidget *bouton_exporter_etat = NULL;
static GtkWidget *bouton_dupliquer_etat = NULL;
GtkWidget *scrolled_window_etat = NULL;          /* contient l'état en cours */
gint nb_colonnes;
gint ligne_debut_partie;
GtkWidget *notebook_etats = NULL;
GtkWidget *notebook_config_etat = NULL;
static GtkWidget *reports_toolbar = NULL;


/*START_EXTERN*/
extern struct struct_etat_affichage csv_affichage;
extern struct struct_etat_affichage html_affichage;
extern struct struct_etat_affichage latex_affichage;
extern GtkWidget *notebook_general;
extern GtkWidget *window;
/*END_EXTERN*/

/** Different formats supported.  */
enum report_export_formats {
    REPORT_EGSB, REPORT_HTML, REPORT_CSV, REPORT_PS, REPORT_TEX,
    REPORT_MAX_FORMATS,
};



/**
 * Create a toolbar containing all necessary controls on reports tab.
 *
 * \return a newly-allocated hbox
 */
GtkWidget *gsb_gui_create_report_toolbar ( void )
{
    GtkWidget *hbox, *handlebox, *hbox2, *button;

    hbox = gtk_hbox_new ( FALSE, 5 );

    /* HandleBox */
    handlebox = gtk_handle_box_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), handlebox, TRUE, TRUE, 0 );
    /* Hbox2 */
    hbox2 = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox2 );

    /* Add various icons */
    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("New report"),
					       "new-report.png",
					       G_CALLBACK ( ajout_etat ),
					       NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  _("Create a new report") );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, FALSE, 0 );

    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_OPEN,
					   _("Import"),
					   G_CALLBACK (importer_etat),
					   NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (button),
				  _("Import a Grisbi report file (.egsb)") );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), button, FALSE, FALSE, 0 );

    bouton_exporter_etat = gsb_automem_stock_button_new ( etat.display_toolbar,
							 GTK_STOCK_SAVE,
							 _("Export"),
							 G_CALLBACK (exporter_etat),
							 NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (bouton_exporter_etat),
				  _("Export selected report to egsb, HTML, Tex, CSV, PostScript") );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_exporter_etat, FALSE, FALSE, 0 );

    /* print button */
    bouton_imprimer_etat = gsb_automem_stock_button_new ( etat.display_toolbar,
							  GTK_STOCK_PRINT,
							  _("Print"),
							  G_CALLBACK (print_report),
							  NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (bouton_imprimer_etat),
                   _("Print selected report") );

    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_imprimer_etat, FALSE, FALSE, 0 );

    bouton_effacer_etat = gsb_automem_stock_button_new ( etat.display_toolbar,
							 GTK_STOCK_DELETE,
							 _("Delete"),
							 G_CALLBACK ( efface_etat ),
							 NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET (bouton_effacer_etat),
				   _("Delete selected report") );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_effacer_etat, FALSE, FALSE, 0 );

    bouton_personnaliser_etat = gsb_automem_stock_button_new ( etat.display_toolbar,
							      GTK_STOCK_PROPERTIES,
							      _("Properties"),
							      G_CALLBACK (personnalisation_etat),
							      NULL ),
    gtk_widget_set_tooltip_text ( GTK_WIDGET (bouton_personnaliser_etat),
				  _("Edit selected report") );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_personnaliser_etat, FALSE, FALSE, 0 );

    bouton_dupliquer_etat = gsb_automem_stock_button_new ( etat.display_toolbar,
							  GTK_STOCK_COPY,
							  _("Clone"),
							  G_CALLBACK (dupliquer_etat),
							  NULL ),
    gtk_widget_set_tooltip_text ( GTK_WIDGET (bouton_dupliquer_etat),
				  _("Clone selected report") );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_dupliquer_etat, FALSE, FALSE, 0 );

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
    reports_toolbar = gsb_gui_create_report_toolbar();
    gtk_box_pack_start ( GTK_BOX ( tab ), reports_toolbar, FALSE, FALSE, 0 );

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

    gtk_widget_show_all ( tab );

    gsb_gui_unsensitive_report_widgets ();

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
    GtkWidget *dialog, *frame, *combobox, *label_description;
    GtkWidget *scrolled_window;


    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general ) ) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), GSB_REPORTS_PAGE );

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_OK_CANCEL,
				       make_hint ( _("Choose template for new report"),
						   _("You are about to create a new report.  For convenience, you can choose between the following templates.  Reports may be customized later." ) ) );

    frame = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, FALSE,
					_("Report type"));

    /* combobox for predefined reports */
    combobox = gtk_combo_box_new_text ();
    gtk_box_pack_start ( GTK_BOX(frame), combobox, FALSE, FALSE, 0 );

    /* on ajoute maintenant la frame */
    frame = new_paddingbox_with_title ( GTK_DIALOG(dialog)->vbox, TRUE,
					_("Description"));

    /* on met le label dans une scrolled window */
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX(frame), scrolled_window, TRUE, TRUE, 6 );

    /* on ajoute maintenant le label */
    label_description = gtk_label_new ( NULL );
    gtk_misc_set_alignment ( GTK_MISC ( label_description ), 0, 0 );
    gtk_label_set_line_wrap ( GTK_LABEL ( label_description ), TRUE );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    label_description );

    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( label_description -> parent ),
				   GTK_SHADOW_NONE );

	/* fill combobox */
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), _("Last month incomes and outgoings") );
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), _("Current month incomes and outgoings") );
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), _("Annual budget") );
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), _("Blank report") );
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), _("Cheques deposit") );
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), _("Monthly outgoings by payee") );
	gtk_combo_box_append_text ( GTK_COMBO_BOX ( combobox ), _("Search") );


	/* set first entry and description */
	gtk_combo_box_set_active ( GTK_COMBO_BOX ( combobox ), 0 );
	change_choix_nouvel_etat ( combobox, label_description );

	/* update description based on selection */
	g_signal_connect ( G_OBJECT ( combobox ), "changed",
			G_CALLBACK ( change_choix_nouvel_etat ),
			G_OBJECT ( label_description ));

    gtk_box_set_spacing ( GTK_BOX(GTK_DIALOG(dialog)->vbox), 6 );
    gtk_widget_show_all ( dialog );

    /* on attend le choix de l'utilisateur */
    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( dialog );
	return FALSE;
    }

	/* get the wanted report */
	resultat = gtk_combo_box_get_active ( GTK_COMBO_BOX (combobox) );
	gtk_widget_destroy ( GTK_WIDGET ( dialog ));

    /* create and fill the new report */

    switch ( resultat )
    {
	case 0:
	    /*  Last month incomes and outgoings  */

	    report_number = gsb_data_report_new (_("Last month incomes and outgoings"));

	    gsb_data_report_set_split_credit_debit ( report_number,
						     1 );
	    gsb_data_report_set_date_type ( report_number, 7 );

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

	    /* Blank report */

	    report_number = gsb_data_report_new (_("Blank report"));

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
	    gsb_data_report_set_show_report_value_date ( report_number,
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
	    gsb_data_report_set_show_report_value_date ( report_number,
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
	    gsb_data_report_set_show_report_value_date ( report_number,
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
						    3 );
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
    if ( etat.modification_fichier == 0 )
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
void change_choix_nouvel_etat ( GtkWidget *combobox, GtkWidget *label_description )
{
    gchar *description;

    switch ( gtk_combo_box_get_active ( GTK_COMBO_BOX (combobox) ) )
    {
	case 0:
	    /* Last month incomes and outgoings */

	    description = _("This report displays totals for last month's transactions sorted by categories and sub-categories. You just need to select the account(s). By default, all accounts are selected.");
	    break;

	case 1:
	    /* Current month incomes and outgoings */

	    description = _("This report displays totals of current month's transactions sorted by categories and sub-categories. You just need to select the account(s). By default, all accounts are selected.");
	    break;

	case 2:
	    /* Annual budget */

	    description = _("This report displays annual budget. You just need to select the account(s). By default all accounts are selected.");
	    break;

	case 3:
	    /* Blank report */

	    description = _("This report is an empty one. You need to customize it entirely.");
	    break;

	case 4:
	    /* Cheques deposit */

	    description = _("This report displays the cheques deposit. You just need to select the account(s). By default all accounts are selected.");
	    break;

	case 5:
	    /* Monthly outgoings by payee */

	    description = _("This report displays current month's outgoings sorted by payees. You just need to select the account(s). By default all accounts are selected.");
	    break;

	case 6:
	    /* Search */

	    description = _("This report displays all the information for all transactions of all accounts for the current year. You just have to add the amount, date, payees etc. criteria thant you want. By default the transactions are clickables.");
	    break;

	default:

	    description = _("No description available");
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

     if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general),
				GSB_REPORTS_PAGE );

   if ( !question_yes_no_hint ( g_strdup_printf (_("Delete report \"%s\"?"),
						  gsb_data_report_get_report_name (current_report_number) ),
				_("This will irreversibly remove this report.  There is no undo for this."),
				GTK_RESPONSE_NO ))
	return;

   /* remove the report */

    gsb_data_report_remove ( current_report_number );

    /* Update reports list in navigation. */
    gsb_gui_navigation_remove_report ( current_report_number);

    gsb_gui_unsensitive_report_widgets ();

    if ( etat.modification_fichier == 0 )
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

    if (gsb_report_get_current () != report_number)
    {
	rafraichissement_etat ( report_number );
	gsb_report_set_current (report_number);
    }
    else
        etat_affiche_finish ( );
;
}



/**
 * Export current report as a HTML file.  It uses a "benj's meta
 * structure" affichage_etat structure as a backend.
 *
 * \param filename		Filename to save report into.
 */
void export_etat_courant_vers_html ( gchar * filename )
{
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    affichage_etat ( gsb_gui_navigation_get_current_report (), &html_affichage, filename );
}



/**
 * Export current report as a CSV file.  It uses a "benj's meta
 * structure" affichage_etat structure as a backend.
 *
 * \param filename		Filename to save report into.
 */
void export_etat_courant_vers_csv ( gchar * filename )
{
    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    affichage_etat ( gsb_gui_navigation_get_current_report (), &csv_affichage, filename );
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
			GINT_TO_POINTER ( gtk_combo_box_get_active ( GTK_COMBO_BOX(combo) ) ));

    name = safe_file_name ( g_object_get_data ( G_OBJECT ( selector ), "basename" ) );
    switch ( gtk_combo_box_get_active ( GTK_COMBO_BOX(combo) ) )
    {
	    case REPORT_EGSB:		/* EGSB */
		extension = "egsb";
		break;

	    case REPORT_HTML:		/* HTML */
		extension = "html";
		break;

	    case REPORT_CSV:		/* CSV */
		extension = "csv";
		break;

	    case REPORT_PS:		/* Postscript */
		extension = "ps";
		break;

	    case REPORT_TEX:		/* Latex */
		extension = "tex";
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
    GtkWidget *fenetre_nom, *hbox, * combo;
    gint resultat, current_report_number;
    gchar * nom_etat;
    struct print_config * print_config_backup;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    fenetre_nom = gtk_file_chooser_dialog_new ( _("Export report"),
					   GTK_WINDOW ( window ),
					   GTK_FILE_CHOOSER_ACTION_SAVE,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_SAVE, GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( fenetre_nom ), gsb_file_get_last_path () );
    gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( fenetre_nom ), TRUE);
    gtk_window_set_position ( GTK_WINDOW ( fenetre_nom ), GTK_WIN_POS_CENTER_ON_PARENT );

    g_object_set_data ( G_OBJECT(fenetre_nom), "basename",
			 gsb_data_report_get_report_name ( gsb_gui_navigation_get_current_report () ) );

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new ( _("File format: ") ),
			 FALSE, FALSE, 0 );

    combo = gtk_combo_box_new_text();
    gtk_box_pack_start ( GTK_BOX(hbox), combo, TRUE, TRUE, 0 );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("Grisbi report file (egsb file)" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("HTML file" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("CSV file" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("Postscript file" ) );
    gtk_combo_box_append_text ( GTK_COMBO_BOX(combo), _("Latex file" ) );

    /* Set initial format. */
    gtk_combo_box_set_active ( GTK_COMBO_BOX(combo), REPORT_HTML );
    gsb_report_export_change_format ( combo, fenetre_nom );

    g_signal_connect ( G_OBJECT(combo), "changed",
		       G_CALLBACK ( gsb_report_export_change_format ),
		       fenetre_nom );
    gtk_widget_show_all ( hbox );
    gtk_file_chooser_set_extra_widget ( GTK_FILE_CHOOSER(fenetre_nom), hbox );

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));
    if ( resultat == GTK_RESPONSE_OK )
    {
	nom_etat = file_selection_get_filename ( GTK_FILE_CHOOSER ( fenetre_nom ));

	gsb_status_message ( _("Exporting report ...") );

	gsb_status_wait ( TRUE );

	switch ( GPOINTER_TO_INT(g_object_get_data ( G_OBJECT(fenetre_nom), "format" ) ) )
	{
	    case REPORT_EGSB:		/* EGSB */
		gsb_file_others_save_report ( nom_etat );
		break;

	    case REPORT_HTML:		/* HTML */
		export_etat_courant_vers_html ( nom_etat );
		break;

	    case REPORT_CSV:		/* CSV */
		export_etat_courant_vers_csv ( nom_etat );
		break;

	    case REPORT_PS:		/* Postscript */
		print_config_backup = print_config_dup ( );
		etat.print_config.printer = FALSE;
		etat.print_config.filetype = POSTSCRIPT_FILE;
		etat.print_config.printer_filename = nom_etat;
		affichage_etat ( gsb_gui_navigation_get_current_report (),
				 &latex_affichage, nom_etat );
		print_config_set ( print_config_backup );
		break;

	    case REPORT_TEX:		/* Latex */
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

	gsb_status_stop_wait ( TRUE );

	gsb_status_message ( _("Done") );
    }
    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (fenetre_nom), TRUE));
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
    GtkFileFilter * filter;

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    fenetre_nom = gtk_file_chooser_dialog_new ( _("Import a report"),
					   GTK_WINDOW ( window ),
					   GTK_FILE_CHOOSER_ACTION_OPEN,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( fenetre_nom ), gsb_file_get_last_path () );
    gtk_window_set_position ( GTK_WINDOW ( fenetre_nom ), GTK_WIN_POS_CENTER_ON_PARENT );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Grisbi report files (*.egsb)") );
    gtk_file_filter_add_pattern ( filter, "*.egsb" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( fenetre_nom ), filter );
    gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( fenetre_nom ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("All files") );
    gtk_file_filter_add_pattern ( filter, "*" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( fenetre_nom ), filter );

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nom_etat =file_selection_get_filename ( GTK_FILE_CHOOSER ( fenetre_nom ));
	    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (fenetre_nom), TRUE));
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

    if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general)) != GSB_REPORTS_PAGE )
	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general), GSB_REPORTS_PAGE );

    report_number = gsb_data_report_dup (current_report_number);

    gsb_gui_navigation_add_report (report_number);
    gsb_gui_update_gui_to_report ( report_number );

    personnalisation_etat ();
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
