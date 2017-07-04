/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
/* 			http://www.grisbi.org				                              */
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

/**
 * \file gsb_utils_font.c
 * this file contains usefull functions to work with fonts
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "utils_font.h"
#include "grisbi_win.h"
#include "utils_str.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean utils_font_choose ( GtkWidget *button,
				    gchar **fontname );
static void utils_font_update_labels ( GtkWidget *button,
				       const gchar *fontname);
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


/**
 * create a button displaying the name and the size of the font
 * clicking on that button show a GtkFontSelectionDialog to change the font
 * and update the fontname
 * !! the fontname can be changed and freed so *fontname must not be const
 *
 * \param fontname	a pointer to a pointer to the name of the font
 * 			to change with the selection of the user
 * \param hook		An optional function to execute when the font has changed
 * 			(hook must be func ( gchar new_font_name, gpointer data ) )
 * \param data		An optional pointer to pass to hooks.
 *
 * \return a GtkButton
 * */
GtkWidget *utils_font_create_button ( gchar **fontname,
				      GCallback hook,
				      gpointer data  )
{
    GtkWidget *font_button;
    GtkWidget *hbox_font;
    GtkWidget *font_name_label;
    GtkWidget *font_size_label;


    font_button = gtk_button_new ();
    g_signal_connect ( G_OBJECT ( font_button ), "clicked",
		       G_CALLBACK ( utils_font_choose ), fontname );

    hbox_font = gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX );
    gtk_container_add (GTK_CONTAINER(font_button), hbox_font);

    font_name_label = gtk_label_new (NULL);
	gtk_widget_set_name (font_name_label, "font_label");
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), font_name_label,
			 TRUE, TRUE, 5 );

    font_size_label = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), font_size_label,
			 FALSE, FALSE, 5 );
    g_object_set_data ( G_OBJECT (font_button),
			"hook", hook);
    g_object_set_data ( G_OBJECT (font_button),
			"data", data);
    g_object_set_data ( G_OBJECT (font_button),
			"name_label", font_name_label);
    g_object_set_data ( G_OBJECT (font_button),
			"size_label", font_size_label);


    utils_font_update_labels(font_button, *fontname);


    return font_button;
}

/**
 * callback called when click on the font button
 * to change the font
 *
 * \param button
 * \param fontname	a pointer to pointer to the font name to change
 *
 * \return FALSE
 * */
static gboolean utils_font_choose ( GtkWidget *button,
				    gchar **fontname )
{
    gchar *new_fontname;
    GtkWidget *dialog;
	GtkWidget *prefs_dialog;
    GCallback (*hook) (gchar *, gpointer);

	prefs_dialog = grisbi_win_get_prefs_dialog (NULL);
    dialog = gtk_font_chooser_dialog_new (_("Choosing font"), GTK_WINDOW (prefs_dialog));

    if (*fontname)
		gtk_font_chooser_set_font (GTK_FONT_CHOOSER (dialog), *fontname);
    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
		case GTK_RESPONSE_OK:
			new_fontname = gtk_font_chooser_get_font (GTK_FONT_CHOOSER (dialog));
			gtk_widget_destroy (dialog);
			utils_font_update_labels (button, new_fontname);
			break;
		default:
			gtk_widget_destroy (dialog);
			return FALSE;
    }

    if (*fontname)
	g_free (*fontname);
    *fontname = new_fontname;

    hook = g_object_get_data (G_OBJECT (button), "hook");
    if (hook)
    {
	hook (new_fontname,
	      g_object_get_data (G_OBJECT (button), "data"));
    }

    return FALSE;
}


/**
 * Update two labels according to font name, which is parsed to
 * separate both name and size
 *
 * \param button	the font_button
 * \param fontname 	a font name in the form "name, size" or "name,
 *                 	attr, size"
 */
static void utils_font_update_labels ( GtkWidget *button,
				       const gchar *fontname)
{
    GtkWidget *font_name_label;
    GtkWidget *font_size_label;
    gchar *font_name;
    gchar *font_size;
	gchar *data;

    font_name_label = g_object_get_data (G_OBJECT (button),
					 "name_label" );
    font_size_label = g_object_get_data (G_OBJECT (button),
					 "size_label" );
    if (fontname)
    {
		GtkCssProvider *css_provider = NULL;
		gchar *tmp;

		css_provider = gtk_css_provider_get_default ();

		data = g_strconcat ("GtkLabel#font_label {font: ", fontname, "}", NULL);
		gtk_css_provider_load_from_data (css_provider, data, -1, NULL);
		g_free (data);

		font_name = my_strdup ( fontname );
		tmp = font_name + strlen(font_name) - 1;
		while (g_ascii_isdigit(*tmp) ||
			   (*tmp) == '.')
			tmp --;
		font_size = tmp+1;

		while (*tmp == ' ' ||
			   *tmp == ',')
		{
			*tmp=0;
			tmp--;
		}
    }
    else
    {
		font_name = my_strdup (_("No font defined"));
		font_size = NULL;
    }

    gtk_label_set_text (GTK_LABEL(font_name_label), font_name);
    gtk_label_set_text (GTK_LABEL(font_size_label), font_size);

    if ( font_name ) g_free ( font_name );
}



