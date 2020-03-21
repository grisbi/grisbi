/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/* 			https://www.grisbi.org				                              */
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
 * \file gsb_reconcile_config.c
 * functions used to configure the reconcile
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_reconcile_config.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_assistant_reconcile_config.h"
#include "gsb_autofunc.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_reconcile.h"
#include "gsb_file.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_real.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_reconcile_config_delete ( GtkWidget *button,
				       GtkWidget *tree_view );
static gboolean gsb_reconcile_config_end_date_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gpointer data );
static gboolean gsb_reconcile_config_find_alone_transactions ( void );
static gboolean gsb_reconcile_config_select ( GtkTreeSelection *selection,
				       GtkWidget *table );
static gboolean gsb_reconcile_config_update_line ( GtkWidget *entry,
					    GtkWidget *tree_view );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/* the tree view used to modify the reconciles */
static GtkWidget *reconcile_treeview;

static GtkWidget *reconcile_name_entry;
static GtkWidget *reconcile_init_date_entry;
static GtkWidget *reconcile_final_date_entry;
static GtkWidget *reconcile_init_balance_entry;
static GtkWidget *reconcile_final_balance_entry;
static GtkWidget *delete_reconcile_button;

/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
 /**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void gsb_reconcile_button_collapse_row_clicked (GtkButton *button,
													   GtkTreeSelection *selection)
{
	GtkTreeView *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
	GtkTreePath *path;

	if ( !gtk_tree_selection_get_selected (selection, &model, &iter))
        return;
	path = gtk_tree_model_get_path (model, &iter);
	tree_view = gtk_tree_selection_get_tree_view (selection);
	gtk_tree_view_collapse_row (tree_view, path);
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
static void gsb_reconcile_treeview_row_collapsed (GtkTreeView *treeview,
												 GtkTreeIter *iter,
												 GtkTreePath *path,
												 GtkWidget *button)
{
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
static void gsb_reconcile_treeview_row_expanded (GtkTreeView *treeview,
												 GtkTreeIter *iter,
												 GtkTreePath *path,
												 GtkWidget *button)
{
	utils_set_tree_store_background_color (GTK_WIDGET (treeview), RECONCILIATION_BACKGROUND_COLOR);
	gtk_widget_set_sensitive (button, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 * */
static void gsb_reconcile_config_sort_reconcile (GtkToggleButton *togglebutton,
                                                 gpointer user_data)
{
    gsb_reconcile_config_fill ();
}

 /**
 * create the config widget for the reconcile
 * to modify/delete a reconcile
 *
 * \param
 *
 * \return a GtkWidget containing the config widget
 */
GtkWidget *gsb_reconcile_config_create ( void )
{
    GtkWidget *scrolled_window;
	GtkWidget *hbox;
    GtkWidget *vbox_pref;
    GtkWidget *paddinggrid;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeStore *reconcile_model;
    GtkTreeSelection *reconcile_selection;
    GtkWidget *label;
    gint i;
    gfloat alignment[] = {
	COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER,
	COLUMN_RIGHT, COLUMN_RIGHT
    };
    gchar *titles[] = {
	_("Account"), _("Init date"), _("Final date"),
	_("Init balance"), _("Final balance")
    };
    GtkWidget *table_selection;
    GtkWidget *button;
    gint width_entry = 80;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
    vbox_pref = new_vbox_with_title_and_icon ( _("Reconciliation"),
					       "gsb-reconciliation-32.png" );

    gsb_automem_radiobutton3_new_with_title ( vbox_pref,
                        _("Select the end date of reconciliation: "),
                        _("Start Date + one month"),
                        _("Today's date"),
                        NULL,
                        &w_etat->reconcile_end_date,
                        G_CALLBACK ( gsb_reconcile_config_end_date_changed ),
                        NULL,
                        GTK_ORIENTATION_HORIZONTAL );

    //~ paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, _("List of reconciliations"));
	//~ gtk_widget_set_vexpand (paddinggrid, TRUE);
	paddinggrid = new_paddingbox_with_title (vbox_pref, FALSE, _("List of reconciliations"));

    /* set the list */
    scrolled_window = utils_prefs_scrolled_window_new ( NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_PG, 200);
    //~ gtk_grid_attach (GTK_GRID (paddinggrid), scrolled_window, 0, 0, 3, 3);
	gtk_box_pack_start (GTK_BOX (paddinggrid), scrolled_window, TRUE, TRUE, 0 );

    /* need to create first the table to set it in the arg of the changed signal of selection */
    table_selection = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (table_selection), 6);
    gtk_grid_set_column_spacing (GTK_GRID (table_selection), 6);

    /* create the model */
    reconcile_model = gtk_tree_store_new ( NUM_RECONCILIATION_COLUMNS,
					   G_TYPE_STRING,    /* Name account or reconciliation */
					   G_TYPE_STRING,    /* init date  */
					   G_TYPE_STRING,    /* final date  */
					   G_TYPE_STRING,    /* init balance  */
					   G_TYPE_STRING,    /* final balance  */
					   G_TYPE_INT,       /* Account number */
					   G_TYPE_INT,       /* Bold or regular text */
					   G_TYPE_INT,		 /* reconciliation number */
					   GDK_TYPE_RGBA);

	reconcile_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (reconcile_model) );
	gtk_widget_set_name (reconcile_treeview, "colorized_tree_view");
    g_object_unref (G_OBJECT(reconcile_model));
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview)),
				  GTK_SELECTION_SINGLE );
    gtk_container_add ( GTK_CONTAINER (scrolled_window), reconcile_treeview );

    reconcile_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview));
    g_signal_connect (reconcile_selection,
                      "changed",
                      G_CALLBACK (gsb_reconcile_config_select),
                      table_selection);

    /* Name */
    for (i=RECONCILIATION_NAME_COLUMN ; i<RECONCILIATION_ACCOUNT_COLUMN ; i++)
    {
	cell = gtk_cell_renderer_text_new ();
	g_object_set ( G_OBJECT (cell),
		       "xalign", alignment[i],
		       NULL );
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_sizing ( column,
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_alignment ( column,
					     alignment[i] );
	gtk_tree_view_column_pack_end ( column, cell, TRUE );
	gtk_tree_view_column_set_title ( column, titles[i] );
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", i,
					     "weight", RECONCILIATION_WEIGHT_COLUMN,
						 "cell-background-rgba", RECONCILIATION_BACKGROUND_COLOR,
					     NULL);
	gtk_tree_view_column_set_expand ( column, TRUE );
	gtk_tree_view_column_set_resizable ( column,
					     TRUE );
	gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);
    }

    /* Various remaining settings */
    gsb_reconcile_config_fill();

    /* Set the reconcile_sort */
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_box_pack_start (GTK_BOX (paddinggrid), hbox, TRUE, TRUE, 0 );

	button = gsb_automem_checkbutton_new (_("Sort by descending date the reconciliations"),
                                          &w_etat->reconcile_sort,
                                          G_CALLBACK (gsb_reconcile_config_sort_reconcile),
                                          NULL);
    gtk_widget_set_margin_top (button, MARGIN_TOP);
    //~ gtk_grid_attach (GTK_GRID (paddinggrid), button, 0, 3, 1, 1);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0 );

	button = gtk_button_new_with_label (_("Collapse row"));
	gtk_widget_set_sensitive (button, FALSE);
	g_signal_connect (G_OBJECT (button),
					  "clicked",
					  G_CALLBACK (gsb_reconcile_button_collapse_row_clicked),
					  reconcile_selection);
	//~ gtk_grid_attach (GTK_GRID (paddinggrid), button, 1, 3, 1, 1);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0 );

	/* set signal here because data is button */
    g_signal_connect (reconcile_treeview,
                      "row-expanded",
                      G_CALLBACK (gsb_reconcile_treeview_row_expanded),
                      button);

    g_signal_connect (reconcile_treeview,
                      "row-collapsed",
                      G_CALLBACK (gsb_reconcile_treeview_row_collapsed),
                      button);

    /* set the modifying part under the list */
    paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref,_("Selected reconcile") );
	gtk_widget_set_hexpand (paddinggrid, TRUE);

    /* for that we make a table 2x3 but with the names 4x3,
     * the table has been set before to accept as arg on the changed selection */
    gtk_grid_attach (GTK_GRID (paddinggrid), table_selection, 0, 0, 1, 1);

    /* set the name */
	label = gtk_label_new ( _("Reconciliation reference: ") );
	utils_labels_set_alignment ( GTK_LABEL (label), 0, 0.5);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_grid_attach (GTK_GRID (table_selection), label, 0, 0, 1, 1);

	reconcile_name_entry = gsb_autofunc_entry_new ( NULL,
			G_CALLBACK (gsb_reconcile_config_update_line), reconcile_treeview,
			G_CALLBACK (gsb_data_reconcile_set_name), 0 );
    gtk_widget_set_size_request ( reconcile_name_entry, width_entry, -1 );
	gtk_grid_attach (GTK_GRID (table_selection), reconcile_name_entry, 1, 0, 1, 1);

	/* set the initial date */
	label = gtk_label_new ( _("Initial date: ") );
	utils_labels_set_alignment ( GTK_LABEL (label), 0, 0.5);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_grid_attach (GTK_GRID (table_selection), label, 0, 1, 1, 1);

	reconcile_init_date_entry = gsb_autofunc_date_new ( NULL,
			G_CALLBACK (gsb_reconcile_config_update_line), reconcile_treeview,
			G_CALLBACK (gsb_data_reconcile_set_init_date), 0 );
    gtk_widget_set_size_request ( reconcile_init_date_entry, width_entry, -1 );
	gtk_grid_attach (GTK_GRID (table_selection), reconcile_init_date_entry, 1, 1, 1, 1);

    /* set the final date */
	label = gtk_label_new ( _("Final date: ") );
	utils_labels_set_alignment ( GTK_LABEL (label), 0, 0.5);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_grid_attach (GTK_GRID (table_selection), label, 0, 2, 1, 1);

	reconcile_final_date_entry = gsb_autofunc_date_new ( NULL,
			G_CALLBACK (gsb_reconcile_config_update_line), reconcile_treeview,
			G_CALLBACK (gsb_data_reconcile_set_final_date), 0 );
    gtk_widget_set_size_request ( reconcile_final_date_entry, width_entry, -1 );
	gtk_grid_attach (GTK_GRID (table_selection), reconcile_final_date_entry, 1, 2, 1, 1);

    /* set the delete button */
	delete_reconcile_button = gtk_button_new_with_label (_("Delete the reconcile"));
    gtk_button_set_relief ( GTK_BUTTON (delete_reconcile_button), GTK_RELIEF_NORMAL);
	g_signal_connect ( G_OBJECT (delete_reconcile_button), "clicked",
			G_CALLBACK (gsb_reconcile_config_delete),
			reconcile_treeview );
	gtk_grid_attach (GTK_GRID (table_selection), delete_reconcile_button, 2, 0, 2, 1);

	/* set the initial balance */
	label = gtk_label_new ( _("Initial balance: ") );
	utils_labels_set_alignment ( GTK_LABEL (label), 0, 0.5);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_grid_attach (GTK_GRID (table_selection), label, 2, 1, 1, 1);

	reconcile_init_balance_entry = gsb_autofunc_real_new ( null_real,
			G_CALLBACK (gsb_reconcile_config_update_line), reconcile_treeview,
			G_CALLBACK (gsb_data_reconcile_set_init_balance), 0 );
    gtk_widget_set_size_request ( reconcile_init_balance_entry, width_entry, -1 );
	gtk_grid_attach (GTK_GRID (table_selection), reconcile_init_balance_entry, 3, 1, 1, 1);

    /* set the final balance */
	label = gtk_label_new ( _("Final balance: ") );
	utils_labels_set_alignment ( GTK_LABEL (label), 0, 0.5);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
	gtk_grid_attach (GTK_GRID (table_selection), label, 2, 2, 1, 1);

	reconcile_final_balance_entry = gsb_autofunc_real_new ( null_real,
			G_CALLBACK (gsb_reconcile_config_update_line), reconcile_treeview,
			G_CALLBACK (gsb_data_reconcile_set_final_balance), 0 );
    gtk_widget_set_size_request ( reconcile_final_balance_entry, width_entry, -1 );
    gtk_grid_attach (GTK_GRID (table_selection), reconcile_final_balance_entry, 3, 2, 1, 1);

	/* at the beginning, the table is unsensitive */
	gtk_widget_set_sensitive ( table_selection, FALSE );

    /* set the button to find non-associated transactions */
	button = gtk_button_new_with_label (
                        _("Find all marked transactions not associated with a reconciliation"));
	gtk_button_set_relief ( GTK_BUTTON (button), GTK_RELIEF_NORMAL );
    utils_widget_set_padding (button, 0, MARGIN_TOP);
	g_signal_connect ( G_OBJECT (button),
                      "clicked",
                      G_CALLBACK (gsb_reconcile_config_find_alone_transactions),
                      NULL );
    gtk_grid_attach (GTK_GRID (paddinggrid), button, 0, 1, 1, 1);

    gtk_widget_show_all (vbox_pref);
	utils_set_tree_store_background_color (reconcile_treeview, RECONCILIATION_BACKGROUND_COLOR);

	if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}


/**
 * fill the reconcile list,
 * sort each reconcile in its account
 *
 * \param
 *
 * \return
 * */
void gsb_reconcile_config_fill ( void )
{
    GtkTreeModel *model;
    GSList *tmp_list;
	GrisbiWinEtat *w_etat;

    if (!reconcile_treeview)
		return;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (reconcile_treeview));
    gtk_tree_store_clear (GTK_TREE_STORE(model));

    /* we make a tree_model containing the accounts,
     * and for each account, all the reconciles */
    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
		gint account_number;
		GtkTreeIter account_iter;
		GList *reconcile_list;
		GList *tmp_reconcile_list;

		account_number = gsb_data_account_get_no_account (tmp_list -> data);

		gtk_tree_store_append ( GTK_TREE_STORE (model),
					&account_iter,
					NULL );
		gtk_tree_store_set ( GTK_TREE_STORE (model),
					 &account_iter,
					 RECONCILIATION_NAME_COLUMN, gsb_data_account_get_name (account_number),
					 RECONCILIATION_WEIGHT_COLUMN, 800,
					 RECONCILIATION_ACCOUNT_COLUMN, account_number,
					 -1 );

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

				gtk_tree_store_append ( GTK_TREE_STORE (model),
							&reconcile_iter,
							&account_iter );
				gtk_tree_store_set ( GTK_TREE_STORE (model),
							 &reconcile_iter,
							 RECONCILIATION_NAME_COLUMN, gsb_data_reconcile_get_name (reconcile_number),
							 RECONCILIATION_WEIGHT_COLUMN, 400,
							 RECONCILIATION_INIT_DATE_COLUMN, init_date,
							 RECONCILIATION_FINAL_DATE_COLUMN, final_date,
							 RECONCILIATION_INIT_BALANCE_COLUMN, init_balance,
							 RECONCILIATION_FINAL_BALANCE_COLUMN, final_balance,
							 RECONCILIATION_RECONCILE_COLUMN, reconcile_number,
							 RECONCILIATION_ACCOUNT_COLUMN, account_number,
							 -1 );
				g_free (init_date);
				g_free (final_date);
				g_free (init_balance);
				g_free (final_balance);
			}
			tmp_reconcile_list = tmp_reconcile_list->next;
		}
		g_list_free (reconcile_list);

		tmp_list = tmp_list -> next;
    }
}



/**
 * callback called when select a line in the tree
 * fill the entry of the name, dates and balances
 *
 * \param selection the GtkTreeSelection
 * \param table the GtkTable containing the widgets to set the value of the selection
 *
 * \return FALSE
 */
gboolean gsb_reconcile_config_select ( GtkTreeSelection *selection,
				       GtkWidget *table )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean good;

    good = gtk_tree_selection_get_selected (selection, &model, &iter);
    if (good)
    {
	gint reconcile_number;

	gtk_tree_model_get (model, &iter,
			    RECONCILIATION_RECONCILE_COLUMN, &reconcile_number,
			    -1 );

	if (reconcile_number)
	{
	    /* we are on a payment method, we fill the fields */
	    gsb_autofunc_entry_set_value ( reconcile_name_entry,
					   gsb_data_reconcile_get_name (reconcile_number),
					   reconcile_number );
	    gsb_autofunc_date_set ( reconcile_init_date_entry,
				    gsb_data_reconcile_get_init_date (reconcile_number),
				    reconcile_number);
	    gsb_autofunc_date_set ( reconcile_final_date_entry,
				    gsb_data_reconcile_get_final_date (reconcile_number),
				    reconcile_number );
	    gsb_autofunc_real_set ( reconcile_init_balance_entry,
				    gsb_data_reconcile_get_init_balance (reconcile_number),
				    reconcile_number );
	    gsb_autofunc_real_set ( reconcile_final_balance_entry,
				    gsb_data_reconcile_get_final_balance (reconcile_number),
				    reconcile_number );

	    /* we make the table sensitive */
	    gtk_widget_set_sensitive ( table, TRUE );
	}
	else
	    gtk_widget_set_sensitive ( table, FALSE );
    }
    else
	gtk_widget_set_sensitive ( table, FALSE );
    return FALSE;
}


/**
 * callback called when something change in the entries of the configuration of the reconcile
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_config_update_line ( GtkWidget *entry,
					    GtkWidget *tree_view )
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

	gtk_tree_model_get (model, &iter,
			    RECONCILIATION_RECONCILE_COLUMN, &reconcile_number,
			    -1 );

	if (reconcile_number)
	{
	    /* we are on a reconcile, change the line with the new values */
		gchar *init_date, *final_date;
		gchar *init_balance, *final_balance;

		init_date = gsb_format_gdate (gsb_data_reconcile_get_init_date (reconcile_number));
		final_date = gsb_format_gdate (gsb_data_reconcile_get_final_date (reconcile_number));
		init_balance = utils_real_get_string (gsb_data_reconcile_get_init_balance (reconcile_number));
		final_balance = utils_real_get_string (gsb_data_reconcile_get_final_balance (reconcile_number));

		gtk_tree_store_set ( GTK_TREE_STORE (model),
				     &iter,
				     RECONCILIATION_NAME_COLUMN, gsb_data_reconcile_get_name (reconcile_number),
				     RECONCILIATION_INIT_DATE_COLUMN, init_date,
				     RECONCILIATION_FINAL_DATE_COLUMN, final_date,
				     RECONCILIATION_INIT_BALANCE_COLUMN, init_balance,
				     RECONCILIATION_FINAL_BALANCE_COLUMN, final_balance,
				     -1 );
		g_free (init_date);
		g_free (final_date);
		g_free (init_balance);
		g_free (final_balance);

	}
    }

    return FALSE;
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
 * */
gboolean gsb_reconcile_config_delete ( GtkWidget *button,
				       GtkWidget *tree_view )
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

	gtk_tree_model_get (model, &iter,
			    RECONCILIATION_RECONCILE_COLUMN, &reconcile_number,
			    -1 );

	if (reconcile_number)
	{
	    /* we are on a reconcile,
	     * we ask if the user want to continue and warn him */
	    gchar* tmpstr = g_strdup_printf (_("Caution, you are about to delete a reconciliation.\nIf you continue, the reconciliation %s will be erased and all the transactions marked by this reconciliation will be un-reconciled and marked P.\nAre you sure you want to continue?"),
							 gsb_data_reconcile_get_name (reconcile_number));
	    if (!dialogue_yes_no (tmpstr, _("Delete reconciliation"),
					GTK_RESPONSE_NO ))
	    {
		g_free ( tmpstr );
		return FALSE;
	    }

	    /* ok we delete the reconcile
	     * this will automatickly remove it from the marked transactions */
	    gtk_tree_store_remove ( GTK_TREE_STORE (model),
				    &iter );
	    gsb_data_reconcile_remove (reconcile_number);

	    /* if we are on an account, we update the list */
	    account_number = gsb_gui_navigation_get_current_account ();
	    if (account_number != -1)
	    {
		gsb_transactions_list_update_tree_view (account_number, TRUE);

		/* update the last statement for that account */
		gsb_gui_navigation_update_statement_label (account_number);
	    }
            gsb_file_set_modified ( TRUE );
	}
    }

    return FALSE;
}

/**
 * called by user, find all the R marked transactions without a number of reconcile
 * show them in a list and propose to associate them to a reconcile
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_config_find_alone_transactions ( void )
{
    gsb_assistant_reconcile_config_run ();
    return FALSE;
}


/**
 *
 *
 * */
gboolean gsb_reconcile_config_end_date_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gpointer data )
{
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
    w_etat->reconcile_end_date = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( checkbutton ), "pointer" ) );

    gsb_file_set_modified ( TRUE );

    return FALSE;
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
