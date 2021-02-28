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
#include "etats_page_accounts.h"
#include "etats_prefs.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "utils.h"
#include "utils_prefs.h"
#include "widgets/widget_list_accounts.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

 /*START_STATIC*/
/*END_STATIC*/

typedef struct _EtatsPageAccountsPrivate   EtatsPageAccountsPrivate;

struct _EtatsPageAccountsPrivate
{
	GtkWidget *			vbox_etats_page_accounts;

    GtkWidget *			bouton_detaille_comptes;
	GtkWidget *			vbox_generale_comptes;
	GtkWidget *			widget_list_accounts;
    GtkWidget *         label_comptes_search_help;

	GtkWidget *         treeview_comptes;
    GtkWidget *         togglebutton_select_all_comptes;
    GtkWidget *         button_bank_comptes;
    GtkWidget *         button_cash_comptes;
    GtkWidget *         button_assets_comptes;
    GtkWidget *         button_liabilities_comptes;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPageAccounts, etats_page_accounts, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des accounts
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void etats_page_accounts_setup_page (EtatsPageAccounts *page,
											GtkWidget *etats_prefs)
{
	GtkWidget *head_page;
	EtatsPageAccountsPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_accounts_get_instance_private (page);

	/* set head page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Account selection"), "gsb-ac-bank-32.png");
    gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_accounts), head_page, FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_accounts), head_page, 0);

	/* init widget list */
	priv->widget_list_accounts = GTK_WIDGET (widget_list_accounts_new (GTK_WIDGET (page)));
	gtk_box_pack_start (GTK_BOX (priv->vbox_generale_comptes), priv->widget_list_accounts, TRUE, TRUE, 0);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
    g_signal_connect (G_OBJECT (priv->bouton_detaille_comptes),
					  "toggled",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
					  GINT_TO_POINTER (ACCOUNT_PAGE_TYPE));

    /* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
    g_signal_connect (G_OBJECT (priv->bouton_detaille_comptes),
                        "toggled",
                        G_CALLBACK (sens_desensitive_pointeur),
                        priv->vbox_generale_comptes);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void etats_page_accounts_init (EtatsPageAccounts *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void etats_page_accounts_dispose (GObject *object)
{
	G_OBJECT_CLASS (etats_page_accounts_parent_class)->dispose (object);
}

static void etats_page_accounts_class_init (EtatsPageAccountsClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = etats_page_accounts_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/etats/etats_page_accounts.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAccounts, vbox_etats_page_accounts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAccounts, bouton_detaille_comptes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAccounts, vbox_generale_comptes);
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
EtatsPageAccounts *etats_page_accounts_new (GtkWidget *etats_prefs)
{
	EtatsPageAccounts *page;

  	page = g_object_new (ETATS_PAGE_ACCOUNTS_TYPE, NULL);
	etats_page_accounts_setup_page (page, etats_prefs);

	return page;
}

/**
 *
 *
 * \param parent
 * \param report_number
 *
 * \return
 **/
void etats_page_accounts_initialise_onglet (GtkWidget *etats_prefs,
											gint report_number)
{
    gint active;
	EtatsPageAccounts *page;
	EtatsPageAccountsPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_ACCOUNTS (etats_prefs_get_page_by_number (etats_prefs, ACCOUNT_PAGE_TYPE));
	priv = etats_page_accounts_get_instance_private (page);

    active = gsb_data_report_get_account_use_chosen (report_number);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_comptes), active);

    if (active)
    {
		widget_list_accounts_init_liste (priv->widget_list_accounts, report_number, ACCOUNT_PAGE_TYPE);
    }
}

/**
 *
 *
 * \param parent
 * \param report_number
 *
 * \return
 **/
void etats_page_accounts_get_info (GtkWidget *etats_prefs,
								   gint report_number)
{
    gint active;
	EtatsPageAccounts *page;
	EtatsPageAccountsPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_ACCOUNTS (etats_prefs_get_page_by_number (etats_prefs, ACCOUNT_PAGE_TYPE));
	priv = etats_page_accounts_get_instance_private (page);

    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_comptes));
    gsb_data_report_set_account_use_chosen (report_number, active);

    if (active)
    {
        gsb_data_report_free_account_numbers_list (report_number);

        if (widget_list_accounts_get_info (priv->widget_list_accounts, report_number, ACCOUNT_PAGE_TYPE))
        {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_comptes), FALSE);
		}
    }
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

