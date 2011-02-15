/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	          */
/*			2006-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*			http://www.grisbi.org				                              */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_account.h"
#include "dialog.h"
#include "gsb_account_property.h"
#include "gsb_category.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_payment.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_form_scheduler.h"
#include "navigation.h"
#include "fenetre_principale.h"
#include "menu.h"
#include "gsb_real.h"
#include "gsb_scheduler_list.h"
#include "main.h"
#include "traitement_variables.h"
#include "tiers_onglet.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "transaction_list.h"
#include "gsb_transactions_list.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/





/*START_EXTERN*/
extern GtkWidget *account_page;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget *notebook_general;
/*END_EXTERN*/



/**
 * called to create a new account
 * called by the account assistant, should not be used directly
 * !! here is just to add an account, for new complete file see gsb_file_new
 * 
 * \param none
 * 
 * \return FALSE if problem, TRUE if ok
 */

gboolean gsb_account_new ( kind_account account_type,
			   gint currency_number,
			   gint bank_number,
			   gsb_real init_amount,
			   const gchar *name,
               gchar *name_icon )
{
    gint account_number;

    /*     create the new account */ 
    account_number = gsb_data_account_new (account_type);
    if ( account_number == -1 )
    {
	dialogue_error_memory ();
	return FALSE;
    }

    /* set the default method of payment */
    gsb_data_payment_create_default (account_number);

    /* set the icon_name */
    if ( name_icon )
        gsb_data_account_set_name_icon ( account_number, name_icon );

    gsb_data_account_set_currency ( account_number, currency_number);
    gsb_data_account_set_bank (account_number, bank_number);
    gsb_data_account_set_init_balance (account_number, init_amount);
    gsb_data_account_set_mini_balance_wanted ( account_number, 
					       gsb_real_new ( 0, 
							      gsb_data_currency_get_floating_point (currency_number) ) );
    gsb_data_account_set_mini_balance_authorized (account_number, 
						  gsb_real_new ( 0, 
								 gsb_data_currency_get_floating_point (currency_number) ) );
    gsb_data_account_set_name (account_number, name);

    /* update the combofix for categ */ 
    gsb_category_update_combofix ( FALSE );

    /* update the name of accounts in form */
    gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
				    FALSE );

    /* update the main page */ 
    mise_a_jour_liste_comptes_accueil = 1;

    /* update the accounts lists */ 
    gsb_menu_update_accounts_in_menus (); 

    /* do the next part only if the widgets are created 
     * (can come here at the end of the new file assistant...) */
    if (notebook_general)
    {
        /* Add an entry in navigation pane. */
        gsb_gui_navigation_add_account ( account_number, TRUE );

        /* Go to accounts properties */
        gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), GSB_ACCOUNT_PAGE );
        gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), GSB_PROPERTIES_PAGE );

        gsb_account_property_fill_page ();
    }

    if ( etat.modification_fichier == 0 )
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
	gchar* tmpstr;

    deleted_account = gsb_gui_navigation_get_current_account ();

    tmpstr = g_strdup_printf (_("Delete account \"%s\"?"),
				     gsb_data_account_get_name ( deleted_account ) ) ;

    if ( !question_yes_no_hint ( tmpstr,
				        _("This will irreversibly remove this account and all operations "
                        "that were previously contained.  There is no undo for this. "
                        "Usually it's a better way to close an account."),
				        GTK_RESPONSE_NO ))
    {
        g_free ( tmpstr );
	    return FALSE;
    }
    g_free ( tmpstr );

    /* if the last account, close the file */
    if ( gsb_data_account_get_accounts_amount () == 1 )
    {
        modification_fichier ( FALSE );
        gsb_file_close ();
        return FALSE;
    }

    /* delete the schedules transactions on that account */
    list_tmp = gsb_data_scheduled_get_scheduled_list ();
    while (list_tmp)
    {
        gint scheduled_number;

        scheduled_number = gsb_data_scheduled_get_scheduled_number ( list_tmp -> data );

        if ( gsb_data_scheduled_get_account_number (scheduled_number) == deleted_account )
            gsb_data_scheduled_remove_scheduled (scheduled_number);

        list_tmp = list_tmp -> next;
    }


    /* remove all the transactions of that account */
    list_tmp = gsb_data_transaction_get_complete_transactions_list ();
    while (list_tmp)
    {
        gint transaction_number;

        transaction_number = gsb_data_transaction_get_transaction_number ( list_tmp -> data );

        /* better to go to the next transaction now */
        list_tmp = list_tmp -> next;

        if (gsb_data_transaction_get_account_number (transaction_number) == deleted_account)
        {
            gint contra_transaction_number;

            /* we are on a transaction on the deleted account, we delete that transaction,
             * but if it's a transfer, modify the contra-transaction to set transfer to deleted account */
            contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (
                                        transaction_number);
            if (contra_transaction_number > 0)
            /* it's a transfer, modify the contra-transaction */
                gsb_data_transaction_set_contra_transaction_number ( contra_transaction_number, -1);

            /* now can remove the transaction */
            gsb_data_transaction_remove_transaction_without_check ( transaction_number );
        }
    }

    /* delete the payment_number */
    list_tmp = gsb_data_account_get_sort_list ( deleted_account );
    while (list_tmp)
    {
        gpointer ptr;
        gint payment_number;

        ptr = list_tmp -> data;
        payment_number = GPOINTER_TO_INT ( ptr );
        gsb_data_payment_remove ( payment_number );

        list_tmp = list_tmp -> next;
    }

    /* delete the account */
    gsb_data_account_delete ( deleted_account );

    /* check gsb_gui_navigation_get_current_account () and gsb_gui_navigation_get_current_account ()_onglet and put them
     * on the first account if they are on the deleted account */

    if ( gsb_gui_navigation_get_current_account () == deleted_account )
    {
	/* update the transaction list */

	page_number = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook_general));

	navigation_change_account ( GINT_TO_POINTER ( gsb_data_account_first_number () ));

	gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), page_number );
    }

    /* update the buttons lists */
    gsb_menu_update_accounts_in_menus();

    /* Replace trees contents. */
    remplit_arbre_categ ();
    remplit_arbre_imputation ();
    payee_fill_tree ();

    /* update the categories in lists */
    transaction_list_update_element (ELEMENT_CATEGORY);

    /* update the name of accounts in form */
    gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
				    FALSE );

    gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    /* Update navigation pane. */
    gsb_gui_navigation_remove_account ( deleted_account );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
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

    /* by default, this is blank, so set the first */
    gtk_combo_box_set_active ( GTK_COMBO_BOX (combobox),
			       0 );

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
 * the the account on the combo box
 *
 * \param combo_box the accounts combo-box
 * \param account_number the account we want to set
 *
 * \return TRUE ok, FALSE pb
 * */
gboolean gsb_account_set_combo_account_number ( GtkWidget *combo_box,
						gint account_number )

{
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (!combo_box
	||
	!GTK_IS_COMBO_BOX (combo_box))
	return FALSE;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));

    if (!gtk_tree_model_get_iter_first (model, &iter))
	return FALSE;

    do
    {
	gint account_test;

	gtk_tree_model_get ( model,
			     &iter,
			     1, &account_test,
			     -1 );
	if (account_number == account_test)
	{
	    gtk_combo_box_set_active_iter ( GTK_COMBO_BOX (combo_box),
					    &iter );
	    return TRUE;
	}
    }
    while (gtk_tree_model_iter_next (model, &iter));
    
    return FALSE;
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
	    g_object_set_data ( G_OBJECT ( item ),
				  "account_number",
				  GINT_TO_POINTER (i));
	    if ( func )
		g_signal_connect ( G_OBJECT ( item ), "activate", G_CALLBACK(func), NULL );
	    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );

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





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
