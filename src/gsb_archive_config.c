/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
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
 * \file gsb_archive_config.c
 * this is the config part for the archive
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_archive_config.h"
#include "gsb_account_property.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_archive.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_fyear.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "utils_dates.h"
#include "navigation.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "traitement_variables.h"
#include "utils.h"
#include "dialog.h"
#include "transaction_list.h"
#include "structures.h"
#include "gsb_dirs.h"
/*END_INCLUDE*/

/** Columns for payment methods tree */
enum archives_columns {
    ARCHIVES_NAME_COLUMN = 0,
    ARCHIVES_INIT_DATE,
    ARCHIVES_FINAL_DATE,
    ARCHIVES_FYEAR_NAME,
    ARCHIVES_REPORT_TITLE,
    ARCHIVES_NUMBER,
    NUM_ARCHIVES_COLUMNS,
};

static GtkWidget *archive_treeview = NULL;
static GtkWidget *archive_name_entry = NULL;
static gint archive_config_sort_order;           /* variable durée de vie session */

/*START_STATIC*/
static gboolean gsb_archive_config_delete_archive ( GtkWidget *button,
                        GtkWidget *tree_view );
static gboolean gsb_archive_config_destroy_archive ( GtkWidget *button,
                        GtkWidget *tree_view );
static void gsb_archive_config_fill_list ( GtkListStore *store );
static gboolean gsb_archive_config_name_changed ( GtkWidget *entry,
                        GtkWidget *tree_view );
static gboolean gsb_archive_config_select ( GtkTreeSelection *selection,
                        GtkWidget *paddingbox );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 * */
static void gsb_archive_config_list_sort_column_clicked (GtkTreeViewColumn *tree_view_column,
                                                    GtkTreeModel *model)
{
    if (archive_config_sort_order == GTK_SORT_ASCENDING)
        archive_config_sort_order = GTK_SORT_DESCENDING;
    else
        archive_config_sort_order = GTK_SORT_ASCENDING;
}

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/**
 * Creates the Archive config tab.
 *
 * \param
 *
 * \returns A newly allocated vbox
 */
GtkWidget *gsb_archive_config_create (void)
{
    GtkWidget *vbox_pref;
    GtkWidget *scrolled_window;
    GtkWidget *paddinggrid;
    GtkWidget *label;
    GtkListStore *archive_model;
    GtkWidget *modification_paddinggrid;
    GtkWidget *check_paddinggrid;
    GtkWidget *button;
    GtkWidget *entry;
    gint i;
	gchar* tmpstr;
    gchar *titles[] = {
	_("Name"), _("Initial date"), _("Final date"), _("Financial year"), _("Report name") };
    gfloat alignment[] = {
	COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER , COLUMN_CENTER, COLUMN_CENTER };

    /* create the page */
    vbox_pref = new_vbox_with_title_and_icon (_("Archives"), "archives.png");

    /* create the list of archives */
    paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, _("Known archives"));

    /* Create scrolled window */
    scrolled_window = utils_prefs_scrolled_window_new (NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_PG, 200);
    gtk_grid_attach (GTK_GRID (paddinggrid), scrolled_window, 0, 0, 1, 1);

    /* Create tree view */
    archive_model = gtk_list_store_new (NUM_ARCHIVES_COLUMNS,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_STRING,
                                        G_TYPE_INT);
    archive_treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (archive_model));
    g_object_unref (G_OBJECT(archive_model));
    gtk_container_add (GTK_CONTAINER (scrolled_window), archive_treeview);

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
        /* on trie sur la colonne ARCHIVES_FYEAR_NAME */
        if ( i == ARCHIVES_FYEAR_NAME)
        {
        gtk_tree_view_column_set_sort_column_id (column, i);
        g_signal_connect (G_OBJECT (column),
                              "clicked",
                              G_CALLBACK (gsb_archive_config_list_sort_column_clicked),
                              archive_model);
        }

        gtk_tree_view_append_column (GTK_TREE_VIEW(archive_treeview), column);
    }

    /* Sort columns accordingly */
    archive_config_sort_order = conf.prefs_sort_order;
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(archive_model),
                                          ARCHIVES_FYEAR_NAME,
                                          archive_config_sort_order);

    /* archive modification */
    modification_paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, ("Archive modification"));
    gtk_widget_set_sensitive (modification_paddinggrid, FALSE);
    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (archive_treeview)),
                      "changed",
                      G_CALLBACK (gsb_archive_config_select),
                      modification_paddinggrid);

    /* archive name (the only thing we can change) */
    label = gtk_label_new (_("Name: "));
    utils_labels_set_alignement (GTK_LABEL (label), 0, 1);
    gtk_grid_attach (GTK_GRID (modification_paddinggrid), label, 0, 0, 1, 1);
    archive_name_entry = gsb_autofunc_entry_new (NULL,
						  G_CALLBACK (gsb_archive_config_name_changed), archive_treeview,
						  G_CALLBACK (gsb_data_archive_set_name), 0);
    g_signal_connect (G_OBJECT (archive_name_entry),
                      "destroy",
                      G_CALLBACK (gtk_widget_destroyed),
                      &archive_name_entry);
    gtk_grid_attach (GTK_GRID (modification_paddinggrid), archive_name_entry, 1,0, 1, 1);

    /* button to delete an archive and free the transactions */
    button = gtk_button_new_with_label (_("Delete the archive"));
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    g_signal_connect (G_OBJECT (button),
                      "clicked",
                      G_CALLBACK (gsb_archive_config_delete_archive),
                      archive_treeview);
    tmpstr = g_build_filename (gsb_dirs_get_pixmaps_dir (), "import.png", NULL);
    gtk_button_set_image (GTK_BUTTON(button), gtk_image_new_from_file (tmpstr));
    g_free (tmpstr);
    gtk_grid_attach (GTK_GRID (modification_paddinggrid), button, 0, 1, 1, 1);

    /* button to delete an archive and delete the transactions */
    button = gtk_button_new_with_label (_("Delete the archive and transactions"));
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    g_signal_connect (G_OBJECT (button),
                      "clicked",
                      G_CALLBACK (gsb_archive_config_destroy_archive),
                      archive_treeview);
    tmpstr = g_build_filename (gsb_dirs_get_pixmaps_dir (), "import.png", NULL);
    gtk_button_set_image (GTK_BUTTON(button), gtk_image_new_from_file (tmpstr));
    g_free (tmpstr);

    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    gtk_grid_attach (GTK_GRID (modification_paddinggrid), button, 1, 1, 1, 1);

    /* check part at the opening of the file */
    check_paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, _("Automatic check"));

    button = gsb_automem_checkbutton_new (_("Check at opening if creating archive is needed."),
                                          &conf.check_for_archival,
                                          NULL,
                                          NULL);

    label = gtk_label_new (_("Warn if more than "));
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    gtk_grid_attach (GTK_GRID (check_paddinggrid), label, 0, 1, 1, 1);

    entry = gsb_automem_spin_button_new (&conf.max_non_archived_transactions_for_check, NULL, NULL);
    utils_widget_set_padding (entry, MARGIN_START,MARGIN_TOP);
    gtk_widget_set_size_request (entry, 100, -1);
    gtk_grid_attach (GTK_GRID (check_paddinggrid), entry, 1, 1, 1, 1);

    label = gtk_label_new (_(" transactions are not archived."));
    gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
    gtk_grid_attach (GTK_GRID (check_paddinggrid), label, 2, 1, 1, 1);

    gtk_grid_attach (GTK_GRID (check_paddinggrid), button, 0, 0, 3, 1);

    /* fill the list */
    gsb_archive_config_fill_list (archive_model);

    gtk_widget_show_all (vbox_pref);

    /** Do not set this tab sensitive is no account file is opened. */
    if (!gsb_data_account_get_accounts_amount ())
        gtk_widget_set_sensitive (vbox_pref, FALSE);

    return (vbox_pref);
}

/**
 * fill the tree view with the existings archives
 *
 * \param store
 *
 * \return
 * */
static void gsb_archive_config_fill_list ( GtkListStore *store )
{
    GSList *tmp_list;

    gtk_list_store_clear (GTK_LIST_STORE (store));
    tmp_list = gsb_data_archive_get_archives_list ();

    while ( tmp_list )
    {
	gint archive_number;
	GtkTreeIter iter;
	gchar *init_date;
	gchar *final_date;

	archive_number = gsb_data_archive_get_no_archive (tmp_list -> data);

	init_date = gsb_format_gdate (gsb_data_archive_get_beginning_date (archive_number));
	final_date = gsb_format_gdate (gsb_data_archive_get_end_date (archive_number));

	gtk_list_store_append ( GTK_LIST_STORE (store),
				&iter );
	gtk_list_store_set ( GTK_LIST_STORE (store),
			     &iter,
			     ARCHIVES_NAME_COLUMN, gsb_data_archive_get_name (archive_number),
			     ARCHIVES_INIT_DATE, init_date,
			     ARCHIVES_FINAL_DATE, final_date,
			     ARCHIVES_FYEAR_NAME, gsb_data_fyear_get_name (gsb_data_archive_get_fyear (archive_number)),
			     ARCHIVES_REPORT_TITLE, gsb_data_archive_get_report_title (archive_number),
			     ARCHIVES_NUMBER, archive_number,
			     -1 );

	if (init_date)
	    g_free (init_date);
	if (final_date)
	    g_free (final_date);

	tmp_list = tmp_list -> next;
    }
}




/**
 * Callback used when an archive is selected
 *
 * \param selection the tree selection
 * \param paddingbox the modification paddingbox
 *
 * \return FALSE
 */
static gboolean gsb_archive_config_select ( GtkTreeSelection *selection,
                        GtkWidget *paddingbox )
{
    GtkTreeIter iter;
    gboolean good;

    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    if (good)
    {
	GtkTreeModel *model;
	gint archive_number;

	/* sensitive the paddingbox */
	gtk_widget_set_sensitive ( paddingbox,
				   TRUE );

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (archive_treeview));
	gtk_tree_model_get (model, &iter,
			    ARCHIVES_NUMBER, &archive_number,
			    -1 );

	/* Filling entries */
	gsb_autofunc_entry_set_value ( archive_name_entry,
				       gsb_data_archive_get_name (archive_number),
				       archive_number );
    }
    else
    {
	/* unsensitive the paddingbox */
	gtk_widget_set_sensitive ( paddingbox,
				   FALSE );
	/* Blanking entries */
	gsb_autofunc_entry_set_value ( archive_name_entry, NULL, 0);
    }
    return ( FALSE );
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
static gboolean gsb_archive_config_name_changed ( GtkWidget *entry,
                        GtkWidget *tree_view )
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

	model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     ARCHIVES_NUMBER, &archive_number,
			     -1 );

	/* update the tree view */
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			    ARCHIVES_NAME_COLUMN, gsb_data_archive_get_name (archive_number),
			    -1 );
    }
    return FALSE;
}


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
static gboolean gsb_archive_config_delete_archive ( GtkWidget *button,
                        GtkWidget *tree_view )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;
	gchar* tmpstr;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
    GtkTreeModel *model;
    gint archive_number;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
    gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
                        ARCHIVES_NUMBER, &archive_number,
                        -1 );

    if (!archive_number)
        return FALSE;

    tmpstr = g_strdup_printf (
                        _("Warning, you are about the delete the archive \"%s\".\n\n"
                          "If you continue, all the transactions linked to that archive "
                          "will loose the link and will begin again not archived.\n"
                          "All the information about that archive will be destroyed.\n\n"
                          "Do you want to continue?"),
                        gsb_data_archive_get_name (archive_number) );
    if (!question_yes_no (tmpstr, _("Deleting an archive"), GTK_RESPONSE_CANCEL ))
    {
        g_free ( tmpstr );
        return FALSE;
    }
        g_free ( tmpstr );

    /* ok, now we delete the archive */
    /* first step, we show it in the lists */
    gsb_transactions_list_restore_archive (archive_number, FALSE);

    /* now we remove the link of all the transactions and the archive itself */
    gsb_data_archive_remove (archive_number);

    /* remove from the list */
    gtk_list_store_remove ( GTK_LIST_STORE (model), &iter );
        gsb_file_set_modified ( TRUE );
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
static gboolean gsb_archive_config_destroy_archive ( GtkWidget *button,
                        GtkWidget *tree_view )
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
	gchar* tmpstr;

	model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     ARCHIVES_NUMBER, &archive_number,
			     -1 );

	if (!archive_number)
	    return FALSE;

	tmpstr = g_strdup_printf (
                        _("Warning, you are about the delete the archive \"%s\" and its "
                          "associated transactions.\n\nIf you continue, all the transactions "
                          "linked to that archive will be deleted and the initials amounts "
                          "of the accounts will be adjusted.\nAll the information about "
                          "that archive will be destroyed.\nYou should have at least exported "
                          "that archive into another file...\n\nAre you sure you want to "
                          "continue ?"),
                        gsb_data_archive_get_name (archive_number) );
	if (!question_yes_no (tmpstr, _("Deleting an archive and its transactions"), GTK_RESPONSE_CANCEL ) )
	{
	    g_free ( tmpstr );
	    return FALSE;
	}
	g_free ( tmpstr );

	/* remove the lines of that archive in the tree view of transactions */
	transaction_list_remove_archive (archive_number);

	/* remove that archive from the archive store and modify the initial amount of accounts */
	tmp_list = gsb_data_archive_store_get_archives_list ();
	while (tmp_list)
	{
	    gint archive_store_number;

	    archive_store_number = gsb_data_archive_store_get_number (tmp_list -> data);
	    if (gsb_data_archive_store_get_archive_number (archive_store_number) == archive_number)
	    {
		/* change the initial amount of the corresponding account */
		account_number = gsb_data_archive_store_get_account_number (archive_store_number);
		gsb_data_account_set_init_balance ( account_number,
                        gsb_real_add ( gsb_data_account_get_init_balance (account_number, -1),
                        gsb_data_archive_store_get_balance (archive_store_number)));

		/* remove the archive store */
		tmp_list = tmp_list -> next;
		gsb_data_archive_store_remove (archive_store_number);
	    }
	    else
		tmp_list = tmp_list -> next;
	}

	/* delete the transactions associated to the archive */
	tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	while (tmp_list)
	{
	    gint transaction_number;

	    transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);
	    if (gsb_data_transaction_get_archive_number (transaction_number) == archive_number)
	    {
		/* we have to remove that transaction */
		gsb_transactions_list_delete_transaction_from_tree_view (transaction_number);

		/* need to go to the next tmp_list now, before deleting the transaction */
		tmp_list = tmp_list -> next;
		gsb_data_transaction_remove_transaction_without_check (transaction_number);
	    }
	    else
		tmp_list = tmp_list -> next;
	}
	/* now we remove the archive  */
	gsb_data_archive_remove (archive_number);

	/* remove from the list */
	gtk_list_store_remove ( GTK_LIST_STORE (model),
				&iter );

	/* if we are on an account, we re-calculate the balances and mark the new initial balance in the account */
	account_number = gsb_gui_navigation_get_current_account ();
	if (account_number >= 0 )
	{
	    /* we are on an account */
	    gsb_account_property_fill_page ();
	    transaction_list_set_balances ();
	}

        gsb_file_set_modified ( TRUE );
    }
    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 * */
void gsb_archive_config_set_sort_order (gpointer *sort_order)
{
    GtkTreeModel *model;

    archive_config_sort_order = GPOINTER_TO_INT (sort_order);
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (archive_treeview));
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model),
                                          ARCHIVES_FYEAR_NAME,
                                          archive_config_sort_order);
    gtk_tree_sortable_sort_column_changed (GTK_TREE_SORTABLE(model));
    gsb_archive_config_fill_list (GTK_LIST_STORE (model));
}

/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
