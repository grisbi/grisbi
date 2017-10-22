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
#include "prefs_page_display_gui.h"
#include "fenetre_principale.h"
#include "grisbi_settings.h"
#include "grisbi_win.h"
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
	GtkWidget *			eventbox_show_headings_bar;

	GtkWidget *			checkbutton_active_scrolling_left_pane;
	GtkWidget *			eventbox_active_scrolling_left_pane;

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
static void prefs_page_display_gui_dump_accels (GtkApplication *app,
												GtkWidget *text_view)
{
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	gchar **actions;
	gint i;

	actions = gtk_application_list_action_descriptions (app);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

	for (i = 0; actions[i]; i++)
	{
		gchar **accels;
		gchar *str;
		gchar *text;

		accels = gtk_application_get_accels_for_action (app, actions[i]);
		str = g_strjoinv (",", accels);
		text = g_strdup_printf ("%s\t-> %s.\n", actions[i], str);
		gtk_text_buffer_insert (buffer, &iter, text, -1);
		g_strfreev (accels);
		g_free (str);
	}
	g_strfreev (actions);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
	gtk_text_buffer_insert_markup (buffer, &iter, _("<b>Actions in transaction list :</b>"), -1);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
	gtk_text_buffer_insert (buffer, &iter, _("(Un)Pointing a transaction\t-> <Primary>p, <Primary>F12\n"), -1);
	gtk_text_buffer_insert (buffer, &iter, _("(Un)Reconcile a transaction\t-> <Primary>r\n"), -1);

}

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
																   gpointer null)
{
	GSettings *settings;

	if (conf.active_scrolling_left_pane)
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
							conf.active_scrolling_left_pane);

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
static gboolean prefs_page_display_gui_change_toolbar_display_mode (GtkRadioButton *button)
{
    GSettings *settings;
    gchar *tmp_str;

    /* Do not execute this callback twice,
     * as it is triggered for both unselected button and newly selected one.
     * We keep the call for the newly selected radio button */
    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
        return FALSE;

    /* save the new parameter */
    conf.display_toolbar = GPOINTER_TO_INT (g_object_get_data (G_OBJECT(button), "display"));

    /* update GSettings */
    switch (conf.display_toolbar)
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
    gsb_gui_update_all_toolbars ();

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
															gpointer null)
{
	GSettings *settings;

    grisbi_win_headings_update_show_headings ();

	settings = grisbi_settings_get_settings (SETTINGS_DISPLAY);
    g_settings_set_boolean (G_SETTINGS (settings),
							"show-headings-bar",
							conf.show_headings_bar);

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
	PrefsPageDisplayGuiPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_gui_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Elements of interface"), "gsb-display-gui-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_display_gui), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_display_gui), head_page, 0);

    /* set the variables for show_headings_bar */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_show_headings_bar),
								  conf.show_headings_bar);
    /* Connect signal */
    g_signal_connect (priv->eventbox_show_headings_bar,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_show_headings_bar);

    g_signal_connect (priv->checkbutton_show_headings_bar,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.show_headings_bar);

	g_signal_connect_after (priv->checkbutton_show_headings_bar,
							"toggled",
							G_CALLBACK (prefs_page_display_gui_switch_headings_bar),
							NULL);


    /* set the variables for active_scrolling_left_pane */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_active_scrolling_left_pane),
								  conf.active_scrolling_left_pane);
    /* Connect signal */
    g_signal_connect (priv->eventbox_active_scrolling_left_pane,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_active_scrolling_left_pane);

    g_signal_connect (priv->checkbutton_active_scrolling_left_pane,
					  "toggled",
					  G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  &conf.active_scrolling_left_pane);

	g_signal_connect_after (priv->checkbutton_active_scrolling_left_pane,
							"toggled",
							G_CALLBACK (prefs_page_display_gui_active_scrolling_left_pane),
							NULL);

    /* set the variables for display toolbar */
	    /* set the variables for title */
	switch (conf.display_toolbar)
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
					  NULL);

	g_signal_connect (G_OBJECT (priv->radiobutton_display_both_horiz),
					  "toggled",
					  G_CALLBACK (prefs_page_display_gui_change_toolbar_display_mode),
					  NULL);

	g_signal_connect (G_OBJECT (priv->radiobutton_display_icon),
					  "toggled",
					  G_CALLBACK (prefs_page_display_gui_change_toolbar_display_mode),
					  NULL);

	g_signal_connect (G_OBJECT (priv->radiobutton_display_text),
					  "toggled",
					  G_CALLBACK (prefs_page_display_gui_change_toolbar_display_mode),
					  NULL);

	/* set shortcuts text_view */
	tabs = pango_tab_array_new (3, TRUE);
	pango_tab_array_set_tab (tabs, 0, PANGO_TAB_LEFT, 0);
	pango_tab_array_set_tab (tabs, 1, PANGO_TAB_LEFT, 300);
	gtk_text_view_set_tabs (GTK_TEXT_VIEW (priv->text_view_display_shorcuts), tabs);
	pango_tab_array_free (tabs);

	gtk_text_view_set_editable (GTK_TEXT_VIEW (priv->text_view_display_shorcuts), FALSE);
	prefs_page_display_gui_dump_accels (GTK_APPLICATION (g_application_get_default ()), priv->text_view_display_shorcuts);

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
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, eventbox_show_headings_bar);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, checkbutton_active_scrolling_left_pane);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayGui, eventbox_active_scrolling_left_pane);
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

