/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                           */
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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */


#include "config.h"

#include <gtk/gtk.h>


/*START_INCLUDE*/
#include "etats_config.h"
#include "etats_calculs.h"
#include "etats_prefs.h"
#include "etats_onglet.h"
#include "etats_page_amount.h"
#include "etats_page_accounts.h"
#include "etats_page_budget.h"
#include "etats_page_category.h"
#include "etats_page_payee.h"
#include "etats_page_period.h"
#include "etats_page_text.h"
#include "etats_page_transfer.h"
#include "grisbi_app.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/* last_report */
static gint last_report_number = -1;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Initialise la boite de dialogue propriétés de l'état.
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean etats_config_initialise_dialog_from_etat (GtkWidget *etats_prefs,
														  gint report_number)
{
    /* onglet période */
    etats_page_period_initialise_onglet (etats_prefs, report_number);

    /* onglet virements */
    etats_page_transfer_initialise_onglet (etats_prefs, report_number);

    /* onglet comptes */
    etats_page_accounts_initialise_onglet (etats_prefs, report_number);

    /* onglet tiers */
    etats_page_payee_initialise_onglet (etats_prefs, report_number);

    /* onglet Categories */
    etats_page_category_initialise_onglet (etats_prefs, report_number);

    /* onglet Budgets */
    etats_page_budget_initialise_onglet (etats_prefs, report_number);

    /* onglet Textes */
	etats_page_text_init_data (etats_prefs, report_number);

	/* onglet Montants */
    etats_page_amount_init_data (etats_prefs, report_number);

    /* onglet modes de paiement */
    etats_prefs_initialise_onglet_mode_paiement (etats_prefs, report_number);

    /* onglet divers */
    etats_prefs_initialise_onglet_divers (etats_prefs, report_number);

    /* onglet data grouping */
    etats_prefs_initialise_onglet_data_grouping (etats_prefs, report_number);

    /* onglet data separation */
    etats_prefs_initialise_onglet_data_separation (etats_prefs, report_number);

    /* onglet generalites */
    etats_prefs_initialise_onglet_affichage_generalites (etats_prefs, report_number);

    /* onglet titres */
    etats_prefs_initialise_onglet_affichage_titres (etats_prefs, report_number);

    /* onglet opérations */
    etats_prefs_initialise_onglet_affichage_operations (etats_prefs, report_number);

    /* onglet devises */
    etats_prefs_initialise_onglet_affichage_devises (etats_prefs, report_number);

    /* return */
    return TRUE;
}


/**
 *
 *
 * \param
 * \param
 *
 * \return
 */
static gboolean etats_config_recupere_info_to_etat (GtkWidget *etats_prefs,
													gint report_number)
{
	/* onglet période */
	etats_page_period_get_info (etats_prefs, report_number);

	/* onglet virements */
	etats_page_transfer_get_info (etats_prefs, report_number);

	/* onglet comptes */
	etats_page_accounts_get_info (etats_prefs, report_number);

	/* onglet tiers */
	etats_page_payee_get_info (etats_prefs, report_number);

	/* onglet Categories */
	etats_page_category_get_info (etats_prefs, report_number);

	/* onglet Budgets */
	etats_page_budget_get_info (etats_prefs, report_number);

	/* onglet Textes */
	etats_page_text_get_data (etats_prefs, report_number);

	/* onglet Montants */
	etats_page_amount_get_data (etats_prefs, report_number);

	/* onglet mode de paiement */
	etats_prefs_recupere_info_onglet_mode_paiement (etats_prefs, report_number);

	/* onglet modes divers */
	etats_prefs_recupere_info_onglet_divers (etats_prefs, report_number);

	/* onglet modes data grouping */
	etats_prefs_recupere_info_onglet_data_grouping (etats_prefs,report_number);

	/* onglet modes data separation */
	etats_prefs_recupere_info_onglet_data_separation (etats_prefs, report_number);

	/* onglet generalites */
	etats_prefs_recupere_info_onglet_affichage_generalites (etats_prefs, report_number);

	/* onglet titres */
	etats_prefs_recupere_info_onglet_affichage_titres (etats_prefs, report_number);

	/* onglet opérations */
	etats_prefs_recupere_info_onglet_affichage_operations (etats_prefs, report_number);

	/* onglet devises */
	etats_prefs_recupere_info_onglet_affichage_devises (etats_prefs, report_number);

	/* on avertit grisbi de la modification à enregistrer */
	gsb_file_set_modified (TRUE);

	/* on réaffiche l'état */
	rafraichissement_etat (report_number);

	/* on repasse à la 1ère page du notebook */
	gtk_notebook_set_current_page (GTK_NOTEBOOK (etats_onglet_get_notebook_etats ()), 0);
	gtk_widget_set_sensitive (gsb_gui_navigation_get_tree_view (), TRUE);

	gsb_gui_navigation_update_report (report_number);

	return TRUE;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * affiche la fenetre de personnalisation
 *
 * \param
 *
 * \return
 */
gint etats_config_personnalisation_etat (void)
{
	GtkWidget *etats_prefs;
	GtkWidget *notebook_general;
	gint current_report_number;
	gint result = 0;

	devel_debug (NULL);

	if (!(current_report_number = gsb_gui_navigation_get_current_report ()))
		return result;

	notebook_general = grisbi_win_get_notebook_general ();
	if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_general)) != GSB_REPORTS_PAGE)
		gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_general), GSB_REPORTS_PAGE);

	/* Création de la fenetre etats_prefs */
	etats_prefs = GTK_WIDGET (etats_prefs_new (GTK_WIDGET (grisbi_app_get_active_window (NULL))));

	if (etats_prefs == NULL)
		return result;

	/* initialisation des données de la fenetre etats_prefs */
	etats_config_initialise_dialog_from_etat (etats_prefs, current_report_number);

	/* on se repositionne sur le dernier onglet si on a le même rapport */
	if (current_report_number == last_report_number)
		etats_prefs_left_panel_tree_view_select_last_page (etats_prefs);

	result = gtk_dialog_run (GTK_DIALOG (etats_prefs));
	switch (result)
	{
		case GTK_RESPONSE_OK:
			etats_config_recupere_info_to_etat (etats_prefs, current_report_number);
			last_report_number = current_report_number;
			break;

		case GTK_RESPONSE_CANCEL:
			{
				GrisbiWinRun *w_run;

				w_run = grisbi_win_get_w_run ();
				if (w_run->empty_report)
				{
					gsb_gui_navigation_remove_report (current_report_number);
					gsb_data_report_remove (current_report_number);
				}
			}
			break;
		default:
			break;
	}

	gtk_widget_destroy (etats_prefs);

	return result;
}

/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
