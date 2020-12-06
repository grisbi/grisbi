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

#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_currency_popup.h"
#include "gsb_currency_popup.h"
#include "gsb_data_currency.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetCurrencyPopupPrivate   WidgetCurrencyPopupPrivate;

struct _WidgetCurrencyPopupPrivate
{
	GtkWidget *			vbox_currency_popup;

    GtkWidget *			checkbutton_currency_popup;
	GtkWidget *			search_currency_popup;
	GtkWidget *			treeview_currency_popup;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetCurrencyPopup, widget_currency_popup, GTK_TYPE_BOX)

/*START_STATIC*/
gint radio_menu_index = 0;
/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_currency_popup_search_function  (GtkTreeModel *model,
														gint column,
														const gchar *key,
														GtkTreeIter *iter,
														gpointer search_data)
{
	gchar *entry;
	gchar *tmp_key;
	gchar *tmp_entry;
	gint result = TRUE;

	gtk_tree_model_get (GTK_TREE_MODEL (model), iter, column, &entry, -1);
	tmp_entry = g_strstrip (g_utf8_strup (entry, -1));
	tmp_key = g_strstrip (g_utf8_strup (key, -1));

	if (g_str_has_prefix (tmp_entry, tmp_key))
	{
		result = FALSE;
	}
	else
	{
		result = TRUE;
	}
	g_free (entry);
	g_free (tmp_key);
	g_free (tmp_entry);

	return result;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_currency_popup_set_search_menu_activate (GtkWidget *item,
															GtkEntry  *tree_view)
{
	gint value;

	devel_debug (NULL);
	value = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (item));
	if (value)
	{
		GtkWidget *entry;
		const gchar *menu_label;

		entry = g_object_get_data (G_OBJECT (item), "entry");

		menu_label = gtk_menu_item_get_label (GTK_MENU_ITEM (item));
		if (g_strcmp0 (menu_label, "Search by country") == 0)
		{
			radio_menu_index = 0;
			gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view), COUNTRY_NAME_COLUMN);
			gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Country"));
		}
		else if (g_strcmp0 (menu_label, "Search by currency name") == 0)
		{
			radio_menu_index = 1;
			gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view), CURRENCY_NAME_COLUMN);
			gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Currency"));
		}
		else if (g_strcmp0 (menu_label, "Search by code ISO") == 0)
		{
			radio_menu_index = 2;
			gtk_tree_view_set_search_column (GTK_TREE_VIEW (tree_view), CURRENCY_CODE_ISO_COLUMN);
			gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("ISO Code"));
		}
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *widget_currency_popup_create_search_menu (GtkWidget *entry,
															GtkWidget *tree_view)
{
	GtkWidget *menu;
	GtkWidget *item;

	menu = gtk_menu_new ();

	item = gtk_radio_menu_item_new_with_label (NULL, "Search by country");
	g_object_set_data (G_OBJECT (item), "entry", entry);
	if (radio_menu_index == 0)
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
	g_signal_connect (item,
					  "activate",
					  G_CALLBACK (widget_currency_popup_set_search_menu_activate),
					  tree_view);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_radio_menu_item_new_with_label_from_widget (GTK_RADIO_MENU_ITEM (item), "Search by currency name");
	g_object_set_data (G_OBJECT (item), "entry", entry);
	if (radio_menu_index == 1)
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
	g_signal_connect (item,
					  "activate",
					  G_CALLBACK (widget_currency_popup_set_search_menu_activate),
					   tree_view);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_radio_menu_item_new_with_label_from_widget (GTK_RADIO_MENU_ITEM (item), "Search by code ISO");
	g_object_set_data (G_OBJECT (item), "entry", entry);
	if (radio_menu_index == 2)
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
	g_signal_connect (item,
					  "activate",
					  G_CALLBACK (widget_currency_popup_set_search_menu_activate),
					  tree_view);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	gtk_widget_show_all (menu);

	return menu;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_currency_popup_entry_menu (GtkEntry *entry,
											  GtkMenu  *menu,
											  GtkWidget *tree_view)
{
  GtkWidget *item;
  GtkWidget *search_menu;

  item = gtk_separator_menu_item_new ();
  gtk_widget_show (item);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  search_menu = widget_currency_popup_create_search_menu (GTK_WIDGET (entry), tree_view);
  item = gtk_menu_item_new_with_label ("Search by");
  gtk_widget_show (item);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), search_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_currency_popup_checkbutton_toggled (GtkToggleButton *button,
													   GtkWidget *tree_view)
{
	gint value;

	value = gtk_toggle_button_get_active (button);
	prefs_page_currency_popup_fill_list (GTK_TREE_VIEW (tree_view), value);

    /* Select default currency. */
    gtk_tree_model_foreach (GTK_TREE_MODEL (gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view))),
										   (GtkTreeModelForeachFunc) gsb_popup_list_select_default,
										   tree_view);

}

/**
 * Création de la page de gestion des currency_popup
 *
 * \param page
 * \param model		tree model de l'appellant
 *
 * \return
 **/
static void widget_currency_popup_setup_page (WidgetCurrencyPopup *page,
											  GtkTreeModel *model)
{
	GtkWidget *menu;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    gint col_offset;
	WidgetCurrencyPopupPrivate *priv;

	devel_debug (NULL);

	priv = widget_currency_popup_get_instance_private (page);

	if (model)
		gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_currency_popup), GTK_TREE_MODEL (model));
 	gtk_widget_set_name (priv->treeview_currency_popup, "colorized_tree_view");
    g_object_unref (G_OBJECT(model));

    /* Flag */
    cell = gtk_cell_renderer_pixbuf_new ();
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (priv->treeview_currency_popup),
															  -1,
															  _("Country name"),
															  cell,
															  "pixbuf", CURRENCY_FLAG_COLUMN,
															  "visible", CURRENCY_HAS_FLAG,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);

	/* Country name */
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview_currency_popup), col_offset - 1);
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
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (priv->treeview_currency_popup),
															  -1,
															  _("Currency name"),
															  cell,
															  "text", CURRENCY_NAME_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview_currency_popup), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

	/* ISO Code */
    cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (priv->treeview_currency_popup),
															  -1,
															  _("ISO Code"),
															  cell,
															  "text", CURRENCY_CODE_ISO_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview_currency_popup), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, GSB_CENTER);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

	/* Sign */
    cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (cell), "xalign", GSB_CENTER, NULL);
    col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (priv->treeview_currency_popup),
															  -1,
															  _("Sign"),
															  cell,
															  "text", CURRENCY_NICKNAME_COLUMN,
															  "cell-background-rgba", CURRENCY_BACKGROUND_COLOR,
															  NULL);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (priv->treeview_currency_popup), col_offset - 1);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_alignment (column, GSB_CENTER);
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

	/* set search column by currency name by default */
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (priv->treeview_currency_popup), COUNTRY_NAME_COLUMN);
	gtk_tree_view_set_search_entry (GTK_TREE_VIEW (priv->treeview_currency_popup),
									GTK_ENTRY (priv->search_currency_popup));
	gtk_tree_view_set_search_equal_func (GTK_TREE_VIEW (priv->treeview_currency_popup),
										 (GtkTreeViewSearchEqualFunc) widget_currency_popup_search_function,
										 NULL,
										 NULL);

	/* Create the menu */
	menu = widget_currency_popup_create_search_menu (priv->search_currency_popup, priv->treeview_currency_popup);
	gtk_menu_attach_to_widget (GTK_MENU (menu), priv->search_currency_popup, NULL);

	/* add accessible alternatives for icon functionality */
	g_signal_connect (priv->search_currency_popup,
					  "populate-popup",
					  G_CALLBACK (widget_currency_popup_entry_menu),
					  priv->treeview_currency_popup);

	/* set background color */
	utils_set_list_store_background_color (GTK_WIDGET (priv->treeview_currency_popup), CURRENCY_BACKGROUND_COLOR);

	g_signal_connect (G_OBJECT(priv->checkbutton_currency_popup),
                      "toggled",
                      (GCallback) widget_currency_popup_checkbutton_toggled,
                      priv->treeview_currency_popup);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_currency_popup_init (WidgetCurrencyPopup *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void widget_currency_popup_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_currency_popup_parent_class)->dispose (object);
}

static void widget_currency_popup_class_init (WidgetCurrencyPopupClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_currency_popup_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_currency_popup.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyPopup, vbox_currency_popup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyPopup, checkbutton_currency_popup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyPopup, search_currency_popup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyPopup, treeview_currency_popup);
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
WidgetCurrencyPopup *widget_currency_popup_new (GtkTreeModel *model)
{
	WidgetCurrencyPopup *w_currency_popup;

	w_currency_popup = g_object_new (WIDGET_CURRENCY_POPUP_TYPE, NULL);
	widget_currency_popup_setup_page (w_currency_popup, model);

	return w_currency_popup;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *widget_currency_popup_get_tree_view (GtkWidget *w_currency_popup)
{
	WidgetCurrencyPopupPrivate *priv;

	priv = widget_currency_popup_get_instance_private (WIDGET_CURRENCY_POPUP (w_currency_popup));
	return priv->treeview_currency_popup;
}

void widget_currency_popup_display_old_currencies (GtkWidget *w_currency_popup,
												   gboolean display)
{
	WidgetCurrencyPopupPrivate *priv;

	priv = widget_currency_popup_get_instance_private (WIDGET_CURRENCY_POPUP (w_currency_popup));
	if (display)
		gtk_widget_show (priv->checkbutton_currency_popup);
	else
		gtk_widget_hide (priv->checkbutton_currency_popup);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void widget_currency_popup_show_hide_search_entry (GtkWidget *w_currency_popup,
												   gboolean show)
{
	WidgetCurrencyPopupPrivate *priv;

	priv = widget_currency_popup_get_instance_private (WIDGET_CURRENCY_POPUP (w_currency_popup));

	if (show)
		gtk_widget_show (priv->search_currency_popup);
	else
		gtk_widget_hide (priv->search_currency_popup);
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

