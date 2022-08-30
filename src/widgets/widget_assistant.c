/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2022 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "widget_assistant.h"
#include "grisbi_app.h"
#include "gsb_assistant.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_select_icon.h"
#include "structures.h"
#include "utils_prefs.h"
#include "widget_reconcile.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetAssistantPrivate   WidgetAssistantPrivate;
typedef gboolean (* gsb_assistant_callback) (GtkWidget *, gint);

struct _WidgetAssistantPrivate
{
	GtkWidget *			vbox_assistant;

	GtkWidget *			button_cancel;
	GtkWidget *			button_close;
	GtkWidget *			button_next;
	GtkWidget *			button_prev;
	GtkWidget *			button_select;
	GtkWidget *			eventbox_grey_box;
	GtkWidget *			hbox_grey_box;
	GtkWidget *			label_grey_box;
	GtkWidget *			notebook;
	GtkWidget *			textview;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetAssistant, widget_assistant, GTK_TYPE_DIALOG)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Call a user-defined optional callback when user change page.  Note
 * that it is called AFTER stock callbacks for various reasons.
 *
 * \param notebook		This Grisbi assistant notebook.
 * \param npage			Not used.
 * \param page			Page selected.
 * \param assistant		Grisbi assistant containing the notebook.
 *
 * \return				Result from user-defined callback or FALSE if no callback defined.
 */
static gboolean widget_assistant_switch_page (GtkNotebook *notebook,
											  gpointer npage,
											  gint page,
											  gpointer dialog)
{
	gchar* tmp_str;
	gsb_assistant_callback callback;

	tmp_str = g_strdup_printf ("enter%d", page);
	callback = (gsb_assistant_callback) g_object_get_data (G_OBJECT (dialog), tmp_str);

	if (callback)
	{
		return callback (dialog, page);
	}

	return FALSE;
}

/**
 * initialisation du widget
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_assistant_setup_widget (WidgetAssistant *dialog,
										   const gchar *title,
										   const gchar *explanation,
										   const gchar *image_filename,
										   GCallback enter_callback)
{
	GtkWidget *image;
	GdkPixbuf *pixbuf;
	GtkTextBuffer *buffer;
	gchar *tmp_str;
	WidgetAssistantPrivate *priv;

	devel_debug (NULL);
	priv = widget_assistant_get_instance_private (dialog);

	priv->button_select = gtk_toggle_button_new_with_label (_("Select all"));
	g_object_set_data (G_OBJECT (dialog), "button_select", priv->button_select);

	/* set buttons */
	gtk_widget_hide (priv->button_close);

	/* set grey box */
	tmp_str = g_markup_printf_escaped ("<b><span size=\"x-large\">%s</span></b>", title);
	gtk_label_set_markup (GTK_LABEL(priv->label_grey_box), tmp_str);
	g_free (tmp_str);

	if (!image_filename)
	{
		pixbuf = gsb_select_icon_get_default_logo_pixbuf ();
	}
	else
	{
		tmp_str = g_build_filename (gsb_dirs_get_pixmaps_dir (), image_filename, NULL);
		pixbuf = gdk_pixbuf_new_from_file_at_scale (tmp_str, LOGO_WIDTH, LOGO_HEIGHT, FALSE, NULL);
		g_free (tmp_str);
	}
	image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_widget_set_margin_bottom (image, MARGIN_BOX);
	gtk_widget_set_margin_top (image, MARGIN_BOX);
	gtk_widget_set_halign (image, GTK_ALIGN_END);
	gtk_box_pack_end (GTK_BOX(priv->hbox_grey_box), image, FALSE, FALSE, 0);
	g_object_unref (pixbuf);

	/* set textview */
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->textview));
	tmp_str = g_strconcat ("\n", explanation, "\n", NULL);
	gtk_text_buffer_set_text (buffer, tmp_str, -1);
	g_free (tmp_str);

	/* Create some handy fonts. */
	gtk_text_buffer_create_tag (buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag (buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
	gtk_text_buffer_create_tag (buffer, "indented", "left-margin", 24, NULL);

	/* set notebbok signal */
	g_signal_connect_after (G_OBJECT (priv->notebook),
							"switch-page",
							G_CALLBACK (widget_assistant_switch_page),
							dialog);

	/* positionne next */
	gsb_assistant_set_next (GTK_WIDGET (dialog), 0, 1);

	/* set initial data */
	g_object_set_data_full (G_OBJECT (dialog), "title", g_strdup (title), g_free);
	g_object_set_data (G_OBJECT (dialog), "enter0", enter_callback);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_assistant_init (WidgetAssistant *dialog)
{
	gtk_widget_init_template (GTK_WIDGET (dialog));
}

static void widget_assistant_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_assistant_parent_class)->dispose (object);
}

static void widget_assistant_class_init (WidgetAssistantClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_assistant_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_assistant.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, vbox_assistant);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, button_cancel);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, button_close);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, button_next);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, button_prev);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, eventbox_grey_box);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, hbox_grey_box);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, label_grey_box);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, notebook);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, textview);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAssistant, );
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * Création de la fenetre de dialog de l'assistant
 *
 * \param
 *
 * \return
 **/
WidgetAssistant *widget_assistant_new (const gchar *title,
									   const gchar *explanation,
									   const gchar *image_filename,
									   GCallback enter_callback)
{
	GtkWindow *parent;
	WidgetAssistant *dialog;

	dialog = g_object_new (WIDGET_ASSISTANT_TYPE, NULL);

	parent = GTK_WINDOW (grisbi_app_get_active_window (NULL));
	widget_assistant_setup_widget (dialog, title, explanation, image_filename, enter_callback);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));
	g_object_set_data (G_OBJECT (parent), "assistant", dialog);

	return dialog;
}

/**
 * Add a page to the Grisbi assistant.
 *
 * \param assistant			Grisbi assistant to add a page to.
 * \param widget			Widget containing the new page to insert.
 * \param position			Number of the page to insert.  Page 0 is reserved to the explanation label.
 * \param prev				Page to display when the "Previous" button is clicked.
 * \param next				Page to display when the "Next" button is clicked.
 * \param enter_callback	A callback to connect to the "switch-page" callback
 *							of the Grisbi assistant notebook. (the callback should be :
 *							gboolean callback (GtkWidget *assistant, gint new_page))
 *
 * \return
 **/
void widget_assistant_add_page (GtkWidget *dialog,
								GtkWidget *widget,
								gint position,
								gint prev,
								gint next,
								GCallback enter_callback)
{
	gchar *tmp_str;
	WidgetAssistantPrivate *priv;

	devel_debug (NULL);
	priv = widget_assistant_get_instance_private (WIDGET_ASSISTANT (dialog));
	gtk_notebook_insert_page (GTK_NOTEBOOK(priv->notebook), widget, gtk_label_new (NULL), position);

	widget_assistant_set_prev (dialog, position, prev);
	widget_assistant_set_next (dialog, position, next);
	tmp_str = g_strdup_printf ("enter%d", position);
	g_object_set_data (G_OBJECT(dialog), tmp_str, enter_callback);
	g_free (tmp_str);

	gtk_widget_show_all (widget);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void widget_assistant_change_button_next (GtkWidget *dialog,
										  const gchar *title,
										  GtkResponseType response)
{
	WidgetAssistantPrivate *priv;

	devel_debug (NULL);
	priv = widget_assistant_get_instance_private (WIDGET_ASSISTANT (dialog));

	if (g_strcmp0 (title, "gtk-close") == 0)
	{
		gtk_widget_hide (priv->button_next);
		gtk_widget_show (priv->button_close);
	}
	else if (g_strcmp0 (title, "gtk-go-forward") == 0)
	{
		gtk_widget_hide (priv->button_next);
		//~ gtk_widget_show (priv->button_close);
	}
}

/**
 * force the assistant to go to the next page
 *
 * \param assistant
 *
 * \return
 **/
void widget_assistant_next_page (GtkWidget *dialog)
{
	WidgetAssistantPrivate *priv;

	devel_debug (NULL);
	priv = widget_assistant_get_instance_private (WIDGET_ASSISTANT (dialog));
	gtk_button_clicked (GTK_BUTTON (priv->button_next));
}

/**
 * Run the Grisbi assistant.  This will pop up a new dialog.
 *
 * \param assistant		Grisbi assistant to run.
 *
 * \return				Out come of the Grisbi assistant.  Can be
 *						GTK_RESPONSE_APPLY for success and
 *						GTK_RESPONSE_CANCEL for failure (user canceled or closed dialog).
 **/
GtkResponseType widget_assistant_run (GtkWidget *dialog)
{
	WidgetAssistantPrivate *priv;

	devel_debug (NULL);
	priv = widget_assistant_get_instance_private (WIDGET_ASSISTANT (dialog));

	/* hide buttons unused */
	gtk_widget_hide (priv->button_select);

	gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), 0);

	gtk_widget_set_sensitive (priv->button_next, TRUE);
	gtk_widget_grab_focus (GTK_WIDGET (priv->button_next));

	while (TRUE)
	{
		gchar*tmp_str;
		gint current = 0;
		gint next = 0;
		gint prev = 0;
		gint result = 0;

		current = gtk_notebook_get_current_page (GTK_NOTEBOOK (priv->notebook));
		tmp_str = g_strdup_printf (_("%s (%d of %d)"),
								   (gchar*) g_object_get_data (G_OBJECT (dialog), "title"),
								   current + 1,
								   gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)));
		gtk_window_set_title (GTK_WINDOW (dialog), tmp_str);
		g_free (tmp_str);

		result = gtk_dialog_run (GTK_DIALOG (dialog));

		tmp_str = g_strdup_printf ("prev%d", current);
		prev = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog), tmp_str));
		g_free (tmp_str);

		tmp_str = g_strdup_printf ("next%d", current);
		next = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (dialog), tmp_str));
		g_free (tmp_str);

		switch (result)
		{
			 case GTK_RESPONSE_YES:			/* button_next */
				gtk_widget_set_sensitive (priv->button_prev, TRUE);
				if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook)) == (next + 1))
				{
					gtk_widget_hide (priv->button_next);
					gtk_widget_show (priv->button_close);
					widget_assistant_sensitive_button_prev (dialog, FALSE);
				}
				gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), next);

			break;

			case GTK_RESPONSE_NO:
				widget_assistant_sensitive_button_next (dialog, TRUE);

				if (prev == 0)
				{
					gtk_widget_set_sensitive (priv->button_prev, FALSE);
				}

				gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook), prev);

				break;

			case GTK_RESPONSE_APPLY:

				 return GTK_RESPONSE_APPLY;

			default:
			case GTK_RESPONSE_CANCEL:

				 return GTK_RESPONSE_CANCEL;
		}
	}

	return GTK_RESPONSE_CANCEL;
}

/**
 * sensitive  the button next
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean widget_assistant_sensitive_button_next (GtkWidget *dialog,
												 gboolean state)
{
	WidgetAssistantPrivate *priv;

	//~ devel_debug (NULL);
	priv = widget_assistant_get_instance_private (WIDGET_ASSISTANT (dialog));
	gtk_widget_set_sensitive (priv->button_next, state);

	return FALSE;
}

/**
 * sensitive  the button previous
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean widget_assistant_sensitive_button_prev (GtkWidget *dialog,
											  gboolean state)
{
	WidgetAssistantPrivate *priv;

	devel_debug (NULL);
	priv = widget_assistant_get_instance_private (WIDGET_ASSISTANT (dialog));
	gtk_widget_set_sensitive (priv->button_prev, state);

	return FALSE;
}

/**
 * Change the next page associated to a notebook page.  This can
 * be used to insert a page between two others.
 *
 * \param assistant		A pointer to a Grisbi assistant.
 * \param page			Page to change next link.
 * \param next			Number of the new next page.
 *
 * \return
 **/
void widget_assistant_set_next (GtkWidget *dialog,
								gint page,
								gint next)
{
	gchar *tmp_str;

	tmp_str = g_strdup_printf ("next%d", page);
	g_object_set_data (G_OBJECT (dialog), tmp_str, GINT_TO_POINTER (next));
	g_free (tmp_str);
}

/**
 * Change the previous page associated to a notebook page.  This can
 * be used to insert a page between two others.
 *
 * \param assistant		A pointer to a Grisbi assistant.
 * \param page			Page to change previous link.
 * \param prev			Number of the new previous page.
 *
 * \return
 **/
void widget_assistant_set_prev (GtkWidget *dialog,
								gint page,
								gint prev)
{
	gchar *tmp_str;

	tmp_str = g_strdup_printf ("prev%d", page);
	g_object_set_data (G_OBJECT (dialog), tmp_str, GINT_TO_POINTER (prev));
	g_free (tmp_str);
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

