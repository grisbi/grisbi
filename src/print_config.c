/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2008 Benjamin Drieu (bdrieu@april.org)		      */
/*			2008 Cedric Auger (cedric@grisbi.org)		      */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "print_config.h"
#include "utils_file_selection.h"
#include "utils_files.h"
#include "utils_str.h"
#include "utils.h"
#include "dialog.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget * print_config_general ( GtkWidget * dialog );
static GtkWidget * print_config_paper ( GtkWidget * dialog );
static gboolean print_config_radio_toggled ( GtkToggleButton * togglebutton, gpointer user_data );
/*END_STATIC*/





/** This array contains paper sizes to be used to construct print
  copies of reports. */
struct paper_config paper_sizes[8] = {
    {N_("A3"), 297, 420},
    {N_("A4"), 210, 297},
    {N_("A5"), 148, 210},
    {N_("A6"), 105, 148},
    {N_("USletter"), 215, 279.4},
    {N_("USlegal"), 215, 355.6},
    {N_("USexecutive"), 184.15, 266.7},
    {NULL, 0, 0},
};

/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/



/* FIXME : remove all of that when debian stable goes into gtk 2.10 */
/**
 * Open a dialog window which asks for information about paper,
 * margins, etc..
 *
 * \return TRUE if print has been setup correctly, FALSE otherwise
 *         (most likely user canceled print).
 */
gboolean print_config ( )
{
    GtkWidget * dialog, *notebook;
    gint response;

    /* Set up dialog */
    dialog = gtk_dialog_new_with_buttons ( _("Print"),
					   GTK_WINDOW ( window ),
					   GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR | GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_PRINT, GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );

    /* Insert notebook */
    notebook = gtk_notebook_new ();
    gtk_box_pack_start ( GTK_BOX(GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 6 );

    /* Add tabs */
    gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), print_config_general(dialog),
			       gtk_label_new (_("General")) );

    gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), print_config_paper(dialog),
			       gtk_label_new (_("Paper")) );

    /*   gtk_notebook_append_page ( GTK_NOTEBOOK(notebook), print_config_appearance(dialog),  */
    /* 			     gtk_label_new (_("Appearance")) ); */

    /* Finish dialog */
    gtk_widget_show_all ( dialog );

    do {
	response = gtk_dialog_run ( GTK_DIALOG (dialog) );
	if ( response == GTK_RESPONSE_OK )
	{
	    FILE * test;
	    gchar * filename;

	    if ( etat.print_config.printer )
	    {
		break;
	    }

	    filename = my_strdup ( gtk_entry_get_text ( GTK_ENTRY (g_object_get_data(G_OBJECT(dialog),
										     "printer_filename") ) ) );

	    if ( ! filename )
	    {
		filename = "";
	    }

	    test = utf8_fopen ( filename, "r" );
	    if ( test )
	    {
		fclose ( test );
		if ( question_yes_no_hint ( g_strdup_printf ( _("File %s already exists."),
							      filename ),
					    _("Do you want to overwrite it?  There is no undo for this."),
					    GTK_RESPONSE_NO  ) )
		{
		    break;
		}
	    }
	    else
	    {
		break;
	    }
	}
	else
	{
	    break;
	}
    }
    while ( 1 );


    if ( response == GTK_RESPONSE_OK )
    {
	etat.print_config.printer = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (g_object_get_data(G_OBJECT(dialog), "printer")) );
	etat.print_config.printer_name = my_strdup ( gtk_entry_get_text ( GTK_ENTRY (g_object_get_data(G_OBJECT(dialog), "printer_name") )));
	etat.print_config.printer_filename = my_strdup ( gtk_entry_get_text ( GTK_ENTRY (g_object_get_data(G_OBJECT(dialog), "printer_filename") )));

	etat.print_config.filetype = gtk_option_menu_get_history ( GTK_OPTION_MENU (g_object_get_data(G_OBJECT(dialog), "filetype") ));

	etat.print_config.paper_config = paper_sizes [ gtk_option_menu_get_history ( GTK_OPTION_MENU (g_object_get_data(G_OBJECT(dialog), "paper_size"))) ];

	etat.print_config.orientation = !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (g_object_get_data(G_OBJECT(dialog), "orientation")) );
    }

    gtk_widget_destroy ( dialog );

    return response == GTK_RESPONSE_OK;
}



/**
 * Handler triggered when user validates a file selector window.
 *
 * \param button Widget that triggered this event.
 * \param data Pointer to a GtkEntry to fill with the result of the
 *             file selector window
 *
 * \return FALSE to allow other handlers to be processed
 */
/* TODO dOm : this function seems not to be used. Is it possible to remove it
gboolean change_print_to_file ( GtkButton *button, gpointer data )
{
    GtkFileSelection * file_selector;

    file_selector = g_object_get_data ( G_OBJECT(button), "entry" );
    gtk_entry_set_text ( GTK_ENTRY(data),
			 file_selection_get_filename (GTK_FILE_CHOOSER (file_selector)));

    return FALSE;
}
*/



/**
 * Create a GtkVbox with stuff needed for general configuration of
 * print.
 *
 * \return a pointer to a newly created GtkVbox
 */
GtkWidget * print_config_general ( GtkWidget * dialog )
{
    GtkWidget *vbox, *paddingbox, *table, *radio1, *radio2;
    GtkWidget *input1, *input2, *omenu, *menu, *item;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );


    /* Printer paddingbox */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Printer") );

    table = gtk_table_new ( 2, 3, FALSE );
    gtk_box_pack_start ( GTK_BOX(paddingbox), table, FALSE, FALSE, 0 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 12 );

    /* Print to printer */
    radio1 = gtk_radio_button_new_with_label ( NULL, _("Printer") );
    gtk_table_attach ( GTK_TABLE(table), radio1, 0, 1, 0, 1,
		       GTK_SHRINK, GTK_SHRINK,
		       0, 0 );
    g_object_set_data ( G_OBJECT(dialog), "printer", radio1 );
    g_object_set_data ( G_OBJECT(radio1), "printer_value", GINT_TO_POINTER (1));
    g_signal_connect ( G_OBJECT(radio1), "toggled",
		       (GCallback) print_config_radio_toggled, NULL );
    input1 = gtk_entry_new ( );
    gtk_table_attach_defaults ( GTK_TABLE(table), input1, 1, 2, 0, 1 );
    g_object_set_data ( G_OBJECT(dialog), "printer_name", input1 );
    gtk_entry_set_text ( GTK_ENTRY(input1), etat.print_config.printer_name );

    /* Print to file */
    radio2 = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (GTK_RADIO_BUTTON(radio1)), _("File") );
    gtk_table_attach ( GTK_TABLE(table), radio2, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(radio2), !etat.print_config.printer );
    g_object_set_data ( G_OBJECT(radio2), "printer_value", 0 );
    g_signal_connect ( G_OBJECT(radio2), "toggled",
		       (GCallback) print_config_radio_toggled, NULL );

    input2 = my_file_chooser ();
    gtk_table_attach_defaults ( GTK_TABLE(table), input2, 1, 2, 1, 2 );
    g_object_set_data ( G_OBJECT(dialog), "printer_filename",
			g_object_get_data (G_OBJECT(input2), "entry") );
    gtk_entry_set_text ( GTK_ENTRY(g_object_get_data (G_OBJECT(input2), "entry")),
			 etat.print_config.printer_filename );

    /* Output file format */
    omenu = gtk_option_menu_new();
    menu = gtk_menu_new();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(omenu), menu );
    g_object_set_data ( G_OBJECT(dialog), "filetype", omenu );

    item = gtk_menu_item_new_with_label ( _("Postscript file") );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    item = gtk_menu_item_new_with_label ( _("LaTeX file") );
    gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    gtk_option_menu_set_history ( GTK_OPTION_MENU(omenu), etat.print_config.filetype );
    gtk_table_attach_defaults ( GTK_TABLE(table), omenu, 1, 2, 2, 3 );

    /* Set pointers to widget that need to be (in)sensitived */
    g_object_set_data ( G_OBJECT(radio1), "peer1", input2 );
    g_object_set_data ( G_OBJECT(radio1), "peer2", omenu );
    g_object_set_data ( G_OBJECT(radio2), "peer1", input1 );
    g_object_set_data ( G_OBJECT(radio2), "peer2", NULL );
    print_config_radio_toggled ( GTK_TOGGLE_BUTTON(radio1), NULL );
    print_config_radio_toggled ( GTK_TOGGLE_BUTTON(radio2), NULL );

    return vbox;
}



/**
 * Create a GtkVbox with stuff needed for paper config print.
 *
 * \return a pointer to a newly created GtkVbox
 */
GtkWidget * print_config_paper ( GtkWidget * dialog )
{
    GtkWidget *vbox, *paddingbox, *omenu, *menu, *item, *radio;
    gint i, history;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    /* Paper size */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Size") );

    omenu = gtk_option_menu_new();
    gtk_box_pack_start ( GTK_BOX(paddingbox), omenu, FALSE, FALSE, 0 );

    menu = gtk_menu_new();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU(omenu), menu );
    g_object_set_data ( G_OBJECT(dialog), "paper_size", omenu );
    history = 0;

    for ( i = 0; paper_sizes[i].name; i++ )
    {
	item = gtk_menu_item_new_with_label (g_strdup_printf ("%s (%2.1fcm × %2.1fcm)",
							      _(paper_sizes[i].name),
							      paper_sizes[i].width/10,
							      paper_sizes[i].height/10));
	if ( !strcmp ( _(paper_sizes[i].name), etat.print_config.paper_config.name ))
	    history = i;

	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
    }
    gtk_option_menu_set_history ( GTK_OPTION_MENU(omenu), history );

    /* Paper orientation */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Orientation") );

    radio = gtk_radio_button_new_with_label ( NULL, _("Portrait") );
    gtk_box_pack_start ( GTK_BOX(paddingbox), radio, FALSE, FALSE, 0 );

    radio = gtk_radio_button_new_with_label ( gtk_radio_button_get_group (GTK_RADIO_BUTTON(radio)), _("Landscape") );
    gtk_box_pack_start ( GTK_BOX(paddingbox), radio, FALSE, FALSE, 0 );
    g_object_set_data ( G_OBJECT(dialog), "orientation", radio );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(radio), !etat.print_config.orientation );

    return vbox;
}



/**
 * Create a GtkVbox with stuff needed for appearance.
 *
 * \return a pointer to a newly created GtkVbox
 */
/* TODO dOm : this function seems not to be used. Is it possible to remove it
GtkWidget * print_config_appearance ( GtkWidget * dialog )
{
    GtkWidget *vbox, *paddingbox;

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 12 );

    !* Headers *!
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Headers") );

    !* Footers *!
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Footers") );

    !* Misc *!
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Misc") );

    return vbox;
}

*/


/**
 * Set associated widgets to a togglebutton (in)sensitive regarding to
 * its state.  Associated widgets are pointers referenced with the
 * "peer1" and "peer2" properties of togglebutton.
 *
 * \param togglebutton  Toggle button to look associated widgets from.
 * \param user_data	Not used.
 *
 * \return FALSE to allow other handlers to be executed
 */
gboolean print_config_radio_toggled ( GtkToggleButton * togglebutton, gpointer user_data )
{
  GtkWidget *peer1, *peer2;
  gboolean value;

  value = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT(togglebutton), "printer_value" ));
  peer1 = g_object_get_data ( G_OBJECT(togglebutton), "peer1" );
  peer2 = g_object_get_data ( G_OBJECT(togglebutton), "peer2" );

  if ( peer1 && GTK_IS_WIDGET(peer1) )
    {
      gtk_widget_set_sensitive ( peer1, !gtk_toggle_button_get_active(togglebutton) );
    }
  if ( peer2 && GTK_IS_WIDGET(peer2) )
    {
      gtk_widget_set_sensitive ( peer2, !gtk_toggle_button_get_active(togglebutton) );
    }

  etat.print_config.printer = value;

  return FALSE;
}



/**
 * Make a backup of print configuration and return it.  Very handy to
 * make temporary backups.
 *
 * \return A newly-allocated print configuration.
 */
struct print_config * print_config_dup ()
{
    return g_memdup ( &(etat.print_config), sizeof ( struct print_config ) );
}



/**
 * Set the print configuration according to print_config structure
 * passed in argument.
 *
 * \param print_config	Print configuration to set.
 *
 */
void print_config_set ( struct print_config * config )
{
    g_memmove ( &(etat.print_config), config, sizeof ( struct print_config ) );
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
