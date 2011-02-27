/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_report.c
 * contains tools to work with the reports
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "gsb_report.h"
#include "gsb_data_report.h"
/*END_INCLUDE*/


/**
 * this is a tree model filter with 3 columns :
 * the name, the number and a boolean to show it or not
 * */
static GtkTreeModel *report_model;

/**
 * this is a tree model filter from report_model_filter wich
 * show only the reports wich must be showed
 * */
static GtkTreeModel *report_model_filter;


enum report_list_columns {
    REPORT_COL_NAME = 0,
    REPORT_COL_NUMBER,
    REPORT_COL_VIEW,
};

/* contain the current report showed, 0 if non showed or to update the report
 * used to avoid to re-calculate the report when leave and come back to the report
 * without needing an update */
static gint current_report;


/*START_STATIC*/
static gboolean gsb_report_create_combobox_store ( void );
static gboolean gsb_report_update_report_list ( void );
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
void gsb_report_init_variables ( void )
{
    if (report_model_filter
	&&
	GTK_IS_LIST_STORE (report_model))
	gtk_list_store_clear (GTK_LIST_STORE (report_model));

    report_model = NULL;
    report_model_filter = NULL;
    current_report = 0;
}


/**
 * create and return a combobox with the reports
 *
 * \param
 *
 * \return a widget combobox or NULL
 * */
GtkWidget *gsb_report_make_combobox ( void )
{
    GtkCellRenderer *renderer;
    GtkWidget *combo_box;

    if (!report_model_filter)
	gsb_report_create_combobox_store ();

    combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL (report_model_filter));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer,
				    "text", REPORT_COL_NAME,
				    NULL);

    gtk_combo_box_set_active ( GTK_COMBO_BOX (combo_box),
			       0 );
    return (combo_box);
}




/** 
 * Get and return the number of the report in the combobox given
 * in param
 * 
 * \param combo_box a combo_box with the reports 
 * 
 * \return the number of report or 0 if problem
 * */
gint gsb_report_get_report_from_combobox ( GtkWidget *combo_box )
{
    gint report_number = 0;
    GtkTreeIter iter;

    if (!report_model_filter)
	gsb_report_create_combobox_store ();

    if (gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combo_box),
				    &iter ))
    {
	gtk_tree_model_get ( GTK_TREE_MODEL (report_model_filter),
			     &iter,
			     REPORT_COL_NUMBER, &report_number,
			     -1 );
    }
    return report_number;
}


/**
 * update the list of the reports, wich change all
 * the current combobox content
 *
 * \param
 *
 * \return FALSE
 */
gboolean gsb_report_update_report_list ( void )
{
    GSList *list_tmp;
    GtkTreeIter iter;

    /* if no filter, thats because not created, but don't create here
     * because we can come here without needed of report button */
    if (!report_model_filter)
	return FALSE;

    gtk_list_store_clear (GTK_LIST_STORE (report_model));

    /* fill the list */
    list_tmp = gsb_data_report_get_report_list ();

    while ( list_tmp )
    {
	gint report_number;

	report_number = gsb_data_report_get_report_number (list_tmp -> data);

	gtk_list_store_append ( GTK_LIST_STORE (report_model),
				&iter );
	/* for now REPORT_COL_VIEW is always TRUE, perhaps can change one day */
	gtk_list_store_set ( GTK_LIST_STORE (report_model),
			     &iter,
			     REPORT_COL_NAME, gsb_data_report_get_report_name (report_number),
			     REPORT_COL_NUMBER, report_number,
			     REPORT_COL_VIEW, TRUE,
			     -1 );
	list_tmp = list_tmp -> next;
    }
    return FALSE;
}


/**
 * create and fill the list store of the report
 * come here mean that report_model_filter is NULL
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_report_create_combobox_store ( void )
{
    /* the report list store, contains 3 columns :
     * REPORT_COL_NAME : the name of the report
     * REPORT_COL_NUMBER : the number of the report
     * REPORT_COL_VIEW : it that report should be showed */

    report_model = GTK_TREE_MODEL ( gtk_list_store_new ( 3,
							 G_TYPE_STRING,
							 G_TYPE_INT,
							 G_TYPE_BOOLEAN ));
    report_model_filter = gtk_tree_model_filter_new ( report_model,
						      NULL );
    gtk_tree_model_filter_set_visible_column ( GTK_TREE_MODEL_FILTER (report_model_filter),
					       REPORT_COL_VIEW );
    gsb_report_update_report_list ();
    return TRUE;
}


/**
 * set the current report calculated, 0 to invalidate the report
 * and re-calculate it the next time we try to show it
 *
 * \param report_number the current report or 0 to invalidate it
 *
 * \return
 * */
void gsb_report_set_current ( gint report_number )
{
    current_report = report_number;
}


/**
 * get the current report showed, or 0 if none (or need to re-calculate )
 *
 * \param
 *
 * \return	the current report number
 * */
gint gsb_report_get_current ( void )
{
    return current_report;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
