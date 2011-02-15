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
#include "bet_hist.h"
#include "bet_config.h"
#include "bet_data.h"
#include "bet_tab.h"
#include "fenetre_principale.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_fyear.h"
#include "gsb_data_transaction.h"
#include "gsb_fyear.h"
#include "gsb_real.h"
#include "mouse.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void bet_historical_add_average_amount ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static void bet_historical_add_last_amount ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection );
static gboolean bet_historical_affiche_div ( GHashTable  *list_div, GtkWidget *tree_view );
static gboolean bet_historical_amount_differ_average ( GtkTreeModel *model,
                        GtkTreeIter *iter );
static gboolean bet_historical_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev );
static void bet_historical_context_menu ( GtkWidget *tree_view );
static void bet_historical_div_cell_edited (GtkCellRendererText *cell,
                        const gchar *path_string,
                        const gchar *new_text,
                        GtkWidget *tree_view );
static void bet_historical_div_cell_editing_started (GtkCellRenderer *cell,
                        GtkCellEditable *editable,
                        const gchar *path_string,
                        GtkWidget *tree_view );
static gboolean bet_historical_div_toggle_clicked ( GtkCellRendererToggle *renderer,
                        gchar *path_string,
                        GtkTreeModel *model );
static gboolean bet_historical_fyear_create_combobox_store ( void );
static void bet_historical_fyear_hide_present_futures_fyears ( void );
static gsb_real bet_historical_get_children_amount ( GtkTreeModel *model, GtkTreeIter *parent );
static GtkWidget *bet_historical_get_data_tree_view ( GtkWidget *container );
static gboolean bet_historical_get_full_div ( GtkTreeModel *model, GtkTreeIter *parent );
static GDate *bet_historical_get_start_date_current_fyear ( void );
static gint bet_historical_get_type_transaction ( const GDate *date,
                        GDate *start_current_fyear,
                        GDate *date_max );
static gboolean bet_historical_initializes_account_settings ( gint account_number );
static void bet_historical_populate_div_model ( gpointer key,
                        gpointer value,
                        gpointer user_data );
static void bet_historical_row_collapse_all ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreeModel *model );
static void bet_historical_row_collapsed_event ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        GtkTreeModel *model );
static void bet_historical_row_expanded_event ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        GtkTreeModel *model );
static gboolean bet_historical_set_background_color ( GtkWidget *tree_view );
static gboolean bet_historical_set_empty_sub_div ( GtkTreeModel *model, GtkTreeIter *parent );
static gboolean bet_historical_set_full_sub_div ( GtkTreeModel *model, GtkTreeIter *parent );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *account_page;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_selection;
extern GtkWidget *notebook_general;
extern gsb_real null_real;
/*END_EXTERN*/

/* blocage des signaux pour le tree_view pour les comptes de type GSB_TYPE_CASH */
static gboolean hist_block_signal = FALSE;

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
GtkWidget *bet_historical_create_page ( void )
{
    GtkWidget *widget;
    GtkWidget *page;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label;
    GtkWidget *button_1, *button_2;
    GtkWidget *tree_view;
    gpointer pointeur;

    devel_debug (NULL);
    page = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_set_name ( page, "historical_page" );

    /* titre de la page */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5) ;
 
    label = gtk_label_new ( "bet_hist_title" );
    gtk_container_add ( GTK_CONTAINER ( align ), label );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_hist_title", label);

    /* Choix des données sources */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_historical_data", hbox );

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("Categories") );
    gtk_widget_set_name ( button_1, "bet_hist_button_1" );
    g_signal_connect ( G_OBJECT ( button_1 ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_origin_data_clicked ),
                        GINT_TO_POINTER ( 1 ) );

    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("Budgetary lines") );
    g_signal_connect ( G_OBJECT ( button_2 ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_origin_data_clicked ),
                        GINT_TO_POINTER ( 1 ) );

    g_object_set_data ( G_OBJECT ( account_page ), "bet_hist_button_1", button_1 );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_hist_button_2", button_2 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );
    bet_data_set_div_ptr ( 0 );

    /* création du sélecteur de périod */
    if ( bet_historical_fyear_create_combobox_store ( ) )
    {
        widget = gsb_fyear_make_combobox_new ( bet_fyear_model_filter, TRUE );
        gtk_widget_set_name ( GTK_WIDGET ( widget ), "bet_hist_fyear_combo" );
        gtk_widget_set_tooltip_text ( GTK_WIDGET ( widget ),
                        _("Choose the financial year or 12 months rolling") );

        g_object_set_data ( G_OBJECT ( account_page ), "bet_hist_fyear_combo", widget );

        gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5);

        /* hide the present and futures financial years */
        bet_historical_fyear_hide_present_futures_fyears ( );

        /* set the signal */
        pointeur = GINT_TO_POINTER ( 1 );
        g_object_set_data ( G_OBJECT ( widget ), "pointer", pointeur );
        g_signal_connect ( G_OBJECT ( widget ),
                        "changed",
                        G_CALLBACK (bet_config_fyear_clicked),
                        pointeur );
    }

    /* création de la liste des données */
    tree_view = bet_historical_get_data_tree_view ( page );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_historical_treeview", tree_view );

    /* set the color of selected row */
    gtk_widget_modify_base ( tree_view, GTK_STATE_SELECTED, &couleur_selection );
    gtk_widget_modify_base ( tree_view, GTK_STATE_ACTIVE, &couleur_selection );

    gtk_widget_show_all ( page );

    return page;
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

    /* devel_debug (path_string); */
    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( model ), &iter, path_string ) )
    {
        gchar *str_amount;
        gchar *str_average;
        gint div_number;
        gint sub_div_nb;
        gint nbre_fils;
        gint account_number;
        gboolean valeur;
        gboolean col_edited;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                        SPP_HISTORICAL_AVERAGE_COLUMN, &str_average,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
                        SPP_HISTORICAL_EDITED_COLUMN, &col_edited,
                        -1 );

        if ( col_edited == FALSE && valeur == FALSE )
            return FALSE;

        if ( renderer )
            valeur = 1 - valeur;

        if ( valeur == 1 )
        {
            /* printf ("avant - account_number = %d, div = %d, sub_div = %d\n",
                        account_number, div, sub_div); */
            bet_data_hist_add_div ( account_number, div_number, sub_div_nb );
            bet_data_set_div_amount ( account_number, div_number, sub_div_nb,
                        gsb_real_safe_real_from_string ( str_amount ) );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, 1,
                        SPP_HISTORICAL_RETAINED_COLUMN, str_average,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_number,
                        SPP_HISTORICAL_DIV_NUMBER, div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div_nb,
                        -1 );
        }
        else
        {
            bet_data_remove_div_hist ( account_number, div_number, sub_div_nb );
            gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, FALSE,
                        SPP_HISTORICAL_RETAINED_COLUMN, "",
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_number,
                        SPP_HISTORICAL_DIV_NUMBER, div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div_nb,
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
                    str_amount = gsb_real_safe_real_to_string ( amount,
                                gsb_data_account_get_currency_floating_point ( account_number ) );
                    if ( str_average )
                        g_free ( str_average );
                    str_average = gsb_real_get_string_with_currency ( amount,
                                gsb_data_account_get_currency ( account_number ), TRUE );
                    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                                &parent,
                                SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                                SPP_HISTORICAL_RETAINED_COLUMN, str_average,
                                SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                                -1);
                    if ( bet_historical_amount_differ_average ( model, &parent) )
                        bet_data_set_div_amount ( account_number, div_number, 0, amount );
                        
                    bet_historical_row_collapse_all ( NULL, &parent, model );
                }
                else
                {
                    bet_data_set_div_amount ( account_number, div_number, 0, null_real );
                    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &parent,
                                SPP_HISTORICAL_SELECT_COLUMN, FALSE,
                                SPP_HISTORICAL_RETAINED_COLUMN, "",
                                SPP_HISTORICAL_EDITED_COLUMN, TRUE,
                                -1 );
                }
            }
        }

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
        bet_data_update_bet_module ( account_number, GSB_HISTORICAL_PAGE );
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

    /* devel_debug (new_text); */
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    if ( gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( model ), &iter, path_string ) )
    {
        GtkTreeIter parent;
        gboolean valeur;
        gboolean edited;
        gint account_number;
        gint currency_number;
        gint div_number;
        gint sub_div_nb;
        gchar *tmp_str;
        gchar *str_amount;
        gboolean is_parent = FALSE;
        gsb_real number;

        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
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
        currency_number = gsb_data_account_get_currency ( account_number );
        tmp_str = gsb_real_get_string_with_currency ( number, currency_number, TRUE );
        str_amount = gsb_real_safe_real_to_string ( number,
                                gsb_data_currency_get_floating_point ( currency_number ) );

        if ( bet_data_search_div_hist ( account_number, div_number, sub_div_nb ) == FALSE )
            bet_data_hist_add_div ( account_number, div_number, sub_div_nb );

        bet_data_set_div_edited  ( account_number, div_number, sub_div_nb, TRUE );
        bet_data_set_div_amount ( account_number, div_number, sub_div_nb, number );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                            SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                            SPP_HISTORICAL_RETAINED_COLUMN, tmp_str,
                            SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                            -1 );
        g_free ( str_amount );
        g_free ( tmp_str );

        if ( is_parent )
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_EDITED_COLUMN, FALSE,
                        -1);

        if ( is_parent && bet_historical_get_full_div ( model, &parent ) )
        {
            number = bet_historical_get_children_amount ( model, &parent );
            bet_data_set_div_amount ( account_number, div_number, 0, number );
            str_amount = gsb_real_safe_real_to_string ( number,
                                gsb_data_currency_get_floating_point ( currency_number ) );
            tmp_str = gsb_real_get_string_with_currency ( number, currency_number, TRUE );
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

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );

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
GtkWidget *bet_historical_get_data_tree_view ( GtkWidget *container )
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GtkTreeStore *tree_model;
    GtkTreeSelection *tree_selection;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    gchar *title;

    /* devel_debug (NULL); */
    tree_view = gtk_tree_view_new ( );
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW (tree_view), FALSE );
    g_object_set_data ( G_OBJECT ( account_page ), "hist_tree_view", tree_view );

    tree_model = gtk_tree_store_new ( SPP_HISTORICAL_NUM_COLUMNS,
                        G_TYPE_BOOLEAN,     /* SPP_HISTORICAL_SELECT_COLUMN     */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_DESC_COLUMN       */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_CURRENT_COLUMN     */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_BALANCE_COLUMN    */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_AVERAGE_COLUMN    */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_AVERAGE_AMOUNT    */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_RETAINED_COLUMN   */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_RETAINED_AMOUNT   */
                        G_TYPE_STRING,      /* SPP_HISTORICAL_BALANCE_COLOR     */
                        G_TYPE_INT,         /* SPP_HISTORICAL_ACCOUNT_NUMBER    */
                        G_TYPE_INT,         /* SPP_HISTORICAL_DIV_NUMBER        */
                        G_TYPE_INT,         /* SPP_HISTORICAL_SUB_DIV_NUMBER    */
                        G_TYPE_BOOLEAN,     /* SPP_HISTORICAL_EDITED_COLUMN     */
                        GDK_TYPE_COLOR );   /* SPP_HISTORICAL_BACKGROUND_COLOR  */
    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL ( tree_model ) );
    g_object_unref ( G_OBJECT ( tree_model ) );

    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( tree_model ),
					    SPP_HISTORICAL_DESC_COLUMN, GTK_SORT_ASCENDING );

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_selection_set_mode ( tree_selection, GTK_SELECTION_SINGLE );

    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_widget_set_size_request ( scrolled_window, -1, 250 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), tree_view );
    gtk_box_pack_start ( GTK_BOX ( container ), scrolled_window, TRUE, TRUE, 15 );

    /* create columns */
    cell = gtk_cell_renderer_toggle_new ( );
    g_object_set_data ( G_OBJECT ( account_page ), "toggle_cell", cell );
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
                        "cell-background-gdk", SPP_HISTORICAL_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), column );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* name of the div sous-div column*/
    title = g_strdup ( _("Category") );
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( title,
                        cell,
                        "text", SPP_HISTORICAL_DESC_COLUMN,
                        "cell-background-gdk", SPP_HISTORICAL_BACKGROUND_COLOR,
                        NULL);
    g_object_set_data ( G_OBJECT ( account_page ), "historical_column_source",
                        column );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* amount column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Amount"), cell,
                        "text", SPP_HISTORICAL_BALANCE_COLUMN,
                        "foreground", SPP_HISTORICAL_BALANCE_COLOR,
                        "cell-background-gdk", SPP_HISTORICAL_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( column ), FALSE );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* average column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Average"), cell,
                        "text", SPP_HISTORICAL_AVERAGE_COLUMN,
                        "foreground", SPP_HISTORICAL_BALANCE_COLOR,
                        "cell-background-gdk", SPP_HISTORICAL_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( column ), FALSE );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* current fyear column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Current fyear"), cell,
                        "text", SPP_HISTORICAL_CURRENT_COLUMN,
                        "foreground", SPP_HISTORICAL_BALANCE_COLOR,
                        "cell-background-gdk", SPP_HISTORICAL_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( column ), FALSE );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    /* amount retained column */
    cell = gtk_cell_renderer_text_new ( );
    g_object_set_data ( G_OBJECT ( account_page ), "edited_cell", cell );
    column = gtk_tree_view_column_new_with_attributes (
                        _("Amount retained"), cell,
                        "text", SPP_HISTORICAL_RETAINED_COLUMN,
                        "foreground", SPP_HISTORICAL_BALANCE_COLOR,
                        "editable", SPP_HISTORICAL_EDITED_COLUMN,
                        "cell-background-gdk", SPP_HISTORICAL_BACKGROUND_COLOR,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ),
                        GTK_TREE_VIEW_COLUMN ( column ) );
    g_object_set ( G_OBJECT ( GTK_CELL_RENDERER ( cell ) ), "xalign", 1.0, NULL );
    gtk_tree_view_column_set_alignment ( column, 1 );
    gtk_tree_view_column_set_clickable ( GTK_TREE_VIEW_COLUMN ( column ), FALSE );
    gtk_tree_view_column_set_expand ( GTK_TREE_VIEW_COLUMN ( column ), TRUE );
    gtk_tree_view_column_set_resizable ( column, TRUE );

    g_signal_connect ( cell,
                        "editing-started",
                        G_CALLBACK (bet_historical_div_cell_editing_started),
                        tree_view );
    g_signal_connect ( cell,
                        "edited",
                        G_CALLBACK (bet_historical_div_cell_edited),
                        tree_view );

    g_signal_connect ( G_OBJECT ( tree_view ),
                        "row-collapsed",
                        G_CALLBACK ( bet_historical_row_collapsed_event ),
                        tree_model );

    g_signal_connect ( G_OBJECT ( tree_view ),
                        "row-expanded",
                        G_CALLBACK ( bet_historical_row_expanded_event ),
                        tree_model );

    g_signal_connect ( G_OBJECT ( tree_view ),
		                "button-press-event",
		                G_CALLBACK ( bet_historical_button_press ),
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
void bet_historical_populate_data ( gint account_number )
{
    GtkWidget *tree_view;
    GtkTreeModel *model;
    gint fyear_number;
    GDate *date_jour;
    GDate *date_min;
    GDate *date_max;
    GDate *start_current_fyear;
    GSList* tmp_list;
    GHashTable  *list_div;

    devel_debug_int ( account_number );
    tree_view = g_object_get_data (G_OBJECT ( account_page ), "bet_historical_treeview" );
    if ( GTK_IS_TREE_VIEW ( tree_view ) == FALSE )
        return;

    /* Initializes account settings */
    bet_historical_initializes_account_settings ( account_number );

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

    /* calculate date_jour, date_min and date_max */
    date_jour = gdate_today ( );

    fyear_number = gsb_data_account_get_bet_hist_fyear ( account_number );
    if ( fyear_number == 0 )
    {
        date_min = gdate_today ( );
        g_date_subtract_years ( date_min, 1 );
        date_max = gdate_today ( );
        g_date_subtract_days ( date_max, 1 );
    }
    else
    {
        date_min = gsb_date_copy ( gsb_data_fyear_get_beginning_date ( fyear_number ) );
        date_max = gsb_date_copy ( gsb_data_fyear_get_end_date ( fyear_number ) );
    }

    /* calculate the current_fyear */
    start_current_fyear = bet_historical_get_start_date_current_fyear ( );
/*     printf ("start_current_fyear = %s\n", gsb_format_gdate ( start_current_fyear ));  */

    list_div = g_hash_table_new_full ( g_str_hash,
                        g_str_equal,
                        (GDestroyNotify) g_free,
                        (GDestroyNotify) struct_free_bet_historical );

    /* search transactions of the account  */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ( );
    while ( tmp_list )
    {
        gint transaction_number;
        gint tmp_account_number;
        const GDate *date;
        gint type_de_transaction;

        transaction_number = gsb_data_transaction_get_transaction_number ( tmp_list->data );
        tmp_list = tmp_list -> next;

        tmp_account_number =  gsb_data_transaction_get_account_number ( transaction_number );
        if ( tmp_account_number != account_number )
            continue;

        date = gsb_data_transaction_get_date ( transaction_number );
        /* ignore transaction which are before date_min */
        if ( g_date_compare ( date, date_min ) < 0 )
            continue;

        /* ignore transaction which is after date_jour */
        if ( g_date_compare ( date, date_jour ) > 0 )
            continue;

        /* ignore splitted transactions */
        if ( gsb_data_transaction_get_split_of_transaction (
         transaction_number ) == TRUE )
            continue;

        /* on détermine le type de transaction pour l'affichage */
        type_de_transaction = bet_historical_get_type_transaction ( date, start_current_fyear, date_max );

        bet_data_populate_div ( transaction_number, TRUE, list_div, type_de_transaction );
    }

    bet_historical_affiche_div ( list_div, tree_view );
    /* bet_data_synchronise_hist_div_list ( list_div ); */

    g_hash_table_remove_all ( list_div );
    g_date_free ( date_jour );
    g_date_free ( date_min );
    g_date_free ( date_max );
    g_date_free ( start_current_fyear );

    bet_historical_set_background_color ( tree_view );
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

    /* devel_debug (NULL); */
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
    gchar *str_current_fyear;
    gint div_number;
    gint account_nb;
    gint currency_number;
    gint nbre_sub_div = 0;
    gboolean sub_div_visible = FALSE;
    gboolean edited = TRUE;
    gsb_real period = { 12, 0 };
    gsb_real average;
    gsb_real retained;
    gsb_real amount;
    kind_account kind;

    div_number = sh -> div;
    div_name = bet_data_get_div_name ( div_number, 0, NULL );
    account_nb = sh -> account_nb;
    kind = gsb_data_account_get_kind ( account_nb );
    if ( kind == GSB_TYPE_CASH )
        edited = FALSE;

    currency_number = gsb_data_account_get_currency ( account_nb );

    model = gtk_tree_view_get_model ( tree_view );

    str_balance = gsb_real_get_string_with_currency ( sbr -> current_balance, currency_number, TRUE );
    average = gsb_real_div ( sbr -> current_balance, period );
    str_amount = gsb_real_safe_real_to_string ( average,
                        gsb_data_currency_get_floating_point ( currency_number ) );
    str_average = gsb_real_get_string_with_currency ( average, currency_number, TRUE );

    str_current_fyear = gsb_real_get_string_with_currency ( sbr -> current_fyear, currency_number, TRUE );

    gtk_tree_store_append ( GTK_TREE_STORE ( model ), &parent, NULL);
    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_DESC_COLUMN, div_name,
                        SPP_HISTORICAL_CURRENT_COLUMN, str_current_fyear,
                        SPP_HISTORICAL_BALANCE_COLUMN, str_balance,
                        SPP_HISTORICAL_AVERAGE_COLUMN, str_average,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, str_amount,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, 0,
                        SPP_HISTORICAL_EDITED_COLUMN, edited,
                        -1);

    if ( bet_data_search_div_hist ( account_nb, div_number, 0 ) 
     &&
     ( bet_data_get_div_edited ( account_nb, div_number, 0 )
      ||
      g_hash_table_size ( sh -> list_sub_div ) < 1 ) )
    {
        retained = bet_data_hist_get_div_amount ( account_nb, div_number, 0 );
        if ( str_amount )
            g_free ( str_amount );
        str_amount = gsb_real_safe_real_to_string ( retained,
                        gsb_data_currency_get_floating_point ( currency_number ) );
        str_retained = gsb_real_get_string_with_currency ( retained, currency_number, TRUE );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                        SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        -1);
        g_free ( str_retained );
    }
/* printf ("division = %d div_name = %s\n", div_number, div_name);  */
    g_free ( div_name );
    g_free ( str_average );
    g_free ( str_amount );
    g_free ( str_current_fyear );

    if ( ( nbre_sub_div = g_hash_table_size ( sh -> list_sub_div ) ) < 1 )
        return;

    g_hash_table_iter_init ( &iter, sh -> list_sub_div );
    while ( g_hash_table_iter_next ( &iter, &sub_key, &sub_value ) ) 
    {
        SH *sub_sh = ( SH* ) sub_value;
        SBR *sub_sbr = sub_sh -> sbr;
        GtkTreeIter fils;
        gchar **tab_str = NULL;

        if ( nbre_sub_div == 1 && sub_sh -> div == 0 )
            return;

        div_name = bet_data_get_div_name ( div_number, sub_sh -> div, NULL );
/*         printf ("division = %d sub_div = %d div_name = %s\n", div_number, sub_sh -> div, div_name);  */
        if ( div_name && g_utf8_strrchr ( div_name, -1, ':' ) )
        {
	        tab_str = g_strsplit ( div_name, ":", 2 );
            if ( g_strv_length ( tab_str ) > 1 )
            {
                g_free ( div_name );
                div_name = g_strdup ( g_strstrip ( tab_str[1] ) );
            }
            if ( tab_str )
                g_strfreev ( tab_str );
        }

        str_balance = gsb_real_get_string_with_currency ( sub_sbr -> current_balance, 
                        currency_number, TRUE );
        average = gsb_real_div ( sub_sbr -> current_balance, period );
        str_amount = gsb_real_safe_real_to_string ( average,
                        gsb_data_currency_get_floating_point ( currency_number ) );
        str_average = gsb_real_get_string_with_currency ( average,
                        currency_number, TRUE );
        str_current_fyear = gsb_real_get_string_with_currency ( sub_sbr -> current_fyear,
                        currency_number, TRUE );

        gtk_tree_store_append ( GTK_TREE_STORE ( model ), &fils, &parent );
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &fils,
                        SPP_HISTORICAL_DESC_COLUMN, div_name,
                        SPP_HISTORICAL_CURRENT_COLUMN, str_current_fyear,
                        SPP_HISTORICAL_BALANCE_COLUMN, str_balance,
                        SPP_HISTORICAL_AVERAGE_COLUMN, str_average,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, str_amount,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, sub_sh -> div,
                        SPP_HISTORICAL_EDITED_COLUMN, edited,
                        -1);

        if ( bet_data_search_div_hist ( account_nb, div_number, sub_sh -> div ) )
        {
            GtkTreePath *path;

            if ( bet_data_get_div_edited ( account_nb, div_number, sub_sh -> div ) )
            {
                /* printf ("account_nb = %d div_number = %d sub_sh -> div = %d\n", account_nb, div_number, sub_sh -> div ); */
                retained = bet_data_hist_get_div_amount ( account_nb, div_number, sub_sh -> div );
                if ( str_amount )
                    g_free ( str_amount );
                str_amount = gsb_real_safe_real_to_string ( retained,
                        gsb_data_currency_get_floating_point ( currency_number ) );
                str_retained = gsb_real_get_string_with_currency ( retained, currency_number, TRUE );

                edited = FALSE;
                gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_EDITED_COLUMN, edited,
                        -1);
            }
            else
            {
                bet_data_set_div_amount ( account_nb, div_number, sub_sh -> div, average );
                str_retained = g_strdup ( str_average );
                edited = TRUE;
            }
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &fils,
                        SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                        SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        -1);
            sub_div_visible = TRUE;
            path = gtk_tree_model_get_path ( model, &fils );
            gtk_tree_view_expand_to_path ( tree_view, path );

            g_free ( str_retained );
            gtk_tree_path_free ( path );
        }
        g_free ( div_name );
        g_free ( str_balance );
        g_free ( str_average );
        g_free ( str_amount );
        g_free ( str_current_fyear );
    }

    if ( sub_div_visible && bet_historical_get_full_div ( model, &parent ) )
    {
        amount = bet_historical_get_children_amount ( model, &parent );
        str_amount = gsb_real_safe_real_to_string ( amount,
                    gsb_data_currency_get_floating_point ( currency_number ) );
        str_retained = gsb_real_get_string_with_currency ( amount, currency_number, TRUE );

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
            GtkTreePath *path;

            path = gtk_tree_model_get_path ( model, &parent );
            gtk_tree_view_collapse_row ( tree_view, path );
            gtk_tree_selection_select_iter ( gtk_tree_view_get_selection ( tree_view ), &parent );

            gtk_tree_path_free ( path );
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

    /* devel_debug (NULL); */
    /* the fyear list store, contains 3 columns :
     * FYEAR_COL_NAME : the name of the fyear
     * FYEAR_COL_NUMBER : the number of the fyear
     * FYEAR_COL_VIEW : it the fyear should be showed */

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

    /* devel_debug (NULL); */
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
void bet_historical_set_fyear_from_combobox ( GtkWidget *combo_box, gint fyear )
{
    gsb_fyear_select_iter_by_number ( combo_box,
                    bet_fyear_model,
                    bet_fyear_model_filter,
                    fyear );
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
    tree_view = g_object_get_data ( G_OBJECT ( account_page ), "bet_historical_treeview" );
    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gboolean valeur;
        gint account_number;

        /* test du numero de compte */
        gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        -1 );
        if ( account_number != gsb_gui_navigation_get_current_account ( ) )
            return;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, &valeur,
                        -1 );
            if ( valeur == 1 )
            {
                bet_array_list_add_new_hist_line ( tab_model,
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
                        bet_array_list_add_new_hist_line ( tab_model,
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
        gint div_number;
        gint sub_div_nb;
        gsb_real retained;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &fils_iter,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
                        SPP_HISTORICAL_EDITED_COLUMN, &edited,
                        -1 );

            retained = gsb_real_safe_real_from_string ( str_amount );
            bet_data_hist_add_div ( account_nb, div_number, sub_div_nb );
            bet_data_set_div_amount ( account_nb, div_number, sub_div_nb,
                        gsb_real_safe_real_from_string ( str_amount ) );
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
        tree_view = g_object_get_data ( G_OBJECT ( account_page ),
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
        gint div_number;
        gint sub_div_nb;

        do
        {
            gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &fils_iter,
                        SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
                        -1 );
            bet_data_remove_div_hist ( account_nb, div_number, sub_div_nb );
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
                                gsb_real_safe_real_from_string ( str_amount ) );
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
void bet_historical_row_collapsed_event ( GtkTreeView *tree_view,
                        GtkTreeIter *iter,
                        GtkTreePath *path,
                        GtkTreeModel *model )
{
    bet_historical_set_background_color ( GTK_WIDGET ( tree_view ) );
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
    bet_historical_set_background_color ( GTK_WIDGET ( tree_view ) );
}


/**
 * Cette fonction compare le montant de la colonne montant retenu avec le montant 
 * de la colonne moyenne
 *
 * \ return O si egal -1 ou 1 si différent
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

    return gsb_real_cmp ( gsb_real_safe_real_from_string ( str_average ),
                        gsb_real_safe_real_from_string ( str_amount ) );
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
        tree_view = g_object_get_data ( G_OBJECT ( account_page ), "bet_historical_treeview" );
    gtk_tree_view_collapse_row ( tree_view,
                gtk_tree_model_get_path ( model, iter ) );
    gtk_tree_selection_select_iter ( gtk_tree_view_get_selection (
                tree_view ), iter );
}


/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 * */
gboolean bet_historical_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev )
{
	/* show the popup */
	if ( ev -> button == RIGHT_BUTTON )
        bet_historical_context_menu ( tree_view );

    return FALSE;
}


/**
 * Pop up a menu with several actions to apply to historical list.
 *
 * \param
 *
 */
void bet_historical_context_menu ( GtkWidget *tree_view )
{
    GtkWidget *menu, *menu_item;
    GtkTreeModel *model;
    GtkTreeSelection *tree_selection;
    GtkTreeIter iter;

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    menu = gtk_menu_new ();

    /* Add last amount menu */
    menu_item = gtk_image_menu_item_new_with_label ( _("Assign the amount of the last operation") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_ADD,
                        GTK_ICON_SIZE_MENU ) );

    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_historical_add_last_amount ),
                        tree_selection );

    if ( gtk_tree_model_iter_n_children ( model, &iter ) > 0 )
        gtk_widget_set_sensitive ( menu_item, FALSE );
    else
        gtk_widget_set_sensitive ( menu_item, TRUE );

    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Separator */
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new ( ) );
    gtk_widget_show ( menu_item );


    /* Add average amount menu */
    menu_item = gtk_image_menu_item_new_with_label ( _("Copy the average amount") );
    gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_COPY,
                        GTK_ICON_SIZE_MENU ) );

    g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( bet_historical_add_average_amount ),
                        tree_selection );

    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

    /* Finish all. */
    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU( menu ), NULL, NULL, NULL, NULL,
                        3, gtk_get_current_event_time ( ) );
}


/**
 * add the amount of the last transaction with the same division and sub-division
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_historical_add_last_amount ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter parent;
    GtkTreeIter iter;
    gint account_number;
    gint div_number;
    gint sub_div_nb;
    gint currency_number;
    gchar *tmp_str;
    gchar *str_amount;
    gsb_real amount;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    gtk_tree_model_get ( GTK_TREE_MODEL ( model ), &iter,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
                        SPP_HISTORICAL_DIV_NUMBER, &div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
                        -1 );

    amount = gsb_data_transaction_get_last_transaction_with_div_sub_div (
                        account_number,
                        div_number,
                        sub_div_nb,
                        gsb_data_account_get_bet_hist_data ( account_number ) );

    currency_number = gsb_data_account_get_currency ( account_number );
    str_amount = gsb_real_safe_real_to_string ( amount, 
                    gsb_data_currency_get_floating_point ( currency_number ) );

    tmp_str = gsb_real_get_string_with_currency ( amount, currency_number, TRUE );
    /* printf ("div = %d sub_div_nb = %d tmp_str = %s\n", div_number, sub_div_nb, tmp_str); */
    if ( bet_data_search_div_hist ( account_number, div_number, sub_div_nb ) == FALSE )
        bet_data_hist_add_div ( account_number, div_number, sub_div_nb );

    bet_data_set_div_edited  ( account_number, div_number, sub_div_nb, TRUE );
    bet_data_set_div_amount ( account_number, div_number, sub_div_nb, amount );
    gtk_tree_store_set ( GTK_TREE_STORE ( model ), &iter,
                        SPP_HISTORICAL_SELECT_COLUMN, TRUE,
                        SPP_HISTORICAL_RETAINED_COLUMN, tmp_str,
                        SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
                        SPP_HISTORICAL_ACCOUNT_NUMBER, account_number,
                        SPP_HISTORICAL_DIV_NUMBER, div_number,
                        SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div_nb,
                        -1 );

    if ( gtk_tree_model_iter_parent ( GTK_TREE_MODEL ( model ), &parent, &iter ) )
        gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &parent,
                        SPP_HISTORICAL_EDITED_COLUMN, FALSE,
                        -1);

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/**
 * add the average amount of the item
 *
 * /param menu item
 * /param row selected
 *
 * */
void bet_historical_add_average_amount ( GtkWidget *menu_item,
                        GtkTreeSelection *tree_selection )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *path_string;

    if ( !gtk_tree_selection_get_selected ( GTK_TREE_SELECTION ( tree_selection ),
     &model, &iter ) )
        return;

    path_string = gtk_tree_path_to_string ( gtk_tree_model_get_path ( model, &iter ) );
    if ( path_string )
        bet_historical_div_toggle_clicked ( NULL, path_string, model );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_historical_initializes_account_settings ( gint account_number )
{
    GtkWidget *button = NULL;
    GtkWidget *combo = NULL;
    GtkTreeViewColumn *column;
    gchar *title;
    gint fyear_number;
    gint origin;
    gpointer pointeur;

    /* set data origin */
    origin = gsb_data_account_get_bet_hist_data ( account_number );
    if ( origin )
    {
        button = g_object_get_data ( G_OBJECT ( account_page ), "bet_hist_button_2" );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
        bet_data_set_div_ptr ( 1 );
        title = g_strdup ( _("Budgetary line") );
    }
    else
    {
        button = g_object_get_data ( G_OBJECT ( account_page ), "bet_hist_button_1" );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
        bet_data_set_div_ptr ( 0 );
        title = g_strdup ( _("Category") );
    }

    column = g_object_get_data ( G_OBJECT ( account_page ),
                        "historical_column_source" );
    gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN ( column ), title );
    g_free ( title );

    /* set fyear */
    combo = g_object_get_data ( G_OBJECT ( account_page ), "bet_hist_fyear_combo" );
    fyear_number = gsb_data_account_get_bet_hist_fyear ( account_number );

    pointeur = g_object_get_data ( G_OBJECT ( combo ), "pointer" );
    g_signal_handlers_block_by_func ( G_OBJECT ( combo ),
                        G_CALLBACK ( bet_config_fyear_clicked ),
                        pointeur );

    gsb_fyear_select_iter_by_number ( combo,
                        bet_fyear_model,
                        bet_fyear_model_filter,
                        fyear_number );

    g_signal_handlers_unblock_by_func ( G_OBJECT ( combo ),
                        G_CALLBACK ( bet_config_fyear_clicked ),
                        pointeur );

    return FALSE;
}


/**
 * set the background colors of the list
 *
 * \param tree_view
 *
 * \return FALSE
 * */
gboolean bet_historical_set_background_color ( GtkWidget *tree_view )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeIter fils_iter;

    if ( !tree_view )
	    return FALSE;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ), &iter ) )
    {
        gint current_color = 0;

        do
        {
            gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &iter,
                        SPP_HISTORICAL_BACKGROUND_COLOR, &couleur_fond[current_color],
                        -1 );
            current_color = !current_color;

            if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL ( model ), &fils_iter, &iter )
             &&
              gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( tree_view ),
              gtk_tree_model_get_path ( GTK_TREE_MODEL ( model ), &iter ) ) )
            {
                do
                {
                    gtk_tree_store_set ( GTK_TREE_STORE ( model ),
                        &fils_iter,
                        SPP_HISTORICAL_BACKGROUND_COLOR, &couleur_fond[current_color],
                        -1 );
                    current_color = !current_color;
                }
                while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ), &fils_iter ) );
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
void bet_historical_g_signal_block_tree_view ( void )
{
    GtkTreeView *tree_view;
    gpointer cell;
    GtkTreeModel *tree_model;

    hist_block_signal = TRUE;
    tree_view = g_object_get_data ( G_OBJECT ( account_page ), "hist_tree_view" );
    tree_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    cell = g_object_get_data ( G_OBJECT ( account_page ), "toggle_cell" );
    g_signal_handlers_block_by_func ( cell, bet_historical_div_toggle_clicked, tree_model );

    cell = g_object_get_data ( G_OBJECT ( account_page ), "edited_cell" );
    g_signal_handlers_block_by_func ( tree_view, bet_historical_button_press, NULL );
}


/**
 *
 *
 *
 *
 * */
void bet_historical_g_signal_unblock_tree_view ( void )
{
    GtkTreeView *tree_view;
    gpointer cell;
    GtkTreeModel *tree_model;

    if ( hist_block_signal == FALSE )
        return;

    hist_block_signal = FALSE;
    tree_view = g_object_get_data ( G_OBJECT ( account_page ), "hist_tree_view" );
    tree_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );
    cell = g_object_get_data ( G_OBJECT ( account_page ), "toggle_cell" );

    g_signal_handlers_unblock_by_func ( cell, bet_historical_div_toggle_clicked, tree_model );

    cell = g_object_get_data ( G_OBJECT ( account_page ), "edited_cell" );
    g_signal_handlers_unblock_by_func ( tree_view, bet_historical_button_press, NULL );
}


/**
 *
 *
 *
 *
 * */
void bet_historical_set_page_title ( gint account_number )
{
    GtkWidget * widget;
    gchar *title;

    title = g_strdup_printf (
                    _("Please select the data source for the account: \"%s\""),
                    gsb_data_account_get_name ( account_number ) );
    widget = GTK_WIDGET ( g_object_get_data ( G_OBJECT ( account_page ), "bet_hist_title") );
    gtk_label_set_label ( GTK_LABEL ( widget ), title );
    g_free ( title );
}


/**
 * retourne la date de début de l'exercice en cours ou la date de l'année en cours.
 *
 *
 * \return date de début de la période courante
 * */
GDate *bet_historical_get_start_date_current_fyear ( void )
{
    GDate *date = NULL;
    gint fyear_number = 0;

    date = gdate_today ( );
    fyear_number = gsb_data_fyear_get_from_date ( date );

    if ( fyear_number <= 0 )
    {
        g_date_set_month ( date, 1 );
        g_date_set_day ( date, 1 );

        return date;
    }
    else
    {
        g_date_free ( date );
        date = gsb_data_fyear_get_beginning_date ( fyear_number );

        return gsb_date_copy ( date );
    }
}


/**
 * \return 0    opération <= à date_max et < start_current_fyear (n'appartient pas à l'exercice en cours)
 * \return 1    opération > à date_max et > start_current_fyear (appartient à l'exercice en cours)
 * \return 2    opération <= à date_max et > start_current_fyear (appartient à l'exercice en cours)
 * \return -1   toutes les autres opérations 
 * */
gint bet_historical_get_type_transaction ( const GDate *date,
                        GDate *start_current_fyear,
                        GDate *date_max )
{
    gint result = -1;

    if ( g_date_compare ( date, date_max ) <= 0 )
    {
        if ( g_date_compare ( date, start_current_fyear ) >= 0 )
            result = 2;
        else
            result = 0;
    }
    else
    {
        if ( g_date_compare ( date, start_current_fyear ) >= 0 )
            result = 1;
    }

    return result;
}


/**
 * cache l'exercice courant et les exercices futurs
 *
 *
 *
 * */
void bet_historical_fyear_hide_present_futures_fyears ( void )
{
    GDate *date;
    GSList *tmp_list;

    date = gdate_today ( );

    tmp_list = gsb_data_fyear_get_fyears_list ( );
    while (tmp_list)
    {
        struct_fyear *fyear;

        fyear = tmp_list -> data;

        /* check the fyear only if the dates are valid */
        if ( fyear -> beginning_date && fyear -> end_date )
        {
            if ( g_date_compare ( date, fyear -> beginning_date ) >= 0
             &&
             g_date_compare ( date, fyear -> end_date ) <= 0 )
            {
                gsb_fyear_hide_iter_by_name ( bet_fyear_model, fyear -> fyear_name );
            }
            else if ( g_date_compare ( date, fyear -> beginning_date ) <= 0 )
            {
                gsb_fyear_hide_iter_by_name ( bet_fyear_model, fyear -> fyear_name );
            }
        }
        tmp_list = tmp_list -> next;
    }

}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
