/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "prefs_page_divers.h"
#include "gsb_automem.h"
#include "parametres.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDiversPrivate   PrefsPageDiversPrivate;

struct _PrefsPageDiversPrivate
{
	GtkWidget *			vbox_divers;

    GtkWidget *			grid_divers_programs;

	GtkWidget *         box_divers_scheduler;
    GtkWidget *         spinbutton_divers_nb_days_before_scheduled;

	GtkWidget *			box_divers_localisation;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDivers, prefs_page_divers, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des divers
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_divers_setup_divers_page (PrefsPageDivers *page)
{
	GtkWidget *head_page;
	GtkWidget *entry_divers_programs;
    GtkWidget *button;
	PrefsPageDiversPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_divers_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Divers"), "generalities.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_divers), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_divers), head_page, 0);

    /* set the variables for programs */
	entry_divers_programs = gsb_automem_entry_new ( &conf.browser_command, NULL, NULL);
	gtk_grid_attach (GTK_GRID (priv->grid_divers_programs), entry_divers_programs, 1, 0, 1, 1);

	/* set the scheduled variables */
	button = utils_prefs_automem_radiobutton_blue_new (_("Warn/Execute the scheduled transactions arriving at expiration date"),
													   _("Warn/Execute the scheduled transactions of the month"),
													   &conf.execute_scheduled_of_month,
													   NULL,
													   NULL);
	gtk_box_pack_start (GTK_BOX (priv->box_divers_scheduler), button, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->box_divers_scheduler), button, 0);

	/* set spinbutton value */
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_divers_nb_days_before_scheduled),
							   conf.nb_days_before_scheduled);

    /* callback for spinbutton_ */
    g_signal_connect (priv->spinbutton_divers_nb_days_before_scheduled,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &conf.nb_days_before_scheduled);

	/* set the localization parameters */
	gsb_config_date_format_chosen (priv->box_divers_localisation, GTK_ORIENTATION_HORIZONTAL);
    gsb_config_number_format_chosen (priv->box_divers_localisation, GTK_ORIENTATION_VERTICAL);

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_divers_init (PrefsPageDivers *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_divers_setup_divers_page (page);
}

static void prefs_page_divers_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_divers_parent_class)->dispose (object);
}

static void prefs_page_divers_class_init (PrefsPageDiversClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_divers_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_divers.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, vbox_divers);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, grid_divers_programs);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, box_divers_scheduler);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, spinbutton_divers_nb_days_before_scheduled);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDivers, box_divers_localisation);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageDivers *prefs_page_divers_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_DIVERS_TYPE, NULL);
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

