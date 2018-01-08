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

#include "include.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_settings.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils_buttons.h"
/*
#include "fenetre_principale.h"
#include "menu.h"
#include "utils_files.h"
*/
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/* START_EXTERN Variables externes PROVISOIRE */
extern struct ConditionalMessage delete_msg[];
extern struct ConditionalMessage messages[];
/*END_EXTERN*/

struct _GrisbiSettingsPrivate
{
    GSettings *settings_root;
    GSettings *settings_backup;
    GSettings *settings_display;
    GSettings *settings_file;
    GSettings *settings_form;
    GSettings *settings_general;
    GSettings *settings_geometry;
    GSettings *settings_messages_delete;
    GSettings *settings_messages_tips;
    GSettings *settings_messages_warnings;
    GSettings *settings_panel;
    GSettings *settings_prefs;
    GSettings *settings_scheduled;
};

G_DEFINE_TYPE_WITH_PRIVATE (GrisbiSettings, grisbi_settings, G_TYPE_OBJECT);

/* singleton object - all consumers of GrisbiSettings get the same object (refcounted) */
static GrisbiSettings *singleton = NULL;


/*******************************************************************************
 * Private Methods
 ******************************************************************************/
/**
 * set root settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_root (GSettings *settings)
{
    /* first use */
    conf.first_use = g_settings_get_boolean (settings, "first-use");

    /* Force classic menu : FALSE par défaut */
    conf.force_classic_menu = g_settings_get_boolean (settings, "force-classic-menu");
}

/**
 * set backup settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_backup (GSettings *settings)
{
    gchar *tmp_path;

    /* on fixe le backup path s'il n'est pas initialisé */
    tmp_path = g_settings_get_string (settings, "backup-path");
    if (tmp_path == NULL || strlen (tmp_path) == 0)
    {
        tmp_path = g_strdup (gsb_dirs_get_user_data_dir ());
        g_settings_set_string (settings, "backup-path", tmp_path);
    }
    gsb_file_set_backup_path (tmp_path);
    g_free (tmp_path);

    conf.compress_backup = g_settings_get_boolean (settings, "compress-backup");
    conf.make_bakup_single_file = g_settings_get_boolean (settings, "make-backup-single-file");
    conf.make_backup = g_settings_get_boolean (settings, "sauvegarde-fermeture");
    conf.make_backup_every_minutes = g_settings_get_boolean (settings, "make-backup-every-minutes");
    conf.make_backup_nb_minutes = g_settings_get_int (settings, "make-backup-nb-minutes");
    conf.sauvegarde_demarrage = g_settings_get_boolean (settings, "sauvegarde-ouverture");

    /* Si sauvegarde automatique on la lance ici */
    if (conf.make_backup_every_minutes
     &&
     conf.make_backup_nb_minutes)
        gsb_file_automatic_backup_start (NULL, NULL);
}

/**
 * set display settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_display (GSettings *settings)
{
    gchar *tmp_str;

    tmp_str = g_settings_get_string (settings, "display-window-title");
    if (g_strcmp0 (tmp_str, "Holder name") == 0)
        conf.display_window_title = 1;
    else if (g_strcmp0 (tmp_str, "Filename") == 0)
        conf.display_window_title = 2;
    else
        conf.display_window_title = 0;
    g_free (tmp_str);

    tmp_str = g_settings_get_string (settings, "display-toolbar");
    if (g_strcmp0 (tmp_str, "Text") == 0)
        conf.display_toolbar = GSB_BUTTON_TEXT;
    else if (g_strcmp0 (tmp_str, "Icons") == 0)
        conf.display_toolbar = GSB_BUTTON_ICON;
    else
        conf.display_toolbar = GSB_BUTTON_BOTH;
    g_free (tmp_str);

    conf.formulaire_toujours_affiche = g_settings_get_boolean (settings, "formulaire-toujours-affiche");
    conf.group_partial_balance_under_accounts = g_settings_get_boolean (settings, "group-partial-balance-under-accounts");
    conf.show_closed_accounts = g_settings_get_boolean (settings, "show-closed-accounts");
    conf.show_headings_bar = g_settings_get_boolean (settings, "show-headings-bar");
}

/**
 * set file settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_file (GSettings *settings)
{
	gchar **recent_array;
	gchar *import_directory;

    conf.compress_file = g_settings_get_boolean (settings, "compress-file");
    conf.dernier_fichier_auto = g_settings_get_boolean (settings, "dernier-fichier-auto");
    conf.force_enregistrement = g_settings_get_boolean (settings, "force-enregistrement");
    conf.force_import_directory = g_settings_get_boolean (settings, "force-import-directory");
    conf.nb_max_derniers_fichiers_ouverts = g_settings_get_int (settings, "nb-max-derniers-fichiers-ouverts");
    conf.sauvegarde_auto = g_settings_get_boolean (settings, "sauvegarde-auto");

	if (conf.force_import_directory)
	{
		import_directory = g_settings_get_string (settings, "import-directory");
		if (import_directory && strlen (import_directory) > 0)
		{
			conf.import_directory = my_strdup (import_directory);
		}
		else
		{
			conf.import_directory = my_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOWNLOAD));
		}
		g_free (import_directory);
	}
	recent_array = g_settings_get_strv (settings, "names-last-files");
    if (recent_array)
	{
        conf.nb_derniers_fichiers_ouverts = g_strv_length (recent_array);
        if (conf.nb_derniers_fichiers_ouverts > 0)
        {
			if (conf.nb_derniers_fichiers_ouverts > conf.nb_max_derniers_fichiers_ouverts)
			{
				conf.nb_derniers_fichiers_ouverts = conf.nb_max_derniers_fichiers_ouverts;
			}

            conf.last_open_file = my_strdup (recent_array[0]);
        }
    }

	grisbi_app_set_recent_files_array (recent_array);

    /* archive stuff */
    conf.archives_check_auto = g_settings_get_boolean (settings, "archives-check-auto");
    conf.max_non_archived_transactions_for_check = g_settings_get_int (settings, "max-transactions-before-warn-archival");
}

/**
 * set form settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_form (GSettings *settings)
{
    conf.affichage_exercice_automatique = g_settings_get_boolean (settings, "affichage-exercice-automatique");
    conf.automatic_completion_payee = g_settings_get_boolean (settings, "automatic-completion-payee");
    conf.automatic_erase_credit_debit = g_settings_get_boolean (settings, "automatic-erase-credit-debit");
    conf.automatic_recover_splits = g_settings_get_boolean (settings, "automatic-recover-splits");
    conf.entree = g_settings_get_boolean (settings, "form-enter-key");
    conf.form_expander_label_width = g_settings_get_int (settings, "form-expander-label-width");
	conf.fyear_combobox_sort_order = g_settings_get_int (settings, "fyear-combobox-sort-order");
    conf.limit_completion_to_current_account = g_settings_get_boolean (settings, "limit-completion-current-account");
}

/**
 * set general settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_general (GSettings *settings)
{
    gchar *tmp_str;

    conf.custom_fonte_listes = g_settings_get_boolean (settings, "custom-fonte-listes");
    if (conf.custom_fonte_listes)
    {
        conf.font_string = g_settings_get_string (settings, "font-string");
		if (strlen (conf.font_string) == 0)
		{
			conf.font_string = my_strdup (_("No font defined"));
			conf.custom_fonte_listes = FALSE;
		}
    }

    tmp_str = g_settings_get_string (settings, "browser-command");
    if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        g_settings_set_string (settings, "browser-command", ETAT_WWW_BROWSER);
        conf.browser_command = g_strdup (ETAT_WWW_BROWSER);
    }
    else
    {
        conf.browser_command = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    tmp_str = g_settings_get_string (settings, "language-chosen");
    if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        conf.language_chosen = NULL;
    }
    else
    {
        conf.language_chosen = g_strdup (tmp_str);
        g_free (tmp_str);
    }

    tmp_str = g_settings_get_string (settings, "last-path");
    if (tmp_str == NULL || strlen (tmp_str) == 0)
    {
        tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
        g_settings_set_string (settings, "last-path", tmp_str);
    }
	else
	{
		if (!g_file_test (tmp_str, G_FILE_TEST_EXISTS))
		{
			g_free (tmp_str);
			tmp_str = g_strdup (g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS));
			g_settings_set_string (settings, "last-path", tmp_str);
		}
	}

	gsb_file_init_last_path (tmp_str);
    g_free (tmp_str);

    /* Home page */
    conf.pluriel_final = g_settings_get_boolean (settings, "pluriel-final");

    /* variables for the list of categories */
    tmp_str = g_settings_get_string (settings, "metatree-action-2button-press");
    if (g_strcmp0 (tmp_str, "Edit Category") == 0)
        conf.metatree_action_2button_press = 1;
    else if (g_strcmp0 (tmp_str, "Manage division") == 0)
        conf.metatree_action_2button_press = 2;
    else
        conf.metatree_action_2button_press = 0;
    g_free (tmp_str);

    /* variables for the list of transactions */
    conf.show_transaction_gives_balance = g_settings_get_boolean (settings, "show-transaction-gives-balance");
    conf.show_transaction_selected_in_form = g_settings_get_boolean (settings, "show-transaction-selected-in-form");

    tmp_str = g_settings_get_string (settings, "transactions-list-primary-sorting");
    if (g_strcmp0 (tmp_str, "Sort by value date") == 0)
        conf.transactions_list_primary_sorting = 0;
    else
        conf.transactions_list_primary_sorting = 1;
    g_free (tmp_str);

    tmp_str = g_settings_get_string (settings, "transactions-list-secondary-sorting");
    if (g_strcmp0 (tmp_str, "Sort by transaction number") == 0)
        conf.transactions_list_secondary_sorting = 0;
    else if (g_strcmp0 (tmp_str, "Sort by type of amount") == 0)
        conf.transactions_list_secondary_sorting = 1;
    else if (g_strcmp0 (tmp_str, "Sort by payee name") == 0)
        conf.transactions_list_secondary_sorting = 2;
    else
        conf.transactions_list_secondary_sorting = 3;
    g_free (tmp_str);
}

/**
 * set geometry
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_geometry (GSettings *settings)
{
    /* fenetre : 1300x900 100+100 fullscreen et maximize FALSE par défaut */
    conf.x_position = g_settings_get_int (settings, "x-position");
    conf.y_position = g_settings_get_int (settings, "y-position");
    conf.main_height = g_settings_get_int (settings, "main-height");
    conf.main_width = g_settings_get_int (settings, "main-width");
    conf.full_screen = g_settings_get_boolean (settings, "fullscreen");
    conf.maximize_screen = g_settings_get_boolean (settings, "maximized");
}

/**
 * set messages delete settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_messages_delete (GSettings *settings)
{
    gint i;

    for (i = 0; delete_msg[i].name; i ++)
    {
        delete_msg[i].hidden = !g_settings_get_boolean (G_SETTINGS (settings),
                        delete_msg[i].name);
        if (delete_msg[i].hidden == 1)
            delete_msg[i].default_answer = 1;
    }
}

/**
 * set messages tips settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_messages_tips (GSettings *settings)
{
    conf.last_tip = g_settings_get_int (settings, "last-tip");
    conf.show_tip = g_settings_get_boolean (settings, "show-tip");
}

/**
 * set messages warnings settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_messages_warnings (GSettings *settings)
{
    gint i;

    for (i = 0; messages[i].name; i ++)
    {
        messages[i].hidden = !g_settings_get_boolean (G_SETTINGS (settings),
                        messages[i].name);
        if (messages[i].hidden == 1)
            messages[i].default_answer = 1;
    }
}

/**
 * set panel settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_panel (GSettings *settings)
{
    conf.active_scrolling_left_pane = g_settings_get_boolean (settings, "active-scrolling-left-pane");
    conf.panel_width = g_settings_get_int (settings, "panel-width");
}

/**
 * set prefs settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_prefs (GSettings *settings)
{
    conf.prefs_archives_sort_order = g_settings_get_int (settings, "prefs-archives-sort-order");
    conf.prefs_fyear_sort_order = g_settings_get_int (settings, "prefs-fyear-sort-order");
    conf.prefs_height = g_settings_get_int (settings, "prefs-height");
    conf.prefs_panel_width = g_settings_get_int (settings, "prefs-panel-width");
    conf.prefs_width = g_settings_get_int (settings, "prefs-width");
}

/**
 * set scheduled settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
static void grisbi_settings_init_settings_scheduled (GSettings *settings)
{
    conf.balances_with_scheduled = g_settings_get_boolean (settings, "balances-with-scheduled");
    conf.execute_scheduled_of_month = g_settings_get_boolean (settings, "execute-scheduled-of-month");
    conf.nb_days_before_scheduled = g_settings_get_int (settings, "nb-days-before-scheduled");
    conf.scheduler_set_fixed_day = g_settings_get_boolean (settings, "scheduler-set-fixed-day");
    conf.scheduler_fixed_day = g_settings_get_int (settings, "scheduler-fixed-day");
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_settings_init (GrisbiSettings *self)
{
    GrisbiSettingsPrivate *priv;

    devel_debug (NULL);

    priv = grisbi_settings_get_instance_private (self);

    priv->settings_root = g_settings_new ("org.gtk.grisbi");
    grisbi_settings_init_settings_root (priv->settings_root);

    priv->settings_backup = g_settings_new ("org.gtk.grisbi.files.backup");
    grisbi_settings_init_settings_backup (priv->settings_backup);

    priv->settings_display = g_settings_new ("org.gtk.grisbi.display");
    grisbi_settings_init_settings_display (priv->settings_display);

    priv->settings_file = g_settings_new ("org.gtk.grisbi.files.file");
    grisbi_settings_init_settings_file (priv->settings_file);

    priv->settings_form = g_settings_new ("org.gtk.grisbi.form");
    grisbi_settings_init_settings_form (priv->settings_form);

    priv->settings_general = g_settings_new ("org.gtk.grisbi.general");
    grisbi_settings_init_settings_general (priv->settings_general);

    priv->settings_geometry = g_settings_new ("org.gtk.grisbi.geometry");
    grisbi_settings_init_settings_geometry (priv->settings_geometry);

    priv->settings_messages_delete = g_settings_new ("org.gtk.grisbi.messages.delete");
    grisbi_settings_init_settings_messages_delete (priv->settings_messages_delete);

    priv->settings_messages_tips = g_settings_new ("org.gtk.grisbi.messages.tips");
    grisbi_settings_init_settings_messages_tips (priv->settings_messages_tips);

    priv->settings_messages_warnings = g_settings_new ("org.gtk.grisbi.messages.warnings");
    grisbi_settings_init_settings_messages_warnings (priv->settings_messages_warnings);

    priv->settings_panel = g_settings_new ("org.gtk.grisbi.panel");
    grisbi_settings_init_settings_panel (priv->settings_panel);

    priv->settings_prefs = g_settings_new ("org.gtk.grisbi.prefs");
    grisbi_settings_init_settings_prefs (priv->settings_prefs);

    priv->settings_scheduled = g_settings_new ("org.gtk.grisbi.scheduled");
    grisbi_settings_init_settings_scheduled (priv->settings_scheduled);
}


 /**
 *
 *
 * \param
 *
 * \return
 **/
static void dispose (GObject *object)
{
    GrisbiSettings *self = GRISBI_SETTINGS (object);
    GrisbiSettingsPrivate *priv;

    priv = grisbi_settings_get_instance_private (self);

    g_clear_object (&priv->settings_root);
    g_clear_object (&priv->settings_backup);
    g_clear_object (&priv->settings_display);
    g_clear_object (&priv->settings_file);
    g_clear_object (&priv->settings_form);
    g_clear_object (&priv->settings_general);
    g_clear_object (&priv->settings_geometry);
    g_clear_object (&priv->settings_messages_delete);
    g_clear_object (&priv->settings_messages_tips);
    g_clear_object (&priv->settings_messages_warnings);
    g_clear_object (&priv->settings_panel);
    g_clear_object (&priv->settings_prefs);
    g_clear_object (&priv->settings_scheduled);

    G_OBJECT_CLASS (grisbi_settings_parent_class)->dispose (object);
}


 /**
 *
 *
 * \param
 *
 * \return
 **/
static void finalize (GObject *object)
{
    singleton = NULL;

    /* Chain up to the parent class */
    G_OBJECT_CLASS (grisbi_settings_parent_class)->finalize (object);
}


 /**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_settings_class_init (GrisbiSettingsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->dispose = dispose;
        object_class->finalize = finalize;
}


/*******************************************************************************
 * Public Methods
 ******************************************************************************/
 /**
 *
 *
 * \param
 *
 * \return
 **/
GrisbiSettings *grisbi_settings_get (void)
{
    if (!singleton)
    {
        singleton = GRISBI_SETTINGS (g_object_new (GRISBI_TYPE_SETTINGS, NULL));
    }
    else
    {
        g_object_ref (singleton);
    }
    g_assert (singleton);

    return singleton;
}

 /**
 * FONCTION PROVISOIRE AVANT TRAITEMENT DES PREFERENCES
 *
 * \param
 *
 * \return
 **/
void grisbi_settings_save_app_config (void)
{
    GrisbiSettingsPrivate *priv;
	gchar **recent_array;
    gchar *tmp_str;
    gint i;

    devel_debug (NULL);

    priv = grisbi_settings_get_instance_private (grisbi_settings_get ());

    /* priv->settings_root */
    g_settings_set_boolean (G_SETTINGS (priv->settings_root),
                        "first-use",
                        conf.first_use);
    g_settings_set_boolean (G_SETTINGS (priv->settings_root),
                        "force-classic-menu",
                        conf.force_classic_menu);

    /* priv->settings_backup */
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "make-backup-single-file",
                        conf.make_bakup_single_file);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "compress-backup",
                        conf.compress_backup);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "sauvegarde-ouverture",
                        conf.sauvegarde_demarrage);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "sauvegarde-fermeture",
                        conf.make_backup);
    g_settings_set_boolean (G_SETTINGS (priv->settings_backup),
                        "make-backup-every-minutes",
                        conf.make_backup_every_minutes);
    g_settings_set_int (G_SETTINGS (priv->settings_backup),
                        "make-backup-nb-minutes",
                        conf.make_backup_nb_minutes);

    /* priv->settings_display */
    switch (conf.display_window_title)
    {
        case 1:
            tmp_str = "Holder name";
            break;
        case 2:
            tmp_str = "Filename";
            break;
        default:
            tmp_str = "Entity name";
    }
    g_settings_set_string (G_SETTINGS (priv->settings_display),
                        "display-window-title",
                        tmp_str);

    g_settings_set_boolean ( G_SETTINGS (priv->settings_display),
                        "formulaire-toujours-affiche",
                        conf.formulaire_toujours_affiche);
    g_settings_set_boolean (priv->settings_display,
                        "group-partial-balance-under-accounts",
                        conf.group_partial_balance_under_accounts);
    g_settings_set_boolean (G_SETTINGS (priv->settings_display),
                        "show-closed-accounts",
                        conf.show_closed_accounts);

    /* priv->settings_file */
    g_settings_set_boolean ( G_SETTINGS (priv->settings_file),
                        "dernier-fichier-auto",
                        conf.dernier_fichier_auto);
    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "sauvegarde-auto",
                        conf.sauvegarde_auto);
    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "force-enregistrement",
                        conf.force_enregistrement);
    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "force-import-directory",
                        conf.force_import_directory);
	if (conf.force_import_directory)
	{
		g_settings_set_string (G_SETTINGS (priv->settings_file),
										   "import-directory",
										   conf.import_directory);
	}

    g_settings_set_boolean (G_SETTINGS (priv->settings_file),
                        "compress-file",
                        conf.compress_file);

	recent_array = grisbi_app_get_recent_files_array ();
    if (g_strv_length (recent_array))
    {
        g_settings_set_strv (G_SETTINGS (priv->settings_file),
                        "names-last-files",
                        (const gchar * const *) recent_array);
    }

    g_settings_set_int (G_SETTINGS (priv->settings_file),
                        "nb-max-derniers-fichiers-ouverts",
                        conf.nb_max_derniers_fichiers_ouverts);

    g_settings_set_boolean (priv->settings_file,
                        "archives-check-auto",
                        conf.archives_check_auto);
    g_settings_set_int (priv->settings_file,
                        "max-transactions-before-warn-archival",
                        conf.max_non_archived_transactions_for_check);

    /* priv->settings_form */
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                        "affichage-exercice-automatique",
                        conf.affichage_exercice_automatique);
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                        "automatic-completion-payee",
                        conf.automatic_completion_payee);
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                        "automatic-erase-credit-debit",
                        conf.automatic_erase_credit_debit);
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                        "automatic-recover-splits",
                        conf.automatic_recover_splits);
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                            "form-enter-key", conf.entree);
    g_settings_set_int (G_SETTINGS (priv->settings_form),
                        "form-expander-label-width",
                        conf.form_expander_label_width);
    g_settings_set_boolean ( G_SETTINGS (priv->settings_form),
                        "limit-completion-current-account",
                        conf.limit_completion_to_current_account);

    /* priv->settings_general */
    if (conf.browser_command)
        g_settings_set_string (G_SETTINGS (priv->settings_general),
                        "browser-command",
                        conf.browser_command);

    g_settings_set_boolean (G_SETTINGS (priv->settings_general),
                        "pluriel-final",
                        conf.pluriel_final);

    switch (conf.metatree_action_2button_press)
    {
        case 1:
            tmp_str = "Edit Category";
            break;
        case 2:
            tmp_str = "Manage division";
            break;
        default:
            tmp_str = "gtk default";
    }
    g_settings_set_string (G_SETTINGS (priv->settings_general),
                           "metatree-action-2button-press",
                           tmp_str);

    g_settings_set_boolean (G_SETTINGS (priv->settings_general),
                            "show-transaction-gives-balance",
                            conf.show_transaction_gives_balance);

    /* priv->settings_geometry */
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "x-position",
                        conf.x_position);
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "y-position",
                        conf.y_position);
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "main-height",
                        conf.main_height);
    g_settings_set_int (G_SETTINGS (priv->settings_geometry),
                        "main-width",
                        conf.main_width);
    g_settings_set_boolean (G_SETTINGS (priv->settings_geometry),
                        "fullscreen",
                        conf.full_screen);
    g_settings_set_boolean (G_SETTINGS (priv->settings_geometry),
                        "maximized",
                        conf.maximize_screen);

    /* priv->settings_messages_delete */
    for (i = 0; delete_msg[i].name; i ++)
    {
        g_settings_set_boolean (G_SETTINGS (priv->settings_messages_delete),
                        delete_msg[i].name,
                        !delete_msg[i].hidden);
    }

    /* priv->settings_messages_tips */
    g_settings_set_int (G_SETTINGS (priv->settings_messages_tips),
                         "last-tip",
                         conf.last_tip);
    g_settings_set_boolean (G_SETTINGS (priv->settings_messages_tips),
                         "show-tip",
                         conf.show_tip);

    /* priv->settings_messages_warnings */
    for (i = 0; messages[i].name; i ++)
    {
        g_settings_set_boolean (G_SETTINGS (priv->settings_messages_warnings),
                        messages[i].name,
                        !messages[i].hidden);
    }

    /* priv->settings_panel */
    g_settings_set_int (G_SETTINGS (priv->settings_panel),
                        "panel-width",
                        conf.panel_width);

    /* priv->settings_scheduled */
    g_settings_set_boolean (priv->settings_scheduled,
                        "execute-scheduled-of-month",
                        conf.execute_scheduled_of_month);
    g_settings_set_boolean (priv->settings_scheduled,
                        "balances-with-scheduled",
                        conf.balances_with_scheduled);
    g_settings_set_int (priv->settings_scheduled,
                        "nb-days-before-scheduled",
                        conf.nb_days_before_scheduled);
    g_settings_set_boolean (priv->settings_scheduled,
							"scheduler-set-fixed-day",
							conf.scheduler_set_fixed_day);
    g_settings_set_int (priv->settings_scheduled,
						"scheduler-fixed-day",
						conf.scheduler_fixed_day);
}

GSettings *grisbi_settings_get_settings (gint schema)
{
    GSettings *settings = NULL;
    GrisbiSettingsPrivate *priv = NULL;

    priv = grisbi_settings_get_instance_private (grisbi_settings_get());

    switch (schema)
    {
        case SETTINGS_ROOT:
            settings = priv->settings_root;
            break;
        case SETTINGS_FILES_BACKUP:
            settings = priv->settings_backup;
            break;
        case SETTINGS_DISPLAY:
            settings = priv->settings_display;
            break;
        case SETTINGS_FILES_FILE:
            settings = priv->settings_file;
            break;
        case SETTINGS_FORM:
            settings = priv->settings_form;
            break;
        case SETTINGS_GENERAL:
            settings = priv->settings_general;
            break;
        case SETTINGS_GEOMETRY:
            settings = priv->settings_geometry;
            break;
        case SETTINGS_MESSAGES_DELETE:
            settings = priv->settings_messages_delete;
            break;
        case SETTINGS_MESSAGES_TIPS:
            settings = priv->settings_messages_tips;
            break;
        case SETTINGS_MESSAGES_WARNINGS:
            settings = priv->settings_messages_warnings;
            break;
        case SETTINGS_PANEL:
            settings = priv->settings_panel;
            break;
        case SETTINGS_PREFS:
            settings = priv->settings_prefs;
            break;
        case SETTINGS_SCHEDULED:
            settings = priv->settings_scheduled;
            break;
    }

    return settings;
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
