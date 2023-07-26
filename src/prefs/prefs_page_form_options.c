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
#include "prefs_page_form_options.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageFormOptionsPrivate   PrefsPageFormOptionsPrivate;

struct _PrefsPageFormOptionsPrivate
{
	GtkWidget *			vbox_form_options;

	GtkWidget *			box_affichage_exercice_automatique;
	GtkWidget *			box_fyear_combobox_sort_order;
    GtkWidget *         box_form_date_force_prev_year;
	GtkWidget *			box_form_enter_key;
	GtkWidget *			box_form_validate_split;
    GtkWidget *			checkbutton_automatic_separator;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageFormOptions, prefs_page_form_options, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des form_options
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_form_options_setup_form_options_page (PrefsPageFormOptions *page)
{
	GtkWidget *head_page;
	GtkWidget *button;
	gboolean is_loading;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;
	PrefsPageFormOptionsPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_form_options_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Form behavior"), "gsb-form-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_form_options), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_form_options), head_page, 0);

	/* set the form_enter_key variable */
	button = gsb_automem_radiobutton_gsettings_new (_("selects next field"),
													_("terminates transaction"),
													&a_conf->form_enter_key,
													NULL,
													NULL);
	gtk_box_pack_start (GTK_BOX (priv->box_form_enter_key), button, FALSE, FALSE, 0);

	/* set the form_date_force_prev_year variable */
	button = gsb_automem_checkbutton_new (_("Replace the year of future dates with the previous year"),
										  &w_etat->form_date_force_prev_year,
										  NULL,
										  NULL);
	gtk_box_pack_start (GTK_BOX (priv->box_form_date_force_prev_year), button, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->box_form_date_force_prev_year), button, 0);

    /* set the affichage_exercice_automatique variable */
    button = gsb_automem_radiobutton_gsettings_new (_("according to transaction date"),
													_("according to transaction value date"),
												    &a_conf->affichage_exercice_automatique,
												    NULL,
												    NULL);
	gtk_box_pack_start (GTK_BOX (priv->box_affichage_exercice_automatique), button, FALSE, FALSE, 0);

    /* set the fyear_combobox_sort_order variable */
	button = gsb_automem_checkbutton_gsettings_new (_("Sorting descending of the exercises"),
													&a_conf->fyear_combobox_sort_order,
													NULL,
													a_conf);
	gtk_box_pack_start (GTK_BOX (priv->box_fyear_combobox_sort_order), button, FALSE, FALSE, 0);

	/* set automatic_separator variable */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_automatic_separator),
								  w_etat->automatic_separator);

	/* set Selected transaction after validate split transaction */
	button = gsb_automem_radiobutton_gsettings_new (_("selects new transaction"),
													_("selects mother transaction"),
													&a_conf->form_validate_split,
													NULL,
													NULL);
	gtk_box_pack_start (GTK_BOX (priv->box_form_validate_split), button, FALSE, FALSE, 0);

	if (!is_loading)
	{
		gtk_widget_set_sensitive (priv->box_form_date_force_prev_year, FALSE);
		gtk_widget_set_sensitive (priv->checkbutton_automatic_separator, FALSE);
	}

    /* Connect signal */
    g_signal_connect (priv->checkbutton_automatic_separator,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->automatic_separator);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_form_options_init (PrefsPageFormOptions *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_form_options_setup_form_options_page (page);
}

static void prefs_page_form_options_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_form_options_parent_class)->dispose (object);
}

static void prefs_page_form_options_class_init (PrefsPageFormOptionsClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_form_options_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_form_options.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFormOptions, vbox_form_options);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageFormOptions,
												  box_affichage_exercice_automatique);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageFormOptions,
												  box_fyear_combobox_sort_order);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageFormOptions,
												  box_form_date_force_prev_year);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageFormOptions,
												  box_form_enter_key);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFormOptions, box_form_enter_key);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageFormOptions,
												  box_form_validate_split);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageFormOptions,
												  checkbutton_automatic_separator);
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
PrefsPageFormOptions *prefs_page_form_options_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_FORM_OPTIONS_TYPE, NULL);
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

