/* ************************************************************************** */
/* work with the struct of accounts                                           */
/*                                                                            */
/*                                  data_account                              */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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
 * \file gsb_transaction_data.c
 * work with the transaction structure, no GUI here
 */


#include "include.h"


/*START_INCLUDE*/
#include "gsb_transaction_data.h"
#include "utils_devises.h"
#include "dialog.h"
#include "gsb_account.h"
#include "utils_dates.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static struct_transaction *gsb_transaction_data_get_transaction_by_no ( gint no_transaction );
static gboolean gsb_transaction_data_save_transaction_pointer ( gpointer transaction );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** the g_slist which contains all the transactions structures */
static GSList *transactions_list;

/** 2 pointers to the 2 last transaction used (to increase the speed) */
static struct_transaction *transaction_buffer[2];

/** set the current buffer used */
static gint current_transaction_buffer;


/** set the transactions global variables to NULL, usually when we init all the global variables
 * \param none
 * \return FALSE
 * */
gboolean gsb_transaction_data_init_variables ( void )
{
    transaction_buffer[0] = NULL;
    transaction_buffer[1] = NULL;
    current_transaction_buffer = 0;
    transactions_list = NULL;

    return FALSE;
}


/** 
 * return a pointer to the g_slist of transactions structure
 * it's not a copy, so we must not free or change it
 * if we want to change something, use gsb_transaction_data_copy_transactions_list instead
 *
 * \param none
 * \return the slist of transactions structures
 * */
GSList *gsb_transaction_data_get_transactions_list ( void )
{
    return transactions_list;
}



/** transitionnal fonction, give back the pointer to the transaction,
 * normally deleted when the transfer from old transactions is finished 
 * set also in the transactions list, check if it's fast enough with only the transactions number ? */
gpointer gsb_transaction_data_get_pointer_to_transaction ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( transaction_number);

    return transaction;
}

/** find the last number of transaction
 * \param none
 * \return the number
 * */
gint gsb_transaction_data_get_last_number (void)
{
    gint last_number = 0;
    GSList *transactions_list_tmp;

    transactions_list_tmp = transactions_list;

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

/** get the number of the transaction and save the pointer in the buffer
 * which will increase the speed later
 * \param transaction a pointer to a transaction
 * \return the number of the transaction
 * */
gint gsb_transaction_data_get_transaction_number ( gpointer transaction_pointer )
{
    struct_transaction *transaction;

    transaction = transaction_pointer;

    if ( !transaction )
	return 0;

    /* if we want the transaction number, usually it's to make other stuff after that
     * so we will save the adr of the transaction to increase the speed after */

    gsb_transaction_data_save_transaction_pointer ( transaction );

    return transaction -> transaction_number;
}


/** save the pointer in a buffer to increase the speed later
 * \param transaction the pointer to the transaction
 * \return TRUE or FALSE if pb
 * */
gboolean gsb_transaction_data_save_transaction_pointer ( gpointer transaction )
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


/** return the transaction which the number is in the parameter. 
 * the new transaction is stored in the buffer
 * \param no_transaction
 * \return a pointer to the transaction, NULL if not found
 * */
struct_transaction *gsb_transaction_data_get_transaction_by_no ( gint no_transaction )
{
    GSList *transactions_list_tmp;

    /* check first if the transaction is in the buffer */

    if ( transaction_buffer[0]
	 &&
	 transaction_buffer[0] -> transaction_number == no_transaction )
	return transaction_buffer[0];

    if ( transaction_buffer[1]
	 &&
	 transaction_buffer[1] -> transaction_number == no_transaction )
	return transaction_buffer[1];

    transactions_list_tmp = transactions_list;

    while ( transactions_list_tmp )
    {
	struct_transaction *transaction;

	transaction = transactions_list_tmp -> data;

	if ( transaction -> transaction_number == no_transaction )
	{
	    gsb_transaction_data_save_transaction_pointer ( transaction );
	    return transaction;
	}

	transactions_list_tmp = transactions_list_tmp -> next;
    }

    /* here, we didn't find any transaction with that number */

    return NULL;
}


/** get the transaction_id
 * \param no_transaction the number of the transaction
 * \param no_account the number of account, may be -1, in that case we will look for the transaction in all accounts
 * \return the id of the transaction
 * */
gchar *gsb_transaction_data_get_transaction_id ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return NULL;

    return transaction -> transaction_id;
}


/** set the transaction_id 
 * \param no_transaction
 * \param no_account
 * \param transaction_id a gchar with the new transaction_id
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_transaction_id ( gint no_transaction,
						   gchar *transaction_id )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> transaction_id = transaction_id;
    
    return TRUE;
}


/** get the account_number
 * \param no_transaction the number of the transaction
 * \return the account of the transaction or -1 if problem
 * */
gint gsb_transaction_data_get_account_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> account_number;
}


/** set the account_number
 * \param no_transaction
 * \param no_account
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_account_number ( gint no_transaction,
						   gint no_account )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> account_number = no_account;
    
    return TRUE;
}



/** get the GDate of the transaction 
 * \param no_transaction the number of the transaction
 * \return the GDate of the transaction
 * */
GDate *gsb_transaction_data_get_date ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return NULL;

    return transaction -> date;
}


/** set the GDate of the transaction
 * \param no_transaction
 * \param no_account
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_date ( gint no_transaction,
					 GDate *date )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> date = date;
    
    return TRUE;
}




/** get the value GDate of the transaction 
 * \param no_transaction the number of the transaction
 * \return the GDate of the transaction
 * */
GDate *gsb_transaction_data_get_value_date ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return NULL;

    return transaction -> value_date;
}


/** set the value GDate of the transaction
 * \param no_transaction
 * \param date the value date
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_value_date ( gint no_transaction,
					       GDate *date )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> value_date = date;
    
    return TRUE;
}





/** get the amount of the transaction without any currency change
 * (so just get the given amout)
 * \param no_transaction the number of the transaction
 * \return the amount of the transaction
 * */
gdouble gsb_transaction_data_get_amount ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return 0;

    return transaction -> transaction_amount;
}


/** set the amount of the transaction
 * \param no_transaction
 * \param amount
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_amount ( gint no_transaction,
					   gdouble amount )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> transaction_amount = amount;
    
    return TRUE;
}



/** get the amount of the transaction, modified to be ok with the currency
 * of the account
 * \param no_transaction the number of the transaction
 * \return the amount of the transaction
 * */
gdouble gsb_transaction_data_get_adjusted_amount ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return 0;

    return gsb_transaction_data_get_adjusted_amount_for_currency ( no_transaction,
								   gsb_account_get_currency ( transaction -> account_number ));
}




/** get the amount of the transaction, modified to be ok with the currency
 * given in param 
 * \param no_transaction the number of the transaction
 * \param no_currency_for_return the currency we want to adjust the transaction's amount
 * \return the amount of the transaction
 * */
gdouble gsb_transaction_data_get_adjusted_amount_for_currency ( gint no_transaction,
								gint no_currency_for_return )
{
    struct_transaction *transaction;
    struct struct_devise *return_currency;
    struct struct_devise *transaction_currency;
    gdouble amount = 0;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( ! (transaction
	    &&
	    no_currency_for_return ))
	return 0;

    /* if the transaction currency is the same of the account's one,
     * we just return the transaction's amount */

    if ( transaction -> currency_number == no_currency_for_return )
	return transaction -> transaction_amount;

    /* get the currencies and check if they exist */
    
    return_currency = devise_par_no ( no_currency_for_return );
    transaction_currency = devise_par_no ( transaction -> currency_number );

    if ( !(return_currency
	   &&
	   transaction_currency ))
	return 0;

    /* now we can adjust the amount */

    if ( return_currency -> passage_euro
	 &&
	 !strcmp ( transaction_currency -> nom_devise,
		   _("Euro")))
    	amount = transaction -> transaction_amount * return_currency -> change;
    else
	if ( transaction_currency -> passage_euro
	     &&
	     !strcmp ( return_currency -> nom_devise, _("Euro") ))
	    amount = transaction -> transaction_amount / transaction_currency -> change;
	else
	    if ( transaction -> exchange_rate )
	    {
		if ( transaction -> change_between_account_and_transaction )
		    amount = transaction -> transaction_amount / transaction -> exchange_rate - transaction -> exchange_fees;
		else
		    amount = transaction -> transaction_amount * transaction -> exchange_rate - transaction -> exchange_fees;
	    }
	    else
	    {
		if ( transaction_currency -> no_devise_en_rapport == return_currency -> no_devise
		     &&
		     transaction_currency -> change )
		{
		    if ( transaction -> change_between_account_and_transaction )
			amount = transaction -> transaction_amount * transaction_currency -> change - transaction -> exchange_fees;
		    else
			amount = transaction -> transaction_amount / transaction_currency -> change - transaction -> exchange_fees;
		} 
	    }

    amount = ( rint (amount * 100 )) / 100;
	
    return amount;
}




/** get the currency_number 
 * \param no_transaction the number of the transaction
 * \return the currency number of the transaction
 * */
gint gsb_transaction_data_get_currency_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> currency_number;
}


/** set the currency_number
 * \param no_transaction
 * \param no_currency
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_currency_number ( gint no_transaction,
						    gint no_currency )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> currency_number = no_currency;
    
    return TRUE;
}




/** get the change_between_account_and_transaction 
 * if the value is 1, we have : 1 account_currency = (exchange_rate * amount) transaction_currency
 * else we have : 1 transaction_currency = (exchange_rate * amount) account_currency
 * \param no_transaction the number of the transaction
 * \return the currency number of the transaction
 * */
gint gsb_transaction_data_get_change_between ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> change_between_account_and_transaction;
}


/** set the change_between_account_and_transaction
 * if the value is 1, we have : 1 account_currency = (exchange_rate * amount) transaction_currency
 * else we have : 1 transaction_currency = (exchange_rate * amount) account_currency
 * \param no_transaction
 * \param value
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_change_between ( gint no_transaction,
						   gint value )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> change_between_account_and_transaction = value;
    
    return TRUE;
}




/** get the exchange_rate of the transaction
 * \param no_transaction the number of the transaction
 * \return the exchange_rate of the transaction
 * */
gdouble gsb_transaction_data_get_exchange_rate ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return 0;

    return transaction -> exchange_rate;
}


/** set the exchange_rate of the transaction
 * \param no_transaction
 * \param rate
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_exchange_rate ( gint no_transaction,
						  gdouble rate )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> exchange_rate = rate;
    
    return TRUE;
}



/** get the exchange_fees of the transaction
 * \param no_transaction the number of the transaction
 * \return the exchange_fees of the transaction
 * */
gdouble gsb_transaction_data_get_exchange_fees ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return 0;

    return transaction -> exchange_fees;
}


/** set the exchange_fees of the transaction
 * \param no_transaction
 * \param rate
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_exchange_fees ( gint no_transaction,
						  gdouble rate )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> exchange_fees = rate;
    
    return TRUE;
}




/** get the party_number 
 * \param no_transaction the number of the transaction
 * \return the currency number of the transaction
 * */
gint gsb_transaction_data_get_party_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> party_number;
}


/** set the party_number
 * \param no_transaction
 * \param value
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_party_number ( gint no_transaction,
						 gint no_party )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> party_number = no_party;

    return TRUE;
}



/** get the category_number 
 * \param no_transaction the number of the transaction
 * \return the category number of the transaction
 * */
gint gsb_transaction_data_get_category_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> category_number;
}


/** set the category_number
 * \param no_transaction
 * \param value
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_category_number ( gint no_transaction,
						    gint no_category )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> category_number = no_category;

    return TRUE;
}


/** get the sub_category_number 
 * \param no_transaction the number of the transaction
 * \return the sub_category number of the transaction
 * */
gint gsb_transaction_data_get_sub_category_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> sub_category_number;
}


/** set the sub_category_number
 * \param no_transaction
 * \param value
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_sub_category_number ( gint no_transaction,
							gint no_sub_category )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> sub_category_number = no_sub_category;

    return TRUE;
}


/** get if the transaction is a breakdown_of_transaction
 * \param no_transaction the number of the transaction
 * \return TRUE if the transaction is a breakdown of transaction
 * */
gint gsb_transaction_data_get_breakdown_of_transaction ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> breakdown_of_transaction;
}


/** set if the transaction is a breakdown_of_transaction
 * \param no_transaction
 * \param is_breakdown
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_breakdown_of_transaction ( gint no_transaction,
							     gint is_breakdown )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> breakdown_of_transaction = is_breakdown;

    return TRUE;
}


/** get the notes
 * \param no_transaction the number of the transaction
 * \return the notes of the transaction
 * */
gchar *gsb_transaction_data_get_notes ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return NULL;

    return transaction -> notes;
}


/** set the notes
 * the notes parameter will be copy before stored in memory
 * \param no_transaction
 * \param no_account
 * \param notes a gchar with the new notes
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_notes ( gint no_transaction,
					  gchar *notes )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    if ( notes
	 &&
	 strlen (notes))
	transaction -> notes = g_strdup (notes);
    else
	transaction -> notes = NULL;
    
    return TRUE;
}



/** get the method_of_payment_number
 * \param no_transaction the number of the transaction
 * \return the method_of_payment_number
 * */
gint gsb_transaction_data_get_method_of_payment_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> method_of_payment_number;
}


/** set the method_of_payment_number
 * \param no_transaction
 * \param 
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_method_of_payment_number ( gint no_transaction,
							     gint number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> method_of_payment_number = number;

    return TRUE;
}


/** get the method_of_payment_content
 * \param no_transaction the number of the transaction
 * \return the method_of_payment_content of the transaction
 * */
gchar *gsb_transaction_data_get_method_of_payment_content ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return NULL;

    return transaction -> method_of_payment_content;
}


/** set the method_of_payment_content
 * dupplicate the parameter before storing it in the transaction
 * \param no_transaction
 * \param no_account
 * \param method_of_payment_content a gchar with the new method_of_payment_content
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_method_of_payment_content ( gint no_transaction,
							      gchar *method_of_payment_content )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    if ( method_of_payment_content
	 &&
	 strlen (method_of_payment_content))
	transaction -> method_of_payment_content = g_strup (method_of_payment_content);
    else
	transaction -> method_of_payment_content = NULL;
    
    return TRUE;
}



/** get the marked_transaction
 * \param no_transaction the number of the transaction
 * \return the marked_transaction : 0=nothing, 1=P, 2=T, 3=R
 * */
gint gsb_transaction_data_get_marked_transaction ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> marked_transaction;
}


/** set the marked_transaction
 * \param no_transaction
 * \param marked_transaction : 0=nothing, 1=P, 2=T, 3=R
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_marked_transaction ( gint no_transaction,
						       gint marked_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> marked_transaction = marked_transaction;

    return TRUE;
}



/** get the automatic_transaction
 * \param no_transaction the number of the transaction
 * \return 1 if the transaction was taken automaticly
 * */
gint gsb_transaction_data_get_automatic_transaction ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> automatic_transaction;
}


/** set the automatic_transaction
 * \param no_transaction
 * \param  automatic_transaction
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_automatic_transaction ( gint no_transaction,
							  gint automatic_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> automatic_transaction = automatic_transaction;

    return TRUE;
}


/** get the reconcile_number
 * \param no_transaction the number of the transaction
 * \return the reconcile_number
 * */
gint gsb_transaction_data_get_reconcile_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> reconcile_number;
}


/** set the reconcile_number
 * \param no_transaction
 * \param  reconcile_number
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_reconcile_number ( gint no_transaction,
						     gint reconcile_number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> reconcile_number = reconcile_number;

    return TRUE;
}


/** get the financial_year_number
 * \param no_transaction the number of the transaction
 * \return the financial_year_number
 * */
gint gsb_transaction_data_get_financial_year_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> financial_year_number;
}


/** set the financial_year_number
 * \param no_transaction
 * \param  financial_year_number
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_financial_year_number ( gint no_transaction,
							  gint financial_year_number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> financial_year_number = financial_year_number;

    return TRUE;
}



/** get the budgetary_number
 * \param no_transaction the number of the transaction
 * \return the budgetary_number of the transaction
 * */
gint gsb_transaction_data_get_budgetary_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> budgetary_number;
}


/** set the budgetary_number
 * \param no_transaction
 * \param budgetary_number
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_budgetary_number ( gint no_transaction,
						     gint budgetary_number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> budgetary_number = budgetary_number;

    return TRUE;
}


/** get the  sub_budgetary_number
 * \param no_transaction the number of the transaction
 * \return the sub_budgetary_number number of the transaction
 * */
gint gsb_transaction_data_get_sub_budgetary_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> sub_budgetary_number;
}


/** set the sub_budgetary_number
 * \param no_transaction
 * \param sub_budgetary_number
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_sub_budgetary_number ( gint no_transaction,
							 gint sub_budgetary_number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> sub_budgetary_number = sub_budgetary_number;

    return TRUE;
}


/** get the voucher
 * \param no_transaction the number of the transaction
 * \return  voucher
 * */
gchar *gsb_transaction_data_get_voucher ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return NULL;

    return transaction -> voucher;
}


/** set the voucher
 * it's a copy of the parameter which will be stored in the transaction
 * \param no_transaction
 * \param voucher
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_voucher ( gint no_transaction,
					    gchar *voucher )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    if ( voucher
	 &&
	 strlen (voucher))
	transaction -> voucher = voucher;
    else
	transaction -> voucher = NULL;
    
    return TRUE;
}



/** get the bank_references
 * \param no_transaction the number of the transaction
 * \return bank_references
 * */
gchar *gsb_transaction_data_get_bank_references ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return NULL;

    return transaction -> bank_references;
}


/** set the bank_references
 * it's a copy of the parameter which will be stored in the transaction
 * \param no_transaction
 * \param bank_references
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_bank_references ( gint no_transaction,
						    gchar *bank_references )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    if ( bank_references
	 &&
	 strlen (bank_references))
	transaction -> bank_references = bank_references;
    else
	transaction -> bank_references = NULL;
    
    return TRUE;
}


/** get the  transaction_number_transfer
 * \param no_transaction the number of the transaction
 * \return the transaction_number_transfer number of the transaction
 * */
gint gsb_transaction_data_get_transaction_number_transfer ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> transaction_number_transfer;
}


/** set the transaction_number_transfer
 * \param no_transaction
 * \param transaction_number_transfer
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_transaction_number_transfer ( gint no_transaction,
							 gint transaction_number_transfer )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> transaction_number_transfer = transaction_number_transfer;

    return TRUE;
}


/** get the  account_number_transfer
 * \param no_transaction the number of the transaction
 * \return the account_number_transfer number of the transaction
 * */
gint gsb_transaction_data_get_account_number_transfer ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return -1;

    return transaction -> account_number_transfer;
}


/** set the account_number_transfer
 * \param no_transaction
 * \param account_number_transfer
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_account_number_transfer ( gint no_transaction,
							    gint account_number_transfer )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> account_number_transfer = account_number_transfer;

    return TRUE;
}



/** get the  mother_transaction_number
 * \param no_transaction the number of the transaction
 * \return the mother_transaction_number of the transaction or 0 if the transaction doen't exist
 * */
gint gsb_transaction_data_get_mother_transaction_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no (no_transaction);

    if ( !transaction )
	return 0;

    return transaction -> mother_transaction_number;
}


/** set the mother_transaction_number
 * \param no_transaction
 * \param mother_transaction_number
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_mother_transaction_number ( gint no_transaction,
							      gint mother_transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction);

    if ( !transaction )
	return FALSE;

    transaction -> mother_transaction_number = mother_transaction_number;

    return TRUE;
}



/** create a new transaction and append it to the list in the right account
 * set the transaction number given in param (if no number, give the last number + 1)
 * set the number of the account, the number of the transaction and the currency number
 * which is by default the currency of the account
 * \param no_account the number of the account where the transaction should be made
 * \param transaction_number the number of the transaction
 * \return the number of the new transaction
 * */
gint gsb_transaction_data_new_transaction_with_number ( gint no_account,
							gint transaction_number )
{
    struct_transaction *transaction;

    transaction = calloc ( 1,
			   sizeof ( struct_transaction ));

    if ( !transaction )
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	return FALSE;
    }

    if ( !transaction_number )
	transaction_number = gsb_transaction_data_get_last_number () + 1;

    transaction -> account_number = no_account;
    transaction -> transaction_number = transaction_number;
    transaction -> currency_number = gsb_account_get_currency (no_account);

    transactions_list = g_slist_append ( transactions_list,
					 transaction );

    gsb_transaction_data_save_transaction_pointer (transaction);

    return transaction -> transaction_number;
}


/** create a new transaction with gsb_transaction_data_new_transaction_with_number
 * but set automatickly the last number
 * \param no_account the number of the account where the transaction should be made
 * \return the number of the new transaction
 * */
gint gsb_transaction_data_new_transaction ( gint no_account )
{
    return gsb_transaction_data_new_transaction_with_number ( no_account,
							      gsb_transaction_data_get_last_number () + 1);
}


/** create a new white line, set the number to -1 and set the mother breakdown if it's a child
 * if it's a child breakdown, the account is set as for its mother,
 * if it's the last white line, the account is set to -1
 * that transaction is not append to the transaction's list, the only mean to access it later
 * is by the gtk list of transactions which remains the adresses
 * \param mother_transaction_number the number of the mother's transaction if it's a breakdown child ; 0 if not
 * \return a pointer to the new transaction
 * */
gpointer gsb_transaction_data_new_white_line ( gint mother_transaction_number)
{
    struct_transaction *transaction;

    transaction = calloc ( 1,
			   sizeof ( struct_transaction ));

    if ( !transaction )
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	return NULL;
    }

    /* we fill some things for the child breakdown to help to sort the list */

    transaction -> account_number = gsb_transaction_data_get_account_number (mother_transaction_number);
    transaction -> date = gsb_date_copy ( gsb_transaction_data_get_date (mother_transaction_number));
    transaction -> party_number = gsb_transaction_data_get_party_number (mother_transaction_number);
    transaction -> mother_transaction_number = mother_transaction_number;

    transaction -> transaction_number = -1;

    gsb_transaction_data_save_transaction_pointer (transaction);

    return transaction;
}



/** copy the content of a transaction into the second one
 * the 2 transactions must exist before
 * only the account_number and the transaction_number will be saved in the target transaction
 * all the char are dupplicated, and transaction_id is set to NULL
 * \param source_transaction_number the transaction we want to copy
 * \param target_transaction_number the trnasaction we want to fill with the content of the first one
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_transaction_data_copy_transaction ( gint source_transaction_number,
						 gint target_transaction_number )
{
    struct_transaction *source_transaction;
    struct_transaction *target_transaction;
    gint target_transaction_account_number;

    source_transaction = gsb_transaction_data_get_transaction_by_no ( source_transaction_number);
    target_transaction = gsb_transaction_data_get_transaction_by_no ( target_transaction_number);

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
    target_transaction -> reconcile_number = 0;
    target_transaction -> marked_transaction = 0;

    if ( target_transaction -> notes)
	target_transaction -> notes = g_strdup ( target_transaction -> notes );
    if ( target_transaction -> voucher)
	target_transaction -> voucher = g_strdup ( target_transaction -> voucher );
    if ( target_transaction -> bank_references)
	target_transaction -> bank_references = g_strdup ( target_transaction -> bank_references );
    if ( target_transaction -> method_of_payment_content)
	target_transaction -> method_of_payment_content = g_strdup ( target_transaction -> method_of_payment_content );

    target_transaction -> transaction_id = NULL;

    return TRUE;
}

/** remove the transaction from the transaction's list
 * don't free the transaction
 * \param transaction_number
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_remove_transaction ( gint transaction_number )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( transaction_number);

    if ( !transaction )
	return FALSE;

    transactions_list = g_slist_remove ( transactions_list,
					 transaction );
    return TRUE;
}
