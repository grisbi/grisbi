/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                                */
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
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "etats_page_budget.h"
#include "dialog.h"
#include "etats_config.h"
#include "etats_prefs.h"
#include "gsb_data_report.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_prefs.h"
#include "widget_list_metatree.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _EtatsPageBudgetPrivate   EtatsPageBudgetPrivate;

struct _EtatsPageBudgetPrivate
{
	GtkWidget *			vbox_etats_page_budget;

	GtkWidget *			bouton_detaille_budget;
	GtkWidget *			vbox_detaille_budget;
	GtkWidget *			widget_list_metatree;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPageBudget, etats_page_budget, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des budget
 *
 * \param
 * \return
 **/
static void etats_page_budget_setup_page (EtatsPageBudget *page,
										  GtkWidget *etats_prefs)
{
	GtkWidget *head_page;
	EtatsPageBudgetPrivate *priv;

	devel_debug (NULL);

	priv = etats_page_budget_get_instance_private (page);

	/* set head page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Budgetary lines"), "gsb-budgetary_lines-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_budget), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_budget), head_page, 0);

	/* init widget list */
	priv->widget_list_metatree = GTK_WIDGET (widget_list_metatree_new (GTK_WIDGET (page), BUDGET_PAGE_TYPE));
	gtk_box_pack_start (GTK_BOX (priv->vbox_detaille_budget), priv->widget_list_metatree, TRUE, TRUE, 0);

	/* on met la connection pour changer le style de la ligne du panneau de gauche */
	g_object_set_data (G_OBJECT (priv->bouton_detaille_budget), "etats_prefs", etats_prefs);
	g_signal_connect (G_OBJECT (priv->bouton_detaille_budget),
					  "toggled",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
					  GINT_TO_POINTER (BUDGET_PAGE_TYPE));

	/* on met la connection pour rendre sensitif la vbox_generale_comptes_etat */
	g_signal_connect (G_OBJECT (priv->bouton_detaille_budget),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->vbox_detaille_budget);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void etats_page_budget_init (EtatsPageBudget *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void etats_page_budget_dispose (GObject *object)
{
	G_OBJECT_CLASS (etats_page_budget_parent_class)->dispose (object);
}

static void etats_page_budget_class_init (EtatsPageBudgetClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = etats_page_budget_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/etats/etats_page_budget.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageBudget, vbox_etats_page_budget);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageBudget, bouton_detaille_budget);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageBudget, vbox_detaille_budget);
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
EtatsPageBudget *etats_page_budget_new (GtkWidget *etats_prefs)
{
	EtatsPageBudget *page;

  	page = g_object_new (ETATS_PAGE_BUDGET_TYPE, NULL);
  	etats_page_budget_setup_page (page, etats_prefs);

	return page;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void etats_page_budget_initialise_onglet (GtkWidget *etats_prefs,
										  gint report_number)
{
	gint active;
	EtatsPageBudget *page;
	EtatsPageBudgetPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_BUDGET (etats_prefs_get_page_by_number (etats_prefs, BUDGET_PAGE_TYPE));
	priv = etats_page_budget_get_instance_private (page);

	active = gsb_data_report_get_budget_detail_used (report_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_budget), active);
	gtk_widget_set_sensitive (priv->vbox_detaille_budget, active);

	if (active)
	{
		widget_list_metatree_init_liste (priv->widget_list_metatree, report_number, BUDGET_PAGE_TYPE);
	}
}

/**
 * Récupère les informations de l'onglet catégorie
 *
 * \param etats_prefs
 * \param report_number
 *
 * \return
 */
void etats_page_budget_get_info (GtkWidget *etats_prefs,
								 gint report_number)
{
	gint active;
	EtatsPageBudget *page;
	EtatsPageBudgetPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_BUDGET (etats_prefs_get_page_by_number (etats_prefs, BUDGET_PAGE_TYPE));
	priv = etats_page_budget_get_instance_private (page);

	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_budget));
	gsb_data_report_set_budget_detail_used (report_number, active);
	if (active)
	{
		gboolean is_all_selected = FALSE;

		is_all_selected = widget_list_metatree_get_info (priv->widget_list_metatree,
														 report_number,
														 BUDGET_PAGE_TYPE);
		if (is_all_selected)
		{
			dialogue_hint (_("All budgets have been selected.  Grisbi will run faster without the "
							 "\"Detail budgets used\" option activated."),
						   _("Performance issue."));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->bouton_detaille_budget), FALSE);
			gsb_data_report_set_budget_detail_used (report_number, FALSE);
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

