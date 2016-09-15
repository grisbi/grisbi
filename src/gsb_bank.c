/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cedric Auger (cedric@grisbi.org)	          */
/*			2003-2009 Benjamin Drieu (bdrieu@april.org)	                      */
/*      2009 Thomas Peel (thomas.peel@live.fr)                                */
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
 * \file gsb_bank.c
 * do the bank configuration
 * do the bank combobox
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_bank.h"
#include "dialog.h"
#include "gsb_account_property.h"
#include "gsb_autofunc.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_transactions_list.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_editables.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_bank_add ( GtkWidget *button,
                        gpointer null );
static void gsb_bank_bic_code_changed ( GtkEntry *entry, gpointer data );
static void gsb_bank_code_changed ( GtkEntry *entry, gpointer data );
static gboolean gsb_bank_create_combobox_model ( void );
static GtkWidget *gsb_bank_create_form ( GtkWidget *parent,
                        GtkWidget *combobox );
static gboolean gsb_bank_delete ( GtkWidget *button,
                        gpointer null );
static gboolean gsb_bank_edit_bank ( gint bank_number,
                        GtkWidget *combobox );
static gboolean gsb_bank_list_change_selection ( GtkTreeSelection *selection,
                        GtkWidget *container );
static gboolean gsb_bank_list_changed ( GtkWidget *combobox,
                        gpointer null );
static gboolean gsb_bank_list_check_separator ( GtkTreeModel *model,
                        GtkTreeIter *iter,
						gpointer null );
static void gsb_bank_update_bank_data ( gint bank_number );
static gboolean gsb_bank_update_form ( gint bank_number,
                        GtkWidget *frame );
static gboolean gsb_bank_update_selected_line ( GtkEntry *entry,
						GtkWidget *combobox );
static gboolean gsb_bank_update_selected_line_model ( GtkWidget *combobox );
/*END_STATIC*/

static GtkWidget *bank_list_tree_view;
enum bank_list_col
{
   BANK_VIEW_NAME_COL = 0,
   BANK_TALKER_COL,
   BANK_NUMBER,
   BANK_LIST_COL_NB
};


/* the model containing the list of the banks for the combobox */
GtkTreeModel *bank_list_model = NULL;

enum bank_list_model_col
{
   BANK_NAME_COL = 0,
   BANK_NUMBER_COL
};


/* entries of the form */
static GtkWidget *delete_bank_button;
static GtkWidget *bank_name;
static GtkWidget *bank_code;
static GtkWidget *bank_BIC;
static GtkWidget *bank_adr;
static GtkWidget *bank_tel;
static GtkWidget *bank_mail;
static GtkWidget *bank_web;
static GtkWidget *bank_contact_name;
static GtkWidget *bank_contact_tel;
static GtkWidget *bank_contact_mail;
static GtkWidget *bank_contact_fax;
static GtkWidget *bank_notes;


/*START_EXTERN*/
/*END_EXTERN*/




/* ----------------------- The first part is about the combobox containing the list of the banks in the property of accounts ---------------- */


/**
 * create a combo_box with the name of all the banks and an 'add' button at the end
 * update too if filled the value of the associated account (see gsb_autofunc_... for more explains)
 *
 * \param bank the index we want to place the combobox
 * \param hook an optional function to execute if the combobox is modified
 * \param data an optional pointer to pass to hook
 * \param default_func a function to change the value in memory (should be gsb_data_account_set_bank or NULL)
 * \param number_for_func the gint to send to the default_func (should be the account number)
 *
 * \return GtkWidget * the combo_box
 * */
GtkWidget *gsb_bank_create_combobox ( gint index )
{
    GtkWidget *combo_box;
    GtkCellRenderer *renderer;

    /* create the model if not done */
    if ( !bank_list_model )
        gsb_bank_update_selected_line_model ( NULL );

    combo_box = gtk_combo_box_new_with_model ( bank_list_model );

    /* show the text column */
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), renderer,
				    "text", 0,
				    NULL);

    /* create the separator */
    gtk_combo_box_set_row_separator_func ( GTK_COMBO_BOX (combo_box),
					   (GtkTreeViewRowSeparatorFunc) gsb_bank_list_check_separator,
					   NULL, NULL );

    /* set the index */
    gsb_bank_list_set_bank ( combo_box, index );

    /* the signal just check if we select new bank, to show the dialog to add a new bank */
    g_signal_connect ( G_OBJECT (combo_box),
		       "changed",
		       G_CALLBACK ( gsb_bank_list_changed ),
		       NULL );

    return combo_box;
}


/**
 * return the number of the curently showed on the combobox
 *
 * \param combobox
 *
 * \return the bank number, 0 for none, -1 for 'new bank'
 * */
gint gsb_bank_list_get_bank_number ( GtkWidget *combobox )
{
    GtkTreeIter iter;
    gint bank_number;

    if (!combobox)
	return -1;

    if (!gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (combobox),
					 &iter))
	return -1;

    gtk_tree_model_get ( GTK_TREE_MODEL (bank_list_model),
			 &iter,
			 BANK_NUMBER_COL, &bank_number,
			 -1 );
    return bank_number;
}


/**
 * show the current bank in the combobox acording to the bank number
 *
 * \param combobox
 * \param bank_number 0 for none, -1 for new bank
 * \param number_for_func the number to give to the default_func when something changed
 *
 * \return FALSE
 * */
gboolean gsb_bank_list_set_bank ( GtkWidget *combobox,
                        gint bank_number )
{
    GtkTreeIter iter;

    if ( !combobox )
        return FALSE;

    if ( !gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( bank_list_model ), &iter ) )
        return FALSE;

    /* on bloque la fonction de callback */
    g_signal_handlers_block_by_func ( G_OBJECT ( combobox ),
                        gsb_bank_list_changed,
                        NULL );

    do
    {
        gint current_bank;

        gtk_tree_model_get ( GTK_TREE_MODEL ( bank_list_model ),
                        &iter,
                        BANK_NUMBER_COL, &current_bank,
                        -1 );

        if ( bank_number == current_bank )
        {
            /* bank found */
            gtk_combo_box_set_active_iter ( GTK_COMBO_BOX ( combobox ), &iter );
            break;
        }
    }
    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( bank_list_model ), &iter ) );

    /* on débloque la fonction de callback */
    g_signal_handlers_unblock_by_func ( G_OBJECT ( combobox ),
                        gsb_bank_list_changed,
                        NULL );

    return FALSE;
}


/**
 * callback called when click on the edit-bank in the property of account
 * show a window with the parameters of the bank in the combobox
 *
 * \param button the button wich was clicked
 * \param combobox the combobox selected a bank
 *
 * \return FALSE
 * */
gboolean gsb_bank_edit_from_button ( GtkWidget *button,
                        GtkWidget *combobox )
{
    gint bank_number;

    bank_number = gsb_bank_list_get_bank_number ( combobox );

    /* if bank_number = 0, it's none ; -1 : it's new bank, so don't edit */
    if ( bank_number <= 0 )
	return FALSE;

    gsb_bank_edit_bank ( bank_number, combobox );
    return FALSE;
}


/**
 * create the model for the combobox of banks
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
static gboolean gsb_bank_create_combobox_model ( void )
{
    bank_list_model = GTK_TREE_MODEL (gtk_list_store_new ( 2,
							   G_TYPE_STRING,
							   G_TYPE_INT ));
    return TRUE;
}


/**
 * update the model for the combobox of the banks
 * add an 'add' button at the end
 *
 * \param combobox a facultative option, if not NULL, the current selection will be saved and restored at the end of the function
 *
 * \return TRUE if ok, FALSE if problem
 * */
static gboolean gsb_bank_update_selected_line_model ( GtkWidget *combobox )
{
    GtkTreeIter iter;
    GSList *list_tmp;
    gint save_bank_number = -1;

    /* save the selection */
    if (combobox)
    {
        save_bank_number = gsb_bank_list_get_bank_number ( combobox );
    }

    /* if no bank model, create it */
    if ( !bank_list_model )
        gsb_bank_create_combobox_model ( );

    gtk_list_store_clear ( GTK_LIST_STORE ( bank_list_model ) );

    /* first : none, whith 0 at number (no bank at 0) */
    gtk_list_store_append ( GTK_LIST_STORE ( bank_list_model ), &iter );
    gtk_list_store_set ( GTK_LIST_STORE ( bank_list_model ),
                        &iter,
                        BANK_NAME_COL, _("None"),
                        BANK_NUMBER_COL, 0,
                        -1 );

    /* set the names */
    list_tmp = gsb_data_bank_get_bank_list ();

    while ( list_tmp )
    {
        gint bank_number;

        bank_number = gsb_data_bank_get_no_bank ( list_tmp->data );

        gtk_list_store_append ( GTK_LIST_STORE ( bank_list_model ), &iter );
        gtk_list_store_set ( GTK_LIST_STORE ( bank_list_model ),
                        &iter,
                        BANK_NAME_COL, gsb_data_bank_get_name ( bank_number ),
                        BANK_NUMBER_COL, bank_number,
                        -1 );
        list_tmp = list_tmp -> next;
    }

    /* separator line : the name of bank is null */
    gtk_list_store_append ( GTK_LIST_STORE ( bank_list_model ), &iter );
    gtk_list_store_set ( GTK_LIST_STORE ( bank_list_model ),
                        &iter,
                        BANK_NAME_COL, NULL,
                        -1 );

    /* item to add a bank : the number is -2 */
    gtk_list_store_append ( GTK_LIST_STORE ( bank_list_model ), &iter );
    gtk_list_store_set ( GTK_LIST_STORE ( bank_list_model ),
                        &iter,
                        BANK_NAME_COL, _("Add new bank"),
                        BANK_NUMBER_COL, -2,
                        -1 );

    /* restore the selection */
    if ( combobox )
        gsb_bank_list_set_bank ( combobox, save_bank_number );

    return TRUE;
}


/**
 * check for the separator line in the combobox (ie the name is null)
 *
 * \param model
 * \param iter
 * \param null
 *
 * \return TRUE if separator, FALSE if not
 * */
static gboolean gsb_bank_list_check_separator ( GtkTreeModel *model,
                        GtkTreeIter *iter,
						gpointer null )
{
    gchar *value;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 iter,
			 BANK_NAME_COL, &value,
			 -1 );

    if (value)
	return FALSE;

    return TRUE;
}

/**
 * called when change the bank in the list
 * here just check if we want to create a new bank
 *
 * \param combo_box
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_bank_list_changed ( GtkWidget *combobox,
                        gpointer null )
{
    gint bank_number;

    if (!combobox)
	    return FALSE;

    bank_number = gsb_bank_list_get_bank_number (combobox);

    /* check if not new bank, ie -2 */
    if ( bank_number != -2 )
    {
        gsb_data_account_set_bank ( gsb_gui_navigation_get_current_account ( ), bank_number );
        /* Mark file as modified */
        gsb_file_set_modified ( TRUE );
        return FALSE;
    }

    /* asked to add a new bank */
    bank_number = gsb_bank_edit_bank ( bank_number, combobox );

    return FALSE;
}


/* ---------------- the second part is on the parameters page of the banks ------------------------ */


/**
 * Creates the "Banks" tab.  It creates a bank list and then a form
 * that allows to edit selected bank.
 *
 * \param default_sensitive normally if there is no account, that page is unsensitive,
 * 	but for assistant, we have to force to be sensitive so set default_sensitive to TRUE
 * 	to let grisbi choose between sensitive or net, set default_sensitive to FALSE
 * \returns A newly allocated vbox
 */
GtkWidget *gsb_bank_create_page ( gboolean default_sensitive )
{
    GtkWidget *vbox_pref;
    GtkWidget *scrolled_window, *vbox, *vbox2;
    GtkWidget *button, *hbox, *paddingbox;
    GtkListStore *store;
    GtkTreeSelection *selection;
    gint i;
    gchar *titles[] = {
	_("Bank"), _("Contact name")
    };
    gfloat alignment[] = {
	COLUMN_LEFT, COLUMN_LEFT
    };
    GtkWidget *vpaned;
    GtkWidget *paned1, *paned2;

    vbox_pref = new_vbox_with_title_and_icon ( _("Banks"), "banks.png" );

    vpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start ( GTK_BOX (vbox_pref), vpaned, TRUE, TRUE, 0 );

    /* Create bank list */
    paned1 = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0);
    gtk_paned_pack1 ( GTK_PANED (vpaned), paned1,
		      FALSE, FALSE );

    paddingbox = new_paddingbox_with_title ( paned1, FALSE,
					     _("Known banks") );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 10 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0);

    /* set the store */
    store = gtk_list_store_new ( BANK_LIST_COL_NB,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_INT );
    bank_list_tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT(store));
    //~ //~ gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW (bank_list_tree_view),
				   //~ TRUE );
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			bank_list_tree_view );

    /* connect the selection */
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (bank_list_tree_view));
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION (selection),
				  GTK_SELECTION_SINGLE );

    /* set the columns */
    for (i=0 ; i< BANK_NUMBER; i++)
    {
	/* this is a text column */
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;

	cell = gtk_cell_renderer_text_new ();
	g_object_set ( G_OBJECT (cell),
		       "xalign", alignment[i],
		       NULL );

	column = gtk_tree_view_column_new ( );
	gtk_tree_view_column_set_sizing ( column,
					  GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	gtk_tree_view_column_set_alignment ( column,
					     alignment[i] );

	gtk_tree_view_column_pack_start ( column, cell, TRUE );
	gtk_tree_view_column_set_title ( column, titles[i] );
	gtk_tree_view_column_set_attributes (column, cell,
					     "text", i,
					     NULL);
	gtk_tree_view_column_set_expand ( column, TRUE );
	gtk_tree_view_column_set_resizable ( column,
					     TRUE );
	gtk_tree_view_append_column ( GTK_TREE_VIEW(bank_list_tree_view), column);
    }

    /* Do not activate unless an account is opened */
    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, default_sensitive );
    else
    {
	/* fill the list */
	GSList *tmp_list;

	tmp_list = gsb_data_bank_get_bank_list ();

	while ( tmp_list )
	{
	    gint bank_number;
	    GtkTreeIter iter;

	    bank_number = gsb_data_bank_get_no_bank (tmp_list -> data);

	    gtk_list_store_append ( GTK_LIST_STORE (store),
				    &iter );
	    gtk_list_store_set ( GTK_LIST_STORE (store),
				 &iter,
				 BANK_VIEW_NAME_COL, gsb_data_bank_get_name (bank_number),
				 BANK_TALKER_COL, gsb_data_bank_get_correspondent_name (bank_number),
				 BANK_NUMBER, bank_number,
				 -1 );
	    tmp_list = tmp_list -> next;
	}
    }

    /* Handle Add & Remove buttons */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			 FALSE, FALSE, 0 );
    /* Add button */
    button = utils_buttons_button_new_from_stock ("gtk-add", _("Add"));
    g_signal_connect ( G_OBJECT ( button ), "clicked",
		       G_CALLBACK  ( gsb_bank_add ),
		       NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );
    /* Remove button */
    delete_bank_button = utils_buttons_button_new_from_stock ("gtk-remove", _("Remove"));
    gtk_widget_set_sensitive ( delete_bank_button, FALSE );
    g_signal_connect ( G_OBJECT ( delete_bank_button ), "clicked",
		       G_CALLBACK ( gsb_bank_delete ),
		       NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), delete_bank_button, FALSE, FALSE, 5 );


    /* Add a scroll because bank details are huge */
    paned2 = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0);
    gtk_paned_pack2 ( GTK_PANED (vpaned), paned2,
		      TRUE, FALSE );

    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( paned2 ), scrolled_window,
			 TRUE, TRUE, 5 );
    vbox2 = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), vbox2 );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_NONE );
    gtk_widget_set_sensitive ( vbox2, FALSE );

    gsb_bank_create_form ( vbox2, NULL );

    /* connect the selection with vbox2, the container of the form */
    g_signal_connect ( G_OBJECT (selection),
		       "changed",
		       G_CALLBACK (gsb_bank_list_change_selection),
		       vbox2 );

    return ( vbox_pref );
}

/**
 * Fills in bank details widgets with corresponding stuff.  Make them
 * active as well.
 *
 * \param selection
 * \param container a vbox containing the form
 *
 * \return FALSE
 */
static gboolean gsb_bank_list_change_selection ( GtkTreeSelection *selection,
                        GtkWidget *container )
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected ( selection, &model, &iter))
    {
	gint bank_number;

	gtk_tree_model_get ( GTK_TREE_MODEL (model),
			     &iter,
			     BANK_NUMBER, &bank_number,
			     -1 );
	gsb_bank_update_form ( bank_number, container );
    }
    else
	gsb_bank_update_form ( -1, container );

    return FALSE;
}



/**
 * create the form containing the details of the banks
 *
 * \param parent the container
 * \param combobox NULL or a pointer to a combobox to keep it state while editing the bank
 *
 * \return the new widget
 * */
static GtkWidget *gsb_bank_create_form ( GtkWidget *parent,
                        GtkWidget *combobox )
{
    GtkWidget *paddingbox, *table, *label, *scrolled_window;
    GtkSizeGroup * size_group;

    /* Bank details */
    paddingbox = new_paddingbox_with_title ( parent, TRUE, _("Bank details") );

    /* Create a table to align things nicely */
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), 5);
    gtk_grid_set_row_spacing (GTK_GRID (table), 5);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 5 );
    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    /* Bank name item */
    label = gtk_label_new ( _("Name: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

    if ( combobox )
        bank_name = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_name = gsb_autofunc_entry_new ( NULL,
                        G_CALLBACK ( gsb_bank_update_selected_line ),
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_name ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_name );
    gtk_grid_attach (GTK_GRID (table), bank_name, 1, 0, 1, 1);

    /* Bank Sort code item */
    label = gtk_label_new ( _("Sort code: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

    if ( combobox )
        bank_code = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_code = gsb_autofunc_entry_new ( NULL,
                        G_CALLBACK ( gsb_bank_code_changed ),
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_code ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_code );
    gtk_grid_attach (GTK_GRID (table), bank_code, 1, 1, 1, 1);

    /* Bank BIC code item */
    label = gtk_label_new ( _("BIC code: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);

    if ( combobox )
        bank_BIC = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_BIC = gsb_autofunc_entry_new ( NULL,
                        G_CALLBACK ( gsb_bank_bic_code_changed ),
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_bic ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_BIC );
    gtk_grid_attach (GTK_GRID (table), bank_BIC, 1, 2, 1, 1);

    /* Bank address */
    label = gtk_label_new ( _("Address: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 4, 1, 1);
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window),
					  GTK_SHADOW_IN );

    /* Create the text view */
    if ( combobox )
        bank_adr = gsb_autofunc_textview_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_adr = gsb_autofunc_textview_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_bank_address ),
                        0 );
    gtk_widget_set_size_request ( bank_adr, -1, 70 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), bank_adr );
    gtk_size_group_add_widget ( size_group, bank_adr );
    gtk_grid_attach (GTK_GRID (table), scrolled_window, 1, 4, 1, 1 );

    /* Phone number */
    label = gtk_label_new ( _("Phone: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 5, 1, 1);

    if ( combobox )
        bank_tel = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_tel = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_bank_tel ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_tel );
    gtk_grid_attach (GTK_GRID (table), bank_tel, 1, 5, 1, 1);

    /* E-mail */
    label = gtk_label_new ( _("E-Mail: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 7, 1, 1);

    if ( combobox )
        bank_mail = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_mail = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_bank_mail ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_mail );
    gtk_grid_attach (GTK_GRID (table), bank_mail, 1, 7, 1, 1);

    /* Website */
    label = gtk_label_new ( _("Website: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 9, 1, 1);
    if ( combobox )
        bank_web = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_web = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_bank_web ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_web );
    gtk_grid_attach (GTK_GRID (table), bank_web, 1, 9, 1, 1);

    /* Contact */
    paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Contact") );
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), 5);
    gtk_grid_set_row_spacing (GTK_GRID (table), 5);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, TRUE, TRUE, 5 );

    /* Name */
    label = gtk_label_new ( _("Name: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

    if ( combobox )
        bank_contact_name = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_contact_name = gsb_autofunc_entry_new ( NULL,
                        G_CALLBACK ( gsb_bank_update_selected_line ),
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_correspondent_name ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_contact_name );
    gtk_grid_attach (GTK_GRID (table), bank_contact_name, 1, 0, 1, 1);

    /* Phone number */
    label = gtk_label_new ( _("Phone: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);


    if ( combobox )
        bank_contact_tel = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_contact_tel = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_correspondent_tel ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_contact_tel );
    gtk_grid_attach (GTK_GRID (table), bank_contact_tel, 1, 1, 1, 1);

    /* Fax */
    label = gtk_label_new ( _("Fax: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 3, 1, 1);

    if ( combobox )
        bank_contact_fax = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_contact_fax = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_correspondent_fax ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_contact_fax );
    gtk_grid_attach (GTK_GRID (table), bank_contact_fax, 1, 3, 1, 1);

    /* E-Mail */
    label = gtk_label_new ( _("E-Mail: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);

    if ( combobox )
        bank_contact_mail = gsb_autofunc_entry_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_contact_mail = gsb_autofunc_entry_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_correspondent_mail ),
                        0 );
    gtk_size_group_add_widget ( size_group, bank_contact_mail );
    gtk_grid_attach (GTK_GRID (table), bank_contact_mail, 1, 2, 1, 1);

    /* Notes */
    paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Notes") );
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window),
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window, TRUE, TRUE, 5 );

    if ( combobox )
        bank_notes = gsb_autofunc_textview_new ( NULL, NULL, NULL, NULL, 0 );
    else
        bank_notes = gsb_autofunc_textview_new ( NULL,
                        NULL,
                        NULL,
                        G_CALLBACK ( gsb_data_bank_set_bank_note ),
                        0 );
    gtk_widget_set_size_request ( bank_notes, -1, 100 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), bank_notes );

    return parent;
}


/**
 * fill the form containing the information of the bank
 *
 * \param bank_number the number to fill or -1 to erase the form
 * \param frame the widget containing the form
 *
 * \return FALSE
 * */
static gboolean gsb_bank_update_form ( gint bank_number,
                        GtkWidget *frame )
{
    gboolean value;

    /* we can let the defaults func even for -1 because with a bank_number of -1, all the gsb_data_bank_get_...
     * will return NULL, and that's we want... */
    gsb_autofunc_entry_set_value ( bank_name, gsb_data_bank_get_name (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( bank_code, gsb_data_bank_get_code (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( bank_BIC, gsb_data_bank_get_bic (bank_number), bank_number);

    gsb_autofunc_entry_set_value ( bank_tel, gsb_data_bank_get_bank_tel (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( bank_mail, gsb_data_bank_get_bank_mail (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( bank_web, gsb_data_bank_get_bank_web (bank_number), bank_number);

    gsb_autofunc_entry_set_value ( bank_contact_name, gsb_data_bank_get_correspondent_name (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( bank_contact_tel, gsb_data_bank_get_correspondent_tel (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( bank_contact_mail, gsb_data_bank_get_correspondent_mail (bank_number), bank_number);
    gsb_autofunc_entry_set_value ( bank_contact_fax, gsb_data_bank_get_correspondent_fax (bank_number), bank_number);

    gsb_autofunc_textview_set_value ( bank_adr, gsb_data_bank_get_bank_address (bank_number), bank_number );
    gsb_autofunc_textview_set_value ( bank_notes, gsb_data_bank_get_bank_note (bank_number), bank_number );

    /* show/hide the frame and the button delete bank */
    if (bank_number == -1)
	value = FALSE;
    else
	value = TRUE;
    gtk_widget_set_sensitive ( frame, value );

    if ( GTK_IS_WIDGET(delete_bank_button))
	gtk_widget_set_sensitive ( delete_bank_button, value );

    return FALSE;
}


/**
 * create a window with a form containing all the parameters of the bank
 * and allow to modify the values
 * the form is the same in the property of banks, but in a window
 *
 * \param bank_number the number of bank
 * \param combobox NULL or a pointer to a specific combobox to save the selection while editing the bank
 *
 * \return FALSE
 * */
static gboolean gsb_bank_edit_bank ( gint bank_number,
                        GtkWidget *combobox )
{
    GtkWidget *dialog, *form, *scrolled_window, *vbox;
    gint result;

    dialog = gtk_dialog_new_with_buttons ( _("Edit bank"),
					   GTK_WINDOW ( run.window ),
					   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					   "gtk-cancel""gtk-cancel", GTK_RESPONSE_CANCEL,
					   "gtk-apply", GTK_RESPONSE_APPLY,
					   NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );

    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window),
					  GTK_SHADOW_NONE );
    gtk_container_add ( GTK_CONTAINER ( dialog_get_content_area ( dialog ) ), scrolled_window );
    gtk_widget_set_size_request ( scrolled_window, 600, 400 );

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER (vbox), 12 );
    form = gsb_bank_create_form ( vbox, combobox );
    gtk_container_add ( GTK_CONTAINER (scrolled_window), vbox);

    gtk_widget_show_all ( dialog );

    if ( bank_number == -2 )
    {
        gchar *tmp_str;

        tmp_str = g_strdup ( _("New bank") );
        gtk_entry_set_text ( GTK_ENTRY ( bank_name ), tmp_str );
        g_free ( tmp_str );

    }
    else
        gsb_bank_update_form ( bank_number, dialog_get_content_area ( dialog ) );

    result = gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    if ( result == GTK_RESPONSE_APPLY )
    {
        if ( bank_number == -2 )
            bank_number = gsb_data_bank_new ( _("New bank") );

        gsb_bank_update_bank_data ( bank_number );
        gsb_bank_update_selected_line_model ( combobox );
        gsb_bank_list_set_bank ( combobox, bank_number );

        /* Mark file as modified */
        gsb_file_set_modified ( TRUE );
    }
    else
    {
        if ( bank_number == -2 )
        {
            gint account_number;

            account_number = gsb_gui_navigation_get_current_account ( );
            gsb_bank_list_set_bank ( combobox,
                        gsb_data_account_get_bank ( account_number ) );
        }
    }

    gtk_widget_destroy ( dialog );

    return FALSE;
}



/**
 * Update bank name in list, called when the name of the bank changes, or
 * the name of the talker
 * update too the model list of the combobox
 *
 * \param entry Widget that triggered signal
 * \param combobox NULL or a combobox we want to keep the state while editing a bank
 *
 * \return FALSE
 */
static gboolean gsb_bank_update_selected_line ( GtkEntry *entry,
						GtkWidget *combobox )
{
    GtkTreeIter iter;

    /* first we update the list if we are in configuration */
    if ( bank_list_tree_view
	 &&
	 GTK_IS_TREE_VIEW (bank_list_tree_view))
    {
        GtkTreeSelection *selection;
        GtkTreeModel *model;

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (bank_list_tree_view));

        if ( gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (selection),
                               &model, &iter ))
            gtk_list_store_set ( GTK_LIST_STORE (model),
                        &iter,
                        BANK_VIEW_NAME_COL, gtk_entry_get_text ( GTK_ENTRY(bank_name)),
                        BANK_TALKER_COL, gtk_entry_get_text (GTK_ENTRY(bank_contact_name)),
                        -1 );
    }

    /* we update the combobox model if exists */
    if ( GTK_IS_COMBO_BOX ( combobox ) )
    {
        if ( gtk_combo_box_get_active_iter ( GTK_COMBO_BOX ( combobox ), &iter ) )
            gtk_list_store_set ( GTK_LIST_STORE ( bank_list_model ),
                        &iter,
                        BANK_NAME_COL, gtk_entry_get_text ( GTK_ENTRY ( bank_name ) ),
                        -1 );
        gsb_bank_update_selected_line_model ( combobox );
    }
    else
        gsb_bank_update_selected_line_model (NULL);

    return FALSE;
}

/**
 * callback used to create a new bank
 *
 * \param button the button "add" or the combobox of the list of banks
 * \param none
 *
 * \return FALSE
 * */
static gboolean gsb_bank_add ( GtkWidget *button,
                        gpointer null )
{
    gint bank_number;

    /* create the new bank */
    bank_number = gsb_data_bank_new (_("New bank"));

    if (!bank_number)
    {
	dialogue_error_memory ();
	return FALSE;
    }

    /* add the new bank to the combobox list */
    gsb_bank_update_selected_line_model (NULL);


    /* play with the list if we are on the configuration */
    if (GTK_IS_BUTTON (button))
    {
	GtkTreeSelection *selection;
	GtkListStore *store;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (bank_list_tree_view));
	store = GTK_LIST_STORE (gtk_tree_view_get_model ( GTK_TREE_VIEW (bank_list_tree_view)));

	/* remove first the selection */
	gtk_tree_selection_unselect_all (selection);

	/* add the selection to the list */
	gtk_list_store_append ( GTK_LIST_STORE (store),
				&iter );
	gtk_list_store_set ( GTK_LIST_STORE (store),
			     &iter,
			     BANK_VIEW_NAME_COL, gsb_data_bank_get_name (bank_number),
			     BANK_TALKER_COL, "",
			     BANK_NUMBER, bank_number,
			     -1 );
	gtk_tree_selection_select_iter ( GTK_TREE_SELECTION (selection),
					 &iter );

	/* select the new bank in the entry and give the focus */
	gtk_editable_select_region ( GTK_EDITABLE ( bank_name ), 0, -1 );
	gtk_widget_grab_focus ( bank_name );
    }
    else
    {
	/* it's a combobox, come here via add new bank */
        gsb_bank_list_set_bank ( button, bank_number );
        gsb_bank_edit_bank ( bank_number, button );
    }

    gsb_file_set_modified ( TRUE );
    return FALSE;
}


/**
 * delete a bank
 *
 * \param button
 * \param null
 *
 * \return FALSE
 * */
static gboolean gsb_bank_delete ( GtkWidget *button,
                        gpointer null )
{
    gint bank_number;
    gboolean result;
    gboolean bank_is_used=FALSE;
    GSList *list_tmp;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (bank_list_tree_view));
    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (selection),
					    &model, &iter ))
	return FALSE;

    /* get the bank number */
    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 BANK_NUMBER, &bank_number,
			 -1 );

    /* check if the bank is used in grisbi */
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( gsb_data_account_get_bank (i) == bank_number )
	    bank_is_used = TRUE;

	list_tmp = list_tmp -> next;
    }

    if ( bank_is_used )
    {
	gchar* tmpstr = g_strdup_printf ( _("Bank \"%s\" is used by one or several accounts.\nDo you really want to remove it?"),
							  gsb_data_bank_get_name (bank_number));
	result = question_yes_no (tmpstr, _("Confirmation of bank removal"),
					GTK_RESPONSE_NO );
       g_free ( tmpstr );
    }
    else
    {
        gchar* tmpstr = g_strdup_printf ( _("Are you sure you want to remove bank \"%s\"?\n"),
							  gsb_data_bank_get_name (bank_number));
	result = question_yes_no (tmpstr, _("Confirmation of bank removal"),
					GTK_RESPONSE_NO );
       g_free ( tmpstr );
    }

    if ( result )
    {
	/* remove the bank */
	gtk_list_store_remove ( GTK_LIST_STORE (model),
				&iter );
	gsb_data_bank_remove (bank_number);
    }

    /* remove the bank from the combobox list */
    gsb_bank_update_selected_line_model (NULL);

    gsb_file_set_modified ( TRUE );
    return FALSE;
}



/**
 * Update bank code in account_property view
 * when it's change in the edit_bank form.
 *
 * \param entry
 * \param null
 *
 * */
static void gsb_bank_code_changed ( GtkEntry *entry, gpointer data )
{
    gint bank_number;

    bank_number = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (entry),
                        "number_for_func") );
    gsb_account_property_set_label_code_banque ( bank_number );
}



/**
 * Update BIC code in account_property view
 * when it's change in the edit_bank form.
 * \param entry
 * \param null
 *
 * */
static void gsb_bank_bic_code_changed ( GtkEntry *entry, gpointer data )
{
    gint bank_number;

    bank_number = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT (entry),
                        "number_for_func") );
    gsb_account_property_set_label_code_bic ( bank_number );
}


/**
 * create or update bank data
 * when it's change in the edit_bank form.
 *
 * \param entry
 * \param null
 *
 * */
static void gsb_bank_update_bank_data ( gint bank_number )
{
    GtkTextBuffer *buffer;

    devel_debug_int ( bank_number );
    /* set bank_name */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_name ) ) > 0 )
        gsb_data_bank_set_name ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_name ) ) );
    else
        gsb_data_bank_set_name ( bank_number, _("New bank") );

    /* set bank_code */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_code ) ) > 0 )
        gsb_data_bank_set_code ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_code ) ) );
    else
        gsb_data_bank_set_code ( bank_number, "" );

    /* set bank_BIC */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_BIC ) ) > 0 )
        gsb_data_bank_set_bic ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_BIC ) ) );
    else
        gsb_data_bank_set_bic ( bank_number, "" );

    /* set bank_adr */
    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( bank_adr ) );
    if ( gtk_text_buffer_get_char_count ( buffer ) > 0 )
        gsb_data_bank_set_bank_address ( bank_number,
                        gsb_editable_text_view_get_content ( bank_adr ) );
    else
        gsb_data_bank_set_bank_address ( bank_number, "" );

    /* set bank_tel */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_tel ) ) > 0 )
        gsb_data_bank_set_bank_tel ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_tel ) ) );
    else
        gsb_data_bank_set_bank_tel ( bank_number, "" );

    /* set bank_mail */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_mail ) ) > 0 )
        gsb_data_bank_set_bank_mail ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_mail ) ) );
    else
        gsb_data_bank_set_bank_mail ( bank_number, "" );

    /* set bank_web */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_web ) ) > 0 )
        gsb_data_bank_set_bank_web ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_web ) ) );
    else
        gsb_data_bank_set_bank_web ( bank_number, "" );

    /* set contact_name */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_contact_name ) ) > 0 )
        gsb_data_bank_set_correspondent_name ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_contact_name ) ) );
    else
        gsb_data_bank_set_correspondent_name ( bank_number, "" );

    /* set contact_tel */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_contact_tel ) ) > 0 )
        gsb_data_bank_set_correspondent_tel ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_contact_tel ) ) );
    else
        gsb_data_bank_set_correspondent_tel ( bank_number, "" );

    /* set contact_mail */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_contact_mail ) ) > 0 )
        gsb_data_bank_set_correspondent_mail ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_contact_mail ) ) );
    else
        gsb_data_bank_set_correspondent_mail ( bank_number, "" );

    /* set contact_fax */
    if ( gtk_entry_get_text_length ( GTK_ENTRY ( bank_contact_fax ) ) > 0 )
        gsb_data_bank_set_correspondent_fax ( bank_number, gtk_entry_get_text ( GTK_ENTRY ( bank_contact_fax ) ) );
    else
        gsb_data_bank_set_correspondent_fax ( bank_number, "" );

    /* set bank_notes */
    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( bank_adr ) );
    if ( gtk_text_buffer_get_char_count ( buffer ) > 0 )
        gsb_data_bank_set_bank_note ( bank_number,
                        gsb_editable_text_view_get_content (  bank_notes ) );
    else
        gsb_data_bank_set_bank_note ( bank_number, "" );

}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
