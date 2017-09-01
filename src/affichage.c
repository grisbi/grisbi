/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger	(cedric@grisbi.org)           */
/*          2006-2008 Benjamin Drieu (bdrieu@april.org)                       */
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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "affichage.h"
#include "accueil.h"
#include "custom_list.h"
#include "fenetre_principale.h"
#include "grisbi_settings.h"
#include "grisbi_win.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_fyear_config.h"
#include "gsb_rgba.h"
#include "gsb_scheduler_list.h"
#include "gsb_select_icon.h"
#include "navigation.h"
#include "parametres.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_file_selection.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean change_toolbar_display_mode ( GtkRadioButton *button );
static gboolean preferences_active_mouse_scrolling_left_pane ( GtkWidget *toggle_button,
                        gpointer null );
static gboolean preferences_switch_headings_bar ( GtkWidget *toggle_button,
                        gpointer null );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *fenetre_preferences;
extern GtkWidget *hbox_title;
extern GtkWidget *logo_accueil;

/*END_EXTERN*/

/** Button used to store a nice preview of the homepage logo */

/** GtkImage containing the preview  */

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/

/* **************************************************************************************************************************** */
/* **************************************************************************************************************************** */

/* **************************************************************************************************************************** */


/**
 * Signal triggered when user configure display mode of toolbar
 * buttons.
 *
 * \param button	Radio button that triggered event.
 *
 * \return FALSE
 */
gboolean change_toolbar_display_mode ( GtkRadioButton *button )
{
    GSettings *settings;
    gchar *tmp_str;

    /* Do not execute this callback twice,
     * as it is triggered for both unselected button and newly selected one.
     * We keep the call for the newly selected radio button */
    if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button ) ) )
        return FALSE;

    /* save the new parameter */
    conf.display_toolbar = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT(button), "display" ));

    /* update GSettings */
    switch ( conf.display_toolbar )
    {
        case GSB_BUTTON_TEXT:
            tmp_str = "Text";
            break;
        case GSB_BUTTON_ICON:
            tmp_str = "Icons";
            break;
        default:
            tmp_str = "Text + Icons";
    }
    settings = grisbi_settings_get_settings ( SETTINGS_DISPLAY );
    g_settings_set_string ( G_SETTINGS ( settings ),
                        "display-toolbar",
                        tmp_str );


    /* update toolbars */
    gsb_gui_update_all_toolbars ( );

    return FALSE;
}



/**
 * Create a vbox containing widgets allowing to change aspects of
 * toolbar buttons.
 *
 * \return A newly created vbox.
 */
GtkWidget *tab_display_toolbar ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *paddingbox;
    GtkWidget *radio;
    GtkWidget *radiogroup;
    GtkWidget *vbox;
    GtkWidget *button;

    vbox_pref = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );

    /* Headings bar */
    vbox = new_vbox_with_title_and_icon ( _("Headings bar"), "gsb-organization-32.png" );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), vbox, FALSE, FALSE, 0 );

    button = gsb_automem_checkbutton_new ( _("Display headings bar"),
                        &(conf.show_headings_bar),
                        G_CALLBACK ( preferences_switch_headings_bar ),
                        NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );

    /* Navigation pane */
    vbox = new_vbox_with_title_and_icon ( _("Navigation pane"), "gsb-organization-32.png" );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), vbox, FALSE, FALSE, 0 );

    button = gsb_automem_checkbutton_new (_("Add mouse scrolling support on the navigation pane"),
                        &conf.active_scrolling_left_pane,
                        G_CALLBACK ( preferences_active_mouse_scrolling_left_pane ),
                        NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 5 );

    /* Toolbar */
    vbox = new_vbox_with_title_and_icon ( _("Toolbars"), "gsb-toolbar-32.png" );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), vbox, FALSE, FALSE, 0 );

    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Display toolbar buttons as") );

    radiogroup = radio = gtk_radio_button_new_with_label ( NULL, _("Text") );
    g_object_set_data ( G_OBJECT ( radio ), "display", GINT_TO_POINTER ( GSB_BUTTON_TEXT ) );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), radio, FALSE, FALSE, 0 );
    if ( conf.display_toolbar == GSB_BUTTON_TEXT )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio ), TRUE );

    g_signal_connect ( G_OBJECT ( radio ),
                        "toggled",
                        G_CALLBACK ( change_toolbar_display_mode ),
                        NULL );

    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( radiogroup ), _("Icons") );
    g_object_set_data ( G_OBJECT ( radio ), "display", GINT_TO_POINTER ( GSB_BUTTON_ICON ) );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), radio, FALSE, FALSE, 0 );
    if ( conf.display_toolbar == GSB_BUTTON_ICON )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio ), TRUE );

    g_signal_connect ( G_OBJECT ( radio ),
                        "toggled",
                        G_CALLBACK ( change_toolbar_display_mode ),
                        NULL );

    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( radiogroup ), _("Both") );
    g_object_set_data ( G_OBJECT ( radio ), "display", GINT_TO_POINTER ( GSB_BUTTON_BOTH ) );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), radio, FALSE, FALSE, 0 );
    if ( conf.display_toolbar == GSB_BUTTON_BOTH )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio ), TRUE );

    g_signal_connect ( G_OBJECT ( radio ),
                        "toggled",
                        G_CALLBACK ( change_toolbar_display_mode ),
                        NULL );

    gtk_widget_show_all ( vbox_pref );

    if ( !gsb_data_account_get_accounts_amount () )
        gtk_widget_set_sensitive ( vbox_pref, FALSE );

    /* return */
    return ( vbox_pref );
}


/**
 * called when switch the preferences heading bar button
 * to show/hide the headings bar
 *
 * \param toggle button
 * \param null
 *
 * \return FALSE
 * */
gboolean preferences_switch_headings_bar ( GtkWidget *toggle_button,
                        gpointer null )
{
    grisbi_win_headings_update_show_headings ();
    return FALSE;
}


/**
 * called when switch the active mouse scrolling option
 * to set unset the scrolling
 *
 * \param toggle button
 * \param null
 *
 * \return FALSE
 * */
gboolean preferences_active_mouse_scrolling_left_pane ( GtkWidget *toggle_button,
                        gpointer null )
{
    if ( conf.active_scrolling_left_pane )
        g_signal_handlers_unblock_by_func ( gsb_gui_navigation_get_tree_view ( ),
                        G_CALLBACK ( gsb_gui_navigation_check_scroll ),
                        NULL );
    else
        g_signal_handlers_block_by_func ( gsb_gui_navigation_get_tree_view ( ),
                        G_CALLBACK ( gsb_gui_navigation_check_scroll ),
                        NULL );

    return FALSE;
}


/**
 * Signal triggered when user configure display grisbi title
 *
 * \param button	Radio button that triggered event.
 *
 * \return FALSE
 */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
