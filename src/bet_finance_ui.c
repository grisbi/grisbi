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

/*START_INCLUDE*/
#include "bet_finance_ui.h"
#include "fenetre_principale.h"
#include "structures.h"
#include "utils.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *account_page;
extern GtkWidget *notebook_general;
/*END_EXTERN*/


/**
 * Create the historical page
 *
 *
 *
 * */
GtkWidget *bet_financial_create_page ( void )
{
    GtkWidget *page;
    GtkWidget *paddingbox;
    GtkWidget *widget;
    GtkWidget *hbox;
    GtkWidget *align;
    GtkWidget *label;
    GtkWidget *spin_button = NULL;
    GtkWidget *button_1, *button_2;
    GtkWidget *tree_view;
    gchar *str_year;
    gint year;
    gpointer pointer;

    devel_debug (NULL);
    page = gtk_vbox_new ( FALSE, 5 );
    gtk_widget_set_name ( page, "financial_page" );

    /* titre de la page */
    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( page ), align, FALSE, FALSE, 5) ;
 
    label = gtk_label_new ( _("Credit Calculator and Amortization") );
    gtk_container_add ( GTK_CONTAINER ( align ), label );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_financial_title", label);

    /* Choix des données sources */
    //~ align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    //~ gtk_box_pack_start ( GTK_BOX ( paddingbox ), align, FALSE, FALSE, 5) ;

    paddingbox = new_paddingbox_with_title ( page, FALSE, _("Common data") );

    align = gtk_alignment_new (0.5, 0.0, 0.0, 0.0);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), align, FALSE, FALSE, 5) ;
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_container_add ( GTK_CONTAINER ( align ), hbox );
    g_object_set_data ( G_OBJECT ( account_page ), "bet_financial_data", hbox );

    label = gtk_label_new ( COLON( _("Loan capital") ) );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    widget = gtk_entry_new ( );
    gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5 );

    label = gtk_label_new ( COLON( _("Annual interest") ) );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    spin_button = gtk_spin_button_new_with_range ( 0.0, 100, 0.01);
    gtk_box_pack_start ( GTK_BOX ( hbox ), spin_button, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON( _("%") ) );
    //~ gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );

    label = gtk_label_new ( COLON( _("Desired duration") ) );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0, 0.5);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_LEFT );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 5 );



    //~ button_1 = gtk_radio_button_new_with_label ( NULL,
                        //~ _("Categories") );
    //~ gtk_widget_set_name ( button_1, "bet_hist_button_1" );
    //~ g_signal_connect ( G_OBJECT ( button_1 ),
                        //~ "released",
                        //~ G_CALLBACK ( bet_config_origin_data_clicked ),
                        //~ GINT_TO_POINTER ( 1 ) );

    //~ button_2 = gtk_radio_button_new_with_label_from_widget (
                        //~ GTK_RADIO_BUTTON ( button_1 ),
                        //~ _("Budgetary lines") );
    //~ g_signal_connect ( G_OBJECT ( button_2 ),
                        //~ "released",
                        //~ G_CALLBACK ( bet_config_origin_data_clicked ),
                        //~ GINT_TO_POINTER ( 1 ) );

    //~ g_object_set_data ( G_OBJECT ( account_page ), "bet_hist_button_1", button_1 );
    //~ g_object_set_data ( G_OBJECT ( account_page ), "bet_hist_button_2", button_2 );
    //~ gtk_box_pack_start ( GTK_BOX ( hbox ), button_1, FALSE, FALSE, 5) ;
    //~ gtk_box_pack_start ( GTK_BOX ( hbox ), button_2, FALSE, FALSE, 5) ;

    //~ gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_1 ), TRUE );
    //~ bet_data_set_div_ptr ( 0 );

    //~ /* création du sélecteur de périod */
    //~ if ( bet_historical_fyear_create_combobox_store ( ) )
    //~ {
        //~ widget = gsb_fyear_make_combobox_new ( bet_fyear_model_filter, TRUE );
        //~ gtk_widget_set_name ( GTK_WIDGET ( widget ), "bet_hist_fyear_combo" );
        //~ gtk_widget_set_tooltip_text ( GTK_WIDGET ( widget ),
                        //~ SPACIFY(_("Choose the financial year or 12 months rolling") ) );

        //~ g_object_set_data ( G_OBJECT ( account_page ), "bet_hist_fyear_combo", widget );

        //~ gtk_box_pack_start ( GTK_BOX ( hbox ), widget, FALSE, FALSE, 5);

        //~ /* hide the present financial year */
        //~ year = g_date_get_year ( gdate_today ( ) );
        //~ str_year = utils_str_itoa ( year );
        //~ gsb_fyear_hide_iter_by_name ( bet_fyear_model, str_year );
        //~ g_free ( str_year );

        //~ /* set the signal */
        //~ pointer = GINT_TO_POINTER ( 1 );
        //~ g_object_set_data ( G_OBJECT ( widget ), "pointer", pointer );
        //~ g_signal_connect ( G_OBJECT ( widget ),
                        //~ "changed",
                        //~ G_CALLBACK (bet_config_fyear_clicked),
                        //~ pointer );
    //~ }

    //~ /* création de la liste des données */
    //~ tree_view = bet_historical_get_data_tree_view ( page );
    //~ g_object_set_data ( G_OBJECT ( account_page ), "bet_historical_treeview", tree_view );

    gtk_widget_show_all ( page );

    return page;
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
