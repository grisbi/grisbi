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
#include "gsb_data_account.h"
#include "gsb_account_property.h"
#include "gsb_form.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "imputation_budgetaire.h"
#include "main.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *gsb_gui_create_general_notebook (void );
static gboolean gsb_gui_fill_general_notebook ( GtkWidget *notebook );
static void gsb_gui_headings_private_update_label_markup ( GtkLabel *label,
                        const gchar *text,
                        gboolean escape_text );
static gboolean gsb_gui_hpaned_size_allocate ( GtkWidget *hpaned,
                        GtkAllocation *allocation,
                        gpointer null );
static gboolean on_simpleclick_event_run ( GtkWidget * button, GdkEvent * button_event,
                        GCallback cb );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/* données des widgets généraux */
static GtkWidget *vbox_general = NULL;
static GtkWidget *hpaned_general = NULL;
static GtkWidget *notebook_general = NULL;
static gint hpaned_left_width;

/** Notebook of the account pane. */
GtkWidget *account_page = NULL;

/**
 * Create the main widget that holds all the user interface save the
 * menus.
 *
 * \return A newly-allocated vbox holding all elements.
 */
GtkWidget *gsb_gui_create_general_widgets ( void )
{
    /* All stuff will be put in a huge vbox, with an hbox containing
     * quick summary. */
    vbox_general = gtk_vbox_new ( FALSE, 0 );

/*     headings_eb = gtk_event_box_new ();
 *     style = gtk_widget_get_style ( headings_eb );
 * 
 *     hbox = gtk_hbox_new ( FALSE, 0 );
 */

    /* Create two arrows. */
/*     arrow_left = gtk_arrow_new ( GTK_ARROW_LEFT, GTK_SHADOW_OUT );
 *     arrow_eb = gtk_event_box_new ();
 *     gtk_widget_modify_bg ( arrow_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
 *     gtk_container_add ( GTK_CONTAINER ( arrow_eb ), arrow_left );
 */
/*     g_signal_connect ( G_OBJECT ( arrow_eb ), "button-press-event", 
 * 		       G_CALLBACK ( on_simpleclick_event_run ), 
 * 		       gsb_gui_navigation_select_prev );
 */
/*     gtk_box_pack_start ( GTK_BOX ( hbox ), arrow_eb, FALSE, FALSE, 0 );  */

/*     arrow_right = gtk_arrow_new ( GTK_ARROW_RIGHT, GTK_SHADOW_OUT );
 *     arrow_eb = gtk_event_box_new ();
 *     gtk_widget_modify_bg ( arrow_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
 *     gtk_container_add ( GTK_CONTAINER ( arrow_eb ), arrow_right );
 */
/*     g_signal_connect ( G_OBJECT ( arrow_eb ), "button-press-event", 
 * 		       G_CALLBACK ( on_simpleclick_event_run ), 
 * 		       gsb_gui_navigation_select_next );
 */
/*     gtk_box_pack_start ( GTK_BOX(hbox), arrow_eb, FALSE, FALSE, 3 );  */

    /* Define labels. */
/*     headings_title = gtk_label_new ( NULL );
 *     gtk_label_set_justify ( GTK_LABEL(headings_title), GTK_JUSTIFY_LEFT );
 *     gtk_misc_set_alignment (GTK_MISC (headings_title), 0.0, 0.5);
 *     gtk_box_pack_start ( GTK_BOX(hbox), headings_title, TRUE, TRUE, 3 );
 *     headings_suffix = gtk_label_new (NULL);
 *     gtk_box_pack_start ( GTK_BOX(hbox), headings_suffix, FALSE, FALSE, 0 );
 */

    /* Change color with an event box trick. */
/*     gtk_container_add ( GTK_CONTAINER ( headings_eb ), hbox );
 *     gtk_widget_modify_bg ( headings_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
 *     gtk_container_set_border_width ( GTK_CONTAINER ( hbox ), 6 );
 * 
 *     gtk_box_pack_start ( GTK_BOX( vbox_general ), headings_eb, FALSE, FALSE, 0 );
 */
/*     gsb_gui_update_show_headings ();  */

    /* Then create and fill the main hpaned. */
    hpaned_general = gtk_hpaned_new ();
    g_signal_connect ( G_OBJECT ( hpaned_general ),
		       "size_allocate",
		       G_CALLBACK ( gsb_gui_hpaned_size_allocate ),
		       NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox_general ), hpaned_general, TRUE, TRUE, 0 );
    gtk_paned_add1 ( GTK_PANED ( hpaned_general ), gsb_gui_navigation_create_navigation_pane ( ) );
    gtk_paned_add2 ( GTK_PANED ( hpaned_general ), gsb_gui_create_general_notebook ( ) );
    gtk_container_set_border_width ( GTK_CONTAINER ( hpaned_general ), 6 );
    if ( hpaned_left_width == -1 )
    {
        gint width, height;

        gtk_window_get_size ( GTK_WINDOW ( grisbi_app_get_active_window ( NULL ) ), &width, &height );
        gtk_paned_set_position ( GTK_PANED ( hpaned_general ), (gint) width / 4 );
    }
    else
    {
        if ( hpaned_left_width )
            gtk_paned_set_position ( GTK_PANED ( hpaned_general ), hpaned_left_width );
        else
            gtk_paned_set_position ( GTK_PANED ( hpaned_general ), 1 );
    }

    gtk_widget_show ( hpaned_general );

    gtk_widget_show ( vbox_general );

    
    return vbox_general;
}



/**
 * Create the main notebook : 
 * a notebook wich contains the pages : main page, accounts, scheduler... and
 * the form on the bottom, the form will be showed only for accounts page and
 * scheduler page
 *
 * \return the notebook
 */
GtkWidget *gsb_gui_create_general_notebook (void )
{
    GtkWidget * vbox, * form;

    devel_debug ( "create_main_notebook" );

    /* the main right page is a vbox with a notebook on the top
     * and the form on the bottom */

    vbox = gtk_vbox_new ( FALSE, 0 );

    /* append the notebook */
    notebook_general = gtk_notebook_new ( );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook_general ), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK ( notebook_general ), FALSE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), notebook_general, TRUE, TRUE, 0 );
    gtk_widget_show ( notebook_general );

    /* append the form */
    form = gsb_form_new ( );
    gtk_box_pack_start ( GTK_BOX ( vbox ), form, FALSE, FALSE, 0 );
    gtk_widget_hide ( form );

    /* fill the notebook */
    gsb_gui_fill_general_notebook ( notebook_general );

    gtk_widget_show ( vbox );

    return vbox;
}


/**
 *
 *
 */
GtkWidget *gsb_gui_get_general_notebook (void )
{
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
    /* append the main page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
                        creation_onglet_accueil(),
                        gtk_label_new ( _("Main page") ) );

    /* append the account page : a notebook with the account configuration
     * the bet pages and transactions page */
    account_page = gtk_notebook_new ();
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(account_page), FALSE );
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
 * Set the account notebook page.
 * 
 * \param page		Page to set.
 */
void gsb_gui_on_account_change_page ( GsbaccountNotebookPages page )
{
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
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook_general ), page );
}


/**
 *
 *
 *
 */
gboolean gsb_gui_hpaned_size_allocate ( GtkWidget *hpaned,
                        GtkAllocation *allocation,
                        gpointer null )
{
    hpaned_left_width = gtk_paned_get_position ( GTK_PANED ( hpaned ) );
    
    return FALSE;
}


/**
 * initialise notebook_general, hpaned_general et vbox_general
 *
 *
 */
void gsb_gui_init_general_vbox ( void )
{
    if ( vbox_general )
    {
        gtk_widget_destroy ( vbox_general );
        vbox_general = NULL;
    }
}


/**
 * initialise notebook_general, hpaned_general et vbox_general
 *
 *
 */
void gsb_gui_init_general_notebook ( void )
{
        notebook_general = NULL;
}


/**
 * test l'existence de hpaned_general
 *
 *
 */
gboolean gsb_gui_is_hpaned_general ( void )
{
    if ( hpaned_general && GTK_IS_WIDGET ( hpaned_general ) )
        return TRUE;
    else
        return FALSE;
}


/**
 * renvoie la largeur de la partie gauche du hpaned
 *
 *
 */
gint gsb_gui_get_hpaned_left_width ( void )
{
    return hpaned_left_width;
}


/**
 * fixe la largeur de la partie gauche du hpaned
 *
 *
 */
gboolean gsb_gui_set_hpaned_left_width ( gint width )
{
    hpaned_left_width = width;

    return TRUE;
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


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
