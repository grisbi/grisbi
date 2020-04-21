/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2006-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          https://www.grisbi.org/                                           */
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
 * \file gsb_currency_config.c
 * contains the part to set the configuration of the currencies
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_currency_config.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "parametres.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_locale.h"
#include "traitement_variables.h"
#include "utils.h"
#include "tiers_onglet.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "utils_str.h"
#include "structures.h"
#include "utils_files.h"
#include "erreur.h"
#include "gsb_dirs.h"
#include "utils_prefs.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *delete_devise_button;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_GLOBAL*/
/*END_GLOBAL*/

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/**
 * append the given currency in the list of known currencies
 * call both for known currencies and word currencies
 *
 * \param model the tree_model of the known currencies
 * \param currency_number
 *
 * \return
 *
 **/
static void gsb_currency_append_currency_to_list (GtkListStore *model,
												  gint currency_number)
{
    GdkPixbuf *pixbuf;
    GtkTreeIter iter;
    gchar *string;

    string = g_strconcat (gsb_dirs_get_pixmaps_dir (),
						  G_DIR_SEPARATOR_S,
						  "flags",
						  G_DIR_SEPARATOR_S,
						  gsb_data_currency_get_code_iso4217 (currency_number),
						  ".png",
						  NULL);
    pixbuf = gdk_pixbuf_new_from_file (string, NULL);
    g_free (string);

    gtk_list_store_append (GTK_LIST_STORE(model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model),
						&iter,
						CURRENCY_FLAG_COLUMN, pixbuf,
						CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
						CURRENCY_CODE_ISO_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
						CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_nickname (currency_number),
						CURRENCY_FLOATING_COLUMN, gsb_data_currency_get_floating_point (currency_number),
						CURRENCY_NUMBER_COLUMN, currency_number,
						CURRENCY_HAS_FLAG, TRUE,
						-1);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_currency_config_change_selection (GtkTreeSelection *selection,
													  gpointer null)
{
    GSList *tmp_list;
    gint nbre_devises;

    tmp_list = gsb_data_currency_get_currency_list ();
    nbre_devises = g_slist_length (tmp_list);
    if (delete_devise_button)
    {
        if (nbre_devises > 1)
            gtk_widget_set_sensitive (delete_devise_button, TRUE);
        else
            gtk_widget_set_sensitive (delete_devise_button, FALSE);
    }
    return FALSE;
}

/**
 * create the tree which contains the currency list
 * used both for the currency known list, and the currency to add list
 *
 * \param
 *
 * \return a GtkTreeView
 **/
static GtkWidget *gsb_currency_config_create_list (void)
{
    GtkTreeSelection *selection;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkListStore *model;
    GtkWidget *tree_view;
    gint col_offset;

    /* Create tree store
       CURRENCY_FLAG_COLUMN,
       CURRENCY_HAS_FLAG,
       COUNTRY_NAME_COLUMN,
       CURRENCY_NAME_COLUMN,
       CURRENCY_CODE_ISO_COLUMN,
       CURRENCY_NICKNAME_COLUMN,
       CURRENCY_FLOATING_COLUMN,
       CURRENCY_NUMBER_COLUMN,
       CURRENCY_MAIN_CURRENCY_COLUMN,
	   CURRENCY_BACKGROUND_COLOR */

	model = gtk_list_store_new (NUM_CURRENCIES_COLUMNS,
								GDK_TYPE_PIXBUF,
								G_TYPE_BOOLEAN,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_INT,
								G_TYPE_INT,
								G_TYPE_INT,
								GDK_TYPE_RGBA);

    /* Create tree tree_view */
    tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
 	gtk_widget_set_name (tree_view, "colorized_tree_view");
    g_object_unref (G_OBJECT(model));

    /* connect the selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);

    /* Flag */
    cell = gtk_cell_renderer_pixbuf_new ();
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("Country name"),
															  cell,
															  "pixbuf", CURRENCY_FLAG_COLUMN,
															  "visible", CURRENCY_HAS_FLAG,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);

	/* Country name */
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (GTK_TREE_VIEW_COLUMN(column), cell, FALSE);
    gtk_tree_view_column_set_attributes (GTK_TREE_VIEW_COLUMN(column),
										 cell,
										 "text", COUNTRY_NAME_COLUMN,
										 "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
										 NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Currency name */
    cell = gtk_cell_renderer_text_new ();
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("Currency name"),
															  cell,
															  "text", CURRENCY_NAME_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

	/* ISO Code */
    cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("ISO Code"),
															  cell,
															  "text", CURRENCY_CODE_ISO_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, GSB_CENTER);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

	/* Sign */
    cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
															  -1,
															  _("Sign"),
															  cell,
															  "text", CURRENCY_NICKNAME_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree_view), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, GSB_CENTER);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model),
					  COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

    /* Set signal */
    g_signal_connect (G_OBJECT (selection),
                      "changed",
                      G_CALLBACK (gsb_currency_config_change_selection),
                      NULL);

    return tree_view;
}

/**
 * fill the currency list in the popup
 * with all the world currencies
 *
 * \param tree_view the tree tree_view to fill in
 * \param include_obsolete TRUE to fill with the obsolete curencies
 *
 * \return
 **/
static void gsb_currency_config_fill_popup_list (GtkTreeView *tree_view,
												 gboolean include_obsolete)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    Iso4217Currency *currency;

	currency = gsb_data_currency_get_tab_iso_4217_currencies ();
    model = gtk_tree_view_get_model (tree_view);

    while (currency->country_name)
    {
		if (include_obsolete || currency->active)
		{
			GdkPixbuf *pixbuf;
			gchar *string;

			string = g_build_filename (gsb_dirs_get_pixmaps_dir (), "flags", currency->flag_filename, NULL);
			pixbuf = gdk_pixbuf_new_from_file (string, NULL);
			g_free (string);

			string = g_strconcat (" ", _(currency->country_name), NULL);
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model),
								&iter,
								CURRENCY_FLAG_COLUMN, pixbuf,
								COUNTRY_NAME_COLUMN, string,
								CURRENCY_NAME_COLUMN, _(currency->currency_name),
								CURRENCY_CODE_ISO_COLUMN, _(currency->currency_code_iso),
								CURRENCY_NICKNAME_COLUMN, _(currency->currency_nickname),
								CURRENCY_FLOATING_COLUMN, currency->floating_point,
								CURRENCY_NUMBER_COLUMN, currency,
								CURRENCY_HAS_FLAG, TRUE,
								CURRENCY_MAIN_CURRENCY_COLUMN, currency->main_currency,
								-1);
			g_free (string);
		}
		currency++;
    }
}

/**
 * called when select a currency in the whole world list in the add popup
 * fill the corresponding entries to append it
 *
 * \param selection
 * \param model 		the tree_model
 *
 * \return
 **/
static gboolean gsb_currency_config_select_currency_popup (GtkTreeSelection *selection,
														   GtkTreeModel *model)
{
    GtkWidget *entry_code;
    GtkWidget *entry_floating_point;
    GtkWidget *entry_iso_code;
    GtkWidget *entry_name;
    GtkTreeIter iter;
    const gchar *currency_iso_code;
    const gchar *currency_name;
    const gchar *currency_nickname;
	gchar *tmp_str;
    gint currency_floating;

    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
		return(FALSE);

    entry_name = g_object_get_data (G_OBJECT(model), "entry_name");
    entry_iso_code = g_object_get_data (G_OBJECT(model), "entry_iso_code");
    entry_code = g_object_get_data (G_OBJECT(model), "entry_code");
    entry_floating_point = g_object_get_data (G_OBJECT(model), "entry_floating_point");


    gtk_tree_model_get (model,
						&iter,
						CURRENCY_NAME_COLUMN, &currency_name,
						CURRENCY_CODE_ISO_COLUMN, &currency_iso_code,
						CURRENCY_NICKNAME_COLUMN, &currency_nickname,
						CURRENCY_FLOATING_COLUMN, &currency_floating,
						-1);

    if (!currency_name)
		currency_name = "";
    if (!currency_nickname)
		currency_nickname = "";
    if (!currency_iso_code)
		currency_iso_code = "";

    gtk_entry_set_text (GTK_ENTRY (entry_name), currency_name);
    gtk_entry_set_text (GTK_ENTRY (entry_iso_code), currency_iso_code);
    gtk_entry_set_text (GTK_ENTRY (entry_code), currency_nickname);
    tmp_str = utils_str_itoa (currency_floating);
    gtk_entry_set_text (GTK_ENTRY (entry_floating_point), tmp_str);
    g_free (tmp_str);

	return (FALSE);
}

/**
 * Set an integer to the value of a menu.  Normally called via a GTK
 * "changed" signal handler.
 *
 * \param menu 		a pointer to a menu widget.
 * \param dummy 	unused
 *
 * \return
 **/
static gboolean gsb_currency_config_set_int_from_combobox (GtkWidget *combobox,
														   gint *dummy)
{
    gint *data;

    data = g_object_get_data (G_OBJECT(combobox), "pointer");

    if (data)
    {
		*data = gsb_currency_get_currency_from_combobox (combobox);
    }

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return (FALSE);
}

/**
 * called when toggle the button show/hide the obselete currencies
 * in the popup
 * fill the list according that button
 *
 * \param checkbox 		contains what the user want
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean gsb_currency_config_update_list (GtkWidget *checkbox,
												 GtkTreeView *tree_view)
{
    GtkTreeModel *model;

    model = gtk_tree_view_get_model (tree_view);
    gtk_list_store_clear (GTK_LIST_STORE (model));
    gsb_currency_config_fill_popup_list (tree_view,
										 gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)));

    /* re-select the default currency
     * TODO : should use the GtkTreeModelFilter to show or not the obsoletes currencies,
     * so the selection shouldn't be lost */
    gtk_tree_model_foreach (GTK_TREE_MODEL(model),
							(GtkTreeModelForeachFunc) gsb_currency_config_select_default,
							tree_view);

	utils_set_list_store_background_color (GTK_WIDGET (tree_view), CURRENCY_BACKGROUND_COLOR);

    return FALSE;
}

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/**
 * Create a new GtkComboBox with a pointer to an integer that will be
 * modified according to the entry's value.
 *
 * \param value		A pointer to a gint which contains the currency number
 * \param hook 		An optional function to execute as a handler
 *
 * \return A newly allocated option menu.
 */
GtkWidget *gsb_currency_config_new_combobox (gint *value,
											 GCallback hook)
{
    GtkWidget *combo_box;

    combo_box = gsb_currency_make_combobox (FALSE);

    if (value && *value)
		gsb_currency_set_combobox_history (combo_box, *value);

    g_signal_connect (G_OBJECT (combo_box),
					  "changed",
					  (GCallback) gsb_currency_config_set_int_from_combobox,
					  value);
    g_object_set_data (G_OBJECT (combo_box), "pointer", value);

    if (hook)
		g_object_set_data (G_OBJECT (combo_box),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(combo_box),
																	 "changed",
																	 G_CALLBACK (hook),
																	 value)));

    return combo_box;
}




/* ********************************************************************************************************************************** */
/* NEXT PART about the list to add a new currency */
/* ********************************************************************************************************************************** */
/**
 * Pop up a dialog to create a new currency, do some sanity checks and
 * call the gsb_data_currency_new_with_data() function to do the grunt work.
 *
 * \param button				GtkButton that triggered event.
 * \param currency_tree_model 	the treemodel to add the new currency (can be NULL)
 *
 * \return TRUE if currency has been created.
 **/
gboolean gsb_currency_config_add_currency (GtkWidget *button,
										   GtkWidget *tree_view)
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *list;
    GtkWidget *main_vbox;
    GtkWidget *paddingbox;
    GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *entry_code;
    GtkWidget *entry_floating_point;
    GtkWidget *entry_iso_code;
    GtkWidget *entry_name;
    GtkTreeModel *model;
	GtkTreeModel *currency_tree_model;
    const gchar *currency_code;
    const gchar *currency_iso_code;
    const gchar *currency_name;
    gint currency_number;
    gint floating_point;
    gint result;

	dialog = gtk_dialog_new_with_buttons (_("Add a currency"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL,
										  "gtk-close", 1,
										  NULL);

    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

    main_vbox = new_vbox_with_title_and_icon (_("Select base currency for your account"),
											  "gsb-currencies-32.png");
    gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), main_vbox, TRUE, TRUE, 0);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (main_vbox), vbox, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), BOX_BORDER_WIDTH);

    paddingbox = new_paddingbox_with_title (vbox, TRUE, _("World currencies"));

    /* Create list */
    list = gsb_currency_config_create_box_popup (G_CALLBACK (gsb_currency_config_select_currency_popup));
    model = g_object_get_data (G_OBJECT(list), "model");

    gtk_box_pack_start (GTK_BOX(paddingbox), list, TRUE, TRUE, 5);

    paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Currency details"));

    /* Create table */
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), 5);
    gtk_grid_set_row_spacing (GTK_GRID (table), 5);
    gtk_box_pack_start (GTK_BOX (paddingbox), table, TRUE, TRUE, 0);

    /* Currency name */
    label = gtk_label_new (_("Currency name: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    entry_name = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entry_name), TRUE);
    gtk_grid_attach (GTK_GRID (table), entry_name, 1, 0, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_name", entry_name);

    /* Currency ISO code */
    label = gtk_label_new (_("Currency international code: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    entry_iso_code = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), entry_iso_code, 1, 1, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_iso_code", entry_iso_code);

    /* Currency usual sign */
    label = gtk_label_new (_("Currency sign: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);
    entry_code = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), entry_code, 1, 2, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_code", entry_code);

    /* Create floating point entry */
    label = gtk_label_new (_("Floating point: "));
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
    gtk_grid_attach (GTK_GRID (table), label, 0, 3, 1, 1);
    entry_floating_point = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), entry_floating_point, 1, 3, 1, 1);
    g_object_set_data (G_OBJECT(model), "entry_floating_point", entry_floating_point);

    /* Select default currency. */
    gtk_tree_model_foreach (GTK_TREE_MODEL(model),
							(GtkTreeModelForeachFunc) gsb_currency_config_select_default,
							g_object_get_data (G_OBJECT(list), "tree_view"));

dialog_return:
    gtk_widget_show_all (GTK_WIDGET (dialog));
    result = gtk_dialog_run (GTK_DIALOG (dialog));

    if (result)
    {
		currency_name = gtk_entry_get_text (GTK_ENTRY (entry_name));
		currency_code = gtk_entry_get_text (GTK_ENTRY (entry_code));
		currency_iso_code = gtk_entry_get_text (GTK_ENTRY (entry_iso_code));
		floating_point = utils_str_atoi (gtk_entry_get_text (GTK_ENTRY (entry_floating_point)));

		if (strlen (currency_name)
			&& (strlen (currency_code) || strlen (currency_iso_code)))
		{
			/* check if the currency exists si la devise existe on ne fait rien */

			if (!gsb_data_currency_get_number_by_name (currency_name)
				&& !gsb_data_currency_get_number_by_code_iso4217 (currency_iso_code))
			{
				currency_number = gsb_data_currency_new_with_data (currency_name,
																	   currency_code,
																	   currency_iso_code,
																	   floating_point);

				/* update the currencies list in account properties */
				gsb_currency_update_combobox_currency_list ();

				currency_tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
				if (currency_tree_model && currency_number > 0)
				{
					gsb_currency_append_currency_to_list (GTK_LIST_STORE (currency_tree_model), currency_number);
					gtk_widget_destroy (GTK_WIDGET (dialog));
					gsb_file_set_modified (TRUE);
					utils_set_list_store_background_color (tree_view, CURRENCY_BACKGROUND_COLOR);

					return TRUE;
				}
			}
		}
		else
		{
			dialogue_warning_hint (_("Currency name and either international "
									 "currency code or currency nickname should be set."),
								   _("All fields are not filled in"));
			goto dialog_return;
		}
    }
    gtk_widget_destroy (GTK_WIDGET (dialog));

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_currency_config_add_currency_set_combobox (GtkWidget *button,
														GtkWidget *combobox)
{
    gsb_currency_config_add_currency (button, FALSE);
    gsb_currency_set_combobox_history (combobox, gsb_data_currency_max_number ());

    return FALSE;
}


/**
 * create the box which contains the world currencies list for the
 * add currency popup
 * the tree view and model are saved into the box with the keys :
 * 	"model" and "tree_view"
 *
 * \param select_callback callback to call when a line is selected
 * 		callback (GtkTreeSelection *, GtkTreeModel *)
 *
 * \return a vbox
 **/
GtkWidget *gsb_currency_config_create_box_popup (GCallback select_callback)
{
    GtkWidget *checkbox;
    GtkWidget *sw;
    GtkWidget *tree_view;
    GtkWidget *vbox;
    GtkTreeModel *model;

    sw = utils_prefs_scrolled_window_new (NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_SW, 200);

    tree_view = gsb_currency_config_create_list ();
    gtk_widget_set_size_request (tree_view, -1, 200);
    model = gtk_tree_view_get_model (GTK_TREE_VIEW(tree_view));
    if (select_callback)
		g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
						  "changed",
						  G_CALLBACK (select_callback),
						  model);

    gtk_container_add (GTK_CONTAINER (sw), tree_view);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX(vbox), sw, TRUE, TRUE, 0);

    checkbox = gtk_check_button_new_with_label (_("Include obsolete currencies"));
    gtk_box_pack_start (GTK_BOX(vbox), checkbox, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT(checkbox),
                      "toggled",
                      (GCallback) gsb_currency_config_update_list,
                      tree_view);

    gsb_currency_config_fill_popup_list (GTK_TREE_VIEW(tree_view), FALSE);

    g_object_set_data (G_OBJECT(vbox), "model", model);
    g_object_set_data (G_OBJECT(vbox), "tree_view", tree_view);

	utils_set_list_store_background_color (tree_view, CURRENCY_BACKGROUND_COLOR);
    gtk_widget_show_all (GTK_WIDGET (vbox));

    return vbox;
}

/**
 * foreach function on the tree_view to find the default currency
 * according to the current locale
 *
 * \param tree_model
 * \param path
 * \param iter
 * \param tree_view
 *
 * \return TRUE if found
 **/
gboolean gsb_currency_config_select_default (GtkTreeModel *tree_model,
											 GtkTreePath *path,
											 GtkTreeIter *iter,
											 GtkTreeView *tree_view)
{
    struct lconv *locale;
    gchar *code;
    gchar *country;
    gchar *symbol;
    gboolean good = FALSE;
    gboolean main_currency;

	locale = gsb_locale_get_locale ();
    gtk_tree_model_get (GTK_TREE_MODEL (tree_model),
						iter,
						CURRENCY_CODE_ISO_COLUMN, &code,
						COUNTRY_NAME_COLUMN, &country,
						CURRENCY_MAIN_CURRENCY_COLUMN, &main_currency,
						-1);
    if (locale && locale->int_curr_symbol && strlen (locale->int_curr_symbol))
    {
		symbol = g_strdup (locale->int_curr_symbol);
		g_strstrip (symbol);
		/* When matching, weed out currencies that are not "main"
		 * currency, that is for example, do not match USD on Panama
		 * or our US folks will be prompted to use USD as in Panama by
		 * default.  --benj */
		if (!strcmp (code, symbol) && main_currency)
		{
			good = TRUE;
		}
    }
    else
    {
		symbol = country;
		if (!strcmp (symbol, _("United States")))
		{
			symbol = g_strdup ("USD");
			good = TRUE;
		}
    }

    if (good)
    {
		gchar*tmp_str;

		tmp_str = g_strdup_printf ("found '%s'", symbol);
		devel_debug (tmp_str);
		g_free (tmp_str);
		g_free (symbol);
		gtk_tree_selection_select_path (gtk_tree_view_get_selection (tree_view), path);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, TRUE, GSB_CENTER, 0);

		return TRUE;
    }

    return FALSE;
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
