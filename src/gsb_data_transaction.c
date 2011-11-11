/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file gsb_transaction_data.c
 * work with the transaction structure, no GUI here
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <errno.h>

/*START_INCLUDE*/
#include "gsb_data_transaction.h"
#include "bet_data.h"
#include "classement_echeances.h"
#include "dialog.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "utils_dates.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a transaction
 */
typedef struct
{
    /** @name general stuff */
    gint transaction_number;
    gchar *transaction_id;              /**< filled by ofx */
    gint account_number;
    gsb_real transaction_amount;
    gint party_number;
    gchar *notes;
    gint marked_transaction;            /**<  OPERATION_NORMALE=nothing, OPERATION_POINTEE=P, OPERATION_TELERAPPROCHEE=T, OPERATION_RAPPROCHEE=R */
    gint archive_number;                /**< if it's an archived transaction, contains the number of the archive */
    gshort automatic_transaction;       /**< 0=manual, 1=automatic (scheduled transaction) */
    gint reconcile_number;              /**< the number of reconcile, carreful : can be filled without marked_transaction=OPERATION_RAPPROCHEE sometimes,
                                             it happen if the user did ctrl R to un-R the transaction, we keep reconcile_number because most of them
                                             will re-R after the change, and that value will help the user to find wich statement it belong.
                                             o always check marked_transaction before checking reconcile_number here */
    guint financial_year_number;
    gchar *voucher;
    gchar *bank_references;

    /** @name dates of the transaction */
    GDate *date;
    GDate *value_date;

    /** @name currency stuff */
    gint currency_number;
    gint change_between_account_and_transaction;    /**< if 1 : 1 account_currency = (exchange_rate * amount) transaction_currency */
    gsb_real exchange_rate;
    gsb_real exchange_fees;

    /** @name category stuff */
    gint category_number;
    gint sub_category_number;
    gint budgetary_number;
    gint sub_budgetary_number;
    gint transaction_number_transfer;   /**< -1 for a transfer to a deleted account, the contra-transaction number else */
    gint split_of_transaction;          /**< 1 if it's a split of transaction */
    gint mother_transaction_number;     /**< for a split, the mother's transaction number */

    /** @name method of payment */
    gint method_of_payment_number;
    gchar *method_of_payment_content;
} struct_transaction;


/*START_STATIC*/
static void gsb_data_transaction_delete_all_transactions ( void );
static void gsb_data_transaction_free ( struct_transaction *transaction);
static gint gsb_data_transaction_get_last_white_number (void);
static struct_transaction *gsb_data_transaction_get_transaction_by_no ( gint transaction_number );
static gboolean gsb_data_transaction_save_transaction_pointer ( gpointer transaction );
/*END_STATIC*/

/*START_EXTERN*/
extern gsb_real null_real;
/*END_EXTERN*/


/** the g_slist which contains the transactions structures not archived */
static GSList *transactions_list = NULL;

/** that list contains all the transactions, archived and not archived */
static GSList *complete_transactions_list = NULL;

/** the g_slist which contains all the white transactions structures
 * ie : 1 general white line
 * and 1 white line per split of transaction */
static GSList *white_transactions_list = NULL;

/** 2 pointers to the 2 last transaction used (to increase the speed) */
static struct_transaction *transaction_buffer[2];

/** set the current buffer used */
static gint current_transaction_buffer;


/**
 * set the transactions global variables to NULL, usually when we init all the global variables
 * 
 * \param
 * 
 * \return FALSE
 * */
gboolean gsb_data_transaction_init_variables ( void )
{
    gsb_data_transaction_delete_all_transactions ();
    return FALSE;
}


/** 
 * return a pointer to the g_slist of transactions structure
 * it's not a copy, so we must not free or change it
 * if we want to change something, use gsb_data_transaction_copy_transactions_list instead
 * THIS IS THE LIST WITHOUT THE ARCHIVED TRANSACTIONS
 *
 * \param none
 *
 * \return the slist of transactions structures
 * */
GSList *gsb_data_transaction_get_transactions_list ( void )
{
    return transactions_list;
}

/** 
 * return a pointer to the complete g_slist of transactions structure
 * it's not a copy, so we must not free or change it
 * if we want to change something, use gsb_data_transaction_copy_transactions_list instead
 * THIS IS THE COMPLETE LIST (WITH THE ARCHIVED TRANSACTIONS)
 *
 * \param none
 *
 * \return the slist of transactions structures
 * */
GSList *gsb_data_transaction_get_complete_transactions_list ( void )
{
    return complete_transactions_list;
}

/**
 * just append the archived transaction given in param
 * into the non archived transactions list
 * called when want to load archived transactions into grisbi
 *
 * \param transaction_number the transaction to append
 *
 * \return TRUE ok, FALSE pb
 * */
gboolean gsb_data_transaction_add_archived_to_list ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transactions_list = g_slist_append ( transactions_list,
					 transaction );
    return TRUE;
}


/**
 * transitionnal fonction, give back the pointer to the transaction,
 * normally deleted when the transfer from old transactions is finished 
 * set also in the transactions list, check if it's fast enough with only the transactions number ?
 * 
 * \param transaction_number
 *
 * \return a pointer to the structure of the transation
 * */
gpointer gsb_data_transaction_get_pointer_of_transaction ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    return transaction;
}

/**
 * find the last number of transaction
 * 
 * \param none
 * 
 * \return the number
 * */
gint gsb_data_transaction_get_last_number (void)
{
    gint last_number = 0;
    GSList *transactions_list_tmp;

    transactions_list_tmp = complete_transactions_list;

    while (transactions_list_tmp)
    {
	struct_transaction *transaction;

	transaction = transactions_list_tmp -> data;
	if ( transaction -> transaction_number > last_number )
	    last_number = transaction -> transaction_number;

	transactions_list_tmp = transactions_list_tmp -> next;
    }
    return last_number;
}



/** find the last number of the white lines
 * all the white lines have a number < 0, and it always exists at least
 * one line, which number -1 which is the general white line (without mother)
 * so we never return 0 to avoid -1 for a number of white split
 * 
 * \param none
 *
 * \return the number
 * */
gint gsb_data_transaction_get_last_white_number (void)
{
    gint last_number = 0;
    GSList *transactions_list_tmp;

    transactions_list_tmp = white_transactions_list;

    while (transactions_list_tmp)
    {
	struct_transaction *transaction;

	transaction = transactions_list_tmp -> data;
	if ( transaction -> transaction_number < last_number )
	    last_number = transaction -> transaction_number;

	transactions_list_tmp = transactions_list_tmp -> next;
    }
    
    /* if the general white line has not been appened already, we
     * return -1 to keep that number for the general white line
     * (the number will be decreased after to numbered the new line) */

    if ( !last_number )
	last_number = -1;

    return last_number;
}

/** get the number of the transaction and save the pointer in the buffer
 * which will increase the speed later
 * \param transaction a pointer to a transaction
 * \return the number of the transaction
 * */
gint gsb_data_transaction_get_transaction_number ( gpointer transaction_pointer )
{
    struct_transaction *transaction;

    transaction = transaction_pointer;

    if ( !transaction )
	return 0;

    /* if we want the transaction number, usually it's to make other stuff after that
     * so we will save the adr of the transaction to increase the speed after */

    gsb_data_transaction_save_transaction_pointer ( transaction );
    return transaction -> transaction_number;
}


/**
 * save the pointer in a buffer to increase the speed later
 * 
 * \param transaction the pointer to the transaction
 * 
 * \return TRUE or FALSE if pb
 * */
gboolean gsb_data_transaction_save_transaction_pointer ( gpointer transaction )
{
    /* check if the transaction isn't already saved */

    if ( transaction == transaction_buffer[0]
	 ||
	 transaction == transaction_buffer[1] )
	return TRUE;

    current_transaction_buffer = !current_transaction_buffer;
    transaction_buffer[current_transaction_buffer] = transaction;
    return TRUE;
}


/**
 * return the transaction which the number is in the parameter. 
 * the new transaction is stored in the buffer
 * 
 * \param transaction_number
 * 
 * \return a pointer to the transaction, NULL if not found
 * */
struct_transaction *gsb_data_transaction_get_transaction_by_no ( gint transaction_number )
{
    GSList *transactions_list_tmp;

    if (!transaction_number)
	return NULL;

    /* check first if the transaction is in the buffer */
    if ( transaction_buffer[0]
	 &&
	 transaction_buffer[0] -> transaction_number == transaction_number )
	return transaction_buffer[0];

    if ( transaction_buffer[1]
	 &&
	 transaction_buffer[1] -> transaction_number == transaction_number )
	return transaction_buffer[1];

    if ( transaction_number < 0 )
	transactions_list_tmp = white_transactions_list;
    else
	transactions_list_tmp = complete_transactions_list;

    while ( transactions_list_tmp )
    {
	struct_transaction *transaction;

	transaction = transactions_list_tmp -> data;

	if ( transaction -> transaction_number == transaction_number )
	{
	    gsb_data_transaction_save_transaction_pointer ( transaction );
	    return transaction;
	}

	transactions_list_tmp = transactions_list_tmp -> next;
    }

    /* here, we didn't find any transaction with that number */
    return NULL;
}


/** get the transaction_id
 * \param transaction_number the number of the transaction
 * \param no_account the number of account, may be -1, in that case we will look for the transaction in all accounts
 * \return the id of the transaction
 * */
const gchar *gsb_data_transaction_get_transaction_id ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return NULL;

    if ( transaction -> transaction_id )
	return transaction -> transaction_id;
    else
	return "";
}


/** set the transaction_id 
 * \param transaction_number
 * \param no_account
 * \param transaction_id a gchar with the new transaction_id
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_transaction_id ( gint transaction_number,
                        const gchar *transaction_id )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    if ( transaction -> transaction_id )
        g_free ( transaction -> transaction_id );

    if (transaction_id)
	transaction -> transaction_id = my_strdup (transaction_id);
    else
    	transaction -> transaction_id = NULL;
    return TRUE;
}


/**
 * get the account_number
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the account of the transaction or -1 if problem
 * */
gint gsb_data_transaction_get_account_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> account_number;
}


/**
 * set the account_number of the transaction
 * if the transaction has some children, they change too
 *
 * \param transaction_number
 * \param no_account
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_account_number ( gint transaction_number,
                        gint no_account )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    gsb_data_account_set_balances_are_dirty ( transaction -> account_number );    
    transaction -> account_number = no_account;
    gsb_data_account_set_balances_are_dirty ( no_account );

    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> account_number = no_account;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}



/**
 * get the GDate of the transaction 
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the GDate of the transaction
 * */
const GDate *gsb_data_transaction_get_date ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
    return NULL;

    return transaction -> date;
}


/**
 * set the GDate of the transaction
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param no_account
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_date ( gint transaction_number,
                        const GDate *date )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    if (transaction -> date)
	g_date_free (transaction -> date);

    transaction -> date = gsb_date_copy (date);

    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;

	    if (transaction -> date)
		g_date_free (transaction -> date);

	    transaction -> date = gsb_date_copy (date);

	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }
    return TRUE;
}




/** get the value GDate of the transaction 
 * \param transaction_number the number of the transaction
 * \return the GDate of the transaction
 * */
const GDate *gsb_data_transaction_get_value_date ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return NULL;

    return transaction -> value_date;
}


/** 
 * set the value GDate of the transaction
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param date the value date
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_value_date ( gint transaction_number,
                        const GDate *date )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    if (transaction ->  value_date)
	g_date_free (transaction ->  value_date);

    transaction ->  value_date = gsb_date_copy (date);

    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;

	    if (transaction ->  value_date)
		g_date_free (transaction -> value_date);

	    transaction -> value_date = gsb_date_copy (date);

	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }
    
    return TRUE;
}


/**
 * get the value GDate of the transaction or the date if not exist
 *
 * \param transaction_number the number of the transaction
 *
 * \return the GDate of the transaction
 * */
const GDate *gsb_data_transaction_get_value_date_or_date ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return NULL;

    if ( transaction -> value_date && g_date_valid ( transaction -> value_date ) )
        return transaction -> value_date;
    else
        return transaction -> date;
}


/**
 * get the amount of the transaction without any currency change
 * (so just get the given amout)
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the amount of the transaction
 * */
gsb_real gsb_data_transaction_get_amount ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no (transaction_number);

    if ( !transaction )
	return null_real;

    return transaction -> transaction_amount;
}


/** set the amount of the transaction
 * 
 * \param transaction_number
 * \param amount
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_amount ( gint transaction_number,
                        gsb_real amount )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
        return FALSE;

    transaction -> transaction_amount = amount;
    gsb_data_account_set_balances_are_dirty ( transaction -> account_number );
    
    return TRUE;
}





/**
 * get the amount of the transaction, modified to be ok with the currency
 * of the account
 * !! we should never use -1 for the return_exponent because the numbers become to deep and
 * 	the functions gsb_real_add and others can return an overflow
 * 	try to pass a return_exponent by gsb_data_currency_get_floating_point before
 * 
 * \param transaction_number the number of the transaction
 * \param return_exponent the exponent we want to have for the returned number, or -1 for the exponent of the returned currency
 * 
 * \return the amount of the transaction
 * */
gsb_real gsb_data_transaction_get_adjusted_amount ( gint transaction_number,
                        gint return_exponent )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return null_real;

    return gsb_data_transaction_get_adjusted_amount_for_currency ( transaction_number,
								   gsb_data_account_get_currency ( transaction -> account_number ),
								   return_exponent );
}




/**
 * get the amount of the transaction, modified to be ok with the currency
 * given in param 
 * 
 * \param transaction_number 		the number of the transaction
 * \param return_currency_number 	the currency we want to adjust the transaction's amount
 * \param return_exponent 		the exponent we want to have for the returned number, or -1 for the exponent of the returned currency
 * 
 * \return the amount of the transaction
 * */
gsb_real gsb_data_transaction_get_adjusted_amount_for_currency ( gint transaction_number,
                        gint return_currency_number,
                        gint return_exponent )
{
    struct_transaction *transaction;
    gsb_real amount = null_real;
    gint link_number;

    if (return_exponent == -1)
	return_exponent = gsb_data_currency_get_floating_point (return_currency_number);

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( ! (transaction && return_currency_number ) )
        return gsb_real_adjust_exponent  ( null_real, return_exponent );

    /* if the transaction currency is the same of the account's one,
     * we just return the transaction's amount */
    if ( transaction -> currency_number == return_currency_number )
        return gsb_real_adjust_exponent  ( transaction -> transaction_amount,
					   return_exponent );
/* printf ("get_adjusted_amount transaction_number = %d transaction -> currency_number = %d return_currency_number = %d\n",
    transaction_number, transaction -> currency_number, return_currency_number); */
    /* now we can adjust the amount */
	/* the exchange is saved in the transaction itself */
    if ( transaction -> exchange_rate.mantissa )
    {
        if ( transaction -> change_between_account_and_transaction )
            amount = gsb_real_div ( transaction -> transaction_amount,
                        transaction -> exchange_rate );
        else
            amount = gsb_real_mul ( transaction -> transaction_amount,
                        transaction -> exchange_rate );

        /* The costs are still deducted from the transaction. In case of internal transfer there is no charge. */
        amount = gsb_real_sub (amount, transaction -> exchange_fees);
    }
    else if ( (link_number = gsb_data_currency_link_search ( transaction -> currency_number,
							return_currency_number ) ) )
    {
	/* there is a hard link between the transaction currency and the return currency */
        if ( gsb_data_currency_link_get_first_currency (link_number) == transaction -> currency_number)
            amount = gsb_real_mul ( transaction -> transaction_amount,
                        gsb_data_currency_link_get_change_rate (link_number));
        else
            amount = gsb_real_div ( transaction -> transaction_amount,
                        gsb_data_currency_link_get_change_rate (link_number));

        /* The costs are still deducted from the transaction. In case of internal transfer there is no charge. */
        amount = gsb_real_sub (amount, transaction -> exchange_fees);
    }
    else if ( return_currency_number > 0 && transaction -> currency_number > 0 )
    {
        gsb_real current_exchange;
        gsb_real current_exchange_fees;

        gsb_currency_exchange_dialog ( return_currency_number,
                        transaction -> currency_number,
                        0,
                        null_real,
                        null_real,
                        TRUE );

        current_exchange = gsb_currency_get_current_exchange ( );
        current_exchange_fees = gsb_currency_get_current_exchange_fees ( );

        gsb_data_transaction_set_exchange_rate ( transaction_number,
                        gsb_real_abs ( current_exchange ) );
        gsb_data_transaction_set_change_between (transaction_number, 0 );
        amount = gsb_real_div ( transaction -> transaction_amount,
                        current_exchange );

        if ( transaction -> exchange_fees.mantissa == 0
         && current_exchange_fees.mantissa != 0 )
            amount = gsb_real_sub ( amount, current_exchange_fees );
        else
            amount = gsb_real_sub ( amount, transaction -> exchange_fees );
            
    }

    return gsb_real_adjust_exponent  ( amount, return_exponent );
}




/** get the currency_number 
 * \param transaction_number the number of the transaction
 * \return the currency number of the transaction
 * */
gint gsb_data_transaction_get_currency_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> currency_number;
}


/**
 * set the currency_number
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param no_currency
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_currency_number ( gint transaction_number,
                        gint no_currency )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> currency_number = no_currency;
    
    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> currency_number = no_currency;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}




/**
 * get the change_between_account_and_transaction 
 * if the value is 1, we have : 1 account_currency = (exchange_rate * amount) transaction_currency
 * else we have : 1 transaction_currency = (exchange_rate * amount) account_currency
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the currency number of the transaction
 * */
gint gsb_data_transaction_get_change_between ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> change_between_account_and_transaction;
}


/**
 * set the change_between_account_and_transaction
 * if the value is 1, we have : 1 account_currency = (exchange_rate * amount) transaction_currency
 * else we have : 1 transaction_currency = (exchange_rate * amount) account_currency
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_change_between ( gint transaction_number,
                        gint value )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> change_between_account_and_transaction = value;
    
    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> change_between_account_and_transaction = value;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}




/**
 * get the exchange_rate of the transaction
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the exchange_rate of the transaction
 * */
gsb_real gsb_data_transaction_get_exchange_rate ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return null_real;

    return transaction -> exchange_rate;
}


/**
 * set the exchange_rate of the transaction
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param rate
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_exchange_rate ( gint transaction_number,
                        gsb_real rate )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> exchange_rate = rate;
    
    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> exchange_rate = rate;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }
    return TRUE;
}



/** 
 * get the exchange_fees of the transaction
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the exchange_fees of the transaction
 * */
gsb_real gsb_data_transaction_get_exchange_fees ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return null_real;

    return transaction -> exchange_fees;
}


/**
 * set the exchange_fees of the transaction
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param rate
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_exchange_fees ( gint transaction_number,
                        gsb_real rate )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> exchange_fees = rate;
    
    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> exchange_fees = rate;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }
    return TRUE;
}




/** 
 * get the party_number 
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the currency number of the transaction
 * */
gint gsb_data_transaction_get_party_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> party_number;
}


/**
 * set the party_number
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_party_number ( gint transaction_number,
                        gint no_party )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> party_number = no_party;

    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> party_number = no_party;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}



/** 
 * get the category_number 
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the category number of the transaction, -1 for error
 * */
gint gsb_data_transaction_get_category_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> category_number;
}


/** set the category_number
 * \param transaction_number
 * \param value
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_category_number ( gint transaction_number,
                        gint no_category )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> category_number = no_category;

    return TRUE;
}


/** get the sub_category_number 
 * \param transaction_number the number of the transaction
 * \return the sub_category number of the transaction
 * */
gint gsb_data_transaction_get_sub_category_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    /* pbiava le 04/04/2009 uniformisation avec les autres fonctions similaires */
    if ( !transaction )
	return -1;

    return transaction -> sub_category_number;
}


/**
 * set the sub_category_number
 * 
 * \param transaction_number
 * \param value
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_sub_category_number ( gint transaction_number,
                        gint no_sub_category )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> sub_category_number = no_sub_category;

    return TRUE;
}


/**
 * check if the transaction is a split (mother)
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return TRUE if the transaction is a split of transaction
 * */
gint gsb_data_transaction_get_split_of_transaction ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> split_of_transaction;
}


/** 
 * set the transaction as split or not
 *
 * \param transaction_number
 * \param is_split
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_split_of_transaction ( gint transaction_number,
                        gint is_split )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> split_of_transaction = is_split;

    return TRUE;
}


/**
 * get the notes
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the notes of the transaction
 * */
const gchar *gsb_data_transaction_get_notes ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return NULL;

    return transaction -> notes;
}


/** set the notes
 * the notes parameter will be copy before stored in memory
 * \param transaction_number
 * \param no_account
 * \param notes a gchar with the new notes. This string is duplicated.
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_notes ( gint transaction_number,
                        const gchar *notes )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    if ( transaction -> notes )
        g_free ( transaction -> notes );

    if ( notes
	 &&
	 strlen (notes))
	transaction -> notes = my_strdup (notes);
    else
	transaction -> notes = NULL;
    
    return TRUE;
}



/**
 * get the method_of_payment_number
 * \param transaction_number the number of the transaction
 * 
 * \return the method_of_payment_number
 * */
gint gsb_data_transaction_get_method_of_payment_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> method_of_payment_number;
}


/**
 * set the method_of_payment_number
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param 
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_method_of_payment_number ( gint transaction_number,
                        gint number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> method_of_payment_number = number;

    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> method_of_payment_number = number;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}


/** 
 * get the method_of_payment_content
 *
 * \param transaction_number the number of the transaction
 * \return the method_of_payment_content of the transaction
 * */
const gchar *gsb_data_transaction_get_method_of_payment_content ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return NULL;

    return transaction -> method_of_payment_content;
}


/** 
 * set the method_of_payment_content
 * dupplicate the parameter before storing it in the transaction
 *
 * \param transaction_number
 * \param no_account
 * \param method_of_payment_content a gchar with the new method_of_payment_content
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_method_of_payment_content ( gint transaction_number,
                        const gchar *method_of_payment_content )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    if ( transaction -> method_of_payment_content )
        g_free ( transaction -> method_of_payment_content );

    if ( method_of_payment_content
	 &&
	 strlen (method_of_payment_content))
	transaction -> method_of_payment_content = my_strdup (method_of_payment_content);
    else
	transaction -> method_of_payment_content = NULL;
    
    return TRUE;
}



/**
 * get the marked_transaction
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the marked_transaction : OPERATION_NORMALE, OPERATION_POINTEE, OPERATION_TELERAPPROCHEE, OPERATION_RAPPROCHEE
 * */
gint gsb_data_transaction_get_marked_transaction ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> marked_transaction;
}


/**
 * set the marked_transaction
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param marked_transaction : OPERATION_NORMALE, OPERATION_POINTEE, OPERATION_TELERAPPROCHEE, OPERATION_RAPPROCHEE
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_marked_transaction ( gint transaction_number,
                        gint marked_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    gsb_data_account_set_balances_are_dirty ( transaction->account_number );
    transaction -> marked_transaction = marked_transaction;

    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
        GSList *tmp_list;
        GSList *save_tmp_list;

        tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
        save_tmp_list = tmp_list;

        while (tmp_list)
        {
            transaction = tmp_list -> data;
            transaction -> marked_transaction = marked_transaction;
            tmp_list = tmp_list -> next;
        }
        g_slist_free (save_tmp_list);
    }

    return TRUE;
}


/**
 * get the number of the archive
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the archive number, 0 if not archived and -1 if transaction not found
 * */
gint gsb_data_transaction_get_archive_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> archive_number;
}


/**
 * set the number of the archive if it's an archived transaction 
 * add or remove the transaction from the list of transactions without archive according to the value
 * 
 * \param transaction_number
 * \param archive_number or 0 if not an archive
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_archive_number ( gint transaction_number,
                        gint archive_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
        return FALSE;

    /* if the archive_number of the transaction is 0 for now, it's already in that list,
     * so we mustn't add it,
     * else, according to the new value, we remove it
    */
    if ( !transaction -> archive_number )
    {
        /* the transaction was not an archive, so it's into the 2 lists,
         * if we transform it as an archive, we remove it from the transactions_list */
        if ( archive_number )
            transactions_list = g_slist_remove ( transactions_list, transaction );
    }

    transaction -> archive_number = archive_number;

    return TRUE;
}



/** get the automatic_transaction
 * \param transaction_number the number of the transaction
 * \return 1 if the transaction was taken automaticly
 * */
gint gsb_data_transaction_get_automatic_transaction ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> automatic_transaction;
}


/** set the automatic_transaction
 * \param transaction_number
 * \param  automatic_transaction
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_automatic_transaction ( gint transaction_number,
                        gint automatic_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> automatic_transaction = automatic_transaction;

    return TRUE;
}


/** get the reconcile_number
 * \param transaction_number the number of the transaction
 * \return the reconcile_number
 * */
gint gsb_data_transaction_get_reconcile_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> reconcile_number;
}


/**
 * set the reconcile_number
 * if the transaction has some children, they change too
 * 
 * \param transaction_number
 * \param  reconcile_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_reconcile_number ( gint transaction_number,
                        gint reconcile_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> reconcile_number = reconcile_number;

    /* if the transaction is a split, change all the children */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;
	GSList *save_tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction -> transaction_number, FALSE);
	save_tmp_list = tmp_list;

	while (tmp_list)
	{
	    transaction = tmp_list -> data;
	    transaction -> reconcile_number = reconcile_number;
	    tmp_list = tmp_list -> next;
	}
	g_slist_free (save_tmp_list);
    }

    return TRUE;
}


/** get the financial_year_number
 * \param transaction_number the number of the transaction
 * \return the financial_year_number
 * */
gint gsb_data_transaction_get_financial_year_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> financial_year_number;
}


/**
 * set the financial_year_number
 * 
 * \param transaction_number
 * \param  financial_year_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_financial_year_number ( gint transaction_number,
                        gint financial_year_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> financial_year_number = financial_year_number;

    return TRUE;
}



/** get the budgetary_number
 * \param transaction_number the number of the transaction
 * \return the budgetary_number of the transaction
 * */
gint gsb_data_transaction_get_budgetary_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> budgetary_number;
}


/** set the budgetary_number
 * \param transaction_number
 * \param budgetary_number
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_budgetary_number ( gint transaction_number,
                        gint budgetary_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> budgetary_number = budgetary_number;

    return TRUE;
}


/** get the  sub_budgetary_number
 * \param transaction_number the number of the transaction
 * \return the sub_budgetary_number number of the transaction
 * */
gint gsb_data_transaction_get_sub_budgetary_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> sub_budgetary_number;
}


/** set the sub_budgetary_number
 * \param transaction_number
 * \param sub_budgetary_number
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_sub_budgetary_number ( gint transaction_number,
                        gint sub_budgetary_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> sub_budgetary_number = sub_budgetary_number;

    return TRUE;
}


/** get the voucher
 * \param transaction_number the number of the transaction
 * \return  voucher
 * */
const gchar *gsb_data_transaction_get_voucher ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return NULL;

    return transaction -> voucher;
}


/** set the voucher
 * it's a copy of the parameter which will be stored in the transaction
 * \param transaction_number
 * \param voucher. The string is duplicated.
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_voucher ( gint transaction_number,
                        const gchar *voucher )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    
    if ( transaction -> voucher )
        g_free ( transaction -> voucher );

    if ( voucher
	 &&
	 strlen (voucher))
	transaction -> voucher = my_strdup (voucher);
    else
	transaction -> voucher = g_strdup("");
    
    return TRUE;
}



/** 
 * get the bank_references
 *
 * \param transaction_number the number of the transaction
 *
 * \return bank_references
 * */
const gchar *gsb_data_transaction_get_bank_references ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return NULL;

    return transaction -> bank_references;
}


/**
 * set the bank_references
 * it's a copy of the parameter which will be stored in the transaction
 * 
 * \param transaction_number
 * \param bank_references. This string may be NULL. If not NULL, the string is duplicated.
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_bank_references ( gint transaction_number,
                        const gchar *bank_references )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    if ( transaction -> bank_references )
        g_free ( transaction -> bank_references );

    if ( bank_references
	 &&
	 strlen (bank_references))
	transaction -> bank_references = my_strdup (bank_references);
    else
	transaction -> bank_references = NULL;
    
    return TRUE;
}


/**
 * get the  transaction_number_transfer
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the transaction_number_transfer number of the transaction
 * */
gint gsb_data_transaction_get_contra_transaction_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    return transaction -> transaction_number_transfer;
}

/**
 * set the transaction_number_transfer
 * 
 * \param transaction_number
 * \param transaction_number_transfer
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_contra_transaction_number ( gint transaction_number,
                        gint transaction_number_transfer )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> transaction_number_transfer = transaction_number_transfer;

    return TRUE;
}


/**
 * get the account number of the contra transaction
 * it's not saved into grisbi, this function avoid just to get the contra-transaction
 * and find its account
 *
 * \param transaction_number the number of the transaction
 * 
 * \return the account number of the contra-transaction
 * */
gint gsb_data_transaction_get_contra_transaction_account ( gint transaction_number )
{
    struct_transaction *transaction;
    struct_transaction *contra_transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return -1;

    contra_transaction = gsb_data_transaction_get_transaction_by_no (transaction -> transaction_number_transfer);
    if (!contra_transaction)
	return -1;

    return contra_transaction -> account_number;
}




/** 
 * get the  mother_transaction_number
 * 
 * \param transaction_number the number of the transaction
 * 
 * \return the mother_transaction_number of the transaction or 0 if the transaction doen't exist
 * */
gint gsb_data_transaction_get_mother_transaction_number ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no (transaction_number);

    if ( !transaction )
	return 0;

    return transaction -> mother_transaction_number;
}


/**
 * set the mother_transaction_number
 * 
 * \param transaction_number
 * \param mother_transaction_number
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_set_mother_transaction_number ( gint transaction_number,
                        gint mother_transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transaction -> mother_transaction_number = mother_transaction_number;

    return TRUE;
}



/**
 * create a new transaction and append it to the list in the right account
 * set the transaction number given in param (if no number, give the last number + 1)
 * set the number of the account, the number of the transaction and the currency number
 * which is by default the currency of the account
 * 
 * \param no_account the number of the account where the transaction should be made
 * \param transaction_number the number of the transaction
 * 
 * \return the number of the new transaction
 * */
gint gsb_data_transaction_new_transaction_with_number ( gint no_account,
                        gint transaction_number )
{
    struct_transaction *transaction;

    transaction = g_malloc0 ( sizeof ( struct_transaction ));

    if ( !transaction )
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	return FALSE;
    }

    if ( !transaction_number )
	transaction_number = gsb_data_transaction_get_last_number () + 1;

    transaction -> account_number = no_account;
    transaction -> transaction_number = transaction_number;
    transaction -> currency_number = gsb_data_account_get_currency (no_account);
    transaction -> voucher = g_strdup("");
    transaction -> bank_references = g_strdup("");

    /* we append the transaction to the complete transactions list and the non archive transaction list */
    transactions_list = g_slist_append ( transactions_list,
					 transaction );
    complete_transactions_list = g_slist_append ( complete_transactions_list,
						  transaction );

    gsb_data_transaction_save_transaction_pointer (transaction);

    return transaction -> transaction_number;
}


/**
 * create a new transaction with gsb_data_transaction_new_transaction_with_number
 * but set automatickly the last number
 * 
 * \param no_account the number of the account where the transaction should be made
 * 
 * \return the number of the new transaction
 * */
gint gsb_data_transaction_new_transaction ( gint no_account )
{
    return gsb_data_transaction_new_transaction_with_number ( no_account,
							      gsb_data_transaction_get_last_number () + 1);
}


/** 
 * create a new white line
 * if there is a mother transaction, it's a split and we increment in the negatives values
 * the number of that line
 * without mother transaction, it's the general white line, the number is -1
 *
 * if it's a child split, the account is set as for its mother,
 * if it's the last white line, the account is set to -1
 * that transaction is appended to the white transactions list
 * 
 * \param mother_transaction_number the number of the mother's transaction if it's a split child ; 0 if not
 *
 * \return the number of the white line
 * */
gint gsb_data_transaction_new_white_line ( gint mother_transaction_number)
{
    struct_transaction *transaction;

    transaction = g_malloc0 ( sizeof ( struct_transaction ));

    if ( !transaction )
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	return 0;
    }

    /* we fill some things for the child split to help to sort the list */

    transaction -> account_number = gsb_data_transaction_get_account_number (mother_transaction_number);

    if ( mother_transaction_number )
    {
	transaction -> transaction_number = gsb_data_transaction_get_last_white_number () - 1;
	transaction -> date = gsb_date_copy ( gsb_data_transaction_get_date (mother_transaction_number));
	transaction -> party_number = gsb_data_transaction_get_party_number (mother_transaction_number);
	transaction -> mother_transaction_number = mother_transaction_number;
    }
    else
	transaction -> transaction_number = -1;

    white_transactions_list = g_slist_append ( white_transactions_list,
					       transaction );

    gsb_data_transaction_save_transaction_pointer (transaction);

    return transaction -> transaction_number;
}



/**
 * copy the content of a transaction into the second one
 * the 2 transactions must exist before
 * only the account_number and the transaction_number will be modified in the target transaction
 * all the char are dupplicated,
 * 	transaction_id is set to NULL,
 * 	marked, reconcile number and archive number are set to 0
 * 
 * \param source_transaction_number the transaction we want to copy
 * \param target_transaction_number the trnasaction we want to fill with the content of the first one
 * \param reset transaction_id, marked, reconcile number and archive number
 * 
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_data_transaction_copy_transaction ( gint source_transaction_number,
                        gint target_transaction_number,
                        gboolean reset_mark )
{
    struct_transaction *source_transaction;
    struct_transaction *target_transaction;
    gint target_transaction_account_number;

    source_transaction = gsb_data_transaction_get_transaction_by_no ( source_transaction_number);
    target_transaction = gsb_data_transaction_get_transaction_by_no ( target_transaction_number);

    if ( !source_transaction
	 ||
	 !target_transaction )
	return FALSE;

    target_transaction_account_number = target_transaction -> account_number;

    memcpy ( target_transaction,
	     source_transaction,
	     sizeof ( struct_transaction ));
    target_transaction -> transaction_number = target_transaction_number;
    target_transaction -> account_number = target_transaction_account_number;
    if ( reset_mark )
    {
        target_transaction -> reconcile_number = 0;
        target_transaction -> marked_transaction = 0;
        target_transaction -> transaction_id = NULL;
        target_transaction -> archive_number = 0;
    }

    /* make the archive_number */
    target_transaction -> archive_number = 0;

    /* make a new copy of all the pointers */
    if ( target_transaction -> notes)
	target_transaction -> notes = my_strdup ( source_transaction -> notes );

    if ( target_transaction -> voucher)
	target_transaction -> voucher = my_strdup ( source_transaction -> voucher );

    if ( target_transaction -> bank_references)
	target_transaction -> bank_references = my_strdup ( source_transaction -> bank_references );

    if ( target_transaction -> date)
	target_transaction -> date = gsb_date_copy (source_transaction -> date);

    if ( target_transaction -> value_date)
	target_transaction -> value_date = gsb_date_copy (source_transaction -> value_date);

    if ( target_transaction -> method_of_payment_content)
	target_transaction -> method_of_payment_content = my_strdup (
                        source_transaction -> method_of_payment_content );
    return TRUE;
}

/**
 * internal function which is called to free the memory used by a struct_transaction structure.
 */
static void gsb_data_transaction_free ( struct_transaction *transaction)
{
    if ( ! transaction )
        return;
    gsb_data_account_set_balances_are_dirty ( transaction -> account_number );
    if ( transaction -> transaction_id )
        g_free ( transaction -> transaction_id );
    if ( transaction -> notes )
        g_free ( transaction -> notes );
    if ( transaction -> voucher )
        g_free ( transaction -> voucher );
    if ( transaction -> bank_references )
        g_free ( transaction -> bank_references );
    if ( transaction -> date )
        g_date_free ( transaction -> date );
    if ( transaction -> value_date )
        g_date_free ( transaction -> value_date );
    if ( transaction -> method_of_payment_content )
        g_free ( transaction -> method_of_payment_content );
    g_free ( transaction );
    transaction_buffer[0] = NULL;
    transaction_buffer[1] = NULL;
}

/**
 * remove the transaction from the transaction's list
 * free the transaction
 * if there is a contra-transaction, remove it too
 * if there is some children, remove them and their contra-transaction if they exist
 * 
 * \param transaction_number
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_remove_transaction ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no (transaction_number);

    if ( !transaction )
	return FALSE;

    /* check if it's a transfer */
    if (transaction -> transaction_number_transfer)
    {
	struct_transaction *contra_transaction;

	contra_transaction = gsb_data_transaction_get_transaction_by_no (transaction -> transaction_number_transfer);
	if (contra_transaction)
	{
	    /* we remove the transaction from the counters */
	    gsb_data_payee_remove_transaction_from_payee (contra_transaction -> transaction_number) ;
	    gsb_data_category_remove_transaction_from_category (contra_transaction -> transaction_number);
	    gsb_data_budget_remove_transaction_from_budget (contra_transaction -> transaction_number);

	    /* we remove the transaction from the 2 lists */
	    transactions_list = g_slist_remove ( transactions_list,
						 contra_transaction );
	    complete_transactions_list = g_slist_remove ( complete_transactions_list,
							  contra_transaction );
	    gsb_data_transaction_free (contra_transaction);
	}
    }

    /* check if it's a split */
    if (transaction -> split_of_transaction)
    {
	GSList *tmp_list;

	tmp_list = gsb_data_transaction_get_children (transaction_number, FALSE);
	while (tmp_list)
	{
	    struct_transaction *child_transaction;
	    struct_transaction *contra_transaction;

	    child_transaction = tmp_list -> data;

	    contra_transaction = gsb_data_transaction_get_transaction_by_no (child_transaction -> transaction_number_transfer);
	    if (contra_transaction)
	    {
		/* it's a transfer, delete the transfer */

		/* we remove the transaction from the counters */
		gsb_data_payee_remove_transaction_from_payee (contra_transaction -> transaction_number) ;
		gsb_data_category_remove_transaction_from_category (contra_transaction -> transaction_number);
		gsb_data_budget_remove_transaction_from_budget (contra_transaction -> transaction_number);

		transactions_list = g_slist_remove ( transactions_list,
						     contra_transaction );
		complete_transactions_list = g_slist_remove ( complete_transactions_list,
							      contra_transaction );
		gsb_data_transaction_free (contra_transaction);
	    }

	    /* delete the child */
	    /* we remove the child from the counters */
	    gsb_data_payee_remove_transaction_from_payee (child_transaction -> transaction_number) ;
	    gsb_data_category_remove_transaction_from_category (child_transaction -> transaction_number);
	    gsb_data_budget_remove_transaction_from_budget (child_transaction -> transaction_number);

	    transactions_list = g_slist_remove ( transactions_list,
						 child_transaction );
	    complete_transactions_list = g_slist_remove ( complete_transactions_list,
							  child_transaction );
	    gsb_data_transaction_free (child_transaction);
	    tmp_list = tmp_list -> next;
	}
    }

    /* we have now to remove the transaction from the counters */
    gsb_data_payee_remove_transaction_from_payee (transaction_number);
    gsb_data_category_remove_transaction_from_category (transaction_number);
    gsb_data_budget_remove_transaction_from_budget (transaction_number);

    /* now can remove safely the transaction */
    transactions_list = g_slist_remove ( transactions_list,
					 transaction );
    complete_transactions_list = g_slist_remove ( complete_transactions_list,
						  transaction );

    gsb_data_transaction_free (transaction);

    /* force the update module budget */
    gsb_data_account_set_bet_maj ( transaction -> account_number, BET_MAJ_ALL );

    return TRUE;
}

/**
 * remove the transaction from the transaction's list
 * just do it and only for that transaction, check nothing
 * usefull when we need to delete a lot of transactions in one time
 * 	(example to delete an archive)
 * 
 * \param transaction_number
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_transaction_remove_transaction_without_check ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no (transaction_number);

    if ( !transaction )
	return FALSE;

    /* delete the transaction from the lists */
    transactions_list = g_slist_remove ( transactions_list,
					 transaction );
    complete_transactions_list = g_slist_remove ( complete_transactions_list,
						  transaction );

    /* we free the buffer to avoid big possibly crashes */
    transaction_buffer[0] = NULL;
    transaction_buffer[1] = NULL;

    g_free (transaction);
    return TRUE;
}

/**
 * Delete all transactions and free memory used by them
 */
void gsb_data_transaction_delete_all_transactions ( void )
{
    if ( complete_transactions_list )
    {
        GSList* tmp_list = complete_transactions_list;
        while ( tmp_list )
        {
	    struct_transaction *transaction;
	    transaction = tmp_list -> data;
	    tmp_list = tmp_list -> next;
            gsb_data_transaction_free ( transaction );
	}
        g_slist_free ( complete_transactions_list );
        complete_transactions_list = NULL;
    }
    if ( transactions_list )
    {
        g_slist_free ( transactions_list );
        transactions_list = NULL;
    }
    transaction_buffer[0] = NULL;
    transaction_buffer[1] = NULL;
    current_transaction_buffer = 0;
}

/**
 * find the children of the split given in param and
 * return their adress or their number in a GSList
 * the list sould be freed
 *
 * \param transaction_number a split of transaction
 * \param return_number TRUE if we want a list of numbers, FALSE if we want a list of adress
 *
 * \return a GSList of the address/numbers of the children, NULL if no child
 * */
GSList *gsb_data_transaction_get_children ( gint transaction_number,
                        gboolean return_number)
{
    struct_transaction *transaction;
    GSList *children_list = NULL;
    GSList *tmp_list;

    transaction = gsb_data_transaction_get_transaction_by_no (transaction_number);

    if ( !transaction
	 ||
	 !transaction -> split_of_transaction)
	return NULL;

    /* get the normal children */
    tmp_list = transactions_list;
    while ( tmp_list )
    {
	struct_transaction *tmp_transaction;

	tmp_transaction = tmp_list -> data;

	if ( tmp_transaction -> mother_transaction_number == transaction_number )
	{
	    if (return_number)
		children_list = g_slist_append ( children_list,
						 GINT_TO_POINTER (tmp_transaction -> transaction_number));
	    else
		children_list = g_slist_append ( children_list,
						 tmp_transaction);
	}
	tmp_list = tmp_list -> next;
    }

    /* get the white line too */
    tmp_list = white_transactions_list;
    while ( tmp_list )
    {
	struct_transaction *tmp_transaction;

	tmp_transaction = tmp_list -> data;

	if ( tmp_transaction -> mother_transaction_number == transaction_number )
	{
	    if (return_number)
		children_list = g_slist_append ( children_list,
						 GINT_TO_POINTER (tmp_transaction -> transaction_number));
	    else
		children_list = g_slist_append ( children_list,
						 tmp_transaction);
	}
	tmp_list = tmp_list -> next;
    }

    return children_list;
}


/**
 * find a transaction by the method of payment content in a given account
 *
 * \param string
 * \param account_number
 *
 * \return the number of the transaction or 0 if not found
 * */
gint gsb_data_transaction_find_by_payment_content ( const gchar *string,
                        gint account_number )
{
    GSList *tmp_list;
    gint64 number_1;
    gchar *endptr;

    if (!string)
        return 0;

    errno = 0;
    number_1 = g_ascii_strtoll ( string, &endptr, 10);
    if ( errno == ERANGE )
        return 0;

    if ( endptr )
        return 0;

    tmp_list = transactions_list;
    while (tmp_list)
    {
        struct_transaction *transaction;

        transaction = tmp_list -> data;

        if ( transaction -> method_of_payment_content
         &&
         transaction -> account_number == account_number )
        {
            gint64 number_2;

            errno = 0;
            number_2 = g_ascii_strtoll ( transaction -> method_of_payment_content, &endptr, 10);
            if ( errno == ERANGE )
                return 0;
            if ( endptr )
                return 0;

            if ( number_1 == number_2 )
                return transaction -> transaction_number;
        }
        tmp_list = tmp_list -> next;
    }
    return 0;
}


/**
 * find a transaction by its id
 *
 * \param id a string containing an id
 *
 * \return the number of transaction or 0 if none found
 * */
gint gsb_data_transaction_find_by_id ( gchar *id, gint account_number )
{
    GSList *tmp_list;

    if ( !id )
        return 0;

    tmp_list = g_slist_copy ( transactions_list );
    
    tmp_list = g_slist_sort (tmp_list, 
                        (GCompareFunc) classement_sliste_transactions_par_date_decroissante );

    while (tmp_list)
    {
        struct_transaction *transaction;

        transaction = tmp_list -> data;

        if ( transaction -> transaction_id
         &&
         !strcmp ( id, transaction -> transaction_id )
         &&
         account_number == gsb_data_transaction_get_account_number (
         transaction -> transaction_number ) )
            return transaction -> transaction_number;

        tmp_list = tmp_list -> next;
    }

    return 0;
}



/**
 * find the white line corresponding to the transaction
 * given in param and return the number
 * if the transaction is not a split, return -1, the general white line
 *
 * \param transaction_number a split transaction number
 *
 * \return the number of the white line of the split or -1
 * */
gint gsb_data_transaction_get_white_line ( gint transaction_number )
{
    struct_transaction *transaction;
    GSList *tmp_list;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction
	 ||
	 !transaction -> split_of_transaction)
	return -1;

    tmp_list = white_transactions_list;

    while ( tmp_list )
    {
	struct_transaction *tmp_transaction;

	tmp_transaction = tmp_list -> data;

	if ( tmp_transaction -> mother_transaction_number == transaction_number )
	    return tmp_transaction -> transaction_number;

	tmp_list = tmp_list -> next;
    }
    return -1;
}


/**
 * check if a number of cheque is not already used for a automatic numbering method of payment
 *
 * \param payment_number an automatic numbering method of payment
 * \param number the number we want to set as content for that method of payment
 * 		(ie the number of cheque...)
 *
 * \return the number of the transaction if one is found, FALSE if none found, so can use it
 * */
gint gsb_data_transaction_check_content_payment ( gint payment_number,
                        const gchar *number )
{
    GSList *tmp_list;

    if (!gsb_data_payment_get_automatic_numbering (payment_number))
	/* the method of payment is not an automatic numbering, return TRUE because
	 * can always use this number */
	return FALSE;

    if (!number)
	return FALSE;

    tmp_list = transactions_list;
    while (tmp_list)
    {
	struct_transaction *transaction;

	transaction = tmp_list -> data;

	if ( transaction -> method_of_payment_number == payment_number
	     &&
         transaction -> method_of_payment_content
         &&
	     g_ascii_strcasecmp (transaction -> method_of_payment_content, number ) == 0 )
	    return transaction -> transaction_number;

	tmp_list = tmp_list -> next;
    }
    return FALSE;
}


/** 
 * return a copy of the g_slist of transactions structure
 * sorted by date
 *  * THIS IS THE LIST WITHOUT THE ARCHIVED TRANSACTIONS
 *
 * \param none
 *
 * \return the slist of transactions structures
 * */
GSList *gsb_data_transaction_get_transactions_list_by_date ( void )
{
    GSList *list_tmp;

    if ( etat.add_archive_in_total_balance )
        list_tmp = g_slist_copy ( complete_transactions_list );
    else
        list_tmp = g_slist_copy ( transactions_list );

    if ( etat.metatree_sort_transactions == 1 )
        list_tmp = g_slist_sort (list_tmp,
                        (GCompareFunc) classement_sliste_transactions_par_date );
    else
        list_tmp = g_slist_sort (list_tmp,
                        (GCompareFunc) classement_sliste_transactions_par_date_decroissante );

    return list_tmp;
}


/**
 * get the id of the transaction
 *
 * \param transaction_number the number of the transaction
 * 
 * \return the id of the transaction
 * */
const gchar *gsb_data_transaction_get_id ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
        return NULL;

    return transaction -> transaction_id;
}


/**
 *
 *
 *
 *
 * */
gsb_real gsb_data_transaction_get_last_transaction_with_div_sub_div (
                        gint account_number,
                        gint div_number,
                        gint sub_div_nb,
                        gint type_div )
{
    GSList *tmp_list;

    tmp_list = g_slist_copy ( transactions_list );
    
    tmp_list = g_slist_sort (tmp_list, 
                        (GCompareFunc) classement_sliste_transactions_par_date_decroissante );

    while ( tmp_list )
    {
        struct_transaction *transaction;

        transaction = tmp_list -> data;

        if ( type_div == 0
         && transaction -> account_number == account_number
         && transaction -> category_number == div_number
         && transaction -> sub_category_number == sub_div_nb )
            return transaction -> transaction_amount;
        else if ( type_div == 1 
         && transaction -> account_number == account_number
         && transaction -> budgetary_number == div_number
         && transaction -> sub_budgetary_number == sub_div_nb )
            return transaction -> transaction_amount;

        tmp_list = tmp_list -> next;
    }

    return null_real;
}


/**
 * get floating point of the currency of the transaction given
 *
 * \param transaction_number the number of the transaction
 *
 * \return the floating_point of currency number of the transaction
 * */
gint gsb_data_transaction_get_currency_floating_point ( gint transaction_number )
{
    struct_transaction *transaction;
    gint floating_point;

    transaction = gsb_data_transaction_get_transaction_by_no ( transaction_number);

    if ( !transaction )
        return -1;
    else
    {
        floating_point = gsb_data_currency_get_floating_point ( transaction -> currency_number );
        return floating_point;
    }
}


/**
 *
 *
 *
 *
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
