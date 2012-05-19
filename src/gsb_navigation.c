/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2008-2012 Pierre Biava (grisbi@pierre.biava.name)     */
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
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_navigation.h"
#include "accueil.h"
#include "bet_data.h"
#include "bet_finance_ui.h"
#include "categories_onglet.h"
#include "etats_onglet.h"
#include "fenetre_principale.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_account_property.h"
#include "gsb_assistant_account.h"
#include "gsb_calendar.h"
#include "gsb_data_account.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_import_rule.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_report.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_navigation_view.h"
#include "gsb_real.h"
#include "gsb_reconcile.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "metatree.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "transaction_list_select.h"
#include "transaction_list_sort.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static void gsb_gui_navigation_update_account_iter ( GtkTreeModel *model,
                        GtkTreeIter * account_iter,
                        gint account_number );
static gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
static void gsb_gui_navigation_update_report_iter ( GtkTreeModel *model,
                        GtkTreeIter * report_iter,
                        gint report_number );
static gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *account_page;
extern GtkWidget *label_last_statement;
extern GtkWidget *menu_import_rules;
extern gint mise_a_jour_liste_comptes_accueil;
/*END_EXTERN*/


/** Holds data for the navigation tree.  */
enum navigation_cols {
    NAVIGATION_PIX,
    NAVIGATION_PIX_VISIBLE,
    NAVIGATION_TEXT,
    NAVIGATION_FONT,
    NAVIGATION_PAGE,
    NAVIGATION_ACCOUNT,
    NAVIGATION_REPORT,
    NAVIGATION_SENSITIVE,
    NAVIGATION_ORDRE,        /* ordre des pages dans le modèle */
    NAVIGATION_TOTAL,
};

/** Widget that hold the scheduler calendar. */
static GtkWidget *scheduler_calendar = NULL;

/** Widget that hold all reconciliation widgets. */
GtkWidget *reconcile_panel;

/* contains the number of the last account
 * when switching between 2 accounts
 * at the end of the switch, contains the current account number */
static gint buffer_last_account = -1;

/**
 * Create the navigation pane on the left of the GUI.  It contains
 * account list as well as shortcuts.
 *
 * \param
 *
 * \return
 */
void gsb_gui_navigation_create_navigation_pane ( void )
{
    GtkWidget *vbox;
    GtkWidget *sw;
    GtkWidget *navigation_tree_view;
    GtkWidget *scheduler_calendar;
    GtkTreeModel *navigation_model;
    GrisbiWindow *window;
    GrisbiWindowEtat *etat;

    window = grisbi_app_get_active_window ( grisbi_app_get_default ( TRUE ) );
    etat = grisbi_window_get_struct_etat ();

    vbox = grisbi_window_get_widget_by_name ( "vbox_left_panel" );
    sw = grisbi_window_get_widget_by_name ( "sw_left_panel" );

    navigation_tree_view = gsb_navigation_view_new ( etat->navigation_sorting_accounts );
    grisbi_window_set_navigation_tree_view ( navigation_tree_view  );
    navigation_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( navigation_tree_view ) );

    gtk_container_add ( GTK_CONTAINER ( sw ), navigation_tree_view );

    /* Create calendar (hidden for now). */
    scheduler_calendar = gsb_calendar_new ();
    gtk_box_pack_end ( GTK_BOX ( vbox ), scheduler_calendar, FALSE, FALSE, 0 );
    grisbi_window_set_scheduler_calendar ( window, scheduler_calendar );

    gtk_widget_show ( navigation_tree_view );
    gtk_widget_hide ( scheduler_calendar );
}


/**
 * return the current page selected
 * the value returned is defined by GSB_GENERAL_NOTEBOOK_PAGES
 *
 * \param
 *
 * \return a gint wich is the numero of the page, -1 if problem
 *
 * */
gint gsb_gui_navigation_get_current_page ( void )
{
    GtkWidget *navigation_tree_view;
    GtkTreeSelection *selection;
    GtkTreeModel *navigation_model;
    GtkTreeIter iter;
    gint page;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    if ( !navigation_tree_view )
        return -1;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW ( navigation_tree_view ) );

    if ( !gtk_tree_selection_get_selected ( selection, NULL, &iter ) )
        return GSB_HOME_PAGE;

    navigation_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( navigation_tree_view ) );
    gtk_tree_model_get ( GTK_TREE_MODEL ( navigation_model ),
                        &iter,
                        NAVIGATION_PAGE, &page,
                        -1 );
    return page;
}



/**
 * return the account number selected
 * rem : this is only for account number of the transactions list,
 * if we want the current account number, for transactions or scheduled, go to
 * see gsb_form_get_account_number
 *
 * \param
 *
 * \return a gint, the account number or -1 if none selected
 * */
gint gsb_gui_navigation_get_current_account ( void )
{
    GtkWidget *navigation_tree_view;
    GtkTreeSelection *selection;
    GtkTreeModel *navigation_model;
    GtkTreeIter iter;
    gint page;
    gint account_number;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    if ( !navigation_tree_view )
        return -1;

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) );

    if (! gtk_tree_selection_get_selected ( selection, NULL, &iter ) )
        return -1;

    navigation_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( navigation_tree_view ) );
    gtk_tree_model_get ( GTK_TREE_MODEL ( navigation_model ),
                        &iter,
                        NAVIGATION_PAGE, &page,
                        NAVIGATION_ACCOUNT, &account_number,
                        -1 );

    if ( page == GSB_ACCOUNT_PAGE )
        return account_number;

    return -1;
}


/*
 * return the content of buffer_last_account
 * used while changing an account, as long as the work
 * is not finished, that variable contains the last account number
 *
 * \param
 *
 * \return the last account number (become the current account number once the
 *  switch is finished...)
 *
 * */
gint gsb_gui_navigation_get_last_account ( void )
{
    return buffer_last_account;
}


/**
 * Return the number of the current selected report
 *
 * \param
 *
 * \return the current number of the report, or 0 if none selected
 * */
gint gsb_gui_navigation_get_current_report ( void )
{
    GtkWidget *navigation_tree_view;
    GtkTreeSelection *selection;
    GtkTreeModel *navigation_model;
    GtkTreeIter iter;
    gint page;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    if ( !navigation_tree_view )
        return 0;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (navigation_tree_view));

    if (! gtk_tree_selection_get_selected ( selection, NULL, &iter ) )
        return 0;

    navigation_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( navigation_tree_view ) );
    gtk_tree_model_get ( GTK_TREE_MODEL ( navigation_model ),
                        &iter,
                        NAVIGATION_PAGE, &page,
                        -1 );

    if ( page == GSB_REPORTS_PAGE )
    {
        gint report_number;

        gtk_tree_model_get ( GTK_TREE_MODEL ( navigation_model ),
                        &iter,
                        NAVIGATION_REPORT, &report_number,
                        -1 );

        return report_number;
    }

    return -1;
}


/**
 * Iterator that iterates over the navigation pane model and update
 * iter of account that is equal to `data'.
 *
 * \param tree_model    Pointer to the model of the navigation tree.
 * \param path          Not used.
 * \param iter          Current iter to test.
 * \param data          Number of an account to match against the
 *                      NAVIGATION_ACCOUNT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_update_account_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint account_number;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ),
                        iter,
                        NAVIGATION_ACCOUNT, &account_number,
                        -1 );

    if ( account_number == GPOINTER_TO_INT ( data ) )
    {
        gsb_gui_navigation_update_account_iter ( tree_model, iter, GPOINTER_TO_INT ( data ) );
        return TRUE;
    }

    return FALSE;
}



/**
 * Iterator that iterates over the navigation pane model and update
 * iter of report that is equal to `data'.
 *
 * \param tree_model    Pointer to the model of the navigation tree.
 * \param path          Not used.
 * \param iter          Current iter to test.
 * \param data          Number of an report to match against the
 *                      NAVIGATION_REPORT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_update_report_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint report_nb;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ),
                        iter,
                        NAVIGATION_REPORT, &report_nb,
                        -1 );

    if ( report_nb == GPOINTER_TO_INT ( data ) )
    {
        gsb_gui_navigation_update_report_iter ( tree_model, iter, GPOINTER_TO_INT ( data ) );
        return TRUE;
    }

    return FALSE;
}



/**
 * Update information for an report in navigation pane.
 *
 * \param report_nb	Number of the report that has to be updated.
 */
void gsb_gui_navigation_update_report ( gint report_number )
{
    GtkTreeModel *navigation_model;

    navigation_model = gsb_gui_navigation_get_model ();

    gtk_tree_model_foreach ( GTK_TREE_MODEL ( navigation_model ),
                        (GtkTreeModelForeachFunc) gsb_gui_navigation_update_report_iterator,
                        GINT_TO_POINTER ( report_number ) );
}



/**
 * Update contents of an iter with report data.
 *
 * \param model         Pointer to the model of the navigation tree.
 * \param report_iter   Iter to update.
 * \param data          Number of report as a reference.
 */
void gsb_gui_navigation_update_report_iter ( GtkTreeModel *model,
                        GtkTreeIter * report_iter,
                        gint report_number )
{
    gtk_tree_store_set(GTK_TREE_STORE(model), report_iter,
                        NAVIGATION_TEXT, gsb_data_report_get_report_name (report_number),
                        NAVIGATION_PAGE, GSB_REPORTS_PAGE,
                        NAVIGATION_REPORT, report_number,
                        NAVIGATION_ACCOUNT, -1,
                        NAVIGATION_SENSITIVE, 1,
                        -1 );
}



/**
 * Iterator that iterates over the navigation pane model and remove
 * iter of report that is equal to `data'.
 *
 * \param tree_model    Pointer to the model of the navigation tree.
 * \param path          Not used.
 * \param iter          Current iter to test.
 * \param data          Number of an report to match against the
 *                      NAVIGATION_REPORT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_remove_report_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint report;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ), iter,
                        NAVIGATION_REPORT, &report,
                        -1 );

    if ( report == GPOINTER_TO_INT (data))
    {
        gtk_tree_store_remove ( GTK_TREE_STORE ( tree_model ), iter );
        return TRUE;
    }

    return FALSE;
}



/**
 * Add an report to the navigation pane.
 *
 * \param report_nb Report ID to add.
 */
void gsb_gui_navigation_add_report ( gint report_number )
{
    GtkWidget *navigation_tree_view;
    GtkTreeModel *navigation_model;
    GtkTreeIter parent, iter;
    GtkTreeSelection *selection;
    GtkTreePath *path;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    navigation_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( navigation_tree_view ) );

    path = gsb_navigation_view_get_page_path ( navigation_model, GSB_REPORTS_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( navigation_model ), &parent, path );
    gtk_tree_store_append ( GTK_TREE_STORE ( navigation_model ), &iter, &parent );
    gtk_tree_view_expand_to_path ( GTK_TREE_VIEW ( navigation_tree_view ), path );

    gsb_gui_navigation_update_report_iter ( GTK_TREE_MODEL ( navigation_model ), &iter,  report_number );

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) );
    gtk_tree_selection_select_iter ( selection, &iter );
}



/**
 * Remove report from the navigation pane.
 *
 * \param report_nb Report ID to add.
 */
void gsb_gui_navigation_remove_report ( gint report_number )
{
    GtkTreeModel *navigation_model;

    navigation_model = gsb_gui_navigation_get_model ();

    gtk_tree_model_foreach ( GTK_TREE_MODEL ( navigation_model ),
                        (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_report_iterator,
                        GINT_TO_POINTER ( report_number ) );
}



/**
 * Update information for an account in navigation pane.
 *
 * \param account_number    Number of the account that has to be updated.
 */
void gsb_gui_navigation_update_account ( gint account_number )
{
    GtkTreeModel *navigation_model;

    navigation_model = gsb_gui_navigation_get_model ();

    gtk_tree_model_foreach ( GTK_TREE_MODEL ( navigation_model ),
                        (GtkTreeModelForeachFunc) gsb_gui_navigation_update_account_iterator,
                        GINT_TO_POINTER ( account_number ) );
}



/**
 * Update contents of an iter with account data.
 *
 * \param model         Pointer to the model of the navigation tree.
 * \param account_iter  Iter to update.
 * \param data          Number of account as a reference.
 */
void gsb_gui_navigation_update_account_iter ( GtkTreeModel *model,
                        GtkTreeIter *account_iter,
                        gint account_number )
{
    GdkPixbuf * pixbuf = NULL;

    pixbuf = gsb_data_account_get_account_icon_pixbuf ( account_number );

    gtk_tree_store_set ( GTK_TREE_STORE ( model ), account_iter,
                        NAVIGATION_PIX, pixbuf,
                        NAVIGATION_PIX_VISIBLE, TRUE,
                        NAVIGATION_TEXT, gsb_data_account_get_name ( account_number ),
                        NAVIGATION_FONT, 400,
                        NAVIGATION_PAGE, GSB_ACCOUNT_PAGE,
                        NAVIGATION_ACCOUNT, account_number,
                        NAVIGATION_SENSITIVE, !gsb_data_account_get_closed_account ( account_number ),
                        NAVIGATION_REPORT, -1,
                        -1 );
}



/**
 * Iterator that iterates over the navigation pane model and remove
 * iter of account that is equal to `data'.
 *
 * \param tree_model    Pointer to the model of the navigation tree.
 * \param path          Not used.
 * \param iter          Current iter to test.
 * \param data          Number of an account to match against the
 *                      NAVIGATION_ACCOUNT column of current iter.
 *
 * \return TRUE if this iter matches.
 */
static gboolean gsb_gui_navigation_remove_account_iterator ( GtkTreeModel *tree_model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data )
{
    gint account_number;

    gtk_tree_model_get ( GTK_TREE_MODEL ( tree_model ),
                        iter,
                        NAVIGATION_ACCOUNT, &account_number,
                        -1 );

    if ( account_number == GPOINTER_TO_INT ( data ) )
    {
        gtk_tree_store_remove ( GTK_TREE_STORE ( tree_model ), iter );
        return TRUE;
    }

    return FALSE;
}



/**
 * Add an account to the navigation pane.
 *
 * \param account_number    Account ID to add.
 * \param switch_to_account TRUE to show the account, FALSE to just create it
 */
void gsb_gui_navigation_add_account ( gint account_number,
                        gboolean switch_to_account )
{
    GtkTreeModel *navigation_model;
    GtkTreeIter parent, iter;
    GtkTreePath *path;

    navigation_model = gsb_gui_navigation_get_model ();
    path = gsb_navigation_view_get_page_path ( navigation_model, GSB_HOME_PAGE );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL( navigation_model ), &parent, path );
    gtk_tree_store_append ( GTK_TREE_STORE ( navigation_model ), &iter, &parent );

    gsb_gui_navigation_update_account_iter ( GTK_TREE_MODEL ( navigation_model ), &iter, account_number );

    if ( switch_to_account )
    {
        GtkWidget *navigation_tree_view;
        GtkTreeSelection * selection;

        navigation_tree_view = grisbi_window_get_navigation_tree_view ();
        selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( navigation_tree_view ) );
        gtk_tree_selection_select_iter ( selection, &iter );
    }
}



/**
 * change the list of transactions, according to the new account
 *
 * \param no_account a pointer on the number of the account we want to see
 *
 * \return FALSE
 * */
gboolean navigation_change_account ( gint new_account )
{
    gint current_account;
    gchar *tmp_menu_path;
    GrisbiAppConf *conf;

    conf = grisbi_app_get_conf ( );

    devel_debug_int (new_account);

    if ( new_account < 0 )
        return FALSE;

    /* the selection on the navigation bar has already changed, so
     * have to use a buffer variable to get the last account */
    current_account = gsb_gui_navigation_get_last_account ();

    /* sensitive the last account in the menu */
    tmp_menu_path = g_strconcat (
        "/menubar/EditMenu/MoveToAnotherAccount/",
        gsb_data_account_get_name (current_account),
        NULL );
    gsb_gui_sensitive_menu_item ( tmp_menu_path, TRUE );
    g_free ( tmp_menu_path );
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/NewTransaction", TRUE );

    /* save the row_align of the last account */
    gsb_data_account_set_row_align ( current_account,
                        gsb_transactions_list_get_row_align ( ) );

    /* set the appearance of the list according to the new account */
    transaction_list_sort_set_column ( gsb_data_account_get_sort_column ( new_account ),
                        gsb_data_account_get_sort_type ( new_account ) );
    gsb_transactions_list_update_tree_view ( new_account, FALSE );
    transaction_list_select ( gsb_data_account_get_current_transaction_number ( new_account ) );
    gsb_transactions_list_set_row_align ( gsb_data_account_get_row_align ( new_account ) );

    /* mise en place de la date du dernier relevé */
    gsb_navigation_update_statement_label ( new_account );

    tmp_menu_path = g_strconcat (
                        "/menubar/EditMenu/MoveToAnotherAccount/",
                        gsb_data_account_get_name (new_account),
                        NULL );
    gsb_gui_sensitive_menu_item ( tmp_menu_path, FALSE );
    g_free ( tmp_menu_path );

    /* Sensitive menu items if something is selected. */
    if ( gsb_data_account_get_current_transaction_number ( new_account ) == -1 )
        gsb_menu_transaction_operations_set_sensitive ( FALSE );
    else
        gsb_menu_transaction_operations_set_sensitive ( TRUE );

    /* show or hide the rules button in toolbar */
    if ( gsb_data_import_rule_account_has_rule ( new_account ) )
        gtk_widget_show ( menu_import_rules );
    else
        gtk_widget_hide ( menu_import_rules );

    /* Update the title of the file if needed */
    if ( conf->display_grisbi_title == GSB_ACCOUNT_HOLDER )
        grisbi_window_set_active_title ( new_account );

    bet_data_select_bet_pages ( new_account );

    /* unset the last date written */
    gsb_date_free_last_date ();

    return FALSE;
}


/**
 * update the statement label for the given account
 *
 * \param account_number
 *
 * \return
 * */
void gsb_navigation_update_statement_label ( gint account_number )
{
    gint reconcile_number;
    gchar* tmp_str;
    gchar* tmp_str1;
    gchar* tmp_str2;
    gsb_real amount;

    reconcile_number = gsb_data_reconcile_get_account_last_number ( account_number );
    amount = gsb_data_account_get_marked_balance ( account_number );
    if ( reconcile_number )
    {
        tmp_str1 = gsb_format_gdate ( gsb_data_reconcile_get_final_date (
                                            reconcile_number ) );
        tmp_str2 = utils_real_get_string_with_currency ( amount,
                        gsb_data_account_get_currency ( account_number ), TRUE );

        tmp_str = g_strconcat ( _("Last statement: "), tmp_str1, " - ",
                               _("Reconciled balance: "), tmp_str2, NULL );
        gtk_label_set_text ( GTK_LABEL ( label_last_statement ), tmp_str);
        g_free ( tmp_str );
        g_free ( tmp_str1 );
        g_free ( tmp_str2 );
    }
    else if ( amount.mantissa != 0 )
    {
        tmp_str2 = utils_real_get_string_with_currency (amount,
                        gsb_data_account_get_currency ( account_number ), TRUE );

        tmp_str = g_strconcat ( _("Last statement: none"), " - ",
                               _("Reconciled balance: "), tmp_str2, NULL );
        gtk_label_set_text ( GTK_LABEL ( label_last_statement ), tmp_str);
        g_free ( tmp_str );
        g_free ( tmp_str2 );
    }
    else
        gtk_label_set_text ( GTK_LABEL ( label_last_statement ),
                                            _("Last statement: none") );
}


/**
 * Update the account name in the heading bar
 *
 * \param account_number
 *
 * \return
 */
void gsb_navigation_update_account_label ( gint account_number )
{
    gchar * title = NULL;

    title = g_strconcat ( _("Account"), " : ",
                        gsb_data_account_get_name ( account_number ),
                        NULL );
    if ( gsb_data_account_get_closed_account ( account_number ) )
    {
        gchar* old_title = title;

        title = g_strconcat ( title, " (", _("closed"), ")", NULL );
        g_free ( old_title );
    }

    gsb_data_account_colorize_current_balance ( account_number );
    gsb_gui_headings_update_title ( title );

    g_free ( title );
}


/**
 * Remove account from the navigation pane.
 *
 * \param account_number    Account ID to remove.
 */
void gsb_gui_navigation_remove_account ( gint account_number )
{
    GtkTreeModel *navigation_model;

    navigation_model = gsb_gui_navigation_get_model ();
    gtk_tree_model_foreach ( GTK_TREE_MODEL ( navigation_model ),
                        (GtkTreeModelForeachFunc) gsb_gui_navigation_remove_account_iterator,
                        GINT_TO_POINTER ( account_number ) );
}



/**
 * Callback executed when the selection of the navigation tree
 * changed.
 * we must write here the changes to do when changing something in that selection,
 * not with a callback "switch-page" on the main notebook
 *
 * \param selection The selection that triggered event.
 * \param model     Tree model associated to selection.
 *
 * \return FALSE
 */
gboolean gsb_gui_navigation_select_line ( GtkTreeSelection *selection,
                        GtkTreeModel *model )
{
    gint account_number, page_number;
    gint report_number;
    gchar *title = NULL;
    gboolean clear_suffix = TRUE;

    devel_debug (NULL);

    page_number = gsb_gui_navigation_get_current_page ();
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( gsb_gui_get_general_notebook ( ) ), page_number );

    if ( page_number != GSB_ACCOUNT_PAGE )
    {
        gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/NewTransaction", FALSE );
        gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/RemoveAccount", FALSE );
        menus_view_sensitifs ( FALSE );
    }

    if ( page_number != GSB_SCHEDULER_PAGE )
    {
        gtk_widget_hide ( scheduler_calendar );
    }

    switch ( page_number )
    {
    case GSB_HOME_PAGE:
        notice_debug ("Home page selected");

        title = g_strdup(_("My accounts"));

        gsb_gui_sensitive_menu_item ( "/menubar/ViewMenu/ShowClosed", TRUE );

        /* what to be done if switch to that page */
        mise_a_jour_accueil ( FALSE );
        gsb_form_set_expander_visible ( FALSE, FALSE );
        break;

    case GSB_ACCOUNT_PAGE:
        notice_debug ("Account page selected");

        menus_view_sensitifs ( TRUE );
        gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/RemoveAccount", TRUE );

        account_number = gsb_gui_navigation_get_current_account ();

        /* update title now -- different from others */
        gsb_navigation_update_account_label (account_number);

        /* what to be done if switch to that page */
        if (account_number >= 0 )
        {
            navigation_change_account ( account_number );
            gsb_account_property_fill_page ();
            clear_suffix = FALSE;
            if ( gsb_data_archive_store_account_have_transactions_visibles ( account_number ) )
                gsb_transaction_list_set_visible_archived_button ( TRUE );
            else
                gsb_transaction_list_set_visible_archived_button ( FALSE );
        }
        gsb_menu_update_accounts_in_menus ();
        gsb_menu_update_view_menu ( account_number );

        /* set the form */
        gsb_gui_on_account_switch_page ( GTK_NOTEBOOK ( gsb_gui_on_account_get_notebook ( ) ),
                        NULL,
                        gtk_notebook_get_current_page ( GTK_NOTEBOOK ( gsb_gui_on_account_get_notebook ( ) ) ),
                        NULL );
        /* gsb_form_show ( FALSE ); */

        buffer_last_account = account_number;

        break;

    case GSB_SCHEDULER_PAGE:
        notice_debug ("Scheduler page selected");

        title = g_strdup(_("Scheduled transactions"));

        /* what to be done if switch to that page */
        /* update the list (can do that because short list, so very fast) */
        gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
        gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());

        gsb_scheduler_list_select (gsb_scheduler_list_get_last_scheduled_number ());

        /* set the form */
        gsb_form_set_expander_visible (TRUE, FALSE );
        gsb_form_scheduler_clean ();
        gsb_form_show ( FALSE );

        /* show the calendar */
        gsb_calendar_update ();
        gtk_widget_show_all ( scheduler_calendar );

        /* show menu InitwidthCol */
        gsb_gui_sensitive_menu_item ( "/menubar/ViewMenu/InitwidthCol", TRUE );
        break;

    case GSB_PAYEES_PAGE:
        notice_debug ("Payee page selected");

        /* what to be done if switch to that page */
        gsb_form_set_expander_visible (FALSE, FALSE );
        payees_fill_list ();
        clear_suffix = FALSE;
        break;

    case GSB_SIMULATOR_PAGE:
        notice_debug ("Credits simulator page selected");

        title = g_strdup(_("Credits simulator"));

        /* what to be done if switch to that page */
        gsb_form_set_expander_visible (FALSE, FALSE);
        bet_finance_switch_simulator_page ( );
        break;

    case GSB_CATEGORIES_PAGE:
        notice_debug ("Category page selected");

        /* what to be done if switch to that page */
        gsb_form_set_expander_visible (FALSE, FALSE );
        categories_fill_list ();
        clear_suffix = FALSE;
        break;

    case GSB_BUDGETARY_LINES_PAGE:
        notice_debug ("Budgetary page selected");

        /* what to be done if switch to that page */
        gsb_form_set_expander_visible (FALSE, FALSE );
        budgetary_lines_fill_list ();
        clear_suffix = FALSE;
    break;

    case GSB_REPORTS_PAGE:
        notice_debug ("Reports page selected");

        report_number = gsb_gui_navigation_get_current_report ();

        if ( report_number >= 0 )
            title = g_strconcat ( _("Report"), " : ", gsb_data_report_get_report_name (report_number), NULL );
        else
            title = g_strdup(_("Reports"));

        /* what to be done if switch to that page */
        gsb_form_set_expander_visible ( FALSE, FALSE );

        if ( report_number > 0 )
            gsb_gui_update_gui_to_report ( report_number );
        else
            gsb_gui_unsensitive_report_widgets ();
        break;

    default:
        notice_debug ("B0rk page selected");
        title = g_strdup("B0rk");
        break;
    }

    /* title is set here if necessary */
    if (title)
    {
        gsb_gui_headings_update_title ( title );
        g_free ( title );
    }
    if (clear_suffix)
        gsb_gui_headings_update_suffix ( "" );

    return FALSE;
}



/**
 * select l'item précédent d'un model
 *
 * \param
 *
 * \return
 **/
gboolean gsb_gui_navigation_select_prev ( void )
{
    GtkWidget *navigation_tree_view;
    GtkTreeSelection * selection;
    GtkTreePath * path;
    GtkTreeModel * model;
    GtkTreeIter iter;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    g_return_val_if_fail ( selection, FALSE );

    if ( !gtk_tree_selection_get_selected ( selection, &model, &iter ) )
        return TRUE;
    path = gtk_tree_model_get_path ( model, &iter );
    g_return_val_if_fail ( path, TRUE );

    if ( !gtk_tree_path_prev ( path ) )
    {
        if ( gtk_tree_path_get_depth ( path ) > 1 )
            gtk_tree_path_up ( path );
    }
    else
    {
        gtk_tree_model_get_iter ( model, &iter, path );
        /* if row has children and if row is expanded, go to the last child */
        if ( gtk_tree_model_iter_has_child ( model, &iter )
             && gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( navigation_tree_view ), path ) )
        {
            GtkTreeIter parent = iter;

            gtk_tree_model_iter_nth_child ( model, &iter, &parent,
                                            gtk_tree_model_iter_n_children ( model,
                                                                             &parent ) - 1 );
            path = gtk_tree_model_get_path ( model, &iter );
        }
    }

    gtk_tree_selection_select_path ( selection, path );
    gtk_tree_path_free ( path );

    return FALSE;
}



/**
 * return l'item suivant d'un model
 *
 * \param
 *
 * \return gboolean
 **/
gboolean gsb_gui_navigation_select_next ( void )
{
    GtkWidget *navigation_tree_view;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeIter iter;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW(navigation_tree_view) );
    g_return_val_if_fail ( selection, FALSE );

    if ( !gtk_tree_selection_get_selected ( selection, &model, &iter ) )
        return TRUE;

    path = gtk_tree_model_get_path ( model, &iter );
    g_return_val_if_fail ( path, TRUE );

    if ( gtk_tree_model_iter_has_child ( model, &iter ) )
    {
        if ( gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( navigation_tree_view ), path ) )
            gtk_tree_path_down ( path );
        else
            gtk_tree_path_next ( path );
    }
    else
    {
        if ( !gtk_tree_model_iter_next ( model, &iter ) )
        {
            if ( gtk_tree_path_get_depth ( path ) > 1 )
            {
                gtk_tree_path_up ( path );
                gtk_tree_path_next ( path );
            }
            else
            {
                gtk_tree_path_free ( path );
                path = gtk_tree_path_new_first ( );
            }
        }
        else
        {
            path = gtk_tree_model_get_path ( model, &iter );
        }
    }

    gtk_tree_selection_select_path ( selection, path );
    gtk_tree_path_free ( path );

    return FALSE;
}




/**
 * Met à jour la page d'accueil immédiatement si elle est affichée sinon plus tard
 *
 */
void gsb_gui_navigation_update_home_page ( void )
{
    if ( gsb_gui_navigation_get_current_page ( ) == GSB_HOME_PAGE )
        mise_a_jour_accueil ( TRUE );
    else
        mise_a_jour_liste_comptes_accueil = TRUE;
}


/**
 *
 *
 *
 */
GtkWidget *gsb_gui_navigation_get_tree_view ( void )
{
    GtkWidget *navigation_tree_view;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();

    return navigation_tree_view;
}


/**
 *
 *
 *
 */
GtkTreeModel *gsb_gui_navigation_get_model ( void )
{
    GtkWidget *navigation_tree_view;
    GtkTreeModel *navigation_model;

    navigation_tree_view = grisbi_window_get_navigation_tree_view ();
    navigation_model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( navigation_tree_view ) );

    if ( navigation_model )
        return navigation_model;
    else
        return NULL;
}


/**
 * Pop up a menu with several actions to apply to array_list.
 *
 * \param gtk_tree_view
 *
 */
void gsb_gui_navigation_context_menu ( GtkWidget *tree_view,
                        GtkTreePath *path )
{
    GtkWidget *image;
    GtkWidget *menu = NULL;
    GtkWidget *menu_item;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *tmp_str;
    gint type_page;
    gint account_number;
    gint report_number;

    model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

    gtk_tree_model_get_iter ( model, &iter, path );
    gtk_tree_model_get ( model, &iter,
                        NAVIGATION_PAGE, &type_page,
                        NAVIGATION_ACCOUNT, &account_number,
                        NAVIGATION_REPORT, &report_number,
                        -1 );

    switch ( type_page )
    {
        case GSB_HOME_PAGE :
        case GSB_ACCOUNT_PAGE :
            menu = gtk_menu_new ();
            menu_item = gtk_image_menu_item_new_with_label ( _("New account") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_NEW, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_assistant_account_run ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            if ( account_number == -1 )
                break;

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = gtk_image_menu_item_new_with_label ( _("Remove this account") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( gsb_account_delete ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        break;
        case GSB_PAYEES_PAGE :
            menu = gtk_menu_new ();
            tmp_str = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "new-payee.png", NULL);
            image = gtk_image_new_from_file ( tmp_str );
            g_free ( tmp_str );
            menu_item = gtk_image_menu_item_new_with_label ( _("New payee") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), image );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_new_payee ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( _("Delete selected payee") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_delete_payee ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( payees_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            menu_item = gtk_image_menu_item_new_with_label ( _("Edit selected payee") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_edit_payee ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( payees_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            tmp_str = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "new-payee.png", NULL);
            image = gtk_image_new_from_file ( tmp_str );
            g_free ( tmp_str );
            menu_item = gtk_image_menu_item_new_with_label ( _("Manage payees") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), image );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_manage_payees ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( _("Remove unused payees") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( payees_remove_unused_payees ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
        break;
        case GSB_CATEGORIES_PAGE :
            menu = gtk_menu_new ();
            tmp_str = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "new-categ.png", NULL);
            image = gtk_image_new_from_file ( tmp_str );
            g_free ( tmp_str );
            menu_item = gtk_image_menu_item_new_with_label ( _("New category") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), image );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_new_category ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( _("Delete selected category") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_delete_category ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( categories_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            menu_item = gtk_image_menu_item_new_with_label ( _("Edit selected category") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_edit_category ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( categories_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = gtk_image_menu_item_new_with_label ( _("Import a file of categories (.cgsb)") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_NEW, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_importer_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( _("Export the list of categories (.cgsb)") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_NEW, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( categories_exporter_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
        break;
        case GSB_BUDGETARY_LINES_PAGE :
            menu = gtk_menu_new ();
            tmp_str = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "new-ib.png", NULL);
            image = gtk_image_new_from_file ( tmp_str );
            g_free ( tmp_str );
            menu_item = gtk_image_menu_item_new_with_label ( _("New budgetary line") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), image );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_new_budgetary_line ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( _("Delete selected budgetary line") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_delete_budgetary_line ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( budgetary_lines_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            menu_item = gtk_image_menu_item_new_with_label ( _("Edit selected budgetary line") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_edit_budgetary_line ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            if ( gtk_tree_selection_count_selected_rows ( gtk_tree_view_get_selection (
             GTK_TREE_VIEW ( budgetary_lines_get_tree_view ( ) ) ) ) )
                gtk_widget_set_sensitive ( menu_item, TRUE );
            else
                gtk_widget_set_sensitive ( menu_item, FALSE );

            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = gtk_image_menu_item_new_with_label ( _("Import a file of budgetary lines (.igsb)") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_NEW, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_importer_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

            menu_item = gtk_image_menu_item_new_with_label ( _("Export the list of budgetary lines (.igsb)") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_NEW, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( budgetary_lines_exporter_list ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
        break;
        case GSB_REPORTS_PAGE :
            menu = gtk_menu_new ();
            tmp_str = g_build_filename ( gsb_dirs_get_pixmaps_dir ( ), "new-report.png", NULL);
            image = gtk_image_new_from_file ( tmp_str );
            g_free ( tmp_str );
            menu_item = gtk_image_menu_item_new_with_label ( _("New report") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ), image );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( ajout_etat ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );
            if ( report_number == -1 )
                break;
            /* Separator */
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), gtk_separator_menu_item_new() );

            menu_item = gtk_image_menu_item_new_with_label ( _("Remove this report") );
            gtk_image_menu_item_set_image ( GTK_IMAGE_MENU_ITEM ( menu_item ),
                        gtk_image_new_from_stock ( GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU ) );
            g_signal_connect ( G_OBJECT ( menu_item ),
                        "activate",
                        G_CALLBACK ( efface_etat ),
                        NULL );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), menu_item );

        break;
    }

    /* Finish all. */
    if ( menu )
    {
        gtk_widget_show_all ( menu );
        gtk_menu_popup ( GTK_MENU( menu ), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time ( ) );
    }
}


/**
 *
 *
 *
 */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
