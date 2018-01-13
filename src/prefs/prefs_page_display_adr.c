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
#include "prefs_page_display_adr.h"
#include "grisbi_win.h"
#include "navigation.h"
#include "structures.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageDisplayAdrPrivate   PrefsPageDisplayAdrPrivate;

struct _PrefsPageDisplayAdrPrivate
{
	GtkWidget *			vbox_display_adr;

	GtkWidget *			entry_accounting_entity;
    GtkWidget *			radiobutton_accounting_entity;
    GtkWidget *			radiobutton_filename;
    GtkWidget *			radiobutton_holder;
    GtkWidget *         textview_adr_common;
    GtkWidget *         textview_adr_secondary;

	GrisbiWinEtat *		w_etat;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageDisplayAdr, prefs_page_display_adr, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
static gboolean prefs_page_display_adr_text_adr_changed (GtkTextBuffer *buffer,
														 gchar **data)
{
    GtkTextIter start, end;

    if (!buffer)
		return FALSE;

    gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
    gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

    if (data)
    {
		*data = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
		utils_prefs_gsb_file_set_modified ();
    }

    return FALSE;
}

/**
 * Update the label that contain main title in homepage.
 *
 * \param entry Widget that triggered this handled.  Not used.
 * \param value Not used handler parameter.
 * \param length Not used handler parameter.
 * \param position Not used handler parameter.
 */
static void prefs_page_display_adr_accounting_entity_changed (GtkEditable *entry,
															  gpointer data)
{
	GrisbiWinEtat *w_etat;
	const gchar *text;

	text = gtk_entry_get_text (GTK_ENTRY (entry));
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	if (text && strlen (text))
		w_etat->accounting_entity = my_strdup (text);
	else
		w_etat->accounting_entity = my_strdup ("");

    /* set Grisbi title */
    grisbi_win_set_window_title (-1);

    /* Mark file as modified */
	utils_prefs_gsb_file_set_modified ();
}

/**
 * Signal triggered when user configure display grisbi title
 *
 * \param button	Radio button that triggered event.
 * \param
 *
 * \return FALSE
 */
static gboolean prefs_page_display_adr_window_title_toggled (GtkRadioButton *button,
															 GtkWidget *entry)
{
	GrisbiWinEtat *w_etat;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
    {
        conf.display_window_title = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "display"));
    }

    switch (conf.display_window_title)
    {
        case GSB_ACCOUNT_ENTITY:
            gtk_widget_set_sensitive (entry, TRUE);
			w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
            if (w_etat->accounting_entity && strlen (w_etat->accounting_entity))
                gtk_entry_set_text (GTK_ENTRY (entry), w_etat->accounting_entity);
            else
            {
                gtk_entry_set_text (GTK_ENTRY (entry), "");
            }
			break;
        case GSB_ACCOUNT_HOLDER:
            gtk_widget_set_sensitive (entry, FALSE);
			break;
        case GSB_ACCOUNT_FILENAME:
            gtk_widget_set_sensitive (entry, FALSE);
			break;
    }

    /* set Grisbi title */
    grisbi_win_set_window_title (gsb_gui_navigation_get_current_account ());

    /* Mark file as modified */
	utils_prefs_gsb_file_set_modified ();

    return FALSE;
}

/**
 * Création de la page de gestion des display_adr
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_display_adr_setup_display_adr_page (PrefsPageDisplayAdr *page)
{
	GtkWidget *head_page;
	GtkTextBuffer *buffer;
	gboolean is_loading;
	GrisbiWinEtat *w_etat;
	PrefsPageDisplayAdrPrivate *priv;

	devel_debug (NULL);

	priv = prefs_page_display_adr_get_instance_private (page);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Addresses & titles"), "gsb-addresses-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_display_adr), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_display_adr), head_page, 0);

	if (is_loading == FALSE)
	{
		gtk_widget_set_sensitive (priv->vbox_display_adr, FALSE);
		return;
	}

    /* set the variables for title */
	switch (conf.display_window_title)
	{
		case GSB_ACCOUNT_ENTITY:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_accounting_entity), TRUE);
			gtk_widget_set_sensitive (priv->entry_accounting_entity, TRUE);
			break;

		case GSB_ACCOUNT_HOLDER:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_holder), TRUE);
			gtk_widget_set_sensitive (priv->entry_accounting_entity, FALSE);
			break;

		case GSB_ACCOUNT_FILENAME:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_filename), TRUE);
			gtk_widget_set_sensitive (priv->entry_accounting_entity, FALSE);
			break;
	}

	/* set data for each widget */
	g_object_set_data (G_OBJECT (priv->radiobutton_accounting_entity),
					   "display",
					   GINT_TO_POINTER ( GSB_ACCOUNT_ENTITY));
	g_object_set_data (G_OBJECT (priv->radiobutton_holder),
					   "display",
					   GINT_TO_POINTER ( GSB_ACCOUNT_HOLDER));
	g_object_set_data (G_OBJECT (priv->radiobutton_filename),
					   "display",
					   GINT_TO_POINTER ( GSB_ACCOUNT_FILENAME));

	gtk_entry_set_text (GTK_ENTRY(priv->entry_accounting_entity), w_etat->accounting_entity);

	/* Connect signal */
	g_signal_connect (G_OBJECT (priv->radiobutton_accounting_entity),
					  "toggled",
					  G_CALLBACK (prefs_page_display_adr_window_title_toggled),
					  priv->entry_accounting_entity);

	g_signal_connect (G_OBJECT (priv->radiobutton_holder),
					  "toggled",
					  G_CALLBACK (prefs_page_display_adr_window_title_toggled),
					  priv->entry_accounting_entity);

	g_signal_connect (G_OBJECT (priv->radiobutton_filename),
					  "toggled",
					  G_CALLBACK (prefs_page_display_adr_window_title_toggled),
					  priv->entry_accounting_entity);

	g_signal_connect (G_OBJECT(priv->entry_accounting_entity),
					  "changed",
					  G_CALLBACK (prefs_page_display_adr_accounting_entity_changed),
					  NULL);

	/* variables for addresses */
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview_adr_common));
	if (w_etat->adr_common && strlen (w_etat->adr_common))
		gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), w_etat->adr_common, -1);

	g_signal_connect (G_OBJECT (buffer),
					  "changed",
					  G_CALLBACK (prefs_page_display_adr_text_adr_changed),
					  &w_etat->adr_common);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview_adr_secondary));
	if (w_etat->adr_secondary && strlen (w_etat->adr_secondary))
		gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), w_etat->adr_secondary, -1);

	g_signal_connect (G_OBJECT (buffer),
					  "changed",
					  G_CALLBACK (prefs_page_display_adr_text_adr_changed),
					  &w_etat->adr_secondary);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_display_adr_init (PrefsPageDisplayAdr *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_display_adr_setup_display_adr_page (page);
}

static void prefs_page_display_adr_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_display_adr_parent_class)->dispose (object);
}

static void prefs_page_display_adr_class_init (PrefsPageDisplayAdrClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_display_adr_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/prefs_page_display_adr.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayAdr, vbox_display_adr);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayAdr, entry_accounting_entity);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayAdr, radiobutton_filename);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayAdr, radiobutton_holder);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayAdr, radiobutton_accounting_entity);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayAdr, textview_adr_common);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageDisplayAdr, textview_adr_secondary);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
PrefsPageDisplayAdr *prefs_page_display_adr_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_DISPLAY_ADR_TYPE, NULL);
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

