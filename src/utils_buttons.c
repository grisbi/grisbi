/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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


GtkWidget * new_stock_image_label ( GsbButtonStyle style, const gchar * stock_id, const gchar * name )
{
    GtkWidget * vbox, *hbox, * label, * image;

    vbox = gtk_vbox_new ( FALSE, 0 );
    hbox = gtk_hbox_new ( FALSE, 5 );

    /* Define image */
    if ( style == GSB_BUTTON_ICON || style == GSB_BUTTON_BOTH )
    {
	image = gtk_image_new_from_stock ( stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR );
	gtk_box_pack_start ( GTK_BOX(vbox), image, TRUE, TRUE, 0 );
    }
    else if (style == GSB_BUTTON_BOTH_HORIZ )
    {
        image = gtk_image_new_from_stock ( stock_id, 
                        GTK_ICON_SIZE_SMALL_TOOLBAR );
        gtk_box_pack_start ( GTK_BOX(hbox), image, TRUE, TRUE, 0 );
    }

    /* Define label */
    if ( style == GSB_BUTTON_TEXT || style == GSB_BUTTON_BOTH )
    {
	label = gtk_label_new ( name );
	gtk_label_set_text_with_mnemonic ( GTK_LABEL(label), name );
	gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_CENTER );
	gtk_box_pack_start ( GTK_BOX(vbox), label, TRUE, TRUE, 0 );
    }
    else if (style == GSB_BUTTON_BOTH_HORIZ )
    {
        label = gtk_label_new ( name );
        gtk_label_set_text_with_mnemonic ( GTK_LABEL(label), name );
        gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_CENTER );
        gtk_box_pack_start ( GTK_BOX(hbox), label, TRUE, TRUE, 0 );
    }

    if (style == GSB_BUTTON_BOTH_HORIZ )
    {
        gtk_box_pack_start ( GTK_BOX(vbox), hbox, TRUE, TRUE, 0 );
    }

    return vbox;
}




GtkWidget * new_image_label ( GsbButtonStyle style, const gchar * image_name, const gchar * name )
{
    GtkWidget * vbox, *hbox, * label, * image;

    vbox = gtk_vbox_new ( FALSE, 0 );
    hbox = gtk_hbox_new ( FALSE, 0 );

    /* Define image */
    if ( style == GSB_BUTTON_ICON || style == GSB_BUTTON_BOTH )
    {
	image = gtk_image_new_from_file (g_build_filename ( gsb_dirs_get_pixmaps_dir ( ),
							   image_name, NULL));
	gtk_box_pack_start ( GTK_BOX(vbox), image, TRUE, TRUE, 0 );
    }
    else if (style == GSB_BUTTON_BOTH_HORIZ )
    {
        image = gtk_image_new_from_file (g_build_filename ( gsb_dirs_get_pixmaps_dir ( ),
							   image_name, NULL));
        gtk_box_pack_start ( GTK_BOX(hbox), image, TRUE, TRUE, 0 );
    }

    /* Define label */
    if ( style == GSB_BUTTON_TEXT || style == GSB_BUTTON_BOTH )
    {
	label = gtk_label_new ( name );
	gtk_label_set_text_with_mnemonic ( GTK_LABEL(label), name );
	gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );
	gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_CENTER );
	gtk_box_pack_start ( GTK_BOX(vbox), label, TRUE, TRUE, 0 );
    }
    else if (style == GSB_BUTTON_BOTH_HORIZ )
    {
        label = gtk_label_new ( name );
        gtk_label_set_text_with_mnemonic ( GTK_LABEL(label), name );
        gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );
        gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_CENTER );
        gtk_box_pack_start ( GTK_BOX(hbox), label, TRUE, TRUE, 0 );
    }

    if (style == GSB_BUTTON_BOTH_HORIZ )
    {
        gtk_box_pack_start ( GTK_BOX(vbox), hbox, TRUE, TRUE, 0 );
    }
    gtk_widget_show_all ( vbox );

    return vbox;
}



/**
 * TODO: document
 * Borrowed from the Gimp Toolkit and modified.
 *
 */
void set_popup_position (GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
    GtkWidget *widget;
    GtkRequisition requisition;
    GtkAllocation allocation;
    gint screen_width, menu_xpos, menu_ypos, menu_width;

    widget = GTK_WIDGET ( user_data );

    gtk_widget_get_child_requisition ( GTK_WIDGET ( menu ), &requisition );
    menu_width = requisition.width;

    gdk_window_get_origin ( gtk_widget_get_window ( widget ), &menu_xpos, &menu_ypos );

    gtk_widget_get_allocation ( widget, &allocation );
    menu_xpos += allocation.x;
    menu_ypos += allocation.y + allocation.height - 2;

    if ( gtk_widget_get_direction ( widget) == GTK_TEXT_DIR_RTL )
        menu_xpos = menu_xpos + allocation.width - menu_width;

    /* Clamp the position on screen */
    screen_width = gdk_screen_get_width ( gtk_widget_get_screen ( widget ) );

    if (menu_xpos < 0)
        menu_xpos = 0;
    else if ( ( menu_xpos + menu_width ) > screen_width )
        menu_xpos -= ((menu_xpos + menu_width) - screen_width);

    *x = menu_xpos;
    *y = menu_ypos;
    *push_in = TRUE;
}


/**
 * called by a gsb_automem_checkbutton_new or a g_signal_connect on a checkbutton
 * sensitive or unsensitive tha param widget, according to the checkbutton
 *
 * \param check_button
 * \param widget
 *
 * \return FALSE
 * */
gboolean gsb_button_sensitive_by_checkbutton ( GtkWidget *check_button,
                        GtkWidget *widget )
{
    gtk_widget_set_sensitive ( widget,
                        gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button ) ) );
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

    if ( gtk_toggle_button_get_active ( togglebutton ) )
    {
        gtk_widget_hide ( hbox_expand );
        gtk_widget_show ( hbox_collapse );
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


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
