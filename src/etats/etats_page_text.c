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
#include "etats_page_text.h"
#include "etats_config.h"
#include "etats_prefs.h"
#include "gsb_data_report.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "utils_widgets.h"
#include "widget_cmp_text.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _EtatsPageTextPrivate   EtatsPageTextPrivate;

struct _EtatsPageTextPrivate
{
	GtkWidget *			vbox_etats_page_text;

	GtkWidget *			button_detaille_text;
	GtkWidget *			vbox_detaille_text;
	GtkWidget *			vbox_rows_text;
};

G_DEFINE_TYPE_WITH_PRIVATE (EtatsPageText, etats_page_text, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * ajoute une ligne de recherche de texte à la liste des lignes de recherche de texte
 *
 * \param last_text_comparison_number
 * \param report_number
 * \param page
 *
 * \return
 */
static void etats_page_text_ajoute_ligne_liste_comparaisons (gint last_text_comparison_number,
															 gint report_number,
															 EtatsPageText *page)
{
	GtkWidget *widget;
	gint text_comparison_number;
	gint position;
	gboolean first_cmp_line;
	EtatsPageTextPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_text_get_instance_private (page);

	/* on récupère tout de suite la position à laquelle il faut insérer la ligne */
	if (last_text_comparison_number)
	{
		position = g_slist_index (gsb_data_report_get_text_comparison_list (report_number),
								  GINT_TO_POINTER (last_text_comparison_number)) + 1;
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
		widget = gsb_data_report_text_comparison_get_widget (last_text_comparison_number);
		widget_cmp_text_show_button_remove (widget, TRUE);
	}

	/* on commence par créer une structure vide */
	text_comparison_number = gsb_data_report_text_comparison_new (0);
	gsb_data_report_text_comparison_set_report_number (text_comparison_number, report_number);

	/* on crée la row et affiche les boutons Ajouter et Enlever */
	widget = GTK_WIDGET (widget_cmp_text_new (GTK_WIDGET (page), text_comparison_number, first_cmp_line));
	widget_cmp_text_show_button_add (widget, TRUE);
	widget_cmp_text_show_button_remove (widget, TRUE);
	gtk_box_pack_start (GTK_BOX (priv->vbox_rows_text), widget, FALSE, FALSE, 0);
	gsb_data_report_text_comparison_set_widget (text_comparison_number, widget);

	/* on met la structure dans la liste à la position demandée */
	gsb_data_report_set_text_comparison_list (report_number,
											  g_slist_insert (gsb_data_report_get_text_comparison_list
															  (report_number),
															  GINT_TO_POINTER (text_comparison_number),
															  position));

	/* on met la row à sa place dans la liste */
	gtk_box_reorder_child (GTK_BOX (priv->vbox_rows_text), widget, position);
}

/**
 * Supprime une ligne de comparaison de texte
 *
 * \param last_text_comparison_number
 * \param report_number
 * \param page
 *
 * \return
 */
static void etats_page_text_retire_ligne_liste_comparaisons (gint last_text_comparison_number,
															 gint report_number,
															 EtatsPageText *page)
{
	gboolean remove_first_line = FALSE;

	/* il faut qu'il y ai plus d'une row affichée */
	if (g_slist_length (gsb_data_report_get_text_comparison_list (report_number)) < 2)
		return;

	/* on commence par supprimer la row dans la liste */
	gtk_widget_destroy (gsb_data_report_text_comparison_get_widget (last_text_comparison_number));

	/* si la structure qu'on retire est la 1ère, on vire le widget de lien */
	if (!g_slist_index (gsb_data_report_get_text_comparison_list (report_number),
						GINT_TO_POINTER (last_text_comparison_number)))
	{
		remove_first_line = TRUE;
	}

	/* et on retire la struct de la sliste */
	gsb_data_report_set_text_comparison_list (report_number,
											  g_slist_remove (gsb_data_report_get_text_comparison_list
															  (report_number),
															  GINT_TO_POINTER (last_text_comparison_number)));

	/* si c'est la première ligne qui a été supprimée on cache button_link */
	if (remove_first_line)
	{
		gint text_comparison_number;
		GtkWidget *widget;
		text_comparison_number = GPOINTER_TO_INT (g_slist_nth_data (gsb_data_report_get_text_comparison_list
																	(report_number), 0));
		widget = gsb_data_report_text_comparison_get_widget (text_comparison_number);
		widget_cmp_text_hide_button_link (widget);

		/*si il ne reste qu'une ligne on cache le bouton remove */
		if (g_slist_length (gsb_data_report_get_text_comparison_list (report_number)) == 1)
			widget_cmp_text_show_button_remove (widget, FALSE);

		return;
	}

	/*si il ne reste qu'une ligne on cache le bouton remove */
	if (g_slist_length (gsb_data_report_get_text_comparison_list (report_number)) == 1)
	{
		gint text_comparison_number;
		GtkWidget *widget;

		text_comparison_number = GPOINTER_TO_INT (g_slist_nth_data (gsb_data_report_get_text_comparison_list
																	(report_number), 0));
		widget = gsb_data_report_text_comparison_get_widget (text_comparison_number);
		widget_cmp_text_show_button_remove (widget, FALSE);
	}
}

/**
 * remplit la liste des comparaisons de texte
 *
 * \param report_number
 * \param page
 *
 * \return
 */
static void etats_page_text_remplit_liste_comparaisons (gint report_number,
														EtatsPageText *page)
{
	GSList *tmp_list;
	guint nbre_ligne;
	guint num_ligne = 1;
	gboolean first_cmp_line = TRUE;
	EtatsPageTextPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_text_get_instance_private (page);

	/* on commence par effacer les anciennes lignes */
	gtk_container_foreach (GTK_CONTAINER (priv->vbox_rows_text),
						   (GtkCallback) utils_widget_gtk_callback,
						   NULL);

	tmp_list = gsb_data_report_get_text_comparison_list (report_number);
	nbre_ligne = g_slist_length (tmp_list);

	/* s'il n'y a rien dans la liste, on met juste une row vide */
	if (!tmp_list)
	{
		etats_page_text_ajoute_ligne_liste_comparaisons (0, report_number, page);

 		return;
	}

	/* on fait le tour de la liste des comparaisons de texte, ajoute une row et la remplit à chaque fois */
	while (tmp_list)
	{
		GtkWidget *widget;
		gint text_comparison_number;

		text_comparison_number = GPOINTER_TO_INT (tmp_list->data);

		/* on crée la row et on l'ajoute dans la fenêtre */
		widget = GTK_WIDGET (widget_cmp_text_new (GTK_WIDGET (page), text_comparison_number, first_cmp_line));
		gtk_box_pack_start (GTK_BOX (priv->vbox_rows_text), widget, FALSE, FALSE, 0);
		gsb_data_report_text_comparison_set_widget (text_comparison_number, widget);

		/* on remplit le widget de la structure */
		widget_cmp_text_init_data (widget, text_comparison_number);

		if (nbre_ligne > 1 && num_ligne == 1)
			widget_cmp_text_show_button_remove (widget, TRUE);

		first_cmp_line = FALSE;
		num_ligne ++;

		tmp_list = tmp_list->next;
	}
}

/**
 * Création de la page de gestion des text
 *
 * \param
 * \param
 *
 * \return
 **/
static void etats_page_text_setup_page (EtatsPageText *page,
										GtkWidget *etats_prefs)
{
	GtkWidget *head_page;
	EtatsPageTextPrivate *priv;

	devel_debug (NULL);
	priv = etats_page_text_get_instance_private (page);

	/* set head page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Transaction content"), "gsb-text-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_etats_page_text), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_etats_page_text), head_page, 0);

	/* set signals */
	g_object_set_data (G_OBJECT (priv->button_detaille_text), "etats_prefs", etats_prefs);
	g_signal_connect (G_OBJECT (priv->button_detaille_text),
			  		  "toggled",
					  G_CALLBACK (etats_prefs_left_panel_tree_view_update_style),
					  GINT_TO_POINTER (TEXT_PAGE_TYPE));

	/* on met la connection pour rendre sensitif la vbox_generale_textes_etat */
	g_signal_connect (G_OBJECT (priv->button_detaille_text),
					  "toggled",
					  G_CALLBACK (sens_desensitive_pointeur),
					  priv->vbox_rows_text);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void etats_page_text_init (EtatsPageText *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void etats_page_text_dispose (GObject *object)
{
	G_OBJECT_CLASS (etats_page_text_parent_class)->dispose (object);
}

static void etats_page_text_class_init (EtatsPageTextClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = etats_page_text_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/etats/etats_page_text.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageText, vbox_etats_page_text);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageText, button_detaille_text);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageText, vbox_detaille_text);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), EtatsPageText, vbox_rows_text);
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
EtatsPageText *etats_page_text_new (GtkWidget *etats_prefs)
{
	EtatsPageText *page;

	page = g_object_new (ETATS_PAGE_TEXT_TYPE, NULL);
	etats_page_text_setup_page (page, etats_prefs);

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
void etats_page_text_get_data (GtkWidget *etats_prefs,
							   gint report_number)
{
	GSList *comparison_list;
	gint active;
	EtatsPageText *page;
	EtatsPageTextPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_TEXT (etats_prefs_get_page_by_number (etats_prefs, TEXT_PAGE_TYPE));
	priv = etats_page_text_get_instance_private (page);

	/* on récupère le choix des textes */
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->button_detaille_text));
	gsb_data_report_set_text_comparison_used (report_number, active);

	/* récupération de la liste des comparaisons de texte */
	/* on a rentré au moins une comparaison */
	/* on rempli les champs de la structure */
	comparison_list = gsb_data_report_get_text_comparison_list (report_number);

	while (comparison_list)
	{
		GtkWidget *widget;
		gint text_comparison_number;

		text_comparison_number = GPOINTER_TO_INT (comparison_list->data);
		widget = GTK_WIDGET (gsb_data_report_text_comparison_get_widget (text_comparison_number));
		widget_cmp_text_get_data (widget, text_comparison_number);

		comparison_list = comparison_list->next;
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
void etats_page_text_init_data (GtkWidget *etats_prefs,
								gint report_number)
{
	gint active;
	EtatsPageText *page;
	EtatsPageTextPrivate *priv;

	devel_debug_int (report_number);
	page = ETATS_PAGE_TEXT (etats_prefs_get_page_by_number (etats_prefs, TEXT_PAGE_TYPE));
	priv = etats_page_text_get_instance_private (page);

	active = gsb_data_report_get_text_comparison_used (report_number);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button_detaille_text), active);

	/* on remplit la liste des lignes de recherche */
	etats_page_text_remplit_liste_comparaisons (report_number, page);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean etats_page_text_line_add (gint text_comparison_number,
								   GtkWidget *page)
{
	gint report_number;

	report_number = gsb_gui_navigation_get_current_report ();
	etats_page_text_ajoute_ligne_liste_comparaisons (text_comparison_number,
													 report_number,
													 ETATS_PAGE_TEXT (page));

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
gboolean etats_page_text_line_remove (gint text_comparison_number,
									  GtkWidget *page)
{
	gint report_number;

	report_number = gsb_gui_navigation_get_current_report ();
	etats_page_text_retire_ligne_liste_comparaisons (text_comparison_number,
													 report_number,
													 ETATS_PAGE_TEXT (page));

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
