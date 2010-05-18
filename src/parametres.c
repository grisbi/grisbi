/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
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
 * \file parametres.c
 * we find here the configuration dialog
 */

#include "include.h"


/*START_INCLUDE*/
#include "parametres.h"
#include "./menu.h"
#include "./utils.h"
#include "./balance_estimate_config.h"
#include "./dialog.h"
#include "./gsb_archive_config.h"
#include "./gsb_automem.h"
#include "./gsb_bank.h"
#include "./gsb_currency_config.h"
#include "./gsb_currency_link_config.h"
#include "./gsb_data_account.h"
#include "./gsb_file.h"
#include "./gsb_form_config.h"
#include "./gsb_fyear_config.h"
#include "./navigation.h"
#include "./import.h"
#include "./gsb_payment_method_config.h"
#include "./gsb_reconcile_config.h"
#include "./gsb_reconcile_sort_config.h"
#include "./traitement_variables.h"
#include "./utils_files.h"
#include "./accueil.h"
#include "./affichage_liste.h"
#include "./affichage.h"
#include "./tiers_onglet.h"
#include "./categories_onglet.h"
#include "./imputation_budgetaire.h"
#include "./structures.h"
#include "./fenetre_principale.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget * create_preferences_tree ( );
static GtkWidget *gsb_config_scheduler_page ( void );
static gboolean gsb_config_scheduler_switch_balances_with_scheduled ( void );
static gboolean gsb_gui_delete_msg_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
                        GtkTreeModel * model );
static gboolean gsb_gui_messages_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
                        GtkTreeModel * model );
static GtkWidget *onglet_delete_messages ( void );
static GtkWidget *onglet_fichier ( void );
static GtkWidget *onglet_messages_and_warnings ( void );
static GtkWidget *onglet_metatree ( void );
static GtkWidget *onglet_programmes (void);
static gboolean preference_selectable_func (GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        gpointer data);
static gboolean selectionne_liste_preference ( GtkTreeSelection *selection,
                        GtkTreeModel *model );
/*END_STATIC*/


GtkWidget *fenetre_preferences = NULL;

static GtkTreeStore *preference_tree_model = NULL;
static GtkNotebook * preference_frame = NULL;

static gint width_spin_button = 50;


/*START_EXTERN*/
extern gboolean balances_with_scheduled;
extern struct conditional_message delete_msg[];
extern gboolean execute_scheduled_of_month;
extern struct conditional_message messages[];
extern gint nb_days_before_scheduled;
extern gint nb_max_derniers_fichiers_ouverts;
extern gchar *titre_fichier;
extern GtkWidget *window;
/*END_EXTERN*/


/**
 * Creates a simple TreeView and a TreeModel to handle preference
 * tabs.  Sets preference_tree_model to the newly created TreeModel.
 *
 * \return a GtkScrolledWindow
 */
GtkWidget * create_preferences_tree ( )
{
    GtkWidget *sw;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection * selection;
    GtkWidget *tree_view;

    /* Create model */
    preference_tree_model = gtk_tree_store_new (3,
                        G_TYPE_STRING,
                        G_TYPE_INT,
                        G_TYPE_INT );

    /* Create container + TreeView */
    sw = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW (sw),
                        GTK_SHADOW_IN );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_NEVER,
                        GTK_POLICY_AUTOMATIC );
    tree_view = gtk_tree_view_new();
    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view),
                        GTK_TREE_MODEL ( preference_tree_model ) );
    g_object_unref ( G_OBJECT(preference_tree_model) );

    /* Make column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Categories",
                        cell,
                        "text", 0,
                        NULL);
    gtk_tree_view_column_add_attribute ( GTK_TREE_VIEW_COLUMN(column), cell,
                        "weight", 2 );

    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                        GTK_TREE_VIEW_COLUMN (column));
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW (tree_view), FALSE );

    /* Handle select */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    g_signal_connect (selection, 
                        "changed",
                        (GCallback) selectionne_liste_preference,
                        preference_tree_model);

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function ( selection,
                        preference_selectable_func,
                        NULL, NULL );

    /* Put the tree in the scroll */
    gtk_container_add (GTK_CONTAINER (sw), tree_view);

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect ( tree_view, 
                        "realize",
                        (GCallback) gtk_tree_view_expand_all,
                        NULL );

    return sw;
}


gboolean preference_selectable_func (GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        gpointer data)
{
    GtkTreeIter iter;
    GValue value = {0, };

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get_value ( model, &iter, 1, &value );

    if ( g_value_get_int(&value) == NOT_A_PAGE )
    {
        g_value_unset (&value);
        return FALSE;
    }

    g_value_unset (&value);
    return TRUE;
}



/**
 * call the preferences page by a menu
 *
 * \param menu_item
 * \param page_ptr  the page to open
 *
 * \return FALSE
 * */
gboolean gsb_preferences_menu_open ( GtkWidget *menu_item,
                        gpointer page_ptr )
{
    preferences (GPOINTER_TO_INT (page_ptr) );
    return FALSE;
}


/**
 * Creates a new GtkDialog with a paned list of topics and a paned
 * notebook that allows to switch between all pages.  A click on the
 * list selects one specific page.
 *
 * \param page Initial page to select.
 */
gboolean preferences ( gint page )
{
    GtkWidget *hbox, *tree;
    GtkTreeIter iter, iter2;
    GtkWidget *hpaned;

    devel_debug_int (page);

    /* Create dialog */
    fenetre_preferences = gtk_dialog_new_with_buttons (_("Grisbi preferences"),
                        GTK_WINDOW ( window ),
                        GTK_DIALOG_MODAL,
                        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                        NULL );

    if ( conf.prefs_width )
        gtk_window_set_default_size ( GTK_WINDOW ( fenetre_preferences ),
                        conf.prefs_width, -1 );
    gtk_window_set_position ( GTK_WINDOW ( fenetre_preferences ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( fenetre_preferences ), TRUE );

    /* Create List & Tree for topics */
    tree = create_preferences_tree();
    hpaned = gtk_hpaned_new();
    gtk_paned_add1 ( GTK_PANED(hpaned), tree );
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_paned_add2 ( GTK_PANED (hpaned), hbox );

    /* Frame for preferences */
    preference_frame = GTK_NOTEBOOK ( gtk_notebook_new () );
    gtk_notebook_set_show_border ( preference_frame, FALSE );
    gtk_notebook_set_show_tabs  ( preference_frame, FALSE );
    gtk_notebook_set_scrollable ( preference_frame, TRUE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), GTK_WIDGET(preference_frame), TRUE, TRUE, 0 );

    /* Main subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter,
                        0, _("Main"),
                        1, NOT_A_PAGE,
                        2, 800,
                        -1);

    /* File tab */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Files"),
                        1, FILES_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_fichier(), NULL);
    /* by default, we select that first page */

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Archives"),
                        1, ARCHIVE_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_archive_config_create (), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Import"),
                        1, IMPORT_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_importation(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Import associations"),
                        1, IMPORT_ASSOCIATION_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, 
                        gsb_import_associations_gere_tiers (), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Programs"),
                        1, SOFTWARE_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_programmes(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Scheduler"),
                        1, SCHEDULER_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_config_scheduler_page (), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Main page"),
                        1, MAIN_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_accueil (), NULL);

    /* Display subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter,
                        0, _("Display"),
                        1, NOT_A_PAGE,
                        2, 800,
                        -1);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Fonts & logo"),
                        1, FONTS_AND_LOGO_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_display_fonts(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Messages & warnings"),
                        1, MESSAGES_AND_WARNINGS_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_messages_and_warnings(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Addresses & titles"),
                        1, ADDRESSES_AND_TITLES_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_display_addresses(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Payees, categories and budgetaries"),
                        1, TOTALS_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page ( preference_frame, onglet_metatree (), NULL );

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Toolbars"),
                        1, TOOLBARS_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, tab_display_toolbar(), NULL);

    /* Transactions subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter,
                        0, _("Transactions"),
                        1, NOT_A_PAGE,
                        2, 800,
                        -1);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("List behavior"),
                        1, TRANSACTIONS_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame,
                        GTK_WIDGET(onglet_affichage_operations()), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Messages before deleting"),
                        1, DELETE_MESSAGES_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_delete_messages(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Reconciliation"),
                        1, RECONCILIATION_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_reconcile_config_create(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Sort for reconciliation"),
                        1, RECONCILIATION_SORT_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_reconcile_sort_config_create(), NULL);

    /* Transaction form subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter,
                        0, _("Transaction form"),
                        1, NOT_A_PAGE,
                        2, 800,
                        -1);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Content"),
                        1, TRANSACTION_FORM_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_form_config_create_page(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2,&iter );
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Behavior"),
                        1, TRANSACTION_FORM_BEHAVIOR_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_diverse_form_and_lists(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2,&iter );
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Completion"),
                        1, TRANSACTION_FORM_COMPLETION_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_form_completion(), NULL);

    /* Resources subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter,
                        0, _("Resources"),
                        1, NOT_A_PAGE,
                        2, 800,
                        -1);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Currencies"),
                        1, CURRENCIES_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_currency_config_create_page(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Currencies links"),
                        1, CURRENCY_LINKS_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_currency_link_config_create_page (), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Banks"),
                        1, BANKS_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_bank_create_page (FALSE), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Financial years"),
                        1, FINANCIAL_YEARS_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_fyear_config_create_page(), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Payment methods"),
                        1, METHODS_OF_PAYMENT_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, gsb_payment_method_config_create (), NULL);

    gtk_widget_show_all ( hpaned );
    gtk_container_set_border_width ( GTK_CONTAINER(hpaned), 6 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(fenetre_preferences) -> vbox ),
                        hpaned, TRUE, TRUE, 0);

#ifdef ENABLE_BALANCE_ESTIMATE 
    /* balance estimate subtree */
    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter,
                        0, _("Balance estimate"),
                        1, NOT_A_PAGE,
                        2, 800,
                        -1);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("General Options"),
                        1, BET_GENERAL_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, bet_config_general_create_general_page (), NULL);

    gtk_tree_store_append (GTK_TREE_STORE (preference_tree_model), &iter2, &iter);
    gtk_tree_store_set (GTK_TREE_STORE (preference_tree_model),
                        &iter2,
                        0, _("Accounts data"),
                        1, BET_ACCOUNT_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, bet_config_account_create_account_page (), NULL);

#endif

    /* select the page */
    if ( page >= 0 && page < NUM_PREFERENCES_PAGES )
        gtk_notebook_set_current_page ( GTK_NOTEBOOK (preference_frame), page );

    switch (gtk_dialog_run ( GTK_DIALOG ( fenetre_preferences ) ))
    {
        case GTK_RESPONSE_HELP:
        /* Hook some help function */
        break;
        default:
        gtk_window_get_size ( GTK_WINDOW ( fenetre_preferences ), &conf.prefs_width, NULL );
        gtk_widget_destroy ( GTK_WIDGET ( fenetre_preferences ));
        return FALSE;
    }
    return FALSE;
}


/* ************************************************************************************************************** */
/* callback appelé quand on sélectionne un membre de la liste */
/* ************************************************************************************************************** */
gboolean selectionne_liste_preference ( GtkTreeSelection *selection,
                        GtkTreeModel *model )
{
    GtkTreeIter iter;
    GValue value = {0, };
    gint preference_selected;

    if ( ! gtk_tree_selection_get_selected (selection, NULL, &iter) )
    return ( FALSE );

    gtk_tree_model_get_value ( model, &iter, 1, &value );

    preference_selected = g_value_get_int ( &value );
    gtk_notebook_set_current_page ( preference_frame, preference_selected );

    g_value_unset ( &value );

    return FALSE;
}


/**
 * Creates the "Warning & Messages" tab.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_messages_and_warnings ( void )
{
    GtkWidget *vbox_pref, *paddingbox, *tip_checkbox, *tree_view, *sw;
    GtkTreeModel * model;
    GtkCellRenderer * cell;
    GtkTreeViewColumn * column;
    gchar *tmpstr;
    int i;

    vbox_pref = new_vbox_with_title_and_icon ( _("Messages & warnings"), "warnings.png" );

    /* Tip of the day */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Tip of the day"));

    /* Display or not tips */
    tip_checkbox = gsb_automem_checkbutton_new ( _("Display tip of the day"),
                        &(etat.show_tip),
                        NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), tip_checkbox, FALSE, FALSE, 0 );

    /* Warnings */
    paddingbox = new_paddingbox_with_title ( vbox_pref, TRUE, 
                        _("Display following warnings messages") );

    model = GTK_TREE_MODEL(gtk_tree_store_new ( 3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT ) );

    sw = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    tree_view = gtk_tree_view_new();
    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model) );
    g_object_unref (G_OBJECT(model));
    gtk_container_add (GTK_CONTAINER (sw), tree_view);
    gtk_box_pack_start ( GTK_BOX(paddingbox), sw, TRUE, TRUE, 0 );

    cell = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes ("", cell, "active", 0, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    g_signal_connect (cell, "toggled", G_CALLBACK (gsb_gui_messages_toggled), model);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Message"), cell, "text", 1, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));

    for  ( i = 0; messages[i].name; i++ )
    {
        GtkTreeIter iter;

        if ( g_utf8_collate ( messages[i].name, "account-already-opened" ) == 0 )
            tmpstr = g_strdup_printf ( _(messages[i] . hint), titre_fichier );
        else if ( g_utf8_collate ( messages[i].name, "development-version" ) == 0 )
            tmpstr = g_strdup_printf ( _(messages[i] . hint), VERSION );
        else if ( g_utf8_collate ( messages[i].name, "gtk_obsolete" ) == 0 )
            tmpstr = g_strdup_printf ( _(messages[i] . hint), get_gtk_run_version ( ) );
        else
            tmpstr = g_strdup ( _(messages[i] . hint) );

        gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                        0, !messages[i] . hidden,
                        1, tmpstr,
                        2, i,
                        -1);

        g_free ( tmpstr );
    }

    /* Show everything */
    gtk_widget_show_all ( vbox_pref );

    if ( !gsb_data_account_get_accounts_amount () )
    {
        gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}


/**
 * Creates the "Delete messages" tab.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_delete_messages ( void )
{
    GtkWidget *vbox_pref, *paddingbox, *tree_view, *sw;
    GtkTreeModel * model;
    GtkCellRenderer * cell;
    GtkTreeViewColumn * column;
    gchar *tmpstr;
    int i;

    vbox_pref = new_vbox_with_title_and_icon ( _("Messages before deleting"), "delete.png" );

    /* Delete messages */
    paddingbox = new_paddingbox_with_title ( vbox_pref, TRUE, 
                        _("Display following messages") );

    model = GTK_TREE_MODEL(gtk_tree_store_new ( 3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT ) );

    sw = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    tree_view = gtk_tree_view_new();
    gtk_tree_view_set_model ( GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model) );
    g_object_unref (G_OBJECT(model));
    gtk_container_add (GTK_CONTAINER (sw), tree_view);
    gtk_box_pack_start ( GTK_BOX(paddingbox), sw, TRUE, TRUE, 0 );

    cell = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes ("", cell, "active", 0, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
    g_signal_connect (cell,
                        "toggled",
                        G_CALLBACK (gsb_gui_delete_msg_toggled),
                        model);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Message"), cell, "text", 1, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));

    for  ( i = 0; delete_msg[i].name; i++ )
    {
        GtkTreeIter iter;

        tmpstr = g_strdup ( _(delete_msg[i] . hint) );

        gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
        gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                        0, !delete_msg[i] . hidden,
                        1, tmpstr,
                        2, i,
                        -1);

        g_free ( tmpstr );
    }

    /* Show everything */
    gtk_widget_show_all ( vbox_pref );

    if ( !gsb_data_account_get_accounts_amount () )
    {
        gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return ( vbox_pref );
}


/**
 *
 *
 */
gboolean gsb_gui_messages_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
                        GtkTreeModel * model )
{
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter;
    gint position;

    /* Get toggled iter */
    gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, 2, &position, -1);

    messages[position] . hidden = !messages[position] . hidden;

    /* Set new value */
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 0, ! messages[position] . hidden, -1);

    return TRUE;
}


/**
 *
 *
 */
gboolean gsb_gui_delete_msg_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
                        GtkTreeModel * model )
{
    GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
    GtkTreeIter iter;
    gint position;

    /* Get toggled iter */
    gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, 2, &position, -1);

    delete_msg[position].hidden = !delete_msg[position].hidden;
    if ( delete_msg[position].hidden == 1 )
        delete_msg[position].default_answer = 1;

    /* Set new value */
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 0, !delete_msg[position].hidden, -1);

    return TRUE;
}


/**
 * Creates the "Files" tab.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_fichier ( void )
{
    GtkWidget *vbox_pref, *paddingbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *button;
    GtkWidget *dialog;

    vbox_pref = new_vbox_with_title_and_icon ( _("Files"), "files.png" );

    /* Account file handling */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE,
                        _("Account files handling"));

    /* Automatically load last file on startup? */
    button = gsb_automem_checkbutton_new (_("Automatically load last file on startup"),
                        &conf.dernier_fichier_auto, NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    button = gsb_automem_checkbutton_new (_("Automatically save on exit"),
                        &conf.sauvegarde_auto, NULL, NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* Warn if file is used by someone else? */
    button = gsb_automem_checkbutton_new ( _("Force saving of locked files"),
                        &conf.force_enregistrement, NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* crypt the grisbi file */
    button = gsb_automem_checkbutton_new ( _("Encrypt Grisbi file"),
                        &(etat.crypt_file), G_CALLBACK (gsb_gui_encryption_toggled), NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* Compression level of files */
    button = gsb_automem_checkbutton_new ( _("Compress Grisbi file"),
                        &conf.compress_file, NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* Memorize last opened files in menu */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Memorise last opened files")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    button = gsb_automem_spin_button_new ( &(nb_max_derniers_fichiers_ouverts),
                        G_CALLBACK ( affiche_derniers_fichiers_ouverts ), NULL );
    gtk_widget_set_size_request ( button, width_spin_button, -1 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    /* Backups */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Backups"));

    /* Single backup file */
    button = gsb_automem_checkbutton_new ( _("Make a single backup file"),
                        &conf.make_bakup_single_file, NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* Compression level of backups */
    button = gsb_automem_checkbutton_new ( _("Compress Grisbi backup"),
                        &conf.compress_backup, NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* Backup at each opening? */
    button = gsb_automem_checkbutton_new ( _("Make a backup copy after opening files"),
                        &conf.sauvegarde_demarrage, NULL, NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* Automatic backup ? */
    button = gsb_automem_checkbutton_new (_("Make a backup copy before saving files"),
                        &conf.make_backup, NULL, NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, FALSE, FALSE, 0 );

    /* Automatic backup every x minutes */
    hbox = gtk_hbox_new ( FALSE, 6);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0);

    button = gsb_automem_checkbutton_new (_("Make a backup copy every "),
                        &conf.make_backup_every_minutes,
                        G_CALLBACK (gsb_file_automatic_backup_start), NULL);
    gtk_box_pack_start ( GTK_BOX (hbox), button, FALSE, FALSE, 0 );

    button = gsb_automem_spin_button_new ( &conf.make_backup_nb_minutes,
                        G_CALLBACK (gsb_file_automatic_backup_change_time), NULL );
    gtk_widget_set_size_request ( button, width_spin_button, -1 );
    gtk_box_pack_start ( GTK_BOX (hbox), button, FALSE, FALSE, 0 );

    label = gtk_label_new (_(" minutes"));
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    /* if automatic backup, choose a dir */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0);

    label = gtk_label_new ( COLON(_("Backup directory")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0);

    /* on passe par une fonction intermédiaire pour pallier à un bug
     * du gtk_file_chooser_button_new qui donne le répertoire home
     * lorsque l'on annule le choix du nouveau répertoire */
    dialog = utils_files_create_file_chooser ( window,
                        _("Select/Create backup directory") );

    button = gtk_file_chooser_button_new_with_dialog ( dialog );
    if ( gsb_file_get_backup_path ( ) )
        gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (button),
                        gsb_file_get_backup_path ());
    else
        gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (button),
                        my_get_XDG_grisbi_data_dir () );
    g_signal_connect ( G_OBJECT (button),
                        "selection-changed",
                        G_CALLBACK ( gsb_config_backup_dir_chosen ),
                        dialog );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, TRUE, 0);

    gtk_widget_show_all ( vbox_pref );

    if ( !gsb_data_account_get_accounts_amount () )
        gtk_widget_set_sensitive ( vbox_pref, FALSE );

    return ( vbox_pref );
}


/**
 * Warns that there is no coming back if password is forgotten when
 * encryption is activated.
 *
 * \param checkbox  Checkbox that triggered event.
 * \param data      Unused.
 *
 * \return          FALSE
 */
gboolean gsb_gui_encryption_toggled ( GtkWidget * checkbox, gpointer data )
{
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (checkbox)))
    {
    dialog_message ( "encryption-is-irreversible" );
    }

    return FALSE;
}

/**
 * called when choose a new directory for the backup
 *
 * \param button the GtkFileChooserButton
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_config_backup_dir_chosen ( GtkWidget *button,
                        GtkWidget *dialog )
{
    gchar *path;

    path = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( button ) );
    devel_debug ( path );
    gsb_file_set_backup_path ( path );
    if ( path && strlen ( path ) > 0 )
        g_free ( path );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return FALSE;
}



/* *******************************************************************************/
/* page de configuration des logiciels externes */
/* *******************************************************************************/
GtkWidget *onglet_programmes (void)
{
    GtkWidget *vbox_pref, *label, *entry, *paddingbox, *table;
    GtkSizeGroup *size_group;
    gchar * text;

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    vbox_pref = new_vbox_with_title_and_icon ( _("Programs"), "exec.png" );

    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, _("Web"));

    table = gtk_table_new ( 0, 2, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    label = gtk_label_new ( COLON(_("Web browser command")));
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 0, 1,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entry = gsb_automem_entry_new ( &conf.browser_command, NULL, NULL );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    text = make_blue ( g_strconcat ( 
                        _("You may use %s to expand the URL I.e:\n'firefox -remote %s'"),
                        NULL ) );
    label = gtk_label_new ( text );
    gtk_label_set_use_markup ( GTK_LABEL(label), TRUE );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 1, 2, 1, 2,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    g_free ( text );


    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("LaTeX support (old print system)") );

    table = gtk_table_new ( 0, 2, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    label = gtk_label_new ( COLON(_("LaTeX command")));
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 0, 1,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entry = gsb_automem_entry_new ( &conf.latex_command, NULL, NULL );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    label = gtk_label_new ( COLON(_("dvips command")));
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entry = gsb_automem_entry_new ( &conf.dvips_command, NULL, NULL );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );


    gtk_size_group_set_mode ( size_group, GTK_SIZE_GROUP_HORIZONTAL );
    gtk_widget_show_all ( vbox_pref );

    if ( ! assert_account_loaded() )
      gtk_widget_set_sensitive ( vbox_pref, FALSE );

    return ( vbox_pref );
}



/**
 * create the scheduler config page
 *
 * \param
 *
 * \return a GtkWidget containing the page of scheduler config
 * */
static GtkWidget *gsb_config_scheduler_page ( void )
{
    GtkWidget *vbox_pref, *paddingbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;

    vbox_pref = new_vbox_with_title_and_icon ( _("Scheduler"), "scheduler.png" );

    /* Number of days before a warning message advertising a scheduled
       transaction */
    button = gsb_automem_radiobutton_new_with_title ( vbox_pref,
                        _("Scheduler warnings at Grisbi's opening"),
                        _("Warn/Execute the scheduled transactions arriving at expiration date"),
                        _("Warn/Execute the scheduled transactions of the month"),
                        &execute_scheduled_of_month,
                        NULL, NULL );

    hbox = gtk_hbox_new ( FALSE, 0);
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( SPACIFY ( COLON (
                        _("Number of days before the warning or the execution"))) );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    entry = gsb_automem_spin_button_new ( &nb_days_before_scheduled, NULL, NULL );
    gtk_widget_set_size_request ( entry, width_spin_button, -1 );

    gtk_box_pack_start ( GTK_BOX (hbox), entry, FALSE, FALSE, 0 );

    /* Take into account the planned operations in the calculation of balances */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Calculation of balances") );

    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    button = gsb_automem_checkbutton_new (
                        _("Take into account the scheduled operations "
                          "in the calculation of balances"),
                        &balances_with_scheduled,
                        G_CALLBACK ( gsb_config_scheduler_switch_balances_with_scheduled ),
                        NULL );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    return vbox_pref;
}


gboolean gsb_config_scheduler_switch_balances_with_scheduled ( void )
{
    GSList *list_tmp;

    devel_debug ( NULL );

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
        gint account_number;

        account_number = gsb_data_account_get_no_account ( list_tmp -> data );
        gsb_data_account_set_balances_are_dirty ( account_number );

        /* MAJ HOME_PAGE */
        gsb_gui_navigation_update_home_page ( );

        list_tmp = list_tmp -> next;
    }
    return FALSE;
}


/**
 * create the metatree config page
 *
 * \param
 *
 * \return a GtkWidget containing the page of scheduler config
 * */
GtkWidget *onglet_metatree ( void )
{
    GtkWidget *vbox_pref, *paddingbox, *total_currencies;

    vbox_pref = new_vbox_with_title_and_icon ( 
                        _("Payees, categories and budgetaries"),
                        "organization.png" );

    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Totals currencies") );
    total_currencies = gsb_currency_config_create_totals_page ( );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), total_currencies, FALSE, FALSE, 0 );

    /* tri des opérations */
    gsb_automem_radiobutton_new_with_title ( vbox_pref,
                        _("Sort option for transactions"),
                        _("by number"),
                        _("by date"),
                        &etat.metatree_sort_transactions,
                        G_CALLBACK (gsb_config_metatree_sort_transactions), NULL );

    return vbox_pref;
}


gboolean gsb_config_metatree_sort_transactions ( GtkWidget *checkbutton,
                        gpointer null )
{
    gint page_number;

    page_number = gsb_gui_navigation_get_current_page ( );

    switch ( page_number )
    {
	case GSB_PAYEES_PAGE:
		payee_fill_tree ();
	    break;

	case GSB_CATEGORIES_PAGE:
        remplit_arbre_categ ();
	    break;

	case GSB_BUDGETARY_LINES_PAGE:
		remplit_arbre_imputation ();
	    break;

	default:
	    notice_debug ("B0rk page selected");
	    break;
    }

    return FALSE;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
