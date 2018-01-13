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

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_display_fonts.h"
#include "accueil.h"
#include "custom_list.h"
#include "grisbi_settings.h"
#include "gsb_data_account.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_rgba.h"
#include "gsb_scheduler_list.h"
#include "gsb_select_icon.h"
#include "structures.h"
#include "transaction_list.h"
#include "utils_files.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDisplayFontsPrivate   PrefsPageDisplayFontsPrivate;

struct _PrefsPageDisplayFontsPrivate
{
	GtkWidget *			vbox_display_fonts;

    GtkWidget *			checkbutton_display_logo;
	GtkWidget *			eventbox_display_logo;
    GtkWidget *         hbox_display_logo;
    GtkWidget *         button_display_logo;
	GtkWidget *			preview_display_logo;
    GtkWidget *         vbox_display_logo;				/* sert à invalider le choix quand pas de fichier chargé */

    GtkWidget *			checkbutton_display_fonts;
	GtkWidget *			eventbox_display_fonts;
    GtkWidget *         hbox_display_fonts;

    GtkWidget *         button_select_colors;
    GtkWidget *         colorbutton_select_colors;
    GtkWidget *         grid_select_colors;

};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDisplayFonts, prefs_page_display_fonts, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * revert to default the selected color into the combobox
 *
 * \param button
 * \param combobox
 *
 * \return FALSE
 * */
static gboolean prefs_page_display_fonts_set_color_default (GtkWidget *button,
                                                            GtkWidget *combobox)
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
		GtkTreeModel *model;
		GdkRGBA *color;
		GdkRGBA *default_color;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &color, 2, &default_color, -1);
		if (color && default_color)
		{
			gboolean return_val;

			color = default_color;

			g_signal_emit_by_name (combobox, "changed", &return_val);

			/* update the colors in the list */
			transaction_list_redraw ();

			/* update scheduled list */
			gsb_scheduler_list_redraw ();
		}
    }
    return FALSE;
}

/**
 * called when the color combobox changed,
 * update the GtkColorButton with the color of the combobox
 *
 * \param combobox
 * \param color_button
 *
 * \return FALSE
 * */
static gboolean prefs_page_display_fonts_color_combobox_changed (GtkWidget *combobox,
																 GtkWidget *color_button)
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
		GtkTreeModel *model;
		GdkRGBA *color;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &color, -1);
		if (color)
			gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (color_button), color);
    }
    return FALSE;
}

/**
 * called when a color is chosen in the GtkColorButton,
 * update the color selected
 *
 * \param color_button
 * \param combobox
 *
 * \return FALSE
 * */
static gboolean prefs_page_display_fonts_view_color_changed (GtkWidget *color_button,
															 GtkWidget *combobox)
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
		GtkTreeModel *model;
		GdkRGBA *color;

		model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 1, &color, -1);
		if (color)
		{
			gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (color_button), color);

			/* update the colors in the transactions list */
			transaction_list_redraw ();

			/* update scheduled list */
			gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
			gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());
			gsb_scheduler_list_select (-1);
		}
    }
    return FALSE;
}

/**
 * update the font in all the transactions in the list
 *
 * \param
 * \param
 *
 * \return
 * */
static void prefs_page_display_fonts_update_fonte_listes (gchar *fontname,
                                                          gpointer null)
{
    GValue value = G_VALUE_INIT;
    gchar *font;

    devel_debug (NULL);

    if (conf.custom_fonte_listes)
		font = fontname;
    else
	{
		font = NULL;
		return;
	}

    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, font);
    transaction_list_update_column (CUSTOM_MODEL_FONT, &value);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 * */
static void prefs_page_display_fonts_checkbutton_checked (GtkWidget *checkbutton,
                                                          gboolean *value)
{
	GSettings *settings;
	gchar *tmp_str;

	settings = grisbi_settings_get_settings (SETTINGS_GENERAL);
	tmp_str = g_object_get_data (G_OBJECT (checkbutton), "custom-fonte-listes");
	g_settings_set_boolean (G_SETTINGS (settings), tmp_str, *value);
}

/**
 * update the preview of the logo file chooser
 *
 * \param file_chooser
 * \param preview
 *
 * \return FALSE
 * */
static void prefs_page_display_fonts_change_logo_accueil (GtkWidget *file_selector,
                                                          PrefsPageDisplayFonts *page)
{
	GtkWidget *logo_accueil;
	GtkWidget *preview;
    GdkPixbuf *pixbuf;
	const gchar *selected_filename;
	gchar *chemin_logo;
	PrefsPageDisplayFontsPrivate *priv;

	devel_debug (NULL);

	if (!gsb_data_account_get_accounts_amount ())
		return;

	priv = prefs_page_display_fonts_get_instance_private (page);
    selected_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_selector));

	/* on supprime d'abord l'ancien logo */
	gtk_widget_destroy (priv->preview_display_logo);

	/* on change le logo dans le bouton */
	chemin_logo = g_strstrip (g_strdup (selected_filename));
	logo_accueil = gsb_main_page_get_logo_accueil ();
	if (logo_accueil && GTK_IS_WIDGET (logo_accueil))
		gtk_widget_hide (logo_accueil);

	/* create logo */
	pixbuf = gdk_pixbuf_new_from_file (chemin_logo, NULL);
	if (!pixbuf)
	{
		pixbuf = gsb_select_icon_get_default_logo_pixbuf ();
	}
	else
	{
		if (g_strcmp0 (g_path_get_dirname (chemin_logo), gsb_dirs_get_pixmaps_dir ()) == 0)
		{
			gchar *name_logo;

			name_logo = g_path_get_basename (chemin_logo);
			if (g_strcmp0 (name_logo, "grisbi-logo.png") != 0)
				etat.name_logo = chemin_logo;
			else
				etat.name_logo = NULL;
		}
		else
		{
			if (etat.name_logo && strlen (etat.name_logo))
			{
				g_free (etat.name_logo);
				etat.name_logo = chemin_logo;
			}
		}
	}

	gsb_select_icon_set_logo_pixbuf (pixbuf);

	/* mis à jour du bouton contenant le logo */
	preview = gtk_image_new_from_pixbuf (gdk_pixbuf_scale_simple (pixbuf,
																  LOGO_WIDTH,
																  LOGO_HEIGHT,
																  GDK_INTERP_BILINEAR));
	gtk_widget_show (preview);
	gtk_container_add (GTK_CONTAINER (priv->button_display_logo), preview);
	priv->preview_display_logo = preview;

	/* Update homepage logo */
	gtk_widget_destroy (logo_accueil);

	logo_accueil = gtk_image_new_from_pixbuf (gdk_pixbuf_scale_simple (pixbuf,
																	   LOGO_WIDTH,
																	   LOGO_HEIGHT,
																	   GDK_INTERP_BILINEAR));
	gsb_main_page_set_logo_accueil (logo_accueil);

	/* modify the icon of grisbi (set in the panel of gnome or other) */
	gtk_window_set_default_icon (pixbuf);

	/* Mark file as modified */
	utils_prefs_gsb_file_set_modified ();
}


static void prefs_page_display_fonts_font_button_clicked (GtkWidget *button,
                                                          gchar *value)
{
	GSettings *settings;
	gchar *tmp_str;

	settings = grisbi_settings_get_settings (SETTINGS_GENERAL);
	tmp_str = g_object_get_data (G_OBJECT (button), "font-string");
	g_settings_set_string (G_SETTINGS (settings), tmp_str, conf.font_string);
}

/**
 * update the preview of the log file chooser
 *
 * \param file_chooser
 * \param preview
 *
 * \return FALSE
 * */
static gboolean prefs_page_display_fonts_update_preview_logo (GtkFileChooser *file_chooser,
															  GtkWidget *preview)
{
  gchar *filename;
  GdkPixbuf *pixbuf;
  gboolean have_preview;

  filename = gtk_file_chooser_get_preview_filename (file_chooser);
  if (!filename)
      return FALSE;

  pixbuf = gdk_pixbuf_new_from_file_at_size (filename, LOGO_WIDTH, LOGO_HEIGHT, NULL);
  have_preview = (pixbuf != NULL);
  g_free (filename);

  gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
  if (pixbuf)
    g_object_unref (pixbuf);

  gtk_file_chooser_set_preview_widget_active (file_chooser, have_preview);

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 * */
static gboolean prefs_page_display_fonts_logo_accueil_changed (PrefsPageDisplayFonts *page)
{
	GtkWidget *prefs_dialog;
    GtkWidget *file_selector;
    GtkWidget *preview;
    gchar *tmp_last_directory;

	prefs_dialog = grisbi_win_get_prefs_dialog (NULL);
    file_selector = gtk_file_chooser_dialog_new (_("Select a new logo"),
												 GTK_WINDOW (prefs_dialog),
												 GTK_FILE_CHOOSER_ACTION_OPEN,
												 "gtk-cancel", GTK_RESPONSE_CANCEL,
												 "gtk-open", GTK_RESPONSE_OK,
												 NULL);

    if (etat.name_logo)
	{
		gchar *dirname;

		dirname = g_path_get_dirname (etat.name_logo);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_selector), dirname);
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_selector), etat.name_logo);
	}
    else
	{
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_selector),
											 gsb_dirs_get_pixmaps_dir ());
	}

    gtk_window_set_position (GTK_WINDOW (file_selector), GTK_WIN_POS_CENTER_ON_PARENT);

    /* create the preview */
    preview = gtk_image_new ();
    gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (file_selector), preview);
    g_signal_connect (G_OBJECT (file_selector),
                        "update-preview",
                        G_CALLBACK (prefs_page_display_fonts_update_preview_logo),
                        preview);

    switch (gtk_dialog_run (GTK_DIALOG (file_selector)))
    {
		case GTK_RESPONSE_OK:
			prefs_page_display_fonts_change_logo_accueil (file_selector, page);
			tmp_last_directory = utils_files_selection_get_last_directory (GTK_FILE_CHOOSER (file_selector), TRUE);
			gsb_file_update_last_path (tmp_last_directory);
			g_free (tmp_last_directory);

		default:
			break;
    }

	gtk_widget_destroy (file_selector);

	return (FALSE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 * */
static gboolean prefs_page_display_fonts_utilise_logo_checked (GtkWidget *check_button,
															   GtkWidget *hbox)
{
    etat.utilise_logo = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button));
    gtk_widget_set_sensitive (hbox, etat.utilise_logo);

    if (etat.utilise_logo)
    {
		GtkWidget *logo_accueil;

		/* 	on recharge l'ancien logo */
		logo_accueil = gsb_main_page_get_logo_accueil ();
        if (GTK_IS_WIDGET (logo_accueil))
		{
            gtk_widget_hide (logo_accueil);
		}
        else
        {
            GdkPixbuf *pixbuf = NULL;

            /* Update homepage logo */
            pixbuf = gsb_select_icon_get_logo_pixbuf ();
            if (pixbuf == NULL)
            {
                pixbuf = gsb_select_icon_get_default_logo_pixbuf ();
            }
            logo_accueil =  gtk_image_new_from_pixbuf (pixbuf);
            if (logo_accueil)
				gsb_main_page_set_logo_accueil (logo_accueil);
			else
				gsb_main_page_set_logo_accueil (NULL);
        }
    }
	else
		gsb_main_page_set_logo_accueil (NULL);

    utils_prefs_gsb_file_set_modified ();

    return (FALSE);
}

/**
 * Création de la page de gestion des display_fonts
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_display_fonts_setup_display_fonts_page (PrefsPageDisplayFonts *page)
{
	GtkWidget *head_page;
	GtkWidget *preview;
	GtkWidget *font_button;
	GdkPixbuf * pixbuf = NULL;
	GtkWidget *combobox_select_colors;
	gboolean is_loading;
	PrefsPageDisplayFontsPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_fonts_get_instance_private (page);
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Fonts & logo"), "gsb-fonts-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_display_fonts), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_display_fonts), head_page, 0);

    /* set the elements for logo */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_display_logo),
								  etat.utilise_logo);

	/* le logo est grisé ou non suivant qu'on l'utilise ou pas */
    gtk_widget_set_sensitive (priv->hbox_display_logo, etat.utilise_logo);

	/* set the logo */
	pixbuf = gsb_select_icon_get_logo_pixbuf ();

    if (!pixbuf)
    {
        preview = gtk_image_new_from_pixbuf (gsb_select_icon_get_default_logo_pixbuf ());
    }
    else
    {
		preview = gtk_image_new_from_pixbuf (pixbuf);
    }
	priv->preview_display_logo = preview;

	gtk_container_add (GTK_CONTAINER (priv->button_display_logo), preview);
    g_signal_connect_swapped (G_OBJECT (priv->button_display_logo),
							  "clicked",
							  G_CALLBACK (prefs_page_display_fonts_logo_accueil_changed),
							  page);

    /* Connect signal */
    g_signal_connect (priv->eventbox_display_logo,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_display_logo);

    g_signal_connect (priv->checkbutton_display_logo,
					  "toggled",
					  G_CALLBACK (prefs_page_display_fonts_utilise_logo_checked),
					  priv->hbox_display_logo);

	/* set the elements for fonts */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_display_fonts),
								  conf.custom_fonte_listes);


	/* Connect signal */
    g_signal_connect (priv->eventbox_display_fonts,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_display_fonts);

	/* Create font button */
    font_button = utils_prefs_fonts_create_button (&conf.font_string,
                                                   G_CALLBACK (prefs_page_display_fonts_update_fonte_listes),
                                                   NULL);
	g_object_set_data (G_OBJECT (priv->checkbutton_display_fonts), "widget", font_button);
    gtk_box_pack_start (GTK_BOX (priv->hbox_display_fonts), font_button, FALSE, FALSE, 0);

    if (!conf.custom_fonte_listes)
    {
        gtk_widget_set_sensitive (font_button, FALSE);
    }
	else
		gtk_widget_set_sensitive (font_button, TRUE);

	g_signal_connect (priv->checkbutton_display_fonts,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.custom_fonte_listes);

	/* set g_settings for fonts */
	g_signal_connect_after (priv->checkbutton_display_fonts,
							"toggled",
							G_CALLBACK (prefs_page_display_fonts_checkbutton_checked),
							&conf.custom_fonte_listes);

	g_object_set_data (G_OBJECT (priv->checkbutton_display_fonts), "custom-fonte-listes", "custom-fonte-listes");

	g_signal_connect_after (font_button,
							"clicked",
							G_CALLBACK (prefs_page_display_fonts_font_button_clicked),
							&conf.font_string);
	g_object_set_data (G_OBJECT (font_button), "font-string", "font-string");

	/* set the elements for colors */
	combobox_select_colors = gsb_rgba_create_color_combobox ();
    gtk_widget_set_margin_end (combobox_select_colors, MARGIN_END);
    gtk_grid_attach (GTK_GRID (priv->grid_select_colors), combobox_select_colors, 0, 0, 1, 1);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox_select_colors), 0);
	gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (priv->colorbutton_select_colors),
								gsb_rgba_get_couleur_with_indice ("couleur_fond", 0));

	/* Connect signal */
    g_signal_connect (G_OBJECT (priv->colorbutton_select_colors),
					  "color-set",
					  G_CALLBACK (prefs_page_display_fonts_view_color_changed),
					  combobox_select_colors);

	g_signal_connect (G_OBJECT (combobox_select_colors),
					  "changed",
					  G_CALLBACK (prefs_page_display_fonts_color_combobox_changed),
					  priv->colorbutton_select_colors);

	g_signal_connect (G_OBJECT (priv->button_select_colors),
					  "clicked",
					  G_CALLBACK (prefs_page_display_fonts_set_color_default),
					  combobox_select_colors);

	if (is_loading == FALSE)
	{
		gtk_widget_set_sensitive (priv->vbox_display_logo, FALSE);
		gtk_widget_set_sensitive (priv->grid_select_colors, FALSE);
	}
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_display_fonts_init (PrefsPageDisplayFonts *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_display_fonts_setup_display_fonts_page (page);
}

static void prefs_page_display_fonts_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_display_fonts_parent_class)->dispose (object);
}

static void prefs_page_display_fonts_class_init (PrefsPageDisplayFontsClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_display_fonts_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_display_fonts.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, vbox_display_fonts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, checkbutton_display_logo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, eventbox_display_logo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, hbox_display_logo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, vbox_display_logo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, button_display_logo);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, checkbutton_display_fonts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, eventbox_display_fonts);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, hbox_display_fonts);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, button_select_colors);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, colorbutton_select_colors);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayFonts, grid_select_colors);
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
PrefsPageDisplayFonts *prefs_page_display_fonts_new (GrisbiPrefs *win)
{
	return g_object_new (PREFS_PAGE_DISPLAY_FONTS_TYPE, NULL);
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

