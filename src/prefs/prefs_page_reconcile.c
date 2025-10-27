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
#include "prefs_page_reconcile.h"
#include "dialog.h"
#include "gsb_assistant_reconcile_config.h"
#include "gsb_autofunc.h"
#include "gsb_data_account.h"
#include "gsb_data_reconcile.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageReconcilePrivate   PrefsPageReconcilePrivate;

struct _PrefsPageReconcilePrivate
{
	GtkWidget *			vbox_reconcile;

	GtkWidget *			button_collapse_row;
	GtkWidget *			button_reconcile_delete;
	GtkWidget *			button_reconcile_find;
    GtkWidget *			checkbutton_reconcile_sort;
	GtkWidget *			entry_reconcile_init_balance;
	GtkWidget *			entry_reconcile_init_date;
	GtkWidget *			entry_reconcile_final_balance;
	GtkWidget *			entry_reconcile_final_date;
	GtkWidget *			entry_reconcile_name;
	GtkWidget *			grid_reconcile;
	GtkWidget * 		radiobutton_end_date_1;
	GtkWidget * 		radiobutton_end_date_2;
	GtkWidget *			treeview_reconcile;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageReconcile, prefs_page_reconcile, GTK_TYPE_BOX)

enum ReconciliationColumns
{
    RECONCILIATION_NAME_COLUMN = 0,
    RECONCILIATION_INIT_DATE_COLUMN,
    RECONCILIATION_FINAL_DATE_COLUMN,
    RECONCILIATION_INIT_BALANCE_COLUMN,
    RECONCILIATION_FINAL_BALANCE_COLUMN,
    RECONCILIATION_ACCOUNT_COLUMN,
    RECONCILIATION_RECONCILE_COLUMN,
    RECONCILIATION_WEIGHT_COLUMN,
	RECONCILIATION_BACKGROUND_COLOR,
    NUM_RECONCILIATION_COLUMNS
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * sensitive unsensitive details of reconcile
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_sensitive_details (PrefsPageReconcile *page,
													gboolean sensitive)
{
	PrefsPageReconcilePrivate *priv;

	priv = prefs_page_reconcile_get_instance_private (page);

	gtk_widget_set_sensitive (priv->button_reconcile_delete, sensitive);
	gtk_widget_set_sensitive (priv->entry_reconcile_init_balance, sensitive);
	gtk_widget_set_sensitive (priv->entry_reconcile_init_date, sensitive);
	gtk_widget_set_sensitive (priv->entry_reconcile_final_balance, sensitive);
	gtk_widget_set_sensitive (priv->entry_reconcile_final_date, sensitive);
	gtk_widget_set_sensitive (priv->entry_reconcile_name, sensitive);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_button_collapse_row_clicked (GtkButton *button,
															  GtkTreeSelection *selection)
{
	GtkTreeView *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
	GtkTreePath *path;
	gint indice;

	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;
	path = gtk_tree_model_get_path (model, &iter);

	/* on rgarde si on est sur un rapprochement pour regrouper le compte */
	indice = gtk_tree_path_get_depth (path);
	if (indice == 2)
		gtk_tree_path_up (path);

	tree_view = gtk_tree_selection_get_tree_view (selection);
	gtk_tree_view_collapse_row (tree_view, path);
	gtk_tree_view_set_cursor_on_cell (tree_view, path, NULL, NULL, FALSE);
	utils_set_tree_store_background_color (GTK_WIDGET (tree_view), RECONCILIATION_BACKGROUND_COLOR);
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_treeview_row_collapsed (GtkTreeView *tree_view,
														 GtkTreeIter *iter,
														 GtkTreePath *path,
														 GtkWidget *button)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (tree_view);
	gtk_tree_selection_select_iter (selection, iter);
	utils_set_tree_store_background_color (GTK_WIDGET (tree_view), RECONCILIATION_BACKGROUND_COLOR);
	gtk_widget_set_sensitive (button, FALSE);
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_treeview_row_expanded (GtkTreeView *tree_view,
													    GtkTreeIter *iter,
													    GtkTreePath *path,
													    GtkWidget *button)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (tree_view);
	gtk_tree_selection_select_iter (selection, iter);
	utils_set_tree_store_background_color (GTK_WIDGET (tree_view), RECONCILIATION_BACKGROUND_COLOR);
	gtk_widget_set_sensitive (button, TRUE);
}

/**
 * callback called when select a line in the tree
 * fill the entry of the name, dates and balances
 *
 * \param selection the GtkTreeSelection
 * \param table the GtkTable containing the widgets to set the value of the selection
 *
 * \return FALSE
 **/
static void prefs_page_reconcile_row_selected (GtkTreeSelection *selection,
											   PrefsPageReconcile *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean good;
	PrefsPageReconcilePrivate *priv;

	priv = prefs_page_reconcile_get_instance_private (page);

    good = gtk_tree_selection_get_selected (selection, &model, &iter);
    if (good)
    {
		gint reconcile_number;

		gtk_tree_model_get (model, &iter, RECONCILIATION_RECONCILE_COLUMN, &reconcile_number, -1);

		if (reconcile_number)
		{
			/* we are on a payment method, we fill the fields */
			gsb_autofunc_entry_set_value (priv->entry_reconcile_name,
										  gsb_data_reconcile_get_name (reconcile_number),
										  reconcile_number);
			gsb_autofunc_date_set (priv->entry_reconcile_init_date,
								   gsb_data_reconcile_get_init_date (reconcile_number),
								   reconcile_number);
			gsb_autofunc_date_set (priv->entry_reconcile_final_date,
								   gsb_data_reconcile_get_final_date (reconcile_number),
								   reconcile_number);
			gsb_autofunc_real_set (priv->entry_reconcile_init_balance,
								   gsb_data_reconcile_get_init_balance (reconcile_number),
								   reconcile_number);
			gsb_autofunc_real_set (priv->entry_reconcile_final_balance,
								   gsb_data_reconcile_get_final_balance (reconcile_number),
								   reconcile_number);

			/* we make the table sensitive */
			prefs_page_reconcile_sensitive_details (page, TRUE);
		}
		else
			prefs_page_reconcile_sensitive_details (page, FALSE);
    }
    else
		prefs_page_reconcile_sensitive_details (page, FALSE);
}

/**
 * callback called when something change in the entries of the configuration of the reconcile
 *
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean prefs_page_reconcile_update_line (GtkWidget *entry,
												  GtkWidget *tree_view)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    gboolean good;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (good)
    {
		gint reconcile_number;

		gtk_tree_model_get (model, &iter, RECONCILIATION_RECONCILE_COLUMN, &reconcile_number, -1);

		if (reconcile_number)
		{
			/* we are on a reconcile, change the line with the new values */
			gchar *init_balance;
			gchar *init_date;
			gchar *final_balance;
			gchar *final_date;

			init_date = gsb_format_gdate (gsb_data_reconcile_get_init_date (reconcile_number));
			final_date = gsb_format_gdate (gsb_data_reconcile_get_final_date (reconcile_number));
			init_balance = utils_real_get_string (gsb_data_reconcile_get_init_balance (reconcile_number));
			final_balance = utils_real_get_string (gsb_data_reconcile_get_final_balance (reconcile_number));

			gtk_tree_store_set (GTK_TREE_STORE (model),
								&iter,
						 		RECONCILIATION_NAME_COLUMN, gsb_data_reconcile_get_name (reconcile_number),
								RECONCILIATION_INIT_DATE_COLUMN, init_date,
								RECONCILIATION_FINAL_DATE_COLUMN, final_date,
								RECONCILIATION_INIT_BALANCE_COLUMN, init_balance,
								RECONCILIATION_FINAL_BALANCE_COLUMN, final_balance,
								-1);
			g_free (init_date);
			g_free (final_date);
			g_free (init_balance);
			g_free (final_balance);
		}
    }

    return FALSE;
}

/**
 * Init the tree_view for the reconcile list configuration
 * set the model given in param
 * set the columns and all the connections
 *
 * \param tree_view
 *
 * \return
 **/
static void prefs_page_reconcile_setup_tree_view (PrefsPageReconcile *page)
{
	GtkTreeStore *store = NULL;
	GtkTreePath *path;
	GtkTreeSelection *selection;
    gfloat alignment[] = {COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER, COLUMN_RIGHT, COLUMN_RIGHT};
    const gchar *titles[] = {N_("Account"), N_("Init date"), N_("Final date"),N_("Init balance"), N_("Final balance") };
    gint i;
	PrefsPageReconcilePrivate *priv;

	priv = prefs_page_reconcile_get_instance_private (page);

	/* create the model */
    store = gtk_tree_store_new (NUM_RECONCILIATION_COLUMNS,
								G_TYPE_STRING,		/* Name account or reconciliation */
							    G_TYPE_STRING,		/* init date  */
							    G_TYPE_STRING,		/* final date  */
							    G_TYPE_STRING,		/* init balance  */
							    G_TYPE_STRING,		/* final balance  */
							    G_TYPE_INT,			/* Account number */
					   			G_TYPE_INT,			/* Bold or regular text */
							    G_TYPE_INT,			/* reconciliation number */
							    GDK_TYPE_RGBA);

    /* Create tree tree_view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_reconcile), GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));

    /* Name */
    for (i = RECONCILIATION_NAME_COLUMN; i < RECONCILIATION_ACCOUNT_COLUMN; i++)
    {
		GtkTreeViewColumn *column;
		GtkCellRenderer *cell;

		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", alignment[i], NULL);
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_alignment (column, alignment[i]);
		gtk_tree_view_column_pack_end (column, cell, TRUE);
		gtk_tree_view_column_set_title (column, gettext (titles[i]));
		gtk_tree_view_column_set_attributes (column, cell,
							 "text", i,
					     	 "weight", RECONCILIATION_WEIGHT_COLUMN,
							 "cell-background-rgba", RECONCILIATION_BACKGROUND_COLOR,
							 NULL);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		gtk_tree_view_column_set_expand (column, TRUE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(priv->treeview_reconcile), column);
    }

	/* fill the model */
	prefs_page_reconcile_fill (priv->treeview_reconcile);

	/* set colors */
	utils_set_tree_store_background_color (priv->treeview_reconcile, RECONCILIATION_BACKGROUND_COLOR);

	/* select the first item */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_reconcile));
	path = gtk_tree_path_new_from_indices (0, -1);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	/* set signals */
	g_signal_connect (G_OBJECT (priv->button_collapse_row),
					  "clicked",
					  G_CALLBACK (prefs_page_reconcile_button_collapse_row_clicked),
					  selection);
    g_signal_connect (selection,
                      "changed",
                      G_CALLBACK (prefs_page_reconcile_row_selected),
                      page);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_reconcile_button_end_date_toggled (GtkWidget *checkbutton,
															  GdkEventButton *event,
															  gpointer null)
{
	gint active;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
	if (active)
    	w_etat->reconcile_end_date = TRUE;
	else
		w_etat->reconcile_end_date = FALSE;

    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return FALSE
 **/
static void prefs_page_reconcile_button_reconcile_sort_toggled (GtkWidget *checkbutton,
															  	GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path_selected = NULL;
	GtkTreeSelection *selection;
	gint active;
	gint depth = 0;
	gint reconcile_number = 0;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
    w_etat->reconcile_sort = active;

	/* update reconcile list */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		path_selected = gtk_tree_model_get_path (model, &iter);
		depth = gtk_tree_path_get_depth (path_selected);
		if (depth > 1)
		{
			gtk_tree_model_get (model, &iter, RECONCILIATION_RECONCILE_COLUMN, &reconcile_number, -1);
		}
	}

    prefs_page_reconcile_fill (tree_view);
	if (path_selected)
	{
		if (depth > 1)
		{
			GtkTreeIter fils_iter;
			GtkTreePath *tmp_path;
			gint tmp_number;

			tmp_path = gtk_tree_path_copy (path_selected);
			gtk_tree_path_up (tmp_path);
			gtk_tree_view_expand_row (GTK_TREE_VIEW (tree_view), tmp_path, FALSE);
			gtk_tree_model_get_iter (model, &iter, tmp_path);
			gtk_tree_selection_select_path (selection, tmp_path);
			gtk_tree_path_free (tmp_path);
	    	if (gtk_tree_model_iter_children (model, &fils_iter, &iter))
    		{
				do
				{
					gtk_tree_model_get (model, &fils_iter, RECONCILIATION_RECONCILE_COLUMN, &tmp_number, -1);
					if (tmp_number == reconcile_number)
						break;
				}
				while (gtk_tree_model_iter_next (model, &fils_iter));
				gtk_tree_selection_select_iter ( selection, &fils_iter);
				tmp_path = gtk_tree_model_get_path (model, &fils_iter);
				gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), tmp_path, NULL, FALSE, 0, 0);
				gtk_tree_path_free (tmp_path);
			}
		}
		else
		{
			gtk_tree_selection_select_path (selection, path_selected);
		}

		gtk_tree_path_free (path_selected);
	}
	utils_set_tree_store_background_color (tree_view, RECONCILIATION_BACKGROUND_COLOR);
    gsb_file_set_modified (TRUE);
}

/**
 * called by user, find all the R marked transactions without a number of reconcile
 * show them in a list and propose to associate them to a reconcile
 *
 * \param
 *
 * \return FALSE
 **/
static void prefs_page_reconcile_find_alone_transactions_clicked (GtkWidget *checkbutton,
																   GtkWidget *tree_view)
{
    gsb_assistant_reconcile_config_run (tree_view);
}

/**
 * callback called when the user click on 'Delete the reconciliation'
 * this will delete the selected reconciliation and will mark all the concerned
 * transactions as P
 *
 * \param button
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean prefs_page_reconcile_delete_clicked (GtkWidget *button,
													 GtkWidget *tree_view)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    gboolean good;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (good)
    {
		gint reconcile_number;
		gint account_number;

		gtk_tree_model_get (model, &iter, RECONCILIATION_RECONCILE_COLUMN, &reconcile_number, -1);

		if (reconcile_number)
		{
			gchar* tmp_str;
            gboolean result;

			/* we are on a reconcile,
			 * we ask if the user want to continue and warn him */
			tmp_str = g_strdup_printf (_("Caution, you are about to delete a reconciliation.\n"
										 "If you continue, the reconciliation '%s' will be erased and "
										 "all the transactions marked by this reconciliation will "
										 "be un-reconciled and marked P.\n"
										 "Are you sure you want to continue?"),
									   gsb_data_reconcile_get_name (reconcile_number));
            result = dialogue_yes_no (tmp_str, _("Delete reconciliation"), GTK_RESPONSE_NO);
            g_free (tmp_str);
			if (!result)
			{
				return FALSE;
			}

			/* ok we delete the reconcile
			 * this will automatickly remove it from the marked transactions */
			gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
			gsb_data_reconcile_remove (reconcile_number);

			/* if we are on an account, we update the list */
			account_number = gsb_gui_navigation_get_current_account ();
			if (account_number != -1)
			{
				gsb_transactions_list_update_tree_view (account_number, TRUE);

				/* update the last statement for that account */
				gsb_gui_navigation_update_statement_label (account_number);
			}
			gsb_file_set_modified (TRUE);
		}
    }

    return FALSE;
}

/**
 * Création de la page de gestion des reconcile
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_reconcile_setup_page (PrefsPageReconcile *page)
{
	GtkWidget *head_page;
	GrisbiWinEtat *w_etat;
	PrefsPageReconcilePrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_reconcile_get_instance_private (page);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Reconciliation"), "gsb-reconciliation-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_reconcile), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_reconcile), head_page, 0);

	/* set the end date of reconcile */
	if (w_etat->reconcile_end_date)
		 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_end_date_2), TRUE);
	else
		 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_end_date_1), TRUE);

	/* set the list of reconcile */
	prefs_page_reconcile_setup_tree_view (page);

	/* set the entries */
	priv->entry_reconcile_name = gsb_autofunc_entry_new (NULL,
														 G_CALLBACK (prefs_page_reconcile_update_line),
													     priv->treeview_reconcile,
													     G_CALLBACK (gsb_data_reconcile_set_name),
													     0);
    gtk_widget_set_size_request (priv->entry_reconcile_name, ENTRY_MIN_WIDTH, -1);
	gtk_grid_attach (GTK_GRID (priv->grid_reconcile), priv->entry_reconcile_name, 2, 6, 1, 1);

	priv->entry_reconcile_init_date = gsb_autofunc_date_new (NULL,
															 G_CALLBACK (prefs_page_reconcile_update_line),
															 priv->treeview_reconcile,
															 G_CALLBACK (gsb_data_reconcile_set_init_date),
															 0);
    gtk_widget_set_size_request (priv->entry_reconcile_init_date, ENTRY_MIN_WIDTH, -1);
	gtk_grid_attach (GTK_GRID (priv->grid_reconcile), priv->entry_reconcile_init_date, 2, 7, 1, 1);

	priv->entry_reconcile_final_date = gsb_autofunc_date_new (NULL,
															  G_CALLBACK (prefs_page_reconcile_update_line),
															  priv->treeview_reconcile,
															  G_CALLBACK (gsb_data_reconcile_set_final_date),
															  0);
    gtk_widget_set_size_request (priv->entry_reconcile_final_date, ENTRY_MIN_WIDTH, -1);
	gtk_grid_attach (GTK_GRID (priv->grid_reconcile), priv->entry_reconcile_final_date, 2, 8, 1, 1);

	priv->entry_reconcile_init_balance = gsb_autofunc_real_new (null_real,
																G_CALLBACK (prefs_page_reconcile_update_line),
																priv->treeview_reconcile,
																G_CALLBACK (gsb_data_reconcile_set_init_balance),
																0);
    gtk_widget_set_size_request (priv->entry_reconcile_init_balance, ENTRY_MIN_WIDTH, -1);
	gtk_grid_attach (GTK_GRID (priv->grid_reconcile), priv->entry_reconcile_init_balance, 4, 7, 1, 1);

	priv->entry_reconcile_final_balance = gsb_autofunc_real_new (null_real,
																 G_CALLBACK (prefs_page_reconcile_update_line),
																 priv->treeview_reconcile,
																 G_CALLBACK (gsb_data_reconcile_set_final_balance),
																 0);
    gtk_widget_set_size_request (priv->entry_reconcile_final_balance, ENTRY_MIN_WIDTH, -1);
    gtk_grid_attach (GTK_GRID (priv->grid_reconcile), priv->entry_reconcile_final_balance, 4, 8, 1, 1);

	/* set the checkbutton_reconcile_sort */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_reconcile_sort),
								  w_etat->reconcile_sort);

	/* set unsensitive details */
	prefs_page_reconcile_sensitive_details (page, FALSE);
	gtk_widget_set_sensitive (priv->button_collapse_row, FALSE);

    /* Connect signal */
    g_signal_connect (G_OBJECT (priv->radiobutton_end_date_2),
					  "toggled",
					  G_CALLBACK (prefs_page_reconcile_button_end_date_toggled),
					  NULL);

    g_signal_connect (G_OBJECT (priv->treeview_reconcile),
                      "row-expanded",
                      G_CALLBACK (prefs_page_reconcile_treeview_row_expanded),
                      priv->button_collapse_row);

    g_signal_connect (G_OBJECT (priv->treeview_reconcile),
                      "row-collapsed",
                      G_CALLBACK (prefs_page_reconcile_treeview_row_collapsed),
                      priv->button_collapse_row);

	g_signal_connect (G_OBJECT (priv->button_reconcile_delete),
					  "clicked",
					  G_CALLBACK (prefs_page_reconcile_delete_clicked),
					  priv->treeview_reconcile);

	g_signal_connect (G_OBJECT (priv->checkbutton_reconcile_sort),
					  "toggled",
					  G_CALLBACK (prefs_page_reconcile_button_reconcile_sort_toggled),
					  priv->treeview_reconcile);

	g_signal_connect (G_OBJECT (priv->button_reconcile_find),
                      "clicked",
                      G_CALLBACK (prefs_page_reconcile_find_alone_transactions_clicked),
                      priv->treeview_reconcile);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_reconcile_init (PrefsPageReconcile *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_reconcile_setup_page (page);
}

static void prefs_page_reconcile_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_reconcile_parent_class)->dispose (object);
}

static void prefs_page_reconcile_class_init (PrefsPageReconcileClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_reconcile_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_reconcile.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, vbox_reconcile);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, button_collapse_row);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, button_reconcile_delete);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, button_reconcile_find);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, checkbutton_reconcile_sort);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, grid_reconcile);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, radiobutton_end_date_1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, radiobutton_end_date_2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcile, treeview_reconcile);
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
PrefsPageReconcile *prefs_page_reconcile_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_RECONCILE_TYPE, NULL);
}

/**
 * fill the reconcile list,
 * sort each reconcile in its account
 *
 * \param
 *
 * \return
 **/
void prefs_page_reconcile_fill (GtkWidget *tree_view)
{
    GtkTreeModel *model = NULL;
    GSList *tmp_list;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	if (!tree_view)
		return;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	if (!model)
		return;

    gtk_tree_store_clear (GTK_TREE_STORE (model));

    /* we make a tree_model containing the accounts,
     * and for each account, all the reconciles */
    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
		GtkTreeIter account_iter;
		GList *reconcile_list;
		GList *tmp_reconcile_list;
		gchar *tmp_name;
		gint account_number;

		account_number = gsb_data_account_get_no_account (tmp_list->data);
		tmp_name = utils_str_break_form_name_field (gsb_data_account_get_name (account_number),
													TRUNC_FORM_FIELD);

		gtk_tree_store_append (GTK_TREE_STORE (model), &account_iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&account_iter,
							RECONCILIATION_NAME_COLUMN, tmp_name,
					 		RECONCILIATION_WEIGHT_COLUMN, 600,
							RECONCILIATION_ACCOUNT_COLUMN, account_number,
							-1);
		g_free (tmp_name);

		/* for each account, get the concerned reconciles */
		reconcile_list = gsb_data_reconcile_get_sort_reconcile_list (account_number);
		if (w_etat->reconcile_sort)
			reconcile_list = g_list_reverse (reconcile_list);

		tmp_reconcile_list = reconcile_list;
		while (tmp_reconcile_list)
		{
			gint reconcile_number;

			reconcile_number = GPOINTER_TO_INT (tmp_reconcile_list->data);
			if (gsb_data_reconcile_get_account (reconcile_number) == account_number)
			{
				GtkTreeIter reconcile_iter;
				gchar *init_date, *final_date;
				gchar *init_balance, *final_balance;

				init_date = gsb_format_gdate (gsb_data_reconcile_get_init_date (reconcile_number));
				final_date = gsb_format_gdate (gsb_data_reconcile_get_final_date (reconcile_number));
				init_balance = utils_real_get_string (gsb_data_reconcile_get_init_balance (reconcile_number));
				final_balance = utils_real_get_string (gsb_data_reconcile_get_final_balance (reconcile_number));
				tmp_name = utils_str_break_form_name_field (gsb_data_reconcile_get_name (reconcile_number),
															TRUNC_FORM_FIELD);
				gtk_tree_store_append (GTK_TREE_STORE (model), &reconcile_iter, &account_iter);
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&reconcile_iter,
									RECONCILIATION_NAME_COLUMN, tmp_name,
							 		RECONCILIATION_WEIGHT_COLUMN, 400,
									RECONCILIATION_INIT_DATE_COLUMN, init_date,
									RECONCILIATION_FINAL_DATE_COLUMN, final_date,
									RECONCILIATION_INIT_BALANCE_COLUMN, init_balance,
									RECONCILIATION_FINAL_BALANCE_COLUMN, final_balance,
									RECONCILIATION_RECONCILE_COLUMN, reconcile_number,
									RECONCILIATION_ACCOUNT_COLUMN, account_number,
									-1);
				g_free (init_date);
				g_free (final_date);
				g_free (init_balance);
				g_free (final_balance);
				g_free (tmp_name);
			}
			tmp_reconcile_list = tmp_reconcile_list->next;
		}
		g_list_free (reconcile_list);

		tmp_list = tmp_list->next;
    }
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

