/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/* 			http://www.grisbi.org				                              */
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
 * \file transaction_list_select.c
 * this contains the select function to use with the CustomList
 * Grisbi shouldn't work directly on the CustomList except by those files
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "transaction_list_select.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "menu.h"
#include "gsb_transactions_list.h"
#include "transaction_model.h"
#include "custom_list.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean transaction_list_select_record ( CustomRecord *record );
/*END_STATIC*/

/*START_EXTERN*/
extern GdkColor couleur_selection;
/*END_EXTERN*/




/**
 * set the selection on the given transaction
 *
 * \param transaction_number	the transaction to select
 *
 * \return FALSE if not possible (not visible), TRUE if ok
 * */
gboolean transaction_list_select ( gint transaction_number )
{
    CustomRecord *record = NULL;
    gboolean return_value;
    GtkTreePath *path;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE );

    /* if the selection didn't change, do nothing */
    if ( gsb_data_transaction_get_transaction_number (custom_list -> selected_row) == 
        transaction_number )
	return FALSE;

    /* first, we un-select the last transaction */
    if (custom_list -> selected_row)
	transaction_list_select_unselect ();

    /* if we want the white line, this will increase a little the speed */
    if (transaction_number == -1)
    {
	record = custom_list -> visibles_rows [custom_list -> num_visibles_rows - custom_list -> nb_rows_by_transaction];
	if (gsb_data_transaction_get_transaction_number (record -> transaction_pointer) != -1)
	    record = NULL;
    }

    if (!record)
    {
	GtkTreeIter iter;

	if (!transaction_model_get_transaction_iter ( &iter,
						      transaction_number,
						      0 ))
	    return FALSE;
	record = iter.user_data;
    }

    /* si l'opération n'est pas visible on la rend visible */
    if ( ( record -> mother_row && record -> mother_row -> filtered_pos == -1 )
     ||
     record -> filtered_pos == -1 )
    {
        gint account_number;

        account_number = gsb_data_transaction_get_account_number ( transaction_number );
        gsb_data_account_set_r ( account_number, TRUE );
        gsb_menu_update_view_menu ( account_number );
        mise_a_jour_affichage_r ( TRUE );
    }

    return_value = transaction_list_select_record (record);

    /* if we are on a child, open the expander if necessary */
    path = gtk_tree_path_new ();

    if (record -> mother_row)
    {
	gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);
	gtk_tree_view_expand_row ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
				   path, FALSE );
    }
    else
	gtk_tree_path_append_index (path, record -> filtered_pos);

    /* move the tree view to the selection */
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
				   path, NULL,
				   FALSE, 0.0, 0.0 );
    gtk_tree_path_free (path);

    return return_value;
}


/**
 * return the selected transaction
 *
 * \param
 *
 * \return the transaction_number or -1 (white line) if problem
 * */
gint transaction_list_select_get ( void )
{
    CustomRecord *record;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE );

    record = custom_list -> selected_row;
    if (!record)
	return -1;

    return gsb_data_transaction_get_transaction_number (record -> transaction_pointer);
}


/**
 * move up the selection
 * this doesn't change the selection in the account, only on the model,
 * so need to update the account
 *
 * \param into_children		if TRUE and the previous transaction is a split and is opened,
 * 				the selection goes on the last child of the previous transactions,
 * 				else it goes directly on the mother
 *
 * \return TRUE : done, FALSE : no change
 * */
gboolean transaction_list_select_up ( gboolean into_children )
{
    CustomRecord *record;
    GtkTreePath *path;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE );

    record = custom_list -> selected_row;
    if (!record)
	/* there is no selection, should not happen
	 * to avoid to block grisbi, set on the white line */
	return transaction_list_select (-1);

    /* check if first record */
    if (record -> filtered_pos == 0
	&&
	!record -> mother_row)
	return FALSE;

    /* go to previous record */
    if (record -> mother_row)
    {
	/* the current selection is a child, go up or on the mother */
	if (record -> pos == 0)
	    /* we are on the first child, go to the mother */
	    record = record -> mother_row -> transaction_records[0];
	else
	    /* go to the prev child */
	    record = record -> mother_row -> children_rows[record -> pos - 1];
    }
    else
    {
	/* the current selection is a mother, we go on the prev mother or the prev child */

	record = custom_list -> visibles_rows[record -> filtered_pos -1];

	/* if the upper row is an archive, do nothin */
	if (record -> what_is_line == IS_ARCHIVE)
	    return FALSE;

	/* if the record is a mother, the expander is opened and into_children is TRUE,
	 * go to the last child */
	path = gtk_tree_path_new ();
	gtk_tree_path_append_index (path, record -> filtered_pos);

	if (record -> number_of_children && into_children
	    &&
	    gtk_tree_view_row_expanded (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()), path))
	    record = record -> children_rows[record -> number_of_children - 1];
	else
	    /* we want the mother, set record on the first row */
	    record = record -> transaction_records[0];

	gtk_tree_path_free (path);
    }

    /* now can remove the selection and select the new one */
    transaction_list_select_unselect ();
    transaction_list_select_record (record);

    /* move the tree view to the selection */
    path = gtk_tree_path_new ();
    if (record -> mother_row)
	gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);
    gtk_tree_path_append_index (path, record -> filtered_pos);
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
				   path, NULL,
				   FALSE, 0.0, 0.0 );
    gtk_tree_path_free (path);
    return TRUE;
}



/**
 * move down the selection
 * this doesn't change the selection in the account, only on the model,
 * so need to update the account
 *
 * \param into_children		if TRUE and the current selection has children and is opened,
 * 				the selection goes on the first child of that transaction
 * 				else it goes directly on the next mother
 *
 * \return TRUE : done, FALSE : no change
 * */
gboolean transaction_list_select_down ( gboolean into_children )
{
    CustomRecord *record;
    GtkTreePath *end_path_list;
    GtkTreePath *path;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE );

    /* the only way to forbid the move is if we are on the white line */
    record = custom_list -> selected_row;
    if (!record)
	/* there is no selection, should not happen
	 * to avoid to block grisbi, set on the white line */
	return transaction_list_select (-1);

    /* the way is different between the selected transaction is a child or a mother */
    if (record -> mother_row)
    {
	/* we are on a child, go to the next child or to the next mother */
	if (record -> pos == (record -> mother_row -> number_of_children - 1))
	    /* go to the next mother */
	    record = custom_list -> visibles_rows[record -> mother_row -> filtered_pos + 1];
	else
	    /* go to the next child */
	    record = record -> mother_row -> children_rows[record -> pos + 1];
    }
    else
    {
	/* we are on a mother, go into the first child or the next mother */

	/* if we are on the white line, go away */
	if (gsb_data_transaction_get_transaction_number (record -> transaction_pointer) == -1)
	    return FALSE;

	/* if we are on a split, and it is opened, and into_children i set, go
	 * to the first child */
	if (record -> number_of_children && into_children)
	{
	    path = gtk_tree_path_new ();
	    /* set the path on the mother row wich have the expander */
	    gtk_tree_path_append_index (path, record -> children_rows[0] -> mother_row -> filtered_pos);
	}
	else
	    path = NULL;

	if (path
	    &&
	    gtk_tree_view_row_expanded (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()), path))
	{
	    /* go to the first child */
	    record = record -> children_rows[0];
	    gtk_tree_path_free (path);
	}
	else
	    /* go to the next transaction mother */
	    record = custom_list -> visibles_rows[record -> filtered_pos + custom_list -> nb_rows_by_transaction];
    }

    /* now can remove the selection and select the new one */
    transaction_list_select_unselect ();
    transaction_list_select_record (record);

    /* move the tree view to the selection, we need to check the visible range and
     * all that stuff because gtk_tree_view_scroll_to_cell set the rows at the upper of the tree view
     * instead of keeping them at the bottom, when transaction goes down beside the bottom of the tree view */
    if ( gtk_tree_view_get_visible_range ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
					   NULL,
					   &end_path_list ))
    {
	path = gtk_tree_path_new ();

	if (record -> mother_row)
	{
	    gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);
	    gtk_tree_path_append_index (path, record -> filtered_pos);
	}
	else
	    /* check with the last visible line of the transaction */
	    gtk_tree_path_append_index (path, record -> transaction_records[custom_list -> nb_rows_by_transaction - 1] -> filtered_pos);

	if (gtk_tree_path_compare (end_path_list, path) == -1)
	    /* the last line of the transaction is above the last visible row */
	    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
					   path, NULL,
					   TRUE, 1.0, 0.0 );

	gtk_tree_path_free (path);
	gtk_tree_path_free (end_path_list);
    }
    return TRUE;
}


/**
 * return the path of the selected transaction
 * according line_in_transaction
 *
 * \param line_in_transaction
 *
 * \return a newly allocated GtkTreePath
 * */
GtkTreePath *transaction_list_select_get_path ( gint line_in_transaction )
{
    CustomRecord *record;
    GtkTreePath *path;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, NULL );

    record = custom_list -> selected_row;
    if (!record)
	return NULL;

    /* if not child, go to the first row */
    if (!record -> mother_row)
	record = record -> transaction_records[line_in_transaction];

    /* if something is wrong in line_in_transaction, it's possible we are not
     * on the same transaction */
    if (! record ||
	record -> transaction_pointer != custom_list -> selected_row -> transaction_pointer)
	return NULL;

    path = gtk_tree_path_new ();
    if (record -> mother_row)
	/* it's a child, need to get the path of the mother */
	gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);

    gtk_tree_path_append_index (path, record -> filtered_pos);

    return path;
}



/**
 * unselect the current selection of the custom list
 *
 * \param
 *
 * \return
 * */
void transaction_list_select_unselect (void)
{
    CustomRecord *record = NULL;
    GtkTreePath *path;
    GtkTreeIter iter;
    gint i;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_if_fail ( custom_list != NULL );
    g_return_if_fail ( custom_list -> selected_row != NULL );

    iter.stamp = custom_list->stamp;

    /* get the selected row */
    record = custom_list -> selected_row;

    if (record)
    {
        /* si l'opération n'est pas visible on sort */
        if ( ( record -> mother_row && record -> mother_row -> filtered_pos == -1 )
         ||
         record -> filtered_pos == -1 )
        {
            custom_list -> selected_row = NULL;
            return;
        }

        /* get the path of the row */
        path = gtk_tree_path_new ();
        if (record -> mother_row)
            /* it's a child, need to get the path of the mother */
            gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);

        gtk_tree_path_append_index (path, record -> filtered_pos);

        for (i=0 ; i < custom_list -> nb_rows_by_transaction ; i++)
        {
            record -> row_bg = record -> row_bg_save;
            record -> row_bg_save = NULL;

            /* inform the world that the row has changed */
            iter.user_data = record;
            gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);

            /* if the selection was a child, we stop now, only 1 line */
            if (record -> mother_row)
            break;

            /* go to the next row of the transaction */
            record = custom_list -> visibles_rows [record -> filtered_pos + 1];
            gtk_tree_path_next (path);
        }
        gtk_tree_path_free (path);
    }
    custom_list -> selected_row = NULL;
}


/**
 * select the transaction by the record
 * static function
 *
 * \param record	the CustomRecord to select
 *
 * \return TRUE ok, FALSE problem
 * */
static gboolean transaction_list_select_record ( CustomRecord *record )
{
    GtkTreePath *path;
    GtkTreeIter iter;
    gint i;
    gint selected_transaction;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE );
    g_return_val_if_fail ( record != NULL, FALSE );

    /* get the transaction number */
    selected_transaction = gsb_data_transaction_get_transaction_number (record -> transaction_pointer);

    /* record is the first row of the transaction to select */
    custom_list -> selected_row = record;

    /* get the path of the row we want to select */
    path = gtk_tree_path_new ();
    if ( record -> mother_row )
        /* it's a child, need to get the path of the mother */
        gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);

    gtk_tree_path_append_index (path, record -> filtered_pos);

    /* colorize the record */
    for (i=0 ; i < custom_list -> nb_rows_by_transaction ; i++)
    {
	record -> row_bg_save = record -> row_bg;
	record -> row_bg = &couleur_selection;

	/* inform the world that the row has changed */
	iter.user_data = record;
	gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);

	/* if the selection was a child, we stop now, only 1 line */
	if (record -> mother_row)
	    break;

	/* go to the next row of the transaction */
	record = custom_list -> visibles_rows [record -> filtered_pos + 1];
	gtk_tree_path_next (path);
    }

    /** update account and other stuff */
    gsb_transactions_list_selection_changed (selected_transaction);
    return TRUE;
}


