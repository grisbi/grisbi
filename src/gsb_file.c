/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                            */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */


#include "config.h"

#include "include.h"
//~ #include <time.h>
//~ #include <limits.h>
//~ #include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#ifdef G_OS_WIN32
#include <stdlib.h> // For realpath()
#endif /* WIN32 */

/*START_INCLUDE*/
#include "gsb_file.h"
#include "accueil.h"
#include "dialog.h"
#include "etats_gtktable.h"
#include "grisbi_app.h"
#include "gsb_assistant_account.h"
#include "gsb_data_account.h"
#include "gsb_data_archive_store.h"
#include "gsb_dirs.h"
#include "gsb_file_load.h"
#include "gsb_file_save.h"
#include "gsb_file_util.h"
#include "gsb_real.h"
#include "gsb_transactions_list.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/**
 * keep the last path used in grisbi, save in the configuration at the end */
static gchar *last_path_used = NULL;

static gchar *backup_path = NULL;

/** the timeout used to save a backup every x minutes */
gint id_timeout = 0;

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void gsb_file_remove_old_backup (const gchar *filename,
										gint remove_backup_months)
{
	GFileEnumerator *direnum;
	GFile *dir;
	GDate *first_old_date;
    GSList *filenames = NULL;
    GSList *iterator;
	gchar *basename;
	gchar *name = NULL;
	gchar *tmp_date;

	devel_debug (filename);

	/* get name of file without extension */
	basename = g_path_get_basename (filename);
	if (g_str_has_suffix (basename, ".gsb"))
	{
		name = g_strndup (basename, strlen (basename) - 4);
		g_free (basename);
	}
	else
	{
		g_free (basename);

		return;
	}

	if (!name)
		return;

	/* get today date and first_old_date */
	first_old_date = gdate_today ();
	g_date_subtract_months (first_old_date, remove_backup_months);

	/* balayage du répertoire de sauvegarde */
	dir =  g_file_new_for_path (backup_path);
	direnum = g_file_enumerate_children (dir,
										"standard::*",
										G_FILE_QUERY_INFO_NONE,
										NULL,
										NULL);
	if (!direnum)
	{
		g_free(name);
		return;
	}

	while (TRUE)
	{
		GFileInfo *info = NULL;
		const gchar *old_filename;

		if (!g_file_enumerator_iterate (direnum, &info, NULL, NULL, NULL))
			break;

		if (!info)
		  break;

		old_filename = g_file_info_get_name (info);
		if (old_filename && strlen (old_filename))
		{
			if (g_str_has_suffix (old_filename, ".gsb"))
			{
				if (name && g_str_has_prefix (old_filename, name))
				{
					GDate *date;
					gchar *str_date;
					gchar *tmp_year;
					gchar *tmp_month;
					gchar *tmp_day;
					size_t long_name;

					long_name = strlen (name);
					str_date = g_strndup (old_filename+long_name+1, 8);
					tmp_year = g_strndup (str_date, 4);
					if (g_date_valid_year (utils_str_atoi (tmp_year)) == FALSE)
					{
						g_free (str_date);
						g_free (tmp_year);

						continue;
					}
					tmp_month = g_strndup (str_date+4, 2);
					if (g_date_valid_month (utils_str_atoi (tmp_month)) == FALSE)
					{
						g_free (str_date);
						g_free (tmp_year);
						g_free (tmp_month);

						continue;
					}
					tmp_day = g_strndup (str_date+6, 2);
					if (g_date_valid_day (utils_str_atoi (tmp_day)) == FALSE)
					{
						g_free (str_date);
						g_free (tmp_year);
						g_free (tmp_month);
						g_free (tmp_day);

						continue;
					}
					date = g_date_new_dmy (utils_str_atoi (tmp_day),
										   utils_str_atoi (tmp_month),
										   utils_str_atoi (tmp_year));
					if (g_date_valid (date) == FALSE)
					{
						g_date_free (date);
						g_free (str_date);
						g_free (tmp_year);
						g_free (tmp_month);
						g_free (tmp_day);

						continue;
					}
					if (g_date_compare (date, first_old_date) < 0)
					{
						gchar *tmp_filename;

						tmp_filename = g_build_filename (backup_path, G_DIR_SEPARATOR_S, old_filename, NULL);
						filenames = g_slist_append (filenames, tmp_filename);
					}
					g_date_free (date);
					g_free (str_date);
					g_free (tmp_year);
					g_free (tmp_month);
					g_free (tmp_day);
				}
			}
		}
		else
		{
			continue;
		}
	}
	g_object_unref (direnum);
	g_object_unref (dir);
	g_free (name);

	if (!filenames)
	{
		g_date_free (first_old_date);

		return;
	}

	tmp_date = gsb_format_gdate (first_old_date);
	dialogue_message ("remove-backup-files", g_slist_length (filenames), tmp_date);
	g_free(tmp_date);

	g_date_free (first_old_date);

	iterator = filenames;
	while (iterator)
    {
		GFile *file;
		gchar *path;

		path = iterator->data;

		file = g_file_new_for_path (path);
		g_file_delete (file, FALSE, NULL);
		iterator = iterator->next;
	}

	g_slist_free_full (filenames, g_free);
}

/**
 * teste la validité d'un fichier
 *
 * \param const gchar 	filename
 *
 * \return TRUE if OK FALSE other
 **/
static gboolean gsb_file_test_file (const gchar *filename)
 {
	gchar* tmp_str1;
	gchar* tmp_str2;

	if (!filename || !strlen (filename) || !g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        tmp_str1 = g_strdup_printf (_("Cannot open file '%s': %s"),
									filename,
									_("File does not exist"));

        tmp_str2 = g_strdup_printf (_("Error loading file '%s'"), filename);
        dialogue_error_hint (tmp_str1, tmp_str2);

        g_free (tmp_str1);
        g_free (tmp_str2);

        return FALSE;
    }
    /* check here if it's not a regular file */
    if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    {
        tmp_str1 = g_strdup_printf (_("%s doesn't seem to be a regular file,\n"
                                      "please check it and try again."),
                                    filename);
        tmp_str2 = g_strdup_printf (_("Error loading file '%s'"),
                                    filename);
        dialogue_error_hint (tmp_str1 , tmp_str2);

        g_free (tmp_str1);
        g_free (tmp_str2);

        return (FALSE);
    }
	return TRUE;
 }

/**
 * ask the name of the file to save it
 *
 * \param
 *
 * \return a newly allocated string containing the new name
 **/
static gchar *gsb_file_dialog_ask_name (const gchar *filename)
{
    GtkWidget *dialog;
	GtkWidget *button_cancel;
	GtkWidget *button_save;
    gchar *new_name;
    gchar *tmp_last_directory;
    gint result;

	dialog = gtk_file_chooser_dialog_new (_("Name the accounts file"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_FILE_CHOOSER_ACTION_SAVE,
										  NULL, NULL,
										  NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_CANCEL);

	button_save = gtk_button_new_with_label (_("Save"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_save, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_save, TRUE);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gsb_file_get_last_path ());
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);

    if (!filename)
    {
        gchar* tmp_str;
		GrisbiWinEtat *w_etat;

		w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

		tmp_str = g_strconcat (w_etat->accounting_entity, ".gsb", NULL);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), tmp_str);
		g_free (tmp_str);
	}
	else
        gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (dialog), filename);

	gtk_widget_show_all (dialog);

	result = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (result)
    {
	case GTK_RESPONSE_OK :
	    new_name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        tmp_last_directory = utils_files_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE);
        gsb_file_update_last_path (tmp_last_directory);
        g_free (tmp_last_directory);
	    gtk_widget_destroy (GTK_WIDGET (dialog));
	    break;

	default :
	    gtk_widget_destroy (GTK_WIDGET (dialog));
	    return NULL;
    }

    if (!g_strrstr (new_name, "."))
    {
        gchar* tmp_str;

        tmp_str = new_name;
        new_name = g_strconcat (tmp_str, ".gsb", NULL);
        g_free (tmp_str);
    }

    return new_name;
}

/**
 * propose to save the file if changed
 * if the current file is not changed, return GTK_RESPONSE_NO directly, without dialog
 *
 * \param
 *
 * \return GTK_RESPONSE_OK to save, GTK_RESPONSE_NO not to save, other to cancel
 **/
static gint gsb_file_dialog_save (const gchar *filename,
								  gint origine)
{
    GtkWidget *dialog;
    GtkWidget *button_cancel;
    GtkWidget *button_close;
    GtkWidget *button_save;
    gchar *hint = NULL;
    gchar *message = NULL;
    gint result;
	gchar *basename = NULL;

	dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_WARNING,
									 GTK_BUTTONS_NONE,
									 " ");

	if (filename)
		basename = g_path_get_basename(filename);

	switch (origine)
	{
		case -1:
		{
			gchar *time_elapsed;
			time_t now;
			gint difference;
			GrisbiWinRun *w_run;

			w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
			now = time (NULL);
			difference = (int) (difftime (now, w_run->file_modification));
			hint = g_strdup_printf (_("Save changes to document '%s' before closing?"),
									(basename ? basename : _("unnamed")));

			button_close = gtk_button_new_with_label (_("Close without saving"));
			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_close, GTK_RESPONSE_NO);

			button_cancel = gtk_button_new_with_label (_("Cancel"));
			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_REJECT);

			button_save = gtk_button_new_with_label (_("Save"));
			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_save, GTK_RESPONSE_OK);

			if (difference >= 120)
			{
				time_elapsed = g_strdup_printf (_("%d minutes and %d seconds"), difference / 60, difference % 60);
			}
			else if (difference >= 60)
			{
				time_elapsed = g_strdup_printf (_("1 minute and %d seconds"), difference % 60);
			}
			else
			{
				time_elapsed = g_strdup_printf (_("%d seconds"), difference);
			}
			message = g_strdup_printf (_("If you close without saving, all of your changes "
										 "since %s will be discarded."),
										time_elapsed);
			g_free (time_elapsed);


			break;
		}
		default:
		{
			hint = g_strdup_printf (_("Save changes in '%s' file?"),
									(basename ? basename : _("unnamed")));

			button_cancel = gtk_button_new_with_label (_("Cancel"));
			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_NO);

			button_save = gtk_button_new_with_label (_("Save"));
			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_save, GTK_RESPONSE_OK);

			message = g_strdup("");
		}
	}
	gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	g_object_set (G_OBJECT (dialog), "text", hint, "secondary-text", message, NULL);

	gtk_widget_show_all (dialog);

    g_free (message);
    g_free (hint);
    g_free(basename);

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);


    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return result;
}

/**
 * save a backup of the file
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 **/
static gboolean gsb_file_save_backup (gboolean make_bakup_single_file,
									  gboolean compress_backup)
{
    gboolean retour;
	gchar *new_filename;
    const gchar *filename;
    struct tm *day_time;
    time_t temps;
    gchar *name;

    if (!gsb_file_get_backup_path () || !gsb_file_get_modified (TRUE))
        return FALSE;

    grisbi_win_status_bar_message (_("Saving backup"));

	/* on récupère d'abord le nom du fichier */
	filename = grisbi_win_get_filename (NULL);
    name = g_path_get_basename (filename);
    if (g_str_has_suffix (name, ".gsb"))
    {
        gchar **tab_str;

        tab_str = g_strsplit (name, ".gsb", 0);
        if (tab_str[0] && strlen (tab_str[0]))
        {
            g_free (name);
            name = g_strdup (tab_str[0]);
        }
        g_strfreev (tab_str);
    }
    /* create a filename for the backup :
     * filename_yyyymmddTmmhhss.gsb */
    if (make_bakup_single_file)
    {
        new_filename =  g_strdup_printf ("%s%s%s_backup.gsb",
										 gsb_file_get_backup_path (),
										 G_DIR_SEPARATOR_S,
										 name);
    }
    else
    {
        time (&temps);
        day_time = localtime (&temps);
        new_filename =  g_strdup_printf ("%s%s%s_%d%02d%02dT%02d%02d%02d.gsb",
										 gsb_file_get_backup_path (),
										 G_DIR_SEPARATOR_S,
										 name,
										 day_time->tm_year + 1900,
										 day_time->tm_mon + 1,
										 day_time->tm_mday,
										 day_time->tm_hour,
										 day_time->tm_min,
										 day_time->tm_sec);
    }

    retour = gsb_file_save_save_file (new_filename, compress_backup, 0);

    g_free (new_filename);
    g_free (name);

    grisbi_win_status_bar_message (_("Done"));

    return (retour);
}

/**
 * called every x minutes defined by user
 * to save a backup of the file
 *
 * \param null
 *
 * \return TRUE to continue the timeout, FALSE to stop the timeout
 **/
static gboolean gsb_file_automatic_backup (gpointer p_conf)
{
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
	a_conf = (GrisbiAppConf *) p_conf;

    if (!a_conf->make_backup_every_minutes)
	/* stop the timeout */
        return FALSE;

    /* we save only if there is a nb of minutes, but don't stop the timer if not */
    if (a_conf->make_backup_nb_minutes)
        gsb_file_save_backup (a_conf->make_bakup_single_file, a_conf->compress_backup);

    return TRUE;
}

/**
 * save the file
 *
 * \param origine 0 from gsb_file_save (menu), -1 from gsb_file_close, -2 from gsb_file_save_as
 *
 * \return TRUE if ok, FALSE if problem
 **/
static gboolean gsb_file_save_file (gint origine)
{
    gint result = 0;
    gchar *nouveau_nom_enregistrement;
	gchar *filename;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;

    devel_debug_int (origine);

	/* on regarde si il y a quelque chose à sauvegarder sauf pour "sauvegarder sous" */
	if ((!gsb_file_get_modified (FALSE) && origine != -2)
        ||
        !gsb_data_account_get_number_of_accounts ())
    {
        notice_debug ("nothing done in gsb_file_save_file");
        return (TRUE);
    }

	a_conf = grisbi_app_get_a_conf ();
	w_etat = grisbi_win_get_w_etat ();

	/* on récupère le nom du fichier */
	filename = g_strdup (grisbi_win_get_filename (NULL));

	/* on vérifie que le fichier n'est pas locké si il l'est on sort */
    if (w_etat->fichier_deja_ouvert && !a_conf->force_enregistrement && origine != -2)
    {
        gchar* tmp_str1;
        gchar* tmp_str2;

        tmp_str1 = g_strdup_printf(_("Grisbi was unable to save this file because it is locked.\n"
									 "Please save it with another name or activate the \"%s\" "
                                     "option in preferences."),
                                   _("Force saving of locked files"));
        tmp_str2 = g_strdup_printf(_("Can not save file \"%s\""), filename);
        dialogue_error_hint (tmp_str1, tmp_str2);
        g_free (tmp_str1);
        g_free (tmp_str2);

        return (FALSE);
    }

    /* on commence par demander si on sauvegarde ou pas sauf pour "sauvegarder sous" */
    if (!a_conf->sauvegarde_auto && origine != -2)
    {
        result = gsb_file_dialog_save (filename, origine);
        if (result == GTK_RESPONSE_NO)
		{
			gsb_file_set_modified (FALSE);
            return (TRUE);
		}
		else if (result != GTK_RESPONSE_OK)
		{
			//~ gsb_file_set_modified (FALSE);
            return (FALSE);
		}
    }

    /* si le fichier de comptes n'a pas de nom ou si on enregistre sous un nouveau nom */
    /*     c'est ici */
    if (!filename || origine == -2)
        nouveau_nom_enregistrement = gsb_file_dialog_ask_name (filename);
    else
        nouveau_nom_enregistrement = g_strdup (filename);

    if (!nouveau_nom_enregistrement)
        return FALSE;

    /* make backup before saving if asked */
    if (a_conf->sauvegarde_fermeture)
        gsb_file_save_backup (a_conf->make_bakup_single_file, a_conf->compress_backup);

    /*  on a maintenant un nom de fichier et on sait qu'on peut sauvegarder */
    grisbi_win_status_bar_message (_("Saving file"));

    result = gsb_file_save_save_file (nouveau_nom_enregistrement, a_conf->compress_file, 0);

    if (result)
    {
		grisbi_win_set_filename (NULL, nouveau_nom_enregistrement);

		/* on ajoute un item au menu recent_file si origine = -2 */
		if (origine == -2)
			utils_files_append_name_to_recent_array (nouveau_nom_enregistrement);

		/* saving was right, so unlock the last name */
        gsb_file_util_modify_lock (filename, FALSE);

        /* and lock the new name */
        gsb_file_util_modify_lock (nouveau_nom_enregistrement, TRUE);

        /* update variables */
        w_etat->fichier_deja_ouvert = 0;
        gsb_file_set_modified (FALSE);
        grisbi_win_set_window_title (gsb_gui_navigation_get_current_account ());

		/* Si nettoyage des fichiers de backup on le fait ici */
		if (a_conf->remove_backup_files)
		{
			GrisbiWinRun *w_run;

			w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
			if (!w_run->remove_backup_files)
			{
				gsb_file_remove_old_backup (nouveau_nom_enregistrement,a_conf->remove_backup_months);
				w_run->remove_backup_files = TRUE;
			}
		}
    }

	g_free (filename);
	g_free (nouveau_nom_enregistrement);
    grisbi_win_status_bar_message (_("Done"));

    return (result);
}

/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 * come here to finish to create the new file
 * at this time, categories and currencies must created
 * this part will just launch the assistant to create a new account
 * a finish the initialisation to lauch grisbi
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_file_new_finish (void)
{
	GrisbiWinRun *w_run;

    /* create the first account */
    if (!gsb_assistant_account_run ())
    {
        init_variables ();
        return FALSE;
    }

    /* init the gui */
    grisbi_win_new_file_gui ();

    mise_a_jour_accueil (TRUE);
    gsb_gui_navigation_set_selection (GSB_HOME_PAGE, -1, 0);

	/* sensibilise les préférences */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->file_is_loading = TRUE;

    gsb_file_set_modified (TRUE);
    return FALSE;
}

/**
 * called by file->open
 * open a new file
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_file_open_menu (void)
{
    GtkWidget *selection_fichier;
	GtkWidget *button_cancel;
	GtkWidget *button_open;
    GtkFileFilter * filter;
    gboolean result = FALSE;
    gchar *tmp_last_directory;
	gchar *filename;

    selection_fichier = gtk_file_chooser_dialog_new (_("Open an accounts file"),
												     GTK_WINDOW (grisbi_app_get_active_window (NULL)),
												     GTK_FILE_CHOOSER_ACTION_OPEN,
												     NULL, NULL,
													 NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (selection_fichier), button_cancel, GTK_RESPONSE_CANCEL);

	button_open = gtk_button_new_with_label (_("Open"));
	gtk_dialog_add_action_widget (GTK_DIALOG (selection_fichier), button_open, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_open, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (selection_fichier), GTK_RESPONSE_OK);

    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (selection_fichier), gsb_file_get_last_path ());
    gtk_window_set_position (GTK_WINDOW (selection_fichier), GTK_WIN_POS_CENTER_ON_PARENT);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("Grisbi files (*.gsb)"));
    gtk_file_filter_add_pattern (filter, "*.gsb");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (selection_fichier), filter);
    gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (selection_fichier), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (selection_fichier), filter);

	gtk_widget_show_all (selection_fichier);

    switch (gtk_dialog_run (GTK_DIALOG (selection_fichier)))
    {
		case GTK_RESPONSE_OK:
			if (gsb_file_close())
			{
				gtk_widget_hide (selection_fichier);
				filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (selection_fichier));

				tmp_last_directory = utils_files_selection_get_last_directory (GTK_FILE_CHOOSER (selection_fichier), TRUE);
				gsb_file_update_last_path (tmp_last_directory);
				g_free (tmp_last_directory);
				result = gsb_file_open_file (filename);
				g_free (filename);
			}
			break;
		  default:
			break;
    }

    tmp_last_directory = utils_files_selection_get_last_directory (GTK_FILE_CHOOSER (selection_fichier), TRUE);
    gsb_file_update_last_path (tmp_last_directory);
    g_free (tmp_last_directory);
    gtk_widget_destroy (selection_fichier);

    return result;
}

/**
 * init the variable last_path_used with the path given in param
 *
 * \param last_path
 *
 * \return
 **/
void gsb_file_init_last_path (const gchar *last_path)
{
    devel_debug (last_path);

    if (last_path && strlen (last_path))
    {
        if (last_path_used)
            g_free (last_path_used);

        last_path_used = my_strdup (last_path);
    }
}

/**
 * update the variable last_path_used with the path given in param
 *
 * \param last_path
 *
 * \return
 **/
void gsb_file_update_last_path (const gchar *last_path)
{
    devel_debug (last_path);

    if (last_path && strlen (last_path))
    {
        if (last_path_used)
            g_free (last_path_used);
        last_path_used = my_strdup (last_path);
    }
}

/**
 * the last_path_used is local variable,
 * that return the content of that variable
 *
 * \param
 *
 * \return a const gchar, the last path used in grisbi
 **/
const gchar *gsb_file_get_last_path (void)
{
    return last_path_used;
}

/**
 * get the backup path
 *
 * \param
 *
 * \return a const gchar with the backup path
 **/
const gchar *gsb_file_get_backup_path (void)
{
    return backup_path;
}

/**
 * set the backup path
 *
 * \param bakcup path
 *
 * \return
 **/
void gsb_file_set_backup_path (const gchar *path)
{
	/* first free backup_path */
	if (backup_path)
		g_free (backup_path);

    if (path == NULL || strlen (path) == 0)
        backup_path = my_strdup (gsb_dirs_get_user_data_dir ());
    else
        backup_path = my_strdup (path);

    if (path && !g_file_test (path, G_FILE_TEST_EXISTS))
    {
        if (utils_files_create_XDG_dir ())
        {
            gchar * previous_backup_path = backup_path;

            backup_path = my_strdup (gsb_dirs_get_user_data_dir ());

            gchar * msg = g_strdup_printf("Backup directory %s has been replaced by %s.", previous_backup_path, backup_path);
            important_debug(msg);
            g_free(msg);

            g_free (previous_backup_path);
        }
    }
}

/**
 * open a new grisbi file, don't check anything about another opened file that must
 * have been done before
 *
 * \para filename the name of the file
 *
 * \return TRUE ok, FALSE problem
 **/
gboolean gsb_file_open_file (const gchar *filename)
{
	GrisbiAppConf *a_conf;

	devel_debug (filename);
	a_conf = grisbi_app_get_a_conf ();

	if (!gsb_file_test_file (filename))
	{
		grisbi_win_stack_box_show (NULL, "accueil_page");
		return FALSE;
	}

    grisbi_win_status_bar_wait (TRUE);
    grisbi_win_status_bar_message (_("Loading accounts"));

	/* initialise les variables d'état */
    init_variables ();

	/* try to load the file */
    /* FIXME:BUG under Windows: for unknwon reason yet filename is cleared
     * when returning from gsb_file_load_open_file!
     * making application crashes!*/

    if (gsb_file_load_open_file (filename))
    {
        /* the file has been opened succesfully */
        /* on met à jour le nom du fichier */
        grisbi_win_set_filename (NULL, filename);

		/* mark the file as opened */
        gsb_file_util_modify_lock (filename, TRUE);

        /* we make a backup if necessary */
        if (a_conf->sauvegarde_demarrage)
        {
			gsb_file_save_backup (a_conf->make_bakup_single_file, a_conf->compress_backup);
        }
    }
    else
    {
#ifdef HAVE_SSL
		gchar *tmp_str1;
		gchar *tmp_str2;
		GrisbiWinRun *w_run;
#endif

		/* Loading failed. */
		grisbi_win_status_bar_message (_("Failed to load accounts"));

#ifdef HAVE_SSL
		w_run = grisbi_win_get_w_run ();
        if (w_run->old_version)
        {
            dialogue_error_hint (_("The version of your file is less than 0.6. "
                                   "This file can not be imported by Grisbi."),
                                 _("Version of Grisbi file too old:"));
            grisbi_win_status_bar_stop_wait (TRUE);
			w_run->file_modification = 0;
			w_run->file_backup_saved = 0;

            return FALSE;
        }

		tmp_str1 = g_strdup_printf (_("Error loading file '%s'"), filename);

        if (a_conf->sauvegarde_demarrage || a_conf->sauvegarde_fermeture || a_conf->make_backup_every_minutes)
        {
            tmp_str2 = g_strdup_printf (_("Grisbi was unable to load file. You should find the last "
										  "backups in '%s', they are saved with date and time into "
										  "their name so you should find easily the last backup "
										  "saved.\n"
										  "Please contact the Grisbi's team on devel@listes.grisbi.org "
										  "to find what happened to you current file."),
										gsb_file_get_backup_path ());
        }
        else
        {
            if (gsb_file_get_backup_path ())
                tmp_str2 = g_strdup_printf ( _("Grisbi was unable to load file and the backups seem not to "
											   "be activated... This is a bad thing.\nYour backup path is '%s', "
											   "try to find if earlier you had some backups in there ?\n"
											   "Please contact the Grisbi's team on devel@listes.grisbi.org "
											   "to find what happened to you current file."),
											gsb_file_get_backup_path ());
            else
                tmp_str2 = my_strdup (_("Grisbi was unable to load file and the backups seem not "
										"to be activated... This is a bad thing.\n"
										"Please contact the Grisbi's team on "
										"devel@listes.grisbi.org to find what happened to you "
										"current file."));
		}
		dialogue_error_hint (tmp_str2, tmp_str1);
		g_free (tmp_str1);
		g_free (tmp_str2);
		w_run->file_modification = 0;
		w_run->file_backup_saved = 0;
#endif
		grisbi_win_status_bar_stop_wait (TRUE);
		grisbi_win_stack_box_show (NULL, "accueil_page");

		return FALSE;
    }

    /* ok, here the file or backup is loaded */
    grisbi_win_status_bar_message (_("Checking schedulers"));

	/* create the archives store data, ie the transaction which will replace the archive in
     * the list of transactions */
    gsb_data_archive_store_create_list ();

    /* create all the gui */
    grisbi_win_new_file_gui ();

	/* check the amounts of all the accounts */
    grisbi_win_status_bar_message (_("Checking amounts"));
	gsb_data_account_set_all_limits_of_balance ();

    /* set Grisbi title */
    grisbi_win_set_window_title (-1);

    /* for now, the flag for modification of the file is ok, but the menu couldn't be set
     * as sensitive/unsensitive so do it now */
    gsb_file_set_modified (gsb_file_get_modified (FALSE));

    grisbi_win_status_bar_message (_("Done"));
    grisbi_win_status_bar_stop_wait (TRUE);

    /* go to the home page */
    gsb_gui_navigation_set_selection (GSB_HOME_PAGE, -1, 0);

    /* set the focus to the selection tree at left */
    gtk_widget_grab_focus (gsb_gui_navigation_get_tree_view ());

    /* Si sauvegarde automatique on la lance ici */
    if (a_conf->make_backup_every_minutes
        && a_conf->make_backup_nb_minutes)
        gsb_file_automatic_backup_start (NULL, NULL);

    return TRUE;
}

/**
 * Perform the "Save" feature in menu
 *
 * \return TRUE on success.  FALSE otherwise.
 */
gboolean gsb_file_save (void)
{
    return gsb_file_save_file (0);
}

/**
 * Perform the "Save as" feature in menu
 *
 * \return TRUE on success.  FALSE otherwise.
 */
gboolean gsb_file_save_as (void)
{
    return gsb_file_save_file (-2);
}

/**
 * called when the user select the backup every x minutes
 *
 * \param checkbutton
 * \param null
 *
 * \return FALSE
 **/
gboolean gsb_file_automatic_backup_start (GtkWidget *checkbutton,
										  gpointer null)
{
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();
	devel_debug_int (a_conf->make_backup_every_minutes);

    /* if there is already a timeout, we remove it */
    if (id_timeout)
    {
		g_source_remove (id_timeout);
		id_timeout = 0;
    }

    /* launch the timeout only if active and if there is some minutes */
    if (a_conf->make_backup_every_minutes && a_conf->make_backup_nb_minutes)
		id_timeout = g_timeout_add_seconds (a_conf->make_backup_nb_minutes * 60,
											gsb_file_automatic_backup,
											a_conf);
    return FALSE;
}

/**
 * called when the user change the interval of automatic backup
 *
 * \param spinbutton
 * \param null
 *
 * \return FALSE
 **/
gboolean gsb_file_automatic_backup_change_time (GtkWidget *spinbutton,
												gpointer null)
{
	GrisbiAppConf *a_conf;

	a_conf = grisbi_app_get_a_conf ();
	devel_debug_int (a_conf->make_backup_every_minutes);

    /* if there is already a timeout, we stop it */
    if (id_timeout)
    {
	g_source_remove (id_timeout);
	id_timeout = 0;
    }

    /* set a new timeout only if there is an interval */
    if (a_conf->make_backup_nb_minutes)
	id_timeout = g_timeout_add_seconds (a_conf->make_backup_nb_minutes * 60,
										gsb_file_automatic_backup,
										a_conf);

    return FALSE;
}

/**
 * close the file
 * if no file loaded or no change, directly return TRUE
 *
 * \param
 *
 * \return FALSE if problem, TRUE if ok
 **/
gboolean gsb_file_close (void)
{
	gchar *filename;
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
    if (!assert_account_loaded ())
	{
        return TRUE;
	}

	a_conf = grisbi_app_get_a_conf ();

	/* on récupère le nom du fichier */
	filename = g_strdup (grisbi_win_get_filename (NULL));

	if (gsb_file_get_modified (FALSE))
    {
        /* try to save */
	    if (!gsb_file_save_file (-1))
            return FALSE;
    }
    else if (a_conf->sauvegarde_auto && filename)
    {
        /* try to save */
	    if (!gsb_file_save_file (-1))
            return FALSE;
    }

    if (!gsb_file_get_modified (FALSE))
    {
		GrisbiWinEtat *w_etat;

		/* remove the lock */
		w_etat = grisbi_win_get_w_etat ();
	    if (!w_etat->fichier_deja_ouvert
			&& gsb_data_account_get_number_of_accounts ()
			&& filename)
		{
			gsb_file_util_modify_lock (filename, FALSE);
		}

	    /* free all the variables */
		etats_gtktable_free_table_etat (); /* set table_etat = NULL: fix crash loading a multiple accounts files */
 	    init_variables ();
        grisbi_win_free_general_vbox ();
		grisbi_win_set_filename (NULL, NULL);

        grisbi_win_set_window_title (-1);

        /* unsensitive the necessaries menus */
        gsb_menu_set_menus_with_file_sensitive (FALSE);
        grisbi_win_menu_move_to_acc_delete ();

		g_free (filename);
	    return TRUE;
    }

    return FALSE;
}

/**
 * set or unset the modified flag (run structure)
 * and sensitive or not the menu to save the file
 *
 * \param modif TRUE to set the modified flag, FALSE to unset
 *
 * \return
 */
void gsb_file_set_modified (gboolean modified)
{
	GrisbiWinRun *w_run;

	/*     devel_debug_int (modified);  */

    /* If no file is loaded, do not change menu items. */
	if (!gsb_data_account_get_number_of_accounts ())
		return;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (modified)
    {
		/* modification pour gerer la non modification par la recherche dans la liste des operations */
		w_run->file_modification = time (NULL);
        gsb_menu_gui_sensitive_win_menu_item ("save", TRUE);
    }
    else
    {
        w_run->file_modification = 0;
        gsb_menu_gui_sensitive_win_menu_item ("save", FALSE);
    }
}

/**
 * Tell if the current file has been modified or not
 *
 * \param for_backup TRUE if we want to know if the flle has been modified since the latest backup
 *
 * \return TRUE if modified, FALSE otherwise
 */
gboolean gsb_file_get_modified (gboolean for_backup)
{
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (w_run->file_modification == 0)
        return FALSE;
    else
    {
        /* backup already done? */
        if (for_backup)
        {
            if (w_run->file_backup_saved > w_run->file_modification)
                return FALSE;
            else
                /* set time of backup */
                w_run->file_backup_saved = time(NULL);
        }
        return TRUE;
    }
}

/**
 * free the last_path path
 *
 * \param
 *
 * \return
 **/
void gsb_file_free_last_path (void)
{
	if (last_path_used)
    {
		g_free (last_path_used);
		last_path_used = NULL;
    }
}

/**
 * free the backup path
 *
 * \param
 *
 * \return
 **/
void gsb_file_free_backup_path (void)
{
	if (backup_path)
    {
		g_free (backup_path);
		backup_path = NULL;
    }
}

/**
 * copy an old grisbi file
 *
 * \param filename the name of the file
 *
 * \return
 **/
void gsb_file_copy_old_file (const gchar *filename)
{
    if (g_str_has_suffix (filename, ".gsb"))
    {
        GFile *file_ori;
        GFile *file_copy;
        GError *error = NULL;
		gchar *copy_old_filename;
		gchar *tmp_str;

        copy_old_filename = g_path_get_basename (filename);
		tmp_str = copy_old_filename;
        copy_old_filename = gsb_string_remplace_string (copy_old_filename, ".gsb", "-old-version.gsb");
		g_free (tmp_str);
		tmp_str = copy_old_filename;
        copy_old_filename = g_build_filename (gsb_dirs_get_user_data_dir (), copy_old_filename, NULL);
		g_free (tmp_str);

		file_ori = g_file_new_for_path (filename);
        file_copy = g_file_new_for_path (copy_old_filename);

		if (!g_file_copy (file_ori, file_copy, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error))
        {
            dialogue_error (error->message);
            g_error_free (error);
        }
		else
		{
			tmp_str = g_strdup_printf (_("The original file was saved saved as:\n"
										 "\"%s\"."),
									   copy_old_filename);

			dialogue_warning (tmp_str);
			g_free (tmp_str);
		}
		g_free (copy_old_filename);
		g_object_unref (file_ori);
		g_object_unref (file_copy);
    }
}
/**
 * Fonction de fermeture de grisbi sans appeler la fonction init_variables ().
 * Demandera une clarification de la libération des variables.
 *
 * \param
 *
 * \return
 **/
gboolean gsb_file_quit (void)
{
	gchar *filename;
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
    if (!assert_account_loaded ())
	{
        return TRUE;
	}

	a_conf = grisbi_app_get_a_conf ();

	/* on récupère le nom du fichier */
	filename = g_strdup (grisbi_win_get_filename (NULL));

	if (gsb_file_get_modified (FALSE))
    {
        /* try to save */
	    if (!gsb_file_save_file (-1))
            return FALSE;
    }
    else if (a_conf->sauvegarde_auto && filename)
    {
        /* try to save */
	    if (!gsb_file_save_file (-1))
            return FALSE;
    }

    if (!gsb_file_get_modified (FALSE))
    {
		GrisbiWinEtat *w_etat;

		/* remove the lock */
		w_etat = grisbi_win_get_w_etat ();
	    if (!w_etat->fichier_deja_ouvert
			&&
			gsb_data_account_get_number_of_accounts ()
			&&
			filename)
		{
			gsb_file_util_modify_lock (filename, FALSE);
		}

	    /* free all the variables */
		etats_gtktable_free_table_etat (); /* set table_etat = NULL: fix crash loading a multiple accounts files */
        grisbi_win_free_general_vbox ();
		grisbi_win_set_filename (NULL, NULL);

        grisbi_win_set_window_title (-1);
        grisbi_win_menu_move_to_acc_delete ();

		g_free (filename);
	    return TRUE;
    }

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_file_remove_account_file (const gchar *filename)
{
	gchar *tmp_str;
	gint result;

	devel_debug (filename);

	tmp_str = g_strdup_printf (_("Are you sure you want to remove this accounts file \"%s\"?\n"),
							   filename);
	result = dialogue_yes_no (tmp_str, _("Confirmation of accounts file removal"), GTK_RESPONSE_NO);

	if (result)
	{
		utils_files_remove_name_to_recent_array (filename);
		g_remove (filename);
	}

	g_free (tmp_str);
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
