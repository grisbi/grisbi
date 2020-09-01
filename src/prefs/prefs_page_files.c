/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
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
    GtkWidget *         checkbutton_sauvegarde_auto;
    GtkWidget *         checkbutton_force_enregistrement;
    GtkWidget *         checkbutton_compress_file;
    GtkWidget *         checkbutton_crypt_file;
	GtkWidget *			label_nb_max_derniers_fichiers;
	GtkWidget *         spinbutton_nb_max_derniers_fichiers;

    GtkWidget *         checkbutton_make_bakup_single_file;
    GtkWidget *         checkbutton_compress_backup;
    GtkWidget *         checkbutton_sauvegarde_demarrage;
    GtkWidget *         checkbutton_make_backup;
    GtkWidget *         checkbutton_make_backup_every_minutes;
    GtkWidget *         spinbutton_make_backup_nb_minutes;
    GtkWidget *         checkbutton_remove_backup_files;
    GtkWidget *         spinbutton_remove_backup_months;
    GtkWidget *         filechooserbutton_backup;

    GtkWidget *			checkbutton_force_import_directory;
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
	gboolean is_loading;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;
	PrefsPageFilesPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_files_get_instance_private (page);
	is_loading = grisbi_win_file_is_loading ();
	a_conf = grisbi_app_get_a_conf ();
	w_etat = grisbi_win_get_w_etat ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Files"), "gsb-files-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_files), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_files), head_page, 0);

    /* set the variables for account */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_load_last_file),
								  a_conf->dernier_fichier_auto);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sauvegarde_auto),
								  a_conf->sauvegarde_auto);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_force_enregistrement),
								  a_conf->force_enregistrement);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_compress_file),
								  a_conf->compress_file);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_crypt_file),
								  w_etat->crypt_file);
	gtk_widget_set_sensitive (priv->checkbutton_crypt_file, is_loading);

	/* set the max number of files */
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_nb_max_derniers_fichiers),
							   a_conf->nb_max_derniers_fichiers_ouverts);

    /* Connect signal */
    g_signal_connect (priv->checkbutton_load_last_file,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->dernier_fichier_auto);

    g_signal_connect (priv->checkbutton_sauvegarde_auto,
                      "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->sauvegarde_auto);

    g_signal_connect (priv->checkbutton_force_enregistrement,
                      "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->force_enregistrement);

    g_signal_connect (priv->checkbutton_compress_file,
                      "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->compress_file);

#ifdef HAVE_SSL
	g_signal_connect (priv->checkbutton_crypt_file,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->crypt_file);

	g_signal_connect_after (priv->checkbutton_crypt_file,
							"toggled",
							G_CALLBACK (utils_prefs_encryption_toggled),
							NULL);
#endif

	/* callback for spinbutton_nb_max_derniers_fichiers_ouverts */
    g_signal_connect (priv->spinbutton_nb_max_derniers_fichiers,
                        "value-changed",
                        G_CALLBACK (utils_prefs_spinbutton_changed),
                        &a_conf->nb_max_derniers_fichiers_ouverts);

	g_signal_connect_after (priv->spinbutton_nb_max_derniers_fichiers,
                        "value-changed",
                        G_CALLBACK (grisbi_app_update_recent_files_menu),
                        NULL);

    /* set the variables for backup tab */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_make_bakup_single_file),
								  a_conf->make_bakup_single_file);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_compress_backup),
								  a_conf->compress_backup);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sauvegarde_demarrage),
								  a_conf->sauvegarde_demarrage);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_make_backup),
								  a_conf->sauvegarde_fermeture);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_make_backup_every_minutes),
								  a_conf->make_backup_every_minutes);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_remove_backup_files),
								  a_conf->remove_backup_files);
    /* set minutes */
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_make_backup_nb_minutes),
							   a_conf->make_backup_nb_minutes);

	/* set months */
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_remove_backup_months),
							   a_conf->remove_backup_months);

    /* set current folder for backup files */
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (priv->filechooserbutton_backup),
										 gsb_file_get_backup_path ());

    /* set current folder for import files */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_force_import_directory),
								  a_conf->force_import_directory);
	if (a_conf->import_directory)
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (priv->filechooserbutton_force_import_directory),
											 a_conf->import_directory);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (priv->filechooserbutton_force_import_directory), FALSE);

	g_object_set_data (G_OBJECT (priv->checkbutton_force_import_directory),
                       "widget",
					   priv->filechooserbutton_force_import_directory);

    /* Connect signal */
    g_signal_connect (priv->checkbutton_make_bakup_single_file,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->make_bakup_single_file);

    g_signal_connect (priv->checkbutton_compress_backup,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->compress_backup);

    g_signal_connect (priv->checkbutton_sauvegarde_demarrage,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->sauvegarde_demarrage);

    g_signal_connect (priv->checkbutton_make_backup,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->sauvegarde_fermeture);

    g_signal_connect (priv->checkbutton_make_backup_every_minutes,
                      "toggled",
                      G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->make_backup_every_minutes);

    g_signal_connect (priv->checkbutton_remove_backup_files,
                      "toggled",
                      G_CALLBACK (utils_prefs_page_checkbutton_changed),
                      &a_conf->remove_backup_files);

    /* callback for spinbutton_make_backup_nb_minutes */
    g_object_set_data (G_OBJECT (priv->spinbutton_make_backup_nb_minutes),
                       "button", priv->checkbutton_make_backup_every_minutes);
	g_object_set_data (G_OBJECT (priv->checkbutton_make_backup_every_minutes),
                       "widget", priv->spinbutton_make_backup_nb_minutes);
	if (!a_conf->make_backup_every_minutes)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_make_backup_nb_minutes), FALSE);

    g_signal_connect (priv->spinbutton_make_backup_nb_minutes,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &a_conf->make_backup_nb_minutes);

    /* callback for spinbutton_remove_backup_months */
    g_object_set_data (G_OBJECT (priv->spinbutton_remove_backup_months),
                       "button", priv->checkbutton_remove_backup_files);
	g_object_set_data (G_OBJECT (priv->checkbutton_remove_backup_files),
                       "widget", priv->spinbutton_remove_backup_months);
	if (!a_conf->remove_backup_files)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->spinbutton_remove_backup_months), FALSE);

    g_signal_connect (priv->spinbutton_remove_backup_months,
					  "value-changed",
					  G_CALLBACK (utils_prefs_spinbutton_changed),
					  &a_conf->remove_backup_months);

    /* connect the signal for filechooserbutton_backup */
    g_signal_connect (G_OBJECT (priv->filechooserbutton_backup),
                      "selection-changed",
                      G_CALLBACK (utils_prefs_page_dir_chosen),
                      (gpointer) "backup_path");

    g_signal_connect (priv->checkbutton_force_import_directory,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->force_import_directory);

    /* connect the signal for filechooserbutton_force_import_directory */
    g_signal_connect (G_OBJECT (priv->filechooserbutton_force_import_directory),
                      "selection-changed",
                      G_CALLBACK (utils_prefs_page_dir_chosen),
                      (gpointer) "import_directory");
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
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_sauvegarde_auto);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_force_enregistrement);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_compress_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_crypt_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, label_nb_max_derniers_fichiers);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, spinbutton_nb_max_derniers_fichiers);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_make_bakup_single_file);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_compress_backup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_sauvegarde_demarrage);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_make_backup);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_make_backup_every_minutes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, spinbutton_make_backup_nb_minutes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_remove_backup_files);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, spinbutton_remove_backup_months);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, filechooserbutton_backup);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_force_import_directory);
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

