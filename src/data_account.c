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
static struct struct_account *gsb_account_get_structure ( gint no );
static gint gsb_account_last_number ( void );
static gint gsb_account_new ( kind_account account_kind );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *tree_view;
/*END_EXTERN*/


/** \struct
 * describe an account
 * */

struct struct_account
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
    GtkWidget *account_button;        /**< the button in the list of accounts on the transactions page */
    gint update_list;                /**< 1 when the list need to be updated when showed */

    /** @name remaining of the balances */
    gdouble init_balance;
    gdouble mini_balance_wanted;
    gdouble mini_balance_authorized;
    gdouble current_balance;
    gdouble marked_balance;

    /** @name transactions list : contains a list of transactions structures */
    GSList *transactions_list;

    /** @name pointer of the transaction selectionned, or -1 for the white line */
    struct structure_operation *current_transaction;

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
    gint sort_type;                           /**< 1 : sort by method of payment ; 0 : sort by date */
    GSList *sort_list;                        /**< the method of payment sorted in a list */
    gint split_neutral_payment;               /**< if 1 : neutral payments are splitted into debits/credits */

    /** @name tree_view stuff */
    GtkWidget *transactions_tree_view;
    GtkWidget *transactions_scrolled_window;
    GtkListStore *transactions_store;
    gdouble transactions_adjustment_value; 
    GtkTreeViewColumn *transactions_column[TRANSACTION_LIST_COL_NB];

    /** @name tree_view sort stuff */
    gint (*current_sort) ( struct structure_operation *operation_1,
				 struct structure_operation *operation_2 );      /**< current sort function */
    gint ascending_sort;          /**< GTK_SORT_DESCENDING / GTK_SORT_ASCENDING */
    gint sort_number;             /**< number of sort (TRANSACTION_LIST_...)*/
    gint sort_column;             /**< used to hide the arrow when change the column */

    /** @name stuff for filling the transactions list */
    GSList *last_transaction;              /**< the slist on the last transaction added or -1 if finished */
    gint finished_background_color;        /**< 1 when the background color is finish */
    gint finished_balance_showed;          /**< 1 when the transactions balance is written */
    gint finished_selection_transaction;   /**< 1 once the transaction is selectionned */

    /** @name struct of the form's organization */
    struct organisation_formulaire *form_organization;
};


/** contains a g_slist of struct_account */
GSList *list_struct_accounts;


/** create a new account and add to the list of accounts
 * \param account_type the type of the account
 * \return no of account
 * */
gint gsb_account_new ( kind_account account_kind )
{
    struct struct_account *account;

    account = calloc ( 1,
		       sizeof ( struct struct_account ));

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
	struct struct_account *account;

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
struct struct_account *gsb_account_get_structure ( gint no )
{
    GSList *tmp;

    tmp = list_struct_accounts;
    
    while ( tmp )
    {
	struct struct_account *account;

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
    struct struct_account *account;

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
    struct struct_account *account;

    if ( nb_rows < 1
	 ||
	 nb_rows > 4 )
	return FALSE;

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
    struct struct_account *account;

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
    struct struct_account *account;

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
    struct struct_account *account;

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
    struct struct_account *account;

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
    struct struct_account *account;

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
    struct struct_account *account;

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
    struct struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return NULL;

    return account -> account_name;
}


/** set the name of the account
 * \param no_account no of the account
 * \param name ,aùe to set
 * \return TRUE, ok ; FALSE, problem
 * */
gboolean gsb_account_set_name ( gint no_account,
				gchar *name )
{
    struct struct_account *account;

    account = gsb_account_get_structure ( no_account );

    if (!account )
	return FALSE;

    account -> account_name = name;

    return TRUE;
}



