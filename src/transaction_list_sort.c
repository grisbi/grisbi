/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
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
 * \file transaction_list_sort.c
 * this contains the sorting functions to use with the CustomList
 * Grisbi shouldn't work directly on the CustomList except by those files
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "transaction_list_sort.h"
#include "custom_list.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "gsb_reconcile_list.h"
#include "gsb_transactions_list.h"
#include "gsb_transactions_list_sort.h"
#include "navigation.h"
#include "transaction_model.h"
#include "utils_dates.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean transaction_list_sort_get_initial_sort ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeViewColumn *transactions_tree_view_columns[CUSTOM_MODEL_N_VISIBLES_COLUMN];
/*END_EXTERN*/



/**
 * Sort the list according to the current column and sort type
 * sort the filtered model because we don't need to sort not showed columns
 * so filter the model before calling this function
 *
 * for now, only sort mother transactions, if need the children too,
 * need to go into each record structure and sort the children.
 * could be time consuming...
 *
 * to update all the tree view, use gsb_transactions_list_update_tree_view instead
 *
 * \param 
 *
 * \return
 * */
void transaction_list_sort (void)
{
    GtkTreePath *path;
    gint        *neworder, i;
    CustomList *custom_list;

    devel_debug (NULL);
    custom_list = transaction_model_get_model ();
    g_return_if_fail ( custom_list != NULL );

    /* resort */
    if (custom_list -> user_sort_reconcile)
        g_qsort_with_data(custom_list->visibles_rows,
                        custom_list->num_visibles_rows,
                        sizeof(CustomRecord*),
                        (GCompareDataFunc) gsb_reconcile_list_sort_func,
                        custom_list);
    else if ( transaction_list_sort_get_initial_sort ( ) && custom_list -> sort_order == GTK_SORT_ASCENDING )
        return;
    else
        g_qsort_with_data(custom_list->visibles_rows,
                        custom_list->num_visibles_rows,
                        sizeof(CustomRecord*),
                        (GCompareDataFunc) gsb_transactions_list_sort,
                        custom_list);

    /* let other objects know about the new order */
    neworder = g_new0(gint, custom_list->num_visibles_rows);

    for (i = 0; i < custom_list->num_visibles_rows; ++i)
    {
        neworder[i] = (custom_list->visibles_rows[i])->filtered_pos;
        (custom_list->visibles_rows[i])->filtered_pos = i;
    }

    path = gtk_tree_path_new();

    gtk_tree_model_rows_reordered(GTK_TREE_MODEL(custom_list), path, NULL, neworder);

    gtk_tree_path_free(path);
    g_free(neworder);
}



/**
 * change the sort column and/or the sort order
 * do nothing to the tree view, use gsb_transactions_list_update_tree_view to update it
 * 	or transaction_list_sort
 *
 * \param custom_list the CustomList
 * \param new_sort_col		column number to use to sort the list
 * \param new_sort_order	GTK_SORT_DESCENDING / GTK_SORT_ASCENDING
 *
 * \return
 * */
void transaction_list_sort_set_column ( gint new_sort_col,
                        GtkSortType new_sort_order )
{
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    devel_debug_int (new_sort_col);

    g_return_if_fail ( custom_list != NULL );

    if (new_sort_col >= CUSTOM_MODEL_VISIBLE_COLUMNS)
    {
	g_warning (_("Asked to sort by column %d, wich is bigger than the visible columns (%d)"),
		   new_sort_col,
		   CUSTOM_MODEL_VISIBLE_COLUMNS );
	return;
    }

    /* update the indicator */
    gtk_tree_view_column_set_sort_indicator (transactions_tree_view_columns[custom_list -> sort_col],
					     FALSE);
    gtk_tree_view_column_set_sort_indicator (transactions_tree_view_columns[new_sort_col],
					     TRUE);
    gtk_tree_view_column_set_sort_order (transactions_tree_view_columns[new_sort_col],
					 new_sort_order );

    /* save the new values */
    custom_list -> sort_col = new_sort_col;
    custom_list -> sort_order = new_sort_order;
}



/**
 * change the sort column and/or the sort order
 *
 * \param custom_list the CustomList
 * \param sort_col	column number to use to sort the list
 * \param sort_order	GTK_SORT_DESCENDING / GTK_SORT_ASCENDING
 *
 * \return
 * */
void transaction_list_sort_get_column ( gint *sort_col,
                        GtkSortType *sort_order )
{
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_if_fail ( custom_list != NULL );

    if (sort_col)
	*sort_col = custom_list -> sort_col;
    if (sort_order)
	*sort_order = custom_list -> sort_order;
}



/**
 * set if the sorting function is normal (by columns) or for reconcile
 *
 * \param use_reconcile_sort	TRUE to sort for reconciliation, FALSE for column sort
 *
 * \return
 * */
void transaction_list_sort_set_reconcile_sort ( gboolean use_reconcile_sort )
{
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    devel_debug_int (use_reconcile_sort);

    g_return_if_fail ( custom_list != NULL );

    custom_list -> user_sort_reconcile = use_reconcile_sort;

    /* update the indicator */
    gtk_tree_view_column_set_sort_indicator (transactions_tree_view_columns[custom_list -> sort_col],
					     !use_reconcile_sort );
}



/**
 * get if the sorting function is normal (by columns) or for reconcile
 *
 * \param
 *
 * \return TRUE if the list is sorted for reconcile, FALSE if column sort
 * */
gboolean transaction_list_sort_get_reconcile_sort ( void )
{
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE );

    return custom_list -> user_sort_reconcile;
}


/**
 * get if the sorting function is initial (by date or value_date) or not
 *
 * \param
 *
 * \return TRUE if the list is sorted by date or value_date, FALSE if not
 * */
gboolean transaction_list_sort_get_initial_sort ( void )
{
    gint account_number;
    gint element_number;

    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number == -1)
        return FALSE;

    element_number = gsb_data_account_get_element_sort ( account_number,
                        gsb_data_account_get_sort_column ( account_number ) );

    if ( element_number == ELEMENT_VALUE_DATE )
        return TRUE;
    else
        return FALSE;
}
