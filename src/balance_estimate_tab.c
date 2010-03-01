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
#include "./balance_estimate_data.h"
#include "./balance_estimate_hist.h"
#include "./utils_dates.h"
#include "./gsb_data_account.h"
#include "./gsb_data_budget.h"
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
static void bet_array_create_page ( GtkWidget *notebook );
static gint bet_array_date_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *itera,
                        GtkTreeIter *iterb,
                        gpointer user_data );
static void bet_array_list_add_substract_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gboolean bet_array_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev );
static void bet_array_list_context_menu ( GtkWidget *tree_view );
static void bet_array_list_delete_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static void bet_array_list_update_balance ( GtkTreeModel *model );
static void bet_array_refresh_scheduled_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max );
static void bet_array_refresh_transactions_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max );
static gboolean bet_array_update_average_column (GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data);
static void bet_graph_create_page ( GtkWidget *notebook );
static gboolean bet_graph_update_graph ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static void bet_parameter_account_selection_changed ( GtkTreeSelection *tree_selection,
                        gpointer user_data );
static void bet_parameter_create_page ( GtkWidget *notebook );
static void bet_parameter_duration_button_clicked ( GtkWidget *togglebutton,
                        GtkWidget *spin_button );
static gboolean bet_parameter_duration_number_changed ( GtkWidget *spin_button,
                        GtkWidget *togglebutton );
static void bet_parameter_duration_period_clicked ( GtkWidget *togglebutton,
                        GtkWidget *button );
static void bet_parameter_select_account ( gint account_nb );
static gboolean bet_parameter_update_list_accounts ( GtkWidget *tree_view,
                        GtkTreeModel *tree_model );
/*END_STATIC*/

/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/

enum bet_account_tree_columns {
    SPP_ACCOUNT_TREE_NUM_COLUMN,
    SPP_ACCOUNT_TREE_NAME_COLUMN,
    SPP_ACCOUNT_TREE_NUM_COLUMNS
};

enum bet_estimation_tree_columns {
    SPP_ESTIMATE_TREE_SELECT_COLUMN,  /* select column for the balance */
    SPP_ESTIMATE_TREE_DATE_COLUMN,
    SPP_ESTIMATE_TREE_DESC_COLUMN,
    SPP_ESTIMATE_TREE_DEBIT_COLUMN,
    SPP_ESTIMATE_TREE_CREDIT_COLUMN,
    SPP_ESTIMATE_TREE_BALANCE_COLUMN,
    SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
    SPP_ESTIMATE_TREE_AMOUNT_COLUMN,	/* the amount without currency */
    SPP_ESTIMATE_TREE_BALANCE_COLOR,
    SPP_ESTIMATE_TREE_NUM_COLUMNS
};

static gchar* bet_duration_array[] = {
    N_("Month"),
    N_("Year"),
    NULL
};

/**
 * the notebook of the bet 
 * */
GtkWidget *bet_container = NULL;


/*
 * bet_create_balance_estimate_tab
 *
 * This function create the widget (notebook) which contains all the
 * balance estimate interface. This widget is added in the main window
 */
GtkWidget *bet_array_create_estimate_page ( void )
{
    GtkWidget   *notebook;
    GtkWidget *tree_view = NULL;

    devel_debug (NULL);
    /* initialise structures */
    
    /* create a notebook for array and graph */
    notebook = gtk_notebook_new ( );
    gtk_widget_show ( notebook );
    bet_container = notebook;

    /****** Parameter page ******/
    bet_parameter_create_page ( notebook );
    tree_view = g_object_get_data ( G_OBJECT ( notebook ), "bet_account_treeview");
    if ( tree_view == NULL )
        return NULL;

    /****** Estimation array page ******/
    bet_array_create_page ( notebook );

    /****** Estimation graph page ******/
    bet_graph_create_page ( notebook );

    /****** Estimation graph page ******/
    bet_historical_create_page ( notebook );

    bet_historical_populate_data ( );
    bet_array_refresh_estimate_tab ( );

    return notebook;
}


/*
 * bet_array_update_estimate_tab
 *
 * This function is called each time that "Balance estimate" is selected in the selection tree.
 */
void bet_array_update_estimate_tab ( void )
{
    GtkWidget *tree_view;
    GtkTreeModel *tree_model;

    //~ devel_debug (NULL);
     /* fill the account list */
    tree_view = g_object_get_data ( G_OBJECT ( bet_container ), "bet_account_treeview" );
    tree_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( bet_data_get_maj ( ) == TRUE )
    {
        bet_data_set_maj ( FALSE );
        bet_array_refresh_estimate_tab ( );
    }
    else
        bet_parameter_update_list_accounts ( tree_view, GTK_TREE_MODEL ( tree_model ) );

}


/*
 * bet_date_sort_function
 * This function is called by the Tree Model to sort
 * two lines by date.
 */
static gint bet_array_date_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *itera,
                        GtkTreeIter *iterb,
                        gpointer user_data )
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

    selected_account = bet_parameter_get_account_selected ( );
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
 * bet_graph_update_graph
 * This function is called for each line of the estimate array and it updates
 * the graph.
 */
static gboolean bet_graph_update_graph ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gsb_real balance;
    gchar* balance_str = NULL;
    GValue date_value = {0,};

    /* get balance */
    gtk_tree_model_get ( model, iter, SPP_ESTIMATE_TREE_BALANCE_COLUMN, &balance_str, -1 );
    balance = gsb_real_get_from_string ( balance_str );
    g_free ( balance_str );

    /* get date */
    gtk_tree_model_get_value ( model, iter,
			            SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
			            &date_value);
    /*
       GDate* date = g_value_get_boxed(&date_value);
       guint32 dt = g_date_get_julian(date);
       */
    g_value_unset(&date_value);
/* xxx ça semble pas fini ici... */
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
    GtkWidget *widget;
    GtkWidget *tree_view;
    GtkTreeIter iter;
    GtkTreeModel *tree_model;
    gchar *account_name = NULL;
    gchar *str_date_init;
    gchar *str_date_min;
    gchar *str_date_max;
    gchar *str_current_balance;
    gchar *str_amount;
    gchar *title;
    gchar *tmp_str;
    gint selected_account;
    GDate *date_init;
    GDate *date_min;
    GDate *date_max;
    gsb_real current_balance;
    SBR *tmp_range;
    GValue date_value = {0, };

    devel_debug (NULL);
    tmp_range = initialise_struct_bet_range ( );

    /* find the selected account */
    selected_account = bet_parameter_get_account_selected ( );

    if ( selected_account == -1 )
        return;

    /* calculate date_min and date_max with user choice */
    date_min = gdate_today ();
    if ( etat.bet_deb_period == 1 )
        g_date_set_day ( date_min, 1 );

    date_init = gsb_date_copy ( date_min );
    g_date_subtract_days ( date_init, 1 );

    date_max = gdate_today ();

    if ( etat.bet_end_period == 1 )
    {
        g_date_add_months (date_max, etat.bet_months - 1 );
        date_max = gsb_date_get_last_day_of_month ( date_max );
    }
    else
        g_date_add_months (date_max, etat.bet_months );

    str_date_init = gsb_format_gdate ( date_init );
    str_date_min = gsb_format_gdate ( date_min );

    g_value_init ( &date_value, G_TYPE_DATE );
    g_value_set_boxed ( &date_value, date_init );

    str_date_max = gsb_format_gdate ( date_max );
    
    /* current balance may be in the future if there are transactions
     * in the future in the account. So we need to calculate the balance
     * of today */
    current_balance = gsb_data_account_calculate_current_day_balance (
                        selected_account, date_min );

    str_amount = gsb_real_save_real_to_string ( current_balance, 2 );
    str_current_balance = gsb_real_get_string_with_currency ( current_balance,
                        gsb_data_account_get_currency ( selected_account ), TRUE );

    title = g_strdup_printf (
                        _("Balance estimate of the account \"%s\" from %s to %s"),
                        gsb_data_account_get_name ( selected_account ),
                        str_date_min, str_date_max );

    widget = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( bet_container ), "bet_array_title") );
    gtk_label_set_label ( GTK_LABEL ( widget ), title );
    widget = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( bet_container ), "bet_graph_title") );
    gtk_label_set_label ( GTK_LABEL ( widget ), title );
    g_free ( title );
    g_free ( account_name );

    /* clear the model */
    tree_view = g_object_get_data ( G_OBJECT ( bet_container ), "bet_estimate_treeview" );
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
                        selected_account,
                        date_min,
                        date_max );

    /* for each schedulded operation */
    bet_array_refresh_scheduled_data ( tree_model,
                        selected_account,
                        date_min,
                        date_max );

    g_free ( date_min );
    g_free ( date_max );

    /* Calculate the balance column */
    tmp_range -> first_pass = TRUE;
    tmp_range -> current_balance = current_balance;

    gtk_tree_model_foreach ( GTK_TREE_MODEL ( tree_model ),
                        bet_array_update_average_column, tmp_range );

    /* update graph */
    widget = g_object_get_data ( G_OBJECT ( bet_container ), "bet_graph_curve" );
    gtk_curve_reset ( GTK_CURVE ( widget ) );
    gtk_tree_model_foreach ( GTK_TREE_MODEL ( tree_model ),
                        bet_graph_update_graph, widget );
}


/*
 * bet_parameter_duration_period_clicked
 * This function is called when a radio button is called to change the inial period.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_period property of the bet container
 */
static void bet_parameter_duration_period_clicked ( GtkWidget *togglebutton, GtkWidget *button )
{
    GtkWidget *ancestor;
    GtkWidget *widget;
    const gchar *name;

    //~ devel_debug (NULL);
    if ( button )
        g_signal_handlers_block_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_parameter_duration_period_clicked ),
                        button );

    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );

    ancestor = g_object_get_data ( G_OBJECT ( bet_container ), "bet_account_duration" );
    if ( gtk_widget_is_ancestor ( togglebutton, ancestor ) == FALSE )
    {
        widget = utils_get_child_widget_by_name ( ancestor, name );
        if ( widget )
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
    }

    if ( g_strcmp0 ( name, "button_1" ) == 0 )
    {
        etat.bet_deb_period = 1;
        etat.bet_end_period = 1;
        if ( gtk_widget_is_ancestor ( togglebutton, ancestor ) == FALSE )
        {
            widget = utils_get_child_widget_by_name ( ancestor, "button_3" );
            if ( widget )
                gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE ); 
        }
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
    }
    else if ( g_strcmp0 ( name, "button_2" ) == 0 )
    {
        etat.bet_deb_period = 2;
        etat.bet_end_period = 2;
        if ( gtk_widget_is_ancestor ( togglebutton, ancestor ) == FALSE )
        {
            widget = utils_get_child_widget_by_name ( ancestor, "button_4" );
            if ( widget )
                gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE ); 
        }
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
    }
    else if ( g_strcmp0 ( name, "button_3" ) == 0 )
        etat.bet_end_period = 1;

    else if ( g_strcmp0 ( name, "button_4" ) == 0 )
        etat.bet_end_period = 2;

    if ( button )
        g_signal_handlers_unblock_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_parameter_duration_period_clicked ),
                        button );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_array_refresh_estimate_tab ( );
}


/*
 * bet_parameter_duration_button_clicked
 * This function is called when a radio button is called to change the estimate duration.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_months property of the bet container
 */
static void bet_parameter_duration_button_clicked ( GtkWidget *togglebutton,
                        GtkWidget *spin_button )
{
    GtkWidget *ancestor;
    GtkWidget *widget;
    const gchar *name;

    //~ devel_debug (NULL);
    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );

    ancestor = g_object_get_data ( G_OBJECT ( bet_container ), "bet_account_duration" );
    if ( gtk_widget_is_ancestor ( togglebutton, ancestor ) == FALSE )
    {
        widget = utils_get_child_widget_by_name ( ancestor, name );
        if ( widget )
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
    }

    if ( g_strcmp0 ( name, "Year" ) == 0 )
    {
        etat.bet_spin_range = 1;
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( spin_button ), 1.0, 20.0 );
        if ( etat.bet_months > 20 )
        {
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), 20.0 );
            etat.bet_months = 240;
        }
        else
            etat.bet_months *= 12;
    }
    else
    {
        etat.bet_spin_range = 0;
        etat.bet_months = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button ) );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( spin_button ), 1.0, 240.0 );
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_array_refresh_estimate_tab ( );
}


/*
 * bet_duration_button changed
 * This function is called when a spin button is changed.
 * It copies the new duration from the spin_button into the bet_months property of
 * the bet container
 */
gboolean bet_parameter_duration_number_changed ( GtkWidget *spin_button,
                        GtkWidget *togglebutton )
{
    GtkWidget *ancestor;
    GtkWidget *widget;
    const gchar *name;
    gboolean toggled;

    //~ devel_debug (NULL);
    etat.bet_months = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button ) );

    toggled = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( togglebutton ) );
    if ( toggled == 1 )
        etat.bet_months *= 12;

    name = gtk_widget_get_name ( GTK_WIDGET ( spin_button ) );
    ancestor = g_object_get_data ( G_OBJECT ( bet_container ), "bet_account_duration" );
    if ( gtk_widget_is_ancestor ( togglebutton, ancestor ) == FALSE )
    {
        widget = utils_get_child_widget_by_name ( ancestor, name );
        if ( widget )
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( widget ),
                        gtk_spin_button_get_value (
                        GTK_SPIN_BUTTON ( spin_button ) ) );
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_array_refresh_estimate_tab ( );

    return ( FALSE );
}


/**
 *
 *
 *
 *
 * */
void bet_array_create_page ( GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkWidget *vbox;
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    devel_debug (NULL);
    widget = gtk_label_new (_("Array") );
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), vbox,  widget );

    /* create the title */
    widget = gtk_label_new ("Estimate array");
    gtk_box_pack_start( GTK_BOX ( vbox ), widget, FALSE, FALSE, 5);
    g_object_set_data ( G_OBJECT ( notebook ), "bet_array_title", widget);

    /* create the estimate treeview */
    tree_view = gtk_tree_view_new ( );
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( tree_view ), TRUE );
    g_object_set_data ( G_OBJECT ( bet_container ), "bet_estimate_treeview", tree_view );

    /* create the model */
    tree_model = gtk_tree_store_new ( SPP_ESTIMATE_TREE_NUM_COLUMNS,
                    G_TYPE_BOOLEAN, /*SPP_ESTIMATE_TREE_SELECT_COLUMN */
				    G_TYPE_STRING,  /* SPP_ESTIMATE_TREE_DATE_COLUMN */
				    G_TYPE_STRING,  /* SPP_ESTIMATE_TREE_DESC_COLUMN */
				    G_TYPE_STRING,  /* SPP_ESTIMATE_TREE_DEBIT_COLUMN */
				    G_TYPE_STRING,  /* SPP_ESTIMATE_TREE_CREDIT_COLUMN */
				    G_TYPE_STRING,  /* SPP_ESTIMATE_TREE_BALANCE_COLUMN */
				    G_TYPE_DATE,    /* SPP_ESTIMATE_TREE_SORT_DATE_COLUMN */
				    G_TYPE_STRING,  /* SPP_ESTIMATE_TREE_AMOUNT_COLUMN */
                    G_TYPE_STRING );/*SPP_ESTIMATE_TREE_BALANCE_COLOR */
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    /* sort by date */
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( tree_model ),
				      SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
				      (GtkTreeIterCompareFunc) bet_array_date_sort_function,
				      NULL, NULL );
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tree_model),
					 SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, GTK_SORT_DESCENDING);

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_widget_show ( scrolled_window );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW ( scrolled_window ),
				        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );
    gtk_widget_show ( scrolled_window );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
		                GTK_WIDGET ( scrolled_window ), TRUE, TRUE, 5 );

    /* Date column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
					    _("Date"), cell,
					    "text", SPP_ESTIMATE_TREE_DATE_COLUMN,
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_min_width(GTK_TREE_VIEW_COLUMN(column), 150);

    /* Description column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
						       _("Description"), cell,
						       "text", SPP_ESTIMATE_TREE_DESC_COLUMN,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_min_width(column, 300);
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), TRUE);

    /* Debit column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
						       _("Debit"), cell,
						       "text", SPP_ESTIMATE_TREE_DEBIT_COLUMN,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_min_width(column, 140);
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment(column, 1);
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);

    /* Credit column */
    cell = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    column = gtk_tree_view_column_new_with_attributes (
						       _("Credit"), cell,
						       "text", SPP_ESTIMATE_TREE_CREDIT_COLUMN,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_min_width(column, 140);
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment(column, 1);
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);

    /* Balance column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
					    _("Balance"), cell,
					    "text", SPP_ESTIMATE_TREE_BALANCE_COLUMN,
                        "foreground", SPP_ESTIMATE_TREE_BALANCE_COLOR,
					    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));
    gtk_tree_view_column_set_min_width(column, 170);
    g_object_set(G_OBJECT(GTK_CELL_RENDERER(cell)), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment(column, 1);
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), FALSE);
    g_signal_connect ( G_OBJECT ( tree_view ),
		                "button-press-event",
		                G_CALLBACK ( bet_array_list_button_press ),
		                NULL );

    gtk_widget_show_all ( vbox );
}


/**
 *
 *
 *
 *
 * */
void bet_graph_create_page ( GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkWidget *vbox;

    devel_debug (NULL);
    widget = gtk_label_new(_("Graph"));
    gtk_widget_show(widget);
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(GTK_WIDGET(vbox));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
			     GTK_WIDGET(vbox), GTK_WIDGET(widget));

    /* create the title */
    widget = gtk_label_new("Estimation graph");
    gtk_widget_show(GTK_WIDGET(widget));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(widget), FALSE, FALSE, 5);
    g_object_set_data (G_OBJECT(notebook), "bet_graph_title", widget);

    widget = gtk_curve_new();
    gtk_widget_show(GTK_WIDGET(widget));
    gtk_box_pack_start(GTK_BOX(vbox),
		       GTK_WIDGET(widget), TRUE, TRUE, 5);
    g_object_set_data (G_OBJECT(notebook), "bet_graph_curve", widget);
}


/**
 *
 *
 *
 *
 * */
void bet_parameter_create_page ( GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *tree_view;
    GtkWidget *duration;

    devel_debug (NULL);
    widget = gtk_label_new ( _("Choice the prevision") );
    gtk_widget_show ( GTK_WIDGET ( widget ) );

    page = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_show ( GTK_WIDGET ( page ) );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        GTK_WIDGET ( page ), GTK_WIDGET ( widget ) );

    /* titre de la page */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_widget_show(GTK_WIDGET(hbox));
    gtk_box_pack_start ( GTK_BOX ( page ), hbox, FALSE, FALSE, 15 );
 
    widget = gtk_image_new_from_stock(
				      GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);
    gtk_widget_show ( GTK_WIDGET ( widget ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );

    widget = gtk_label_new ( NULL );
    gtk_label_set_markup ( GTK_LABEL ( widget ),
			 _("Please select an account and a duration\n"\
			   "and select the estimate array tab.") );
    gtk_widget_show ( GTK_WIDGET ( widget ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );

    /* boite des paramètres */
    vbox = gtk_hbox_new ( FALSE, 5 );
    gtk_widget_show ( GTK_WIDGET ( vbox ) );
    gtk_box_pack_start ( GTK_BOX ( page ), vbox, FALSE, FALSE, 5 );

    /* create the account list */
    tree_view = bet_parameter_get_list_accounts ( vbox );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_account_treeview", tree_view );

    /* create duration selection */
    duration = bet_parameter_get_duration_widget ( vbox, FALSE );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_account_duration", duration );
}


/*
 * bet_parameter_account_selection_changed
 * This function is called for each change in the selected line in the account list.
 * It refreshs balance estimation.
 */
void bet_parameter_account_selection_changed ( GtkTreeSelection *tree_selection,
                        gpointer data )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint account_nb;

    //~ devel_debug (NULL);
    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( model, &iter, SPP_ACCOUNT_TREE_NUM_COLUMN, &account_nb, -1 );

    if ( etat.bet_last_account == account_nb )
        return;

    etat.bet_last_account = account_nb;

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_historical_populate_data ( );
    bet_array_refresh_estimate_tab ( );
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_parameter_get_list_accounts ( GtkWidget *container )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkTreeSelection *tree_selection;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    //~ devel_debug (NULL);
    tree_view = gtk_tree_view_new();

    tree_model = gtk_tree_store_new (SPP_ACCOUNT_TREE_NUM_COLUMNS,
                        G_TYPE_INT,
                        G_TYPE_STRING );

    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_selection_set_mode ( tree_selection, GTK_SELECTION_SINGLE );
    g_signal_connect ( G_OBJECT ( tree_selection ),
                        "changed",
                        G_CALLBACK (bet_parameter_account_selection_changed),
                        NULL );

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_window),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_widget_set_size_request ( scrolled_window, -1, 250 );
    gtk_container_add (GTK_CONTAINER(scrolled_window), tree_view);
    gtk_widget_show(scrolled_window);
    gtk_box_pack_start ( GTK_BOX ( container ), scrolled_window, TRUE, TRUE, 15 );

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (
                        _("Account"), cell,
                        "text", SPP_ACCOUNT_TREE_NAME_COLUMN,
                        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
				GTK_TREE_VIEW_COLUMN(column));

    gtk_widget_show_all ( scrolled_window );

    /* fill the account list */
    if ( bet_parameter_update_list_accounts ( tree_view, GTK_TREE_MODEL ( tree_model ) ) )

        return tree_view;
    else
        return NULL;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_parameter_update_list_accounts ( GtkWidget *tree_view,
                        GtkTreeModel *tree_model )
{
    GtkTreeSelection *tree_selection;
    GtkTreeIter iter;
    GSList *tmp_list;
    gint last_account;

    //~ devel_debug (NULL);
    last_account = gsb_gui_navigation_get_last_account ( );
    if ( last_account == -1 )
        last_account = etat.bet_last_account;

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    if ( !tree_selection )
        return FALSE;

    gtk_tree_store_clear ( GTK_TREE_STORE ( tree_model ) );

    tmp_list = gsb_data_account_get_list_accounts ();
    while ( tmp_list )
    {
        gint account_nb;
        gchar *account_name;

        account_nb = gsb_data_account_get_no_account ( tmp_list -> data );
        if ( gsb_data_account_get_closed_account ( account_nb ) )
        {
            tmp_list = tmp_list -> next;
            continue;
        }

        account_name = gsb_data_account_get_name ( account_nb );
        gtk_tree_store_append ( GTK_TREE_STORE ( tree_model ), &iter, NULL );
        gtk_tree_store_set ( GTK_TREE_STORE ( tree_model ),
                        &iter,
                        SPP_ACCOUNT_TREE_NUM_COLUMN, account_nb,
                        SPP_ACCOUNT_TREE_NAME_COLUMN, account_name,
                        -1);
        if ( last_account == account_nb )
            gtk_tree_selection_select_iter ( GTK_TREE_SELECTION ( tree_selection ),
                        &iter );

        tmp_list = tmp_list -> next;
    }

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_parameter_get_duration_widget ( GtkWidget *container, gboolean config )
{
    GtkWidget* main_vbox;
    GtkWidget *label;
    GtkWidget *button_1, *button_2, *button_3, *button_4;
    GtkWidget *spin_button = NULL;
    GtkWidget *widget = NULL;
    GtkWidget *hbox;
    GtkWidget *previous = NULL;
    GtkSizeGroup *size_group;
    gint iduration;

    //~ devel_debug (NULL);
    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    main_vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( container ), main_vbox, FALSE, FALSE, 5) ;

    if ( !config )
    {
        label = gtk_label_new ( _("Calculation of period") );
        gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;
    }

    label = gtk_label_new ( _("Beginning of period") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), hbox, FALSE, FALSE, 5) ;

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("1st day of month") );
    gtk_widget_set_name ( button_1, "button_1" );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), button_1 );
    
    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("date today") );
    gtk_widget_set_name ( button_2, "button_2" );

    if ( etat.bet_deb_period == 1 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );
    else
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_2 ), TRUE );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    label = gtk_label_new ( _("End of period") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), hbox, FALSE, FALSE, 5) ;

    button_3 = gtk_radio_button_new_with_label ( NULL,
                        _("last day of the month") );
    gtk_widget_set_name ( button_3, "button_3" );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), button_3 );
    
    button_4 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_3 ),
                        _("From date to date") );
    gtk_widget_set_name ( button_4, "button_4" );

    if ( etat.bet_end_period == 1 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_3 ), TRUE );
    else
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_4 ), TRUE );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button_3, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_4, FALSE, FALSE, 5) ;

    /*set the signals */
    g_signal_connect (G_OBJECT ( button_1 ),
                        "released",
                        G_CALLBACK ( bet_parameter_duration_period_clicked ),
                        button_3 );
    g_signal_connect (G_OBJECT ( button_2 ),
                        "released",
                        G_CALLBACK ( bet_parameter_duration_period_clicked ),
                        button_4 );
    g_signal_connect (G_OBJECT ( button_3 ),
                        "released",
                        G_CALLBACK ( bet_parameter_duration_period_clicked ),
                        NULL );
    g_signal_connect (G_OBJECT ( button_4 ),
                        "released",
                        G_CALLBACK ( bet_parameter_duration_period_clicked ),
                        NULL );

    /* partie mensuelle */
    label = gtk_label_new ( _("Duration estimation") );

        if ( config )
    {
        gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
        gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    }
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), hbox, FALSE, FALSE, 5) ;

    if ( etat.bet_spin_range == 0 )
    {
        spin_button = gtk_spin_button_new_with_range ( 1.0, 240.0, 1.0);
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ),
                        (gdouble) etat.bet_months );
    }
    else
    {
        spin_button = gtk_spin_button_new_with_range ( 1.0, 20.0, 1.0 );
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ),
                        (gdouble) ( etat.bet_months / 12 ) );
    }
    gtk_widget_set_name ( spin_button, "spin_button" );

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
                        G_CALLBACK ( bet_parameter_duration_button_clicked ),
                        spin_button );
    }

    if ( etat.bet_spin_range == 0 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( previous ), TRUE );
    else
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );

    g_signal_connect ( G_OBJECT ( spin_button ),
                        "value-changed",
                        G_CALLBACK ( bet_parameter_duration_number_changed ),
                        widget );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 0 );
    
    gtk_widget_show_all ( main_vbox );

    return main_vbox;
}


/**
 * find the selected account
 *
 * */
gint bet_parameter_get_account_selected ( void )
{
    GtkWidget *tree_view;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *tree_selection;
    gint account_nb;

    /* récuperation du n° de compte à utiliser */
    tree_view = g_object_get_data ( G_OBJECT ( bet_container ), "bet_account_treeview" );
    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
    {
        /* no selection, select the first account */
        etat.bet_last_account = gsb_data_account_first_number ( );
        bet_parameter_select_account ( etat.bet_last_account );

        return etat.bet_last_account ;
    }

    gtk_tree_model_get ( model, &iter, SPP_ACCOUNT_TREE_NUM_COLUMN, &account_nb, -1 );
    
    return account_nb;
}


/**
 * select the account
 *
 * */
void bet_parameter_select_account ( gint account_nb )
{
    GtkWidget *tree_view;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *tree_selection;
    gboolean valid = FALSE;

    /* récuperation du n° de compte à utiliser */
    tree_view = g_object_get_data ( G_OBJECT ( bet_container ), "bet_account_treeview" );
    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    valid = gtk_tree_model_get_iter_first ( model, &iter );
    while (valid)
    {
        gint number;

        gtk_tree_model_get ( model, &iter, SPP_ACCOUNT_TREE_NUM_COLUMN, &number, -1 );
        if ( number == account_nb )
            break;
        
        valid = gtk_tree_model_iter_next ( model, &iter );
    }
    gtk_tree_selection_select_iter ( GTK_TREE_SELECTION ( tree_selection ), &iter );
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
        if ( bet_data_search_div_hist ( account_number, div_number, sub_div_nb ) )
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
    GDate *date_jour;
    GSList *tmp_list;

    //~ devel_debug (NULL);
    date_jour = gdate_today ( );

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

        /* ignore transaction which are before date_min (today) */
        if ( g_date_compare ( date, date_min ) < 0 )
            continue;
        /* ignore transaction which are after date_max */
        if ( g_date_compare (date, date_max ) > 0 )
            continue;

        /* ignore splitted transactions */
        if ( gsb_data_transaction_get_mother_transaction_number (
         transaction_number ) != 0 )
            continue;

        amount = gsb_data_transaction_get_amount ( transaction_number );

        div_number = bet_data_get_div_number ( transaction_number, TRUE );
        sub_div_nb = bet_data_get_sub_div_nb ( transaction_number, TRUE );
        if ( bet_data_search_div_hist ( account_number, div_number, sub_div_nb ) )
        {
            if ( g_date_compare ( date, date_jour ) > 1 )
                continue;
            else
                bet_array_adjust_hist_data ( div_number, sub_div_nb, amount, tab_model );
        }

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
        str_description = bet_data_get_div_name ( div_number, sub_div_nb, FALSE );
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

        if ( g_date_compare ( date, date_min ) < 0 )
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
    menu_item = gtk_image_menu_item_new_with_label ( _("Delete selection") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE,
						GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_delete_menu ),
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
void bet_array_list_delete_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &iter );
    bet_array_list_update_balance ( model );
}


/**
 *
 *
 *
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

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, &str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, &str_desc,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &str_amount,
                        -1 );

            div_name = bet_data_get_div_name ( div_number, sub_div_nb, FALSE );
            if ( g_strstr_len ( str_desc, -1, div_name ) )
            {
                date = gsb_parse_date_string ( str_date );
                date_today = gdate_today ( );
                if ( g_date_get_month ( date ) - g_date_get_month ( date_today ) == 0 )
                {
                    number = gsb_real_import_from_string ( str_amount );
                    number = gsb_real_sub ( number, amount );
                    if ( number.mantissa == 0 )
                        gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &iter );
                    else
                    {
                        if ( str_amount )
                            g_free ( str_amount );
                        str_amount = gsb_real_save_real_to_string ( number, 2 );
                        if ( number.mantissa < 0 )
                            str_debit = gsb_real_get_string_with_currency (
                                        gsb_real_abs ( number ),
                                        bet_data_get_selected_currency ( ),
                                        TRUE );
                        else
                            str_credit = gsb_real_get_string_with_currency (
                                        gsb_real_abs ( number ),
                                        bet_data_get_selected_currency ( ),
                                        TRUE );
                        if ( str_desc )
                            g_free ( str_desc );
                        str_desc = g_strconcat ( div_name, _(" (still available)"), NULL);
                        
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
            }
            g_free ( str_date );
            g_free ( str_desc );
            g_free ( str_amount );
            g_free ( div_name );
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
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
