/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
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
 * \file gsb_fyear.c
 * contains tools to work with the financial years
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_fyear.h"
#include "gsb_data_fyear.h"
/*END_INCLUDE*/


/**
 * this is a tree model filter with 3 columns :
 * the name, the number and a boolean to show it or not
 * */
static GtkTreeModel *fyear_model;

/**
 * this is a tree model filter from fyear_model_filter wich
 * show only the financial years wich must be showed
 * */
static GtkTreeModel *fyear_model_filter;


/*START_STATIC*/
static gboolean gsb_fyear_create_combobox_store ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/



/**
 * set to NULL the static variables
 *
 * \param
 *
 * \return
 * */
void gsb_fyear_init_variables ( void )
{
    if (fyear_model_filter
	&&
	GTK_IS_LIST_STORE (fyear_model))
	gtk_list_store_clear (GTK_LIST_STORE (fyear_model));

    fyear_model = NULL;
    fyear_model_filter = NULL;
}


/**
 * create and return a combobox with the financial years
 *
 * \param set_automatic if TRUE, will show the choice "Automatic"
 *
 * \return a widget combobox or NULL
 * */
GtkWidget *gsb_fyear_make_combobox ( gboolean set_automatic )
{
    GtkWidget *combo_box;

    if (!fyear_model_filter)
        gsb_fyear_create_combobox_store ();

    combo_box = gsb_fyear_make_combobox_new ( fyear_model_filter, set_automatic );

    return (combo_box);
}


/**
 * create and return a combobox with the financial years
 *
 * \param model for the combobox
 * \param set_automatic if TRUE, will show the choice "Automatic"
 *
 * \return a widget combobox or NULL
 * */
GtkWidget *gsb_fyear_make_combobox_new ( GtkTreeModel *model,
                        gboolean set_automatic )
{
    GtkCellRenderer *renderer;
    GtkWidget *combo_box;

    combo_box = gtk_combo_box_new_with_model ( GTK_TREE_MODEL ( model ) );

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box),
                        renderer,
                        "text", FYEAR_COL_NAME,
                        NULL);

    /* show or hide the automatic line */
    gsb_fyear_set_automatic ( set_automatic );
    gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_box), 0 );

    return (combo_box);
}


/**
 * set the combobox on the fyear given in param
 * if the fyear exists but is not showed normally, we show it because it's for
 * a modification of a transaction (when the form will be freed, that fyear won't be showed again)
 *
 * \combo_box the combo-box to set
 * \fyear_number the fyear we want to set on the combo-box, if 0, will be set on "Automatic"
 *
 * \return TRUE fyear found, FALSE fyear not found, nothing change
 * */
gboolean gsb_fyear_set_combobox_history ( GtkWidget *combo_box,
                        gint fyear_number )
{
    return gsb_fyear_select_iter_by_number ( combo_box,
                        fyear_model,
                        fyear_model_filter,
                        fyear_number );
}


/** 
 * Get and return the number of the fyear in the combobox given
 * in param
 * if the fyear is 0, try to find a fyear with the givent date
 * 
 * \param combo_box a combo_box with the financials years
 * \param date date to find a corresponding fyear if the combobox is on Automatic or NULL
 * 
 * \return the number of fyear or 0 if problem
 * */
gint gsb_fyear_get_fyear_from_combobox ( GtkWidget *combo_box,
                        const GDate *date )
{
    gint fyear_number = 0;
    GtkTreeIter iter;

    if ( !fyear_model_filter )
        gsb_fyear_create_combobox_store ( );

    if ( gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combo_box), &iter ) )
    {

        gtk_tree_model_get ( GTK_TREE_MODEL (fyear_model_filter),
                    &iter,
                    FYEAR_COL_NUMBER, &fyear_number,
                    -1 );
        if ( !fyear_number && date )
            fyear_number = gsb_data_fyear_get_from_date ( date );
    }

    return fyear_number;
}


/**
 * show or hide the Automatic line in the fyear list
 * this will apply to all fyears combobox
 *
 * \param set_automatic TRUE to show the "Automatic" line, FALSE to hide it
 *
 *
 * \return TRUE done, FALSE problem
 * */
gboolean gsb_fyear_set_automatic ( gboolean set_automatic )
{
    GtkTreeIter iter;

    if (!fyear_model)
	gsb_fyear_create_combobox_store ();

    if (!gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (fyear_model),
					 &iter ))
	return FALSE;

    gtk_list_store_set ( GTK_LIST_STORE (fyear_model),
			 &iter,
			 FYEAR_COL_VIEW, set_automatic,
			 -1 );
    return TRUE;
}


/**
 * update the list of the financial years, wich change all
 * the current combobox content
 * set the first row with Automatic with 0 as number
 *
 * \param
 *
 * \return FALSE
 */
gboolean gsb_fyear_update_fyear_list ( void )
{
    gsb_fyear_update_fyear_list_new ( fyear_model, fyear_model_filter, "Automatic" );

    return FALSE;
}


/**
 * update the list of the financial years, wich change all
 * the current combobox content
 * set the first row with title with 0 as number
 *
 * \param model
 * \param model_filter
 * \param title
 *
 * \return FALSE
 */
gboolean gsb_fyear_update_fyear_list_new ( GtkTreeModel *model,
                        GtkTreeModel *model_filter,
                        gchar *title )
{
    GSList *list_tmp;
    GtkTreeIter iter;

    /* if no filter, thats because not created, but don't create here
     * because we can come here without needed of fyear button */
    if ( !model_filter )
        return FALSE;

    gtk_list_store_clear (GTK_LIST_STORE ( model ) );

    /* put at the beginning title */
    gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
    gtk_list_store_set ( GTK_LIST_STORE ( model ),
                        &iter,
                        FYEAR_COL_NAME, _(title),
                        FYEAR_COL_NUMBER, 0,
                        FYEAR_COL_VIEW, TRUE,
                        -1 );

    /* fill the list with the copy of fyears_list */
    list_tmp = g_slist_sort ( g_slist_copy ( gsb_data_fyear_get_fyears_list ( ) ),
                        ( GCompareFunc ) gsb_data_fyear_compare_from_struct );

    while ( list_tmp )
    {
        gint fyear_number;

        fyear_number = gsb_data_fyear_get_no_fyear (list_tmp -> data);

        gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
        gtk_list_store_set ( GTK_LIST_STORE ( model ),
                            &iter,
                            FYEAR_COL_NAME, gsb_data_fyear_get_name ( fyear_number ),
                            FYEAR_COL_NUMBER, fyear_number,
                            FYEAR_COL_VIEW, gsb_data_fyear_get_form_show ( fyear_number ),
                            -1 );
        list_tmp = list_tmp -> next;
    }

    return FALSE;
}


/**
 * create and fill the list store of the fyear
 * come here mean that fyear_model_filter is NULL
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_fyear_create_combobox_store ( void )
{
    /* the fyear list store, contains 3 columns :
     * FYEAR_COL_NAME : the name of the fyear
     * FYEAR_COL_NUMBER : the number of the fyear
     * FYEAR_COL_VIEW : it tha fyear should be showed */

    fyear_model = GTK_TREE_MODEL ( gtk_list_store_new ( 3,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_BOOLEAN ));
    fyear_model_filter = gtk_tree_model_filter_new ( fyear_model, NULL );
    gtk_tree_model_filter_set_visible_column ( GTK_TREE_MODEL_FILTER ( fyear_model_filter ),
                        FYEAR_COL_VIEW );
    gsb_fyear_update_fyear_list ( );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean gsb_fyear_hide_iter_by_name ( GtkTreeModel *model, gchar *name )
{
    GtkTreeIter iter;
    gint result;

    result = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					     &iter );
    while (result)
    {
        gchar *value;
        gboolean show;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                        &iter,
                        FYEAR_COL_NAME, &value,
                        FYEAR_COL_VIEW, &show,
                        -1 );

        if ( g_utf8_collate ( value, name ) == 0 )
        {
            if ( show )
                gtk_list_store_set ( GTK_LIST_STORE ( model ),
                        &iter,
                        FYEAR_COL_VIEW, FALSE,
                        -1 );
            return TRUE;
        }
        result = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
                            &iter );
    }
    return FALSE;
}


/**
 * set the combobox on the fyear given in param
 * if the fyear exists but is not showed normally, we show it because it's for
 * a modification of a transaction (when the form will be freed, that fyear won't be showed again)
 *
 * \combo_box the combo-box to set
 * \tree_model
 * \tree_model_filter
 * \fyear_number the fyear we want to set on the combo-box, if 0, will be set on "Automatic"
 *
 * \return TRUE fyear found, FALSE fyear not found, nothing change
 * */
gboolean gsb_fyear_select_iter_by_number ( GtkWidget *combo_box,
                        GtkTreeModel *model,
                        GtkTreeModel *model_filter,
                        gint fyear_number )
{
    GtkTreeIter iter;
    gint result;

    if ( !combo_box )
	    return FALSE;

    if ( !model )
	    return FALSE;

    /* we look for the fyear in the model and not in the filter
     * because the fyear may not be visible */
    result = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					     &iter );
    while (result)
    {
        gint value;
        gboolean show;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                        &iter,
                        FYEAR_COL_NUMBER, &value,
                        FYEAR_COL_VIEW, &show,
                        -1 );

        if ( value == fyear_number )
        {
            GtkTreeIter child_iter;

            /* if normally not showed, we show it now and later
             * it will be back to show */
            if ( !show )
                gtk_list_store_set ( GTK_LIST_STORE ( model ),
                        &iter,
                        FYEAR_COL_VIEW, TRUE,
                        -1 );

            /* as we were in the model and not the filter, we need to change the iter */
            gtk_tree_model_filter_convert_child_iter_to_iter (
                        GTK_TREE_MODEL_FILTER ( model_filter ),
                        &child_iter,
                        &iter );
            gtk_combo_box_set_active_iter ( GTK_COMBO_BOX ( combo_box ),
                        &child_iter );

            return TRUE;
        }
        result = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),
                            &iter );
    }
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
