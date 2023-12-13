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
#include "widget_cmp_text.h"
#include "etats_page_text.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_file.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetCmpTextPrivate   WidgetCmpTextPrivate;

struct _WidgetCmpTextPrivate
{
	GtkWidget *			vbox_cmp_text;

	GtkWidget *			hbox_field_type_text;
	GtkWidget *			button_field;
	GtkWidget *			button_link;

	GtkWidget *			hbox_use_text;
	GtkWidget *			button_operator;
	GtkWidget *			entry_text;

	GtkWidget *			vbox_use_number;
	GtkWidget *			hbox_first_part;
	GtkWidget *			button_first_comparison;
	GtkWidget *			entry_first_amount;
	GtkWidget *			button_link_first_to_second_part;

	GtkWidget *			hbox_second_part;
	GtkWidget *			button_second_comparison;
	GtkWidget *			entry_second_amount;

	GtkWidget *			hbox_buttons_line;
	GtkWidget *			button_add_line;
	GtkWidget *			button_remove_line;
	GtkWidget *			separator_line;

	/* memorise les donnes de base */
	gboolean			hide_button_link;
	gint				text_cmp_number;
	gint				field_type_text;		/* selecteur de type de champ recherché */

	/* position de hbox_buttons_line */
	gint				old_position;
};

enum ButtonsPosition
{
	DETACHED,
	TEXT_LINE,
	NUMBER_LINE,
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetCmpText, widget_cmp_text, GTK_TYPE_BOX)
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
static void widget_cmp_text_button_add_line_clicked (GtkButton *button,
													 GtkWidget *page)
{
	gint text_cmp_number;

	text_cmp_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "text_cmp_number"));
	etats_page_text_line_add (text_cmp_number, page);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_cmp_text_button_remove_line_clicked (GtkButton *button,
														GtkWidget *page)
{
	gint text_cmp_number;

	text_cmp_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "text_cmp_number"));
	etats_page_text_line_remove (text_cmp_number, page);
}

/**
 * Affichage des buttons  "Ajouter" "Enlever"
 *
 * \param combo_box
 * \param priv
 *
 * \return
 */
static void widget_cmp_text_display_add_remove_buttons (gint position,
														WidgetCmpTextPrivate *priv)
{
	devel_debug_int (position);

	if (position == NUMBER_LINE)
	{
		/* on réaffecte la boite de boutons hbox_buttons_line sur la ligne 1 de la recherche de nombre */
		gtk_widget_unparent (priv->hbox_buttons_line);
		gtk_container_remove (GTK_CONTAINER (priv->hbox_use_text), priv->hbox_buttons_line);
		gtk_box_pack_start (GTK_BOX (priv->hbox_first_part), priv->hbox_buttons_line, FALSE, FALSE, 0);
	}
	else if (position == TEXT_LINE)
	{
		gint index;		/* sert à savoir si on a une ou deux lignes numériques */

		index = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->button_link_first_to_second_part));

		/* on réaffecte la boite de boutons hbox_buttons_line */
		gtk_widget_unparent (priv->hbox_buttons_line);
		if (index == 3)
			gtk_container_remove (GTK_CONTAINER (priv->hbox_first_part), priv->hbox_buttons_line);
		else
			gtk_container_remove (GTK_CONTAINER (priv->hbox_second_part), priv->hbox_buttons_line);

		gtk_box_pack_start (GTK_BOX (priv->hbox_use_text), priv->hbox_buttons_line, FALSE, FALSE, 0);
	}
}

/**
 * Rend sensitif le choix de la suite de la comparaison de numéro
 *
 * \param combo_box
 * \param priv
 *
 * \return
 */
static void widget_cmp_text_button_first_comparison_changed (GtkComboBox *combo,
															 WidgetCmpTextPrivate *priv)
{
	gint index;

	index = gtk_combo_box_get_active (combo);
	if (index == 0)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_link_first_to_second_part), 3);
		gtk_widget_set_sensitive (priv->button_link_first_to_second_part, FALSE);
	}
	else
		gtk_widget_set_sensitive (priv->button_link_first_to_second_part, TRUE);
}

/**
 * Rend visible la suite de la comparaison de numéro
 *
 * \param combo_box
 * \param priv
 *
 * \return
 */
static void widget_cmp_text_button_link_first_to_second_part_changed (GtkComboBox *combo,
																	  WidgetCmpTextPrivate *priv)
{
	gint index;

	index = gtk_combo_box_get_active (combo);
	if (index == 3)
	{
		gtk_widget_hide (priv->hbox_second_part);

		/* on réaffecte la boite de boutons hbox_buttons_line */
		gtk_widget_unparent (priv->hbox_buttons_line);
		gtk_container_remove (GTK_CONTAINER (priv->hbox_second_part), priv->hbox_buttons_line);
		gtk_box_pack_start (GTK_BOX (priv->hbox_first_part), priv->hbox_buttons_line, FALSE, FALSE, 0);
	}
	else
	{
		gtk_widget_show (priv->hbox_second_part);

		/* on réaffecte la boite de boutons hbox_buttons_line */
		gtk_widget_unparent (priv->hbox_buttons_line);
		gtk_container_remove (GTK_CONTAINER (priv->hbox_first_part), priv->hbox_buttons_line);
		gtk_box_pack_start (GTK_BOX (priv->hbox_second_part), priv->hbox_buttons_line, FALSE, FALSE, 0);
	}
}

/**
 * Rend visible la ligne en fonction du choix du combo_box type_text
 *
 * \param combo_box
 * \param priv
 *
 * \return
 */
static void widget_cmp_text_combo_type_text_changed (GtkComboBox *combo,
													 WidgetCmpTextPrivate *priv)
{
	gint field_type_text;

	devel_debug (NULL);
	field_type_text = gtk_combo_box_get_active (combo);
	if (field_type_text == 9) /* recherche du numero de cheque */
	{
		priv->field_type_text = field_type_text;
		gtk_widget_hide (priv->hbox_use_text);
		gtk_widget_show (priv->vbox_use_number);

		/* on réaffecte la boite de boutons hbox_buttons_line */
		widget_cmp_text_display_add_remove_buttons (NUMBER_LINE, priv);
		gsb_data_report_text_comparison_set_use_text (priv->text_cmp_number, FALSE);
	}
	else  /* autres choix : recherche de texte */
	{
		priv->field_type_text = field_type_text;
		gtk_widget_show (priv->hbox_use_text);
		gtk_widget_hide (priv->vbox_use_number);

		/* on réaffecte la boite de boutons hbox_buttons_line */
		widget_cmp_text_display_add_remove_buttons (TEXT_LINE, priv);
		gsb_data_report_text_comparison_set_use_text (priv->text_cmp_number, TRUE);
	}
}

/**
 * Création du widget de gestion des cmp_text
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_cmp_text_setup_widget (WidgetCmpText *widget,
										  GtkWidget *page,
										  gint text_cmp_number,
										  gboolean first_cmp_line)
{
	WidgetCmpTextPrivate *priv;

	devel_debug (NULL);

	priv = widget_cmp_text_get_instance_private (widget);

	/* on ajoute les bouton "Ajouter" et "Enlever" */
	gtk_box_pack_start (GTK_BOX (priv->hbox_use_text), priv->hbox_buttons_line, FALSE, FALSE, 0);

	/* select widgets */
	if (first_cmp_line)
	{
		/* on cache button_link */
		gtk_widget_hide (priv->button_link);
		priv->hide_button_link = TRUE;

		/* on cache le bouton "Enlever" */
		gtk_widget_hide (priv->button_remove_line);

		/* on cache le separateur de ligne */
		gtk_widget_hide (priv->separator_line);
	}

	/* memorise le numero de comparaison */
	priv->text_cmp_number = text_cmp_number;

	/* on positionne le type de champ recherché à 0 par défaut */
	priv->field_type_text = 0;

	/* on cache la recherche numerique */
	gtk_widget_hide (priv->hbox_second_part);

	/* on cache la deuxième ligne de la recherche numerique */
	gtk_widget_hide (priv->vbox_use_number);

	/* on attache aux boutons add et remove le numero de comparaison */
	g_object_set_data (G_OBJECT (priv->button_add_line),
					   "text_cmp_number",
					   GINT_TO_POINTER (text_cmp_number));
	g_object_set_data (G_OBJECT (priv->button_remove_line),
					   "text_cmp_number",
					   GINT_TO_POINTER (text_cmp_number));

	/* set signals */
	/* action a faire lorsque l'on change le choix du combobox button_field */
	g_signal_connect (G_OBJECT (priv->button_field),
					  "changed",
					  G_CALLBACK (widget_cmp_text_combo_type_text_changed),
					  priv);

	/* action a faire lorsque l'on change le choix du combobox button_first_comparison */
	g_signal_connect (G_OBJECT (priv->button_first_comparison),
					  "changed",
					  G_CALLBACK (widget_cmp_text_button_first_comparison_changed),
					  priv);

	/* action a faire lorsque l'on change le choix du combobox button_link_first_to_second_part */
	g_signal_connect (G_OBJECT (priv->button_link_first_to_second_part),
					  "changed",
					  G_CALLBACK (widget_cmp_text_button_link_first_to_second_part_changed),
					  priv);

	/* signal pour le bouton button_add_line */
	g_signal_connect (G_OBJECT (priv->button_add_line),
					  "clicked",
					  G_CALLBACK (widget_cmp_text_button_add_line_clicked),
					  page);

	/* signal pour le bouton button_remove_line */
	g_signal_connect (G_OBJECT (priv->button_remove_line),
					  "clicked",
					  G_CALLBACK (widget_cmp_text_button_remove_line_clicked),
					  page);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_cmp_text_init (WidgetCmpText *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));
}

static void widget_cmp_text_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_cmp_text_parent_class)->dispose (object);
}

static void widget_cmp_text_class_init (WidgetCmpTextClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_cmp_text_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_cmp_text.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, vbox_cmp_text);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, hbox_field_type_text);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, button_field);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, button_link);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, hbox_use_text);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, button_operator);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, entry_text);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, vbox_use_number);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, hbox_first_part);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetCmpText,
												  button_first_comparison);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetCmpText,
												  button_link_first_to_second_part);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, entry_first_amount);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, hbox_second_part);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetCmpText,
												  button_second_comparison);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, entry_second_amount);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, hbox_buttons_line);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, button_add_line);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, button_remove_line);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpText, separator_line);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
WidgetCmpText *widget_cmp_text_new (GtkWidget *page,
									gint text_cmp_number,
									gboolean first_cmp_line)
{
	WidgetCmpText *widget;

	widget = g_object_new (WIDGET_CMP_TEXT_TYPE, NULL);
	widget_cmp_text_setup_widget (widget, page, text_cmp_number, first_cmp_line);

	return widget;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean widget_cmp_text_get_data (GtkWidget *widget,
								   gint text_cmp_number)
{
	const gchar *tmp_str;
	WidgetCmpTextPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_text_get_instance_private (WIDGET_CMP_TEXT (widget));

	/* on regarde si button_link est utilisé */
	if (!priv->hide_button_link)
	{
		gsb_data_report_text_comparison_set_link_to_last_text_comparison (text_cmp_number,
																		  gtk_combo_box_get_active
																		  (GTK_COMBO_BOX (priv->button_link)));
	}
	else
		gsb_data_report_text_comparison_set_link_to_last_text_comparison (text_cmp_number, -1);

	/* on récupère l'index de button_field */
	gsb_data_report_text_comparison_set_field (text_cmp_number,
											   gtk_combo_box_get_active
											   (GTK_COMBO_BOX (priv->button_field)));

	/* on sépare le traitement en fonction field_type_text */
	if (priv->field_type_text < 8)
	{
		/* on récupère l'index de button_operator */
		gsb_data_report_text_comparison_set_operator (text_cmp_number,
													  gtk_combo_box_get_active
													  (GTK_COMBO_BOX (priv->button_operator)));

		/* on récupère le texte si il existe */
		tmp_str = gtk_entry_get_text (GTK_ENTRY (priv->entry_text));
		gsb_data_report_text_comparison_set_text (text_cmp_number, tmp_str);

		/* pour compatibilité avec ancienne version */
		gsb_data_report_text_comparison_set_use_text (text_cmp_number, TRUE);
	}
	else
	{
		gint index;		/* sert à savoir si on a une ou deux lignes numériques */

		index = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->button_link_first_to_second_part));
		gsb_data_report_text_comparison_set_link_first_to_second_part (text_cmp_number, index);

		gsb_data_report_text_comparison_set_first_comparison (text_cmp_number,
															  gtk_combo_box_get_active
															  (GTK_COMBO_BOX (priv->button_first_comparison)));

		tmp_str = gtk_entry_get_text (GTK_ENTRY (priv->entry_first_amount));
		gsb_data_report_text_comparison_set_first_amount (text_cmp_number, utils_str_atoi (tmp_str));

		if (index < 3)	/* on traite deux lignes */
		{
			gsb_data_report_text_comparison_set_second_comparison (text_cmp_number,
																   gtk_combo_box_get_active
																   (GTK_COMBO_BOX (priv->button_second_comparison)));

			tmp_str = gtk_entry_get_text (GTK_ENTRY (priv->entry_second_amount));
			gsb_data_report_text_comparison_set_second_amount (text_cmp_number, utils_str_atoi (tmp_str));
		}

		/* pour compatibilité avec ancienne version */
		gsb_data_report_text_comparison_set_use_text (text_cmp_number, FALSE);
	}

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
void widget_cmp_text_init_data (GtkWidget *widget,
								gint text_cmp_number)
{
	gchar *tmp_str;
	gint field_type_text;
	WidgetCmpTextPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_text_get_instance_private (WIDGET_CMP_TEXT (widget));

	/* set button_field */
	field_type_text = gsb_data_report_text_comparison_get_field (text_cmp_number);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_field), field_type_text);

	/* on sépare le traitement en fonction field_type_text */
	if (field_type_text < 8)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_operator),
								  gsb_data_report_text_comparison_get_operator (text_cmp_number));

		tmp_str = gsb_data_report_text_comparison_get_text (text_cmp_number);
		if (tmp_str && strlen (tmp_str))
			gtk_entry_set_text (GTK_ENTRY (priv->entry_text), tmp_str);
	}
	else
	{
		gint index;		/* sert à savoir si on a une ou deux lignes numériques */

		index = gsb_data_report_text_comparison_get_link_first_to_second_part (text_cmp_number);

		/* set buttons */
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_first_comparison),
												 gsb_data_report_text_comparison_get_first_comparison
												 (text_cmp_number));

		/* set first entry */
		tmp_str = utils_str_itoa (gsb_data_report_text_comparison_get_first_amount (text_cmp_number));
		gtk_entry_set_text (GTK_ENTRY (priv->entry_first_amount), tmp_str);
		g_free (tmp_str);

		if (index < 3)	/* on affiche deux lignes */
		{
			gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_link_first_to_second_part),
									  gsb_data_report_text_comparison_get_link_first_to_second_part
									  (text_cmp_number));

			gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_second_comparison),
									  gsb_data_report_text_comparison_get_second_comparison
									  (text_cmp_number));

			tmp_str = utils_str_itoa (gsb_data_report_text_comparison_get_second_amount (text_cmp_number));
			gtk_entry_set_text (GTK_ENTRY (priv->entry_second_amount), tmp_str);
			g_free (tmp_str);
		}
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
void widget_cmp_text_show_button_add (GtkWidget *widget,
									  gboolean show)
{
	WidgetCmpTextPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_text_get_instance_private (WIDGET_CMP_TEXT (widget));

	if (show)
		gtk_widget_show (priv->button_add_line);
	else
		gtk_widget_hide (priv->button_add_line);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void widget_cmp_text_show_button_remove (GtkWidget *widget,
										 gboolean show)
{
	WidgetCmpTextPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_text_get_instance_private (WIDGET_CMP_TEXT (widget));

	if (show)
		gtk_widget_show (priv->button_remove_line);
	else
		gtk_widget_hide (priv->button_remove_line);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void widget_cmp_text_hide_button_link (GtkWidget *widget)
{
	WidgetCmpTextPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_text_get_instance_private (WIDGET_CMP_TEXT (widget));

	gtk_widget_hide (priv->button_link);
	priv->hide_button_link = TRUE;

	if (gtk_widget_is_visible (priv->separator_line))
		gtk_widget_hide (priv->separator_line);
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

