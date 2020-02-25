/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
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

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_msg_warning.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "gsb_file.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageMsgWarningPrivate   PrefsPageMsgWarningPrivate;

struct _PrefsPageMsgWarningPrivate
{
	GtkWidget *			vbox_msg_warning;

    GtkWidget *			checkbutton_show_tip;
	GtkWidget *			grid_display_msg;
    GtkWidget *         sw_display_msg;
    GtkWidget *         treeview_display_msg;

};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageMsgWarning, prefs_page_msg_warning, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean prefs_page_msg_warning_msg_toggled (GtkCellRendererToggle *cell,
													 gchar *path_str,
													 GtkTreeModel *model)
{
    GtkTreePath *path;
    GtkTreeIter iter;
	GSettings *settings;
	gint position;
	ConditionalMsg *warning;

	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
	path = gtk_tree_path_new_from_string (path_str);

	/* Get toggled iter */
    gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, 2, &position, -1);

    (warning+position)->hidden = !(warning+position)->hidden;

    /* Set new value */
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 0, !(warning+position)->hidden, -1);

	/* set g_settings */
	settings = grisbi_settings_get_settings (SETTINGS_MESSAGES_WARNINGS);
	if ((warning+position)->hidden)
		g_settings_set_boolean (G_SETTINGS (settings),
								(warning+position)->name,
								FALSE);
	else
		g_settings_reset (G_SETTINGS (settings),
						  (warning+position)->name);

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_msg_warning_fill_model (GtkTreeModel *model)
{
	gchar *tmp_str;
	gint i;
	gboolean is_loading;
	ConditionalMsg *warning;

	warning = (ConditionalMsg*) dialogue_get_tab_warning_msg ();
	is_loading = grisbi_win_file_is_loading ();

	for  (i = 0; (warning+i)->name; i++)
	{
		GtkTreeIter iter;

		if (g_utf8_collate ((warning+i)->name, "account-already-opened") == 0)
		{
			/* on récupère le nom du fichier si un fichier est chargé */
			if (is_loading)
			{
				gchar *filename;

				filename = g_path_get_basename (grisbi_win_get_filename (NULL));
				tmp_str = g_strdup_printf (_((warning+i)->hint), filename);
				g_free (filename);
			}
			else
			{
				tmp_str = g_strdup (_("No file loading"));
			}
		}
		else if (g_utf8_collate ((warning+i)->name, "development-version") == 0)
			tmp_str = g_strdup_printf (_((warning+i)->hint), VERSION);
		else
			tmp_str = g_strdup (_((warning+i)->hint));

		gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&iter,
							0, !(warning+i)->hidden,
							1, tmp_str,
							2, i,
							-1);

		g_free (tmp_str);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *prefs_page_msg_warning_create_tree_view (PrefsPageMsgWarning *page)
{
	GtkWidget *treeview;
	GtkTreeModel *model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;

    /* create the model */
    model = GTK_TREE_MODEL(gtk_tree_store_new (4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, GDK_TYPE_RGBA));

    /* create the treeview */
    treeview = gtk_tree_view_new();
	gtk_widget_set_name (treeview, "gsettings_tree_view");
    gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (model));
    g_object_unref (G_OBJECT(model));

    cell = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes ("", cell, "active", 0, "cell-background-rgba", 3, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (column));
    g_signal_connect (cell,
					  "toggled",
					  G_CALLBACK (prefs_page_msg_warning_msg_toggled),
					  model);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Message"),
													   cell,
													   "text", 1,
													   "cell-background-rgba", 3,
													   NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), GTK_TREE_VIEW_COLUMN (column));

	/* set and fill the model */
	prefs_page_msg_warning_fill_model (model);

	/* set column color */
	utils_set_tree_store_background_color (treeview, 3);

	return treeview;
}

/**
 * Création de la page de gestion des msg_warning
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_msg_warning_setup_page (PrefsPageMsgWarning *page)
{
	GtkWidget *head_page;
	PrefsPageMsgWarningPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_msg_warning_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Messages & warnings"), "gsb-warnings-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_msg_warning), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_msg_warning), head_page, 0);

    /* set variable for checkbutton_show_tip */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_show_tip), conf.show_tip);

	/* set sw widget */
	priv->sw_display_msg = utils_prefs_scrolled_window_new (NULL, GTK_SHADOW_IN, SW_COEFF_UTIL_PG, 400);
    gtk_grid_attach (GTK_GRID (priv->grid_display_msg), priv->sw_display_msg, 0, 0, 1, 1);

	/* set treeview */
	priv->treeview_display_msg = prefs_page_msg_warning_create_tree_view (page);
	gtk_container_add (GTK_CONTAINER (priv->sw_display_msg), priv->treeview_display_msg);

	/* Connect signal */
    g_signal_connect (priv->checkbutton_show_tip,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.show_tip);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_msg_warning_init (PrefsPageMsgWarning *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_msg_warning_setup_page (page);
}

static void prefs_page_msg_warning_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_msg_warning_parent_class)->dispose (object);
}

static void prefs_page_msg_warning_class_init (PrefsPageMsgWarningClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_msg_warning_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_msg_warning.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMsgWarning, vbox_msg_warning);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMsgWarning, checkbutton_show_tip);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageMsgWarning, grid_display_msg);
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
PrefsPageMsgWarning *prefs_page_msg_warning_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_MSG_WARNING_TYPE, NULL);
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
