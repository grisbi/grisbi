/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
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

/* ./configure --with-balance-estimate */

/*
 * prefix bet : Balance Estimate Tab
 *
 * TODO : change the color of each line in the graph :
 * red if balance is less than 0.
 * orange if balance is less than the minimum desired balance.
 * TODO : add a select button to display the selected line in the array
 * in the scheduler tab or in the account tab.
 */

#include "include.h"
#include <config.h>

#ifdef ENABLE_BALANCE_ESTIMATE

/*START_INCLUDE*/
#include "balance_estimate_tab.h"
#include "./balance_estimate_config.h"
#include "./balance_estimate_data.h"
#include "./balance_estimate_hist.h"
#include "./utils_dates.h"
#include "./gsb_data_account.h"
#include "./gsb_data_budget.h"
#include "./gsb_calendar_entry.h"
#include "./gsb_data_category.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_fyear.h"
#include "./gsb_real.h"
#include "./gsb_scheduler.h"
#include "./gsb_transactions_list_sort.h"
#include "./main.h"
#include "./mouse.h"
#include "./navigation.h"
#include "./include.h"
#include "./structures.h"
#include "./traitement_variables.h"
#include "./erreur.h"
#include "./utils.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void bet_array_adjust_hist_data ( gint div_number,
                        gint sub_div_nb,
                        gsb_real amount,
                        GtkTreeModel *model );
static gint bet_array_date_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *itera,
                        GtkTreeIter *iterb,
                        gpointer data );
static gboolean  bet_array_entry_key_press ( GtkWidget *entry,
                        GdkEventKey *ev,
                        gpointer data );
static gboolean bet_array_initializes_account_settings ( gint account_number );
static void bet_array_list_add_substract_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gboolean bet_array_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev );
static void bet_array_list_context_menu ( GtkWidget *tree_view );
static void bet_array_list_redo_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gboolean bet_array_list_set_background_color ( GtkWidget *tree_view );
static void bet_array_list_update_balance ( GtkTreeModel *model );
static void bet_array_refresh_scheduled_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max );
static void bet_array_refresh_transactions_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max );
static gboolean  bet_array_start_date_focus_out ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data );
static gboolean bet_array_update_average_column (GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data);
/*END_STATIC*/

/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern gchar* bet_duration_array[];
extern GdkColor couleur_fond[0];
extern GdkColor couleur_bet_division;
extern GtkWidget *notebook_general;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/

enum bet_estimation_tree_columns {
    SPP_ESTIMATE_TREE_SELECT_COLUMN,    /* select column for the balance */
    SPP_ESTIMATE_TREE_DIVISION_COLUMN,  /* div_number */
    SPP_ESTIMATE_TREE_SUB_DIV_COLUMN,   /* sub_div_nb */
    SPP_ESTIMATE_TREE_DATE_COLUMN,
    SPP_ESTIMATE_TREE_DESC_COLUMN,
    SPP_ESTIMATE_TREE_DEBIT_COLUMN,
    SPP_ESTIMATE_TREE_CREDIT_COLUMN,
    SPP_ESTIMATE_TREE_BALANCE_COLUMN,
    SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
    SPP_ESTIMATE_TREE_AMOUNT_COLUMN,    /* the amount without currency */
    SPP_ESTIMATE_TREE_BALANCE_COLOR,
    SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
    SPP_ESTIMATE_TREE_NUM_COLUMNS
};



/*
 * 
 *
 * 
 */
void bet_array_update_estimate_tab ( void )
{
    bet_historical_populate_data ( );
    bet_array_refresh_estimate_tab ( );
}


/*
 * bet_date_sort_function
 * This function is called by the Tree Model to sort
 * two lines by date.
 */
static gint bet_array_date_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *itera,
                        GtkTreeIter *iterb,
                        gpointer data )
{
    GValue date_value_a = {0,};
    GValue date_value_b = {0,};
    GDate* date_a;
    GDate* date_b;

    if ( itera == NULL )
        return -1;
    if ( iterb == NULL)
        return -1;

    /* get first date to compare */
    gtk_tree_model_get_value ( model, itera, SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value_a );
    date_a = g_value_get_boxed ( &date_value_a );
    if ( date_a == NULL )
        return -1;

    /* get second date to compare */
    gtk_tree_model_get_value ( model, iterb, SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value_b );
    date_b = g_value_get_boxed ( &date_value_b );
    if ( date_b == NULL )
        return -1;

    gint result = g_date_compare (date_b, date_a);

    g_value_unset ( &date_value_b );
    g_value_unset ( &date_value_a );

    return result;
}


/*
 * bet_array_update_average_column
 *
 * This function is called for each line of the array.
 * It calculates the balance column by adding the amount of the line
 * to the balance of the previous line.
 * It calculates the minimum and the maximum values of the balance column.
 */
static gboolean bet_array_update_average_column ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gchar *str_balance = NULL;
    gchar *tmp_str;
    gchar *color_str = NULL;
    gint selected_account;
    gboolean select = FALSE;
    SBR *tmp_range = (SBR*) data;
    gsb_real amount;

    if ( tmp_range -> first_pass )
    {
        tmp_range -> first_pass = FALSE;
        return FALSE;
    }

    gtk_tree_model_get ( model, iter, SPP_ESTIMATE_TREE_SELECT_COLUMN, &select, -1 );
    if ( select )
    {
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        iter,
                        SPP_ESTIMATE_TREE_BALANCE_COLUMN, "",
                        -1 );

        return FALSE;
    }

    selected_account = gsb_gui_navigation_get_current_account ( );
    if ( selected_account == -1 )
        return FALSE;

    gtk_tree_model_get ( model, iter, SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &tmp_str, -1 );

    amount = gsb_real_get_from_string ( tmp_str );

    tmp_range -> current_balance = gsb_real_add ( tmp_range -> current_balance, amount );
    str_balance = gsb_real_get_string_with_currency ( tmp_range -> current_balance, 
                                gsb_data_account_get_currency ( selected_account ), TRUE );

    if ( tmp_range->current_balance.mantissa < 0 )
        color_str = "red";
    else
        color_str = NULL;
    
    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        iter,
                        SPP_ESTIMATE_TREE_BALANCE_COLUMN, str_balance,
                        SPP_ESTIMATE_TREE_BALANCE_COLOR, color_str,
                        -1 );
    g_free ( str_balance );

    return FALSE;
}


/*
 * bet_array_refresh_estimate_tab
 * This function clears the estimate array and calculates new estimates.
 * It updates the estimate graph.
 * This function is called when the refresh button is pressed and when
 * the balance estimate tab is selected.
 */
void bet_array_refresh_estimate_tab ( void )
{
    GtkWidget *notebook;
    GtkWidget *widget;
    GtkWidget *tree_view;
    GtkTreeIter iter;
    GtkTreeModel *tree_model;
    gchar *color_str = NULL;
    gchar *str_date_init;
    gchar *str_date_min;
    gchar *str_date_max;
    gchar *str_current_balance;
    gchar *str_amount;
    gchar *title;
    gchar *tmp_str;
    gint account_nb;
    GDate *date_init;
    GDate *date_min;
    GDate *date_max;
    gsb_real current_balance;
    SBR *tmp_range;
    GValue date_value = {0, };

    //~ devel_debug (NULL);
    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");
    tmp_range = initialise_struct_bet_range ( );

    /* find the selected account */
    account_nb = gsb_gui_navigation_get_current_account ( );

    if ( account_nb == -1 )
        return;

    /* Initializes account settings */
    bet_array_initializes_account_settings ( account_nb );

    /* calculate date_min and date_max with user choice */
    date_min = gsb_data_account_get_bet_start_date ( account_nb );

    date_init = gsb_date_copy ( date_min );
    g_date_subtract_days ( date_init, 1 );

    date_max = gsb_date_copy ( date_min );

    if ( g_date_get_day ( date_min ) == 1 )
    {
        g_date_add_months (date_max, gsb_data_account_get_bet_months ( account_nb ) - 1 );
        date_max = gsb_date_get_last_day_of_month ( date_max );
    }
    else
    {
        g_date_add_months (date_max, gsb_data_account_get_bet_months ( account_nb ) );
        g_date_subtract_days ( date_max, 1 );
    }

    widget = g_object_get_data ( G_OBJECT ( notebook ), "initial_date");
    gsb_calendar_entry_set_date ( widget, date_min );

    str_date_init = gsb_format_gdate ( date_init );
    str_date_min = gsb_format_gdate ( date_min );

    g_value_init ( &date_value, G_TYPE_DATE );
    g_value_set_boxed ( &date_value, date_init );

    str_date_max = gsb_format_gdate ( date_max );
    
    /* current balance may be in the future if there are transactions
     * in the future in the account. So we need to calculate the balance
     * of today */
    current_balance = gsb_data_account_calculate_current_day_balance (
                        account_nb, date_min );

    str_amount = gsb_real_save_real_to_string ( current_balance, 2 );
    str_current_balance = gsb_real_get_string_with_currency ( current_balance,
                        gsb_data_account_get_currency ( account_nb ), TRUE );

    if ( current_balance.mantissa < 0 )
        color_str = "red";
    else
        color_str = NULL;

    /* set the titles of tabs module budget */
    title = g_strdup_printf (
                        _("Balance estimate of the account \"%s\" from %s to %s"),
                        gsb_data_account_get_name ( account_nb ),
                        str_date_min, str_date_max );

    widget = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( notebook ), "bet_array_title") );
    gtk_label_set_label ( GTK_LABEL ( widget ), title );
    g_free ( title );

    title = g_strdup_printf (
                        _("Please select the data source for the account: \"%s\""),
                        gsb_data_account_get_name ( account_nb ) );
    widget = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( notebook ), "bet_hist_title") );
    gtk_label_set_markup ( GTK_LABEL ( widget ), title );
    g_free ( title );

    /* clear the model */
    tree_view = g_object_get_data ( G_OBJECT ( notebook ), "bet_estimate_treeview" );
    tree_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_store_clear ( GTK_TREE_STORE ( tree_model ) );

    tmp_str = g_strdup ( _("balance beginning of period") );
    gtk_tree_store_append ( GTK_TREE_STORE ( tree_model ), &iter, NULL );
    gtk_tree_store_set_value (GTK_TREE_STORE ( tree_model ), &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value);
    gtk_tree_store_set(GTK_TREE_STORE(tree_model), &iter,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date_min,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, tmp_str,
                        SPP_ESTIMATE_TREE_BALANCE_COLUMN, str_current_balance,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        SPP_ESTIMATE_TREE_BALANCE_COLOR, color_str,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_fond[0],
                        -1);

    g_value_unset ( &date_value );
    g_free ( str_date_init );
    g_free ( str_date_min );
    g_free ( str_date_max );
    g_free ( tmp_str );
    g_free ( str_current_balance );
    g_free ( str_amount );

    /* search data from the past */
    bet_historical_refresh_data ( tree_model, date_min, date_max );
    
    /* search transactions of the account which are in the period */
    bet_array_refresh_transactions_data ( tree_model,
                        account_nb,
                        date_min,
                        date_max );

    /* for each schedulded operation */
    bet_array_refresh_scheduled_data ( tree_model,
                        account_nb,
                        date_min,
                        date_max );

    g_free ( date_min );
    g_free ( date_init );
    g_free ( date_max );

    /* Calculate the balance column */
    tmp_range -> first_pass = TRUE;
    tmp_range -> current_balance = current_balance;

    gtk_tree_model_foreach ( GTK_TREE_MODEL ( tree_model ),
                        bet_array_update_average_column, tmp_range );

    bet_array_list_set_background_color ( tree_view );
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_array_create_page ( void )
{
    GtkWidget *notebook;
    GtkWidget *widget = NULL;
    GtkWidget *initial_date = NULL;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *previous = NULL;
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkTreeModel *sortable;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    gint iduration;

    devel_debug (NULL);
    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");
    vbox = gtk_vbox_new ( FALSE, 5 );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_account_duration", vbox );

    /* create the title */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( vbox ), align, FALSE, FALSE, 5) ;

    label = gtk_label_new ("Estimate array");
    gtk_container_add ( GTK_CONTAINER ( align ), label );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_array_title", label );

    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( vbox ), align, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );

    label = gtk_label_new ( _("Duration estimation") );
    gtk_misc_set_padding ( GTK_MISC (label), 5, 0 );
    gtk_box_pack_start( GTK_BOX ( hbox ), label, FALSE, FALSE, 5);

    spin_button = gtk_spin_button_new_with_range ( 1.0, 240.0, 1.0);
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), 1.0 );
    gtk_widget_grab_focus ( spin_button );

    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 5 );

    for (iduration = 0; bet_duration_array[iduration] != NULL; iduration++)
    {
        if (previous == NULL)
        {
            widget = gtk_radio_button_new_with_label ( NULL,
                        _(bet_duration_array[iduration]) );
            previous = widget;
        }  
        else 
        {
            widget = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( previous ),
                        _(bet_duration_array[iduration]) );
        }
        gtk_widget_set_name ( widget, bet_duration_array[iduration] );
        gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );
        g_signal_connect (G_OBJECT ( widget ),
                        "released",
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        spin_button );
    }

    g_object_set_data ( G_OBJECT ( notebook ), "bet_account_previous", previous );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_account_widget", widget );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_account_spin_button", spin_button );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( previous ), TRUE );

    g_signal_connect ( G_OBJECT ( spin_button ),
                        "value-changed",
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        NULL );

    label = gtk_label_new ( COLON ( _("Start date" ) ) );
    gtk_misc_set_padding ( GTK_MISC (label), 5, 0 );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    initial_date = gsb_calendar_entry_new ( FALSE );
    g_signal_connect ( G_OBJECT ( initial_date ),
                        "focus-out-event",
                        G_CALLBACK ( bet_array_start_date_focus_out ),
                        NULL );
    g_signal_connect ( G_OBJECT ( initial_date ),
			            "key-press-event",
			            G_CALLBACK ( bet_array_entry_key_press ),
			            NULL );
    g_object_set_data ( G_OBJECT ( notebook ), "initial_date", initial_date );
    gtk_box_pack_start ( GTK_BOX (hbox), initial_date, FALSE, FALSE, 0 );

    /* create the estimate treeview */
    tree_view = gtk_tree_view_new ( );
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( tree_view ), TRUE );
    g_object_set_data ( G_OBJECT ( notebook), "bet_estimate_treeview", tree_view );

    /* create the model */
    tree_model = gtk_tree_store_new ( SPP_ESTIMATE_TREE_NUM_COLUMNS,
                    G_TYPE_BOOLEAN,     /* SPP_ESTIMATE_TREE_SELECT_COLUMN */
                    G_TYPE_INT,         /* SPP_ESTIMATE_TREE_DIVISION_COLUMN */
                    G_TYPE_INT,         /* SPP_ESTIMATE_TREE_SUB_DIV_COLUMN */
				    G_TYPE_STRING,      /* SPP_ESTIMATE_TREE_DATE_COLUMN */
				    G_TYPE_STRING,      /* SPP_ESTIMATE_TREE_DESC_COLUMN */
				    G_TYPE_STRING,      /* SPP_ESTIMATE_TREE_DEBIT_COLUMN */
				    G_TYPE_STRING,      /* SPP_ESTIMATE_TREE_CREDIT_COLUMN */
				    G_TYPE_STRING,      /* SPP_ESTIMATE_TREE_BALANCE_COLUMN */
				    G_TYPE_DATE,        /* SPP_ESTIMATE_TREE_SORT_DATE_COLUMN */
				    G_TYPE_STRING,      /* SPP_ESTIMATE_TREE_AMOUNT_COLUMN */
                    G_TYPE_STRING,      /* SPP_ESTIMATE_TREE_BALANCE_COLOR */
                    GDK_TYPE_COLOR );   /* SPP_ESTIMATE_TREE_BACKGROUND_COLOR */

    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    /* sort by date */
    sortable = gtk_tree_model_sort_new_with_model ( GTK_TREE_MODEL ( tree_model ) );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( tree_model ),
				      SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
				      (GtkTreeIterCompareFunc) bet_array_date_sort_function,
				      NULL, NULL );
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tree_model),
					 SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, GTK_SORT_DESCENDING);

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_widget_show ( scrolled_window );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );
    gtk_widget_show ( scrolled_window );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
		                GTK_WIDGET ( scrolled_window ), TRUE, TRUE, 5 );

    /* Date column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
					    _("Date"), cell,
					    "text", SPP_ESTIMATE_TREE_DATE_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
                        NULL);
    g_object_set (cell, "xalign", 0.5, NULL);

    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    //~ gtk_tree_view_column_set_min_width(GTK_TREE_VIEW_COLUMN(column), 150);
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* Description column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
					    _("Description"), cell,
					    "text", SPP_ESTIMATE_TREE_DESC_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
					    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ),
				GTK_TREE_VIEW_COLUMN ( column ) );
    //~ gtk_tree_view_column_set_min_width (column, 300 );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* Debit column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
						_("Debit"), cell,
					    "text", SPP_ESTIMATE_TREE_DEBIT_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    //~ gtk_tree_view_column_set_min_width(column, 140);
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment(column, 1);
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* Credit column */
    cell = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    column = gtk_tree_view_column_new_with_attributes (
					    _("Credit"), cell,
					    "text", SPP_ESTIMATE_TREE_CREDIT_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
						NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    //~ gtk_tree_view_column_set_min_width(column, 140);
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* Balance column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
					    _("Balance"), cell,
					    "text", SPP_ESTIMATE_TREE_BALANCE_COLUMN,
                        "foreground", SPP_ESTIMATE_TREE_BALANCE_COLOR,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    //~ gtk_tree_view_column_set_min_width(column, 170);
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment(column, 1);
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);
    gtk_tree_view_column_set_resizable ( column, TRUE );
    g_signal_connect ( G_OBJECT ( tree_view ),
		                "button-press-event",
		                G_CALLBACK ( bet_array_list_button_press ),
		                NULL );

    gtk_widget_show_all ( vbox );

    return vbox;
}


/**
 *
 *
 *
 *
 * */
void bet_array_refresh_scheduled_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max )
{
    GtkTreeIter iter;
    GSList* tmp_list;

    //~ devel_debug (NULL);
    tmp_list = gsb_data_scheduled_get_scheduled_list();

    while (tmp_list)
    {
        gchar *str_amount;
        gchar *str_debit = NULL;
        gchar *str_credit = NULL;
        const gchar *str_description = NULL;
        gchar *str_date;
        gint scheduled_number;
        gint account_number;
        gint transfer_account_number;
        gint div_number;
        gint sub_div_nb;
        GDate *date;
        GValue date_value = {0, };
        gsb_real amount;

        scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list->data );

        tmp_list = tmp_list->next;

        /* ignore children scheduled operations */
        if (gsb_data_scheduled_get_mother_scheduled_number ( scheduled_number ) )
            continue;

        /* ignore scheduled operations of other account */
        account_number = gsb_data_scheduled_get_account_number ( scheduled_number );

        div_number = bet_data_get_div_number ( scheduled_number, FALSE );
        sub_div_nb = bet_data_get_sub_div_nb ( scheduled_number, FALSE );
        if ( div_number > 0
         &&
         bet_data_search_div_hist ( account_number, div_number, 0 ) )
            continue;

        if ( gsb_data_scheduled_is_transfer ( scheduled_number ) )
        {
            transfer_account_number = gsb_data_scheduled_get_account_number_transfer (
                        scheduled_number );
            if ( transfer_account_number == selected_account )
            {
                str_description = g_strdup_printf ( _("Transfer between account: %s\n"
                        "and account: %s"),
                        gsb_data_account_get_name ( transfer_account_number ),
                        gsb_data_account_get_name ( account_number ) );
            
                amount = gsb_real_opposite ( gsb_data_scheduled_get_amount (
                        scheduled_number ) );
            }
            else if ( account_number == selected_account )
            {
                str_description = g_strdup_printf ( _("Transfer between account: %s\n"
                        "and account: %s"),
                        gsb_data_account_get_name ( account_number ),
                        gsb_data_account_get_name ( transfer_account_number ) );

                amount = gsb_data_scheduled_get_amount ( scheduled_number );
            }
            else
                continue;
        }
        else if ( account_number == selected_account )
        {
            str_description = gsb_data_scheduled_get_notes ( scheduled_number );

            if ( !str_description || !strlen ( str_description ) )
                str_description = gsb_data_payee_get_name (
                            gsb_data_scheduled_get_party_number ( scheduled_number ), TRUE );

            amount = gsb_data_scheduled_get_amount ( scheduled_number );
        }
        else
            continue;

        str_amount = gsb_real_save_real_to_string ( amount, 2 );
        if (amount.mantissa < 0)
            str_debit = gsb_real_get_string_with_currency ( gsb_real_abs ( amount ),
                        gsb_data_scheduled_get_currency_number ( scheduled_number ), TRUE );
        else
            str_credit = gsb_real_get_string_with_currency ( gsb_real_abs ( amount ),
                        gsb_data_scheduled_get_currency_number ( scheduled_number ), TRUE );

        /* calculate each instance of the scheduled operation
         * in the range from date_min (today) to date_max */
        date = gsb_data_scheduled_get_date ( scheduled_number );

        while (date != NULL && g_date_valid ( date ) )
        {
            if ( g_date_compare ( date, date_max ) > 0 )
                break;
            if ( g_date_compare ( date, date_min ) < 0 )
            {
                date = gsb_scheduler_get_next_date ( scheduled_number, date );
                continue;
            }
            if ( g_date_valid ( date ) == FALSE )
                return;
            str_date = gsb_format_gdate ( date );

            g_value_init ( &date_value, G_TYPE_DATE );
            if ( date == NULL )
                return;
            g_value_set_boxed ( &date_value, date ); 

            /* add a line in the estimate array */
            gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &iter, NULL );
            gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );
            gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, 0,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, 0,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
                        SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
                        SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        -1 );

            g_value_unset ( &date_value );
            g_free ( str_date );
            date = gsb_scheduler_get_next_date ( scheduled_number, date );
        }

        g_free ( str_amount );
        if ( str_credit )
            g_free ( str_credit );
        if ( str_debit )
            g_free ( str_debit );
    }
}


/**
 *
 *
 *
 *
 * */
void bet_array_refresh_transactions_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max )
{
    GDate *date_jour_1;
    GSList *tmp_list;

    //~ devel_debug (NULL);
    date_jour_1 = gsb_date_copy ( date_min );
    if ( g_date_get_day ( date_min ) > 1 )
        g_date_set_day ( date_jour_1, 1 );

    /* search transactions of the account which are in the period */
    tmp_list = gsb_data_transaction_get_transactions_list ( );

    while ( tmp_list )
    {
        GtkTreeIter iter;
        gchar* str_amount;
        gchar* str_debit = NULL;
        gchar* str_credit = NULL;
        const gchar* str_description;
        gchar* str_date;
        gint transaction_number;
        gint transfer_number;
        gint account_number;
        gint transfer_account_number;
        gint div_number;
        gint sub_div_nb;
        const GDate *date;
        GValue date_value = {0, };
        gsb_real amount;

        transaction_number = gsb_data_transaction_get_transaction_number ( tmp_list->data );
        tmp_list = tmp_list -> next;

        account_number =  gsb_data_transaction_get_account_number ( transaction_number );
        if ( account_number != selected_account )
            continue;

        date = gsb_data_transaction_get_date ( transaction_number );

        /* ignore transaction which are after date_max */
        if ( g_date_compare (date, date_max ) > 0 )
            continue;

        /* ignore splitted transactions */
        if ( gsb_data_transaction_get_mother_transaction_number (
         transaction_number ) != 0 )
            continue;

        /* Ignore transactions that are before the first day of the month */
        if ( g_date_compare ( date, date_jour_1 ) < 0 )
            continue;

        amount = gsb_data_transaction_get_amount ( transaction_number );

        div_number = bet_data_get_div_number ( transaction_number, TRUE );
        sub_div_nb = bet_data_get_sub_div_nb ( transaction_number, TRUE );
        //~ if ( div_number == 18 || div_number == 27 )
        //~ printf ("div_number = %d sub_div_number = %d\n", div_number, sub_div_nb);
        if ( div_number > 0
         &&
         bet_data_search_div_hist ( account_number, div_number, 0 ) )
        {
            
            //~ printf ("après test div_number = %d sub_div_nb = %d\n", div_number, sub_div_nb);
            if ( g_date_get_month ( date ) == g_date_get_month ( date_jour_1 ) )
                bet_array_adjust_hist_data ( div_number, sub_div_nb, amount, tab_model );
        }

        /* ignore transaction which are before date_min */
        if ( g_date_compare ( date, date_min ) < 0 )
            continue;

        str_date = gsb_format_gdate ( date );
        g_value_init ( &date_value, G_TYPE_DATE );
        g_value_set_boxed ( &date_value, date );

        str_amount = gsb_real_save_real_to_string ( amount, 2 );

        if (amount.mantissa < 0)
            str_debit = gsb_real_get_string_with_currency ( gsb_real_abs ( amount ),
                        gsb_data_transaction_get_currency_number ( transaction_number), TRUE );
        else
            str_credit = gsb_real_get_string_with_currency ( gsb_real_abs ( amount ),
                        gsb_data_transaction_get_currency_number (transaction_number), TRUE);

        transfer_number =
                        gsb_data_transaction_get_contra_transaction_number (
                        transaction_number );
        if ( transfer_number > 0 )
        {
            transfer_account_number = gsb_data_transaction_get_account_number (
                        transfer_number );
            str_description = g_strdup_printf ( _("Transfer between account: %s\n"
                        "and account: %s"),
                        gsb_data_account_get_name ( account_number ),
                        gsb_data_account_get_name ( transfer_account_number ) );
        }
        else
        {
            str_description = gsb_data_transaction_get_notes ( transaction_number );

            if (!str_description || !strlen (str_description))
                str_description = gsb_data_payee_get_name (
                            gsb_data_transaction_get_party_number ( transaction_number ), TRUE );
        }

        /* add a line in the estimate array */
        gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &iter, NULL );
        gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );
        gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, 0,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, 0,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
                        SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
                        SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        -1 );

        g_value_unset ( &date_value );
        g_free ( str_date );
        g_free ( str_amount );
        if ( str_debit )
            g_free ( str_debit );
        if ( str_credit )
            g_free ( str_credit );
    }
}


/**
 *
 *
 *
 *
 * */
void bet_array_list_add_new_line ( GtkTreeModel *tab_model,
                        GtkTreeModel *model,
                        GtkTreeIter *iter,
                        GDate *date_min,
                        GDate *date_max )
{
    GtkTreeIter tab_iter;
    GDate *date;
    GDate *date_jour;
    GValue date_value = {0, };
    gchar *str_date;
    gchar *str_description;
    gchar *str_value;
    gchar *str_debit = NULL;
    gchar *str_credit = NULL;
    gchar *str_amount;
    gint div_number;
    gint sub_div_nb;
    gsb_real amount;

    //~ devel_debug (NULL);
    date_jour = gdate_today ( );
    date = gsb_date_get_last_day_of_month ( date_min );
        
    /* initialise les données de la ligne insérée */
    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), iter,
                        SPP_HISTORICAL_DESC_COLUMN, &str_description,
                        SPP_HISTORICAL_RETAINED_COLUMN, &str_value,
                        SPP_HISTORICAL_RETAINED_AMOUNT, &str_amount,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
                        -1 );

    if ( sub_div_nb > 0 )
    {
        g_free ( str_description );
        str_description = bet_data_get_div_name ( div_number, sub_div_nb, NULL );
    }

    amount = gsb_real_get_from_string ( str_amount );

    if (amount.mantissa < 0)
        str_debit = gsb_real_get_string_with_currency ( gsb_real_opposite ( amount ),
                        bet_data_get_selected_currency ( ), TRUE );
    else
        str_credit = str_value;

    while (date != NULL && g_date_valid ( date ) )
    {
        if ( g_date_compare ( date, date_max ) > 0 )
            break;

        if ( g_date_compare ( date, date_min ) < 0 
         ||
         g_date_get_month ( date ) < g_date_get_month ( date_jour ) )
        {
            g_date_add_months ( date, 1 );
            continue;
        }

        if ( g_date_valid ( date ) == FALSE )
            return;

        str_date = gsb_format_gdate ( date );

        g_value_init ( &date_value, G_TYPE_DATE );
        if ( date == NULL )
            return;
        g_value_set_boxed ( &date_value, date ); 

        /* add a line in the estimate array */
        gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &tab_iter, NULL );
        gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );
        gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, div_number,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, sub_div_nb,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
                        SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
                        SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        -1);

        g_value_unset ( &date_value );
        g_free ( str_date );
        g_date_add_months ( date, 1 );
        date = gsb_date_get_last_day_of_month ( date );
    }

    g_free ( str_description );
    if ( str_debit )
        g_free ( str_debit );
    g_free ( str_value );
    g_free ( str_amount );
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean bet_array_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev )
{
	/* show the popup */
	if ( ev -> button == RIGHT_BUTTON )
        bet_array_list_context_menu ( tree_view );

    return FALSE;
}


/**
 * Pop up a menu with several actions to apply to array_list.
 *
 * \param
 *
 */
void bet_array_list_context_menu ( GtkWidget *tree_view )
{
    GtkWidget *menu, *menu_item;
    GtkTreeModel *model;
    GtkTreeSelection *tree_selection;
    GtkTreeIter iter;
    gboolean select = FALSE;

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;
    gtk_tree_model_get ( model, &iter, SPP_ESTIMATE_TREE_SELECT_COLUMN, &select, -1 );

    menu = gtk_menu_new ();

    /* Add substract amount menu */
    if ( select == FALSE )
    {
        menu_item = gtk_image_menu_item_new_with_label ( _("Subtract to the balance") );
        gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                            gtk_image_new_from_stock ( GTK_STOCK_REMOVE,
                            GTK_ICON_SIZE_MENU ) );
    }
    else
    {
        menu_item = gtk_image_menu_item_new_with_label ( _("Adding to the balance") );
        gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                            gtk_image_new_from_stock ( GTK_STOCK_ADD,
                            GTK_ICON_SIZE_MENU ) );
    }
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_add_substract_menu ),
                        tree_selection );

    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new ( ) );
    gtk_widget_show ( menu_item );

    /* Delete item */
    menu_item = gtk_image_menu_item_new_with_label ( _("Reset data") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_REFRESH,
						GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_redo_menu ),
                        tree_selection );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Finish all. */
    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time ( ) );
}


/**
 * add or substract amount of the balance
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_array_list_add_substract_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean select = FALSE;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( model, &iter, SPP_ESTIMATE_TREE_SELECT_COLUMN, &select, -1 );
    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                        SPP_ESTIMATE_TREE_SELECT_COLUMN, 1 - select,
                        -1);
    bet_array_list_update_balance ( model );
}


/**
 * delete a row
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_array_list_redo_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    bet_array_update_estimate_tab ( );
}


/**
 * Cette fonction recalcule le montant des données historiques en fonction de la
 * consommation mensuelle précédente. affiche le nouveau montant si même signe ou 0 
 * et un message pour budget dépassé.
 * 
 * */
void bet_array_adjust_hist_data ( gint div_number,
                        gint sub_div_nb,
                        gsb_real amount,
                        GtkTreeModel *model )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gchar* str_date;
        gchar *str_desc;
        gchar* str_debit = NULL;
        gchar* str_credit = NULL;
        gchar *str_amount;
        gchar *div_name;
        GDate *date;
        GDate *date_today;
        gsb_real number;
        gint sign = 1;
        gint tmp_div_number;
        gint tmp_sub_div_nb;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, &tmp_div_number,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, &tmp_sub_div_nb,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, &str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, &str_desc,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &str_amount,
                        -1 );

            if ( tmp_div_number == 0 || tmp_div_number != div_number )
                continue;
            //~ printf ("ligne analysée div_number = %d, sub_div_number = %d\n", tmp_div_number, tmp_sub_div_nb);

            if ( tmp_sub_div_nb == 0 || tmp_sub_div_nb == sub_div_nb )
            {
                div_name = bet_data_get_div_name ( tmp_div_number, tmp_sub_div_nb, FALSE );
				//~ printf ("div_number = %d, sub_div_number = %d div_name = %s str_desc = %s\n",
                         //~ tmp_div_number, tmp_sub_div_nb, div_name, str_desc );
                date = gsb_parse_date_string ( str_date );
                date_today = gdate_today ( );
                if ( g_date_get_month ( date ) - g_date_get_month ( date_today ) == 0 )
                {
                    number = gsb_real_import_from_string ( str_amount );
                    if ( number .mantissa < 0 )
                        sign = -1;
                    number = gsb_real_sub ( number, amount );
                    if ( number.mantissa == 0 )
                        gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &iter );
                    else
                    {
                        if ( str_amount )
                            g_free ( str_amount );
                        if ( str_desc )
                            g_free ( str_desc );

                        str_amount = gsb_real_save_real_to_string ( number, 2 );
                        if ( number.mantissa < 0 && sign < 0 )
                        {
                            str_debit = gsb_real_get_string_with_currency (
                                        gsb_real_abs ( number ),
                                        bet_data_get_selected_currency ( ),
                                        TRUE );
                            str_desc = g_strconcat ( div_name, _(" (still available)"), NULL);
                        }
                        else if ( number.mantissa > 0 && sign > 0 )
                        {
                            str_credit = gsb_real_get_string_with_currency (
                                        gsb_real_abs ( number ),
                                        bet_data_get_selected_currency ( ),
                                        TRUE );
                            str_desc = g_strconcat ( div_name, _(" (yet to receive)"), NULL);
                        }
                        else if ( sign == -1 )
                        {
                            str_debit = gsb_real_get_string_with_currency (
                                        null_real,
                                        bet_data_get_selected_currency ( ),
                                        TRUE );
                            if ( str_amount )
                                g_free ( str_amount );
                            str_amount = g_strdup ( "0.00" );
                            str_desc = g_strconcat ( div_name, _(" (budget exceeded)"), NULL);
                        }
                        else
                        {
                            str_credit = gsb_real_get_string_with_currency (
                                        null_real,
                                        bet_data_get_selected_currency ( ),
                                        TRUE );
                            if ( str_amount )
                                g_free ( str_amount );
                            str_amount = g_strdup ( "0.00" );
                            str_desc = g_strconcat ( div_name, _(" (budget exceeded)"), NULL);
                        }

                        gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                                        SPP_ESTIMATE_TREE_DESC_COLUMN, str_desc,
                                        SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
                                        SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
                                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                                        -1 );

                        g_free ( str_desc );
                        g_free ( str_credit );
                        g_free ( str_debit );
                        g_free ( str_amount );
                    }
                    g_free ( str_date );
                    g_free ( div_name );
                    break;
                }
                g_free ( div_name );
            }
            g_free ( str_date );
            g_free ( str_desc );
            g_free ( str_amount );
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );
    }
}


/**
 *
 *
 *
 *
 * */
void bet_array_list_update_balance ( GtkTreeModel *model )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_first ( model, &iter) )
    {
        gchar *str_current_balance;
        gsb_real current_balance;
        SBR *tmp_range;

        gtk_tree_model_get ( model, &iter,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &str_current_balance, -1 ); 
        current_balance = gsb_real_get_from_string ( str_current_balance );

        tmp_range = initialise_struct_bet_range ( );
        tmp_range -> first_pass = TRUE;
        tmp_range -> current_balance = current_balance;

        gtk_tree_model_foreach ( GTK_TREE_MODEL ( model ),
                        bet_array_update_average_column, tmp_range );
    }
}


/**
 *
 *
 *
 *
 * */
gboolean  bet_array_entry_key_press ( GtkWidget *entry,
                        GdkEventKey *ev,
                        gpointer data )
{
    gint account_nb;
    GDate *date;

    switch ( ev -> keyval )
    {
    case GDK_Escape :
        account_nb = gsb_gui_navigation_get_current_account ( );
	    date = gsb_data_account_get_bet_start_date ( account_nb );
        gtk_entry_set_text ( GTK_ENTRY ( entry ),
                        gsb_format_gdate ( date ) );
	    break;
    case GDK_KP_Enter :
    case GDK_Return :
        bet_array_start_date_focus_out ( entry, NULL, data );
        break;
    }

    return FALSE;
}
/**
 *
 *
 *
 *
 * */
gboolean  bet_array_start_date_focus_out ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data )
{
    gint account_nb;
    GDate *date;

    gtk_editable_select_region ( GTK_EDITABLE ( entry ), 0, 0 );
    account_nb = gsb_gui_navigation_get_current_account ( );
    date = gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
    gsb_data_account_set_bet_start_date ( account_nb, date );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_array_refresh_estimate_tab ( );

    return FALSE;
}


/**
 * set the background colors of the list
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean bet_array_list_set_background_color ( GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if ( !tree_view )
	    return FALSE;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ));

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gint div_number = 0;
        gint sub_div_number;
        gint current_color = 0;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
			            &iter,
			            SPP_ESTIMATE_TREE_DIVISION_COLUMN, &div_number,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, &sub_div_number,
			            -1 );
            if ( div_number == 0 )
            {
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_fond[current_color],
                        -1 );
                current_color = !current_color;
            }
            else
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_bet_division,
                        -1 );
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_array_initializes_account_settings ( gint account_number )
{
    GtkWidget *notebook;
    GtkWidget *ancestor;
    GtkWidget *widget = NULL;
    GtkWidget *button = NULL;
    gint param;
    gint months;

    //~ devel_debug_int ( account_number );
    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );
    button = g_object_get_data ( G_OBJECT ( notebook ), "bet_account_spin_button" );
    param = gsb_data_account_get_bet_spin_range ( account_number );
    months = gsb_data_account_get_bet_months ( account_number );

    if ( param == 0 )
    {
        widget = g_object_get_data ( G_OBJECT ( notebook ), "bet_account_previous" );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, 240.0 );
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months );
    }
    else
    {
        widget = g_object_get_data ( G_OBJECT ( notebook ), "bet_account_widget" );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, 20.0 );
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months / 12.0 );
    }

    ancestor = g_object_get_data ( G_OBJECT ( notebook ), "historical_data" );

    param = gsb_data_account_get_bet_hist_data ( account_number );

    if ( param == 1 )
        button = g_object_get_data ( G_OBJECT ( notebook ), "button_2" );
    else
        button = g_object_get_data ( G_OBJECT ( notebook ), "button_1" );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );

    param = gsb_data_account_get_bet_hist_fyear ( account_number );
    widget = g_object_get_data ( G_OBJECT ( notebook ), "fyear_combo" );
    bet_historical_set_fyear_from_combobox ( widget, param );

    return FALSE;
}
/**
 *
 *
 *
 *
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
