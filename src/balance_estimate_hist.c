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
#include "balance_estimate_hist.h"
#include "./balance_estimate_data.h"
#include "./balance_estimate_tab.h"
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
#include "./include.h"
#include "./structures.h"
#include "./traitement_variables.h"
#include "./erreur.h"
#include "./utils.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean bet_historical_amount_differ_average ( GtkTreeModel *model,
                        GtkTreeIter *iter );
static void bet_historical_div_cell_edited (GtkCellRendererText *cell,
                        const gchar *path_string,
                        const gchar *new_text,
                        GtkWidget *tree_view );
static void bet_historical_div_cell_editing_started ( GtkCellRenderer *cell,
                        GtkCellEditable *editable,
                        const gchar *path_string,
                        GtkWidget *tree_view );
static gboolean bet_historical_div_toggle_clicked ( GtkCellRendererToggle *renderer,
                        gchar *path_string,
                        GtkTreeModel *store );
gsb_real bet_historical_get_children_amount ( GtkTreeModel *model, GtkTreeIter *parent );
static GtkWidget *bet_historical_get_data ( GtkWidget *container );
static gboolean bet_historical_get_full_div ( GtkTreeModel *model, GtkTreeIter *parent );
static void bet_historical_populate_div_model ( gpointer key,
                        gpointer value,
                        gpointer user_data);
static void bet_historical_row_collapse_all ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreeModel *model );
static void bet_historical_row_expanded_event ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        GtkTreeModel *model );
static gboolean bet_historical_set_empty_sub_div ( GtkTreeModel *model, GtkTreeIter *parent );
static gboolean bet_historical_set_full_sub_div ( GtkTreeModel *model, GtkTreeIter *parent );
/*END_STATIC*/

/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern GtkWidget *bet_container;
extern gsb_real null_real;
extern GtkWidget *window;
/*END_EXTERN*/

/**
 * this is a tree model filter with 3 columns :
 * the name, the number and a boolean to show it or not
 * */
GtkTreeModel *bet_fyear_model = NULL;

/**
 * this is a tree model filter from fyear_model_filter wich
 * show only the financial years wich must be showed
 * */
GtkTreeModel *bet_fyear_model_filter = NULL;

 
/**
 * Create the historical page
 *
 *
 *
 * */
void bet_historical_create_page ( GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *button_1, *button_2;
    GtkWidget *tree_view;
    gchar *str_year;
    gchar *title;
    gint year;

    devel_debug (NULL);
    widget = gtk_label_new( _("Historical data") );
    page = gtk_vbox_new ( FALSE, 5 );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        GTK_WIDGET ( page ), GTK_WIDGET ( widget ) );

    /* titre de la page */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( page ), hbox, FALSE, FALSE, 15 );
 
    widget = gtk_image_new_from_stock(
                        GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );

    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, FALSE, FALSE, 5) ;

    widget = gtk_label_new ( NULL );
    title = g_strdup_printf (
                        _("Please select the data source for the account: \"%s\""),
                        gsb_data_account_get_name (
                        bet_parameter_get_account_selected ( ) ) );
    gtk_label_set_markup ( GTK_LABEL ( widget ), title );
    g_free ( title );
    gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 5 );

    /* Choix des données sources */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 15 );
    g_object_set_data ( G_OBJECT ( bet_container ), "bet_historical_data", hbox );

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("Categories") );
    gtk_widget_set_name ( button_1, "button_1" );
    g_signal_connect ( G_OBJECT ( button_1 ),
                        "released",
                        G_CALLBACK ( bet_historical_origin_data_clicked ),
                        NULL );
    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("Budgetary lines") );
    gtk_widget_set_name ( button_2, "button_2" );
    g_signal_connect ( G_OBJECT ( button_2 ),
                        "released",
                        G_CALLBACK ( bet_historical_origin_data_clicked ),
                        NULL );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    if ( etat.bet_hist_data == 0 )
    {
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );
        bet_data_set_div_ptr ( etat.bet_hist_data );
    }
    else
    {
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_2 ), TRUE );
        bet_data_set_div_ptr ( etat.bet_hist_data );
    }

    /* création du sélecteur de périod */
    if ( bet_historical_fyear_create_combobox_store ( ) )
    {
        widget = gsb_fyear_make_combobox_new ( bet_fyear_model_filter, TRUE );
        gtk_widget_set_name ( GTK_WIDGET ( widget ), "fyear_combo" );
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( widget ),
                        SPACIFY(_("Choose the financial year or 12 months rolling") ) );

        g_object_set_data ( G_OBJECT ( notebook ), "bet_historical_fyear", widget );

        gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5);

        /* hide the present financial year */
        year = g_date_get_year ( gdate_today ( ) );
        str_year = utils_str_itoa ( year );
        gsb_fyear_hide_iter_by_name ( bet_fyear_model, str_year );
        g_free ( str_year );

        /* show the old choice */
        if ( etat.bet_hist_fyear > 0 )
        {
            gsb_fyear_select_iter_by_number ( widget,
                        bet_fyear_model,
                        bet_fyear_model_filter,
                        etat.bet_hist_fyear );
        }

        /* set the signal */
        g_signal_connect ( G_OBJECT ( widget ),
                        "changed",
                        G_CALLBACK (bet_historical_fyear_clicked),
                        NULL );
    }

    /* création de la liste des données */
    tree_view = bet_historical_get_data ( page );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_historical_treeview", tree_view );

    gtk_widget_show_all ( page );
}


/**
 *
 *
 *
 *
 * */
void bet_historical_origin_data_clicked ( GtkWidget *togglebutton, gpointer data )
{
    GtkWidget *ancestor;
    GtkWidget *widget;
    GtkTreeViewColumn *column;
    const gchar *name;
    gchar *title;

    devel_debug (NULL);
    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );
    ancestor = g_object_get_data ( G_OBJECT ( bet_container ), "bet_historical_data" );
    if ( gtk_widget_is_ancestor ( togglebutton, ancestor ) == FALSE )
    {
        widget = utils_get_child_widget_by_name ( ancestor, name );
        if ( widget )
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
    }

    if ( g_strcmp0 ( name, "button_1" ) == 0 )
    {
        etat.bet_hist_data = 0;
        bet_data_set_div_ptr ( etat.bet_hist_data );
        title = g_strdup ( _("Category") );
    }
    else
    {
        etat.bet_hist_data = 1;
        bet_data_set_div_ptr ( etat.bet_hist_data );
        title = g_strdup ( _("Budgetary line") );
    }

    column = g_object_get_data ( G_OBJECT ( bet_container ),
                        "historical_column_source" );
    gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN ( column ), title );

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
void bet_historical_fyear_clicked ( GtkWidget *combo, gpointer data )
{
    GtkWidget *ancestor;
    GtkWidget *widget;
    const gchar *name;

    devel_debug (NULL);
    etat.bet_hist_fyear = bet_historical_get_fyear_from_combobox ( combo );

    name = gtk_widget_get_name ( GTK_WIDGET ( combo ) );
    ancestor = g_object_get_data ( G_OBJECT ( bet_container ), "bet_historical_data" );
    if ( gtk_widget_is_ancestor ( combo, ancestor ) == FALSE )
    {
        widget = utils_get_child_widget_by_name ( ancestor, name );
        if ( widget )
        {
            gsb_fyear_select_iter_by_number ( widget,
                        bet_fyear_model,
                        bet_fyear_model_filter,
                        etat.bet_hist_fyear );
        }

    }

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
gboolean bet_historical_div_toggle_clicked ( GtkCellRendererToggle *renderer,
                        gchar *path_string,
                        GtkTreeModel *model )
{
    GtkTreeIter iter;

    devel_debug (path_string);
    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( model ), &iter, path_string ) )
    {
        gchar *str_amount;
        gchar *str_average;
        gint div;
        gint sub_div;
        gint nbre_fils;
        gint account_nb;
        gboolean valeur;
        gboolean col_edited;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                        SPP_HISTORICAL_AVERAGE_COLUMN, &str_average,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div,
                        SPP_HISTORICAL_EDITED_COLUMN, &col_edited,
                        -1 );

        if ( col_edited == FALSE && valeur == FALSE )
            return FALSE;

        valeur = 1 - valeur;
        if ( valeur == 1 )
        {
            //~ printf ("avant - account_nb = %d, div = %d, sub_div = %d\n", account_nb, div, sub_div);
            bet_data_add_div_hist ( account_nb, div, sub_div );
            bet_data_set_div_amount ( account_nb, div, sub_div,
                        gsb_real_import_from_string ( str_amount ) );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, 1,
                        SPP_HISTORICAL_RETAINED_COLUMN, str_average,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, div,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div,
                        -1 );
        }
        else
        {
            bet_data_remove_div_hist ( account_nb, div, sub_div );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, FALSE,
                        SPP_HISTORICAL_RETAINED_COLUMN, "",
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, div,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div,
                        SPP_HISTORICAL_EDITED_COLUMN, TRUE,
                        -1 );
        }

        nbre_fils = gtk_tree_model_iter_n_children ( GTK_TREE_MODEL ( model ), &iter );
        if ( nbre_fils > 0 )
        {
            if ( valeur == 1 )
                bet_historical_set_full_sub_div ( model, &iter );
            else
                bet_historical_set_empty_sub_div ( model, &iter );
        }
        else
        {
            GtkTreeIter parent;
            gsb_real amount;

            if ( gtk_tree_model_iter_parent ( GTK_TREE_MODEL ( model ), &parent, &iter ) )
            {
                if ( bet_historical_get_full_div ( model, &parent ) )
                {
                    amount = bet_historical_get_children_amount ( model, &parent );
                    if ( str_amount )
                        g_free ( str_amount );
                    str_amount = gsb_real_save_real_to_string ( amount, 2 );
                    if ( str_average )
                        g_free ( str_average );
                    str_average = gsb_real_get_string_with_currency ( amount,
                                gsb_data_account_get_currency ( account_nb ), TRUE );
                    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                                &parent,
                                SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                                SPP_HISTORICAL_RETAINED_COLUMN, str_average,
                                SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                                -1);
                    if ( bet_historical_amount_differ_average ( model, &parent) )
                        bet_data_set_div_amount ( account_nb, div, 0, amount );
                        
                    bet_historical_row_collapse_all ( NULL, &parent, model );
                }
                else
                {
                    bet_data_set_div_amount ( account_nb, div, 0, null_real );
                    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &parent,
                                SPP_HISTORICAL_SELECT_COLUMN, FALSE,
                                SPP_HISTORICAL_RETAINED_COLUMN, "",
                                SPP_HISTORICAL_EDITED_COLUMN, TRUE,
                                -1 );
                }
            }
        }

        bet_array_refresh_estimate_tab ( );
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void bet_historical_div_cell_editing_started (GtkCellRenderer *cell,
                        GtkCellEditable *editable,
                        const gchar *path_string,
                        GtkWidget *tree_view )
{
    if ( GTK_IS_ENTRY ( editable ) ) 
        gtk_editable_delete_text ( GTK_EDITABLE ( editable ), 0, -1 );
}


/**
 *
 *
 *
 *
 * */
void bet_historical_div_cell_edited (GtkCellRendererText *cell,
                        const gchar *path_string,
                        const gchar *new_text,
                        GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    devel_debug (new_text);
    
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( model ), &iter, path_string ) )
    {
        GtkTreeIter parent;
        gboolean valeur;
        gboolean edited;
        gint account_nb;
        gint currency_number;
        gint div;
        gint sub_div;
        gchar *tmp_str;
        gchar *str_amount;
        gboolean is_parent = FALSE;
        gsb_real number;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div,
                        SPP_HISTORICAL_EDITED_COLUMN, &edited,
                        -1 );

        if ( edited == FALSE )
            return;

        is_parent = gtk_tree_model_iter_parent ( GTK_TREE_MODEL ( model ), &parent, &iter );
        if ( is_parent == FALSE )
        {
            bet_historical_set_empty_sub_div ( model, &iter );
            bet_historical_row_collapse_all ( GTK_TREE_VIEW ( tree_view ), &iter, model );
        }

        number = gsb_real_get_from_string ( new_text );
        currency_number = gsb_data_account_get_currency ( account_nb );
        tmp_str = gsb_real_get_string_with_currency ( number, currency_number, TRUE );
        //~ printf ("div = %d sub_div = %d tmp_str = %s\n", div, sub_div, tmp_str);
        if ( bet_data_search_div_hist ( account_nb, div, sub_div ) == FALSE )
            bet_data_add_div_hist ( account_nb, div, sub_div );

        bet_data_set_div_edited  ( account_nb, div, sub_div, TRUE );
        bet_data_set_div_amount ( account_nb, div, sub_div, number );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                            SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                            SPP_HISTORICAL_RETAINED_COLUMN, tmp_str,
                            SPP_HISTORICAL_RETAINED_AMOUNT, new_text,
                            -1 );

        if ( is_parent && bet_historical_get_full_div ( model, &parent ) )
        {
            number = bet_historical_get_children_amount ( model, &parent );
            bet_data_set_div_amount ( account_nb, div, 0, number );
            str_amount = gsb_real_save_real_to_string ( number, 2 );
            if ( tmp_str )
                g_free ( tmp_str );
            tmp_str = gsb_real_get_string_with_currency ( number,
                        gsb_data_account_get_currency ( account_nb ), TRUE );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                        SPP_HISTORICAL_RETAINED_COLUMN, tmp_str,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_EDITED_COLUMN, FALSE,
                        -1);
            g_free ( str_amount );
            g_free ( tmp_str );
        }

        bet_array_refresh_estimate_tab ( );

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
GtkWidget *bet_historical_get_data ( GtkWidget *container )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkTreeSelection *tree_selection;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    gchar *title;

    //~ devel_debug (NULL);
    tree_view = gtk_tree_view_new ( );
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW (tree_view), TRUE);

    tree_model = gtk_tree_store_new ( SPP_HISTORICAL_NUM_COLUMNS,
                        G_TYPE_BOOLEAN,     /* SPP_HISTORICAL_SELECT_COLUMN     */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_DESC_COLUMN       */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_PERIOD_COLUMN     */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_BALANCE_COLUMN    */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_AVERAGE_COLUMN    */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_AVERAGE_AMOUNT    */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_RETAINED_COLUMN   */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_RETAINED_AMOUNT   */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_BALANCE_COLOR     */
                        G_TYPE_INT,         /* SPP_HISTORICAL_ACCOUNT_NUMBER    */
                        G_TYPE_INT,         /* SPP_HISTORICAL_DIV_NUMBER        */
                        G_TYPE_INT,         /* SPP_HISTORICAL_SUB_DIV_NUMBER    */
                        G_TYPE_BOOLEAN );   /* SPP_HISTORICAL_EDITED_COLUMN     */
    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( tree_model ),
					    SPP_HISTORICAL_DESC_COLUMN, GTK_SORT_ASCENDING );

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_selection_set_mode ( tree_selection, GTK_SELECTION_SINGLE );

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_widget_set_size_request ( scrolled_window, -1, 250 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );
    gtk_box_pack_start ( GTK_BOX ( container ), scrolled_window, TRUE, TRUE, 15 );

    /* create columns */
    cell = gtk_cell_renderer_toggle_new ( );
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE ( cell ),
                        FALSE );
    g_object_set (cell, "xalign", 0.5, NULL);
	g_signal_connect ( cell,
                        "toggled",
                        G_CALLBACK (bet_historical_div_toggle_clicked),
                        tree_model );

    column = gtk_tree_view_column_new_with_attributes ( _("Select"),
                        cell,
                        "active", SPP_HISTORICAL_SELECT_COLUMN,
                        NULL);
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), column );

    /* name of the div sous-div column*/
    if ( etat.bet_hist_data == 0 )
        title = g_strdup ( _("Category") );
    else
        title = g_strdup ( _("Budgetary line") );
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", SPP_HISTORICAL_DESC_COLUMN,
                        NULL);
    g_object_set_data ( G_OBJECT ( bet_container ), "historical_column_source",
                        column );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_min_width ( column, 300 );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );

    /* period analysis column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Period"), cell,
                        "text", SPP_HISTORICAL_PERIOD_COLUMN,
                        NULL);
    g_object_set (cell, "xalign", 0.5, NULL);

    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_min_width ( column, 120 );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );

    /* amount column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Amount"), cell,
                        "text", SPP_HISTORICAL_BALANCE_COLUMN,
                        "foreground", SPP_HISTORICAL_BALANCE_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_min_width ( column, 100 );
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );

    /* average column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Average"), cell,
                        "text", SPP_HISTORICAL_AVERAGE_COLUMN,
                        "foreground", SPP_HISTORICAL_BALANCE_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_min_width ( column, 100 );
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );

    /* amount retained column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Amount retained"), cell,
                        "text", SPP_HISTORICAL_RETAINED_COLUMN,
                        "foreground", SPP_HISTORICAL_BALANCE_COLOR,
                        "editable", SPP_HISTORICAL_EDITED_COLUMN,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_min_width ( column, 100 );
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );

    g_signal_connect ( cell,
                        "editing-started",
                        G_CALLBACK (bet_historical_div_cell_editing_started),
                        tree_view );
    g_signal_connect ( cell,
                        "edited",
                        G_CALLBACK (bet_historical_div_cell_edited),
                        tree_view );

    g_signal_connect ( G_OBJECT ( tree_view ),
                        "row-expanded",
                        G_CALLBACK ( bet_historical_row_expanded_event ),
                        tree_model );

    gtk_widget_show_all ( scrolled_window );

    return tree_view;
}


/**
 *
 *
 *
 *
 * */
void bet_historical_populate_data ( void )
{
    GtkWidget *tree_view;
    GtkWidget *combo;
    GtkTreeModel *model;
    gint selected_account;
    gint fyear_number;
    GDate *date_min;
    GDate *date_max;
    GSList* tmp_list;
    GHashTable  *list_div;

    //~ devel_debug (NULL);
    /* récuperation du n° de compte à utiliser */
    selected_account = bet_parameter_get_account_selected ( );
    if ( selected_account == -1 )
        return;

    tree_view = g_object_get_data (G_OBJECT ( bet_container ), "bet_historical_treeview" );
    if ( GTK_IS_TREE_VIEW ( tree_view ) == FALSE )
        return;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

    /* calculate date_min and date_max */
    combo = g_object_get_data ( G_OBJECT ( bet_container ), "bet_historical_fyear" );
    fyear_number = bet_historical_get_fyear_from_combobox ( combo );
    if ( fyear_number == 0 )
    {
        date_min = gdate_today ( );
        g_date_subtract_years ( date_min, 1 );
        date_max = gdate_today ( );
        g_date_subtract_days ( date_max, 1 );
        g_object_set_data ( G_OBJECT ( bet_container ), "bet_historical_period",
                g_strdup ( _("12 months rolling") ) );
    }
    else
    {
        date_min = gsb_data_fyear_get_beginning_date ( fyear_number );
        date_max = gsb_data_fyear_get_end_date ( fyear_number );
        g_object_set_data ( G_OBJECT ( bet_container ), "bet_historical_period",
                g_strdup ( gsb_data_fyear_get_name ( fyear_number ) ) );
    }
    list_div = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) free_struct_historical );

    /* search transactions of the account  */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ( );

    while ( tmp_list )
    {
        gint transaction_number;
        gint account_number;
        const GDate *date;

        transaction_number = gsb_data_transaction_get_transaction_number (
                        tmp_list->data );
        tmp_list = tmp_list -> next;

        account_number =  gsb_data_transaction_get_account_number (
                        transaction_number );
        if ( account_number != selected_account )
            continue;

        date = gsb_data_transaction_get_date ( transaction_number );
        /* ignore transaction which are before date_min (today) */
        if ( g_date_compare ( date, date_min ) <= 0 )
            continue;
        /* ignore transaction which are after date_max */
        if ( g_date_compare (date, date_max ) > 0 )
            continue;

        /* ignore splitted transactions */
        if ( gsb_data_transaction_get_mother_transaction_number (
         transaction_number ) != 0 )
            continue;

        bet_data_populate_div ( transaction_number, TRUE, list_div );
    }
    bet_historical_affiche_div ( list_div, tree_view );
    //~ bet_data_synchronise_hist_div_list ( list_div );

    g_hash_table_remove_all ( list_div );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_historical_affiche_div ( GHashTable  *list_div, GtkWidget *tree_view )
{
    GtkTreeModel *model;

    devel_debug (NULL);
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    g_hash_table_foreach ( list_div, bet_historical_populate_div_model, tree_view );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void bet_historical_populate_div_model ( gpointer key,
                        gpointer value,
                        gpointer user_data )
{
    SH *sh = ( SH* ) value;
    SBR *sbr = sh -> sbr;
    GtkTreeView *tree_view = ( GtkTreeView * ) user_data;
    GtkTreeModel *model;
    GtkTreeIter parent;
    GHashTableIter iter;
    gpointer sub_key, sub_value;
    gchar *div_name = NULL;
    gchar *str_balance;
    gchar *str_average;
    gchar *str_amount;
    gchar *str_retained = NULL;
    gchar *titre;
    gint div_number;
    gint account_nb;
    gboolean sub_div_visible = FALSE;
    gboolean edited = TRUE;
    gsb_real period = { 12, 0 };
    gsb_real average;
    gsb_real retained;
    gsb_real amount;

    div_number = sh -> div;
    div_name = bet_data_get_div_name ( div_number, 0, FALSE );
    account_nb = sh -> account_nb;

    titre = g_object_get_data ( G_OBJECT ( bet_container ), "bet_historical_period" );

    model = gtk_tree_view_get_model ( tree_view );

    str_balance = gsb_real_get_string_with_currency ( sbr -> current_balance, 
                        gsb_data_account_get_currency ( account_nb ), TRUE );
    average = gsb_real_div ( sbr -> current_balance, period );
    str_amount = gsb_real_save_real_to_string ( average, 2 );
    str_average = gsb_real_get_string_with_currency ( average,
                        gsb_data_account_get_currency ( account_nb ), TRUE );

    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &parent, NULL);
    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_DESC_COLUMN, div_name,
                        SPP_HISTORICAL_PERIOD_COLUMN, titre,
                        SPP_HISTORICAL_BALANCE_COLUMN, str_balance,
                        SPP_HISTORICAL_AVERAGE_COLUMN, str_average,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, str_amount,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, 0,
                        SPP_HISTORICAL_EDITED_COLUMN, TRUE,
                        -1);

    if ( bet_data_search_div_hist ( account_nb, div_number, 0 ) 
     &&
     ( bet_data_get_div_edited ( account_nb, div_number, 0 )
      ||
      g_hash_table_size ( sh -> list_sub_div ) <= 1 ) )
    {
        retained = bet_data_get_div_amount ( account_nb, div_number, 0 );
        if ( str_amount )
            g_free ( str_amount );
        str_amount = gsb_real_save_real_to_string ( retained, 2 );
        str_retained = gsb_real_get_string_with_currency ( retained,
                        gsb_data_account_get_currency ( sh -> account_nb ), TRUE );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                        SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        -1);
    }

    g_free ( div_name );
    g_free ( str_balance );
    g_free ( str_average );
    g_free ( str_amount );
    g_free ( str_retained );

    if ( g_hash_table_size ( sh -> list_sub_div ) <= 1 )
        return;

    g_hash_table_iter_init ( &iter, sh -> list_sub_div );
    while ( g_hash_table_iter_next ( &iter, &sub_key, &sub_value ) ) 
    {
        SH *sub_sh = ( SH* ) sub_value;
        SBR *sub_sbr = sub_sh -> sbr;
        GtkTreeIter fils;
        gchar **tab_str = NULL;

        //~ printf ("division = %d sub_div = %d\n", div_number, sub_sh -> div);
        div_name = bet_data_get_div_name ( div_number, sub_sh -> div, FALSE );

        if ( div_name && g_utf8_strrchr ( div_name, -1, ':' ) )
        {
	        tab_str = g_strsplit ( div_name, ":", 2 );
            if (tab_str[1])
                div_name = g_strdup ( g_strstrip ( tab_str[1] ) );
            if ( tab_str )
                g_strfreev ( tab_str );
        }

        str_balance = gsb_real_get_string_with_currency ( sub_sbr -> current_balance, 
                        gsb_data_account_get_currency ( sh -> account_nb ), TRUE );
        //~ printf ("str_balance = %s\n", str_balance );
        average = gsb_real_div ( sub_sbr -> current_balance, period );
        str_amount = gsb_real_save_real_to_string ( average, 2 );
        str_average = gsb_real_get_string_with_currency ( average,
                        gsb_data_account_get_currency ( sh -> account_nb ), TRUE );

        gtk_tree_store_append ( GTK_TREE_STORE ( model ), &fils, &parent );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &fils,
                        SPP_HISTORICAL_DESC_COLUMN, div_name,
                        SPP_HISTORICAL_BALANCE_COLUMN, str_balance,
                        SPP_HISTORICAL_AVERAGE_COLUMN, str_average,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, str_amount,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, sub_sh -> div,
                        SPP_HISTORICAL_EDITED_COLUMN, TRUE,
                        -1);

        if ( bet_data_search_div_hist ( account_nb, div_number, sub_sh -> div ) )
        {
            //~ printf ("account_nb = %d div_number = %d sub_sh -> div = %d\n", account_nb, div_number, sub_sh -> div );
            retained = bet_data_get_div_amount ( account_nb, div_number, sub_sh -> div );
            if ( str_amount )
                g_free ( str_amount );
            str_amount = gsb_real_save_real_to_string ( retained, 2 );
            str_retained = gsb_real_get_string_with_currency ( retained,
                                gsb_data_account_get_currency ( sh -> account_nb ), TRUE );

            if ( bet_data_get_div_edited ( account_nb, div_number, sub_sh -> div ) )
            {
                edited = FALSE;
            }
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &fils,
                        SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                        SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        -1);
            sub_div_visible = TRUE;
            gtk_tree_view_expand_to_path ( tree_view, gtk_tree_model_get_path ( model, &fils ) );
        }

        g_free ( div_name );
        g_free ( str_balance );
        g_free ( str_average );
        g_free ( str_amount );
        g_free ( str_retained );
    }

    if ( sub_div_visible && bet_historical_get_full_div ( model, &parent ) )
    {
        amount = bet_historical_get_children_amount ( model, &parent );
        if ( str_amount )
            g_free ( str_amount );
        str_amount = gsb_real_save_real_to_string ( amount, 2 );
        //~ printf ("str_amount = %s\n", str_amount );
        if ( str_retained )
            g_free ( str_retained );
        str_retained = gsb_real_get_string_with_currency ( amount,
                    gsb_data_account_get_currency ( account_nb ), TRUE );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                    &parent,
                    SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                    SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
                    SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                    SPP_HISTORICAL_EDITED_COLUMN, edited,
                    -1);
        g_free ( str_amount );
        g_free ( str_retained );

        if ( edited )
        {
            gtk_tree_view_collapse_row ( tree_view, gtk_tree_model_get_path ( model, &parent ) );
            gtk_tree_selection_select_iter ( gtk_tree_view_get_selection ( tree_view ), &parent );

        }
    }
}


/**
 * create and fill the list store of the fyear
 * come here mean that fyear_model_filter is NULL
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 * */
gboolean bet_historical_fyear_create_combobox_store ( void )
{
    gchar *titre;

    //~ devel_debug (NULL);
    /* the fyear list store, contains 3 columns :
     * FYEAR_COL_NAME : the name of the fyear
     * FYEAR_COL_NUMBER : the number of the fyear
     * FYEAR_COL_VIEW : it tha fyear should be showed */

    titre = g_strdup ( _("12 months rolling") );
    bet_fyear_model = GTK_TREE_MODEL ( gtk_list_store_new ( 3,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_BOOLEAN ));
    bet_fyear_model_filter = gtk_tree_model_filter_new ( bet_fyear_model, NULL );
    gtk_tree_model_filter_set_visible_column ( GTK_TREE_MODEL_FILTER (
                        bet_fyear_model_filter ),
                        FYEAR_COL_VIEW );
    gsb_fyear_update_fyear_list_new ( bet_fyear_model,
                        bet_fyear_model_filter,
                        titre );
    g_free ( titre );

    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gint bet_historical_get_fyear_from_combobox ( GtkWidget *combo_box )
{
    gint fyear_number = 0;
    GtkTreeIter iter;

    //~ devel_debug (NULL);
    if ( !gtk_combo_box_get_active_iter ( GTK_COMBO_BOX ( combo_box ), &iter ) )
        return 0;

    if ( bet_fyear_model_filter )
    {
        gtk_tree_model_get ( GTK_TREE_MODEL ( bet_fyear_model_filter ),
                        &iter,
                        FYEAR_COL_NUMBER, &fyear_number,
                        -1 );
    }

    return fyear_number;
}


/**
 *
 *
 *
 *
 * */
void bet_historical_refresh_data ( GtkTreeModel *tab_model,
                        GDate *date_min,
                        GDate *date_max )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeIter fils_iter;

    devel_debug (NULL);
    tree_view = g_object_get_data ( G_OBJECT ( bet_container ), "bet_historical_treeview" );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gboolean valeur;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                        -1 );
            if ( valeur == 1 )
            {
                bet_array_list_add_new_line ( tab_model,
                        GTK_TREE_MODEL ( model ), &iter,
                        date_min, date_max );
            }
            else if ( gtk_tree_model_iter_children (
                        GTK_TREE_MODEL ( model ),
                        &fils_iter, &iter ) )
            {
                do
                {
                    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
                                &fils_iter,
                                SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                                -1 );

                    if ( valeur == 1 )
                    {
                        bet_array_list_add_new_line ( tab_model,
                                GTK_TREE_MODEL ( model ), &fils_iter,
                                date_min, date_max );
                    }
                }
	            while ( gtk_tree_model_iter_next (
                        GTK_TREE_MODEL ( model ), &fils_iter ) );
            }

        } while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &iter ) );
    }
}


/**
 *
 *
 *
 *
 * */
gboolean bet_historical_get_full_div ( GtkTreeModel *model, GtkTreeIter *parent )
{
    GtkTreeIter fils_iter;
    gint valeur;

    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &fils_iter, parent ) )
    {
        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &fils_iter,
                        SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                        -1 );

            if ( valeur == FALSE )
            {
                return FALSE;
            }
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &fils_iter ) );
    }
    return TRUE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_historical_set_full_sub_div ( GtkTreeModel *model, GtkTreeIter *parent )
{
    GtkTreeView *tree_view;
    GtkTreeIter fils_iter;
    gint edited;

    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &fils_iter, parent ) )
    {
        gchar *str_amount;
        gchar *str_retained;
        gint account_nb;
        gint div;
        gint sub_div;
        gsb_real retained;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &fils_iter,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div,
                        SPP_HISTORICAL_EDITED_COLUMN, &edited,
                        -1 );

            retained = gsb_real_import_from_string ( str_amount );
            bet_data_add_div_hist ( account_nb, div, sub_div );
            bet_data_set_div_amount ( account_nb, div, sub_div,
                        gsb_real_import_from_string ( str_amount ) );
            str_retained = gsb_real_get_string_with_currency ( retained,
                        gsb_data_account_get_currency ( account_nb ), TRUE );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &fils_iter,
                        SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                        SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        -1 );
            g_free ( str_amount );
            g_free ( str_retained );
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &fils_iter ) );
    }

    if ( edited )
    {
        tree_view = g_object_get_data ( G_OBJECT ( bet_container ),
                        "bet_historical_treeview" );
        gtk_tree_view_collapse_row ( tree_view,
                        gtk_tree_model_get_path ( model, parent ) );
        gtk_tree_selection_select_iter ( gtk_tree_view_get_selection (
                        tree_view ), parent );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_historical_set_empty_sub_div ( GtkTreeModel *model, GtkTreeIter *parent )
{
    GtkTreeIter fils_iter;

    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &fils_iter, parent ) )
    {
        gchar *str_amount;
        gint account_nb;
        gint div;
        gint sub_div;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &fils_iter,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, &div,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div,
                        -1 );
            bet_data_remove_div_hist ( account_nb, div, sub_div );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &fils_iter,
                        SPP_HISTORICAL_SELECT_COLUMN, FALSE,
                        SPP_HISTORICAL_RETAINED_COLUMN, "",
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_EDITED_COLUMN, TRUE,
                        -1 );
            g_free ( str_amount );
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &fils_iter ) );
    }

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gsb_real bet_historical_get_children_amount ( GtkTreeModel *model, GtkTreeIter *parent )
{
    GtkTreeIter fils_iter;
    gchar *str_amount;
    gsb_real amount = null_real;

    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &fils_iter, parent ) )
    {
        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &fils_iter,
                        SPP_HISTORICAL_RETAINED_AMOUNT, &str_amount,
                        -1 );
            amount = gsb_real_add ( amount,
                                gsb_real_get_from_string ( str_amount ) );
        }
        while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &fils_iter ) );
    }

    return amount;
}


/**
 *
 *
 *
 *
 * */
void bet_historical_row_expanded_event ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        GtkTreeModel *model )
{
    gint account_nb;
    gint div_number;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), iter,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        -1 );

    if ( bet_data_get_div_edited ( account_nb, div_number, 0 ) == TRUE )
    {
        gtk_tree_view_collapse_row ( tree_view, path );
        gtk_tree_selection_select_iter ( gtk_tree_view_get_selection ( tree_view ), iter );
    }
}


/**
 *
 *
 *
 *
 * */
gboolean bet_historical_amount_differ_average ( GtkTreeModel *model,
                        GtkTreeIter *iter )
{
    gchar *str_average;
    gchar *str_amount;
    
    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), iter,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, &str_average,
                        SPP_HISTORICAL_RETAINED_AMOUNT, &str_amount,
                        -1 );

    return gsb_real_cmp ( gsb_real_get_from_string ( str_average ),
                        gsb_real_get_from_string ( str_amount ) );
}


/**
 *
 *
 *
 *
 * */
void bet_historical_row_collapse_all ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreeModel *model )
{
    if ( tree_view == NULL )
        tree_view = g_object_get_data ( G_OBJECT ( bet_container ),
                "bet_historical_treeview" );
    gtk_tree_view_collapse_row ( tree_view,
                gtk_tree_model_get_path ( model, iter ) );
    gtk_tree_selection_select_iter ( gtk_tree_view_get_selection (
                tree_view ), iter );
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
