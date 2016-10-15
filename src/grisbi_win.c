/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2016 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include "grisbi_app.h"
#include "accueil.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_form.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "erreur.h"
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
	GtkBuilder *        builder;

    /* box principale */
    GtkWidget *         main_box;

    /* page d'accueil affichée si pas de fichier chargé automatiquement */
    GtkWidget *         page_accueil;

    /* widget général si un fichier est chargé */
    GtkWidget *         vbox_general;
    GtkWidget *         hpaned_general;
    GtkWidget *         notebook_general;

    /* nom du fichier associé à la fenêtre */
    gchar *             filename;

    /* titre du fichier */
    gchar *             file_title;

    /* titre de la fenêtre */
    gchar *             window_title;

    /* Menus et barres d'outils */
    /* menu move-to-acc */
    GMenu *             menu;
    gboolean            init_move_to_acc;

    /* statusbar */
    GtkWidget *         statusbar;
    GdkWindow *         tracked_window;
    guint               context_id;
    guint               message_id;

    /* headings_bar */
    GtkWidget *         headings_eb;
    GtkWidget *         headings_title;          /** Title for the heading bar. */
    GtkWidget *         headings_suffix;         /** Suffix for the heading bar.  */

    /* variables de configuration de la fenêtre */
/*	GrisbiWinEtat		*etat;
*/
    /* structure run */
/*    GrisbiWindowRun     *run;
*/
};

G_DEFINE_TYPE_WITH_PRIVATE(GrisbiWin, grisbi_win, GTK_TYPE_APPLICATION_WINDOW);

/* variables initialisées lors de l'exécution de grisbi PROVISOIRE */
struct gsb_run_t run;

/* global "etat" structure shared in the entire program */
struct gsb_etat_t etat;

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/* STATUS_BAR */
/**
 * initiate the GtkStatusBar to hold various status
 * information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_init_statusbar (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* set status_bar PROVISOIRE */
    priv->context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (priv->statusbar), "Grisbi");
    priv->message_id = G_MAXUINT;

    gtk_widget_show_all (priv->statusbar);

}


/* HEADINGS_EB */
static void grisbi_win_headings_private_update_label_markup (GtkLabel *label,
                                                             const gchar *text,
                                                             gboolean escape_text)
{
    gchar* tmp_str;

    if (escape_text)
        tmp_str = g_markup_printf_escaped ("<b>%s</b>", text);
    else
        tmp_str = g_strconcat ("<b>", text, "</b>", NULL);
    gtk_label_set_markup (label,tmp_str);

    g_free (tmp_str);
}

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
static gboolean grisbi_win_headings_simpleclick_event_run (GtkWidget *button,
                        GdkEvent *button_event,
                        GCallback callback)
{
    if (button_event -> type == GDK_BUTTON_PRESS)
    {
        callback ();
    }

    return TRUE;
}

/**
 * retourne headings_eb
 *
 * \param GrisbiWin *win
 *
 * \return main_box
 **/
static GtkWidget *grisbi_win_get_headings_eb (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

    if (win == NULL)
        win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    return priv->headings_eb;
}

/**
 * initiate the headings_bar information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_create_headings_eb (GrisbiWin *win)
{
    GtkWidget *hbox;
    GtkWidget *arrow_eb;
    GtkWidget *arrow_left;
    GtkWidget *arrow_right;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    priv->headings_eb = gtk_event_box_new ();
    gtk_widget_set_name ( priv->headings_eb, "grey_box");

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_end (hbox, MARGIN_END);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);

    /* Create two arrows. */
    arrow_left = gtk_image_new_from_icon_name ("pan-start-symbolic", GTK_ICON_SIZE_BUTTON);
    arrow_eb = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (arrow_eb), arrow_left);
    g_signal_connect (G_OBJECT (arrow_eb), "button-press-event",
                        G_CALLBACK (grisbi_win_headings_simpleclick_event_run),
                        gsb_gui_navigation_select_prev);
    gtk_box_pack_start (GTK_BOX (hbox), arrow_eb, FALSE, FALSE, 0);

    arrow_right = gtk_image_new_from_icon_name ("pan-end-symbolic", GTK_ICON_SIZE_BUTTON);
    arrow_eb = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (arrow_eb), arrow_right);
    g_signal_connect (G_OBJECT (arrow_eb), "button-press-event",
                      G_CALLBACK (grisbi_win_headings_simpleclick_event_run),
                      gsb_gui_navigation_select_next);
    gtk_box_pack_start (GTK_BOX(hbox), arrow_eb, FALSE, FALSE, 3);

    /* Define labels. */
    priv->headings_title = gtk_label_new (NULL);
    gtk_label_set_justify (GTK_LABEL(priv->headings_title), GTK_JUSTIFY_LEFT);
    utils_labels_set_alignement (GTK_LABEL (priv->headings_title), 0.0, 0.5);
    gtk_box_pack_start (GTK_BOX(hbox), priv->headings_title, TRUE, TRUE, 3);

    priv->headings_suffix = gtk_label_new (NULL);
    gtk_box_pack_start (GTK_BOX(hbox), priv->headings_suffix, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (priv->headings_eb), hbox);
}

/* HPANED_GENERAL */
/**
 * save hpahed width
 *
 * \param
 * \param
 * \param
 *
 * \return FALSE
 */
static gboolean grisbi_win_hpaned_size_allocate (GtkWidget *hpaned,
                                          GtkAllocation *allocation,
                                          gpointer null)
{
    conf.panel_width = gtk_paned_get_position (GTK_PANED (hpaned));

    return FALSE;
}

/* NOTEBOOK_GENERAL */
/**
 * Init the main notebook :
 * a notebook wich contains the pages : main page, accounts, scheduler... and
 * the form on the bottom, the form will be showed only for accounts page and
 * scheduler page
 *
 * \param
 *
 * \return
 */
static GtkWidget *grisbi_win_create_general_notebook (GrisbiWin *win)
{
    GtkWidget *vbox;
    GtkWidget *form;
	GrisbiWinPrivate *priv;

    devel_debug ( "create_main_notebook" );
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* the main right page is a vbox with a notebook on the top
     * and the form on the bottom */
    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

    /* append the notebook */
    priv->notebook_general = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK (priv->notebook_general ), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK (priv->notebook_general ), FALSE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), priv->notebook_general, TRUE, TRUE, 0 );
    gtk_widget_show (priv->notebook_general);

    /* append the form */
    form = gsb_form_new ();
    gtk_box_pack_start ( GTK_BOX ( vbox ), form, FALSE, FALSE, 0 );
    gtk_widget_hide (form);

    gtk_widget_show (vbox);

    return vbox;
}

/* WIN STATE */
/**
 * check on any change on the main window
 * for now, only to check if we set/unset the full-screen
 *
 * \param window
 * \param event
 * \param null
 *
 * \return FALSE
 * */
static gboolean grisbi_win_change_state_window (GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null)
{
    gboolean show;

    if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
    {
        show = !(event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED);

/*        gtk_window_set_has_resize_grip (GTK_WINDOW (window), show);
*/        conf.maximize_screen = !show;
    }

    /* return value */
    return FALSE;
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 *
 *
 * \param GrisbiWin *win
 *
 * \return
 **/
static void grisbi_win_init (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    priv->filename = NULL;
    priv->file_title = NULL;
    priv->window_title = NULL;

    /* initialisations des widgets liés à gsb_file_new_gui */
    priv->vbox_general = NULL;
    priv->notebook_general = NULL;

    /* init widgets in grisbi_win.ui */
	gtk_widget_init_template (GTK_WIDGET (win));

	/* initialisation de la barre d'état */
	grisbi_win_init_statusbar (GRISBI_WIN (win));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_class_init (GrisbiWinClass *klass)
{
	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
                                                 "/org/gtk/grisbi/ui/grisbi_win.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, main_box);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiWin, statusbar);

    /* signaux */
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_win_change_state_window);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *grisbi_win_get_filename (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	if (win == NULL)
		win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

	return priv->filename;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_set_filename (GrisbiWin *win,
						const gchar *filename)
{
	GrisbiWinPrivate *priv;

	if (!win)
        win = grisbi_app_get_active_window (NULL);

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
	priv->filename = g_strdup (filename);
}

/* GET WIDGET */
/**
 * retourne main_box
 *
 * \param GrisbiWin *win
 *
 * \return main_box
 **/
GtkWidget *grisbi_win_get_main_box (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    return priv->main_box;
}

/**
 * retourne notebook_general
 *
 * \param GrisbiWin *win
 *
 * \return main_box
 **/
GtkWidget *grisbi_win_get_notebook_general (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    return priv->notebook_general;
}

/* WIN_MENU */
/**
 *
 *
 * \param GrisbiWin 	win
 * \param GMenuModel  	menubar
 *
 * \return
 **/
void grisbi_win_init_menubar (GrisbiWin *win,
						gpointer app)
{
	GAction *action;
    gchar * items[] = {
        "save",
        "save-as",
        "export-accounts",
        "create-archive",
        "export-archive",
        "debug-acc-file",
        "obf-acc-file",
        "debug-mode",
        "file-close",
        "edit-ope",
        "new-ope",
        "remove-ope",
        "template-ope",
        "clone-ope",
        "convert-ope",
        "new-acc",
        "remove-acc",
        "show-form",
        "show-reconciled",
        "show-archived",
        "show-closed-acc",
        "show-ope",
        "reset-width-col",
        NULL
    };
    gchar **tmp = items;
    gboolean has_app_menu;

	/* initialisations sub menus */
	action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-form");
    g_action_change_state (G_ACTION (action), g_variant_new_boolean (conf.formulaire_toujours_affiche));
	action = g_action_map_lookup_action (G_ACTION_MAP (win), "show-closed-acc");
    g_action_change_state (G_ACTION (action), g_variant_new_boolean (conf.show_closed_accounts));

	/* disabled menus */
    while (*tmp)
    {
        gsb_menu_gui_sensitive_win_menu_item (*tmp, FALSE);

        tmp++;
    }

    /* sensibilise le menu new-window PROVISOIRE*/
    //~ has_app_menu = grisbi_app_get_has_app_menu (GRISBI_APP (app));
	//~ if (!has_app_menu)
		//~ gsb_menu_gui_sensitive_win_menu_item ("new-window", FALSE);

    /* sensibilise le menu preferences */
    action = grisbi_app_get_prefs_action ();
    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_delete (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;
    GMenu *menu;
    GSList *tmp_list;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    if (priv->init_move_to_acc == FALSE)
        return;

    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
        gint i;

        i = gsb_data_account_get_no_account (tmp_list -> data);

        if (!gsb_data_account_get_closed_account (i))
        {
            gchar *tmp_name;

            tmp_name = g_strdup_printf ("move-to-acc%d", i);
            g_action_map_remove_action (G_ACTION_MAP (win), tmp_name);

            g_free (tmp_name);
        }
        tmp_list = tmp_list -> next;
    }

    menu = grisbi_app_get_menu_edit ();

    g_menu_remove (menu, 3);
    priv->init_move_to_acc = FALSE;
}

/**
 *
 *
 * \param GrisbiWin 	win
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_new (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;
    GAction *action;
    GMenu *menu;
    GMenu *submenu;
    GMenuItem *menu_item;
    GSList *tmp_list;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    menu = grisbi_app_get_menu_edit ();

    submenu = g_menu_new ();

    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
        gint i;

        i = gsb_data_account_get_no_account (tmp_list -> data);

        if (!gsb_data_account_get_closed_account (i))
        {
            gchar *tmp_name;
            gchar *account_name;
            gchar *action_name;

            tmp_name = g_strdup_printf ("move-to-acc%d", i);
            account_name = gsb_data_account_get_name (i);
            if (!account_name)
                account_name = _("Unnamed account");

            action = (GAction *) g_simple_action_new (tmp_name, NULL);
            g_signal_connect (action,
                        "activate",
                        G_CALLBACK (grisbi_cmd_move_to_account_menu),
                        GINT_TO_POINTER (i));
            g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

            g_action_map_add_action (G_ACTION_MAP (grisbi_app_get_active_window (NULL)), action);
            g_object_unref (G_OBJECT (action));

            action_name = g_strconcat ("win.", tmp_name, NULL);
            g_menu_append (submenu, account_name, action_name);

            g_free (tmp_name);
            g_free (action_name);
        }

        tmp_list = tmp_list -> next;
    }

    menu_item = g_menu_item_new_submenu (_("Move transaction to another account"), (GMenuModel*) submenu);
    g_menu_item_set_detailed_action (menu_item, "win.move-to-acc");
    action = g_action_map_lookup_action (G_ACTION_MAP (win), "move-to-acc");
    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

    g_menu_insert_item (G_MENU (menu), 3, menu_item);
    g_object_unref (menu_item);
    g_object_unref (submenu);
    priv->init_move_to_acc = TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_update (gboolean active)
{
    GrisbiWin *win;
    GAction *action;
    GSList *tmp_list;
    static gboolean flag_active = FALSE;

    if (flag_active == active)
        return;

    win = grisbi_app_get_active_window (NULL);

    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
        gint i;

        i = gsb_data_account_get_no_account (tmp_list -> data);

        if (!gsb_data_account_get_closed_account (i))
        {
            gchar *tmp_name;

            tmp_name = g_strdup_printf ("move-to-acc%d", i);
            action = g_action_map_lookup_action (G_ACTION_MAP (win), tmp_name);

            if (gsb_gui_navigation_get_current_account () == i)
            {
                g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);
                tmp_list = tmp_list -> next;
                continue;
            }
            if (active)
                g_simple_action_set_enabled (G_SIMPLE_ACTION (action), TRUE);
            else
                g_simple_action_set_enabled (G_SIMPLE_ACTION (action), FALSE);

            g_free (tmp_name);
        }
        tmp_list = tmp_list -> next;
    }
    flag_active = active;
}

/* MAIN WINDOW */
/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_free_private_struct (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

    devel_debug (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    g_free (priv->filename);
    priv->filename = NULL;

    g_free (priv->file_title);
    priv->file_title = NULL;

    g_free (priv->window_title);
    priv->window_title = NULL;

    g_clear_object (&priv->builder);
    g_clear_object (&priv->menu);
}

/**
 * set the title of the window
 *
 * \param gint 		account_number
 *
 * \return			TRUE if OK, FALSE otherwise
 * */
gboolean grisbi_win_set_grisbi_title (gint account_number)
{
    gchar *titre_grisbi = NULL;
    gchar *titre = NULL;
    gint tmp_number;
    gboolean return_value;

    if (nom_fichier_comptes == NULL)
    {
        titre_grisbi = g_strdup (_("Grisbi"));
        return_value = TRUE;
    }
    else
    {
        switch (conf.display_grisbi_title)
        {
            case GSB_ACCOUNTS_TITLE:
                if (titre_fichier && strlen (titre_fichier))
                    titre = g_strdup (titre_fichier);
            break;
            case GSB_ACCOUNT_HOLDER:
            {
                if (account_number == -1)
                    tmp_number = gsb_data_account_first_number ();
                else
                    tmp_number = account_number;

                if (tmp_number == -1)
                {
                    if (titre_fichier && strlen (titre_fichier))
                        titre = g_strdup (titre_fichier);
                }
                else
                {
                    titre = g_strdup (gsb_data_account_get_holder_name (tmp_number));

                    if (titre == NULL)
                        titre = g_strdup (gsb_data_account_get_name (tmp_number));
                }
            break;
            }
            case GSB_ACCOUNTS_FILE:
                if (nom_fichier_comptes && strlen (nom_fichier_comptes))
                    titre = g_path_get_basename (nom_fichier_comptes);
            break;
        }

        if (titre && strlen (titre) > 0)
        {
            titre_grisbi = g_strconcat (titre, " - ", _("Grisbi"), NULL);
            g_free (titre);

            return_value = TRUE;
        }
        else
        {
            titre_grisbi = g_strconcat ("<", _("unnamed"), ">", NULL);
            return_value = FALSE;
        }
    }
    gtk_window_set_title (GTK_WINDOW (grisbi_app_get_active_window (NULL)), titre_grisbi);

    if (titre_grisbi && strlen (titre_grisbi) > 0)
    {
        gsb_main_page_update_homepage_title (titre_grisbi);
        g_free (titre_grisbi);
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
void grisbi_win_set_size_and_position (GtkWindow *win)
{
	GrisbiWinPrivate *priv;
    GdkGeometry size_hints = {
    1200, 600, -1, -1, 1300, 750, 10, 10, 1.5, 1.5, GDK_GRAVITY_NORTH_WEST
  };

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* on fixe la taille mini remplacer main_box par plus pertinent.*/
    gtk_window_set_geometry_hints (GTK_WINDOW (win),
                                   priv->main_box,
                                   &size_hints,
                                   GDK_HINT_RESIZE_INC |
                                   GDK_HINT_MIN_SIZE |
                                   GDK_HINT_BASE_SIZE);

    /* set the size of the window */
    if (conf.main_width && conf.main_height)
        gtk_window_set_default_size (GTK_WINDOW (win), conf.main_width, conf.main_height);
    else
        gtk_window_set_default_size (GTK_WINDOW (win), 900, 600);

    /* display window at position */
    gtk_window_move (GTK_WINDOW (win), conf.x_position, conf.y_position);

    /* set the full screen if necessary */
    if (conf.full_screen)
        gtk_window_fullscreen (GTK_WINDOW (win));

    /* put up the screen if necessary */
    if (conf.maximize_screen)
        gtk_window_maximize (GTK_WINDOW (win));
}

/* VBOX_GENERAL */
/**
 * free vbox_general
 *
 * \param
 *
 * \return
 **/
GtkWidget *grisbi_win_create_general_widgets (GrisbiWin *win)
{
	GrisbiWinPrivate *priv;

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    /* création de vbox_general */
    priv->vbox_general = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

    /* chargement de headings_eb */
    /* initialisation de headings_eb */
    grisbi_win_create_headings_eb (GRISBI_WIN (win));
    gtk_box_pack_start (GTK_BOX (priv->vbox_general), priv->headings_eb, FALSE, FALSE, 0);
    if (conf.show_headings_bar)
        gtk_widget_show_all (priv->headings_eb);
    else
        gtk_widget_hide (priv->headings_eb);

    /* Then create and fill the main hpaned. */
    priv->hpaned_general = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
    g_signal_connect (G_OBJECT (priv->hpaned_general),
                      "size_allocate",
                      G_CALLBACK (grisbi_win_hpaned_size_allocate),
                      NULL);
    gtk_box_pack_start (GTK_BOX (priv->vbox_general), priv->hpaned_general, TRUE, TRUE, 0);
    gtk_paned_add1 (GTK_PANED (priv->hpaned_general), gsb_gui_navigation_create_navigation_pane ());
    gtk_paned_add2 (GTK_PANED (priv->hpaned_general), grisbi_win_create_general_notebook (win));
    gtk_container_set_border_width (GTK_CONTAINER (priv->hpaned_general), 6);

    if (conf.panel_width > 250)
        gtk_paned_set_position (GTK_PANED (priv->hpaned_general), conf.panel_width);
    else
    {
        gint width, height;

        gtk_window_get_size (GTK_WINDOW (win), &width, &height);
        gtk_paned_set_position (GTK_PANED (priv->hpaned_general), (gint) width / 4);
    }

    /* show the widgets */
    gtk_widget_show (priv->hpaned_general);
    gtk_widget_show (priv->vbox_general);

    return priv->vbox_general;
}

/**
 * free vbox_general
 *
 * \param
 *
 * \return
 **/
void grisbi_win_free_general_vbox (void)
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    if (priv->vbox_general)
    {
        gtk_widget_destroy (priv->vbox_general);
        priv->vbox_general = NULL;
    }
}

/**
 * free notebook_general
 *
 * \param
 *
 * \return
 **/
void grisbi_win_free_general_notebook ( void )
{
    GrisbiWin *win = NULL;
    GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    if (win == NULL)
        return;

    priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    if (priv->notebook_general)
        priv->notebook_general = NULL;
}

/* HEADINGS */
/**
 * sensitive or unsensitive the headings
 *
 * \param sensitive TRUE to sensitive
 *
 * \return
 * */
void grisbi_win_headings_sensitive_headings (gboolean sensitive)
{
    GtkWidget *headings_eb;

    headings_eb = grisbi_win_get_headings_eb (NULL);
    gtk_widget_set_sensitive ( headings_eb, sensitive );
}

/**
 * Display or hide the headings bar depending on configuration.
 *
 * \param
 *
 * \return		FALSE
 */
gboolean grisbi_win_headings_update_show_headings (void)
{
    GtkWidget *headings_eb;

    headings_eb = grisbi_win_get_headings_eb (NULL);
    if (conf.show_headings_bar)
    {
        gtk_widget_show_all (headings_eb);
    }
    else
    {
        gtk_widget_hide (headings_eb);
    }
    return FALSE;
}

/**
 * Update headings bar with a new title.
 *
 * \param title		String to display as a title in headings bar.
 *
 */
void grisbi_win_headings_update_title (gchar *title)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

    grisbi_win_headings_private_update_label_markup (GTK_LABEL (priv->headings_title), title, TRUE);
}

/**
 * Update headings bar with a new suffix.
 *
 * \param suffix	String to display as a suffix in headings bar.
 *
 */
void grisbi_win_headings_update_suffix (gchar *suffix)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));
    grisbi_win_headings_private_update_label_markup (GTK_LABEL (priv->headings_suffix), suffix, FALSE);
}

/* STATUS_BAR */
/**
 * Clear any message in the status bar.
 *
 * \param
 *
 * \return
 **/
void grisbi_win_status_bar_clear (void)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

    if (!priv->statusbar || !GTK_IS_STATUSBAR (priv->statusbar))
        return;

    if (priv->message_id < G_MAXUINT)
    {
        gtk_statusbar_remove (GTK_STATUSBAR (priv->statusbar), priv->context_id, priv->message_id);
        priv->message_id = G_MAXUINT;
    }

    /* force status message to be displayed NOW */
    update_gui ();
}

/**
 * Display a message in the status bar.
 *
 * \param message	Message to display.
 *
 * \return
 **/
void grisbi_win_status_bar_message (gchar *message)
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (grisbi_app_get_active_window (NULL)));

    if (!priv->statusbar || !GTK_IS_STATUSBAR (priv->statusbar))
        return;

    if (priv->message_id < G_MAXUINT)
        gtk_statusbar_remove (GTK_STATUSBAR (priv->statusbar), priv->context_id, priv->message_id);

    priv->message_id = gtk_statusbar_push (GTK_STATUSBAR (priv->statusbar), priv->context_id, message);

    /* force status message to be displayed NOW */
    update_gui ();
}

/**
 * Change current cursor to a animated watch (if animation supported
 * by environment).
 *
 * \param force_update		Call a gtk iteration to ensure cursor
 *			                is updated.  May cause trouble if
 *				            called from some signal handlers.
 *
 * \return
 **/
void grisbi_win_status_bar_wait (gboolean force_update)
{
    GrisbiWin *win;
	GrisbiWinPrivate *priv;
    GdkDeviceManager *device_manager;
    GdkDisplay *display;
    GdkDevice *device;
    GdkWindow *current_window;
    GdkWindow *run_window;

    win = grisbi_app_get_active_window (NULL);
	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));

    run_window = gtk_widget_get_window (GTK_WIDGET (win));
    display = gdk_window_get_display (run_window);
    device_manager = gdk_display_get_device_manager (display);
    device = gdk_device_manager_get_client_pointer (device_manager);

    gdk_window_set_cursor (run_window, gdk_cursor_new_for_display (display, GDK_WATCH));

    current_window = gdk_device_get_window_at_position (device, NULL, NULL);

    if (current_window && GDK_IS_WINDOW (current_window) && current_window != run_window)
    {
        GdkWindow *parent = gdk_window_get_toplevel (current_window);

        if (parent && parent != current_window)
        {
            current_window = parent;
        }

        gdk_window_set_cursor (current_window,
                               gdk_cursor_new_for_display (gdk_display_get_default (),
                                                           GDK_WATCH));

        priv->tracked_window = current_window;
    }

    if (force_update)
        update_gui ();
}


/**
 * Change current cursor to default cursor.
 *
 * \param force_update		Call a gtk iteration to ensure cursor
 *				            is updated.  May cause trouble if
 *				            called from some signal handlers.
 *
 * \return
 **/
void grisbi_win_status_bar_stop_wait (gboolean force_update)
{
    GrisbiWin *win;
	GrisbiWinPrivate *priv;

    win = grisbi_app_get_active_window (NULL);
    if (!win)
        return;

	priv = grisbi_win_get_instance_private (GRISBI_WIN (win));
    if (priv->tracked_window && gdk_window_is_visible (priv->tracked_window))
    {
        gdk_window_set_cursor (priv->tracked_window, NULL);
        priv->tracked_window = NULL;
    }
    else
    gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (win)), NULL);

    if (force_update)
        update_gui ();
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
