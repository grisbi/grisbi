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

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_cmp_amount.h"
#include "etats_page_amount.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_real.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetCmpAmountPrivate   WidgetCmpAmountPrivate;

struct _WidgetCmpAmountPrivate
{
	GtkWidget *			vbox_cmp_amount;

	GtkWidget *			button_link;
	GtkWidget *			button_first_comparison;
	GtkWidget *			entry_first_amount;
	GtkWidget *			button_link_first_to_second_part;

	GtkWidget *			hbox_second_part;
	GtkWidget *			hbox_operators;
	GtkWidget *			button_second_comparison;
	GtkWidget *			entry_second_amount;

	GtkWidget *			button_add_line;
	GtkWidget *			button_remove_line;
	GtkWidget *			separator_line;

	/* memorise les donnes de base */
	gboolean			hide_button_link;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetCmpAmount, widget_cmp_amount, GTK_TYPE_BOX)
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
static void widget_cmp_amount_button_add_line_clicked (GtkButton *button,
													   GtkWidget *page)
{
	gint amount_cmp_number;

	amount_cmp_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "amount_cmp_number"));
	etats_page_amount_line_add (amount_cmp_number, page);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_cmp_amount_button_remove_line_clicked (GtkButton *button,
														  GtkWidget *page)
{
	gint amount_cmp_number;

	amount_cmp_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "amount_cmp_number"));
	etats_page_amount_line_remove (amount_cmp_number, page);
}

/**
 * Rend sensitif le choix de la suite de la comparaison de numéro
 *
 * \param combo_box
 * \param priv
 *
 * \return
 **/
static void widget_cmp_amount_button_first_comparison_changed (GtkComboBox *combo,
															   WidgetCmpAmountPrivate *priv)
{
	gint index;

	index = gtk_combo_box_get_active (combo);
	if (index == 0)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_link_first_to_second_part), 3);
		gtk_widget_set_sensitive (priv->button_link_first_to_second_part, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->button_link_first_to_second_part, TRUE);
		if (index > 5)
		{
			gtk_widget_set_sensitive (priv->entry_first_amount, FALSE);
		}
		else
		{
			gtk_widget_set_sensitive (priv->entry_first_amount, TRUE);
		}
	}
}

/**
 * Rend sensitif le choix de la suite de la comparaison de numéro
 *
 * \param combo_box
 * \param priv
 *
 * \return
 **/
static void widget_cmp_amount_button_second_comparison_changed (GtkComboBox *combo,
																WidgetCmpAmountPrivate *priv)
{
	gint index;

	index = gtk_combo_box_get_active (combo);
	if (index > 5)
	{
		gtk_widget_set_sensitive (priv->entry_second_amount, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->entry_second_amount, TRUE);
	}
}

/**
 * Rend sensitif la suite de la comparaison de numéro
 *
 * \param combo_box
 * \param widget		widget à rendre sensitif
 *
 * \return
 **/
static void widget_cmp_amount_button_link_first_to_second_part_changed (GtkComboBox *combo,
																		GtkWidget *widget)
{
	gint index;

	index = gtk_combo_box_get_active (combo);
	if (index == 3)
		gtk_widget_set_sensitive (widget, FALSE);
	else
		gtk_widget_set_sensitive (widget, TRUE);
}

/**
 * Création du widget cmp_amount
 *
 * \param
 * \return
 **/
static void widget_cmp_amount_setup_widget (WidgetCmpAmount *widget,
											GtkWidget *page,
											gint amount_cmp_number,
											gboolean first_cmp_line)
{
	WidgetCmpAmountPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_amount_get_instance_private (widget);

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

	/* on attache aux boutons add et remove le numero de comparaison */
	g_object_set_data (G_OBJECT (priv->button_add_line),
					   "amount_cmp_number",
					   GINT_TO_POINTER (amount_cmp_number));
	g_object_set_data (G_OBJECT (priv->button_remove_line),
					   "amount_cmp_number",
					   GINT_TO_POINTER (amount_cmp_number));

	/* set signals */
	/* action a faire lorsque l'on change le choix du combobox button_link_first_to_second_part */
	g_signal_connect (G_OBJECT (priv->button_link_first_to_second_part),
					  "changed",
					  G_CALLBACK (widget_cmp_amount_button_link_first_to_second_part_changed),
					  priv->hbox_operators);

	/* action a faire lorsque l'on change le choix du combobox button_first_comparison */
	g_signal_connect (G_OBJECT (priv->button_first_comparison),
					  "changed",
					  G_CALLBACK (widget_cmp_amount_button_first_comparison_changed),
					  priv);

	/* action a faire lorsque l'on change le choix du combobox button_second_comparison */
	g_signal_connect (G_OBJECT (priv->button_second_comparison),
					  "changed",
					  G_CALLBACK (widget_cmp_amount_button_second_comparison_changed),
					  priv);

	/* signal pour le bouton button_add_line */
	g_signal_connect (G_OBJECT (priv->button_add_line),
					  "clicked",
					  G_CALLBACK (widget_cmp_amount_button_add_line_clicked),
					  page);

	/* signal pour le bouton button_remove_line */
	g_signal_connect (G_OBJECT (priv->button_remove_line),
					  "clicked",
					  G_CALLBACK (widget_cmp_amount_button_remove_line_clicked),
					  page);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_cmp_amount_init (WidgetCmpAmount *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));
}

static void widget_cmp_amount_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_cmp_amount_parent_class)->dispose (object);
}

static void widget_cmp_amount_class_init (WidgetCmpAmountClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_cmp_amount_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_cmp_amount.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, vbox_cmp_amount);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, button_link);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetCmpAmount,
												  button_first_comparison);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, entry_first_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetCmpAmount,
												  button_link_first_to_second_part);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, hbox_second_part);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, hbox_operators);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetCmpAmount, button_second_comparison);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  WidgetCmpAmount, entry_second_amount);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, button_add_line);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, button_remove_line);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCmpAmount, separator_line);
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
WidgetCmpAmount *widget_cmp_amount_new (GtkWidget *page,
										gint amount_cmp_number,
										gboolean first_cmp_line)
{
	WidgetCmpAmount *widget;

	widget = g_object_new (WIDGET_CMP_AMOUNT_TYPE, NULL);
	widget_cmp_amount_setup_widget (widget, page, amount_cmp_number, first_cmp_line);

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
void widget_cmp_amount_init_data (GtkWidget *widget,
								  gint amount_cmp_number)
{
	gchar *tmp_str;
	gint index;						/* sert à sensibiliser hbox_operators si deux lignes actives */
	gint comparison_number;			/* sert a insensibiliser les entrées si > 5 */
	WidgetCmpAmountPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_amount_get_instance_private (WIDGET_CMP_AMOUNT (widget));

	index = gsb_data_report_amount_comparison_get_link_first_to_second_part (amount_cmp_number);

	/* set buttons */
	comparison_number = gsb_data_report_amount_comparison_get_first_comparison (amount_cmp_number);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_first_comparison), comparison_number);

	/* set first entry */
	tmp_str = utils_real_get_string (gsb_data_report_amount_comparison_get_first_amount (amount_cmp_number));
	gtk_entry_set_text (GTK_ENTRY (priv->entry_first_amount), tmp_str);
	g_free (tmp_str);

	if (index < 3)	/* on traite deux lignes */
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_link_first_to_second_part),
								  gsb_data_report_amount_comparison_get_link_first_to_second_part
								  (amount_cmp_number));

		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->button_second_comparison),
								  gsb_data_report_amount_comparison_get_second_comparison
								  (amount_cmp_number));

		tmp_str = utils_real_get_string (gsb_data_report_amount_comparison_get_second_amount (amount_cmp_number));
		gtk_entry_set_text (GTK_ENTRY (priv->entry_second_amount), tmp_str);
		g_free (tmp_str);
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
gboolean widget_cmp_amount_get_data (GtkWidget *widget,
									 gint amount_cmp_number)
{
	const gchar *tmp_str;
	gint index;						/* sert à savoir si on a une ou deux lignes numériques */
	gint comparison_number;
	gboolean is_empty = FALSE;
	GsbReal number;
	WidgetCmpAmountPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_amount_get_instance_private (WIDGET_CMP_AMOUNT (widget));

	/* on regarde si button_link est utilisé */
	if (!priv->hide_button_link)
	{
		gsb_data_report_amount_comparison_set_link_to_last_amount_comparison (amount_cmp_number,
																			  gtk_combo_box_get_active
																			  (GTK_COMBO_BOX
																			  (priv->button_link)));
	}
	else
		gsb_data_report_amount_comparison_set_link_to_last_amount_comparison (amount_cmp_number, -1);

	index = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->button_link_first_to_second_part));
	gsb_data_report_amount_comparison_set_link_first_to_second_part (amount_cmp_number, index);

	/* get first_comparison */
	comparison_number = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->button_first_comparison));
	gsb_data_report_amount_comparison_set_first_comparison (amount_cmp_number, comparison_number);

	/* get amount 1 if comparison_number <= 5 */
	if (comparison_number <= 5)
	{
		tmp_str = gtk_entry_get_text (GTK_ENTRY (priv->entry_first_amount));
		number = utils_real_get_from_string (tmp_str);
		gsb_data_report_amount_comparison_set_first_amount (amount_cmp_number, number);
		if (strlen (tmp_str))
		{
			number = utils_real_get_from_string (tmp_str);
			gsb_data_report_amount_comparison_set_first_amount (amount_cmp_number, number);
			is_empty = FALSE;
		}
		else if (index == 3)
		{
			return FALSE;
		}
		else
		{
			is_empty = TRUE;
		}
	}
	if (index < 3)	/* on traite la deuxieme ligne */
	{
		comparison_number = gtk_combo_box_get_active (GTK_COMBO_BOX (priv->button_second_comparison));
		gsb_data_report_amount_comparison_set_second_comparison (amount_cmp_number, comparison_number);

		/* get amount 2 if comparison_number <= 5 */
		if (comparison_number <= 5)
		{
			tmp_str = gtk_entry_get_text (GTK_ENTRY (priv->entry_second_amount));
			number = utils_real_get_from_string (tmp_str);
			gsb_data_report_amount_comparison_set_second_amount (amount_cmp_number, number);

			if (strlen (tmp_str) == 0 && is_empty == TRUE)
			{
				return FALSE;
			}
		}
}

	return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void widget_cmp_amount_show_button_add (GtkWidget *widget,
										gboolean show)
{
	WidgetCmpAmountPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_amount_get_instance_private (WIDGET_CMP_AMOUNT (widget));

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
void widget_cmp_amount_show_button_remove (GtkWidget *widget,
										   gboolean show)
{
	WidgetCmpAmountPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_amount_get_instance_private (WIDGET_CMP_AMOUNT (widget));

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
void widget_cmp_amount_hide_button_link (GtkWidget *widget)
{
	WidgetCmpAmountPrivate *priv;

	devel_debug (NULL);
	priv = widget_cmp_amount_get_instance_private (WIDGET_CMP_AMOUNT (widget));

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

