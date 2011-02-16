/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
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

/**
 * \file gsb_assistant_first.c
 * we find here the complete assistant launched at the first use of grisbi
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_first.h"
#include "gsb_assistant.h"
#include "gsb_assistant_file.h"
#include "gsb_automem.h"
#include "parametres.h"
#include "gsb_file.h"
#include "traitement_variables.h"
#include "dialog.h"
#include "utils_files.h"
#include "utils.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_assistant_first_enter_page_2 ( GtkWidget *assistant,
                        gint new_page );
static gboolean gsb_assistant_first_enter_page_3 ( GtkWidget *assistant,
                        gint new_page );
static GtkWidget *gsb_assistant_first_page_2 ( GtkWidget *assistant );
static GtkWidget *gsb_assistant_first_page_3 ( GtkWidget *assistant );
/*END_STATIC*/

/*START_EXTERN*/
extern gboolean result_reconcile;
extern GtkWidget *window;
/*END_EXTERN*/

enum first_assistant_page
{
    FIRST_ASSISTANT_INTRO= 0,
    FIRST_ASSISTANT_PAGE_2,
    FIRST_ASSISTANT_PAGE_3,
    FIRST_ASSISTANT_PAGE_FINISH,
};


/**
 * this function is called to launch the first opening assistant
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_first_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Welcome to Grisbi!"),
				    _("This is the first time you use Grisbi, this assistant will guide you through the configuration process. "
				      "You can change any option later in the configuration window, "
				      "where many other options are available.\n\n"
				      "Thanks for using Grisbi, enjoy!" ),
				    "grisbi.png",
				    NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_2 (assistant),
			     FIRST_ASSISTANT_PAGE_2,
			     FIRST_ASSISTANT_INTRO,
			     0,
			     G_CALLBACK (gsb_assistant_first_enter_page_2) );

    /* set up all the default variables */
    init_variables ();

    /* set up the XDG Environment variables for linux*/
    #ifndef _WIN32
        utils_files_create_XDG_dir ( );
    #endif

    /* now we launch the assistant */
    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);

    if (return_value == GTK_RESPONSE_CANCEL)
    {
	/* the user stopped the assistant, we delete all the configured data */
	init_variables ();
	return return_value;
    }

    /* the assistant is finish, we launch the new file assistant */
    return_value = gsb_assistant_file_run (TRUE, FALSE);

    return return_value;
}

/**
 * this function is called to launch the first opening assistant to pass to the 0.6 release
 * the purpose is to choose the directory of autosaving
 * the difference between gsb_assistant_first_run is we show only that first page,
 * don't create anything later
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_first_come_to_0_6 ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Welcome to Grisbi 0.6!"),
				    _("You are using Grisbi 0.6 for the first time.  While most of the " 
				      "interface has not changed, you will notice a lot of improvements. "
				      "Do not hesitate to read some tips of the day to learn more on the new "
				      "features.\n\n" 
				      "Of the most notable enhancements to Grisbi, we redesigned the "
				      "backup function. "
				      "Grisbi will now automatically put backup files in a directory "
				      "that you will be able to set manually in next screen.\n\n"
				      "Also, we advise you to configure your web browser "
				      "to your system configuration and to configure various settings "
				      "in next screen.\n\n" 
				      "If you want to make backups of your Grisbi file in case you want to revert "
				      "to old version of Grisbi, we would advise you to do that right now.\n\n"
				      "You can find out other improvements on http://www.grisbi.org/."),
				    "grisbi.png",
				    NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_2 (assistant),
			     FIRST_ASSISTANT_PAGE_2,
			     FIRST_ASSISTANT_INTRO,
			     FIRST_ASSISTANT_PAGE_3,
			     G_CALLBACK (gsb_assistant_first_enter_page_2) );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_3 (assistant),
			     FIRST_ASSISTANT_PAGE_3,
			     FIRST_ASSISTANT_PAGE_2,
			     0,
			     G_CALLBACK (gsb_assistant_first_enter_page_3) );

    /* now we launch the assistant */
    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);
    return return_value;
}


/**
 * create the page 2 of the first assistant
 * this is the page for general configuration (web browser, automatic opening and saving... )
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_2 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *paddingbox;
    GtkWidget *table;
    GtkSizeGroup *size_group;
    gchar *text;
    GtkWidget *hbox;
    GtkWidget *dialog;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    vbox = new_vbox_with_title_and_icon ( _("General configuration of Grisbi"),
					  "generalities.png" );
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 TRUE, TRUE, 0 );

    /* configure the browser */
    paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Web"));

    table = gtk_table_new ( 0, 3, FALSE );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table, FALSE, FALSE, 0 );
    gtk_table_set_col_spacings ( GTK_TABLE(table), 6 );
    gtk_table_set_row_spacings ( GTK_TABLE(table), 6 );

    label = gtk_label_new ( _("Web browser command: ") );
    gtk_size_group_add_widget ( size_group, label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    entry = gsb_automem_entry_new ( &conf.browser_command, NULL, NULL );
    gtk_table_attach ( GTK_TABLE(table), entry, 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0 );

    text = make_blue ( _("You may use %s to expand the URL - I.e: 'firefox -remote %s' ") );
    label = gtk_label_new ( text );
    gtk_label_set_use_markup ( GTK_LABEL(label), TRUE );
    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.5 );
    gtk_table_attach ( GTK_TABLE(table), label, 1, 2, 2, 3,
                        GTK_SHRINK | GTK_FILL, GTK_SHRINK | GTK_FILL, 0, 0 );
    g_free ( text );

    /* Account file handling */
    paddingbox = new_paddingbox_with_title (vbox, FALSE,
					    _("Account files handling"));

    /* Automatically load last file on startup? */
    button = gsb_automem_checkbutton_new (_("Automatically load last file on startup"),
					  &conf.dernier_fichier_auto, NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, 
			 FALSE, FALSE, 0 );

    /* automatically save file at closing */
    button = gsb_automem_checkbutton_new (_("Automatically save on exit"),
					  &conf.sauvegarde_auto, NULL, NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button, 
			 FALSE, FALSE, 0 );

    /* crypt the grisbi file */
    button = gsb_automem_checkbutton_new ( _("Encrypt Grisbi file"),
					   &(etat.crypt_file), G_CALLBACK (gsb_gui_encryption_toggled), NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button,
			 FALSE, FALSE, 0 );

    if ( etat.crypt_file )
        run.new_crypted_file = TRUE;

    /* Automatic backup ? */
    button = gsb_automem_checkbutton_new (_("Make a backup copy before saving files"),
					  &conf.make_backup, NULL, NULL);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button,
			 FALSE, FALSE, 0 );

    /* Automatic backup every x minutes */
    hbox = gtk_hbox_new ( FALSE, 6);
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    button = gsb_automem_checkbutton_new (_("Make a backup copy every "),
					  &conf.make_backup_every_minutes,
					  G_CALLBACK (gsb_file_automatic_backup_start), NULL);
    gtk_box_pack_start ( GTK_BOX (hbox), button,
			 FALSE, FALSE, 0 );

    button = gsb_automem_spin_button_new ( &conf.make_backup_nb_minutes,
					   G_CALLBACK (gsb_file_automatic_backup_change_time), NULL );
    gtk_box_pack_start ( GTK_BOX (hbox), button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( _(" minutes") );
    gtk_box_pack_start ( GTK_BOX (hbox), label,
			 FALSE, FALSE, 0 );

    /* if automatic backup, choose a dir */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( _("Backup directory: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    dialog = utils_files_create_file_chooser ( window,
                        _("Select/Create backup directory") );
    button = gtk_file_chooser_button_new_with_dialog (dialog);
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (button),
                        my_get_XDG_grisbi_data_dir () );
    g_signal_connect ( G_OBJECT (button),
                        "selection-changed",
                        G_CALLBACK (gsb_config_backup_dir_chosen),
                        dialog );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button,
			 FALSE, TRUE, 0);

    gtk_widget_show_all (page);
    return page;
}


/**
 * create the page 3 of the first assistant
 * this is the page for report error of reconcile import of an old grisbi file
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_3 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *paddingbox;
    GtkSizeGroup *size_group;
    GtkWidget *hbox;
    GtkWidget *image;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page), 10 );

    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    vbox = new_vbox_with_title_and_icon ( _("Reconciliation"), "reconciliationlg.png" );
    gtk_box_pack_start ( GTK_BOX (page), vbox, TRUE, TRUE, 0 );

    paddingbox = new_paddingbox_with_title (vbox, FALSE,
					    _("Error getting reconciliations"));

    hbox = gtk_hbox_new ( FALSE, 15 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 15 );

    image = gtk_image_new_from_stock ( GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_DIALOG );
    gtk_box_pack_start ( GTK_BOX ( hbox ), image, FALSE, FALSE, 0);

    label = gtk_label_new ( _("When importing the file from the previous version, "
                              "Grisbi has found inconsistencies\n"
                              "and could not restore all the reconciliatoins of one or "
                              "more accounts.\n\n"
                              "Although this has no impact on the integrity of data, "
                              "wrong values can be found.\n\n"
                              "Check in the \"Preferences\" for more information.") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0);

    gtk_widget_show_all (page);
    return page;
}


/**
 * If error creating reconcilaitions, display the third page 
 * otherwise exit normally
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_first_enter_page_2 ( GtkWidget *assistant,
                        gint new_page )
{
    if ( result_reconcile == TRUE )
        gsb_assistant_change_button_next ( assistant, GTK_STOCK_GO_FORWARD, 
				       GTK_RESPONSE_APPLY );

    return FALSE;
}

/**
 * keep the forward button of the last page instead of
 * a close button
 *
 * \param assistant
 * \param new_page
 *
 * \return FALSE
 * */
static gboolean gsb_assistant_first_enter_page_3 ( GtkWidget *assistant,
                        gint new_page )
{
    gsb_assistant_change_button_next ( assistant, GTK_STOCK_GO_FORWARD, 
				       GTK_RESPONSE_APPLY );

    return FALSE;
}
