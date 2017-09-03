/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <time.h>
#include <limits.h>
#include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

#ifdef G_OS_WIN32
#include <stdlib.h> // For realpath()
#endif /* WIN32 */

/*START_INCLUDE*/
#include "gsb_file.h"
#include "accueil.h"
#include "affichage_liste.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_account_property.h"
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
#include "parametres.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_file_selection.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/**
 * keep the last path used in grisbi, save in the configuration at the end */
static gchar *last_path_used;

static gchar *backup_path;

/** the timeout used to save a backup every x minutes */
gint id_timeout = 0;

/*START_EXTERN*/
extern GtkWidget *table_etat;
extern GtkWidget *tree_view_vbox;
/*END_EXTERN*/

/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
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
    gchar *new_name;
    GtkWidget *dialog;
    gint result;
    gchar *tmp_last_directory;

    dialog = gtk_file_chooser_dialog_new (_("Name the accounts file"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_FILE_CHOOSER_ACTION_SAVE,
										  "gtk-cancel", GTK_RESPONSE_CANCEL,
										  "gtk-save", GTK_RESPONSE_OK,
										  NULL);

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

    result = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (result)
    {
	case GTK_RESPONSE_OK :
	    new_name = file_selection_get_filename (GTK_FILE_CHOOSER (dialog));
        tmp_last_directory = file_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE);
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
static gint gsb_file_dialog_save (const gchar *filename)
{
    gchar * hint;
    gchar* time_elapsed;
    time_t now = time (NULL);
    gint result;
    GtkWidget *dialog;
    gint difference = (gint) difftime (now, run.file_modification);
    gchar* message;
	gchar* tmp_str1;
	gchar* tmp_str2;

    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_WARNING,
									 GTK_BUTTONS_NONE,
									 " ");

    /*     si le fichier était déjà locké et que force enregistrement n'est pas mis, */
    /*     on prévient ici */
    if (etat.fichier_deja_ouvert && !conf.force_enregistrement)
    {
        hint = g_strdup(_("Save locked files?"));
        message = g_strdup_printf (_("The document '%s' is locked but modified. "
                                     "If you want to save it, you must cancel and "
                                     "save it with another name or activate the "
                                     "\"%s\" option in setup."),
                                   (filename ? g_path_get_basename(filename) : _("unnamed")),
                                   _("Force saving of locked files"));

        gtk_dialog_add_buttons (GTK_DIALOG(dialog),
                                _("Close without saving"),
                                GTK_RESPONSE_NO,
                                "gtk-cancel",
                                GTK_RESPONSE_CANCEL,
                                NULL);
        gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
    }
    else
    {
        hint = g_strdup_printf (_("Save changes to document '%s' before closing?"),
                                (filename ? g_path_get_basename(filename) : _("unnamed")));
        message = g_strdup("");

        gtk_dialog_add_buttons (GTK_DIALOG(dialog),
                                _("Close without saving"), GTK_RESPONSE_NO,
                                "gtk-cancel", GTK_RESPONSE_CANCEL,
                                "gtk-save", GTK_RESPONSE_OK,
                                NULL);
        gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    }

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
    tmp_str1 = message;
    tmp_str2 = g_strdup_printf (_("If you close without saving, all of your changes "
                                 "since %s will be discarded."),
								time_elapsed);
    message = g_strconcat (tmp_str1, tmp_str2 , NULL);
    g_free (tmp_str1);
    g_free (tmp_str2);
    g_free (time_elapsed);

    g_object_set (G_OBJECT (dialog), "text", hint, "secondary-text", message, NULL);

    g_free (message);
    g_free (hint);

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
static gboolean gsb_file_save_backup (void)
{
    gboolean retour;
	gchar *new_filename;
    const gchar *filename;
    struct tm *day_time;
    time_t temps;
    gchar *name;

    if (!gsb_file_get_backup_path () || !gsb_file_get_modified ())
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
    if (conf.make_bakup_single_file)
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

    retour = gsb_file_save_save_file (new_filename, conf.compress_backup, FALSE);

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
static gboolean gsb_file_automatic_backup (gpointer null)
{
    devel_debug (NULL);

    if (!conf.make_backup_every_minutes)
	/* stop the timeout */
        return FALSE;

    /* we save only if there is a nb of minutes, but don't stop the timer if not */
    if (conf.make_backup_nb_minutes)
        gsb_file_save_backup ();

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
	const gchar *filename;

    devel_debug_int (origine);
	/* on récupère le nom du fichier */
	filename = grisbi_win_get_filename (NULL);

    /* on commence par demander si on sauvegarde ou pas */
    if (!conf.sauvegarde_auto)
    {
        result = gsb_file_dialog_save (filename);
        if (result == GTK_RESPONSE_NO)
            return (TRUE);
    }

    if ((!gsb_file_get_modified () && origine != -2)
        ||
        !gsb_data_account_get_accounts_amount ())
    {
        notice_debug ("nothing done in gsb_file_save_file");
        return (TRUE);
    }

    /* si le fichier de comptes n'a pas de nom ou si on enregistre sous un nouveau nom */
    /*     c'est ici */
    if (!filename || origine == -2)
        nouveau_nom_enregistrement = gsb_file_dialog_ask_name (filename);
    else
        nouveau_nom_enregistrement = g_strdup (filename);

    if (!nouveau_nom_enregistrement)
        return FALSE;

    /*     on vérifie que le fichier n'est pas locké */
    if (etat.fichier_deja_ouvert && !conf.force_enregistrement && origine != -2)
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
		g_free (nouveau_nom_enregistrement);

        return (FALSE);
    }

    /* make backup before saving if asked */
    if (conf.make_backup)
        gsb_file_save_backup();

    /*   on a maintenant un nom de fichier */
    /*     et on sait qu'on peut sauvegarder */
    grisbi_win_status_bar_message (_("Saving file"));

    result = gsb_file_save_save_file (nouveau_nom_enregistrement, conf.compress_file, FALSE);

    if (result)
    {
		grisbi_win_set_filename (NULL, nouveau_nom_enregistrement);

		/* saving was right, so unlock the last name */
        gsb_file_util_modify_lock (filename, FALSE);

        /* and lock the new name */
        gsb_file_util_modify_lock (nouveau_nom_enregistrement, TRUE);

        /* update variables */
        etat.fichier_deja_ouvert = 0;
        gsb_file_set_modified (FALSE);
        grisbi_win_set_window_title (gsb_gui_navigation_get_current_account ());
    }

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
    /* create the first account */
    if (!gsb_assistant_account_run ())
    {
        init_variables ();
        return FALSE;
    }

    /* init the gui */
    gsb_file_new_gui ();

    mise_a_jour_accueil (TRUE);
    gsb_gui_navigation_set_selection (GSB_HOME_PAGE, -1, 0);

    gsb_file_set_modified (TRUE);
    return FALSE;
}

/**
 * Initialize user interface part when a new accounts file is created.
 *
 * \param
 *
 * \return
 **/
void gsb_file_new_gui (void)
{
    GrisbiWin *win;
    GtkWidget *tree_view_widget;
    GtkWidget *notebook_general;

    win = grisbi_app_get_active_window (NULL);

    /* dégrise les menus nécessaire */
    gsb_menu_set_menus_with_file_sensitive (TRUE);

    /* récupère l'organisation des colonnes */
    recuperation_noms_colonnes_et_tips ();

    /* Create main widget. */
    grisbi_win_status_bar_message (_("Creating main window"));

    /* création de grid_general */
    grisbi_win_create_general_widgets (GRISBI_WIN (win));
	grisbi_win_stack_box_show (win, "file_page");

    /* fill the general notebook */
    notebook_general = grisbi_win_get_notebook_general (win);
    gsb_gui_fill_general_notebook (notebook_general);

    /* create the model */
    if (!transaction_list_create ())
    {
		dialogue_error (_("The model of the list couldn't be created... "
						  "Bad things will happen very soon..."));
		return;
    }

    /* Create transaction list. */
    tree_view_widget = gsb_transactions_list_make_gui_list ();
    gtk_box_pack_start (GTK_BOX (tree_view_vbox), tree_view_widget, TRUE, TRUE, 0);
    gtk_widget_show (tree_view_widget);

    navigation_change_account (gsb_gui_navigation_get_current_account ());

    /* Display accounts in menus */
	grisbi_win_menu_move_to_acc_delete ();
	grisbi_win_menu_move_to_acc_new ();

    notebook_general = gsb_gui_get_general_notebook ();
    gtk_notebook_set_current_page (GTK_NOTEBOOK(notebook_general), GSB_HOME_PAGE);

    gtk_widget_show (notebook_general);
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
    GtkFileFilter * filter;
    gboolean result = FALSE;
    gchar *tmp_last_directory;
	gchar *filename;

    selection_fichier = gtk_file_chooser_dialog_new (_("Open an accounts file"),
												     GTK_WINDOW (grisbi_app_get_active_window (NULL)),
												     GTK_FILE_CHOOSER_ACTION_OPEN,
												     "gtk-cancel", GTK_RESPONSE_CANCEL,
												     "gtk-open", GTK_RESPONSE_OK,
												     NULL);

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

    switch (gtk_dialog_run (GTK_DIALOG (selection_fichier)))
    {
		case GTK_RESPONSE_OK:
			if (gsb_file_close())
			{
				gtk_widget_hide (selection_fichier);
				filename = file_selection_get_filename (GTK_FILE_CHOOSER (selection_fichier));

				tmp_last_directory = file_selection_get_last_directory (GTK_FILE_CHOOSER (selection_fichier), TRUE);
				gsb_file_update_last_path (tmp_last_directory);
				g_free (tmp_last_directory);
				result = gsb_file_open_file (filename);
				g_free (filename);
			}
			break;
		  default:
			break;
    }

    tmp_last_directory = file_selection_get_last_directory (GTK_FILE_CHOOSER (selection_fichier), TRUE);
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
    GSettings *settings;

    devel_debug (last_path);

    if (last_path && strlen (last_path))
    {
        if (last_path_used)
            g_free (last_path_used);
        last_path_used = my_strdup (last_path);
        settings = grisbi_settings_get_settings (SETTINGS_GENERAL);
        g_settings_set_string (G_SETTINGS (settings), "last-path", last_path_used);
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

    if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
        utils_files_create_XDG_dir ();
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
    GSList *list_tmp;

    devel_debug (filename);

	if (!gsb_file_test_file (filename))
		return FALSE;

    grisbi_win_status_bar_wait (TRUE);
    grisbi_win_status_bar_message (_("Loading accounts"));

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
        if (conf.sauvegarde_demarrage)
        {
            gsb_file_save_backup ();
            gsb_file_set_modified (FALSE);
        }
    }
    else
    {
		gchar *tmp_str1;
		gchar *tmp_str2;

        if (run.old_version)
        {
            dialogue_error_hint (_("The version of your file is less than 0.6. "
                                   "This file can not be imported by Grisbi."),
                                 _("Version of Grisbi file too old :"));
            grisbi_win_status_bar_stop_wait (TRUE);

            return FALSE;
        }

        /* Loading failed. */
        grisbi_win_status_bar_message (_("Failed to load accounts"));

		tmp_str1 = g_strdup_printf (_("Error loading file '%s'"), filename);

        if (conf.sauvegarde_demarrage || conf.make_backup || conf.make_backup_every_minutes)
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

		grisbi_win_status_bar_stop_wait (TRUE);

		return FALSE;
    }

    /* ok, here the file or backup is loaded */
    grisbi_win_status_bar_message (_("Checking schedulers"));

	/* create the archives store data, ie the transaction wich will replace the archive in
     * the list of transactions */
    gsb_data_archive_store_create_list ();

    /* create all the gui */
    gsb_file_new_gui ();

	/* check the amounts of all the accounts */
    grisbi_win_status_bar_message (_("Checking amounts"));
    list_tmp = gsb_data_account_get_list_accounts ();

    while (list_tmp)
    {
		gint account_number;
		volatile gint value;

		account_number = gsb_data_account_get_no_account (list_tmp->data);

		/* set the minimum balances to be shown or not */
		value = gsb_real_cmp (gsb_data_account_get_current_balance (account_number),
							  gsb_data_account_get_mini_balance_authorized (account_number)) == -1;
		gsb_data_account_set_mini_balance_authorized_message (account_number, value);
		value = gsb_real_cmp (gsb_data_account_get_current_balance (account_number),
							  gsb_data_account_get_mini_balance_wanted (account_number)) == -1;
		gsb_data_account_set_mini_balance_wanted_message (account_number, value);

		list_tmp = list_tmp->next;
    }

    /* set Grisbi title */
    grisbi_win_set_window_title (-1);

    /* update the main page */
    mise_a_jour_accueil (TRUE);

    /* for now, the flag for modification of the file is ok, but the menu couldn't be set
     * as sensitive/unsensitive so do it now */
    gsb_file_set_modified (gsb_file_get_modified ());

    grisbi_win_status_bar_message (_("Done"));
    grisbi_win_status_bar_stop_wait (TRUE);

    /* go to the home page */
    gsb_gui_navigation_set_selection (GSB_HOME_PAGE, -1, 0);

    /* set the focus to the selection tree at left */
    gtk_widget_grab_focus (gsb_gui_navigation_get_tree_view ());

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
    devel_debug_int (conf.make_backup_every_minutes);

    /* if there is already a timeout, we remove it */
    if (id_timeout)
    {
		g_source_remove (id_timeout);
		id_timeout = 0;
    }

    /* launch the timeout only if active and if there is some minutes */
    if (conf.make_backup_every_minutes && conf.make_backup_nb_minutes)
		id_timeout = g_timeout_add_seconds (conf.make_backup_nb_minutes * 60,
											(GSourceFunc) (gsb_file_automatic_backup),
											NULL);
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
    devel_debug_int (conf.make_backup_nb_minutes);

    /* if there is already a timeout, we stop it */
    if (id_timeout)
    {
	g_source_remove (id_timeout);
	id_timeout = 0;
    }

    /* set a new timeout only if there is an interval */
    if (conf.make_backup_nb_minutes)
	id_timeout = g_timeout_add_seconds (conf.make_backup_nb_minutes * 60,
					     (GSourceFunc) (gsb_file_automatic_backup),
					     NULL);

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
	const gchar *filename;
    devel_debug (NULL);

    if (!assert_account_loaded ())
	{
        return TRUE;
	}

	/* on récupère le nom du fichier */
	filename = grisbi_win_get_filename (NULL);

	if (gsb_file_get_modified ())
    {
        /* try to save */
	    if (!gsb_file_save_file (-1))
            return FALSE;
    }
    else if (conf.sauvegarde_auto
        && (!etat.fichier_deja_ouvert || conf.force_enregistrement)
        && filename)
    {
        /* try to save */
	    if (!gsb_file_save_file (-1))
            return FALSE;
    }

    if (!gsb_file_get_modified ())
    {
	     /* remove the lock */
	    if (!etat.fichier_deja_ouvert
			&&
			gsb_data_account_get_accounts_amount ()
			&&
			filename)
		{
			gsb_file_util_modify_lock (filename, FALSE);
		}

	    /* free all the variables */
 	    init_variables ();
        grisbi_win_free_general_vbox ();
        gsb_account_property_clear_config ();
		grisbi_win_set_filename (NULL, NULL);

        grisbi_win_set_window_title (-1);

        /* unsensitive the necessaries menus */
        gsb_menu_set_menus_with_file_sensitive (FALSE);
        grisbi_win_menu_move_to_acc_delete ();

        table_etat = NULL;

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
/*     devel_debug_int (modified);  */

    /* If no file is loaded, do not change menu items. */
    if (!gsb_data_account_get_accounts_amount ())
        return;

    if (modified)
    {
        if (!run.file_modification)
        {
            run.file_modification = time (NULL);
            gsb_menu_gui_sensitive_win_menu_item ("save", TRUE);
        }
    }
    else
    {
        run.file_modification = 0;
        gsb_menu_gui_sensitive_win_menu_item ("save", FALSE);
    }
}

/**
 * Tell if the current file has been modified or not
 *
 * \return TRUE if modified, FALSE otherwise
 */
gboolean gsb_file_get_modified (void)
{
    if (run.file_modification == 0)
        return FALSE;
    else
        return TRUE;
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
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
