/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_bet_general.h"
#include "bet_data.h"
#include "gsb_account.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageBetGeneralPrivate   PrefsPageBetGeneralPrivate;

struct _PrefsPageBetGeneralPrivate
{
	GtkWidget *			vbox_bet_general;

    GtkWidget *			checkbutton_bet_cash_account_option;
	GtkWidget *			radiobutton_bet_debut_period_1;
    GtkWidget *         radiobutton_bet_debut_period_2;

};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageBetGeneral, prefs_page_bet_general, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_bet_general_sensitive_account_parameters (gint account_number,
																 gboolean sensitive )
{
    GtkWidget *widget = NULL;
    GtkWidget *account_page;

	devel_debug (NULL);
    account_page = grisbi_win_get_account_page ();
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
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_hist" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
            gtk_widget_hide ( widget );
            break;
        case BET_ONGLETS_HIST:
            widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_hist" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
            gtk_widget_hide ( widget );
            break;
        case BET_ONGLETS_CAP:
            widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
            gtk_widget_show_all ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_hist" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
            gtk_widget_show_all ( widget );
            break;
		case BET_ONGLETS_SANS:
		case BET_ONGLETS_ASSET:
        default:
            widget = g_object_get_data ( G_OBJECT ( account_page ), "bet_credit_card_hbox" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_forecast" );
            gtk_widget_hide ( widget );
            widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_hist" );
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
        widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_hist" );
        gtk_widget_hide ( widget );
        widget = g_object_get_data ( G_OBJECT ( account_page ), "Data_for_credit" );
        gtk_widget_hide ( widget );
    }
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_bet_general_bet_debut_period_toggled (GtkWidget *checkbutton,
															 gboolean *value)
{
	*value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));

	gsb_file_set_modified ( TRUE );
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_bet_general_cash_account_option_toggled (GtkWidget *checkbutton,
																gboolean *value)
{
    GtkWidget *combo;

	*value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));

	combo = g_object_get_data ( G_OBJECT ( grisbi_win_get_account_page () ), "account_combo" );
    if ( combo )
    {
        gint account_number;

        account_number = gsb_account_get_combo_account_number ( combo );
        gsb_data_account_set_bet_show_onglets_all_accounts ();
        prefs_page_bet_general_sensitive_account_parameters ( account_number, TRUE );
    }

    bet_data_select_bet_pages ( gsb_gui_navigation_get_current_account ( ) );

    gsb_file_set_modified ( TRUE );
}

/**
 * Création de la page de gestion des bet_general
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_bet_general_setup_page (PrefsPageBetGeneral *page)
{
	GtkWidget *head_page;
	GrisbiWinEtat *w_etat;
	PrefsPageBetGeneralPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bet_general_get_instance_private (page);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("General Options"), "gsb-balance_estimate-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_bet_general), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_bet_general), head_page, 0);

	/* set the begin of period */
	if (w_etat->bet_debut_period)
		 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_bet_debut_period_1), TRUE);
	else
		 gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_bet_debut_period_2), TRUE);

	/* set checkbutton_bet_cash_account_option */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_bet_cash_account_option),
								  w_etat->bet_cash_account_option);

    /* Connect signal */
    g_signal_connect (priv->radiobutton_bet_debut_period_1,
					  "toggled",
					  G_CALLBACK (prefs_page_bet_general_bet_debut_period_toggled),
					  &w_etat->bet_debut_period);

    g_signal_connect (priv->checkbutton_bet_cash_account_option,
					  "toggled",
					  G_CALLBACK (prefs_page_bet_general_cash_account_option_toggled),
					  &w_etat->bet_cash_account_option);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_bet_general_init (PrefsPageBetGeneral *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_bet_general_setup_page (page);
}

static void prefs_page_bet_general_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_bet_general_parent_class)->dispose (object);
}

static void prefs_page_bet_general_class_init (PrefsPageBetGeneralClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_bet_general_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_bet_general.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetGeneral, vbox_bet_general);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetGeneral, radiobutton_bet_debut_period_1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetGeneral, radiobutton_bet_debut_period_2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetGeneral, checkbutton_bet_cash_account_option);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
PrefsPageBetGeneral *prefs_page_bet_general_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_BET_GENERAL_TYPE, NULL);
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

