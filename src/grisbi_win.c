/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2015 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <sys/types.h>

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_win.h"
#include "grisbi_app.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

#define GSB_NBRE_CHAR 15
#define GSB_NAMEFILE_TOO_LONG 45

/*START_EXTERN Variables externes PROVISOIRE*/

/* declared in parametres.c PROVISOIRE*/
extern struct gsb_etat_t etat;

/* declared in main.c PROVISOIRE*/
extern struct gsb_run_t run;
extern gchar *nom_fichier_comptes;
extern gchar *titre_fichier;

/*END_EXTERN*/

struct _GrisbiWin
{
  GtkApplicationWindow parent;
};

struct _GrisbiWinClass
{
  GtkApplicationWindowClass parent_class;
};

typedef struct _GrisbiWinPrivate GrisbiWinPrivate;

struct _GrisbiWinPrivate
{
	GtkBuilder 			*builder;

    /* box principale */
    GtkWidget           *main_box;

    /* page d'accueil affichée si pas de fichier chargé automatiquement */
    GtkWidget           *accueil_page;

    /* widget général si un fichier est chargé */
    GtkWidget           *vbox_general;
    GtkWidget           *hpaned_general;

    /* nom du fichier associé à la fenêtre */
    gchar               *filename;

    /* titre du fichier */
    gchar               *file_title;

    /* titre de la fenêtre */
    gchar               *window_title;

    /* Menus et barres d'outils */
	GtkMenuBar 			*menubar;

	/* File Menu */
	GtkMenuItem			*item_new_acc_file;
	GtkMenuItem			*item_open_file;
	GtkMenuItem			*item_save;
	GtkMenuItem			*item_save_as;
	GtkMenuItem			*item_import_file;
	GtkMenuItem			*item_export_accounts;
	GtkMenuItem			*item_create_archive;
	GtkMenuItem			*item_export_archive;
	GtkMenuItem			*item_debug_acc_file;
	GtkMenuItem			*item_obf_acc_file;
	GtkMenuItem			*item_obf_qif_file;
	GtkMenuItem			*item_debug_mode;
	GtkMenuItem			*item_file_close;
	GtkMenuItem			*item_quit;

	/*gestion des fichiers récents */
	GtkWidget 			*recent_menu;
	GtkMenuItem			*item_recent_files;		/* item parent du sous_menu recent_files */

	/* Edit Menu */
	GtkMenuItem			*item_edit_ope;
	GtkMenuItem			*item_new_ope;
	GtkMenuItem			*item_remove_ope;
	GtkMenuItem			*item_template_ope;
	GtkMenuItem			*item_clone_ope;
	GtkMenuItem			*item_convert_ope;
	GtkMenuItem			*item_move_to_account;	/* item parent du sous_menu accounts */
	GtkMenuItem			*item_new_acc;
	GtkMenuItem			*item_remove_acc;
	GtkMenuItem			*item_prefs;

    /* statusbar */
    GtkWidget           *statusbar;
    guint               context_id;
    guint               message_id;

    /* headings_bar */
    GtkWidget           *headings_eb;
	GtkWidget			*arrow_eb_left;
	GtkWidget			*arrow_eb_right;


    /* variables de configuration de la fenêtre */
/*	GrisbiWinEtat		*etat;
*/
    /* structure run */
/*    GrisbiWindowRun     *run;
*/
};

G_DEFINE_TYPE_WITH_PRIVATE(GrisbiWin, grisbi_win, GTK_TYPE_APPLICATION_WINDOW);

/* MAIN WINDOW */
/**
 * set the title of the window
 *
 * \param gint 		account_number
 *
 * \return			TRUE if OK, FALSE otherwise
 * */
gboolean grisbi_win_set_grisbi_title ( gint account_number )
{
    gchar *titre_grisbi = NULL;
    gchar *titre = NULL;
    gint tmp_number;
    gboolean return_value;

    if ( nom_fichier_comptes == NULL )
    {
        titre_grisbi = g_strdup ( _("Grisbi") );
        return_value = TRUE;
    }
    else
    {
        switch ( conf.display_grisbi_title )
        {
            case GSB_ACCOUNTS_TITLE:
                if ( titre_fichier && strlen ( titre_fichier ) )
                    titre = g_strdup ( titre_fichier );
            break;
            case GSB_ACCOUNT_HOLDER:
            {
                if ( account_number == -1 )
                    tmp_number = gsb_data_account_first_number ();
                else
                    tmp_number = account_number;

                if ( tmp_number == -1 )
                {
                    if ( titre_fichier && strlen ( titre_fichier ) )
                        titre = g_strdup ( titre_fichier );
                }
                else
                {
                    titre = g_strdup ( gsb_data_account_get_holder_name ( tmp_number ) );

                    if ( titre == NULL )
                        titre = g_strdup ( gsb_data_account_get_name ( tmp_number ) );
                }
            break;
            }
            case GSB_ACCOUNTS_FILE:
                if ( nom_fichier_comptes && strlen ( nom_fichier_comptes ) )
                    titre = g_path_get_basename ( nom_fichier_comptes );
            break;
        }

        if ( titre && strlen ( titre ) > 0 )
        {
            titre_grisbi = g_strconcat ( titre, " - ", _("Grisbi"), NULL );
            g_free ( titre );

            return_value = TRUE;
        }
        else
        {
            titre_grisbi = g_strconcat ( "<", _("unnamed"), ">", NULL );
            return_value = FALSE;
        }
    }
    gtk_window_set_title ( GTK_WINDOW ( run.window ), titre_grisbi );

    if ( titre_grisbi && strlen ( titre_grisbi ) > 0 )
    {
        gsb_main_page_update_homepage_title ( titre_grisbi );
        g_free ( titre_grisbi );
    }

    return return_value;
}

/**
 * set size and position of the main window of grisbi.
 *
 * \param GrisbiWin *win
 *
 * \return
 * */
void grisbi_win_set_size_and_position ( GtkWindow *win )
{
	GrisbiWinPrivate *priv;

	printf ("grisbi_win_set_size_and_position\n");

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    /* set the size of the window */
    if ( conf.main_width && conf.main_height )
        gtk_window_set_default_size ( GTK_WINDOW ( win ), conf.main_width, conf.main_height );
    else
        gtk_window_set_default_size ( GTK_WINDOW ( win ), 900, 600 );

    /* display window at position */
    gtk_window_move ( GTK_WINDOW ( win ), conf.root_x, conf.root_y );

    /* set the full screen if necessary */
    if ( conf.full_screen )
        gtk_window_fullscreen ( GTK_WINDOW ( win ) );

    /* put up the screen if necessary */
    if ( conf.maximize_screen )
        gtk_window_maximize ( GTK_WINDOW ( win ) );
}

/**
 * retourne main_box
 *
 * \param GrisbiWin *win
 *
 * \return main_box
 **/
GtkWidget *grisbi_win_get_main_box ( GrisbiWin *win )
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );
    return priv->main_box;
}

/* STATUS_BAR */
/**
 * initiate the GtkStatusBar to hold various status
 * information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_init_statusbar ( GrisbiWin *win )
{
    GtkWidget *statusbar;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    priv->context_id = gtk_statusbar_get_context_id ( GTK_STATUSBAR ( priv->statusbar ), "Grisbi" );
    priv->message_id = -1;
}


/* HEADINGS_EB */
/**
 * Trigger a callback functions only if button event that triggered it
 * was a simple click.
 *
 * \param button
 * \param event
 * \param callback function
 *
 * \return  TRUE.
 */
static gboolean grisbi_win_headings_simpleclick_event_run ( GtkWidget *button,
                        GdkEvent *button_event,
                        GCallback callback )
{
    if ( button_event -> type == GDK_BUTTON_PRESS )
    {
        callback ( );
    }

    return TRUE;
}

/**
 * initiate the headings_bar information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_init_headings_eb ( GrisbiWin *win )
{
    GtkStyleContext *style;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    style = gtk_widget_get_style_context ( priv->headings_eb );

	gtk_widget_override_background_color ( priv->arrow_eb_left, GTK_STATE_FLAG_ACTIVE, NULL );
    g_signal_connect ( G_OBJECT ( priv->arrow_eb_left ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_win_headings_simpleclick_event_run ),
                        gsb_gui_navigation_select_prev );

	gtk_widget_override_background_color ( priv->arrow_eb_right, GTK_STATE_FLAG_ACTIVE, NULL );
    g_signal_connect ( G_OBJECT ( priv->arrow_eb_right ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_win_headings_simpleclick_event_run ),
                        gsb_gui_navigation_select_prev );

    gtk_widget_override_background_color ( priv->headings_eb, GTK_STATE_FLAG_ACTIVE, NULL );
}


/* WIN_MENU */
static const GActionEntry win_always_enabled_entries[] =
{
	{ "new_acc_file", grisbi_cmd_file_new, NULL, NULL, NULL },
	{ "open_file", grisbi_cmd_file_open_menu, NULL, NULL, NULL },
	{ "import_file", grisbi_cmd_importer_fichier, NULL, NULL, NULL },
	{ "obf_qif_file", grisbi_cmd_obf_qif_file, NULL, NULL, NULL }
};

static const GActionEntry win_context_enabled_entries[] =
{
	{ "save", grisbi_cmd_file_save, NULL, NULL, NULL },
	{ "save-as", grisbi_cmd_file_save_as, NULL, NULL, NULL },
	{ "export_accounts", grisbi_cmd_export_accounts, NULL, NULL, NULL },
	{ "create_archive", grisbi_cmd_create_archive, NULL, NULL, NULL },
	{ "export_archive", grisbi_cmd_export_archive, NULL, NULL, NULL },
	{ "debug_acc_file", grisbi_cmd_debug_acc_file, NULL, NULL, NULL },
	{ "obf_acc_file", grisbi_cmd_obf_acc_file, NULL, NULL, NULL },
	{ "debug_mode", grisbi_cmd_debug_mode_toggle, NULL, "false", NULL },
	{ "file_close", grisbi_cmd_file_close, NULL, NULL, NULL },
	{ "edit_ope", grisbi_cmd_edit_ope, NULL, NULL, NULL },
	{ "new_ope", grisbi_cmd_new_ope, NULL, NULL, NULL },
	{ "remove_ope", grisbi_cmd_remove_ope, NULL, NULL, NULL },
	{ "template_ope", grisbi_cmd_template_ope, NULL, NULL, NULL },
	{ "clone_ope", grisbi_cmd_clone_ope, NULL, NULL, NULL },
	{ "convert_ope", grisbi_cmd_convert_ope, NULL, NULL, NULL },
	{ "new_acc", grisbi_cmd_new_acc, NULL, NULL, NULL },
	{ "remove_acc", grisbi_cmd_remove_acc, NULL, NULL, NULL },
	{ "show_form", NULL, NULL, "false", grisbi_cmd_show_form_toggle },
	{ "show_reconciled", NULL, NULL, "false", grisbi_cmd_show_reconciled_toggle },
	{ "show_archived", NULL, NULL, "false", grisbi_cmd_show_archived_toggle },
	{ "show_closed_acc", NULL, NULL, "false", grisbi_cmd_show_closed_acc_toggle },
	{ "show_ope", grisbi_cmd_show_ope, G_VARIANT_TYPE_STRING, "2", NULL },
	{ "reset_width_col", grisbi_cmd_reset_width_col, NULL, NULL, NULL }
};

/**
 *
 *
 * \param GrisbiWin 	win
 * \param GMenuModel  	menubar
 *
 * \return
 **/
void grisbi_win_set_menubar ( GrisbiWin *win,
						gpointer app )
{
	GrisbiWinPrivate *priv;
	GAction *action;
	GtkMenuItem *item = NULL;
	GtkRecentFilter *filter;
	GtkRecentManager *recent_manager;
	gint i = 0;

	printf ("grisbi_win_set_menubar\n");
	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

	/* chargement des actions */
	/* actions toujours actives */
	g_action_map_add_action_entries ( G_ACTION_MAP ( win ),
						win_always_enabled_entries,
						G_N_ELEMENTS ( win_always_enabled_entries ),
						win );

	/* actions actives selon le contexte */
	g_action_map_add_action_entries ( G_ACTION_MAP ( win ),
						win_context_enabled_entries,
						G_N_ELEMENTS ( win_context_enabled_entries ),
						win );

	/* creation du sous menu fichiers récents */
	recent_manager = grisbi_app_get_recent_manager ();
	priv->recent_menu = gtk_recent_chooser_menu_new_for_manager ( recent_manager );
	gtk_recent_chooser_set_local_only ( GTK_RECENT_CHOOSER ( priv->recent_menu ), TRUE );
	gtk_recent_chooser_set_show_icons (GTK_RECENT_CHOOSER ( priv->recent_menu ), TRUE );
	gtk_recent_chooser_set_sort_type (GTK_RECENT_CHOOSER ( priv->recent_menu ),
						GTK_RECENT_SORT_MRU);
	gtk_recent_chooser_menu_set_show_numbers (GTK_RECENT_CHOOSER_MENU ( priv->recent_menu ), TRUE);
	gtk_recent_chooser_set_show_tips (GTK_RECENT_CHOOSER ( priv->recent_menu ), TRUE);
	filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_pattern (filter, "*.gsb");
	gtk_recent_chooser_set_filter ( GTK_RECENT_CHOOSER ( priv->recent_menu ), filter );

	g_signal_connect_swapped ( priv->recent_menu,
						"item-activated",
						G_CALLBACK ( grisbi_cmd_file_open_direct_menu ),
						recent_manager );

	gtk_menu_item_set_submenu ( priv->item_recent_files, priv->recent_menu );

	/* set actions menu "File" */
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_new_acc_file ), "win.new_acc_file" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_open_file ), "win.open_file" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_save ), "win.save" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_save_as ), "win.save-as" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_import_file ), "win.import_file" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_export_accounts ), "win.export_accounts" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_create_archive ), "win.create_archive" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_export_archive ), "win.export_archive" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_debug_acc_file ), "win.debug_acc_file" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_obf_acc_file ), "win.obf_acc_file" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_obf_qif_file ), "win.obf_qif_file" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_debug_mode ), "win.debug_mode" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_file_close ), "win.file_close" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_quit ), "app.quit" );

	/* set actions menu "Edit" */
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_edit_ope ), "win.edit_ope" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_new_ope ), "win.new_ope" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_remove_ope ), "win.remove_ope" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_template_ope ), "win.template_ope" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_clone_ope ), "win.clone_ope" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_convert_ope ), "win.convert_ope" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_new_acc ), "win.new_acc" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_remove_acc ), "win.remove_acc" );
	gtk_actionable_set_action_name ( GTK_ACTIONABLE ( priv->item_prefs ), "app.prefs" );


	/* disabled menus */
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "save");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "save-as");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "export_accounts");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "create_archive");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "export_archive");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "debug_acc_file");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "obf_acc_file");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "debug_mode");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "file_close");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "edit_ope");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "new_ope");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "remove_ope");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "template_ope");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "clone_ope");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "convert_ope");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );

	/* traitement du sous menu Move to account */
	gtk_widget_set_sensitive ( GTK_WIDGET ( priv->item_move_to_account ), FALSE );

	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "new_acc");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "remove_acc");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );
	action = g_action_map_lookup_action (G_ACTION_MAP ( win ), "reset_width_col");
	g_simple_action_set_enabled (G_SIMPLE_ACTION ( action ), FALSE );

}

/* WIN INIT */
/**
 *
 *
 * \param GrisbiWin *win
 *
 * \return
 **/
static void grisbi_win_init ( GrisbiWin *win )
{
	GrisbiWinPrivate *priv;
	GtkWidget *statusbar;
    GtkWidget *headings_eb;

	printf ("grisbi_win_init\n");
	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

	gtk_widget_init_template ( GTK_WIDGET ( win ) );

	/* initialisation de la barre d'état */
/*	grisbi_win_init_statusbar ( GRISBI_WIN ( win ) );
*/
	/* initialisation de headings_eb */
/*	grisbi_win_init_headings_eb ( GRISBI_WIN ( win ) );
*/
	run.window = GTK_WIDGET ( win );
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_class_init ( GrisbiWinClass *class )
{
	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/org/gtk/grisbi/ui/grisbi_win.ui");

	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, main_box );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, headings_eb );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, arrow_eb_left );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, arrow_eb_right );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, statusbar );
	/* Menus */
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_new_acc_file );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_open_file );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_recent_files );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_save );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_save_as );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_import_file );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_export_accounts );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_create_archive );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_export_archive );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_debug_acc_file );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_obf_acc_file );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_obf_qif_file );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_debug_mode );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_file_close );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_quit );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_edit_ope );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_new_ope );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_remove_ope );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_template_ope );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_clone_ope );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_convert_ope );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_move_to_account );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_new_acc );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_remove_acc );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( class ), GrisbiWin, item_prefs );
}


/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *grisbi_win_get_filename ( GrisbiWin *win )
{
	GrisbiWin *tmp_win;
	GrisbiWinPrivate *priv;

	if ( win )
		tmp_win = win;
	else
		tmp_win = grisbi_app_get_active_window ( NULL );

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( tmp_win ) );

	return priv->filename;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_set_filename ( GrisbiWin *win,
						const gchar *filename )
{
	GrisbiWin *tmp_win;
	GrisbiWinPrivate *priv;

	if ( win )
		tmp_win = win;
	else
		tmp_win = grisbi_app_get_active_window ( NULL );

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( tmp_win ) );
	priv->filename = g_strdup ( filename );
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
