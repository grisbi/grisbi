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

#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_currency_popup.h"
#include "gsb_currency.h"
#include "gsb_data_currency.h"
#include "gsb_dirs.h"
#include "gsb_locale.h"
#include "utils.h"
#include "structures.h"
#include "utils_widgets.h"
#include "widget_currency_details.h"
#include "widget_currency_popup.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_GLOBAL*/
/*END_GLOBAL*/

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/**
 * fill the currency list in the popup
 * with all the world currencies
 *
 * \param tree_view the tree tree_view to fill in
 * \param include_obsolete TRUE to fill with the obsolete curencies
 *
 * \return
 **/
void gsb_popup_list_fill_model (GtkTreeView *tree_view,
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
 * renvoie le model pour la popup list
 *
 * \param
 *
 * \return
 **/
GtkTreeModel *gsb_popup_list_store_new (void)
{
    GtkListStore *model;

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

	return GTK_TREE_MODEL (model);
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
gboolean gsb_popup_list_select_default (GtkTreeModel *tree_model,
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
 * ajoute une nouvelle devise si elle n'existe pas déjà
 *
 * \param
 *
 * \return
 **/
gint gsb_popup_list_selected_currency_new (GtkTreeSelection *selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
    const gchar *currency_isocode;
    const gchar *currency_name;
    const gchar *currency_nickname;
    gint currency_floating;
	gint currency_number = 0;

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return FALSE;

    gtk_tree_model_get (model,
						&iter,
						CURRENCY_NAME_COLUMN, &currency_name,
						CURRENCY_CODE_ISO_COLUMN, &currency_isocode,
						CURRENCY_NICKNAME_COLUMN, &currency_nickname,
						CURRENCY_FLOATING_COLUMN, &currency_floating,
						-1);

	if (!gsb_data_currency_get_number_by_name (currency_name)
		&& !gsb_data_currency_get_number_by_code_iso4217 (currency_isocode))
	{
		currency_number = gsb_data_currency_new_with_data (currency_name,
														   currency_nickname,
														   currency_isocode,
														   currency_floating);
		return currency_number;
	}

	return 0;
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
void gsb_popup_list_selection_changed (GtkTreeSelection *selection,
									   GtkWidget *w_currency_details)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
    const gchar *currency_iso_code;
    const gchar *currency_name;
    const gchar *currency_nickname;
    gint currency_floating;

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;

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
	widget_currency_details_update_from_iso_4217 (w_currency_details,
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
GtkWidget *gsb_popup_list_iso_4217_new (GtkWidget *parent,
										GCallback select_callback)
{
	GtkWidget *tree_view;
	GtkWidget *vbox;
    GtkWidget *w_currency_popup;
    GtkTreeModel *model;

    vbox = new_vbox_with_title_and_icon ( _("Select base currency"), "gsb-currencies-32.png" );
	if (parent)
	    gtk_box_pack_start (GTK_BOX (parent), vbox, TRUE, TRUE, 0);

	/* get model */
	model = gsb_popup_list_store_new ();

	/* set popup */
	w_currency_popup = GTK_WIDGET (widget_currency_popup_new (GTK_TREE_MODEL (model)));
	gtk_box_pack_start (GTK_BOX (vbox), w_currency_popup, TRUE, TRUE, 0);

	/* fill model */
	tree_view = widget_currency_popup_get_tree_view (w_currency_popup);
	gsb_popup_list_fill_model (GTK_TREE_VIEW (tree_view), FALSE);

	/* tree_view est attaché à 2 objets pour 2 fonctions différentes */
	g_object_set_data (G_OBJECT (vbox), "tree_view", tree_view);
	g_object_set_data (G_OBJECT (w_currency_popup), "tree_view", tree_view);

	g_object_set_data (G_OBJECT (vbox), "w_currency_popup", w_currency_popup);

	if (select_callback)
		g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
						  "changed",
						  G_CALLBACK (select_callback),
						  model);

	return vbox;
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
