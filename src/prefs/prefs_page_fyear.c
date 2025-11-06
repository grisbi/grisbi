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
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_fyear.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_autofunc.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_fyear.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_fyear.h"
#include "structures.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageFyearPrivate   PrefsPageFyearPrivate;

struct _PrefsPageFyearPrivate
{
	GtkWidget *			vbox_fyear;

	GtkWidget *			grid_fyear_details;
	GtkWidget *			sw_fyear;
    GtkWidget *         treeview_fyear;

    GtkWidget *         button_fyear_add;
    GtkWidget *         button_fyear_associate;
    GtkWidget *         button_fyear_remove;
	GtkWidget *			checkbutton_fyear_show;
    GtkWidget *         checkbutton_fyear_sort_order;
    GtkWidget *         label_fyear_invalid;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageFyear, prefs_page_fyear, GTK_TYPE_BOX)

enum prefs_fyear_list_column {
    FYEAR_NAME_COLUMN = 0,
    FYEAR_BEGIN_DATE_COLUMN,
    FYEAR_END_DATE_COLUMN,
    FYEAR_INVALID_COLUMN,
    FYEAR_NUMBER_COLUMN,
	FYEAR_ROW_COLOR,
    NUM_FYEARS_COLUMNS
};


/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_fyear_button_fyear_show_toggled (GtkWidget *toggle_button,
													    GtkWidget *tree_view)
{
	GtkTreeModel *model;
    GtkTreeIter iter;
	GtkTreeSelection *selection;
	gint value = 0;
    gint fyear_number = 0;

	value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));
	devel_debug_int (value);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
		return;

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, FYEAR_NUMBER_COLUMN, &fyear_number, -1);

	if (fyear_number)
		gsb_data_fyear_set_form_show (fyear_number, value);
}

/**
 * called when change the selection of the fyear
 *
 * \param tree_selection
 * \param null
 *
 * \return FALSE
 **/
static gboolean prefs_page_fyear_select (GtkTreeSelection *tree_selection,
										 GtkWidget *tree_view)
{
	GtkWidget *widget;
	GtkTreeModel *model;
    GtkTreeIter iter;
    gint fyear_number;

	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (tree_selection), &model, &iter))
		return FALSE;

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, FYEAR_NUMBER_COLUMN, &fyear_number, -1);
    /* set the name */
    widget = g_object_get_data (G_OBJECT (model), "entry_fyear_name");
    gsb_autofunc_entry_set_value (widget, gsb_data_fyear_get_name (fyear_number), fyear_number);

    /* set the beginning date */
    widget = g_object_get_data (G_OBJECT (model), "entry_fyear_begin_date");
    gsb_calendar_entry_set_color (widget, TRUE);
    gsb_autofunc_date_set (widget, gsb_data_fyear_get_beginning_date (fyear_number), fyear_number);

    /* set the end date */
    widget = g_object_get_data (G_OBJECT (model), "entry_fyear_end_date");
    gsb_calendar_entry_set_color (widget, TRUE);
    gsb_autofunc_date_set (widget, gsb_data_fyear_get_end_date (fyear_number), fyear_number);

    /* set the button */
    widget = g_object_get_data (G_OBJECT (model), "checkbutton_fyear_show");
	g_signal_handlers_block_by_func (widget, prefs_page_fyear_button_fyear_show_toggled, tree_view);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
								  gsb_data_fyear_get_form_show (fyear_number));
	g_signal_handlers_unblock_by_func (widget, prefs_page_fyear_button_fyear_show_toggled, tree_view);

    /* set the invalid label */
    widget = g_object_get_data (G_OBJECT (model), "label_fyear_invalid");
    if (gsb_data_fyear_get_invalid (fyear_number))
    {
		gtk_label_set_markup (GTK_LABEL (widget), gsb_data_fyear_get_invalid_message (fyear_number));
		gtk_widget_show (widget);
    }
    else
		gtk_widget_hide (widget);

    /* sensitive what is needed */
    gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (model), "grid_fyear_details"), TRUE);
    gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (model), "button_fyear_remove"), TRUE);

    return FALSE;
}

/**
 * append a new line to the tree_view and fill it with the
 * link given in param
 *
 * \param model
 * \param fyear_number
 * \param iter 				a pointer to an iter to fill it with the position of the new link, or NULL
 *
 * \return
 **/
static void prefs_page_fyear_append_line (GtkTreeModel *model,
										  gint fyear_number,
										  GtkTreeIter *iter_to_fill)
{
    const gchar *invalid;
    GtkTreeIter local_iter;
    GtkTreeIter *iter_ptr;
	gchar *begin_date;
	gchar *end_date;

    if (iter_to_fill)
		iter_ptr = iter_to_fill;
    else
		iter_ptr = &local_iter;

    if (gsb_data_fyear_get_invalid (fyear_number))
		invalid = "gtk-dialog-warning";
    else
		invalid = NULL;

	begin_date = gsb_format_gdate (gsb_data_fyear_get_beginning_date (fyear_number));
	end_date = gsb_format_gdate (gsb_data_fyear_get_end_date (fyear_number));

    gtk_list_store_append (GTK_LIST_STORE (model), iter_ptr);
    gtk_list_store_set (GTK_LIST_STORE (model),
						iter_ptr,
						FYEAR_NAME_COLUMN, gsb_data_fyear_get_name (fyear_number),
						FYEAR_BEGIN_DATE_COLUMN, begin_date,
						FYEAR_END_DATE_COLUMN, end_date,
						FYEAR_INVALID_COLUMN, invalid,
						FYEAR_NUMBER_COLUMN, fyear_number,
						-1);
	g_free  (begin_date);
	g_free (end_date);
}

/**
 * called for add a new financial year
 *
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean prefs_page_fyear_button_add_clicked (GtkWidget *tree_view)
{
    GtkWidget *entry;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GDate *date;
    gint fyear_number;
	static gboolean add_mod = FALSE;

	if (add_mod)
	{
		add_mod = FALSE;

		/* Update various menus */
		gsb_fyear_update_fyear_list ();
    	gsb_file_set_modified (TRUE);

		return FALSE;
	}

    fyear_number = gsb_data_fyear_new (_("New financial year"));

    /* if bad things will append soon ... */
    if (!fyear_number)
		return FALSE;

	date = gdate_today ();
    gsb_data_fyear_set_form_show (fyear_number, TRUE);
    gsb_data_fyear_set_beginning_date (fyear_number, date);
    gsb_data_fyear_set_end_date (fyear_number, date);
    g_date_free (date);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

		/* append to the list and get back the iter */
		prefs_page_fyear_append_line (model, fyear_number, &iter);

		/* select it */
		gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);

		/* select the new name and give it the focus */
		entry = g_object_get_data (G_OBJECT (model), "entry_fyear_name");
		gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
		gtk_widget_grab_focus (entry);

		add_mod = TRUE;

	return FALSE;
}

/**
 * called to remove a financial year, check before if
 * some transactions are associated with it and warn if yes
 *
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean prefs_page_fyear_button_remove_clicked (GtkWidget *tree_view)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    gint fyear_number;
    gboolean warning_showed = FALSE;
    GSList *tmp_list;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    if (! gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
        return FALSE;
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, FYEAR_NUMBER_COLUMN, &fyear_number, -1);
	if (!fyear_number)
		return FALSE;

    /* first, we check if one transaction uses that financial year */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();

    while (tmp_list)
    {
		gint transaction_number;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
		if (fyear_number == gsb_data_transaction_get_financial_year_number (transaction_number))
		{
			/* at the beginning warning_showed is FALSE and we show a warning,
			 * if the user doesn't want to continue, we go out of the while so cannot come
			 * here again ; but if he wants to continue, warning_showed is set to TRUE, we delete
			 * the financial year and continue to come here to set the fyear of the
			 * transactions to 0 */
			if (warning_showed)
				gsb_data_transaction_set_financial_year_number (transaction_number, 0);
			else
			{
				gint result;

				result = dialogue_yes_no (_("If you really remove it, all the associated transactions "
											 "will be without financial year.\n"
											 "Are you sure?"),
										   _("The selected financial year is used in the file"),
										   GTK_RESPONSE_NO);
				if (result)
				{
					gsb_data_transaction_set_financial_year_number (transaction_number, 0);
					warning_showed = TRUE;
				}
				else
					break;
			}
		}
		tmp_list = tmp_list -> next;
    }

    /* if warning_showed is FALSE, it's because none transaction have that fyear,
     * or we answer NO to the warning but in that case, tmp_list is non NULL */
    if (warning_showed || !tmp_list)
    {
        gsb_data_fyear_remove (fyear_number);

        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

        if (g_slist_length (gsb_data_fyear_get_fyears_list ()) == 0)
        {
            gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (model), "grid_fyear_details"), FALSE);
            gtk_widget_set_sensitive (g_object_get_data (G_OBJECT (model), "button_fyear_remove"), FALSE);
        }

        /* Update various menus */
        gsb_file_set_modified (TRUE);
    }

    return FALSE;
}

/**
 * associate all transactions without fyear to the corresponding
 * fyear
 *
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_fyear_button_associate_clicked (GtkWidget *button,
														   gpointer null)
{
    GSList *tmp_list;
    gint modification_number = 0;

    if (!dialogue_yes_no (_("This function assigns each transaction without a financial year "
							"to the one related to its transaction date.  If no financial "
							"year matches, the transaction will not be changed."),
						  _("Automatic association of financial years?"),
						  GTK_RESPONSE_NO))
		return FALSE;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();

    while (tmp_list)
    {
		gint fyear_ope;
		TransactionStruct *transaction;

		transaction = tmp_list->data;
		fyear_ope = transaction->financial_year_number;
		if (fyear_ope <= 0)
		{
			gint fyear_number;
//~ printf ("fyear_ope = %d transaction_number = %d\n", fyear_ope, transaction_number);

			fyear_number = gsb_data_fyear_get_from_date (transaction->date);
			if (fyear_number)
			{
				gsb_data_transaction_set_financial_year_number (transaction->transaction_number, fyear_number);
				modification_number++;
			}
		}
		tmp_list = tmp_list -> next;
    }

    if (modification_number)
    {
		gchar* tmp_str;

		tmp_str = g_strdup_printf (_("%d transactions associated"), modification_number);
		dialogue (tmp_str);
		g_free (tmp_str);
		transaction_list_update_element (ELEMENT_EXERCICE);
		gsb_file_set_modified (TRUE);
    }
    else
		dialogue (_("No transaction to associate"));

	return FALSE;
}

/**
 * update the invalid fyears in the list view
 *
 * \param tree_view the tree_view
 *
 * \return
 **/
static void prefs_page_fyear_update_invalid (GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    /* first update all the invalids flags for the fyears */
    gsb_data_fyear_check_all_for_invalid ();
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    /* now go throw the list and show/hide the invalid flag */
	if (!gtk_tree_model_get_iter_first (model, &iter))
		return;

    do
    {
		gint fyear_number;
		const gchar *invalid;

		gtk_tree_model_get (model, &iter, FYEAR_NUMBER_COLUMN, &fyear_number, -1);
		if (gsb_data_fyear_get_invalid (fyear_number))
			invalid = "gtk-dialog-warning";
		else
			invalid = NULL;

		gtk_list_store_set (GTK_LIST_STORE (model), &iter, FYEAR_INVALID_COLUMN, invalid, -1);
	}
    while (gtk_tree_model_iter_next (model, &iter));
}

/**
 * called when something change for a fyear
 * update the list and the invalid
 *
 * \param entry the entry which change
 * \param tree_view the tree_view
 *
 * \return FALSE
 **/
static gboolean prefs_page_fyear_modify_fyear (GtkWidget *entry,
											   GtkWidget *tree_view)
{
    GtkTreeModel *model;
	GtkTreeSelection *selection;
    GtkTreeIter iter;
    gint fyear_number;
    GtkWidget *widget;
    gchar *invalid;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
		return FALSE;

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, FYEAR_NUMBER_COLUMN, &fyear_number, -1);

    /* normally should not happen */
    if (!fyear_number)
		return FALSE;

	/* check the invalid and show the message if needed */
	widget = g_object_get_data (G_OBJECT (model), "label_fyear_invalid");

	/* check all the fyear to set them invalid if need */
	prefs_page_fyear_update_invalid (tree_view);

     /* and check if the current fyear was set as invalid */
     if (gsb_data_fyear_get_invalid (fyear_number))
     {
		 /* and now focus on the current fyear */
		 invalid = g_strdup ("gtk-dialog-warning");
		 gtk_label_set_markup (GTK_LABEL (widget), gsb_data_fyear_get_invalid_message (fyear_number));
		 gtk_widget_show (widget);
	 }
	 else
	 {
		 invalid = NULL;
		 gtk_widget_hide (widget);
     }

	gtk_list_store_set (GTK_LIST_STORE (model),
						&iter,
						FYEAR_NAME_COLUMN, gsb_data_fyear_get_name (fyear_number),
						FYEAR_BEGIN_DATE_COLUMN, gsb_format_gdate (gsb_data_fyear_get_beginning_date (fyear_number)),
						FYEAR_END_DATE_COLUMN, gsb_format_gdate (gsb_data_fyear_get_end_date (fyear_number)),
						FYEAR_INVALID_COLUMN, invalid,
						FYEAR_NUMBER_COLUMN, fyear_number,
						-1);
	if (invalid)
		g_free (invalid);

     gsb_file_set_modified (TRUE);

     return FALSE;
}

/**
 * fill the list of links
 *
 * \param model the tree_model to fill
 *
 * \return
 **/
static void prefs_page_fyear_fill_list (GtkTreeModel *store)
{
    GSList *tmp_list;

    gtk_list_store_clear (GTK_LIST_STORE (store));
    tmp_list = gsb_data_fyear_get_fyears_list ();

    while (tmp_list)
    {
		gint fyear_number;

		fyear_number = gsb_data_fyear_get_no_fyear (tmp_list -> data);
		prefs_page_fyear_append_line (store, fyear_number, NULL);

		tmp_list = tmp_list -> next;
    }
}

/**
 * Init the tree_view for the fyears list configuration
 * set the model given in param
 * set the columns and all the connections
 *
 * \param model the model to set in the tree_view
 *
 * \return the tree_view
 **/
static void prefs_page_fyear_setup_tree_view (PrefsPageFyear *page,
											  GrisbiAppConf *a_conf)
{
    GtkCellRenderer *cell_renderer;
	GtkListStore *store = NULL;
	GtkTreeSelection *selection;
    gchar *title[] = {_("Name"), _("Begin date"), _("End date"), _("Invalid")};
    gint i;
	PrefsPageFyearPrivate *priv;

	priv = prefs_page_fyear_get_instance_private (page);

	/* Create tree store */
    store = gtk_list_store_new (NUM_FYEARS_COLUMNS,
								G_TYPE_STRING,			/* FYEAR_NAME_COLUMN */
								G_TYPE_STRING,			/* FYEAR_BEGIN_DATE_COLUMN */
								G_TYPE_STRING,			/* FYEAR_END_DATE_COLUMN */
								G_TYPE_STRING,			/* FYEAR_INVALID_COLUMN affiche une icone */
								G_TYPE_INT,				/* FYEAR_NUMBER_COLUMN */
								GDK_TYPE_RGBA);			/* FYEAR_ROW_COLOR */

    /* Create tree tree_view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_fyear), GTK_TREE_MODEL (store));
	gtk_widget_set_name (priv->treeview_fyear, "tree_view");
    g_object_unref (G_OBJECT (store));

    /* for all the columns it's a text */
    cell_renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (cell_renderer), "xalign", 0.5, NULL);

    /* fill the columns : set FYEAR_NUMBER_COLUMN and not NUM_FYEARS_COLUMNS because */
    /* the last value of the model mustn't be to text...							 */
    /* so FYEAR_NUMBER_COLUMN must be the first column after the last column showed  */

    for (i=0 ; i<FYEAR_NUMBER_COLUMN; i++)
    {
		GtkTreeViewColumn *column;

		if (i == FYEAR_INVALID_COLUMN)
		{
			column = gtk_tree_view_column_new_with_attributes (title[i],
															   gtk_cell_renderer_pixbuf_new (),
															   "icon-name", i,
															   "cell-background-rgba", FYEAR_ROW_COLOR,
															   NULL);
		}
		else
		{
			column = gtk_tree_view_column_new_with_attributes (title[i],
															   cell_renderer,
															   "text", i,
															   "cell-background-rgba", FYEAR_ROW_COLOR,
															   NULL);
			gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
			gtk_tree_view_column_set_expand (column, TRUE);
		}
		gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_fyear), column);
    }

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), FYEAR_NAME_COLUMN, a_conf->prefs_fyear_sort_order);
	prefs_page_fyear_fill_list (GTK_TREE_MODEL (store));

	/* set signal */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_fyear));
    g_signal_connect (selection,
					  "changed",
					  G_CALLBACK (prefs_page_fyear_select),
					  priv->treeview_fyear);

}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_fyear_button_sort_order_toggled (GtkWidget *toggle_button,
													    GtkWidget *tree_view)
{
    GtkTreeModel *model;
	gboolean is_loading;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	is_loading = grisbi_win_file_is_loading ();

	if (is_loading)
	{
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
		gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model),
											  FYEAR_NAME_COLUMN,
											  a_conf->prefs_fyear_sort_order);
		gtk_tree_sortable_sort_column_changed (GTK_TREE_SORTABLE(model));
		prefs_page_fyear_fill_list (model);

		utils_set_list_store_background_color (tree_view, FYEAR_ROW_COLOR);
	}
}

/**
 * Création de la page de gestion des fyear
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_fyear_setup_page (PrefsPageFyear *page)
{
	GtkWidget *head_page;
	gboolean is_loading;
	GrisbiAppConf *a_conf;
	PrefsPageFyearPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_fyear_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Financial years"), "gsb-financial-years-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_fyear), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_fyear), head_page, 0);

    /* set the checkbutton_fyear_sort_order  */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_fyear_sort_order),
								  a_conf->prefs_fyear_sort_order);

	if (is_loading)
	{
		GtkWidget *entry;
		GtkTreeModel *model;

		prefs_page_fyear_setup_tree_view (page, a_conf);
		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_fyear));

		g_object_set_data (G_OBJECT (model), "button_fyear_associate", priv->button_fyear_associate);
    	g_object_set_data (G_OBJECT (model), "button_fyear_remove", priv->button_fyear_remove);

		/* set entry name */
		entry = gsb_autofunc_entry_new (NULL,
									    G_CALLBACK (prefs_page_fyear_modify_fyear), priv->treeview_fyear,
									    G_CALLBACK (gsb_data_fyear_set_name), 0);
		g_object_set_data (G_OBJECT (model), "entry_fyear_name",  entry);
		gtk_widget_set_margin_end (entry, MARGIN_END);
		gtk_grid_attach (GTK_GRID (priv->grid_fyear_details), entry, 1, 0, 1, 1);

		/* set entry date */
		entry = gsb_autofunc_date_new (NULL,
									   G_CALLBACK (prefs_page_fyear_modify_fyear), priv->treeview_fyear,
									   G_CALLBACK (gsb_data_fyear_set_beginning_date), 0);
		g_object_set_data (G_OBJECT (model), "entry_fyear_begin_date", entry);
		gtk_widget_set_margin_end (entry, MARGIN_END);
		gtk_grid_attach (GTK_GRID (priv->grid_fyear_details), entry, 1, 1, 1, 1);

		/* set end date */
		entry = gsb_autofunc_date_new (NULL,
									   G_CALLBACK (prefs_page_fyear_modify_fyear), priv->treeview_fyear,
									   G_CALLBACK (gsb_data_fyear_set_end_date), 0);
		g_object_set_data (G_OBJECT (model), "entry_fyear_end_date", entry);
		gtk_widget_set_margin_end (entry, MARGIN_END);
		gtk_grid_attach (GTK_GRID (priv->grid_fyear_details), entry, 1, 2, 1, 1);

		g_object_set_data (G_OBJECT (model), "checkbutton_fyear_show", priv->checkbutton_fyear_show);
		g_object_set_data (G_OBJECT (model), "grid_fyear_details", priv->grid_fyear_details);
		g_object_set_data (G_OBJECT (model), "label_fyear_invalid", priv->label_fyear_invalid);
		utils_set_list_store_background_color (priv->treeview_fyear, FYEAR_ROW_COLOR);

		/* initialise  button_fyear_associate */
		if (gsb_data_fyear_get_fyears_list () == NULL)
			gtk_widget_set_sensitive (priv->button_fyear_associate, FALSE);
		else
			gtk_widget_set_sensitive (priv->button_fyear_associate, TRUE);

		/* set signals */
		g_signal_connect_swapped (G_OBJECT (priv->button_fyear_add),
								  "clicked",
								  G_CALLBACK  (prefs_page_fyear_button_add_clicked),
								  priv->treeview_fyear);
		g_signal_connect_swapped (G_OBJECT (priv->button_fyear_remove),
								  "clicked",
								  G_CALLBACK  (prefs_page_fyear_button_remove_clicked),
								  priv->treeview_fyear);
    	g_signal_connect (G_OBJECT (priv->button_fyear_associate),
						  "clicked",
						  G_CALLBACK (prefs_page_fyear_button_associate_clicked),
						  NULL);
		g_signal_connect (priv->checkbutton_fyear_show,
						  "toggled",
						  G_CALLBACK (prefs_page_fyear_button_fyear_show_toggled),
						  priv->treeview_fyear);
	}
	else
	{
		gtk_widget_set_sensitive (priv->sw_fyear, FALSE);
		gtk_widget_set_sensitive (priv->button_fyear_add, FALSE);
	}

	/* insensibilise les autres widgets inutilisés */
	gtk_widget_set_sensitive (priv->button_fyear_remove, FALSE);
	gtk_widget_set_sensitive (priv->grid_fyear_details, FALSE);

    /* Connect signal */
    g_signal_connect (priv->checkbutton_fyear_sort_order,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->prefs_fyear_sort_order);

    g_signal_connect_after (priv->checkbutton_fyear_sort_order,
							"toggled",
							G_CALLBACK (prefs_page_fyear_button_sort_order_toggled),
							priv->treeview_fyear);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_fyear_init (PrefsPageFyear *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_fyear_setup_page (page);
}

static void prefs_page_fyear_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_fyear_parent_class)->dispose (object);
}

static void prefs_page_fyear_class_init (PrefsPageFyearClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_fyear_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_fyear.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, vbox_fyear);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, grid_fyear_details);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, sw_fyear);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, treeview_fyear);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, button_fyear_add);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, button_fyear_associate);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, button_fyear_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, checkbutton_fyear_show);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, checkbutton_fyear_sort_order);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFyear, label_fyear_invalid);
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
PrefsPageFyear *prefs_page_fyear_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_FYEAR_TYPE, NULL);
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

