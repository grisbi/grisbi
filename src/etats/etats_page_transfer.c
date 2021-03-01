/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "etats_page_transfer.h"
#include "dialog.h"
#include "etats_config.h"
#include "etats_prefs.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_prefs.h"
#include "utils_prefs.h"
#include "widgets/widget_list_accounts.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/

typedef struct _EtatsPageTransferPrivate   EtatsPageTransferPrivate;

struct _EtatsPageTransferPrivate
{
	GtkWidget *			vbox_etats_page_transfer;

	GtkWidget *			bouton_inclusion_transfer_actifs;
	GtkWidget *			bouton_inclusion_transfer_hors;
	GtkWidget *			bouton_inclusion_transfer_perso;
	GtkWidget *			bouton_non_inclusion_transfer;

	GtkWidget *			widget_list_accounts;

	GtkWidget *			togglebutton_exclure_non_transfer;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPageTransfer, etats_page_transfer, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Initialise les boutons pour sensibiliser bouton_exclure_non_virements_etat
 *  et la liste des comptes.
 *
 * \param
 *
 * \return
 **/
static void etats_page_transfer_init_buttons_choix_utilisation_virements (EtatsPageTransfer *page,
																		   gint num_page)
{
	EtatsPageTransferPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_transfer_get_instance_private (page);

    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    g_signal_connect (G_OBJECT (priv->bouton_inclusion_transfer_actifs),
                        "toggled",
                        G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
                        GINT_TO_POINTER (num_page));

	/* on connecte le signal pour gérer la sensibilité du bouton bouton_bouton_inclusion_virements_actifs_etat */
    g_signal_connect (G_OBJECT (priv->bouton_inclusion_transfer_actifs),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        priv->togglebutton_exclure_non_transfer);

    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    g_signal_connect (G_OBJECT (priv->bouton_inclusion_transfer_hors),
                        "toggled",
                        G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
                        GINT_TO_POINTER (num_page));

	/* on connecte le signal pour gérer la sensibilité du bouton_inclusion_virements_hors_etat */
    g_signal_connect (G_OBJECT (priv->bouton_inclusion_transfer_hors),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        priv->togglebutton_exclure_non_transfer);

    /* on met la connection pour changer le style de la ligne du panneau de gauche */
    g_signal_connect (G_OBJECT (priv->bouton_inclusion_transfer_perso),
                        "toggled",
                        G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
                        GINT_TO_POINTER (num_page));

	/* on connecte le signal pour gérer la sensibilité du bouton_inclusion_virements_perso */
    g_signal_connect (G_OBJECT (priv->bouton_inclusion_transfer_perso),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        priv->togglebutton_exclure_non_transfer);

	/* on connecte le signal pour gérer l'affichage de la liste des comptes */
    g_signal_connect (G_OBJECT (priv->bouton_inclusion_transfer_perso),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        priv->widget_list_accounts);
}

/**
 * Création de la page de gestion des transfer
 *
 * \param
 * \param
 *
 * \return
 **/
static void etats_page_transfer_setup_page (EtatsPageTransfer *page,
											 GtkWidget *etats_prefs)
{
	GtkWidget *head_page;
	EtatsPageTransferPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_transfer_get_instance_private (page);

	/* set head page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Transfers"), "gsb-transfer-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_transfer), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_transfer), head_page, 0);

	/* init widget list */
	priv->widget_list_accounts = GTK_WIDGET (widget_list_accounts_new (GTK_WIDGET (page)));
	gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_transfer), priv->widget_list_accounts, TRUE, TRUE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_transfer), priv->widget_list_accounts, 5);

    /* on initialise les boutons pour sensibiliser bouton_exclure_non_virements_etat et la liste des comptes */
    etats_page_transfer_init_buttons_choix_utilisation_virements (page, TRANSFER_PAGE_TYPE);

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void etats_page_transfer_init (EtatsPageTransfer *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void etats_page_transfer_dispose (GObject *object)
{
	G_OBJECT_CLASS (etats_page_transfer_parent_class)->dispose (object);
}

static void etats_page_transfer_class_init (EtatsPageTransferClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = etats_page_transfer_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/etats/etats_page_transfer.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageTransfer, vbox_etats_page_transfer);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageTransfer, bouton_inclusion_transfer_actifs);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageTransfer, bouton_inclusion_transfer_hors);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageTransfer, bouton_inclusion_transfer_perso);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageTransfer, bouton_non_inclusion_transfer);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageTransfer, togglebutton_exclure_non_transfer);
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
EtatsPageTransfer *etats_page_transfer_new (GtkWidget *etats_prefs)
{
	EtatsPageTransfer *page;

  	page = g_object_new (ETATS_PAGE_TRANSFER_TYPE, NULL);
  	etats_page_transfer_setup_page (page, etats_prefs);

	return page;
}

/**
 * Initialise les informations de l'onglet virements
 *
 * \param parent
 * \param report_number
 *
 * \return
 **/
void etats_page_transfer_initialise_onglet (GtkWidget *etats_prefs,
											 gint report_number)
{
    gint index;
	EtatsPageTransfer *page;
	EtatsPageTransferPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_TRANSFER (etats_prefs_get_page_by_number (etats_prefs, TRANSFER_PAGE_TYPE));
	priv = etats_page_transfer_get_instance_private (page);

    index = gsb_data_report_get_transfer_choice (report_number);
    utils_radiobutton_set_active_index (priv->bouton_non_inclusion_transfer, index);

    if (index == 3)
    {
		widget_list_accounts_init_liste (priv->widget_list_accounts, report_number, TRANSFER_PAGE_TYPE);
    }
	else
	{
		gtk_widget_set_sensitive (priv->widget_list_accounts, FALSE);
	}

    if (index > 0)
    {
        gtk_widget_set_sensitive (priv->togglebutton_exclure_non_transfer, TRUE);
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->togglebutton_exclure_non_transfer),
									  gsb_data_report_get_transfer_reports_only (report_number));
    }
    else
        gtk_widget_set_sensitive (priv->togglebutton_exclure_non_transfer, FALSE);
}

/**
 * Récupère les informations de l'onglet virements
 *
 * \param parent
 * \param report_number
 *
 * \return
 **/
void etats_page_transfer_get_info (GtkWidget *etats_prefs,
									gint report_number)
{
    gint index;
	EtatsPageTransfer *page;
	EtatsPageTransferPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_TRANSFER (etats_prefs_get_page_by_number (etats_prefs, TRANSFER_PAGE_TYPE));
	priv = etats_page_transfer_get_instance_private (page);

    index = utils_radiobutton_get_active_index (priv->bouton_non_inclusion_transfer);
    gsb_data_report_set_transfer_choice (report_number, index);

    if (index == 3)
    {
        gsb_data_report_free_transfer_account_numbers_list (report_number);
		if (widget_list_accounts_get_info (priv->widget_list_accounts, report_number, TRANSFER_PAGE_TYPE))
        {
			utils_radiobutton_set_active_index (priv->bouton_non_inclusion_transfer, 0);
		}

	}

    gsb_data_report_set_transfer_reports_only (report_number,
											   gtk_toggle_button_get_active
											   (GTK_TOGGLE_BUTTON (priv->togglebutton_exclure_non_transfer)));
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

