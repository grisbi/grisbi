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
#include "etats_page_amount.h"
#include "etats_config.h"
#include "etats_prefs.h"
#include "gsb_data_report.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_widgets.h"
#include "widget_cmp_amount.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _EtatsPageAmountPrivate   EtatsPageAmountPrivate;

struct _EtatsPageAmountPrivate
{
	GtkWidget *			vbox_etats_page_amount;

	GtkWidget *			button_detaille_amount;
	GtkWidget *			button_exclure_ope_nulles;
	GtkWidget *			vbox_detaille_amount;
	GtkWidget *			vbox_rows_amount;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPageAmount, etats_page_amount, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * ajoute une ligne de recherche de montant à la liste des lignes de recherche de montant
 *
 * \param last_amount_comparison_number
 * \param report number
 * \param priv
 *
 *\return
 */
static void etats_page_amount_ajoute_ligne_liste_comparaisons (gint last_amount_comparison_number,
															   gint report_number,
															   EtatsPageAmount *page)
{
	GtkWidget *widget;
	gint amount_comparison_number;
	gint position;
	gboolean first_cmp_line;
	EtatsPageAmountPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_amount_get_instance_private (page);

	/* on récupère tout de suite la position à laquelle il faut insérer la row */
	if (last_amount_comparison_number)
	{
		position = g_slist_index (gsb_data_report_get_amount_comparison_list (report_number),
								  GINT_TO_POINTER (last_amount_comparison_number)) + 1;
		first_cmp_line = FALSE;
	}
	else
	{
		position = 0;
		first_cmp_line = TRUE;
	}

	/*si c'est la deuxième ligne on affiche le bouton remove sur la première ligne */
	if (position == 1)
	{
		widget = gsb_data_report_amount_comparison_get_widget (last_amount_comparison_number);
		widget_cmp_amount_show_button_remove (widget, TRUE);
	}

	/* on commence par créer une structure vide */
	amount_comparison_number = gsb_data_report_amount_comparison_new (0);
	gsb_data_report_amount_comparison_set_report_number (amount_comparison_number, report_number);

	/* on crée la row et remplit les widget de la structure */
	widget = GTK_WIDGET (widget_cmp_amount_new (GTK_WIDGET (page), amount_comparison_number, first_cmp_line));
	widget_cmp_amount_show_button_add (widget, TRUE);
	widget_cmp_amount_show_button_remove (widget, TRUE);
	gtk_box_pack_start (GTK_BOX (priv->vbox_rows_amount), widget, FALSE, FALSE, 0);
	gsb_data_report_amount_comparison_set_widget (amount_comparison_number, widget);

	/* on met la structure dans la liste à la position demandée */
	gsb_data_report_set_amount_comparison_list (report_number,
												g_slist_insert (gsb_data_report_get_amount_comparison_list
																(report_number),
																GINT_TO_POINTER (amount_comparison_number),
																position));


	/* on met la row à sa place dans la liste */
	gtk_box_reorder_child (GTK_BOX (priv->vbox_rows_amount), widget, position);
}

/**
 * supprime une liste de comparaison
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void etats_page_amount_retire_ligne_liste_comparaisons (gint last_amount_comparison_number,
   															   gint report_number,
															   EtatsPageAmount *page)
{
	gboolean remove_first_line = FALSE;

	/* il faut qu'il y ai plus d'une row affichée */
	if (g_slist_length (gsb_data_report_get_amount_comparison_list (report_number)) < 2)
		return;

	/* on commence par supprimer la row dans la liste */
	gtk_widget_destroy (gsb_data_report_amount_comparison_get_widget (last_amount_comparison_number));

	/* si la structure qu'on retire est la 1ère, on vire le widget de lien */
	if (!g_slist_index (gsb_data_report_get_amount_comparison_list (report_number),
						GINT_TO_POINTER (last_amount_comparison_number)))
	{
		remove_first_line = TRUE;
	}

	/* et on retire la struct de la sliste */
	gsb_data_report_set_amount_comparison_list (report_number,
												g_slist_remove (gsb_data_report_get_amount_comparison_list
																(report_number),
																GINT_TO_POINTER (last_amount_comparison_number)));

	/* si c'est la première ligne qui a été supprimée on cache button_link */
	if (remove_first_line)
	{
		gint amount_comparison_number;
		GtkWidget *widget;

		amount_comparison_number = GPOINTER_TO_INT (g_slist_nth_data (gsb_data_report_get_amount_comparison_list
																	  (report_number), 0));
		widget = gsb_data_report_amount_comparison_get_widget (amount_comparison_number);
		widget_cmp_amount_hide_button_link (widget);

		/*si il ne reste qu'une ligne on cache le bouton remove */
		if (g_slist_length (gsb_data_report_get_amount_comparison_list (report_number)) == 1)
			widget_cmp_amount_show_button_remove (widget, FALSE);

		return;
	}

	/*si il ne reste qu'une ligne on cache le bouton remove */
	if (g_slist_length (gsb_data_report_get_amount_comparison_list (report_number)) == 1)
	{
		gint amount_comparison_number;
		GtkWidget *widget;

		amount_comparison_number = GPOINTER_TO_INT (g_slist_nth_data (gsb_data_report_get_amount_comparison_list
																	(report_number), 0));
		widget = gsb_data_report_amount_comparison_get_widget (amount_comparison_number);
		widget_cmp_amount_show_button_remove (widget, FALSE);
	}
}

/**
 * remplit la liste des comparaisons de montants
 *
 * \param report_number
 * \param
 *
 * \return
 */
static void etats_page_amount_remplit_liste_comparaisons (gint report_number,
														  EtatsPageAmount *page)
{
	GSList *tmp_list;
	guint nbre_ligne;
	guint num_ligne = 1;
	gboolean first_cmp_line = TRUE;
	EtatsPageAmountPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_amount_get_instance_private (page);

	/* on commence par effacer les anciennes lignes */
	gtk_container_foreach (GTK_CONTAINER (priv->vbox_rows_amount),
						   (GtkCallback) utils_widget_gtk_callback,
						   NULL);

	tmp_list = gsb_data_report_get_amount_comparison_list (report_number);
	nbre_ligne = g_slist_length (tmp_list);

	/*   s'il n'y a rien dans la liste, on met juste une row vide */
	if (!tmp_list)
	{
		etats_page_amount_ajoute_ligne_liste_comparaisons (0, report_number, page);

		return;
	}

	/* on fait le tour de la liste des comparaisons de montant, ajoute une row
	 * et la remplit à chaque fois */
	while (tmp_list)
	{
		GtkWidget *widget;
		gint amount_comparison_number;

		amount_comparison_number = GPOINTER_TO_INT (tmp_list->data);

		/* on crée la row et on l'ajoute dans la fenêtre */
		widget = GTK_WIDGET (widget_cmp_amount_new (GTK_WIDGET (page), amount_comparison_number, first_cmp_line));
		gtk_box_pack_start (GTK_BOX (priv->vbox_rows_amount), widget, FALSE, FALSE, 0);
		gsb_data_report_amount_comparison_set_widget (amount_comparison_number, widget);

		/* on remplit le widget de la structure */
		widget_cmp_amount_init_data (widget, amount_comparison_number);

		if (nbre_ligne > 1 && num_ligne == 1)
			widget_cmp_amount_show_button_remove (widget, TRUE);

		first_cmp_line = FALSE;
		num_ligne ++;

		tmp_list = tmp_list->next;
	}
}

/**
 * Création de la page de gestion des amount
 *
 * \param
 * \param
 *
 * \return
 **/
static void etats_page_amount_setup_page (EtatsPageAmount *page,
										  GtkWidget *etats_prefs)
{
	GtkWidget *head_page;
	EtatsPageAmountPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_amount_get_instance_private (page);

	/* set head page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Amount"), "gsb-amount-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_amount), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_amount), head_page, 0);

	/* set signals */
	g_object_set_data (G_OBJECT (priv->button_detaille_amount), "etats_prefs", etats_prefs);
	g_signal_connect (G_OBJECT (priv->button_detaille_amount),
					  "toggled",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
					  GINT_TO_POINTER (AMOUNT_PAGE_TYPE));

	/* on met la connection pour rendre sensitif la vbox_generale_amountes_etat */
	g_signal_connect (G_OBJECT (priv->button_detaille_amount),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->vbox_rows_amount);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void etats_page_amount_init (EtatsPageAmount *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void etats_page_amount_dispose (GObject *object)
{
	G_OBJECT_CLASS (etats_page_amount_parent_class)->dispose (object);
}

static void etats_page_amount_class_init (EtatsPageAmountClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = etats_page_amount_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/etats/etats_page_amount.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAmount, vbox_etats_page_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAmount, button_detaille_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAmount, button_exclure_ope_nulles);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAmount, vbox_detaille_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageAmount, vbox_rows_amount);
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
EtatsPageAmount *etats_page_amount_new (GtkWidget *etats_prefs)
{
	EtatsPageAmount *page;

	page = g_object_new (ETATS_PAGE_AMOUNT_TYPE, NULL);
	etats_page_amount_setup_page (page, etats_prefs);

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
void etats_page_amount_get_data (GtkWidget *etats_prefs,
								 gint report_number)
{
	GSList *comparison_list;
	gint active;
	guint nbre_ligne;
	EtatsPageAmount *page;
	EtatsPageAmountPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_AMOUNT (etats_prefs_get_page_by_number (etats_prefs, AMOUNT_PAGE_TYPE));
	priv = etats_page_amount_get_instance_private (page);

	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->button_detaille_amount));
	gsb_data_report_set_amount_comparison_used (report_number, active);

	comparison_list = gsb_data_report_get_amount_comparison_list (report_number);
	nbre_ligne = g_slist_length (comparison_list);

	while (comparison_list)
	{
		GtkWidget *widget;
		gint amount_comparison_number;
		gboolean result = TRUE;

		amount_comparison_number = GPOINTER_TO_INT (comparison_list->data);
		widget = GTK_WIDGET (gsb_data_report_amount_comparison_get_widget (amount_comparison_number));
		result = widget_cmp_amount_get_data (widget, amount_comparison_number);

		if (nbre_ligne == 1 && result == FALSE)
		{
			g_slist_free (gsb_data_report_get_amount_comparison_list (report_number));
			gsb_data_report_set_amount_comparison_list (report_number, NULL);
			gsb_data_report_set_amount_comparison_used (report_number, 0);

			return;
		}
		else
		{
			if (result == FALSE)
			{
				/* voir si traitement à faire */
			}
		}

		comparison_list = comparison_list->next;
	}

	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->button_exclure_ope_nulles));
	gsb_data_report_set_amount_comparison_only_report_non_null (report_number, active);
}

/**
 *
 *
 * \param parent
 * \param report_number
 *
 * \return
 **/
void etats_page_amount_init_data (GtkWidget *etats_prefs,
								  gint report_number)
{
	gint active = 0;
	EtatsPageAmount *page;
	EtatsPageAmountPrivate *priv;

	page = ETATS_PAGE_AMOUNT (etats_prefs_get_page_by_number (etats_prefs, AMOUNT_PAGE_TYPE));
	priv = etats_page_amount_get_instance_private (page);

	active = gsb_data_report_get_amount_comparison_used (report_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_detaille_amount), active);

	/* on remplit la liste des lignes de recherche */
	etats_page_amount_remplit_liste_comparaisons (report_number, page);

}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean etats_page_amount_line_add (gint amount_comparison_number,
									 GtkWidget *page)
{
	gint report_number;

	report_number = gsb_gui_navigation_get_current_report ();
	etats_page_amount_ajoute_ligne_liste_comparaisons (amount_comparison_number,
													   report_number,
													   ETATS_PAGE_AMOUNT (page));

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean etats_page_amount_line_remove (gint amount_comparison_number,
										GtkWidget *page)
{
	gint report_number;

	report_number = gsb_gui_navigation_get_current_report ();
	etats_page_amount_retire_ligne_liste_comparaisons (amount_comparison_number,
													   report_number,
													   ETATS_PAGE_AMOUNT (page));
	return FALSE;
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

