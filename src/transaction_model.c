/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)            */
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
 * \file transaction_model.c
 * this contains the functions to play with iter and path in the CustomList
 * Grisbi shouldn't work directly on the CustomList except by those files
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "transaction_model.h"
#include "gsb_data_transaction.h"
#include "custom_list.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * THE transaction list in Grisbi
 * can be here in global because only 1 list and i don't
 * see why it would change */
static CustomList *custom_list = NULL;




/**
 * erase the custom_list variable and free the memory used by the list
 * called when closing a file, while destroying the list
 *
 * \param
 *
 * \return
 * */
void transaction_model_initialize ( void )
{
    gint i;

    if (!custom_list)
	return;

    /* free all records and free all memory used by the list */
    for (i=0 ; i<custom_list -> num_rows ; i++)
    {
	CustomRecord *record;
	gint j;

	record = custom_list -> rows[i];
	for (j=0 ; j<CUSTOM_MODEL_N_VISIBLES_COLUMN ; j++)
	    if (record -> visible_col[j])
		g_free (record -> visible_col[j]);

	if (record -> number_of_children)
	{
	    gint k;
	    for (k=0 ; k<record -> number_of_children ; k++)
	    {
		CustomRecord *child_record;

		child_record = record -> children_rows[k];

		for (j=0 ; j<CUSTOM_MODEL_N_VISIBLES_COLUMN ; j++)
		    if (child_record -> visible_col[j])
			g_free (child_record -> visible_col[j]);
		g_free (child_record);
	    }
	    g_free (record -> children_rows);

	    for (k=0 ; k<TRANSACTION_LIST_ROWS_NB ; k++)
		record -> transaction_records[k] -> number_of_children = 0;
	}
	g_free (record);
    }
    g_free (custom_list -> rows);
    g_free (custom_list -> visibles_rows);

    /* cannot free custom_list without crashing grisbi... perhaps done automatically
     * when destroying the tree view ? */
    /*     g_free (custom_list); */
    custom_list = NULL;
}

/**
 * return the CustomList
 *
 * \param
 *
 * \return the CustomList
 * */
CustomList *transaction_model_get_model (void)
{
    return custom_list;
}

/**
 * set the CustomList
 *
 * \param new_custom_list	a CustomList to save
 *
 * \return 
 * */
void transaction_model_set_model ( CustomList *new_custom_list )
{
    custom_list = new_custom_list;
}



/**
 *  find the iter of the line of the asked transaction
 *  the transaction can be a mother or a child
 *
 * \param iter			iter to fill
 * \param transaction_number 	transaction to search
 * \param line_in_transaction	line of the transaction we want
 *
 * \return FALSE not found, TRUE ok
 * */
gboolean transaction_model_get_transaction_iter ( GtkTreeIter *iter,
						  gint transaction_number,
						  gint line_in_transaction )
{
    gint i;
    gint mother_transaction_number = transaction_number;
    gboolean is_child;
    CustomRecord *record = NULL;

    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (transaction_number != 0, FALSE);

    /* the transaction can be a mother or a child,
     * in all cases, we have to find first the mother (or herself)
     * because the children are saved into the mother structure */

    /* if it's a child, look for the mother first */
    is_child = gsb_data_transaction_get_mother_transaction_number (transaction_number) != 0;
    if (is_child)
	mother_transaction_number = gsb_data_transaction_get_mother_transaction_number (transaction_number);

    /* search the mother */
    for (i=0 ; i<custom_list -> num_rows ; i++)
    {
	record = custom_list -> rows[i];

	/* if we are on the good transaction, check the line, or if child can only be that line */
	if (record -> what_is_line == IS_TRANSACTION
	    &&
	    gsb_data_transaction_get_transaction_number (record -> transaction_pointer) == mother_transaction_number )
	{
	    /* ok, found the mother ; if search a child, stop here and continue to search,
	     * if want the mother, need to check if it's the good line */
	    if (is_child)
		break;

	    if (record -> line_in_transaction == line_in_transaction)
	    {
		/* fill the iter */
		iter->stamp     = custom_list->stamp;
		iter->user_data = record;
		return TRUE;
	    }
	}
    }

    /* come here only if it's a child, else, iter not found */
    if (!is_child)
	return FALSE;

    /* for now, record is the mother, we search the child */
    for (i=0 ; i < record -> number_of_children ; i++)
    {
	CustomRecord *child_record;

	child_record = record -> children_rows[i];

	if (child_record -> what_is_line == IS_TRANSACTION
	    &&
	    gsb_data_transaction_get_transaction_number (child_record -> transaction_pointer) == transaction_number)
	{
	    /* ok, the child is found ; and only 1 line, so no more check */
	    /* fill the iter */
	    iter->stamp     = custom_list->stamp;
	    iter->user_data = child_record;
	    return TRUE;
	}
    }
    return FALSE;
}

/**
 * convert a tree path into a tree iter structure
 * this function work with the complete list, so path refer a row wich can be not visible
 * to work in the filtered list, use gtk_tree_model_get_iter
 *
 * \param tree_model 	the custom list
 * \param iter		iter to fill
 * \param path		the tree path
 *
 * \return		TRUE ok, FALSE problem
 * */
gboolean transaction_model_get_iter (GtkTreeIter  *iter,
				     GtkTreePath  *path)
{
    CustomRecord  *record = NULL;
    gint          *indices, depth;
    gint mother_n;

    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    /* get the list of indices, should have maximum 2 */
    indices = gtk_tree_path_get_indices(path);

    /* depth 1 = top level */
    depth   = gtk_tree_path_get_depth(path);

    /* get the mother line first */
    mother_n = indices[0];

    /* get the parent record */
    if ( mother_n >= custom_list->num_rows || mother_n < 0 )
	return FALSE;

    record = custom_list -> rows[mother_n];
    g_return_val_if_fail (record != NULL, FALSE);

    /* if we want a child, get it here */
    if (depth == 2)
    {
	gint child_n;

	child_n = indices[1];
	if (child_n >= record -> number_of_children || child_n < 0)
	    return FALSE;

	record = record -> children_rows[child_n];
	g_return_val_if_fail (record != NULL, FALSE);
    }

    /* We simply store a pointer to our custom record in the iter */
    iter->stamp      = custom_list->stamp;
    iter->user_data  = record;
    /*     iter->user_data2 = NULL; */
    /*     iter->user_data3 = NULL; */
    return TRUE;
}


/**
 * Takes an iter structure and sets it to point to the next row.
 * this function stay at the same level (ie don't go into the children)
 * this function work on the complete model (not filtered),
 * 	to work in the filtered list, use gtk_tree_model_iter_next
 *
 * \param tree_model	the CustomList
 * \param iter		the iter to increase
 *
 * \return TRUE ok, FALSE problem or end of list
 * */
gboolean transaction_model_iter_next (GtkTreeIter *iter)
{
    CustomRecord  *record;

    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (iter->user_data != NULL, FALSE);

    record = (CustomRecord *) iter->user_data;

    /* 2 way now, either we are on a mother, either on a child */
    if (record -> mother_row)
    {
	/* it's a child iter */
	CustomRecord  *mother_record;

	mother_record = record -> mother_row;
	if (record -> pos == (mother_record -> number_of_children - 1))
	    /* we are on the list child, no next */
	    return FALSE;

	/* go to the next child */
	iter->stamp     = custom_list->stamp;
	iter->user_data = mother_record -> children_rows[record -> pos + 1];
    }
    else
    {
	/* it's a mother iter */
	if (record -> pos == (custom_list -> num_rows - 1))
	    /* we are on the last mother, no next */
	    return FALSE;

	iter->stamp     = custom_list->stamp;
	iter->user_data = custom_list -> rows[record -> pos + 1];
    }
    return TRUE;
}


