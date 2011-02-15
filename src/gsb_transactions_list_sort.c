/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2008-2009 Pierre Biava (grisbi@pierre.biava.name)     */
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
 * \file gsb_transactions_list_sort.c
 * functions to sort the transactions list according to the columns
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "gsb_transactions_list_sort.h"
#include "gsb_data_account.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_budget.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "navigation.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "custom_list.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint gsb_transactions_list_sort_by_amount ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_bank ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_budget ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_category ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_chq ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_credit ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_date ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_debit ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_financial_year ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_mark ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_no ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_no_sort (  gint transaction_number_1,
                        gint transaction_number_2,
                        gint element_number );
static gint gsb_transactions_list_sort_by_notes ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_party ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_reconcile_nb ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_transaction_date_and_amount ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_transaction_date_and_no ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_type ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_value_date ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_by_voucher ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_initial_by_primary_key_and_secondary_key ( gint transaction_number_1,
                        gint transaction_number_2 );
static gint gsb_transactions_list_sort_initial_by_secondary_key ( gint transaction_number_1,
                        gint transaction_number_2 );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


/**
 * called by a click on the column, used to sort the list
 *
 * \param model
 * \param iter_1
 * \param iter_2
 * \param column_number the number of the column (0 to CUSTOM_MODEL_VISIBLE_COLUMNS) we want to sort by
 *
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort (CustomRecord **a,
                        CustomRecord **b,
                        CustomList *custom_list)
{
    gint account_number;
    gint return_value;
    CustomRecord *record_1 = NULL;
    CustomRecord *record_2 = NULL;

    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number == -1)
	/* normally cannot happen, except come here at the opening
	 * of grisbi, and must return 0 if we don't want a crash */
	return 0;

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
    else
    {
	/* get the transaction numbers */
	gint element_number;
	gint transaction_number_1 = gsb_data_transaction_get_transaction_number (record_1 -> transaction_pointer);
	gint transaction_number_2 = gsb_data_transaction_get_transaction_number (record_2 -> transaction_pointer);

	/* get the element used to sort the list */
	element_number = gsb_data_account_get_element_sort ( account_number,
							     custom_list -> sort_col);
	return_value = gsb_transactions_list_sort_by_no_sort ( transaction_number_1,
							       transaction_number_2,
							       element_number );
    }

    if (custom_list -> sort_order == GTK_SORT_DESCENDING)
	return_value = -return_value;
    return return_value;
}


/**
 * this is the first check of all : the archive
 * we put them always at the top of the list
 *
 * \param model
 * \param iter_1
 * \param iter_2
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_check_archive (  CustomRecord *record_1,
                        CustomRecord *record_2 )

{
    gint return_value = 0;

    /* most of the time we are not on an archive, so check now for transaction
     * to increase the speed */
    if (record_1 -> what_is_line != IS_ARCHIVE
	&&
	record_2 -> what_is_line != IS_ARCHIVE)
        return 0;

    /* ok, there is at least 1 archive */
    if (record_1 -> what_is_line == IS_ARCHIVE)
    {
        if (record_2 -> what_is_line == IS_ARCHIVE)
	    /* the first and second line are archives, we return a comparison by number of archive
	     * we can do better, by date or by financial year, but more complex because no check for now
	     * that the date must be different, and problem when created by report
	     * so we assume the user created the archive in the good order, if some complains about that
	     * can change here later */
            return_value = gsb_data_archive_store_get_archive_number (
                        gsb_data_archive_store_get_number ( record_1->transaction_pointer ) )-
            gsb_data_archive_store_get_archive_number (
                        gsb_data_archive_store_get_number ( record_2->transaction_pointer ) );
        else
            /* the first line is an archive and not the second, so first line before */
            return_value = -1;
    }
    else
    {
        if (record_2 -> what_is_line == IS_ARCHIVE)
            /* the first line is not an archive but the second one is, so second line before */
            return_value = 1;
        else
            /* we have 2 transactions, just return 0 here to make tests later
             * shouldn't come here */
            return 0;
    }
    return return_value;
}

/**
 * check for the part wich cannot change : the white line must always be at
 * the end of the list
 * and into a transaction, the lines are not sorted in ascending or descending method
 * 
 * \param model
 * \param iter_1
 * \param iter_2
 * 
 * \return 0 if that test cannot say the return_value between the 2 lines,
 * or the return_value if it's possible here
 * */
gint gsb_transactions_list_sort_general_test ( CustomRecord *record_1,
                        CustomRecord *record_2 )
{
    gint return_value = 0;

    /* check first for the white lines, it's always set at the end */
    if ( gsb_data_transaction_get_transaction_number (record_1 -> transaction_pointer) <= 0 )
	return_value = 1;
    else
    {
	if (gsb_data_transaction_get_transaction_number (record_2 -> transaction_pointer) <= 0)
	    return_value = -1;
    }

    /* check if we are on the same transaction */
    if ( record_1 -> transaction_pointer == record_2 -> transaction_pointer )
	/* the 2 records belong at the same transaction,
	 * we keep always the order of the lines in transaction */
	return_value = record_1->line_in_transaction - record_2->line_in_transaction;

    return return_value;
}



/**
 * find the right function to sort the list and sort the 2 iters given
 * 
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_sort permit to find the right function
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_no_sort (  gint transaction_number_1,
                        gint transaction_number_2,
                        gint element_number )
{
    gchar* tmpstr;

    switch (element_number)
    {
	case ELEMENT_DATE:  /* = 1 */
	    return ( gsb_transactions_list_sort_by_date ( transaction_number_1,
							  transaction_number_2));
	    break;
	case ELEMENT_VALUE_DATE:
	    return ( gsb_transactions_list_sort_by_value_date ( transaction_number_1,
								transaction_number_2));
	    break;
	case ELEMENT_PARTY:
	    return ( gsb_transactions_list_sort_by_party ( transaction_number_1,
							   transaction_number_2));
	    break;
	case ELEMENT_BUDGET:
	    return ( gsb_transactions_list_sort_by_budget ( transaction_number_1,
							    transaction_number_2));
	    break;
	case ELEMENT_CREDIT:
	    return ( gsb_transactions_list_sort_by_credit ( transaction_number_1,
							    transaction_number_2));
	    break;
	case ELEMENT_DEBIT:
	    return ( gsb_transactions_list_sort_by_debit ( transaction_number_1,
							   transaction_number_2));
	    break;
	case ELEMENT_BALANCE:
	    /* 	    balance, normally, shouldn't be here... in case, give back the date */
	    return ( gsb_transactions_list_sort_by_date ( transaction_number_1,
							  transaction_number_2));
	    break;
	case ELEMENT_AMOUNT:
	    return ( gsb_transactions_list_sort_by_amount ( transaction_number_1,
							    transaction_number_2));
	    break;
	case ELEMENT_PAYMENT_TYPE:
	    return ( gsb_transactions_list_sort_by_type ( transaction_number_1,
							  transaction_number_2));
	    break;
	case ELEMENT_RECONCILE_NB:
	    return ( gsb_transactions_list_sort_by_reconcile_nb ( transaction_number_1,
								  transaction_number_2));
	    break;
	case ELEMENT_EXERCICE:
	    return ( gsb_transactions_list_sort_by_financial_year ( transaction_number_1,
								    transaction_number_2));
	    break;
	case ELEMENT_CATEGORY:
	    return ( gsb_transactions_list_sort_by_category ( transaction_number_1,
							      transaction_number_2));
	    break;
	case ELEMENT_MARK:
	    return ( gsb_transactions_list_sort_by_mark ( transaction_number_1,
							  transaction_number_2));
	    break;
	case ELEMENT_VOUCHER:
	    return ( gsb_transactions_list_sort_by_voucher ( transaction_number_1,
							     transaction_number_2));
	    break;
	case ELEMENT_NOTES:
	    return ( gsb_transactions_list_sort_by_notes ( transaction_number_1,
							   transaction_number_2));
	    break;
	case ELEMENT_BANK:
	    return ( gsb_transactions_list_sort_by_bank ( transaction_number_1,
							  transaction_number_2));
	    break;
	case ELEMENT_NO:
	    return ( gsb_transactions_list_sort_by_no ( transaction_number_1,
							transaction_number_2));
	    break;
	case ELEMENT_CHQ:
	    return ( gsb_transactions_list_sort_by_chq ( transaction_number_1,
							 transaction_number_2));
	    break;
	default :
	    tmpstr = g_strdup_printf ( _("ask for the sort number %d which doesn't exist... return by date"), element_number );
	    warning_debug (tmpstr);
	    g_free(tmpstr);
	    return ( gsb_transactions_list_sort_by_date ( transaction_number_1,
							  transaction_number_2));
    }
}



/**
 * used to compare the 2 dates first, and if they are the same
 * the 2 no of transactions to find the good return for sort
 * called at the end of each sort test, if they are equal
 * 
 * \param none but the local variables transaction_number_1 and transaction_number_2 MUST be set
 * 
 * \return -1 if transaction_number_1 is above transaction_number_2
 * */
gint gsb_transactions_list_sort_by_transaction_date_and_no ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    if ( !gsb_data_transaction_get_date (transaction_number_1) )
    {
    return 1;
    }
    if ( !gsb_data_transaction_get_date (transaction_number_2) )
    {
    return -1;
    }

    /* i tried to set in the transactions a value for date as a gint as yyyymmdd
     * and here only do (gint transaction_1_date) - (gint transaction_2_date)
     * to increase the speed, but change nothing (perhaps 0,05sec for 250000 rows...)
     * so let like that */
    return_value = g_date_compare ( gsb_data_transaction_get_date (transaction_number_1),
                        gsb_data_transaction_get_date (transaction_number_2));

    /* no difference in the dates, sort by number of transaction */
    if ( !return_value )
    return_value = transaction_number_1 - transaction_number_2;

    return return_value;
}


/**
 * compared by date and by amount
 * 
 * \param none but the local variables transaction_number_1 and transaction_number_2 MUST be set
 * 
 * \return -1 if amount_2 is above amount_number_1
 * */
gint gsb_transactions_list_sort_by_transaction_date_and_amount ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    gsb_real amount_1;
    gsb_real amount_2;

    if ( !gsb_data_transaction_get_date (transaction_number_1) )
    {
        return 1;
    }
    if ( !gsb_data_transaction_get_date (transaction_number_2) )
    {
        return -1;
    }

    return_value = g_date_compare ( gsb_data_transaction_get_date (transaction_number_1),
                        gsb_data_transaction_get_date (transaction_number_2));

    if ( return_value == 0 )
    {
        /* no difference in the dates, sort by amount of transaction */
        amount_1 = gsb_data_transaction_get_amount ( transaction_number_1 );
        amount_2 = gsb_data_transaction_get_amount ( transaction_number_2 );
        return_value = amount_2.mantissa - amount_1.mantissa;
        if ( return_value == 0 )
            return_value = transaction_number_1 - transaction_number_2;
    }

    return return_value;
}


/**
 * compared by date and by party
 *
 * \param none but the local variables transaction_number_1 and transaction_number_2 MUST be set
 *
 * \return -1 if amount_2 is above amount_number_1
 * */
gint gsb_transactions_list_sort_by_transaction_date_and_party ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    gsb_real amount_1;
    gsb_real amount_2;

    if ( !gsb_data_transaction_get_date (transaction_number_1) )
    {
        return 1;
    }
    if ( !gsb_data_transaction_get_date (transaction_number_2) )
    {
        return -1;
    }

    return_value = g_date_compare ( gsb_data_transaction_get_date (transaction_number_1),
                        gsb_data_transaction_get_date (transaction_number_2));

    if ( return_value == 0 )
    {
        /* no difference in the dates, sort by amount of transaction */
        amount_1 = gsb_data_transaction_get_amount ( transaction_number_1 );
        amount_2 = gsb_data_transaction_get_amount ( transaction_number_2 );
        return_value = gsb_transactions_list_sort_by_party ( transaction_number_1,
                        transaction_number_2 );
    }

    return return_value;
}


/**
 * used to compare 2 iters and sort the by no of transaction
 * always put the white line below
 *
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 *
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_no ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    return transaction_number_1 - transaction_number_2;
}



/**
 * used to compare 2 iters and sort the by date first, and no
 * or amount transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_date ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    if ( conf.transactions_list_secondary_sorting == 1 )
        return gsb_transactions_list_sort_by_transaction_date_and_amount (
                        transaction_number_1, transaction_number_2 );
    else if ( conf.transactions_list_secondary_sorting == 2 )
        return gsb_transactions_list_sort_by_transaction_date_and_party (
                        transaction_number_1, transaction_number_2 );
    else
        return gsb_transactions_list_sort_by_transaction_date_and_no (
                        transaction_number_1, transaction_number_2 );
}


/**
 * used to compare 2 iters and sort the by value date or date if not exist
 * always put the white line below
 * 
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_value_date ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    const GDate *value_date_1;
    const GDate *value_date_2;

    /* need to work a little more here because value date is not obligatory filled,
     * if we compare 2 transactions and 1 has no value date, set the value date before */
    value_date_1 = gsb_data_transaction_get_value_date ( transaction_number_1 );
    if ( ! value_date_1 && !conf.transactions_list_primary_sorting )
        value_date_1 = gsb_data_transaction_get_date ( transaction_number_1 );

    value_date_2 = gsb_data_transaction_get_value_date ( transaction_number_2 );
    if ( ! value_date_2 && !conf.transactions_list_primary_sorting )
        value_date_2 = gsb_data_transaction_get_date ( transaction_number_2 );

    if ( value_date_1 )
    {
        if (value_date_2)
            return_value = g_date_compare ( value_date_1, value_date_2);
        else
            return_value = -1;
    }
    else
    {
        if (value_date_2)
            return_value = 1;
        else
            return gsb_transactions_list_sort_by_date (
                        transaction_number_1, transaction_number_2 );
    }

    if ( return_value )
        return return_value;
    else
        return gsb_transactions_list_sort_initial_by_secondary_key (
                        transaction_number_1, transaction_number_2 );
}


/** used to compare 2 iters and sort the by party first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_party ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    gint party_number_1;
    gint party_number_2;

    party_number_1 = gsb_data_transaction_get_party_number ( transaction_number_1 );
    party_number_2 = gsb_data_transaction_get_party_number ( transaction_number_2 );

    if (  party_number_1 == party_number_2 )
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no (
                        transaction_number_1, transaction_number_2 );
    else
    {
        const gchar *temp_1;
        const gchar *temp_2;

        temp_1 = gsb_data_payee_get_name ( party_number_1, TRUE );
        if ( temp_1 == NULL )
            return -1;

        temp_2 = gsb_data_payee_get_name ( party_number_2, TRUE );
        if ( temp_2 == NULL )
            return 1;

        /* g_utf8_collate is said not very fast, must try with big big account to check
         * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
        return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "", -1 ),
                        g_utf8_casefold ( temp_2 ? temp_2 : "", -1 ));
    }

    if ( return_value )
        return return_value;
    else
        return gsb_transactions_list_sort_by_transaction_date_and_no (
                        transaction_number_1, transaction_number_2);
}


/** used to compare 2 iters and sort the by budgetary first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_budget ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    gint budgetary_number_1;
    gint budgetary_number_2;
    gint sub_budgetary_number_1;
    gint sub_budgetary_number_2;

    budgetary_number_1 = gsb_data_transaction_get_budgetary_number ( transaction_number_1 );
    budgetary_number_2 = gsb_data_transaction_get_budgetary_number ( transaction_number_2 );
    sub_budgetary_number_1 = gsb_data_transaction_get_sub_budgetary_number ( transaction_number_1 );
    sub_budgetary_number_2 = gsb_data_transaction_get_sub_budgetary_number ( transaction_number_2 );

    if ( budgetary_number_1 == budgetary_number_2
	 &&
	 sub_budgetary_number_1 == sub_budgetary_number_2 )
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no(
                        transaction_number_1, transaction_number_2 );
    else
    {
        const gchar *temp_1;
        const gchar *temp_2;

        temp_1 = gsb_data_budget_get_name ( budgetary_number_1,
                            sub_budgetary_number_1,
                            NULL );
        if ( temp_1 == NULL )
            return -1;

        temp_2 = gsb_data_budget_get_name ( budgetary_number_2,
                            sub_budgetary_number_2,
                            NULL);
        if ( temp_2 == NULL )
            return 1;

        /* g_utf8_collate is said not very fast, must try with big big account to check
         * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
        return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "", -1 ),
                        g_utf8_casefold ( temp_2 ? temp_2 : "", -1 ) );
    }

    if ( return_value )
	    return return_value;
    else
	    return gsb_transactions_list_sort_by_transaction_date_and_no (
                        transaction_number_1, transaction_number_2);
}


/**
 * used to compare 2 iters and sort the by credit amount first, and 
 * by date and no transaction after
 * always put the white line below
 * 
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_credit ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    /* for the amounts, we have to check also the currency */
    return_value = gsb_real_cmp ( gsb_data_transaction_get_adjusted_amount ( transaction_number_1, -1),
				  gsb_data_transaction_get_adjusted_amount ( transaction_number_2, -1));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}



/** used to compare 2 iters and sort the by debit amount first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_debit ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    /* for the amounts, we have to check also the currency */
    return_value = gsb_real_cmp ( gsb_data_transaction_get_adjusted_amount ( transaction_number_2, -1),
				  gsb_data_transaction_get_adjusted_amount ( transaction_number_1, -1));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}



/** used to compare 2 iters and sort the by amount first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_amount ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    /* for the amounts, we have to check also the currency */
    return_value = gsb_real_cmp ( gsb_data_transaction_get_adjusted_amount ( transaction_number_2, -1),
				  gsb_data_transaction_get_adjusted_amount ( transaction_number_1, -1));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no (transaction_number_1, transaction_number_2);
}



/** used to compare 2 iters and sort the by type first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_type ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    /* if it's the same type, we sort by the content of the types */

    if ( gsb_data_transaction_get_method_of_payment_number ( transaction_number_1) == gsb_data_transaction_get_method_of_payment_number ( transaction_number_2))
    {
	return_value = g_utf8_collate ( g_utf8_casefold ( gsb_data_transaction_get_method_of_payment_content ( transaction_number_1) ? gsb_data_transaction_get_method_of_payment_content ( transaction_number_1) : "",
							  -1 ),
					g_utf8_casefold ( gsb_data_transaction_get_method_of_payment_content ( transaction_number_2) ? gsb_data_transaction_get_method_of_payment_content ( transaction_number_2) : "",
							  -1 ));

	if ( !return_value )
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
    }
    else
    {
	const gchar *temp_1;
	const gchar *temp_2;

	temp_1 = gsb_data_payment_get_name (gsb_data_transaction_get_method_of_payment_number (transaction_number_1));
	temp_2 = gsb_data_payment_get_name (gsb_data_transaction_get_method_of_payment_number (transaction_number_2));

	/* g_utf8_collate is said not very fast, must try with big big account to check
	 * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
	return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
							  -1 ),
					g_utf8_casefold ( temp_2 ? temp_2 : "",
							  -1 ));
    }

    if ( return_value )
	return return_value;
    else
    {
	/* 	it seems that the 2 types are different no but same spell... */

	return_value = g_utf8_collate ( g_utf8_casefold ( gsb_data_transaction_get_method_of_payment_content ( transaction_number_1)? gsb_data_transaction_get_method_of_payment_content ( transaction_number_1): "",
							  -1 ),
					g_utf8_casefold ( gsb_data_transaction_get_method_of_payment_content ( transaction_number_2)? gsb_data_transaction_get_method_of_payment_content ( transaction_number_2): "",
							  -1 ));

	if ( !return_value )
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
    }
    return return_value;
}



/** used to compare 2 iters and sort the by reconcile number first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_reconcile_nb ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    if ( gsb_data_transaction_get_reconcile_number ( transaction_number_1)== gsb_data_transaction_get_reconcile_number ( transaction_number_2))
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
    else
    {
	const gchar *temp_1;
	const gchar *temp_2;

	temp_1 = gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( transaction_number_1));
	temp_2 = gsb_data_reconcile_get_name ( gsb_data_transaction_get_reconcile_number ( transaction_number_2));

	/* g_utf8_collate is said not very fast, must try with big big account to check
	 * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
	return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
							  -1 ),
					g_utf8_casefold ( temp_2 ? temp_2 : "",
							  -1 ));
    }

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}



/** used to compare 2 iters and sort the by financial_year first, and no
 * transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_financial_year ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    if ( gsb_data_transaction_get_financial_year_number ( transaction_number_1)== gsb_data_transaction_get_financial_year_number ( transaction_number_2))
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
    else
    {
	GDate *date_1;
	GDate *date_2;

	date_1 = gsb_data_fyear_get_beginning_date (gsb_data_transaction_get_financial_year_number ( transaction_number_1));
	date_2 = gsb_data_fyear_get_beginning_date (gsb_data_transaction_get_financial_year_number ( transaction_number_2));

	if ( date_1 )
	{
	    if ( date_2 )
		return_value = g_date_compare ( date_1,
						date_2 );
	    else
		return_value = 1;
	}
	else
	{
	    if ( date_2 )
		return_value = -1;
	    else
		return_value = 0;
	}
    }

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);

}



/** used to compare 2 iters and sort the by category first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_category ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    gchar *temp_1;
    gchar *temp_2;

    /** we want to take the name of the categ, so, either
     * split of transaction
     * transfer : ...
     * categ : under-categ
     * and after, we sort by str
     * */

    temp_1 = gsb_transactions_get_category_real_name ( transaction_number_1);
    temp_2 = gsb_transactions_get_category_real_name ( transaction_number_2);

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    g_free (temp_1);
    g_free (temp_2);

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}


/** used to compare 2 iters and sort the by mark first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_mark ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;

    return_value = gsb_data_transaction_get_marked_transaction (
                        transaction_number_1)-
                        gsb_data_transaction_get_marked_transaction (
                        transaction_number_2);


    if ( return_value )
	    return return_value;
    else
	return - gsb_transactions_list_sort_by_transaction_date_and_no (
                        transaction_number_1, transaction_number_2 );
}



/** used to compare 2 iters and sort the by voucher first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_voucher ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;

    temp_1 = gsb_data_transaction_get_voucher ( transaction_number_1);
    temp_2 = gsb_data_transaction_get_voucher ( transaction_number_2);

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}


/** used to compare 2 iters and sort the by notes first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_notes ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;

    temp_1 = gsb_data_transaction_get_notes ( transaction_number_1);
    temp_2 = gsb_data_transaction_get_notes ( transaction_number_2);

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}



/** used to compare 2 iters and sort the by bank first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_bank ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;

    temp_1 = gsb_data_transaction_get_bank_references ( transaction_number_1);
    temp_2 = gsb_data_transaction_get_bank_references ( transaction_number_2);

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}



/** used to compare 2 iters and sort the by cheque or no of transfer first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_chq ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;

    temp_1 = gsb_data_transaction_get_method_of_payment_content ( transaction_number_1);
    temp_2 = gsb_data_transaction_get_method_of_payment_content ( transaction_number_2);

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(transaction_number_1, transaction_number_2);
}


/**
 * Called to sort transactions by key primary and secondary key
 *
 * \param model
 * \param iter_1
 * \param iter_2
 *
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_initial (CustomRecord **a,
                        CustomRecord **b,
                        CustomList *custom_list)
{
    gint account_number;
    gint return_value;
    CustomRecord *record_1 = NULL;
    CustomRecord *record_2 = NULL;

    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number == -1)
    {
        /* normally cannot happen, except come here at the opening
         * of grisbi, and must return 0 if we don't want a crash */
        return 0;
    }   

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
    else
    {
        /* get the transaction numbers */
        gint transaction_number_1;
        gint transaction_number_2;

        transaction_number_1 = gsb_data_transaction_get_transaction_number (record_1 -> transaction_pointer);
        transaction_number_2 = gsb_data_transaction_get_transaction_number (record_2 -> transaction_pointer);

        return_value = gsb_transactions_list_sort_initial_by_primary_key_and_secondary_key (
                        transaction_number_1, transaction_number_2 );
    }

    return return_value;
}


/**
 * used to compare 2 iters and sort the by primary key
 *
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_initial_by_primary_key_and_secondary_key ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    gint return_value;
    const GDate *value_date_1;
    const GDate *value_date_2;

    /* need to work a little more here because value date is not obligatory filled,
     * if we compare 2 transactions and 1 has no value date, set the value date before */
    value_date_1 = gsb_data_transaction_get_value_date ( transaction_number_1 );
    if ( !value_date_1 )
        value_date_1 = gsb_data_transaction_get_date ( transaction_number_1 );

    value_date_2 = gsb_data_transaction_get_value_date ( transaction_number_2 );
    if ( !value_date_2 )
        value_date_2 = gsb_data_transaction_get_date ( transaction_number_2 );

    if ( value_date_1 )
    {
        if (value_date_2)
            return_value = g_date_compare ( value_date_1, value_date_2);
        else
            return_value = -1;
    }
    else
    {
        if ( value_date_2 )
            return_value = 1;
        else
            return_value = 0;
    }

    if ( return_value )
        return return_value;
    else
        return gsb_transactions_list_sort_initial_by_secondary_key (
                        transaction_number_1, transaction_number_2 );
}


/**
 * used to compare 2 iters and sort the by secondary key:
 * no or amount or payee_name
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_initial_by_secondary_key ( gint transaction_number_1,
                        gint transaction_number_2 )
{
    if ( conf.transactions_list_secondary_sorting == 1 )
        return gsb_transactions_list_sort_by_amount (
                        transaction_number_1, transaction_number_2 );
    else if ( conf.transactions_list_secondary_sorting == 2 )
        return gsb_transactions_list_sort_by_party (
                        transaction_number_1, transaction_number_2 );
    else
        return transaction_number_1 - transaction_number_2;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
