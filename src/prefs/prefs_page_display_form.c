/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_display_form.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_form.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint start_drag_column;		/* tmp for drag'n drop */
static gint start_drag_row;			/* tmp for drag'n drop */
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDisplayFormPrivate   PrefsPageDisplayFormPrivate;

struct _PrefsPageDisplayFormPrivate
{
	GtkWidget *			vbox_display_form;

	GtkWidget *			grid_buttons_cols_rows;
	GtkWidget *			button_add_col;
	GtkWidget *			button_add_row;
	GtkWidget *			button_remove_col;
	GtkWidget *			button_remove_row;
    GtkWidget *         grid_list_buttons;
    GtkWidget *			sw_list_form;
	GtkWidget *			tree_view_list_form;

	GtkWidget *			tab_list_buttons[TRANSACTION_FORM_WIDGET_NB-2];

};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDisplayForm, prefs_page_display_form, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean prefs_page_display_form_fill_store (GtkListStore *store)
{
    gint row;

    gtk_list_store_clear (store);

    for (row=0 ; row < gsb_data_form_get_nb_rows () ; row++)
    {
		GtkTreeIter iter;
		gint column;

		gtk_list_store_append (GTK_LIST_STORE (store), &iter);

		for (column = 0 ; column < gsb_data_form_get_nb_columns () ; column++)
		{
			gchar *name;
			gchar *tmp_name;

			name = g_strdup (_(gsb_form_widget_get_name (gsb_data_form_get_value (column, row))));
			tmp_name = utils_str_break_form_name_field (name, TRUNC_FORM_FIELD);

			gtk_list_store_set (GTK_LIST_STORE (store), &iter, column, tmp_name, -1);

			g_free (name);
			g_free (tmp_name);
		}
    }
    return FALSE;
}

/**
 *
 *
 * \param tree_view			form list tree_view
 * \param update_list_ope	update transactions list if necessary
 *
 * \return
 **/
static void prefs_page_display_form_update_form_list (GtkWidget *tree_view,
													  gboolean update_list_ope)
{
	GtkTreeModel *store;
	gint account_number;

    /* get the store */
    store = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	/* fill the list */
    prefs_page_display_form_fill_store (GTK_LIST_STORE (store));

	/* update the form */
	account_number = gsb_form_get_account_number();
	gsb_form_clean (account_number);
    gsb_form_create_widgets ();

	/* update tree_view  list if necessary */
	if (update_list_ope)
		gsb_transactions_list_update_tree_view (account_number, TRUE);

    utils_prefs_gsb_file_set_modified ();
}

/**
 * called when toggle a button of the form configuration, append or remove
 * the value from the tree view
 *
 * \param toggle_button the button we click
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_form_toggle_element_button (GtkWidget *toggle_button,
															   PrefsPageDisplayForm *page)
{
    gint element_number;
    gint no_second_element;
    gint i, j;
	PrefsPageDisplayFormPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_form_get_instance_private (page);

    /* get the element number */
    element_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (toggle_button),
														 "element_number"));

	/* set the second element number if necessary */
    switch (element_number)
    {
		case TRANSACTION_FORM_TYPE:
			/* c'est le mode de paiement, on met le chq */
			no_second_element = TRANSACTION_FORM_CHEQUE;
			break;

		case TRANSACTION_FORM_CHEQUE:
			/* c'est le chq, on met mode de paiement */
			no_second_element = TRANSACTION_FORM_TYPE;
			break;

		case TRANSACTION_FORM_DEVISE:
			/* c'est la devise, on met le button de change */
			no_second_element = TRANSACTION_FORM_CHANGE;
			break;

		case TRANSACTION_FORM_CHANGE:
			/* c'est le button de change, on met la devise */
			no_second_element = TRANSACTION_FORM_DEVISE;
			break;

		default:
			no_second_element = -1;
    }

    /* update the table */
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button)))
    {
		/* button is on, append the element */
		gint place_trouvee = 0;
		gint ligne_premier_elt = -1;
		gint colonne_premier_elt = -1;

		for (i=0 ; i < gsb_data_form_get_nb_rows () ; i++)
		{
			for (j=0 ; j < gsb_data_form_get_nb_columns () ; j++)
			{
				if (!gsb_data_form_get_value (j, i))
				{
					/* if only 1 element, end here, else continue to look after the second one */
					if (no_second_element == -1)
					{
						/* il n'y a qu'un elt */
						gsb_data_form_set_value (j,i,element_number);
						place_trouvee = 1;
						i = gsb_data_form_get_nb_rows ();
						j = gsb_data_form_get_nb_columns ();
					}
					else
					{
						/* there are 2 elements */
						if (ligne_premier_elt == -1)
						{
							/* found the place for the first element */
							ligne_premier_elt = i;
							colonne_premier_elt = j;
						}
						else
						{
							/* found the place for the second element */
							gsb_data_form_set_value (colonne_premier_elt,
													 ligne_premier_elt,
													 element_number);
							gsb_data_form_set_value (j, i, no_second_element);
							place_trouvee = 1;
							i = gsb_data_form_get_nb_rows ();
							j = gsb_data_form_get_nb_columns ();
						}
					}
				}
			}
		}

		if (place_trouvee)
		{
			/* there is a place for the element, active if necessary an associated element */
			if (no_second_element != -1)
			{
				g_signal_handlers_block_by_func (G_OBJECT (priv->tab_list_buttons[no_second_element-4]),
												 G_CALLBACK (prefs_page_display_form_toggle_element_button),
												 page);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->tab_list_buttons[no_second_element-4]), TRUE);
				g_signal_handlers_unblock_by_func (G_OBJECT (priv->tab_list_buttons[no_second_element-4]),
												   G_CALLBACK (prefs_page_display_form_toggle_element_button),
												   page);
			}
		}
		else
		{
			/* there is no place to add an element */
			g_signal_handlers_block_by_func (G_OBJECT (toggle_button),
											 G_CALLBACK (prefs_page_display_form_toggle_element_button),
											 page);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_button), FALSE);
			g_signal_handlers_unblock_by_func (G_OBJECT (toggle_button),
											   G_CALLBACK (prefs_page_display_form_toggle_element_button),
											   page);

			if (no_second_element == -1)
				dialogue_hint (_("There is no place enough to put the element. You need to increase "
								  "the number of rows or columns to add an element."),
								_("The table is full"));
			else
				dialogue_hint (_("There is no place enough to put the two elements (you have clicked on "
								  "an element which contains two). You need to increase the number of rows "
								  "or columns to add the elements."),
								_("The table is full"));

			return TRUE;
		}
    }
    else
    {
		/* un-toggle the button */
		if (no_second_element != -1)
		{
			g_signal_handlers_block_by_func (G_OBJECT (priv->tab_list_buttons[no_second_element-4]),
											 G_CALLBACK (prefs_page_display_form_toggle_element_button),
											 page);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->tab_list_buttons[no_second_element-4]), FALSE);
			g_signal_handlers_unblock_by_func (G_OBJECT (priv->tab_list_buttons[no_second_element-4]),
											   G_CALLBACK (prefs_page_display_form_toggle_element_button),
											   page);
		}

		for (i=0 ; i < gsb_data_form_get_nb_rows () ; i++)
		{
			for (j=0 ; j < gsb_data_form_get_nb_columns () ; j++)
			{
				if (gsb_data_form_get_value (j, i) == element_number)
				{
					gsb_data_form_set_value (j, i, 0);
					if (no_second_element == -1)
					{
						i = gsb_data_form_get_nb_rows ();
						j = gsb_data_form_get_nb_columns ();
					}
					else
					{
						element_number = no_second_element;
						no_second_element = -1;
						i = 0;
						j = 0;
					}
				}
			}
		}
    }

	/* update form and list */
	prefs_page_display_form_update_form_list (priv->tree_view_list_form, TRUE);

    return FALSE;
}

/**
 * Set the correct buttons as active/passive
 *
 * \param account_number
 *
 * \return FALSE
 * */
static gboolean prefs_page_display_form_set_buttons_table (PrefsPageDisplayForm *page)
{
    gint column;
    gint current_element_number;
	gint nbre_cols;
    gint row;
	PrefsPageDisplayFormPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_form_get_instance_private (page);

    /* show/hide the necessary columns in the tree view */
	nbre_cols = gsb_data_form_get_nb_columns ();
    for (column=0 ; column < MAX_WIDTH ; column++)
	gtk_tree_view_column_set_visible (gtk_tree_view_get_column (GTK_TREE_VIEW (priv->tree_view_list_form),
																column),
									  column < nbre_cols);

    /* active/unactive the buttons, begin on the fourth element number because the
     * date, debit and credit are obligatory */

    current_element_number = 4;

    for (row=0 ; row<3 ; row++)
	for (column=0 ; column<6 ; column++)
	{
	    if (column + row*6 < TRANSACTION_FORM_WIDGET_NB - 3
			&&
			priv->tab_list_buttons[column + row*6])
	    {
			g_signal_handlers_block_by_func (G_OBJECT (priv->tab_list_buttons[column + row*6]),
											 G_CALLBACK (prefs_page_display_form_toggle_element_button),
											 page);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->tab_list_buttons[column + row*6]),
										  gsb_data_form_look_for_value (current_element_number, NULL, NULL));
			g_signal_handlers_unblock_by_func (G_OBJECT (priv->tab_list_buttons[column + row*6]),
											   G_CALLBACK (prefs_page_display_form_toggle_element_button),
											   page);
	    }
	    current_element_number++;
	}
    return FALSE;
}

/**
 * create the table of buttons and set the signals
 *
 * \param
 *
 * \return the new table of buttons
 **/
static void prefs_page_display_form_create_buttons_table (PrefsPageDisplayForm *page)
{
    gint current_element_number;
    gint column;
    gint max_column;
    gint max_row;
    gint row;
	gint button_width;
	PrefsPageDisplayFormPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_form_get_instance_private (page);

	/* calcul du nombre de colonnes et de lignes */
	if (grisbi_app_get_low_definition_screen ())
	{
		max_column = 5;
		max_row = 6;
	}
	else
	{
		max_column = 7;
		max_row = 4;
	}

	/* calcul de la largeur du bouton */
	button_width = (SW_MAX_CONTENT_WIDTH - 24)/max_column-1;

    /* the date, debit and credit are obligatory, so begin to number 4 */
    current_element_number = 4;

    for (row = 0 ; row < max_row-1 ; row++)
	for (column = 0 ; column < max_column-1 ; column++)
	{
	    const gchar *string;
	    gchar *changed_string;
		gchar *tmp_str;

	    string = _(gsb_form_widget_get_name (current_element_number));
	    if (string)
	    {
			/* the max string in the button is 10 characters */
			changed_string = limit_string (string, 10);

			priv->tab_list_buttons[current_element_number-4] = gtk_toggle_button_new_with_label (changed_string);
			gtk_widget_set_size_request (priv->tab_list_buttons[current_element_number-4], button_width, -1);
			gtk_widget_set_hexpand (priv->tab_list_buttons[current_element_number-4], TRUE);
			gtk_widget_set_name (priv->tab_list_buttons[current_element_number-4], "list_config_buttons");
			g_object_set_data (G_OBJECT (priv->tab_list_buttons[current_element_number-4]),
							   "element_number",
							   GINT_TO_POINTER (current_element_number));
			g_signal_connect (G_OBJECT (priv->tab_list_buttons[current_element_number-4]),
							  "toggled",
							  G_CALLBACK (prefs_page_display_form_toggle_element_button),
							  page);
			gtk_grid_attach (GTK_GRID (priv->grid_list_buttons),
							 priv->tab_list_buttons[current_element_number-4],
							 column,
							 row,
							 1,
							 1);

			/* set the tooltip with the real name */
			tmp_str = g_strconcat (" ", string, " ", NULL);
			gtk_widget_set_tooltip_text (GTK_WIDGET (priv->tab_list_buttons[current_element_number-4]), tmp_str);
			g_free (changed_string);
			g_free (tmp_str);
	    }
		else
			break;

	    current_element_number++;
	}
}

/**
 * check if we can remove a row or a column according the number
 * of values inside
 * it is possible to remove a row or column only if grisbi can replace
 * the hidden element in the form without that row/column
 *
 * \param account_number
 * \param removing_row if TRUE it's a row we want to remove, else it's a column
 *
 * \return TRUE ok we can remove it, FALSE else
 * */
static gboolean prefs_page_display_form_check_for_removing (gint removing_row)
{
    gint values;
    gint rows;
    gint columns;

    rows = gsb_data_form_get_nb_rows ();
    columns = gsb_data_form_get_nb_columns ();

	if (!rows || !columns)
		return FALSE;

	if (removing_row)
		rows--;
	else
		columns--;

    /* the minimum of values is 3 : date, debit, credit*/
	if (rows * columns < 3)
		return FALSE;

    values = gsb_data_form_get_values_total ();
	if (values <= rows*columns)
		return TRUE;
	else
		return FALSE;
}

/**
 * called when the user click on "add column"
 *
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_form_add_column (GtkButton *button,
													GtkWidget *tree_view)
{
    gint nb_columns;

    nb_columns = gsb_data_form_get_nb_columns ();

	if (nb_columns == MAX_WIDTH)
		return FALSE;

    /* add the new column */
    gsb_data_form_set_nb_columns (nb_columns + 1);

	/* show the new column */
	gtk_tree_view_column_set_visible (gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view),
																nb_columns),
									  TRUE);

	/* update form and list */
	prefs_page_display_form_update_form_list (tree_view, FALSE);

	return FALSE;
}

/**
 * called when the user click on "remove column"
 *
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_form_remove_column (GtkButton *button,
													   GtkWidget *tree_view)
{
    gint row;
    gint nb_columns;

    nb_columns = gsb_data_form_get_nb_columns ();

	if (nb_columns == 1)
		return FALSE;

    /* check if it's possible */
	if (!prefs_page_display_form_check_for_removing (0))
		return FALSE;

    /* erase the last column */
    nb_columns--;
    gsb_data_form_set_nb_columns (nb_columns);

    /* move the values in the last column to another place */
    for (row = 0 ; row< gsb_data_form_get_nb_rows () ; row++)
    {
		if (gsb_data_form_get_value (nb_columns, row))
		{
			/* found something, look for the first place to set it */
			gint tmp_column;
			gint tmp_row;

			for (tmp_row = 0 ; tmp_row< gsb_data_form_get_nb_rows () ; tmp_row++)
			for (tmp_column = 0 ; tmp_column<nb_columns ; tmp_column++)
			{
				if (!gsb_data_form_get_value (tmp_column, tmp_row))
				{
					gsb_data_form_set_value (tmp_column,
											 tmp_row,
											 gsb_data_form_get_value (nb_columns, row));
					gsb_data_form_set_value (nb_columns, row, 0);
					tmp_row = gsb_data_form_get_nb_rows ();
					tmp_column = nb_columns;
				}
			}
		}
    }

	/* hide the new column */
	gtk_tree_view_column_set_visible (gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view),
																nb_columns),
									  FALSE);

	/* update form and list */
	prefs_page_display_form_update_form_list (tree_view, FALSE);

    return FALSE;
}

/**
 * called when the user click on "add line"
 *
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_form_add_row (GtkButton *button,
												 GtkWidget *tree_view)
{
	devel_debug (NULL);

	if (gsb_data_form_get_nb_rows () == MAX_HEIGHT)
		return FALSE;

	gsb_data_form_set_nb_rows (gsb_data_form_get_nb_rows () + 1);

	/* update form and list */
	prefs_page_display_form_update_form_list (tree_view, FALSE);

    return FALSE;
}

/**
 * called when the user click on "remove line"
 *
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_form_remove_row (GtkButton *button,
													GtkWidget *tree_view)
{
    gint column;
    gint nb_rows;
    gint nb_columns;

    nb_rows = gsb_data_form_get_nb_rows ();

	if (nb_rows == 1)
		return FALSE;

    /* check if it's possible */
	if (!prefs_page_display_form_check_for_removing (1))
		return FALSE;

    nb_columns = gsb_data_form_get_nb_columns ();

    /* remove the row */
    nb_rows--;
    gsb_data_form_set_nb_rows (nb_rows);

    /* move automatickly the values inside the new tinier form */
    for (column=0 ; column< nb_columns ; column++)
    {
		if (gsb_data_form_get_value (column, nb_rows))
		{
			/* there is something inside the part which will be removed, so look for the first */
			/* place possible to move it */

			gint tmp_row, tmp_column;

			for (tmp_row=0 ; tmp_row < nb_rows ; tmp_row++)
			for (tmp_column=0 ; tmp_column < nb_columns ; tmp_column++)
			{
				if (!gsb_data_form_get_value (tmp_column, tmp_row))
				{
					gsb_data_form_set_value (tmp_column,
											 tmp_row,
											 gsb_data_form_get_value (column, nb_rows));
					gsb_data_form_set_value (column, nb_rows, 0);
					tmp_row = nb_rows;
					tmp_column = nb_columns;
				}
			}
		}
    }

	/* update form and list */
	prefs_page_display_form_update_form_list (tree_view, FALSE);

    return FALSE;
}

/**
 * called when we begin a drag,
 * find what cell was under the cursor and change it
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 * */
static gboolean prefs_page_display_form_drag_begin (GtkWidget *tree_view,
													GdkDragContext *drag_context,
													gpointer null)
{
    GdkDevice *device;
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;

    /* get the cell coord */
    device = gdk_drag_context_get_device (drag_context);
    gdk_window_get_device_position (gtk_tree_view_get_bin_window (GTK_TREE_VIEW (tree_view)),
                                    device,
                                    &x,
                                    &y,
                                    NULL);
    gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
                                   x,
                                   y,
                                   &path,
                                   &tree_column,
                                   NULL,
                                   NULL);

	if (!path || !tree_column)
		return FALSE;

    start_drag_column = g_list_index (gtk_tree_view_get_columns (GTK_TREE_VIEW (tree_view)), tree_column);
    start_drag_row = utils_str_atoi (gtk_tree_path_to_string (path));

    return FALSE;
}


/**
 * called when we end a drag,
 * find what cell was under the cursor and do the split between the 2 cells
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_form_drag_end (GtkWidget *tree_view,
												  GdkDragContext *drag_context,
												  PrefsPageDisplayForm *page)
{
    GdkDevice *device;
    gint x, y;
    GtkTreeModel *store;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gint end_drag_row;
    gint end_drag_column;
    gint buffer;

	PrefsPageDisplayFormPrivate *priv;

	priv = prefs_page_display_form_get_instance_private (page);

    /* get the cell position */
    device = gdk_drag_context_get_device (drag_context);
    gdk_window_get_device_position (gtk_tree_view_get_bin_window (GTK_TREE_VIEW (tree_view)),
                                    device,
                                    &x,
                                    &y,
                                    NULL);
    gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
                                   x,
                                   y,
                                   &path,
                                   &tree_column,
                                   NULL,
                                   NULL);

	if (!path || !tree_column)
		return FALSE;

    end_drag_column = g_list_index (gtk_tree_view_get_columns (GTK_TREE_VIEW (tree_view)), tree_column);
    end_drag_row = utils_str_atoi (gtk_tree_path_to_string (path));

    /* if we are on the same cell, go away */
	if (start_drag_row == end_drag_row && start_drag_column == end_drag_column)
		return (FALSE);

    /* swap the cells in the tab */
    buffer = gsb_data_form_get_value (start_drag_column, start_drag_row);
    gsb_data_form_set_value (start_drag_column,
							 start_drag_row,
							 gsb_data_form_get_value (end_drag_column, end_drag_row));
    gsb_data_form_set_value (end_drag_column, end_drag_row, buffer);

    /* get the store */
    store = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->tree_view_list_form));

	/* fill the list */
    prefs_page_display_form_fill_store (GTK_LIST_STORE (store));

	/* update form and list */
	prefs_page_display_form_update_form_list (tree_view, TRUE);

	return (FALSE);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *prefs_page_display_form_create_tree_view (PrefsPageDisplayForm *page)
{
    GtkWidget *tree_view;
    GtkListStore *store;
	GtkTreeSelection *selection;
    const gchar *columns_titles[6] = {N_("Col 1"), N_("Col 2"), N_("Col 3"), N_("Col 4"), N_("Col 5"), N_("Col 6")};
    gint column;
    GtkTargetEntry target_entry[] = {{(gchar*) "text", GTK_TARGET_SAME_WIDGET, 0}};

	devel_debug (NULL);

	tree_view = gtk_tree_view_new ();
	gtk_widget_set_name (tree_view, "tree_view");
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_GRID_LINES_BOTH);

	/* create store */
    store = gtk_list_store_new (MAX_WIDTH,
								G_TYPE_STRING,		/* Col1 */
								G_TYPE_STRING,		/* Col2 */
								G_TYPE_STRING,		/* Col3 */
								G_TYPE_STRING,		/* Col4 */
								G_TYPE_STRING,		/* Col5 */
								G_TYPE_STRING);		/* Col6 */

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT(store));

	/* set the columns */
    for (column=0 ; column< MAX_WIDTH ; column++)
    {
		GtkTreeViewColumn *tree_view_column;

		tree_view_column = gtk_tree_view_column_new_with_attributes (columns_titles[column],
																	 gtk_cell_renderer_text_new (),
										 							 "text", column,
										 							 NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (tree_view_column));
		gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (tree_view_column), GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN (tree_view_column), TRUE);
    }

    /* enable the drag'n drop, we need to use low-level api because
     * gtk_tree_view api can only move the entire row, not only a cell
     * (at least, didn't find how...) */
    gtk_drag_source_set (tree_view,
						 GDK_BUTTON1_MASK,
			 			 target_entry, 1,
			 			 GDK_ACTION_MOVE);
    g_signal_connect (G_OBJECT (tree_view),
					  "drag-begin",
					  G_CALLBACK (prefs_page_display_form_drag_begin),
					  NULL);
    gtk_drag_dest_set (tree_view,
					   GTK_DEST_DEFAULT_ALL,
					   target_entry, 1,
					   GDK_ACTION_MOVE);
	g_signal_connect (G_OBJECT (tree_view),
					  "drag-end",
					  G_CALLBACK (prefs_page_display_form_drag_end),
					  page);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_NONE);

	prefs_page_display_form_fill_store (store);

    return tree_view;
}

/**
 * Création de la page de gestion des display_form
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_display_form_setup_page (PrefsPageDisplayForm *page)
{
	GtkWidget *head_page;
	gboolean is_loading;
	PrefsPageDisplayFormPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_form_get_instance_private (page);
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Transaction form"), "gsb-form-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_display_form), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_display_form), head_page, 0);

	/* set tree_view */
	priv->tree_view_list_form = prefs_page_display_form_create_tree_view (page);
	gtk_container_add (GTK_CONTAINER (priv->sw_list_form), priv->tree_view_list_form);

	/* set the buttons line to increase/decrease the form */
    g_signal_connect (G_OBJECT (priv->button_add_col),
					  "clicked",
					  G_CALLBACK (prefs_page_display_form_add_column),
					  priv->tree_view_list_form);
    g_signal_connect (G_OBJECT (priv->button_remove_col),
					  "clicked",
		     		  G_CALLBACK (prefs_page_display_form_remove_column),
					  priv->tree_view_list_form);

    g_signal_connect (G_OBJECT (priv->button_add_row),
					  "clicked",
		      		  G_CALLBACK (prefs_page_display_form_add_row),
					  priv->tree_view_list_form);
    g_signal_connect (G_OBJECT (priv->button_remove_row),
					  "clicked",
		       		  G_CALLBACK (prefs_page_display_form_remove_row),
					  priv->tree_view_list_form);

	/* set table des boutons */
	prefs_page_display_form_create_buttons_table (page);
	prefs_page_display_form_set_buttons_table (page);

	if (is_loading)
	{
		gtk_widget_set_sensitive (priv->tree_view_list_form, TRUE);
		gtk_widget_set_sensitive (priv->grid_buttons_cols_rows, TRUE);
		gtk_widget_set_sensitive (priv->grid_list_buttons, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->tree_view_list_form, FALSE);
		gtk_widget_set_sensitive (priv->grid_buttons_cols_rows, FALSE);
		gtk_widget_set_sensitive (priv->grid_list_buttons, FALSE);
	}
		gtk_widget_show_all (priv->vbox_display_form);

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_display_form_init (PrefsPageDisplayForm *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_display_form_setup_page (page);
}

static void prefs_page_display_form_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_display_form_parent_class)->dispose (object);
}

static void prefs_page_display_form_class_init (PrefsPageDisplayFormClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_display_form_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_display_form.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, vbox_display_form);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, grid_buttons_cols_rows);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, button_add_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, button_add_row);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, button_remove_col);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, button_remove_row);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, sw_list_form);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayForm, grid_list_buttons);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
PrefsPageDisplayForm *prefs_page_display_form_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_DISPLAY_FORM_TYPE, NULL);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

