/* ************************************************************************** */
/*                                  gsb_automem.c                             */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cedric Auger (cedric@grisbi.org)	          */
/*			2003-2009 Benjamin Drieu (bdrieu@april.org)	                      */
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
 * \file gsb_automem.c
 * this file groups some widget functions wich update value automatickly
 * when the widget change
 * a change in the widget change directly the value in memory
 * (for changes on internal structure in gsb_data_x_set_x, see gsb_autofunc.c)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "gsb_automem.h"
#include "utils_editables.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils_buttons.h"
#include "utils.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_automem_checkbutton_changed ( GtkWidget *checkbutton,
						  gpointer null );
static gboolean gsb_automem_entry_changed (GtkWidget *entry,
					   gpointer null );
static gboolean gsb_automem_spin_button_changed ( GtkWidget *spin,
						  gpointer null);
static GtkWidget *gsb_automem_spin_button_new_full ( gint *value, 
					      gdouble lower, gdouble upper, 
					      gdouble step_increment, gdouble page_increment, 
					      gdouble page_size, 
					      gdouble climb_rate,
					      GCallback hook, gpointer data );
static gboolean gsb_automem_textview_changed ( GtkTextBuffer *buffer,
					       gpointer null );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/




/*
 * Creates a new GtkEntry with a pointer to a string that will be
 * modified according to the entry's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * 	entry's contents are modified. (hook must be func ( GtkWidget *entry, gpointer data ) )
 * \param data An optional pointer to pass to hooks.
 */
GtkWidget *gsb_automem_entry_new ( gchar **value,
				   GCallback hook,
				   gpointer data )
{
    GtkWidget *entry;

    entry = gtk_entry_new ();

    if (value && *value)
	gtk_entry_set_text ( GTK_ENTRY(entry),
			     *value );

    g_object_set_data ( G_OBJECT ( entry ),
			"pointer", value);
    g_object_set_data ( G_OBJECT (entry), "changed",
			(gpointer) g_signal_connect_after ( G_OBJECT(entry), "changed",
							    G_CALLBACK (gsb_automem_entry_changed), NULL));
    if (hook)
	g_object_set_data ( G_OBJECT (entry), "changed-hook",
			    (gpointer) g_signal_connect_after ( G_OBJECT(entry), "changed",
								G_CALLBACK (hook), data));
    return entry;
}



/** 
 * set the value in an gsb_automem_entry
 *
 * \param entry the gsb_automem_enty
 * \param value a pointer to a string
 *
 * \return
 */
/* TODO dOm : this function seems not to be used. Is it possible to remove it ?
void gsb_automem_entry_set_value ( GtkWidget *entry,
				   gchar **value )
{
    !* Block everything *!
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_block ( G_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_block ( G_OBJECT(entry),
				 (gulong) g_object_get_data (G_OBJECT (entry), 
							     "changed-hook"));

    !* Fill in value *!
    if (value && *value)
	gtk_entry_set_text ( GTK_ENTRY (entry), *value );
    else
	gtk_entry_set_text ( GTK_ENTRY (entry), "" );

    g_object_set_data ( G_OBJECT(entry),
			"pointer", value );

    !* Unblock everything *!
    if ( g_object_get_data (G_OBJECT (entry), "changed") > 0 )
	g_signal_handler_unblock ( G_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed"));
    if ( g_object_get_data (G_OBJECT (entry), "changed-hook") > 0 )
	g_signal_handler_unblock ( G_OBJECT(entry),
				   (gulong) g_object_get_data (G_OBJECT (entry), 
							       "changed-hook"));
}
*/



/**
 * called by a "changed" signal in a gsb_automem_entry
 * Set a string to the value of an GtkEntry.
 *
 * \param entry The reference GtkEntry
 * \param null not used
 *
 * \return FALSE
 */
static gboolean gsb_automem_entry_changed (GtkWidget *entry,
					   gpointer null )
{
    gchar **data;

    data = g_object_get_data ( G_OBJECT ( entry ), "pointer");
    if (data)
    {
        if ( *data )
	    g_free ( *data );
	*data = my_strdup (gtk_entry_get_text ( GTK_ENTRY (entry) ));
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }

    return FALSE;
}



/**
 * Creates a new GtkTextView with a pointer to a string that will be
 * modified according to the text view's value.
 *
 * \param value A pointer to a string
 * \param hook An optional function to execute as a handler if the
 * 	textview's contents are modified. (hook must be : func ( GtkTextBuffer *buffer, gpointer data ) )
 * \param data  An optional pointer to pass to hook
 */
GtkWidget *gsb_automem_textview_new ( gchar **value,
				      GCallback hook,
				      gpointer data )
{
    GtkWidget * text_view;
    GtkTextBuffer *buffer;

    if (value)
	text_view = gsb_editable_text_view_new(*value);
    else
	text_view = gsb_editable_text_view_new(NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    g_object_set_data ( G_OBJECT ( buffer ), "pointer", value);

    g_object_set_data ( G_OBJECT ( buffer ), "changed",
			(gpointer) g_signal_connect (G_OBJECT(buffer),
						     "changed",
						     G_CALLBACK (gsb_automem_textview_changed),
						     NULL));
    if ( hook )
	g_object_set_data ( G_OBJECT ( buffer ), "changed-hook",
			    (gpointer) g_signal_connect (G_OBJECT(buffer),
							 "changed",
							 G_CALLBACK (hook),
							 data ));
    return text_view;
}


/** 
 * set the value in an gsb_automem_textview
 *
 * \param entry the gsb_automem_enty
 * \param value a pointer to a string
 *
 * \return
 */
/* TODO dOm : this function seems not to be used. Is it possible to remove it 
void gsb_automem_textview_set_value ( GtkWidget *text_view,
				      gchar **value )
{
    GtkTextBuffer *buffer;

    if (!text_view)
	return;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    !* Block everything *!
    if ( g_object_get_data (G_OBJECT(buffer), "changed") > 0 )
	g_signal_handler_block ( G_OBJECT(buffer),
				 (gulong) g_object_get_data ( G_OBJECT(buffer), "changed" ));
    if ( g_object_get_data (G_OBJECT(buffer), "changed-hook") > 0 )
	g_signal_handler_block ( G_OBJECT(buffer),
				 (gulong) g_object_get_data ( G_OBJECT(buffer), 
							      "changed-hook" ));
    !* Fill in value *!
    if (value && *value)
	gtk_text_buffer_set_text (buffer, *value, -1 );
    else
	gtk_text_buffer_set_text (buffer, "", -1 );

    g_object_set_data ( G_OBJECT(buffer), "pointer", value );

    !* Unblock everything *!
    if ( g_object_get_data (G_OBJECT(buffer), "changed") > 0 )
	g_signal_handler_unblock ( G_OBJECT(buffer),
				   (gulong) g_object_get_data ( G_OBJECT(buffer), "changed" ));
    if ( g_object_get_data (G_OBJECT(buffer), "changed-hook") > 0 )
	g_signal_handler_unblock ( G_OBJECT(buffer),
				   (gulong) g_object_get_data ( G_OBJECT(buffer), 
								"changed-hook" ));
}
*/


/**
 * called by a "changed" signal
 * Set a string to the value of an GtkTextView
 *
 * \param buffer The reference GtkTextBuffer
 * \param null Handler parameter.  Not used.
 */
static gboolean gsb_automem_textview_changed ( GtkTextBuffer *buffer,
					       gpointer null )
{
    GtkTextIter start, end;
    gchar **data;

    if (!buffer)
	return FALSE;

    data = g_object_get_data ( G_OBJECT ( buffer ), "pointer");

    gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
    gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

    if (data)
    {
	*data = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }

    return FALSE;
}


/**
 * Create a GtkCheckButton with a callback associated.  Initial value
 * of this checkbutton is set to the value of *value.  This checkbutton calls
 * gsb_automem_checkbutton_changed upon toggle, which in turn modifies *data.  If a hook
 * is possibly executed as well.
 *
 * \param label The label for this checkbutton
 * \param value A pointer to a boolean integer
 * \param hook A GCallBack to execute if not null (hook must be : func ( GtkWidget *toggle_button, gpointer data) )
 * \param data An optional pointer to pass to hook
 *
 * \return A newly allocated GtkCheckButton
 */
GtkWidget *gsb_automem_checkbutton_new ( const gchar *label,
					 gboolean *value,
					 GCallback hook,
					 gpointer data )
{
    GtkWidget *checkbutton;

    checkbutton = gtk_check_button_new_with_mnemonic (label);
    if (value)
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (checkbutton),
				       *value );

    g_object_set_data ( G_OBJECT (checkbutton),
			"pointer", value);

    g_object_set_data ( G_OBJECT (checkbutton), "changed", 
			(gpointer) g_signal_connect (checkbutton, "toggled",
						     G_CALLBACK (gsb_automem_checkbutton_changed), NULL));

    if ( hook )
	g_object_set_data ( G_OBJECT ( checkbutton ), "changed-hook", 
			    (gpointer) g_signal_connect (checkbutton, "toggled",
							 G_CALLBACK (hook), data ));
    return checkbutton;
}



/**
 * Update the widget's appearance accordingly.  If update is set, update
 * property as well.
 * 
 * \param checkbutton The checkbutton to update
 * \param value A pointer to a boolean which contains the new value to
 * 	fill in checkbutton's properties.  This boolean will be modified by
 * 	checkbutton's handlers as well.
 */
/*void gsb_automem_checkbutton_set_value ( GtkWidget *checkbutton,
					 gboolean *value )
{
    // Block everything
    if (g_object_get_data (G_OBJECT(checkbutton), "changed") > 0)
	g_signal_handler_block ( checkbutton, 
				 (gulong) g_object_get_data (G_OBJECT(checkbutton), 
							     "changed" ));
    if (g_object_get_data (G_OBJECT(checkbutton), "changed-hook") > 0)
	g_signal_handler_block ( checkbutton,
				 (gulong) g_object_get_data (G_OBJECT(checkbutton),
							     "changed-hook" ));

    if (value)
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( checkbutton ),
				       *value );
    g_object_set_data ( G_OBJECT (checkbutton),
			"pointer", value);

    // Unblock everything
    if (g_object_get_data (G_OBJECT(checkbutton), "changed") > 0)
	g_signal_handler_unblock ( checkbutton, 
				   (gulong) g_object_get_data (G_OBJECT(checkbutton),
							       "changed" ));
    if (g_object_get_data (G_OBJECT(checkbutton), "changed-hook") > 0)
	g_signal_handler_unblock ( checkbutton,
				   (gulong) g_object_get_data ( G_OBJECT(checkbutton),
								"changed-hook" ));
}*/




/**
 * Set a boolean integer to the value of a checkbutton.  Normally called
 * via a GTK "toggled" signal handler.
 * 
 * \param checkbutton a pointer to a checkbutton widget.
 * \param null not used
 */
static gboolean gsb_automem_checkbutton_changed ( GtkWidget *checkbutton,
						  gpointer null )
{
    gboolean *value;

    value = g_object_get_data ( G_OBJECT (checkbutton), "pointer");
    if (value)
    {
	*value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(checkbutton));
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }

    return FALSE;
}


/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * change the content of an integer passed as an argument.
 * the 2 buttons are set in a paddingbox with a title
 *
 * \param parent A widget to pack all created radio buttons in
 * \param title The title for this group
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param value A pointer to an integer that will be set to 0 or 1
 *        according to buttons toggles.
 * \param hook An optional hook to run at each toggle
 * \param data optional data to send to hook
 *
 * \return A newly created paddingbox
 */
GtkWidget *gsb_automem_radiobutton_new_with_title ( GtkWidget *parent,
					    const gchar *title,
					    const gchar *choice1, const gchar *choice2,
					    gboolean *value,
					    GCallback hook,
                        gpointer data )
{
    GtkWidget *paddingbox;

    paddingbox = new_paddingbox_with_title (parent, FALSE, _(title));
    gtk_box_pack_start (GTK_BOX(paddingbox),
			gsb_automem_radiobutton_new ( choice1, choice2,
						      value,
						      hook, data ), 
			FALSE, FALSE, 0 );
    return paddingbox;
}



/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * change the content of an integer passed as an argument.
 *
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param value A pointer to an integer that will be set to 0 or 1
 *        according to buttons toggles.
 * \param hook An optional hook to run at each toggle
 * \param data optional data to send to hook
 *
 * \return a vbox containing the radiobuttons
 */
GtkWidget *gsb_automem_radiobutton_new ( const gchar *choice1,
					    const gchar *choice2,
					    gboolean *value,
					    GCallback hook,
					    gpointer data )
{
    GtkWidget *button1, *button2, *vbox;

    vbox = gtk_vbox_new ( FALSE, 6 );

    button1 = gtk_radio_button_new_with_mnemonic ( NULL, choice1 );
    gtk_box_pack_start (GTK_BOX(vbox), button1, FALSE, FALSE, 0 );
    button2 = gtk_radio_button_new_with_mnemonic ( gtk_radio_button_get_group (GTK_RADIO_BUTTON(button1)), 
						   choice2 );
    gtk_box_pack_start (GTK_BOX(vbox), button2, FALSE, FALSE, 0 );

    if (value)
    {
	if (*value)
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button2 ), TRUE );
	else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button1 ), TRUE );
    }

    g_object_set_data ( G_OBJECT ( button2 ),
			"pointer", value);
    g_signal_connect ( G_OBJECT ( button2 ), "toggled",
		       G_CALLBACK (gsb_automem_checkbutton_changed), NULL );

    if (hook)
	g_signal_connect ( G_OBJECT ( button2 ), "toggled",
			   G_CALLBACK (hook), data );

    return vbox;
}


/**
 * Creates a new radio buttons group with 3 choices.  Toggling will
 * change the content of an integer passed as an argument.
 * the 3 buttons are set in a paddingbox with a title
 *
 * \param parent A widget to pack all created radio buttons in
 * \param title The title for this group
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param choice3 Second choice label
 * \param value A pointer to an integer that will be set to 0, 1 or 2
 *        according to buttons toggles.
 *
 * \return A newly created paddingbox
 */
GtkWidget *gsb_automem_radiobutton3_new_with_title ( GtkWidget *parent,
						const gchar *title,
					    const gchar *choice1, const gchar *choice2, const gchar *choice3,
					    gint *value,
					    GCallback hook,
					    gpointer data,
                        gint orientation )
{
    GtkWidget *paddingbox;

    paddingbox = new_paddingbox_with_title ( parent, FALSE, title );

    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			gsb_automem_radiobutton3_new ( choice1, choice2, choice3,
						       value, hook, data, orientation ),
			FALSE, FALSE, 0 );
    return paddingbox;
}



/**
 * Creates a new radio buttons group with 3 choices.
 *
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param choice3 Second choice label
 * \param value A pointer to an integer that will be set to 0, 1 or 2
 *        according to buttons toggles.
 * \param callback A callback function to run at each toggle
 * \param data optional data to send to callback
 *
 * \return a hbox containing the radiobuttons
 */

GtkWidget *gsb_automem_radiobutton3_new ( const gchar *choice1,
					    const gchar *choice2,
					    const gchar *choice3,
					    gint *value,
					    GCallback callback,
					    gpointer data,
                        gint orientation )
{
    GtkWidget *box;
    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *button3 = NULL;

    if ( orientation == GTK_ORIENTATION_HORIZONTAL )
        box = gtk_hbox_new ( FALSE, 6 );
    else
        box = gtk_vbox_new ( FALSE, 6 );

    button1 = gtk_radio_button_new_with_mnemonic ( NULL, choice1 );
    gtk_box_pack_start ( GTK_BOX ( box ), button1, FALSE, FALSE, 0 );

    button2 = gtk_radio_button_new_with_mnemonic ( gtk_radio_button_get_group (
                        GTK_RADIO_BUTTON ( button1 ) ),
						choice2 );
    gtk_box_pack_start ( GTK_BOX ( box ), button2, FALSE, FALSE, 0 );

    if ( choice3 && strlen ( choice3 ) )
    {
        button3 = gtk_radio_button_new_with_mnemonic ( gtk_radio_button_get_group (
                        GTK_RADIO_BUTTON ( button1 ) ),
						choice3 );
        gtk_box_pack_start ( GTK_BOX ( box ), button3, FALSE, FALSE, 0 );
    }

    if (value)
    {
        switch ( *value )
        {
            case 0:
                gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button1 ), TRUE );
                break;
            case 1:
                gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button2 ), TRUE );
                break;
            case 2:
                if ( button3 )
                    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button3 ), TRUE );
                else
                    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button2 ), TRUE );
                break;
        }
    }
    else
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button1 ), TRUE );

    /* we associate the value for the buttons */
    g_object_set_data ( G_OBJECT ( button1 ), "pointer", GINT_TO_POINTER ( 0 ) );
    g_object_set_data ( G_OBJECT ( button2 ), "pointer", GINT_TO_POINTER ( 1 ) );
    if ( button3 )
        g_object_set_data ( G_OBJECT ( button3 ), "pointer", GINT_TO_POINTER ( 2 ) );

    if ( callback )
    {
	    g_signal_connect ( G_OBJECT ( button1 ), "button-release-event", G_CALLBACK ( callback ), data );
        g_signal_connect ( G_OBJECT ( button2 ), "button-release-event", G_CALLBACK ( callback ), data );
        if ( button3 )
            g_signal_connect ( G_OBJECT ( button3 ), "button-release-event", G_CALLBACK ( callback ), data );
    }
 
    return box;
}


/**
 * Creates a new GtkSpinButton with a pointer to a string that will be
 * modified according to the spin's value.
 * this is a gint spin button
 * this function is the same as gsb_automem_spin_button_new_full, but
 * set defaults parameters
 * by default : between 0 and G_MAXDOUBLE ; value is integer, step increment is 1 and page is 5
 *
 * \param value a pointer to an integer wich will change with the value of the spin button
 * \param hook An optional hook to run at each change
 * \param data optional data to send to hook
 * 
 * \return a GtkSpinButton
 * */
GtkWidget *gsb_automem_spin_button_new ( gint *value, 
					 GCallback hook,
					 gpointer data )
{
    gdouble lower, upper, step_increment, page_increment;
    gdouble page_size, climb_rate;

    lower = 0.0;
    upper = G_MAXDOUBLE;
    step_increment = 1.0;
    page_increment = 5.0;
    page_size = 0.0;
    climb_rate = 2.0;

    return gsb_automem_spin_button_new_full ( value,
					      lower, upper,
					      step_increment, page_increment,
					      page_size, climb_rate,
					      G_CALLBACK (hook), data);
}



/**
 * Creates a new GtkSpinButton with a pointer to a string that will be
 * modified according to the spin's value.
 * this is a gint spin button
 *
 * \param value a pointer to an integer wich will change with the value of the spin button
 * \param lower min value
 * \param upper high value
 * \param step_increment
 * \param page_increment
 * \param page_size
 * \param climb_rate
 * \param hook An optional hook to run at each change
 * \param data optional data to send to hook
 */
GtkWidget *gsb_automem_spin_button_new_full ( gint *value, 
					      gdouble lower, gdouble upper, 
					      gdouble step_increment, gdouble page_increment, 
					      gdouble page_size, 
					      gdouble climb_rate,
					      GCallback hook, gpointer data )
{
    GtkWidget *spin;
    GtkAdjustment *adjustment;
    gint initial = 0;

    if ( value )  /* Sanity check */
	initial = *value;

    adjustment = GTK_ADJUSTMENT( gtk_adjustment_new ( initial, lower, upper, 
						      step_increment, page_increment,
						      page_size ));

    spin = gtk_spin_button_new ( adjustment, climb_rate, 0 );
    gtk_spin_button_set_numeric ( GTK_SPIN_BUTTON (spin), TRUE );
    g_object_set_data ( G_OBJECT (spin), "pointer", value);
    g_object_set_data ( G_OBJECT (spin), "adj", adjustment);

    g_object_set_data ( G_OBJECT (spin), "changed",
			(gpointer) g_signal_connect ( G_OBJECT (spin),
						      "value-changed", 
						      G_CALLBACK (gsb_automem_spin_button_changed), 
						      NULL ));
    if ( hook )
	g_object_set_data ( G_OBJECT (spin), "changed-hook",
			    (gpointer) g_signal_connect ( G_OBJECT (spin), 
							  "value-changed", 
							  G_CALLBACK (hook),
							  data ));
    return spin;
}



/**
 * set the value of a automem spin button
 *
 * \param spin the automem_spin_button
 * \param value a pointer to an integer wich will change with the value of the spin button
 *
 * \return
 *
 */
/* TODO dOm : this function seems not to be used. Is it possible to remove it 
void gsb_automem_spin_button_set_value ( GtkWidget *spin,
					 gint *value )
{
    GtkAdjustment *adjustment;

    if (!spin)
	return;

    adjustment = g_object_get_data ( G_OBJECT(spin), "adj" );
    if (!adjustment)
	return;

    !* Block everything *!
    if ( g_object_get_data (G_OBJECT (spin), "changed") > 0 )
	g_signal_handler_block ( G_OBJECT(adjustment),
				 (gulong) g_object_get_data ( G_OBJECT (spin), 
							      "changed"));
    if ( g_object_get_data (G_OBJECT (spin), "changed-hook") > 0 )
	g_signal_handler_block ( G_OBJECT(adjustment),
				 (gulong) g_object_get_data ( G_OBJECT (spin),
							      "changed-hook"));

    if (value)
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spin), *value);
    else
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(spin), 0);

    g_object_set_data ( G_OBJECT(spin),
			"pointer", value);

    !* Unblock everything *!
    if ( g_object_get_data (G_OBJECT (spin), "changed") > 0 )
	g_signal_handler_unblock ( G_OBJECT(adjustment),
				   (gulong) g_object_get_data ( G_OBJECT (spin), 
								"changed"));
    if ( g_object_get_data (G_OBJECT (spin), "changed-hook") > 0 )
	g_signal_handler_unblock ( G_OBJECT(adjustment),
				   (gulong) g_object_get_data ( G_OBJECT (spin),
								"changed-hook"));
}
*/


/**
 * Set an int to the value of a spin button.  Normally called via
 * a GTK "changed" signal handler.
 * 
 * \param spin a pointer to a spinbutton widget.
 * \param null unused
 */
static gboolean gsb_automem_spin_button_changed ( GtkWidget *spin,
						  gpointer null)
{
    gint *data;

    data = g_object_get_data ( G_OBJECT(spin), "pointer" );

    if ( data )
    {
	*data = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(spin));
	if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );
    }

    return (FALSE);
}



/**
 * TODO : document
 *
 */
GtkWidget *gsb_automem_stock_button_new ( GsbButtonStyle style,
					  const gchar * stock_id, const gchar * name, 
					  GCallback callback, gpointer data )
{
    GtkWidget *button, *vbox;

    vbox = new_stock_image_label ( style, stock_id, name );

    button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON(button), GTK_RELIEF_NONE );

    gtk_container_add ( GTK_CONTAINER(button), vbox );
    gtk_widget_show_all ( button );

    if ( callback )
    {
	if ( data >= 0 )
	{
	    g_signal_connect_swapped ( G_OBJECT(button), "clicked", 
				       G_CALLBACK(callback), data );
	}
	else
	{
	    g_signal_connect ( G_OBJECT(button), "clicked", 
			       G_CALLBACK(callback), data );
	}
    }
    return button;
}



/**
 *TODO : document
 *
 */
GtkWidget *gsb_automem_stock_button_menu_new ( GsbButtonStyle style,
					       const gchar * stock_id, const gchar * name, 
					       GCallback callback, gpointer data )
{
    GtkWidget * button, * vbox, * hbox, * arrow;

    vbox = new_stock_image_label ( style, stock_id, name );

    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), vbox, TRUE, TRUE, 0 );

    arrow = gtk_arrow_new ( GTK_ARROW_DOWN, GTK_SHADOW_NONE );
    gtk_box_pack_start ( GTK_BOX(hbox), arrow, FALSE, FALSE, 0 );

    button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON(button), GTK_RELIEF_NONE );

    gtk_container_add ( GTK_CONTAINER(button), hbox );
    gtk_widget_show_all ( button );

    if ( callback )
    {
	g_signal_connect ( G_OBJECT(button), "button-press-event", 
			   G_CALLBACK(callback), data );
    }

    return button;
}


/**
 *TODO : document
 *
 */
GtkWidget *gsb_automem_imagefile_button_new ( GsbButtonStyle style,
					      const gchar * name, const gchar * filename,
					      GCallback callback, gpointer data )
{
    GtkWidget * button, *vbox;

    vbox = new_image_label ( style, filename, name );

    button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON(button), GTK_RELIEF_NONE );

    gtk_container_add ( GTK_CONTAINER(button), vbox );
    gtk_widget_show_all ( button );

    if ( callback )
    {
	if ( data >= 0 )
	{
	    g_signal_connect_swapped ( G_OBJECT(button), "clicked", 
				       G_CALLBACK(callback), data );
	}
	else
	{
	    g_signal_connect ( G_OBJECT(button), "clicked", 
			       G_CALLBACK(callback), data );
	}
    }
    return button;
}


