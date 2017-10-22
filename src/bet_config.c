/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2012 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include "config.h"
#endif

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "bet_config.h"
#include "bet_data.h"
#include "bet_data_finance.h"
#include "bet_finance_ui.h"
#include "bet_hist.h"
#include "bet_tab.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_account.h"
#include "gsb_automem.h"
#include "gsb_calendar_entry.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_file.h"
#include "gsb_form_widget.h"
#include "gsb_fyear.h"
#include "gsb_rgba.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean bet_config_change_account ( GtkWidget *combo );
static gint bet_config_get_account_from_combo ( void );
static gboolean bet_config_general_cash_account_option_clicked ( GtkWidget *checkbutton,
                        gpointer null );
static GtkWidget *bet_config_general_get_period_widget ( GtkWidget *container );
static GtkWidget *bet_config_account_get_finance_data ( gchar *title );
static GtkWidget *bet_config_get_finance_widget ( GtkWidget *parent );
static GtkWidget *bet_config_account_get_forecast_data ( gchar *title );
static GtkWidget *bet_config_account_get_select_account ( gchar *title );
static GtkWidget *bet_config_account_get_select_bank_card ( void );
static GtkWidget *bet_config_account_get_select_historical_data ( gchar *title );
static void bet_config_finance_apply_clicked ( GtkButton *button, GtkWidget *parent );
static GtkWidget *bet_config_get_select_labels_widget ( GtkWidget *container );
static void bet_config_period_clicked ( GtkWidget *togglebutton, GdkEventButton *event, GtkWidget *button );
static gboolean bet_config_select_label_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gpointer data );
static GtkWidget *bet_config_get_select_historical_data ( GtkWidget *container,
                        GtkWidget *notebook );
static void bet_config_sensitive_account_parameters ( gint account_number, gboolean sensitive );
static void bet_config_use_budget_toggle ( GtkToggleButton *button, GtkWidget *notebook );
static void bet_config_initialise_duration_widget ( gint account_number,
                        GtkWidget *notebook );
static void bet_config_initialise_finance_widget ( gint account_number,
                        GtkWidget *notebook );
static void bet_config_initialise_select_bank_card ( gint account_number );
static void bet_config_initialise_select_historical_data ( gint account_number,
                        GtkWidget *notebook );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeModel *bet_fyear_model;
extern GtkTreeModel *bet_fyear_model_filter;
/*END_EXTERN*/

static gchar* bet_duration_array[] = {
    N_("Month"),
    N_("Year"),
    NULL
};

const gdouble prev_month_max = 60.0;

/**
 *
 *
 *
 *
 * */

GtkWidget *bet_config_general_create_general_page ( void )
{
    GtkWidget *vbox;
    GtkWidget *paddingbox;
    GtkWidget *widget;


    vbox = new_vbox_with_title_and_icon ( _("General Options"), "gsb-balance_estimate-32.png" );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), BOX_BORDER_WIDTH );

    /* Calculation of period */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Common data") );
    bet_config_general_get_period_widget ( paddingbox );

    /* add a separator */
    gtk_box_pack_start ( GTK_BOX ( vbox ), gtk_separator_new ( GTK_ORIENTATION_HORIZONTAL ), FALSE, FALSE, 5 );

    /* option pour les comptes de caisse */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Option for cash accounts") );

    widget = gsb_automem_checkbutton_new ( _("Add the forecast tab for cash accounts"),
                        &etat.bet_deb_cash_account_option,
                        G_CALLBACK ( bet_config_general_cash_account_option_clicked ),
                        NULL);

    gtk_box_pack_start ( GTK_BOX ( paddingbox ), widget, FALSE, FALSE, 0 );

    if ( etat.bet_deb_cash_account_option == 1 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );

    gtk_widget_show_all ( vbox );

    return vbox;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_general_get_period_widget ( GtkWidget *container )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button_1, *button_2;
    GtkWidget *hbox;
    GtkSizeGroup *size_group;
    gchar *text;

    /* devel_debug (NULL); */
    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( container ), vbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Beginning of period") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5) ;

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5) ;

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("1st day of month") );
    gtk_widget_set_name ( button_1, "bet_period_button_1" );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), button_1 );

    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("date today") );

    if ( etat.bet_deb_period == 1 )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );
    else
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_2 ), TRUE );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    /*set the signals */
    g_signal_connect (G_OBJECT ( button_1 ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_period_clicked ),
                        NULL );
    g_signal_connect (G_OBJECT ( button_2 ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_period_clicked ),
                        NULL );

    text = make_blue ( _("Note: This option is used if no starting date for forecasts.") );
    label = gtk_label_new ( text );
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
    gtk_label_set_use_markup ( GTK_LABEL(label), TRUE );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5) ;
    g_free ( text );

    gtk_widget_show_all ( vbox );

    return vbox;
}


/**
 *
 *
 *
 *
 * */
gboolean bet_config_general_cash_account_option_clicked ( GtkWidget *checkbutton,
                        gpointer null )
{
    GtkWidget *combo;

    combo = g_object_get_data ( G_OBJECT ( gsb_gui_get_account_page () ), "account_combo" );
    if ( combo )
    {
        gint account_number;

        account_number = gsb_account_get_combo_account_number ( combo );
        gsb_data_account_set_bet_show_onglets_all_accounts ();
        bet_config_sensitive_account_parameters ( account_number, TRUE );
    }

    bet_data_select_bet_pages ( gsb_gui_navigation_get_current_account ( ) );

    gsb_file_set_modified ( TRUE );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */

GtkWidget *bet_config_account_create_account_page ( void )
{
    GtkWidget *notebook;
    GtkWidget *account_page;
    GtkWidget *vbox_pref;
    GtkWidget *vbox;
    GtkWidget *widget;

    vbox_pref = new_vbox_with_title_and_icon ( _("Accounts data"), "gsb-balance_estimate-32.png" );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_pref ), BOX_BORDER_WIDTH );
    account_page = gsb_gui_get_account_page ();

    /* set the choice of account */
    widget = bet_config_account_get_select_account ( _("Select an account") );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), widget, FALSE, FALSE, 0 );
    gtk_widget_show_all ( vbox_pref );

    /* sélectionne un compte carte bancaire à débit différé */
    widget = bet_config_account_get_select_bank_card ();
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), widget, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox", widget );

    notebook = gtk_notebook_new ( );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK ( notebook ), FALSE );
    gtk_notebook_set_show_tabs  ( GTK_NOTEBOOK ( notebook ), FALSE );
    gtk_notebook_set_scrollable ( GTK_NOTEBOOK ( notebook ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), notebook, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT ( account_page ), "config_notebook", notebook );
    gtk_widget_show ( notebook );

    /* Data for the account of type GSB_TYPE_BANK, GSB_TYPE_CASH */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), vbox, NULL );
    gtk_widget_show ( vbox );

    /* Data for the forecast */
    widget = bet_config_account_get_forecast_data ( _("Data for the forecast") );
    g_object_set_data ( G_OBJECT ( account_page ), "Data_for_forecast", widget );
    gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );

    /* Sources of historical data */
    widget = bet_config_account_get_select_historical_data ( _("Sources of historical data") );
    g_object_set_data ( G_OBJECT ( account_page ), "Data_for_historical", widget );
    gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );

    /* Data for the account of type GSB_TYPE_LIABILITIES */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), vbox, NULL );
    gtk_widget_show ( vbox );

    /* Data for the credit */
    widget = bet_config_account_get_finance_data ( _("Credit Data") );
    g_object_set_data ( G_OBJECT ( account_page ), "Data_for_credit", widget );
    gtk_box_pack_start ( GTK_BOX ( vbox ), widget, FALSE, FALSE, 0 );

    /* mettre à jour les données du compte */
    widget = g_object_get_data ( G_OBJECT ( account_page ), "account_combo" );
    bet_config_change_account ( widget );

    return vbox_pref;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_account_get_select_account ( gchar *title )
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *combo;
    GtkWidget *button;
    GtkWidget *paddingbox;
    GtkWidget *label;
    gint account_number;

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

    /* set the choice of account */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, title );

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 5 );

    label = gtk_label_new ( _("Account: ") );
    utils_labels_set_alignement ( GTK_LABEL (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX (hbox ), label, FALSE, FALSE, 0 );

    combo = gsb_account_create_combo_list ( G_CALLBACK ( bet_config_change_account ),
                        NULL, FALSE );
    g_object_set_data ( G_OBJECT ( gsb_gui_get_account_page () ), "account_combo", combo );
    if ( ( account_number = gsb_gui_navigation_get_current_account ( ) ) == -1 )
        gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), 0 );
    else
        gsb_account_set_combo_account_number ( combo, account_number );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combo, FALSE, FALSE, 0 );

    button = gtk_check_button_new_with_label ( _("Use the budget module") );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
    g_object_set_data ( G_OBJECT ( combo ), "bet_use_budget", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( bet_config_use_budget_toggle ),
                        combo );

    return vbox;
}



/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_account_get_forecast_data ( gchar *title )
{
    GtkWidget *vbox;
    GtkWidget *widget;
    GtkWidget *paddingbox;

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _(title) );

    /* Calculation of duration */
    widget = bet_config_get_duration_widget ( SPP_ORIGIN_CONFIG );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), widget, FALSE, FALSE, 0);

    /* Select the labels of the list */
    widget = bet_config_get_select_labels_widget ( paddingbox );

    return vbox;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_get_duration_widget ( gint origin )
{
    GtkWidget *box = NULL;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *widget = NULL;
    GtkWidget *hbox;
    GtkWidget *previous = NULL;
    GtkWidget *account_page;
    gpointer pointeur = NULL;
    gint iduration;

    if ( origin == SPP_ORIGIN_CONFIG )
    {
        box = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
        pointeur = GINT_TO_POINTER ( 0 );
    }
    else if ( origin == SPP_ORIGIN_ARRAY )
    {
        box = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
        pointeur = GINT_TO_POINTER ( 1 );
    }
    account_page = gsb_gui_get_account_page ();

    /* partie mensuelle */
    label = gtk_label_new ( _("Duration estimation:") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( box ), label, FALSE, FALSE, 5 ) ;

    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 5 );
    gtk_box_pack_start ( GTK_BOX ( box ), hbox, FALSE, FALSE, 0 ) ;

    spin_button = gtk_spin_button_new_with_range ( 1.0, prev_month_max, 1.0);
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), 1.0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 0 );

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
        gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 0 );
        g_signal_connect (G_OBJECT ( widget ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        spin_button );
    }

    if ( origin == SPP_ORIGIN_CONFIG )
    {

        g_object_set_data ( G_OBJECT ( spin_button ), "bet_origin_signal", pointeur );
        g_object_set_data ( G_OBJECT ( account_page ), "bet_config_account_previous",
                        previous );
        g_object_set_data ( G_OBJECT ( account_page ), "bet_config_account_widget",
                        widget );
        g_object_set_data ( G_OBJECT ( account_page ), "bet_config_account_spin_button",
                        spin_button );
    }
    else if ( origin == SPP_ORIGIN_ARRAY )
    {
        g_object_set_data ( G_OBJECT ( spin_button ), "bet_origin_signal", pointeur );
        g_object_set_data ( G_OBJECT ( account_page ), "bet_account_previous", previous );
        g_object_set_data ( G_OBJECT ( account_page ), "bet_account_widget", widget );
        g_object_set_data ( G_OBJECT ( account_page ), "bet_account_spin_button", spin_button );
    }

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( previous ), TRUE );

    g_signal_connect ( G_OBJECT ( spin_button ),
                        "value-changed",
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        pointeur );
    g_object_set_data ( G_OBJECT ( spin_button ), "pointer", pointeur );

    gtk_widget_show_all ( box );

    return box;
}


/**
 *
 *
 *
 *
 * */
void bet_config_initialise_duration_widget ( gint account_number,
                        GtkWidget *notebook )
{
    GtkWidget *widget = NULL;
    GtkWidget *button = NULL;
    GtkWidget *account_page;
    gpointer ptr = NULL;
    gint param;
    gint months;

    account_page = gsb_gui_get_account_page ();

    param = gsb_data_account_get_bet_spin_range ( account_number );
    months = gsb_data_account_get_bet_months ( account_number );
    button = g_object_get_data ( G_OBJECT ( account_page ), "bet_config_account_spin_button" );

    if ( button && G_IS_OBJECT ( button ) )
    {
        ptr = g_object_get_data ( G_OBJECT ( button ), "pointer" );
        g_signal_handlers_block_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        ptr );
    }

    if ( param == 0 )
    {
        widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_config_account_previous" );
        if ( widget && G_IS_OBJECT ( widget ) )
        {
            g_signal_handlers_block_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        }
        if ( button )
        {
            gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, prev_month_max );
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ), (gdouble) months );
        }
    }
    else
    {
        widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_config_account_widget" );
        if ( widget && G_IS_OBJECT ( widget ) )
        {
            g_signal_handlers_block_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        }
        if ( button )
        {
            gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0,
                        prev_month_max / 12.0 );
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months / 12.0 );
        }
    }

    if ( widget && G_IS_OBJECT ( widget ) )
        g_signal_handlers_unblock_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
    if ( button && G_IS_OBJECT ( button ) )
        g_signal_handlers_unblock_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        ptr );

}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_get_select_labels_widget ( GtkWidget *container )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button;
    gchar *text;
    gint origine;
    gint select;

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_box_pack_start ( GTK_BOX ( container ), vbox, FALSE, FALSE, 0 );

    /* set labels for transactions */
    origine = SPP_ORIGIN_TRANSACTION;
    select = gsb_data_account_get_bet_select_label ( bet_config_get_account_from_combo ( ),
                        origine );
    label = gtk_label_new ( _("Labels for transactions:") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 );

    button = gsb_automem_radiobutton3_new ( _("By default"),
					    _("Categories"),
					    _("Budgetary lines"),
					    &select,
                        G_CALLBACK ( bet_config_select_label_changed ),
                        GINT_TO_POINTER ( origine ),
                        GTK_ORIENTATION_HORIZONTAL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );


    /* set labels for scheduled */
    origine = SPP_ORIGIN_SCHEDULED;
    select = gsb_data_account_get_bet_select_label ( bet_config_get_account_from_combo ( ),
                        origine );
    label = gtk_label_new ( _("Labels for scheduled transactions:") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 );

    button = gsb_automem_radiobutton3_new ( _("By default"),
					    _("Categories"),
					    _("Budgetary lines"),
					    &select,
                        G_CALLBACK ( bet_config_select_label_changed ),
                        GINT_TO_POINTER ( origine ),
                        GTK_ORIENTATION_HORIZONTAL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    /* set labels for futur data */
    origine = SPP_ORIGIN_FUTURE;
    select = gsb_data_account_get_bet_select_label ( bet_config_get_account_from_combo ( ),
                        origine );
    label = gtk_label_new ( _("Labels for futur data:") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 );

    button = gsb_automem_radiobutton3_new ( _("By default"),
					    _("Categories"),
					    _("Budgetary lines"),
					    &select,
                        G_CALLBACK ( bet_config_select_label_changed ),
                        GINT_TO_POINTER ( origine ),
                        GTK_ORIENTATION_HORIZONTAL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    text = make_blue ( _("Order by default if the data are not zero:\n"
                        "\tnotes, payee, category and budgetary line.") );
    label = gtk_label_new ( text );
    gtk_label_set_use_markup ( GTK_LABEL(label), TRUE );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 ) ;
    g_free ( text );

    gtk_widget_show_all ( vbox );

    return vbox;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_account_get_select_historical_data ( gchar *title )
{
    GtkWidget *vbox;
    GtkWidget *paddingbox;

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _(title) );
    bet_config_get_select_historical_data ( paddingbox, gsb_gui_get_account_page () );

    return vbox;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_get_select_historical_data ( GtkWidget *container,
                        GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkWidget *hbox;
    GtkWidget *button_1, *button_2;
    GDate *date_jour;
    gchar *str_year;
    gint year;

    /* Choix des données sources */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( container ), hbox, FALSE, FALSE, 0 );

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("Categories") );
    gtk_widget_set_name ( button_1, "bet_hist_button_1" );
    g_signal_connect (G_OBJECT ( button_1 ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_origin_data_clicked ),
                        GINT_TO_POINTER ( 0 ) );

    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("Budgetary lines") );
    g_signal_connect (G_OBJECT ( button_2 ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_origin_data_clicked ),
                        GINT_TO_POINTER ( 0 ) );

    g_object_set_data ( G_OBJECT ( notebook ), "bet_config_hist_button_1", button_1 );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_config_hist_button_2", button_2 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    /* création du sélecteur de périod */
    widget = gsb_fyear_make_combobox_new ( bet_fyear_model_filter, TRUE );
    gtk_widget_set_name ( GTK_WIDGET ( widget ), "bet_hist_fyear_combo" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( widget ),
                      _("Choose the financial year or 12 months rolling") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5);
    g_object_set_data ( G_OBJECT ( notebook ), "bet_config_hist_fyear_combo", widget );

    /* hide the present financial year */
    date_jour = gdate_today ( );
    year = g_date_get_year ( date_jour );
    g_date_free ( date_jour );

    str_year = utils_str_itoa ( year );
    gsb_fyear_hide_iter_by_name ( bet_fyear_model, str_year );
    g_free ( str_year );

    /* set the signal */
    g_signal_connect ( G_OBJECT ( widget ),
                    "changed",
                    G_CALLBACK ( bet_config_fyear_clicked ),
                    GINT_TO_POINTER ( 0 ) );

    /* return */
    return hbox;
}


/**
 *
 *
 *
 *
 * */
void bet_config_initialise_select_historical_data ( gint account_number,
                        GtkWidget *notebook )
{
    GtkWidget *widget;
    GtkWidget *button = NULL;
    gint param;

    param = gsb_data_account_get_bet_hist_data ( account_number );

    if ( param == 1 )
        button = g_object_get_data ( G_OBJECT ( notebook ), "bet_config_hist_button_2" );
    else
        button = g_object_get_data ( G_OBJECT ( notebook ), "bet_config_hist_button_1" );

    if ( button )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );

    param = gsb_data_account_get_bet_hist_fyear ( account_number );
    widget = g_object_get_data ( G_OBJECT ( notebook ), "bet_config_hist_fyear_combo" );
    bet_historical_set_fyear_from_combobox ( widget, param );
}


/*
 * bet_config_duration_period_clicked
 * This function is called when a radio button is called to change the inial period.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_period property of the bet container
 */
void bet_config_period_clicked ( GtkWidget *togglebutton, GdkEventButton *event, GtkWidget *button )
{
    const gchar *name;

    if ( button )
        g_signal_handlers_block_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_period_clicked ),
                        button );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );
    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );

    if ( g_strcmp0 ( name, "bet_period_button_1" ) == 0 )
    {
        etat.bet_deb_period = 1;
    }
    else
    {
        etat.bet_deb_period = 2;
    }
    if ( button )
        g_signal_handlers_unblock_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_period_clicked ),
                        button );

    gsb_file_set_modified ( TRUE );
}


/*
 * This function is called when a radio button is called to change the estimate duration.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_months property of the bet container
 */
void bet_config_duration_button_clicked ( GtkWidget *togglebutton,
                        GdkEventButton *event,
                        GtkWidget *spin_button )
{
    const gchar *name;
    gpointer data;
    gint account_number;
    gint months;
    gint origin;

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );

    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );
    data = g_object_get_data ( G_OBJECT ( spin_button ), "bet_origin_signal");

    if ( ( origin = GPOINTER_TO_INT ( data ) ) == 0 )
        account_number = bet_config_get_account_from_combo ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    g_signal_handlers_block_by_func ( G_OBJECT ( spin_button ),
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        data );

    if ( g_strcmp0 ( name, "Year" ) == 0 )
    {
        gsb_data_account_set_bet_spin_range ( account_number, 1 );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( spin_button ), 1.0, prev_month_max / 12.0);
        months = gsb_data_account_get_bet_months ( account_number );
        if ( months > ( prev_month_max / 12.0 ) )
        {
            gint number;

            number = months / 12;
            if ( number < 1 )
                number = 1;
            else if (  months % 12 )
                number ++;
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), number );
            gsb_data_account_set_bet_months ( account_number, number * 12 );
        }
        else
            gsb_data_account_set_bet_months ( account_number, months * 12 );
    }
    else
    {
        gsb_data_account_set_bet_spin_range ( account_number, 0 );
        months = gsb_data_account_get_bet_months ( account_number );
        gsb_data_account_set_bet_months ( account_number, months );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( spin_button ), 1.0, prev_month_max );
        if ( origin == 0 )
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), months );
    }

    g_signal_handlers_unblock_by_func ( G_OBJECT ( spin_button ),
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        data );

    gsb_file_set_modified ( TRUE );

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
    bet_data_update_bet_module ( account_number, -1 );
}


/*
 * bet_config_duration_button changed
 * This function is called when a spin button is changed.
 * It copies the new duration from the spin_button into the bet_months property of
 * the bet container
 */
gboolean bet_config_duration_number_changed ( GtkWidget *spin_button,
                        gpointer data )
{
    gint account_number;
    gint months;

    if ( GPOINTER_TO_INT ( data ) == 0 )
        account_number = bet_config_get_account_from_combo ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    months = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button ) );
    if ( gsb_data_account_get_bet_spin_range ( account_number ) == 1 )
        months *= 12;

    gsb_data_account_set_bet_months ( account_number, months );

    gsb_file_set_modified ( TRUE );

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
    bet_data_update_bet_module ( account_number, -1 );

    return ( FALSE );
}


/**
 *
 *
 *
 *
 * */
void bet_config_origin_data_clicked ( GtkWidget *togglebutton, GdkEventButton *event, gpointer data )
{
    GtkTreeViewColumn *column;
    const gchar *name;
    gchar *title;
    gint account_number;
    gint origin;

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );

    if ( ( origin = GPOINTER_TO_INT ( data ) ) == 0 )
        account_number = bet_config_get_account_from_combo ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );

    if ( g_strcmp0 ( name, "bet_hist_button_1" ) == 0 )
    {
        gsb_data_account_set_bet_hist_data ( account_number, 0 );
        bet_data_set_div_ptr ( 0 );
        title = g_strdup ( _("Category") );
        if ( origin == 0 )
        {
            GtkWidget *button;

            button = g_object_get_data ( G_OBJECT ( gsb_gui_get_account_page () ), "bet_hist_button_1" );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
        }
    }
    else
    {
        gsb_data_account_set_bet_hist_data ( account_number, 1 );
        bet_data_set_div_ptr ( 1 );
        title = g_strdup ( _("Budgetary line") );
        if ( origin == 0 )
        {
            GtkWidget *button;

            button = g_object_get_data ( G_OBJECT ( gsb_gui_get_account_page () ), "bet_hist_button_2" );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
        }
    }

    column = g_object_get_data ( G_OBJECT ( gsb_gui_get_account_page () ),
                        "historical_column_source" );
    gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN ( column ), title );
    g_free ( title );

    gsb_file_set_modified ( TRUE );

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ALL );
    bet_data_update_bet_module ( account_number, -1 );
}


/**
 *
 *
 *
 *
 * */
void bet_config_fyear_clicked ( GtkWidget *combo, gpointer data )
{
    gint account_number;

    if ( GPOINTER_TO_INT ( data ) == 0 )
        account_number = bet_config_get_account_from_combo ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    gsb_data_account_set_bet_hist_fyear ( account_number,
                        bet_historical_get_fyear_from_combobox ( combo ) );

    gsb_file_set_modified ( TRUE );

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ALL );
    bet_data_update_bet_module ( account_number, -1 );
}


/**
 * callback called when changing the account from the button
 * re-fill the page with the account data
 *
 * \param button
 * \param null
 *
 * \return FALSE
 * */
gboolean bet_config_change_account ( GtkWidget *combo )
{
    GtkWidget *notebook;
    GtkWidget *widget = NULL;
    GtkWidget *fyear_combo;
    GtkWidget *account_page;
    gint account_number;
    gint bet_use_budget;
    BetTypeOnglets bet_show_onglets;

    devel_debug (NULL);
    widget = g_object_get_data ( G_OBJECT ( combo ), "bet_use_budget" );

    if ( !GTK_IS_TOGGLE_BUTTON ( widget ) )
        return FALSE;

    account_number = gsb_account_get_combo_account_number ( combo );
    bet_use_budget = gsb_data_account_get_bet_use_budget ( account_number );
    account_page = gsb_gui_get_account_page ();

    /* on bloque l'appel à la fonction bet_config_fyear_clicked */
    fyear_combo = g_object_get_data ( G_OBJECT ( account_page ), "bet_config_hist_fyear_combo" );
    g_signal_handlers_block_by_func ( G_OBJECT ( fyear_combo ),
                        G_CALLBACK ( bet_config_fyear_clicked ),
                        GINT_TO_POINTER ( 0 ) );

    switch ( bet_use_budget )
    {
        case -1:
            gtk_widget_set_sensitive ( widget, FALSE );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), FALSE );
            bet_config_sensitive_account_parameters ( account_number, FALSE );
            return FALSE;
            break;
        case 0:
            gtk_widget_set_sensitive ( widget, TRUE );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), FALSE );
            bet_config_sensitive_account_parameters ( account_number, FALSE );
            return FALSE;
            break;
        case 1:
            gtk_widget_set_sensitive ( widget, TRUE );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
            bet_config_sensitive_account_parameters ( account_number, TRUE );
            break;
    }

    bet_show_onglets = gsb_data_account_get_bet_show_onglets ( account_number );
    notebook = g_object_get_data ( G_OBJECT ( account_page ), "config_notebook" );
    switch ( bet_show_onglets )
    {
        case BET_ONGLETS_PREV:
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), 0 );
            bet_config_initialise_select_bank_card ( account_number );
            bet_config_initialise_duration_widget ( account_number, account_page );
            bet_config_initialise_select_historical_data ( account_number, account_page );
            break;
        case BET_ONGLETS_HIST:
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), 0 );
            bet_config_initialise_select_bank_card ( account_number );
            bet_config_initialise_select_historical_data ( account_number, account_page );
            break;
        case BET_ONGLETS_CAP:
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), 1 );
            bet_config_initialise_select_bank_card ( account_number );
            bet_config_initialise_finance_widget ( account_number, account_page );
            break;
        default:
            gtk_notebook_set_current_page ( GTK_NOTEBOOK ( notebook ), 0 );
            break;
    }

    /* on débloque l'appel à la fonction bet_config_fyear_clicked */
    g_signal_handlers_unblock_by_func ( G_OBJECT ( fyear_combo ),
                        G_CALLBACK ( bet_config_fyear_clicked ),
                        GINT_TO_POINTER ( 0 ) );

    /* return */
    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gint bet_config_get_account_from_combo ( void )
{
    GtkWidget *combo;
    gint account_number = -1;

    combo = g_object_get_data ( G_OBJECT ( gsb_gui_get_account_page () ), "account_combo" );
    account_number = gsb_account_get_combo_account_number ( combo );

    return account_number;
}


/**
 * called for a change in automem_radiobutton3
 *
 * \param checkbutton	the button wich changed
 * \param value_ptr	a gint* wich is the value to set in the memory (0, 1 or 2)
 *
 * \return FALSE
 * */
gboolean bet_config_select_label_changed ( GtkWidget *checkbutton,
                        GdkEventButton *event,
                        gpointer data )
{
    gint value;
    gint origine;
    gint account_number;

    devel_debug (NULL);

    /* we are on the active button, so save the value for it */
    value = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( checkbutton ), "pointer" ) );
    origine = GPOINTER_TO_INT ( data );
    account_number = bet_config_get_account_from_combo ( );

    gsb_data_account_set_bet_select_label ( account_number, origine, value );

    gsb_file_set_modified ( TRUE );

    gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ESTIMATE );
    bet_data_update_bet_module ( account_number, -1 );

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
void bet_config_sensitive_account_parameters ( gint account_number, gboolean sensitive )
{
    GtkWidget *widget = NULL;
    GtkWidget *account_page;

    account_page = gsb_gui_get_account_page ();
    if ( sensitive )
    {
        BetTypeOnglets bet_show_onglets;

        bet_show_onglets = gsb_data_account_get_bet_show_onglets ( account_number );

        switch ( bet_show_onglets )
        {
        case BET_ONGLETS_PREV:
            widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_historical" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
            gtk_widget_hide ( widget );
            break;
        case BET_ONGLETS_HIST:
            widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_historical" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
            gtk_widget_hide ( widget );
            break;
        case BET_ONGLETS_CAP:
            widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_historical" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
            gtk_widget_show_all ( widget );
            break;
        default:
            widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_historical" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
            gtk_widget_hide ( widget );
            break;
        }
    }
    else
    {
        widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
        gtk_widget_hide ( widget );
        widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
        gtk_widget_hide ( widget );
        widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_historical" );
        gtk_widget_hide ( widget );
        widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
        gtk_widget_hide ( widget );
    }
}


/**
 *
 *
 *
 *
 * */
void bet_config_use_budget_toggle ( GtkToggleButton *button, GtkWidget *combo )
{
    gint account_number;

    account_number = gsb_account_get_combo_account_number ( combo );

    if ( gtk_toggle_button_get_active ( button ) )
    {
        gsb_data_account_set_bet_use_budget ( account_number, 1 );
        gsb_data_account_set_bet_show_onglets ( account_number );
        bet_config_change_account ( combo );

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ALL );
    }
    else
    {
        bet_data_remove_all_bet_data ( account_number );
        gsb_data_account_set_bet_use_budget ( account_number, 0 );
        bet_config_sensitive_account_parameters ( account_number, FALSE );
    }

    if ( gsb_gui_navigation_get_current_account ( ) == account_number )
        bet_data_select_bet_pages ( account_number );
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_account_get_finance_data ( gchar *title )
{
    GtkWidget *vbox;
    GtkWidget *widget;
    GtkWidget *paddingbox;

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _(title) );

    /* Data of credit */
    widget = bet_config_get_finance_widget ( vbox );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), widget, FALSE, FALSE, 0);

    return vbox;
}


/**
 * called when entry changed
 * check the entry and set the entry red/invalid if not a good number
 *
 * \param entry
 * \param parent
 *
 * \return
 * */
static void bet_config_account_amount_entry_changed ( GtkWidget *entry,
                        GtkWidget *parent )
{
    gboolean valide = FALSE;
    const gchar *text;

    text = gtk_entry_get_text ( GTK_ENTRY ( entry ) );
    /* if nothing in the entry, keep the normal style */
    if ( strlen ( text ) == 0 )
    {
        gtk_widget_set_name (entry, "form_entry");
        return;
    }

    valide = gsb_form_widget_get_valide_amout_entry ( text );
    if ( valide )
    {
        /* the entry is valid, make it normal */
        gtk_widget_set_name (entry, "form_entry");
    }
    else
    {
        /* the entry is not valid, make it red */
        gtk_widget_set_name (entry, "form_entry_error");
    }
}


/**
 * fonction appellée lorsqu'on change le type de taux
 *
 * \param togglebutton
 * \param event
 * \param parent        parent widget for callback
 *
 * \return
 **/
static void bet_config_account_type_taux_changed ( GtkWidget *togglebutton,
                        GdkEventButton *event,
                        GtkWidget *parent )
{
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );
    bet_config_finance_apply_clicked ( NULL, parent );
}


/**
 * function called when changing the start date
 *
 * \param entry         date entry
 * \param event
 * \param parent        parent widget for callback
 *
 * \return              TRUE to stop other handlers from being invoked for the event.
 **/
static gboolean bet_config_account_date_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *event,
                        GtkWidget *parent )
{
    if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) ) )
    {
        GDate *date = NULL;

        date = gsb_calendar_entry_get_date ( entry );
        if ( date && g_date_valid ( date ) )
            bet_config_finance_apply_clicked ( NULL, parent );
    }

    return TRUE;
}


/**
 * function called when changing the capital or fees
 *
 * \param entry         entry for capital
 * \param event
 * \param parent        parent widget for callback
 *
 * \return              TRUE to stop other handlers from being invoked for the event.
 **/
static gboolean bet_config_account_entry_lose_focus ( GtkWidget *entry,
                        GdkEventFocus *event,
                        GtkWidget *parent )
{
    const gchar *text;

    text = gtk_entry_get_text ( GTK_ENTRY ( entry ) );
    if ( strlen ( text ) )
    {
        if ( gsb_form_widget_get_valide_amout_entry ( text ) )
            bet_config_finance_apply_clicked ( NULL, parent );
    }

    return TRUE;
}


/**
 * called when we press the button in an entry field in the entry
 *
 * \param entry     wich receive the signal
 * \param event     can be NULL
 * \param parent    parent widget for callback
 *
 * \return TRUE or FALSE
 * */
static gboolean bet_config_account_date_key_press_event ( GtkWidget *entry,
                        GdkEventKey *event,
                        GtkWidget *parent )
{
    if (  event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter )
    {
        GtkWidget *widget;

        widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_taux" );
        gtk_widget_grab_focus ( widget );
    }

    return FALSE;
}


/**
 * Create the amortization array
 *
 * \param parent    widget parent
 *
 * \return          Widget for options of amortization array
 **/
GtkWidget *bet_config_get_finance_widget ( GtkWidget *parent )
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *widget;
    GtkWidget *spin_button = NULL;
    GtkWidget *button_1, *button_2;

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );

    /* Loan Capital */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Loan Capital: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    widget = gtk_entry_new ( );
    gtk_widget_set_size_request ( widget, 90, -1 );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_capital", widget );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_capital_devise", label );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* Duration */
    label = gtk_label_new ( _("Duration: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    spin_button = gtk_spin_button_new_with_range ( 1.0, 30, 1.0);
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_duree", spin_button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("year") );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_unit_duree", label );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* Date of first Repayment */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Date of first repayment: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    widget = gsb_calendar_entry_new ( FALSE );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_start_date", widget );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );

    /* Annuel rate interest */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Annuel rate interest: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    spin_button = gtk_spin_button_new_with_range ( 0.0, 100,
                        bet_data_finance_get_bet_taux_step ( BET_TAUX_DIGITS ) );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_taux", spin_button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("%") );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* frais */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Fees per month: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    widget = gtk_entry_new ( );
    gtk_widget_set_size_request ( widget, 50, -1 );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_montant_frais", widget );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );

    label = gtk_label_new ( NULL );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_frais_devise", label );
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    /* Rate Type */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Rate Type: ") );
    utils_labels_set_alignement ( GTK_LABEL ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );
    button_1 = gtk_radio_button_new_with_label ( NULL, _("CAGR") );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_type_taux_1", button_1 );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );

    button_2 = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( button_1 ),
                        _("Proportional rate") );
    g_object_set_data ( G_OBJECT ( parent ), "bet_config_type_taux", button_2 );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    return vbox;
}


/**
 *
 *
 *
 *
 * */
void bet_config_initialise_finance_widget ( gint account_number,
                        GtkWidget *notebook )
{
    GtkWidget *parent;
    GtkWidget *widget;
    GtkWidget *button = NULL;
    gchar *code_devise;
    gint devise;
    gint nbre_ans;
    gdouble taux;

    /* devel_debug (NULL); */
    parent = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_credit" );
    /* get devise */
    devise = gsb_data_account_get_currency ( account_number );
    code_devise = gsb_data_currency_get_code_or_isocode ( devise );

    /* set devise labels */
    widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_capital_devise" );
    gtk_label_set_label ( GTK_LABEL ( widget ), code_devise );

    widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_frais_devise" );
    gtk_label_set_label ( GTK_LABEL ( widget ), code_devise );

    /* set capital */
    widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_capital" );
    gtk_entry_set_text ( GTK_ENTRY ( widget ),
                        utils_real_get_string_with_currency (
                        gsb_real_double_to_real (
                        gsb_data_account_get_bet_finance_capital ( account_number ) ),
                        devise, FALSE ) );
    g_signal_connect ( G_OBJECT ( widget ),
                        "changed",
                        G_CALLBACK ( bet_config_account_amount_entry_changed ),
                        parent );
    g_signal_connect_after ( G_OBJECT (widget),
                        "focus-out-event",
                        G_CALLBACK ( bet_config_account_entry_lose_focus ),
                        parent );

    /* set duration */
    nbre_ans = gsb_data_account_get_bet_months ( account_number ) / 12;
    button = g_object_get_data ( G_OBJECT ( parent ), "bet_config_duree" );
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ), nbre_ans );
    widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_unit_duree" );
    if ( nbre_ans == 1 )
        gtk_label_set_label ( GTK_LABEL ( widget ), _(" year ") );
    else
        gtk_label_set_label ( GTK_LABEL ( widget ), _(" years ") );
    g_signal_connect ( button,
                        "value-changed",
                        G_CALLBACK ( bet_config_finance_apply_clicked ),
                        parent );

    /* set start_date */
    widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_start_date" );
    gsb_calendar_entry_set_date ( widget,
                        gsb_data_account_get_bet_start_date ( account_number ) );
    g_signal_connect_after ( G_OBJECT (widget),
                        "focus-out-event",
                        G_CALLBACK ( bet_config_account_date_entry_lose_focus ),
                        parent );
    g_signal_connect ( G_OBJECT ( widget ),
                       "key-press-event",
                       G_CALLBACK ( bet_config_account_date_key_press_event ),
                       parent );

    /* set taux */
    button = g_object_get_data ( G_OBJECT ( parent ), "bet_config_taux" );
    taux = gsb_data_account_get_bet_finance_taux_annuel ( account_number );
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ), taux );
    g_signal_connect ( button,
                        "value-changed",
                        G_CALLBACK ( bet_config_finance_apply_clicked ),
                        parent );

    /* set frais */
    widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_montant_frais" );
    gtk_entry_set_text ( GTK_ENTRY ( widget ),
                        utils_real_get_string_with_currency (
                        gsb_real_double_to_real (
                        gsb_data_account_get_bet_finance_frais ( account_number ) ),
                        devise, FALSE ) );
    g_signal_connect ( G_OBJECT ( widget ),
                        "changed",
                        G_CALLBACK ( bet_config_account_amount_entry_changed ),
                        parent );
    g_signal_connect_after ( G_OBJECT (widget),
                        "focus-out-event",
                        G_CALLBACK ( bet_config_account_entry_lose_focus ),
                        parent );

    /* set type taux */
    button = g_object_get_data ( G_OBJECT ( parent ), "bet_config_type_taux" );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ),
                        gsb_data_account_get_bet_finance_type_taux ( account_number ) );
    g_signal_connect ( G_OBJECT ( button ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_account_type_taux_changed ),
                        parent );

    button = g_object_get_data ( G_OBJECT ( parent ), "bet_config_type_taux_1" );
    g_signal_connect ( G_OBJECT ( button ),
                        "button-release-event",
                        G_CALLBACK ( bet_config_account_type_taux_changed ),
                        parent );
}


/**
 *
 *
 *
 *
 * */
void bet_config_finance_apply_clicked ( GtkButton *button, GtkWidget *parent )
{
    GtkWidget *bouton, *widget;
    gdouble number;
    gint type_taux;
    gint account_number;
    GDate *start_date;

    account_number = bet_config_get_account_from_combo ( );

    /* start date */
    widget = g_object_get_data ( G_OBJECT ( parent ), "bet_config_start_date" );
    start_date = gsb_calendar_entry_get_date( widget );
    gsb_data_account_set_bet_start_date( account_number, start_date );

    /* capital */
    number = bet_finance_get_number_from_string ( parent, "bet_config_capital" );
    gsb_data_account_set_bet_finance_capital ( account_number, number );

    if ( number == 0 )
    {
        gchar *tmp_str;

        tmp_str = g_strdup ( _("You must enter at least one value for the capital") );
        dialogue_error ( tmp_str );
        g_free ( tmp_str );
        return;
    }

    /* Duration */
    number = bet_finance_get_number_from_string ( parent, "bet_config_duree" );
    gsb_data_account_set_bet_months ( account_number, number * 12 );

    /* rate */
    number = bet_finance_get_number_from_string ( parent, "bet_config_taux" );
    gsb_data_account_set_bet_finance_taux_annuel ( account_number, number );

    /* frais */
    number = bet_finance_get_number_from_string ( parent, "bet_config_montant_frais" );
    gsb_data_account_set_bet_finance_frais ( account_number, number );

    /* type de taux */
    bouton = g_object_get_data ( G_OBJECT ( parent ), "bet_config_type_taux" );
    type_taux = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton ) );
    gsb_data_account_set_bet_finance_type_taux ( account_number, type_taux );

    gsb_file_set_modified ( TRUE );

    bet_finance_ui_update_amortization_tab ( account_number );
}


/**
 *
 *
 *
 *
 * */
static void bet_config_select_bank_card_toggle ( GtkToggleButton *button,
                        GtkWidget *combo )
{
    gint account_number;

    account_number = gsb_account_get_combo_account_number ( combo );

    if ( gtk_toggle_button_get_active ( button ) )
        gsb_data_account_set_bet_credit_card ( account_number, 1 );
    else
        gsb_data_account_set_bet_credit_card ( account_number, 0 );

    gsb_data_account_set_bet_show_onglets (  account_number );
    bet_config_change_account ( combo );

    if ( gsb_gui_navigation_get_current_account ( ) == account_number )
        bet_data_select_bet_pages ( account_number );
}

/**
 * widget qui permet de dire que le compte concerné sert à gérer une carte
 * bancaire à débit différé.
 *
 * \param
 *
 * \return hbox
 * */
static GtkWidget *bet_config_account_get_select_bank_card ( void )
{
    GtkWidget *hbox;
    GtkWidget *button;
    GtkWidget *combo;
    GtkWidget *account_page;

    account_page = gsb_gui_get_account_page ();
    /* partie de gestion des comptes cartes avec débit différé */
    hbox = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );

    button = gtk_check_button_new_with_label ( _("Account with deferred debit card") );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_credit_card_button", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    /* set the signal */
    combo = g_object_get_data ( G_OBJECT ( account_page ), "account_combo" );
    g_signal_connect ( G_OBJECT ( button ),
                        "toggled",
                        G_CALLBACK ( bet_config_select_bank_card_toggle ),
                        combo );

    /* return */
    return hbox;
}


/**
 * initialise l'option credit_card
 *
 * \param account_number
 *
 * \return
 * */
static void bet_config_initialise_select_bank_card ( gint account_number )
{
    GtkWidget *button;
    gint active;

    button = g_object_get_data ( G_OBJECT ( gsb_gui_get_account_page () ), "bet_credit_card_button" );
    active = gsb_data_account_get_bet_credit_card ( account_number );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), active );
}


/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
