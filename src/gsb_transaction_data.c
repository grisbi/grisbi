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
 * \file data_account.c
 * work with the account structure, no GUI here
 */


#include "include.h"


/*START_INCLUDE*/
#include "gsb_transaction_data.h"
#include "gsb_account.h"
/*END_INCLUDE*/

/*START_STATIC*/
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

/** get the number of the transaction
 * \param transaction a pointer to a transaction
 * \return the number of the transaction
 * */
gint gsb_transaction_data_get_transaction_number ( gpointer transaction_pointer )
{
    struct_transaction *transaction;

    transaction = transaction_pointer;

    if ( !transaction )
	return 0;

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

