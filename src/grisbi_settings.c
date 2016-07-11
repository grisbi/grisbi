/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2016 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include <config.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"
#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_settings.h"
#include "gsb_dirs.h"

 /*#include "gsb_file.h"
*/
#include "gsb_file_config.h"
/*#include "dialog.h"
#include "fenetre_principale.h"
#include "menu.h"
#include "structures.h"
#include "utils_buttons.h"
#include "utils_files.h"
#include "utils_str.h"
*/#include "erreur.h"

/*END_INCLUDE*/

struct _GrisbiSettingsPrivate
{
    GSettings *settings_root;
    GSettings *settings_backup;
    GSettings *settings_display;
    GSettings *settings_file;
    GSettings *settings_form;
    GSettings *settings_general;
    GSettings *settings_geometry;
    GSettings *settings_messages;
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
void grisbi_settings_init_settings_root ( GSettings *settings_root )
{
    /* Menu type OSX : FALSE par défaut */
    conf.prefer_app_menu = g_settings_get_boolean ( settings_root, "prefer-app-menu" );

    /* utiliser une copie du fichier de configuration de grisbi stable pour la version de développement
     * ça permet de ne pas polluer le fonctionnement de la version stable de Grisbi */
    conf.stable_config_file_model = g_settings_get_boolean ( settings_root, "stable-config-file-model" );
}

/**
 * set backup settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_backup ( GSettings *settings_backup )
{
    gchar *tmp_path;

    tmp_path = g_settings_get_string ( settings_backup, "backup-path" );
    if ( tmp_path == NULL || strlen ( tmp_path ) == 0 )
    {
        tmp_path = g_strdup ( gsb_dirs_get_user_data_dir () );
        g_settings_set_string ( settings_backup, "backup-path", tmp_path );
    }
    gsb_file_set_backup_path ( tmp_path );
    g_free ( tmp_path );
    conf.compress_backup = g_settings_get_boolean ( settings_backup, "compress-backup" );
    conf.make_backup = g_settings_get_boolean ( settings_backup, "make-backup" );
    conf.make_backup_every_minutes = g_settings_get_boolean ( settings_backup, "make-backup-every-minutes" );
    conf.make_backup_nb_minutes = g_settings_get_int ( settings_backup, "make-backup-nb-minutes" );
    conf.make_bakup_single_file = g_settings_get_boolean ( settings_backup, "make-backup-single-file" );
    /* Si sauvegarde automatique on la lance ici */
    if ( conf.make_backup_every_minutes
     &&
     conf.make_backup_nb_minutes )
        gsb_file_automatic_backup_start ( NULL, NULL );
}

/**
 * set display settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_display ( GSettings *settings_display )
{
    conf.display_grisbi_title = g_settings_get_int ( settings_display, "display-grisbi-title" );
    conf.display_toolbar = g_settings_get_boolean ( settings_display, "display-toolbar" );
    conf.show_closed_accounts = g_settings_get_boolean ( settings_display, "show-closed-accounts" );
    conf.show_headings_bar = g_settings_get_boolean ( settings_display, "show-headings-bar" );
}

/**
 * set file settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_file ( GSettings *settings_file )
{
    conf.compress_file = g_settings_get_boolean ( settings_file, "compress-file" );
    conf.dernier_fichier_auto = g_settings_get_boolean ( settings_file, "dernier-fichier-auto" );
    conf.force_enregistrement = g_settings_get_boolean ( settings_file, "force-enregistrement" );
    conf.nb_derniers_fichiers_ouverts = g_settings_get_int ( settings_file, "nb-derniers-fichiers-ouverts" );
    conf.nb_max_derniers_fichiers_ouverts = g_settings_get_int ( settings_file, "nb-max-derniers-fichiers-ouverts" );
    conf.sauvegarde_auto = g_settings_get_boolean ( settings_file, "sauvegarde-auto" );
    conf.sauvegarde_demarrage = g_settings_get_boolean ( settings_file, "sauvegarde-demarrage" );
}

/**
 * set form settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_form ( GSettings *settings_form )
{
    conf.affichage_exercice_automatique = g_settings_get_boolean ( settings_form, "affichage-exercice-automatique" );
    conf.automatic_completion_payee = g_settings_get_boolean ( settings_form, "automatic-completion-payee" );
    conf.automatic_erase_credit_debit = g_settings_get_boolean ( settings_form, "automatic-erase-credit-debit" );
    conf.automatic_recover_splits = g_settings_get_boolean ( settings_form, "automatic-recover-splits" );
    conf.formulaire_toujours_affiche = g_settings_get_boolean ( settings_form, "formulaire-toujours-affiche" );
    conf.entree = g_settings_get_boolean ( settings_form, "form-enter-key" );
    conf.limit_completion_to_current_account = g_settings_get_boolean ( settings_form, "limit-completion-current-account" );
}

/**
 * set general settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_general ( GSettings *settings_general )
{
    gchar *tmp_str;

    conf.utilise_fonte_listes = g_settings_get_boolean ( settings_general, "utilise-fonte-listes" );
    if ( conf.utilise_fonte_listes )
    {
        conf.font_string = g_settings_get_string ( settings_general, "font-string" );
    }
    tmp_str = g_settings_get_string ( settings_general, "browser-command" );
    if ( tmp_str == NULL || strlen ( tmp_str ) == 0 )
    {
        g_settings_set_string ( settings_general, "browser-command", ETAT_WWW_BROWSER );
        conf.browser_command = g_strdup ( ETAT_WWW_BROWSER );
    }
    else
        conf.browser_command = tmp_str;
    conf.pluriel_final = g_settings_get_boolean ( settings_general, "pluriel-final" );
}

/**
 * set geometry
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_geometry ( GSettings *settings_geometry )
{
    /* fenetre : 1300x900 100+100 fullscreen et maximize FALSE par défaut */
    conf.x_position = g_settings_get_int ( settings_geometry, "x-position" );
    conf.y_position = g_settings_get_int ( settings_geometry, "y-position" );
    conf.main_height = g_settings_get_int ( settings_geometry, "main-height" );
    conf.main_width = g_settings_get_int ( settings_geometry, "main-width" );
    conf.full_screen = g_settings_get_boolean ( settings_geometry, "fullscreen" );
    conf.maximize_screen = g_settings_get_boolean ( settings_geometry, "maximized" );
}

/**
 * set messages settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_messages( GSettings *settings_messages )
{

}

/**
 * set panel settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_panel ( GSettings *settings_panel )
{
    conf.active_scrolling_left_pane = g_settings_get_boolean ( settings_panel, "active-scrolling-left-pane" );
    conf.panel_width = g_settings_get_int ( settings_panel, "panel-width" );
}

/**
 * set prefs settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_prefs ( GSettings *settings_prefs )
{
    conf.prefs_width = g_settings_get_int ( settings_prefs, "prefs-width" );
}

/**
 * set scheduled settings
 *
 * \param GSettings     settings
 *
 * \return
 **/
void grisbi_settings_init_settings_scheduled ( GSettings *settings_scheduled )
{

}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_settings_init ( GrisbiSettings *self )
{
    GrisbiSettingsPrivate *priv;

    devel_debug (NULL);
    printf ("gsb_config_init_app_config\n");

    priv = grisbi_settings_get_instance_private ( self );

    priv->settings_root = g_settings_new ( "org.gtk.grisbi" );
    grisbi_settings_init_settings_root ( priv->settings_root );

    priv->settings_backup = g_settings_new ( "org.gtk.grisbi.backup" );
    grisbi_settings_init_settings_backup ( priv->settings_backup );

    priv->settings_display = g_settings_new ( "org.gtk.grisbi.display" );
    grisbi_settings_init_settings_display ( priv->settings_display );

    priv->settings_file = g_settings_new ( "org.gtk.grisbi.file" );
    grisbi_settings_init_settings_file ( priv->settings_file );

    priv->settings_form = g_settings_new ( "org.gtk.grisbi.form" );
    grisbi_settings_init_settings_form ( priv->settings_form );

    priv->settings_general = g_settings_new ( "org.gtk.grisbi.general" );
    grisbi_settings_init_settings_general ( priv->settings_general );

    priv->settings_geometry = g_settings_new ( "org.gtk.grisbi.geometry" );
    grisbi_settings_init_settings_geometry ( priv->settings_geometry );

    priv->settings_messages = g_settings_new ( "org.gtk.grisbi.messages" );
    grisbi_settings_init_settings_messages ( priv->settings_messages );

    priv->settings_panel = g_settings_new ( "org.gtk.grisbi.panel" );
    grisbi_settings_init_settings_panel ( priv->settings_panel );

    priv->settings_prefs = g_settings_new ( "org.gtk.grisbi.prefs" );
    grisbi_settings_init_settings_prefs ( priv->settings_prefs );

    priv->settings_scheduled = g_settings_new ( "org.gtk.grisbi.scheduled" );
    grisbi_settings_init_settings_scheduled ( priv->settings_scheduled );
}


static void dispose ( GObject *object )
{
    GrisbiSettings *self = GRISBI_SETTINGS ( object );
    GrisbiSettingsPrivate *priv;

    priv = grisbi_settings_get_instance_private ( self );

    g_clear_object ( &priv->settings_root );
    g_clear_object ( &priv->settings_backup );
    g_clear_object ( &priv->settings_display );
    g_clear_object ( &priv->settings_file );
    g_clear_object ( &priv->settings_form );
    g_clear_object ( &priv->settings_general );
    g_clear_object ( &priv->settings_geometry );
    g_clear_object ( &priv->settings_messages );
    g_clear_object ( &priv->settings_panel );
    g_clear_object ( &priv->settings_prefs );
    g_clear_object ( &priv->settings_scheduled );

    G_OBJECT_CLASS ( grisbi_settings_parent_class )->dispose ( object );
}


static void finalize ( GObject *object )
{
    singleton = NULL;

    /* Chain up to the parent class */
    G_OBJECT_CLASS ( grisbi_settings_parent_class )->finalize ( object );
}


static void grisbi_settings_class_init ( GrisbiSettingsClass *klass )
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->dispose = dispose;
        object_class->finalize = finalize;
}


/*******************************************************************************
 * Public Methods
 ******************************************************************************/
GrisbiSettings *grisbi_settings_get ( void )
{
    if (!singleton)
    {
        singleton = GRISBI_SETTINGS ( g_object_new ( GRISBI_TYPE_SETTINGS, NULL) );
    }
    else
    {
        g_object_ref ( singleton );
    }
        g_assert ( singleton );

        return singleton;
}

 /**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_settings_save_app_config ( GrisbiSettings *settings )
{
/*    g_settings_set_boolean ( settings, "prefer-app-menu", conf.prefer_app_menu );
*/
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
