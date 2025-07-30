/* ************************************************************************** */
/*                                                                            */
/*    copyright (c) 2000-2008 Cédric Auger (cedric@grisbi.org)                */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*                  2008-2009 Pierre Biava (grisbi@pierre.biava.name)         */
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
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#include "config.h"

#include "include.h"
#include <glib/gi18n.h>
#include <sys/stat.h>
#include <glib/gstdio.h>

 /*START_INCLUDE*/
#include "gsb_select_icon.h"
#include "utils_files.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "utils_str.h"
#include "structures.h"
#include "utils.h"
#include "erreur.h"
#include "gsb_dirs.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *dialog;
static GtkWidget *entry_text;
static GtkWidget *icon_view;
static GdkPixbuf *pixbuf_logo = NULL;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


static GtkListStore *store = NULL;

static gchar *path_icon;
static gchar *new_icon;

enum {
        PIXBUF_COLUMN,
        TEXT_COLUMN,
        FILENAME_COLUMN	/* On conserve le nom du fichier de l'icône */
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * callback pour traiter les changements de sélection dans le GtkIconView
 *
 * \param le GtkIconView appellant
 * \param
 *
 * \return
 *
 **/
static void gsb_select_icon_selection_changed (GtkIconView *view,
											   GtkWidget *bouton_OK)
{
    GList *liste;
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *name_icon = NULL;

	liste = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (view));

    /* Could happen if selection is unset, exiting then. */
    if (! liste)
		return;

    path = liste->data;

    model = gtk_icon_view_get_model (GTK_ICON_VIEW (view));
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path))
        gtk_tree_model_get (model, &iter, FILENAME_COLUMN, &name_icon, -1);
    devel_debug (name_icon);
    if (name_icon && strlen (name_icon) > 0)
    {
        new_icon = g_strdup (name_icon);
        gtk_entry_set_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (entry_text))), new_icon);
        gtk_widget_set_sensitive (bouton_OK, TRUE);
        g_free (name_icon);
    }
	g_list_free_full (liste, (GDestroyNotify) gtk_tree_path_free);
}

/**
 * coupe le nom des fichiers pour garder un même nombre de colonnes
 * dans le  GtkIconView quelque soit la longueur du nom
 *
 * \param nom de l'icône et la longueur maxi de la ligne
 * \param
 *
 * \return le nom de l'icône sur une ou plusieurs lignes
 **/
static gchar *gsb_select_icon_troncate_name_icon (gchar *name_icon,
												  gint trunc)
{
    glong size = g_utf8_strlen (name_icon, -1);

    if (size > 10)
    {
        gchar *tmpstr, *tmpstr2 = NULL;
        gchar *end;
        gchar *ptr = NULL;
        gint i = 1;
        glong n = 0;

        n = size / trunc;
        if ((size % trunc) == 0)
			n--;

        tmpstr = g_malloc (size + n + 1);
        /* devel_debug_int (n);
        devel_debug (name_icon); */
        tmpstr = g_utf8_strncpy (tmpstr, name_icon, trunc);
        do
        {
            end = g_utf8_offset_to_pointer (name_icon, (i *trunc));
            /* devel_debug (end); */
            if (i < n)
                ptr = g_utf8_offset_to_pointer (name_icon, (i + 1) *10);
            if (ptr)
			{
				gchar *end2;
				end2 = g_strndup (end, ptr - end);
				g_free(tmpstr2);
                tmpstr2 = g_strconcat (tmpstr, "\n",
									   end2, NULL);
				g_free(end2);
			}
            else
			{
				g_free(tmpstr2);
                tmpstr2 = g_strconcat (tmpstr, "\n", end, NULL);
			}
            ptr = NULL;
            i++;
        } while (i <= n);

        g_free (tmpstr);
        return tmpstr2;
    }
    else
        return g_strdup (name_icon);
}

/**
 * remplit le modèle qu'il crée et attache au GtkIconView
 *
 * \param nom de l'icône initiale ou NULL
 *
 * \return un GtkTreePath qui donne la position de l'icône passée
 * en paramètre
 **/
static GtkTreePath *gsb_select_icon_fill_icon_view (const gchar *name_icon)
{
    GDir *dir;
    GError *error = NULL;
    GtkTreePath *tree_path = NULL;

    devel_debug (path_icon);

    dir = g_dir_open (path_icon, 0, &error);
    if (dir)
    {
        GtkListStore *list_store;
        GtkTreeIter iter;
        GdkPixbuf *pixbuf;
        GSList *liste = NULL;
        gint i = 0;
        const gchar *name = NULL;

        while ((name = g_dir_read_name (dir)))
        {
            liste = g_slist_append (liste, g_strdup (name));
        }
		//~ printf ("nbre elements = %u\n", g_slist_length (liste));
        liste = g_slist_sort (liste, (GCompareFunc) my_strcasecmp);

        list_store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
        while (liste)
        {
			gchar *tmp_filename;

            tmp_filename = g_strconcat (path_icon, G_DIR_SEPARATOR_S, liste->data, NULL);
            if (g_strcmp0 (tmp_filename, name_icon) == 0)
            {
				gchar *tmp_str;

				tmp_str = utils_str_itoa (i);
                tree_path = gtk_tree_path_new_from_string (tmp_str);
                g_free (tmp_str);
            }

            pixbuf = gdk_pixbuf_new_from_file_at_size (tmp_filename, 32, 32, NULL);
            if (pixbuf)
            {
                gchar *tmp_str;

                gtk_list_store_append (list_store, &iter);
                tmp_str = gsb_select_icon_troncate_name_icon (liste->data, 10);
                gtk_list_store_set (list_store, &iter, PIXBUF_COLUMN, pixbuf, TEXT_COLUMN, tmp_str, -1);
                gtk_list_store_set (list_store, &iter,
									PIXBUF_COLUMN, pixbuf,
									TEXT_COLUMN, tmp_str,
									FILENAME_COLUMN, tmp_filename,
									-1);
                g_free (tmp_str);
                g_object_unref (pixbuf);
            }

            liste = liste->next;
			if (!g_file_test (tmp_filename, G_FILE_TEST_IS_DIR))
				i++;
			g_free (tmp_filename);
        }
        gtk_icon_view_set_model (GTK_ICON_VIEW (icon_view), GTK_TREE_MODEL (list_store));
        g_object_unref (G_OBJECT (list_store));
        g_dir_close (dir);
    }
    else
    {
        dialogue_error (error->message);
        g_error_free (error);
    }

	if (tree_path == NULL)
        tree_path = gtk_tree_path_new_from_string ("0");

    return tree_path;
}

/**
 * ajoute le nouveau path. S'il existe déjà dans la liste on le supprime
 * de telle façon qu'il n'existe qu'une fois et apparaisse en premier dans
 * la liste
 *
 * \param néant
 *
 * \return TRUE
 **/
static gboolean gsb_select_icon_add_path (void)
{
    GtkTreeIter iter;
    gboolean result = FALSE;

    result = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store),
                                &iter);

    while (result)
    {
        gchar *rep;

        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &rep, -1);
        if (strcmp (path_icon, rep) == 0)
        {
            gtk_list_store_remove (store, &iter);
            break;
        }
        result = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);
    }
    gtk_list_store_prepend (store, &iter);
    gtk_list_store_set (store, &iter, 0, path_icon, -1);
    devel_debug (path_icon);

    return TRUE;
}

/**
 * Crée le dialogue pour le choix du nouveau répertoire et entre le choix
 * dans le GtkComboBoxEntry
 *
 * \param bouton appelant
 * \param
 *
 * \return void
 *
 **/
static void gsb_select_icon_create_file_chooser (GtkWidget *button,
												 GtkWidget *bouton_OK)
{
    GtkWidget *chooser;
	GtkWidget *button_cancel;
	GtkWidget *button_open;

 	devel_debug (NULL);
	chooser = gtk_file_chooser_dialog_new (_("Select icon directory"),
										   GTK_WINDOW (dialog),
										   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
										   NULL, NULL,
										   NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (chooser), button_cancel, GTK_RESPONSE_CANCEL);
	gtk_widget_set_can_default (button_cancel, TRUE);

	button_open = gtk_button_new_with_label (_("Open"));
	gtk_dialog_add_action_widget (GTK_DIALOG (chooser), button_open, GTK_RESPONSE_ACCEPT);
	gtk_widget_set_can_default (button_open, TRUE);

	gtk_window_set_position (GTK_WINDOW (chooser), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_transient_for (GTK_WINDOW (chooser), GTK_WINDOW (dialog));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (chooser), TRUE);
    gtk_widget_set_size_request (chooser, 600, 750);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), path_icon);

	gtk_widget_show_all (chooser);

    if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {
        GtkTreePath *path;

        path_icon = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
        path = gsb_select_icon_fill_icon_view (NULL);
        gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (icon_view),
                            path, TRUE, 0.5, 0);
        gsb_select_icon_add_path ();
        gtk_entry_set_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (entry_text))), path_icon);
        gtk_widget_set_sensitive (bouton_OK, FALSE);
    }

    gtk_widget_destroy (chooser);
}

/**
 * callback pour traiter les changements dans le GtkComboBoxEntry
 *
 * \param le GtkComboBoxEntry appellant
 *
 * \return void
 *
 **/
static void gsb_select_icon_entry_text_changed (GtkComboBox *entry,
												GtkWidget *bouton_OK)
{
    GtkTreePath *path;
    const gchar *tmp_str;
    gchar *ptr;

    tmp_str = gtk_entry_get_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (entry))));
    devel_debug (tmp_str);

    ptr = g_strstr_len (tmp_str, -1, path_icon);
    if (ptr == NULL)
    {
        if (g_file_test (tmp_str, G_FILE_TEST_IS_DIR))
        {
            path_icon = g_strdup (tmp_str);
            path = gsb_select_icon_fill_icon_view (NULL);
            gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (icon_view), path, TRUE, 0.5, 0);

            gtk_widget_set_sensitive (bouton_OK, FALSE);
        }
    }
}

/**
 * crée le GtkComboBox pour l'entrée d'un nom de fichier ou de répertoire
 * mémorise la liste des répertoires utilisés
 *
 * \param nom de l'icône
 *
 * \return  le GtkComboBox
 **/
static GtkWidget *gsb_select_icon_create_entry_text (const gchar *name_icon)
{
    GtkWidget *combo;
    GtkTreeIter iter;

    if (!store)
    {
        store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
        gtk_list_store_append (store, &iter);
        if (g_strcmp0 (gsb_dirs_get_pixmaps_dir (), path_icon) != 0)
        {
            gtk_list_store_set (store, &iter, 0, gsb_dirs_get_pixmaps_dir (), -1);
            gtk_list_store_prepend (store, &iter);
        }
        gtk_list_store_set (store, &iter, 0, path_icon, -1);
    }

    combo = gtk_combo_box_new_with_model_and_entry (GTK_TREE_MODEL (store));
    gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (combo), 0);

    gtk_entry_set_text (GTK_ENTRY (gtk_bin_get_child (GTK_BIN (combo))), name_icon);

    return combo;
}

/**
 * Crée le GtkIconView
 *
 * \param nom de l'icône
 *
 * \return le GtkIconView rempli avec l'icône sélectionnée au premier plan
 *
 **/
static GtkWidget *gsb_select_icon_create_icon_view (const gchar *name_icon)
{
    GtkTreePath *tree_path;

    /* construct the GtkIconView */
    icon_view = gtk_icon_view_new ();
    gtk_icon_view_set_margin (GTK_ICON_VIEW (icon_view), 0);
    gtk_icon_view_set_spacing (GTK_ICON_VIEW (icon_view), 0);
    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view), GTK_SELECTION_SINGLE);
    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view), PIXBUF_COLUMN);
    gtk_icon_view_set_text_column (GTK_ICON_VIEW (icon_view), TEXT_COLUMN);

    /* remplissage et positionnement initial du curseur dans le GtkIconView */
    tree_path = gsb_select_icon_fill_icon_view (name_icon);

    gtk_icon_view_select_path (GTK_ICON_VIEW (icon_view), tree_path);
    gtk_icon_view_set_cursor (GTK_ICON_VIEW (icon_view), tree_path,
                            NULL, FALSE);
    gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (icon_view),
                            tree_path, TRUE, 0.5, 0);

    return icon_view;
}

/**
 * redimmensionne le logo
 *
 * \param pixbuf à redimmensionner
 *
 * return a new pixbuf
 **/
static GdkPixbuf *gsb_select_icon_resize_logo_pixbuf (GdkPixbuf *pixbuf)
{
    GdkPixbuf *tmp;

    if (!pixbuf)
        return NULL;

    tmp = gdk_pixbuf_scale_simple (pixbuf, LOGO_WIDTH, LOGO_HEIGHT, GDK_INTERP_HYPER);

    return tmp;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * crée la boite de dialogue initiale avec le  GtkIconView
 *
 * \param nom de l'icône
 *
 * \return le nouveau nom de l'icône ou NULL
 **/
gchar *gsb_select_icon_create_window (const gchar *name_icon)
{
	GtkWidget *bouton_cancel;
	GtkWidget *bouton_OK;
    GtkWidget *content_area;
    GtkWidget *hbox;
    GtkWidget *chooser_button;
    GtkWidget *scroll;
    GtkWidget *view;
	gint result;

    devel_debug (name_icon);

    if (new_icon && strlen (new_icon) > 0)
        g_free (new_icon);

    new_icon = g_strdup (name_icon);

    if (path_icon && strlen (path_icon) > 0)
        g_free (path_icon);

    path_icon = g_path_get_dirname (name_icon);
    dialog = gtk_dialog_new ();

	gtk_window_set_title (GTK_WINDOW (dialog), _("Browse icons"));
	gtk_window_set_transient_for (GTK_WINDOW (dialog),
								  GTK_WINDOW (grisbi_app_get_active_window (NULL)));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	bouton_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), bouton_cancel, GTK_RESPONSE_REJECT);
	gtk_widget_set_can_default (bouton_cancel, TRUE);

	bouton_OK = gtk_button_new_with_label (_("Validate"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), bouton_OK, GTK_RESPONSE_ACCEPT);
	gtk_widget_set_can_default (bouton_OK, TRUE);

    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

    /* création hbox pour GtkEntry répertoire et bouton sélection des répertoires */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), 6);
    gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 5);

    /* création du GtkComboBoxEntry pour la saisie du répertoire */
    entry_text = gsb_select_icon_create_entry_text (name_icon);
    gtk_box_pack_start (GTK_BOX (hbox), entry_text, TRUE, TRUE, 0);

    /* création du bouton de sélection des répertoires */
    chooser_button = gtk_button_new_with_label (_("Browse"));
    gtk_box_pack_start (GTK_BOX (hbox), chooser_button, FALSE, FALSE, 0);

    /* création de la vue pour les icônes */
    scroll = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                             GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (content_area), scroll, TRUE, TRUE, 0);
    view = gsb_select_icon_create_icon_view (name_icon);
    gtk_container_set_border_width (GTK_CONTAINER(scroll), 6);
    gtk_container_add (GTK_CONTAINER (scroll), view);

    /* limitation de la fenêtre */
	gtk_window_set_default_size (GTK_WINDOW (dialog), 480, 550);

    /* gestion des signaux */
    g_signal_connect (G_OBJECT (view),
                        "selection-changed",
                        G_CALLBACK(gsb_select_icon_selection_changed),
                        bouton_OK);

    g_signal_connect (G_OBJECT (chooser_button),
                        "clicked",
                        G_CALLBACK(gsb_select_icon_create_file_chooser),
                        bouton_OK);

    g_signal_connect (G_OBJECT(entry_text),
                        "changed",
                        G_CALLBACK(gsb_select_icon_entry_text_changed),
                        bouton_OK);

    gtk_widget_show_all (dialog);

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    switch (result)
    {
      case GTK_RESPONSE_ACCEPT:
         devel_debug ("réponse OK");
         break;
      default:
         devel_debug ("réponse Non OK");
         if (new_icon && strlen (new_icon) > 0)
            g_free (new_icon);
         new_icon = NULL;
         break;
    }
    gtk_widget_destroy (dialog);

    return new_icon;
}

/**
 * retourne le logo par défaut de grisbi
 *
 * \param
 *
 * return a new pixbuf
 **/
GdkPixbuf *gsb_select_icon_get_default_logo_pixbuf (void)
{
    GdkPixbuf *pixbuf = NULL;
    GError *error = NULL;
	gchar *filename;

	filename = g_build_filename (gsb_dirs_get_pixmaps_dir (), "grisbi.svg", NULL);
    pixbuf = gdk_pixbuf_new_from_file_at_scale (filename, LOGO_WIDTH, LOGO_HEIGHT, FALSE, &error);
	g_free (filename);

    if (!pixbuf)
    {
        g_error_free (error);
		return NULL;
    }
	else
        return pixbuf;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GdkPixbuf *gsb_select_icon_get_logo_pixbuf (void)
{
    return pixbuf_logo;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_select_icon_init_logo_variables (void)
{
    if (pixbuf_logo)
		g_object_unref (pixbuf_logo);
    pixbuf_logo = NULL;

    return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
GdkPixbuf *gsb_select_icon_new_account_pixbuf_from_file (const gchar *filename)
{
    GdkPixbuf *pixbuf;

    pixbuf = gdk_pixbuf_new_from_file_at_size (filename , 32, 32, NULL);
	if (pixbuf)
	{
		return pixbuf;
	}
	else
	{
		return NULL;
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_select_icon_set_gtk_icon_theme_path (void)
{
	GtkIconTheme *icon_theme;

	icon_theme = gtk_icon_theme_get_default ();
	gtk_icon_theme_append_search_path (icon_theme, gsb_dirs_get_pixmaps_dir ());
	gtk_icon_theme_add_resource_path (icon_theme, "/org/gtk/grisbi/images");
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_select_icon_set_logo_pixbuf (GdkPixbuf *pixbuf)
{
    if (pixbuf_logo != NULL)
        g_object_unref (G_OBJECT (pixbuf_logo));

    if (gdk_pixbuf_get_width (pixbuf) > LOGO_WIDTH ||
	     gdk_pixbuf_get_height (pixbuf) > LOGO_HEIGHT)
	{
        pixbuf_logo = gsb_select_icon_resize_logo_pixbuf (pixbuf);
	}
    else
    {
        pixbuf_logo = pixbuf ;
        g_object_ref (G_OBJECT (pixbuf_logo));
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *gsb_select_icon_set_icon_in_user_icons_dir (const gchar *icon_name)
{
	GFile *source;
	GFile *destination;
	gchar *icon_basename;
	const gchar *icon_dir;
	gchar *new_icon_name;

	icon_dir = gsb_dirs_get_user_icons_dir ();
	if (!g_file_test (icon_dir, G_FILE_TEST_IS_DIR))
	{
		int mode = S_IRUSR | S_IWUSR | S_IXUSR;

		g_mkdir (icon_dir, mode);
	}

	icon_basename = g_path_get_basename (icon_name);
	new_icon_name = g_build_filename (gsb_dirs_get_user_icons_dir (), icon_basename, NULL);

	source = g_file_new_for_path (icon_name);
	destination = g_file_new_for_path (new_icon_name);

	if (g_file_copy (source, destination, G_FILE_COPY_OVERWRITE, FALSE, NULL, NULL, NULL))
	{
		g_free (icon_basename);
		g_object_unref (source);
		g_object_unref (destination);

		return new_icon_name;
	}
	else
	{
		g_free (icon_basename);
		g_free (new_icon_name);
		g_object_unref (source);
		g_object_unref (destination);

		return NULL;
	}
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
