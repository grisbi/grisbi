/* ************************************************************************** */
/*                                  utils_buttons.c                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org)	      */
/* 			http://www.grisbi.org				      */
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

#include "include.h"


/*START_INCLUDE*/
#include "utils_buttons.h"
#include "utils.h"
#include "traitement_variables.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean set_boolean ( GtkWidget * checkbox, guint * dummy);
static gboolean set_double ( GtkWidget * spin, gdouble * dummy);
static GtkWidget * new_stock_image_label ( gchar * stock_id, gchar * name );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/









/**
 * Create a GtkCheckButton with a callback associated.  Initial value
 * of this checkbox is set to the value of *data.  This checkbox calls
 * set_boolean upon toggle, which in turn modifies *data.  If a hook
 * is possibly executed as well.
 *
 * \param label The label for this checkbutton
 * \param data A pointer to a boolean integer
 * \param hook A GCallBack to execute if not null
 * \return A newly allocated GtkVBox
 */
GtkWidget *new_checkbox_with_title ( gchar * label, guint * data, GCallback hook)
{
    GtkWidget * checkbox;

    checkbox = gtk_check_button_new_with_label ( label );
    checkbox_set_value ( checkbox, data, TRUE );

    g_object_set_data ( G_OBJECT (checkbox), "set-boolean", 
			(gpointer) g_signal_connect (checkbox, "toggled",
						     ((GCallback) set_boolean), data));

    if ( hook )
    {
	g_object_set_data ( G_OBJECT ( checkbox ), "hook", 
			    (gpointer) g_signal_connect (checkbox, "toggled",
							 ((GCallback) hook), data ));
    }

    return checkbox;
}



/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * change the content of an integer passed as an argument.
 *
 * \param parent A widget to pack all created radio buttons in
 * \param title The title for this group
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param data A pointer to an integer that will be set to 0 or 1
 *        according to buttons toggles.
 * \param hook An optional hook to run at each toggle
 *
 * \return A newly created paddingbox
 */
GtkWidget *new_radiogroup_with_title (GtkWidget * parent,
				      gchar * title, gchar * choice1, gchar * choice2,
				      guint * data, GCallback hook)
{
    GtkWidget * button1, *button2, *paddingbox;

    paddingbox = new_paddingbox_with_title (parent, FALSE, COLON(title));

    button1 = gtk_radio_button_new_with_label ( NULL, choice1 );
    gtk_box_pack_start (GTK_BOX(paddingbox), button1, FALSE, FALSE, 0 );
    button2 = gtk_radio_button_new_with_label ( gtk_radio_button_group (GTK_RADIO_BUTTON(button1)), 
						choice2 );
    gtk_box_pack_start (GTK_BOX(paddingbox), button2, FALSE, FALSE, 0 );

    if (data)
    {
	if (*data)
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button2 ), TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button1 ), TRUE );
    }

    g_object_set_data ( G_OBJECT ( button2 ), "pointer", data);
    g_signal_connect ( GTK_OBJECT ( button2 ), "toggled",
		       (GCallback) set_boolean, NULL );

    if (hook)
    {
	g_signal_connect ( GTK_OBJECT ( button2 ), "toggled",
			   (GCallback) hook, data );
    }

    return paddingbox;
}


/**
 * Update the widget's appearance accordingly.  If update is set, update
 * property as well.
 * 
 * \param checkbox The checkbox to update
 * \param data A pointer to a boolean which contains the new value to
 * fill in checkbox's properties.  This boolean will be modified by
 * checkbox's handlers as well.
 * \param update Whether to update checkbox's data as well.
 */
void checkbox_set_value ( GtkWidget * checkbox, guint * data, gboolean update )
{
    if (data)
    {
	if (g_object_get_data (G_OBJECT(checkbox), "hook") > 0)
	    g_signal_handler_block ( checkbox, 
				     (gulong) g_object_get_data (G_OBJECT(checkbox), 
								 "hook" ));
	if (g_object_get_data (G_OBJECT(checkbox), "set-boolean") > 0)
	    g_signal_handler_block ( checkbox,
				     (gulong) g_object_get_data (G_OBJECT(checkbox),
								 "set-boolean" ));
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( checkbox ), *data );
	if (g_object_get_data (G_OBJECT(checkbox), "hook") > 0)
	    g_signal_handler_unblock ( checkbox, 
				       (gulong) g_object_get_data (G_OBJECT(checkbox),
								   "hook" ));
	if (g_object_get_data (G_OBJECT(checkbox), "set-boolean") > 0)
	    g_signal_handler_unblock ( checkbox,
				       (gulong) g_object_get_data ( G_OBJECT(checkbox),
								    "set-boolean" ));
    }

    if (update)
	g_object_set_data ( G_OBJECT ( checkbox ), "pointer", data);

}




/**
 * Set a boolean integer to the value of a checkbox.  Normally called
 * via a GTK "toggled" signal handler.
 * 
 * \param checkbox a pointer to a checkbox widget.
 * \param data a pointer to an integer that is to be modified.
 */
gboolean set_boolean ( GtkWidget * checkbox, guint * dummy)
{
    gboolean *data;

    data = g_object_get_data ( G_OBJECT ( checkbox ), "pointer");
    if (data)
	*data = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbox));

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}




/**
 * Creates a new GtkSpinButton with a pointer to a string that will be
 * modified according to the spin's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * textview's contents are modified.
 */
GtkWidget * new_spin_button ( gint * value, 
			      gdouble lower, gdouble upper, 
			      gdouble step_increment, gdouble page_increment, 
			      gdouble page_size, 
			      gdouble climb_rate, guint digits,
			      GCallback hook )
{
    GtkWidget * spin;
    GtkAdjustment * adjustment;
    gdouble initial = 0;

    if ( value )  /* Sanity check */
	initial = *value;

    adjustment = GTK_ADJUSTMENT( gtk_adjustment_new ( initial, lower, upper, 
						      step_increment, page_increment,
						      page_size ));
    spin = gtk_spin_button_new ( adjustment, climb_rate, digits );
    gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON (spin), TRUE );
    g_object_set_data ( G_OBJECT (spin), "pointer", value);
    g_object_set_data ( G_OBJECT (spin), "adj", adjustment);

    g_object_set_data ( G_OBJECT (spin), "value-changed",
			(gpointer) g_signal_connect_swapped ( GTK_OBJECT (adjustment),
							      "value-changed", 
							      (GCallback) set_double, spin));
    if ( hook )
    {
	g_object_set_data ( G_OBJECT (spin), "hook",
			    (gpointer) g_signal_connect_swapped ( GTK_OBJECT (adjustment), 
								  "value-changed", 
								  (GCallback) hook, spin ));
    }

    return spin;
}



/**
 *  TODO: document
 *
 */
void spin_button_set_value ( GtkWidget * spin, gdouble * value )
{
    GtkAdjustment * adjustment;

    adjustment = g_object_get_data ( G_OBJECT(spin), "adj" );
    if (!adjustment)
	return;

    /* Block everything */
    if ( g_object_get_data ((GObject*) spin, "value-changed") > 0 )
	g_signal_handler_block ( GTK_OBJECT(adjustment),
				 (gulong) g_object_get_data ((GObject*) spin, 
							     "value-changed"));
    if ( g_object_get_data ((GObject*) spin, "hook") > 0 )
	g_signal_handler_block ( GTK_OBJECT(adjustment),
				 (gulong) g_object_get_data ((GObject*) spin, "hook"));

    if (value)
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spin), *value);
    else
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spin), 0);

    g_object_set_data ( G_OBJECT(spin), "pointer", value);

    /* Unblock everything */
    if ( g_object_get_data ((GObject*) spin, "value-changed") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(adjustment),
				   (gulong) g_object_get_data ((GObject*) spin, 
							       "value-changed"));
    if ( g_object_get_data ((GObject*) spin, "hook") > 0 )
	g_signal_handler_unblock ( GTK_OBJECT(adjustment),
				   (gulong) g_object_get_data ((GObject*) spin, "hook"));
}



/**
 * Set an integer to the value of a spin button.  Normally called via
 * a GTK "changed" signal handler.
 * 
 * \param spin a pointer to a spinbutton widget.
 * \param dummy unused
 */
gboolean set_double ( GtkWidget * spin, gdouble * dummy)
{
   gint *data;

    data = g_object_get_data ( G_OBJECT(spin), "pointer" );

    if ( data )
    {
	*data = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(spin) );
    }

	/* Mark file as modified */
    modification_fichier ( TRUE );
    return (FALSE);
}



/* **************************************************************************************************************************** */
GtkWidget *cree_bouton_url ( const gchar *adr,
			     const gchar *inscription )
{
    GtkWidget *bouton;

    bouton = gtk_button_new_with_label ( inscription );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_data ( G_OBJECT ( bouton ),
			    "clicked",
			    G_CALLBACK ( lance_navigateur_web ),
			    g_strdup ( adr),
			    NULL,
			    G_CONNECT_SWAPPED );
    return ( bouton );

}
/* **************************************************************************************************************************** */



/**
 * TODO : document
 *
 */
GtkWidget * new_stock_button_with_label ( gchar * stock_id, gchar * name, 
					  GCallback callback )
{
    GtkWidget * button, *vbox;

    vbox = new_stock_image_label ( stock_id, name );

    button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON(button), GTK_RELIEF_NONE );

    gtk_container_add ( GTK_CONTAINER(button), vbox );
    gtk_widget_show_all ( button );

    if ( callback )
    {
	g_signal_connect ( G_OBJECT(button), "clicked", G_CALLBACK(callback), NULL );
    }
    return button;
}



GtkWidget * new_stock_button_with_label_menu ( gchar * stock_id, gchar * name, 
					       GCallback callback )
{
    GtkWidget * button, * vbox, * hbox, * arrow;

    vbox = new_stock_image_label ( stock_id, name );

    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), vbox, FALSE, FALSE, 0 );

    arrow = gtk_arrow_new ( GTK_ARROW_DOWN, GTK_SHADOW_NONE );
    gtk_box_pack_start ( GTK_BOX(hbox), arrow, FALSE, FALSE, 0 );

    button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON(button), GTK_RELIEF_NONE );

    gtk_container_add ( GTK_CONTAINER(button), hbox );
    gtk_widget_show_all ( button );

    if ( callback )
    {
	g_signal_connect ( G_OBJECT(button), "clicked", G_CALLBACK(callback), NULL );
    }

    return button;
}


GtkWidget * new_stock_image_label ( gchar * stock_id, gchar * name )
{
    GtkWidget * vbox, * label, * image;

    /* Define label */
    label = gtk_label_new ( name );
    gtk_label_set_text_with_mnemonic ( GTK_LABEL(label), name );

    /* define image */
    image = gtk_image_new_from_stock ( stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR );
    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(vbox), image, FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(vbox), label, FALSE, FALSE, 0 );

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
  gint screen_width, menu_xpos, menu_ypos, menu_width;

  widget = GTK_WIDGET (user_data);

  gtk_widget_get_child_requisition (GTK_WIDGET (menu), &requisition);
  menu_width = requisition.width;

  gdk_window_get_origin (widget->window, &menu_xpos, &menu_ypos);

  menu_xpos += widget->allocation.x;
  menu_ypos += widget->allocation.y + widget->allocation.height - 2;

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    menu_xpos = menu_xpos + widget->allocation.width - menu_width;

  /* Clamp the position on screen */
  screen_width = gdk_screen_get_width (gtk_widget_get_screen (widget));
  
  if (menu_xpos < 0)
    menu_xpos = 0;
  else if ((menu_xpos + menu_width) > screen_width)
    menu_xpos -= ((menu_xpos + menu_width) - screen_width);

  *x = menu_xpos;
  *y = menu_ypos;
  *push_in = TRUE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
