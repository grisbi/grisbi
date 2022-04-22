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

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_currency.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_currency.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_dirs.h"
#include "gsb_locale.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "widget_currency_details.h"
#include "widget_currency_popup.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageCurrencyPrivate   PrefsPageCurrencyPrivate;

struct _PrefsPageCurrencyPrivate
{
	GtkWidget *			vbox_currency;

	GtkWidget *			button_currency_add;
    GtkWidget *         button_currency_remove;
	GtkWidget *			entry_currency_floating_point;
	GtkWidget *			entry_currency_iso_code;
	GtkWidget *			entry_currency_name;
	GtkWidget *			entry_currency_nickname;
	GtkWidget *			grid_currency;
    GtkWidget *			treeview_currency;
	GtkWidget *			w_currency_details;
	GtkWidget *			w_currency_popup;

	/* Add dialog */
	GtkWidget *			dialog_currency_add;
	GtkWidget *			box_dialog_titre;
	GtkWidget *			box_for_popup;
	GtkWidget *			treeview_popup;
	GtkWidget *			w_popup_details;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageCurrency, prefs_page_currency, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
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
static gboolean prefs_page_currency_select_default (GtkTreeModel *tree_model,
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

		g_free (code);
		g_free (country);

		return TRUE;
    }

    return FALSE;
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
static void prefs_page_currency_popup_selection_changed (GtkTreeSelection *selection,
														 PrefsPageCurrency *page)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
    const gchar *currency_iso_code;
    const gchar *currency_name;
    const gchar *currency_nickname;
    gint currency_floating;
	PrefsPageCurrencyPrivate *priv;

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;

	priv = prefs_page_currency_get_instance_private (page);

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

	/* update details */
	widget_currency_details_update_from_iso_4217 (priv->w_popup_details,
												   currency_name,
												   currency_iso_code,
												   currency_nickname,
												   currency_floating);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_currency_popup_init_dialog (PrefsPageCurrency *page,
												   GrisbiPrefs *win)
{
	GtkWidget *head_titre;
    GtkWidget *w_currency_popup;
    GtkListStore *model;
	GtkTreePath *path;
    GtkTreeSelection *selection;
	PrefsPageCurrencyPrivate *priv;

	priv = prefs_page_currency_get_instance_private (page);

	gtk_window_set_destroy_with_parent (GTK_WINDOW (priv->dialog_currency_add), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (priv->dialog_currency_add), GTK_WINDOW (win));
	gtk_window_set_modal (GTK_WINDOW (priv->dialog_currency_add), TRUE);
    gtk_window_set_resizable (GTK_WINDOW (priv->dialog_currency_add), TRUE);

	head_titre = utils_prefs_head_page_new_with_title_and_icon (_("Select base currency for your account"),
																"gsb-currencies-32.png");
    gtk_box_pack_start (GTK_BOX (priv->box_dialog_titre), head_titre, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->box_dialog_titre), head_titre, 0);
	gtk_widget_show (head_titre);

	/* set model */
	model = gtk_list_store_new (NUM_CURRENCIES_COLUMNS,
								GDK_TYPE_PIXBUF,			/* CURRENCY_FLAG_COLUMN */
								G_TYPE_BOOLEAN,				/* CURRENCY_HAS_FLAG */
								G_TYPE_STRING,				/* COUNTRY_NAME_COLUMN */
								G_TYPE_STRING,				/* CURRENCY_NAME_COLUMN */
								G_TYPE_STRING,				/* CURRENCY_CODE_ISO_COLUMN */
								G_TYPE_STRING,				/* CURRENCY_NICKNAME_COLUMN */
								G_TYPE_INT,					/* CURRENCY_FLOATING_COLUMN */
								G_TYPE_INT,					/* CURRENCY_NUMBER_COLUMN */
								G_TYPE_INT,					/* CURRENCY_MAIN_CURRENCY_COLUMN */
								GDK_TYPE_RGBA);				/* CURRENCY_BACKGROUND_COLOR */
	/* set popup */
	w_currency_popup = GTK_WIDGET (widget_currency_popup_new (GTK_TREE_MODEL (model)));
	gtk_box_pack_start (GTK_BOX (priv->box_for_popup), w_currency_popup, TRUE, TRUE, 0);

	/* fill model */
	priv->treeview_popup = widget_currency_popup_get_tree_view (w_currency_popup);
	prefs_page_currency_popup_fill_list (GTK_TREE_VIEW (priv->treeview_popup), FALSE);

	/* set currency details */
	priv->w_popup_details = GTK_WIDGET (widget_currency_details_new (GTK_WIDGET (page), TRUE));
	gtk_box_pack_start (GTK_BOX (priv->box_dialog_titre), priv->w_popup_details, FALSE, FALSE, 0);

	/* set editable to FALSE */
	widget_currency_details_set_entry_editable (priv->w_popup_details, FALSE);

	/* colorise le tree_view */
	utils_set_list_store_background_color (GTK_WIDGET (priv->treeview_popup), CURRENCY_BACKGROUND_COLOR);

	/* set selection signal to update first currency */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_popup));
	g_signal_connect (G_OBJECT (selection),
					  "changed",
					  G_CALLBACK (prefs_page_currency_popup_selection_changed),
					  page);

	/* select the first item */
	path = gtk_tree_path_new_from_indices (0, -1);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	/* hide search entry */
	widget_currency_popup_show_hide_search_entry (priv->w_currency_popup, FALSE);
}

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
static void prefs_page_currency_append_currency_to_model (GtkListStore *model,
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

static void prefs_page_currency_button_add_clicked (GtkWidget *button,
													PrefsPageCurrency *page)
{
	GtkTreeModel *currency_tree_model;
	GtkTreeModel *popup_model;
    gint currency_number;
	gint result;
	PrefsPageCurrencyPrivate *priv;

	priv = prefs_page_currency_get_instance_private (page);

	popup_model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_popup));

	/* Select default currency. */
    gtk_tree_model_foreach (GTK_TREE_MODEL(popup_model),
							(GtkTreeModelForeachFunc) prefs_page_currency_select_default,
							priv->treeview_popup);

	/* show search entry */
	widget_currency_popup_show_hide_search_entry (priv->w_currency_popup, TRUE);

	gtk_widget_show_all (priv->dialog_currency_add);

	result = gtk_dialog_run (GTK_DIALOG (priv->dialog_currency_add));
	if (result == GTK_RESPONSE_OK)
	{
		GtkWidget *entry_popup_floating_point;
		GtkWidget *entry_popup_iso_code;
		GtkWidget *entry_popup_name;
		GtkWidget *entry_popup_nickname;
		const gchar *currency_nickname;
		const gchar *currency_iso_code;
		const gchar *currency_name;
    	gint floating_point;

		entry_popup_name = widget_currency_details_get_entry (priv->w_popup_details, "entry_name");
		entry_popup_iso_code = widget_currency_details_get_entry (priv->w_popup_details, "entry_iso_code");
		entry_popup_nickname = widget_currency_details_get_entry (priv->w_popup_details, "entry_nickname");
		entry_popup_floating_point = widget_currency_details_get_entry (priv->w_popup_details, "entry_floating_point");

		currency_name = gtk_entry_get_text (GTK_ENTRY (entry_popup_name));
		currency_nickname = gtk_entry_get_text (GTK_ENTRY (entry_popup_nickname));
		currency_iso_code = gtk_entry_get_text (GTK_ENTRY (entry_popup_iso_code));
		floating_point = utils_str_atoi (gtk_entry_get_text (GTK_ENTRY (entry_popup_floating_point)));

		if (strlen (currency_name)
			&& (strlen (currency_nickname) || strlen (currency_iso_code)))
		{
			/* check if the currency exists si la devise existe on ne fait rien */
			if (!gsb_data_currency_get_number_by_name (currency_name)
				&& !gsb_data_currency_get_number_by_code_iso4217 (currency_iso_code))
			{
				GrisbiWinRun *w_run;

				w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
				w_run->block_update_links = TRUE;

				currency_number = gsb_data_currency_new_with_data (currency_name,
																   currency_nickname,
																   currency_iso_code,
																   floating_point);

				/* update the currencies list in account properties */
				gsb_currency_update_combobox_currency_list ();

				currency_tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_currency));
				if (currency_tree_model && currency_number > 0)
				{
					prefs_page_currency_append_currency_to_model (GTK_LIST_STORE (currency_tree_model), currency_number);
					utils_prefs_gsb_file_set_modified ();
				}
				w_run->block_update_links = FALSE;
			}
		}
	}

	widget_currency_popup_show_hide_search_entry (priv->w_currency_popup, FALSE);
	gtk_widget_hide (GTK_WIDGET (priv->dialog_currency_add));
}

/**
 * Obtain selected currency from currency tree.
 *
 * \param tree_view		GtkTreeView to remove currency from.
 *
 * \return				the number of the selected currency.
 **/
static gint prefs_page_currency_get_selected (GtkTreeView *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *tree_model;
    gint currency_number;

	selection = gtk_tree_view_get_selection (tree_view);
    if (!selection || !gtk_tree_selection_get_selected (selection, &tree_model, &iter))
		return(FALSE);

    gtk_tree_model_get (tree_model,
						&iter,
						CURRENCY_NUMBER_COLUMN, &currency_number,
						-1);

	return currency_number;
}

/**
 * Remove selected currency from tree.  In fact, this is a generic
 * function that could be used for any purpose (and could be then
 * renamed).
 *
 * \param tree_view		GtkTreeView to remove selected entry from.
 *
 * \return
 **/
static void prefs_page_currency_remove_selected_from_view (GtkTreeView *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *tree_model;

	selection = gtk_tree_view_get_selection (tree_view);
    if (!selection || !gtk_tree_selection_get_selected (selection, &tree_model, &iter))
		return;

    gtk_list_store_remove (GTK_LIST_STORE (tree_model), &iter);
}

/**
 * Remove selected currency from currency list.  First, be sure there
 * is no use for it.
 *
 * \param button		Widget that triggered event.
 * \param tree_view		GtkTreeView that contains selected currency.
 *
 * \return
 **/
static void prefs_page_currency_button_remove_clicked (GtkWidget *button,
													   PrefsPageCurrency *page)
{
    GSList *list_tmp;
    gint currency_number;
	gchar *tmp_str1 = NULL;
	gchar *tmp_str2 = NULL;
	gboolean trouve = FALSE;
	GrisbiWinRun *w_run;
	PrefsPageCurrencyPrivate *priv;

	priv = prefs_page_currency_get_instance_private (page);
    currency_number = prefs_page_currency_get_selected (GTK_TREE_VIEW (priv->treeview_currency));
    if (!currency_number)
		return;

    /* we look for that currency in all the transactions,
     * if we find it, we cannot delete it */
    list_tmp = gsb_data_transaction_get_complete_transactions_list ();
    while (list_tmp)
    {
		gint transaction_number;

		transaction_number = gsb_data_transaction_get_transaction_number (list_tmp->data);
		if (gsb_data_transaction_get_currency_number (transaction_number) == currency_number)
		{
			g_free (tmp_str1);
			tmp_str1 = g_strdup_printf (_("Currency '%s' is used in current "
										  "file.  Grisbi can't delete it."),
										gsb_data_currency_get_name (currency_number));
			g_free (tmp_str2);
			tmp_str2 = g_strdup_printf (_("Impossible to remove currency '%s'"),
										gsb_data_currency_get_name (currency_number));
			trouve = TRUE;
			break;
		}
		else
			list_tmp = list_tmp->next;
    }

    /* check the currency in the scheduled transactions */
    list_tmp = gsb_data_scheduled_get_scheduled_list ();
    while (list_tmp)
    {
		gint scheduled_number;

		scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp->data);
		if (gsb_data_scheduled_get_currency_number (scheduled_number) == currency_number)
		{
			g_free (tmp_str1);
			tmp_str1 = g_strdup_printf (_("Currency '%s' is used in current "
										  "file. Grisbi can't delete it."),
										gsb_data_currency_get_name (currency_number));
			g_free (tmp_str2);
			tmp_str2 = g_strdup_printf (_("Impossible to remove currency '%s'"),
										gsb_data_currency_get_name (currency_number));
			trouve = TRUE;
			break;
		}
		else
			list_tmp = list_tmp->next;
    }

	/* check if a currency link exists */
	list_tmp = gsb_data_currency_link_get_currency_link_list ();
    while (list_tmp)
    {
		gint currency_link_number;
		gint currency_number_1;
		gint currency_number_2;

		currency_link_number = gsb_data_currency_get_no_currency (list_tmp -> data);

		currency_number_1 = gsb_data_currency_link_get_first_currency (currency_link_number);
		if (currency_number_1 == currency_number)
		{
			trouve = TRUE;
			break;
		}
		else
			currency_number_2 = gsb_data_currency_link_get_second_currency (currency_link_number);
		if (currency_number_2 == currency_number)
		{
			trouve = TRUE;
			break;
		}
		else
			list_tmp = list_tmp->next;
	}

	/* trouve = TRUE */
	if (trouve)
	{
		if (tmp_str1 == NULL)
		{
			tmp_str1 = g_strdup (_("There is a link between this currency and another currency.\n"
										  "Please delete this link first."));
			tmp_str2 = g_strdup_printf (_("Impossible to remove currency '%s'"),
										gsb_data_currency_get_name (currency_number));
		}
		dialogue_error_hint (tmp_str1, tmp_str2);
		g_free (tmp_str1);
		g_free (tmp_str2);

		return;
	}
	/* bloque la MAJ des liens */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->block_update_links = TRUE;

	widget_currency_details_update_currency (0, priv->w_currency_details);
    gsb_data_currency_remove (currency_number);
    prefs_page_currency_remove_selected_from_view (GTK_TREE_VIEW(priv->treeview_currency));
    gsb_currency_update_combobox_currency_list ();
	w_run->block_update_links = FALSE;
}

/**
 * fill the currencies list with all the known currencies
 *
 * \param tree_view
 *
 * \return FALSE
 **/
static void prefs_page_currency_fill_model (GtkWidget *tree_view)
{
	GtkTreeModel *model;
    GSList *tmp_list;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    if (!model)
		return;

    tmp_list = gsb_data_currency_get_currency_list ();
    while (tmp_list)
    {
		gint currency_number;

		currency_number = gsb_data_currency_get_no_currency (tmp_list->data);

		prefs_page_currency_append_currency_to_model (GTK_LIST_STORE (model), currency_number);
		tmp_list = tmp_list->next;
    }
}

/**
 * called when select a currency in the known list of currencies
 * fill the corresponding entries to append it
 *
 * \param selection
 * \param model the tree_model
 *
 * \return
 **/
static void prefs_page_currency_selection_changed (GtkTreeSelection *selection,
												   PrefsPageCurrency *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint currency_number;
	PrefsPageCurrencyPrivate *priv;

	priv = prefs_page_currency_get_instance_private (page);
    if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_widget_set_sensitive (priv->button_currency_remove, FALSE);

		return;
	}
	else
	{
	    GSList *tmp_list;
		gint nbre_devises;

		tmp_list = gsb_data_currency_get_currency_list ();
		nbre_devises = g_slist_length (tmp_list);
		if (priv->button_currency_remove)
		{
			if (nbre_devises > 1)
				gtk_widget_set_sensitive (priv->button_currency_remove, TRUE);
			else
				gtk_widget_set_sensitive (priv->button_currency_remove, FALSE);
		}
	}

	/* get the contents of the selected line */
    gtk_tree_model_get (model, &iter, CURRENCY_NUMBER_COLUMN, &currency_number, -1);

	widget_currency_details_update_currency (currency_number, priv->w_currency_details);
}

/**
 * create the tree which contains the currency list
 * used both for the currency known list, and the currency to add list
 *
 * \param
 *
 * \return a GtkTreeView
 **/
static void prefs_page_currency_setup_treeview (PrefsPageCurrency *page)
{
    GtkListStore *model;
    GtkTreeSelection *selection;
	PrefsPageCurrencyPrivate *priv;

	priv = prefs_page_currency_get_instance_private (page);
	model = gtk_list_store_new (NUM_CURRENCIES_COLUMNS,
								GDK_TYPE_PIXBUF,			/* CURRENCY_FLAG_COLUMN */
								G_TYPE_BOOLEAN,				/* CURRENCY_HAS_FLAG */
								G_TYPE_STRING,				/* COUNTRY_NAME_COLUMN */
								G_TYPE_STRING,				/* CURRENCY_NAME_COLUMN */
								G_TYPE_STRING,				/* CURRENCY_CODE_ISO_COLUMN */
								G_TYPE_STRING,				/* CURRENCY_NICKNAME_COLUMN */
								G_TYPE_INT,					/* CURRENCY_FLOATING_COLUMN */
								G_TYPE_INT,					/* CURRENCY_NUMBER_COLUMN */
								G_TYPE_INT,					/* CURRENCY_MAIN_CURRENCY_COLUMN */
								GDK_TYPE_RGBA);				/* CURRENCY_BACKGROUND_COLOR */

    /* get tree priv->treeview_currency */
	priv->w_currency_popup = GTK_WIDGET (widget_currency_popup_new (GTK_TREE_MODEL (model)));
	gtk_grid_attach (GTK_GRID (priv->grid_currency), priv->w_currency_popup, 0, 0, 2, 1);
	priv->treeview_currency = widget_currency_popup_get_tree_view (priv->w_currency_popup);

    /* connect the selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), COUNTRY_NAME_COLUMN, GTK_SORT_ASCENDING);

	/* fill the model */
	prefs_page_currency_fill_model (priv->treeview_currency);

	/* set colors */
	utils_set_list_store_background_color (priv->treeview_currency, CURRENCY_BACKGROUND_COLOR);
}

/**
 * Création de la page de gestion des currency
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_currency_setup_page (PrefsPageCurrency *page,
											GrisbiPrefs *win)
{
	GtkWidget *head_page;
    GtkTreeSelection *selection;
	GtkTreePath *path;
	PrefsPageCurrencyPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_currency_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Currencies"), "gsb-currencies-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_currency), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_currency), head_page, 0);

	/* set currency_list */
	prefs_page_currency_setup_treeview (page);

	/* hide checkbutton_currency_popup */
	widget_currency_popup_display_old_currencies (priv->w_currency_popup, FALSE);

	/* set currency details */
	priv->w_currency_details = GTK_WIDGET (widget_currency_details_new (GTK_WIDGET (page), FALSE));
	gtk_box_pack_start (GTK_BOX (priv->vbox_currency), priv->w_currency_details, FALSE, FALSE, 0);

	/* init add dialog */
	prefs_page_currency_popup_init_dialog (page, win);

	/* set selection signal to update first currency */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency));
	g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency)),
					  "changed",
					  G_CALLBACK (prefs_page_currency_selection_changed),
					  page);

	/* select the first item */
	path = gtk_tree_path_new_from_indices (0, -1);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	/* Connect signal */
	g_signal_connect (G_OBJECT (priv->button_currency_add),
					  "clicked",
					  G_CALLBACK  (prefs_page_currency_button_add_clicked),
					  page);

    g_signal_connect (G_OBJECT (priv->button_currency_remove),
					  "clicked",
					  G_CALLBACK (prefs_page_currency_button_remove_clicked),
					  page);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_currency_init (PrefsPageCurrency *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void prefs_page_currency_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_currency_parent_class)->dispose (object);
}

static void prefs_page_currency_class_init (PrefsPageCurrencyClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_currency_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_currency.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrency, vbox_currency);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrency, button_currency_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrency, button_currency_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrency, grid_currency);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrency, dialog_currency_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrency, box_dialog_titre);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageCurrency, box_for_popup);
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
PrefsPageCurrency *prefs_page_currency_new (GrisbiPrefs *win)
{
	PrefsPageCurrency *page;

	page = g_object_new (PREFS_PAGE_CURRENCY_TYPE, NULL);
	prefs_page_currency_setup_page (page, win);

	return page;
}

/**
 * called when user change something in an entry
 * set the content of the entry for in the tree_view
 * (changes in memory done automatickly by gsb_autofunc_entry_new)
 *
 * \param entry
 * \param tree_view
 *
 * \return FALSE
 */
void prefs_page_currency_entry_changed (GtkWidget *entry,
										PrefsPageCurrency *page)
{
    GtkTreeModel *tree_model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gint currency_number;
	PrefsPageCurrencyPrivate *priv;

	priv = prefs_page_currency_get_instance_private (page);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_currency));
    currency_number = prefs_page_currency_get_selected (GTK_TREE_VIEW (priv->treeview_currency));

	if (!selection || !gtk_tree_selection_get_selected (selection, &tree_model, &iter))
		return;

    gtk_list_store_set (GTK_LIST_STORE (tree_model),
						&iter,
						CURRENCY_NAME_COLUMN, gsb_data_currency_get_name (currency_number),
						CURRENCY_CODE_ISO_COLUMN, gsb_data_currency_get_code_iso4217 (currency_number),
						CURRENCY_NICKNAME_COLUMN, gsb_data_currency_get_nickname (currency_number),
						CURRENCY_FLOATING_COLUMN,  gsb_data_currency_get_floating_point (currency_number),
						-1);
    gsb_currency_update_combobox_currency_list ();
    utils_prefs_gsb_file_set_modified ();
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
void prefs_page_currency_popup_fill_list (GtkTreeView *tree_view,
										  gboolean include_obsolete)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    Iso4217Currency *currency;

	currency = gsb_data_currency_get_tab_iso_4217_currencies ();

    model = gtk_tree_view_get_model (tree_view);
	gtk_list_store_clear (GTK_LIST_STORE (model));

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
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *prefs_page_currency_popup_get_treeview (GtkWidget *page)
{
	PrefsPageCurrencyPrivate *priv;

	priv = prefs_page_currency_get_instance_private (PREFS_PAGE_CURRENCY (page));

	return priv->treeview_popup;
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

