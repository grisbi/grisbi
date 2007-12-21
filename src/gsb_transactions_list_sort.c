/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
/*			http://www.grisbi.org   			      */
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


#include "include.h"


/*START_INCLUDE*/
#include "gsb_transactions_list_sort.h"
#include "./gsb_data_account.h"
#include "./gsb_data_archive.h"
#include "./gsb_data_budget.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_reconcile.h"
#include "./gsb_data_transaction.h"
#include "./navigation.h"
#include "./gsb_real.h"
#include "./utils_str.h"
#include "./gsb_transactions_list.h"
#include "./gsb_transactions_list.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint gsb_transactions_list_sort_by_amount ( GtkTreeModel *model,
					    GtkTreeIter *iter_1,
					    GtkTreeIter *iter_2,
					    GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_bank ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_budget ( GtkTreeModel *model,
					    GtkTreeIter *iter_1,
					    GtkTreeIter *iter_2,
					    GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_category ( GtkTreeModel *model,
					      GtkTreeIter *iter_1,
					      GtkTreeIter *iter_2,
					      GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_chq ( GtkTreeModel *model,
					 GtkTreeIter *iter_1,
					 GtkTreeIter *iter_2,
					 GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_credit ( GtkTreeModel *model,
					    GtkTreeIter *iter_1,
					    GtkTreeIter *iter_2,
					    GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_date ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_debit ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_financial_year ( GtkTreeModel *model,
						    GtkTreeIter *iter_1,
						    GtkTreeIter *iter_2,
						    GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_mark ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_no ( GtkTreeModel *model,
					GtkTreeIter *iter_1,
					GtkTreeIter *iter_2,
					GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_no_sort (  GtkTreeModel *model,
					      GtkTreeIter *iter_1,
					      GtkTreeIter *iter_2,
					      GtkSortType sort_type,
					      gint no_sort );
static gint gsb_transactions_list_sort_by_notes ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_party ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_reconcile_nb ( GtkTreeModel *model,
						  GtkTreeIter *iter_1,
						  GtkTreeIter *iter_2,
						  GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_transaction_date_and_no ( GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_type ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_value_date ( GtkTreeModel *model,
						GtkTreeIter *iter_1,
						GtkTreeIter *iter_2,
						GtkSortType sort_type );
static gint gsb_transactions_list_sort_by_voucher ( GtkTreeModel *model,
					     GtkTreeIter *iter_1,
					     GtkTreeIter *iter_2,
					     GtkSortType sort_type );
static gint gsb_transactions_list_sort_check_archive (  GtkTreeModel *model,
						 GtkTreeIter *iter_1,
						 GtkTreeIter *iter_2,
						 GtkSortType sort_type );
static gint gsb_transactions_list_sort_general_test ( GtkTreeModel *model,
					       GtkTreeIter *iter_1,
					       GtkTreeIter *iter_2,
					       GtkSortType sort_type );
/*END_STATIC*/



/*START_EXTERN*/
extern GtkTreeViewColumn *transactions_tree_view_columns[TRANSACTION_LIST_COL_NB];
/*END_EXTERN*/

static gint transaction_number_1;
static gint transaction_number_2;
static gint line_1;
static gint line_2;


/**
 * called by a click on the column, used to sort the list
 *
 * \param model
 * \param iter_1
 * \param iter_2
 * \param column_number the number of the column (0 to TRANSACTION_LIST_COL_NB) we want to sort by
 *
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column ( GtkTreeModel *model,
					 GtkTreeIter *iter_1,
					 GtkTreeIter *iter_2,
					 gint *column_number )
{
    gint account_number;
    gint element_number;
    gint return_value;
    GtkSortType sort_type;

    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number == -1)
	/* normally cannot happen, except come here at the opening
	 * of grisbi, and must return 0 if we don't want a crash */
	return 0;

    sort_type = gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( transactions_tree_view_columns[GPOINTER_TO_INT (column_number)]));

    /* first of all, check the archive */
    return_value = gsb_transactions_list_sort_check_archive ( model,
							      iter_1,
							      iter_2,
							      sort_type );
    if (return_value)
	/* there is an archive in the tested line, return now */
	return return_value;

    /* get the element used to sort the list */
    element_number = gsb_data_account_get_element_sort ( account_number,
							 GPOINTER_TO_INT (column_number));

    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   sort_type,
						   element_number );
}


/**
 * this is the first check of all : the archive
 * we put them always at the top of the list
 *
 * \param model
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_check_archive (  GtkTreeModel *model,
						 GtkTreeIter *iter_1,
						 GtkTreeIter *iter_2,
						 GtkSortType sort_type )

{
    gint return_value = 0;
    gpointer archive_1;
    gpointer archive_2;
    gint what_is_line_1;
    gint what_is_line_2;

    gtk_tree_model_get ( model,
			 iter_1,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &archive_1,
			 TRANSACTION_COL_NB_WHAT_IS_LINE, &what_is_line_1,
			 -1 );
    gtk_tree_model_get ( model,
			 iter_2,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &archive_2,
			 TRANSACTION_COL_NB_WHAT_IS_LINE, &what_is_line_2,
			 -1 );

    if (what_is_line_1 == IS_ARCHIVE)
    {
	if (what_is_line_2 == IS_ARCHIVE)
	{
	    /* the first and second line are archives, we return a comparison by number of archive
	     * we can do better, by date or by financial year, but more complex because no check for now
	     * that the date must be different, and problem when created by report
	     * so we assume the user created the archive in the good order, if some complains about that
	     * can change here later */
	    return_value = gsb_data_archive_get_no_archive (archive_1) - gsb_data_archive_get_no_archive (archive_2);
	}
	else
	    /* the first line is an archive and not the second, so first line before */
	    return_value = -1;
    }
    else
    {
	if (what_is_line_2 == IS_ARCHIVE)
	    /* the first line is not an archive but the second one is, so second line before */
	    return_value = 1;
	else
	    /* we have 2 transactions, just return 0 here to make tests later */
	    return 0;
    }
    /* for a descending sort, gtk invert the values and -1 is to set iter_1 after iter_2
     * so invert here */
    if ( sort_type == GTK_SORT_ASCENDING )
	return return_value;
    else
	return -return_value;
}


/**
 * find the right function to sort the list and sort the 2 iters given
 * 
 * \param model
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \param no_sort permit to find the right function
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_no_sort (  GtkTreeModel *model,
					      GtkTreeIter *iter_1,
					      GtkTreeIter *iter_2,
					      GtkSortType sort_type,
					      gint no_sort )

{
    gchar* tmpstr;
    switch ( no_sort )
    {
	case TRANSACTION_LIST_DATE:
	    return ( gsb_transactions_list_sort_by_date ( model,
							  iter_1,
							  iter_2,
							  sort_type ));
	    break;
	case TRANSACTION_LIST_VALUE_DATE:
	    return ( gsb_transactions_list_sort_by_value_date ( model,
								iter_1,
								iter_2,
								sort_type ));
	    break;
	case TRANSACTION_LIST_PARTY:
	    return ( gsb_transactions_list_sort_by_party ( model,
							   iter_1,
							   iter_2,
							   sort_type ));
	    break;
	case TRANSACTION_LIST_BUDGET:
	    return ( gsb_transactions_list_sort_by_budget ( model,
							    iter_1,
							    iter_2,
							    sort_type ));
	    break;
	case TRANSACTION_LIST_CREDIT:
	    return ( gsb_transactions_list_sort_by_credit ( model,
							    iter_1,
							    iter_2,
							    sort_type ));
	    break;
	case TRANSACTION_LIST_DEBIT:
	    return ( gsb_transactions_list_sort_by_debit ( model,
							   iter_1,
							   iter_2,
							   sort_type ));
	    break;
	case TRANSACTION_LIST_BALANCE:
	    /* 	    balance, normally, shouldn't be here... in case, give back the date */
	    return ( gsb_transactions_list_sort_by_date ( model,
							  iter_1,
							  iter_2,
							  sort_type ));
	    break;
	case TRANSACTION_LIST_AMOUNT:
	    return ( gsb_transactions_list_sort_by_amount ( model,
							    iter_1,
							    iter_2,
							    sort_type ));
	    break;
	case TRANSACTION_LIST_TYPE:
	    return ( gsb_transactions_list_sort_by_type ( model,
							  iter_1,
							  iter_2,
							  sort_type ));
	    break;
	case TRANSACTION_LIST_RECONCILE_NB:
	    return ( gsb_transactions_list_sort_by_reconcile_nb ( model,
								  iter_1,
								  iter_2,
								  sort_type ));
	    break;
	case TRANSACTION_LIST_EXERCICE:
	    return ( gsb_transactions_list_sort_by_financial_year ( model,
								    iter_1,
								    iter_2,
								    sort_type ));
	    break;
	case TRANSACTION_LIST_CATEGORY:
	    return ( gsb_transactions_list_sort_by_category ( model,
							      iter_1,
							      iter_2,
							      sort_type ));
	    break;
	case TRANSACTION_LIST_MARK:
	    return ( gsb_transactions_list_sort_by_mark ( model,
							  iter_1,
							  iter_2,
							  sort_type ));
	    break;
	case TRANSACTION_LIST_VOUCHER:
	    return ( gsb_transactions_list_sort_by_voucher ( model,
							     iter_1,
							     iter_2,
							     sort_type ));
	    break;
	case TRANSACTION_LIST_NOTES:
	    return ( gsb_transactions_list_sort_by_notes ( model,
							   iter_1,
							   iter_2,
							   sort_type ));
	    break;
	case TRANSACTION_LIST_BANK:
	    return ( gsb_transactions_list_sort_by_bank ( model,
							  iter_1,
							  iter_2,
							  sort_type ));
	    break;
	case TRANSACTION_LIST_NO:
	    return ( gsb_transactions_list_sort_by_no ( model,
							iter_1,
							iter_2,
							sort_type ));
	    break;
	case TRANSACTION_LIST_CHQ:
	    return ( gsb_transactions_list_sort_by_chq ( model,
							 iter_1,
							 iter_2,
							 sort_type ));
	    break;
	default :
	    tmpstr = g_strdup_printf ( _("problem in gsb_transactions_list_sort_by_no_sort : ask for the sort number %d which doesn't exist... return by date\n"), no_sort );
	    warning_debug (tmpstr);
	    g_free(tmpstr);
	    return ( gsb_transactions_list_sort_by_date ( model,
							  iter_1,
							  iter_2,
							  sort_type ));
    }
}


/**
 * used by all the sort functions at the begining for the transaction list,
 * check for the part wich cannot change : the white line must always be at
 * the end of the list
 * and into a transaction, the lines are not sorted in ascending or descending method
 * so it's here we do that
 * that function fill too the globals variables transaction_number_1,
 * transaction_number_2, line_1 and line_2 used later in the others functions
 * 
 * \param model
 * \param iter_1
 * \param iter_2
 * \param sort_type
 * 
 * \return 0 if that test cannot say the return_value between the 2 lines,
 * or the return_value if it's possible here
 * */
gint gsb_transactions_list_sort_general_test ( GtkTreeModel *model,
					       GtkTreeIter *iter_1,
					       GtkTreeIter *iter_2,
					       GtkSortType sort_type )
{
    gint return_value = 0;
    gpointer transaction_1;
    gpointer transaction_2;

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
	alert_debug ( "Local variable value NULL in the function gsb_transactions_list_sort_general_test, transaction_1 or transaction_2 is NULL ; it souldn't happen, it's seems that the function is called by a bad way" ) ;
	return 0;
    }

    transaction_number_1 = gsb_data_transaction_get_transaction_number (transaction_1);
    transaction_number_2 = gsb_data_transaction_get_transaction_number (transaction_2);

    /* check first for the white lines, it's always set at the end */
    if ( transaction_number_1 <= 0 )
	return_value = 1;
    else
    {
	if (transaction_number_2 <= 0)
	    return_value = -1;
    }

    if ( transaction_number_1 == transaction_number_2 )
	return_value = line_1 - line_2;

    /* for a descending sort, gtk invert the values and -1 is to set iter_1 after iter_2
     * so invert here */
    if ( sort_type == GTK_SORT_ASCENDING )
	return return_value;
    else
	return -return_value;
}



/**
 * used to compare the 2 dates first, and if they are the same
 * the 2 no of transactions to find the good return for sort
 * called at the end of each sort test, if they are equal
 * 
 * \param none but the local variables transaction_number_1 and transaction_number_2 MUST be set
 * 
 * \return -1 if transaction_1 is above transaction_2
 * */
gint gsb_transactions_list_sort_by_transaction_date_and_no ( GtkSortType sort_type )
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

    return_value = g_date_compare ( gsb_data_transaction_get_date (transaction_number_1),
				    gsb_data_transaction_get_date (transaction_number_2));

    /* no difference in the dates, sort by number of transaction */
    if ( !return_value )
	return_value = transaction_number_1 - transaction_number_2;

    return return_value;
}


/**
 * used to compare 2 iters and sort the by no of transaction
 * always put the white line below
 * 
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_no ( GtkTreeModel *model,
					GtkTreeIter *iter_1,
					GtkTreeIter *iter_2,
					GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    return transaction_number_1 - transaction_number_2;
}



/** used to compare 2 iters and sort the by date first, and no
 * transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_date ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */
    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by value date first, and date 
 * and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_value_date ( GtkTreeModel *model,
						GtkTreeIter *iter_1,
						GtkTreeIter *iter_2,
						GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    return_value = g_date_compare ( gsb_data_transaction_get_value_date (transaction_number_1),
				    gsb_data_transaction_get_value_date (transaction_number_2));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);

}



/** used to compare 2 iters and sort the by party first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_party ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    if ( gsb_data_transaction_get_party_number ( transaction_number_1)== gsb_data_transaction_get_party_number ( transaction_number_2))
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
    else
    {
	const gchar *temp_1;
	const gchar *temp_2;

	temp_1 = gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number_1),
					   TRUE );
	temp_2 = gsb_data_payee_get_name ( gsb_data_transaction_get_party_number ( transaction_number_2),
					   TRUE );

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
	return gsb_transactions_list_sort_by_transaction_date_and_no (sort_type);
}




/** used to compare 2 iters and sort the by budgetary first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_budget ( GtkTreeModel *model,
					    GtkTreeIter *iter_1,
					    GtkTreeIter *iter_2,
					    GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    if ( gsb_data_transaction_get_budgetary_number ( transaction_number_1) == gsb_data_transaction_get_budgetary_number ( transaction_number_2)
	 &&
	 gsb_data_transaction_get_sub_budgetary_number ( transaction_number_1)== gsb_data_transaction_get_sub_budgetary_number ( transaction_number_2))
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
    else
    {
	const gchar *temp_1;
	const gchar *temp_2;

	temp_1 = gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( transaction_number_1),
					    gsb_data_transaction_get_sub_budgetary_number ( transaction_number_1),
					    NULL );
	temp_2 = gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number ( transaction_number_2),
					    gsb_data_transaction_get_sub_budgetary_number ( transaction_number_2),
					    NULL);

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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}




/**
 * used to compare 2 iters and sort the by credit amount first, and 
 * by date and no transaction after
 * always put the white line below
 * 
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * 
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_credit ( GtkTreeModel *model,
					    GtkTreeIter *iter_1,
					    GtkTreeIter *iter_2,
					    GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    /* for the amounts, we have to check also the currency */
    return_value = gsb_real_cmp ( gsb_data_transaction_get_adjusted_amount ( transaction_number_1, -1),
				  gsb_data_transaction_get_adjusted_amount ( transaction_number_2, -1));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by debit amount first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_debit ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    /* for the amounts, we have to check also the currency */

    return_value = gsb_real_cmp ( gsb_data_transaction_get_adjusted_amount ( transaction_number_2, -1),
				  gsb_data_transaction_get_adjusted_amount ( transaction_number_1, -1));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by amount first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_amount ( GtkTreeModel *model,
					    GtkTreeIter *iter_1,
					    GtkTreeIter *iter_2,
					    GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    /* for the amounts, we have to check also the currency */
    return_value = gsb_real_cmp ( gsb_data_transaction_get_adjusted_amount ( transaction_number_1, -1),
				  gsb_data_transaction_get_adjusted_amount ( transaction_number_2, -1));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by type first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_type ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    /* if it's the same type, we sort by the content of the types */

    if ( gsb_data_transaction_get_method_of_payment_number ( transaction_number_1)== gsb_data_transaction_get_method_of_payment_number ( transaction_number_2))
    {
	return_value = g_utf8_collate ( g_utf8_casefold ( gsb_data_transaction_get_method_of_payment_content ( transaction_number_1) ? gsb_data_transaction_get_method_of_payment_content ( transaction_number_1) : "",
							  -1 ),
					g_utf8_casefold ( gsb_data_transaction_get_method_of_payment_content ( transaction_number_2) ? gsb_data_transaction_get_method_of_payment_content ( transaction_number_2) : "",
							  -1 ));

	if ( !return_value )
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
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
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
    }
    return return_value;
}



/** used to compare 2 iters and sort the by reconcile number first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_reconcile_nb ( GtkTreeModel *model,
						  GtkTreeIter *iter_1,
						  GtkTreeIter *iter_2,
						  GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    if ( gsb_data_transaction_get_reconcile_number ( transaction_number_1)== gsb_data_transaction_get_reconcile_number ( transaction_number_2))
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by financial_year first, and no
 * transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_financial_year ( GtkTreeModel *model,
						    GtkTreeIter *iter_1,
						    GtkTreeIter *iter_2,
						    GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    if ( gsb_data_transaction_get_financial_year_number ( transaction_number_1)== gsb_data_transaction_get_financial_year_number ( transaction_number_2))
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
    else
    {
	GDate *date_1;
	GDate *date_2;

	date_1 = gsb_data_fyear_get_begining_date (gsb_data_transaction_get_financial_year_number ( transaction_number_1));
	date_2 = gsb_data_fyear_get_begining_date (gsb_data_transaction_get_financial_year_number ( transaction_number_2));

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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);

}



/** used to compare 2 iters and sort the by category first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_category ( GtkTreeModel *model,
					      GtkTreeIter *iter_1,
					      GtkTreeIter *iter_2,
					      GtkSortType sort_type )
{
    gint return_value;
    gchar *temp_1;
    gchar *temp_2;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    /** we want to take the name of the categ, so, either
     * breakdown of transaction
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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}


/** used to compare 2 iters and sort the by mark first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_mark ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_data_transaction_get_marked_transaction ( transaction_number_1)- gsb_data_transaction_get_marked_transaction ( transaction_number_2);


    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by voucher first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_voucher ( GtkTreeModel *model,
					     GtkTreeIter *iter_1,
					     GtkTreeIter *iter_2,
					     GtkSortType sort_type )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}


/** used to compare 2 iters and sort the by notes first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_notes ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   GtkSortType sort_type )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by bank first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_bank ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}



/** used to compare 2 iters and sort the by cheque or no of transfer first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_chq ( GtkTreeModel *model,
					 GtkTreeIter *iter_1,
					 GtkTreeIter *iter_2,
					 GtkSortType sort_type )
{
    gint return_value;
    const gchar *temp_1;
    const gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_sort_general_test ( model,
							     iter_1,
							     iter_2,
							     sort_type );
    if ( return_value )
	return return_value;

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
	return gsb_transactions_list_sort_by_transaction_date_and_no(sort_type);
}





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
