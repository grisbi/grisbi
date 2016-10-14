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
#include "grisbi_win.h"
#include "gsb_account_property.h"
#include "gsb_data_account.h"
#include "gsb_form.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_buttons.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** Notebook of the account pane. */
static GtkWidget *account_page = NULL;

 /**
 *
 *
 */
GtkWidget *gsb_gui_get_general_notebook (void )
{
    GtkWidget *notebook;

    notebook = grisbi_win_get_notebook_general (grisbi_app_get_active_window (NULL));

    return notebook;
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
    /* append the main page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        creation_onglet_accueil(),
                        gtk_label_new ( _("Main page") ) );

    /* append the account page : a notebook with the account configuration
     * the bet pages and transactions page */
    account_page = gtk_notebook_new ();
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(account_page), TRUE );
    gtk_widget_show ( account_page );

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

    g_signal_connect ( G_OBJECT ( account_page ),
                        "switch-page",
                        G_CALLBACK ( gsb_gui_on_account_switch_page ),
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
    gsb_gui_update_all_toolbars ( );

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

    grisbi_win_menu_move_to_acc_update  ( FALSE );
    if ( page_number != GSB_TRANSACTIONS_PAGE )
    {
        gsb_menu_set_menus_view_account_sensitive ( FALSE );
        gsb_menu_gui_sensitive_win_menu_item ( "new-ope", FALSE );
    }

    switch ( page_number )
    {
    case GSB_TRANSACTIONS_PAGE:
        gsb_form_set_expander_visible ( TRUE, TRUE );
        gsb_menu_set_menus_view_account_sensitive ( TRUE );
        gsb_menu_gui_sensitive_win_menu_item ( "new-ope", TRUE );
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
 * Set the account notebook page.
 *
 * \param page		Page to set.
 */
void gsb_gui_on_account_change_page ( GsbAccountNotebookPages page )
{
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( account_page ), page );
}

/**
 * Set the main notebook page.
 *
 * \param page		Page to set.
 */
void gsb_gui_notebook_change_page ( GsbGeneralNotebookPages page )
{
    GtkWidget *notebook;

    notebook = grisbi_win_get_notebook_general (grisbi_app_get_active_window (NULL));
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page);
}

/**
 *
 *
 *
 */
void gsb_gui_update_all_toolbars ( void )
{
    gint toolbar_style = 0;

    switch ( conf.display_toolbar )
    {
        case GSB_BUTTON_TEXT:
            toolbar_style = GTK_TOOLBAR_TEXT;
            break;
        case GSB_BUTTON_ICON:
            toolbar_style = GTK_TOOLBAR_ICONS;
            break;
        case GSB_BUTTON_BOTH:
            toolbar_style = GTK_TOOLBAR_BOTH;
            break;
        case GSB_BUTTON_BOTH_HORIZ:
            toolbar_style = GTK_TOOLBAR_BOTH_HORIZ;
            break;
    }

    gsb_gui_transaction_toolbar_set_style ( toolbar_style );
    gsb_gui_scheduler_toolbar_set_style ( toolbar_style );
    gsb_gui_payees_toolbar_set_style ( toolbar_style );
    gsb_gui_categories_toolbar_set_style ( toolbar_style );
    gsb_gui_budgetary_lines_toolbar_set_style ( toolbar_style );
    gsb_gui_reports_toolbar_set_style ( toolbar_style );
    bet_array_update_toolbar ( toolbar_style );
    bet_historical_update_toolbar ( toolbar_style );
    bet_finance_update_all_finance_toolbars ( toolbar_style );
}

GtkWidget *gsb_gui_get_account_page ( void )
{
    return account_page;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
