/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2008-2009 Pierre Biava (grisbi@pierre.biava.name)                 */
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
 * \file transaction_list.c
 * this contains the general functions to work with the CustomList
 * as append/remove some transactions, filter, colorize the list...
 * 
 * Grisbi shouldn't work directly on the CustomList except by those files
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "transaction_list.h"
#include "affichage_liste.h"
#include "custom_list.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "gsb_data_archive.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_currency.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "gsb_transactions_list_sort.h"
#include "navigation.h"
#include "structures.h"
#include "transaction_list_select.h"
#include "transaction_list_sort.h"
#include "transaction_model.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void transaction_list_append_child ( gint transaction_number );
static CustomRecord *transaction_list_create_record ( gint transaction_number,
                        gint line_in_transaction );
static gboolean transaction_list_update_white_child ( CustomRecord *white_record );
/*END_STATIC*/

/*START_EXTERN*/
extern GdkColor archive_background_color;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_jour;
extern gint display_one_line;
extern gint display_three_lines;
extern gint display_two_lines;
extern gsb_real null_real;
extern GSList *orphan_child_transactions;
extern GdkColor split_background;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];
extern GdkColor text_color[2];
extern GtkWidget *window;
/*END_EXTERN*/


/* this is a buffer wich contains the last mother appended, to increase
 * the speed when filling the list and adding children */
static CustomRecord *last_mother_appended = NULL;



/**
 * create the new custom list
 *
 * \param
 *
 * \return TRUE : ok, FALSE : pb while creating the list
 * */
gboolean transaction_list_create (void)
{
    CustomList *custom_list;

    custom_list = custom_list_new ();
    transaction_model_set_model (custom_list);

    return (custom_list != NULL);
}


/**
 * append a transaction to the list
 * that transaction can be a mother or a child (split)
 *
 * \param transaction_number    the transaction to append
 *
 * \return
 * */
void transaction_list_append_transaction ( gint transaction_number )
{
    gulong newsize;
    guint pos;
    gint account_number;
    CustomRecord *newrecord[TRANSACTION_LIST_ROWS_NB];
    gint i, j;
    gint line_p;
    gboolean marked_transaction;
    CustomList *custom_list;
    CustomRecord **children_rows = NULL;
    CustomRecord *white_record = NULL;
    GdkColor *mother_text_color;

    custom_list = transaction_model_get_model ();

    g_return_if_fail ( custom_list != NULL );

    /* if this is a child, go to append_child_record */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
        transaction_list_append_child (transaction_number);
        return;
    }

    /* the transaction is a mother */
    account_number = gsb_gui_navigation_get_current_account ( );

    /* if the transaction is a split, create a white line, we will append it later */
    if ( gsb_data_transaction_get_split_of_transaction ( transaction_number ) )
    {
	/* append a white line */
	gint white_line_number;
	gchar *amount_string;
	gchar *variance_string;

	white_line_number = gsb_data_transaction_new_white_line ( transaction_number );
	children_rows = g_malloc0 (sizeof (CustomRecord*));

	/* create and fill the white line record */
	white_record = g_malloc0 (sizeof (CustomRecord));
	white_record -> transaction_pointer = gsb_data_transaction_get_pointer_of_transaction (white_line_number);
	white_record -> what_is_line = IS_TRANSACTION;
	white_record -> row_bg = &split_background;

	/* as we append just now the white line, there are no child split, so the total is 0 */
	amount_string = gsb_real_get_string_with_currency (null_real,
							   gsb_data_transaction_get_currency_number (transaction_number), TRUE);
	variance_string = gsb_real_get_string_with_currency (gsb_data_transaction_get_amount (transaction_number),
							     gsb_data_transaction_get_currency_number (transaction_number), TRUE);
	white_record -> visible_col[2] = g_strdup_printf ( _("Total : %s (variance : %s)"),
							   amount_string,
							   variance_string );
	g_free (amount_string);
	g_free (variance_string);

	children_rows[0] = white_record;
    }

    /* get the P position if the transaction is marked */
    marked_transaction = gsb_data_transaction_get_marked_transaction (
                        transaction_number) != OPERATION_NORMALE;
    line_p = find_element_line (ELEMENT_MARK);

    /* get the new number of the first row in the complete list of row */
    pos = custom_list->num_rows;

    /* increase the table of pointer of struct CustomRecord */
    /* we add the 4 rows of the transaction in one time,
     * so increase 4*sizeof (CustomRecord *) */
    custom_list->num_rows = custom_list -> num_rows + TRANSACTION_LIST_ROWS_NB;
    newsize = custom_list->num_rows * sizeof(CustomRecord*);
    custom_list->rows = g_realloc(custom_list->rows, newsize);

    /* increase too the size of visibles rows, either if that row is not visible,
     * it's the only way to be sure to never go throw the end while filtering */
    custom_list->visibles_rows = g_realloc(custom_list->visibles_rows, newsize);

    /* now we can save the 4 new rows, ie the complete transaction */
    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
    {
	/* create the row */
	newrecord[i] = transaction_list_create_record (transaction_number, i);

	/* set the white line if necessary */
	if (children_rows)
	{
	    newrecord[i] -> number_of_children = 1;
	    newrecord[i] -> children_rows = children_rows;
	}

	/* save the newrecord pointer */
	custom_list->rows[pos] = newrecord[i];
	/* and the pos (number) of the row */
	newrecord[i]->pos = pos;

	/* set the checkbox is the transaction is marked */
	if (line_p == i)
	    newrecord[i] -> checkbox_active = marked_transaction;

	/* we check if the row is visible or not only if we are on an account,
	 * in other case, the model will be filtered when change account so no need
	 * to do that here */
	if ( account_number != -1
	    &&
	    gsb_transactions_list_transaction_visible ( newrecord[i] -> transaction_pointer,
							account_number,
							i,
							newrecord[i] -> what_is_line ))
	{
	    /* the row need to be shown */
	    GtkTreePath *path;
	    GtkTreeIter iter;
        gint last_line_visible;

	    newrecord[i] -> line_visible = TRUE;

        last_line_visible = transaction_list_get_last_line (
			            custom_list -> nb_rows_by_transaction );

	    newrecord[i] -> filtered_pos = custom_list -> num_visibles_rows;
	    custom_list -> visibles_rows[newrecord[i] -> filtered_pos] = newrecord[i];
	    custom_list -> num_visibles_rows++;

        /* if we are the last line visible and are mother, we set the expander */
	    if ( children_rows && i == last_line_visible )
	    {
            newrecord[i] -> has_expander = TRUE;
            white_record -> mother_row = newrecord[i];
            /* set the color of the mother */
            mother_text_color = &text_color[1];
            newrecord[i] -> text_color = mother_text_color;
	    }

	    /* inform the tree view */
	    path = gtk_tree_path_new();
	    gtk_tree_path_append_index(path, newrecord[i]->filtered_pos);

	    iter.stamp = custom_list -> stamp;
	    iter.user_data = newrecord[i];

	    gtk_tree_model_row_inserted (GTK_TREE_MODEL(custom_list), path, &iter);
	    /* if there is a child (white line), set the expander */
	    if ( newrecord[i] -> has_expander )
		    gtk_tree_model_row_has_child_toggled ( GTK_TREE_MODEL ( custom_list ),
						      path, &iter );
	    gtk_tree_path_free ( path );
	}
	else
	    newrecord[i] -> filtered_pos = -1;
	pos++;
    }

    /* in case of the transaction is not visible, we save in the white number
     * the record of the mother to avoid crash later */
    if (white_record && !white_record -> mother_row)
	white_record -> mother_row = newrecord[0];

    /* save the records in all the records of the transaction
     * this will increase the speed for associate the children to
     * the transaction mother (ie the 4 rows) */
    for (i=0 ; i < TRANSACTION_LIST_ROWS_NB ; i++)
	for (j=0 ; j < TRANSACTION_LIST_ROWS_NB ; j++)
	    newrecord[i] -> transaction_records[j] = newrecord[j];

    /* we save the adress of the last row in the buffer to increase speed if there is any child */
    last_mother_appended = newrecord[TRANSACTION_LIST_ROWS_NB - 1];
}


/**
 * append an archive to the list
 * create the CustomList if still not created
 *
 * \param archive_store_number    the archive to append
 *
 * \return
 * */
void transaction_list_append_archive (gint archive_store_number)
{
    gint archive_number;
    gulong newsize;
    guint pos;
    CustomRecord *newrecord;
    gint amount_col;
    CustomList *custom_list;
    gint col_archive;
    gint element_date = 0;

    custom_list = transaction_model_get_model ();

    if ( custom_list == NULL )
        return;

    /* get the new number of the first row in the complete list of row */
    pos = custom_list->num_rows;
    /* increase the table of pointer of struct CustomRecord,
     * 1 for the archive */
    custom_list->num_rows = custom_list -> num_rows + 1;
    newsize = custom_list->num_rows * sizeof(CustomRecord*);
    custom_list->rows = g_realloc(custom_list->rows, newsize);

    /* increase too the size of visibles rows, either if that row is not visible,
     * it's the only way to be sure to never go throw the end while filtering */
    custom_list->visibles_rows = g_realloc(custom_list->visibles_rows, newsize);

    /* create and fill the record */
    newrecord = g_malloc0 (sizeof (CustomRecord));

    archive_number = gsb_data_archive_store_get_archive_number (archive_store_number);

    if ( find_element_col ( ELEMENT_DATE ) == 0 )
        element_date = find_element_col_for_archive ( );

    newrecord -> visible_col[element_date] = gsb_format_gdate (
                        gsb_data_archive_get_beginning_date ( archive_number ) );

    if ( ( col_archive = find_element_col_for_archive ( ) ) >= 0 )
        newrecord -> visible_col[col_archive] = g_strdup_printf (
                        _("%s (%d transactions)"),
                        gsb_data_archive_get_name (archive_number),
                        gsb_data_archive_store_get_transactions_number (
                        archive_store_number ) );

    if ((gsb_data_archive_store_get_balance (archive_store_number)).mantissa < 0)
        amount_col = find_element_col (ELEMENT_DEBIT);
    else
        amount_col = find_element_col (ELEMENT_CREDIT);

    newrecord -> visible_col[amount_col] = gsb_real_get_string_with_currency (
                        gsb_data_archive_store_get_balance (archive_store_number),
                        gsb_data_account_get_currency (
                        gsb_data_archive_store_get_account_number ( archive_store_number ) ),
                        TRUE);
    newrecord -> transaction_pointer = gsb_data_archive_store_get_structure (
                        archive_store_number);
    newrecord -> what_is_line = IS_ARCHIVE;
    newrecord -> row_bg = &archive_background_color;

    /* save the newrecord pointer */
    custom_list->rows[pos] = newrecord;
    /* and the pos (number) of the row */
    newrecord->pos = pos;
}


/**
 * remove a transaction from the tree model
 * this doesn't change any filter or sort parameters,
 * so just need to colorize and set the balances after that
 *
 * WARNING : the transaction (and its children) must not be freed when come here
 *
 * NOTE : if the removed transaction is the selected transaction, the model won't have
 *         selected transaction anymore, so need to do selection_next before
 *
 * \param transaction_number    the transaction to remove
 *
 * \return TRUE : transaction removed, FALSE : problem, nothing done
 * */
gboolean transaction_list_remove_transaction ( gint transaction_number )
{
    CustomRecord *record;
    gint i;
    GtkTreeIter iter;
    gint record_filtered_pos;
    gulong newsize;
    GtkTreePath *path;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    if ( custom_list == NULL )
        return FALSE;

    /* get the record */
    if (!transaction_model_get_transaction_iter (&iter, transaction_number, 0))
        return FALSE;

    record = iter.user_data;
    if (!record)
        return FALSE;

    /* if selected transaction, remove it */
    if (record == custom_list -> selected_row)
	custom_list -> selected_row = NULL;

    /* if there are some children, remove them first */
    while (record -> number_of_children)
        transaction_list_remove_transaction (
                        gsb_data_transaction_get_transaction_number (
                        record -> children_rows[0] -> transaction_pointer ) );

    /* now we can delete the rows */
    for (i=0 ; i<CUSTOM_MODEL_N_VISIBLES_COLUMN ; i++)
	if (record -> visible_col[i])
	    g_free (record -> visible_col[i]);

    if (record -> mother_row)
    {
	/* we are deleting a child, update the mother
	 * the child can never be the last one of the mother because there is 
     * always a white line at the end */
	gint new_number_of_children;
	CustomRecord **new_children_rows;

	new_number_of_children = record -> mother_row -> number_of_children - 1;
	new_children_rows = record -> mother_row -> children_rows;

	for (i = record -> pos ; i < new_number_of_children ; i++)
	{
	    new_children_rows[i] = record -> mother_row -> children_rows[i+1];
	    new_children_rows[i] -> pos = i;
	    new_children_rows[i] -> filtered_pos = i;
	}

	/* free the end of array of pointers of children */
	newsize = new_number_of_children * sizeof (CustomRecord*);
	new_children_rows = g_realloc (new_children_rows,
				       newsize);

	/* update all the rows of the mother */
	for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
	{
	    record -> mother_row -> transaction_records[i] -> number_of_children = new_number_of_children;
	    record -> mother_row -> transaction_records[i] -> children_rows = new_children_rows;
	}

	/* update the white line */
	if ( transaction_number > 0 )
	    transaction_list_update_white_child (
                        record -> mother_row -> children_rows[new_number_of_children -1]);

	/* delete the row of the tree view */
	if (record -> mother_row -> filtered_pos != -1)
	{
	    path = gtk_tree_path_new ();
	    gtk_tree_path_append_index(path, record -> mother_row -> filtered_pos);
	    gtk_tree_path_append_index(path, record -> pos );
	    gtk_tree_model_row_deleted (GTK_TREE_MODEL(custom_list), path);
	    gtk_tree_path_free(path);
	}

	g_free (record);

	/* it's a child, so only one line, so we have finished here */
	return TRUE;
    }

    record_filtered_pos = record -> filtered_pos;

    /* we are on a mother transaction, remove the row */
    custom_list -> num_rows = custom_list -> num_rows - TRANSACTION_LIST_ROWS_NB;

    for (i=record -> pos ; i < custom_list -> num_rows ; i++)
    {
	custom_list -> rows[i] = custom_list -> rows[i+TRANSACTION_LIST_ROWS_NB];
	custom_list -> rows[i] -> pos = i;
    }

    /* if the transaction was showed, modify the visible transactions */
    if (record -> filtered_pos != -1)
    {
	custom_list -> num_visibles_rows = custom_list -> num_visibles_rows - custom_list -> nb_rows_by_transaction;

	for (i=record -> filtered_pos ; i < custom_list -> num_visibles_rows ; i++)
	{
	    custom_list -> visibles_rows[i] = custom_list -> visibles_rows[i + custom_list -> nb_rows_by_transaction];
	    custom_list -> visibles_rows[i] -> filtered_pos = i;
	}
    }

    /* resize the array */
    newsize = custom_list->num_rows * sizeof(CustomRecord*);
    custom_list->rows = g_realloc(custom_list->rows, newsize);
    custom_list->visibles_rows = g_realloc(custom_list->visibles_rows, newsize);

    /* free the records */
    for (i=TRANSACTION_LIST_ROWS_NB ; i ; i--)
    {
	if (record -> transaction_records[i-1] -> filtered_pos != -1)
	{
	    path = gtk_tree_path_new ();
	    gtk_tree_path_append_index(path, record -> transaction_records[i-1] -> filtered_pos);
	    gtk_tree_model_row_deleted (GTK_TREE_MODEL(custom_list), path);
	    gtk_tree_path_free(path);
	}
	g_free (record -> transaction_records[i-1]);
    }

    return TRUE;
}



/**
 * remove an archive from the tree model
 * it should be a good thing to update the tree_view after that
 *
 * \param archive_number    the archive to remove
 *
 * \return TRUE : archive removed, FALSE : problem, nothing done
 * */
gboolean transaction_list_remove_archive ( gint archive_number )
{
    gint i;
    CustomList *custom_list;
    gboolean return_val = FALSE;

    custom_list = transaction_model_get_model ();

    if ( custom_list == NULL )
        return FALSE;

    /* there is several archive store records which correspond to the archive */
    for (i=0 ; i < custom_list -> num_rows ; i++)
    {
        CustomRecord *record;
        gulong newsize;
        GtkTreePath *path;
        gint j;

        record = custom_list -> rows[i];
        if ( record -> what_is_line != IS_ARCHIVE
             ||
             gsb_data_archive_store_get_archive_number (gsb_data_archive_store_get_number (
                        record -> transaction_pointer )) != archive_number )
        {
            continue;
        }

        /* we are on a good archive store, delete it */

        /* delete the rows */
        for ( j=0 ; j<CUSTOM_MODEL_N_VISIBLES_COLUMN ; j++ )
            if (record -> visible_col[j])
                g_free (record -> visible_col[j]);
        
        /* remove the row. I decrement "i" because the next line of model is shifted
         * and has  "i" for index. Otherwise we do not test. */
        custom_list -> num_rows--;
        i--;

        for (j=record -> pos ; j < custom_list -> num_rows ; j++)
        {
            custom_list -> rows[j] = custom_list -> rows[j+1];
            custom_list -> rows[j] -> pos = j;
        }

        /* if the archive was showed, modify the visible transactions */
        if ( record -> filtered_pos != -1  && custom_list -> num_visibles_rows > 0 )
        {
            custom_list -> num_visibles_rows--;

            for (j=record -> filtered_pos ; j < custom_list -> num_visibles_rows ; j++)
            {
            custom_list -> visibles_rows[j] = custom_list -> visibles_rows[j + 1];
            custom_list -> visibles_rows[j] -> filtered_pos = j;
            }

            path = gtk_tree_path_new ();
            gtk_tree_path_append_index(path, record -> filtered_pos);
            gtk_tree_model_row_deleted (GTK_TREE_MODEL(custom_list), path);
            gtk_tree_path_free(path);
        }

        /* resize the array */
        newsize = custom_list->num_rows * sizeof(CustomRecord*);
        custom_list->rows = g_realloc(custom_list->rows, newsize);
        custom_list->visibles_rows = g_realloc(custom_list->visibles_rows, newsize);

        /* free the record */
        g_free (record);
        return_val = TRUE;
    }

    return return_val;
}


/**
 * this function is called to filter the model, according to account number
 * check each line of the list and mark it as visible or not
 * this function will initialize and fill visibles_rows and num_visibles_rows in the model
 *
 * usually, we need to call that function first, then transaction_list_sort, the transaction_list_colorize
 *
 * to update all the tree view, use gsb_transactions_list_update_tree_view instead
 *
 * \param account_number account we want the filter
 *
 * \return
 * */
void transaction_list_filter ( gint account_number )
{
    gint current_pos_general_list;
    gint current_pos_filtered_list = 0;
    GtkTreePath  *path;
    gint previous_visible_rows;
    GtkTreeIter iter;
    gint i;
    gint *neworder;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    devel_debug (NULL);

    g_return_if_fail ( custom_list != NULL );
    g_return_if_fail ( custom_list->num_rows != 0 );

    /* there is a bug, i think in gtk, wich when we re-filter the list with opened split, and when
     * there is less lines in the list that the window, gtk close the split opened without
     * informing the tree view so tree view errors laters... i didn't find anything here
     * wich close the split, so i assume is gtk. the solution is to close all the split.
     * this is very important to keep gtk_tree_view_collapse_all to avoid very nuts bugs !!!  */
    gtk_tree_view_collapse_all (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()));

    /* we erase the selection */
    if (custom_list -> selected_row)
	transaction_list_select_unselect ();

    custom_list -> nb_rows_by_transaction = gsb_data_account_get_nb_rows (account_number);

    /* begin to fill the iter for later */
    iter.stamp = custom_list->stamp;

    /* save the lenght of the current list */
    previous_visible_rows = custom_list -> num_visibles_rows;

    path = gtk_tree_path_new_first ();

    for (current_pos_general_list=0 ; current_pos_general_list<custom_list -> num_rows ; current_pos_general_list++)
    {
	CustomRecord *record;
	gboolean shown;
	gboolean previous_shown;
	guint last_pos_filtered_list;

	/* get the current record to check */
	record = custom_list -> rows[current_pos_general_list];

	/* was the line visible before ? */
	previous_shown = record -> line_visible;

	/* check if line is shown */
	shown = gsb_transactions_list_transaction_visible ( record -> transaction_pointer,
							    account_number,
							    record -> line_in_transaction,
							    record -> what_is_line );
	record -> line_visible = shown;

	/* if the line is shown, append it to the filtered parameters of the model */
	if (!shown)
	{
	    /* the row will be hidden, set it and continue to the next row */
	    /* the content of that row and children if exist will be changed with the
	     * next visible row */
	    record -> filtered_pos = -1;

	    /* if the row had some visible children, we hide the expander,
	     * either the transaction is visible but that row is not visible
	     * (line_in_transaction is bigger than visible line in account) and
	     * the children have already moved to previous row in that transaction,
	     * either the transaction is not visible anymore, and when we will show
	     * it again, has_expander will be set again at the good position */
	    record -> has_expander = FALSE;
	    continue;
	}

	/* the row will be showed */

	/* save the last position */
	last_pos_filtered_list = record -> filtered_pos;

	/* update the custom list with that row */
	record -> filtered_pos = current_pos_filtered_list;
	custom_list -> visibles_rows[current_pos_filtered_list] = record;
	current_pos_filtered_list++;
	custom_list -> num_visibles_rows = current_pos_filtered_list;

	/* set the iter */
	iter.user_data = record;

	switch (record -> what_is_line)
	{
	    case IS_ARCHIVE:
		/* we are on an  archive line, the only thing to know is
		 * if it is at the same place as before or not,
		 * if yes, just continue, if not, we will see later if
		 * we need to add or change the row */
		if (previous_shown && (record -> filtered_pos == last_pos_filtered_list))
		{
		    gtk_tree_path_next (path);
		    continue;
		}
		break;

	    case IS_TRANSACTION:
		/* the row is shown, if it's the last row chowed of the transaction
		 * we have to move the children to that row if they exist */
		if ( record -> number_of_children && 
		     record -> line_in_transaction == ( transaction_list_get_last_line (
											gsb_data_account_get_nb_rows (account_number) ) ) )
		{
		    /* ok, we are on the last visible line, there is some children and if before
		     * the children were not associated to that row, we have to move them */
		    if (record -> has_expander == FALSE)
		    {
                for (i=0 ; i < record -> number_of_children ; i++)
			    	((CustomRecord *) record -> children_rows[i]) -> mother_row = record;
			    record -> has_expander = TRUE;
		    }
		}
		else
		    record -> has_expander = FALSE;
        /* printf ("transaction_number = %d record -> filtered_pos = %d record -> line_in_transaction =%d\n",
                        gsb_data_transaction_get_transaction_number ( record -> transaction_pointer),
                        record -> filtered_pos,
                        record -> line_in_transaction); */

		/* the value of the row is set in memory, we update the tree view if necessary */
		/* if the record was already shown and the position hasn't change, nothing to do */
		if (previous_shown && (record -> filtered_pos == last_pos_filtered_list))
		{
		    /* the row itself didn't change but perhaps it got or losed some children
		     * in that case, the children were moved before, so we just have to 
		     * tell to the tree view that children changed */
		    gtk_tree_model_row_has_child_toggled (GTK_TREE_MODEL (custom_list),
							  path, &iter);
		    gtk_tree_path_next (path);
		    continue;
		}
		break;
	}

	/* now, the record is not at the same place as before (or was not shown before),
	 * so,
	 * either we modify the current row if it exits already,
	 * either we insert a new row if we reached the end */
	if (current_pos_filtered_list > previous_visible_rows)
	    /* we reached the end of list, need to add a row */
	    gtk_tree_model_row_inserted ( GTK_TREE_MODEL(custom_list),
					  path, &iter);
	else
	    /* ok, the end of list is not reached, we can just change the row */
	    gtk_tree_model_row_changed ( GTK_TREE_MODEL (custom_list),
					 path, &iter );

	/* the new row can have some children, or the last row (if we just changed the row)
	 * could have some children we don't want now. that function is magic because
	 * automatically update if there are children or not */
	if (record -> what_is_line == IS_TRANSACTION)
	    gtk_tree_model_row_has_child_toggled (GTK_TREE_MODEL (custom_list),
						  path, &iter);

	gtk_tree_path_next (path);
    }

    /* if the previous list was bigger than now, we need to delete some rows */
    if (previous_visible_rows > current_pos_filtered_list)
	for (i=0 ; i < (previous_visible_rows - current_pos_filtered_list) ; i++)
	    gtk_tree_model_row_deleted ( GTK_TREE_MODEL (custom_list),
					 path );
    gtk_tree_path_free(path);

    /* initial sort of the list */
    g_qsort_with_data(custom_list->visibles_rows,
                        custom_list->num_visibles_rows,
                        sizeof(CustomRecord*),
                        (GCompareDataFunc) gsb_transactions_list_sort_initial,
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
 * colorize transactions in the model
 *
 * to update all the tree view, use gsb_transactions_list_update_tree_view instead
 *
 * \param 
 *
 * \return
 * */
void transaction_list_colorize (void)
{
    gpointer current_transaction_pointer = NULL;
    gint current_color = 0;
    gint i, j;
    CustomList *custom_list;

    devel_debug (NULL);

    custom_list = transaction_model_get_model ();
    g_return_if_fail ( custom_list != NULL );

    /* we erase the selection */
    if (custom_list -> selected_row)
	transaction_list_select_unselect ();

    for (i=0 ; i < custom_list -> num_visibles_rows ; i++)
    {
	CustomRecord *record;

	record = custom_list -> visibles_rows[i];

	if (record -> what_is_line == IS_TRANSACTION)
	{
        gint transaction_number;

        /* update the color of the mother if necessary */
        transaction_number = gsb_data_transaction_get_transaction_number (
                        record -> transaction_pointer);
        if (gsb_data_transaction_get_split_of_transaction (transaction_number))
        {
            CustomRecord *white_record = NULL;

            if ( record -> number_of_children )
            {
                white_record = record -> children_rows[record -> number_of_children -1];
                transaction_list_update_white_child ( white_record );
            }
            else
            {
                /* set the color of the mother */
                for (j=0 ; j<TRANSACTION_LIST_ROWS_NB ; j++)
                    record -> transaction_records[j] -> text_color = &text_color[0];
            }

        }
	    /* if we changed of transaction, change the color */
	    if (record -> transaction_pointer != current_transaction_pointer)
	    {
		current_color = !current_color;
		current_transaction_pointer = record -> transaction_pointer;
	    }

	    /* set the color of the row */
	    record -> row_bg = &couleur_fond[current_color];
	}
    }
}

/**
 * fill the balance cell of the transactions in the sorted model
 *
 * to update all the tree view, use gsb_transactions_list_update_tree_view instead
 *
 * \param
 *
 * \return
 * */
void transaction_list_set_balances ( void )
{
    gint account_number;
    gsb_real current_total;
    gint column_balance;
    gint line_balance;
    gint nb_rows;
    gint floating_point;
    gint currency_number;
    gint i;
    GtkTreeIter iter;
    GtkTreePath *path;
    gpointer last_transaction_pointer = NULL;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    devel_debug (NULL);

    g_return_if_fail ( custom_list != NULL );

    /* column and line of balance are user defined */
    column_balance = find_element_col (ELEMENT_BALANCE);
    line_balance = find_element_line (ELEMENT_BALANCE);

    /* check if the balance is visible */
    account_number = gsb_gui_navigation_get_current_account ();
    nb_rows = gsb_data_account_get_nb_rows (account_number);

    if ( account_number == -1
	 ||
	 line_balance == -1 
	 ||
	 !display_mode_check_line ( line_balance, nb_rows ) )
	return;

    /* begin to fill the iter for later */
    iter.stamp = custom_list->stamp;

    currency_number = gsb_data_account_get_currency ( account_number );
    floating_point = gsb_data_currency_get_floating_point ( currency_number );

    /* get the beginning balance */
    current_total = solde_debut_affichage ( account_number, floating_point);

    for (i=0 ; i < custom_list -> num_visibles_rows ; i++)
    {
        CustomRecord *record;
        gsb_real amount = null_real;

        record = custom_list -> visibles_rows[i];

        /* a transaction is several rows, and an archive only one row
         * we come only one time for each transaction/archive */
        if (record -> transaction_pointer == last_transaction_pointer
            ||
            gsb_data_transaction_get_transaction_number (record -> transaction_pointer) < 0 )
            continue;
        last_transaction_pointer = record -> transaction_pointer;

        switch (record -> what_is_line)
        {
            case IS_ARCHIVE:
            amount = gsb_data_archive_store_get_balance (
                            gsb_data_archive_store_get_number (record -> transaction_pointer));
            break;

            case IS_TRANSACTION:
            amount = gsb_data_transaction_get_adjusted_amount (
                            gsb_data_transaction_get_transaction_number (
                            record -> transaction_pointer),
                            floating_point);
            /* go on the good row to set the amount */
            record = record -> transaction_records[line_balance];
            break;
        }

        /* calculate the new balance */
        current_total = gsb_real_add ( current_total, amount );
        record -> visible_col[column_balance] = gsb_real_get_string_with_currency ( current_total,
                                                 currency_number , TRUE);
        if (current_total.mantissa >= 0)
            record -> amount_color = NULL;
        else
            record -> amount_color = "red";

        /* inform the tree view the row has changed */
        /* set the iter */
        iter.user_data = record;
        path = gtk_tree_path_new();
        gtk_tree_path_append_index(path, record->filtered_pos);
        gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);
        gtk_tree_path_free(path);
    }

    /* update the headings balance */
    gsb_data_account_colorize_current_balance ( account_number );
}



/**
 * update a transaction on the model
 * avoid to change it with transaction_list_set
 * it's slower but lower code... and now, slower is not really visible,
 * so use that function instead of transaction_list_set
 *
 * this function modify just the transaction on the view,
 * call gsb_transactions_list_update_transaction will make all the other necessary changes
 *     when a transaction is modified
 *
 * \param transaction_number
 *
 * \return TRUE : transaction updated, FALSE : problem, nothing done
 * */
gboolean transaction_list_update_transaction ( gint transaction_number )
{
    CustomRecord *record;
    gint i, j;
    GtkTreeIter iter;
    gint nb_rows;
    gint line_p;
    gboolean marked_transaction;
    CustomList *custom_list;
    CustomRecord *white_record = NULL;
    CustomRecord **children_rows = NULL;

    devel_debug_int (transaction_number);

    if ( transaction_number == 0 )
        return FALSE;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE);

    /* we can update a transaction wich is not visible, so need to find the iter into
     * all the transactions */
    if (!transaction_model_get_transaction_iter (&iter, transaction_number, 0))
	return FALSE;

    record = iter.user_data;
    if (!record)
	return FALSE;
    /* printf ("transaction_number = %d record -> filtered_pos = %d record -> line_in_transaction = %d\n",
        transaction_number,
        record -> filtered_pos,
        record -> line_in_transaction); */

    /* if the transaction is a split, we need to check if there are already children,
     * else we add the white line */
    if (gsb_data_transaction_get_split_of_transaction (transaction_number)
	&&
	!record -> number_of_children )
    {
	/* there is no children, we add a white line */
	gint white_line_number;
	gchar *amount_string;
	gchar *variance_string;

	white_line_number = gsb_data_transaction_new_white_line (transaction_number);
	children_rows = g_malloc0 (sizeof (CustomRecord*));

	/* create and fill the white line record */
	white_record = g_malloc0 (sizeof (CustomRecord));
	white_record -> transaction_pointer = gsb_data_transaction_get_pointer_of_transaction (
                        white_line_number);
	white_record -> what_is_line = IS_TRANSACTION;
	white_record -> row_bg = &split_background;

	/* as we append just now the white line, there are no child split, so the total is 0 */
	amount_string = gsb_real_get_string_with_currency (null_real,
							   gsb_data_transaction_get_currency_number (transaction_number), TRUE);
	variance_string = gsb_real_get_string_with_currency (gsb_data_transaction_get_amount (transaction_number),
							     gsb_data_transaction_get_currency_number (transaction_number), TRUE);
	white_record -> visible_col[2] = g_strdup_printf ( _("Total : %s (variance : %s)"),
							   amount_string,
							   variance_string );
	g_free (amount_string);
	g_free (variance_string);

	children_rows[0] = white_record;
    }

    /* get the P position if the transaction is marked */
    marked_transaction = gsb_data_transaction_get_marked_transaction (transaction_number) != OPERATION_NORMALE;
    line_p = find_element_line (ELEMENT_MARK);

    /* if we update a child, only 1 line */
    if (record -> mother_row)
	nb_rows = 1;
    else
	nb_rows = TRANSACTION_LIST_ROWS_NB;

    /* now we can save the new rows */
    for (i=0 ; i<nb_rows ; i++)
    {
	CustomRecord *tmp_record;

	/* get the good line in the record */
	if (!record -> mother_row)
	    record = record -> transaction_records[i];

        if ( !record )
            return FALSE;

        /* the tmp record will contain the new gchars of columns */
        tmp_record = transaction_list_create_record (transaction_number, i);

        for (j=0 ; j<CUSTOM_MODEL_N_VISIBLES_COLUMN ; j++)
        {
            if (record -> visible_col[j])
                g_free (record -> visible_col[j]);
            record -> visible_col[j] = tmp_record -> visible_col[j];
        }

        g_free (tmp_record);

        /* set the white line if necessary */
        if (children_rows)
        {
            record -> number_of_children = 1;
            record -> children_rows = children_rows;
        }

        /* set the checkbox is the transaction is marked */
        if (line_p == i)
            record -> checkbox_active = marked_transaction;

        /* inform the tree view we changed the row, only if visible */
        if (record -> filtered_pos != -1)
        {
            GtkTreePath *path = gtk_tree_path_new();

            /* if there is some children and we are the last row, set the expander */
            if ( white_record
            &&
            i == ( transaction_list_get_last_line ( custom_list -> nb_rows_by_transaction ) ) )
            {
                record -> has_expander = TRUE;
                white_record -> mother_row = record;
            }

            /* set the path */
            if (record -> mother_row)
            /* it's a child, need to get the path of the mother */
            gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);
            gtk_tree_path_append_index(path, record->filtered_pos);

            /* set the iter */
            iter.user_data = record;

            /* update the transaction */
            gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);

            /* if there is a child (white line), set the expander */
            if (record -> has_expander)
            gtk_tree_model_row_has_child_toggled (GTK_TREE_MODEL (custom_list),
                                  path, &iter);
            gtk_tree_path_free(path);
        }
    }

    /* if the modification is on a child or a mother, update the white line */
    if (record -> mother_row || record -> number_of_children)
    {
        /* we need now to recalculate the amount of split and update the white line */
        CustomRecord *mother_record;
        CustomRecord *white_record;

        /* get the mother if child */
        if (record -> mother_row)
            mother_record = record -> mother_row;
        else
            mother_record = record;
        white_record = mother_record -> children_rows[mother_record -> number_of_children -1];

        transaction_list_update_white_child (white_record);
    }

    /* set the checkbox is the transaction is marked */
    if (line_p == i)
        record -> checkbox_active = marked_transaction;

    /* inform the tree view we changed the row, only if visible */
    if (record -> filtered_pos != -1)
    {
        GtkTreePath *path = gtk_tree_path_new();

        /* if there is some children and we are the last row, set the expander */
        if ( white_record
         &&
         i == ( transaction_list_get_last_line ( custom_list -> nb_rows_by_transaction ) ) )
        {
        record -> has_expander = TRUE;
        white_record -> mother_row = record;
        }

        /* set the path */
        if (record -> mother_row)
        /* it's a child, need to get the path of the mother */
        gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);
        gtk_tree_path_append_index(path, record->filtered_pos);

        /* set the iter */
        iter.user_data = record;

        /* update the transaction */
        gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);

        /* if there is a child (white line), set the expander */
        if (record -> has_expander)
        gtk_tree_model_row_has_child_toggled (GTK_TREE_MODEL (custom_list),
                              path, &iter);
        gtk_tree_path_free(path);
    }

    if (nb_rows == 1)
    {
        CustomRecord *white_record;

        /* we need now to recalculate the amount of split and update the white line */
        white_record = record -> mother_row -> children_rows[record -> 
                            mother_row -> number_of_children -1];
        transaction_list_update_white_child (white_record);
    }

    return TRUE;
}


/**
 * update the given element for all the transactions in the list
 * an element is something showed in the list, as date, payee...
 * to update a column not showed, as color, font...
 *     use instead transaction_list_update_column
 *
 * the elements are defined in gsb_transactions_list.h
 *
 * \param element_number    the element to update
 *
 * \return FALSE : problem, nothing done ; TRUE : ok
 * */
gboolean transaction_list_update_element ( gint element_number )
{
    gint column_element;
    gint line_element;

    /* for now, this is the same position for all accounts, so no problem */

    /* get the position of the element */
    column_element = find_element_col (element_number);        /*  0 to CUSTOM_MODEL_N_VISIBLES_COLUMN */
    line_element = find_element_line (element_number);        /*  0 to TRANSACTION_LIST_ROWS_NB */

    if ( column_element == -1
	 ||
	 line_element == -1 )
	return FALSE;

    return transaction_list_update_cell ( column_element, line_element );
}


/**
 * update the column of all the record with the value
 * this is used to update some non seen fixed columns, as font for example
 * to update some visible columns, use instead transaction_list_update_element
 * 
 * the columns are defined in custom_list.h
 *
 * \param column    the column of the value we want to update
 * \param value        the value to set in the column
 *
 * \return FALSE : problem, nothing done ; TRUE : ok
 * */
gboolean transaction_list_update_column ( gint column,
                        GValue *value )
{
    gint i;
    GtkTreeIter iter;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail (G_IS_VALUE (value), FALSE);
    g_return_val_if_fail (value != NULL, FALSE);
    g_return_val_if_fail (column < CUSTOM_LIST(custom_list)->n_columns, FALSE);

    /* check first if value is correction for the column */
    switch(column)
    {
	case CUSTOM_MODEL_COL_0:
	case CUSTOM_MODEL_COL_1:
	case CUSTOM_MODEL_COL_2:
	case CUSTOM_MODEL_COL_3:
	case CUSTOM_MODEL_COL_4:
	case CUSTOM_MODEL_COL_5:
	case CUSTOM_MODEL_COL_6:
	    devel_debug ("update column for visible column, shouldn't use that function. Blocked for now.");
	    return FALSE;
	    /*         g_return_val_if_fail (G_VALUE_TYPE (value) == G_TYPE_STRING, FALSE); */
	    break;
	case CUSTOM_MODEL_BACKGROUND:
	case CUSTOM_MODEL_SAVE_BACKGROUND:
	    g_return_val_if_fail (G_VALUE_TYPE (value) == G_TYPE_BOXED, FALSE);
	    break;
	case CUSTOM_MODEL_FONT:
	case CUSTOM_MODEL_AMOUNT_COLOR:
	    g_return_val_if_fail (G_VALUE_TYPE (value) == G_TYPE_STRING, FALSE);
	    break;
	case CUSTOM_MODEL_TRANSACTION_ADDRESS:
	    g_return_val_if_fail (G_VALUE_TYPE (value) == G_TYPE_POINTER, FALSE);
	    break;
	case CUSTOM_MODEL_WHAT_IS_LINE:
	case CUSTOM_MODEL_TRANSACTION_LINE:
	    g_return_val_if_fail (G_VALUE_TYPE (value) == G_TYPE_INT, FALSE);
	    break;
	case CUSTOM_MODEL_VISIBLE:
	case CUSTOM_MODEL_CHECKBOX_VISIBLE:
	case CUSTOM_MODEL_CHECKBOX_VISIBLE_RECONCILE:
	case CUSTOM_MODEL_CHECKBOX_ACTIVE:
	    g_return_val_if_fail (G_VALUE_TYPE (value) == G_TYPE_BOOLEAN, FALSE);
	    break;
	default:
	    devel_debug ("Wrong column asked. Should not come here...");
    }

    /* value is correct, update the model */
    iter.stamp = custom_list -> stamp;

    for (i=0 ; i < custom_list -> num_rows ; i++)
    {
	CustomRecord  *record;

	record = custom_list -> rows[i];
	iter.user_data = record;
	custom_list_set_value (GTK_TREE_MODEL (custom_list),
			       &iter, column,
			       value );

	if (record -> number_of_children)
	{
	    gint j;
	    for (j=0 ; j < record -> number_of_children ; j++)
	    {
		record = record -> children_rows[j];
		iter.user_data = record;
		custom_list_set_value (GTK_TREE_MODEL (custom_list),
				       &iter, column,
				       value );
	    }
	}
    }
    return TRUE;
}


/**
 * update a cell with it's element
 * usually you need transaction_list_update_element wich will update all the cells of the element
 * this function is called to erase some cell after moving the element
 * (ie the notes element has moved the cell location, we need to erase the last cells,
 *     so we call this with the last cell_col and cell_line
 *     In that case, be sure to set before tab_affichage_ope[cell_col][cell_line] to 0, else the last cells
 *     will be re-written without change )
 *
 * \param cell_col    the column of the element we want to update
 * \param cell_line    the line of the element we want to update
 *
 * \return FALSE : problem, nothing done ; TRUE : ok
 * */
gboolean transaction_list_update_cell ( gint cell_col,
                        gint cell_line )
{
    gint element_number;
    gint column_element_split;
    GtkTreeIter iter;
    gint i;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    element_number = tab_affichage_ope[cell_line][cell_col];

    /* the element exists in the view, find the column of the split if exists (-1 if don't exist) */
    column_element_split = find_element_col_split (element_number);

    /* begin to fill the iter for later */
    iter.stamp = custom_list->stamp;

    for (i=0 ; i<custom_list -> num_rows ; i++)
    {
	CustomRecord *record;
	gint transaction_number;
	GtkTreePath *path;

	record = custom_list -> rows[i];
	if (record -> what_is_line == IS_ARCHIVE
	    ||
	    record -> line_in_transaction != cell_line )
	    continue;

	transaction_number = gsb_data_transaction_get_transaction_number (record -> transaction_pointer);
	if (transaction_number == -1)
	    continue;

	/* now, we are on the good row of the transaction, update the element */
	if (record -> visible_col[cell_col])
	    g_free (record -> visible_col[cell_col]);
	if (element_number)
	    record -> visible_col[cell_col] = gsb_transactions_list_grep_cell_content (transaction_number, element_number);
	else
	    record -> visible_col[cell_col] = NULL;

	/* inform the tree view we changed the row, only if visible */
	if (record -> filtered_pos != -1)
	{
	    path = gtk_tree_path_new();
	    gtk_tree_path_append_index(path, record->filtered_pos);
	    iter.user_data = record;
	    gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);
	    gtk_tree_path_free(path);
	}

	/* if the transaction has children and they contain too the element, update them */
	if (column_element_split != -1
	    &&
	    record -> number_of_children )
	{
	    gint j;

	    for (j=0 ; j<record -> number_of_children ; j++)
	    {
		CustomRecord *child_record;

		child_record = record -> children_rows[j];
		transaction_number = gsb_data_transaction_get_transaction_number (child_record -> transaction_pointer);
		if (transaction_number < 0)
		    continue;

		/* update the element */
		if (child_record -> visible_col[column_element_split])
		    g_free (child_record -> visible_col[column_element_split]);
		child_record -> visible_col[column_element_split] = gsb_transactions_list_grep_cell_content (transaction_number, element_number);

		/* inform the tree view we changed the row, only if visible
		 * we check the mother because the children are alway visible */
		if (record -> filtered_pos != -1)
		{
		    path = gtk_tree_path_new();

		    /* it's a child, need to get the path of the mother */
		    gtk_tree_path_append_index (path, record -> filtered_pos);
		    gtk_tree_path_append_index(path, child_record->pos);
		    iter.user_data = child_record;

		    gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);
		    gtk_tree_path_free(path);
		}
	    }
	}
    }
    return TRUE;
}


/**
 * show or hide the toggle button on the P column
 *
 * \param show    TRUE/FALSE
 *
 * \return TRUE : ok, FALSE : pb, nothing done
 * */
gboolean transaction_list_show_toggle_mark ( gboolean show )
{
    gint line_p, col_p;
    GtkTreeIter iter;
    gint i;
    CustomList *custom_list;

    if ( show )
        /* remet le gtk_tree_view en mode normal */
        gtk_tree_view_set_fixed_height_mode ( GTK_TREE_VIEW (
                        gsb_transactions_list_get_tree_view ( ) ), FALSE );
    else
        /* remet le gtk_tree_view en mode ligne fixe */
        gtk_tree_view_set_fixed_height_mode ( GTK_TREE_VIEW (
                        gsb_transactions_list_get_tree_view ( ) ), TRUE );

    custom_list = transaction_model_get_model ();

    line_p = find_element_line (ELEMENT_MARK);
    col_p = find_element_col (ELEMENT_MARK);
    if (line_p == -1 || col_p == -1)
	return FALSE;

    /* begin to fill the iter for later */
    iter.stamp = custom_list->stamp;

    for (i=0 ; i<custom_list -> num_rows ; i++)
    {
	CustomRecord *record;
	GtkTreePath *path;

	record = custom_list -> rows[i];
	if (record -> what_is_line == IS_ARCHIVE
	    ||
	    record -> line_in_transaction != line_p
	    ||
	    gsb_data_transaction_get_transaction_number (record -> transaction_pointer) == -1)
	    continue;

	record -> checkbox_visible = show;

	/* inform the tree view we changed the row, only if visible */
	if (record -> filtered_pos != -1)
	{
	    path = gtk_tree_path_new();
	    gtk_tree_path_append_index(path, record->filtered_pos);
	    iter.user_data = record;
	    gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);
	    gtk_tree_path_free(path);
	}
    }
    return TRUE;
}



/*
 * Sets the value of one or more cells in the row referenced by iter.
 * The variable argument list should contain integer column numbers, each column number followed by the value to be set.
 * The list is terminated by a -1
 * 
 * \param iter        iter of the row

 * \param column    number of column
 * \param value        value for the column (gchar *, GdkColor, gpointer, gint, PangoFontDescription, gboolean)
 * ...
 * \para -1        finish the list of cells to modify
 *
 * \return
 * */
void transaction_list_set ( GtkTreeIter *iter, ... )
{
    va_list var_args;
    gint column;
    CustomRecord  *record;
    GtkTreePath  *path;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    g_return_if_fail ( custom_list != NULL );
    g_return_if_fail ( custom_list->num_rows != 0 );

    /* get the CustomRecord structure */
    record = (CustomRecord*) iter->user_data;

    g_return_if_fail ( record != NULL );

    /* record can be a mother or a child */
    if((!record -> mother_row && record->pos >= custom_list->num_rows) /* mother test */
       ||
       (record -> mother_row && record -> pos >= record -> mother_row -> number_of_children )) /* child test */
	g_return_if_reached();

    /* set all the args */
    va_start (var_args, iter);

    /* first get the column */
    column = va_arg (var_args, gint);

    while (column != -1)
    {
	if (column < 0 || column >= CUSTOM_MODEL_N_COLUMNS)
	{
	    g_warning ("%s: Invalid column number %d added to iter (remember to end your list of columns with a -1)", G_STRLOC, column);
	    break;
	}

	switch (column)
	{
	    case CUSTOM_MODEL_COL_0:
	    case CUSTOM_MODEL_COL_1:
	    case CUSTOM_MODEL_COL_2:
	    case CUSTOM_MODEL_COL_3:
	    case CUSTOM_MODEL_COL_4:
	    case CUSTOM_MODEL_COL_5:
	    case CUSTOM_MODEL_COL_6:
		record -> visible_col[column] = va_arg (var_args, gchar *);
		break;
	    case CUSTOM_MODEL_BACKGROUND:
		record -> row_bg = va_arg (var_args, GdkColor *);
		break;
	    case CUSTOM_MODEL_AMOUNT_COLOR:
		record -> amount_color = va_arg (var_args, gchar *);
		break;
	    case CUSTOM_MODEL_TRANSACTION_ADDRESS:
		record -> transaction_pointer = va_arg (var_args, gpointer);
		break;
	    case CUSTOM_MODEL_WHAT_IS_LINE:
		record -> what_is_line = va_arg (var_args, gint);
		break;
	    case CUSTOM_MODEL_SAVE_BACKGROUND:
		record -> row_bg_save = va_arg (var_args, GdkColor *);
		break;
	    case CUSTOM_MODEL_FONT:
		record -> font = va_arg (var_args, gchar *);
		break;
	    case CUSTOM_MODEL_TRANSACTION_LINE:
		record -> line_in_transaction = va_arg (var_args, gint);
		break;
	    case CUSTOM_MODEL_VISIBLE:
		/* this value should not be changed here, check later if i'm right,
		 * for now just show a message */
		devel_debug (_("Try to change a visible line by transaction_list_set. It shouldn't append ! Check that."));
		/*         record -> line_visible = va_arg (var_args, gboolean); */
		break;
	    case CUSTOM_MODEL_CHECKBOX_VISIBLE:
		record -> checkbox_visible = va_arg (var_args, gboolean);
		break;
	    case CUSTOM_MODEL_CHECKBOX_VISIBLE_RECONCILE:
		record -> checkbox_visible_reconcile = va_arg (var_args, gboolean);
		break;
	    case CUSTOM_MODEL_CHECKBOX_ACTIVE:
		record -> checkbox_active = va_arg (var_args, gboolean);
		break;
	}
	column = va_arg (var_args, gint);
    }

    va_end (var_args);

    /* inform the tree view and other interested objects
     *  (e.g. tree row references) that we have inserted
     *  a new row, and where it was inserted */
    path = gtk_tree_path_new();
    gtk_tree_path_append_index(path, record->filtered_pos);
    gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, iter);
    gtk_tree_path_free(path);
}


/**
 * return the number of children of the transaction, in the tree view
 * so the white line is count into
 *
 * \param transaction_number    the split we want the children number
 *
 * \return the number of children or 0 if not a split
 * */
gint transaction_list_get_n_children ( gint transaction_number )
{
    CustomRecord *record;
    CustomList *custom_list;
    GtkTreeIter iter;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, 0);

    if (!gsb_data_transaction_get_split_of_transaction (transaction_number))
	return 0;

    if (!transaction_model_get_transaction_iter (&iter, transaction_number, 0))
	return 0;

    record = iter.user_data;
    if (!record)
	return 0;

    return record -> number_of_children;
}


/**
 * send a "row-changed" to all the row of the showed transactions,
 * so in fact re-draw the list and colors
 *
 * \param
 *
 * \return
 * */
gboolean transaction_list_redraw ( void )
{
    CustomList *custom_list;
    gint i;
    GtkTreeIter iter;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, 0);

    iter.stamp = custom_list -> stamp;

    for ( i=0 ; i < custom_list -> num_visibles_rows ; i++)
    {
	CustomRecord *record;
	GtkTreePath *path;

	record = custom_list -> visibles_rows[i];

	path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, record->filtered_pos);
	iter.user_data = record;
	gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);
	gtk_tree_path_free(path);

	if (record -> number_of_children)
	{
	    gint j;

	    for (j=0 ; j<record -> number_of_children ; j++)
	    {
		CustomRecord *child_record;

		child_record = record -> children_rows[j];

		path = gtk_tree_path_new();

		/* it's a child, need to get the path of the mother */
		gtk_tree_path_append_index (path, record -> filtered_pos);
		gtk_tree_path_append_index(path, child_record->pos);
		iter.user_data = child_record;

		gtk_tree_model_row_changed(GTK_TREE_MODEL(custom_list), path, &iter);
		gtk_tree_path_free(path);
	    }
	}
    }
    return FALSE;
}


/**
 * append a child to the mother in the custom list
 * this function is called internally by transaction_list_append_transaction
 *
 * \param transaction_number     the child to append
 *
 * \return
 * */
static void transaction_list_append_child ( gint transaction_number )
{
    gulong newsize;
    guint pos;
    gint i;
    CustomRecord *newrecord;
    CustomRecord *mother_record;
    GtkTreeIter mother_iter;
    CustomRecord **new_children_rows;
    gint new_number_of_children;
    CustomList *custom_list;
    CustomRecord *white_record;

    custom_list = transaction_model_get_model ();

    /* the children are stored into a table of pointer into each mother
     * the filter will determine later what mother row will have the
     * expander according to the number of row showed */

    /* create the row */
    newrecord = transaction_list_create_record (transaction_number, 0);

    /* find 1 of the mother row (the others are into the structure of the mother)
     * usually, it's the last record appended */
    mother_record = last_mother_appended;

    if ( !mother_record
	 ||
	 (gsb_data_transaction_get_transaction_number (mother_record -> transaction_pointer)
	  !=
	  gsb_data_transaction_get_mother_transaction_number (transaction_number)))
    {
        /* the last record appended was not the mother, need to search into the entire list */
        if ( transaction_model_get_transaction_iter ( &mother_iter,
                        gsb_data_transaction_get_mother_transaction_number (
                        transaction_number),
                        0 ) )
            mother_record = (CustomRecord *) mother_iter.user_data;
        else
        {
            /* we didn't find the mother of the child, we append the child number to
             * orphan_child_transactions and the function wich called here has to play
             * with it
             * (it happens in the versions before 0.6 when the children could have a number
             * before the mother */
            orphan_child_transactions = g_slist_append ( orphan_child_transactions,
                                 GINT_TO_POINTER (transaction_number));
            return;
        }
    }

    /* ok, at this level, mother_record is ok and is one of the row of the mother transaction
     * we go on the first mother */
    mother_record = mother_record -> transaction_records[0];

    /* set by default the mother row of the child. avoids a crash when creating 
     * an archive directly on the home page */
    newrecord -> mother_row = mother_record;

    /* get the new number of the row into the mother */
    pos = mother_record -> number_of_children;

    /* it must have a child, because while creating the mother, it makes it
     * so if no child, better to stop here */
    if (!pos)
    {
	gchar *tmpstr;
	tmpstr = g_strdup_printf (
                        _("Trying to append the child number %d to the mother %d "
                        "in the model, but no white line was created before... "
                        "Better to stop here, please contact the Grisbi team to "
                        "fix that issue."),
                        transaction_number, gsb_data_transaction_get_mother_transaction_number (transaction_number));
	dialogue_error (tmpstr);
	g_free (tmpstr);
	g_free (newrecord);
	return;
    }

    /* increase the children rows */
    new_number_of_children = mother_record -> number_of_children + 1;
    newsize = new_number_of_children * sizeof (CustomRecord*);
    new_children_rows = g_realloc (mother_record -> children_rows,
				   newsize);

    /* save the new record before the white line */
    white_record = new_children_rows[pos -1];

    new_children_rows[pos - 1] = newrecord;
    newrecord -> pos = pos - 1;
    newrecord -> filtered_pos = pos -1;

    new_children_rows[pos] = white_record;
    white_record -> pos = pos;
    white_record -> filtered_pos = pos;

    /* set the color */
    newrecord -> row_bg = &split_background;

    /* we can now save the new number of children and the new children rows into their mothers */
    for (i=0 ; i < TRANSACTION_LIST_ROWS_NB ; i++)
    {
	mother_record -> transaction_records[i] -> children_rows = new_children_rows;
	mother_record -> transaction_records[i] -> number_of_children = new_number_of_children;

	/* set the good mother row of the child */
	if (mother_record -> transaction_records[i] -> has_expander)
	{
	    newrecord -> mother_row = mother_record -> transaction_records[i];
	    white_record -> mother_row = mother_record -> transaction_records[i];
	}
    }

    /* we need now to recalculate the amount of split and update the white line */
    transaction_list_update_white_child (white_record);

    if (mother_record -> filtered_pos != -1)
    {
	/* the mother is visible, inform the tree view we append a child */
	GtkTreePath *path;
	GtkTreeIter iter;
    gint last_line;

	/* we go on the last mother record, wich contains the expander */
    last_line = transaction_list_get_last_line ( custom_list -> nb_rows_by_transaction );
	mother_record = mother_record -> transaction_records[last_line];

	newrecord -> mother_row = mother_record;
	/* this is very important to keep to compatibility with the normal transactions */
	newrecord -> transaction_records[0] = newrecord;

	/* update the new line, wich replace the white line */
	path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, mother_record->filtered_pos);
	gtk_tree_path_append_index(path, newrecord->filtered_pos);

	iter.stamp = custom_list -> stamp;
	iter.user_data = newrecord;

	gtk_tree_model_row_changed (GTK_TREE_MODEL(custom_list), path, &iter);
	gtk_tree_path_free(path);

	/* update white line, wich moved to the end */
	path = gtk_tree_path_new();
	gtk_tree_path_append_index(path, mother_record->filtered_pos);
	gtk_tree_path_append_index(path, white_record->filtered_pos);

	iter.stamp = custom_list -> stamp;
	iter.user_data = white_record;

	gtk_tree_model_row_inserted (GTK_TREE_MODEL(custom_list), path, &iter);
	gtk_tree_path_free(path);
    }
}



/**
 * create a new record structure according to the line in transaction and return it
 *
 * \param transaction_number     the transaction of the record
 * \param line_in_transaction    the line in the transaction of the record
 *
 * \return NULL if problem, or the newly allocated record
 * */
static CustomRecord *transaction_list_create_record ( gint transaction_number,
                        gint line_in_transaction )
{
    gint column;
    CustomRecord *newrecord;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();

    /* create the new record */
    newrecord = g_malloc0 (sizeof (CustomRecord));

    /* fill the row with the visibles columns */
    for ( column = 0 ; column < CUSTOM_MODEL_N_VISIBLES_COLUMN ; column++ )
	newrecord -> visible_col[column] = gsb_transactions_list_grep_cell_content ( transaction_number,
										     tab_affichage_ope[line_in_transaction][column]);

    if ( conf.utilise_fonte_listes )
	    newrecord -> font = conf.font_string;
    newrecord -> transaction_pointer = gsb_data_transaction_get_pointer_of_transaction (transaction_number);
    newrecord -> what_is_line = IS_TRANSACTION;
    newrecord -> line_in_transaction = line_in_transaction;
    newrecord -> checkbox_visible_reconcile = line_in_transaction == 0 ? 1 : 0;
    newrecord -> checkbox_active = gsb_data_transaction_get_marked_transaction (transaction_number) != 0;
    newrecord -> text_color = &text_color[0];

    return newrecord;
}

/**
 * get a white line record of split
 * and update it to set the total of split children and the variance
 * we just modify the content of the record, no inform the tree view here
 *
 * \param white_record    the record of white line we want update
 *
 * \return FALSE : nothing done, TRUE : ok
 * */
static gboolean transaction_list_update_white_child ( CustomRecord *white_record )
{
    gsb_real total_split = null_real;
    gsb_real variance;
    gchar *amount_string;
    gchar *variance_string;
    CustomRecord *mother_record;
    gint i;
    gint transaction_number;
    GdkColor *mother_text_color;

    if (!white_record)
	    return FALSE;

    mother_record = white_record -> mother_row;

    for (i=0 ; i < mother_record -> number_of_children -1 ; i++)
    {
	CustomRecord *child_record;
	gint child_number;

	child_record = mother_record -> children_rows[i];
	child_number = gsb_data_transaction_get_transaction_number (child_record -> transaction_pointer);
	total_split = gsb_real_add ( total_split,
				     gsb_data_transaction_get_amount (child_number));
    }

    transaction_number = gsb_data_transaction_get_transaction_number (mother_record -> transaction_pointer);
    variance = gsb_real_sub ( gsb_data_transaction_get_amount (transaction_number),
			      total_split);
    /* update the white line */
    amount_string = gsb_real_get_string_with_currency (total_split,
						       gsb_data_transaction_get_currency_number (transaction_number), TRUE);
    variance_string = gsb_real_get_string_with_currency (variance,
							 gsb_data_transaction_get_currency_number (transaction_number), TRUE);

    /* show the variance and sub-total only if different of the transaction */
    if (variance.mantissa)
    {
	white_record -> visible_col[2] = g_strdup_printf ( _("Total : %s (variance : %s)"),
							   amount_string,
							   variance_string );
	mother_text_color = &text_color[1];
    }
    else
    {
	white_record -> visible_col[2] = NULL;
	mother_text_color = &text_color[0];
    }

    /* set the color of the mother */
    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
	mother_record -> transaction_records[i] -> text_color = mother_text_color;

    g_free (amount_string);
    g_free (variance_string);

    return TRUE;
}


/**
 * return the last visible line
 *
 * \param nb_rows    the number of visibles lines (1, 2, 3 or 4)
 *
 * \return numero of the last line
 * */
gint transaction_list_get_last_line ( gint nb_rows )
{

    switch (nb_rows)
    {
	case 1:
	    /* 1 line visible mode */
	    return display_one_line;
	    break;
	case 2:
	    /* 2 lines visibles mode */
	    switch (display_two_lines)
	    {
		case 0:
		    /* show lines 1-2 */
		    return 1;
		    break;
		case 1:
		    /* show lines 1-3 */
		    return 2;
		    break;
		case 2:
		    /* show lines 1-4 */
		    return 3;
		    break;
		case 3:
		    /* show lines 2-3 */
		    return 2;
		    break;
		case 4:
		    /* show lines 2-4 */
		    return 3;
		    break;
		case 5:
		    /* show lines 3-4 */
		    return 3;
		    break;
	    }
	    break;
	case 3:
	    /* 3 lines visibles mode */
	    switch (display_three_lines)
	    {
		case 0:
		    /* show lines 1-2-3 */
		    return 2;
		    break;
		case 1:
		    /* show lines 1-2-4 */
		    return 3;
		    break;
		case 2:
		    /* show lines 1-3-4 */
		    return 3;
		    break;
		case 3:
		    /* show lines 2-3-4 */
		    return 3;
		    break;
	    }
	    break;
    }
    return 3;
}


/**
 * colorise avec un fond gris la ligne qui correspond à la date du jour
 *
 * \param 
 *
 * \return
 * */
void transaction_list_set_color_jour ( gint account_number )
{
    GDate *date_jour;
    gint i, j;
    gint res;
    gint transaction_number;
    gint element_sort;
    CustomList *custom_list;

    custom_list = transaction_model_get_model ();
    g_return_if_fail ( custom_list != NULL );

    date_jour = gdate_today ( );
    element_sort = gsb_data_account_get_element_sort ( account_number,
							     custom_list -> sort_col);

    for ( i = custom_list -> num_visibles_rows -1; i >= 0  ; i-- )
    {
        CustomRecord *record;

        record = custom_list -> visibles_rows[i];

        if (record -> what_is_line == IS_TRANSACTION)
        {
            transaction_number = gsb_data_transaction_get_transaction_number (
                        record -> transaction_pointer);
            if ( transaction_number > 0 )
            {
                if ( element_sort == 2 )
                    res = g_date_compare ( date_jour,
                        gsb_data_transaction_get_value_date_or_date (
                         transaction_number ) );
                else
                    res = g_date_compare ( date_jour,
                        gsb_data_transaction_get_date ( transaction_number ) );

                if ( res >= 0 )
                {
                    /* colorize the record */
                    for (j=0 ; j < custom_list -> nb_rows_by_transaction ; j++)
                    {
                        record -> row_bg = &couleur_jour;
                        i--;
                        if ( i >= 0 )
                            record = custom_list -> visibles_rows[i];
                    }
                    /* on se positionne sur la première ligne */
                    record = custom_list -> visibles_rows[0];
                    break;
                }
            }
        }
    }
    g_date_free ( date_jour ); 
}


/**
 * retourne TRUE si la variance de l'opération ventilée == 0
 *
 *
 *
 * */
gboolean transaction_list_get_variance ( gint transaction_number )
{
    CustomRecord *white_record = NULL;
    CustomList *custom_list;
    gsb_real total_split = null_real;
    gsb_real variance;
    CustomRecord *mother_record;
    gint i;

    custom_list = transaction_model_get_model ();

    g_return_val_if_fail ( custom_list != NULL, FALSE );

    /* if the selection didn't change, do nothing */
    if ( gsb_data_transaction_get_transaction_number (custom_list -> selected_row) == 
        transaction_number )
        white_record = custom_list -> selected_row;

    if ( !white_record )
    {
        GtkTreeIter iter;

        if ( !transaction_model_get_transaction_iter ( &iter, transaction_number, 0 ) )
	        return FALSE;
	    white_record = iter.user_data;
    }

    mother_record = white_record -> mother_row;

    for (i=0 ; i < mother_record -> number_of_children -1 ; i++)
    {
        CustomRecord *child_record;
        gint child_number;

        child_record = mother_record -> children_rows[i];
        child_number = gsb_data_transaction_get_transaction_number (
                        child_record -> transaction_pointer );
        total_split = gsb_real_add ( total_split,
                        gsb_data_transaction_get_amount ( child_number ) );
    }

    transaction_number = gsb_data_transaction_get_transaction_number (
                        mother_record -> transaction_pointer);
    variance = gsb_real_sub ( gsb_data_transaction_get_amount ( transaction_number ),
			            total_split);

    if ( variance.mantissa == 0 )
        return TRUE;
    else
        return FALSE;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
