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
#include "gsb_account.h"
/*END_INCLUDE*/

/*START_STATIC*/
static struct_transaction *gsb_transaction_data_get_transaction_by_no ( gint no_transaction,
								 gint no_account );
static gboolean gsb_transaction_data_save_transaction_pointer ( gpointer transaction );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



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

    return FALSE;
}


/** find the last number of transaction
 * \param none
 * \return the number
 * */
gint gsb_transaction_data_get_last_number (void)
{
    gint last_number = 0;
    GSList *accounts_list;

    accounts_list = gsb_account_get_list_accounts ();

    while ( accounts_list )
    {
	gint i;
	GSList *transactions_list;

	i = gsb_account_get_no_account ( accounts_list -> data );
	transactions_list = gsb_account_get_transactions_list (i);

	while ( transactions_list )
	{
	    struct_transaction *transaction;

	    transaction = transactions_list -> data;
	    if ( transaction -> transaction_number > last_number )
		last_number = transaction -> transaction_number;

	    transactions_list = transactions_list -> next;
	}
	accounts_list = accounts_list -> next;
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


/** get the number of the transaction
 * \param transaction a pointer to a transaction
 * \param no_transaction the new no
 * \return TRUE if ok
 * */
gboolean gsb_transaction_data_set_transaction_number ( gpointer transaction_pointer,
						       gint no_transaction )
{
    struct_transaction *transaction;

    transaction = transaction_pointer;

    if ( !transaction
	 ||
	 !no_transaction )
	return FALSE;
    
    transaction -> transaction_number = no_transaction;
    
    return TRUE;
}




/** save the pointer in a buffer to increase the speed later
 * \param transaction the pointer to the transaction
 * \return TRUE or FALSE if pb
 * */
gboolean gsb_transaction_data_save_transaction_pointer ( gpointer transaction )
{
    if ( transaction <= 0 )
	return FALSE;

    /* check if the transaction isn't already saved */

    if ( transaction == transaction_buffer[0]
	 ||
	 transaction == transaction_buffer[1] )
	return TRUE;

    current_transaction_buffer = !current_transaction_buffer;
    transaction_buffer[current_transaction_buffer] = transaction;
    return TRUE;
}


/** return the transaction which the number is in the parameter. if no_account is -1 or
 * if the transaction cannot be found in the account given, we look for it in all
 * the accounts
 * the new transaction is stored in the buffer
 * \param no_transaction
 * \param no_account
 * \return a pointer to the transaction, NULL if not found
 * */
struct_transaction *gsb_transaction_data_get_transaction_by_no ( gint no_transaction,
								 gint no_account )
{
    GSList *accounts_list;
    GSList *transactions_list;
    struct_transaction *transaction;

    /* if it's a white line or nothing, go away */

    if ( no_transaction <= 0)
	return NULL;

    /* check first if the transaction is in the buffer */

    if ( transaction_buffer[0]
	 &&
	 transaction_buffer[0] -> transaction_number == no_transaction )
	return transaction_buffer[0];
	
    if ( transaction_buffer[1]
	 &&
	 transaction_buffer[1] -> transaction_number == no_transaction )
	return transaction_buffer[1];
	
    /* if we have an account, we look for the transaction */

    if ( no_account != -1 )
    {
	transactions_list = gsb_account_get_transactions_list ( no_account );

	while ( transactions_list )
	{
	    transaction = transactions_list -> data;

	    if ( transaction -> transaction_number == no_transaction )
	    {
		gsb_transaction_data_save_transaction_pointer ( transaction );
		return transaction;
	    }

	    transactions_list = transactions_list -> next;
	}
    }

    /* if we are here, it's either if no_account = -1
     * either we didn't find the transaction in the given account
     * so, we check all the accounts, except the one given in the parameters */

    accounts_list = gsb_account_get_list_accounts ();

    while ( accounts_list )
    {
	gint i;

	i = gsb_account_get_no_account ( accounts_list -> data );

	if ( i != no_account )
	{
	    transactions_list = gsb_account_get_transactions_list ( i );

	    while ( transactions_list )
	    {
		transaction = transactions_list -> data;

		if ( transaction -> transaction_number == no_transaction )
		{
		    gsb_transaction_data_save_transaction_pointer ( transaction );
		    return transaction;
		}

		transactions_list = transactions_list -> next;
	    }
	}
	accounts_list = accounts_list -> next;
    }

    /* we didn't find any transaction with that number */

    return NULL;
}


/** get the transaction_id
 * \param no_transaction the number of the transaction
 * \param no_account the number of account, may be -1, in that case we will look for the transaction in all accounts
 * \return the id of the transaction
 * */
gchar *gsb_transaction_data_get_transaction_id ( gint no_transaction,
						 gint no_account )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       no_account );

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
						   gint no_account,
						   gchar *transaction_id )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       no_account );

    if ( !transaction )
	return FALSE;

    transaction -> transaction_id = transaction_id;
    
    return TRUE;
}


/** get the account_number
 * \param no_transaction the number of the transaction
 * \return the account of the transaction
 * */
gint gsb_transaction_data_get_account_number ( gint no_transaction )
{
    struct_transaction *transaction;

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       no_account );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

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

    transaction = gsb_transaction_data_get_transaction_by_no ( no_transaction,
							       -1 );

    if ( !transaction )
	return FALSE;

    transaction -> exchange_rate = rate;
    
    return TRUE;
}





