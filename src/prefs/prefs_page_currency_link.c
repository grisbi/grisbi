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
#include "prefs_page_currency_link.h"
#include "dialog.h"
#include "gsb_currency.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "widget_currency_link.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageCurrencyLinkPrivate   PrefsPageCurrencyLinkPrivate;

struct _PrefsPageCurrencyLinkPrivate
{
	GtkWidget *			vbox_currency_link;

    GtkWidget *			sw_currency_link;
	GtkWidget *			treeview_currency_link;
    GtkWidget *         button_currency_link_add;
    GtkWidget *         button_currency_link_remove;

	GtkWidget *			w_currency_link;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageCurrencyLink, prefs_page_currency_link, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * append a new line to the tree_view and fill it with the
 * link given in param
 *
 * \param model
 * \param link_number
 * \param iter a pointer to an iter to fill it with the position of the new link, or NULL
 *
 * \return
 **/
static void prefs_page_currency_link_append_line (GtkTreeModel *model,
										   		  gint link_number,
												  GtkTreeIter *iter_to_fill)
{
    const gchar *invalid;
    GtkTreeIter local_iter;
    GtkTreeIter *iter_ptr;
	gchar *tmp_str;
    gchar *str_date;

	if (iter_to_fill)
		iter_ptr = iter_to_fill;
    else
		iter_ptr = &local_iter;

    str_date = gsb_format_gdate (gsb_data_currency_link_get_modified_date (link_number));

    if (gsb_data_currency_link_get_invalid_link (link_number))
		invalid = "gtk-dialog-warning";
    else
		invalid = NULL;

    tmp_str = utils_real_get_string (gsb_data_currency_link_get_change_rate (link_number));
    gtk_list_store_append (GTK_LIST_STORE (model), iter_ptr);
    gtk_list_store_set (GTK_LIST_STORE (model),
					    iter_ptr,
					    LINK_1_COLUMN, "1",
					    LINK_CURRENCY1_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_first_currency(link_number)),
					    LINK_EQUAL_COLUMN, "=",
					    LINK_EXCHANGE_COLUMN, tmp_str,
					    LINK_CURRENCY2_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_second_currency(link_number)),
					    LINK_DATE_COLUMN, str_date,
					    LINK_INVALID_COLUMN, invalid,
					    LINK_NUMBER_COLUMN, link_number,
					    -1);
    g_free (tmp_str);
    g_free (str_date);
}

/**
 * called by a click on the "add link" in the configuration
 * add a new link and set the selection on it
 *
 * \param page
 *
 * \return FALSE
 **/
static void prefs_page_currency_link_add_link_clicked (PrefsPageCurrencyLink *page)
{
    gint link_number;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *tmp_list;
    gint nbre_devises;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_link_get_instance_private (page);

    tmp_list = gsb_data_currency_get_currency_list ();
    nbre_devises = g_slist_length (tmp_list);
    if (nbre_devises == 1)
    {
        dialogue_hint (_("You must create at least another currency to continue."),
                        _("The number of currencies is insufficient"));

        return;
    }

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_currency_link));

    link_number = gsb_data_currency_link_new (0);

    /* we are sure that at least there is a currency with the number 1,
     * so we set the 2 currencies on the number 1, even if it makes that link
     * invalid, but it's not a problem because i think the user will change
     * it quickly... */
    gsb_data_currency_link_set_first_currency (link_number, 1);
    gsb_data_currency_link_set_second_currency (link_number, 1);

    prefs_page_currency_link_append_line (model, link_number, &iter);

    gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency_link)), &iter);
	utils_set_list_store_background_color (GTK_WIDGET (priv->treeview_currency_link), LINK_BACKGROUND_COLOR);

	utils_prefs_gsb_file_set_modified ();
}

/**
 * called by a click on the "remove link" in the configuration
 * remove the link
 * there is no warning message, perhaps sould be better, but i don't
 * think re-creating the deleted link is too difficult...
 *
 * \param page
 *
 * \return
 **/
static void prefs_page_currency_link_remove_link_clicked (PrefsPageCurrencyLink *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_link_get_instance_private (page);

	if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency_link)),
										 &model,
										 &iter))
	{
		GtkWidget *label;
		gint link_number;

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, LINK_NUMBER_COLUMN, &link_number, -1);

		/* set here to sensitive the delete_link_button */
		gsb_data_currency_link_remove (link_number);
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		widget_currency_link_set_sensitive (priv->w_currency_link, FALSE);

		/* hide the warning label */
		label = widget_currency_link_get_widget (priv->w_currency_link, "label_warning");
		if (gtk_widget_get_visible (label))
			gtk_widget_hide (label);
	}

	utils_prefs_gsb_file_set_modified ();
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_currency_link_selection_changed (GtkTreeSelection *selection,
													    PrefsPageCurrencyLink *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_link_get_instance_private (page);
    if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_widget_set_sensitive (priv->button_currency_link_remove, FALSE);
	}
	else
	{
		GtkWidget *combobox_1;
		GtkWidget *combobox_2;
		GtkWidget *checkbutton_fixed;
		GtkWidget *entry_exchange;
		GSList *tmp_list;
		gint link_number;
		gint nbre_links;

		tmp_list = gsb_data_currency_link_get_currency_link_list ();
		nbre_links = g_slist_length (tmp_list);
		if (nbre_links)
			gtk_widget_set_sensitive (priv->button_currency_link_remove, TRUE);
		else
			gtk_widget_set_sensitive (priv->button_currency_link_remove, FALSE);

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, LINK_NUMBER_COLUMN, &link_number, -1);

		combobox_1 = widget_currency_link_get_widget (priv->w_currency_link, "combo_1");
		combobox_2 = widget_currency_link_get_widget (priv->w_currency_link, "combo_2");
		checkbutton_fixed = widget_currency_link_get_widget (priv->w_currency_link, "checkbutton_fixed");
		entry_exchange = widget_currency_link_get_widget (priv->w_currency_link, "entry_exchange");

		/* update widget_link */
		g_signal_handlers_block_by_func (G_OBJECT (combobox_1),
										 G_CALLBACK (prefs_page_currency_link_widget_link_changed),
										 page);
		g_signal_handlers_block_by_func (G_OBJECT (combobox_2),
										 G_CALLBACK (prefs_page_currency_link_widget_link_changed),
										 page);
		g_signal_handlers_block_by_func (G_OBJECT (entry_exchange),
										 G_CALLBACK (prefs_page_currency_link_widget_link_changed),
										 page);
		g_signal_handlers_block_by_func (G_OBJECT (checkbutton_fixed),
										 G_CALLBACK (prefs_page_currency_link_checkbutton_fixed_changed),
										 priv->treeview_currency_link);

		widget_currency_details_update_link (link_number, priv->w_currency_link);

		g_signal_handlers_unblock_by_func (G_OBJECT (combobox_1),
										   G_CALLBACK (prefs_page_currency_link_widget_link_changed),
										   page);
		g_signal_handlers_unblock_by_func (G_OBJECT (combobox_2),
										   G_CALLBACK (prefs_page_currency_link_widget_link_changed),
										   page);
		g_signal_handlers_unblock_by_func (G_OBJECT (entry_exchange),
										   G_CALLBACK (prefs_page_currency_link_widget_link_changed),
										   page);
		g_signal_handlers_unblock_by_func (G_OBJECT (checkbutton_fixed),
										   G_CALLBACK (prefs_page_currency_link_checkbutton_fixed_changed),
										   priv->treeview_currency_link);
	}
}

/**
 * fill the list of links
 *
 * \param model the tree_view to fill
 *
 * \return
 **/
static void prefs_page_currency_link_fill_model (GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GSList *tmp_list;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    tmp_list = gsb_data_currency_link_get_currency_link_list ();

    while (tmp_list)
    {
		gint link_number;

		link_number = gsb_data_currency_link_get_no_currency_link (tmp_list->data);
		prefs_page_currency_link_append_line (model, link_number, NULL);
		tmp_list = tmp_list->next;
    }
}

/**
 * create the tree which contains the currency link list
 *
 * \param
 *
 * \return
 **/
static void prefs_page_currency_link_setup_treeview (PrefsPageCurrencyLink *page)
{
    GtkTreeSelection *selection;
    GtkListStore *model;
    GtkCellRenderer *cell_renderer;
    const gchar *title[] = {"", N_("First currency"), "", N_("Exchange"),
							N_("Second currency"), N_("Modified date"), N_("Invalid")};
	gint i;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_link_get_instance_private (page);

	/* Create list store */
    model = gtk_list_store_new (NUM_LINKS_COLUMNS,
								G_TYPE_STRING,		/* LINK_1_COLUMN */
								G_TYPE_STRING,		/* LINK_CURRENCY1_COLUMN */
								G_TYPE_STRING,		/* LINK_EQUAL_COLUMN */
								G_TYPE_STRING,		/* LINK_EXCHANGE_COLUMN */
								G_TYPE_STRING,		/* LINK_CURRENCY2_COLUMN */
								G_TYPE_STRING,		/* LINK_DATE_COLUMN */
								G_TYPE_STRING,		/* LINK_INVALID_COLUMN */
								G_TYPE_INT,			/* LINK_NUMBER_COLUMN */
								GDK_TYPE_RGBA);		/* LINK_BACKGROUND_COLOR */

	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_currency_link), GTK_TREE_MODEL (model));
    g_object_unref (G_OBJECT (model));

    /* connect the selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency_link));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);

    /* for all the columns it's a text */
    cell_renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell_renderer), "xalign", COLUMN_CENTER, NULL);

	/* set columns */
    for (i=0 ; i< LINK_NUMBER_COLUMN; i++)
    {
		GtkTreeViewColumn *column = NULL;

		if (i == LINK_INVALID_COLUMN)
		{
			column = gtk_tree_view_column_new_with_attributes (gettext (title[i]),
															   gtk_cell_renderer_pixbuf_new (),
															   "stock-id", i,
															   "cell-background-rgba", LINK_BACKGROUND_COLOR,
															   NULL);
		}
		else if (i == LINK_1_COLUMN || i == LINK_EQUAL_COLUMN)
		{
			column = gtk_tree_view_column_new_with_attributes (title[i],
															   cell_renderer,
															   "text", i,
															   "cell-background-rgba", LINK_BACKGROUND_COLOR,
															   NULL);
			gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		}
		else
		{
			column = gtk_tree_view_column_new_with_attributes (gettext (title[i]),
															   cell_renderer,
															   "text", i,
															   "cell-background-rgba", LINK_BACKGROUND_COLOR,
															   NULL);
			gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		}
		gtk_tree_view_column_set_expand (column, TRUE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(priv->treeview_currency_link), column);
    }

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), LINK_CURRENCY1_COLUMN, GTK_SORT_ASCENDING);

	/* fill the model */
	prefs_page_currency_link_fill_model (priv->treeview_currency_link);

	/* set colors */
	utils_set_list_store_background_color (priv->treeview_currency_link, LINK_BACKGROUND_COLOR);
}

/**
 * Création de la page de gestion des currency_link
 *
 * \param page
 *
 * \return
 **/
static void prefs_page_currency_link_setup_page (PrefsPageCurrencyLink *page)
{
    GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkWidget *head_page;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_link_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Links between currencies"), "gsb-currencies-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_currency_link), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_currency_link), head_page, 0);

	/* set currency_link_list */
	prefs_page_currency_link_setup_treeview (page);

	/* set currency combo exchange */
	priv->w_currency_link = GTK_WIDGET (widget_currency_link_new (GTK_WIDGET (page)));
	gtk_box_pack_start (GTK_BOX (priv->vbox_currency_link), priv->w_currency_link, FALSE, FALSE, 0);

	/* set selection signal to update first currency */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency_link));
    g_signal_connect (G_OBJECT (selection),
                      "changed",
                      G_CALLBACK (prefs_page_currency_link_selection_changed),
                      page);

	/* select the first item */
	path = gtk_tree_path_new_from_indices (0, -1);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	/* set signals */
    g_signal_connect_swapped (G_OBJECT (priv->button_currency_link_add),
							  "clicked",
							  G_CALLBACK (prefs_page_currency_link_add_link_clicked),
							  page);
    g_signal_connect_swapped (G_OBJECT (priv->button_currency_link_remove),
							  "clicked",
							  G_CALLBACK (prefs_page_currency_link_remove_link_clicked),
							  page);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_currency_link_init (PrefsPageCurrencyLink *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_currency_link_setup_page (page);
}

static void prefs_page_currency_link_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_currency_link_parent_class)->dispose (object);
}

static void prefs_page_currency_link_class_init (PrefsPageCurrencyLinkClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_currency_link_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_currency_link.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrencyLink, vbox_currency_link);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrencyLink, button_currency_link_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrencyLink, button_currency_link_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrencyLink, sw_currency_link);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrencyLink, treeview_currency_link);
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
PrefsPageCurrencyLink *prefs_page_currency_link_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_CURRENCY_LINK_TYPE, NULL);
}

/**
 * met à jour le lien à chaque changement du check_button
 *
 *
 * \param
 *
 * \return
 **/
void  prefs_page_currency_link_checkbutton_fixed_changed (GtkWidget *checkbutton,
														  PrefsPageCurrencyLink *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint link_number;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_link_get_instance_private (page);

	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency_link)),
										   &model,
										   &iter))
		return;

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, LINK_NUMBER_COLUMN, &link_number, -1);

    gsb_data_currency_link_set_fixed_link (link_number,
										   gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton)));
}

/**
 * called when something change for a link
 *
 * \param tree_view the tree_view
 *
 * \return FALSE
 **/
void prefs_page_currency_link_widget_link_changed (GtkWidget *widget,
												   PrefsPageCurrencyLink *page)
{
    GtkWidget *entry_exchange;
    GtkWidget *label_warning;
    GtkTreeModel *model;
    GtkTreeIter iter;
	GtkTreeSelection *selection;
	GDate *tmp_date;
	const gchar *widget_name = NULL;
	const gchar *devise_name = NULL;
    gchar *invalid = NULL;
	gchar *markup = NULL;
	gchar* tmp_str;
    gchar *str_date;
    GsbReal number;
	gint currency_number_1 = 0;
	gint currency_number_2 = 0;
    gint link_number;
	gint new_currency_number;
	GrisbiWinRun *w_run;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_currency_link_get_instance_private (page);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency_link));
    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, LINK_NUMBER_COLUMN, &link_number, -1);

    /* normally should not happen */
	if (!link_number)
		return;

	/* On reinitialise les combobox si ajout ou suppression d'une devise */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (w_run->block_update_links == TRUE)
	{
		gtk_tree_selection_unselect_all (selection);
		prefs_page_currency_link_selection_changed (selection, page);

		return;
	}


	widget_name = gtk_widget_get_name (widget);
	if (g_strcmp0 (widget_name, "combobox_devise_1") == 0)
	{
		new_currency_number = gsb_currency_get_currency_from_combobox (widget);
		currency_number_1 = gsb_data_currency_link_get_first_currency (link_number);
		if (new_currency_number == currency_number_1)
			goto test;
		devise_name = gsb_data_currency_get_name  (currency_number_1);
    	gsb_data_currency_link_set_first_currency (link_number, new_currency_number);
	}
	else if (g_strcmp0 (widget_name, "combobox_devise_2") == 0)
	{
		new_currency_number = gsb_currency_get_currency_from_combobox (widget);
		currency_number_2 = gsb_data_currency_link_get_second_currency (link_number);
		if (new_currency_number == currency_number_2)
			goto test;
		devise_name = gsb_data_currency_get_name  (currency_number_2);
    	gsb_data_currency_link_set_second_currency (link_number, new_currency_number);
	}
	else
	{
		const gchar *entry_str;

		entry_exchange = widget;
		entry_str = gtk_entry_get_text (GTK_ENTRY (entry_exchange));
		number = utils_real_get_from_string (entry_str);
		if (number.exponent > 8)
			gtk_entry_set_max_length (GTK_ENTRY (entry_exchange), (gint) strlen (entry_str - 1));

    	gsb_data_currency_link_set_change_rate (link_number, number);
	}


	tmp_date = gdate_today ();
    gsb_data_currency_link_set_modified_date (link_number, tmp_date);

    str_date = gsb_format_gdate (tmp_date);

    if (gsb_data_currency_link_get_invalid_link (link_number))
		invalid = g_strdup ("gtk-dialog-warning");
    else
		invalid = NULL;

    tmp_str = utils_real_get_string (gsb_data_currency_link_get_change_rate (link_number));
    gtk_list_store_set (GTK_LIST_STORE (model),
						&iter,
						LINK_CURRENCY1_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_first_currency
																		   (link_number)),
						LINK_EXCHANGE_COLUMN, tmp_str,
						LINK_CURRENCY2_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_second_currency
																		   (link_number)),
						LINK_DATE_COLUMN, str_date,
						LINK_INVALID_COLUMN, invalid,
						-1);
    g_free (tmp_str);
    g_free (str_date);

test:
    /* set or hide the warning label */
    label_warning = widget_currency_link_get_widget(priv->w_currency_link, "label_warning");

    if (invalid)
    {
		markup = g_strdup (gsb_data_currency_link_get_invalid_message (link_number));
		gtk_label_set_markup (GTK_LABEL (label_warning), markup);
		gtk_widget_show (label_warning);
		g_free (markup);
    }
    else if (devise_name)
	{
		markup = g_strdup_printf (_("Warning : The currency '%s' may be used for transactions.\n"
									"The next time you use it, you will need to enter a new exchange rate."),
								  devise_name);
		gtk_label_set_markup (GTK_LABEL (label_warning), markup);
		gtk_widget_show (label_warning);
		g_free (markup);
	}
	else
		gtk_widget_hide (label_warning);

	if (invalid)
		g_free (invalid);

    utils_prefs_gsb_file_set_modified ();
    gsb_gui_navigation_update_home_page ();
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint prefs_page_currency_link_get_selected_link_number (PrefsPageCurrencyLink *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
	GtkTreeSelection *selection;
    gint link_number = 0;
	PrefsPageCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_link_get_instance_private (page);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency_link));
    if (gtk_tree_selection_get_selected (selection, &model, &iter))
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, LINK_NUMBER_COLUMN, &link_number, -1);

	return link_number;
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

