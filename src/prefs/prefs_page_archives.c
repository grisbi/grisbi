/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "prefs_page_archives.h"
#include "dialog.h"
#include "grisbi_settings.h"
#include "gsb_account_property.h"
#include "gsb_autofunc.h"
#include "gsb_data_account.h"
#include "gsb_data_archive.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_fyear.h"
#include "gsb_data_transaction.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "transaction_list.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageArchivesPrivate   PrefsPageArchivesPrivate;

struct _PrefsPageArchivesPrivate
{
	GtkWidget *			vbox_archives;

	GtkWidget *			treeview_archives;
    GtkWidget *			checkbutton_archives_sort_order;
	GtkWidget *			eventbox_archives_sort_order;

	GtkWidget *			grid_archives_modification;
	GtkWidget *			entry_archives_name;
	GtkWidget * 		button_archives_delete;
	GtkWidget * 		button_archives_destroy;

	GtkWidget *			checkbutton_archives_check_auto;
	GtkWidget *			eventbox_archives_check_auto;
    GtkWidget *         spinbutton_archives_check_auto;

};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageArchives, prefs_page_archives, GTK_TYPE_BOX)

/** Columns for archives tree */
enum archives_columns {
    ARCHIVES_NAME_COLUMN = 0,
    ARCHIVES_INIT_DATE,
    ARCHIVES_FINAL_DATE,
    ARCHIVES_FYEAR_NAME,
    ARCHIVES_REPORT_TITLE,
    ARCHIVES_NUMBER,
    NUM_ARCHIVES_COLUMNS,
};


/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * call by a click on the delete archive
 * this will delete the archive and free the transactions from the link
 * 	to that archive
 * nothing will be deleted except the archive
 *
 * \param button
 * \param tree_view
 *
 * \return FALSE
 * */
static gboolean prefs_page_archives_delete_archive (GtkWidget *button,
													GtkWidget *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkTreeModel *model;
		gchar* tmp_str;
		gint archive_number;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
							ARCHIVES_NUMBER, &archive_number,
							-1);

		if (!archive_number)
		{
			return FALSE;
		}

		tmp_str = g_strdup_printf (
							_("Warning, you are about the delete the archive \"%s\".\n\n"
							  "If you continue, all the transactions linked to that archive "
							  "will loose the link and will begin again not archived.\n"
							  "All the information about that archive will be destroyed.\n\n"
							  "Do you want to continue?"),
							gsb_data_archive_get_name (archive_number));
		if (!question_yes_no (tmp_str, _("Deleting an archive"), GTK_RESPONSE_CANCEL))
		{
			g_free (tmp_str);
			return FALSE;
		}
			g_free (tmp_str);

		/* ok, now we delete the archive */
		/* first step, we show it in the lists */
		gsb_transactions_list_restore_archive (archive_number, FALSE);

		/* now we remove the link of all the transactions and the archive itself */
		gsb_data_archive_remove (archive_number);

		/* remove from the list */
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		gtk_tree_selection_select_path (selection, gtk_tree_path_new_first ());

		gsb_file_set_modified (TRUE);
    }
    return FALSE;
}

/**
 * call by a click on the delete archive and transactions
 * this will delete the archive and the associated transactions
 * the initials amount of the accounts will be changed according to the new initial amount
 *
 * \param button
 * \param tree_view
 *
 * \return FALSE
 * */
static gboolean prefs_page_archives_destroy_archive (GtkWidget *button,
													 GtkWidget *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;
    GtkTreeModel *model;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		gint archive_number;
		GSList *tmp_list;
		gint account_number;
		gchar* tmp_str;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
							ARCHIVES_NUMBER, &archive_number,
							-1);

		if (!archive_number)
		{
			return FALSE;
		}

		tmp_str = g_strdup_printf (_("Warning, you are about the delete the archive \"%s\" and its "
									  "associated transactions.\n\nIf you continue, all the transactions "
									  "linked to that archive will be deleted and the initials amounts "
									  "of the accounts will be adjusted.\nAll the information about "
									  "that archive will be destroyed.\nYou should have at least exported "
									  "that archive into another file...\n\nAre you sure you want to "
									  "continue ?"),
								   gsb_data_archive_get_name (archive_number));

		if (!question_yes_no (tmp_str, _("Deleting an archive and its transactions"), GTK_RESPONSE_CANCEL))
		{
			g_free (tmp_str);
			return FALSE;
		}
		g_free (tmp_str);

		/* remove the lines of that archive in the tree view of transactions */
		transaction_list_remove_archive (archive_number);

		/* remove that archive from the archive store and modify the initial amount of accounts */
		tmp_list = gsb_data_archive_store_get_archives_list ();
		while (tmp_list)
		{
			gint archive_store_number;

			archive_store_number = gsb_data_archive_store_get_number (tmp_list->data);
			if (gsb_data_archive_store_get_archive_number (archive_store_number) == archive_number)
			{
				/* change the initial amount of the corresponding account */
				account_number = gsb_data_archive_store_get_account_number (archive_store_number);
				gsb_data_account_set_init_balance (account_number,
												   gsb_real_add (gsb_data_account_get_init_balance (account_number, -1),
																 gsb_data_archive_store_get_balance (archive_store_number)));

				/* remove the archive store */
				tmp_list = tmp_list->next;
				gsb_data_archive_store_remove (archive_store_number);
			}
			else
			{
				tmp_list = tmp_list->next;
			}
		}

		/* delete the transactions associated to the archive */
		tmp_list = gsb_data_transaction_get_complete_transactions_list ();
		while (tmp_list)
		{
			gint transaction_number;

			transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
			if (gsb_data_transaction_get_archive_number (transaction_number) == archive_number)
			{
				/* we have to remove that transaction */
				gsb_transactions_list_delete_transaction_from_tree_view (transaction_number);

				/* need to go to the next tmp_list now, before deleting the transaction */
				tmp_list = tmp_list->next;
				gsb_data_transaction_remove_transaction_without_check (transaction_number);
			}
			else
			{
				tmp_list = tmp_list->next;
			}
		}
		/* now we remove the archive  */
		gsb_data_archive_remove (archive_number);

		/* remove from the list */
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		gtk_tree_selection_select_path (selection, gtk_tree_path_new_first ());

		/* if we are on an account, we re-calculate the balances and mark the new initial balance in the account */
		account_number = gsb_gui_navigation_get_current_account ();
		if (account_number >= 0)
		{
			/* we are on an account */
			gsb_account_property_fill_page ();
			transaction_list_set_balances ();
		}

		gsb_file_set_modified (TRUE);
    }
    return FALSE;
}

/**
 * Callback used when an archive is selected
 *
 * \param selection the tree selection
 * \param paddingbox the modification paddingbox
 *
 * \return FALSE
 */
static gboolean prefs_page_archives_select_archive (GtkTreeSelection *selection,
													PrefsPageArchives *page)
{
    GtkTreeIter iter;
    gboolean good;
	PrefsPageArchivesPrivate *priv;

	priv = prefs_page_archives_get_instance_private (page);
	good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkTreeModel *model;
		gint archive_number;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_archives));
		gtk_tree_model_get (model, &iter,
							ARCHIVES_NUMBER, &archive_number,
							-1);

		/* Filling entries */
		gsb_autofunc_entry_set_value (priv->entry_archives_name,
									  gsb_data_archive_get_name (archive_number),
									  archive_number);
    }
    else
    {
		/* Blanking entries */
		gsb_autofunc_entry_set_value (priv->entry_archives_name, NULL, 0);
    }

    return (FALSE);
}

/**
 * Callback called when the payment method name is changed in the
 * GtkEntry associated.  It updates the GtkTreeView list of payment
 * methods as well as it updates transaction form.
 *
 * \param entry the entry changed (payment method name)
 * \param tree_view
 *
 * \return FALSE
 */
static gboolean prefs_page_archives_name_changed (GtkWidget *entry,
												  GtkWidget *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkTreeModel *model;
		gint archive_number;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, ARCHIVES_NUMBER, &archive_number, -1);

		/* update the tree view */
		gtk_list_store_set (GTK_LIST_STORE (model),
							&iter,
							ARCHIVES_NAME_COLUMN, gsb_data_archive_get_name (archive_number),
							-1);
    }
    return FALSE;
}

/**
 * fill the tree view with the existings archives
 *
 * \param store
 *
 * \return
 * */
static void prefs_page_archives_fill_list (GtkListStore *store)
{
    GSList *tmp_list;

    gtk_list_store_clear (GTK_LIST_STORE (store));
    tmp_list = gsb_data_archive_get_archives_list ();

    while (tmp_list)
    {
		gint archive_number;
		GtkTreeIter iter;
		gchar *init_date;
		gchar *final_date;

		archive_number = gsb_data_archive_get_no_archive (tmp_list->data);

		init_date = gsb_format_gdate (gsb_data_archive_get_beginning_date (archive_number));
		final_date = gsb_format_gdate (gsb_data_archive_get_end_date (archive_number));

		gtk_list_store_append (GTK_LIST_STORE (store), &iter);
		gtk_list_store_set (GTK_LIST_STORE (store),
							&iter,
							ARCHIVES_NAME_COLUMN, gsb_data_archive_get_name (archive_number),
							ARCHIVES_INIT_DATE, init_date,
							ARCHIVES_FINAL_DATE, final_date,
							ARCHIVES_FYEAR_NAME, gsb_data_fyear_get_name (gsb_data_archive_get_fyear (archive_number)),
							ARCHIVES_REPORT_TITLE, gsb_data_archive_get_report_title (archive_number),
							ARCHIVES_NUMBER, archive_number,
							-1);

		if (init_date)
			g_free (init_date);
		if (final_date)
			g_free (final_date);

		tmp_list = tmp_list->next;
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 * */
static void prefs_page_archives_button_sort_order_clicked (GtkWidget *toggle_button,
														   GtkWidget *treeview)
{
	GSettings *settings;
    GtkTreeModel *model;

	settings = grisbi_settings_get_settings (SETTINGS_PREFS);
	g_settings_set_int ( G_SETTINGS (settings),
                        "prefs-archives-sort-order",
                        conf.prefs_archives_sort_order);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
                                          ARCHIVES_FYEAR_NAME,
                                          conf.prefs_archives_sort_order);
    gtk_tree_sortable_sort_column_changed (GTK_TREE_SORTABLE (model));
    prefs_page_archives_fill_list (GTK_LIST_STORE (model));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_archives_setup_treeview_archives (PrefsPageArchives *page)
{
	PrefsPageArchivesPrivate *priv;
    GtkListStore *archives_model;
	GtkTreeSelection *selection;
    gint i;
    gchar *titles[] = {
	_("Name"), _("Initial date"), _("Final date"), _("Financial year"), _("Report name") };
    gfloat alignment[] = {
	COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER , COLUMN_CENTER, COLUMN_CENTER };

	devel_debug (NULL);

	priv = prefs_page_archives_get_instance_private (page);

	gtk_widget_set_name (priv->treeview_archives, "tree_view");

	/* Create tree model */
    archives_model = gtk_list_store_new (NUM_ARCHIVES_COLUMNS,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_INT);
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_archives), GTK_TREE_MODEL (archives_model));
    g_object_unref (G_OBJECT(archives_model));

    /* set the columns */
    for (i=0 ; i<5 ; i++)
    {
        GtkTreeViewColumn *column;
        GtkCellRenderer *cell;

        cell = gtk_cell_renderer_text_new ();
        g_object_set (G_OBJECT (cell), "xalign", alignment[i], NULL);

        column = gtk_tree_view_column_new ();
        gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
        gtk_tree_view_column_set_alignment (column, alignment[i]);

        gtk_tree_view_column_pack_start (column, cell, TRUE);
        gtk_tree_view_column_set_title (column, titles[i]);
        gtk_tree_view_column_set_attributes (column, cell, "text", i, NULL);
        gtk_tree_view_column_set_expand (column, TRUE);
        gtk_tree_view_column_set_resizable (column, TRUE);
        gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_archives), column);
    }

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (archives_model),
                                          ARCHIVES_FYEAR_NAME,
                                          conf.prefs_archives_sort_order);

	/* fill the list */
    prefs_page_archives_fill_list (archives_model);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_archives));
	g_signal_connect (selection,
                      "changed",
                      G_CALLBACK (prefs_page_archives_select_archive),
                      page);

	/* select the first item */
	gtk_tree_selection_select_path (selection, gtk_tree_path_new_first ());
}

/**
 * Création de la page de gestion des archives
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_archives_setup_archives_page (PrefsPageArchives *page)
{
	GtkWidget *head_page;
	gchar* tmp_str;
	gboolean is_loading;
	PrefsPageArchivesPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_archives_get_instance_private (page);
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Archives"), "gsb-archive-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_archives), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_archives), head_page, 0);

	/* set the entry widget entry_archives_name */
	priv->entry_archives_name = gsb_autofunc_entry_new (NULL,
												  G_CALLBACK (prefs_page_archives_name_changed),
												  priv->treeview_archives,
												  G_CALLBACK (gsb_data_archive_set_name),
												  0);

    gtk_grid_attach (GTK_GRID (priv->grid_archives_modification), priv->entry_archives_name, 1,0, 1, 1);

	/* setup treeview_archives */
	if (is_loading)
	{
		prefs_page_archives_setup_treeview_archives (page);
	}
	else
	{
		gtk_widget_set_sensitive (priv->treeview_archives, FALSE);
		gtk_widget_set_sensitive (priv->button_archives_delete, FALSE);
		gtk_widget_set_sensitive (priv->button_archives_destroy, FALSE);
		gtk_widget_set_sensitive (priv->entry_archives_name, FALSE);
	}

    /* set the checkbutton to sort archives */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_archives_sort_order),
								  conf.prefs_archives_sort_order);

    /* Connect signal */
    g_signal_connect (priv->eventbox_archives_sort_order,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_archives_sort_order);

    g_signal_connect (priv->checkbutton_archives_sort_order,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.prefs_archives_sort_order);

	g_signal_connect_after (priv->checkbutton_archives_sort_order,
							"toggled",
							G_CALLBACK (prefs_page_archives_button_sort_order_clicked),
							priv->treeview_archives);

    /* button to delete an archive and free the transactions */
    tmp_str = g_build_filename (gsb_dirs_get_pixmaps_dir (), "gsb-import-24.png", NULL);
    gtk_button_set_image (GTK_BUTTON (priv->button_archives_delete),
						  gtk_image_new_from_file (tmp_str));
    g_free (tmp_str);
    g_signal_connect (G_OBJECT (priv->button_archives_delete),
                      "clicked",
                      G_CALLBACK (prefs_page_archives_delete_archive),
                      priv->treeview_archives);

	/* button to delete an archive and delete the transactions */
    tmp_str = g_build_filename (gsb_dirs_get_pixmaps_dir (), "gsb-import-24.png", NULL);
    gtk_button_set_image (GTK_BUTTON (priv->button_archives_destroy),
						  gtk_image_new_from_file (tmp_str));
    g_free (tmp_str);
    g_signal_connect (G_OBJECT (priv->button_archives_destroy),
                      "clicked",
                      G_CALLBACK (prefs_page_archives_destroy_archive),
                      priv->treeview_archives);

	/* set the checkbutton for the automatic check */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_archives_check_auto),
								  conf.archives_check_auto);

	/* Connect signal */
    g_signal_connect (priv->eventbox_archives_check_auto,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_archives_check_auto);

    g_signal_connect (priv->checkbutton_archives_check_auto,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.archives_check_auto);

	/* set the max of transactions before archival and state */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_archives_check_auto),
							   conf.max_non_archived_transactions_for_check);

	if (!conf.archives_check_auto)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_archives_check_auto), FALSE);

    /* callback for spinbutton_ */
    g_object_set_data (G_OBJECT (priv->spinbutton_archives_check_auto),
                       "button", priv->checkbutton_archives_check_auto);
	g_object_set_data (G_OBJECT (priv->checkbutton_archives_check_auto),
                       "spinbutton", priv->spinbutton_archives_check_auto);

    g_signal_connect (priv->spinbutton_archives_check_auto,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &conf.max_non_archived_transactions_for_check);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_archives_init (PrefsPageArchives *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_archives_setup_archives_page (page);
}

static void prefs_page_archives_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_archives_parent_class)->dispose (object);
}

static void prefs_page_archives_class_init (PrefsPageArchivesClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_archives_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_archives.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, vbox_archives);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, treeview_archives);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, checkbutton_archives_sort_order);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, eventbox_archives_sort_order);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, grid_archives_modification);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, button_archives_delete);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, button_archives_destroy);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, checkbutton_archives_check_auto);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, eventbox_archives_check_auto);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageArchives, spinbutton_archives_check_auto);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageArchives * prefs_page_archives_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_ARCHIVES_TYPE, NULL);
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

