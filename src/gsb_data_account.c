/* ************************************************************************** */
/* work with the struct of accounts                                           */
/*                                                                            */
/*                                  gsb_data_account                          */
/*                                                                            */
/*     Copyright (C)	2000-2005 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2005 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_data_account.c
 * work with the account structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_account.h"
#include "dialog.h"
#include "data_currency.h"
#include "gsb_data_form.h"
#include "data_payment.h"
#include "utils_str.h"
#include "gsb_transactions_list.h"
#include "include.h"
/*END_INCLUDE*/

/** \struct
 * describe an account
 * */

typedef struct
{
    /** @name general stuff */
    gint account_number;
    gchar *account_id;                       /**< for ofx import, invisible for the user */
    kind_account account_kind;
    gchar *account_name;
    gint currency;
    gint closed_account;                     /**< if 1 => closed */
    gchar *comment;
    gchar *holder_name;
    gchar *holder_address;

    /** @name method of payment */
    GSList *method_payment_list;         /**< list of method of payment struct */
    gint default_debit;
    gint default_credit;

    /** @name showed list stuff */
    gint show_r;                      /**< 1 : reconciled transactions are showed */
    gint nb_rows_by_transaction;      /**< 1, 2, 3, 4  */
    gint update_list;                /**< 1 when the list need to be updated when showed */

    /** @name remaining of the balances */
    gdouble init_balance;
    gdouble mini_balance_wanted;
    gdouble mini_balance_authorized;
    gdouble current_balance;
    gdouble marked_balance;

    /** @name remaining of the minimun balance message */
    gint mini_balance_wanted_message;
    gint mini_balance_authorized_message;

    /** @name number of the transaction selectionned, or -1 for the white line */
    gint current_transaction_number;

    /** @name bank stuff */
    gint bank_number;
    gchar *bank_branch_code;
    gchar *bank_account_number;
    gchar *bank_account_key;

    /** @name reconcile stuff */
    GDate *reconcile_date;
    gdouble reconcile_balance;
    gint reconcile_last_number;

    /** @name reconcile sort */
    gint reconcile_sort_type;                           /**< 1 : sort by method of payment ; 0 : sort by date */
    GSList *sort_list;                        /**< the method of payment sorted in a list */
    gint split_neutral_payment;               /**< if 1 : neutral payments are splitted into debits/credits */

    /** @name tree_view sort stuff */
    gint sort_type;          /**< GTK_SORT_DESCENDING / GTK_SORT_ASCENDING */
    gint sort_column;             /**< used to hide the arrow when change the column */
    gint transactions_column_sort[TRANSACTION_LIST_COL_NB];  /**< contains the default sort type for each column */

    /** @name current graphic position in the list */

    gdouble vertical_adjustment_value;

    /** @name struct of the form's organization */
    gpointer form_organization;
} struct_account;


/*START_STATIC*/
static struct_account *gsb_data_account_get_structure ( gint no );
static gboolean gsb_data_account_get_update_list ( gint no_account );
static gint gsb_data_account_max_number ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern     gchar * buffer ;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
/*END_EXTERN*/



/** contains a g_slist of struct_account */
static GSList *list_accounts;

/** a pointer to the last account used (to increase the speed) */
static struct_account *account_buffer;

/**
 * set the accounts global variables to NULL, usually when we init all the global variables
 * 
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_account_init_variables ( void )
{
    account_buffer = NULL;
    list_accounts = NULL;

    return FALSE;
}

/**
 * return a pointer on the g_slist of accounts
 * carrefull : it's not a copy, so we must not free or change it
 * if we want to change the list, use gsb_data_account_get_copy_list_accounts instead
 * 
 * \param none
 * \return a g_slist on the accounts
 * */
GSList *gsb_data_account_get_list_accounts ( void )
{
    return list_accounts;
}


/**
 * create a new account and add to the list of accounts
 * 
 * \param account_type the type of the account
 * 
 * \return no of account, -1 if problem
 * */
gint gsb_data_account_new ( kind_account account_kind )
{
    struct_account *account;

    account = calloc ( 1,
		       sizeof ( struct_account ));

    if ( !account )
    {
	dialogue_error_memory ();
	return -1;
    }

    /* we have to append the account first because some functions later will
     * look for that account */

    list_accounts = g_slist_append ( list_accounts,
				     account );

    account -> account_number = gsb_data_account_max_number () + 1;
    account -> account_name = g_strdup_printf ( _("No name %d"),
						account -> account_number );
    account -> currency = gsb_currency_default_currency ();
    account -> update_list = 1;
    account -> account_kind = account_kind;
    account -> method_payment_list = gsb_payment_default_payment_list ();
    account -> sort_type = GTK_SORT_DESCENDING;
    account -> current_transaction_number = -1;
    account -> vertical_adjustment_value = (gdouble) -1;
    
    /*     if it's the first account, we set default conf (R not displayed and 3 lines per transaction) */
    /*     else we keep the conf of the last account */
    /*     same for the form organization */

    if ( account -> account_number == 1 )
    {
	account -> nb_rows_by_transaction = 3;
	gsb_data_form_new_organization (account -> account_number);
	gsb_data_form_set_default_organization (account -> account_number);
    }
    else
    {
	account -> show_r = gsb_data_account_get_r ( gsb_data_account_max_number ());
	account -> nb_rows_by_transaction = gsb_data_account_get_nb_rows ( gsb_data_account_max_number ());

	/* try to copy the form of the last account, else make a new form */
	if ( !gsb_data_form_dup_organization ( gsb_data_account_max_number () - 1,
					       account -> account_number ))
	{
	    gsb_data_form_new_organization (account -> account_number);
	    gsb_data_form_set_default_organization (account -> account_number);
	}
    }

    return account -> account_number;
}



/**
 * delete and free the account given
 * 
 * \param no_account the no of account
 * 
 * \return TRUE if ok
 * */
gboolean gsb_data_account_delete ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    g_slist_free ( account -> method_payment_list );
/* FIXME : faire une fonction pour mieux faire ça quand la struct liste opé sera faite */
    g_slist_free ( account -> sort_list );
    
    list_accounts = g_slist_remove ( list_accounts,
				     account );

    /* remove the budget from the buffers */

    if ( account_buffer == account )
	account_buffer = NULL;
    free ( account );

    return TRUE;
}



/** return the amount of accounts
 * \param none
 * \return amount of accounts
 * */
gint gsb_data_account_get_accounts_amount ( void )
{
    if ( !list_accounts )
	return 0;

    return g_slist_length ( list_accounts );
}



/** find and return the last number of account
 * \param none
 * \return last number of account
 * */
gint gsb_data_account_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = list_accounts;
    
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


/** find and return the first number of account
 * \param none
 * \return first number of account, -1 if no accounts
 * */
gint gsb_data_account_first_number ( void )
{
    struct_account *account;

    if ( !list_accounts )
	return -1;

    account = list_accounts -> data;

    return  account -> account_number;
}





/**
 * find and return the number of the account which the struct is the param 
 * 
 * \param the struct of the account
 * 
 * \return the number of account, -1 if pb
 * */
gint gsb_data_account_get_no_account ( gpointer account_ptr )
{
    struct_account *account;

    if ( !account_ptr )
	return -1;

    account = account_ptr;
    account_buffer = account;

    return  account -> account_number;
}


/** change the number of the account given in param
 * it returns the new number (given in param also)
 * it is called ONLY when loading a file to change the default
 * number, given when we create the account
 * \param no_account no of the account to change
 * \param new_no new number to the account
 * \return the new number, or -1 if failed
 * */
gint gsb_data_account_set_account_number ( gint no_account,
					   gint new_no )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return -1;

    account -> account_number = new_no;

    return new_no;
}




/** 
 * find and return the structure of the account asked
 * 
 * \param no number of account
 * 
 * \return the adr of the struct of the account (NULL if doesn't exit)
 * */
struct_account *gsb_data_account_get_structure ( gint no )
{
    GSList *tmp;

    /* before checking all the accounts, we check the buffer */

    if ( account_buffer
	 &&
	 account_buffer -> account_number == no )
	return account_buffer;

    tmp = list_accounts;
    
    while ( tmp )
    {
	struct_account *account;

	account = tmp -> data;

	if ( account -> account_number == no )
	{
	    account_buffer = account;
	    return account;
	}

	tmp = tmp -> next;
    }
    return NULL;
}



/** get the nb of rows displayed on the account given
 * \param no_account no of the account
 * \return nb of rows displayed or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_nb_rows ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> nb_rows_by_transaction;
}


/** set the nb of rows displayed in the account given
 * \param no_account no of the account
 * \param nb_rows number of rows per transaction (1, 2, 3 or 4)
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_nb_rows ( gint no_account,
					gint nb_rows )
{
    struct_account *account;

    if ( nb_rows < 1
	 ||
	 nb_rows > 4 )
    {
	printf ( _("Bad nb rows to gsb_data_account_set_nb_rows () in data_account.c\n" ));
	return FALSE;
    }

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> nb_rows_by_transaction = nb_rows;

    return TRUE;
}


/** return if R are displayed in the account asked
 * \param no_account no of the account
 * \return boolean show/not show R
 * */
gboolean gsb_data_account_get_r ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> show_r;
}

/** set if R are displayed in the account asked
 * \param no_account no of the account
 * \param show_r boolean
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_r ( gint no_account,
				  gboolean show_r )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> show_r = show_r;
    return TRUE;
}


/** get the id of the account
 * \param no_account no of the account
 * \return id or 0 if the account doesn't exist
 * */
gchar *gsb_data_account_get_id ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> account_id;
}


/** 
 * set the id of the account
 * the id is copied in memory
 * 
 * \param no_account no of the account
 * \param id id to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_id ( gint no_account,
				   const gchar *id )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> account_id = my_strdup (id);

    return TRUE;
}



/** get the account kind of the account
 * \param no_account no of the account
 * \return account type or 0 if the account doesn't exist
 * */
kind_account gsb_data_account_get_kind ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> account_kind;
}


/** set the kind of the account
 * \param no_account no of the account
 * \param account_kind type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_kind ( gint no_account,
				kind_account account_kind )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> account_kind = account_kind;

    return TRUE;
}



/** get the name of the account
 * \param no_account no of the account
 * \return name or NULL if the account doesn't exist
 * */
gchar *gsb_data_account_get_name ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> account_name;
}


/** 
 * set the name of the account
 * the name is copied in memory
 * 
 * \param no_account no of the account
 * \param name name to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_name ( gint no_account,
				     const gchar *name )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> account_name = my_strdup (name);

    return TRUE;
}


/** find and return the number of account which
 * have the name given in param
 * \param account_name
 * \return the number of account or -1
 * */
gint gsb_data_account_get_no_account_by_name ( gchar *account_name )
{
    GSList *list_tmp;

    if ( !account_name )
	return -1;

    list_tmp = list_accounts;

    while ( list_tmp )
    {
	struct_account *account;

	account = list_tmp -> data;
	
	if ( !strcmp ( account -> account_name,
		       account_name ))
	    return account -> account_number;

	list_tmp = list_tmp -> next;
    }

    return -1;
}



/** get the init balance of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_data_account_get_init_balance ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> init_balance;
}


/** set the init balance of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_init_balance ( gint no_account,
					gdouble balance )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> init_balance = balance;

    return TRUE;
}



/** get the minimum balance wanted of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_data_account_get_mini_balance_wanted ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> mini_balance_wanted;
}


/** set the minimum balance wanted of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_mini_balance_wanted ( gint no_account,
					       gdouble balance )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> mini_balance_wanted = balance;

    return TRUE;
}

/** get the minimum balance authorized of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_data_account_get_mini_balance_authorized ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> mini_balance_authorized;
}


/** set the minimum balance authorized of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_mini_balance_authorized ( gint no_account,
						   gdouble balance )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> mini_balance_authorized = balance;

    return TRUE;
}



/** get the current balance  of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_data_account_get_current_balance ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> current_balance;
}


/** set the current balance  of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_current_balance ( gint no_account,
					   gdouble balance )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> current_balance = balance;

    return TRUE;
}



/** get the marked balance  of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_data_account_get_marked_balance ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> marked_balance;
}


/** set the marked balance  of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_marked_balance ( gint no_account,
					  gdouble balance )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> marked_balance = balance;

    return TRUE;
}



/** get the column_sort of the account
 * \param no_account no of the account
 * \param no_column no of the column
 * \return  or NULL if the account doesn't exist
 * */
gint gsb_data_account_get_column_sort ( gint no_account,
				   gint no_column )
{
    struct_account *account;

    if ( no_column < 0
	 ||
	 no_column > TRANSACTION_LIST_COL_NB )
    {
	g_strdup_printf ( _("Bad no column to gsb_data_account_get_column_sort () in data_account.c\nno_column = %d\n" ),
			  no_column );
	return FALSE;
    }

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> transactions_column_sort[no_column];
}


/** set the column_sort of the account
 * \param no_account no of the account
 * \param no_column no of the column
 * \param column_sort  column_sort to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_column_sort ( gint no_account,
				       gint no_column,
				       gint column_sort )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if ( no_column < 0
	 ||
	 no_column > TRANSACTION_LIST_COL_NB )
    {
	g_strdup_printf ( _("Bad no column to gsb_data_account_set_column_sort () in data_account.c\nno_column = %d\n" ),
			  no_column );
	return FALSE;
    }

    if (!account )
	return FALSE;

    account -> transactions_column_sort[no_column] = column_sort;

    return TRUE;
}




/**
 * get the number of the current transaction in the given account
 *
 * \param no_account
 *
 * \return the number of the transaction or 0 if problem
 * */
gint gsb_data_account_get_current_transaction_number ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> current_transaction_number;
}



/** set the current transaction of the account
 * \param no_account no of the account
 * \param transaction_number number of the transaction selection
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_current_transaction_number ( gint no_account,
						      gint transaction_number )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> current_transaction_number = transaction_number;

    return TRUE;
}



/** get the value of mini_balance_wanted_message  on the account given
 * \param no_account no of the account
 * \return mini_balance_wanted_message or 0 if the account doesn't exist
 * */
gboolean gsb_data_account_get_mini_balance_wanted_message ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> mini_balance_wanted_message;
}


/** set the value of mini_balance_wanted_message in the account given
 * \param no_account no of the account
 * \param value 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_mini_balance_wanted_message ( gint no_account,
						       gboolean value )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> mini_balance_wanted_message = value;

    return TRUE;
}


/** get the value of mini_balance_authorized_message  on the account given
 * \param no_account no of the account
 * \return mini_balance_authorized_message or 0 if the account doesn't exist
 * */
gboolean gsb_data_account_get_mini_balance_authorized_message ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> mini_balance_authorized_message;
}


/**
 * set the value of mini_balance_authorized_message in the account given
 * 
 * \param no_account no of the account
 * \param value 
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_mini_balance_authorized_message ( gint no_account,
								gboolean value )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> mini_balance_authorized_message = value;

    return TRUE;
}



/**
 * get the reconcile_date of the account
 * 
 * \param no_account no of the account
 * 
 * \return a GDate of the reconcile date or NULL if the account doesn't exist
 * */
GDate *gsb_data_account_get_current_reconcile_date ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> reconcile_date;
}


/** set the reconcile_date of the account
 * \param no_account no of the account
 * \param date date to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_current_reconcile_date ( gint no_account,
						  GDate *date )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> reconcile_date = date;

    return TRUE;
}



/** get the reconcile balance of the account
 * \param no_account no of the account
 * \return balance or NULL if the account doesn't exist
 * */
gdouble gsb_data_account_get_reconcile_balance ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> reconcile_balance;
}


/** set the reconcile balance of the account
 * \param no_account no of the account
 * \param balance balance to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_reconcile_balance ( gint no_account,
					     gdouble balance )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> reconcile_balance = balance;

    return TRUE;
}



/** get the reconcile_last_number on the account given
 * \param no_account no of the account
 * \return last number of reconcile or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_reconcile_last_number ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> reconcile_last_number;
}


/** set the reconcile_last_number in the account given
 * \param no_account no of the account
 * \param number last number of reconcile
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_reconcile_last_number ( gint no_account,
						 gint number )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> reconcile_last_number = number;

    return TRUE;
}



/** get the update_list on the account given
 * \param no_account no of the account
 * \return update_list or 0 if the account doesn't exist
 * */
gboolean gsb_data_account_get_update_list ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> update_list;
}


/** set the update_list in the account given
 * \param no_account no of the account
 * \param value 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_update_list ( gint no_account,
				       gboolean value )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> update_list = value;

    return TRUE;
}


/** get the currency on the account given
 * \param no_account no of the account
 * \return last number of reconcile or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_currency ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> currency;
}


/** set the currency in the account given
 * \param no_account no of the account
 * \param currency the currency to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_currency ( gint no_account,
				    gint currency )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> currency = currency;

    return TRUE;
}


/** get the bank on the account given
 * \param no_account no of the account
 * \return last number of reconcile or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_bank ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> bank_number;
}


/** set the bank in the account given
 * \param no_account no of the account
 * \param bank the bank to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_bank ( gint no_account,
				gint bank )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> bank_number = bank;

    return TRUE;
}


/** get the bank_branch_code of the account
 * \param no_account no of the account
 * \return id or NULL if the account doesn't exist
 * */
gchar *gsb_data_account_get_bank_branch_code ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> bank_branch_code;
}


/** 
 * set the bank_branch_code of the account
 * the code is copied in memory
 * 
 * \param no_account no of the account
 * \param bank_branch_code bank_branch_code to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_bank_branch_code ( gint no_account,
						 const gchar *bank_branch_code )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> bank_branch_code = my_strdup (bank_branch_code);

    return TRUE;
}


/** get the bank_account_number of the account
 * \param no_account no of the account
 * \return id or NULL if the account doesn't exist
 * */
gchar *gsb_data_account_get_bank_account_number ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> bank_account_number;
}


/**
 * set the bank_account_number of the account
 * the number is copied in memory
 * 
 * \param no_account no of the account
 * \param bank_account_number bank_account_number to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_bank_account_number ( gint no_account,
						    const gchar *bank_account_number )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> bank_account_number = my_strdup (bank_account_number);

    return TRUE;
}



/** get the bank_account_key of the account
 * \param no_account no of the account
 * \return id or NULL if the account doesn't exist
 * */
gchar *gsb_data_account_get_bank_account_key ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> bank_account_key;
}


/** 
 * set the bank_account_key of the account
 * the key is copied in memory
 * 
 * \param no_account no of the account
 * \param bank_account_key bank_account_key to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_bank_account_key ( gint no_account,
						 const gchar *bank_account_key )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> bank_account_key = my_strdup (bank_account_key);

    return TRUE;
}


/** get closed_account on the account given
 * \param no_account no of the account
 * \return true if account is closed
 * */
gint gsb_data_account_get_closed_account ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> closed_account;
}


/** set closed_account in the account given
 * \param no_account no of the account
 * \param closed_account closed_account to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_closed_account ( gint no_account,
					  gint closed_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> closed_account = closed_account;

    return TRUE;
}


/** get the comment of the account
 * \param no_account no of the account
 * \return comment or NULL if the account doesn't exist
 * */
gchar *gsb_data_account_get_comment ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> comment;
}


/**
 * set the comment of the account
 * the comment is copied in memory
 * 
 * \param no_account no of the account
 * \param comment comment to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_comment ( gint no_account,
					const gchar *comment )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> comment = my_strdup (comment);

    return TRUE;
}



/** get reconcile_sort_type on the account given
 * \param no_account no of the account
 * \return sort_type or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_reconcile_sort_type ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> reconcile_sort_type;
}


/** set reconcile_sort_type in the account given
 * \param no_account no of the account
 * \param sort_type sort_type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_reconcile_sort_type ( gint no_account,
					       gint sort_type )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> reconcile_sort_type = sort_type;

    return TRUE;
}


/** get the sort_list of the account
 * \param no_account no of the account
 * \return the g_slist or NULL if the account doesn't exist
 * */
GSList *gsb_data_account_get_sort_list ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> sort_list;
}


/** set the sort_list list of the account
 * \param no_account no of the account
 * \param list g_slist to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_sort_list ( gint no_account,
				     GSList *list )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> sort_list = list;

    return TRUE;
}


/** get split_neutral_payment on the account given
 * \param no_account no of the account
 * \return split_neutral_payment or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_split_neutral_payment ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> split_neutral_payment;
}


/** set split_neutral_payment in the account given
 * \param no_account no of the account
 * \param split_neutral_payment split_neutral_payment to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_split_neutral_payment ( gint no_account,
						 gint split_neutral_payment )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> split_neutral_payment = split_neutral_payment;

    return TRUE;
}


/** get the holder_name of the account
 * \param no_account no of the account
 * \return holder_name or NULL if the account doesn't exist
 * */
gchar *gsb_data_account_get_holder_name ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> holder_name;
}


/**
 * set the holder_name of the account
 * the name is copied in memory
 * 
 * \param no_account no of the account
 * \param holder_name holder_name to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_holder_name ( gint no_account,
					    const gchar *holder_name )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> holder_name = my_strdup (holder_name);

    return TRUE;
}


/** get the holder_address of the account
 * \param no_account no of the account
 * \return holder_address or NULL if the account doesn't exist
 * */
gchar *gsb_data_account_get_holder_address ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> holder_address;
}


/**
 * set the holder_address of the account
 * the address is copied in memory
 * 
 * \param no_account no of the account
 * \param holder_address holder_address to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_holder_address ( gint no_account,
					       const gchar *holder_address )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> holder_address = my_strdup (holder_address);

    return TRUE;
}


/** get the method_payment_list  of the account
 * \param no_account no of the account
 * \return the g_slist or NULL if the account doesn't exist
 * */
GSList *gsb_data_account_get_method_payment_list ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> method_payment_list;
}


/** set the method_payment_list of the account
 * \param no_account no of the account
 * \param list g_slist to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_method_payment_list ( gint no_account,
					       GSList *list )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> method_payment_list = list;

    return TRUE;
}


/** get default_debit on the account given
 * \param no_account no of the account
 * \return default_debit or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_default_debit ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> default_debit;
}


/** set default_debit in the account given
 * \param no_account no of the account
 * \param default_debit default_debit to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_default_debit ( gint no_account,
					 gint default_debit )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> default_debit = default_debit;

    return TRUE;
}



/** get default_credit on the account given
 * \param no_account no of the account
 * \return default_credit or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_default_credit ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> default_credit;
}


/** set default_credit in the account given
 * \param no_account no of the account
 * \param default_credit default_credit to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_default_credit ( gint no_account,
					  gint default_credit )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> default_credit = default_credit;

    return TRUE;
}



/** get vertical_adjustment_value on the account given
 * \param no_account no of the account
 * \return vertical_adjustment_value or 0 if the account doesn't exist
 * */
gdouble gsb_data_account_get_vertical_adjustment_value ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> vertical_adjustment_value;
}


/** set vertical_adjustment_value in the account given
 * \param no_account no of the account
 * \param vertical_adjustment_value vertical_adjustment_value to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_vertical_adjustment_value ( gint no_account,
						     gint vertical_adjustment_value )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> vertical_adjustment_value = vertical_adjustment_value;

    return TRUE;
}


/** get sort_type on the account given
 * \param no_account no of the account
 * \return sort_type or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_sort_type ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> sort_type;
}


/** set sort_type in the account given
 * \param no_account no of the account
 * \param sort_type sort_type to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_sort_type ( gint no_account,
					  gint sort_type )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> sort_type = sort_type;

    return TRUE;
}




/** get sort_column on the account given
 * \param no_account no of the account
 * \return sort_column or 0 if the account doesn't exist
 * */
gint gsb_data_account_get_sort_column ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return 0;

    return account -> sort_column;
}


/** set sort_column in the account given
 * \param no_account no of the account
 * \param sort_column sort_column to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_sort_column ( gint no_account,
				       gint sort_column )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> sort_column = sort_column;

    return TRUE;
}



/**
 * get the form_organization of the account
 * 
 * \param no_account no of the account
 * 
 * \return form_organization or NULL if the account doesn't exist
 * */
gpointer gsb_data_account_get_form_organization ( gint no_account )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> form_organization;
}


/**
 * set the form_organization of the account
 * 
 * \param no_account no of the account
 * \param form_organization form_organization to set
 * 
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_data_account_set_form_organization ( gint no_account,
						  gpointer form_organization )
{
    struct_account *account;

    account = gsb_data_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> form_organization = form_organization;

    return TRUE;
}


/** set a new order in the list of accounts
 * all the accounts which are not in the new order are appened at the end of the new list
 * \param new_order a g_slist which contains the number of accounts in the new order
 * \return FALSE
 * */
gboolean gsb_data_account_reorder ( GSList *new_order )
{
    GSList *last_list;
    GSList *new_list_accounts;
    GSList *list_tmp;

    new_list_accounts = NULL;

    while ( new_order )
    {
	new_list_accounts = g_slist_append ( new_list_accounts,
					     gsb_data_account_get_structure ( GPOINTER_TO_INT ( new_order -> data )));
	new_order = new_order -> next;
    }
					     
    last_list = list_accounts;
    list_accounts = new_list_accounts;
    
    /* now we go to check if all accounts are in the list and
     * append the at the end */

    list_tmp = last_list;

    while ( list_tmp )
    {
	struct_account *account;
	struct_account *check_account;

	account = list_tmp -> data;
	
	check_account = gsb_data_account_get_structure ( account -> account_number );

	if ( !check_account )
	    list_accounts = g_slist_append ( list_accounts,
					     account );

	list_tmp = list_tmp -> next;
    }

    g_slist_free (last_list);
    return TRUE;
}


