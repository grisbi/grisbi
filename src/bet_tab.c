/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2011 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "bet_tab.h"
#include "bet_config.h"
#include "bet_data.h"
#include "bet_future.h"
#include "bet_hist.h"
#include "dialog.h"
#include "export_csv.h"
#include "fenetre_principale.h"
#include "gsb_automem.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_real.h"
#include "gsb_scheduler.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "mouse.h"
#include "navigation.h"
#include "print_tree_view_list.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list_select.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_file_selection.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void bet_array_adjust_hist_amount ( gint div_number,
                        gint sub_div_nb,
                        gsb_real amount,
                        GtkTreeModel *model );
static void bet_array_auto_inc_month_toggle ( GtkToggleButton *togglebutton, gpointer  data );
static GtkWidget *bet_array_create_tree_view ( GtkWidget *container );
static gint bet_array_date_sort_function ( GtkTreeModel *model,
                        GtkTreeIter *itera,
                        GtkTreeIter *iterb,
                        gpointer data );
static gboolean bet_array_entry_key_press ( GtkWidget *entry,
                        GdkEventKey *ev,
                        gpointer data );
static void bet_array_export_tab ( GtkWidget *menu_item, GtkTreeView *tree_view );
static gboolean bet_array_initializes_account_settings ( gint account_number );
static void bet_array_list_add_substract_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gboolean bet_array_list_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev );
static void bet_array_list_change_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static GtkWidget *bet_array_list_create_toolbar ( GtkWidget *parent, GtkWidget *tree_view );
static void bet_array_list_context_menu ( GtkWidget *tree_view );
static void bet_array_list_delete_all_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static void bet_array_list_delete_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gchar *bet_array_list_get_description ( gint account_number,
                        gint origine,
                        gpointer value );
static void bet_array_list_insert_account_balance_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static void bet_array_list_insert_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static void bet_array_list_redo_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gboolean bet_array_list_replace_planned_line_by_transfert ( GtkTreeModel *tab_model,
                        struct_transfert_data *transfert );
static gint bet_array_list_schedule_line ( gint origine, gint account_number, gint number );
static void bet_array_list_schedule_selected_line ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gboolean bet_array_list_set_background_color ( GtkWidget *tree_view );
static gboolean bet_array_list_size_allocate ( GtkWidget *tree_view,
                        GtkAllocation *allocation,
                        gpointer null );
static void bet_array_list_traite_double_click ( GtkTreeView *tree_view );
static void bet_array_list_update_balance ( GtkTreeModel *model );
static void bet_array_refresh_estimate_tab ( gint account_number );
static gboolean bet_array_refresh_futur_data ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max );
static void bet_array_refresh_scheduled_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max );
static void bet_array_refresh_transactions_data ( GtkTreeModel *tab_model,
                        gint selected_account,
                        GDate *date_min,
                        GDate *date_max );
static gboolean bet_array_refresh_transfert_data ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max );
static gboolean bet_array_shows_balance_at_beginning_of_month ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max );
static gboolean bet_array_sort_scheduled_transactions ( gint div_number,
                        gint sub_div_nb,
                        GtkTreeModel *model );
static gboolean bet_array_start_date_focus_out ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data );
static gboolean bet_array_update_average_column ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *account_page;
extern gchar* bet_duration_array[];
extern GdkColor couleur_bet_division;
extern GdkColor couleur_bet_future;
extern GdkColor couleur_selection;
extern GdkColor couleur_bet_solde;
extern GdkColor couleur_bet_transfert;
extern GdkColor couleur_fond[2];
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern GtkWidget *notebook_general;
extern gsb_real null_real;
extern const gdouble prev_month_max;
extern gint valeur_echelle_recherche_date_import;
extern GtkWidget *window;
/*END_EXTERN*/

/* gestion de la largeur des colonnes du tableau */

/* tableau des colonnes */
GtkTreeViewColumn *bet_array_tree_view_columns[BET_ARRAY_COLUMNS];
/* the initial width of each column */
gint bet_array_col_width[BET_ARRAY_COLUMNS];
/* the initial width of the tree_view */
gint bet_array_current_tree_view_width = 0;


enum bet_estimation_tree_columns
{
    SPP_ESTIMATE_TREE_SELECT_COLUMN,    /* select column for the balance */
    SPP_ESTIMATE_TREE_ORIGIN_DATA,      /* origin of data : transaction, scheduled, hist, future */
    SPP_ESTIMATE_TREE_DIVISION_COLUMN,  /* div_number, transaction_number, futur_number, scheduled_number*/
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
    SPP_ESTIMATE_TREE_COLOR_STRING,
    SPP_ESTIMATE_TREE_NUM_COLUMNS
};


/*
 * Met à jour les données à afficher dans les différentes vues du module
 *
 * 
 */
void bet_array_update_estimate_tab ( gint account_number, gint type_maj )
{
    devel_debug_int ( account_number );
    /* test account number */

    bet_array_initializes_account_settings ( account_number );
    switch ( type_maj )
    {
        case BET_MAJ_ESTIMATE:
            bet_array_refresh_estimate_tab ( account_number );
        break;
        case BET_MAJ_HISTORICAL:
            bet_historical_populate_data ( account_number );
        break;
        case BET_MAJ_ALL:
            bet_historical_populate_data ( account_number );
            bet_array_refresh_estimate_tab ( account_number );
        break;
    }
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
    gint result;

    if ( itera == NULL )
        return -1;
    if ( iterb == NULL)
        return -1;

    /* get first date to compare */
    gtk_tree_model_get_value ( model, itera, 
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value_a );
    date_a = g_value_get_boxed ( &date_value_a );
    if ( date_a == NULL )
        return -1;

    /* get second date to compare */
    gtk_tree_model_get_value ( model, iterb, SPP_ESTIMATE_TREE_SORT_DATE_COLUMN, &date_value_b );
    date_b = g_value_get_boxed ( &date_value_b );
    if ( date_b == NULL )
        return -1;

    result = g_date_compare (date_b, date_a);

    if ( result == 0 )
    {
        gint origine;
        gchar *str_amount_a;
        gchar *str_amount_b;
        gsb_real amount_a = null_real;
        gsb_real amount_b = null_real;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), itera,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &str_amount_a,
                        -1 );

        if ( origine == SPP_ORIGIN_HISTORICAL )
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), iterb,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, &str_amount_b,
                        -1 );
            if ( origine != SPP_ORIGIN_HISTORICAL )
                result = -1;
            else
            {
                amount_a = gsb_real_safe_real_from_string ( str_amount_a );
                amount_b = gsb_real_safe_real_from_string ( str_amount_b );
                result = - ( gsb_real_cmp ( amount_a, amount_b ) );
            }

            g_free ( str_amount_b );
        }

        if ( origine == SPP_ORIGIN_SOLDE )
        {
            result = 1;
        }

        g_free ( str_amount_a );
    }

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

    amount = gsb_real_safe_real_from_string ( tmp_str );

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
void bet_array_refresh_estimate_tab ( gint account_number )
{
    GtkWidget *widget;
    GtkWidget *tree_view;
    GtkTreeIter iter;
    GtkTreeModel *tree_model;
    GtkTreePath *path = NULL;
    gchar *color_str = NULL;
    gchar *str_date_min;
    gchar *str_date_max;
    gchar *str_current_balance;
    gchar *str_amount;
    gchar *title;
    gchar *tmp_str;
    GDate *date_init;
    GDate *date_min;
    GDate *date_max;
    gsb_real current_balance;
    SBR *tmp_range;
    GValue date_value = {0, };
    gint currency_number;

    devel_debug (NULL);

    tmp_range = struct_initialise_bet_range ( );

    /* calculate date_min and date_max with user choice */
    date_min = gsb_data_account_get_bet_start_date ( account_number );

    date_max = bet_data_array_get_date_max ( account_number );

    widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_initial_date");
    gsb_calendar_entry_set_date ( widget, date_min );

    str_date_min = gsb_format_gdate ( date_min );

    /* sert à mettre en première ligne le solde de début de période */
    date_init = gsb_date_copy ( date_min );
    g_date_subtract_days ( date_init, 1 );

    g_value_init ( &date_value, G_TYPE_DATE );
    g_value_set_boxed ( &date_value, date_init );

    str_date_max = gsb_format_gdate ( date_max );
    
    /* current balance may be in the future if there are transactions
     * in the future in the account. So we need to calculate the balance
     * of today */
    current_balance = gsb_data_account_calculate_current_day_balance (
                        account_number, date_min );

    currency_number = gsb_data_account_get_currency ( account_number );

    str_amount = gsb_real_safe_real_to_string ( current_balance, 
                    gsb_data_currency_get_floating_point ( currency_number ) );
    str_current_balance = gsb_real_get_string_with_currency ( current_balance, currency_number, TRUE );

    if ( current_balance.mantissa < 0 )
        color_str = "red";
    else
    {
        g_free ( color_str );
        color_str = NULL;
    }

    /* set the titles of tabs module budget */
    title = g_strdup_printf (
                        _("Balance estimate of the account \"%s\" from %s to %s"),
                        gsb_data_account_get_name ( account_number ),
                        str_date_min, str_date_max );

    widget = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( account_page ), "bet_array_title") );
    gtk_label_set_label ( GTK_LABEL ( widget ), title );
    g_free ( title );

    tree_view = g_object_get_data ( G_OBJECT ( account_page ), "bet_estimate_treeview" );
    if ( gtk_tree_selection_get_selected (
                        gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ),
                        &tree_model, &iter ) )
        path = gtk_tree_model_get_path ( tree_model, &iter );

    /* clear the model */
    gtk_tree_store_clear ( GTK_TREE_STORE ( tree_model ) );

    tmp_str = g_strdup ( _("balance beginning of period") );
    gtk_tree_store_append ( GTK_TREE_STORE ( tree_model ), &iter, NULL );
    gtk_tree_store_set_value (GTK_TREE_STORE ( tree_model ), &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value);
    gtk_tree_store_set(GTK_TREE_STORE(tree_model), &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, SPP_ORIGIN_SOLDE,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date_min,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, tmp_str,
                        SPP_ESTIMATE_TREE_BALANCE_COLUMN, str_current_balance,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        SPP_ESTIMATE_TREE_BALANCE_COLOR, color_str,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_bet_solde,
                        -1);

    g_value_unset ( &date_value );
    g_free ( str_date_min );
    g_free ( str_date_max );
    g_free ( tmp_str );
    g_free ( str_current_balance );
    g_free ( str_amount );

    /* search data from the past */
    bet_historical_refresh_data ( tree_model, date_min, date_max );

    /* search data from the futur */
    bet_array_refresh_futur_data ( tree_model, date_min, date_max );

    /* search transactions of the account which are in the period */
    bet_array_refresh_transactions_data ( tree_model,
                        account_number,
                        date_min,
                        date_max );

    /* for each schedulded operation */
    bet_array_refresh_scheduled_data ( tree_model,
                        account_number,
                        date_min,
                        date_max );

    /* search data from a transfer */
    bet_array_refresh_transfert_data ( tree_model, date_min, date_max );

    /* shows the balance at beginning of month */
    bet_array_shows_balance_at_beginning_of_month ( tree_model, date_min, date_max );

    g_date_free ( date_min );
    g_date_free ( date_init );
    g_date_free ( date_max );

    /* Calculate the balance column */
    tmp_range -> first_pass = TRUE;
    tmp_range -> current_balance = current_balance;

    gtk_tree_model_foreach ( GTK_TREE_MODEL ( tree_model ),
                        bet_array_update_average_column, tmp_range );

    bet_array_list_set_background_color ( tree_view );
    bet_array_list_select_path ( tree_view, path );
    gtk_tree_path_free ( path );
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_array_create_page ( void )
{
    GtkWidget *page;
    GtkWidget *widget = NULL;
    GtkWidget *initial_date = NULL;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label_title;
    GtkWidget *label;
    GtkWidget *tree_view;
    GtkWidget *toolbar;

    devel_debug (NULL);
    page = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_set_name ( page, "forecast_page" );

    /* create the title */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5) ;

    label_title = gtk_label_new ("Estimate array");
    gtk_container_add ( GTK_CONTAINER ( align ), label_title );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_array_title", label_title );

    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5) ;

    /* set the duration widget */
    hbox = bet_config_get_duration_widget ( SPP_ORIGIN_ARRAY );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );

    /* set the start date and the automatic change of month */
    label = gtk_label_new ( _("Start date: " ) );
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
    g_object_set_data ( G_OBJECT ( account_page ), "bet_initial_date", initial_date );
    gtk_box_pack_start ( GTK_BOX (hbox), initial_date, FALSE, FALSE, 0 );

    widget = gtk_check_button_new ( );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), FALSE );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( widget ),
                        _("Check the box to automatically change start date") );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_auto_inc_month", widget );
    gtk_box_pack_start ( GTK_BOX (hbox), widget, FALSE, FALSE, 0 );
    g_signal_connect ( G_OBJECT ( widget ),
			            "toggled",
			            G_CALLBACK ( bet_array_auto_inc_month_toggle ),
			            NULL );

    tree_view = bet_array_create_tree_view ( page );
    g_object_set_data ( G_OBJECT ( tree_view ), "label_title", label_title );

    /* on y ajoute la barre d'outils */
    toolbar = bet_array_list_create_toolbar ( page, tree_view );
    gtk_box_pack_start ( GTK_BOX ( page ), toolbar, FALSE, FALSE, 0 );
    gtk_box_reorder_child ( GTK_BOX ( page ), toolbar, 0 );

    gtk_widget_show_all ( page );

    return page;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_array_create_tree_view ( GtkWidget *container )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkTreeModel *sortable;
    GtkCellRenderer *cell;
    gint i;

    /* create the estimate treeview */
    tree_view = gtk_tree_view_new ( );
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( tree_view ), FALSE );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_estimate_treeview", tree_view );
    g_object_set_data ( G_OBJECT ( tree_view ), "origin_data_model",
                        GINT_TO_POINTER ( SPP_ESTIMATE_TREE_ORIGIN_DATA ) );
    g_object_set_data ( G_OBJECT ( tree_view ), "color_data_model",
                        GINT_TO_POINTER ( SPP_ESTIMATE_TREE_COLOR_STRING ) );

    /* set the color of selected row */
    gtk_widget_modify_base ( tree_view, GTK_STATE_SELECTED, &couleur_selection );
    gtk_widget_modify_base ( tree_view, GTK_STATE_ACTIVE, &couleur_selection );

    /* create the model */
    tree_model = gtk_tree_store_new ( SPP_ESTIMATE_TREE_NUM_COLUMNS,
                    G_TYPE_BOOLEAN,     /* SPP_ESTIMATE_TREE_SELECT_COLUMN */
                    G_TYPE_INT,         /* SPP_ESTIMATE_TREE_ORIGIN_DATA */
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
                    GDK_TYPE_COLOR,     /* SPP_ESTIMATE_TREE_BACKGROUND_COLOR */
                    G_TYPE_STRING );    /* SPP_ESTIMATE_TREE_COLOR_STRING */

    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    /* sort by date */
    sortable = gtk_tree_model_sort_new_with_model ( GTK_TREE_MODEL ( tree_model ) );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( tree_model ),
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        (GtkTreeIterCompareFunc) bet_array_date_sort_function,
                        NULL,
                        NULL );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( tree_model ),
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        GTK_SORT_DESCENDING );

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_widget_show ( scrolled_window );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );
    gtk_box_pack_start ( GTK_BOX ( container ), scrolled_window, TRUE, TRUE, 5 );

    /* create columns */
    i = 0;
    /* Date column */
    cell = gtk_cell_renderer_text_new ();
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 0.5, NULL );

    bet_array_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes (
                        _("Date"), cell,
                        "text", SPP_ESTIMATE_TREE_DATE_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
                        NULL);

    gtk_tree_view_column_set_alignment ( bet_array_tree_view_columns[i], 0.5 );
    gtk_tree_view_append_column( GTK_TREE_VIEW ( tree_view ), bet_array_tree_view_columns[i] );
    gtk_tree_view_column_set_sizing ( bet_array_tree_view_columns[i], GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_resizable ( bet_array_tree_view_columns[i], TRUE );
    g_object_set_data ( G_OBJECT ( bet_array_tree_view_columns[i] ), "num_col_model",
                        GINT_TO_POINTER ( SPP_ESTIMATE_TREE_DATE_COLUMN ) );

    i++;
    /* Description column */
    cell = gtk_cell_renderer_text_new ();

    bet_array_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes (
					    _("Description"), cell,
					    "text", SPP_ESTIMATE_TREE_DESC_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
					    NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ), bet_array_tree_view_columns[i] );
    gtk_tree_view_column_set_sizing ( bet_array_tree_view_columns[i], GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_resizable ( bet_array_tree_view_columns[i], TRUE );
    g_object_set_data ( G_OBJECT ( bet_array_tree_view_columns[i] ), "num_col_model",
                        GINT_TO_POINTER ( SPP_ESTIMATE_TREE_DESC_COLUMN ) );

    i++;
    /* Debit column */
    cell = gtk_cell_renderer_text_new ();
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );

    bet_array_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes (
						_("Debit"), cell,
					    "text", SPP_ESTIMATE_TREE_DEBIT_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
					    NULL);

    gtk_tree_view_column_set_alignment ( bet_array_tree_view_columns[i], 1 );
    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ), bet_array_tree_view_columns[i] );
    gtk_tree_view_column_set_sizing ( bet_array_tree_view_columns[i], GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_resizable ( bet_array_tree_view_columns[i], TRUE );
    g_object_set_data ( G_OBJECT ( bet_array_tree_view_columns[i] ), "num_col_model",
                        GINT_TO_POINTER ( SPP_ESTIMATE_TREE_DEBIT_COLUMN ) );

    i++;
    /* Credit column */
    cell = gtk_cell_renderer_text_new ();
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );

    bet_array_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes (
					    _("Credit"), cell,
					    "text", SPP_ESTIMATE_TREE_CREDIT_COLUMN,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
						NULL);

    gtk_tree_view_column_set_alignment ( bet_array_tree_view_columns[i], 1 );
    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ), bet_array_tree_view_columns[i] );
    gtk_tree_view_column_set_sizing ( bet_array_tree_view_columns[i], GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_resizable ( bet_array_tree_view_columns[i], TRUE );
    g_object_set_data ( G_OBJECT ( bet_array_tree_view_columns[i] ), "num_col_model",
                        GINT_TO_POINTER ( SPP_ESTIMATE_TREE_CREDIT_COLUMN ) );

    i++;
    /* Balance column */
    cell = gtk_cell_renderer_text_new ();
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );

    bet_array_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes (
					    _("Balance"), cell,
					    "text", SPP_ESTIMATE_TREE_BALANCE_COLUMN,
                        "foreground", SPP_ESTIMATE_TREE_BALANCE_COLOR,
                        "cell-background-gdk", SPP_ESTIMATE_TREE_BACKGROUND_COLOR,
					    NULL);

    gtk_tree_view_column_set_alignment ( bet_array_tree_view_columns[i], 1 );
    gtk_tree_view_append_column (GTK_TREE_VIEW ( tree_view ), bet_array_tree_view_columns[i] );
    gtk_tree_view_column_set_sizing ( bet_array_tree_view_columns[i], GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_resizable ( bet_array_tree_view_columns[i], TRUE );
    g_object_set_data ( G_OBJECT ( bet_array_tree_view_columns[i] ), "num_col_model",
                        GINT_TO_POINTER ( SPP_ESTIMATE_TREE_BALANCE_COLUMN ) );

    /* signals of tree_view */
    g_signal_connect ( G_OBJECT ( tree_view ),
		                "button-press-event",
		                G_CALLBACK ( bet_array_list_button_press ),
		                NULL );

    g_signal_connect ( G_OBJECT ( tree_view ),
		                "size_allocate",
		                G_CALLBACK ( bet_array_list_size_allocate ),
		                NULL );

    gtk_widget_show_all ( scrolled_window );

    return tree_view;
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

    /* devel_debug (NULL); */
    tmp_list = gsb_data_scheduled_get_scheduled_list ( );

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
        gint currency_number;
        GDate *date;
        GValue date_value = {0, };
        gsb_real amount;

        scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list->data );

        tmp_list = tmp_list->next;

        /* ignore children scheduled operations */
        if (gsb_data_scheduled_get_mother_scheduled_number ( scheduled_number ) )
            continue;

        /* ignores transactions replaced with historical data */
        account_number = gsb_data_scheduled_get_account_number ( scheduled_number );

        div_number = bet_data_get_div_number ( scheduled_number, FALSE );
        sub_div_nb = bet_data_get_sub_div_nb ( scheduled_number, FALSE );
        
        if ( div_number > 0
         &&
         bet_data_search_div_hist ( account_number, div_number, 0 )
         && 
         bet_array_sort_scheduled_transactions ( div_number, sub_div_nb, tab_model ) )
            continue;

        /* ignore scheduled operations of other account */
        if ( gsb_data_scheduled_is_transfer ( scheduled_number ) )
        {
            transfer_account_number = gsb_data_scheduled_get_account_number_transfer (
                        scheduled_number );
            if ( transfer_account_number == selected_account )
            {
                gint floating_point;

                currency_number = gsb_data_account_get_currency ( selected_account );
                floating_point = gsb_data_account_get_currency_floating_point ( selected_account );
                str_description = g_strdup_printf ( _("Transfer between account: %s\n"
                        "and account: %s"),
                        gsb_data_account_get_name ( transfer_account_number ),
                        gsb_data_account_get_name ( account_number ) );

                amount = gsb_real_opposite ( gsb_data_scheduled_get_adjusted_amount_for_currency ( scheduled_number,
                                    currency_number,
                                    floating_point ) );
                str_amount = gsb_real_safe_real_to_string ( amount, floating_point );
            }
            else if ( account_number == selected_account )
            {
                currency_number = gsb_data_scheduled_get_currency_number ( scheduled_number );
                str_description = g_strdup_printf ( _("Transfer between account: %s\n"
                        "and account: %s"),
                        gsb_data_account_get_name ( account_number ),
                        gsb_data_account_get_name ( transfer_account_number ) );

                amount = gsb_data_scheduled_get_amount ( scheduled_number );
                str_amount = bet_data_get_str_amount_in_account_currency ( amount,
                        account_number,
                        scheduled_number,
                        SPP_ORIGIN_SCHEDULED );
            }
            else
                continue;
        }
        else if ( account_number == selected_account )
        {
            currency_number = gsb_data_scheduled_get_currency_number ( scheduled_number );
            str_description = bet_array_list_get_description ( account_number,
                        SPP_ORIGIN_SCHEDULED,
                        GINT_TO_POINTER ( scheduled_number ) );

            amount = gsb_data_scheduled_get_amount ( scheduled_number );
            str_amount = bet_data_get_str_amount_in_account_currency ( amount,
                        account_number,
                        scheduled_number,
                        SPP_ORIGIN_SCHEDULED );
        }
        else
            continue;

        if (amount.mantissa < 0)
            str_debit = gsb_real_get_string_with_currency ( gsb_real_abs ( amount ), currency_number, TRUE );
        else
            str_credit = gsb_real_get_string_with_currency ( amount, currency_number, TRUE );

        /* calculate each instance of the scheduled operation
         * in the range from date_min (today) to date_max */
        date = gsb_data_scheduled_get_date ( scheduled_number );

        while ( date != NULL && g_date_valid ( date ) )
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

            if ( date == NULL )
                return;
            g_value_init ( &date_value, G_TYPE_DATE );
            g_value_set_boxed ( &date_value, date ); 

            /* add a line in the estimate array */
            gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &iter, NULL );
            gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );
            gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, SPP_ORIGIN_SCHEDULED,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, scheduled_number,
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
    GDate *date_comp;
    GSList *tmp_list;

    /* devel_debug (NULL); */
    /* init dates */
    date_jour_1 = gdate_today ( );
    if ( g_date_get_day ( date_min ) > 1 )
        g_date_set_day ( date_jour_1, 1 );
    if ( g_date_compare ( date_jour_1, date_min ) < 0 )
        date_comp = date_jour_1;
    else
        date_comp = date_min;

    /* search transactions of the account which are in the period */
    tmp_list = gsb_data_transaction_get_transactions_list ( );

    while ( tmp_list )
    {
        GtkTreeIter iter;
        gchar* str_amount;
        gchar* str_debit = NULL;
        gchar* str_credit = NULL;
        gchar* str_description;
        gchar* str_date;
        gint transaction_number;
        gint transfer_number;
        gint account_number;
        gint transfer_account_number;
        gint div_number;
        gint sub_div_nb;
        gint currency_number;
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

        /* Ignore transactions that are before date_com */
        if ( g_date_compare ( date, date_comp ) < 0 )
            continue;

        /* ignores transactions replaced with historical data */
        amount = gsb_data_transaction_get_amount ( transaction_number );

        div_number = bet_data_get_div_number ( transaction_number, TRUE );
        sub_div_nb = bet_data_get_sub_div_nb ( transaction_number, TRUE );
        if ( div_number > 0
         &&
         bet_data_search_div_hist ( account_number, div_number, 0 ) )
        {
            if ( g_date_get_month ( date ) == g_date_get_month ( date_jour_1 ) )
                bet_array_adjust_hist_amount ( div_number, sub_div_nb, amount, tab_model );
        }

        /* ignore transaction which are before date_min */
        if ( g_date_compare ( date, date_min ) < 0 )
            continue;

        str_date = gsb_format_gdate ( date );
        g_value_init ( &date_value, G_TYPE_DATE );
        g_value_set_boxed ( &date_value, date );

        str_amount = bet_data_get_str_amount_in_account_currency ( amount,
                        account_number,
                        transaction_number,
                        SPP_ORIGIN_TRANSACTION );

        currency_number = gsb_data_transaction_get_currency_number ( transaction_number);
        if (amount.mantissa < 0)
            str_debit = gsb_real_get_string_with_currency ( gsb_real_abs ( amount ), currency_number, TRUE );
        else
            str_credit = gsb_real_get_string_with_currency ( amount, currency_number, TRUE);

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
            str_description = bet_array_list_get_description ( account_number,
                        SPP_ORIGIN_TRANSACTION,
                        GINT_TO_POINTER ( transaction_number ) );
        }

        /* add a line in the estimate array */
        gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &iter, NULL );
        gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );
        gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, SPP_ORIGIN_TRANSACTION,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, transaction_number,
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
        g_free ( str_description );
        if ( str_debit )
            g_free ( str_debit );
        if ( str_credit )
            g_free ( str_credit );
    }
}


/**
 * Add a new line with historical data
 *
 *
 *
 * */
void bet_array_list_add_new_hist_line ( GtkTreeModel *tab_model,
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

    /* devel_debug (NULL); */
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

    amount = gsb_real_safe_real_from_string ( str_amount );

    if ( amount.mantissa < 0 )
        str_debit = gsb_real_get_string_with_currency ( gsb_real_opposite ( amount ),
                        bet_data_get_selected_currency ( ), TRUE );
    else
        str_credit = str_value;

    while ( date != NULL && g_date_valid ( date ) )
    {
        if ( g_date_compare ( date, date_max ) > 0 )
            break;

        if ( g_date_compare ( date, date_min ) < 0 
         ||
         ( g_date_get_year ( date ) == g_date_get_year ( date_jour )
         &&
         g_date_get_month ( date ) < g_date_get_month ( date_jour ) ) )
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
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, SPP_ORIGIN_HISTORICAL,
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
 * Ajoute la ligne future au tableau des résultats
 *
 *
 *
 * */
gboolean bet_array_refresh_futur_data ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max )
{
    GHashTable *future_list;
    GHashTableIter iter;
    gpointer key, value;
    gint account_number;

    /* devel_debug (NULL); */

    account_number = gsb_gui_navigation_get_current_account ( );
    future_list = bet_data_future_get_list ( );

    g_hash_table_iter_init ( &iter, future_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        struct_futur_data *scheduled = ( struct_futur_data *) value;
        GtkTreeIter tab_iter;
        GValue date_value = {0, };
        gchar *str_debit = NULL;
        gchar *str_credit = NULL;
        gchar *str_date;
        gchar *str_description;
        gchar *str_amount;
        GDate *date_tomorrow;
        gsb_real amount;
        gboolean inverse_amount = FALSE;
        gint currency_number;

        if ( account_number != scheduled -> account_number )
        {
            if ( scheduled -> is_transfert == 0
             || ( scheduled -> is_transfert && account_number != scheduled -> account_transfert ) )
                continue;
            else
                inverse_amount = TRUE;
        }

        date_tomorrow = gsb_date_tomorrow ( );
        if ( g_date_compare ( scheduled -> date, date_tomorrow ) < 0 )
        {
            bet_data_future_remove_line ( account_number, scheduled -> number, FALSE );
            g_hash_table_iter_init ( &iter, future_list );
            g_date_free ( date_tomorrow );
            if ( g_hash_table_size ( future_list ) == 0 )
                return FALSE;
            continue;
        }
        else
            g_date_free ( date_tomorrow );

        if ( g_date_compare ( scheduled -> date, date_max ) > 0 )
            continue;
        if ( g_date_compare ( scheduled -> date, date_min ) < 0 )
            continue;

        str_description = bet_array_list_get_description ( account_number,
                        SPP_ORIGIN_FUTURE,
                        value );

        if ( inverse_amount )
            amount = gsb_real_opposite ( scheduled -> amount );
        else
            amount = scheduled -> amount;

        currency_number = gsb_data_account_get_currency ( account_number );
        str_amount = gsb_real_safe_real_to_string ( amount, 
                    gsb_data_currency_get_floating_point ( currency_number ) );

        if ( amount.mantissa < 0 )
            str_debit = gsb_real_get_string_with_currency ( gsb_real_opposite ( amount ),
                            currency_number, TRUE );
        else
            str_credit = gsb_real_get_string_with_currency ( amount, currency_number, TRUE );

        str_date = gsb_format_gdate ( scheduled -> date );

        g_value_init ( &date_value, G_TYPE_DATE );
        g_value_set_boxed ( &date_value, scheduled -> date ); 

        /* add a line in the estimate array */
        gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &tab_iter, NULL );
        gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );
        gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, SPP_ORIGIN_FUTURE,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, scheduled -> number,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, scheduled -> mother_row,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
                        SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
                        SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        -1);

        g_value_unset ( &date_value );
        g_free ( str_date );
        g_free ( str_description );
        g_free ( str_amount );
        if ( str_debit )
            g_free ( str_debit );
        if ( str_credit )
            g_free ( str_credit );
    }

    return TRUE;
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
    if ( ev -> type == GDK_2BUTTON_PRESS )
        bet_array_list_traite_double_click ( GTK_TREE_VIEW ( tree_view ) );

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
    GtkWidget *image;
    GtkWidget *menu, *menu_item;
    GtkTreeModel *model;
    GtkTreeSelection *tree_selection;
    GtkTreeIter iter;
    GDate *date;
    GDate *date_jour;
    gchar *str_date;
    gchar *tmp_str;
    gboolean select = FALSE;
    gint origine;

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( model, &iter, 
                        SPP_ESTIMATE_TREE_SELECT_COLUMN, &select,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, &str_date,
                        -1 );

    date = gsb_parse_date_string ( str_date );
    date_jour = gdate_today ( ); 

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

    /* Insert Row */
    menu_item = gtk_image_menu_item_new_with_label ( _("Insert Row") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                    gtk_image_new_from_stock ( GTK_STOCK_ADD,
                    GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                    "activate",
                    G_CALLBACK ( bet_array_list_insert_menu ),
                    tree_selection );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Delete convert item */
    switch ( origine )
    {
        case SPP_ORIGIN_TRANSACTION:
        case SPP_ORIGIN_SCHEDULED:
            if ( g_date_compare ( date, date_jour ) > 0 )
            {
                menu_item = gtk_image_menu_item_new_with_label ( _("Delete selection") );
                gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE,
						GTK_ICON_SIZE_MENU ) );
                g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_delete_menu ),
                        tree_selection );
                gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            }
            break;
        case SPP_ORIGIN_HISTORICAL:
            menu_item = gtk_image_menu_item_new_with_label ( _("Delete selection") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE,
						GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_delete_menu ),
                        tree_selection );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            break;
        case SPP_ORIGIN_FUTURE:
            menu_item = gtk_image_menu_item_new_with_label ( _("Change selection") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_EDIT,
						GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_change_menu ),
                        tree_selection );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( _("Delete selection") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE,
						GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_delete_menu ),
                        tree_selection );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( 
                        _("Delete all occurences of the selection") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE,
						GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_delete_all_menu ),
                        tree_selection );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            
            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            /* Convert to scheduled transaction */
            menu_item = gtk_image_menu_item_new_with_label ( 
                                _("Convert selection to scheduled transaction") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                                gtk_image_new_from_stock ( GTK_STOCK_CONVERT,
                                GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                                "activate",
                                G_CALLBACK ( bet_array_list_schedule_selected_line ),
                                tree_selection );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            break;
    }

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new ( ) );
    gtk_widget_show ( menu_item );

    /* Insert an account balance */
    if ( gsb_data_account_get_kind ( gsb_gui_navigation_get_current_account ( ) ) != GSB_TYPE_CASH )
    {
        tmp_str = g_build_filename ( GRISBI_PIXMAPS_DIR, "ac_bank_16.png", NULL);
        image = gtk_image_new_from_file ( tmp_str );
        g_free ( tmp_str );
        menu_item = gtk_image_menu_item_new_with_label (
                        _("Insert the balance of a cash account") );
        gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), image );
        g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_insert_account_balance_menu ),
                        tree_selection );
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        if ( origine == SPP_ORIGIN_ACCOUNT )
        {
            menu_item = gtk_image_menu_item_new_with_label ( _("Delete selection") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE,
						GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_delete_menu ),
                        tree_selection );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
        }

        /* Separator */
        gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );
        gtk_widget_show ( menu_item );
    }
    /* redo item */
    menu_item = gtk_image_menu_item_new_with_label ( _("Reset data") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_REFRESH,
						GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_list_redo_menu ),
                        tree_selection );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

    /* Print list */
    menu_item = gtk_image_menu_item_new_with_label ( _("Print the array") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_PRINT, GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( print_tree_view_list ),
                        tree_view );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Export list */
    menu_item = gtk_image_menu_item_new_with_label ( _("Export the array") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU ) );
    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_array_export_tab ),
                        tree_view );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Finish all. */
    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 3,
                        gtk_get_current_event_time ( ) );

    g_date_free ( date_jour );
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


void bet_array_list_change_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint origine;
    gint number;
    gint mother_row;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, &number,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, &mother_row,
                        -1 );

    if ( origine == SPP_ORIGIN_TRANSACTION )
    {
        gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), 0 );
        transaction_list_select ( number );
        gsb_transactions_list_edit_transaction ( number );
    }
    else if ( origine == SPP_ORIGIN_SCHEDULED )
    {
        gsb_gui_navigation_set_selection ( GSB_SCHEDULER_PAGE, 0, NULL );
        gsb_scheduler_list_select ( number );
        gsb_scheduler_list_edit_transaction ( number );
    }
    else if ( origine == SPP_ORIGIN_FUTURE )
        bet_future_modify_line ( gsb_gui_navigation_get_current_account ( ), number, mother_row );
    else if ( origine == SPP_ORIGIN_ACCOUNT )
        bet_transfert_modify_line ( gsb_gui_navigation_get_current_account ( ), number );
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
    GtkTreeView *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint origine;
    gint number;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, &number,
                        -1 );

    if ( origine == SPP_ORIGIN_HISTORICAL )
    {
        gint account_number;
        gint sub_div_nb;

        account_number = gsb_gui_navigation_get_current_account ( );
        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, &sub_div_nb,
                        -1 );
        bet_data_remove_div_hist ( account_number, number, sub_div_nb );
        gtk_tree_store_remove ( GTK_TREE_STORE ( model ), &iter );

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_HISTORICAL );
    }
    else if ( origine == SPP_ORIGIN_FUTURE )
    {
        bet_data_future_remove_line ( gsb_gui_navigation_get_current_account ( ), number, TRUE );
    }
    else if ( origine == SPP_ORIGIN_ACCOUNT )
    {
        bet_data_transfert_remove_line ( gsb_gui_navigation_get_current_account ( ), number );
    }

    tree_view = gtk_tree_selection_get_tree_view ( tree_selection );
    bet_array_list_set_background_color ( GTK_WIDGET ( tree_view ) );
    bet_array_list_update_balance ( model );
}


/**
 * delete all occurences of future data row
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_array_list_delete_all_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint origine;
    gint number;
    gint mother_row;
    gint account_number;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ), &model, &iter ) )
        return;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, &number,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, &mother_row,
                        -1 );

    if ( origine == SPP_ORIGIN_FUTURE )
    {
        account_number = gsb_gui_navigation_get_current_account ( );
        bet_data_future_remove_lines ( account_number, number, mother_row );

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
        bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
    }
}


/**
 * insert a row
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_array_list_insert_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *str_date;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, &str_date,
                        -1 );

    bet_future_new_line_dialog ( model, str_date );
}


/**
 * insert an account balance
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_array_list_insert_account_balance_menu ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *str_date;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, &str_date,
                        -1 );

    bet_transfert_new_line_dialog ( model, str_date );
}


/**
 * init data
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
    gint account_number;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    account_number = gsb_gui_navigation_get_current_account ( );
    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
    bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
}


/**
 * Cette fonction recalcule le montant des données historiques en fonction de la
 * consommation mensuelle précédente. affiche le nouveau montant si même signe ou 0 
 * et un message pour budget dépassé.
 * 
 * */
void bet_array_adjust_hist_amount ( gint div_number,
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
        gint currency_number;

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

            if ( tmp_sub_div_nb == 0 || tmp_sub_div_nb == sub_div_nb )
            {
                div_name = bet_data_get_div_name ( tmp_div_number, tmp_sub_div_nb, FALSE );

                currency_number = bet_data_get_selected_currency ( );
                date = gsb_parse_date_string ( str_date );
                date_today = gdate_today ( );
                if ( g_date_get_month ( date ) - g_date_get_month ( date_today ) == 0 )
                {
                    number = gsb_real_safe_real_from_string ( str_amount );
                    if ( number.mantissa != 0 )
                    {
                        sign = bet_data_get_div_type ( div_number );
                        number = gsb_real_sub ( number, amount );

                        if ( str_amount )
                            g_free ( str_amount );
                        if ( str_desc )
                            g_free ( str_desc );
                        if ( sign == 1 )
                        {
                            if ( number.mantissa < 0 )
                            {
                                str_amount = gsb_real_safe_real_to_string ( number, 
                                            gsb_data_currency_get_floating_point ( currency_number ) );
                                str_debit = gsb_real_get_string_with_currency (
                                            gsb_real_abs ( number ),
                                            currency_number,
                                            TRUE );
                                str_desc = g_strconcat ( div_name, _(" (still available)"), NULL);
                            }
                            else
                            {
                                str_debit = gsb_real_get_string_with_currency (
                                            null_real,
                                            currency_number,
                                            TRUE );
                                str_amount = g_strdup ( "0.00" );
                                str_desc = g_strconcat ( div_name, _(" (budget exceeded)"), NULL);
                            }
                        }
                        else
                        {
                            if ( number.mantissa > 0 )
                            {
                                str_amount = gsb_real_safe_real_to_string ( number, 
                                            gsb_data_currency_get_floating_point ( currency_number ) );
                                str_credit = gsb_real_get_string_with_currency (
                                            gsb_real_abs ( number ),
                                            currency_number,
                                            TRUE );
                                str_desc = g_strconcat ( div_name, _(" (yet to receive)"), NULL);
                            }
                            else
                            {
                                str_credit = gsb_real_get_string_with_currency (
                                            null_real,
                                            currency_number,
                                            TRUE );
                                str_amount = g_strdup ( "0.00" );
                                str_desc = g_strconcat ( div_name, _(" (budget exceeded)"), NULL);
                            }
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
        current_balance = gsb_real_safe_real_from_string ( str_current_balance );

        tmp_range = struct_initialise_bet_range ( );
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
gboolean bet_array_entry_key_press ( GtkWidget *entry,
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
gboolean bet_array_start_date_focus_out ( GtkWidget *entry,
                        GdkEventFocus *ev,
                        gpointer data )
{
    gint account_number;
    GDate *date;

    devel_debug (gtk_entry_get_text ( GTK_ENTRY ( entry ) ));
    gtk_editable_select_region ( GTK_EDITABLE ( entry ), 0, 0 );
    account_number = gsb_gui_navigation_get_current_account ( );

    date = gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );
    if ( gsb_data_account_get_bet_auto_inc_month (  account_number ) )
    {
        GDate *old_date;

        old_date = gsb_data_account_get_bet_start_date ( account_number );
        if ( g_date_compare ( date, old_date ) != 0 )
            gsb_data_account_set_bet_auto_inc_month ( account_number, FALSE );
    }
    
    gsb_data_account_set_bet_start_date ( account_number, date );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
    bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );

    return FALSE;
}


/**
 * select path in the list
 *
 * \param tree_view, path
 *
 * \return FALSE
 * */
gboolean bet_array_list_select_path ( GtkWidget *tree_view, GtkTreePath *path )
{
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( path == NULL )
        path = gtk_tree_path_new_from_string ( "0" );

    gtk_widget_grab_focus ( tree_view );
    gtk_tree_selection_select_path ( selection, path );

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

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gint origine;
        gint current_color = 0;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
			            &iter,
			            SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
			            -1 );
            switch ( origine )
            {
            case SPP_ORIGIN_TRANSACTION:
            case SPP_ORIGIN_SCHEDULED:

                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_fond[current_color],
                        -1 );
                current_color = !current_color;
                break;
            case SPP_ORIGIN_HISTORICAL:
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_bet_division,
                        SPP_ESTIMATE_TREE_COLOR_STRING, gdk_color_to_string ( &couleur_bet_division ),
                        -1 );
                break;
            case SPP_ORIGIN_FUTURE:
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_bet_future,
                        SPP_ESTIMATE_TREE_COLOR_STRING, gdk_color_to_string ( &couleur_bet_future ),
                        -1 );
                break;
            case SPP_ORIGIN_ACCOUNT:
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_bet_transfert,
                        SPP_ESTIMATE_TREE_COLOR_STRING, gdk_color_to_string ( &couleur_bet_transfert ),
                        -1 );
                break;
            case SPP_ORIGIN_SOLDE:
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_bet_solde,
                        SPP_ESTIMATE_TREE_COLOR_STRING, gdk_color_to_string ( &couleur_bet_solde ),
                        -1 );
            }
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
    GtkWidget *widget = NULL;
    GtkWidget *button = NULL;
    GtkWidget *toggled = NULL;
    gpointer pointeur;
    gint param;
    gint months;

    /* devel_debug_int ( account_number ); */
    button = g_object_get_data ( G_OBJECT ( account_page ), "bet_account_spin_button" );

    param = gsb_data_account_get_bet_spin_range ( account_number );
    months = gsb_data_account_get_bet_months ( account_number );

    g_signal_handlers_block_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        GINT_TO_POINTER ( 1 ) );

    if ( param == 0 )
    {
        widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_account_previous" );
        g_signal_handlers_block_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, prev_month_max );
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months );
    }
    else
    {
        widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_account_widget" );
        g_signal_handlers_block_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, prev_month_max / 12.0 );
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months / 12.0 );
    }

    g_signal_handlers_unblock_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        GINT_TO_POINTER ( 1 ) );

    toggled = g_object_get_data ( G_OBJECT ( account_page ), "bet_auto_inc_month" );
    g_signal_handlers_block_by_func ( G_OBJECT ( toggled ),
                        G_CALLBACK ( bet_array_auto_inc_month_toggle ),
                        NULL );

    param = gsb_data_account_get_bet_auto_inc_month ( account_number );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( toggled ), param );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( toggled ),
                        G_CALLBACK ( bet_array_auto_inc_month_toggle ),
                        NULL );

    param = gsb_data_account_get_bet_hist_data ( account_number );

    if ( param == 1 )
        button = g_object_get_data ( G_OBJECT ( account_page ), "bet_hist_button_2" );
    else
        button = g_object_get_data ( G_OBJECT ( account_page ), "bet_hist_button_1" );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );

    param = gsb_data_account_get_bet_hist_fyear ( account_number );
    widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_hist_fyear_combo" );
    pointeur = g_object_get_data ( G_OBJECT ( widget ), "pointer" );
    g_signal_handlers_block_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_fyear_clicked ),
                        pointeur );

    bet_historical_set_fyear_from_combobox ( widget, param );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_fyear_clicked ),
                        pointeur );

    return FALSE;
}
/**
 * Cette fonction permet de sauter les opérations planifiées qui sont
 * remplacées par des données historiques
 * 
 * \return TRUE si l'opération doit être ignorée
 * */
gboolean bet_array_sort_scheduled_transactions ( gint div_number,
                        gint sub_div_nb,
                        GtkTreeModel *model )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gint tmp_div_number;
        gint tmp_sub_div_nb;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                        &iter,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, &tmp_div_number,
                        SPP_ESTIMATE_TREE_SUB_DIV_COLUMN, &tmp_sub_div_nb,
                        -1 );

            if ( tmp_div_number == 0 || tmp_div_number != div_number )
                continue;

            if ( tmp_sub_div_nb == 0 || tmp_sub_div_nb == sub_div_nb )
                return TRUE;
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
void bet_array_list_traite_double_click ( GtkTreeView *tree_view )
{
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection ( tree_view );
    bet_array_list_change_menu ( NULL, selection );
}


/**
 * Convert selected line to a template of scheduled transaction.
 * 
 */
void bet_array_list_schedule_selected_line ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint origine;
    gint number;
    gint scheduled_number;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ), &model, &iter ) )
        return;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, &number,
                        -1 );

    scheduled_number = bet_array_list_schedule_line ( origine,
                        gsb_gui_navigation_get_current_account ( ),
                        number );

    mise_a_jour_liste_echeances_auto_accueil = 1;

    gsb_gui_navigation_set_selection ( GSB_SCHEDULER_PAGE, 0, NULL );
    gsb_scheduler_list_select ( scheduled_number );
    gsb_scheduler_list_edit_transaction ( scheduled_number );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/**
 *  Convert future line to a template of scheduled transaction.
 *
 * \param transaction  to use as a template.
 *
 * \return the number of the scheduled transaction
 */
gint bet_array_list_schedule_line ( gint origine, gint account_number, gint number )
{
    gint scheduled_number = 0;

    scheduled_number = gsb_data_scheduled_new_scheduled ();

    if ( !scheduled_number)
	return FALSE;

    if ( origine == SPP_ORIGIN_FUTURE )
    {
        struct_futur_data *scheduled;

        scheduled = bet_data_future_get_struct ( account_number, number );
        if ( scheduled == NULL )
            return 0;

        gsb_data_scheduled_set_account_number ( scheduled_number, scheduled -> account_number );
        gsb_data_scheduled_set_date ( scheduled_number, scheduled -> date );
        gsb_data_scheduled_set_amount ( scheduled_number, scheduled -> amount );
        gsb_data_scheduled_set_currency_number ( scheduled_number,
                             gsb_data_account_get_currency ( account_number ) );
        gsb_data_scheduled_set_party_number ( scheduled_number, scheduled -> party_number );
        gsb_data_scheduled_set_category_number ( scheduled_number, scheduled -> category_number );
        gsb_data_scheduled_set_sub_category_number ( scheduled_number, scheduled -> sub_category_number );
        gsb_data_scheduled_set_notes ( scheduled_number, scheduled -> notes );
        gsb_data_scheduled_set_method_of_payment_number ( scheduled_number, scheduled -> payment_number );
        gsb_data_scheduled_set_financial_year_number ( scheduled_number, scheduled -> fyear_number );
        gsb_data_scheduled_set_budgetary_number ( scheduled_number, scheduled -> budgetary_number );
        gsb_data_scheduled_set_sub_budgetary_number ( scheduled_number, scheduled -> sub_budgetary_number );

        /* par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date,
         * (c'est le cas, à 0 avec g_malloc0) que l'opé soit enregistrée immédiatement */
        gsb_data_scheduled_set_frequency ( scheduled_number, scheduled -> frequency );
        
    }

    return scheduled_number;
}


/**
 *
 *
 *
 *
 * */
gchar *bet_array_list_get_description ( gint account_number,
                        gint origine,
                        gpointer value )
{
    gchar *desc = NULL;
    gint type;

    type = gsb_data_account_get_bet_select_label ( account_number, origine );

    if ( origine == SPP_ORIGIN_TRANSACTION )
    {
        gint transaction_number = GPOINTER_TO_INT ( value );

        switch ( type )
        {
            case 0:
                desc = g_strdup ( gsb_data_transaction_get_notes ( transaction_number ) );
                if ( desc && strlen ( desc ) )
                    break;

                desc = g_strdup ( gsb_data_payee_get_name (
                                    gsb_data_transaction_get_party_number (
                                    transaction_number ), TRUE ) );
                if ( desc && strlen ( desc ) )
                    break;

                desc = g_strdup ( gsb_data_category_get_name (
                                    gsb_data_transaction_get_category_number (
                                    transaction_number ),
                                    gsb_data_transaction_get_sub_category_number (
                                    transaction_number ), NULL) );
                if ( desc && strlen ( desc ) )
                    break;

                desc = g_strdup ( gsb_data_budget_get_name (
                                    gsb_data_transaction_get_budgetary_number (
                                    transaction_number ),
                                    gsb_data_transaction_get_sub_category_number (
                                    transaction_number ),
                                    _("No data by default") ) );
                break;
            case 1:
                desc = g_strdup ( gsb_data_category_get_name (
                                    gsb_data_transaction_get_category_number (
                                    transaction_number ),
                                    gsb_data_transaction_get_sub_category_number (
                                    transaction_number ),
                                    _("No category") ) );
                break;
            case 2:
                desc = g_strdup ( gsb_data_budget_get_name (
                                    gsb_data_transaction_get_budgetary_number (
                                    transaction_number ),
                                    gsb_data_transaction_get_sub_category_number (
                                    transaction_number ),
                                    _("No budgetary line") ) );
                break;
        }
    }
    else if ( origine == SPP_ORIGIN_SCHEDULED )
    {
        gint scheduled_number = GPOINTER_TO_INT ( value );

        switch ( type )
        {
            case 0:
                desc = g_strdup ( gsb_data_scheduled_get_notes ( scheduled_number ) );
                if ( desc && strlen ( desc ) )
                    break;

                desc = g_strdup ( gsb_data_payee_get_name (
                                    gsb_data_scheduled_get_party_number (
                                    scheduled_number ), TRUE ) );
                if ( desc && strlen ( desc ) )
                    break;

                desc = g_strdup ( gsb_data_category_get_name (
                                    gsb_data_scheduled_get_category_number (
                                    scheduled_number ),
                                    gsb_data_scheduled_get_sub_category_number (
                                    scheduled_number ), NULL) );
                if ( desc && strlen ( desc ) )
                    break;

                desc = g_strdup ( gsb_data_budget_get_name (
                                    gsb_data_scheduled_get_budgetary_number (
                                    scheduled_number ),
                                    gsb_data_scheduled_get_sub_category_number (
                                    scheduled_number ),
                                    _("No data by default") ) );
                break;
            case 1:
                desc = g_strdup ( gsb_data_category_get_name (
                                    gsb_data_scheduled_get_category_number (
                                    scheduled_number ),
                                    gsb_data_scheduled_get_sub_category_number (
                                    scheduled_number ),
                                    _("No category") ) );
                break;
            case 2:
                desc = g_strdup ( gsb_data_budget_get_name (
                                    gsb_data_scheduled_get_budgetary_number (
                                    scheduled_number ),
                                    gsb_data_scheduled_get_sub_category_number (
                                    scheduled_number ),
                                    _("No budgetary line") ) );
                break;
        }
    }
    else if ( origine == SPP_ORIGIN_FUTURE )
    {
        struct_futur_data *scheduled = ( struct_futur_data * ) value;

        if ( scheduled -> is_transfert )
        {
            if ( account_number == scheduled -> account_number )
                desc = g_strdup_printf ( _("Transfer between account: %s\n"
                        "and account: %s"),
                        gsb_data_account_get_name ( account_number ),
                        gsb_data_account_get_name ( scheduled -> account_transfert ) );
            else
                desc = g_strdup_printf ( _("Transfer between account: %s\n"
                        "and account: %s"),
                        gsb_data_account_get_name ( scheduled -> account_transfert ),
                        gsb_data_account_get_name ( scheduled -> account_number ) );
        }
        else
        {
            switch ( type )
            {
                case 0:
                    desc = g_strdup ( scheduled -> notes );
                    if ( desc && strlen ( desc ) )
                        break;
                    
                    desc = g_strdup ( gsb_data_payee_get_name (
                                    scheduled -> party_number, TRUE ) );
                    if ( desc && strlen ( desc ) )
                        break;

                    desc = g_strdup ( gsb_data_category_get_name (
                                    scheduled -> category_number,
                                    scheduled -> sub_category_number, NULL) );
                    if ( desc && strlen ( desc ) )
                        break;

                    desc = g_strdup ( gsb_data_budget_get_name (
                                    scheduled -> budgetary_number,
                                    scheduled -> sub_budgetary_number,
                                    _("No data by default") ) );
                    break;
                case 1:
                    desc = g_strdup ( gsb_data_category_get_name (
                                    scheduled -> category_number,
                                    scheduled -> sub_category_number,
                                    _("No category") ) );
                    break;
                case 2:
                    desc = g_strdup ( gsb_data_budget_get_name (
                                    scheduled -> budgetary_number,
                                    scheduled -> sub_budgetary_number,
                                    _("No budgetary line") ) );
                    break;
            }
        }
    }
    else if ( origine == SPP_ORIGIN_ACCOUNT )
    {
        struct_transfert_data *transfert = ( struct_transfert_data * ) value;

        if ( transfert -> type == 0 )
            desc = g_strdup_printf ("Solde du compte : %s",
                        gsb_data_account_get_name ( transfert -> replace_account ) );
        else
            desc = g_strdup_printf ("Solde partiel : %s",
                        gsb_data_partial_balance_get_name ( transfert -> replace_account ) );
    }

    return desc;
}


/**
 *
 *
 *
 *
 * */
void bet_array_auto_inc_month_toggle ( GtkToggleButton *togglebutton, gpointer  data )
{
    gint account_number;
    gboolean value;
    gint auto_inc_month;
    
    devel_debug_int (gtk_toggle_button_get_active ( togglebutton ));

    account_number = gsb_gui_navigation_get_current_account ( );
    auto_inc_month = gsb_data_account_get_bet_auto_inc_month ( account_number );

    if ( ( value = gtk_toggle_button_get_active ( togglebutton ) ) != auto_inc_month )
    {
        gsb_data_account_set_bet_auto_inc_month ( account_number, value );

        if ( value )
        {
            gsb_data_account_bet_update_initial_date_if_necessary ( account_number );
            gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
            bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
        }

        if ( etat.modification_fichier == 0 )
            modification_fichier ( TRUE );
    }
}


/**
 *
 *
 *
 *
 * */
gboolean bet_array_refresh_transfert_data ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max )
{
    GHashTable *transfert_list;
    GHashTableIter iter;
    gpointer key, value;
    gint account_number;

    devel_debug (NULL);

    account_number = gsb_gui_navigation_get_current_account ( );
    transfert_list = bet_data_transfert_get_list ( );

    g_hash_table_iter_init ( &iter, transfert_list );
    while ( g_hash_table_iter_next ( &iter, &key, &value ) )
    {
        struct_transfert_data *transfert = ( struct_transfert_data *) value;
        GtkTreeIter tab_iter;
        GValue date_value = {0, };
        gchar *str_debit = NULL;
        gchar *str_credit = NULL;
        gchar *str_date;
        gint currency_number;
        gint replace_currency;
        gchar *str_description;
        gchar *str_amount;
        gsb_real amount;

        if ( account_number != transfert -> account_number )
            continue;

        if ( transfert -> auto_inc_month )
            bet_data_transfert_update_date_if_necessary ( transfert );

        if ( g_date_compare ( transfert -> date, date_max ) > 0 )
            continue;
        if ( g_date_compare ( transfert -> date, date_min ) < 0 )
            continue;

        str_description = bet_array_list_get_description ( transfert -> replace_account,
                        SPP_ORIGIN_ACCOUNT,
                        value );

        if ( transfert -> type == 0 )
        {
            amount = gsb_data_account_get_current_balance ( transfert -> replace_account );
            replace_currency = gsb_data_account_get_currency ( transfert -> replace_account );
        }
        else
        {
            amount = gsb_data_partial_balance_get_current_amount ( transfert -> replace_account );
            replace_currency = gsb_data_partial_balance_get_currency ( transfert -> replace_account );
        }

        str_amount = bet_data_get_str_amount_in_account_currency ( amount,
                        account_number,
                        replace_currency,
                        SPP_ORIGIN_ACCOUNT );

        currency_number = gsb_data_account_get_currency ( transfert -> replace_account );
        if (amount.mantissa < 0)
            str_debit = gsb_real_get_string_with_currency ( gsb_real_abs ( amount ), currency_number, TRUE );
        else
            str_credit = gsb_real_get_string_with_currency ( amount, currency_number, TRUE);

        str_date = gsb_format_gdate ( transfert -> date );

        g_value_init ( &date_value, G_TYPE_DATE );
        g_value_set_boxed ( &date_value, transfert -> date ); 

        /* add a line in the estimate array */
        gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &tab_iter, NULL );
        gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );
        gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, SPP_ORIGIN_ACCOUNT,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, transfert -> number,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
                        SPP_ESTIMATE_TREE_DEBIT_COLUMN, str_debit,
                        SPP_ESTIMATE_TREE_CREDIT_COLUMN, str_credit,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        -1);

        if (  transfert -> replace_transaction )
            bet_array_list_replace_planned_line_by_transfert ( tab_model, transfert );
        g_value_unset ( &date_value );
        g_free ( str_date );
        g_free ( str_description );
        g_free ( str_amount );
        if ( str_debit )
            g_free ( str_debit );
        if ( str_credit )
            g_free ( str_credit );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_array_list_replace_planned_line_by_transfert ( GtkTreeModel *tab_model,
                        struct_transfert_data *transfert )
{
    GtkTreeIter iter;

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( tab_model ), &iter ) )
    {
        GtkTreeIter *tmp_iter = NULL;
        gchar* str_date;
        GDate *date_debut_comparaison;
        GDate *date_fin_comparaison;
        GDate *date;
        gint scheduled_number;
        gint origine;

        date_debut_comparaison = g_date_new_dmy ( g_date_get_day ( transfert -> date ),
                    g_date_get_month ( transfert -> date ),
                    g_date_get_year ( transfert -> date ));
        g_date_subtract_days ( date_debut_comparaison,
                    valeur_echelle_recherche_date_import );

        date_fin_comparaison = g_date_new_dmy ( g_date_get_day ( transfert -> date ),
                    g_date_get_month ( transfert -> date ),
                    g_date_get_year ( transfert -> date ));
        g_date_add_days ( date_fin_comparaison,
                    valeur_echelle_recherche_date_import );

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( tab_model ),
                        &iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, &origine,
                        SPP_ESTIMATE_TREE_DIVISION_COLUMN, &scheduled_number,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, &str_date,
                        -1 );

            if ( origine != SPP_ORIGIN_SCHEDULED )
                continue;

            date = gsb_parse_date_string ( str_date );
            if ( g_date_compare ( date, date_debut_comparaison ) < 0 )
                continue;

            if ( g_date_compare ( date, date_fin_comparaison ) > 0 )
            {
                if ( tmp_iter )
                    gtk_tree_store_remove ( GTK_TREE_STORE ( tab_model ), tmp_iter );
                break;
            }

            if ( transfert -> category_number )
            {
                /* on cherche une opération par sa catégorie */
                gint tmp_category_number = 0;
                gint tmp_sub_category_number = 0;

                tmp_category_number = gsb_data_scheduled_get_category_number ( scheduled_number );
                if ( transfert -> sub_category_number )
                    tmp_sub_category_number = gsb_data_scheduled_get_sub_category_number (
                                                    scheduled_number );

                if ( transfert -> category_number == tmp_category_number
                 &&
                    transfert -> sub_category_number == tmp_sub_category_number )
                {
                    if ( g_date_compare ( date, transfert -> date ) == 0 )
                    {
                        gtk_tree_store_remove ( GTK_TREE_STORE ( tab_model ), &iter );
                        break;
                    }
                    tmp_iter = gtk_tree_iter_copy ( &iter );
                }
            }
            else if ( transfert -> budgetary_number )
            {
                /* on cherche une opération par son IB */
                gint tmp_budget_number;
                gint tmp_sub_budget_number;

                tmp_budget_number = gsb_data_scheduled_get_budgetary_number ( scheduled_number );
                if ( transfert -> sub_budgetary_number )
                    tmp_sub_budget_number = gsb_data_scheduled_get_sub_budgetary_number (
                                                    scheduled_number );

                if ( transfert -> budgetary_number == tmp_budget_number
                 &&
                    transfert -> sub_budgetary_number == tmp_sub_budget_number )
                {
                    if ( g_date_compare ( date, transfert -> date ) == 0 )
                    {
                        gtk_tree_store_remove ( GTK_TREE_STORE ( tab_model ), &iter );
                        break;
                    }
                    tmp_iter = gtk_tree_iter_copy ( &iter );
                }
            }
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( tab_model ), &iter ) );
    }

    return FALSE;
}


/**
 * called when the size of the tree view changed, to keep the same ration
 * between the columns
 *
 * \param tree_view	    the tree view of the list
 * \param allocation	the new size
 * \param null
 *
 * \return FALSE
 * */
gboolean bet_array_list_size_allocate ( GtkWidget *tree_view,
                        GtkAllocation *allocation,
                        gpointer null )
{
    gint i;

    if ( allocation -> width == bet_array_current_tree_view_width )
    {
        /* size of the tree view didn't change, but we received an allocated signal
         * it happens several times, and especially when we change the columns,
         * so we update the colums */

        /* sometimes, when the list is not visible, he will set all the columns to 1%... we block that here */
        if ( gtk_tree_view_column_get_width ( bet_array_tree_view_columns[0]) == 1 )
            return FALSE;

        for ( i = 0 ; i < BET_ARRAY_COLUMNS; i++ )
        {
            bet_array_col_width[i] = ( gtk_tree_view_column_get_width (
                                            bet_array_tree_view_columns[i]) * 100 ) / allocation -> width + 1;
        }

        return FALSE;
    }

    /* the size of the tree view changed, we keep the ratio between the columns,
     * we don't set the size of the last column to avoid the calculate problems,
     * it will take the end of the width alone */
    bet_array_current_tree_view_width = allocation -> width;

    for ( i = 0; i < BET_ARRAY_COLUMNS - 1; i++ )
    {
        gint width;

        width = ( bet_array_col_width[i] * ( allocation -> width ) ) / 100;
        if ( width > 0 )
            gtk_tree_view_column_set_fixed_width ( bet_array_tree_view_columns[i], width );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_array_list_create_toolbar ( GtkWidget *parent, GtkWidget *tree_view )
{
    GtkWidget *handlebox;
    GtkWidget *hbox;
    GtkWidget *button;

    /* HandleBox */
    handlebox = gtk_handle_box_new ( );

    /* Hbox */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( handlebox ), hbox );

    /* print button */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
                        GTK_STOCK_PRINT,
                        _("Print"),
                        NULL,
                        NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("Print the array") );
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( print_tree_view_list ),
                        tree_view );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );

    /* Export button */
    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_SAVE,
					   _("Export"),
					   NULL,
					   NULL );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( button ), _("Export the array of forecast") );
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( bet_array_export_tab ),
                        tree_view );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );

    gtk_widget_show_all ( handlebox );

    return ( handlebox );

}


/**
 *
 *
 *
 *
 * */
gboolean bet_array_shows_balance_at_beginning_of_month ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max )
{
    GDate *date;
    gchar *str_amount;

    date = gsb_date_copy ( date_min );
    g_date_add_months ( date, 1 );
    g_date_set_day ( date, 1 );

    str_amount = gsb_real_safe_real_to_string ( null_real, 
                        gsb_data_currency_get_floating_point (
                        bet_data_get_selected_currency ( ) ) );

    while ( g_date_compare ( date, date_max ) < 0 )
    {
        GtkTreeIter tab_iter;
        GValue date_value = {0, };
        gchar *str_date;
        gchar *str_description;

        g_value_init ( &date_value, G_TYPE_DATE );
        g_value_set_boxed ( &date_value, date );

        str_date = gsb_format_gdate ( date );
        str_description = g_strconcat ( _("Balance at "), str_date, NULL ); 

        /* add a line in the estimate array */
        gtk_tree_store_append ( GTK_TREE_STORE ( tab_model ), &tab_iter, NULL );
        gtk_tree_store_set_value ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_SORT_DATE_COLUMN,
                        &date_value );

        gtk_tree_store_set ( GTK_TREE_STORE ( tab_model ), &tab_iter,
                        SPP_ESTIMATE_TREE_ORIGIN_DATA, SPP_ORIGIN_SOLDE,
                        SPP_ESTIMATE_TREE_DATE_COLUMN, str_date,
                        SPP_ESTIMATE_TREE_DESC_COLUMN, str_description,
                        SPP_ESTIMATE_TREE_AMOUNT_COLUMN, str_amount,
                        SPP_ESTIMATE_TREE_BALANCE_COLOR, NULL,
                        SPP_ESTIMATE_TREE_BACKGROUND_COLOR, &couleur_bet_solde,
                        -1);

        g_value_unset ( &date_value );
        g_free ( str_date );
        g_free ( str_description );

        g_date_add_months ( date, 1 );
    }
    g_free ( str_amount );
    g_date_free ( date );
    
    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void bet_array_export_tab ( GtkWidget *menu_item, GtkTreeView *tree_view )
{
    GtkWidget *dialog;
    gint resultat;
    gchar *filename;

    dialog = gtk_file_chooser_dialog_new ( _("Export the array of forecast"),
					   GTK_WINDOW ( window ),
					   GTK_FILE_CHOOSER_ACTION_SAVE,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_SAVE, GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( dialog ),  _("forecast.csv"));
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( dialog ), gsb_file_get_last_path () );
    gtk_file_chooser_set_do_overwrite_confirmation ( GTK_FILE_CHOOSER ( dialog ), TRUE);
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    filename = file_selection_get_filename ( GTK_FILE_CHOOSER ( dialog ) );
	    gsb_file_update_last_path ( file_selection_get_last_directory ( GTK_FILE_CHOOSER ( dialog ), TRUE ) );
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

	    /* vérification que c'est possible est faite par la boite de dialogue */
	    if ( !gsb_csv_export_tree_view_list ( filename, tree_view ) )
	    {
            dialogue_error ( _("Cannot save file.") );
            return;
	    }

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( dialog ));
	    return;
    }
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
