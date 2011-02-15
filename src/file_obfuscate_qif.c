/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
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

/**
 * \file file_obfuscate_qif.c
 * anonymyze a qif file to send with bugreports
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "file_obfuscate_qif.h"
#include "dialog.h"
#include "utils_file_selection.h"
#include "gsb_assistant.h"
#include "gsb_file.h"
#include "gsb_status.h"
/*END_INCLUDE*/


/*START_STATIC*/
static void file_obfuscate_qif_next_line ( gchar *file_content,
				    gint *position,
				    gint *position_in_line );
static void file_obfuscate_qif_start ( gchar *filename );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * called by menubar to obfuscate_qif the file
 *
 * \param
 *
 * \return TRUE
 * */
gboolean file_obfuscate_qif_run ( void )
{
    GtkWidget *assistant;
    gint result;

    gsb_status_message ( _("Obfuscating qif file...") );

    assistant = gsb_assistant_new ( _("Grisbi QIF obfuscation"),
				    _("This assistant produces anonymized copies of qif files, with "
				      "all personal data replaced with harmless random data, in order to "
				      "attach an anonimized copy of your Grisbi file with any bug report "
				      "you submit."
				      "\n\n"
				      "That said, please check that bugs you submit are still valid with "
				      "anonymized version of that anonymized QIF file.\n"
				      "\n"
				      "In next page, you will have to choose a QIF file wich will be renamed"
				      "with -obfuscate at the end."),
				    "bug.png",
				    NULL );
    gsb_assistant_change_button_next (assistant, GTK_STOCK_GO_FORWARD, GTK_RESPONSE_APPLY);
    result = gsb_assistant_run ( assistant );

    if (result == GTK_RESPONSE_APPLY)
    {
	GtkWidget *file_selection;
	GtkFileFilter * filter;
	gchar *qif_name;

    file_selection = gtk_file_chooser_dialog_new ( _("Open a QIF file"),
					   GTK_WINDOW ( assistant ),
					   GTK_FILE_CHOOSER_ACTION_OPEN,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					   NULL);

    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( file_selection ), gsb_file_get_last_path () );
    gtk_window_set_position ( GTK_WINDOW ( file_selection ), GTK_WIN_POS_CENTER_ON_PARENT );

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name ( filter, _("QIF files (*.qif)") );
	gtk_file_filter_add_pattern ( filter, "*.qif" );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( file_selection ), filter );
	gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( file_selection ), filter );

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name ( filter, _("All files") );
	gtk_file_filter_add_pattern ( filter, "*" );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( file_selection ), filter );

	switch ( gtk_dialog_run ( GTK_DIALOG (file_selection)))
	{
	    case GTK_RESPONSE_OK:
		    qif_name = file_selection_get_filename ( GTK_FILE_CHOOSER ( file_selection ) ) ;
		    gtk_widget_destroy ( file_selection );
		    if (qif_name || strlen (qif_name))
			file_obfuscate_qif_start (qif_name);
		    break;
	    default:
		    gtk_widget_destroy ( file_selection );
		    break;
	}
    }

    gtk_widget_destroy ( assistant );

    gsb_status_message ( _("Done.") );

    return FALSE;
}


/**
 * get a qif filename, obfuscate it and save it
 *
 * \param filename
 *
 * \return
 * */
static void file_obfuscate_qif_start ( gchar *filename )
{
    gchar *file_content;
    gchar *new_filename;
    gint position = 0;
    gint position_in_line = 0;
    gchar *tmpstr;

    if (!g_file_get_contents (filename,
			      &file_content, NULL, NULL ))
    {
	dialogue_error (_("Cannot open the QIF file."));
	return;
    }

    if (file_content[0] != '!')
    {
	dialogue_error (_("This file doesn't seem to be a QIF file,\noperation canceled."));
	g_free (file_content);
	return;
    }

    /* the qif file is opened, we can change the content */
    /* it's not too complex :
     * we need to keep the first letter, if it's ^, go to the next line
     * if the second letter is [, we need to keep the content of the line, it's the name of an account
     * 		and we need it for transfer
     * same if the first letter is C (it's the mark for transaction) or ! or D (don't touch the date...)
     * after that, we change letter by other letter, and digits by other digits, but keep all the rest (. , ' ...)
     * */

    do
    {
	/* first check when we have to pass the line */
	switch (position_in_line)
	{
	    case 0:
		switch (file_content[position])
		{
		    case '!':
		    case '^':
		    case 'C':
		    case 'D':
			file_obfuscate_qif_next_line (file_content,
						      &position,
						      &position_in_line);
			continue;
			break;
		    default:
			/* we are on the first col but don't pass this line so go to the second col
			 * and don't break here to check just after */
			position++;
			position_in_line++;
		}

	    case 1:
		/* we are on the 2nd col, we pass this line if it's a transfer */
		if (file_content[position] == '[')
		{
		    file_obfuscate_qif_next_line (file_content,
						  &position,
						  &position_in_line);
		    continue;
		}
		break;
	}

	/* now we are on the 2nd col or more
	 * if we are at the end of line, go to next line
	 * in windows format, we can have \r before the \n */
	if (file_content[position] == '\r')
	{
	    position++;
	    position_in_line++;
	}
	if (file_content[position] == '\n')
	{
	    position++;
	    position_in_line = 0;
	    continue;
	}

	/* ok, so now we need to change the content of the position, only for
	 * digits or characters */
	if (g_ascii_isdigit (file_content[position]))
	    file_content[position] = g_random_int_range ('0', '9');
	else
	    if (g_ascii_isalpha (file_content[position]))
		file_content[position] = g_random_int_range ('a', 'z');

	position++;
	position_in_line++;
    }
    while (file_content[position] != 0);

    filename[strlen(filename) - 4] = 0;
    new_filename = g_strconcat (filename, "-anonymized.qif", NULL);

    if (g_file_set_contents ( new_filename,
			      file_content,
			      -1, NULL ))
    {
	tmpstr = g_strdup_printf (_("File '%s' successfully saved"),
				  new_filename );
	dialogue (tmpstr);
	g_free (tmpstr);
    }
    else
    {
	tmpstr = g_strdup_printf (_("File '%s' could not be saved"),
				  new_filename );
	dialogue_error (tmpstr);
	g_free (tmpstr);
    }

    g_free (new_filename);
    g_free (file_content);
}


/**
 * go to the next line in file_content
 *
 * \param file_content
 * \param position
 * \param position_in_line
 *
 * \return
 * */
void file_obfuscate_qif_next_line ( gchar *file_content,
				    gint *position,
				    gint *position_in_line )
{
    while (file_content[*position] != '\n')
	*position = *position + 1;

    *position = *position + 1;
    *position_in_line = 0;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
