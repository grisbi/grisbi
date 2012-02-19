/* ************************************************************************** */
/*  Fichier qui s'occupe de former les différentes fenêtres de travail        */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2005-2008 Benjamin Drieu (bdrieu@april.org)                       */
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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "fenetre_principale.h"
#include "accueil.h"
#include "bet_data.h"
#include "bet_data_finance.h"
#include "bet_finance_ui.h"
#include "bet_hist.h"
#include "bet_tab.h"
#include "categories_onglet.h"
#include "etats_onglet.h"
#include "grisbi_app.h"
#include "grisbi_window.h"
#include "gsb_data_account.h"
#include "gsb_account_property.h"
#include "gsb_form.h"
#include "gsb_scheduler_list.h"
#include "gsb_status.h"
#include "gsb_transactions_list.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *creation_fenetre_operations ( void );
static void gsb_gui_create_general_notebook ( GrisbiWindow *window );
static gboolean gsb_gui_fill_general_notebook ( GtkWidget *notebook );
static void gsb_gui_headings_private_update_label_markup ( GtkLabel *label,
                        const gchar *text,
                        gboolean escape_text );
static gboolean on_simpleclick_event_run ( GtkWidget * button, GdkEvent * button_event,
                        GCallback cb );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * Initialize user interface part.
 *
 * \param
 *
 * \return
 */
void gsb_gui_new_gui ( void )
{
    GtkWidget *main_vbox;
    GtkWidget *general_widget;
    GtkWidget *notebook_general;

    /* dégrise les menus nécessaire */
    gsb_menu_sensitive ( TRUE );

    /* Create main widget. */
    gsb_status_message ( _("Creating main window") );
    main_vbox = grisbi_window_get_widget_by_name ( "main_vbox" );

    /* affiche headings_bar si nécessaire */
    gsb_gui_update_show_headings ( );

    /* récupération du widget general */
    general_widget = grisbi_window_new_general_widget ();
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), general_widget, TRUE, TRUE, 0 );

    navigation_change_account ( gsb_gui_navigation_get_current_account () );

    /* Display accounts in menus */
    gsb_menu_update_accounts_in_menus ();

    notebook_general = grisbi_window_get_widget_by_name ( "notebook_general" );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK( notebook_general ), GSB_HOME_PAGE );

    /* return */
}


/**
 * Create a new general notebook
 *
 * \param
 *
 * \return
 */
void gsb_gui_new_general_notebook ( void )
{
    GrisbiWindow *window;

    window = grisbi_app_get_active_window ( grisbi_app_get_default ( ) );

    gsb_gui_create_general_notebook ( window );
}



/**
 * Create the main notebook : 
 * a notebook wich contains the pages : main page, accounts, scheduler... and
 * the form on the bottom, the form will be showed only for accounts page and
 * scheduler page
 *
 * \return the notebook
 */
void gsb_gui_create_general_notebook ( GrisbiWindow *window )
{
    GtkWidget *vbox;
/*     GtkWidget *form;  */
    GtkWidget *notebook_general;

    devel_debug ( "create_main_notebook" );

    /* the main right page is a vbox with a notebook on the top
     * and the form on the bottom */
    vbox = grisbi_window_get_widget_by_name ( "vbox_transactions_list" );

    /* append the notebook */
    notebook_general = grisbi_window_get_widget_by_name ( "notebook_general" );

    /* append the form */
/*     form = gsb_form_new ( );
 *     gtk_box_pack_start ( GTK_BOX ( vbox ), form, FALSE, FALSE, 0 );
 *     gtk_widget_hide ( form );
 */

    /* fill the notebook */
    gsb_gui_fill_general_notebook ( notebook_general );

    /* return */
}


/**
 *
 *
 */
GtkWidget *gsb_gui_get_general_notebook (void )
{
    GtkWidget *notebook_general;

    notebook_general = grisbi_window_get_widget_by_name ( "notebook_general" );

    return notebook_general;
}


/**
 * fill the notebook given in param
 *
 * \param notebook a notebook (usually the main_notebook...)
 *
 * \return FALSE
 * */
gboolean gsb_gui_fill_general_notebook ( GtkWidget *notebook )
{
    GtkWidget *account_page;

    /* append the main page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        creation_onglet_accueil(),
                        gtk_label_new ( _("Main page") ) );

    /* append the account page : a notebook with the account configuration
     * the bet pages and transactions page */
    account_page = grisbi_window_get_widget_by_name ( "account_page" );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        account_page,
                        gtk_label_new ( _("Accounts") ) );
   
    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ),
                        creation_fenetre_operations (),
                        gtk_label_new ( _("Transactions") ) );

    /* append the balance estimate pages */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ),
                        bet_array_create_page ( ),
                        gtk_label_new ( _("Forecast") ) );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ),
                        bet_historical_create_page ( ),
                        gtk_label_new ( _("Historical data") ) );

    /* append the amortization page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ),
                        bet_finance_create_account_page ( ),
                        gtk_label_new ( _("Amortization array") ) );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ),
                        gsb_account_property_create_page (),
                        gtk_label_new ( _("Properties") ) );

    g_signal_connect ( G_OBJECT (account_page),
                        "switch_page",
                        G_CALLBACK (gsb_gui_on_account_switch_page),
                        NULL );

    /* append the scheduled transactions page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        gsb_scheduler_list_create_list (),
                        gtk_label_new ( _("Scheduler") ) );

    /* append the payee page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        payees_create_list ( ),
                        gtk_label_new ( _("Payee") ) );

    /* append the financial page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        bet_finance_create_page (),
                        gtk_label_new ( _("Credits simulator") ) );

    /* append the categories page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        categories_create_list ( ),
                        gtk_label_new ( _("Categories") ) );

    /* append the budget page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        budgetary_lines_create_list ( ),
                        gtk_label_new ( _("Budgetary lines") ) );

    /* append the reports page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        creation_onglet_etats (),
                        gtk_label_new ( _("Reports") ) );

    /* update toolbars */
/*     gsb_gui_update_all_toolbars ( );  */

    /* return */
    return FALSE;
}


/**
 * called when the account notebook changed page between
 * transactions list and account description
 *
 * \param notebook	Widget that triggered event.
 * \param page		Not used.
 * \param page_number	Page set.
 * \param null		Not used.
 *
 * \return		FALSE
 */
gboolean gsb_gui_on_account_switch_page ( GtkNotebook *notebook,
                        gpointer page,
                        guint page_number,
                        gpointer null )
{
    gint account_number;

    /* devel_debug_int (page_number); */
    switch ( page_number )
    {
    case GSB_TRANSACTIONS_PAGE:
        gsb_form_set_expander_visible ( TRUE, TRUE );
        break;
    case GSB_ESTIMATE_PAGE:
        gsb_form_set_expander_visible (FALSE, FALSE );
        account_number = gsb_gui_navigation_get_current_account ( );
        if ( gsb_data_account_get_bet_maj ( account_number ) )
            bet_data_update_bet_module ( account_number, GSB_ESTIMATE_PAGE );
        break;
    case GSB_HISTORICAL_PAGE:
        gsb_form_set_expander_visible (FALSE, FALSE );
        account_number = gsb_gui_navigation_get_current_account ( );
        if ( gsb_data_account_get_bet_maj ( account_number ) )
            bet_data_update_bet_module ( account_number, GSB_HISTORICAL_PAGE );
        bet_historical_set_page_title ( account_number );
        break;
    case GSB_FINANCE_PAGE:
        gsb_form_set_expander_visible (FALSE, FALSE );
        account_number = gsb_gui_navigation_get_current_account ( );
        bet_finance_ui_update_amortization_tab ( account_number );
    case GSB_PROPERTIES_PAGE:
        gsb_form_set_expander_visible (FALSE, FALSE );
        break;
    }

    return ( FALSE );
}


/**
 * Get the account notebook.
 *
 * \param page
 *
 * \return 
 */
GtkWidget *gsb_gui_on_account_get_notebook ( void )
{
    GtkWidget *account_page;

    account_page = grisbi_window_get_widget_by_name ( "account_page" );

    return account_page;
}


/**
 * Set the account notebook page.
 * 
 * \param page		Page to set.
 */
void gsb_gui_on_account_change_page ( GsbaccountNotebookPages page )
{
    GtkWidget *account_page;

    account_page = grisbi_window_get_widget_by_name ( "account_page" );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), page );
}


/**
 * Update headings bar with a new title.
 *
 * \param title		String to display as a title in headings bar.
 *
 */
void gsb_gui_headings_update_title ( gchar * title )
{
    grisbi_window_headings_update_label_markup ( "headings_title", title, TRUE );
}

/**
 * Update headings bar with a new suffix.
 *
 * \param suffix	String to display as a suffix in headings bar.
 *
 */
void gsb_gui_headings_update_suffix ( gchar * suffix )
{
    grisbi_window_headings_update_label_markup ( "headings_suffix", suffix, FALSE );
}




/**
 * Display or hide the headings bar depending on configuration.
 *
 * return		FALSE
 */
gboolean gsb_gui_update_show_headings ( void )
{
    GrisbiAppConf *conf;
    GrisbiWindow *window;

    conf = grisbi_app_get_conf ( );
    window = grisbi_app_get_active_window ( NULL );

    if ( conf->show_headings_bar )
    {
        gtk_widget_show_all ( grisbi_window_get_headings_eb ( window ) );
    }
    else
    {
        gtk_widget_hide ( grisbi_window_get_headings_eb ( window ) );
    }
    return FALSE;
}

/**
 * sensitive or unsensitive the headings
 *
 * \param sensitive TRUE to sensitive
 *
 * \return
 * */
void gsb_gui_sensitive_headings ( gboolean sensitive )
{
    GrisbiWindow *window;

    window = grisbi_app_get_active_window ( NULL );

    gtk_widget_set_sensitive ( grisbi_window_get_headings_eb ( window ), sensitive );
}


/**
 * Set the main notebook page.
 * 
 * \param page		Page to set.
 */
void gsb_gui_notebook_change_page ( GsbGeneralNotebookPages page )
{
    GtkWidget *notebook_general;

    notebook_general = gsb_gui_get_general_notebook ( );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), page );
}


/**
 *
 *
 *
 */
void gsb_gui_update_all_toolbars ( void )
{
    gsb_gui_update_transaction_toolbar ( );
    gsb_gui_update_scheduler_toolbar ( );
    payees_update_toolbar_list ( );
    categories_update_toolbar_list ( );
    budgetary_lines_update_toolbar_list ( );
    gsb_gui_update_reports_toolbar ( );
    bet_array_update_toolbar ( );
    bet_historical_update_toolbar ( );
    bet_finance_update_all_finance_toolbars ( );
}


/**
 * Create the transaction window with all components needed.
 *
 *
 */
GtkWidget *creation_fenetre_operations ( void )
{
    GtkWidget *win_operations;

    /*   la fenetre des opé est une vbox : la liste en haut, le solde et  */
    /*     des boutons de conf au milieu, le transaction_form en bas */
    win_operations = grisbi_window_get_widget_by_name ( "win_operations" );

    return ( win_operations );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
