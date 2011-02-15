/* ************************************************************************** */
/*                                                                            */
/*                                gsb_combo_box                               */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_combo_box.c
 * plays with special combo_box for grisbi
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_combo_box.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/**
 * create a text only combo_box with an index
 * column 0 will contain the text
 * column 1 will have the index, in the order of the strings
 * 
 * this function takes an array of string and attribute a number beginning by the first element of the array
 * to link some text with a special number, use gsb_combo_box_new_with_index_by_list
 *
 * \param string a pointer to an array of strings terminated by NULL, string will be appended in that order
 * \param func an optional function to call when change the current item (gboolean func (GtkWidget *combox, gpointer data)
 * \param data the data to send to the func
 *
 * \return a combo box widget
 * */
GtkWidget *gsb_combo_box_new_with_index ( gchar **string,
					  GCallback func,
					  gpointer data )
{
    GtkWidget *combo_box;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    gint i = 0;

    combo_box = gtk_combo_box_new ();

    store = gtk_list_store_new ( 2,
				 G_TYPE_STRING,
				 G_TYPE_INT );
    if (string)
    {
	while (string[i])
	{
	    GtkTreeIter iter;

	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( store,
				 &iter,
				 0, string[i],
				 1, i,
				 -1 );
	    i++;
	}
    }
    gtk_combo_box_set_model ( GTK_COMBO_BOX (combo_box),
			      GTK_TREE_MODEL (store));
    if ( func )
	g_signal_connect ( G_OBJECT (combo_box),
			   "changed",
			   G_CALLBACK(func),
			   data );

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer,
				    "text", 0,
				    NULL);

    return combo_box;
}


/**
 * create a text only combo_box with an index
 * column 0 will contain the text
 * column 1 will have the index
 *
 * the function takes a g_slist of text and number to attribute
 * 	this list must be as : text_1 -> number_1 -> text_2 -> number_2 -> ...
 * 
 * \param list	a g_slist containing succession of text and number to associate to the text
 * \param func an optional function to call when change the current item (gboolean func (GtkWidget *combox, gpointer data)
 * \param data the data to send to the func
 *
 * \return a combo box widget
 * */
GtkWidget *gsb_combo_box_new_with_index_by_list ( GSList *list,
						  GCallback func,
						  gpointer data )
{
    GSList *tmp_list;
    GtkWidget *combo_box;
    GtkListStore *store;
    GtkCellRenderer *renderer;

    combo_box = gtk_combo_box_new ();

    store = gtk_list_store_new ( 2,
				 G_TYPE_STRING,
				 G_TYPE_INT );
    tmp_list = list;
    if (tmp_list)
    {
	while (tmp_list)
	{
	    GtkTreeIter iter;
	    gchar *string;

	    string = tmp_list -> data;
	    tmp_list = tmp_list -> next;

	    /* should not append */
	    if (!tmp_list)
		break;

	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( store,
				 &iter,
				 0, string,
				 1, GPOINTER_TO_INT (tmp_list -> data),
				 -1 );
	    tmp_list = tmp_list -> next;
	}
    }
    gtk_combo_box_set_model ( GTK_COMBO_BOX (combo_box),
			      GTK_TREE_MODEL (store));
    if ( func )
	g_signal_connect ( G_OBJECT (combo_box),
			   "changed",
			   G_CALLBACK(func),
			   data );

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer,
				    "text", 0,
				    NULL);

    return combo_box;
}

/**
 * get the index of the current item in the combo_box given in param
 * the combo_box must have been created with gsb_combo_box_new_with_index
 *
 * \param combo_box
 *
 * \return a number, the index, or -1 if nothing selected
 * */
gint gsb_combo_box_get_index ( GtkWidget *combo_box )
{
    gint index;
    GtkTreeIter iter;

    if (!combo_box)
	return -1;

    if ( !gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combo_box),
					  &iter ))
	return -1;

    gtk_tree_model_get ( GTK_TREE_MODEL (gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box))),
			 &iter,
			 1, &index,
			 -1 );
    return index;
}


/**
 * set the index of the item in the combo_box given in param,
 * ie makes visible the text corresponding of the index
 * the combo_box must have been created with gsb_combo_box_new_with_index
 *
 * \param combo_box
 * \param index the index of the text we want to set visible
 *
 * \return TRUE ok, FALSE problem
 * */
gint gsb_combo_box_set_index ( GtkWidget *combo_box,
			       gint index )
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    /* if no account button, go away... */
    if (!combo_box)
	return FALSE;

    model = gtk_combo_box_get_model ( GTK_COMBO_BOX (combo_box));

    if ( gtk_tree_model_get_iter_first (model, &iter))
    {
	do
	{
	    gint number;

	    gtk_tree_model_get ( model, &iter,
				 1, &number,
				 -1 );
	    if (number == index)
	    {
		gtk_combo_box_set_active_iter ( GTK_COMBO_BOX (combo_box),
						&iter );
		return TRUE;
	    }
	}
	while (gtk_tree_model_iter_next (model, &iter));
    }
    return FALSE;
}

