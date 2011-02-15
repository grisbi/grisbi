/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*			2004-2008 Benjamin Drieu (bdrieu@april.org) 	                  */
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
 * \file gsb_form_config.c
 * configuration page for the form
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_form_config.h"
#include "utils.h"
#include "dialog.h"
#include "gsb_account.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_form.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "navigation.h"
#include "utils_str.h"
#include "traitement_variables.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_form_config_add_column ( void );
static gboolean gsb_form_config_add_line ( void );
static gboolean gsb_form_config_change_account_choice ( GtkWidget *combobox,
						 gpointer null );
static gboolean gsb_form_config_change_column_size ( GtkWidget *tree_view,
					      GtkAllocation *allocation,
					      gpointer null );
static gboolean gsb_form_config_check_for_removing ( gint account_number,
					      gint removing_row );
static GtkWidget *gsb_form_config_create_buttons_table ( void );
static GtkWidget *gsb_form_config_create_sizing_buttons_line ( void );
static GtkListStore *gsb_form_config_create_store ( void );
static GtkWidget *gsb_form_config_create_tree_view ( GtkListStore *store );
static gboolean gsb_form_config_drag_begin ( GtkWidget *tree_view,
				      GdkDragContext *drag_context,
				      gpointer null );
static gboolean gsb_form_config_drag_end ( GtkWidget *tree_view,
				    GdkDragContext *drag_context,
				    gpointer null );
static gboolean gsb_form_config_fill_store ( gint account_number );
static void gsb_form_config_make_configuration_box ( GtkWidget *vbox_parent );
static gboolean gsb_form_config_realized ( GtkWidget *tree_view,
				    gpointer null );
static gboolean gsb_form_config_remove_column ( void );
static gboolean gsb_form_config_remove_line ( void );
static gboolean gsb_form_config_toggle_element_button ( GtkWidget *toggle_button );
static gboolean gsb_form_config_update_form_config ( gint account_number );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *form_transaction_part;
extern gint saved_allocation_size;
extern GtkWidget *window;
/*END_EXTERN*/



/** tmp for drag'n drop */
static gint start_drag_column;
static gint start_drag_row;

/* buttons to config the form */
static GtkWidget *form_config_buttons[TRANSACTION_FORM_WIDGET_NB-3];

/** the option menu which contains all the accounts, to configure the form */
static GtkWidget *accounts_combobox;

/** the tree view */
static GtkWidget *form_config_tree_view;



/**
 * create and return the page to be set in the configuration page
 * this is normally the only function which can be called from outside
 *
 * \param
 *
 * \return the complete page
 * */
GtkWidget *gsb_form_config_create_page ( void )
{
    GtkWidget *vbox_pref;

    vbox_pref = new_vbox_with_title_and_icon ( _("Transaction form"), "form.png" );

    /* add the page into the box */

    gsb_form_config_make_configuration_box (vbox_pref);

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}


/**
 * create the box for configuring the form,
 * append that box into the vbox_parent
 *
 * \param vbox_parent the box into we want to make the configuration box
 *
 * \return
 * */
void gsb_form_config_make_configuration_box ( GtkWidget *vbox_parent )
{
    GtkWidget *sw;
    GtkWidget *hbox;
    GtkWidget *paddingbox;
    GtkListStore* list_store;

    /* create the paddingbox into the parent */
    paddingbox = new_paddingbox_with_title ( vbox_parent, TRUE, _("Form structure preview"));

    /* we can organize the form
     * either the same for all the accounts
     * either each account has its own configuration */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX (paddingbox), hbox, FALSE, FALSE, 0 );

    /* the accounts option_menu */
    accounts_combobox = gsb_account_create_combo_list ((GtkSignalFunc) gsb_form_config_change_account_choice, NULL, FALSE );

    /*create the scolled window for tree_view */
    sw = gtk_scrolled_window_new ( NULL, NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX ( paddingbox), sw, TRUE, TRUE, 0 );

    /* create the tree_view */
    list_store = gsb_form_config_create_store ();
    form_config_tree_view = gsb_form_config_create_tree_view (list_store);
    g_object_unref (G_OBJECT(list_store));
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw ),
                        form_config_tree_view );

    /* set the buttons line to increase/decrease the form */
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_form_config_create_sizing_buttons_line (), FALSE, FALSE, 0 );

    /* set the box with the buttons */
    paddingbox = new_paddingbox_with_title ( vbox_parent, FALSE, _("Form structure content") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_form_config_create_buttons_table (), FALSE, FALSE, 0 );

    gtk_widget_show_all (paddingbox);
}


/**
 * create and return the store (a list) for the configuration of the form
 *
 * \param
 *
 * \return the new store
 * */
GtkListStore *gsb_form_config_create_store ( void )
{
    GtkListStore *store;

    store = gtk_list_store_new ( MAX_WIDTH,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING,
				 G_TYPE_STRING );
     return store;
}


/**
 * create the tree_view for the form configuration
 * set the model given in param
 * set the columns and all the connections
 *
 * \param model the model to set in the tree_view
 *
 * \return the tree_view
 * */
GtkWidget *gsb_form_config_create_tree_view ( GtkListStore *store )
{
    GtkWidget *tree_view;
    gint column;
    GtkTargetEntry target_entry[] = {
	{ "text", GTK_TARGET_SAME_WIDGET, 0 }
    };
    gchar *columns_titles[6] = {
	N_("Col 1"),
	N_("Col 2"),
	N_("Col 3"),
	N_("Col 4"),
	N_("Col 5"),
	N_("Col 6")
    };

    if ( !store )
	return NULL;

    /* create the tree_view, it would be better to set it into a scrolled window,
     * but when we do that, there is automatickly a line on the bottom of the list
     * and it's annoying for the drag'n drop ; i didn't succeed in removing that line...[cedric]
     * the problem of the choice here is if we try to increase too much the width of the column,
     * the config box increase too */

    tree_view = gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ));
    
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view))),
				  GTK_SELECTION_NONE );

    /* set the columns */

    for ( column=0 ; column< MAX_WIDTH ; column++ )
    {
	GtkTreeViewColumn *tree_view_column;

	tree_view_column = gtk_tree_view_column_new_with_attributes ( columns_titles[column],
								      gtk_cell_renderer_text_new (),
								      "text", column,
								      NULL );
	gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
				      GTK_TREE_VIEW_COLUMN ( tree_view_column));
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN (tree_view_column),
					  GTK_TREE_VIEW_COLUMN_FIXED );
	gtk_tree_view_column_set_resizable ( GTK_TREE_VIEW_COLUMN (tree_view_column),
					     TRUE );
    }

    /* we will fill the form and size the columns when the window will be realized */
    g_signal_connect ( G_OBJECT ( tree_view ),
		       "realize",
		       G_CALLBACK ( gsb_form_config_realized ),
		       NULL );

    /* save and modify the form when change the size of the columns */
    g_signal_connect ( G_OBJECT (tree_view),
		       "size_allocate",
		       G_CALLBACK ( gsb_form_config_change_column_size ),
		       NULL );

    /* enable the drag'n drop, we need to use low-level api because
     * gtk_tree_view api can only move the entire row, not only a cell
     * (at least, didn't find how...) */
    gtk_drag_source_set ( tree_view,
			  GDK_BUTTON1_MASK,
			  target_entry, 1,
			  GDK_ACTION_MOVE );
    g_signal_connect ( G_OBJECT (tree_view),
		       "drag-begin",
		       G_CALLBACK ( gsb_form_config_drag_begin ),
		       NULL );
    gtk_drag_dest_set ( tree_view,
			GTK_DEST_DEFAULT_ALL,
			target_entry, 1,
			GDK_ACTION_MOVE );
    g_signal_connect ( G_OBJECT (tree_view),
		       "drag-end",
		       G_CALLBACK ( gsb_form_config_drag_end ),
		       NULL );

    return tree_view;
}


/**
 * create the line to change the size of the tree_view / form
 *
 * \param
 *
 * \return a hbox wich contains the buttons
 * */
GtkWidget *gsb_form_config_create_sizing_buttons_line ( void )
{
    GtkWidget *hbox, *hbox2;
    GtkWidget *button;
    GtkWidget *label;

    hbox = gtk_hbox_new ( FALSE,
			  5 );

    /* line number choice */

    hbox2 = gtk_hbox_new ( FALSE,
			   5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 hbox2,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox2 );

    button = gtk_button_new_from_stock ( GTK_STOCK_REMOVE );
    g_signal_connect ( G_OBJECT ( button ),
		       "clicked",
		       G_CALLBACK ( gsb_form_config_remove_line ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( button ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 button,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( button );

    label = gtk_label_new ( _("rows"));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    button = gtk_button_new_from_stock ( GTK_STOCK_ADD );
    g_signal_connect ( G_OBJECT ( button ),
		       "clicked",
		       G_CALLBACK ( gsb_form_config_add_line ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( button ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 button,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( button );

    /* columns number choice */

    hbox2 = gtk_hbox_new ( FALSE,
			   5 );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       FALSE,
		       0 );
    gtk_widget_show ( hbox2 );

    button = gtk_button_new_from_stock ( GTK_STOCK_REMOVE );
    g_signal_connect ( G_OBJECT ( button ),
		       "clicked",
		       G_CALLBACK ( gsb_form_config_remove_column ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( button ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 button,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( button );

    label = gtk_label_new ( _("columns"));
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 label,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( label );

    button = gtk_button_new_from_stock ( GTK_STOCK_ADD );
    g_signal_connect ( G_OBJECT ( button ),
		       "clicked",
		       G_CALLBACK ( gsb_form_config_add_column ),
		       NULL );
    gtk_button_set_relief ( GTK_BUTTON( button ),
			    GTK_RELIEF_NONE );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			 button,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show ( button );

    return hbox;
}


/**
 * create the table of buttons and set the signals
 *
 * \param
 *
 * \return the new table of buttons
 * */
GtkWidget *gsb_form_config_create_buttons_table ( void )
{
    GtkWidget *table;
    gint current_element_number;
    gint row, column;

    /* the table is 3x6 buttons */
    table = gtk_table_new ( 3, 6, FALSE );

    /* the date, debit and credit are obligatory, so begin to number 4 */

    current_element_number = 4;

    for ( row=0 ; row<3 ; row++ )
	for ( column=0 ; column<6 ; column++ )
	{
	    gchar *string;
	    gchar *changed_string;

	    string = _(gsb_form_widget_get_name (current_element_number));

	    if ( string )
	    {
		/* the max string in the button is 10 characters */

		changed_string = limit_string ( string,
						10 );

		form_config_buttons[column + row*6] = gtk_toggle_button_new_with_label ( changed_string );
		g_object_set_data ( G_OBJECT ( form_config_buttons[column + row*6] ),
				    "element_number",
				    GINT_TO_POINTER ( current_element_number));
		g_signal_connect ( G_OBJECT ( form_config_buttons[column + row*6] ),
				   "toggled",
				   G_CALLBACK (gsb_form_config_toggle_element_button),
				   NULL );
		gtk_table_attach_defaults ( GTK_TABLE ( table ),
					    form_config_buttons[column + row*6],
					    column, column+1,
					    row, row+1 );

		/* set the tooltip with the real name */
		string = g_strconcat ( " ", string, " ", NULL );
		gtk_widget_set_tooltip_text ( GTK_WIDGET (form_config_buttons[column + row*6]),
					      string);
		g_free (changed_string);
	    }

	    current_element_number++;
	}
    return table;
}


/**
 * fill the form and set the correct buttons as active/passive
 *
 * \param account_number
 *
 * \return FALSE
 * */
gboolean gsb_form_config_update_form_config ( gint account_number )
{
    gint row;
    gint column;
    gint current_element_number;
    
    /* fill the store */
    gsb_form_config_fill_store (account_number);

    /* show/hide the necessary columns in the tree view */
    for ( column=0 ; column<MAX_WIDTH ; column++ )
	gtk_tree_view_column_set_visible ( gtk_tree_view_get_column ( GTK_TREE_VIEW ( form_config_tree_view ),
								      column ),
					   column<gsb_data_form_get_nb_columns (account_number ));

    /* active/unactive the buttons, begin on the fourth element number because the
     * date, debit and credit are obligatory */

    current_element_number = 4;

    for ( row=0 ; row<3 ; row++ )
	for ( column=0 ; column<6 ; column++ )
	{
	    if ( column + row*6 < TRANSACTION_FORM_WIDGET_NB - 3 &&
		 form_config_buttons[column + row*6] )
	    {
		g_signal_handlers_block_by_func ( G_OBJECT ( form_config_buttons[column + row*6] ),
						  G_CALLBACK ( gsb_form_config_toggle_element_button ),
						  NULL );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( form_config_buttons[column + row*6] ),
					       gsb_data_form_look_for_value ( account_number,
									      current_element_number,
									      NULL, NULL ));
		g_signal_handlers_unblock_by_func ( G_OBJECT ( form_config_buttons[column + row*6] ),
						    G_CALLBACK ( gsb_form_config_toggle_element_button ),
						    NULL );
	    }
	    current_element_number++;
	}
    return FALSE;
}


/**
 * called if we change the account in the option menu of the accounts
 *
 * \param combobox the combobox of the list of accounts
 *
 * \return FALSE
 * */
gboolean gsb_form_config_change_account_choice ( GtkWidget *combobox,
						 gpointer null )
{
    gint account_number;

    account_number = gsb_account_get_combo_account_number (combobox);
    gsb_form_config_update_form_config ( account_number );

    return FALSE;
}


/**
 * called when toggle a button of the form configuration, append or remove
 * the value from the tree view
 *
 * \param toggle_button the button we click
 *
 * \return FALSE
 * */
gboolean gsb_form_config_toggle_element_button ( GtkWidget *toggle_button )
{
    gint element_number;
    gint no_second_element;
    gint i, j;
    gint account_number;

    /* get the element number */
    element_number = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( toggle_button ),
							   "element_number" ));
    /* set the second element number if necessary */
    switch ( element_number )
    {
	case TRANSACTION_FORM_TYPE:
	    /* 	    c'est le mode de paiement, on met le chq */
	    no_second_element = TRANSACTION_FORM_CHEQUE;
	    break;

	case TRANSACTION_FORM_CHEQUE:
	    /* 	    c'est le chq, on met mode de paiement */
	    no_second_element = TRANSACTION_FORM_TYPE;
	    break;

	case TRANSACTION_FORM_DEVISE:
	    /* 	    c'est la devise, on met le button de change */
	    no_second_element = TRANSACTION_FORM_CHANGE;
	    break;

	case TRANSACTION_FORM_CHANGE:
	    /* 	    c'est le button de change, on met la devise */
	    no_second_element = TRANSACTION_FORM_DEVISE;
	    break;

	default:
	    no_second_element = -1;
    }

    account_number = gsb_account_get_combo_account_number ( accounts_combobox );

    /* update the table */
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( toggle_button )))
    {
	/* button is on, append the element */
	gint place_trouvee = 0;
	gint ligne_premier_elt = -1;
	gint colonne_premier_elt = -1;

	for ( i=0 ; i < gsb_data_form_get_nb_rows (account_number) ; i++)
	    for ( j=0 ; j < gsb_data_form_get_nb_columns (account_number) ; j++ )
		if ( !gsb_data_form_get_value ( account_number, j, i ) )
		{
		    /* if only 1 element, end here, else continue to look after the second one */
		    if ( no_second_element == -1 )
		    {
			/* 			il n'y a qu'un elt */
			
			gsb_data_form_set_value ( account_number,
						  j,
						  i,
						  element_number );
			place_trouvee = 1;
			i = gsb_data_form_get_nb_rows (account_number);
			j = gsb_data_form_get_nb_columns (account_number);
		    }
		    else
		    {
			/* there are 2 elements */
			if ( ligne_premier_elt == -1 )
			{
			    /* found the place for the first element */
			    ligne_premier_elt = i;
			    colonne_premier_elt = j;
			}
			else
			{
			    /* found the place for the second element */
			    gsb_data_form_set_value ( account_number,
						      colonne_premier_elt,
						      ligne_premier_elt,
						      element_number );
			    gsb_data_form_set_value ( account_number,
						      j,
						      i,
						      no_second_element );
			    place_trouvee = 1;
			    i = gsb_data_form_get_nb_rows (account_number);
			    j = gsb_data_form_get_nb_columns (account_number);
			}
		    }
		}

	if ( place_trouvee )
	{
	    /* there is a place for the element, active if necessary an associated element */
	    if ( no_second_element != -1 )
	    {
		g_signal_handlers_block_by_func ( G_OBJECT ( form_config_buttons[no_second_element-4] ),
						  G_CALLBACK ( gsb_form_config_toggle_element_button ),
						  NULL );
		gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( form_config_buttons[no_second_element-4] ),
					       TRUE );
		g_signal_handlers_unblock_by_func ( G_OBJECT ( form_config_buttons[no_second_element-4] ),
						    G_CALLBACK ( gsb_form_config_toggle_element_button ),
						    NULL );
	    }
	}
	else
	{
	    /* there is no place to add an element */
	    g_signal_handlers_block_by_func ( G_OBJECT ( toggle_button ),
					      G_CALLBACK ( gsb_form_config_toggle_element_button ),
					      NULL );
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( toggle_button ),
					   FALSE );
	    g_signal_handlers_unblock_by_func ( G_OBJECT ( toggle_button ),
						G_CALLBACK ( gsb_form_config_toggle_element_button ),
						NULL );

	    if ( no_second_element == -1 )
		dialogue_hint ( _("There is no place enough to put the element. You need to increase "
                          "the number of rows or columns to add an element."),
                        _("The table is full"));
	    else
		dialogue_hint ( _("There is no place enough to put the two elements (you have clicked on "
                          "an element which contains two). You need to increase the number of rows "
                          "or columns to add the elements."),
                        _("The table is full"));

	    return TRUE;
	}
    }
    else
    {
	/* un-toggle the button */
	if ( no_second_element != -1 )
	{
	    g_signal_handlers_block_by_func ( G_OBJECT ( form_config_buttons[no_second_element-4] ),
					      G_CALLBACK ( gsb_form_config_toggle_element_button ),
					      NULL );
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( form_config_buttons[no_second_element-4] ),
					   FALSE );
	    g_signal_handlers_unblock_by_func ( G_OBJECT ( form_config_buttons[no_second_element-4] ),
						G_CALLBACK ( gsb_form_config_toggle_element_button ),
						NULL );
	}

	for ( i=0 ; i < gsb_data_form_get_nb_rows (account_number) ; i++ )
	    for ( j=0 ; j < gsb_data_form_get_nb_columns (account_number) ; j++ )
		if ( gsb_data_form_get_value (account_number,
					      j,
					      i ) == element_number )
		{
		    gsb_data_form_set_value ( account_number,
					      j,
					      i,
					      0 );
		    if ( no_second_element == -1 )
		    {
			i = gsb_data_form_get_nb_rows (account_number);
			j = gsb_data_form_get_nb_columns (account_number);
		    }
		    else
		    {
			element_number = no_second_element;
			no_second_element = -1;
			i = 0;
			j = 0;
		    }
		}
    }

    /* fill the list */
    gsb_form_config_fill_store (account_number);
    gsb_form_fill_from_account (account_number);

    gsb_form_config_update_from_account (
                        gsb_account_get_combo_account_number ( accounts_combobox ) );
    /* update the form */
    gsb_form_config_fill_store (account_number);
    gsb_form_create_widgets ();

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * fill the configuration store according to the organization for the account given
 *
 * \param account_number
 *
 * \return FALSE
 * */
gboolean gsb_form_config_fill_store ( gint account_number )
{
    gint row;
    GtkListStore *store;

    store = GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( form_config_tree_view )));
						
    gtk_list_store_clear ( store );

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
    {
	GtkTreeIter iter;
	gint column;

	gtk_list_store_append ( GTK_LIST_STORE ( store ),
				&iter );

	for ( column = 0 ; column < gsb_data_form_get_nb_columns (account_number) ; column++ )
	    gtk_list_store_set ( GTK_LIST_STORE ( store ),
				 &iter,
				 column, _(gsb_form_widget_get_name (gsb_data_form_get_value ( account_number,
											     column,
											     row ))),
				 -1 );
    }
    return FALSE;
}


/**
 * called once the window is realized,
 * fill the form and size the columns according to the configuration
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean gsb_form_config_realized ( GtkWidget *tree_view,
				    gpointer null )
{
    gint column;
    gint account_number;
    gint width;

    if ( !assert_account_loaded())
      return FALSE;

    account_number = gsb_account_get_combo_account_number ( accounts_combobox );
    width = tree_view -> allocation.width;

    /* fill and update the form list and buttons */
    gsb_form_config_update_form_config(account_number);

    for ( column=0 ; column < gsb_data_form_get_nb_columns (account_number) ; column++ )
    {
	gtk_tree_view_column_set_fixed_width ( gtk_tree_view_get_column ( GTK_TREE_VIEW ( tree_view ),
									  column ),
					       gsb_data_form_get_width_column ( account_number,
										column ) * width / 100 );
    }

    gdk_window_set_cursor ( tree_view -> window, 
			    gdk_cursor_new ( GDK_FLEUR ) );

    return FALSE;
}



/**
 * called when change the size of the columns,
 * save the percent and change the size of the form according the new size
 *
 * \param tree_view
 * \param allocation
 *
 * \return FALSE
 * */
gboolean gsb_form_config_change_column_size ( GtkWidget *tree_view,
					      GtkAllocation *allocation,
					      gpointer null )
{
    gint column;
    gint account_number;
    gint i;

    if ( !GTK_WIDGET_REALIZED (tree_view))
	return FALSE;

    account_number = gsb_account_get_combo_account_number ( accounts_combobox );

    for (i=0 ; i<gsb_data_account_get_accounts_amount () ; i++)
    {
	    for ( column=0 ; column < gsb_data_form_get_nb_columns (i) ; column++ )
	    {
		gint size_column;

		size_column = gtk_tree_view_column_get_width ( gtk_tree_view_get_column ( GTK_TREE_VIEW ( tree_view ),
											  column ));
		gsb_data_form_set_width_column ( i,
						 column,
						 size_column * 100 / allocation -> width );
	    }
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    /* update the form if needed */
	saved_allocation_size = 0;
	gsb_form_allocate_size ( NULL, &(form_transaction_part -> allocation), NULL );
    gsb_form_create_widgets ();

    return FALSE;
}


/**
 * called when the user click on "add line"
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_config_add_line ( void )
{
    gint account_number;

    account_number = gsb_account_get_combo_account_number ( accounts_combobox );

    if ( gsb_data_form_get_nb_rows (account_number) == MAX_HEIGHT )
	return FALSE;

    gsb_data_form_set_nb_rows ( account_number,
				gsb_data_form_get_nb_rows (account_number) + 1 );

    /* update the form */
    gsb_form_config_fill_store (account_number);
    gsb_form_create_widgets ();

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * called when the user click on "remove line"
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_config_remove_line ( void )
{
    gint column;
    gint account_number;
    gint nb_rows;
    gint nb_columns;

    account_number = gsb_account_get_combo_account_number ( accounts_combobox );
    nb_rows = gsb_data_form_get_nb_rows (account_number);

    if ( nb_rows == 1 )
	return FALSE;

    /* check if it's possible */
    if ( !gsb_form_config_check_for_removing ( account_number,
					       1 ))
	return FALSE;

    nb_columns = gsb_data_form_get_nb_columns (account_number);

    /* remove the row */
    nb_rows--;
    gsb_data_form_set_nb_rows ( account_number,
				nb_rows );

    /* move automatickly the values inside the new tinier form */
    for ( column=0 ; column< nb_columns ; column++ )
    {
	if ( gsb_data_form_get_value ( account_number,
				       column,
				       nb_rows))
	{
	    /* there is something inside the part wich will be removed, so look for the first
	     * place possible to move it */

	    gint tmp_row, tmp_column;

	    for ( tmp_row=0 ; tmp_row < nb_rows ; tmp_row++ )
		for ( tmp_column=0 ; tmp_column < nb_columns ; tmp_column++ )
		    if ( !gsb_data_form_get_value ( account_number,
						    tmp_column,
						    tmp_row ))
		    {
			gsb_data_form_set_value ( account_number,
						  tmp_column,
						  tmp_row,
						  gsb_data_form_get_value ( account_number,
									    column,
									    nb_rows));
			gsb_data_form_set_value ( account_number,
						  column,
						  nb_rows,
						  0 );
			tmp_row = nb_rows;
			tmp_column = nb_columns;
		    }
	}
    }

    /* update the form */
    gsb_form_config_fill_store (account_number);
    gsb_form_create_widgets ();

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * called when the user click on "add column"
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_config_add_column ( void )
{
    gint account_number;
    gint nb_columns;
    gint new_size;

    account_number = gsb_account_get_combo_account_number ( accounts_combobox );
    nb_columns = gsb_data_form_get_nb_columns (account_number);

    if ( nb_columns == MAX_WIDTH )
	return FALSE;

    /* split by 2 the size of the current last column to add the new column */
    new_size = gsb_data_form_get_width_column ( account_number,
						nb_columns - 1) / 2;
    gsb_data_form_set_width_column ( account_number,
				     nb_columns - 1,
				     new_size );
    gsb_data_form_set_width_column ( account_number,
				     nb_columns,
				     new_size );
    gsb_data_form_set_nb_columns ( account_number,
				   nb_columns + 1 );

    /* show the result */
    gsb_form_config_realized ( form_config_tree_view, NULL );
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}


/**
 * called when the user click on "remove column"
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_config_remove_column ( void )
{
    gint row;
    gint account_number;
    gint nb_columns;
    
    account_number = gsb_account_get_combo_account_number ( accounts_combobox );
    nb_columns = gsb_data_form_get_nb_columns (account_number);

    if ( nb_columns == 1 )
	return FALSE;

    /* check if it's possible */
    if ( !gsb_form_config_check_for_removing ( account_number,
					       0 ))
	return FALSE;

    /* erase the last column */
    nb_columns--;
    gsb_data_form_set_nb_columns ( account_number,
				   nb_columns );

    /* move the values in the last column to another place */
    for ( row=0 ; row< gsb_data_form_get_nb_rows (account_number) ; row++ )
    {
	if ( gsb_data_form_get_value (account_number,
				      nb_columns,
				      row ))
	{
	    /* found something, look for the first place to set it */

	    gint tmp_row, tmp_column;

	    for ( tmp_row=0 ; tmp_row< gsb_data_form_get_nb_rows (account_number) ; tmp_row++ )
		for ( tmp_column=0 ; tmp_column<nb_columns ; tmp_column++ )
		    if ( !gsb_data_form_get_value ( account_number,
						    tmp_column,
						    tmp_row ))
		    {
			gsb_data_form_set_value ( account_number,
						  tmp_column,
						  tmp_row,
						  gsb_data_form_get_value ( account_number,
									    nb_columns,
									    row ));
			gsb_data_form_set_value ( account_number,
						  nb_columns,
						  row,
						  0 );
			tmp_row = gsb_data_form_get_nb_rows (account_number);
			tmp_column = nb_columns;
		    }
	}
    }

    /* change the size of the last column */
    gsb_data_form_set_width_column ( account_number,
				     nb_columns - 1,
				     gsb_data_form_get_width_column ( account_number,
								      nb_columns )
				     +
				     gsb_data_form_get_width_column ( account_number,
								      nb_columns ));
    gsb_data_form_set_width_column ( account_number,
				     nb_columns,
				     0 );

    /* fill the list */
    gsb_form_config_realized ( form_config_tree_view, NULL );
    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return FALSE;
}



/**
 * check if we can remove a row or a column according the number
 * of values inside
 * it is possible to remove a row or column only if grisbi can replace
 * the hidden element in the form without that row/column
 *
 * \param account_number
 * \param removing_row if TRUE it's a row we want to remove, else it's a column
 *
 * \return TRUE ok we can remove it, FALSE else
 * */
gboolean gsb_form_config_check_for_removing ( gint account_number,
					      gint removing_row )
{
    gint values;
    gint rows;
    gint columns;

    rows = gsb_data_form_get_nb_rows (account_number);
    columns = gsb_data_form_get_nb_columns (account_number);

    if ( !rows
	 ||
	 !columns )
	return FALSE;

    if ( removing_row )
	rows--;
    else
	columns--;

    /* the minimum of values is 3 : date, debit, credit*/

    if ( rows * columns < 3 )
	return FALSE;

    values = gsb_data_form_get_values_total (account_number);

     if ( values <= rows*columns )
	return TRUE;
    else
	return FALSE;
}


/**
 * called when we begin a drag,
 * find what cell was under the cursor and change it
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_form_config_drag_begin ( GtkWidget *tree_view,
				      GdkDragContext *drag_context,
				      gpointer null )
{
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    GdkWindow *drawable;
    GdkRectangle rectangle;
    GdkPixbuf *pixbuf_cursor;

    /* get the cell coord */
    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );
    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
				    x,
				    y,
				    &path,
				    &tree_column,
				    NULL, NULL );

    if ( !path
	 ||
	 !tree_column )
	return FALSE;

    start_drag_column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
				       tree_column );
    start_drag_row = utils_str_atoi ( gtk_tree_path_to_string ( path ));

    /* draw the new cursor */
    drawable = gtk_tree_view_get_bin_window (GTK_TREE_VIEW ( tree_view ));
    gtk_tree_view_get_cell_area ( GTK_TREE_VIEW ( tree_view ),
				  path,
				  tree_column,
				  &rectangle );
    pixbuf_cursor = gdk_pixbuf_get_from_drawable ( NULL,
						   GDK_DRAWABLE (drawable),
						   gdk_colormap_get_system (),
						   rectangle.x, rectangle.y,
						   0, 0,
						   rectangle.width, rectangle.height );
    gtk_drag_source_set_icon_pixbuf ( tree_view,
				      pixbuf_cursor );
    g_object_unref (pixbuf_cursor);

    return FALSE;
}
					

/**
 * called when we end a drag,
 * find what cell was under the cursor and do the split between the 2 cells
 *
 * \param tree_view
 * \param drag_context
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_form_config_drag_end ( GtkWidget *tree_view,
				    GdkDragContext *drag_context,
				    gpointer null )
{
    gint x, y;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gint end_drag_row;
    gint end_drag_column;
    gint buffer;
    gint account_number;

    /* get the cell position */
    gdk_window_get_pointer ( gtk_tree_view_get_bin_window ( GTK_TREE_VIEW ( tree_view )),
			     &x,
			     &y,
			     FALSE );
    gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW ( tree_view ),
				    x,
				    y,
				    &path,
				    &tree_column,
				    NULL,
				    NULL );

    if ( !path
	 ||
	 !tree_column )
	return FALSE;

    end_drag_column = g_list_index ( gtk_tree_view_get_columns ( GTK_TREE_VIEW ( tree_view )),
				     tree_column );
    end_drag_row = utils_str_atoi ( gtk_tree_path_to_string ( path ));

    /* if we are on the same cell, go away */
    if ( start_drag_row == end_drag_row
	 &&
	 start_drag_column == end_drag_column )
	return ( FALSE );

    /* swap the cells in the tab */
    account_number = gsb_account_get_combo_account_number ( accounts_combobox );

    buffer = gsb_data_form_get_value ( account_number,
				       start_drag_column,
				       start_drag_row );
    gsb_data_form_set_value ( account_number,
			      start_drag_column,
			      start_drag_row,
			      gsb_data_form_get_value ( account_number,
							end_drag_column,
							end_drag_row ));
    gsb_data_form_set_value ( account_number,
			      end_drag_column,
			      end_drag_row,
			      buffer );

    /* fill the list */
    gsb_form_config_fill_store (account_number);
    gsb_form_fill_from_account (account_number);

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    return (FALSE);
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */






