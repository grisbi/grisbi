/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_reconcile_list.c
 * functions about the transactions list while reconciling
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_reconcile_list.h"
#include "erreur.h"
#include "gsb_data_account.h"
#include "gsb_data_payment.h"
#include "gsb_data_transaction.h"
#include "navigation.h"
#include "gsb_transactions_list.h"
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint gsb_reconcile_list_sort_func ( GtkTreeModel *model,
				    GtkTreeIter *iter_1,
				    GtkTreeIter *iter_2,
				    gpointer null );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** when switch to the method of payment sort while reconciling,
 * that 2 variables save the previous sort column */
static gint saved_sort_column_id;
static GtkSortType saved_order;


/**
 * called by a switch on the button to sort the list with the method of payment
 * this function switches between the 2 modes
 *
 * \param button the check_button
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_list_button_clicked ( GtkWidget *button,
					     gpointer null )
{
    GtkTreeSortable *sortable;
    gint account_number;

    sortable = GTK_TREE_SORTABLE (gsb_transactions_list_get_sortable ());

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
    {
	/* the button is pressed, so sort by the method of payment */
	/* save the current column */
	gtk_tree_sortable_get_sort_column_id ( sortable,
					       &saved_sort_column_id,
					       &saved_order );

	/* set the new sort function */
	gtk_tree_sortable_set_default_sort_func ( sortable,
						  (GtkTreeIterCompareFunc) gsb_reconcile_list_sort_func,
						  NULL,
						  NULL );
	gtk_tree_sortable_set_sort_column_id ( sortable,
					       GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					       GTK_SORT_ASCENDING );
    }
    else
    {
	/* the button is not active, sort by previous column */
	gtk_tree_sortable_set_sort_column_id ( sortable,
					       saved_sort_column_id,
					       saved_order );
	/* unset the default func */
	gtk_tree_sortable_set_default_sort_func ( sortable,
						  NULL, NULL, NULL );
    }

    account_number = gsb_gui_navigation_get_current_account ();
    gsb_transactions_list_set_background_color (account_number);
    gsb_transactions_list_set_transactions_balances (account_number);

    return FALSE;
}

/* xxx une fois le payment method fini, revenir jouer avec le reconcile et reconcile config */
/**
 * function used to sort the list according to the user conf, with the method of payment
 *
 * \param model
 * \param iter_1
 * \param iter_2
 * \param null not used
 *
 * \return -1 if iter_1 before iter_2 ...
 * */
gint gsb_reconcile_list_sort_func ( GtkTreeModel *model,
				    GtkTreeIter *iter_1,
				    GtkTreeIter *iter_2,
				    gpointer null )
{
    gint return_value = 0;
    gpointer transaction_1, transaction_2;
    gint transaction_number_1, transaction_number_2;
    gint line_1, line_2;
    gint position_1, position_2;
    gint method_number_1, method_number_2;
    gint account_number;

    /* ***** the first part is a fast copy of the generals tests in the transaction list sort ***** */
    /* get the transaction and line first */
    gtk_tree_model_get ( model,
			 iter_1,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_1,
			 TRANSACTION_COL_NB_TRANSACTION_LINE, &line_1,
			 -1 );
    gtk_tree_model_get ( model,
			 iter_2,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_2,
			 TRANSACTION_COL_NB_TRANSACTION_LINE, &line_2,
			 -1 );
    if ( !transaction_1
	 ||
	 !transaction_2 )
    {
	debug_message ( _( "Local variable value NULL" ),
			_( "in the function gsb_reconcile_list_sort_func, transaction_1 or transaction_2 is NULL ; it souldn't happen, it's seems that the function is called by a bad way" ),
			DEBUG_LEVEL_ALERT,
			FALSE );
	return 0;
    }

    transaction_number_1 = gsb_data_transaction_get_transaction_number (transaction_1);
    transaction_number_2 = gsb_data_transaction_get_transaction_number (transaction_2);

     /* check first for the white lines, it's always set at the end */
    if ( transaction_number_1 <= 0 )
	return 1;
    else
    {
	if (transaction_number_2 <= 0)
	    return -1;
    }

    /* check the line in a transaction */
    if ( transaction_number_1 == transaction_number_2 )
	return (line_1 - line_2);

    /* check if same method of payment */
    if ( gsb_data_transaction_get_method_of_payment_number (transaction_number_1)
	 ==
	 gsb_data_transaction_get_method_of_payment_number (transaction_number_2))
    {
	/* same method of payment, return by date or no */
	GDate *date_1;
	GDate *date_2;

	date_1 = gsb_data_transaction_get_date (transaction_number_1);
	date_2 = gsb_data_transaction_get_date (transaction_number_2);

	/* should not happen */
	if ( !date_1 )
	    return 1;
	if ( !date_2 )
	    return -1;

	return_value = g_date_compare ( date_1,
					date_2);

	/* no difference in the dates, sort by number of transaction */
	if ( !return_value )
	    return_value = transaction_number_1 - transaction_number_2;

	return return_value;
    }

    /* ***** second part : specific tests on method of payment */
    /* if we are here, all the previous test passed, so we know it's 2 different transactions
     * with 2 different method of payment, we sort according to the conf */

    account_number = gsb_gui_navigation_get_current_account ();

    /* get the position of the 1st method */
    method_number_1 = gsb_data_transaction_get_method_of_payment_number (transaction_number_1);
    if ( gsb_data_account_get_split_neutral_payment (account_number)
	 &&
	 !gsb_data_payment_get_sign (method_number_1)
	 &&
	 gsb_data_transaction_get_amount (transaction_number_1).mantissa < 0 )
	method_number_1 = -method_number_1;
    position_1 = g_slist_index ( gsb_data_account_get_sort_list (account_number),
				 GINT_TO_POINTER (method_number_1));

    /* get the position of the 2nd method */
    method_number_2 = gsb_data_transaction_get_method_of_payment_number (transaction_number_2);
    if ( gsb_data_account_get_split_neutral_payment (account_number)
	 &&
	 gsb_data_payment_get_sign (method_number_2)
	 &&
	 gsb_data_transaction_get_amount (transaction_number_2).mantissa < 0 )
	method_number_2 = -method_number_2;
    position_2 = g_slist_index ( gsb_data_account_get_sort_list (account_number),
				 GINT_TO_POINTER (method_number_2));

    return (position_1 - position_2);
}

