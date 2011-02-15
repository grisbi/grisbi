/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	      */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_reconcile_list.h"
#include "gsb_data_account.h"
#include "gsb_data_payment.h"
#include "gsb_data_transaction.h"
#include "navigation.h"
#include "gsb_transactions_list_sort.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "transaction_list_sort.h"
#include "custom_list.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * called by a switch on the button to sort the list with the method of payment
 * while reconciling
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
    gint selected_transaction;

    transaction_list_sort_set_reconcile_sort (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)));

    /* re-sort the list */
    selected_transaction = transaction_list_select_get ();
    transaction_list_sort ();
    transaction_list_colorize ();
    transaction_list_set_balances ();
    transaction_list_select (selected_transaction);

    return FALSE;
}

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
gint gsb_reconcile_list_sort_func (CustomRecord **a,
				   CustomRecord **b,
				   CustomList *custom_list)
{
    gint account_number;
    gint return_value;
    CustomRecord *record_1 = NULL;
    CustomRecord *record_2 = NULL;

    gint transaction_number_1, transaction_number_2;
    gint position_1, position_2;
    gint payment_number_1, payment_number_2;

    /* ***** the first part is a copy of the generals tests in the transaction list sort ***** */
    if ( !*a )
        return -1;

    if ( !*b )
        return 1;

    /* i don't know why but sometimes there is a comparison between the 2 same rows... */
    if (*a == *b)
	return 0;

    /* first of all, check the archive */
    return_value = gsb_transactions_list_sort_check_archive ( *a, *b );
    if (!return_value)
    {
	/* check the general tests (white line...) */
	/* get the records */
	record_1 = *a;
	record_2 = *b;
	return_value = gsb_transactions_list_sort_general_test ( record_1, record_2 );
    }

    if (return_value)
    {
	/* we have already a returned value, but for archive or general test,
	 * the pos of the row need not to change, so must keep the return_value */
	return return_value;
    }

    account_number = gsb_gui_navigation_get_current_account ();

    /* get the transaction numbers */
    transaction_number_1 = gsb_data_transaction_get_transaction_number (record_1 -> transaction_pointer);
    transaction_number_2 = gsb_data_transaction_get_transaction_number (record_2 -> transaction_pointer);

    payment_number_1 = gsb_data_transaction_get_method_of_payment_number (transaction_number_1);
    payment_number_2 = gsb_data_transaction_get_method_of_payment_number (transaction_number_2);

    /* ***** second part : specific tests on method of payment */
    /* if we are here, all the previous test passed, so we know it's 2 different transactions
     * with 2 different method of payment, we sort according to the conf */
    account_number = gsb_gui_navigation_get_current_account ();

    /* get the position of the 1st method */
    if ( gsb_data_account_get_split_neutral_payment (account_number)
	 &&
	 gsb_data_payment_get_sign (payment_number_1) == GSB_PAYMENT_NEUTRAL
	 &&
	 gsb_data_transaction_get_amount (transaction_number_1).mantissa < 0 )
	payment_number_1 = -payment_number_1;
    position_1 = g_slist_index ( gsb_data_account_get_sort_list (account_number),
				 GINT_TO_POINTER (payment_number_1));

    /* get the position of the 2nd method */
    if ( gsb_data_account_get_split_neutral_payment (account_number)
	 &&
	 gsb_data_payment_get_sign (payment_number_2) == GSB_PAYMENT_NEUTRAL
	 &&
	 gsb_data_transaction_get_amount (transaction_number_2).mantissa < 0 )
	payment_number_2 = -payment_number_2;
    position_2 = g_slist_index ( gsb_data_account_get_sort_list (account_number),
				 GINT_TO_POINTER (payment_number_2));
    return_value = position_1 - position_2;

    if (!return_value)
    {
	/* same method of payment, return by date or no */
	const GDate *date_1;
	const GDate *date_2;

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
    }
    return return_value;
}

