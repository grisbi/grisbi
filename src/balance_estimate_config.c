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

#include "include.h"
#include <config.h>

#ifdef ENABLE_BALANCE_ESTIMATE

/*START_INCLUDE*/
#include "balance_estimate_config.h"
#include "./balance_estimate_data.h"
#include "./balance_estimate_hist.h"
#include "./balance_estimate_tab.h"
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
static GtkWidget *bet_config_select_historical_data ( GtkWidget *container );

/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeModel *bet_fyear_model;
extern GtkTreeModel *bet_fyear_model_filter;
/*END_EXTERN*/


/**
 *
 *
 *
 *
 * */

GtkWidget *bet_estimate_config_create_page ( void )
{
    GtkWidget *vbox_pref, *paddingbox;


    vbox_pref = new_vbox_with_title_and_icon ( _("Balance estimate"),
                        "balance_estimate.png" );

    /* Calculation of period */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Calculation of period") );

    bet_parameter_get_duration_widget ( paddingbox, TRUE );

    /* Sources of historical data */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
                        _("Sources of historical data") );
    bet_config_select_historical_data ( paddingbox );

    gtk_widget_show_all ( vbox_pref );

    return vbox_pref;
}


/**
 *
 *
 *
 *
 * */
GtkWidget *bet_config_select_historical_data ( GtkWidget *container )
{
    GtkWidget *widget;
    GtkWidget *hbox;
    GtkWidget *button_1, *button_2;
    gchar *str_year;
    gint year;

    /* Choix des données sources */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( container ), hbox, FALSE, FALSE, 15 );

    button_1 = gtk_radio_button_new_with_label ( NULL,
                        _("Categories") );
    gtk_widget_set_name ( button_1, "button_1" );

    button_2 = gtk_radio_button_new_with_label_from_widget (
                        GTK_RADIO_BUTTON ( button_1 ),
                        _("Budgetary lines") );
    gtk_widget_set_name ( button_2, "button_2" );
    g_signal_connect (G_OBJECT ( button_2 ),
                        "released",
                        G_CALLBACK ( bet_historical_origin_data_clicked ),
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

    /* show the old choice */
    if ( etat.bet_hist_fyear > 0 )
    {
        gsb_fyear_select_iter_by_number ( widget,
                    bet_fyear_model,
                    bet_fyear_model_filter,
                    etat.bet_hist_fyear );
    }

    /* set the signal */
    g_signal_connect ( G_OBJECT ( widget ),
                    "changed",
                    G_CALLBACK (bet_historical_fyear_clicked),
                    NULL );

    return hbox;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
#endif /* ENABLE_BALANCE_ESTIMATE */
