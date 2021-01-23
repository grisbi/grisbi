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
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_select_icon.h"
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
	GtkWidget *			checkbutton_use_icons_file_dir;
    GtkWidget *         checkbutton_crypt_file;
	GtkWidget *			label_use_icons_file_dir;
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
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_files_use_icons_file_dir_toggled (GtkToggleButton *checkbutton,
														 PrefsPageFilesPrivate *priv)
{
	if (gtk_toggle_button_get_active (checkbutton))
	{
		GSList *icons_name_list = NULL;
		GSList *tmp_list;
		gchar *msg;
		gchar *str_to_free = NULL;
		gchar *tmp_str = NULL;
		gint nbre_account = 0;
		gint nbre_errors = 0;
		gint i = 0;

		devel_debug (NULL);
		tmp_list = gsb_data_account_get_list_accounts ();
		while (tmp_list)
		{
			const gchar *account_icon_name;
			gint tmp_account;

			tmp_account = gsb_data_account_get_no_account (tmp_list->data);
			account_icon_name = gsb_data_account_get_name_icon (tmp_account);
			if (account_icon_name && strlen (account_icon_name))
			{
				gchar *icon_basename;
				gchar *new_icon_name;

				icon_basename = g_path_get_basename (account_icon_name);

				/* on ignore les doublons */
				if (!g_slist_find_custom (icons_name_list, account_icon_name, (GCompareFunc) g_strcmp0))
				{
					icons_name_list = g_slist_insert_sorted (icons_name_list,
															 g_strdup (account_icon_name),
															 (GCompareFunc) g_strcmp0);
				}

				new_icon_name = gsb_select_icon_set_icon_in_user_icons_dir (account_icon_name);
				gsb_data_account_set_name_icon (tmp_account, new_icon_name);
				if (new_icon_name == NULL)
					nbre_errors++;
				else
					g_free (new_icon_name);
				g_free (icon_basename);
				nbre_account++;
			}
			tmp_list = tmp_list->next;
		}

		/* set list icons for msg */
		tmp_list = icons_name_list;
		while (tmp_list)
		{
			if (tmp_str == NULL)
			{
				tmp_str =  g_strdup_printf ("%s\n", (gchar *) g_slist_nth_data (icons_name_list, i));
			}
			else
			{
				tmp_str =  g_strconcat (tmp_str, "\t- ", (gchar *) g_slist_nth_data (icons_name_list, i), "\n", NULL);
				g_free (str_to_free);
			}
			str_to_free = tmp_str;
			i++;
			tmp_list = tmp_list->next;
		}
		if (nbre_errors)
			msg = g_strdup_printf (_("%d comptes ont une icône spécifique parmi les %d icônes suivantes :\n"
									 "\t- %s"
									 "\n"
									 "Une ou plusieurs erreurs de copie ont eu lieu probablement "
									 "pour des raisons de droits d'écriture dans le répertoire de "
									 "destination.\n"),
								   nbre_account,
								   g_slist_length (icons_name_list),
								   tmp_str);
		else
			msg = g_strdup_printf (_("%d comptes ont une icône spécifique parmi les %d icônes suivantes :\n"
									 "\t- %s"
									 "\n"
									 "Ces icônes, préfixées avec le nom du compte, ont été copiées "
									 "dans le répertoire :\n%s\n"),
								   nbre_account,
								   g_slist_length (icons_name_list),
								   tmp_str,
								   gsb_dirs_get_user_icons_dir ());

		dialogue_warning_hint (msg,_("Déplacement des icones de compte dans le répertoire du fichier de comptes"));

		g_free (tmp_str);
		g_slist_free_full (icons_name_list, (GDestroyNotify) g_free);
	}
}

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
	const gchar *icons_dir;
	gchar *tmp_str;
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

	/* set the label for accounts icons */
	icons_dir = gsb_dirs_get_user_icons_dir ();
	tmp_str = g_strconcat (_("Put user icons in the directory: "), icons_dir, NULL);
	gtk_label_set_text (GTK_LABEL (priv->label_use_icons_file_dir), tmp_str);
	g_free (tmp_str);

	/* test and set value */
	if (w_etat->use_icons_file_dir)
	{
		const gchar *icon_dir;

		icon_dir = gsb_dirs_get_user_icons_dir ();
		if (!g_file_test (icon_dir, G_FILE_TEST_IS_DIR))
		{
			gchar *msg;

			msg = g_strdup (_("Vous avez selectionné l'option \"mettre les icônes spécifiques "
							  "des comptes\" dans le répertoire du fichier de comptes mais vous "
							  "n'avez pas utilisé cette possibilité.\n"
							  "Si vous en avez besoin faites le sinon décochez l'option"));

			dialogue_warning_hint (msg,_("Mettre les icones des comptes dans le répertoire du fichier de comptes"));
			g_free (msg);

		}
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_icons_file_dir),
									  w_etat->use_icons_file_dir);
	}

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

	g_signal_connect (priv->checkbutton_use_icons_file_dir,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &w_etat->use_icons_file_dir);

	g_signal_connect_after (priv->checkbutton_use_icons_file_dir,
							"toggled",
							G_CALLBACK (prefs_page_files_use_icons_file_dir_toggled),
							NULL);

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
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, checkbutton_use_icons_file_dir);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageFiles, label_use_icons_file_dir);
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

