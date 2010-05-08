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

#include "include.h"
#include <config.h>

#ifdef ENABLE_BALANCE_ESTIMATE

/*START_INCLUDE*/
#include "balance_estimate_config.h"
#include "./balance_estimate_data.h"
#include "./balance_estimate_hist.h"
#include "./utils_dates.h"
#include "./gsb_account.h"
#include "./gsb_automem.h"
#include "./gsb_data_account.h"
#include "./gsb_fyear.h"
#include "./navigation.h"
#include "./dialog.h"
#include "./traitement_variables.h"
#include "./utils.h"
#include "./utils_str.h"
#include "./balance_estimate_data.h"
#include "./structures.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
static gboolean bet_config_change_account ( GtkWidget *combo,
                        gpointer null );
static gint bet_config_get_account ( void );
static GtkWidget *bet_config_get_duration_widget ( GtkWidget *container );
static GtkWidget *bet_config_get_period_widget ( GtkWidget *container );
static GtkWidget *bet_config_get_select_historical_data ( GtkWidget *container );
static GtkWidget *bet_config_get_select_labels_widget ( GtkWidget *container );
static void bet_config_period_clicked ( GtkWidget *togglebutton, GtkWidget *button );
static gboolean bet_config_select_label_changed ( GtkWidget *checkbutton,
						   gpointer data );
static void bet_config_sensitive_account_parameters ( gint account_number, gboolean sensitive );
static void bet_config_use_budget_toggle ( GtkToggleButton *button, GtkWidget *notebook );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeModel *bet_fyear_model;
extern GtkTreeModel *bet_fyear_model_filter;
extern GtkWidget *notebook_general;
/*END_EXTERN*/

gchar* bet_duration_array[] = {
    N_("Month"),
    N_("Year"),
    NULL
};



/**
 *
 *
 *
 *
 * */

GtkWidget *bet_config_create_general_page ( void )
{
    GtkWidget *notebook;
    GtkWidget *vbox_pref;
    GtkWidget *paddingbox;
    GtkWidget *widget;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );

    vbox_pref = new_vbox_with_title_and_icon ( _("General Options"),
                        "balance_estimate.png" );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_pref ), 12 );
    
    /* Calculation of period */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Common data") );

    widget = bet_config_get_period_widget ( paddingbox );

    /* add a separator */
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), gtk_hseparator_new (), FALSE, FALSE, 5 );

    gtk_widget_show_all ( vbox_pref );

    return vbox_pref;
}


/**
 *
 *
 *
 *
 * */

GtkWidget *bet_config_create_account_page ( void )
{
    GtkWidget *notebook;
    GtkWidget *vbox_pref;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *combo;
    GtkWidget *button;
    GtkWidget *paddingbox;
    GtkWidget *label;
    GtkWidget *widget;
    gint account_number;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );

    vbox_pref = new_vbox_with_title_and_icon ( _("Accounts data"),
                        "balance_estimate.png" );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_pref ), 12 );
    
    /* set the choice of account */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Select an account") );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 5 );

	label = gtk_label_new (COLON(_("Account")));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX (hbox ), label, FALSE, FALSE, 0 );

	combo = gsb_account_create_combo_list ( (GtkSignalFunc) bet_config_change_account,
                        NULL, FALSE );
    g_object_set_data ( G_OBJECT ( notebook ), "account_combo", combo );
    if ( ( account_number = gsb_gui_navigation_get_current_account ( ) ) == -1 )
	    gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), 0 );
    else
        gsb_account_set_combo_account_number ( combo, account_number );
    gtk_box_pack_start ( GTK_BOX (hbox ), combo, FALSE, FALSE, 0 );

    button = gtk_check_button_new_with_label ( _("Use the budget module") );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_use_budget", button );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 5 );
    g_signal_connect ( G_OBJECT ( button ),
			            "toggled",
			            G_CALLBACK ( bet_config_use_budget_toggle ),
			            combo );

    gtk_widget_show_all ( vbox_pref );

    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), vbox, FALSE, FALSE, 0 );
    gtk_widget_show ( vbox );

    /* Data for the forecast */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT ( notebook ), "Data_for_forecast", hbox );

    paddingbox = new_paddingbox_with_title ( hbox, FALSE, _("Data for the forecast") );

    /* Calculation of duration */
    widget = bet_config_get_duration_widget ( paddingbox );

    /* Select the labels of the list */
    widget = bet_config_get_select_labels_widget ( paddingbox );

    /* Sources of historical data */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT ( notebook ), "Data_for_historical", hbox );

    paddingbox = new_paddingbox_with_title ( hbox, FALSE,
                        _("Sources of historical data") );

    widget = bet_config_get_select_historical_data ( paddingbox );

    bet_config_change_account ( combo, NULL );

    return vbox_pref;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_get_period_widget ( GtkWidget *container )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button_1, *button_2;
    GtkWidget *hbox;
    GtkSizeGroup *size_group;
    gchar *text;

    //~ devel_debug (NULL);
    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( container ), vbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Beginning of period") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
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
                        "released",
                        G_CALLBACK ( bet_config_period_clicked ),
                        NULL );
    g_signal_connect (G_OBJECT ( button_2 ),
                        "released",
                        G_CALLBACK ( bet_config_period_clicked ),
                        NULL );

    text = make_blue ( _("Note: This option is used if no starting date for forecasts.") );
    label = gtk_label_new ( text );
    gtk_label_set_use_markup ( GTK_LABEL(label), TRUE );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
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
GtkWidget *bet_config_get_duration_widget ( GtkWidget *container )
{
    GtkWidget *notebook;
    GtkWidget* vbox;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *widget = NULL;
    GtkWidget *hbox;
    GtkWidget *previous = NULL;
    GtkSizeGroup *size_group;
    gint iduration;

    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");
    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( container ), vbox, FALSE, FALSE, 0);

    /* partie mensuelle */
    label = gtk_label_new ( _("Duration estimation") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 ) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 ) ;

    spin_button = gtk_spin_button_new_with_range ( 1.0, 240.0, 1.0);
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
                        "released",
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        spin_button );
    }

    g_object_set_data ( G_OBJECT ( spin_button ), "bet_origin_signal", GINT_TO_POINTER ( 0 ) );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_config_account_previous", previous );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_config_account_widget", widget );
    g_object_set_data ( G_OBJECT ( notebook ), "bet_config_account_spin_button", spin_button );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( previous ), TRUE );

    g_signal_connect ( G_OBJECT ( spin_button ),
                        "value-changed",
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        GINT_TO_POINTER ( 0 ) );

    gtk_widget_show_all ( vbox );

    return vbox;
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

    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( container ), vbox, FALSE, FALSE, 0 );

    /* set labels for transactions */
    origine = SPP_ORIGIN_TRANSACTION;
    select = gsb_data_account_get_bet_select_label ( bet_config_get_account ( ), origine );
    label = gtk_label_new ( _("Labels for transactions") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 );

    button = gsb_automem_radiobutton3_new ( _("By default"),
					  _("Categories"),
					  _("Budgetary lines"),
					  &select,
                      G_CALLBACK ( bet_config_select_label_changed ),
                      GINT_TO_POINTER ( origine ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );


    /* set labels for scheduled */
    origine = SPP_ORIGIN_SCHEDULED;
    select = gsb_data_account_get_bet_select_label ( bet_config_get_account ( ), origine );
    label = gtk_label_new ( _("Labels for scheduled transactions") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 );

    button = gsb_automem_radiobutton3_new ( _("By default"),
					  _("Categories"),
					  _("Budgetary lines"),
					  &select,
                      G_CALLBACK ( bet_config_select_label_changed ),
                      GINT_TO_POINTER ( origine ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    /* set labels for futur data */
    origine = SPP_ORIGIN_FUTURE;
    select = gsb_data_account_get_bet_select_label ( bet_config_get_account ( ), origine );
    label = gtk_label_new ( _("Labels for futur data") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 5 );

    button = gsb_automem_radiobutton3_new ( _("By default"),
					  _("Categories"),
					  _("Budgetary lines"),
					  &select,
                      G_CALLBACK ( bet_config_select_label_changed ),
                      GINT_TO_POINTER ( origine ) );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    text = make_blue ( _("Order by default if the data are not zero: notes, payee, "
                        "category and budgetary line.") );
    label = gtk_label_new ( text );
    gtk_label_set_use_markup ( GTK_LABEL(label), TRUE );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
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
GtkWidget *bet_config_get_select_historical_data ( GtkWidget *container )
{
    GtkWidget *notebook;
    GtkWidget *vbox;
    GtkWidget *widget;
    GtkWidget *hbox;
    GtkWidget *button_1, *button_2;
    gchar *str_year;
    gint year;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");
    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( container ), vbox, FALSE, FALSE, 0);

    /* Choix des données sources */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("Categories") );
    gtk_widget_set_name ( button_1, "bet_hist_button_1" );
    g_signal_connect (G_OBJECT ( button_1 ),
                        "released",
                        G_CALLBACK ( bet_config_origin_data_clicked ),
                        GINT_TO_POINTER ( 0 ) );

    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("Budgetary lines") );
    g_signal_connect (G_OBJECT ( button_2 ),
                        "released",
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
                      SPACIFY(_("Choose the financial year or 12 months rolling") ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5);
    g_object_set_data ( G_OBJECT ( notebook ), "bet_config_hist_fyear_combo", widget );

    /* hide the present financial year */
    year = g_date_get_year ( gdate_today ( ) );
    str_year = utils_str_itoa ( year );
    gsb_fyear_hide_iter_by_name ( bet_fyear_model, str_year );
    g_free ( str_year );

    /* set the signal */
    g_signal_connect ( G_OBJECT ( widget ),
                    "changed",
                    G_CALLBACK ( bet_config_fyear_clicked ),
                    GINT_TO_POINTER ( 0 ) );

    return hbox;
}


/*
 * bet_config_duration_period_clicked
 * This function is called when a radio button is called to change the inial period.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_period property of the bet container
 */
void bet_config_period_clicked ( GtkWidget *togglebutton, GtkWidget *button )
{
    const gchar *name;

    if ( button )
        g_signal_handlers_block_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_period_clicked ),
                        button );

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

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
}


/*
 * This function is called when a radio button is called to change the estimate duration.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_months property of the bet container
 */
void bet_config_duration_button_clicked ( GtkWidget *togglebutton,
                        GtkWidget *spin_button )
{
    const gchar *name;
    gpointer data;
    gint account_number;
    gint months;

    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );
    data = g_object_get_data ( G_OBJECT ( spin_button ), "bet_origin_signal");

    if ( GPOINTER_TO_INT ( data ) == 0 )
        account_number = bet_config_get_account ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    if ( g_strcmp0 ( name, "Year" ) == 0 )
    {
        gsb_data_account_set_bet_spin_range ( account_number, 1 );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( spin_button ), 1.0, 20.0 );
        months = gsb_data_account_get_bet_months ( account_number );
        if ( months > 20 )
        {
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), 20.0 );
            gsb_data_account_set_bet_months ( account_number, 240 );
        }
        else
            gsb_data_account_set_bet_months ( account_number, months * 12 );
    }
    else
    {
        gsb_data_account_set_bet_spin_range ( account_number, 0 );
        gsb_data_account_set_bet_months ( account_number,
                        gtk_spin_button_get_value_as_int (
                        GTK_SPIN_BUTTON ( spin_button ) ) );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( spin_button ), 1.0, 240.0 );
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_data_set_maj ( account_number, BET_MAJ_ESTIMATE );
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
        account_number = bet_config_get_account ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    months = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button ) );
    if ( gsb_data_account_get_bet_spin_range ( account_number ) == 1 )
        months *= 12;
                                               
    gsb_data_account_set_bet_months ( account_number, months );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_data_set_maj ( account_number, BET_MAJ_ESTIMATE );

    return ( FALSE );
}


/**
 *
 *
 *
 *
 * */
void bet_config_origin_data_clicked ( GtkWidget *togglebutton, gpointer data )
{
    GtkWidget *notebook;
    GtkTreeViewColumn *column;
    const gchar *name;
    gchar *title;
    gint account_number;

    if ( GPOINTER_TO_INT ( data ) == 0 )
        account_number = bet_config_get_account ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");

    if ( g_strcmp0 ( name, "bet_hist_button_1" ) == 0 )
    {
        gsb_data_account_set_bet_hist_data ( account_number, 0 );
        bet_data_set_div_ptr ( 0 );
        title = g_strdup ( _("Category") );
    }
    else
    {
        gsb_data_account_set_bet_hist_data ( account_number, 1 );
        bet_data_set_div_ptr ( 1 );
        title = g_strdup ( _("Budgetary line") );
    }

    column = g_object_get_data ( G_OBJECT ( notebook ),
                        "historical_column_source" );
    gtk_tree_view_column_set_title ( GTK_TREE_VIEW_COLUMN ( column ), title );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_data_set_maj ( account_number, BET_MAJ_HISTORICAL );
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
        account_number = bet_config_get_account ( );
    else
        account_number = gsb_gui_navigation_get_current_account ( );

    gsb_data_account_set_bet_hist_fyear ( account_number,
                        bet_historical_get_fyear_from_combobox ( combo ) );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_data_set_maj ( account_number, BET_MAJ_HISTORICAL );
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
gboolean bet_config_change_account ( GtkWidget *combo,
                        gpointer null )
{
    GtkWidget *notebook;
    GtkWidget *widget = NULL;
    GtkWidget *button = NULL;
    gpointer ptr = NULL;
    gint account_number;
    gint bet_use_budget;
    gint param;
    gint months;

    devel_debug (NULL);
    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );
    account_number = gsb_account_get_combo_account_number ( combo );

    widget = g_object_get_data ( G_OBJECT ( notebook ), "bet_use_budget" );
    if ( !GTK_IS_TOGGLE_BUTTON ( widget ) )
        return FALSE;

    bet_use_budget = gsb_data_account_get_bet_use_budget ( account_number );
    switch ( bet_use_budget )
    {
        case -1:
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), FALSE );
            gtk_widget_set_sensitive ( widget, FALSE );
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
                    
    param = gsb_data_account_get_bet_spin_range ( account_number );
    months = gsb_data_account_get_bet_months ( account_number );
    button = g_object_get_data ( G_OBJECT ( notebook ), "bet_config_account_spin_button" );

    if ( button && G_IS_OBJECT ( button ) )
    {
        ptr = g_object_get_data ( G_OBJECT ( button ), "pointer" );
        g_signal_handlers_block_by_func ( G_OBJECT ( button ),
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        ptr );
    }

    if ( param == 0 )
    {
        widget = g_object_get_data ( G_OBJECT ( notebook ), "bet_config_account_previous" );
        if ( widget && G_IS_OBJECT ( widget ) )
        {
            g_signal_handlers_block_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        }
        if ( button )
        {
            gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, 240.0 );
            gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months );
        }
    }
    else
    {
        widget = g_object_get_data ( G_OBJECT ( notebook ), "bet_config_account_widget" );
        if ( widget && G_IS_OBJECT ( widget ) )
        {
            g_signal_handlers_block_by_func ( G_OBJECT ( widget ),
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        button );
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        }
        if ( button )
        {
            gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, 20.0 );
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

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
gint bet_config_get_account ( void )
{
    GtkWidget *notebook;
    GtkWidget *combo;
    gint account_number = -1;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );
    combo = g_object_get_data ( G_OBJECT ( notebook ), "account_combo" );
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
static gboolean bet_config_select_label_changed ( GtkWidget *checkbutton,
						   gpointer data )
{
    gint value;
    gint origine;
    gint account_number;

    devel_debug (NULL);

    /* we are on the active button, so save the value for it */
    value = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( checkbutton ), "pointer" ) );
    origine = GPOINTER_TO_INT ( data );
    account_number = bet_config_get_account ( );

    gsb_data_account_set_bet_select_label ( account_number, origine, value );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    
    bet_data_set_maj ( account_number, BET_MAJ_ESTIMATE );

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
    GtkWidget *notebook;
    GtkWidget *widget = NULL;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );

    if ( sensitive )
    {
        kind_account kind;

        kind = gsb_data_account_get_kind ( account_number );
        switch ( kind )
        {
        case GSB_TYPE_BANK:
            widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_forecast" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_historical" );
            gtk_widget_show_all ( widget );
            break;
        case GSB_TYPE_CASH:
            widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_forecast" );
            gtk_widget_hide_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_historical" );
            gtk_widget_show_all ( widget );
            break;
        case GSB_TYPE_LIABILITIES:
            break;
        case GSB_TYPE_ASSET:
            break;
        }

    }
    else
    {
            widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_forecast" );
            gtk_widget_hide_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_historical" );
            gtk_widget_hide_all ( widget );
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
    GtkWidget *notebook;
    GtkWidget *widget;
    gint account_number;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );
    account_number = gsb_account_get_combo_account_number ( combo );

    if ( gtk_toggle_button_get_active ( button ) )
    {
        gsb_data_account_set_bet_use_budget ( account_number, 1 );
        bet_config_change_account ( combo, NULL );

        bet_data_set_maj ( account_number, BET_MAJ_ALL );
        if ( gsb_gui_navigation_get_current_account ( ) == account_number )
            bet_data_select_bet_pages ( account_number );
    }
    else
    {
        gsb_data_account_set_bet_use_budget ( account_number, 0 );
        widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_forecast" );
        gtk_widget_hide_all ( widget );
        widget = g_object_get_data ( G_OBJECT ( notebook ), "Data_for_historical" );
        gtk_widget_hide_all ( widget );

        if ( gsb_gui_navigation_get_current_account ( ) == account_number )
            bet_data_select_bet_pages ( account_number );
    }
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
