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
#include "account_constants.h"


/*START_INCLUDE*/
#include "data_account.h"
#include "dialog.h"
#include "data_currency.h"
#include "data_form.h"
#include "data_payment.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gpointer gsb_account_find_sort_by_no ( gint sort_number );
static struct_account *gsb_account_get_structure ( gint no );
static gint gsb_account_last_number ( void );
static gint gsb_account_new ( kind_account account_kind );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/** contains a g_slist of struct_account */
GSList *list_struct_accounts;


/** create a new account and add to the list of accounts
 * \param account_type the type of the account
 * \return no of account
 * */
gint gsb_account_new ( kind_account account_kind )
{
    struct_account *account;

    account = calloc ( 1,
		       sizeof ( struct_account ));

    if ( !account )
    {
	dialogue_error_memory ();
	return 0;
    }

    account -> account_number = gsb_account_last_number () + 1;
    account -> account_name = g_strdup_printf ( _("No name %d"),
						account -> account_number );
    account -> currency = gsb_currency_default_currency ();
    account -> update_list = 1;
    account -> current_transaction = GINT_TO_POINTER (-1);
    account -> account_kind = account_kind;
    account -> method_payment_list = gsb_payment_default_payment_list ();
    account -> sort_number = TRANSACTION_LIST_DATE;
    account -> current_sort = gsb_account_find_sort_by_no ( account -> sort_number );
    account -> ascending_sort = GTK_SORT_DESCENDING;
    
    /*     if it's the first account, we put default conf (R not displayed and 3 lines per transaction) */
    /*     else we keep the conf of the last account */
    /*     same for the form organization */

    if ( account -> account_number == 1 )
    {
	account -> nb_rows_by_transaction = 3;
	account -> form_organization = gsb_form_new_organization ();
    }
    else
    {
	account -> show_r = gsb_account_get_r ( gsb_account_last_number ());
	account -> nb_rows_by_transaction = gsb_account_get_nb_rows ( gsb_account_last_number ());
	account -> form_organization = gsb_form_dup_organization ( gsb_account_get_structure (gsb_account_last_number ()) -> form_organization );
    }

    list_struct_accounts = g_slist_append ( list_struct_accounts,
					    account );

    return account -> account_number;
}




/** find and return the last number of account
 * \param none
 * \return last number of account
 * */
gint gsb_account_last_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = list_struct_accounts;
    
    while ( tmp )
    {
	struct_account *account;

	account = tmp -> data;

	if ( account -> account_number > number_tmp )
	    number_tmp = account -> account_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/** find and return the structure of the account asked
 * \param no number of account
 * \return the adr of the struct of the account (NULL if doesn't exit)
 * */
struct_account *gsb_account_get_structure ( gint no )
{
    GSList *tmp;

    tmp = list_struct_accounts;
    
    while ( tmp )
    {
	struct_account *account;

	account = tmp -> data;

	if ( account -> account_number == no )
	    return account;

	tmp = tmp -> next;
    }
    return NULL;
}

/** return the adr of the sort function given by the number
 * \param sort_number the number of sort for the transactions list
 * \return the function
 * */
gpointer gsb_account_find_sort_by_no ( gint sort_number )
{
    printf ( "faire le gsb_account_find_sort_by_no\n" );
    /*     FIXME */

    return NULL;
}


/** get the nb of rows displayed on the account given
 * \param no_account no of the account
 * \return nb of rows displayed or 0 if the account doesn't exist
 * */
gint gsb_account_get_nb_rows ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> nb_rows_by_transaction;
}


/** set the nb of rows displayed in the account given
 * \param no_account no of the account
 * \param nb_rows number of rows per transaction (1, 2, 3 or 4)
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_nb_rows ( gint no_account,
				   gint nb_rows )
{
    struct_account *account;

    if ( nb_rows < 1
	 ||
	 nb_rows > 4 )
    {
	printf ( _("Bad nb rows to gsb_account_set_nb_rows () in data_account.c\n" ));
	return FALSE;
    }

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> nb_rows_by_transaction = nb_rows;

    return TRUE;
}


/** return if R are displayed in the account asked
 * \param no_account no of the account
 * \return boolean show/not show R
 * */
gboolean gsb_account_get_r ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> show_r;
}

/** set if R are displayed in the account asked
 * \param no_account no of the account
 * \param show_r boolean
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_r ( gint no_account,
			     gboolean show_r )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> show_r = show_r;
    return TRUE;
}


/** get the id of the account
 * \param no_account no of the account
 * \return id or 0 if the account doesn't exist
 * */
gchar *gsb_account_get_id ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> account_id;
}


/** set the id of the account
 * \param no_account no of the account
 * \param id id to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_id ( gint no_account,
			      gchar *id )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> account_id = id;

    return TRUE;
}



/** get the account kind of the account
 * \param no_account no of the account
 * \return account type or 0 if the account doesn't exist
 * */
kind_account gsb_account_get_kind ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> account_kind;
}


/** set the kind of the account
 * \param no_account no of the account
 * \param account_kind type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_kind ( gint no_account,
				kind_account account_kind )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> account_kind = account_kind;

    return TRUE;
}



/** get the name of the account
 * \param no_account no of the account
 * \return name or NULL if the account doesn't exist
 * */
gchar *gsb_account_get_name ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> account_name;
}


/** set the name of the account
 * \param no_account no of the account
 * \param name name to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_name ( gint no_account,
				gchar *name )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> account_name = name;

    return TRUE;
}




/** get the init balance of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_account_get_init_balance ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> init_balance;
}


/** set the init balance of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_init_balance ( gint no_account,
					gdouble balance )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> init_balance = balance;

    return TRUE;
}



/** get the minimum balance wanted of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_account_get_mini_balance_wanted ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> mini_balance_wanted;
}


/** set the minimum balance wanted of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_mini_balance_wanted ( gint no_account,
					       gdouble balance )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> mini_balance_wanted = balance;

    return TRUE;
}

/** get the minimum balance authorized of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_account_get_mini_balance_authorized ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> mini_balance_authorized;
}


/** set the minimum balance authorized of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_mini_balance_authorized ( gint no_account,
						   gdouble balance )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> mini_balance_authorized = balance;

    return TRUE;
}



/** get the current balance  of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_account_get_current_balance ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> current_balance;
}


/** set the current balance  of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_current_balance ( gint no_account,
					   gdouble balance )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> current_balance = balance;

    return TRUE;
}



/** get the marked balance  of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_account_get_marked_balance ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> marked_balance;
}


/** set the marked balance  of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_marked_balance ( gint no_account,
					  gdouble balance )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> marked_balance = balance;

    return TRUE;
}


/** get the tree of the account
 * \param no_account no of the account
 * \return tree or NULL if the account doesn't exist
 * */
gpointer gsb_account_get_tree_view ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> transactions_tree_view;
}


/** set the tree of the account
 * \param no_account no of the account
 * \param tree tree to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_tree_view ( gint no_account,
				     gpointer tree )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> transactions_tree_view = tree;

    return TRUE;
}


/** get the scrolled_window of the account
 * \param no_account no of the account
 * \return tree or NULL if the account doesn't exist
 * */
gpointer gsb_account_get_scrolled_window ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> transactions_scrolled_window;
}


/** set the scrolled_window of the account
 * \param no_account no of the account
 * \param scrolled_window scrolled_window to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_scrolled_window ( gint no_account,
					   gpointer scrolled_window )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> transactions_scrolled_window = scrolled_window;

    return TRUE;
}



/** get the store of the account
 * \param no_account no of the account
 * \return  or NULL if the account doesn't exist
 * */
gpointer gsb_account_get_store ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> transactions_store;
}


/** set the store of the account
 * \param no_account no of the account
 * \param store  store to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_store ( gint no_account,
				 gpointer store )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> transactions_store = store;

    return TRUE;
}



/** get the adjustment_value of the account
 * \param no_account no of the account
 * \return value or NULL if the account doesn't exist
 * */
gdouble gsb_account_get_adjustment_value ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> transactions_adjustment_value;
}


/** set the adjustment_value balance  of the account
 * \param no_account no of the account
 * \param value value to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_adjustment_value ( gint no_account,
					    gdouble value )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> transactions_adjustment_value = value;

    return TRUE;
}

/** get the column of the account
 * \param no_account no of the account
 * \param no_column no of the column
 * \return  or NULL if the account doesn't exist
 * */
gpointer gsb_account_get_column ( gint no_account,
				  gint no_column )
{
    struct_account *account;

    if ( no_column < 0
	 ||
	 no_column > TRANSACTION_LIST_COL_NB )
    {
	g_strdup_printf ( _("Bad no column to gsb_account_get_column () in data_account.c\nno_column = %d\n" ),
			  no_column );
	return FALSE;
    }

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> transactions_column[no_column];
}


/** set the column of the account
 * \param no_account no of the account
 * \param no_column no of the column
 * \param column  column to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_column ( gint no_account,
				  gint no_column,
				  gpointer column )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if ( no_column < 0
	 ||
	 no_column > TRANSACTION_LIST_COL_NB )
    {
	g_strdup_printf ( _("Bad no column to gsb_account_set_column () in data_account.c\nno_column = %d\n" ),
			  no_column );
	return FALSE;
    }

    if (!account )
	return FALSE;

    account -> transactions_column[no_column] = column;

    return TRUE;
}



/** get the transactions list of the account
 * \param no_account no of the account
 * \return the g_slist or NULL if the account doesn't exist
 * */
GSList *gsb_account_get_transactions_list ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> transactions_list;
}


/** set the transactions list of the account
 * \param no_account no of the account
 * \param list g_slist to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_transactions_list ( gint no_account,
					     GSList *list )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> transactions_list = list;

    return TRUE;
}



/** get the current transaction of the account
 * \param no_account no of the account
 * \return pointer to the transaction or NULL if the account doesn't exist
 * */
gpointer gsb_account_get_current_transaction ( gint no_account )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> current_transaction;
}


/** set the current transaction of the account
 * \param no_account no of the account
 * \param transaction  store to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_current_transaction ( gint no_account,
					       gpointer transaction )
{
    struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> current_transaction = transaction;

    return TRUE;
}



