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
#include "prefs_page_import_files.h"
#include "gsb_automem.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageImportFilesPrivate   PrefsPageImportFilesPrivate;

struct _PrefsPageImportFilesPrivate
{
	GtkWidget *			vbox_import_files;

    GtkWidget *         spinbutton_import_files_nb_days;
    GtkWidget *			checkbutton_fusion_import_transactions;
	GtkWidget *			eventbox_fusion_import_transactions;
    GtkWidget *			checkbutton_associate_categorie_for_payee;
	GtkWidget *			eventbox_associate_categorie_for_payee;
    GtkWidget *			checkbutton_extract_number_for_check;
	GtkWidget *			eventbox_extract_number_for_check;
    GtkWidget *			checkbutton_copy_payee_in_note;
	GtkWidget *			eventbox_copy_payee_in_note;
    GtkWidget *			checkbutton_csv_force_date_valeur_with_date;
	GtkWidget *			eventbox_csv_force_date_valeur_with_date;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageImportFiles, prefs_page_import_files, GTK_TYPE_BOX)

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
gboolean prefs_page_import_files_spinbutton_import_files_nb_days_changed (GtkWidget *spinbutton,
																		  gpointer null)
{
    etat.import_files_nb_days = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

    gsb_file_set_modified (TRUE);

    return ( FALSE );
}

/**
 * Création de la page de gestion des import_files
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_import_files_setup_import_files_page (PrefsPageImportFiles *page)
{
	PrefsPageImportFilesPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_import_files_get_instance_private (page);

	/* set the variables for import of files*/
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_import_files_nb_days),
							   etat.import_files_nb_days);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_fusion_import_transactions),
								  etat.fusion_import_transactions);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_associate_categorie_for_payee),
								  etat.associate_categorie_for_payee);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_extract_number_for_check),
								  etat.extract_number_for_check);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_copy_payee_in_note),
								  etat.extract_number_for_check);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_csv_force_date_valeur_with_date),
								  etat.csv_force_date_valeur_with_date);

	/* set the choice of date for the financial year */
	gsb_automem_radiobutton_new_with_title (priv->vbox_import_files,
											_("Set the financial year"),
											_("According to the date"),
											_("According to the value date (if fail, try with the date)"),
											&etat.get_fyear_by_value_date,
											NULL,
											NULL);

    /* Connect signal */
    g_signal_connect ( G_OBJECT (priv->spinbutton_import_files_nb_days),
					  "value-changed",
					  G_CALLBACK (prefs_page_import_files_spinbutton_import_files_nb_days_changed),
					  NULL );

    g_signal_connect (priv->eventbox_fusion_import_transactions,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_fusion_import_transactions);
    g_signal_connect (priv->checkbutton_fusion_import_transactions,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.fusion_import_transactions);
    g_signal_connect_after (priv->checkbutton_fusion_import_transactions,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    g_signal_connect (priv->eventbox_associate_categorie_for_payee,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_associate_categorie_for_payee);
    g_signal_connect (priv->checkbutton_associate_categorie_for_payee,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.associate_categorie_for_payee);
    g_signal_connect_after (priv->checkbutton_associate_categorie_for_payee,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    g_signal_connect (priv->eventbox_extract_number_for_check,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_extract_number_for_check);
    g_signal_connect (priv->checkbutton_extract_number_for_check,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.extract_number_for_check);
    g_signal_connect_after (priv->checkbutton_extract_number_for_check,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    g_signal_connect (priv->eventbox_copy_payee_in_note,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_copy_payee_in_note);
    g_signal_connect (priv->checkbutton_copy_payee_in_note,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.copy_payee_in_note);
    g_signal_connect_after (priv->checkbutton_copy_payee_in_note,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);

    g_signal_connect (priv->eventbox_csv_force_date_valeur_with_date,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_csv_force_date_valeur_with_date);
    g_signal_connect (priv->checkbutton_csv_force_date_valeur_with_date,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &etat.csv_force_date_valeur_with_date);
    g_signal_connect_after (priv->checkbutton_csv_force_date_valeur_with_date,
							"toggled",
							G_CALLBACK (utils_prefs_gsb_file_set_modified),
							NULL);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_import_files_init (PrefsPageImportFiles *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_import_files_setup_import_files_page (page);
}

static void prefs_page_import_files_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_import_files_parent_class)->dispose (object);
}

static void prefs_page_import_files_class_init (PrefsPageImportFilesClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_import_files_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_import_files.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, vbox_import_files);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, spinbutton_import_files_nb_days);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, checkbutton_fusion_import_transactions);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, eventbox_fusion_import_transactions);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, checkbutton_associate_categorie_for_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, eventbox_associate_categorie_for_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, checkbutton_extract_number_for_check);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, eventbox_extract_number_for_check);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, checkbutton_copy_payee_in_note);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, eventbox_copy_payee_in_note);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, checkbutton_csv_force_date_valeur_with_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageImportFiles, eventbox_csv_force_date_valeur_with_date);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageImportFiles * prefs_page_import_files_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_IMPORT_FILES_TYPE, NULL);
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

