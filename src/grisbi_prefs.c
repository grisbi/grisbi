/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2012 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_file.h"
#include "menu.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_gtkbuilder.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/


#define GRISBI_PREFS_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GRISBI_TYPE_PREFS, GrisbiPrefsPrivate))

static GtkBuilder *grisbi_prefs_builder = NULL;
static GtkWidget *grisbi_prefs_dialog = NULL;

/* last_page */
/* static gint last_page;  */

struct _GrisbiPrefsPrivate
{
    GtkWidget           *hpaned;

    /* tree_store du panel de gauche */
    GtkWidget           *treeview_left_panel;
    GtkTreeStore        *prefs_tree_model;

    /* notebook de droite */
    GtkWidget           *notebook_prefs;

    /* notebook_files */
    GtkWidget           *notebook_files;
    GtkWidget           *checkbutton_load_last_file;
    GtkWidget           *checkbutton_sauvegarde_auto;
    GtkWidget           *checkbutton_force_enregistrement;
    GtkWidget           *checkbutton_crypt_file;
    GtkWidget           *checkbutton_compress_file;
    GtkWidget           *spinbutton_nb_max_derniers_fichiers_ouverts;
    GtkWidget           *checkbutton_stable_config_file_model;
    GtkWidget           *filechooserbutton_accounts;
    GtkWidget           *checkbutton_make_bakup_single_file;
    GtkWidget           *checkbutton_compress_backup;
    GtkWidget           *checkbutton_sauvegarde_demarrage;
    GtkWidget           *checkbutton_make_backup;
    GtkWidget           *checkbutton_make_backup_every_minutes;
    GtkWidget           *spinbutton_make_backup_nb_minutes;
    GtkWidget           *filechooserbutton_backup;
};


G_DEFINE_TYPE(GrisbiPrefs, grisbi_prefs, GTK_TYPE_DIALOG)

/**
 * finalise GrisbiPrefs
 *
 * \param object
 *
 * \return
 */
static void grisbi_prefs_finalize ( GObject *object )
{
/*     GrisbiPrefs *prefs = GRISBI_PREFS ( object );  */

    devel_debug (NULL);
    grisbi_prefs_dialog = NULL;

    /* libération de l'objet prefs */
    G_OBJECT_CLASS ( grisbi_prefs_parent_class )->finalize ( object );
}


/**
 * Initialise GrisbiPrefsClass
 *
 * \param
 *
 * \return
 */
static void grisbi_prefs_class_init ( GrisbiPrefsClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS ( klass );

    object_class->finalize = grisbi_prefs_finalize;

    g_type_class_add_private ( object_class, sizeof( GrisbiPrefsPrivate ) );
}


/* CALLBACKS - UTILS */
/**
 * callback pour la fermeture des preferences
 *
 * \param prefs_dialog
 * \param result_id
 *
 * \return
 **/
static void grisbi_prefs_dialog_response  ( GtkDialog *prefs,
                        gint result_id )
{
    if ( !prefs )
        return;

    switch (result_id)
    {
        case GTK_RESPONSE_HELP:

            g_signal_stop_emission_by_name ( prefs, "response" );

            break;

        default:
        {
            GrisbiAppConf *conf;

            conf = grisbi_app_get_conf ( );

            /* on récupère la dimension de la fenêtre */
            gtk_window_get_size ( GTK_WINDOW ( prefs ), &conf->prefs_width, &conf->prefs_height );

            gtk_widget_destroy ( GTK_WIDGET ( prefs ) );
        }
    }
}


/**
 * sensitive a prefs
 *
 * \param object the object wich receive the signal, not used so can be NULL
 * \param widget the widget to sensitive
 *
 * \return FALSE
 * */
static void grisbi_prefs_sensitive_etat_widgets ( GrisbiPrefs *prefs,
                        gboolean sensitive )
{
    gtk_widget_set_sensitive ( prefs->priv->checkbutton_crypt_file, sensitive );


}

/* GTK_BUILDER */
/**
 * Crée un builder et récupère les widgets du fichier grisbi.ui
 *
 * \param
 *
 * \rerurn
 * */
static gboolean grisbi_prefs_initialise_builder ( GrisbiPrefs *prefs )
{
    /* Creation d'un nouveau GtkBuilder */
    grisbi_prefs_builder = gtk_builder_new ( );
    if ( grisbi_prefs_builder == NULL )
        return FALSE;

    /* récupère les widgets */
    if ( !utils_gtkbuilder_merge_ui_data_in_builder ( grisbi_prefs_builder, "grisbi_prefs.ui" ) )
        return FALSE;

    prefs->priv->hpaned = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "dialog_hpaned" ) );
    prefs->priv->treeview_left_panel = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "treeview_left_panel" ) );

    prefs->priv->notebook_prefs = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "notebook_prefs" ) );

    /* notebook files - onglet files */
    prefs->priv->notebook_files = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "notebook_files" ) );
    prefs->priv->checkbutton_load_last_file = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_load_last_file" ) );
    prefs->priv->checkbutton_sauvegarde_auto = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_sauvegarde_auto" ) );
    prefs->priv->checkbutton_force_enregistrement = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_force_enregistrement" ) );
    prefs->priv->checkbutton_crypt_file = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_crypt_file" ) );
    prefs->priv->checkbutton_compress_file = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_compress_file" ) );
    prefs->priv->spinbutton_nb_max_derniers_fichiers_ouverts = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "spinbutton_nb_max_derniers_fichiers_ouverts" ) );
    prefs->priv->checkbutton_stable_config_file_model = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_stable_config_file_model" ) );
    prefs->priv->filechooserbutton_accounts = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "filechooserbutton_accounts" ) );

    /* notebook files - onglet backup */
    prefs->priv->checkbutton_make_bakup_single_file = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_make_bakup_single_file" ) );
    prefs->priv->checkbutton_compress_backup = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_compress_backup" ) );
    prefs->priv->checkbutton_sauvegarde_demarrage = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_sauvegarde_demarrage" ) );
    prefs->priv->checkbutton_make_backup = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_make_backup" ) );
    prefs->priv->checkbutton_make_backup_every_minutes = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "checkbutton_make_backup_every_minutes" ) );
    prefs->priv->spinbutton_make_backup_nb_minutes = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "spinbutton_make_backup_nb_minutes" ) );
    prefs->priv->filechooserbutton_backup = GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "filechooserbutton_backup" ) );


    return TRUE;
}


/* RIGHT_PANEL : CALLBACKS */
/**
 * Set a boolean integer to the value of a checkbutton.  Normally called
 * via a GTK "toggled" signal handler.
 * 
 * \param checkbutton a pointer to a checkbutton widget.
 * \param value to change
 */
static void grisbi_prefs_checkbutton_changed ( GtkToggleButton *checkbutton,
                        gboolean *value )
{

    if ( value )
    {
        grisbi_app_conf_mutex_lock ();
        *value = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( checkbutton ) );
        grisbi_app_conf_mutex_unlock ();
    }
}


/**
 * Set a boolean integer to the value of a checkbutton.  Normally called
 * via a GTK "toggled" signal handler.
 *
 * \param eventbox a pointer to a eventbox widget.
 * \param value to change
 */
static gboolean grisbi_prefs_eventbox_clicked ( GObject *eventbox,
                        GdkEvent *event,
                        GtkToggleButton *checkbutton )
{
    gboolean state;

    state = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( checkbutton ) );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( checkbutton ), !state );

    return FALSE;
}


/**
 * Warns that there is no coming back if password is forgotten when
 * encryption is activated.
 *
 * \param checkbox  Checkbox that triggered event.
 * \param data      Unused.
 *
 * \return          FALSE
 */
static void grisbi_prefs_encryption_toggled ( GtkToggleButton *checkbutton,
                        gboolean *value )
{
    gboolean state;

    state = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( checkbutton ) );
    if ( value )
    {
        *value = state;
        gsb_file_set_modified ( TRUE );
    }

    if ( state )
    {
        GrisbiAppRun *run;

        run = grisbi_app_get_run ();
        dialog_message ( "encryption-is-irreversible" );
        run->new_crypted_file = TRUE;
    }
}


/**
 * set nb_max_derniers_fichiers_ouverts
 *
 * \param spinbutton a pointer to a spinbutton widget.
 * \param value to change
 */
static void grisbi_prefs_spinbutton_changed ( GtkSpinButton *spinbutton,
                        gboolean *value )
{

    if ( value )
    {
        GtkWidget *button = NULL;

        grisbi_app_conf_mutex_lock ();
        *value = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spinbutton ) );
        grisbi_app_conf_mutex_unlock ();

        affiche_derniers_fichiers_ouverts ();

        button = g_object_get_data ( G_OBJECT ( spinbutton ), "button" );
        if ( button && GTK_IS_TOGGLE_BUTTON ( button ) )
        {
            if ( gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( spinbutton ) ) == 0 )
                gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), FALSE );
            else
                gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button ), TRUE );
        }
    }
}


/**
 * called when choose a new directory for the account files or backup
 *
 * \param button the GtkFileChooserButton
 * \param dirname
 *
 * \return FALSE
 * */
static void grisbi_prefs_dir_chosen ( GtkWidget *button,
                        gchar *dirname )
{
    gchar *tmp_dir;

    g_signal_handlers_block_by_func ( button,
                        G_CALLBACK ( grisbi_prefs_dir_chosen ),
                        dirname );

    tmp_dir = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( button ) );

    grisbi_app_conf_mutex_lock ();
    if ( strcmp ( dirname, "account_files_path" ) == 0 )
        gsb_file_set_account_files_path ( tmp_dir, grisbi_app_get_conf () );
    else
        gsb_file_set_backup_path ( tmp_dir, grisbi_app_get_conf () );
    grisbi_app_conf_mutex_unlock ();

    g_signal_handlers_unblock_by_func ( button,
                        G_CALLBACK ( grisbi_prefs_dir_chosen ),
                        dirname );

    /* memory free */
    g_free ( tmp_dir );
}


/* RIGHT_PANEL : FILES - BACKUP */
/**
 * Création de la page de gestion des fichiers
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_setup_files_page ( GrisbiPrefs *prefs )
{
    GrisbiAppConf *conf;
    GrisbiWindowEtat *etat;

    conf = grisbi_app_get_conf ();
    etat = grisbi_window_get_window_etat ();

    /* set the variables for account tab */
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_load_last_file ),
                        conf->load_last_file );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_sauvegarde_auto ),
                        conf->sauvegarde_auto );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_force_enregistrement ),
                        conf->force_enregistrement );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_crypt_file ),
                        etat->crypt_file );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_compress_file ),
                        conf->compress_file );

    /* set the max number of files */
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( prefs->priv->spinbutton_nb_max_derniers_fichiers_ouverts ),
                        conf->nb_max_derniers_fichiers_ouverts );

    /* set current folder for account files */
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( prefs->priv->filechooserbutton_accounts ),
                        conf->account_files_path );

    /* Connect signal */
    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_load_last_file" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_load_last_file );

    g_signal_connect ( prefs->priv->checkbutton_load_last_file,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->load_last_file );

    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_sauvegarde_auto" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_sauvegarde_auto );

    g_signal_connect ( prefs->priv->checkbutton_sauvegarde_auto,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->sauvegarde_auto );

    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_force_enregistrement" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_force_enregistrement );

    g_signal_connect ( prefs->priv->checkbutton_force_enregistrement,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->force_enregistrement );

    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_compress_file" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_compress_file );

    g_signal_connect ( prefs->priv->checkbutton_compress_file,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->compress_file );

    if ( IS_DEVELOPMENT_VERSION )
    {
        gtk_widget_show ( GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "paddingbox_file_config" ) ) );
        g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_stable_config_file_model" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_stable_config_file_model );

        g_signal_connect ( prefs->priv->checkbutton_stable_config_file_model,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->stable_config_file_model );
    }
    else
        gtk_widget_hide ( GTK_WIDGET ( gtk_builder_get_object (
                        grisbi_prefs_builder, "paddingbox_file_config" ) ) );

    /* callback for encrypted files */
    g_signal_connect ( prefs->priv->checkbutton_crypt_file,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_encryption_toggled ),
                        NULL );

    /* callback for spinbutton_nb_max_derniers_fichiers_ouverts */
    g_signal_connect ( prefs->priv->spinbutton_nb_max_derniers_fichiers_ouverts,
                        "value-changed",
                        G_CALLBACK ( grisbi_prefs_spinbutton_changed ),
                        &conf->nb_max_derniers_fichiers_ouverts );

    /* connect the signal for filechooserbutton_accounts */
    g_signal_connect ( G_OBJECT ( prefs->priv->filechooserbutton_accounts ),
                        "selection-changed",
                        G_CALLBACK ( grisbi_prefs_dir_chosen ),
                        "account_files_path" );

    /* set the variables for backup tab */
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_make_bakup_single_file ),
                        conf->make_bakup_single_file );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_compress_backup ),
                        conf->compress_backup );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_sauvegarde_demarrage ),
                        conf->sauvegarde_demarrage );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_make_backup ),
                        conf->make_backup );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( prefs->priv->checkbutton_make_backup_every_minutes ),
                        conf->make_backup_every_minutes );

    /* set minutes */
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( prefs->priv->spinbutton_make_backup_nb_minutes ),
                        conf->make_backup_nb_minutes );

    /* set current folder for backup files */
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( prefs->priv->filechooserbutton_backup ),
                        conf->backup_path );

    /* Connect signal */
    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_make_bakup_single_file" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_make_bakup_single_file );

    g_signal_connect ( prefs->priv->checkbutton_make_bakup_single_file,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->make_bakup_single_file );

    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_compress_backup" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_compress_backup );

    g_signal_connect ( prefs->priv->checkbutton_compress_backup,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->compress_backup );

    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_sauvegarde_demarrage" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_sauvegarde_demarrage );

    g_signal_connect ( prefs->priv->checkbutton_sauvegarde_demarrage,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->sauvegarde_demarrage );

    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_make_backup" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_make_backup );

    g_signal_connect ( prefs->priv->checkbutton_make_backup,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->make_backup );

    g_signal_connect ( gtk_builder_get_object ( grisbi_prefs_builder, "eventbox_make_backup_every_minutes" ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_prefs_eventbox_clicked ),
                        prefs->priv->checkbutton_make_backup_every_minutes );

    g_signal_connect ( prefs->priv->checkbutton_make_backup_every_minutes,
                        "toggled",
                        G_CALLBACK ( grisbi_prefs_checkbutton_changed ),
                        &conf->make_backup_every_minutes );

    /* callback for spinbutton_make_backup_nb_minutes */
    g_object_set_data ( G_OBJECT ( prefs->priv->spinbutton_make_backup_nb_minutes ),
                        "button", prefs->priv->checkbutton_make_backup_every_minutes );
                        
    g_signal_connect ( prefs->priv->spinbutton_make_backup_nb_minutes,
                        "value-changed",
                        G_CALLBACK ( grisbi_prefs_spinbutton_changed ),
                        &conf->make_backup_nb_minutes );

    /* connect the signal for filechooserbutton_backup */
    g_signal_connect ( G_OBJECT ( prefs->priv->filechooserbutton_backup ),
                        "selection-changed",
                        G_CALLBACK ( grisbi_prefs_dir_chosen ),
                        "backup_path" );

    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( prefs->priv->notebook_files ), 0 );
}


/**
 * Création de la page de gestion des archives
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_setup_archives_page ( GrisbiPrefs *prefs )
{
    GrisbiAppConf *conf;
    GrisbiWindowEtat *etat;

    conf = grisbi_app_get_conf ();
    etat = grisbi_window_get_window_etat ();

}


/**
 * Création de la page de gestion de l'importation des fichiers
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_setup_import_page ( GrisbiPrefs *prefs )
{
    GrisbiAppConf *conf;
    GrisbiWindowEtat *etat;

    conf = grisbi_app_get_conf ();
    etat = grisbi_window_get_window_etat ();

}


/* LEFT_PANEL */
/**
 * met à jour la taille du panneau de gauche
 *
 * \param hpaned
 * \param allocation
 * \param null
 *
 * \return
 **/
static gboolean grisbi_prefs_hpaned_size_allocate ( GtkWidget *hpaned,
                        GtkAllocation *allocation,
                        gpointer null )
{
    GrisbiAppConf *conf;

    conf = grisbi_app_get_conf ( );
    conf->prefs_panel_width = gtk_paned_get_position ( GTK_PANED ( hpaned ) );
    
    return FALSE;

}


/**
 * remplit le model pour la configuration des états
 *
 * \param
 * \param
 *
 * \return
 * */
static void grisbi_prefs_left_panel_populate_tree_model ( GtkTreeStore *tree_model,
                        GrisbiPrefs *prefs )
{
    GtkTreeIter iter;
    gint page = 0;

    /* append group page "Main" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Main"), -1 );

    /* append page Fichiers */
    grisbi_prefs_setup_files_page ( prefs );
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Files"), page );
    page++;

     /* append page Archives */
    grisbi_prefs_setup_archives_page ( prefs );
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Archives"), page );
    page++;

     /* append page Import */
    grisbi_prefs_setup_import_page ( prefs );
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Import"), page );
    page++;

   /* append group page "Display" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Display"), -1 );
    page++;

    /* append group page "Transactions" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Transactions"), -1 );
    page++;

    /* append group page "Transaction form" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Transaction form"), -1 );
    page++;

    /* append group page "Resources" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Resources"), -1 );
    page++;

    /* append group page "Balance estimate" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Balance estimate"), -1 );
    page++;

    /* append group page "Graphiques" */
    utils_ui_left_panel_add_line ( tree_model, &iter, NULL, NULL, _("Graphs"), -1 );
    page++;

    if ( grisbi_app_get_active_filename () == NULL )
        grisbi_prefs_sensitive_etat_widgets ( prefs, FALSE );
    else
        grisbi_prefs_sensitive_etat_widgets ( prefs, TRUE );

    /* return */
}


/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 *
 *\return tree_view or NULL;
 * */
static GtkWidget *grisbi_prefs_left_panel_setup_tree_view ( GrisbiPrefs *prefs )
{
    GtkWidget *tree_view = NULL;
    GtkTreeStore *model = NULL;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;

    devel_debug (NULL);

    /* Création du model */
    model = gtk_tree_store_new ( LEFT_PANEL_TREE_NUM_COLUMNS,
                        G_TYPE_STRING,  /* LEFT_PANEL_TREE_TEXT_COLUMN */
                        G_TYPE_INT,     /* LEFT_PANEL_TREE_PAGE_COLUMN */
                        G_TYPE_INT,     /* LEFT_PANEL_TREE_BOLD_COLUMN */
                        G_TYPE_INT );   /* LEFT_PANEL_TREE_ITALIC_COLUMN */

    /* Create container + TreeView */
    tree_view = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "treeview_left_panel" ) );
    prefs->priv->treeview_left_panel = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "treeview_left_panel" ) );
    gtk_tree_view_set_model ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_MODEL ( model ) );
    g_object_unref ( G_OBJECT ( model ) );

    /* set the color of selected row */
/*     utils_set_tree_view_selection_and_text_color ( tree_view );  */

    /* make column */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( "Categories",
                        cell,
                        "text", LEFT_PANEL_TREE_TEXT_COLUMN,
                        "weight", LEFT_PANEL_TREE_BOLD_COLUMN,
                        "style", LEFT_PANEL_TREE_ITALIC_COLUMN,
                        NULL );
    gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN ( column ), GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), GTK_TREE_VIEW_COLUMN ( column ) );

    /* Handle select */
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) );
    g_signal_connect ( selection,
                        "changed",
                        G_CALLBACK ( utils_ui_left_panel_tree_view_selection_changed ),
                        prefs->priv->notebook_prefs );

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function ( selection,
                        utils_ui_left_panel_tree_view_selectable_func, NULL, NULL );

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect ( tree_view,
                        "realize",
                        G_CALLBACK ( utils_tree_view_set_expand_all_and_select_path_realize ),
                        "0:0" );

    /* remplissage du paned gauche */
    grisbi_prefs_left_panel_populate_tree_model ( model, prefs );

    /* show all widgets */
    gtk_widget_show_all ( tree_view );

    /* return */
    return tree_view;
}


/**
 * initialise le bouton expand collapse all
 *
 * \param suffixe name
 * \param tree_view
 *
 * \return
 */
static void grisbi_prefs_left_panel_init_button_expand ( GtkWidget *tree_view )
{
    GtkWidget *button;

    button = GTK_WIDGET ( gtk_builder_get_object ( grisbi_prefs_builder, "togglebutton_expand_prefs" ) );

    g_object_set_data ( G_OBJECT ( button ), "hbox_expand",
                        gtk_builder_get_object ( grisbi_prefs_builder, "hbox_toggle_expand_prefs" ) );

    g_object_set_data ( G_OBJECT ( button ), "hbox_collapse",
                        gtk_builder_get_object ( grisbi_prefs_builder, "hbox_toggle_collapse_prefs" ) );

    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( utils_togglebutton_collapse_expand_all_rows ),
                        tree_view );
}


/* CREATE OBJECT */
/**
 * Initialise GrisbiPrefs
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_init ( GrisbiPrefs *prefs )
{
    GrisbiAppConf *conf;
    GtkWidget *tree_view;

    devel_debug (NULL);
    conf = grisbi_app_get_conf ( );

    prefs->priv = GRISBI_PREFS_GET_PRIVATE ( prefs );

    if ( !grisbi_prefs_initialise_builder ( prefs ) )
        exit ( 1 );

    gtk_dialog_add_buttons ( GTK_DIALOG ( prefs ),
                        GTK_STOCK_CLOSE,
                        GTK_RESPONSE_CLOSE,
                        GTK_STOCK_HELP,
                        GTK_RESPONSE_HELP,
                        NULL );

    gtk_window_set_title ( GTK_WINDOW ( prefs ), _("Grisbi preferences") );
    gtk_dialog_set_has_separator ( GTK_DIALOG ( prefs ), FALSE );
    gtk_window_set_destroy_with_parent ( GTK_WINDOW ( prefs ), TRUE );

    gtk_container_set_border_width ( GTK_CONTAINER ( prefs ), 5 );
    gtk_box_set_spacing (GTK_BOX ( gtk_dialog_get_content_area ( GTK_DIALOG ( prefs ) ) ), 2 );
    gtk_container_set_border_width ( GTK_CONTAINER ( gtk_dialog_get_action_area ( GTK_DIALOG ( prefs ) ) ), 5 );
    gtk_box_set_spacing (GTK_BOX ( gtk_dialog_get_action_area (GTK_DIALOG ( prefs ) ) ), 6 );

    /* set the default size */
    if ( conf->prefs_width )
        gtk_window_set_default_size ( GTK_WINDOW ( prefs ),
                        conf->prefs_width, conf->prefs_height );

    /* create the tree_view */
    tree_view = grisbi_prefs_left_panel_setup_tree_view ( prefs );

    /* initialise le bouton expand all */
    grisbi_prefs_left_panel_init_button_expand ( tree_view );

    /* connect the signals */
    g_signal_connect ( G_OBJECT ( prefs->priv->hpaned ),
                        "size_allocate",
                        G_CALLBACK ( grisbi_prefs_hpaned_size_allocate ),
                        NULL );

    g_signal_connect ( prefs,
                        "response",
                        G_CALLBACK ( grisbi_prefs_dialog_response ),
                        NULL);

    gtk_box_pack_start ( GTK_BOX ( gtk_dialog_get_content_area ( GTK_DIALOG ( prefs ) ) ),
                        prefs->priv->hpaned, TRUE, TRUE, 0 );
    g_object_unref ( prefs->priv->hpaned );

    gtk_widget_show_all ( prefs->priv->hpaned );

    /* return */
}


/**
 * show the preferences dialog
 *
 * \param parent
 *
 * \return
 **/
void grisb_prefs_show_dialog ( GrisbiWindow *parent )
{
    if ( !GRISBI_IS_WINDOW ( parent ) )
        return;

    if ( grisbi_prefs_dialog == NULL )
    {
        grisbi_prefs_dialog = GTK_WIDGET ( g_object_new ( GRISBI_TYPE_PREFS, NULL ) );
        g_signal_connect ( grisbi_prefs_dialog,
                        "destroy",
                        G_CALLBACK ( gtk_widget_destroyed ),
                        &grisbi_prefs_dialog );
    }

    if ( GTK_WINDOW ( parent) != gtk_window_get_transient_for ( GTK_WINDOW ( grisbi_prefs_dialog ) ) )
    {
        gtk_window_set_transient_for ( GTK_WINDOW ( grisbi_prefs_dialog ), GTK_WINDOW ( parent ) );
    }

    gtk_window_present ( GTK_WINDOW ( grisbi_prefs_dialog ) );

}


/**
 * retourne le widget demandé
 *
 * \param
 *
 * \return
 */
GtkWidget *grisbi_prefs_widget_get_widget_by_name ( const gchar *name )
{
    return utils_gtkbuilder_get_widget_by_name ( grisbi_prefs_builder, name, NULL);
}


/**
 *
 *
 * \param parent
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

