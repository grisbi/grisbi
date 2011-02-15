/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
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
 * \file custom_list.c
 * this is a custom model used for the transaction list to improve the original gtk model
 * big thanks to Tim-Philipp Müller for his tutorial without i never could do that
 * you can find the tutorial there :
 * http://scentric.net/tutorial/treeview-tutorial.html
 * this file contains the definitions and functioning of the custom list
 * the funtions using it are in gsb_list_model.c
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "custom_list.h"
#include "gsb_data_transaction.h"
#include "transaction_list.h"
#include "transaction_model.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void custom_list_class_init (CustomListClass *klass);
static void custom_list_finalize (GObject *object);
static GType custom_list_get_column_type (GtkTreeModel *tree_model,
					  gint          index);
static GtkTreeModelFlags custom_list_get_flags (GtkTreeModel *tree_model);
static gboolean custom_list_get_iter (GtkTreeModel *tree_model,
				      GtkTreeIter  *iter,
				      GtkTreePath  *path);
static gint custom_list_get_n_columns (GtkTreeModel *tree_model);
static GtkTreePath *custom_list_get_path (GtkTreeModel *tree_model,
					  GtkTreeIter  *iter);
static void custom_list_get_value (GtkTreeModel *tree_model,
				   GtkTreeIter  *iter,
				   gint          column,
				   GValue       *value);
static void custom_list_init (CustomList *custom_list);
static gboolean custom_list_iter_children (GtkTreeModel *tree_model,
					   GtkTreeIter  *iter,
					   GtkTreeIter  *parent);
static gboolean custom_list_iter_has_child (GtkTreeModel *tree_model,
					    GtkTreeIter  *iter);
static gint custom_list_iter_n_children (GtkTreeModel *tree_model,
					 GtkTreeIter  *iter);
static gboolean custom_list_iter_next (GtkTreeModel  *tree_model,
				       GtkTreeIter   *iter);
static gboolean custom_list_iter_nth_child (GtkTreeModel *tree_model,
					    GtkTreeIter  *iter,
					    GtkTreeIter  *parent,
					    gint          n);
static gboolean custom_list_iter_parent (GtkTreeModel *tree_model,
					 GtkTreeIter  *iter,
					 GtkTreeIter  *child);
static void custom_list_tree_model_init (GtkTreeModelIface *iface);
/*END_STATIC*/

/*START_EXTERN*/
extern GdkColor text_color[2];
/*END_EXTERN*/


static GObjectClass *parent_class = NULL;  /* GObject stuff - nothing to worry about */


/**
 * create the new custom list
 * add automatically the white line
 *
 * \param
 *
 * \return the new CustomList tree model
 * */
CustomList *custom_list_new (void)
{
    CustomList *custom_list;
    CustomRecord *white_record[TRANSACTION_LIST_ROWS_NB];
    gint white_line_number;
    gint i, j;
    gulong newsize;

    custom_list = (CustomList*) g_object_new (CUSTOM_TYPE_LIST, NULL);

    if ( custom_list == NULL )
        return NULL;

    /* increase the table of CustomRecord */
    custom_list->num_rows = TRANSACTION_LIST_ROWS_NB;
    newsize = custom_list->num_rows * sizeof(CustomRecord*);
    custom_list->rows = g_malloc0 (newsize);
    custom_list->visibles_rows = g_malloc0 (newsize);

    /* create and fill the white line record */
    white_line_number = gsb_data_transaction_new_white_line (0);

    /* now we can save the 4 new rows, ie the complete white line */
    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
    {
	/* fill the white_record structure */
	white_record[i] = g_malloc0 (sizeof (CustomRecord));
	white_record[i] -> transaction_pointer = gsb_data_transaction_get_pointer_of_transaction (white_line_number);
	white_record[i] -> what_is_line = IS_TRANSACTION;
	white_record[i] -> line_in_transaction = i;

	/* save the newrecord pointer */
	custom_list->rows[i] = white_record[i];
	/* and the pos (number) of the row */
	white_record[i]->pos = i;
    }

    /* save the records in all the records of the transaction */
    for (i=0 ; i < TRANSACTION_LIST_ROWS_NB ; i++)
	for (j=0 ; j < TRANSACTION_LIST_ROWS_NB ; j++)
	    white_record[i] -> transaction_records[j] = white_record[j];

    return custom_list;
}


/**
 * we register our new type and its interfaces with the type system.
 * extern is necessary for mk_include to set that function in custom_list.h
 *
 * \param
 *
 * \return	Gtype
 * */
extern GType custom_list_get_type (void)
{
    static GType custom_list_type = 0;

    /* Some boilerplate type registration stuff */
    if (custom_list_type == 0)
    {
	static const GTypeInfo custom_list_info =
	{
	    sizeof (CustomListClass),
	    NULL,                                         /* base_init */
	    NULL,                                         /* base_finalize */
	    (GClassInitFunc) custom_list_class_init,
	    NULL,                                         /* class finalize */
	    NULL,                                         /* class_data */
	    sizeof (CustomList),
	    0,                                           /* n_preallocs */
	    (GInstanceInitFunc) custom_list_init
	};
	static const GInterfaceInfo tree_model_info =
	{
	    (GInterfaceInitFunc) custom_list_tree_model_init,
	    NULL,
	    NULL
	};

	/* First register the new derived type with the GObject type system */
	custom_list_type = g_type_register_static (G_TYPE_OBJECT, "CustomList",
						   &custom_list_info, (GTypeFlags)0);

	/* Now register our GtkTreeModel interface with the type system */
	g_type_add_interface_static (custom_list_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
    }

    return custom_list_type;
}


/**
 * Init callback for the type system
 * called once when our new class is created
 *
 * \param CustomListClass
 *
 * \return
 * */
static void custom_list_class_init (CustomListClass *klass)
{
    GObjectClass *object_class;

    parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
    object_class = (GObjectClass*) klass;

    object_class->finalize = custom_list_finalize;
}


/**
 * init callback for the interface registration in custom_list_get_type
 * Here we override the GtkTreeModel interface functions that we implement
 *
 * \param iface
 *
 * \return
 * */
static void custom_list_tree_model_init (GtkTreeModelIface *iface)
{
    iface->get_flags       = custom_list_get_flags;
    iface->get_n_columns   = custom_list_get_n_columns;
    iface->get_column_type = custom_list_get_column_type;
    iface->get_iter        = custom_list_get_iter;
    iface->get_path        = custom_list_get_path;
    iface->get_value       = custom_list_get_value;
    iface->iter_next       = custom_list_iter_next;
    iface->iter_children   = custom_list_iter_children;
    iface->iter_has_child  = custom_list_iter_has_child;
    iface->iter_n_children = custom_list_iter_n_children;
    iface->iter_nth_child  = custom_list_iter_nth_child;
    iface->iter_parent     = custom_list_iter_parent;
}


/**
 * this is called everytime a new custom list object
 * instance is created (we do that in custom_list_new).
 * Initialise the list structure's fields here.
 *
 * \pararm custom_list
 *
 * \return
 * */
static void custom_list_init (CustomList *custom_list)
{
    custom_list->n_columns = CUSTOM_MODEL_N_COLUMNS;

    custom_list->column_types[0] = G_TYPE_STRING;  	/* col 0 ( check by default) */
    custom_list->column_types[1] = G_TYPE_STRING;   	/* col 1 ( date by default) */
    custom_list->column_types[2] = G_TYPE_STRING;    	/* col 2 ( payee by default) */
    custom_list->column_types[3] = G_TYPE_STRING;     	/* col 3 ( P/R by default) */
    custom_list->column_types[4] = G_TYPE_STRING;     	/* col 4 ( debit by default) */
    custom_list->column_types[5] = G_TYPE_STRING;     	/* col 5 ( credit by default) */
    custom_list->column_types[6] = G_TYPE_STRING;     	/* col 6 ( balance by default) */

    custom_list->column_types[7] = GDK_TYPE_COLOR;     	/* col 7 ( color of bg ) */
    custom_list->column_types[8] = GDK_TYPE_COLOR; 	/* col 8 ( bg saved when selected ) */
    custom_list->column_types[9] = G_TYPE_STRING;     	/* col 9 ( color of amount ) */
    custom_list->column_types[10] = GDK_TYPE_COLOR; 	/* col 10 ( color of the text ) */
    custom_list->column_types[11] = G_TYPE_POINTER;     /* col 11 ( transaction struct address ) */
    custom_list->column_types[12] = G_TYPE_INT;     	/* col 12 ( what is line ) */
    custom_list->column_types[13] = G_TYPE_STRING;    	/* col 12 ( font ) */
    custom_list->column_types[14] = G_TYPE_INT;     	/* col 13 ( line in transaction ) */
    custom_list->column_types[15] = G_TYPE_BOOLEAN;    	/* col 14 ( transaction visible ) */
    custom_list->column_types[16] = G_TYPE_BOOLEAN;    	/* col 15 ( checkbox visible ) */
    custom_list->column_types[17] = G_TYPE_BOOLEAN;    	/* col 16 ( checkbox visible during reconciliation ) */
    custom_list->column_types[18] = G_TYPE_BOOLEAN;    	/* col 17 ( checkbox active ) */

    /* no row at the beginning */
    custom_list->num_rows = 0;
    custom_list->rows     = NULL;

    custom_list -> num_visibles_rows = 0;
    custom_list -> visibles_rows = NULL;

    /* default sort is col 1 (date) and descending */
    custom_list->sort_col    = CUSTOM_MODEL_COL_1;
    custom_list->sort_order = GTK_SORT_ASCENDING;

    custom_list->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */
}




/**
 * this is called just before a custom list is
 * destroyed. Free dynamically allocated memory here.
 *
 * \param object	the CustomList
 *
 * \param
 * */
void custom_list_finalize (GObject *object)
{
    transaction_model_initialize ();

    /* must chain up - finalize parent */
    (* parent_class->finalize) (object);
}


/**
 * tells the rest of the world whether our tree model
 * has any special characteristics. In our case,
 * each tree iter is valid as long as the row in question
 * exists, as it only contains a pointer to our struct.
 *
 * reached by gtk_tree_model_get_flags
 *
 * \param 	tree_model
 *
 * \return	GtkTreeModelFlags
 * */
static GtkTreeModelFlags custom_list_get_flags (GtkTreeModel *tree_model)
{
    g_return_val_if_fail (CUSTOM_IS_LIST(tree_model), (GtkTreeModelFlags)0);

    return (GTK_TREE_MODEL_ITERS_PERSIST);
}


/**
 * tells the rest of the world how many data
 * columns we export via the tree model interface
 * 
 * reached by gtk_tree_model_get_n_columns
 *
 * \param 	tree_model
 * \return	the number of columns
 * */
static gint custom_list_get_n_columns (GtkTreeModel *tree_model)
{

    g_return_val_if_fail (CUSTOM_IS_LIST(tree_model), 0);

    return CUSTOM_LIST(tree_model)->n_columns;
}


/**
 * tells the rest of the world which type of
 * data an exported model column contains
 *
 * reached by gtk_tree_model_get_column_type
 *
 * \param tree_model	the CustomList
 * \param index		the n column we want the GType
 *
 * \return the Gtype of the column
 * */
static GType custom_list_get_column_type (GtkTreeModel *tree_model,
					  gint          index)
{
    g_return_val_if_fail (CUSTOM_IS_LIST(tree_model), G_TYPE_INVALID);
    g_return_val_if_fail (index < CUSTOM_LIST(tree_model)->n_columns && index >= 0, G_TYPE_INVALID);

    return CUSTOM_LIST(tree_model)->column_types[index];
}


/**
 * convert a tree path into a tree iter structure
 * this function work with the filtered list, so path refer a row in the filtered list
 * to work in the complete list, use transaction_model_get_iter
 *
 * reached by gtk_tree_model_get_iter
 *
 * \param tree_model 	the custom list
 * \param iter		iter to fill
 * \param path		the tree path
 *
 * \return		TRUE ok, FALSE problem
 * */
static gboolean custom_list_get_iter (GtkTreeModel *tree_model,
				      GtkTreeIter  *iter,
				      GtkTreePath  *path)
{
    CustomList    *custom_list;
    CustomRecord  *record = NULL;
    gint          *indices, depth;
    gint mother_n;

    g_return_val_if_fail (CUSTOM_IS_LIST(tree_model), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    custom_list = CUSTOM_LIST(tree_model);

    /* get the list of indices, should have maximum 2 */
    indices = gtk_tree_path_get_indices(path);

    /* depth 1 = top level */
    depth   = gtk_tree_path_get_depth(path);

    mother_n = indices[0];
    /* get the first depth record */
    if ( mother_n >= custom_list->num_visibles_rows || mother_n < 0 )
	return FALSE;

    record = custom_list -> visibles_rows[mother_n];
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
 * converts a tree iter into a tree path (ie. the
 * physical position of that row in the list).
 * that function works in the filtered model
 *
 * reached by gtk_tree_model_get_path
 *
 * \param tree_model	the CustomList
 * \param iter		a valid iter to find the path
 *
 * \param a newly allocated GtkTreePath
 * */
static GtkTreePath *custom_list_get_path (GtkTreeModel *tree_model,
					  GtkTreeIter  *iter)
{
    GtkTreePath  *path;
    CustomRecord *record;
    CustomList   *custom_list;

    g_return_val_if_fail (CUSTOM_IS_LIST(tree_model), NULL);
    g_return_val_if_fail (iter != NULL,               NULL);
    g_return_val_if_fail (iter->user_data != NULL,    NULL);

    custom_list = CUSTOM_LIST(tree_model);

    record = (CustomRecord*) iter->user_data;

    path = gtk_tree_path_new();

    /* if we are on a child, we need to append the index of the mother */
    if (record -> mother_row)
	gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);

    gtk_tree_path_append_index(path, record->filtered_pos);
    return path;
}


/**
 * Returns a row's exported data columns
 *
 * reached by gtk_tree_model_get_value
 *
 * \param tree_model	the CustomList
 * \param iter		iter of the row of the value
 * \param column	column of the value
 * \param value		value to fill
 *
 * \return
 * */
static void custom_list_get_value (GtkTreeModel *tree_model,
				   GtkTreeIter  *iter,
				   gint          column,
				   GValue       *value)
{
    CustomRecord  *record;
    CustomList    *custom_list;

    g_return_if_fail (CUSTOM_IS_LIST (tree_model));
    g_return_if_fail (iter != NULL);
    g_return_if_fail (column < CUSTOM_LIST(tree_model)->n_columns);

    g_value_init (value, CUSTOM_LIST(tree_model)->column_types[column]);

    custom_list = CUSTOM_LIST(tree_model);

    record = (CustomRecord*) iter->user_data;
    g_return_if_fail ( record != NULL );

    switch(column)
    {
	case CUSTOM_MODEL_COL_0:
	case CUSTOM_MODEL_COL_1:
	case CUSTOM_MODEL_COL_2:
	case CUSTOM_MODEL_COL_3:
	case CUSTOM_MODEL_COL_4:
	case CUSTOM_MODEL_COL_5:
	case CUSTOM_MODEL_COL_6:
	    g_value_set_string(value, record->visible_col[column]);
	    break;
	case CUSTOM_MODEL_BACKGROUND:
	    g_value_set_boxed(value, (gpointer) record->row_bg);
	    break;
	case CUSTOM_MODEL_SAVE_BACKGROUND:
	    g_value_set_boxed(value, (gpointer) record->row_bg_save);
	    break;
	case CUSTOM_MODEL_AMOUNT_COLOR:
	    g_value_set_string(value, record->amount_color);
	    break;
	case CUSTOM_MODEL_TEXT_COLOR:
	    g_value_set_boxed(value, (gpointer) record->text_color);
	    break;
	case CUSTOM_MODEL_TRANSACTION_ADDRESS:
	    g_value_set_pointer(value, record->transaction_pointer);
	    break;
	case CUSTOM_MODEL_WHAT_IS_LINE:
	    g_value_set_int(value, record->what_is_line);
	    break;
	case CUSTOM_MODEL_FONT:
	    g_value_set_string(value, record->font);
	    break;
	case CUSTOM_MODEL_TRANSACTION_LINE:
	    g_value_set_int(value, record->line_in_transaction);
	    break;
	case CUSTOM_MODEL_VISIBLE:
	    g_value_set_boolean(value, record->line_visible);
	    break;
	case CUSTOM_MODEL_CHECKBOX_VISIBLE:
	    g_value_set_boolean(value, record->checkbox_visible);
	    break;
	case CUSTOM_MODEL_CHECKBOX_VISIBLE_RECONCILE:
	    g_value_set_boolean(value, record->checkbox_visible_reconcile);
	    break;
	case CUSTOM_MODEL_CHECKBOX_ACTIVE:
	    g_value_set_boolean(value, record->checkbox_active);
	    break;
	default:
	    devel_debug ("Wrong value asked for CustomList");
    }
}

/**
 * Sets the data in the cell specified by iter and column.
 * The type of value must be convertible to the type of the column.
 * this function doesn't check the type of the value,
 * it's done already in transaction_list_update_column
 * so the type must be the good one
 * (todo if we begin to call that function directly)
 *
 * shouldn't be called to set a string in the visibles columns,
 * 	should use transaction_list_update_element instead
 *
 * reached by gtk_tree_model_get_value
 *
 * \param tree_model	the CustomList
 * \param iter		iter of the row of the value
 * \param column	column of the value
 * \param value		value to fill
 *
 * \return
 * */
void custom_list_set_value (GtkTreeModel *tree_model,
			    GtkTreeIter  *iter,
			    gint          column,
			    GValue       *value)
{
    CustomRecord  *record;

    g_return_if_fail (iter != NULL);
    g_return_if_fail (column < CUSTOM_LIST(tree_model)->n_columns);
    g_return_if_fail (G_IS_VALUE (value));

    record = (CustomRecord*) iter->user_data;
    g_return_if_fail ( record != NULL );

    switch(column)
    {
	case CUSTOM_MODEL_COL_0:
	case CUSTOM_MODEL_COL_1:
	case CUSTOM_MODEL_COL_2:
	case CUSTOM_MODEL_COL_3:
	case CUSTOM_MODEL_COL_4:
	case CUSTOM_MODEL_COL_5:
	case CUSTOM_MODEL_COL_6:
	    record -> visible_col[column] = g_value_dup_string(value);
	    break;
	case CUSTOM_MODEL_BACKGROUND:
	    record -> row_bg = g_value_get_boxed(value);
	    break;
	case CUSTOM_MODEL_SAVE_BACKGROUND:
	    record -> row_bg_save = g_value_get_boxed(value);
	    break;
	case CUSTOM_MODEL_AMOUNT_COLOR:
	    record -> amount_color = g_value_dup_string(value);
	    break;
	case CUSTOM_MODEL_TEXT_COLOR:
	    record -> text_color = g_value_get_boxed(value);
	    break;
	case CUSTOM_MODEL_TRANSACTION_ADDRESS:
	    record -> transaction_pointer = g_value_get_pointer(value);
	    break;
	case CUSTOM_MODEL_WHAT_IS_LINE:
	    record -> what_is_line = g_value_get_int(value);
	    break;
	case CUSTOM_MODEL_FONT:
	    record -> font = g_value_dup_string (value);
	    break;
	case CUSTOM_MODEL_TRANSACTION_LINE:
	    record -> line_in_transaction = g_value_get_int(value);
	    break;
	case CUSTOM_MODEL_VISIBLE:
	    record -> line_visible = g_value_get_boolean(value);
	    break;
	case CUSTOM_MODEL_CHECKBOX_VISIBLE:
	    record -> checkbox_visible = g_value_get_boolean(value);
	    break;
	case CUSTOM_MODEL_CHECKBOX_VISIBLE_RECONCILE:
	    record -> checkbox_visible_reconcile = g_value_get_boolean(value);
	    break;
	case CUSTOM_MODEL_CHECKBOX_ACTIVE:
	    record -> checkbox_active = g_value_get_boolean(value);
	    break;
	default:
	    devel_debug ("Wrong value asked for CustomList");
	    return;
    }

    if ((!record -> mother_row && record -> filtered_pos != -1)
	||
	(record -> mother_row && record -> mother_row -> filtered_pos != -1))
    {
	GtkTreePath *path;

	path = gtk_tree_path_new ();
	if (record -> mother_row)
	    gtk_tree_path_append_index (path, record -> mother_row -> filtered_pos);
	gtk_tree_path_append_index (path, record -> filtered_pos);
	gtk_tree_model_row_changed (GTK_TREE_MODEL (tree_model), path, iter);
	gtk_tree_path_free (path);
    }
}


/**
 * Takes an iter structure and sets it to point to the next row.
 * this function stay at the same level (ie don't go into the children)
 * this function work on the filtered model,
 * 	to work in the complete list, use transaction_model_iter_next
 *
 * reached by gtk_tree_model_iter_next
 *
 * \param tree_model	the CustomList
 * \param iter		the iter to increase
 *
 * \return TRUE ok, FALSE problem or end of list
 * */
static gboolean custom_list_iter_next (GtkTreeModel  *tree_model,
				       GtkTreeIter   *iter)
{
    CustomRecord  *record;
    CustomList    *custom_list;

    g_return_val_if_fail (CUSTOM_IS_LIST (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (iter->user_data != NULL, FALSE);

    custom_list = CUSTOM_LIST(tree_model);

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
	if (record -> filtered_pos == (custom_list -> num_visibles_rows - 1))
	    /* we are on the last mother, no next */
	    return FALSE;

	iter->stamp     = custom_list->stamp;
	iter->user_data = custom_list -> visibles_rows[record -> filtered_pos + 1];
    }
    return TRUE;
}


/**
 * Returns TRUE or FALSE depending on whether
 * the row specified by 'parent' has any children.
 * If it has children, then 'iter' is set to
 * point to the first child. Special case: if
 * 'parent' is NULL, then the first top-level
 * row should be returned if it exists.
 *
 * reached by gtk_tree_model_iter_children
 *
 * \param tree_model	the CustomList
 * \param iter		the iter to place on the first child
 * \param parent	the parent iter to check if any children
 *
 * \return	TRUE, has children, FALSE, no children
 * */
static gboolean custom_list_iter_children (GtkTreeModel *tree_model,
					   GtkTreeIter  *iter,
					   GtkTreeIter  *parent)
{
    CustomList  *custom_list;
    CustomRecord *parent_record;

    g_return_val_if_fail (CUSTOM_IS_LIST (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);
    custom_list = CUSTOM_LIST(tree_model);

    /* No rows => no first row */
    if (!custom_list->num_visibles_rows)
	return FALSE;

    /* if parent is NULL, we want the first line */
    if (parent == NULL)
    {
	/* parent == NULL is a special case; we need to return the first top-level row */
	/* Set iter to first item in list */
	iter->stamp     = custom_list->stamp;
	iter->user_data = custom_list->visibles_rows[0];
	return TRUE;
    }

    /* get the parent structure */
    parent_record = parent->user_data;
    g_return_val_if_fail (parent_record != NULL, FALSE);

    /* if no child, return FALSE */
    if (!parent_record -> has_expander)
	/* no need to check number_of_children, if no child or no good line, has_expander is FALSE */
	return FALSE;

    /* there is at least one child, so set iter on that child */
    iter->stamp     = custom_list->stamp;
    iter->user_data = parent_record -> children_rows[0];
    return TRUE;
}


/**
 * Returns TRUE or FALSE depending on whether
 * the row specified by 'iter' has any children.
 *
 * reached by gtk_tree_model_iter_has_child
 *
 * \param tree_model	the CustomList
 * \param iter		the iter to check
 *
 * \return TRUE : has child, FALSE : no child
 * */
static gboolean custom_list_iter_has_child (GtkTreeModel *tree_model,
					    GtkTreeIter  *iter)
{
    CustomRecord *parent_record;

    g_return_val_if_fail (CUSTOM_IS_LIST (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    /* get the parent structure */
    parent_record = iter->user_data;
    g_return_val_if_fail (parent_record != NULL, FALSE);

    /* the transaction can have some children but they can be displayed in another row */
    if (!parent_record -> has_expander)
	return FALSE;

    return TRUE;
}


/**
 * Returns the number of children the row
 * specified by 'iter' has. 
 * A special case i when 'iter' is NULL, in which case we need
 * to return the number of top-level nodes, ie. the number of rows in our list.
 *
 * reached by gtk_tree_model_iter_n_children
 *
 * \param tree_model	the CustomList
 * \param iter		the iter to check
 *
 * \return the number of children
 * */
static gint custom_list_iter_n_children (GtkTreeModel *tree_model,
					 GtkTreeIter  *iter)
{
    CustomList  *custom_list;
    CustomRecord *parent_record;

    g_return_val_if_fail (CUSTOM_IS_LIST (tree_model), -1);

    custom_list = CUSTOM_LIST(tree_model);

    if (!iter)
	/* special case: if iter == NULL, return number of top-level rows */
	return custom_list->num_visibles_rows;

    /* get the parent structure */
    parent_record = iter->user_data;
    g_return_val_if_fail (parent_record != NULL, 0);

    /* the transaction can have some children but they can be displayed in another row */
    if (!parent_record -> has_expander)
	return 0;

    return parent_record -> number_of_children;
}


/**
 * If the row specified by 'parent' has any
 * children, set 'iter' to the n-th child and
 * return TRUE if it exists, otherwise FALSE.
 * A special case is when 'parent' is NULL, in
 * which case we need to set 'iter' to the n-th
 * row if it exists.
 *
 * reached by gtk_tree_model_iter_nth_child
 *
 * \param tree_model	the CustomList
 * \param iter		the iter to set on the n-th child
 * \param parent	the parent iter
 * \param n		the n-th child we want
 *
 * \return TRUE if child exist and iter place, FALSE otherwise
 * */
static gboolean custom_list_iter_nth_child (GtkTreeModel *tree_model,
					    GtkTreeIter  *iter,
					    GtkTreeIter  *parent,
					    gint          n)
{
    CustomList    *custom_list;
    CustomRecord  *parent_record;

    g_return_val_if_fail (CUSTOM_IS_LIST (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    custom_list = CUSTOM_LIST(tree_model);

    if (parent == NULL)
    {
	/* special case: if parent == NULL, set iter to n-th top-level row */
	if( n >= custom_list->num_visibles_rows )
	    return FALSE;

	iter->stamp = custom_list->stamp;
	iter->user_data = custom_list->visibles_rows[n];
	return TRUE;
    }

    /* get the parent structure */
    parent_record = parent->user_data;
    g_return_val_if_fail (parent_record != NULL, FALSE);

    /* check n not too big (also check if child exist) */
    if (n >= parent_record -> number_of_children
	||
	!parent_record -> has_expander)
	return FALSE;

    /* there is at least one child, so set iter on that child */
    iter->stamp     = custom_list->stamp;
    iter->user_data = parent_record -> children_rows[n];
    return TRUE;
}


/**
 * Point 'iter' to the parent node of 'child'
 *
 * reached by gtk_tree_model_iter_parent
 *
 * \param tree_model	The CustomList
 * \param iter		the iter to place on the parent
 * \param child		the child iter
 *
 * \return TRUE : iter placed, FALSE : problem
 * */
static gboolean custom_list_iter_parent (GtkTreeModel *tree_model,
					 GtkTreeIter  *iter,
					 GtkTreeIter  *child)
{
    CustomList    *custom_list;
    CustomRecord  *child_record;

    g_return_val_if_fail (CUSTOM_IS_LIST (tree_model), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);
    g_return_val_if_fail (child != NULL, FALSE);

    custom_list = CUSTOM_LIST(tree_model);

    /* get the child structure */
    child_record = child->user_data;
    g_return_val_if_fail (child_record != NULL, FALSE);
    g_return_val_if_fail (child_record -> mother_row != NULL, FALSE);

    /* set iter on the parent */
    iter->stamp     = custom_list->stamp;
    iter->user_data = child_record -> mother_row;
    return TRUE;
}



