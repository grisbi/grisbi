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
#include "utils_dates.h"
#include "navigation.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "traitement_variables.h"
#include "utils.h"
#include "dialog.h"
#include "transaction_list.h"
#include "structures.h"
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
extern GtkWidget *window;
/*END_EXTERN*/

/**
 * Creates the Archive config tab.
 *
 * \param
 *
 * \returns A newly allocated vbox
 */
GtkWidget *gsb_archive_config_create ( void )
{
    GtkWidget *vbox_pref, *hbox, *scrolled_window, *paddingbox;
    GtkWidget *table, *label;
    GtkListStore *archive_model;
    gchar *titles[] = {
	_("Name"), _("Initial date"), _("Final date"), _("Financial year"), _("Report name")
    };
    gfloat alignment[] = {
	COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER , COLUMN_CENTER, COLUMN_CENTER
    };
    gint i;
    GtkWidget *modification_paddingbox;
    GtkWidget *check_paddingbox;
    GtkWidget *button;
    GtkWidget *entry;
	gchar* tmpstr;

    /* create the page */
    vbox_pref = new_vbox_with_title_and_icon ( _("Archives"),
					       "archives.png" );

    /* create the list of archives */
    paddingbox = new_paddingbox_with_title (vbox_pref, TRUE,
					    _("Known archives"));

    /* Create scrolled window */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN);
    gtk_box_pack_start ( GTK_BOX (paddingbox),
			 scrolled_window,
			 TRUE, TRUE, 0 );

    /* Create tree view */
    archive_model = gtk_list_store_new (NUM_ARCHIVES_COLUMNS,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_STRING,
					G_TYPE_INT );
    archive_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (archive_model) );
    g_object_unref (G_OBJECT(archive_model) );
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (archive_treeview), TRUE);
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			archive_treeview );

    /* set the columns */
    for (i=0 ; i<5 ; i++)
    {
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
	gtk_tree_view_append_column ( GTK_TREE_VIEW(archive_treeview), column);
    }

    /* archive modification */
    modification_paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
						    _("Archive modification"));
    gtk_widget_set_sensitive ( modification_paddingbox, FALSE );
    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (archive_treeview)), 
		      "changed", 
		      G_CALLBACK (gsb_archive_config_select),
		      modification_paddingbox);

    /* archive name (the only thing we can change) */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX ( modification_paddingbox ), table,
			 FALSE, FALSE, 6 );

    label = gtk_label_new ( _("Name: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    archive_name_entry = gsb_autofunc_entry_new ( NULL,
						  G_CALLBACK (gsb_archive_config_name_changed), archive_treeview,
						  G_CALLBACK (gsb_data_archive_set_name), 0 );
    g_signal_connect ( G_OBJECT (archive_name_entry ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &archive_name_entry );
    gtk_table_attach ( GTK_TABLE ( table ),
		       archive_name_entry, 1, 2, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );

    /* button to delete an archive and free the transactions */
    button = gtk_button_new_with_label (_("Delete the archive"));
    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (gsb_archive_config_delete_archive),
		       archive_treeview );
    tmpstr = g_build_filename ( GRISBI_PIXMAPS_DIR, "import.png", NULL );
    gtk_button_set_image ( GTK_BUTTON(button), 
			   gtk_image_new_from_file ( tmpstr ) );
    g_free ( tmpstr );
    gtk_table_attach ( GTK_TABLE ( table ),
		       button, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );

    /* button to delete an archive and delete the transactions */
    button = gtk_button_new_with_label ( _("Delete the archive and transactions") );
    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (gsb_archive_config_destroy_archive),
		       archive_treeview );
    tmpstr = g_build_filename ( GRISBI_PIXMAPS_DIR, "import.png", NULL );
    gtk_button_set_image ( GTK_BUTTON(button), 
			   gtk_image_new_from_file ( tmpstr ) );
    g_free ( tmpstr );

    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    gtk_table_attach ( GTK_TABLE ( table ),
		       button, 1, 2, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );

    /* check part at the opening of the file */
    check_paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
						    _("Automatic check"));

    button = gsb_automem_checkbutton_new ( _("Check at opening if creating archive is needed."),
					   &conf.check_for_archival,
					   NULL, NULL );
    gtk_box_pack_start ( GTK_BOX (check_paddingbox),
			 button,
			 FALSE, FALSE,
			 0 );

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (check_paddingbox),
			 hbox,
			 TRUE, FALSE,
			 0 );

    label = gtk_label_new (_("Warn if more than "));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE,
			 0 );
    entry = gsb_automem_spin_button_new (&conf.max_non_archived_transactions_for_check,
					 NULL, NULL );
    gtk_widget_set_size_request ( entry,
			   100, -1 );
    gtk_box_pack_start ( GTK_BOX (hbox),
			 entry,
			 FALSE, FALSE,
			 0 );
    label = gtk_label_new (_(" transactions are not archived."));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 label,
			 FALSE, FALSE,
			 0 );

    /* fill the list */
    gsb_archive_config_fill_list (archive_model);

    gtk_widget_show_all (vbox_pref);

    /** Do not set this tab sensitive is no account file is opened. */
    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }
    return ( vbox_pref );
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
                          "Do you want to continue ?"),
                        gsb_data_archive_get_name (archive_number) );
    if (!question_yes_no_hint (_("Deleting an archive"), tmpstr , GTK_RESPONSE_CANCEL ))
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
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
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
	if (!question_yes_no_hint ( _("Deleting an archive and its transactions"),
                        tmpstr, GTK_RESPONSE_CANCEL ) )
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
	
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }
    return FALSE;
}

