/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "prefs_page_files.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_file.h"
#include "parametres.h"
#include "structures.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageFilesPrivate   PrefsPageFilesPrivate;

struct _PrefsPageFilesPrivate
{
	GtkWidget *			vbox_files;

    GtkWidget *			checkbutton_load_last_file;
	GtkWidget *			eventbox_load_last_file;
    GtkWidget *         checkbutton_sauvegarde_auto;
	GtkWidget *			eventbox_sauvegarde_auto;
    GtkWidget *         checkbutton_force_enregistrement;
	GtkWidget *			eventbox_force_enregistrement;
    GtkWidget *         checkbutton_compress_file;
	GtkWidget *			eventbox_compress_file;
    GtkWidget *         checkbutton_crypt_file;
	GtkWidget *			eventbox_crypt_file;
    GtkWidget *         spinbutton_nb_max_derniers_fichiers;

    GtkWidget *         checkbutton_make_bakup_single_file;
	GtkWidget *			eventbox_make_bakup_single_file;
    GtkWidget *         checkbutton_compress_backup;
	GtkWidget *			eventbox_compress_backup;
    GtkWidget *         checkbutton_sauvegarde_demarrage;
	GtkWidget *			eventbox_sauvegarde_demarrage;
    GtkWidget *         checkbutton_make_backup;
	GtkWidget *			eventbox_make_backup;
    GtkWidget *         checkbutton_make_backup_every_minutes;
	GtkWidget *			eventbox_make_backup_every_minutes;
    GtkWidget *         spinbutton_make_backup_nb_minutes;
    GtkWidget *         filechooserbutton_backup;

    GtkWidget *			checkbutton_force_import_directory;
	GtkWidget *			eventbox_force_import_directory;
    GtkWidget *         filechooserbutton_force_import_directory;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageFiles, prefs_page_files, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des fichiers
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_files_setup_files_page (PrefsPageFiles *page)
{
	GtkWidget *head_page;
	PrefsPageFilesPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_files_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Files"), "gsb-files-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_files), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_files), head_page, 0);

    /* set the variables for account */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_load_last_file),
								  conf.dernier_fichier_auto);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sauvegarde_auto),
								  conf.sauvegarde_auto);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_force_enregistrement),
								  conf.force_enregistrement);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_compress_file),
								  conf.compress_file);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_crypt_file),
								  etat.crypt_file);

    /* set the max number of files */
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_nb_max_derniers_fichiers),
							   conf.nb_max_derniers_fichiers_ouverts);

    /* Connect signal */
    g_signal_connect (priv->eventbox_load_last_file,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_load_last_file);

    g_signal_connect (priv->checkbutton_load_last_file,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.dernier_fichier_auto);

    g_signal_connect (priv->eventbox_sauvegarde_auto,
                      "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
                      priv->checkbutton_sauvegarde_auto);

    g_signal_connect (priv->checkbutton_sauvegarde_auto,
                      "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &conf.sauvegarde_auto);

    g_signal_connect (priv->eventbox_force_enregistrement,
                      "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
                      priv->checkbutton_force_enregistrement);

    g_signal_connect (priv->checkbutton_force_enregistrement,
                      "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &conf.force_enregistrement);

    g_signal_connect (priv->eventbox_compress_file,
                      "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
                      priv->checkbutton_compress_file);

    g_signal_connect (priv->checkbutton_compress_file,
                      "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &conf.compress_file);

	g_signal_connect (priv->eventbox_crypt_file,
                      "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
                      priv->checkbutton_crypt_file);

    g_signal_connect (priv->checkbutton_crypt_file,
                      "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &etat.crypt_file);

	g_signal_connect_after (priv->checkbutton_crypt_file,
							"toggled",
							G_CALLBACK (gsb_gui_encryption_toggled),
							NULL);

    /* callback for spinbutton_nb_max_derniers_fichiers_ouverts */
    g_signal_connect (priv->spinbutton_nb_max_derniers_fichiers,
                        "value-changed",
                        G_CALLBACK (utils_prefs_spinbutton_changed),
                        &conf.nb_max_derniers_fichiers_ouverts);

	g_signal_connect_after (priv->spinbutton_nb_max_derniers_fichiers,
                        "value-changed",
                        G_CALLBACK (grisbi_app_update_recent_files_menu),
                        NULL);

    /* set the variables for backup tab */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_make_bakup_single_file),
								  conf.make_bakup_single_file);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_compress_backup),
								  conf.compress_backup);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sauvegarde_demarrage),
								  conf.sauvegarde_demarrage);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_make_backup),
								  conf.make_backup);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_make_backup_every_minutes),
								  conf.make_backup_every_minutes);

    /* set minutes */
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_make_backup_nb_minutes),
							   conf.make_backup_nb_minutes);

    /* set current folder for backup files */
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (priv->filechooserbutton_backup),
										 gsb_file_get_backup_path ());

    /* set current folder for import files */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_force_import_directory),
								  conf.force_import_directory);
	if (conf.import_directory)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (priv->filechooserbutton_force_import_directory),
											 conf.import_directory);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (priv->filechooserbutton_force_import_directory), FALSE);

	g_object_set_data (G_OBJECT (priv->checkbutton_force_import_directory),
                       "widget",
					   priv->filechooserbutton_force_import_directory);

    /* Connect signal */
    g_signal_connect (priv->eventbox_make_bakup_single_file,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_make_bakup_single_file);

    g_signal_connect (priv->checkbutton_make_bakup_single_file,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.make_bakup_single_file);

    g_signal_connect (priv->eventbox_compress_backup,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_compress_backup);

    g_signal_connect (priv->checkbutton_compress_backup,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &conf.compress_backup);

    g_signal_connect (priv->eventbox_sauvegarde_demarrage,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
                      priv->checkbutton_sauvegarde_demarrage);

    g_signal_connect (priv->checkbutton_sauvegarde_demarrage,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &conf.sauvegarde_demarrage);

    g_signal_connect (priv->eventbox_make_backup,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
                      priv->checkbutton_make_backup);

    g_signal_connect (priv->checkbutton_make_backup,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &conf.make_backup);

    g_signal_connect (priv->eventbox_make_backup_every_minutes,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
                      priv->checkbutton_make_backup_every_minutes);

    g_signal_connect (priv->checkbutton_make_backup_every_minutes,
                      "toggled",
                      G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &conf.make_backup_every_minutes);

    /* callback for spinbutton_make_backup_nb_minutes */
    g_object_set_data (G_OBJECT (priv->spinbutton_make_backup_nb_minutes),
                       "button", priv->checkbutton_make_backup_every_minutes);
	g_object_set_data (G_OBJECT (priv->checkbutton_make_backup_every_minutes),
                       "widget", priv->spinbutton_make_backup_nb_minutes);
	if (!conf.make_backup_every_minutes)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_make_backup_nb_minutes), FALSE);

    g_signal_connect (priv->spinbutton_make_backup_nb_minutes,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &conf.make_backup_nb_minutes);

    /* connect the signal for filechooserbutton_backup */
    g_signal_connect (G_OBJECT (priv->filechooserbutton_backup),
                      "selection-changed",
                      G_CALLBACK (utils_prefs_page_dir_chosen),
                      "backup_path");

    g_signal_connect (priv->eventbox_force_import_directory,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_force_import_directory);
    g_signal_connect (priv->checkbutton_force_import_directory,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.force_import_directory);

    /* connect the signal for filechooserbutton_force_import_directory */
    g_signal_connect (G_OBJECT (priv->filechooserbutton_force_import_directory),
                      "selection-changed",
                      G_CALLBACK (utils_prefs_page_dir_chosen),
                      "import_directory");
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_files_init (PrefsPageFiles *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_files_setup_files_page (page);
}

static void prefs_page_files_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_files_parent_class)->dispose (object);
}

static void prefs_page_files_class_init (PrefsPageFilesClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_files_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_files.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, vbox_files);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_load_last_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_load_last_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_sauvegarde_auto);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_sauvegarde_auto);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_force_enregistrement);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_force_enregistrement);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_compress_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_compress_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_crypt_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_crypt_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, spinbutton_nb_max_derniers_fichiers);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_make_bakup_single_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_make_bakup_single_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_compress_backup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_compress_backup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_sauvegarde_demarrage);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_sauvegarde_demarrage);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_make_backup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_make_backup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_make_backup_every_minutes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_make_backup_every_minutes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, spinbutton_make_backup_nb_minutes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, filechooserbutton_backup);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_force_import_directory);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, eventbox_force_import_directory);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, filechooserbutton_force_import_directory);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageFiles * prefs_page_files_new (GrisbiPrefs *win)
{
	return g_object_new (PREFS_PAGE_FILES_TYPE, NULL);
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

