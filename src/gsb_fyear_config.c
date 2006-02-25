/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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
 * \file gsb_fyear_config.c
 * work with the configuration of the finacial year
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_fyear_config.h"
#include "utils_buttons.h"
#include "utils_dates.h"
#include "gsb_transactions_list.h"
#include "utils_editables.h"
#include "gsb_data_account.h"
#include "gsb_data_fyear.h"
#include "gsb_data_transaction.h"
#include "traitement_variables.h"
#include "utils.h"
#include "dialog.h"
#include "gsb_fyear.h"
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajout_exercice ( GtkWidget *bouton,
		      GtkWidget *clist );
static void association_automatique ( void );
static void deselection_ligne_exercice ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev);
static void selection_ligne_exercice ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev );
static void supprime_exercice ( GtkWidget *bouton, GtkWidget *liste );
static gboolean update_financial_year_list ( GtkEntry *entry, gchar *value, 
				      gint length, gint * position );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *affichage_exercice;
extern GtkWidget *bouton_supprimer_exercice;
extern GtkWidget *clist_exercices_parametres;
extern GtkWidget *debut_exercice;
extern GtkWidget *fenetre_preferences;
extern GtkWidget *fin_exercice;
extern gint ligne_selection_exercice;
extern GtkWidget *nom_exercice;
extern GtkWidget *paddingbox_details;
/*END_EXTERN*/


/**
 * Creates the "Financial years" tab.  It creates a financial years
 * list and then a form that allows to edit selected financial year.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_exercices ( void )
{
    GtkWidget *vbox_pref, *label;
    GtkWidget *scrolled_window, *vbox, *bouton, *hbox;
    GtkWidget *paddingbox, *table;
    GSList *tmp_list;
    gchar *titres[]={_("Name")};

    vbox_pref = new_vbox_with_title_and_icon ( _("Financial years"),
					       "financial-years.png" );
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Known financial years") );
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    /* Create financial years list */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize ( GTK_WIDGET ( scrolled_window ), FALSE, 120 );
    clist_exercices_parametres = gtk_clist_new_with_titles ( 1, titres );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_exercices_parametres ),
				       0, TRUE );
    gtk_clist_column_titles_passive ( GTK_CLIST ( clist_exercices_parametres ));
    gtk_signal_connect_object  ( GTK_OBJECT ( fenetre_preferences ),
				 "close",
				 GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				 GTK_OBJECT ( clist_exercices_parametres ));
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			clist_exercices_parametres );


    /* Do not activate unless an account is opened */
    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }
    else
    {
	tmp_list = gsb_data_fyear_get_fyears_list ();

	while ( tmp_list )
	{
	    gint fyear_number;
	    gchar *string[1];
	    gint row;

	    fyear_number = gsb_data_fyear_get_no_fyear (tmp_list -> data);

	    string[0] = gsb_data_fyear_get_name (fyear_number);

	    row = gtk_clist_append ( GTK_CLIST (clist_exercices_parametres),
				     string );

	    /* on associe à la string la struct de la exercice */
	    gtk_clist_set_row_data ( GTK_CLIST ( clist_exercices_parametres ),
				     row,
				     GINT_TO_POINTER (fyear_number));
	    tmp_list = tmp_list -> next;
	}
    }

    /* Order buttons in a vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			 FALSE, FALSE, 0 );

    /* Add button */
    bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
			 GTK_SIGNAL_FUNC  ( ajout_exercice ),
			 clist_exercices_parametres );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
			 FALSE, FALSE, 5 );
    /* Remove button */
    bouton_supprimer_exercice = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_exercice ), "clicked",
			 GTK_SIGNAL_FUNC  ( supprime_exercice ),
			 clist_exercices_parametres );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_exercice,
			 FALSE, FALSE, 5 );
    /* Associate operations */
    bouton = gtk_button_new_with_label ( _("Associate operations without financial years") );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( association_automatique ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
			 FALSE, FALSE, 5 );
    gtk_widget_show ( bouton );

    /* Handle clist selections */
    gtk_signal_connect ( GTK_OBJECT ( clist_exercices_parametres ), "select-row",
			 GTK_SIGNAL_FUNC ( selection_ligne_exercice ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( clist_exercices_parametres ), "unselect-row",
			 GTK_SIGNAL_FUNC ( deselection_ligne_exercice ),
			 NULL );


    /* Financial year details */
    paddingbox_details = new_paddingbox_with_title ( vbox_pref, FALSE,
						     _("Financial year details") );

    /* Put stuff in a table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox_details ), table,
			 FALSE, FALSE, 0 );

    /* Financial year name */
    label = gtk_label_new ( COLON(_("Name")) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    nom_exercice = new_text_entry ( NULL, (GCallback) update_financial_year_list, NULL );
    gtk_table_attach ( GTK_TABLE ( table ),
		       nom_exercice, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Start */
    label = gtk_label_new ( COLON(_("Start")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    debut_exercice = new_date_entry ( NULL, NULL );
    gtk_table_attach ( GTK_TABLE ( table ),
		       debut_exercice, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* End */
    label = gtk_label_new ( COLON(_("End")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    fin_exercice = new_date_entry ( NULL, NULL );
    gtk_table_attach ( GTK_TABLE ( table ),
		       fin_exercice, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Activate in transaction form? */
    affichage_exercice = new_checkbox_with_title (_("Activate financial year in transaction form"),
						  NULL, 
						  G_CALLBACK(update_financial_year_menus));
    gtk_box_pack_start ( GTK_BOX ( paddingbox_details ), affichage_exercice,
			 FALSE, FALSE, 0 );

    gtk_widget_set_sensitive ( paddingbox_details, FALSE );
    gtk_widget_set_sensitive ( bouton_supprimer_exercice, FALSE );

    /* Select first entry if applicable */
    gtk_clist_select_row ( GTK_CLIST(clist_exercices_parametres), 0, 0 );

    return ( vbox_pref );
}
/* ************************************************************************************************************** */




/** 
 * Update financial year name in list.  Normally called as a signal
 * handler.
 *
 * \param entry Widget that triggered signal
 * \param value Handler parameter, not used
 * \param length Handler parameter, not used
 * \param position Handler parameter, not used
 */
gboolean update_financial_year_list ( GtkEntry *entry, gchar *value, 
				      gint length, gint * position )
{
    gint fyear_number;

    fyear_number = GPOINTER_TO_INT (gtk_clist_get_row_data ( GTK_CLIST ( clist_exercices_parametres ),
							     ligne_selection_exercice ));
    gsb_data_fyear_set_name ( fyear_number,
			      gtk_entry_get_text (GTK_ENTRY (nom_exercice)));
    gtk_clist_set_text ( GTK_CLIST(clist_exercices_parametres), 
			 ligne_selection_exercice, 0, 
			 gtk_entry_get_text (GTK_ENTRY(nom_exercice)) );
    return FALSE;
}



/* ***************************************************************************************************** */
/* Fonction ajout_exercice */
/* appelée par le bouton ajouter de l'onglet exercices des paramètres */
/* ***************************************************************************************************** */

void ajout_exercice ( GtkWidget *bouton,
		      GtkWidget *clist )
{
    gchar *string[1];
    gint row;
    gint fyear_number;

    /* Enlève la sélection de la liste ( ce qui nettoie les entrées ) */
    gtk_clist_unselect_all ( GTK_CLIST ( clist ));

    fyear_number = gsb_data_fyear_new (_("New financial year"));
    gsb_data_fyear_set_form_show ( fyear_number,
				   TRUE );

    string[0] = gsb_data_fyear_get_name (fyear_number);
    row = gtk_clist_append ( GTK_CLIST ( clist ), string );

    /* on associe à la ligne la struct de la exercice */
    gtk_clist_set_row_data ( GTK_CLIST ( clist ), row, GINT_TO_POINTER (fyear_number));

    /* on sélectionne le nouveau venu */
    gtk_clist_select_row ( GTK_CLIST ( clist ), row, 0 );

    /* on sélectionne le "nouvel exercice" et lui donne le focus */
    gtk_entry_select_region ( GTK_ENTRY ( nom_exercice ), 0, -1 );
    gtk_widget_grab_focus ( nom_exercice );

    /* Update various menus */
    update_financial_year_menus ();
}


/* **************************************************************************************************************************** */
/* Fonction supprime_exercice */
/* appelée lorsqu'on clicke sur le bouton annuler dans les paramètres */
/* **************************************************************************************************************************** */

void supprime_exercice ( GtkWidget *bouton, GtkWidget *liste )
{
    gint fyear_number;

    fyear_number = GPOINTER_TO_INT (gtk_clist_get_row_data ( GTK_CLIST ( liste ),
							     ligne_selection_exercice ));

    gtk_clist_remove ( GTK_CLIST(liste), ligne_selection_exercice );
    gsb_data_fyear_remove (fyear_number);

    if ( ligne_selection_exercice > 0 )
	gtk_clist_select_row ( GTK_CLIST(liste), ligne_selection_exercice-1, 0 );
    else
	gtk_clist_select_row ( GTK_CLIST(liste), 0, 0 );

    /* Update various menus */
    update_financial_year_menus ();
}
/* **************************************************************************************************************************** */



/**
 * Called when a line is selected in the financial years list.
 *
 * \param ligne Line number of selected line
 * \param colonne Column number of selected cell.  Not used.
 * \param ev GdkEventButton that triggered this handler.  Not used
 */
void selection_ligne_exercice ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev )
{
    gint fyear_number;
    /* with that, should not work, just for compile
     * have to change when do again that file */
    gchar *name;
    GDate *begining_date;
    GDate *end_date;
    gint form_show;

    ligne_selection_exercice = ligne;

    fyear_number = GPOINTER_TO_INT (gtk_clist_get_row_data ( GTK_CLIST ( liste ), ligne ));

    name = gsb_data_fyear_get_name(fyear_number);
    begining_date = gsb_data_fyear_get_begining_date (fyear_number);
    end_date = gsb_data_fyear_get_end_date (fyear_number);
    form_show = gsb_data_fyear_get_form_show (fyear_number);
    
    entry_set_value ( nom_exercice, &name);
    date_set_value ( debut_exercice, &begining_date, TRUE );
    date_set_value ( fin_exercice, &end_date, TRUE );
    checkbox_set_value ( affichage_exercice,
			 &form_show, 
			 TRUE );

    gtk_widget_set_sensitive ( paddingbox_details, TRUE );
    gtk_widget_set_sensitive ( bouton_supprimer_exercice, TRUE );
}




/**
 * Called when a line is deselected in the financial years list.
 *
 * \param ligne Line number of last selected line
 * \param colonne Column number of last selected cell.  Not used.
 * \param ev GdkEventButton that triggered this handler.  Not used
 */
void deselection_ligne_exercice ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev)
{
    /* Blank all entries */

    entry_set_value ( nom_exercice, NULL );
    entry_set_value ( get_entry_from_date_entry(debut_exercice), NULL );
    entry_set_value ( get_entry_from_date_entry(fin_exercice), NULL );
    checkbox_set_value ( affichage_exercice, FALSE, TRUE );

    gtk_widget_set_sensitive ( paddingbox_details, FALSE );
    gtk_widget_set_sensitive ( bouton_supprimer_exercice, FALSE );
}


/* ************************************************************************************************************** */
/* fonction association_automatique */
/* recherche les opés sans exercice, et les associe si possible avec un des exercice créé */
/* ************************************************************************************************************** */

void association_automatique ( void )
{
    GSList *list_tmp;

    if (!question_yes_no_hint ( _("Automatic association of financial years?"),
				_("This function assigns each transaction without a financial year to the one related to its transaction date.  If no financial year matches, the transaction will not be changed.")))
	return;

    list_tmp = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp )
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	if (!gsb_data_transaction_get_financial_year_number (transaction_number))
	{
	    gint fyear_number;

	    fyear_number = gsb_data_fyear_get_from_date (gsb_data_transaction_get_date (transaction_number));
	    if (fyear_number)
		gsb_data_transaction_set_financial_year_number ( transaction_number,
								 fyear_number );
	}
	list_tmp = list_tmp -> next;
    }

    demande_mise_a_jour_tous_comptes ();
    modification_fichier ( TRUE );
}
/* ************************************************************************************************************** */




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
