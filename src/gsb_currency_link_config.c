/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	          */
/*			2003 Benjamin Drieu (bdrieu@april.org)		                      */
/*                      2009 Pierre Biava (grisbi@pierre.biava.name)          */
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
 * \file gsb_currency_link_config.c
 * contains the part to set the configuration of the currency_links
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_currency_link_config.h"
#include "utils_dates.h"
#include "dialog.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_file.h"
#include "gsb_real.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_prefs.h"
#include "utils_real.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_currency_link_config_add_link ( GtkWidget *tree_view );
static void gsb_currency_link_config_append_line ( GtkTreeModel *model,
					    gint link_number,
					    GtkTreeIter *iter_to_fill );
static gboolean gsb_currency_link_config_button_fixed_changed ( GtkWidget *checkbutton,
						GtkWidget *tree_view );
static GtkWidget *gsb_currency_link_config_create_list ( void );
static void gsb_currency_link_config_fill_list ( GtkTreeModel *model );
static gboolean gsb_currency_link_config_key_press ( GtkWidget *tree_view, GdkEventKey *ev );
static gboolean gsb_currency_link_config_modify_link ( GtkWidget *tree_view );
static gboolean gsb_currency_link_config_remove_link ( GtkWidget *tree_view );
static gboolean gsb_currency_link_config_select_currency ( GtkTreeSelection *tree_selection,
					    gpointer null );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static GtkWidget *delete_link_button;

/** Columns numbers for links list  */
enum LinkListColumns {
    LINK_1_COLUMN = 0,
    LINK_CURRENCY1_COLUMN,
    LINK_EQUAL_COLUMN,
    LINK_EXCHANGE_COLUMN,
    LINK_CURRENCY2_COLUMN,
    LINK_DATE_COLUMN,
    LINK_INVALID_COLUMN,
    LINK_NUMBER_COLUMN,
    NUM_LINKS_COLUMNS,
};

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
static gboolean gsb_currency_link_config_change_selection (GtkTreeSelection *selection,
                                                           gpointer null)
{
    GSList *tmp_list;
    gint nbre_links;

    tmp_list = gsb_data_currency_link_get_currency_link_list ();
    nbre_links = g_slist_length (tmp_list);
    if (nbre_links)
        gtk_widget_set_sensitive (delete_link_button, TRUE);
    else
        gtk_widget_set_sensitive (delete_link_button, FALSE);

    return FALSE;
}

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/**
 * create the currency_link page for the config
 *
 * \param
 *
 * \return a newly created box
 * */
GtkWidget *gsb_currency_link_config_create_page ( void )
{
    GtkWidget *vbox_pref, *label, *hbox;
    GtkWidget *paddinggrid;
    GtkWidget *scrolled_window;
    GtkWidget *button;
    GtkWidget *entry;
    GtkWidget *combobox;
    GtkTreeView *tree_view;
    GtkTreeModel *tree_model;
    gint width_entry = 170;

    vbox_pref = new_vbox_with_title_and_icon (_("Links between currencies"), "gsb-currencies-32.png");
    paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, _("Known links"));


    /* links list */
    scrolled_window = utils_prefs_scrolled_window_new (NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_PG, SW_MIN_HEIGHT);
    gtk_grid_attach (GTK_GRID (paddinggrid), scrolled_window, 0, 0, 2, 3);

    /* Create it. */
    tree_view = GTK_TREE_VIEW ( gsb_currency_link_config_create_list () );
    tree_model = gtk_tree_view_get_model ( tree_view );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), GTK_WIDGET(tree_view) );
    g_signal_connect ( gtk_tree_view_get_selection (GTK_TREE_VIEW ( tree_view ) ),
		       "changed",
		       G_CALLBACK ( gsb_currency_link_config_select_currency ),
		       NULL );
    /* check the keys on the list */
    g_signal_connect ( G_OBJECT ( tree_view ),
                        "key_press_event",
                        G_CALLBACK ( gsb_currency_link_config_key_press ),
                        NULL );

    /* if nothing opened, all is unsensitive */
    if ( !gsb_data_account_get_accounts_amount () )
	    gtk_widget_set_sensitive ( vbox_pref, FALSE );
    else
	    gsb_currency_link_config_fill_list ( tree_model );

    /* Create Add/Remove buttons */

    /* Button "Add" */
    button = utils_buttons_button_new_from_stock ("gtk-add", _("Add"));
    gtk_widget_set_margin_end (button, MARGIN_END);
    utils_widget_set_padding (button, 0, MARGIN_TOP);
    g_signal_connect_swapped ( G_OBJECT ( button ),
			       "clicked",
			       G_CALLBACK  ( gsb_currency_link_config_add_link ),
			       tree_view );
    gtk_grid_attach (GTK_GRID (paddinggrid), button, 0, 3, 1, 1);

    /* Button "Remove" */
    delete_link_button = utils_buttons_button_new_from_stock ("gtk-remove", _("Remove"));
    utils_widget_set_padding (delete_link_button, 0, MARGIN_TOP);
    gtk_widget_set_sensitive (delete_link_button, FALSE);
    g_signal_connect_swapped ( G_OBJECT ( delete_link_button ),
			       "clicked",
			       G_CALLBACK ( gsb_currency_link_config_remove_link ),
			       tree_view );
    gtk_grid_attach (GTK_GRID (paddinggrid), delete_link_button, 1, 3, 1, 1);

    /* Input form for currencies */
    paddinggrid = utils_prefs_paddinggrid_new_with_title (vbox_pref, _("Link properties"));

    /* Create hbox line */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_widget_set_margin_top (hbox, MARGIN_TOP);
    gtk_grid_attach (GTK_GRID (paddinggrid), hbox, 0, 0, 1, 1);

    gtk_widget_set_sensitive ( hbox, FALSE );
    g_object_set_data ( G_OBJECT (tree_model), "hbox_line", hbox );

    /* Create first currency link entry */
    label = gtk_label_new (_("1 "));
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    combobox = gsb_currency_make_combobox (TRUE);
    gtk_widget_set_size_request ( combobox, width_entry, -1 );
    g_object_set_data ( G_OBJECT (tree_model), "combobox_1", combobox );
    g_signal_connect_swapped ( G_OBJECT (combobox),
			       "changed",
			       G_CALLBACK (gsb_currency_link_config_modify_link),
			       tree_view );
    gtk_box_pack_start ( GTK_BOX (hbox), combobox, FALSE, FALSE, 0 );

    /* create the exchange rate between the 2 currencies */
    label = gtk_label_new (_(" = "));
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    entry = gtk_entry_new ();
    gtk_widget_set_size_request ( entry, width_entry/2, -1 );
    g_object_set_data ( G_OBJECT (tree_model), "exchange_entry", entry );
    g_signal_connect_swapped ( G_OBJECT (entry),
			       "changed",
			       G_CALLBACK (gsb_currency_link_config_modify_link),
			       tree_view );
    gtk_box_pack_start ( GTK_BOX (hbox), entry, FALSE, FALSE, 0 );

    /* Create second currency link entry */
    combobox = gsb_currency_make_combobox (TRUE);
    gtk_widget_set_size_request ( combobox, width_entry, -1 );
    g_object_set_data ( G_OBJECT (tree_model), "combobox_2", combobox );
    g_signal_connect_swapped ( G_OBJECT (combobox),
			       "changed",
			       G_CALLBACK (gsb_currency_link_config_modify_link),
			       tree_view );
    gtk_box_pack_start ( GTK_BOX (hbox), combobox, FALSE, FALSE, 0 );

    /* create fixed exchange rate */
    button = gtk_check_button_new_with_label ( _("Fixed exchange rate") );
    gtk_widget_set_margin_top (button, MARGIN_TOP);
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
    gtk_widget_set_sensitive ( button, FALSE );
    g_object_set_data ( G_OBJECT ( tree_model ), "fixed_button", button );
    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( gsb_currency_link_config_button_fixed_changed ),
                        tree_view );
    gtk_grid_attach (GTK_GRID (paddinggrid), button, 0, 1, 1, 1);

    /* Create warning label */
    label = gtk_label_new (NULL);
    gtk_widget_set_margin_top (label, 2*MARGIN_TOP);
    g_object_set_data ( G_OBJECT (tree_model), "warning_label", label );
    gtk_grid_attach (GTK_GRID (paddinggrid), label, 0, 2, 1, 1);

    return ( vbox_pref );
}


/**
 * create the tree wich contains the currency list
 * used both for the currency known list, and the currency to add list
 *
 * \param
 *
 * \return a GtkTreeView
 */
GtkWidget *gsb_currency_link_config_create_list ( void )
{
    GtkTreeSelection *selection;
    GtkListStore * model;
    GtkWidget * treeview;
    gint i;
    gchar *title[] = {
	"",
	_("First currency"),
	"",
	_("Exchange"),
	_("Second currency"),
    _("Modified date"),
	_("Invalid"),
    };
    GtkCellRenderer *cell_renderer;

	/* Create tree store
	   LINK_1_COLUMN,
	   LINK_CURRENCY1_COLUMN,
	   LINK_EQUAL_COLUMN,
	   LINK_EXCHANGE_COLUMN,
	   LINK_CURRENCY2_COLUMN,
       LINK_DATE_COLUMN,
	   LINK_NUMBER_COLUMN */
    model = gtk_list_store_new ( NUM_LINKS_COLUMNS,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_INT );

    /* Create tree tree_view */
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
 	gtk_widget_set_name (treeview, "tree_view");
    g_object_unref (G_OBJECT(model));

    /* connect the selection */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection), GTK_SELECTION_SINGLE);

    /* for all the columns it's a text */
    cell_renderer = gtk_cell_renderer_text_new ();
    g_object_set ( G_OBJECT ( cell_renderer),
		   "xalign", 0.5,
		   NULL );

    /* fill the columns : set LINK_NUMBER_COLUMN and not NUM_LINKS_COLUMNS because
     * the last value of the model mustn't be to text...
     * so LINK_NUMBER_COLUMN must be the first column after the last column showed */

    for (i=0 ; i< LINK_NUMBER_COLUMN; i++ )
    {
	GtkTreeViewColumn *column = NULL;

	if ( i == LINK_INVALID_COLUMN )
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
	gtk_tree_view_column_set_expand ( column, TRUE );
	gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview),
				      column );
    }

    /* Sort columns accordingly */
    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(model),
					  LINK_CURRENCY1_COLUMN, GTK_SORT_ASCENDING);

    /* Set signal */
    g_signal_connect (G_OBJECT (selection),
                      "changed",
                      G_CALLBACK (gsb_currency_link_config_change_selection),
                      NULL);

    return treeview;
}


/**
 * fill the list of links
 *
 * \param model the tree_model to fill
 *
 * \return
 * */
void gsb_currency_link_config_fill_list ( GtkTreeModel *model )
{
    GSList *tmp_list;

    gtk_list_store_clear (GTK_LIST_STORE (model));
    tmp_list = gsb_data_currency_link_get_currency_link_list ();

    while ( tmp_list )
    {
	gint link_number;

	link_number = gsb_data_currency_link_get_no_currency_link (tmp_list -> data);
	gsb_currency_link_config_append_line ( model,
					       link_number,
					       NULL );
	tmp_list = tmp_list -> next;
    }
}

/**
 * append a new line to the tree_view and fill it with the
 * link given in param
 *
 * \param model
 * \param link_number
 * \param iter a pointer to an iter to fill it with the position of the new link, or NULL
 *
 * \return
 * */
void gsb_currency_link_config_append_line ( GtkTreeModel *model,
					    gint link_number,
					    GtkTreeIter *iter_to_fill )
{
    gchar *invalid;
    GtkTreeIter local_iter;
    GtkTreeIter *iter_ptr;
	gchar *tmpstr;
    gchar *strdate;

    if (iter_to_fill)
	iter_ptr = iter_to_fill;
    else
	iter_ptr = &local_iter;

    strdate = gsb_format_gdate ( gsb_data_currency_link_get_modified_date ( link_number ) );

    if ( gsb_data_currency_link_get_invalid_link (link_number))
	invalid = "gtk-dialog-warning";
    else
	invalid = NULL;

    tmpstr = utils_real_get_string (gsb_data_currency_link_get_change_rate (link_number));
    gtk_list_store_append ( GTK_LIST_STORE ( model ), iter_ptr );
    gtk_list_store_set ( GTK_LIST_STORE ( model ),
			 iter_ptr,
			 LINK_1_COLUMN, "1",
			 LINK_CURRENCY1_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_first_currency(link_number)),
			 LINK_EQUAL_COLUMN, "=",
			 LINK_EXCHANGE_COLUMN, tmpstr,
			 LINK_CURRENCY2_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_second_currency(link_number)),
             LINK_DATE_COLUMN, strdate,
			 LINK_INVALID_COLUMN, invalid,
			 LINK_NUMBER_COLUMN, link_number,
			 -1 );
    g_free ( tmpstr );
    g_free ( strdate );
}


/**
 * called when select a link
 *
 * \param tree_selection
 * \param null not used
 *
 * \return FALSE
 * */
gboolean gsb_currency_link_config_select_currency ( GtkTreeSelection *tree_selection,
						    gpointer null )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint link_number;
    GtkWidget *combobox_1;
    GtkWidget *combobox_2;
    GtkWidget *exchange_entry;
    GtkWidget *tree_view;
    GtkWidget *label;
    GtkWidget *button;
	gchar* tmpstr;

    if (!gtk_tree_selection_get_selected ( GTK_TREE_SELECTION (tree_selection),
					   &model,
					   &iter ))
	return FALSE;

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, LINK_NUMBER_COLUMN, &link_number, -1);

    /* normally should not happen */
    if (!link_number)
        return FALSE;

    gtk_widget_set_sensitive (GTK_WIDGET (g_object_get_data (G_OBJECT (model), "hbox_line")),
                                  TRUE );

    combobox_1 = g_object_get_data ( G_OBJECT (model), "combobox_1" );
    combobox_2 = g_object_get_data ( G_OBJECT (model), "combobox_2" );
    exchange_entry = g_object_get_data ( G_OBJECT (model), "exchange_entry" );
    button = g_object_get_data ( G_OBJECT (model), "fixed_button" );
    gtk_widget_set_sensitive ( button, TRUE );

    tree_view = GTK_WIDGET ( gtk_tree_selection_get_tree_view ( tree_selection ) );

    g_signal_handlers_block_by_func ( G_OBJECT (combobox_1),
				      G_CALLBACK (gsb_currency_link_config_modify_link),
				      tree_view );
    gsb_currency_set_combobox_history ( combobox_1,
					gsb_data_currency_link_get_first_currency (link_number));
    g_signal_handlers_unblock_by_func ( G_OBJECT (combobox_1),
					G_CALLBACK (gsb_currency_link_config_modify_link),
					tree_view );

    g_signal_handlers_block_by_func ( G_OBJECT (combobox_2),
				      G_CALLBACK (gsb_currency_link_config_modify_link),
				      tree_view );
    gsb_currency_set_combobox_history ( combobox_2,
					gsb_data_currency_link_get_second_currency (link_number));
    g_signal_handlers_unblock_by_func ( G_OBJECT (combobox_2),
					G_CALLBACK (gsb_currency_link_config_modify_link),
					tree_view );

    g_signal_handlers_block_by_func ( G_OBJECT (exchange_entry),
				      G_CALLBACK (gsb_currency_link_config_modify_link),
				      tree_view );
    tmpstr = utils_real_get_string (gsb_data_currency_link_get_change_rate (link_number));
    gtk_entry_set_text ( GTK_ENTRY (exchange_entry), tmpstr);
    g_free ( tmpstr );
    g_signal_handlers_unblock_by_func ( G_OBJECT (exchange_entry),
					G_CALLBACK (gsb_currency_link_config_modify_link),
					tree_view );

    /* set the fixed_link flag */
    g_signal_handlers_block_by_func ( G_OBJECT ( button ),
				      G_CALLBACK ( gsb_currency_link_config_button_fixed_changed ),
				      tree_view );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ),
					gsb_data_currency_link_get_fixed_link ( link_number ) );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( button ),
				      G_CALLBACK ( gsb_currency_link_config_button_fixed_changed ),
				      tree_view );

    /* set or hide the warning label */
    label = g_object_get_data (G_OBJECT (model), "warning_label");

    if ( gsb_data_currency_link_get_invalid_link (link_number))
    {
	gtk_label_set_markup ( GTK_LABEL (label),
			       gsb_data_currency_link_get_invalid_message (link_number));
	gtk_widget_show (label);
    }
    else
	gtk_widget_hide (label);

    return FALSE;
}

/**
 * called when something change for a link
 *
 * \param tree_view the tree_view
 *
 * \return FALSE
 * */
gboolean gsb_currency_link_config_modify_link ( GtkWidget *tree_view )
{
    GtkWidget *combobox_1;
    GtkWidget *combobox_2;
    GtkWidget *exchange_entry;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint link_number;
    gchar *invalid;
    GtkWidget *label;
	gchar* tmpstr;
    gchar *strdate;
    gsb_real number;

    if ( !gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (
                        GTK_TREE_VIEW ( tree_view ) ),
					    &model,
					    &iter ) )
	return FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 &iter,
			 LINK_NUMBER_COLUMN, &link_number,
			 -1 );

    /* normally should not happen */
    if (!link_number)
	return FALSE;

    combobox_1 = g_object_get_data ( G_OBJECT (model),
				     "combobox_1" );
    combobox_2 = g_object_get_data ( G_OBJECT (model),
				     "combobox_2" );
    exchange_entry = g_object_get_data ( G_OBJECT (model),
					 "exchange_entry" );

    number = utils_real_get_from_string ( gtk_entry_get_text ( GTK_ENTRY (exchange_entry) ) );
    if ( number.exponent > 8 )
        gtk_entry_set_max_length (GTK_ENTRY (exchange_entry),
                        strlen( gtk_entry_get_text ( GTK_ENTRY ( exchange_entry ) ) - 1) );

    gsb_data_currency_link_set_first_currency ( link_number,
						gsb_currency_get_currency_from_combobox (combobox_1));
    gsb_data_currency_link_set_second_currency ( link_number,
						 gsb_currency_get_currency_from_combobox (combobox_2));
    gsb_data_currency_link_set_change_rate ( link_number, number );
    gsb_data_currency_link_set_modified_date ( link_number, gdate_today ( ) );

    strdate = gsb_format_gdate ( gsb_data_currency_link_get_modified_date ( link_number ) );

    if ( gsb_data_currency_link_get_invalid_link (link_number))
	invalid = "gtk-dialog-warning";
    else
	invalid = NULL;

    tmpstr = utils_real_get_string ( gsb_data_currency_link_get_change_rate ( link_number ) );
    gtk_list_store_set ( GTK_LIST_STORE (model),
			 &iter,
			 LINK_CURRENCY1_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_first_currency(link_number)),
			 LINK_EXCHANGE_COLUMN, tmpstr,
			 LINK_CURRENCY2_COLUMN, gsb_data_currency_get_name (gsb_data_currency_link_get_second_currency(link_number)),
             LINK_DATE_COLUMN, strdate,
			 LINK_INVALID_COLUMN, invalid,
			 -1 );
    g_free ( tmpstr );
    g_free ( strdate );

    /* set or hide the warning label */
    label = g_object_get_data (G_OBJECT (model), "warning_label");

    if ( gsb_data_currency_link_get_invalid_link (link_number))
    {
	gtk_label_set_markup ( GTK_LABEL (label),
			       gsb_data_currency_link_get_invalid_message (link_number));
	gtk_widget_show (label);
    }
    else
	gtk_widget_hide (label);

    gsb_file_set_modified ( TRUE );
    gsb_gui_navigation_update_home_page ( );

    return FALSE;
}


/**
 * called by a click on the "add link" in the configuration
 * add a new link and set the selection on it
 *
 * \param tree_view the tree view of the links list
 *
 * \return FALSE
 * */
gboolean gsb_currency_link_config_add_link ( GtkWidget *tree_view )
{
    gint link_number;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GSList *tmp_list;
    gint nbre_devises;

    tmp_list = gsb_data_currency_get_currency_list ();
    nbre_devises = g_slist_length (tmp_list);
    if (nbre_devises == 1)
    {
        dialogue_hint (_("You must create at least another currency to continue."),
                        _("The number of currencies is insufficient"));

        return FALSE;
    }

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW (tree_view));

    link_number = gsb_data_currency_link_new (0);

    /* we are sure that at least there is a currency with the number 1,
     * so we set the 2 currencies on the number 1, even if it makes that link
     * invalid, but it's not a problem because i think the user will change
     * it quickly... */
    gsb_data_currency_link_set_first_currency ( link_number, 1 );
    gsb_data_currency_link_set_second_currency ( link_number, 1 );

    gsb_currency_link_config_append_line ( model,
					   link_number,
					   &iter );

    gtk_tree_selection_select_iter ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)), &iter );
    gsb_file_set_modified ( TRUE );
    return FALSE;
}


/**
 * called by a click on the "remove link" in the configuration
 * remove the link
 * there is no warning message, perhaps sould be better, but i don't
 * think re-creating the deleted link is too difficult...
 *
 * \param tree_view the tree view of the links list
 *
 * \return FALSE
 * */
gboolean gsb_currency_link_config_remove_link ( GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if ( gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
					   &model,
					   &iter ))
	{
		GtkWidget *label;
		gint link_number;

		gtk_tree_model_get ( GTK_TREE_MODEL (model),
				&iter,
				LINK_NUMBER_COLUMN, &link_number,
				-1 );
		/* set here to sensitive the delete_link_button */
		gsb_data_currency_link_remove (link_number);
		gtk_list_store_remove ( GTK_LIST_STORE (model), &iter );
		gtk_widget_set_sensitive ( GTK_WIDGET ( g_object_get_data ( G_OBJECT (model), "hbox_line")),
				FALSE );

		/* hide the warning label */
		label = g_object_get_data (G_OBJECT (model), "warning_label");
		if ( gtk_widget_get_visible ( label ) )
			gtk_widget_hide (label);

		gsb_file_set_modified ( TRUE );
	}
    return FALSE;
}


/**
 * gère le clavier sur la liste des liens
 *
**/
gboolean gsb_currency_link_config_key_press ( GtkWidget *tree_view, GdkEventKey *ev )
{
    switch ( ev -> keyval )
    {
    case GDK_KEY_Delete:    /*  del  */
        gsb_currency_link_config_remove_link ( tree_view );
        return TRUE;
        break;
    }

    return FALSE;
}


/**
 * met à jour le lien à chaque changement du check_button
 *
**/
static gboolean gsb_currency_link_config_button_fixed_changed ( GtkWidget *checkbutton,
						  GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint link_number;

    if ( !gtk_tree_selection_get_selected ( gtk_tree_view_get_selection (
                        GTK_TREE_VIEW ( tree_view ) ),
					    &model,
					    &iter ) )
        return FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter, LINK_NUMBER_COLUMN, &link_number, -1 );

    gsb_data_currency_link_set_fixed_link ( link_number,
                        gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( checkbutton ) ) );

    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
