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
#include "./balance_estimate_tab.h"
#include "./gsb_account.h"
#include "./parametres.h"
#include "./menu.h"
#include "./utils.h"
#include "./utils_dates.h"
#include "./dialog.h"
#include "./gsb_archive_config.h"
#include "./gsb_automem.h"
#include "./gsb_bank.h"
#include "./gsb_currency_config.h"
#include "./gsb_currency_link_config.h"
#include "./gsb_data_account.h"
#include "./gsb_file.h"
#include "./gsb_form_config.h"
#include "./gsb_fyear.h"
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
static gboolean bet_config_change_account ( GtkWidget *combo,
                        gpointer null );
static GtkWidget *bet_config_get_duration_widget ( GtkWidget *container );
static GtkWidget *bet_config_get_period_widget ( GtkWidget *container );
static GtkWidget *bet_config_select_historical_data ( GtkWidget *container );
static void bet_config_period_clicked ( GtkWidget *togglebutton,
                        GtkWidget *button );
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

GtkWidget *bet_config_create_page ( void )
{
    GtkWidget *notebook;
    GtkWidget *vbox_pref;
    GtkWidget *hbox;
    GtkWidget *combo;
    GtkWidget *paddingbox;
    GtkWidget *label;
    GtkWidget *widget;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );

    vbox_pref = new_vbox_with_title_and_icon ( _("Accounts data"),
                        "balance_estimate.png" );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox_pref ), 12 );
    
    /* Calculation of period */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Common data") );

    widget = bet_config_get_period_widget ( paddingbox );
    g_object_set_data ( G_OBJECT ( notebook ), "period_widget", widget );

    /* add a separator */
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), gtk_hseparator_new (), FALSE, FALSE, 5 );

    /* set the choice of account */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Data Per Account") );
    
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 5 );

	label = gtk_label_new (COLON(_("Account")));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
	gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX (hbox ), label, FALSE, FALSE, 0 );

	combo = gsb_account_create_combo_list ( (GtkSignalFunc) bet_config_change_account,
                        NULL, FALSE );
	gtk_combo_box_set_active ( GTK_COMBO_BOX ( combo ), 0 );
    gtk_box_pack_start ( GTK_BOX (hbox ), combo, FALSE, FALSE, 0 );
    
    /* Calculation of duration */
    widget = bet_config_get_duration_widget ( paddingbox );
    g_object_set_data ( G_OBJECT ( notebook ), "duration_widget", widget );

    /* Sources of historical data */
    widget = bet_config_select_historical_data ( paddingbox );
    g_object_set_data ( G_OBJECT ( notebook ), "historical_data", widget );

    gtk_widget_show_all ( vbox_pref );

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
    GtkWidget *main_vbox;
    GtkWidget *label;
    GtkWidget *button_1, *button_2;
    GtkWidget *hbox;
    GtkSizeGroup *size_group;

    //~ devel_debug (NULL);
    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    main_vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( container ), main_vbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Beginning of period") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), hbox, FALSE, FALSE, 5) ;

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("1st day of month") );
    gtk_widget_set_name ( button_1, "button_1" );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), button_1 );
    
    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("date today") );
    gtk_widget_set_name ( button_2, "button_2" );

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

    label = gtk_label_new ( 
                        _("Note: This option is used if no starting date for forecasts.") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;


    gtk_widget_show_all ( main_vbox );

    return main_vbox;
}

/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_get_duration_widget ( GtkWidget *container )
{
    GtkWidget* main_vbox;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *widget = NULL;
    GtkWidget *hbox;
    GtkWidget *previous = NULL;
    GtkSizeGroup *size_group;
    gint iduration;

    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    main_vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( container ), main_vbox, FALSE, FALSE, 5);

    /* partie mensuelle */
    label = gtk_label_new ( _("Duration estimation") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), hbox, FALSE, FALSE, 5) ;

    spin_button = gtk_spin_button_new_with_range ( 1.0, 240.0, 1.0);
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( spin_button ), 1.0 );
    gtk_widget_set_name ( spin_button, "spin_button" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 5 );

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
        gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );
        g_signal_connect (G_OBJECT ( widget ),
                        "released",
                        G_CALLBACK ( bet_config_duration_button_clicked ),
                        spin_button );
    }

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( previous ), TRUE );

    g_signal_connect ( G_OBJECT ( spin_button ),
                        "value-changed",
                        G_CALLBACK ( bet_config_duration_number_changed ),
                        widget );

    gtk_widget_show_all ( main_vbox );

    return main_vbox;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_select_historical_data ( GtkWidget *container )
{
    GtkWidget *main_vbox;
    GtkWidget *label;
    GtkWidget *widget;
    GtkWidget *hbox;
    GtkWidget *button_1, *button_2;
    gchar *str_year;
    gint year;

    main_vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( container ), main_vbox, FALSE, FALSE, 5);

    label = gtk_label_new ( _("Sources of historical data") );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), label, FALSE, FALSE, 5) ;

    /* Choix des données sources */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( main_vbox ), hbox, FALSE, FALSE, 15 );

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("Categories") );
    gtk_widget_set_name ( button_1, "button_1" );
    g_signal_connect (G_OBJECT ( button_1 ),
                        "released",
                        G_CALLBACK ( bet_config_origin_data_clicked ),
                        NULL );

    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("Budgetary lines") );
    gtk_widget_set_name ( button_2, "button_2" );
    g_signal_connect (G_OBJECT ( button_2 ),
                        "released",
                        G_CALLBACK ( bet_config_origin_data_clicked ),
                        NULL );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    /* création du sélecteur de périod */
    widget = gsb_fyear_make_combobox_new ( bet_fyear_model_filter, TRUE );
    gtk_widget_set_name ( GTK_WIDGET ( widget ), "fyear_combo" );
    gtk_widget_set_tooltip_text ( GTK_WIDGET ( widget ),
                      SPACIFY(_("Choose the financial year or 12 months rolling") ) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5);

    /* hide the present financial year */
    year = g_date_get_year ( gdate_today ( ) );
    str_year = utils_str_itoa ( year );
    gsb_fyear_hide_iter_by_name ( bet_fyear_model, str_year );
    g_free ( str_year );

    /* set the signal */
    g_signal_connect ( G_OBJECT ( widget ),
                    "changed",
                    G_CALLBACK ( bet_config_fyear_clicked ),
                    NULL );

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

    if ( g_strcmp0 ( name, "button_1" ) == 0 )
    {
        etat.bet_deb_period = 1;
    }
    else if ( g_strcmp0 ( name, "button_2" ) == 0 )
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
 * bet_config_duration_button_clicked
 * This function is called when a radio button is called to change the estimate duration.
 * It copies the new durations from the data parameter (of the radio button) into
 * the bet_months property of the bet container
 */
void bet_config_duration_button_clicked ( GtkWidget *togglebutton,
                        GtkWidget *spin_button )
{
    const gchar *name;
    gint account_number;
    gint months;

    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );
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

    bet_array_refresh_estimate_tab ( );
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

    account_number = gsb_gui_navigation_get_current_account ( );

    months = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spin_button ) );
    if ( gsb_data_account_get_bet_spin_range ( account_number ) == 1 )
        months *= 12;
                                               
    gsb_data_account_set_bet_months ( account_number, months );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_array_refresh_estimate_tab ( );

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

    account_number = gsb_gui_navigation_get_current_account ( );
    name = gtk_widget_get_name ( GTK_WIDGET ( togglebutton ) );
    //~ devel_debug (name);
    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");

    if ( g_strcmp0 ( name, "button_1" ) == 0 )
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

    bet_array_update_estimate_tab ( );
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

    account_number = gsb_gui_navigation_get_current_account ( );
    gsb_data_account_set_bet_hist_fyear ( account_number,
                        bet_historical_get_fyear_from_combobox ( combo ) );

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    bet_array_update_estimate_tab ( );
}


/**
 *
 *
 *
 *
 * */
gboolean bet_config_set_property_widget_mirror ( GtkWidget *widget,
                        gchar *ancestor_name,
                        gint widget_type,
                        gboolean value )
{
    GtkWidget *notebook;
    GtkWidget *ancestor;
    GtkWidget *mirror;
    //~ GtkTreeViewColumn *column;
    const gchar *name;
    //~ gchar *title;

    name = gtk_widget_get_name ( GTK_WIDGET ( widget ) );
    devel_debug (name);
    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook");
    ancestor = g_object_get_data ( G_OBJECT ( notebook ), ancestor_name );
    if ( gtk_widget_is_ancestor ( widget, ancestor ) == FALSE )
    {
        mirror = utils_get_child_widget_by_name ( ancestor, name );
        if ( mirror )
            printf ("mirror trouvé\n");
            //~ gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( mirror ), TRUE );
    }

    return FALSE;
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
    GtkWidget *ancestor;
    GtkWidget *widget;
    GtkWidget *button;
    gint account_number;
    gint param;
    gint months;

    notebook = g_object_get_data ( G_OBJECT ( notebook_general ), "account_notebook" );
    account_number = gsb_account_get_combo_account_number ( combo );

    ancestor = g_object_get_data ( G_OBJECT ( notebook ), "duration_widget" );

    param = gsb_data_account_get_bet_spin_range ( account_number );
    months = gsb_data_account_get_bet_months ( account_number );
    button = utils_get_child_widget_by_name ( ancestor, "spin_button" );

    if ( param == 1 )
    {
        widget = utils_get_child_widget_by_name ( ancestor, "Year" );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, 20.0 );
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months / 12.0 );
    }
    else
    {
        widget = utils_get_child_widget_by_name ( ancestor, "Month" );
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON ( button ), 1.0, 240.0 );
        gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( button ),
                        (gdouble) months );
    }
    
    ancestor = g_object_get_data ( G_OBJECT ( notebook ), "historical_data" );

    param = gsb_data_account_get_bet_hist_data ( account_number );

    if ( param == 1 )
        button = utils_get_child_widget_by_name ( ancestor, "button_2" );
    else
        button = utils_get_child_widget_by_name ( ancestor, "button_1" );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( widget ), TRUE );

    param = gsb_data_account_get_bet_hist_fyear ( account_number );
    widget = utils_get_child_widget_by_name ( ancestor, "fyear_combo" );
    bet_historical_set_fyear_from_combobox ( widget, param );

    return FALSE;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
