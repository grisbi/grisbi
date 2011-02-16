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
 * \file gsb_fyear_config.c
 * work with the configuration of the finacial year
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_fyear_config.h"
#include "dialog.h"
#include "utils_dates.h"
#include "gsb_autofunc.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_fyear.h"
#include "gsb_data_transaction.h"
#include "gsb_fyear.h"
#include "traitement_variables.h"
#include "utils.h"
#include "transaction_list.h"
#include "structures.h"
#include "gsb_transactions_list.h"
/*END_INCLUDE*/


/** Columns numbers for fyears list  */
enum fyear_list_column {
    FYEAR_NAME_COLUMN = 0,
    FYEAR_BEGIN_DATE_COLUMN,
    FYEAR_END_DATE_COLUMN,
    FYEAR_INVALID_COLUMN,
    FYEAR_NUMBER_COLUMN,
    NUM_FYEARS_COLUMNS,
};


/*START_STATIC*/
static gboolean gsb_fyear_config_add_fyear ( GtkWidget *tree_view );
static void gsb_fyear_config_append_line ( GtkTreeModel *model,
				    gint fyear_number,
				    GtkTreeIter *iter_to_fill );
static gboolean gsb_fyear_config_associate_transactions ( void );
static GtkWidget *gsb_fyear_config_create_list ();
static void gsb_fyear_config_fill_list ( GtkTreeModel *model );
static gboolean gsb_fyear_config_modify_fyear ( GtkWidget *entry,
					 GtkWidget *tree_view);
static gboolean gsb_fyear_config_remove_fyear ( GtkWidget *tree_view );
static gboolean gsb_fyear_config_select ( GtkTreeSelection *tree_selection,
				   gpointer null );
static void gsb_fyear_update_invalid ( GtkWidget *tree_view );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * Creates the "Financial years" tab.  It creates a financial years
 * list and then a form that allows to edit selected financial year.
 *
 * \param
 *
 * \returns A newly allocated vbox
 */
GtkWidget *gsb_fyear_config_create_page ( void )
{
    GtkWidget *vbox_pref, *label;
    GtkWidget *scrolled_window, *vbox, *hbox;
    GtkWidget *paddingbox, *table;
    GtkWidget *tree_view;
    GtkTreeModel *tree_model;
    GtkWidget *entry;
    GtkWidget *button;

    vbox_pref = new_vbox_with_title_and_icon ( _("Financial years"),
					       "financial-years.png" );
    paddingbox = new_paddingbox_with_title ( vbox_pref, TRUE,
					     _("Known financial years") );
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 TRUE, TRUE, 0);

    /* Create financial years list */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC);

    tree_view = gsb_fyear_config_create_list ();
    tree_model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			tree_view);
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)), 
		       "changed",
		       G_CALLBACK (gsb_fyear_config_select), 
		       NULL );


    /* Do not activate unless an account is opened */
    if ( gsb_data_account_get_accounts_amount () )
	gsb_fyear_config_fill_list (tree_model);
    else
	gtk_widget_set_sensitive ( vbox_pref, FALSE );


    /* Order buttons in a vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			 FALSE, FALSE, 0 );

    /* Add button */
    button = gtk_button_new_from_stock (GTK_STOCK_ADD);
    g_signal_connect_swapped ( G_OBJECT (button),
			       "clicked",
			       G_CALLBACK  (gsb_fyear_config_add_fyear),
			       tree_view );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button,
			 FALSE, FALSE, 5 );
    /* Remove button */
    button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    gtk_widget_set_sensitive ( button, FALSE );
    g_object_set_data ( G_OBJECT (tree_model),
			"remove_fyear_button", button );
    g_signal_connect_swapped ( G_OBJECT (button),
			       "clicked",
			       G_CALLBACK  (gsb_fyear_config_remove_fyear),
			       tree_view );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button,
			 FALSE, FALSE, 5 );

    /* Associate operations : under the list */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    button = gtk_button_new_with_label ( _("Associate operations without financial years") );
    g_signal_connect ( G_OBJECT ( button ),
			 "clicked",
			 G_CALLBACK ( gsb_fyear_config_associate_transactions ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX (hbox), button,
			 FALSE, FALSE, 5 );
    gtk_widget_show ( button );

    /* Financial year details */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
						     _("Financial year details") );
    g_object_set_data ( G_OBJECT (tree_model),
			"paddingbox_details", paddingbox );

    /* create a hbox, the table on the left
     * and the warning on the right */
    hbox = gtk_hbox_new ( FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (paddingbox), hbox,
			 FALSE, FALSE, 0 );

    /* Put stuff in a table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX (hbox), table,
			 FALSE, FALSE, 0 );

    /* Financial year name */
    label = gtk_label_new ( _("Name: ") );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    entry = gsb_autofunc_entry_new ( NULL,
				     G_CALLBACK (gsb_fyear_config_modify_fyear), tree_view,
				     G_CALLBACK (gsb_data_fyear_set_name), 0 );
    g_object_set_data ( G_OBJECT (tree_model),
			"fyear_name_entry", entry );
    gtk_table_attach ( GTK_TABLE ( table ),
		       entry, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Start */
    label = gtk_label_new ( _("Start: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    entry = gsb_autofunc_date_new ( NULL,
				    G_CALLBACK (gsb_fyear_config_modify_fyear), tree_view,
				    G_CALLBACK (gsb_data_fyear_set_beginning_date), 0 );
    g_object_set_data ( G_OBJECT (tree_model),
			"fyear_begin_date_entry", entry );
    gtk_table_attach ( GTK_TABLE ( table ),
		       entry, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* End */
    label = gtk_label_new ( _("End: ") );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    entry = gsb_autofunc_date_new ( NULL,
				    G_CALLBACK (gsb_fyear_config_modify_fyear), tree_view,
				    G_CALLBACK (gsb_data_fyear_set_end_date), 0 );
    g_object_set_data ( G_OBJECT (tree_model),
			"fyear_end_date_entry", entry );
    gtk_table_attach ( GTK_TABLE ( table ),
		       entry, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* label showed if the fyear is invalid */
    label = gtk_label_new (NULL);
    g_object_set_data ( G_OBJECT (tree_model),
			"invalid_label", label );
    gtk_box_pack_start ( GTK_BOX (hbox), label,
			 FALSE, FALSE, 0 );

    /* Activate in transaction form? */
    button = gsb_autofunc_checkbutton_new ( _("Activate financial year in transaction form"), FALSE,
					    NULL, NULL,
					    G_CALLBACK (gsb_data_fyear_set_form_show), 0);
    g_object_set_data ( G_OBJECT (tree_model),
			"fyear_show_button", button );
    gtk_box_pack_start ( GTK_BOX (paddingbox), button,
			 FALSE, FALSE, 0 );

    gtk_widget_set_sensitive (paddingbox, FALSE );

    return ( vbox_pref );
}



/**
 * create the tree wich contains the fyear list
 *
 * \param
 *
 * \return a GtkTreeView
 */
GtkWidget *gsb_fyear_config_create_list ()
{
    GtkListStore *model;
    GtkWidget *treeview;
    gint i;
    gchar *title[] = {
	_("Name"),
	_("Begin date"),
	_("End date"),
	_("Invalid"),
    };
    GtkCellRenderer *cell_renderer;

	/* Create tree store */
    model = gtk_list_store_new ( NUM_FYEARS_COLUMNS,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_INT );

    /* Create tree tree_view */
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
    g_object_unref (G_OBJECT(model));
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

    /* for all the columns it's a text */
    cell_renderer = gtk_cell_renderer_text_new ();
    g_object_set ( G_OBJECT ( cell_renderer),
		   "xalign", 0.5,
		   NULL );

    /* fill the columns : set FYEAR_NUMBER_COLUMN and not NUM_FYEARS_COLUMNS because
     * the last value of the model mustn't be to text...
     * so FYEAR_NUMBER_COLUMN must be the first column after the last column showed */

    for (i=0 ; i<FYEAR_NUMBER_COLUMN ; i++ )
    {
	GtkTreeViewColumn *column;

	if ( i == FYEAR_INVALID_COLUMN )
	{
	    column = gtk_tree_view_column_new_with_attributes ( title[i],
								gtk_cell_renderer_pixbuf_new (),
								"stock-id", i,
								NULL );
	}
	else
	{
	    column = gtk_tree_view_column_new_with_attributes ( title[i],
								cell_renderer,
								"text", i,
								NULL );
	    gtk_tree_view_column_set_sizing ( column,
					      GTK_TREE_VIEW_COLUMN_AUTOSIZE );
	    gtk_tree_view_column_set_expand ( column, TRUE );
	}
	gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview),
				      column );
    }

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model), 
					  FYEAR_NAME_COLUMN, GTK_SORT_ASCENDING);

    return treeview;
}


/**
 * fill the list of links
 *
 * \param model the tree_model to fill
 *
 * \return
 * */
void gsb_fyear_config_fill_list ( GtkTreeModel *model )
{
    GSList *tmp_list;

    gtk_list_store_clear (GTK_LIST_STORE (model));
    tmp_list = gsb_data_fyear_get_fyears_list ();

    while ( tmp_list )
    {
	gint fyear_number;
	fyear_number = gsb_data_fyear_get_no_fyear (tmp_list -> data);

	gsb_fyear_config_append_line ( model,
				       fyear_number,
				       NULL );
	tmp_list = tmp_list -> next;
    }
}

/**
 * append a new line to the tree_view and fill it with the
 * link given in param
 *
 * \param model
 * \param fyear_number
 * \param iter a pointer to an iter to fill it with the position of the new link, or NULL
 *
 * \return 
 * */
void gsb_fyear_config_append_line ( GtkTreeModel *model,
				    gint fyear_number,
				    GtkTreeIter *iter_to_fill )
{
    gchar *invalid;
    GtkTreeIter local_iter;
    GtkTreeIter *iter_ptr;

    if (iter_to_fill)
	iter_ptr = iter_to_fill;
    else
	iter_ptr = &local_iter;

    if ( gsb_data_fyear_get_invalid (fyear_number))
	invalid = GTK_STOCK_DIALOG_WARNING;
    else
	invalid = NULL;

    gtk_list_store_append ( GTK_LIST_STORE (model),
			    iter_ptr );
    gtk_list_store_set ( GTK_LIST_STORE (model),
			 iter_ptr,
			 FYEAR_NAME_COLUMN, gsb_data_fyear_get_name (fyear_number),
			 FYEAR_BEGIN_DATE_COLUMN, gsb_format_gdate (gsb_data_fyear_get_beginning_date (fyear_number)),
			 FYEAR_END_DATE_COLUMN, gsb_format_gdate (gsb_data_fyear_get_end_date (fyear_number)),
			 FYEAR_INVALID_COLUMN, invalid,
			 FYEAR_NUMBER_COLUMN, fyear_number,
			 -1 );
}


/**
 * called when change the selection of the fyear
 *
 * \param tree_selection
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_fyear_config_select ( GtkTreeSelection *tree_selection,
				   gpointer null )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint fyear_number;
    GtkWidget *tree_view;
    GtkWidget *widget;

    if (!gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (tree_selection),
					   &model,
					   &iter ))
	return FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 FYEAR_NUMBER_COLUMN, &fyear_number,
			 -1 );

    tree_view = GTK_WIDGET (gtk_tree_selection_get_tree_view (tree_selection));

    /* set the name */
    widget = g_object_get_data ( G_OBJECT (model),
				 "fyear_name_entry" );
    gsb_autofunc_entry_set_value ( widget,
				   gsb_data_fyear_get_name (fyear_number),
				   fyear_number );

    /* set the beginning date */
    widget = g_object_get_data ( G_OBJECT (model),
				 "fyear_begin_date_entry" );
    gsb_calendar_entry_set_color (widget, TRUE);
    gsb_autofunc_date_set ( widget,
			    gsb_data_fyear_get_beginning_date (fyear_number),
			    fyear_number );

    /* set the end date */
    widget = g_object_get_data ( G_OBJECT (model),
				 "fyear_end_date_entry" );
    gsb_calendar_entry_set_color (widget, TRUE);
    gsb_autofunc_date_set ( widget,
			    gsb_data_fyear_get_end_date (fyear_number),
			    fyear_number );

    /* set the button */
    widget = g_object_get_data ( G_OBJECT (model),
				 "fyear_show_button" );
    gsb_autofunc_checkbutton_set_value ( widget,
					 gsb_data_fyear_get_form_show (fyear_number),
					 fyear_number );

    /* set the invalid label */
    widget = g_object_get_data ( G_OBJECT (model),
				 "invalid_label" );
    if (gsb_data_fyear_get_invalid (fyear_number))
    {
	gtk_label_set_markup ( GTK_LABEL (widget),
			       gsb_data_fyear_get_invalid_message (fyear_number));
	gtk_widget_show (widget);
    }
    else
	gtk_widget_hide (widget);

    /* sensitive what is needed */
    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (model),
						   "paddingbox_details" ),
			       TRUE );
    gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (model),
						   "remove_fyear_button" ),
			       TRUE );

    return FALSE;
}



/**
 * called when something change for a fyear 
 * update the list and the invalid
 *
 * \param entry the entry wich change
 * \param tree_view the tree_view
 *
 * \return FALSE
 * */
gboolean gsb_fyear_config_modify_fyear ( GtkWidget *entry,
					 GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint fyear_number;
    GtkWidget *widget;
    gchar *invalid;

    if (!gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view))),
					   &model,
					   &iter ))
	return FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 FYEAR_NUMBER_COLUMN, &fyear_number,
			 -1 );

    /* normally should not happen */
    if (!fyear_number)
	return FALSE;

    /* check the invalid and show the message if needed */
    widget = g_object_get_data ( G_OBJECT (model),
				 "invalid_label" );

     /* check all the fyear to set them invalid if need */
     gsb_fyear_update_invalid (tree_view);

     /* and check if the current fyear was set as invalid */
     if (gsb_data_fyear_get_invalid (fyear_number))
     {
	 /* and now focus on the current fyear */
	 invalid = GTK_STOCK_DIALOG_WARNING;
	 gtk_label_set_markup ( GTK_LABEL (widget),
				gsb_data_fyear_get_invalid_message (fyear_number));
	 gtk_widget_show (widget);
     }
     else
     {
	 invalid = NULL;
	 gtk_widget_hide (widget);
     }

     gtk_list_store_set ( GTK_LIST_STORE (model),
			  &iter,
			  FYEAR_NAME_COLUMN, gsb_data_fyear_get_name (fyear_number),
			  FYEAR_BEGIN_DATE_COLUMN, gsb_format_gdate (gsb_data_fyear_get_beginning_date (fyear_number)),
			  FYEAR_END_DATE_COLUMN, gsb_format_gdate (gsb_data_fyear_get_end_date (fyear_number)) ,
			  FYEAR_INVALID_COLUMN, invalid,
			  FYEAR_NUMBER_COLUMN, fyear_number,
			  -1 );
     if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
     return FALSE;
}

/**
 * update the invalid fyears in the list view
 *
 * \param tree_view the tree_view
 *
 * \return
 * */
void gsb_fyear_update_invalid ( GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    /* first update all the invalids flags for the fyears */
    gsb_data_fyear_check_all_for_invalid ();

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    /* now go throw the list and show/hide the invalid flag */
    if (!gtk_tree_model_get_iter_first ( model, &iter ))
	return;

    do
    {
	gint fyear_number;
	gchar *invalid;

	gtk_tree_model_get ( model, &iter,
			     FYEAR_NUMBER_COLUMN, &fyear_number,
			     -1 );
	if (gsb_data_fyear_get_invalid (fyear_number))
	    invalid = GTK_STOCK_DIALOG_WARNING;
	else
	    invalid = NULL;

	gtk_list_store_set ( GTK_LIST_STORE (model), &iter,
			     FYEAR_INVALID_COLUMN, invalid,
			     -1 );
    }
    while ( gtk_tree_model_iter_next ( model, &iter ));
}


/**
 * called for add a new financial year
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_fyear_config_add_fyear ( GtkWidget *tree_view )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkWidget *entry;
    gint fyear_number;
    GDate *date;

    fyear_number = gsb_data_fyear_new (_("New financial year"));

    /* if bad things will append soon ... */
    if (!fyear_number)
	return FALSE;
    
    gsb_data_fyear_set_form_show ( fyear_number,
				   TRUE );
    gsb_data_fyear_set_beginning_date ( fyear_number,
				       date = gdate_today());
    gsb_data_fyear_set_end_date ( fyear_number,
				  date );
    g_date_free (date);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

    /* append to the list and get back the iter */
    gsb_fyear_config_append_line ( model,
				   fyear_number,
				   &iter );

    /* select it */
    gtk_tree_selection_select_iter ( GTK_TREE_SELECTION (selection),
				     &iter );

    /* select the new name and give it the focus */
    entry = g_object_get_data ( G_OBJECT (model),
				"fyear_name_entry");
    gtk_editable_select_region ( GTK_EDITABLE (entry), 0, -1 );
    gtk_widget_grab_focus (entry);

    /* Update various menus */
    gsb_fyear_update_fyear_list ();
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * called to remove a financial year, check before if
 * some transactions are associated with it and warn if yes
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_fyear_config_remove_fyear ( GtkWidget *tree_view )
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    gint fyear_number;
    gboolean warning_showed = FALSE;
    GSList *tmp_list;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (selection),
				      &model,
				      &iter );
    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 FYEAR_NUMBER_COLUMN, &fyear_number,
			 -1 );
    if (!fyear_number)
	return FALSE;

    /* first, we check if one transaction uses that financial year */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();

    while (tmp_list)
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

	if ( fyear_number
	     ==
	     gsb_data_transaction_get_financial_year_number (transaction_number))
	{
	    /* at the beginning warning_showed is FALSE and we show a warning,
	     * if the user doesn't want to continue, we go out of the while so cannot come
	     * here again ; but if he wants to continue, warning_showed is set to TRUE, we delete
	     * the financial year and continue to come here to set the fyear of the
	     * transactions to 0 */
	    if (warning_showed)
		gsb_data_transaction_set_financial_year_number (transaction_number, 0);
	    else
	    {
		gint result;

		result = question_yes_no_hint ( _("The selected financial year is used in the file"),
						_("If you really remove it, all the associated transactions will be without financial year.\nAre you sure ?"),
						GTK_RESPONSE_NO );
		if (result)
		{
		    gsb_data_transaction_set_financial_year_number (transaction_number, 0);
		    warning_showed = TRUE;
		}
		else
		    break;
	    }
	}
	tmp_list = tmp_list -> next;
    }

    /* if warning_showed is FALSE, it's because none transaction have that fyear,
     * or we answer NO to the warning but in that case, tmp_list is non NULL */
    if (warning_showed
	||
	!tmp_list )
    {
	gsb_data_fyear_remove (fyear_number);

	gtk_list_store_remove ( GTK_LIST_STORE (model),
				&iter );
	gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (model),
						       "paddingbox_details" ),
				   FALSE );
	gtk_widget_set_sensitive ( g_object_get_data ( G_OBJECT (model),
						       "remove_fyear_button" ),
				   FALSE );

	/* Update various menus */
	gsb_fyear_update_fyear_list ();
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }
    return FALSE;
}




/**
 * associate all transactions without fyear to the corresponding
 * fyear
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_fyear_config_associate_transactions ( void )
{
    GSList *list_tmp;
    gint modification_number = 0;

    if (!question_yes_no_hint ( _("Automatic association of financial years?"),
				_("This function assigns each transaction without a financial year to the one related to its transaction date.  If no financial year matches, the transaction will not be changed."),
				GTK_RESPONSE_NO ))
	return FALSE;

    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp )
    {
	gint transaction_number;

	transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	if (!gsb_data_transaction_get_financial_year_number (transaction_number))
	{
	    gint fyear_number;

	    fyear_number = gsb_data_fyear_get_from_date (gsb_data_transaction_get_date (transaction_number));
	    if (fyear_number)
	    {
		gsb_data_transaction_set_financial_year_number ( transaction_number,
								 fyear_number );
		modification_number++;
	    }
	}
	list_tmp = list_tmp -> next;
    }

    if (modification_number)
    {
	gchar* tmpstr = g_strdup_printf (_("%d transactions associated"),
				    modification_number);
	dialogue (  tmpstr );
	g_free ( tmpstr );
	transaction_list_update_element (ELEMENT_EXERCICE);
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }
    else
	dialogue ( _("no transaction to associate"));
    return FALSE;
}




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
