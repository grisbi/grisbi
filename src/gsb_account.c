/* ************************************************************************** */
/* Fichier qui s'occupe des manipulations de comptes                          */
/*                                                                            */
/*                         comptes_traitements.c                              */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2006 Benjamin Drieu (bdrieu@april.org)		      */
/*			2004 Alain Portal (aportal@univ-montp2.fr)	      */
/*			http://www.grisbi.org				      */
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

/*START_INCLUDE*/
#include "gsb_account.h"
#include "./comptes_gestion.h"
#include "./dialog.h"
#include "./gsb_data_account.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file.h"
#include "./gsb_form_scheduler.h"
#include "./navigation.h"
#include "./menu.h"
#include "./gsb_scheduler_list.h"
#include "./main.h"
#include "./categories_onglet.h"
#include "./imputation_budgetaire.h"
#include "./tiers_onglet.h"
#include "./traitement_variables.h"
#include "./etats_config.h"
#include "./structures.h"
#include "./fenetre_principale.h"
#include "./gsb_form_scheduler.h"
#include "./include.h"
#include "./gsb_data_account.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint gsb_account_ask_account_type ( void );
/*END_STATIC*/





/*START_EXTERN*/
extern GtkWidget *account_page;
extern GtkWidget *bouton_supprimer_compte;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget *notebook_general;
/*END_EXTERN*/



/**
 * called to create a new account
 * called by menu edit -> new account
 * !! here is just to add an account, for new complete file see gsb_file_new
 * 
 * \param none
 * 
 * \return FALSE if problem, TRUE if ok
 */

gboolean gsb_account_new ( void )
{
    kind_account type_de_compte;
    gint account_number;

    /*     ask for the kind_account */ 
    type_de_compte = gsb_account_ask_account_type ();
    if ( type_de_compte == -1 )
	return FALSE;

    /*     create the new account */ 
    account_number = gsb_data_account_new ( type_de_compte );
    if ( account_number == -1 )
    {
	dialogue_error_memory ();
	return FALSE;
    }

    /* set the default method of payment */
    gsb_data_payment_create_default (account_number);

    /* update the combofix for categ */ 
    mise_a_jour_combofix_categ();

    /* update the name of accounts in form */
    gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
				    FALSE );

    /* update the main page */ 
    mise_a_jour_liste_comptes_accueil = 1;

    remplissage_liste_comptes_etats ();
    selectionne_liste_comptes_etat_courant ();

    gtk_widget_set_sensitive ( bouton_supprimer_compte, TRUE );

    /* update the accounts lists */ 
    gsb_menu_update_accounts_in_menus (); 

    /* Add an entry in navigation pane. */
    gsb_gui_navigation_add_account ( account_number );

    /* Go to accounts properties */
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
			    GSB_ACCOUNT_PAGE );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( account_page ), 1 );
    remplissage_details_compte ();
    modification_fichier ( TRUE );
    return TRUE;
}



/** that function delete the current account selected in the account properties
 * \param none
 * \return FALSE FALSE
 * */
gboolean gsb_account_delete ( void )
{
    gint deleted_account;
    gint page_number;
    GSList *list_tmp;

    deleted_account = gsb_gui_navigation_get_current_account ();

    if ( !question_yes_no_hint ( g_strdup_printf (_("Delete account \"%s\"?"),
						  gsb_data_account_get_name (deleted_account)),
				 _("This will irreversibly remove this account and all operations that were previously contained.  There is no undo for this. Usually it's a better way to close an account."),
				 GTK_RESPONSE_NO ))
	return FALSE;

    /* if the las account, close the file */

    if ( gsb_data_account_get_accounts_amount () == 1 )
    {
	etat.modification_fichier = 0;
	gsb_file_close ();
	return FALSE;
    }

    /* delete the schedules transactions on that account */

    list_tmp = gsb_data_scheduled_get_scheduled_list ();

    while (list_tmp)
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp -> data);

	if ( gsb_data_scheduled_get_account_number (scheduled_number) == deleted_account )
	    gsb_data_scheduled_remove_scheduled (scheduled_number);

	list_tmp = list_tmp -> next;
    }


    /*     delete the account */

    gsb_data_account_delete ( deleted_account );

    /*     check all the transactions, and put -1 if it's a transfer to the deleted account */

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GSList *list_tmp_transactions;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number;
	    transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number) == i
		 &&
		 gsb_data_transaction_get_account_number_transfer (transaction_number) == deleted_account )
		gsb_data_transaction_set_account_number_transfer ( transaction_number,
								   -1);
	    list_tmp_transactions = list_tmp_transactions -> next;
	}
	list_tmp = list_tmp -> next;
    }

    /* check gsb_gui_navigation_get_current_account () and gsb_gui_navigation_get_current_account ()_onglet and put them
     * on the first account if they are on the deleted account */

    if ( gsb_gui_navigation_get_current_account () == deleted_account )
    {
	/* update the transaction list */

	page_number = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook_general));

	navigation_change_account ( GINT_TO_POINTER ( gsb_data_account_first_number () ));

	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), page_number );
    }

    /* update the buttons lists */

    gsb_menu_update_accounts_in_menus();

    /* update the combofixes if needed */

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation();

    /* Replace trees contents. */

    remplit_arbre_categ ();
    remplit_arbre_imputation ();
    remplit_arbre_tiers ();

    /* update the name of accounts in form */
    gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
				    FALSE );

    gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    remplissage_liste_comptes_etats ();
    selectionne_liste_comptes_etat_courant ();

    /* Update navigation pane. */
    gsb_gui_navigation_remove_account ( deleted_account );

    modification_fichier( TRUE ); 
    return FALSE;
}



/**
 * create a combobox containing the list of the accounts
 *
 * \param func Function to call when a line is selected (type : gboolean func ( GtkWidget *button, gpointer data )
 * \param data data to send to the function
 * \param include_closed If set to TRUE, include the closed accounts
 *
 * \return a new GtkCombobox containing the list of the accounts
 */
GtkWidget *gsb_account_create_combo_list ( GCallback func, 
					   gpointer data,
					   gboolean include_closed )
{
    GSList *list_tmp;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkWidget *combobox;

    combobox = gtk_combo_box_new ();

    store = gtk_list_store_new ( 2,
				 G_TYPE_STRING,
				 G_TYPE_INT );

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint account_number;
	GtkTreeIter iter;

	account_number = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( account_number >= 0 && ( !gsb_data_account_get_closed_account (account_number)
				      || include_closed ) )
	{
	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( store,
				 &iter,
				 0, gsb_data_account_get_name (account_number),
				 1, account_number,
				 -1 );
	}
	list_tmp = list_tmp -> next;
    }

    gtk_combo_box_set_model ( GTK_COMBO_BOX (combobox),
			      GTK_TREE_MODEL (store));
    if ( func )
	g_signal_connect ( G_OBJECT (combobox),
			   "changed",
			   G_CALLBACK(func),
			   data );

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer,
				    "text", 0,
				    NULL);

    return combobox;
}

/**
 * update the list of accounts in a combo_box filled
 * by gsb_account_create_combo_list
 *
 * \param combo_box
 * \param include_closed
 *
 * \return FALSE
 * */
gboolean gsb_account_update_combo_list ( GtkWidget *combo_box,
					 gboolean include_closed )
{
    GSList *list_tmp;
    GtkListStore *store;

    if (!combo_box)
	return FALSE;

    store = GTK_LIST_STORE (gtk_combo_box_get_model ( GTK_COMBO_BOX (combo_box)));;
    gtk_list_store_clear (store);

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint account_number;
	GtkTreeIter iter;

	account_number = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( account_number >= 0 && ( !gsb_data_account_get_closed_account (account_number)
				      || include_closed ) )
	{
	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( store,
				 &iter,
				 0, gsb_data_account_get_name (account_number),
				 1, account_number,
				 -1 );
	}
	list_tmp = list_tmp -> next;
    }
    return FALSE;
}

/**
 * get the selected account number in a combo_box filled by
 * gsb_account_create_combo_list
 *
 * \param combo_box the accounts combo-box
 *
 * \return the account number selected or -1 if none active
 * */
gint gsb_account_get_combo_account_number ( GtkWidget *combo_box )
{
    gint account_number;
    GtkTreeIter iter;

    if (!combo_box
	||
	!GTK_IS_COMBO_BOX (combo_box))
	return -1;

    if ( !gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combo_box),
					  &iter ))
	return -1;

    gtk_tree_model_get ( GTK_TREE_MODEL (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box))),
			 &iter,
			 1, &account_number,
			 -1 );
    return account_number;
}


/**
 *  Create a menu with the list of accounts.  This list is
 *  clickable and activates func if specified.
 *  used for now to add a submenu item in a main menu
 *
 * \param func Function to call when a line is selected
 * \param activate_currrent If set to TRUE, does not mark as
 *        unsensitive current account
 * \param include_closed If set to TRUE, include the closed accounts
 *
 * \return A newly created menu
 */
GtkWidget *gsb_account_create_menu_list ( GtkSignalFunc func, 
					  gboolean activate_currrent,
					  gboolean include_closed )
{
    GtkWidget *menu;
    GtkWidget *item;
    GSList *list_tmp;

    menu = gtk_menu_new ();

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if (  i >= 0 && ( !gsb_data_account_get_closed_account (i) || include_closed ) )
	{
	    item = gtk_menu_item_new_with_label ( gsb_data_account_get_name (i));
	    gtk_object_set_data ( GTK_OBJECT ( item ),
				  "account_number",
				  GINT_TO_POINTER (i));
	    if ( func )
		gtk_signal_connect ( GTK_OBJECT ( item ), "activate", GTK_SIGNAL_FUNC(func), NULL );
	    gtk_menu_append ( GTK_MENU ( menu ), item );

	    if ( !activate_currrent && 
		 gsb_gui_navigation_get_current_account () == i)
	    {
		gtk_widget_set_sensitive ( item, FALSE );
	    }      

	    gtk_widget_show ( item );
	}
	list_tmp = list_tmp -> next;
    }

    return ( menu );
}



/**
 * show a dialog to choose the type of an account
 * and return that type
 *
 * \param
 *
 * \return the type of account
 *
 * FIXME : should disappear with the wizard
 * */
gint gsb_account_ask_account_type ( void )
{
    GtkWidget *dialog;
    gint resultat;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *bouton;
    kind_account type_compte;

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_OK_CANCEL,
				       make_hint ( _("Choose account type"),
						   _("If you choose to continue, an account will be created with default payment methods chosen according to your choice.\nYou will be able to change account type later." ) ) );

    /* creation de la ligne du type de compte */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog) -> vbox ), hbox,
			 FALSE, FALSE, 6 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Account type")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 12 );
    gtk_widget_show ( label );

    bouton = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       creation_menu_type_compte() );
    gtk_box_pack_start ( GTK_BOX (hbox), bouton, TRUE, TRUE, 12 );
    gtk_widget_show ( bouton );

    resultat = gtk_dialog_run ( GTK_DIALOG(dialog) );

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( dialog );
	return ( -1 );
    }

    type_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton ) -> menu_item ),
							  "no_type_compte" ));

    gtk_widget_destroy ( dialog );

    return ( type_compte );
}




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
