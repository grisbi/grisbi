/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
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
#include "prefs_page_display_gui.h"
#include "grisbi_app.h"
#include "grisbi_settings.h"
#include "navigation.h"
#include "structures.h"
#include "utils_buttons.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDisplayGuiPrivate   PrefsPageDisplayGuiPrivate;

struct _PrefsPageDisplayGuiPrivate
{
	GtkWidget *			vbox_display_gui;

    GtkWidget *			checkbutton_show_headings_bar;

	GtkWidget *			checkbutton_active_scrolling_left_pane;
	GtkWidget *			checkbutton_low_resolution_screen;

    GtkWidget *			radiobutton_display_both;
    GtkWidget *			radiobutton_display_both_horiz;
    GtkWidget *			radiobutton_display_icon;
    GtkWidget *			radiobutton_display_text;

	GtkWidget *			text_view_display_shorcuts;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDisplayGui, prefs_page_display_gui, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * called when switch the active mouse scrolling option
 * to set unset the scrolling
 *
 * \param toggle button
 * \param null
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_gui_active_scrolling_left_pane (GtkWidget *toggle_button,
																   GrisbiAppConf *a_conf)
{
	GSettings *settings;

	if (a_conf->active_scrolling_left_pane)
        g_signal_handlers_unblock_by_func (gsb_gui_navigation_get_tree_view (),
										   G_CALLBACK (gsb_gui_navigation_check_scroll),
										   NULL);
    else
        g_signal_handlers_block_by_func (gsb_gui_navigation_get_tree_view (),
										 G_CALLBACK (gsb_gui_navigation_check_scroll),
										 NULL);

	settings = grisbi_settings_get_settings (SETTINGS_PANEL);
    g_settings_set_boolean (G_SETTINGS (settings),
							"active-scrolling-left-pane",
							a_conf->active_scrolling_left_pane);

	return FALSE;
}
/**
 * Signal triggered when user configure display mode of toolbar
 * buttons.
 *
 * \param button	Radio button that triggered event.
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_gui_change_toolbar_display_mode (GtkRadioButton *button,
																	GrisbiAppConf *a_conf)
{
    GSettings *settings;
    const gchar *tmp_str;

    /* Do not execute this callback twice,
     * as it is triggered for both unselected button and newly selected one.
     * We keep the call for the newly selected radio button */
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
        return FALSE;

    /* save the new parameter */
    a_conf->display_toolbar = GPOINTER_TO_INT (g_object_get_data (G_OBJECT(button), "display"));

    /* update GSettings */
    switch (a_conf->display_toolbar)
    {
        case GSB_BUTTON_TEXT:
            tmp_str = "Text";
            break;
        case GSB_BUTTON_ICON:
            tmp_str = "Icons";
            break;
        default:
            tmp_str = "Text + Icons";
    }
    settings = grisbi_settings_get_settings (SETTINGS_DISPLAY);
    g_settings_set_string (G_SETTINGS (settings),
						   "display-toolbar",
						   tmp_str);

    /* update toolbars */
	if (grisbi_win_file_is_loading ())
    	grisbi_win_update_all_toolbars ();

    return FALSE;
}

/**
 * called when check the preference low resoltion screen button
 *
 * \param toggle button
 * \param null
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_gui_resolution_screen_toggled (GtkWidget *toggle_button,
																  GrisbiWinRun *w_run)

{
	GtkTreeModel *model;
	GSettings *settings;
	gint etat;
	GrisbiAppConf *a_conf;

	devel_debug (NULL);
	etat = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button));
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_run->resolution_screen_toggled = etat;

	settings = grisbi_settings_get_settings (SETTINGS_GEOMETRY);
    g_settings_set_boolean (G_SETTINGS (settings),
							"low-resolution-screen",
							a_conf->low_resolution_screen);
	if (etat)
	{
		/*reset all geometry keys */
		g_settings_reset (G_SETTINGS (settings), "main-height");
		g_settings_reset (G_SETTINGS (settings), "main-width");
		a_conf->main_height = g_settings_get_int (settings, "main-height");
		a_conf->main_width = g_settings_get_int (settings, "main-width");

		settings = grisbi_settings_get_settings (SETTINGS_PANEL);
		g_settings_reset (G_SETTINGS (settings), "panel-width");
		a_conf->panel_width = g_settings_get_int (settings, "panel-width");

		gtk_window_resize (GTK_WINDOW (grisbi_app_get_active_window (NULL)), a_conf->main_width, a_conf->main_height);
	}

	/* init status bar */
	grisbi_win_init_statusbar (NULL);

	/* update navigation pane */
	if (grisbi_win_file_is_loading ())
	{
		model = gsb_gui_navigation_get_model ();
		gsb_gui_navigation_create_account_list (model);
		gsb_gui_navigation_create_report_list (model);
	}

	return FALSE;
}

/**
 * called when switch the preferences heading bar button
 * to show/hide the headings bar
 *
 * \param toggle button
 * \param null
 *
 * \return FALSE
 **/
static gboolean prefs_page_display_gui_switch_headings_bar (GtkWidget *toggle_button,
															GrisbiAppConf *a_conf)
{
	GSettings *settings;

    grisbi_win_headings_update_show_headings ();

	settings = grisbi_settings_get_settings (SETTINGS_DISPLAY);
    g_settings_set_boolean (G_SETTINGS (settings),
							"show-headings-bar",
							a_conf->show_headings_bar);

	return FALSE;
}

/**
 * Création de la page de gestion des display_gui
 *
 * \param prefs
 *
 * \return
 **/
static void prefs_page_display_gui_setup_display_gui_page (PrefsPageDisplayGui *page)
{
	GtkWidget *head_page;
	PangoTabArray *tabs;
	GrisbiAppConf *a_conf;
	GrisbiWinRun *w_run;
	PrefsPageDisplayGuiPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_gui_get_instance_private (page);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Elements of interface"), "gsb-display-gui-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_display_gui), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_display_gui), head_page, 0);

    /* set the variables for low_resolution_screen */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_low_resolution_screen),
								  a_conf->low_resolution_screen);

    /* Connect signal */
    g_signal_connect (priv->checkbutton_low_resolution_screen,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->low_resolution_screen);

	g_signal_connect_after (priv->checkbutton_low_resolution_screen,
							"toggled",
							G_CALLBACK (prefs_page_display_gui_resolution_screen_toggled),
							w_run);

	/* set the variables for show_headings_bar */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_show_headings_bar),
								  a_conf->show_headings_bar);
    /* Connect signal */
    g_signal_connect (priv->checkbutton_show_headings_bar,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->show_headings_bar);

	g_signal_connect_after (priv->checkbutton_show_headings_bar,
							"toggled",
							G_CALLBACK (prefs_page_display_gui_switch_headings_bar),
							a_conf);

    /* set the variables for active_scrolling_left_pane */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_active_scrolling_left_pane),
								  a_conf->active_scrolling_left_pane);
    /* Connect signal */
    g_signal_connect (priv->checkbutton_active_scrolling_left_pane,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &a_conf->active_scrolling_left_pane);

	g_signal_connect_after (priv->checkbutton_active_scrolling_left_pane,
							"toggled",
							G_CALLBACK (prefs_page_display_gui_active_scrolling_left_pane),
							a_conf);

    /* set the variables for display toolbar */
	switch (a_conf->display_toolbar)
	{
		case GSB_BUTTON_BOTH:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_display_both), TRUE);
			break;

		case GSB_BUTTON_BOTH_HORIZ:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_display_both_horiz), TRUE);
			break;

		case GSB_BUTTON_ICON:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_display_icon), TRUE);
			break;

		case GSB_BUTTON_TEXT:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_display_text), TRUE);
			break;
	}

	/* choix non disponible */
	gtk_widget_set_sensitive (priv->radiobutton_display_both_horiz, FALSE);

	/* set data for each widget */
	g_object_set_data (G_OBJECT (priv->radiobutton_display_both),
					   "display",
					   GINT_TO_POINTER (GSB_BUTTON_BOTH));
	g_object_set_data (G_OBJECT (priv->radiobutton_display_both_horiz),
					   "display",
					   GINT_TO_POINTER (GSB_BUTTON_BOTH_HORIZ));
	g_object_set_data (G_OBJECT (priv->radiobutton_display_icon),
					   "display",
					   GINT_TO_POINTER (GSB_BUTTON_ICON));
	g_object_set_data (G_OBJECT (priv->radiobutton_display_text),
					   "display",
					   GINT_TO_POINTER (GSB_BUTTON_TEXT));

	/* Connect signal */
	g_signal_connect (G_OBJECT (priv->radiobutton_display_both),
					  "toggled",
					  G_CALLBACK (prefs_page_display_gui_change_toolbar_display_mode),
					  a_conf);

	g_signal_connect (G_OBJECT (priv->radiobutton_display_both_horiz),
					  "toggled",
					  G_CALLBACK (prefs_page_display_gui_change_toolbar_display_mode),
					  a_conf);

	g_signal_connect (G_OBJECT (priv->radiobutton_display_icon),
					  "toggled",
					  G_CALLBACK (prefs_page_display_gui_change_toolbar_display_mode),
					  a_conf);

	g_signal_connect (G_OBJECT (priv->radiobutton_display_text),
					  "toggled",
					  G_CALLBACK (prefs_page_display_gui_change_toolbar_display_mode),
					  a_conf);

	/* set shortcuts text_view */
	tabs = pango_tab_array_new (3, TRUE);
	pango_tab_array_set_tab (tabs, 0, PANGO_TAB_LEFT, 0);
	pango_tab_array_set_tab (tabs, 1, PANGO_TAB_LEFT, 350);
	gtk_text_view_set_tabs (GTK_TEXT_VIEW (priv->text_view_display_shorcuts), tabs);
	pango_tab_array_free (tabs);

	gtk_text_view_set_editable (GTK_TEXT_VIEW (priv->text_view_display_shorcuts), FALSE);
	grisbi_app_display_gui_dump_accels (GTK_APPLICATION (g_application_get_default ()),
										priv->text_view_display_shorcuts);

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_display_gui_init (PrefsPageDisplayGui *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_display_gui_setup_display_gui_page (page);
}

static void prefs_page_display_gui_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_display_gui_parent_class)->dispose (object);
}

static void prefs_page_display_gui_class_init (PrefsPageDisplayGuiClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_display_gui_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_display_gui.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, vbox_display_gui);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, checkbutton_show_headings_bar);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, checkbutton_active_scrolling_left_pane);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, checkbutton_low_resolution_screen);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, radiobutton_display_both);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, radiobutton_display_both_horiz);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, radiobutton_display_icon);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, radiobutton_display_text);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, text_view_display_shorcuts);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageDisplayGui *prefs_page_display_gui_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_DISPLAY_GUI_TYPE, NULL);
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

