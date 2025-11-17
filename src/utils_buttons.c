/* ************************************************************************** */
/*                                  utils_buttons.c			                  */
/*                                                                            */
/*     Copyright (C)	2000-2008 C�dric Auger (cedric@grisbi.org)	          */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                 2009-2016 Pierre Biava (grisbi@pierre.biava.name)          */
/* 			https://www.grisbi.org				                              */
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

/*START_INCLUDE*/
#include "utils_buttons.h"
#include "gsb_automem.h"
#include "gsb_dirs.h"
#include "structures.h"
#include "utils.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

/**
 * called by a gsb_automem_checkbutton_new or a g_signal_connect on a checkbutton
 * sensitive or unsensitive tha param widget, according to the checkbutton
 *
 * \param check_button
 * \param widget
 *
 * \return FALSE
 * */
gboolean utils_buttons_sensitive_by_checkbutton (GtkWidget *check_button,
												 GtkWidget *widget )
{
    gtk_widget_set_sensitive ( widget,
			       gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button)));
    return FALSE;
}


/**
 * Cette fonction réduit ou développe toutes les lignes du tree_view.
 * Le libellé du bouton est modifié en conséquence.
 *
 * \param le button de commande
 * \param le tree_view considéré
 *
 * \return
 */
void utils_togglebutton_collapse_expand_all_rows ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view )
{
    GtkWidget *hbox_expand;
    GtkWidget *hbox_collapse;

    hbox_expand = g_object_get_data ( G_OBJECT ( togglebutton ), "hbox_expand" );
    hbox_collapse = g_object_get_data ( G_OBJECT ( togglebutton ), "hbox_collapse" );
	/* on remet à FALSE la propriété "no-show-all" utilisée pour initialiser le bouton */
	/* voir etats_prefs_toggle_button_init_button_expand () */
	if (gtk_widget_get_no_show_all (hbox_collapse))
		gtk_widget_set_no_show_all (hbox_collapse, FALSE);

    if ( gtk_toggle_button_get_active ( togglebutton ) )
    {
        gtk_widget_hide ( hbox_expand );
        gtk_widget_show_all ( hbox_collapse );
        gtk_tree_view_expand_all ( GTK_TREE_VIEW ( tree_view ) );
    }
    else
    {
        gtk_widget_show ( hbox_expand );
        gtk_widget_hide ( hbox_collapse );
        gtk_tree_view_collapse_all ( GTK_TREE_VIEW ( tree_view ) );
    }
}


/**
 * Cette fonction (dé)sélectionne toutes les lignes du tree_view.
 * Le libellé du bouton est modifié en conséquence.
 *
 * \param le button de commande
 * \param le tree_view considéré
 *
 * \return
 */
void utils_togglebutton_select_unselect_all_rows ( GtkToggleButton *togglebutton,
                        GtkWidget *tree_view )
{
    gchar *label;

    if ( gtk_toggle_button_get_active ( togglebutton ) )
    {
        gtk_tree_selection_select_all ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) );
        label = g_strdup ( _("Unselect all") );
    }
    else
    {
        gtk_tree_selection_unselect_all ( gtk_tree_view_get_selection ( GTK_TREE_VIEW ( tree_view ) ) );
        label = g_strdup ( _("Select all") );
    }

    gtk_button_set_label ( GTK_BUTTON ( togglebutton ), label );

    g_free ( label );
}


/**
 * Cette fonction remplace le libellé select par unselect et vice versa
 * en fonction de l'état du bouton.
 *
 * \param le button de commande
 *
 * \return
 */
void utils_togglebutton_change_label_select_unselect ( GtkToggleButton *togglebutton,
                        gint toggle )
{
    gchar *label;
    const gchar *string;

    string = gtk_button_get_label ( GTK_BUTTON ( togglebutton ) );

    if ( ( toggle ) )
        label = gsb_string_remplace_string ( string, _("Select"), _("Unselect") );
    else
        label = gsb_string_remplace_string ( string, _("Unselect"), _("Select") );

    gtk_button_set_label ( GTK_BUTTON ( togglebutton ), label );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( togglebutton ), toggle );

    g_free ( label );
}


/**
 * Cette fonction remplace le libellé select par unselect et positionne le bouton sur ON
 *
 * \param le button de commande
 *
 * \return
 */
void utils_togglebutton_set_label_position_unselect ( GtkWidget *togglebutton,
                        GCallback callback,
                        GtkWidget *tree_view )
{
    if ( callback == NULL )
    {
        g_signal_handlers_block_by_func ( G_OBJECT ( togglebutton ),
                                utils_togglebutton_select_unselect_all_rows,
                                tree_view );

        utils_togglebutton_change_label_select_unselect ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );
        g_signal_handlers_unblock_by_func ( G_OBJECT ( togglebutton ),
                                utils_togglebutton_select_unselect_all_rows,
                                tree_view );
    }
    else
    {
        g_signal_handlers_block_by_func ( G_OBJECT ( togglebutton ),
                                G_CALLBACK ( callback ),
                                tree_view );

        utils_togglebutton_change_label_select_unselect ( GTK_TOGGLE_BUTTON ( togglebutton ), TRUE );
        g_signal_handlers_unblock_by_func ( G_OBJECT ( togglebutton ),
                                G_CALLBACK ( callback ),
                                tree_view );
    }
}


/**
 * retourne l'index du radiobutton actif.
 *
 * \param radio_button
 *
 * \return index bouton actif
 */
gint utils_radiobutton_get_active_index ( GtkWidget *radiobutton )
{
    GSList *liste;
    GSList *tmp_list;
    gint index = 0;

    liste = g_slist_copy ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( radiobutton ) ) );
    tmp_list = g_slist_reverse ( liste );

    while ( tmp_list )
    {
        GtkWidget *button;

        button = tmp_list->data;
        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button ) ) )
            break;

        index++;
        tmp_list = tmp_list->next;
    }

    g_slist_free ( liste );

    return index;
}


/**
 * rend actif le button qui correspond à l'index passé en paramètre.
 *
 * \param radio_button
 * \param index du bouton à rendre actif
 *
 * \return
 */
void utils_radiobutton_set_active_index ( GtkWidget *radiobutton,
                        gint index )
{
    GSList *liste;
    GSList *tmp_list;

    liste = g_slist_copy ( gtk_radio_button_get_group ( GTK_RADIO_BUTTON ( radiobutton ) ) );
    tmp_list = g_slist_reverse ( liste );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( g_slist_nth_data ( tmp_list, index ) ), TRUE );

    g_slist_free ( liste );
}

/**
 * Crée un bouton avec une image
 *
 * \param const gchar   name of image
 *
 * \return GtkWidget
 * */
GtkWidget *utils_buttons_button_new_from_image (const gchar *image_name)
{
    GtkWidget *button = NULL;
    gchar *filename;

    button = gtk_button_new ();

    filename = g_build_filename (gsb_dirs_get_pixmaps_dir (), image_name, NULL);
    if (filename)
    {
        GtkWidget *image;

        image = gtk_image_new_from_file (filename);
        g_free (filename);
		gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
        gtk_button_set_image (GTK_BUTTON (button), image);
    }

    return button;
}

/**
 * Crée un bouton à partir d'une ressource image
 *
 * \param const gchar   name of image
 *
 * \return GtkWidget
 **/
GtkWidget *utils_buttons_button_new_from_resource (const gchar *image_name)
{
	GtkWidget *button = NULL;
	GtkWidget *image;
	gchar *resource;

	button = gtk_button_new ();

	resource = g_strconcat ("/org/gtk/grisbi/images/", image_name, NULL);

	image = gtk_image_new_from_resource (resource);
	g_free (resource);
	gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
	gtk_button_set_image (GTK_BUTTON (button), image);

	return button;
}

/**
 * similaire à gtk_button_new_from_stock ()
 *
 * \param const gchar   stock item
 *
 * \return GtkWidget
 * */
GtkWidget *utils_buttons_button_new_from_icon_name (const gchar *icon_name,
													const gchar *label_name)
{
    GtkWidget *button = NULL;
    GtkWidget *image;

    image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_BUTTON);
    button = gtk_button_new_with_mnemonic (label_name);
	gtk_button_set_always_show_image (GTK_BUTTON (button), TRUE);
    gtk_button_set_image (GTK_BUTTON (button), image);

    return button;
}

/**
 * Création d'un GtkToolButton à partir d'une image et d'un label
 *
 * \param image_name    filename
 * \param label_name    label for button
 *
 * \return a GtkToolItem or NULL
 * */
GtkToolItem *utils_buttons_tool_button_new_from_image_label ( const gchar *image_name,
                        const gchar *label_name )
{
    GtkToolItem *button = NULL;
    gchar *filename;

    filename = g_build_filename ( gsb_dirs_get_pixmaps_dir (), image_name, NULL );
    if ( filename )
    {
        GtkWidget *image;

        image = gtk_image_new_from_file ( filename );
        g_free ( filename );
        button = gtk_tool_button_new ( image, label_name );
    }

    return button;
}

/**
 * Création d'un GtkToolButton à partir d'une ressource et d'un label
 *
 * \param image_name    resource of button
 * \param label_name    label for button
 *
 * \return a GtkToolItem or NULL
 * */
GtkToolItem *utils_buttons_tool_button_new_from_image_resource (const gchar *image_name,
																const gchar *label_name)
{
	GtkToolItem *button = NULL;
	GtkWidget *image;
	gchar *resource;

	resource = g_strconcat ("/org/gtk/grisbi/images/", image_name, NULL);

	image = gtk_image_new_from_resource (resource);
	g_free (resource);
	button = gtk_tool_button_new (image, label_name);

    return button;
}

/**
 * Création d'un GtkMenuToolButton à partir d'une image et d'un label
 *
 * \param image_name    filename
 * \param label_name    label for button
 *
 * \return a GtkToolItem or NULL
 * */
GtkToolItem *utils_buttons_tool_menu_new_from_image_label ( const gchar *image_name,
                        const gchar *label_name )
{
    GtkToolItem *button = NULL;
    GtkWidget *image;
    gchar *filename;

    filename = g_build_filename ( gsb_dirs_get_pixmaps_dir (), image_name, NULL );
    if ( filename )
    {
        image = gtk_image_new_from_file ( filename );
        g_free ( filename );
        button = gtk_menu_tool_button_new ( image, label_name );
    }

    return button;
}

/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
