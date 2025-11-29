/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                                */
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
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_display_ope.h"
#include "custom_list.h"
#include "grisbi_app.h"
#include "gsb_file.h"
#include "structures.h"
#include "gsb_transactions_list.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint start_drag_column;		/** tmp for drag'n drop */
static gint start_drag_row;			/** tmp for drag'n drop */
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDisplayOpePrivate   PrefsPageDisplayOpePrivate;

struct _PrefsPageDisplayOpePrivate
{
	GtkWidget *			vbox_display_ope;

    GtkWidget *         grid_list_buttons;
    GtkWidget *			sw_list_ope;
	GtkWidget *			tree_view_list_ope;

	GtkWidget *			tab_list_buttons[18];
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDisplayOpe, prefs_page_display_ope, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * fill the configuration store according to the organization for transaction list
 *
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_ope_fill_store (GtkListStore *store)
{
    gchar *row[CUSTOM_MODEL_VISIBLE_COLUMNS];
    gint i;
    gint j;
	gint *ptr;

	devel_debug (NULL);

    gtk_list_store_clear (store);

	ptr = gsb_transactions_list_get_tab_affichage_ope ();
    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
    {
        GtkTreeIter iter;

        gtk_list_store_append (GTK_LIST_STORE (store), &iter);

        for (j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
        {
			gchar *tmp_str;
			gint element_number;

			element_number = *(ptr + (i * CUSTOM_MODEL_VISIBLE_COLUMNS) + j);
			tmp_str = gsb_transactions_list_get_column_title_from_element (element_number-1);
            row[j] = utils_str_break_form_name_field (tmp_str, TRUNC_LIST_COL_NAME);
			g_free (tmp_str);

			/* on met le nom dans les lignes paires et le numéro de l'élément dans les lignes impaires */
            gtk_list_store_set (GTK_LIST_STORE (store), &iter, 2*j, row[j], 2*j+1, element_number, -1);

            if (row[j])
                g_free (row[j]);
        }
    }

    return FALSE;
}

/**
 * called when toggle a button of the form configuration, append or remove
 * the value from the tree view
 *
 * \param 	toggle_button the button we click
 * \param	tree_view
 *
 * \return FALSE
 **/
static void prefs_page_display_ope_toggle_element_button (GtkWidget *toggle_button,
														  GtkWidget *tree_view)
{
    GtkTreeModel *store;
	gint element;
	gint *ptr;

	devel_debug (NULL);

	/* get store */
    store = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    /* get the element number */
    element = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (toggle_button), "element_number"));
	ptr = gsb_transactions_list_get_tab_affichage_ope ();

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button)))
    {
        gint row, column = 0;
        gboolean place_trouvee = FALSE;

        /* button is on, append the element */
        for (row = 3 ; row >= 0 ; row--)
        {
            for (column = 6 ; column >= 0 ; column--)
            {
                gint tmp_element;

                tmp_element = *(ptr + (row * CUSTOM_MODEL_VISIBLE_COLUMNS) + column);
                if (tmp_element == 0)
                {
                    place_trouvee = TRUE;
                    break;
                }
            }
            if (place_trouvee)
                break;
        }

        if (place_trouvee)
        {
            /* on sauvegarde la position du nouvel élément */
			gsb_transactions_list_set_element_tab_affichage_ope (element, row, column);

			/* met à jour la liste des opérations */
            transaction_list_update_element (element);
        }
    }
    else
    {
        GtkTreeIter iter;

        /* on supprime la donnée dans la liste */
        if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
        {
            gint i = 0, j;

            do
            {
                for (j = 0; j < CUSTOM_MODEL_VISIBLE_COLUMNS; j++)
                {
                    gint num = 0;

                    gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 2*j+1, &num, -1);
                    if (element == num)
                    {
						gsb_transactions_list_set_element_tab_affichage_ope (0, i, j);

						/* met à jour la liste des opérations */
                        transaction_list_update_cell (j, i);

						break;
                    }
                }
                i++;
            }
            while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));
        }
    }

    /* fill the list */
    prefs_page_display_ope_fill_store (GTK_LIST_STORE (store));

    /* on finit de mettre à jour la liste des opérations */
	gsb_transactions_list_set_titles_tips_col_list_ope ();
	gsb_transactions_list_update_titres_tree_view ();

    gsb_file_set_modified (TRUE);
}

/**
 * set the correct buttons as active/passive
 *
 * \param search string
 *
 * \return FALSE
 * */
static gboolean prefs_page_display_ope_button_set_active_from_string (PrefsPageDisplayOpe *page,
															 		  gchar *string,
															 		  gboolean active)
{
    gint i;
	PrefsPageDisplayOpePrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_ope_get_instance_private (page);

    for (i = 0 ; i < 18 ; i++)
	{
        gchar *tmp_str;

        tmp_str = gtk_widget_get_tooltip_text (priv->tab_list_buttons[i]);

        if (string && g_utf8_collate (string, tmp_str) == 0)
        {
            g_signal_handlers_block_by_func (G_OBJECT (priv->tab_list_buttons[i]),
											 G_CALLBACK (prefs_page_display_ope_toggle_element_button),
											 priv->tree_view_list_ope);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->tab_list_buttons[i]), active);
            g_signal_handlers_unblock_by_func (G_OBJECT (priv->tab_list_buttons[i]),
											   G_CALLBACK (prefs_page_display_ope_toggle_element_button),
											   priv->tree_view_list_ope);

            g_free (tmp_str);

            return TRUE;
        }
    }

    return FALSE;
}

/**
 * set the correct buttons as active/passive
 *
 * \param page
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_ope_set_buttons_table (PrefsPageDisplayOpe *page)
{
    GtkTreeModel *store;
    gint i;
	PrefsPageDisplayOpePrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_ope_get_instance_private (page);

    /* get the store */
    store = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->tree_view_list_ope));

    /* active/unactive the buttons */
    for (i = 0 ; i < 18 ; i++)
	{
        GtkTreeIter iter;
        gint current_number;

        current_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (priv->tab_list_buttons[i]),
															 "element_number"));

        if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter))
        {
            do
            {
                gint num_1 = 0;
                gint num_2 = 0;
                gint num_3 = 0;
                gint num_4 = 0;
                gint num_5 = 0;
                gint num_6 = 0;
                gint num_7 = 0;

                gtk_tree_model_get (GTK_TREE_MODEL (store),
									&iter,
									1, &num_1,
									3, &num_2,
									5, &num_3,
									7, &num_4,
									9, &num_5,
									11, &num_6,
									13, &num_7,
									-1);

                if (current_number == num_1 ||
					current_number == num_2 ||
					current_number == num_3 ||
					current_number == num_4 ||
					current_number == num_5 ||
					current_number == num_6 ||
					current_number == num_7)
                {
                    g_signal_handlers_block_by_func (G_OBJECT (priv->tab_list_buttons[i]),
													 G_CALLBACK (prefs_page_display_ope_toggle_element_button),
													 priv->tree_view_list_ope);
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->tab_list_buttons[i]), TRUE);

                    g_signal_handlers_unblock_by_func (G_OBJECT (priv->tab_list_buttons[i]),
													   G_CALLBACK (prefs_page_display_ope_toggle_element_button),
													   priv->tree_view_list_ope);
                }
            }
            while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));
        }
    }

    return FALSE;
}

/**
 * create the table of buttons and set the signals
 *
 * \param	page
 *
 * \return the new table of buttons
 **/
static void prefs_page_display_ope_create_buttons_table (PrefsPageDisplayOpe *page)
{
    gint current_number = 0;
    gint column;
    gint max_column;
    gint max_row;
    gint row;
	gint button_width;
	PrefsPageDisplayOpePrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_display_ope_get_instance_private (page);

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

    for (row = 0 ; row < max_row-1 ; row++)
	for (column = 0 ; column < max_column-1 ; column++)
	{
	    gchar *string;
	    gchar *changed_string;

	    string = gsb_transactions_list_get_column_title_from_element (current_number);
	    if (string)
	    {
            /* the max string in the button is 10 characters */
            changed_string = limit_string (string, 10);

            priv->tab_list_buttons[current_number] = gtk_toggle_button_new_with_label (changed_string);
            gtk_widget_set_size_request (priv->tab_list_buttons[current_number], button_width, -1);
            gtk_widget_set_name (priv->tab_list_buttons[current_number], "list_config_buttons");
			gtk_widget_set_hexpand (priv->tab_list_buttons[current_number], TRUE);
            g_object_set_data (G_OBJECT (priv->tab_list_buttons[current_number]),
							   "element_number",
							   GINT_TO_POINTER (current_number + 1));
            g_signal_connect (G_OBJECT (priv->tab_list_buttons[current_number]),
							  "toggled",
							  G_CALLBACK (prefs_page_display_ope_toggle_element_button),
							  priv->tree_view_list_ope);
            gtk_grid_attach (GTK_GRID (priv->grid_list_buttons),
							 priv->tab_list_buttons[current_number],
							 column, row, 1,1);

            /* set the tooltip with the real name */
            gtk_widget_set_tooltip_text (GTK_WIDGET (priv->tab_list_buttons[current_number]), string);

            g_free (string);
            g_free (changed_string);
	    }
		else
			break;

	    current_number++;
	}
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
 **/
static gboolean prefs_page_display_ope_drag_begin (GtkWidget *tree_view,
												   GdkDragContext *drag_context,
												   gpointer null)
{
    GdkDevice *device;
    GdkRectangle rectangle;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gint x;
    gint y;

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

    /* draw the new cursor */
    gtk_tree_view_get_cell_area (GTK_TREE_VIEW (tree_view),
								 path,
								 tree_column,
								 &rectangle);

	if (start_drag_row == 0)
		gtk_tree_view_column_set_title  (tree_column, "");

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
static gboolean prefs_page_display_ope_drag_end (GtkWidget *tree_view,
												 GdkDragContext *drag_context,
												 PrefsPageDisplayOpe *page)
{
    GdkDevice *device;
    GtkTreePath *path;
    GtkTreeModel *store;
    GtkTreeViewColumn *tree_column;
    gchar *string;
    gint element;
    gint end_drag_row;
    gint end_drag_column;
    gint old_element;
    gint x;
    gint y;
	gint *ptr;

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

	ptr = gsb_transactions_list_get_tab_affichage_ope ();

    element = *(ptr + (start_drag_row * CUSTOM_MODEL_VISIBLE_COLUMNS) + start_drag_column);

    /* save the old position et désensitive le bouton correspondant */
    old_element = *(ptr + (end_drag_row * CUSTOM_MODEL_VISIBLE_COLUMNS) + end_drag_column);

    if (old_element)
    {
        string = gsb_transactions_list_get_column_title_from_element (old_element - 1);
        prefs_page_display_ope_button_set_active_from_string (page, string, FALSE);

        g_free (string);
    }

    /* positionne le nouvel élément */
	*(ptr + (end_drag_row * CUSTOM_MODEL_VISIBLE_COLUMNS) + end_drag_column) = element;

    /* the element was already showed, we need to erase the last cell first */
	*(ptr + (start_drag_row * CUSTOM_MODEL_VISIBLE_COLUMNS) + start_drag_column) = 0;

    transaction_list_update_cell (start_drag_column, start_drag_row);

    /* modifie le titre de la colonne si nécessaire */
    if (end_drag_row == 0)
    {
        string = gsb_transactions_list_get_column_title_from_element (element - 1);
        gtk_tree_view_column_set_title  (tree_column, string);

        g_free (string);
    }

    /* fill the list */
    store = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    prefs_page_display_ope_fill_store (GTK_LIST_STORE (store));

    /* met à jour la liste des opérations */
    transaction_list_update_element (element);
    gsb_transactions_list_set_titles_tips_col_list_ope ();
    gsb_transactions_list_update_titres_tree_view ();

    gsb_file_set_modified (TRUE);
    return (FALSE);
}

/**
 * Init the tree_view for the transaction list configuration
 * set the model given in param
 * set the columns and all the connections
 *
 * \param model the model to set in the tree_view
 *
 * \return the tree_view
 **/
static GtkWidget *prefs_page_display_ope_create_tree_view (PrefsPageDisplayOpe *page)
{
	GtkWidget *tree_view;
	GtkListStore *store = NULL;
	GtkTreeSelection *selection;
    gint column;
    GtkTargetEntry target_entry[] = { {(gchar*) "text", GTK_TARGET_SAME_WIDGET, 0 } };
	gint *pointer;

	devel_debug (NULL);
	pointer = gsb_transactions_list_get_tab_align_col_treeview ();

	tree_view = gtk_tree_view_new ();
	gtk_widget_set_name (tree_view, "tree_view");
    gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_GRID_LINES_BOTH);

	/* create store */
	store = gtk_list_store_new (2 * CUSTOM_MODEL_VISIBLE_COLUMNS,
							G_TYPE_STRING,	/* first name of visible column */
							G_TYPE_INT,		/* first number of visible column */
							G_TYPE_STRING,	/* second name */
							G_TYPE_INT,		/* ... */
							G_TYPE_STRING,	/* third name */
							G_TYPE_INT,		/* ... */
							G_TYPE_STRING,	/* fourth name */
							G_TYPE_INT,		/* ... */
							G_TYPE_STRING,	/* fifth name */
							G_TYPE_INT,		/* ... */
							G_TYPE_STRING,	/* sixth name */
							G_TYPE_INT,		/* ... */
							G_TYPE_STRING,	/* seventh name */
							G_TYPE_INT);	/* ... */

	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT(store));

    /* set the columns */
    for (column=0 ; column < CUSTOM_MODEL_VISIBLE_COLUMNS ; column++)
    {
        GtkTreeViewColumn *tree_view_column;
		GtkCellRenderer *cell_renderer;
		gchar *title_col = NULL;

		title_col = gsb_transactions_list_get_column_title (0, column);
		cell_renderer = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell_renderer),
					  "xalign",
					  (gfloat)pointer[column]/2,
					  NULL);

        tree_view_column = gtk_tree_view_column_new_with_attributes (title_col,
																	cell_renderer,
																	"text", 2*column,
																	NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
									 GTK_TREE_VIEW_COLUMN (tree_view_column));
		gtk_tree_view_column_set_alignment (GTK_TREE_VIEW_COLUMN (tree_view_column),
											(gfloat)pointer[column]/2);
        gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (tree_view_column),
										 GTK_TREE_VIEW_COLUMN_FIXED);
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
                        G_CALLBACK (prefs_page_display_ope_drag_begin),
                        NULL);

    gtk_drag_dest_set (tree_view,
                        GTK_DEST_DEFAULT_ALL,
                        target_entry, 1,
                        GDK_ACTION_MOVE);
    g_signal_connect (G_OBJECT (tree_view),
                        "drag-end",
                        G_CALLBACK (prefs_page_display_ope_drag_end),
                        page);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_NONE);

	prefs_page_display_ope_fill_store (store);

	return tree_view;
}

/**
 * Création de la page de gestion des display_ope
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_display_ope_setup_page (PrefsPageDisplayOpe *page)
{
	GtkWidget *head_page;
	gboolean is_loading;
	PrefsPageDisplayOpePrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_ope_get_instance_private (page);
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Transactions list cells"),
															   "gsb-transaction-list-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_display_ope), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_display_ope), head_page, 0);

	if (!is_loading)
	    gsb_transactions_list_init_tab_affichage_ope (NULL);

	/* set tree_view */
	priv->tree_view_list_ope = prefs_page_display_ope_create_tree_view (page);
	gtk_container_add (GTK_CONTAINER (priv->sw_list_ope), priv->tree_view_list_ope);

	/* set table des boutons */
	prefs_page_display_ope_create_buttons_table (page);
	prefs_page_display_ope_set_buttons_table (page);

	if (is_loading)
	{
		gtk_widget_set_sensitive (priv->tree_view_list_ope, TRUE);
		gtk_widget_set_sensitive (priv->grid_list_buttons, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->tree_view_list_ope, FALSE);
		gtk_widget_set_sensitive (priv->grid_list_buttons, FALSE);
	}

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_display_ope_init (PrefsPageDisplayOpe *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_display_ope_setup_page (page);
}

static void prefs_page_display_ope_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_display_ope_parent_class)->dispose (object);
}

static void prefs_page_display_ope_class_init (PrefsPageDisplayOpeClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_display_ope_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_display_ope.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayOpe, vbox_display_ope);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayOpe, sw_list_ope);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayOpe, grid_list_buttons);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageDisplayOpe *prefs_page_display_ope_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_DISPLAY_OPE_TYPE, NULL);
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

