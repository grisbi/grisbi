/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	          */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	                      */
/* 			http://www.grisbi.org				                              */
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
 * \file gsb_reconcile_sort_config.c
 * functions used to configure the sorting functions of the reconcile
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <math.h>
#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_reconcile_sort_config.h"
#include "gsb_data_account.h"
#include "gsb_data_payment.h"
#include "gsb_file.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_prefs.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_reconcile_sort_config_move_down ( GtkWidget *button,
					       GtkWidget *tree_view );
static gboolean gsb_reconcile_sort_config_move_up ( GtkWidget *button,
					     GtkWidget *tree_view );
static gboolean gsb_reconcile_sort_config_neutral_toggled ( GtkCellRendererToggle *cell,
						     gchar *path_str,
						     GtkWidget *tree_view );
static gboolean gsb_reconcile_sort_config_payment_toggled ( GtkCellRendererToggle *cell,
						     gchar *path_str,
						     GtkWidget *tree_view );
static gboolean gsb_reconcile_sort_config_select ( GtkTreeSelection *tselection,
					    GtkTreeModel *model );
static gboolean gsb_reconcile_sort_config_update_account_sort_list ( gint account_number,
							      GtkTreeModel *model );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static GtkWidget *reconcile_treeview;
static GtkWidget *button_move_up;
static GtkWidget *button_move_down;

/**
 * create the config widget for the reconcile
 * to choose the order of the method of payment to sort the list
 * while reconciling
 *
 * \param
 *
 * \return a GtkWidget containing the config widget
 */
GtkWidget *gsb_reconcile_sort_config_create ( void )
{
    GtkWidget *scrolled_window;
    GtkWidget *vbox_pref;
    GtkWidget *paddinggrid;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeStore *reconcile_model;
    GtkTreeSelection *reconcile_selection;

    vbox_pref = new_vbox_with_title_and_icon ( _("Sort for reconciliation"),
					       "gsb-reconciliation-32.png" );
    paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref,
                                                          _("Reconciliation: sort transactions") );

    /* mise en place de la liste qui contient les types classés */
    scrolled_window = utils_prefs_scrolled_window_new ( NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_PG, 400 );
    gtk_grid_attach (GTK_GRID (paddinggrid), scrolled_window, 0, 0, 2, 3);

    reconcile_model = gtk_tree_store_new ( NUM_RECONCILIATION_SORT_COLUMNS,
					   G_TYPE_STRING, /* Name */
					   G_TYPE_BOOLEAN, /* Visible */
					   G_TYPE_BOOLEAN, /* Sort by method of payment */
					   G_TYPE_BOOLEAN, /* Split neutrals */
					   G_TYPE_INT, /* Account number */
					   G_TYPE_INT,     /* payment number */
					   G_TYPE_BOOLEAN ); /* sensitive line */

    reconcile_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (reconcile_model) );
	gtk_widget_set_name (reconcile_treeview, "tree_view");
    g_object_unref (G_OBJECT(reconcile_model));
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview)),
				  GTK_SELECTION_SINGLE );

    reconcile_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview));
    g_signal_connect (reconcile_selection,
                      "changed",
                      G_CALLBACK (gsb_reconcile_sort_config_select),
                      reconcile_model);

    /* Name */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Payment method") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "text", RECONCILIATION_SORT_NAME_COLUMN,
					 "sensitive", RECONCILIATION_SORT_SENSITIVE_COLUMN,
					 NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Sort by date */
    cell = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_signal_connect (cell, "toggled",
		      G_CALLBACK (gsb_reconcile_sort_config_payment_toggled), reconcile_treeview);
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Sort by payment method") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", RECONCILIATION_SORT_SORT_COLUMN,
					 "activatable", RECONCILIATION_SORT_VISIBLE_COLUMN,
					 "visible", RECONCILIATION_SORT_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Split neutral payment methods */
    cell = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_signal_connect (cell, "toggled",
		      G_CALLBACK (gsb_reconcile_sort_config_neutral_toggled), reconcile_treeview);
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Split neutral payment methods") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN,
					 "activatable", RECONCILIATION_SORT_VISIBLE_COLUMN,
					 "visible", RECONCILIATION_SORT_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Various remaining settings */
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), reconcile_treeview );

    gsb_reconcile_sort_config_fill();

    button_move_up = utils_buttons_button_new_from_stock ("gtk-go-up", _("Go up"));
    gtk_widget_set_margin_top (button_move_up, MARGIN_TOP);
    gtk_button_set_relief ( GTK_BUTTON ( button_move_up ), GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( button_move_up ),
                      "clicked",
                      G_CALLBACK (gsb_reconcile_sort_config_move_up),
                      reconcile_treeview );
    gtk_grid_attach (GTK_GRID (paddinggrid), button_move_up, 0, 3, 1, 1);
    gtk_widget_set_sensitive ( button_move_up, FALSE );

    button_move_down = utils_buttons_button_new_from_stock ("gtk-go-down", _("Go down"));
    gtk_widget_set_margin_top (button_move_down, MARGIN_TOP);
    gtk_button_set_relief ( GTK_BUTTON ( button_move_down ), GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( button_move_down ),
                      "clicked",
                      G_CALLBACK (gsb_reconcile_sort_config_move_down),
                      reconcile_treeview);
    gtk_grid_attach (GTK_GRID (paddinggrid), button_move_down, 1, 3, 1, 1);
    gtk_widget_set_sensitive ( button_move_down, FALSE );

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}


/**
 * clear and fill the reconciliation tree with the accounts,
 * and for each account, set the method of payments in the good order
 *
 * \param
 *
 * \return
 */
void gsb_reconcile_sort_config_fill ( void )
{
    GtkTreeIter account_iter, payment_method_iter;
    GSList *list_tmp;
    GtkTreeModel *model;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (reconcile_treeview));

    gtk_tree_store_clear (GTK_TREE_STORE (model));
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint account_number;
	GSList *sorted_list;
	GSList *payment_list;
	gboolean visible;

	account_number = gsb_data_account_get_no_account ( list_tmp -> data );

	/* when no method of payment, hide the checkbuttons because non sense */
	payment_list = gsb_data_payment_get_list_for_account (account_number);
	if (payment_list)
	{
	    visible = TRUE;
	    g_slist_free (payment_list);
	}
	else
	    visible = FALSE;

	gtk_tree_store_append (GTK_TREE_STORE (model), &account_iter, NULL);
	gtk_tree_store_set (GTK_TREE_STORE (model), &account_iter,
			    RECONCILIATION_SORT_NAME_COLUMN, gsb_data_account_get_name (account_number),
			    RECONCILIATION_SORT_VISIBLE_COLUMN, visible,
			    RECONCILIATION_SORT_SORT_COLUMN, gsb_data_account_get_reconcile_sort_type(account_number),
			    RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, gsb_data_account_get_split_neutral_payment (account_number),
			    RECONCILIATION_SORT_ACCOUNT_COLUMN, account_number,
			    RECONCILIATION_SORT_TYPE_COLUMN, 0,
			    RECONCILIATION_SORT_SENSITIVE_COLUMN, visible,
			    -1 );

	/* the sorted list is a list of numbers of method of payment, in the good order */
	sorted_list = gsb_data_account_get_sort_list (account_number);

	while ( sorted_list )
	{
	    gint payment_number;

	    /* in the sorted list, a number can be negative, when split a neutral into 2 parts
	     * so payment number here can be negative for neutral payments */
	    payment_number = GPOINTER_TO_INT (sorted_list -> data);

	    if (payment_number)
	    {
		gchar *name = NULL;

		gtk_tree_store_append (GTK_TREE_STORE (model),
				       &payment_method_iter,
				       &account_iter);

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
				name = g_strconcat ( gsb_data_payment_get_name (-payment_number),
						     " ( - )", NULL );
			    else
				name = g_strconcat ( gsb_data_payment_get_name (payment_number),
						     " ( + )", NULL );
			}
			else
			    name = my_strdup (gsb_data_payment_get_name (payment_number));
			break;
		}

		gtk_tree_store_set (GTK_TREE_STORE (model), &payment_method_iter,
				    RECONCILIATION_SORT_NAME_COLUMN, name,
				    RECONCILIATION_SORT_VISIBLE_COLUMN, FALSE,
				    RECONCILIATION_SORT_SORT_COLUMN, FALSE,
				    RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, FALSE,
				    RECONCILIATION_SORT_ACCOUNT_COLUMN, account_number,
				    RECONCILIATION_SORT_TYPE_COLUMN, payment_number,
				    RECONCILIATION_SORT_SENSITIVE_COLUMN, TRUE,
				    -1 );
		if (name)
		    g_free (name);
	    }
	    sorted_list = sorted_list -> next;
	}

	if ( gtk_tree_model_iter_has_child( GTK_TREE_MODEL(model), &account_iter)
	     &&
	     gsb_data_account_get_reconcile_sort_type (account_number) )
	{
	    GtkTreePath * treepath;
	    treepath = gtk_tree_model_get_path (GTK_TREE_MODEL(model), &account_iter);
	    if ( treepath )
	    {
		gtk_tree_view_expand_row ( GTK_TREE_VIEW(reconcile_treeview), treepath, TRUE );
		gtk_tree_path_free ( treepath );
	    }
	}
	list_tmp = list_tmp -> next;
    }
}


/**
 * callback called when select a line in the tree
 * show/hide the buttons up and down to move the method of payment
 *
 * \param tselection the GtkTreeSelection
 * \param model
 *
 * \return FALSE
 */
gboolean gsb_reconcile_sort_config_select ( GtkTreeSelection *tselection,
					    GtkTreeModel *model )
{
    GtkTreeIter iter;
    gboolean good;

    good = gtk_tree_selection_get_selected (tselection, NULL, &iter);
    if (good)
    {
	gint payment_method;

	gtk_tree_model_get (model, &iter,
			    RECONCILIATION_SORT_TYPE_COLUMN, &payment_method,
			    -1 );

	if (payment_method)
	{
	    GtkTreePath * treepath;

	    /* Is there something before? */
	    treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(model), &iter );
	    gtk_widget_set_sensitive ( button_move_up,
				       gtk_tree_path_prev(treepath) );
	    gtk_tree_path_free ( treepath );

	    /* Is there something after? */
	    gtk_widget_set_sensitive ( button_move_down,
				       gtk_tree_model_iter_next (model, &iter) );
	}
	else
	{
	    gtk_widget_set_sensitive ( button_move_up, FALSE );
	    gtk_widget_set_sensitive ( button_move_down, FALSE );
	}
    }
    return FALSE;
}



/**
 * Callback called by a click on the up arrow
 * to move a payment method up
 *
 * \param button
 * \param tree_view
 *
 * \return FALSE
 */
gboolean gsb_reconcile_sort_config_move_up ( GtkWidget *button,
					     GtkWidget *tree_view )
{
    GtkTreePath * treepath;
    gboolean good, visible = 0;
    GtkTreeIter iter, other;
    gint payment_number;
    gint account_number = -1;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));


    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     RECONCILIATION_SORT_VISIBLE_COLUMN, &visible,
			     RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
			     RECONCILIATION_SORT_TYPE_COLUMN, &payment_number,
			     -1 );

    if ( good && ! visible )
    {
	treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(model),
					     &iter );

	if ( gtk_tree_path_prev ( treepath ) &&
	     gtk_tree_model_get_iter ( GTK_TREE_MODEL(model),
				       &other, treepath ) )
	{
	    gtk_tree_store_move_before ( GTK_TREE_STORE(model),
					 &iter, &other );
	}
    }

    gsb_reconcile_sort_config_select ( selection,
				       GTK_TREE_MODEL(model) );
    gsb_reconcile_sort_config_update_account_sort_list (account_number, model);
    return FALSE;
}


/**
 * Callback called by a click on the up arrow
 * to move a payment method up
 *
 * \param button
 * \param tree_view
 *
 * \return FALSE
 */
gboolean gsb_reconcile_sort_config_move_down ( GtkWidget *button,
					       GtkWidget *tree_view )
{
    GtkTreePath * treepath;
    gboolean good, visible = 0;
    GtkTreeIter iter, other;
    gint payment_number;
    gint account_number = -1;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     RECONCILIATION_SORT_VISIBLE_COLUMN, &visible,
			     RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
			     RECONCILIATION_SORT_TYPE_COLUMN, &payment_number,
			     -1 );

    if ( good && ! visible )
    {
	treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(model),
					     &iter );

	gtk_tree_path_next ( treepath ) ;
	if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(model),
				       &other, treepath ) )
	{
	    gtk_tree_store_move_after ( GTK_TREE_STORE(model),
					&iter, &other );
	}
    }

    gsb_reconcile_sort_config_select ( selection,
				       GTK_TREE_MODEL(model) );
    gsb_reconcile_sort_config_update_account_sort_list (account_number, model);
    return FALSE;
}



/**
 * update the sort list of an account according to the model
 * called when change something in the order of method of payment
 *
 * \param account_number
 * \param model
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_sort_config_update_account_sort_list ( gint account_number,
							      GtkTreeModel *model )
{
    GtkTreeIter iter;

    gsb_data_account_sort_list_free (account_number);

    if (gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (model),
					&iter ))
    {
	/* first, we try to find the account line */
	do
	{
	    gint account_number_tmp;

	    gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
				 RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number_tmp,
				 -1 );
	    if (account_number_tmp == account_number)
	    {
		/* ok, we are on the good account,
		 * now we append the method of payment in the same order as the list */
		GtkTreeIter iter_child;

		if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (model), &iter_child, &iter))
		{
		    do
		    {
			gint payment_number;

			gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter_child,
					     RECONCILIATION_SORT_TYPE_COLUMN, &payment_number,
					     -1 );
			gsb_data_account_sort_list_add ( account_number, payment_number );
		    }
		    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_child));
		}
	    }
	}
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter)
	       &&
	       !gsb_data_account_get_sort_list (account_number));
    }
    return FALSE;
}



/**
 * Callback called when toggle the check button to sort by method of payment
 *
 * \param cell
 * \param path_str
 * \param tree_view
 *
 * \return FALSE
 */
gboolean gsb_reconcile_sort_config_payment_toggled ( GtkCellRendererToggle *cell,
						     gchar *path_str,
						     GtkWidget *tree_view )
{
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean toggle;
    gint account_number;
    GtkTreeModel *model;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    treepath = gtk_tree_path_new_from_string ( path_str );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (model),
			      &iter, treepath );

    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter,
			RECONCILIATION_SORT_SORT_COLUMN, &toggle,
			RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
			-1);

    toggle ^= 1;

    /* set new value */
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			RECONCILIATION_SORT_SORT_COLUMN, toggle,
			-1);

    /* Set to 1 (sort by types) if toggle is selected */
    gsb_data_account_set_reconcile_sort_type ( account_number,
					       toggle );

    if (toggle)
	gtk_tree_view_expand_row ( GTK_TREE_VIEW(tree_view), treepath, FALSE );
    else
	gtk_tree_view_collapse_row ( GTK_TREE_VIEW(tree_view), treepath );

    gtk_tree_path_free ( treepath );
    gsb_file_set_modified ( TRUE );
    return FALSE;
}



/**
 * Callback called when toggle the check button to split the neutral method of payments
 *
 * \param cell
 * \param path_str
 * \param tree_view
 *
 * \return FALSE
 */
gboolean gsb_reconcile_sort_config_neutral_toggled ( GtkCellRendererToggle *cell,
						     gchar *path_str,
						     GtkWidget *tree_view )
{
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean toggle;
    gint account_number;
    GSList *sorted_list_copy;
    GSList *tmp_list;
    GtkTreeModel *model;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    /* invert the toggle */
    treepath = gtk_tree_path_new_from_string ( path_str );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (model),
			      &iter, treepath );

    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter,
			RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, &toggle,
			RECONCILIATION_SORT_ACCOUNT_COLUMN, &account_number,
			-1);
    toggle ^= 1;
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN, toggle,
			-1);

    /* and save it */
    gsb_data_account_set_split_neutral_payment ( account_number,
						 toggle );

    /* need to copy the sorted_list to avoid an infinite loop when add a negative payment neutral later */
    sorted_list_copy = g_slist_copy (gsb_data_account_get_sort_list (account_number));
    tmp_list = sorted_list_copy;

    while (tmp_list)
    {
	gint payment_number;

	payment_number = GPOINTER_TO_INT (tmp_list -> data);

	/* payment_number can be negative, so do for it abs */
	if ( gsb_data_payment_get_sign (abs (payment_number)) == GSB_PAYMENT_NEUTRAL )
	{
	    if (toggle)
		gsb_data_account_sort_list_add ( account_number,
						 -payment_number );
	    else
		if (payment_number < 0 )
		    gsb_data_account_sort_list_remove ( account_number,
							payment_number );
	}
	tmp_list = tmp_list -> next;
    }
    g_slist_free (sorted_list_copy);
    gsb_reconcile_sort_config_fill ();
    gsb_file_set_modified ( TRUE );
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
