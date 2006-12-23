/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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


#include "include.h"

/*START_INCLUDE*/
#include "gsb_reconcile_config.h"
#include "gsb_data_account.h"
#include "gsb_data_payment.h"
#include "utils_str.h"
#include "utils.h"
#include "structures.h"
#include "gsb_reconcile_config.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean deplacement_type_tri_bas ( void );
static gboolean deplacement_type_tri_haut ( GtkWidget * button, gpointer data );
static gboolean gsb_reconcile_config_update_account_sort_list ( gint account_number );
static void reconcile_by_date_toggled ( GtkCellRendererToggle *cell, 
				 gchar *path_str, gpointer data );
static void reconcile_include_neutral_toggled ( GtkCellRendererToggle *cell, 
					 gchar *path_str, gpointer data );
static void select_reconciliation_entry ( GtkTreeSelection * tselection, 
				   GtkTreeModel * model );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static GtkWidget *reconcile_treeview;
static GtkTreeStore *reconcile_model;
static GtkWidget *button_move_up;
static GtkWidget *button_move_down;
static GtkTreeSelection *reconcile_selection;


/**
 * TODO: document this
 *
 */
GtkWidget * tab_display_reconciliation ( void )
{
    GtkWidget *hbox, *scrolled_window;
    GtkWidget *vbox_pref, *paddingbox;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkWidget *vbox_fleches_tri;

    vbox_pref = new_vbox_with_title_and_icon ( _("Reconciliation"),
					       "reconciliation.png" );

    paddingbox = new_paddingbox_with_title ( vbox_pref, TRUE,
					     COLON(_("Reconciliation: sort transactions") ) );

    /* la partie du milieu est une hbox avec les types */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 TRUE, TRUE, 0 );

    /* mise en place de la liste qui contient les types classés */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    reconcile_model = gtk_tree_store_new ( NUM_RECONCILIATION_COLUMNS,
					   G_TYPE_STRING, /* Name */
					   G_TYPE_BOOLEAN, /* Visible */
					   G_TYPE_BOOLEAN, /* Sort by date */
					   G_TYPE_BOOLEAN, /* Split neutrals */
					   G_TYPE_POINTER, /* Account pointer */
					   G_TYPE_INT ); /* payment number */
    reconcile_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (reconcile_model) );
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (reconcile_treeview), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview)),
				  GTK_SELECTION_SINGLE );
    reconcile_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview));
    g_signal_connect (reconcile_selection, "changed", 
		      G_CALLBACK (select_reconciliation_entry), reconcile_model);

    /* Name */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Payment method") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "text", RECONCILIATION_NAME_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Sort by date */
    cell = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_signal_connect (cell, "toggled", 
		      G_CALLBACK (reconcile_by_date_toggled), reconcile_model);
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Sort by date") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", RECONCILIATION_SORT_COLUMN,
					 "activatable", RECONCILIATION_VISIBLE_COLUMN,
					 "visible", RECONCILIATION_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Split neutral payment methods */
    cell = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_signal_connect (cell, "toggled", 
		      G_CALLBACK (reconcile_include_neutral_toggled), reconcile_model);
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Split neutral payment methods") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", RECONCILIATION_SPLIT_NEUTRAL_COLUMN,
					 "activatable", RECONCILIATION_VISIBLE_COLUMN,
					 "visible", RECONCILIATION_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Various remaining settings */
    /*   g_signal_connect (treeview, "realize", G_CALLBACK (gtk_tree_view_expand_all),  */
    /* 		    NULL); */
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), reconcile_treeview );

    fill_reconciliation_tree();

    /* on place ici les flèches sur le côté de la liste */
    vbox_fleches_tri = gtk_vbutton_box_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox_fleches_tri,
			 FALSE, FALSE, 0);

    button_move_up = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
    gtk_button_set_relief ( GTK_BUTTON ( button_move_up ), GTK_RELIEF_NONE );
    g_signal_connect ( GTK_OBJECT ( button_move_up ), "clicked",
		       (GCallback) deplacement_type_tri_haut, NULL );
    gtk_container_add ( GTK_CONTAINER ( vbox_fleches_tri ), button_move_up );
    gtk_widget_set_sensitive ( button_move_up, FALSE );

    button_move_down = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
    gtk_button_set_relief ( GTK_BUTTON ( button_move_down ), GTK_RELIEF_NONE );
    g_signal_connect ( GTK_OBJECT ( button_move_down ), "clicked",
		       (GCallback) deplacement_type_tri_bas, NULL);
    gtk_container_add ( GTK_CONTAINER ( vbox_fleches_tri ), button_move_down );
    gtk_widget_set_sensitive ( button_move_down, FALSE );

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}


/** 
 * TODO: document this + move
 */
void fill_reconciliation_tree ()
{
    GtkTreeIter account_iter, payment_method_iter;
    GSList *list_tmp;

    gtk_tree_store_clear (reconcile_model);
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint account_number;
	GSList *sorted_list;

	account_number = gsb_data_account_get_no_account ( list_tmp -> data );

	gtk_tree_store_append (reconcile_model, &account_iter, NULL);
	gtk_tree_store_set (reconcile_model, &account_iter,
			    RECONCILIATION_NAME_COLUMN, gsb_data_account_get_name (account_number),
			    RECONCILIATION_VISIBLE_COLUMN, TRUE,
			    RECONCILIATION_SORT_COLUMN, !gsb_data_account_get_reconcile_sort_type(account_number),
			    RECONCILIATION_SPLIT_NEUTRAL_COLUMN, gsb_data_account_get_split_neutral_payment (account_number),
			    RECONCILIATION_ACCOUNT_COLUMN, account_number,
			    RECONCILIATION_TYPE_COLUMN, -1,
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

		gtk_tree_store_append (reconcile_model,
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

		gtk_tree_store_set (reconcile_model, &payment_method_iter,
				    RECONCILIATION_NAME_COLUMN, name,
				    RECONCILIATION_VISIBLE_COLUMN, FALSE,
				    RECONCILIATION_SORT_COLUMN, FALSE,
				    RECONCILIATION_SPLIT_NEUTRAL_COLUMN, FALSE,
				    RECONCILIATION_ACCOUNT_COLUMN, account_number,
				    RECONCILIATION_TYPE_COLUMN, abs (payment_number),
				    -1 );
		if (name)
		    g_free (name);
	    }
	    sorted_list = sorted_list -> next;
	}

	if ( gtk_tree_model_iter_has_child( GTK_TREE_MODEL(reconcile_model), &account_iter)
	     &&
	     gsb_data_account_get_reconcile_sort_type (account_number) )
	{
	    GtkTreePath * treepath;
	    treepath = gtk_tree_model_get_path (GTK_TREE_MODEL(reconcile_model), &account_iter);
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
 * TODO: move + document this
 *
 */
void select_reconciliation_entry ( GtkTreeSelection * tselection, 
				   GtkTreeModel * model )
{
    GtkTreeIter iter;
    GtkTreePath * treepath;
    GValue value_visible = {0, };
    gboolean good;

    good = gtk_tree_selection_get_selected (tselection, NULL, &iter);
    if (good)
	gtk_tree_model_get_value (model, &iter, 
				  RECONCILIATION_VISIBLE_COLUMN, &value_visible);

    if ( good && ! g_value_get_boolean(&value_visible) )
    {
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



/** 
 * TODO: document this
 */
gboolean deplacement_type_tri_haut ( GtkWidget * button, gpointer data )
{
    GtkTreePath * treepath;
    gboolean good, visible;
    GtkTreeIter iter, other;
    gint payment_number;
    gint account_number;

    good = gtk_tree_selection_get_selected (reconcile_selection, NULL, &iter);
    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(reconcile_model), &iter, 
			     RECONCILIATION_VISIBLE_COLUMN, &visible,
			     RECONCILIATION_ACCOUNT_COLUMN, &account_number,
			     RECONCILIATION_TYPE_COLUMN, &payment_number,
			     -1 );

    if ( good && ! visible )
    {
	treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(reconcile_model), 
					     &iter );

	if ( gtk_tree_path_prev ( treepath ) &&
	     gtk_tree_model_get_iter ( GTK_TREE_MODEL(reconcile_model), 
				       &other, treepath ) )
	{
	    gtk_tree_store_move_before ( GTK_TREE_STORE(reconcile_model), 
					 &iter, &other );
	}
    }

    select_reconciliation_entry ( reconcile_selection, 
				  GTK_TREE_MODEL(reconcile_model) );
    gsb_reconcile_config_update_account_sort_list (account_number);
    return FALSE;
}


/** 
 * TODO: document this
 */
gboolean deplacement_type_tri_bas ( void )
{
    GtkTreePath * treepath;
    gboolean good, visible;
    GtkTreeIter iter, other;
    gint payment_number;
    gint account_number;

    good = gtk_tree_selection_get_selected (reconcile_selection, NULL, &iter);
    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(reconcile_model), &iter, 
			     RECONCILIATION_VISIBLE_COLUMN, &visible,
			     RECONCILIATION_ACCOUNT_COLUMN, &account_number,
			     RECONCILIATION_TYPE_COLUMN, &payment_number,
			     -1 );

    if ( good && ! visible )
    {
	treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(reconcile_model), 
					     &iter );

	gtk_tree_path_next ( treepath ) ;
	if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(reconcile_model), 
				       &other, treepath ) )
	{
	    gtk_tree_store_move_after ( GTK_TREE_STORE(reconcile_model), 
					&iter, &other );
	}
    }

    select_reconciliation_entry ( reconcile_selection, 
				  GTK_TREE_MODEL(reconcile_model) );
    gsb_reconcile_config_update_account_sort_list (account_number);
    return FALSE;
}



/**
 * update the sort list of an account according to the model 
 * called when change something in the order of method of payment
 *
 * \param account_number
 *
 * \return FALSE
 * */
gboolean gsb_reconcile_config_update_account_sort_list ( gint account_number )
{
    GtkTreeIter iter;

    gsb_data_account_sort_list_free (account_number);

    if (gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (reconcile_model),
					&iter ))
    {
	/* first, we try to find the account line */
	do
	{
	    gint account_number_tmp;

	    gtk_tree_model_get ( GTK_TREE_MODEL(reconcile_model), &iter, 
				 RECONCILIATION_ACCOUNT_COLUMN, &account_number_tmp,
				 -1 );
	    if (account_number_tmp == account_number)
	    {
		/* ok, we are on the good account,
		 * now we append the method of payment in the same order as the list */
		GtkTreeIter iter_child;

		if (gtk_tree_model_iter_children ( GTK_TREE_MODEL (reconcile_model), &iter_child, &iter))
		{
		    do
		    {
			gint payment_number;

			gtk_tree_model_get ( GTK_TREE_MODEL(reconcile_model), &iter_child, 
					     RECONCILIATION_TYPE_COLUMN, &payment_number,
					     -1 );
			gsb_data_account_sort_list_add ( account_number, payment_number );
		    }
		    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (reconcile_model), &iter_child));
		}
	    }
	}
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (reconcile_model), &iter)
	       &&
	       !gsb_data_account_get_sort_list (account_number));
    }
    return FALSE;
}



/**
 * TODO: document this
 *
 */
void reconcile_by_date_toggled ( GtkCellRendererToggle *cell, 
				 gchar *path_str, gpointer data )
{
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean toggle;
    gint account_number;

    treepath = gtk_tree_path_new_from_string ( path_str );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (reconcile_model),
			      &iter, treepath );

    gtk_tree_model_get (GTK_TREE_MODEL(reconcile_model), &iter, 
			RECONCILIATION_SORT_COLUMN, &toggle, 
			RECONCILIATION_ACCOUNT_COLUMN, &account_number,
			-1);

    toggle ^= 1;

    /* set new value */
    gtk_tree_store_set (GTK_TREE_STORE (reconcile_model), &iter, 
			RECONCILIATION_SORT_COLUMN, toggle, 
			-1);

    /* Set to 1 (sort by types) if toggle is not selected */

    gsb_data_account_set_reconcile_sort_type ( account_number,
					       ! toggle );

    if (toggle)
    {
	gtk_tree_view_collapse_row ( GTK_TREE_VIEW(reconcile_treeview), treepath );
    }
    else
    {
	gtk_tree_view_expand_row ( GTK_TREE_VIEW(reconcile_treeview), treepath, FALSE );
    }

    gtk_tree_path_free ( treepath );
}



/**
 * TODO: document this
 *
 */
void reconcile_include_neutral_toggled ( GtkCellRendererToggle *cell, 
					 gchar *path_str, gpointer data )
{
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean toggle;
    gint account_number;

    /* invert the toggle */
    treepath = gtk_tree_path_new_from_string ( path_str );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (reconcile_model),
			      &iter, treepath );

    gtk_tree_model_get (GTK_TREE_MODEL(reconcile_model), &iter, 
			RECONCILIATION_SPLIT_NEUTRAL_COLUMN, &toggle, 
			RECONCILIATION_ACCOUNT_COLUMN, &account_number,
			-1);
    toggle ^= 1;
    gtk_tree_store_set (GTK_TREE_STORE (reconcile_model), &iter, 
			RECONCILIATION_SPLIT_NEUTRAL_COLUMN, toggle, 
			-1);

    /* and save it */
    gsb_data_account_set_split_neutral_payment ( account_number,
						 toggle );

    GSList *sorted_list;

    sorted_list = gsb_data_account_get_sort_list (account_number);

    while ( sorted_list )
    {
	gint payment_number;

	payment_number = GPOINTER_TO_INT (sorted_list -> data);

	/* need to do that first to avoid problems later with remove,
	 * but cannot use again sorted list after that */
	sorted_list = sorted_list -> next;

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
    }
    gtk_tree_store_clear ( GTK_TREE_STORE(reconcile_model) );
    fill_reconciliation_tree ( );
}





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
