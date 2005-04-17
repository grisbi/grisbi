/* ************************************************************************** */
/* Contient toutes les fonctions utilisées pour classer la liste des opé      */
/* 			classement_liste.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
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

#include "include.h"


/*START_INCLUDE*/
#include "classement_operations.h"
#include "utils_devises.h"
#include "erreur.h"
#include "gsb_account.h"
#include "utils_exercices.h"
#include "gsb_transaction_data.h"
#include "operations_liste.h"
#include "utils_ib.h"
#include "utils_operations.h"
#include "utils_rapprochements.h"
#include "utils_tiers.h"
#include "utils_types.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint gsb_transactions_list_breakdown_test ( GtkSortType sort_type );
static gint gsb_transactions_list_general_test ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type );
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
static gint gsb_transactions_list_sort_by_transaction_date_and_no ( void );
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
/*END_STATIC*/



/*START_EXTERN*/
extern GtkTreeStore *model;
/*END_EXTERN*/

static struct structure_operation *transaction_1;
static struct structure_operation *transaction_2;
static gint line_1;
static gint line_2;


/** called by a click on the column
 * find the right parameter to sort
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_account
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column_0 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account )
{
    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( GPOINTER_TO_INT ( no_account ),
																	 TRANSACTION_COL_NB_CHECK ))),
						   gsb_account_get_column_sort ( GPOINTER_TO_INT ( no_account ),
										 TRANSACTION_COL_NB_CHECK ));
}

/** called by a click on the column
 * find the right parameter to sort
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_account
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column_1 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account )
{
    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( GPOINTER_TO_INT ( no_account ),
																	 TRANSACTION_COL_NB_DATE ))),
						   gsb_account_get_column_sort ( GPOINTER_TO_INT ( no_account ),
										 TRANSACTION_COL_NB_DATE ));
}


/** called by a click on the column
 * find the right parameter to sort
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_account
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column_2 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account )
{
    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( GPOINTER_TO_INT ( no_account ),
																	 TRANSACTION_COL_NB_PARTY ))),
						   gsb_account_get_column_sort ( GPOINTER_TO_INT ( no_account ),
										 TRANSACTION_COL_NB_PARTY ));
}


/** called by a click on the column
 * find the right parameter to sort
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_account
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column_3 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account )
{
    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( GPOINTER_TO_INT ( no_account ),
																	 TRANSACTION_COL_NB_PR ))),
						   gsb_account_get_column_sort ( GPOINTER_TO_INT ( no_account ),
										 TRANSACTION_COL_NB_PR ));
}


/** called by a click on the column
 * find the right parameter to sort
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_account
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column_4 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account )
{
    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( GPOINTER_TO_INT ( no_account ),
																	 TRANSACTION_COL_NB_DEBIT ))),
						   gsb_account_get_column_sort ( GPOINTER_TO_INT ( no_account ),
										 TRANSACTION_COL_NB_DEBIT ));
}


/** called by a click on the column
 * find the right parameter to sort
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_account
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column_5 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account )
{
    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( GPOINTER_TO_INT ( no_account ),
																	 TRANSACTION_COL_NB_CREDIT ))),
						   gsb_account_get_column_sort ( GPOINTER_TO_INT ( no_account ),
										 TRANSACTION_COL_NB_CREDIT ));
}


/** called by a click on the column
 * find the right parameter to sort
 * \param model
 * \param iter_1
 * \param iter_2
 * \param no_account
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_column_6 ( GtkTreeModel *model,
					   GtkTreeIter *iter_1,
					   GtkTreeIter *iter_2,
					   gint *no_account )
{
    return gsb_transactions_list_sort_by_no_sort ( model,
						   iter_1,
						   iter_2,
						   gtk_tree_view_column_get_sort_order ( GTK_TREE_VIEW_COLUMN ( gsb_account_get_column ( GPOINTER_TO_INT ( no_account ),
																	 TRANSACTION_COL_NB_BALANCE ))),
						   gsb_account_get_column_sort ( GPOINTER_TO_INT ( no_account ),
										 TRANSACTION_COL_NB_BALANCE ));
}


/** find the right function to sort the list and sort the 2 iters given
 * \param model
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \param no_sort permit to find the right function
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_no_sort (  GtkTreeModel *model,
					      GtkTreeIter *iter_1,
					      GtkTreeIter *iter_2,
					      GtkSortType sort_type,
					      gint no_sort )

{
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
	    printf ( "Bug : ask for the sort number %d which doesn't exist... return by date\n",
		     no_sort );
	    return ( gsb_transactions_list_sort_by_date ( model,
							  iter_1,
							  iter_2,
							  sort_type ));
    }
}
/******************************************************************************/


/** used by all the sort functions for the transaction list,
 * get the 2 transactions and the 2 lines, and do the minimun
 * check, for white line and for the other lines shown of the
 * transaction
 * check also with sort_type (ascending or descending)
 * \param model
 * \param iter_1
 * \param iter_2
 * \param sort_type
 * \return 0 if that test cannot say the return_value between the 2 lines,
 * or the return_value if it's possible here
 * */
gint gsb_transactions_list_general_test ( GtkTreeModel *model,
					  GtkTreeIter *iter_1,
					  GtkTreeIter *iter_2,
					  GtkSortType sort_type )
{
    gint return_value = 0;

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
	debug_message ( _( "Local variable value NULL" ),
			_( "in the function gsb_transactions_list_general_test, transaction_1 or transaction_2 is NULL ; it souldn't happen, it's seems that the function is called by a bad way" ),
			DEBUG_LEVEL_ALERT,
			FALSE );
	return 0;
    }

    if ( transaction_1 == GINT_TO_POINTER (-1))
	return_value = 1;
    if ( transaction_2 == GINT_TO_POINTER (-1))
	return_value = -1;
    if ( transaction_1 == transaction_2 )
	return_value = line_1 - line_2;

     if ( sort_type == GTK_SORT_ASCENDING )
	return return_value;
    else
	return -return_value;
}



/** used by all the sort functions for the transaction list,
 * keep the breakdown under the mother, and white breakdown under
 * the others
 * \param sort_type
 * \return 0 if that test cannot say the return_value between the 2 lines,
 * or the return_value if it's possible here
 * */
gint gsb_transactions_list_breakdown_test ( GtkSortType sort_type )
{
    gint return_value = 0;

    if ( transaction_1 -> no_operation_ventilee_associee )
    {
	if ( gsb_transaction_data_get_transaction_number (transaction_2) == transaction_1 -> no_operation_ventilee_associee )
	    return_value = 1;
	else
	{
	    if ( transaction_2 -> no_operation_ventilee_associee )
	    {
		if ( transaction_2 -> no_operation_ventilee_associee == transaction_1 -> no_operation_ventilee_associee )
		{
		    if ( gsb_transaction_data_get_transaction_number (transaction_1) == -2 )
			return_value = 1;
		    else
		    {
			if ( gsb_transaction_data_get_transaction_number (transaction_2) == -2 )
			    return_value = -1;
		    }
		}
		else
		{
		    transaction_1 = operation_par_no ( transaction_1 -> no_operation_ventilee_associee,
						       gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1)));
		    transaction_2 = operation_par_no ( transaction_2 -> no_operation_ventilee_associee,
						       gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_2)));
		}
	    }
	    else
		transaction_1 = operation_par_no ( transaction_1 -> no_operation_ventilee_associee,
						   gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1)));
	}
    }
    else
    {
	if ( transaction_2 -> no_operation_ventilee_associee )
	{
	    if ( gsb_transaction_data_get_transaction_number (transaction_1) == transaction_2 -> no_operation_ventilee_associee )
		return_value = -1;
	    else
		transaction_2 = operation_par_no ( transaction_2 -> no_operation_ventilee_associee,
						   gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_2)));
	}
    }

    if ( sort_type == GTK_SORT_ASCENDING )
	return return_value;
    else
	return -return_value;
}


/** used to compare the 2 dates first, and if they are the same
 * the 2 no of transactions to find the good return for sort
 * called at the end of each sort test, if they are equal
 * \param none but the local variables transaction_1 an transaction_2 MUST be set
 * \return -1 if transaction_1 is above transaction_2
 * */
gint gsb_transactions_list_sort_by_transaction_date_and_no ( void )
{
    gint return_value;

    return_value = g_date_compare ( transaction_1 -> date,
				    transaction_2 -> date );

    if ( return_value )
	return return_value;
    else
	return gsb_transaction_data_get_transaction_number (transaction_1) - gsb_transaction_data_get_transaction_number (transaction_2);
}

/** used to compare 2 iters and sort the by no of transaction
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_no ( GtkTreeModel *model,
					GtkTreeIter *iter_1,
					GtkTreeIter *iter_2,
					GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    return gsb_transaction_data_get_transaction_number (transaction_1) - gsb_transaction_data_get_transaction_number (transaction_2);
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    return gsb_transactions_list_sort_by_transaction_date_and_no();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    return_value = g_date_compare ( transaction_1 -> date_bancaire,
				    transaction_2 -> date_bancaire );

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();

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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    if ( transaction_1 -> tiers == transaction_2 -> tiers )
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    else
    {
	gchar *temp_1;
	gchar *temp_2;

	temp_1 = tiers_name_by_no ( transaction_1 -> tiers,
				    TRUE );
	temp_2 = tiers_name_by_no ( transaction_2 -> tiers,
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
	return gsb_transactions_list_sort_by_transaction_date_and_no ();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    if ( transaction_1 -> imputation == transaction_2 -> imputation
	 &&
	 transaction_1 -> sous_imputation == transaction_2 -> sous_imputation )
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    else
    {
	gchar *temp_1;
	gchar *temp_2;

	temp_1 = nom_imputation_par_no ( transaction_1 -> imputation,
					 transaction_1 -> sous_imputation );
	temp_2 = nom_imputation_par_no ( transaction_2 -> imputation,
					 transaction_2 -> sous_imputation );

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
	return gsb_transactions_list_sort_by_transaction_date_and_no();
}




/** used to compare 2 iters and sort the by credit amount first, and 
 * by date and no transaction after
 * always put the white line below
 * \param model the GtkTreeModel
 * \param iter_1
 * \param iter_2
 * \param sort_type GTK_SORT_ASCENDING or GTK_SORT_DESCENDING
 * \return -1 if iter_1 is above iter_2
 * */
gint gsb_transactions_list_sort_by_credit ( GtkTreeModel *model,
					    GtkTreeIter *iter_1,
					    GtkTreeIter *iter_2,
					    GtkSortType sort_type )
{
    gint return_value;

    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    /* for the amounts, we have to check also the currency */

    if ( fabs (transaction_1 -> montant - transaction_2 -> montant) < 0.01
	 &&
	 transaction_1 -> devise == transaction_2 -> devise )
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    else
    {
	if ( transaction_1 -> devise == transaction_2 -> devise )
	    return_value = transaction_1 -> montant - transaction_2 -> montant;
	else
	{
	    gdouble amount_1, amount_2;

	    amount_1 = calcule_montant_devise_renvoi ( transaction_1 -> montant,
						       gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1))),
						       transaction_1 -> devise,
						       transaction_1 -> une_devise_compte_egale_x_devise_ope,
						       transaction_1 -> taux_change,
						       transaction_1 -> frais_change );
	    amount_2 = calcule_montant_devise_renvoi ( transaction_2 -> montant,
						       gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1 ))),
						       transaction_2 -> devise,
						       transaction_2 -> une_devise_compte_egale_x_devise_ope,
						       transaction_2 -> taux_change,
						       transaction_2 -> frais_change );
	    return_value = amount_1 - amount_2;

	}
    }


    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    /* for the amounts, we have to check also the currency */

    if ( fabs (transaction_1 -> montant - transaction_2 -> montant) < 0.01
	 &&
	 transaction_1 -> devise == transaction_2 -> devise )
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    else
    {
	if ( transaction_1 -> devise == transaction_2 -> devise )
	    return_value = transaction_2 -> montant - transaction_1 -> montant;
	else
	{
	    gdouble amount_1, amount_2;

	    amount_1 = calcule_montant_devise_renvoi ( transaction_1 -> montant,
						       gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1 ))),
						       transaction_1 -> devise,
						       transaction_1 -> une_devise_compte_egale_x_devise_ope,
						       transaction_1 -> taux_change,
						       transaction_1 -> frais_change );
	    amount_2 = calcule_montant_devise_renvoi ( transaction_2 -> montant,
						       gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1))),
						       transaction_2 -> devise,
						       transaction_2 -> une_devise_compte_egale_x_devise_ope,
						       transaction_2 -> taux_change,
						       transaction_2 -> frais_change );
	    return_value = amount_2 - amount_1;

	}
    }

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    /* for the amounts, we have to check also the currency */

    if ( fabs (transaction_1 -> montant - transaction_2 -> montant) < 0.01
	 &&
	 transaction_1 -> devise == transaction_2 -> devise )
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    else
    {
	if ( transaction_1 -> devise == transaction_2 -> devise )
	    return_value = fabs(transaction_1 -> montant) - fabs (transaction_2 -> montant);
	else
	{
	    gdouble amount_1, amount_2;

	    amount_1 = calcule_montant_devise_renvoi ( transaction_1 -> montant,
						       gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1))),
						       transaction_1 -> devise,
						       transaction_1 -> une_devise_compte_egale_x_devise_ope,
						       transaction_1 -> taux_change,
						       transaction_1 -> frais_change );
	    amount_2 = calcule_montant_devise_renvoi ( transaction_2 -> montant,
						       gsb_account_get_currency (gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1))),
						       transaction_2 -> devise,
						       transaction_2 -> une_devise_compte_egale_x_devise_ope,
						       transaction_2 -> taux_change,
						       transaction_2 -> frais_change );
	    return_value = fabs(amount_1) - fabs(amount_2);

	}
    }

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    /* if it's the same type, we sort by the content of the types */

    if ( transaction_1 -> type_ope == transaction_2 -> type_ope )
    {
	return_value = g_utf8_collate ( g_utf8_casefold ( transaction_1 -> contenu_type ? transaction_1 -> contenu_type : "",
							  -1 ),
					g_utf8_casefold ( transaction_2 -> contenu_type ? transaction_2 -> contenu_type : "",
							  -1 ));

	if ( !return_value )
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    }
    else
    {
	gchar *temp_1;
	gchar *temp_2;

	temp_1 = type_ope_name_by_no ( transaction_1 -> type_ope,
				       gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_1)));
	temp_2 = type_ope_name_by_no ( transaction_2 -> type_ope,
				       gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (transaction_2)));

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

	return_value = g_utf8_collate ( g_utf8_casefold ( transaction_1 -> contenu_type ? transaction_1 -> contenu_type : "",
							  -1 ),
					g_utf8_casefold ( transaction_2 -> contenu_type ? transaction_2 -> contenu_type : "",
							  -1 ));

	if ( !return_value )
	    return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    if ( transaction_1 -> no_rapprochement == transaction_2 -> no_rapprochement )
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    else
    {
	gchar *temp_1;
	gchar *temp_2;

	temp_1 = rapprochement_name_by_no ( transaction_1 -> no_rapprochement );
	temp_2 = rapprochement_name_by_no ( transaction_2 -> no_rapprochement );

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
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    if ( transaction_1 -> no_exercice == transaction_2 -> no_exercice )
	return_value = gsb_transactions_list_sort_by_transaction_date_and_no();
    else
    {
	GDate *date_1;
	GDate *date_2;

	date_1 = gsb_financial_year_get_begining_date (transaction_1 -> no_exercice);
	date_2 = gsb_financial_year_get_begining_date (transaction_2 -> no_exercice);

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
	return gsb_transactions_list_sort_by_transaction_date_and_no();

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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    /** we want to take the name of the categ, so, either
     * breakdown of transaction
     * transfer : ...
     * categ : under-categ
     * and after, we sort by str
     * */

    temp_1 = gsb_transactions_get_category_real_name ( transaction_1 );
    temp_2 = gsb_transactions_get_category_real_name ( transaction_2 );

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    return_value = transaction_1 -> pointe - transaction_2 -> pointe;


    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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
    gchar *temp_1;
    gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    temp_1 = transaction_1 -> no_piece_comptable;
    temp_2 = transaction_2 -> no_piece_comptable;

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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
    gchar *temp_1;
    gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    temp_1 = transaction_1 -> notes;
    temp_2 = transaction_2 -> notes;

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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
    gchar *temp_1;
    gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    temp_1 = transaction_1 -> info_banque_guichet;
    temp_2 = transaction_2 -> info_banque_guichet;

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
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
    gchar *temp_1;
    gchar *temp_2;


    /*     general test first (white line, other rows of the transaction */

    return_value = gsb_transactions_list_general_test ( model,
							iter_1,
							iter_2,
							sort_type );
    if ( return_value )
	return return_value;

    return_value = gsb_transactions_list_breakdown_test ( sort_type );

    if ( return_value )
	return return_value;

    temp_1 = transaction_1 -> contenu_type;
    temp_2 = transaction_2 -> contenu_type;

    /* g_utf8_collate is said not very fast, must try with big big account to check
     * if it's enough, for me it's ok (cedric), eventually, change with gsb_strcasecmp */
    return_value = g_utf8_collate ( g_utf8_casefold ( temp_1 ? temp_1 : "",
						      -1 ),
				    g_utf8_casefold ( temp_2 ? temp_2 : "",
						      -1 ));

    if ( return_value )
	return return_value;
    else
	return gsb_transactions_list_sort_by_transaction_date_and_no();
}




/* ************************************************************************** */
/* classement par no d'opé (donc d'entrée)                                    */
/* FIXME : encore utilisé par les échéances */
/* ************************************************************************** */
gint classement_liste_par_no_ope_ventil ( GtkWidget *liste,
					  GtkCListRow *ligne_1,
					  GtkCListRow *ligne_2 )
{
    struct operation_echeance *operation_1;
    struct operation_echeance *operation_2;

    operation_1 = ligne_1 -> data;
    operation_2 = ligne_2 -> data;

    if ( operation_1 == GINT_TO_POINTER ( -1 ) )
	return ( 1 );

    if ( operation_2 == GINT_TO_POINTER ( -1 ) )
	return ( -1 );

    if ( operation_1 == NULL )
	return ( 1 );

    if ( operation_2 == NULL )
	return ( -1 );

    return ( operation_1 -> no_operation - operation_2 -> no_operation );
}
/* ************************************************************************** */



/** do the same as g_strcasecmp but works alse with the accents on the words
 * \param string_1 the first string to cmp
 * \param string_2 the second string to cmp
 * \return -1 if string_1 berfore string_2
 * */
gint gsb_strcasecmp ( gchar *string_1,
		      gchar *string_2 )
{
    string_1 = g_strdup ( string_1 );
    string_1 = g_strdelimit ( string_1, "éÉèÈêÊ", 'e' );
    string_1 = g_strdelimit ( string_1, "çÇ", 'c' );
    string_1 = g_strdelimit ( string_1, "àÀ", 'a' );
    string_1 = g_strdelimit ( string_1, "ùûÙÛ", 'u' );
    string_1 = g_strdelimit ( string_1, "ôÔ", 'o' );
    string_1 = g_strdelimit ( string_1, "îÎ", 'i' );

    string_2 = g_strdup ( string_2 );
    string_2 = g_strdelimit ( string_2, "éÉèÈêÊ", 'e' );
    string_2 = g_strdelimit ( string_2, "çÇ", 'c' );
    string_2 = g_strdelimit ( string_2, "àÀ", 'a' );
    string_2 = g_strdelimit ( string_2, "ùûÙÛ", 'u' );
    string_2 = g_strdelimit ( string_2, "ôÔ", 'o' );
    string_2 = g_strdelimit ( string_2, "îÎ", 'i' );

    return ( g_strcasecmp ( string_1, string_2 ));
}
/* ************************************************************************** */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
