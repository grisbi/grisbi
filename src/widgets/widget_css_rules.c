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
#include "widget_css_rules.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_combo_box.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_rgba.h"
#include "gsb_scheduler_list.h"
#include "structures.h"
#include "transaction_list.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetCssRulesPrivate   WidgetCssRulesPrivate;

struct _WidgetCssRulesPrivate
{
	GtkWidget *			vbox_css_rules;
	GtkWidget *         notebook_css_rules;

	GtkWidget *			button_back_home_rules;
	GtkWidget *			button_home_rules;
	GtkWidget *			combo_home_rules;
	GtkWidget *			button_back_height_statusbar;
	GtkWidget *			spinbutton_height_statusbar;

	GtkWidget *			button_back_transactions_rules;
	GtkWidget *			button_transactions_rules;
	GtkWidget *			combo_transactions_rules;

	GtkWidget *			button_back_bet_rules;
	GtkWidget *			button_bet_rules;
	GtkWidget *			combo_bet_rules;

	GtkWidget *			button_back_prefs_rules;
	GtkWidget *			button_prefs_rules;
	GtkWidget *			combo_prefs_rules;

	GtkWidget *			button_back_global_font;
	GtkWidget *			combo_global_font;
	GtkWidget *			label_global_font;

	GtkWidget *			button_default_all_rules;
};

/* éléments pour la gestion de la dimension de la police de caractères */
static const gchar *	AbsoluteSize[] = {"xx-small", "x-small", "small",
										  "medium",
										  "large", "x-large", "xx-large",
										  NULL};

/* mémorise la hauteur initiale de la police de caractère */
static gint				old_global_font_size = 0;
static gboolean			global_font_size_changed = FALSE;

/* mémorise la hauteur du texte de la barre de statut */
static gint				old_statusbar_font_size = 0;

G_DEFINE_TYPE_WITH_PRIVATE (WidgetCssRules, widget_css_rules, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * called when back button of global font is clicked,
 *
 * \param back_button
 * \param combobox
 *
 * \return
 **/
static void widget_css_rules_button_back_global_font_clicked (GtkWidget *back_button,
															  GtkWidget *combo)
{
	gsb_combo_box_set_index (combo, old_global_font_size);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_css_rules_combobox_global_font_changed (GtkWidget *combo,
														   GtkWidget *back_button)
{
	GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
	{
		GtkTreeModel *model;
		gchar *value;
		gint index;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 0, &value, 1, &index, -1);
		gsb_css_set_property_from_selector ("*", "font-size", value);

		/* on gère le changement de réglage */
		if (index == old_global_font_size)
		{
			gtk_widget_set_sensitive (back_button, FALSE);
			global_font_size_changed = FALSE;
			gsb_css_count_change_dec (TRUE);
		}
		else
		{
			gtk_widget_set_sensitive (back_button, TRUE);
			global_font_size_changed = TRUE;
		}

		g_free (value);
	}
	else
		gtk_widget_set_sensitive (back_button, FALSE);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean widget_css_rules_init_combo_font_size_index (GtkWidget *combo)
{
	const gchar *css_data;
	gchar *tmp_str = NULL;

	css_data = grisbi_app_get_css_data ();
	tmp_str = g_strstr_len (css_data, -1, "*");
	if (tmp_str)
	{
		gchar *start_str;
		gchar *end_str;
		gchar *rule;

		start_str = g_strstr_len (tmp_str, -1, "{");
		end_str = g_strstr_len (tmp_str, -1, "}");
		rule = g_strndup (start_str+1, (end_str-start_str-1));
		tmp_str = g_strrstr (rule, "font-size");
		if (tmp_str)
		{
			gchar **tab1;
			gchar **tab2;
			gchar *font_size_str;
			gint i;

			tab1 = g_strsplit (tmp_str, ":", 2);
			tab2 = g_strsplit (tab1[1], ";", 2);
			font_size_str = g_strdup (g_strstrip (tab2[0]));
			g_strfreev (tab1);
			g_strfreev (tab2);

			for (i = 0; AbsoluteSize[i]; i++)
			{
				if (strcmp (font_size_str, AbsoluteSize[i]) == 0)
				{
					gsb_combo_box_set_index (combo, i);
					if (old_global_font_size == 0)
						old_global_font_size = i;

					g_free (font_size_str);
					g_free (rule);

					return TRUE;
				}
			}
			g_free (rule);
		}
		else
			g_free (rule);
	}
	gsb_combo_box_set_index (combo, 3);
	if (old_global_font_size == 0)
		old_global_font_size = 3;

	return FALSE;
}

/**
 * called when a back button of colors is clicked,
 *
 * \param back_button
 * \param combobox
 *
 * \return
 **/
static void widget_css_rules_button_back_rules_clicked (GtkWidget *back_button,
														GtkWidget *combobox)
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
		GtkWidget *color_button;
		GtkTreeModel *model;
		GdkRGBA *old_color;
		gchar *couleur;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &couleur, 3, &old_color, -1);
		color_button = g_object_get_data (G_OBJECT (back_button), "button_color_rules");
		if (old_color)
		{
			gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (color_button), old_color);
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, 3, 0, -1);

		}
    }
}

static void widget_css_rules_button_default_all_rules_clicked  (GtkWidget *button,
																gpointer null)
{
	gchar *css_filename = NULL;
	gchar *tmp_str = NULL;
	gchar *text;
	gboolean ret;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	css_filename = gsb_css_get_filename ();
	text = g_strdup_printf (_("Warning: you will delete your grisbi color adjustment file.\n"
							  "This operation will be final.\n"
							  "Do you want to delete the file: \"%s\"?"),
							css_filename);

	ret = dialogue_yes_no (text, _("Delete CSS user file"), GTK_RESPONSE_NO);
	g_free (text);

	if (!ret)
		return;

	if (a_conf->use_type_theme == 2)
		tmp_str = g_strdup ("grisbi-dark.css");
	else
		tmp_str = g_strdup ("grisbi.css");

	css_filename = g_build_filename (gsb_dirs_get_user_config_dir (), tmp_str, NULL);
	if (css_filename)
		g_remove (css_filename);

	/* set old css rules */
	gsb_css_load_css_data_from_file (NULL);
	gsb_css_count_change_init ();
}

/**
 * called when a color is chosen in the GtkColorButton,
 * update the color selected
 *
 * \param color_button
 * \param combobox
 *
 * \return FALSE
 **/
static void widget_css_rules_button_select_colors_changed (GtkWidget *color_button,
														   GtkWidget *combobox)
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
		GtkWidget *back_button;
		GtkTreeModel *model;
		GdkRGBA *color;
		gchar *couleur;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &couleur, 3, &color, -1);
		back_button = g_object_get_data (G_OBJECT (color_button), "button_back_rules");
		if (color)
		{
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (color_button), color);
			gtk_widget_set_sensitive (back_button, TRUE);

			if (grisbi_win_file_is_loading ())
			{
				/* update the colors in the transactions list */
				transaction_list_redraw ();

				/* update scheduled list */
				gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
				gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());
				gsb_scheduler_list_select (-1);
			}
			gsb_css_set_color_property (color, couleur);
		}
		else
			gtk_widget_set_sensitive (back_button, FALSE);
    }
}

/**
 * called when a color button is clicked,
 *
 * \param color_button
 * \param combobox
 *
 * \return FALSE
 **/
static void widget_css_rules_button_select_colors_clicked (GtkWidget *color_button,
														   GtkWidget *combobox)
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
		GtkTreeModel *model;
		GdkRGBA *color;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 2, &color, -1);
		if (color)
		{
			/* save color for back function */
			gtk_list_store_set (GTK_LIST_STORE (model), &iter, 3, color, -1);
		}
    }
}

/**
 * called when the color combobox changed,
 * update the GtkColorButton with the color of the combobox
 *
 * \param combobox
 * \param color_button
 *
 * \return FALSE
 **/
static gboolean widget_css_rules_combobox_select_colors_changed (GtkWidget *combobox,
															 GtkWidget *color_button)
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
		GtkTreeModel *model;
		GdkRGBA *color;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 2, &color, -1);
		if (color)
			gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (color_button), color);
    }
    return FALSE;
}

/**
 * called when the spin back button is clicked,
 *
 * \param back_button
 * \param spinbutton
 *
 * \return
 **/
static void widget_css_rules_spinbutton_back_clicked (GtkWidget *back_button,
													  GtkWidget *spin_button)
{
	gtk_widget_set_sensitive (back_button, FALSE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), (gdouble) old_statusbar_font_size);
	grisbi_win_status_bar_set_font_size (old_statusbar_font_size);
}

/**
 * called when the spinbutton value changed,
 *
 * \param spinbutton
 * \param back_button
 *
 * \return
 **/
static void widget_css_rules_spinbutton_value_changed (GtkSpinButton *spin_button,
													   GtkWidget *back_button)
{
	gint new_font_size;

	new_font_size = gtk_spin_button_get_value_as_int (spin_button);
	if (old_statusbar_font_size == new_font_size)
	{
		return;
	}

	gtk_widget_set_sensitive (back_button, TRUE);
	grisbi_win_status_bar_set_font_size (new_font_size);
}

/**
 * Création de la page de gestion des couleurs de grisbi
 *
 * \param
 *
 * \return
 **/
static void widget_css_rules_setup_page (WidgetCssRules *page)
{
	gchar *font_size_str;
	gint font_size;
	GrisbiAppConf *a_conf;
	WidgetCssRulesPrivate *priv;

	devel_debug (NULL);
	priv = widget_css_rules_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	/* set home rules */
	gsb_rgba_create_color_combobox_from_ui (priv->combo_home_rules, CSS_HOME_RULES);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_home_rules), 0);
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->button_home_rules),
								gsb_rgba_get_couleur ("background_ligne_compte"));
	g_object_set_data (G_OBJECT (priv->button_home_rules), "button_back_rules", priv->button_back_home_rules);
	g_object_set_data (G_OBJECT (priv->button_back_home_rules), "button_color_rules", priv->button_home_rules);

	/* set transactions rules */
	gsb_rgba_create_color_combobox_from_ui (priv->combo_transactions_rules, CSS_TRANSACTIONS_RULES);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_transactions_rules), 0);
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->button_transactions_rules),
								gsb_rgba_get_couleur ("couleur_fond_0"));
	g_object_set_data (G_OBJECT (priv->button_transactions_rules),
					   "button_back_rules",
					   priv->button_back_transactions_rules);
	g_object_set_data (G_OBJECT (priv->button_back_transactions_rules),
					   "button_color_rules",
					   priv->button_transactions_rules);

	/* set bet rules */
	gsb_rgba_create_color_combobox_from_ui (priv->combo_bet_rules, CSS_BET_RULES);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_bet_rules), 0);
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->button_bet_rules),
								gsb_rgba_get_couleur ("background_bet_division"));
	g_object_set_data (G_OBJECT (priv->button_bet_rules), "button_back_rules", priv->button_back_bet_rules);
	g_object_set_data (G_OBJECT (priv->button_back_bet_rules), "button_color_rules", priv->button_bet_rules);

	/* set prefs rules */
	gsb_rgba_create_color_combobox_from_ui (priv->combo_prefs_rules, CSS_PREFS_RULES);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_prefs_rules), 0);
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->button_prefs_rules),
								gsb_rgba_get_couleur ("text_gsetting_option_normal"));
	g_object_set_data (G_OBJECT (priv->button_prefs_rules), "button_back_rules", priv->button_back_prefs_rules);
	g_object_set_data (G_OBJECT (priv->button_back_prefs_rules), "button_color_rules", priv->button_prefs_rules);

	/* set font size of statusbar */
	font_size_str = gsb_css_get_property_from_name ("#global_statusbar","font-size");
	font_size = utils_str_atoi (font_size_str);
	g_free (font_size_str);

	/* on mémorise la valeur initiale de font_size pendant la session */
	if (old_statusbar_font_size == 0)
		old_statusbar_font_size = font_size;
	else
		gtk_widget_set_sensitive (priv->button_back_height_statusbar, TRUE);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_height_statusbar), (gdouble) font_size);

	/* set global font */
	gsb_combo_box_new_with_index_from_ui (priv->combo_global_font,
										  AbsoluteSize,
										  NULL,
										  NULL);
	if (a_conf->use_css_local_file || global_font_size_changed)
	{
		if (widget_css_rules_init_combo_font_size_index (priv->combo_global_font))
		{
			/* on désactive les boutons pour la statusbar */
			gtk_widget_set_sensitive (priv->spinbutton_height_statusbar, FALSE);
			gtk_widget_set_sensitive (priv->button_back_height_statusbar, FALSE);

			/* on active éventuellement le bouton retour pour global font */
			if (global_font_size_changed)
				gtk_widget_set_sensitive (priv->button_back_global_font, TRUE);
			else
				gtk_widget_set_sensitive (priv->button_back_global_font, FALSE);
		}
		else
			gtk_widget_set_sensitive (priv->button_back_global_font, FALSE);

		/* set button remove all user rules */
		gtk_widget_set_sensitive (priv->button_default_all_rules, TRUE);
	}
	else
	{
		gsb_combo_box_set_index (priv->combo_global_font, 3);
		old_global_font_size = 3;
		gtk_widget_set_sensitive (priv->button_back_global_font, FALSE);
	}

	/* Connect signal home rules*/
	g_signal_connect (G_OBJECT (priv->combo_home_rules),
					  "changed",
					  G_CALLBACK (widget_css_rules_combobox_select_colors_changed),
					  priv->button_home_rules);
	g_signal_connect (G_OBJECT (priv->button_home_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_select_colors_clicked),
					  priv->combo_home_rules);
	g_signal_connect_after (G_OBJECT (priv->button_home_rules),
					  "color-set",
					  G_CALLBACK (widget_css_rules_button_select_colors_changed),
					  priv->combo_home_rules);
	g_signal_connect (G_OBJECT (priv->button_back_home_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_back_rules_clicked),
					  priv->combo_home_rules);
	g_signal_connect (G_OBJECT (priv->spinbutton_height_statusbar),
					  "value-changed",
					  G_CALLBACK (widget_css_rules_spinbutton_value_changed),
					  priv->button_back_height_statusbar);
	g_signal_connect (G_OBJECT (priv->button_back_height_statusbar),
					  "clicked",
					  G_CALLBACK (widget_css_rules_spinbutton_back_clicked),
					  priv->spinbutton_height_statusbar);

	/* Connect signal transactions rules*/
	g_signal_connect (G_OBJECT (priv->combo_transactions_rules),
					  "changed",
					  G_CALLBACK (widget_css_rules_combobox_select_colors_changed),
					  priv->button_transactions_rules);
	g_signal_connect (G_OBJECT (priv->button_transactions_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_select_colors_clicked),
					  priv->combo_transactions_rules);
	g_signal_connect_after (G_OBJECT (priv->button_transactions_rules),
					  "color-set",
					  G_CALLBACK (widget_css_rules_button_select_colors_changed),
					  priv->combo_transactions_rules);
	g_signal_connect (G_OBJECT (priv->button_back_transactions_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_spinbutton_back_clicked),
					  priv->combo_transactions_rules);

	/* Connect signal bet rules*/
	g_signal_connect (G_OBJECT (priv->combo_bet_rules),
					  "changed",
					  G_CALLBACK (widget_css_rules_combobox_select_colors_changed),
					  priv->button_bet_rules);
	g_signal_connect (G_OBJECT (priv->button_bet_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_select_colors_clicked),
					  priv->combo_bet_rules);
	g_signal_connect_after (G_OBJECT (priv->button_bet_rules),
					  "color-set",
					  G_CALLBACK (widget_css_rules_button_select_colors_changed),
					  priv->combo_bet_rules);
	g_signal_connect (G_OBJECT (priv->button_back_bet_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_back_rules_clicked),
					  priv->combo_bet_rules);

	/* Connect signal prefs rules*/
	g_signal_connect (G_OBJECT (priv->combo_prefs_rules),
					  "changed",
					  G_CALLBACK (widget_css_rules_combobox_select_colors_changed),
					  priv->button_prefs_rules);
	g_signal_connect (G_OBJECT (priv->button_prefs_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_select_colors_clicked),
					  priv->combo_prefs_rules);
	g_signal_connect_after (G_OBJECT (priv->button_prefs_rules),
					  "color-set",
					  G_CALLBACK (widget_css_rules_button_select_colors_changed),
					  priv->combo_prefs_rules);
	g_signal_connect (G_OBJECT (priv->button_back_prefs_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_back_rules_clicked),
					  priv->combo_prefs_rules);

	/* set signal global font */
	g_signal_connect (G_OBJECT (priv->combo_global_font),
					  "changed",
					  G_CALLBACK (widget_css_rules_combobox_global_font_changed),
					  priv->button_back_global_font);

	g_signal_connect (G_OBJECT (priv->button_back_global_font),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_back_global_font_clicked),
					  priv->combo_global_font);

	/* remove css user file */
	g_signal_connect (G_OBJECT (priv->button_default_all_rules),
					  "clicked",
					  G_CALLBACK (widget_css_rules_button_default_all_rules_clicked),
					  NULL);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_css_rules_init (WidgetCssRules *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	widget_css_rules_setup_page (page);
}

static void widget_css_rules_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_css_rules_parent_class)->dispose (object);
}

static void widget_css_rules_class_init (WidgetCssRulesClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_css_rules_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_css_rules.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, vbox_css_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, notebook_css_rules);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_back_home_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_home_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, combo_home_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_back_height_statusbar);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, spinbutton_height_statusbar);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_back_transactions_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_transactions_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, combo_transactions_rules);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_back_bet_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_bet_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, combo_bet_rules);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_back_prefs_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_prefs_rules);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, combo_prefs_rules);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_back_global_font);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, combo_global_font);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, label_global_font);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCssRules, button_default_all_rules);
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
WidgetCssRules *widget_css_rules_new (GtkWidget *page)
{
	WidgetCssRules *w_color;

	w_color = g_object_new (WIDGET_CSS_RULES_TYPE, NULL);

	return w_color;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *widget_css_rules_get_notebook (GtkWidget *w_css_rules)
{
	WidgetCssRulesPrivate *priv;

	priv = widget_css_rules_get_instance_private (WIDGET_CSS_RULES (w_css_rules));

	return priv->notebook_css_rules;
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

