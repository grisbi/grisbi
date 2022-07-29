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
#include <stdlib.h>

/*START_INCLUDE*/
#include "prefs_page_reconcile_sort.h"
#include "gsb_data_account.h"
#include "gsb_data_payment.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageReconcileSortPrivate   PrefsPageReconcileSortPrivate;

struct _PrefsPageReconcileSortPrivate
{
	GtkWidget *			vbox_reconcile_sort;

	GtkWidget *			button_collapse_all;
    GtkWidget *			button_move_down;
	GtkWidget *			button_move_up;
    GtkWidget *			treeview_reconcile_sort;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageReconcileSort, prefs_page_reconcile_sort, GTK_TYPE_BOX)

enum ReconciliationSortColumns {
    RECONCILIATION_SORT_NAME_COLUMN = 0,
    RECONCILIATION_SORT_VISIBLE_COLUMN,
    RECONCILIATION_SORT_SORT_COLUMN,
    RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN,
    RECONCILIATION_SORT_ACCOUNT_COLUMN,
    RECONCILIATION_SORT_TYPE_COLUMN,
    RECONCILIATION_SORT_SENSITIVE_COLUMN,
    NUM_RECONCILIATION_SORT_COLUMNS
};

/* variable servant la gestion du bouton "Collapse all" */
static gint expand_number = 0;

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * referme la ligne selectionnee du tree_view
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_sort_button_collapse_row_clicked (GtkButton *button,
																   GtkWidget *tree_view)
{
	gtk_tree_view_collapse_all (GTK_TREE_VIEW (tree_view));
	expand_number = 0;
}

/**
 * clear and fill the reconciliation tree with the accounts,
 * and for each account, set the method of payments in the good order
 *
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_sort_model_fill (GtkWidget *tree_view)
{
    GtkTreeIter account_iter;
    GtkTreeIter payment_method_iter;
    GtkTreeModel *model;
    GSList *list_tmp;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_tree_store_clear (GTK_TREE_STORE (model));

	list_tmp = gsb_data_account_get_list_accounts ();
    while (list_tmp)
    {
		gint account_number;
		GSList *sorted_list;
		GSList *payment_list;
		gchar *tmp_name;
		gboolean visible;

		account_number = gsb_data_account_get_no_account (list_tmp->data);

		/* when no method of payment, hide the checkbuttons because non sense */
		payment_list = gsb_data_payment_get_list_for_account (account_number);
		if (payment_list)
		{
			visible = TRUE;
			g_slist_free (payment_list);
		}
		else
			visible = FALSE;

		tmp_name = utils_str_break_form_name_field (gsb_data_account_get_name (account_number), TRUNC_FORM_FIELD);
		gtk_tree_store_append (GTK_TREE_STORE (model), &account_iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&account_iter,
							RECONCILIATION_SORT_NAME_COLUMN, tmp_name,
							RECONCILIATION_SORT_VISIBLE_COLUMN, visible,
							RECONCILIATION_SORT_SORT_COLUMN, gsb_data_account_get_reconcile_sort_type (account_number),
							RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, gsb_data_account_get_split_neutral_payment
							(account_number),
							RECONCILIATION_SORT_ACCOUNT_COLUMN, account_number,
							RECONCILIATION_SORT_TYPE_COLUMN, 0,
							RECONCILIATION_SORT_SENSITIVE_COLUMN, visible,
							-1);
		g_free (tmp_name);

		/* the sorted list is a list of numbers of method of payment, in the good order */
		sorted_list = gsb_data_account_get_sort_list (account_number);

		while (sorted_list)
		{
			gint payment_number;

			/* in the sorted list, a number can be negative, when split a neutral into 2 parts
			 * so payment number here can be negative for neutral payments */
			payment_number = GPOINTER_TO_INT (sorted_list->data);

			if (payment_number)
			{
				gchar *name = NULL;

				gtk_tree_store_append (GTK_TREE_STORE (model), &payment_method_iter, &account_iter);

				/* if split the neutrals, show here with the name */
				switch (gsb_data_payment_get_sign (abs (payment_number)))
				{
					case GSB_PAYMENT_DEBIT:
					case GSB_PAYMENT_CREDIT:
						name = my_strdup (gsb_data_payment_get_name (payment_number));
						break;

					case GSB_PAYMENT_NEUTRAL:
						if (gsb_data_account_get_split_neutral_payment (account_number))
						{
							/* the neutrals are splitted */
							if (payment_number < 0)
								name = g_strconcat (gsb_data_payment_get_name (-payment_number), " (-)", NULL);
							else
								name = g_strconcat (gsb_data_payment_get_name (payment_number), " (+)", NULL);
						}
						else
							name = my_strdup (gsb_data_payment_get_name (payment_number));
						break;
				}
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&payment_method_iter,
									RECONCILIATION_SORT_NAME_COLUMN, name,
									RECONCILIATION_SORT_VISIBLE_COLUMN, FALSE,
									RECONCILIATION_SORT_SORT_COLUMN, FALSE,
									RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, FALSE,
									RECONCILIATION_SORT_ACCOUNT_COLUMN, account_number,
									RECONCILIATION_SORT_TYPE_COLUMN, payment_number,
									RECONCILIATION_SORT_SENSITIVE_COLUMN, TRUE,
									-1);
				if (name)
					g_free (name);
			}
			sorted_list = sorted_list->next;
		}

		if (gtk_tree_model_iter_has_child (GTK_TREE_MODEL (model), &account_iter)
			&&
			gsb_data_account_get_reconcile_sort_type (account_number))
		{
			GtkTreePath *path;

			path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &account_iter);
			if (path)
			{
				gtk_tree_view_expand_row (GTK_TREE_VIEW (tree_view), path, TRUE);
				gtk_tree_path_free (path);
			}
		}
		list_tmp = list_tmp->next;
    }
}

/**
 * Callback called when toggle the check button to split the neutral method of payments
 * or to separate reconcile between debit credit
 *
 * \param cell
 * \param path_str
 * \param tree_view
 *
 * \return FALSE
 **/
static void prefs_page_reconcile_sort_neutral_toggled (GtkCellRendererToggle *cell,
													   gchar *path_str,
													   GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path;
    GSList *sorted_list_copy;
    GSList *tmp_list;
    gint account_number;
    gboolean toggle;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    /* invert the toggle */
    path = gtk_tree_path_new_from_string (path_str);
    gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);

    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter,
						RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, &toggle,
						RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
						-1);
    toggle ^= 1;
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, toggle, -1);

    /* and save it */
    gsb_data_account_set_split_neutral_payment (account_number, toggle);

    /* need to copy the sorted_list to avoid an infinite loop when add a negative payment neutral later */
    sorted_list_copy = g_slist_copy (gsb_data_account_get_sort_list (account_number));
    tmp_list = sorted_list_copy;

    while (tmp_list)
    {
		gint payment_number;

		payment_number = GPOINTER_TO_INT (tmp_list->data);

		/* payment_number can be negative, so do for it abs */
		if (gsb_data_payment_get_sign (abs (payment_number)) == GSB_PAYMENT_NEUTRAL)
		{
			if (toggle)
			{
				gsb_data_account_sort_list_add (account_number, -payment_number);
			}
			else
			{
				if (payment_number < 0)
					gsb_data_account_sort_list_remove (account_number, payment_number);
			}
		}
		tmp_list = tmp_list->next;
    }
    prefs_page_reconcile_sort_model_fill (tree_view);
	gtk_tree_view_expand_row (GTK_TREE_VIEW (tree_view), path, FALSE);

    g_slist_free (sorted_list_copy);
	gtk_tree_path_free (path);

	gsb_file_set_modified (TRUE);
}

/**
 * Callback called when toggle the check button to sort by method of payment
 *
 * \param cell
 * \param path_str
 * \param tree_view
 *
 * \return FALSE
 **/
static void prefs_page_reconcile_sort_payment_toggled (GtkCellRendererToggle *cell,
													   gchar *path_str,
													   GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreePath * path;
    GtkTreeIter iter;
    gint account_number;
    gboolean toggle;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    path = gtk_tree_path_new_from_string (path_str);
    gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);

    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter,
						RECONCILIATION_SORT_SORT_COLUMN, &toggle,
						RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
						-1);
    toggle ^= 1;

    /* set new value */
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, RECONCILIATION_SORT_SORT_COLUMN, toggle, -1);

    /* Set to 1 (sort by types) if toggle is selected */
    gsb_data_account_set_reconcile_sort_type (account_number, toggle);

    if (toggle)
		gtk_tree_view_expand_row (GTK_TREE_VIEW(tree_view), path, FALSE);
    else
		gtk_tree_view_collapse_row (GTK_TREE_VIEW(tree_view), path);

    gtk_tree_path_free (path);
    gsb_file_set_modified (TRUE);
}

/**
 * callback called when select a line in the tree
 * show/hide the buttons up and down to move the method of payment
 *
 * \param selection 	the GtkTreeSelection
 * \param page
 *
 * \return FALSE
 */
static void prefs_page_reconcile_sort_row_selected (GtkTreeSelection *selection,
													PrefsPageReconcileSort *page)
{
	GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean good;
	PrefsPageReconcileSortPrivate *priv;

	priv = prefs_page_reconcile_sort_get_instance_private (page);

    good = gtk_tree_selection_get_selected (selection, &model, &iter);
    if (good)
    {
		gint payment_method;

		gtk_tree_model_get (model, &iter, RECONCILIATION_SORT_TYPE_COLUMN, &payment_method, -1);

		if (payment_method)
		{
			GtkTreePath *path;

			/* Is there something before? */
			path = gtk_tree_model_get_path (GTK_TREE_MODEL(model), &iter);
			gtk_widget_set_sensitive (priv->button_move_up, gtk_tree_path_prev(path));
			gtk_tree_path_free (path);

			/* Is there something after? */
			gtk_widget_set_sensitive (priv->button_move_down, gtk_tree_model_iter_next (model, &iter));
		}
		else
		{
			gtk_widget_set_sensitive (priv->button_move_up, FALSE);
			gtk_widget_set_sensitive (priv->button_move_down, FALSE);
		}
    }
}

/**
 * update the sort list of an account according to the model
 * called when change something in the order of method of payment
 *
 * \param account_number
 * \param model
 *
 * \return
 **/
static void prefs_page_reconcile_sort_update_account_sort_list (gint account_number,
																GtkTreeModel *model)
{
    GtkTreeIter iter;

    gsb_data_account_sort_list_free (account_number);

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
		/* first, we try to find the account line */
		do
		{
			gint account_number_tmp;

			gtk_tree_model_get (GTK_TREE_MODEL(model),
								&iter,
								RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number_tmp,
								-1);
			if (account_number_tmp == account_number)
			{
				/* ok, we are on the good account,
				 * now we append the method of payment in the same order as the list */
				GtkTreeIter iter_child;

				if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &iter_child, &iter))
				{
					do
					{
						gint payment_number;

						gtk_tree_model_get (GTK_TREE_MODEL(model),
											&iter_child,
											RECONCILIATION_SORT_TYPE_COLUMN, &payment_number,
											-1);
						gsb_data_account_sort_list_add (account_number, payment_number);
					}
					while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_child));
				}
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter)
			   &&
			   !gsb_data_account_get_sort_list (account_number));
    }
}

/**
 * Callback called by a click on the up arrow
 * to move a payment method up
 *
 * \param button
 * \param page
 *
 * \return
 **/
static void prefs_page_reconcile_sort_move_down (GtkWidget *button,
												 PrefsPageReconcileSort *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeIter other;
    GtkTreePath *path;
    GtkTreeSelection *selection;
    gint account_number = -1;
    gint payment_number;
    gboolean good, visible = 0;
	PrefsPageReconcileSortPrivate *priv;

	priv = prefs_page_reconcile_sort_get_instance_private (page);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_reconcile_sort));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_reconcile_sort));

    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    if (good)
		gtk_tree_model_get (GTK_TREE_MODEL(model),
							&iter,
							RECONCILIATION_SORT_VISIBLE_COLUMN, &visible,
							RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
							RECONCILIATION_SORT_TYPE_COLUMN, &payment_number,
					 		-1);

    if (good && ! visible)
    {
		path = gtk_tree_model_get_path (GTK_TREE_MODEL(model), &iter);

		gtk_tree_path_next (path) ;
		if (gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &other, path))
		{
			gtk_tree_store_move_after (GTK_TREE_STORE(model), &iter, &other);
		}
    }

    prefs_page_reconcile_sort_row_selected (selection, page);
    prefs_page_reconcile_sort_update_account_sort_list (account_number, model);
}

/**
 * Callback called by a click on the up arrow
 * to move a payment method up
 *
 * \param button
 * \param page
 *
 * \return
 **/
static void prefs_page_reconcile_sort_move_up (GtkWidget *button,
											   PrefsPageReconcileSort *page)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeIter other;
    GtkTreePath *path;
    GtkTreeSelection *selection;
    gint account_number = -1;
    gint payment_number;
    gboolean good, visible = 0;
	PrefsPageReconcileSortPrivate *priv;

	priv = prefs_page_reconcile_sort_get_instance_private (page);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_reconcile_sort));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_reconcile_sort));


    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    if (good)
		gtk_tree_model_get (GTK_TREE_MODEL(model),
							&iter,
							RECONCILIATION_SORT_VISIBLE_COLUMN, &visible,
							RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
							RECONCILIATION_SORT_TYPE_COLUMN, &payment_number,
					 		-1);

    if (good && ! visible)
    {
		path = gtk_tree_model_get_path (GTK_TREE_MODEL(model), &iter);

		if (gtk_tree_path_prev (path)
			&&
			gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &other, path))
		{
			gtk_tree_store_move_before (GTK_TREE_STORE(model), &iter, &other);
		}
    }

    prefs_page_reconcile_sort_row_selected (selection, page);
    prefs_page_reconcile_sort_update_account_sort_list (account_number, model);
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
static void prefs_page_reconcile_sort_treeview_row_collapsed (GtkTreeView *tree_view,
														 GtkTreeIter *iter,
														 GtkTreePath *path,
														 GtkWidget *button)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (tree_view);
	gtk_tree_selection_select_iter (selection, iter);

	expand_number--;
	if (expand_number == 0)
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
static void prefs_page_reconcile_sort_treeview_row_expanded (GtkTreeView *tree_view,
															 GtkTreeIter *iter,
															 GtkTreePath *path,
															 GtkWidget *button)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (tree_view);
	gtk_tree_selection_select_iter (selection, iter);

	if (expand_number == 0)
		gtk_widget_set_sensitive (button, TRUE);
	expand_number++;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_sort_setup_button_collapse_all (GtkWidget *button)
{
	GtkWidget *image;
    gchar *filename;

    filename = g_build_filename (gsb_dirs_get_pixmaps_dir (), "gsb-up-16.png", NULL);
	image = gtk_image_new_from_file (filename);
	g_free (filename);

	gtk_button_set_image (GTK_BUTTON (button), image);
	gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
	gtk_widget_set_sensitive (button, FALSE);
	expand_number = 0;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_sort_setup_tree_view (PrefsPageReconcileSort *page)
{
	GtkTreeStore *store = NULL;
	GtkTreePath *path;
	GtkTreeSelection *selection;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
	PrefsPageReconcileSortPrivate *priv;

	priv = prefs_page_reconcile_sort_get_instance_private (page);

	/* create the model */
    store = gtk_tree_store_new (NUM_RECONCILIATION_SORT_COLUMNS,
								G_TYPE_STRING,			/* Name */
								G_TYPE_BOOLEAN,			/* visible pour cacher les cases à cocher */
								G_TYPE_BOOLEAN,			/* Sort by method of payment */
								G_TYPE_BOOLEAN,			/* Split neutrals */
								G_TYPE_INT,				/* Account number */
								G_TYPE_INT,				/* Payment number */
								G_TYPE_BOOLEAN);		/* sensitive pour les comptes sans moyen de paiement */


    /* Create tree tree_view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_reconcile_sort), GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));

    /* Name */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_pack_end (column, cell, TRUE);
    gtk_tree_view_column_set_title (column, _("Payment method"));
    gtk_tree_view_column_set_attributes (column,
										 cell,
										 "text", RECONCILIATION_SORT_NAME_COLUMN,
										 "sensitive", RECONCILIATION_SORT_SENSITIVE_COLUMN,
										 NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_reconcile_sort), column);

    /* Sort by date */
    cell = gtk_cell_renderer_toggle_new ();
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_alignment (column, 0.5);
    gtk_tree_view_column_pack_end (column, cell, TRUE);
    gtk_tree_view_column_set_title (column, _("Sort by payment method"));
    gtk_tree_view_column_set_attributes (column,
										 cell,
										 "active", RECONCILIATION_SORT_SORT_COLUMN,
										 "activatable", RECONCILIATION_SORT_VISIBLE_COLUMN,
										 "visible", RECONCILIATION_SORT_VISIBLE_COLUMN,
										 NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(priv->treeview_reconcile_sort), column);
	g_signal_connect (cell,
					  "toggled",
					  G_CALLBACK (prefs_page_reconcile_sort_payment_toggled),
					  priv->treeview_reconcile_sort);

    /* Split neutral payment methods */
    cell = gtk_cell_renderer_toggle_new ();
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_alignment (column, 0.5);
    gtk_tree_view_column_pack_end (column, cell, TRUE);
    gtk_tree_view_column_set_title (column, _("Split neutral payment methods"));
    gtk_tree_view_column_set_attributes (column,
										 cell,
										 "active", RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN,
										 "activatable", RECONCILIATION_SORT_VISIBLE_COLUMN,
										 "visible", RECONCILIATION_SORT_VISIBLE_COLUMN,
										 NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_append_column (GTK_TREE_VIEW(priv->treeview_reconcile_sort), column);
    g_signal_connect (cell,
					  "toggled",
					  G_CALLBACK (prefs_page_reconcile_sort_neutral_toggled),
					  priv->treeview_reconcile_sort);

	/* fill the model */
	prefs_page_reconcile_sort_model_fill (priv->treeview_reconcile_sort);

	/* select the first item */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_reconcile_sort));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	path = gtk_tree_path_new_from_indices (0, -1);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_path_free (path);

	/* set signals */
    g_signal_connect (selection,
                      "changed",
                      G_CALLBACK (prefs_page_reconcile_sort_row_selected),
                      page);
}

/**
 * create the config widget for the reconcile
 * to choose the order of the method of payment to sort the list
 * while reconciling
 *
 * \param
 *
 * \return
 **/
static void prefs_page_reconcile_sort_setup_page (PrefsPageReconcileSort *page)
{
	GtkWidget *head_page;
	PrefsPageReconcileSortPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_reconcile_sort_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Sort for reconciliation"), "gsb-reconciliation-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_reconcile_sort), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_reconcile_sort), head_page, 0);

	/* set the list of reconcile sort */
	prefs_page_reconcile_sort_setup_tree_view (page);

	/* set button_collapse_all */
	 prefs_page_reconcile_sort_setup_button_collapse_all (priv->button_collapse_all);

	 /* Connect signal */
    g_signal_connect (G_OBJECT (priv->treeview_reconcile_sort),
                      "row-expanded",
                      G_CALLBACK (prefs_page_reconcile_sort_treeview_row_expanded),
                      priv->button_collapse_all);

    g_signal_connect (G_OBJECT (priv->treeview_reconcile_sort),
                      "row-collapsed",
                      G_CALLBACK (prefs_page_reconcile_sort_treeview_row_collapsed),
                      priv->button_collapse_all);

	g_signal_connect (G_OBJECT (priv->button_collapse_all),
					  "clicked",
					  G_CALLBACK (prefs_page_reconcile_sort_button_collapse_row_clicked),
					  priv->treeview_reconcile_sort);

    g_signal_connect (G_OBJECT (priv->button_move_down),
                      "clicked",
                      G_CALLBACK (prefs_page_reconcile_sort_move_down),
                      priv->treeview_reconcile_sort);

	g_signal_connect (G_OBJECT (priv->button_move_up),
                      "clicked",
                      G_CALLBACK (prefs_page_reconcile_sort_move_up),
                      priv->treeview_reconcile_sort);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_reconcile_sort_init (PrefsPageReconcileSort *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_reconcile_sort_setup_page (page);
}

static void prefs_page_reconcile_sort_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_reconcile_sort_parent_class)->dispose (object);
}

static void prefs_page_reconcile_sort_class_init (PrefsPageReconcileSortClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_reconcile_sort_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_reconcile_sort.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcileSort, vbox_reconcile_sort);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcileSort, button_collapse_all);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcileSort, button_move_down);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcileSort, button_move_up);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageReconcileSort, treeview_reconcile_sort);
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
PrefsPageReconcileSort *prefs_page_reconcile_sort_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_RECONCILE_SORT_TYPE, NULL);
}

/**
 * Fonction d'interface avec l'onglet payment_method_config
 *
 * \param
 *
 * \return
 **/
void prefs_page_reconcile_sort_fill (void)
{
	GtkWidget *page;

	devel_debug (NULL);
	page = grisbi_prefs_get_child_by_page_name ("reconcile_sort_page_num");
	if (page && PREFS_IS_PAGE_RECONCILE_SORT (page))
	{
		PrefsPageReconcileSortPrivate *priv;

		priv = prefs_page_reconcile_sort_get_instance_private (PREFS_PAGE_RECONCILE_SORT (page));

		/* fill the model */
		prefs_page_reconcile_sort_model_fill (priv->treeview_reconcile_sort);
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

