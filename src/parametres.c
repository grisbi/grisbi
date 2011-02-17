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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "parametres.h"
#include "accueil.h"
#include "affichage.h"
#include "affichage_liste.h"
#include "bet_config.h"
#include "bet_data.h"
#include "bet_finance_ui.h"
#include "categories_onglet.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_archive_config.h"
#include "gsb_automem.h"
#include "gsb_bank.h"
#include "gsb_currency_config.h"
#include "gsb_currency_link_config.h"
#include "gsb_data_account.h"
#include "gsb_file.h"
#include "gsb_form_config.h"
#include "gsb_fyear_config.h"
#include "gsb_payment_method_config.h"
#include "gsb_real.h"
#include "gsb_reconcile_config.h"
#include "gsb_reconcile_sort_config.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "import.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *create_preferences_tree ( );
static gboolean gsb_config_onglet_metatree_action_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gint *pointeur );
static GtkWidget *gsb_config_scheduler_page ( void );
static gboolean gsb_gui_delete_msg_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
                        GtkTreeModel * model );
static gboolean gsb_gui_messages_toggled ( GtkCellRendererToggle *cell, gchar *path_str,
                        GtkTreeModel * model );
static void gsb_localisation_decimal_point_changed ( GtkComboBox *widget, gpointer user_data );
static gboolean gsb_localisation_format_date_toggle ( GtkToggleButton *togglebutton,
                        GdkEventButton *event,
                        gpointer user_data);
static void gsb_localisation_thousands_sep_changed ( GtkComboBox *widget, gpointer user_data );
static void gsb_localisation_update_affichage ( gint type_maj );
static GtkWidget *onglet_delete_messages ( void );
static GtkWidget *onglet_fichier ( void );
static GtkWidget *onglet_localisation ( void );
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


/* global "etat" structure shared in the entire program */
struct gsb_etat_t etat;

GtkWidget *fenetre_preferences = NULL;

static GtkTreeStore *preference_tree_model = NULL;
static GtkNotebook * preference_frame = NULL;

static gint width_spin_button = 50;


/*START_EXTERN*/
extern GtkWidget *account_page;
extern GtkWidget *arbre_categ;
extern GtkWidget *budgetary_line_tree;
extern struct conditional_message delete_msg[];
extern gboolean execute_scheduled_of_month;
extern struct conditional_message messages[];
extern gint mise_a_jour_liste_comptes_accueil;
extern gchar *nom_fichier_comptes;
extern gint nb_days_before_scheduled;
extern gint nb_max_derniers_fichiers_ouverts;
extern GtkWidget *payee_tree;
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

    if ( gsb_gui_navigation_get_current_page ( ) == - 1 )
        return FALSE;

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
                        0, _("Localization"),
                        1, LOCALISATION_PAGE,
                        2, 400,
                        -1);
    gtk_notebook_append_page (preference_frame, onglet_localisation (), NULL);

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
            tmpstr = g_strdup_printf ( _(messages[i] . hint),
                        g_path_get_basename ( nom_fichier_comptes ) );
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

    label = gtk_label_new ( _("Memorise last opened files: ") );
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

    label = gtk_label_new ( _("Backup directory: ") );
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

    /* Config file */
#if IS_DEVELOPMENT_VERSION == 1
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Config file") );

    hbox = gtk_hbox_new ( FALSE, 6);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0);

    button = gsb_automem_checkbutton_new (_("Use the config file of version stable as model"),
                        &conf.stable_config_file_model,
                        NULL, NULL);
    gtk_box_pack_start ( GTK_BOX (hbox), button, FALSE, FALSE, 0 );
#endif

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
        run.new_crypted_file = TRUE;
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

    label = gtk_label_new ( _("Web browser command: ") );
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

    label = gtk_label_new ( _("LaTeX command: ") );
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 0, 1,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entry = gsb_automem_entry_new ( &conf.latex_command, NULL, NULL );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    label = gtk_label_new ( _("dvips command: ") );
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
    GtkWidget *vbox_pref;
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

    label = gtk_label_new (
                        _("Number of days before the warning or the execution: ") );
    gtk_box_pack_start ( GTK_BOX (hbox), label, FALSE, FALSE, 0 );

    entry = gsb_automem_spin_button_new ( &nb_days_before_scheduled, NULL, NULL );
    gtk_widget_set_size_request ( entry, width_spin_button, -1 );

    gtk_box_pack_start ( GTK_BOX (hbox), entry, FALSE, FALSE, 0 );

    return vbox_pref;
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
    gsb_automem_radiobutton3_new_with_title ( vbox_pref,
                        _("Sort option for transactions"),
                        _("by number"),
                        _("by increasing date"),
                        _("by date descending"),
                        &etat.metatree_sort_transactions,
                        G_CALLBACK ( gsb_config_metatree_sort_transactions_changed ),
                        &etat.metatree_sort_transactions,
                        GTK_ORIENTATION_VERTICAL );

    gsb_automem_radiobutton3_new_with_title ( vbox_pref,
						_("Choice of the action for double click of the mouse: "),
                        _("Expand the line"),
                        _("Edit the line"),
                        _("Manage the line"),
					    &conf.metatree_action_2button_press,
					    G_CALLBACK ( gsb_config_onglet_metatree_action_changed ),
					    &conf.metatree_action_2button_press,
                        GTK_ORIENTATION_VERTICAL );

    return vbox_pref;
}


/**
 *
 *
 *
 *
 * */
gboolean gsb_config_metatree_sort_transactions_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gint *pointeur )
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path = NULL;
    gint page_number;

    if ( pointeur )
    {
        gint value = 0;

        value = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( checkbutton ), "pointer" ) );
        *pointeur = value;
    }

    page_number = gsb_gui_navigation_get_current_page ( );

    switch ( page_number )
    {
	case GSB_PAYEES_PAGE:
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( payee_tree ) );
        if ( gtk_tree_selection_get_selected ( selection, &model, &iter ) )
            path = gtk_tree_model_get_path ( model, &iter );
		payee_fill_tree ();
        gtk_tree_path_up ( path );
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( payee_tree ), path );
        gtk_tree_path_free ( path );
	    break;

	case GSB_CATEGORIES_PAGE:
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( arbre_categ ) );
        if ( gtk_tree_selection_get_selected ( selection, &model, &iter ) )
            path = gtk_tree_model_get_path ( model, &iter );
        remplit_arbre_categ ();
        gtk_tree_path_up ( path );
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( payee_tree ), path );
        gtk_tree_path_free ( path );
	    break;

	case GSB_BUDGETARY_LINES_PAGE:
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( budgetary_line_tree ) );
        if ( gtk_tree_selection_get_selected ( selection, &model, &iter ) )
            path = gtk_tree_model_get_path ( model, &iter );
		remplit_arbre_imputation ();
        gtk_tree_path_up ( path );
        gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( payee_tree ), path );
        gtk_tree_path_free ( path );
	    break;

	default:
	    notice_debug ("B0rk page selected");
	    break;
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
GtkWidget *onglet_localisation ( void )
{
    GtkWidget *vbox_pref, *paddingbox;


    vbox_pref = new_vbox_with_title_and_icon ( _("Localization"), "locale.png" );

/*    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Choose Language") ); */

    paddingbox = gsb_config_date_format_chosen ( vbox_pref, GTK_ORIENTATION_VERTICAL );

    paddingbox = gsb_config_number_format_chosen ( vbox_pref, GTK_ORIENTATION_VERTICAL );

    return vbox_pref;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *gsb_config_date_format_chosen ( GtkWidget *parent, gint sens )
{
    GtkWidget *hbox, *paddingbox;
    GtkWidget *button_1, *button_2;
    gchar *format_date;

    button_1 =gtk_radio_button_new_with_label ( NULL, "dd/mm/yyyy" );
    format_date = g_strdup ( "%d/%m/%Y" );
    g_object_set_data_full ( G_OBJECT ( button_1 ),
                        "pointer",
                        format_date,
                        g_free );

    button_2 = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (
                        GTK_RADIO_BUTTON ( button_1 ) ),
						"mm/dd/yyyy" );
    format_date = g_strdup ( "%m/%d/%Y" );
    g_object_set_data_full ( G_OBJECT ( button_2 ),
                        "pointer",
                        format_date,
                        g_free );

    if ( sens == GTK_ORIENTATION_VERTICAL )
    {
        paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Choose the date format") );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_1, FALSE, FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_2, FALSE, FALSE, 0 );
    }
    else
    {
        paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Date format") );
        hbox = gtk_hbox_new ( FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 0 );
    }

    format_date = gsb_date_get_format_date ( );
    if ( format_date && strcmp ( format_date, "%m/%d/%Y" ) == 0 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_2 ), TRUE );
    g_free ( format_date );

    g_signal_connect ( G_OBJECT ( button_1 ),
                        "button-release-event",
                        G_CALLBACK ( gsb_localisation_format_date_toggle ),
                        GINT_TO_POINTER ( sens ) );
    g_signal_connect ( G_OBJECT ( button_2 ),
                        "button-release-event",
                        G_CALLBACK ( gsb_localisation_format_date_toggle ),
                        GINT_TO_POINTER ( sens ) );

    return paddingbox;
}


/**
 *
 *
 *
 *
 * */
gboolean gsb_localisation_format_date_toggle ( GtkToggleButton *togglebutton,
                        GdkEventButton *event,
                        gpointer user_data)
{
    const gchar *format_date;

    format_date = g_object_get_data ( G_OBJECT ( togglebutton ), "pointer" );
    gsb_date_set_format_date ( format_date );

    if ( GPOINTER_TO_INT ( user_data ) == GTK_ORIENTATION_HORIZONTAL )
        return FALSE;

    gsb_localisation_update_affichage ( 0 );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *gsb_config_number_format_chosen ( GtkWidget *parent, gint sens )
{
    GtkWidget *hbox, *paddingbox, *label;
    GtkWidget *dec_hbox, *dec_sep, *thou_hbox, *thou_sep;
    GtkSizeGroup *size_group;
    gchar *mon_decimal_point;
    gchar *mon_thousands_sep;

    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    dec_hbox = gtk_hbox_new ( FALSE, 0 );
    label = gtk_label_new ( _("Decimal point: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( dec_hbox ), label, FALSE, FALSE, 0 );

    dec_sep = gtk_combo_box_entry_new_text ( );
    gtk_editable_set_editable ( GTK_EDITABLE ( GTK_BIN ( dec_sep ) -> child ), FALSE );
    gtk_entry_set_width_chars ( GTK_ENTRY ( GTK_BIN ( dec_sep ) -> child ), 5 );
    gtk_combo_box_append_text ( GTK_COMBO_BOX ( dec_sep ), "." );
    gtk_combo_box_append_text ( GTK_COMBO_BOX ( dec_sep ), "," );
    gtk_box_pack_start ( GTK_BOX ( dec_hbox ), dec_sep, FALSE, FALSE, 0 );

    thou_hbox = gtk_hbox_new ( FALSE, 0 );
    label = gtk_label_new (_("Thousands separator: ") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( thou_hbox ), label, FALSE, FALSE, 0 );

    thou_sep = gtk_combo_box_entry_new_text ( );
    gtk_editable_set_editable ( GTK_EDITABLE ( GTK_BIN ( thou_sep ) -> child ), FALSE );
    gtk_entry_set_width_chars ( GTK_ENTRY ( GTK_BIN ( thou_sep ) -> child ), 5 );
    gtk_combo_box_append_text ( GTK_COMBO_BOX ( thou_sep ), "' '" );
    gtk_combo_box_append_text ( GTK_COMBO_BOX ( thou_sep ), "." );
    gtk_combo_box_append_text ( GTK_COMBO_BOX ( thou_sep ), "," );
    gtk_combo_box_append_text ( GTK_COMBO_BOX ( thou_sep ), "''" );

    gtk_box_pack_start ( GTK_BOX ( thou_hbox ), thou_sep, FALSE, FALSE, 0 );

    if ( sens == GTK_ORIENTATION_VERTICAL )
    {
        paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Choose the decimal and thousands separator") );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), dec_hbox, FALSE, FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), thou_hbox, FALSE, FALSE, 0 );
    }
    else
    {
        paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Decimal and thousands separator") );
        hbox = gtk_hbox_new ( TRUE, 0 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( hbox ), dec_hbox, FALSE, FALSE, 0 );
        gtk_box_pack_start ( GTK_BOX ( hbox ), thou_hbox, FALSE, FALSE, 0 );
    }

    mon_decimal_point = gsb_real_get_decimal_point ( );
    if ( strcmp ( mon_decimal_point, "," ) == 0 )
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( dec_sep ), 1 );
    else
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( dec_sep ), 0 );
    g_free ( mon_decimal_point );

    mon_thousands_sep = gsb_real_get_thousands_sep ( );
    if ( mon_thousands_sep == NULL )
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( thou_sep ), 3 );
    else if ( strcmp ( mon_thousands_sep, "." ) == 0 )
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( thou_sep ), 1 );
    else if ( strcmp ( mon_thousands_sep, "," ) == 0 )
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( thou_sep ), 2 );
    else
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( thou_sep ), 0 );

    if ( mon_thousands_sep )
        g_free ( mon_thousands_sep );

    g_object_set_data ( G_OBJECT ( dec_sep ), "separator", thou_sep );
    g_object_set_data ( G_OBJECT ( thou_sep ), "separator", dec_sep );

    g_signal_connect ( G_OBJECT ( dec_sep ),
                        "changed",
                        G_CALLBACK ( gsb_localisation_decimal_point_changed ),
                        GINT_TO_POINTER ( sens ) );
    g_signal_connect ( G_OBJECT ( thou_sep ),
                        "changed",
                        G_CALLBACK ( gsb_localisation_thousands_sep_changed ),
                        GINT_TO_POINTER ( sens ) );

    return paddingbox;
}


/**
 *
 *
 *
 *
 * */
void gsb_localisation_decimal_point_changed ( GtkComboBox *widget, gpointer user_data )
{
    GtkWidget *combo_box;
    GtkWidget *entry;
    gchar *str_capital;
    const gchar *text;

    text = gtk_combo_box_get_active_text ( widget );
    combo_box = g_object_get_data ( G_OBJECT ( widget ), "separator" );

    if ( g_strcmp0 ( text, "," ) == 0 )
    {
        gsb_real_set_decimal_point ( "," );

        if ( g_strcmp0 ( gtk_combo_box_get_active_text ( GTK_COMBO_BOX ( combo_box ) ), "," ) == 0 )
        {
            gsb_real_set_thousands_sep ( " " );
            gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_box ), 0 );
        }
    }
    else
    {
        gsb_real_set_decimal_point ( "." );
        if ( g_strcmp0 ( gtk_combo_box_get_active_text ( GTK_COMBO_BOX ( combo_box ) ), "." ) == 0 )
        {
            gsb_real_set_thousands_sep ( "," );
            gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_box ), 2 );
        }
    }

    /* reset capital */
    entry = bet_finance_get_capital_entry ( );
    str_capital = gsb_real_get_string_with_currency ( gsb_real_double_to_real (
                    etat.bet_capital ),
                    etat.bet_currency,
                    FALSE );

    gtk_entry_set_text ( GTK_ENTRY ( entry ), str_capital );
    g_free ( str_capital );

    if ( GPOINTER_TO_INT ( user_data ) == GTK_ORIENTATION_HORIZONTAL )
        return;

    gsb_localisation_update_affichage ( 1 );
}


/**
 *
 *
 *
 *
 * */
void gsb_localisation_thousands_sep_changed ( GtkComboBox *widget, gpointer user_data )
{
    GtkWidget *combo_box;
    GtkWidget *entry;
    gchar *str_capital;
    const gchar *text;

    text = gtk_combo_box_get_active_text ( widget );
    combo_box = g_object_get_data ( G_OBJECT ( widget ), "separator" );
    
    if ( g_strcmp0 ( text, "' '" ) == 0 )
    {
        gsb_real_set_thousands_sep ( " " );
    }
    else if ( g_strcmp0 ( text, "." ) == 0 )
    {

        gsb_real_set_thousands_sep ( "." );
        if ( g_strcmp0 ( gtk_combo_box_get_active_text ( GTK_COMBO_BOX ( combo_box ) ), "." ) == 0 )
        {
            gsb_real_set_decimal_point ( "," );
            gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_box ), 1 );
        }
    }
    else if ( g_strcmp0 ( text, "," ) == 0 )
    {

        gsb_real_set_thousands_sep ( "," );
        if ( g_strcmp0 ( gtk_combo_box_get_active_text ( GTK_COMBO_BOX ( combo_box ) ), "," ) == 0 )
        {
            gsb_real_set_decimal_point ( "." );
            gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo_box ), 0 );
        }
    }
    else
        gsb_real_set_thousands_sep ( NULL );

    /* reset capital */
    entry = bet_finance_get_capital_entry ( );
    str_capital = gsb_real_get_string_with_currency ( gsb_real_double_to_real (
                    etat.bet_capital ),
                    etat.bet_currency,
                    FALSE );

    gtk_entry_set_text ( GTK_ENTRY ( entry ), str_capital );
    g_free ( str_capital );

    if ( GPOINTER_TO_INT ( user_data ) == GTK_ORIENTATION_HORIZONTAL )
        return;

    gsb_localisation_update_affichage ( 1 );
}


/**
 * met à jour l'affichage suite à modification des données de localisation
 *
 *\param type_maj 0 = ELEMENT_DATE 1 =  ELEMENT_CREDIT && ELEMENT_DEBIT
 *
 * */
void gsb_localisation_update_affichage ( gint type_maj )
{
    gint current_page;

    current_page = gsb_gui_navigation_get_current_page ( );

    /* update home page */
    if ( current_page == GSB_HOME_PAGE )
        mise_a_jour_accueil ( TRUE );
    else
        mise_a_jour_liste_comptes_accueil = TRUE;

    /* update sheduled liste */
    gsb_scheduler_list_fill_list ( gsb_scheduler_list_get_tree_view ( ) );
    gsb_scheduler_list_set_background_color ( gsb_scheduler_list_get_tree_view ( ) );
    if ( current_page == GSB_SCHEDULER_PAGE )
        gsb_scheduler_list_select (-1);

    /* update transaction liste */
    if ( type_maj == 0 )
    {
        transaction_list_update_element ( ELEMENT_DATE );
    }
    else
    {
        transaction_list_update_element ( ELEMENT_CREDIT );
        transaction_list_update_element ( ELEMENT_DEBIT );
        gsb_transactions_list_update_tree_view ( gsb_gui_navigation_get_current_account ( ), FALSE );
    }

    /* update home page */
    if ( current_page == GSB_ACCOUNT_PAGE )
    {
        gint account_number;
        gint account_current_page;
        kind_account kind;

        account_number = gsb_gui_navigation_get_current_account ( );
        account_current_page = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( account_page ) );

        kind = gsb_data_account_get_kind ( account_number );
        switch ( kind )
        {
            case GSB_TYPE_BANK:
            case GSB_TYPE_CASH:
                if ( account_current_page == 1 || account_current_page == 2 )
                {
                    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ALL );
                    bet_data_update_bet_module ( account_number, -1 );
                }
                break;
            case GSB_TYPE_LIABILITIES:
                if ( account_current_page == 3 )
                    bet_finance_ui_update_amortization_tab ( account_number );
                break;
            case GSB_TYPE_ASSET:
                break;
        }
    }

    /* update payees, categories and budgetary lines */
    if ( current_page == GSB_PAYEES_PAGE )
        payee_fill_tree ( );
    else if ( current_page == GSB_CATEGORIES_PAGE )
        remplit_arbre_categ ( );
    else if ( current_page == GSB_BUDGETARY_LINES_PAGE )
        remplit_arbre_imputation ( );

    /* update simulator page */
    if ( current_page == GSB_SIMULATOR_PAGE )
        bet_finance_switch_simulator_page ( );

}


/**
 *
 *
 * */
gboolean gsb_config_onglet_metatree_action_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gint *pointeur )
{
    if ( pointeur )
    {
        gint value = 0;

        value = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( checkbutton ), "pointer" ) );
        *pointeur = value;
        if ( etat.modification_fichier == 0 )
            modification_fichier ( TRUE );
    }

    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
