/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2007 Cédric Auger (cedric@grisbi.org)	          */
/*			2004-2007 Benjamin Drieu (bdrieu@april.org) 	                  */
/*			http://www.grisbi.org   			                              */
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
 * \file gsb_payment_method_config.c
 * all that you need for the config of the payment method is here !!!
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_payment_method_config.h"
#include "dialog.h"
#include "gsb_autofunc.h"
#include "gsb_data_account.h"
#include "gsb_data_form.h"
#include "gsb_data_payment.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_payment_method.h"
#include "gsb_reconcile_sort_config.h"
#include "traitement_variables.h"
#include "utils.h"
#include "transaction_list.h"
#include "structures.h"
#include "gsb_transactions_list.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_payment_method_config_add ( GtkWidget *button,
					 GtkWidget *tree_view );
static gboolean gsb_payment_method_config_auto_button_changed ( GtkWidget *button,
							 GtkWidget *tree_view );
static gboolean gsb_payment_method_config_auto_entry_changed ( GtkWidget *spin_button,
							GtkWidget *tree_view );
static void gsb_payment_method_config_fill_list ( GtkTreeModel *model);
static gboolean gsb_payment_method_config_foreach_select ( GtkTreeModel *model, GtkTreePath *path,
						    GtkTreeIter *iter, gint *payment_number_data );
static gint gsb_payment_method_config_get_transaction_by_sign ( gint account_number,
							 gint signe_type,
							 gint exclude );
static gboolean gsb_payment_method_config_name_changed ( GtkWidget *entry,
						  GtkWidget *tree_view );
static gboolean gsb_payment_method_config_remove ( GtkWidget *button,
					    GtkWidget *tree_view );
static gboolean gsb_payment_method_config_select ( GtkTreeSelection *selection,
					    GtkWidget *tree_view );
static gboolean gsb_payment_method_config_show_entry_changed ( GtkWidget *button,
							GtkWidget *tree_view );
static gboolean gsb_payment_method_config_sign_changed ( GtkWidget *menu_item,
						  gint *sign );
static gboolean gsb_payment_method_config_switch_payment ( gint payment_number );
static gboolean gsb_payment_method_config_toggled ( GtkCellRendererToggle *cell,
					     gchar *path_str,
					     GtkTreeView *tree_view );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *fenetre_preferences;
/*END_EXTERN*/

static GtkWidget *payment_method_treeview;

static GtkWidget *payment_remove_button;
static GtkWidget *payment_name_entry;
static GtkWidget *payment_last_number_entry;
static GtkWidget *payment_sign_button;
static GtkWidget *button_auto_numbering;
static GtkWidget *button_show_entry;


/** Global to handle sensitiveness */
static GtkWidget *details_paddingbox;


/**
 * Creates the "Payment methods" tab.  It uses a nice GtkTreeView.
 *
 * \param
 *
 * \returns A newly allocated vbox
 */
GtkWidget *gsb_payment_method_config_create ( void )
{
    GtkWidget *vbox_pref, *hbox, *scrolled_window, *paddingbox;
    GtkWidget *vbox, *table, *menu, *item, *label;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkWidget *bouton_ajouter_type;
    GtkTreeStore *payment_method_model;
    gint width_entry = 80;

    /* Now we have a model, create view */
    vbox_pref = new_vbox_with_title_and_icon ( _("Payment methods"),
					       "payment.png" );

    /* Known payment methods */
    paddingbox = new_paddingbox_with_title (vbox_pref, TRUE,
					    _("Known payment methods"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 TRUE, TRUE, 0 );

    /* Create tree */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN);
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0 );

    /* Create tree view */
    payment_method_model = gtk_tree_store_new (NUM_PAYMENT_METHODS_COLUMNS,
					       G_TYPE_STRING,
					       G_TYPE_STRING,
					       G_TYPE_BOOLEAN,
					       G_TYPE_INT,
					       G_TYPE_BOOLEAN,
					       G_TYPE_BOOLEAN,
					       G_TYPE_INT,
					       G_TYPE_INT );
    payment_method_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (payment_method_model) );
    g_object_unref (G_OBJECT(payment_method_model));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (payment_method_treeview), TRUE);
    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview)),
		      "changed",
		      G_CALLBACK (gsb_payment_method_config_select),
		      payment_method_treeview);

    /* Account */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Account") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "text", PAYMENT_METHODS_NAME_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(payment_method_treeview), column);

    /* Defaults */
    cell = gtk_cell_renderer_toggle_new ();
    g_signal_connect (cell, "toggled", G_CALLBACK (gsb_payment_method_config_toggled), payment_method_treeview);
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), TRUE );
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Default") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", PAYMENT_METHODS_DEFAULT_COLUMN,
					 "activatable", PAYMENT_METHODS_ACTIVABLE_COLUMN,
					 "visible", PAYMENT_METHODS_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(payment_method_treeview), column);

    /* Numbering */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Numbering") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "text", PAYMENT_METHODS_NUMBERING_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(payment_method_treeview), column);

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect (payment_method_treeview, "realize",
		      G_CALLBACK (gtk_tree_view_expand_all), NULL);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			payment_method_treeview );

    gsb_payment_method_config_fill_list (GTK_TREE_MODEL (payment_method_model));

    /* Create "Add" & "Remove" buttons */
    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			 FALSE, FALSE, 0 );

    /* "Add payment method" button */
    bouton_ajouter_type = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_type ),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( bouton_ajouter_type ),
		       "clicked",
		       G_CALLBACK (gsb_payment_method_config_add),
		       payment_method_treeview );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_ajouter_type,
			 TRUE, FALSE, 5 );

    /* "Remove payment method" button */
    payment_remove_button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    gtk_button_set_relief ( GTK_BUTTON ( payment_remove_button ),
			    GTK_RELIEF_NONE );
    gtk_widget_set_sensitive ( payment_remove_button, FALSE );
    g_signal_connect ( G_OBJECT ( payment_remove_button ),
		       "clicked",
		       G_CALLBACK (gsb_payment_method_config_remove),
		       payment_method_treeview );
    gtk_box_pack_start ( GTK_BOX ( vbox ), payment_remove_button,
			 TRUE, FALSE, 5 );

    /* Payment method details */
    details_paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
						    _("Payment method details"));
    gtk_widget_set_sensitive ( details_paddingbox, FALSE );

    /* Payment method name */
    table = gtk_table_new ( 3, 3, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX ( details_paddingbox ), table,
			 TRUE, TRUE, 6 );

    label = gtk_label_new ( _("Name: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    payment_name_entry = gsb_autofunc_entry_new ( NULL,
                        G_CALLBACK (gsb_payment_method_config_name_changed),
                        payment_method_treeview,
                        G_CALLBACK (gsb_data_payment_set_name), 0 );
    gtk_widget_set_size_request ( payment_name_entry, width_entry, -1 );
    gtk_table_attach ( GTK_TABLE ( table ),
		       payment_name_entry, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* button show entry, automatic numbering button will be shown only if entry is showed */
    button_show_entry = gsb_autofunc_checkbutton_new ( _("Need entry field"),
						       FALSE,
						       G_CALLBACK (gsb_payment_method_config_show_entry_changed), payment_method_treeview,
						       G_CALLBACK (gsb_data_payment_set_show_entry), 0 );
    gtk_table_attach ( GTK_TABLE ( table ),
		       button_show_entry, 2, 3, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );


    /* Automatic numbering */
    label = gtk_label_new ( _("Automatic numbering: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    payment_last_number_entry = gsb_autofunc_spin_new ( 0,
						G_CALLBACK (gsb_payment_method_config_auto_entry_changed),
                        payment_method_treeview,
						G_CALLBACK (gsb_data_payment_set_last_number_from_int),
                        0 );
    gtk_widget_set_size_request ( payment_last_number_entry, width_entry, -1 );
    gtk_table_attach ( GTK_TABLE ( table ),
		       payment_last_number_entry, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* button automatic numbering, activate it sensitive the automatic numbering entry */
    button_auto_numbering = gsb_autofunc_checkbutton_new ( _("Activate"),
							   FALSE,
							   G_CALLBACK (gsb_payment_method_config_auto_button_changed), payment_method_treeview,
							   G_CALLBACK (gsb_data_payment_set_automatic_numbering), 0 );
    gtk_table_attach ( GTK_TABLE ( table ),
		       button_auto_numbering, 2, 3, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );

    /* Payment method method_ptr */
    label = gtk_label_new ( _("Type: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );

    /* Create menu */
    payment_sign_button = gtk_option_menu_new ();
    menu = gtk_menu_new();
    /* Neutral method_ptr */
    item = gtk_menu_item_new_with_label ( _("Neutral") );
    g_signal_connect ( G_OBJECT ( item ),
		       "activate",
		       G_CALLBACK ( gsb_payment_method_config_sign_changed ),
		       GINT_TO_POINTER (GSB_PAYMENT_NEUTRAL));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    /* Debit method_ptr */
    item = gtk_menu_item_new_with_label ( _("Debit") );
    g_signal_connect ( G_OBJECT ( item ),
		       "activate",
		       G_CALLBACK ( gsb_payment_method_config_sign_changed ),
		       GINT_TO_POINTER (GSB_PAYMENT_DEBIT));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    /* Credit method_ptr */
    item = gtk_menu_item_new_with_label ( _("Credit") );
    g_signal_connect ( G_OBJECT ( item ),
		       "activate",
		       G_CALLBACK ( gsb_payment_method_config_sign_changed ),
		       GINT_TO_POINTER (GSB_PAYMENT_CREDIT));
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    /* Set menu */
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( payment_sign_button ), menu );
    gtk_table_attach ( GTK_TABLE ( table ),
		       payment_sign_button, 1, 3, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /** Do not set this tab sensitive is no account file is opened. */
    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}


/**
 * Fill the `model' GtkTreeModel with all payment methods known.  They
 * are organized by account and then my method_ptr of payment method: Debit,
 * Credit, Neutral.
 *
 *  \param model
 *
 *  \return
 */
void gsb_payment_method_config_fill_list ( GtkTreeModel *model)
{
    GSList *list_tmp;

    gtk_tree_store_clear ( GTK_TREE_STORE (model) );

    /* Fill tree, iter over with accounts */
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint account_number;
	GSList *payment_list;
	GtkTreeIter account_iter, debit_iter, credit_iter;

	account_number = gsb_data_account_get_no_account ( list_tmp -> data );

	gtk_tree_store_append (GTK_TREE_STORE (model), &account_iter, NULL);
	gtk_tree_store_set (GTK_TREE_STORE (model), &account_iter,
			    PAYMENT_METHODS_NAME_COLUMN, gsb_data_account_get_name (account_number),
			    PAYMENT_METHODS_NUMBERING_COLUMN, NULL,
			    PAYMENT_METHODS_TYPE_COLUMN, 0,
			    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			    PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE,
			    PAYMENT_METHODS_VISIBLE_COLUMN, FALSE,
			    PAYMENT_METHODS_NUMBER_COLUMN, NULL,
			    PAYMENT_METHODS_ACCOUNT_COLUMN, account_number,
			    -1 );

	/* Create the "Debit" node */
	gtk_tree_store_append (GTK_TREE_STORE (model), &debit_iter, &account_iter);
	gtk_tree_store_set (GTK_TREE_STORE (model), &debit_iter,
			    PAYMENT_METHODS_NAME_COLUMN, _("Debit"),
			    PAYMENT_METHODS_NUMBERING_COLUMN, NULL,
			    PAYMENT_METHODS_TYPE_COLUMN, 0,
			    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			    PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE,
			    PAYMENT_METHODS_VISIBLE_COLUMN, FALSE,
			    PAYMENT_METHODS_NUMBER_COLUMN, NULL,
			    PAYMENT_METHODS_ACCOUNT_COLUMN, account_number,
			    -1 );

	/* Create the "Credit" node */
	gtk_tree_store_append (GTK_TREE_STORE (model), &credit_iter, &account_iter);
	gtk_tree_store_set (GTK_TREE_STORE (model), &credit_iter,
			    PAYMENT_METHODS_NAME_COLUMN, _("Credit"),
			    PAYMENT_METHODS_NUMBERING_COLUMN, NULL,
			    /* This is a hack: account number is put in
			       Debit/Credit nodes */
			    PAYMENT_METHODS_TYPE_COLUMN, 0,
			    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			    PAYMENT_METHODS_ACTIVABLE_COLUMN, FALSE,
			    PAYMENT_METHODS_VISIBLE_COLUMN, FALSE,
			    PAYMENT_METHODS_NUMBER_COLUMN, NULL,
			    PAYMENT_METHODS_ACCOUNT_COLUMN, account_number,
			    -1 );


	/* Iter over account payment methods */
	payment_list = gsb_data_payment_get_payments_list ();
	while (payment_list)
	{
	    gint payment_number;
	    GtkTreeIter *parent_iter = NULL;
	    GtkTreeIter method_iter;
	    gboolean isdefault;
	    const gchar *number;

	    payment_number = gsb_data_payment_get_number (payment_list -> data);

	    /* check if we have to show this payment here */
	    if (gsb_data_payment_get_account_number (payment_number) != account_number)
	    {
		payment_list = payment_list -> next;
		continue;
	    }

	    /* set if default value */
	    if ( payment_number == gsb_data_account_get_default_debit (account_number)
		 ||
		 payment_number == gsb_data_account_get_default_credit (account_number) )
		isdefault = 1;
	    else
		isdefault = 0;

	    /* look for the parent iter according to the sign */
	    switch (gsb_data_payment_get_sign (payment_number))
	    {
		case GSB_PAYMENT_NEUTRAL:
		    parent_iter = &account_iter;
		    break;
		case GSB_PAYMENT_DEBIT:
		    parent_iter = &debit_iter;
		    break;
		case GSB_PAYMENT_CREDIT:
		    parent_iter = &credit_iter;
		    break;
	    }

	    /* set the last number */
	    if ( gsb_data_payment_get_automatic_numbering (payment_number))
		number = gsb_data_payment_get_last_number ( payment_number );
	    else
		number = g_strdup("");

	    /* Insert a child node */
	    gtk_tree_store_append (GTK_TREE_STORE (model), &method_iter, parent_iter);
	    gtk_tree_store_set (GTK_TREE_STORE (model), &method_iter,
				PAYMENT_METHODS_NAME_COLUMN, gsb_data_payment_get_name (payment_number),
				PAYMENT_METHODS_NUMBERING_COLUMN, number,
				PAYMENT_METHODS_TYPE_COLUMN, gsb_data_payment_get_sign (payment_number),
				PAYMENT_METHODS_DEFAULT_COLUMN, isdefault,
				PAYMENT_METHODS_ACTIVABLE_COLUMN, gsb_data_payment_get_sign (payment_number) != 0,
				PAYMENT_METHODS_VISIBLE_COLUMN, gsb_data_payment_get_sign (payment_number) != 0,
				PAYMENT_METHODS_NUMBER_COLUMN, payment_number,
				PAYMENT_METHODS_ACCOUNT_COLUMN, account_number,
				-1 );

	    payment_list = payment_list -> next;
	}
	list_tmp = list_tmp -> next;
    }
}


/**
 * Callback used when a payment method is selected in payment methods
 * list.
 *
 * \param selection the tree selection
 * \param tree_view
 *
 * \return FALSE
 */
gboolean gsb_payment_method_config_select ( GtkTreeSelection *selection,
					    GtkWidget *tree_view )
{
    GtkTreeIter iter;
    gboolean good;

    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    if (good)
    {
	GtkTreeModel *model;
	gint payment_number;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	gtk_tree_model_get (model, &iter,
			    PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			    -1 );

	if (payment_number )
	{
	    gtk_widget_set_sensitive ( details_paddingbox, TRUE );

	    /* Filling entries */
	    gsb_autofunc_entry_set_value ( payment_name_entry,
					   gsb_data_payment_get_name (payment_number),
					   payment_number );
	    gsb_autofunc_spin_set_value ( payment_last_number_entry,
					  gsb_data_payment_get_last_number_to_int ( payment_number ),
					  payment_number );
	    gsb_autofunc_checkbutton_set_value ( button_show_entry,
						 gsb_data_payment_get_show_entry (payment_number),
						 payment_number );
	    gsb_autofunc_checkbutton_set_value ( button_auto_numbering,
						 gsb_data_payment_get_automatic_numbering (payment_number),
						 payment_number );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( payment_sign_button ),
					  gsb_data_payment_get_sign (payment_number));
	    /* Activating widgets */
	    gtk_widget_set_sensitive ( button_auto_numbering,
				       gsb_data_payment_get_show_entry (payment_number));
	    gtk_widget_set_sensitive ( payment_last_number_entry,
				       gsb_data_payment_get_automatic_numbering (payment_number));
	    /* We can remove this entry */
	    gtk_widget_set_sensitive ( payment_remove_button, TRUE );
	}
	else
	{
	    /* Blanking entries */
	    gsb_autofunc_entry_set_value ( payment_name_entry, NULL, 0);
	    gsb_autofunc_spin_set_value ( payment_last_number_entry, 0, 0 );
	    gsb_autofunc_checkbutton_set_value ( button_show_entry, FALSE, 0 );
	    gsb_autofunc_checkbutton_set_value ( button_auto_numbering, FALSE, 0 );
	    /* Some widgets are useless */
	    gtk_widget_set_sensitive ( button_auto_numbering, FALSE );
	    gtk_widget_set_sensitive ( payment_last_number_entry, FALSE );
	    /* We set menu to "Neutral" as a default*/
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( payment_sign_button ), 0);
	    /* Nothing to remove */
	    gtk_widget_set_sensitive ( payment_remove_button, TRUE );

	    gtk_widget_set_sensitive ( details_paddingbox, FALSE );
	}
    }
    else
    {
	/* Blanking entries */
	gsb_autofunc_entry_set_value ( payment_name_entry, NULL, 0);
	gsb_autofunc_spin_set_value ( payment_last_number_entry, 0, 0 );
	gsb_autofunc_checkbutton_set_value ( button_show_entry, FALSE, 0 );
	gsb_autofunc_checkbutton_set_value ( button_auto_numbering, FALSE, 0 );
	/* Some widgets are useless */
	gtk_widget_set_sensitive ( button_auto_numbering, FALSE );
	gtk_widget_set_sensitive ( payment_last_number_entry, FALSE );
	/* We set menu to "Neutral" as a default*/
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( payment_sign_button ), 0);
	/* Nothing to remove */
	gtk_widget_set_sensitive ( payment_remove_button, TRUE );

	gtk_widget_set_sensitive ( details_paddingbox, FALSE );
    }
   return ( FALSE );
}


/**
 * This is a GtkTreeModelForeachFunc function.  It is used together
 * with gtk_tree_model_foreach() to search for `data' in the list.
 * Upon success, gsb_payment_method_config_foreach_select() also scrolls on current entry.
 *
 * \param model A pointer to the GtkTreeModel of the list.
 * \param path A GtkTreePath set to the current entry.
 * \param iter A GtkTreeIter set to the current entry.
 * \param payment_number_data The payment number to match with current entry.
 *
 * \returns TRUE on success, which means gtk_tree_model_foreach() will
 *          stop browsing the tree.  FALSE otherwise.
 */
gboolean gsb_payment_method_config_foreach_select ( GtkTreeModel *model, GtkTreePath *path,
						    GtkTreeIter *iter, gint *payment_number_data )
{
    GtkTreeSelection *selection;
    gint payment_number;

    gtk_tree_model_get ( GTK_TREE_MODEL(model), iter,
			 PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			 -1 );

    if (payment_number == GPOINTER_TO_INT (payment_number_data))
    {
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
	gtk_tree_selection_select_iter (selection, iter);
	gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW(payment_method_treeview), path, NULL,
				       TRUE, 0.5, 0);
	return TRUE;
    }

    return FALSE;
}


/**
 * Callback called when toggle the default payment method
 *
 * \param cell
 * \param path_str
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_payment_method_config_toggled ( GtkCellRendererToggle *cell,
					     gchar *path_str,
					     GtkTreeView *tree_view )
{
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter, parent, child;
    gboolean toggle_item;
    gint payment_number;
    GtkTreeModel *model;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    /* get toggled iter */
    gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter,
			PAYMENT_METHODS_DEFAULT_COLUMN, &toggle_item,
			PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			-1);

    if (gsb_data_payment_get_sign (payment_number) == GSB_PAYMENT_DEBIT)
	gsb_data_account_set_default_debit ( gsb_data_payment_get_account_number (payment_number),
					     payment_number );
    else
    {
	if (gsb_data_payment_get_sign (payment_number) == GSB_PAYMENT_CREDIT)
	    gsb_data_account_set_default_credit (gsb_data_payment_get_account_number (payment_number),
						 payment_number );
    }

    if (! toggle_item)
    {
	gtk_tree_model_iter_parent ( GTK_TREE_MODEL(model), &parent, &iter );

	if ( gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &parent) )
	{
	    do
	    {
		gtk_tree_store_set (GTK_TREE_STORE (model), &child,
				    PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
				    -1);
	    }
	    while ( gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &child) );
	}
	else
	{
	    /* Should not happen theorically */
	    dialogue_error_brain_damage () ;
	}

	/* set new value */
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			    PAYMENT_METHODS_DEFAULT_COLUMN, 1,
			    -1);
    }

    /* clean up */
    gtk_tree_path_free (path);
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
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
gboolean gsb_payment_method_config_name_changed ( GtkWidget *entry,
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
	gint payment_number;

	model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			     -1 );

	if (payment_number)
	{
	    /* update the tree view */
	    gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
				PAYMENT_METHODS_NAME_COLUMN, gsb_data_payment_get_name (payment_number),
				-1);
	    gsb_reconcile_sort_config_fill ();


	    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_TYPE ))
	    {
		GtkWidget *widget;
		gint account_number;

		account_number = gsb_form_get_account_number ();
		widget = gsb_form_widget_get_widget (TRANSACTION_FORM_TYPE);

		if (widget)
		{
		    gsb_payment_method_create_combo_list ( widget,
							   GSB_PAYMENT_DEBIT,
							   account_number, 0, FALSE );

		    if (GTK_WIDGET_VISIBLE (widget))
		    {
			gsb_payment_method_set_combobox_history ( widget,
								  gsb_data_account_get_default_debit (account_number), TRUE );
		    }
		    else
		    {
			gtk_widget_hide ( gsb_form_widget_get_widget (TRANSACTION_FORM_CHEQUE));
		    }
		}
	    }
	    /* update the transactions list */
	    transaction_list_update_element (ELEMENT_PAYMENT_TYPE);
	}
    }
    return FALSE;
}


/**
 * Callback called when show entry  is activated or
 * deactivated for current payment method.  It activates or
 * deactivates the automatic numbering button
 *
 * \param button the checkbutton wich send the signal
 * \param tree_view
 *
 * \return FALSE
 */
gboolean gsb_payment_method_config_show_entry_changed ( GtkWidget *button,
							GtkWidget *tree_view )
{
    GtkTreeSelection * selection;
    gboolean good;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
	GtkTreeModel *model;
	gint payment_number;

	model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			     -1 );
	if (payment_number )
	    gtk_widget_set_sensitive ( button_auto_numbering,
				       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (button_show_entry)));
    }
    return FALSE;
}


/**
 * Callback called when automatic numbering is activated or
 * deactivated for current payment method.  It activates or
 * deactivates the "current number" field and updates display of
 * current number in the list.
 *
 * \param button the checkbutton wich send the signal
 * \param tree_view
 *
 * \return FALSE
 */
gboolean gsb_payment_method_config_auto_button_changed ( GtkWidget *button,
							 GtkWidget *tree_view )
{
    GtkTreeSelection * selection;
    gboolean good;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
	GtkTreeModel *model;
	gint payment_number;

	model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
	gtk_tree_model_get ( GTK_TREE_MODEL(model),
                    &iter,
			        PAYMENT_METHODS_NUMBER_COLUMN,
                    &payment_number,
			        -1 );

	if (payment_number)
	{
	    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_auto_numbering )))
	    {
            const gchar* tmpstr;

            gtk_widget_set_sensitive ( payment_last_number_entry, TRUE );
            tmpstr = gsb_data_payment_get_last_number ( payment_number );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                            &iter,
                            PAYMENT_METHODS_NUMBERING_COLUMN,
                            tmpstr,
                            -1);
	    }
	    else
	    {
		gtk_widget_set_sensitive ( payment_last_number_entry, FALSE );
		gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
				    PAYMENT_METHODS_NUMBERING_COLUMN, NULL,
				    -1);
	    }
	}
    }
    return FALSE;
}


/**
 * Callback called when the GtkSpinButton corresponding to the last
 * automatic number of the payment method is changed.  It updates the
 * list.
 *
 * \param spin_button
 * \param tree_view
 *
 * \return FALSE
 *
 */
gboolean gsb_payment_method_config_auto_entry_changed ( GtkWidget *spin_button,
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
	gint payment_number;

	model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			     -1 );
	if (payment_number)
	{
        const gchar* tmpstr;

        tmpstr = gsb_data_payment_get_last_number ( payment_number );
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                        PAYMENT_METHODS_NUMBERING_COLUMN,
                        tmpstr,
                        -1);
	}
    }
    return FALSE;
}



/**
 * This function looks for a method of payment which matches
 * `signe_type' and returns its number.
 *
 * \param account_number	The account to process.
 * \param signe_type	A payment method sign to match against payment
 *			methods of specified account.
 *			(GSB_PAYMENT_NEUTRAL, GSB_PAYMENT_DEBIT, GSB_PAYMENT_CREDIT)
 * \param exclude	A payment method that should not be checked,
 *			as it may be the one which is to be removed.
 *
 * \return		The matching payment method number on success,
 *			0 otherwise (for transfer).
 */
gint gsb_payment_method_config_get_transaction_by_sign ( gint account_number,
							 gint signe_type,
							 gint exclude )
{
    GSList *tmp_list;

    tmp_list = gsb_data_payment_get_payments_list ();

    while (tmp_list)
    {
	gint payment_number;

	payment_number = gsb_data_payment_get_number (tmp_list -> data);

	if ( gsb_data_payment_get_account_number (payment_number) == account_number
	     &&
	     payment_number != exclude
	     &&
	     gsb_data_payment_get_sign (payment_number) == signe_type )
	    return payment_number;

	tmp_list = tmp_list -> next;
    };

    /* Defaults to first method_ptr, whatever it may be */
    return 0;
}


/**
 * callback called when changing the sign type of the method of payment
 *
 * \param menu_item the widget wich receive the signal
 * \param sign a pointer to an int : GSB_PAYMENT_NEUTRAL, GSB_PAYMENT_DEBIT or GSB_PAYMENT_CREDIT
 *
 * \return FALSE
 */
gboolean gsb_payment_method_config_sign_changed ( GtkWidget *menu_item,
						  gint *sign )
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (payment_method_treeview));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
	GtkTreeModel *model;
	gint payment_number;

	model = gtk_tree_view_get_model ( GTK_TREE_VIEW (payment_method_treeview));
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			     -1 );

	if (payment_number)
	{
	    gint account_number;

	    /* Call this callback so that we "unselect" things */
	    gsb_payment_method_config_select ( selection, payment_method_treeview);

	    account_number = gsb_data_payment_get_account_number (payment_number);

	    /* as we have changed the sign of the method of payment, check if it was the default
	     * for the account, and if yes, change the default for that account */
	    switch (gsb_data_payment_get_sign (payment_number))
	    {
		case GSB_PAYMENT_DEBIT:
		    if ( gsb_data_account_get_default_debit (account_number) == payment_number)
		    {
			/* the current method of payment was a debit and was the default debit for its account,
			 * so change the default to another debit */
			gsb_data_account_set_default_debit ( account_number,
							     gsb_payment_method_config_get_transaction_by_sign (account_number,
											  GSB_PAYMENT_DEBIT,
											  payment_number));
		    }
		    break;

		case GSB_PAYMENT_CREDIT:
		    if ( gsb_data_account_get_default_credit (account_number) == payment_number)
		    {
			/* the current method of payment was a credit and was the default credit for its account,
			 * so change the default to another credit */
			gsb_data_account_set_default_credit ( account_number,
							      gsb_payment_method_config_get_transaction_by_sign (account_number,
											   GSB_PAYMENT_CREDIT,
											   payment_number));
		    }
		    break;
	    }
	    gsb_data_payment_set_sign ( payment_number,
					GPOINTER_TO_INT (sign));

	    /* Update tree */
	    g_signal_handlers_block_by_func ( selection,
					      G_CALLBACK (gsb_payment_method_config_select),
					      model );
	    gsb_payment_method_config_fill_list (model);
	    gtk_tree_view_expand_all ( GTK_TREE_VIEW(payment_method_treeview) );
	    g_signal_handlers_unblock_by_func ( selection,
						G_CALLBACK (gsb_payment_method_config_select),
						model );
	    gtk_tree_model_foreach ( GTK_TREE_MODEL (model),
				     (GtkTreeModelForeachFunc) gsb_payment_method_config_foreach_select,
				     GINT_TO_POINTER (payment_number));
	    /* need to clear and fill the reconciliation tree becaus if it was a neutral changing to credit/debit
	     * and neutral was split... */
	    gsb_reconcile_sort_config_fill ();
	}
    }
    return FALSE;
}


/**
 * callback called by the button to add a new method of payment
 *
 * \param button
 * \param tree_view the payment method config tree view
 *
 * \return FALSE
 * */
gboolean gsb_payment_method_config_add ( GtkWidget *button,
					 GtkWidget *tree_view )
{
    GtkTreeSelection * selection;
    GtkTreeIter iter, parent, root, child, *final;
    GtkTreePath * treepath;
    gint account_number, type_final;
    gboolean good;
    gint payment_number;
    GtkTreeModel *model;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    if ( good )
    {
	/* there is a selection, get corrent iters */
	gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			     PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			     -1 );
	if (payment_number)
	{
	    /* Select parent */
	    gtk_tree_model_iter_parent ( GTK_TREE_MODEL(model),
					 &parent, &iter );
	    final = &parent;
	    type_final = gsb_data_payment_get_sign (payment_number);
	    gtk_tree_model_iter_parent (GTK_TREE_MODEL(model), &root, &parent);
	}
	else
	{
	    gchar *name;

	    gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
				 PAYMENT_METHODS_NAME_COLUMN, &name,
				 -1 );

	    if (gtk_tree_model_iter_parent (GTK_TREE_MODEL(model), &root, &iter))
	    {
		/* We are on "Credit" or "Debit" or "Neutral" */
		final = &iter;
		if ( !strcmp(name, _("Credit")) )
		{
		    type_final = GSB_PAYMENT_CREDIT;
		}
		else if ( !strcmp(name, _("Debit")) )
		{
		    type_final = GSB_PAYMENT_DEBIT;
		}
		else 		/* Neutral */
		{
		    type_final = GSB_PAYMENT_NEUTRAL;
		}
	    }
	    else
	    {
		/* We are on an account, method_ptr will be the same as the
		   first node  */
		if (!gtk_tree_model_iter_children( GTK_TREE_MODEL(model),
						   &child, &iter ))
		    /* Should not happen! */
		    dialogue_error_brain_damage ();

		final = &child;
		type_final = GSB_PAYMENT_DEBIT;
	    }
	}
    }
    else
    {
	/* No selection, we use first account, first method*/
	gtk_tree_model_get_iter_first (GTK_TREE_MODEL(model), &iter);
	gtk_tree_model_iter_children( GTK_TREE_MODEL(model),
				      &child, &iter );
	final = &child;
	type_final = GSB_PAYMENT_DEBIT;

    }

    /* final is now set on debit or credit line where we want to set the method of payment */
    gtk_tree_model_get ( GTK_TREE_MODEL(model), final,
			 PAYMENT_METHODS_ACCOUNT_COLUMN, &account_number,
			 -1);

    /* create the new method of payment */
    payment_number = gsb_data_payment_new ( _("New payment method") );
    gsb_data_payment_set_sign ( payment_number,
				type_final );
    gsb_data_payment_set_account_number ( payment_number,
					  account_number );

    /* append it to the store */
    gtk_tree_store_append (GTK_TREE_STORE (model), &iter, final);
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			PAYMENT_METHODS_NAME_COLUMN, gsb_data_payment_get_name (payment_number),
			PAYMENT_METHODS_NUMBERING_COLUMN, NULL,
			PAYMENT_METHODS_TYPE_COLUMN, type_final,
			PAYMENT_METHODS_DEFAULT_COLUMN, FALSE,
			PAYMENT_METHODS_ACTIVABLE_COLUMN, type_final != 0,
			PAYMENT_METHODS_VISIBLE_COLUMN, type_final != 0,
			PAYMENT_METHODS_NUMBER_COLUMN, payment_number,
			-1 );

    /* Select and view new position */
    gtk_tree_selection_select_iter ( selection, &iter );
    treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(model), &iter );
    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW(tree_view), treepath, NULL,
				   TRUE, 0.5, 0);
    gtk_tree_path_free ( treepath );

    /* add to the sorted list */
    gsb_data_account_sort_list_add ( account_number,
				     payment_number );
    gsb_reconcile_sort_config_fill ();

    /* Mark file as modified */
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * callback called by a clicked signal on the removing button
 *
 * \param button
 * \param tree_view
 *
 * \return FALSE
 *
 */
gboolean gsb_payment_method_config_remove ( GtkWidget *button,
					    GtkWidget *tree_view )
{
    gint payment_number;
    gint account_number;
    GtkTreeSelection * selection;
    GtkTreeIter iter;
    gboolean good;
    GSList *list_tmp_transactions;
    GtkTreeModel *model;

    /** First, we find related GtkTreeIter and stsruct_type_ope pointer. */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if ( !good )
	return FALSE;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_tree_model_get ( GTK_TREE_MODEL(model), &iter,
			 PAYMENT_METHODS_NUMBER_COLUMN, &payment_number,
			 -1 );
    if ( !payment_number )
	return FALSE;

    /* we check if some transactions have this method of payment,
     * if yes, we propose to switch to another method of payment or cancel */

    /* first, check if there is some transactions */
    list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
    while (list_tmp_transactions)
    {
	gint transaction_number;
	transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_method_of_payment_number (transaction_number) == payment_number)
	{
	    /* ok, there is some transactions with that method of payment, we switch to another one or cancel */
	    if ( !gsb_payment_method_config_switch_payment (payment_number))
		return FALSE;
	    list_tmp_transactions = NULL;
	}
	else
	    list_tmp_transactions = list_tmp_transactions -> next;
    }

    account_number = gsb_data_payment_get_account_number (payment_number);

    /* If it was a default, change default */
    switch (gsb_data_payment_get_sign (payment_number))
    {
	case GSB_PAYMENT_DEBIT:
	    if ( gsb_data_account_get_default_debit (account_number) == payment_number)
	    {
		gsb_data_account_set_default_debit ( account_number,
						     gsb_payment_method_config_get_transaction_by_sign (account_number,
										  GSB_PAYMENT_DEBIT,
										  payment_number));
	    }
	    break;

	case GSB_PAYMENT_CREDIT:
	    if ( gsb_data_account_get_default_credit (account_number) == payment_number)
	    {
		gsb_data_account_set_default_credit ( account_number,
						      gsb_payment_method_config_get_transaction_by_sign (account_number,
										   GSB_PAYMENT_CREDIT,
										   payment_number));
	    }
	    break;
    }

    /* remove that method of payment from the sort list */
    gsb_data_account_sort_list_remove ( account_number,
					payment_number );

    /* Remove the method of payment from tree & memory */
    gtk_tree_store_remove ( GTK_TREE_STORE(model), &iter );
    gsb_data_payment_remove (payment_number);

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * called to switch the transactions from a method of payment to another
 * popup a dialog to ask for what method of payment we want to move
 *
 * \param payment_number the method of payment we come from
 *
 * \return TRUE done, FALSE cancel
 * */
gboolean gsb_payment_method_config_switch_payment ( gint payment_number )
{
    GtkWidget *dialog, *label, * combo_box, *hbox, *paddingbox;
    gint resultat;
    gint new_payment_number;
    GSList *tmp_list;
    gint account_number;

    dialog = gtk_dialog_new_with_buttons ( _("Delete a payment method"),
					   GTK_WINDOW ( fenetre_preferences ),
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), FALSE );

    /* Ugly dance to avoid side effects on dialog's vbox. */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 0 );
    paddingbox = new_paddingbox_with_title ( hbox, TRUE, _("Payment method is used by transactions") );
    gtk_container_set_border_width ( GTK_CONTAINER(hbox), 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(paddingbox), 6 );
    gtk_widget_show_all ( hbox );

    /* create first the combo_box with the possible target to the method of payment,
     * if no target, say it in the label and let that function choose if it has to be shown or not */
    account_number = gsb_data_payment_get_account_number (payment_number);
    combo_box = gtk_combo_box_new ();
    if (!gsb_payment_method_create_combo_list ( combo_box,
						gsb_data_payment_get_sign (payment_number),
						account_number,
						payment_number, FALSE ))
    {
	dialogue_error ( _("Problem while filling the target method of payment to delete it.\nThis is not normal, please contact the grisbi team."));
	return FALSE;
    }

    if (GTK_WIDGET_VISIBLE (combo_box))
	label = gtk_label_new ( _("Some transactions still use that payment method.  As deletion of this method is irreversible, these transactions are to be associated with another payment method."));
    else
	label = gtk_label_new (_("No target method of payment to switch the transactions to another method of payment. If you continue, the transactions with this method of payment will have no new one."));
    gtk_label_set_line_wrap ( GTK_LABEL (label), TRUE );
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
    gtk_widget_set_size_request ( label, 400, -1 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), label, FALSE, FALSE, 0 );
    gtk_widget_show (label);

    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Associate transactions with: "));
	gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, TRUE, TRUE, 0 );

    /* the combo_box was created before */
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo_box, TRUE, TRUE, 0 );

    if (GTK_WIDGET_VISIBLE (combo_box))
    gtk_widget_show_all ( hbox );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat != GTK_RESPONSE_OK )
    {
	if ( GTK_IS_DIALOG ( dialog ))
	    gtk_widget_destroy ( GTK_WIDGET(dialog) );
	return FALSE;
    }

    /* Find new method of payment */
    new_payment_number = gsb_payment_method_get_selected_number (combo_box);

    /* Then, we change the method of payment for related operations. */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
	gint transaction_number;
	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	if ( gsb_data_transaction_get_method_of_payment_number (transaction_number) == payment_number)
	    gsb_data_transaction_set_method_of_payment_number (transaction_number, new_payment_number);
	tmp_list = tmp_list -> next;
    }

    /* change too for the scheduled transactions */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();
    while (tmp_list)
    {
	gint scheduled_number;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list -> data);

	if ( gsb_data_scheduled_get_method_of_payment_number (scheduled_number) == payment_number)
	    gsb_data_scheduled_set_method_of_payment_number ( scheduled_number, new_payment_number);

	tmp_list = tmp_list -> next;
    }

    /* change for the transactions tree_view */
    transaction_list_update_element (ELEMENT_PAYMENT_TYPE);

    gtk_widget_destroy ( GTK_WIDGET(dialog) );
    return TRUE;
}



