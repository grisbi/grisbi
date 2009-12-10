/* ************************************************************************** */
/*                                  utils_files.c                             */
/*                                                                            */
/*     Copyright (C)    2000-2007 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2007 Benjamin Drieu (bdrieu@april.org)                       */
/*          2003-2004 Alain Portal (aportal@univ-montp2.fr)                   */
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

#include "include.h"


/*START_INCLUDE*/
#include "utils_files.h"
#include "./dialog.h"
#include "./utils_file_selection.h"
#include "./utils_dates.h"
#include "./gsb_file.h"
#include "./utils_str.h"
#include "./gsb_file_config.h"
#include "./utils_file_selection.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void browse_file ( GtkButton *button, gpointer data );
static void utils_files_file_chooser_cancel ( GtkWidget *bouton, GtkWidget *chooser);
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/


/**
 * Handler triggered by clicking on the button of a "print to file"
 * combo.  Pop ups a file selector.
 *
 * \param button GtkButton widget that triggered this handler.
 * \param data A pointer to a GtkEntry that will be filled with the
 *             result of the file selector.
 */
void browse_file ( GtkButton *button, gpointer data )
{
    GtkWidget * file_selector;

    file_selector = file_selection_new (_("Print to file"),FILE_SELECTION_IS_SAVE_DIALOG);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_selector),
					 gsb_file_get_last_path ());
    gtk_window_set_transient_for ( GTK_WINDOW ( file_selector ),
				   GTK_WINDOW ( window ));
    gtk_window_set_modal ( GTK_WINDOW ( file_selector ), TRUE );

    switch ( gtk_dialog_run ( GTK_DIALOG (file_selector)))
    {
	case GTK_RESPONSE_OK:
	    gtk_entry_set_text ( GTK_ENTRY(data),
				 file_selection_get_filename (GTK_FILE_CHOOSER (file_selector)));
	    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (file_selector), TRUE));

	default:
	    gtk_widget_destroy ( file_selector );
	    break;
    }
}

/**
 * return the absolute path of where the configuration file should be located
 * on Un*x based system return $HOME
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_grisbirc_dir(void)
{
#ifndef _WIN32
    return (gchar *) g_get_home_dir ();
#else
    return win32_get_grisbirc_folder_path();
#endif
}


/**
 * return the absolute path of where the configuration file should be located
 * On UNIX platforms this is determined using the mechanisms described 
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_XDG_grisbirc_dir(void)
{
#ifndef _WIN32
    return (gchar *) C_PATH_CONFIG;
#else
    return win32_get_grisbirc_folder_path();
#endif
}


/**
 * return the absolute path of where the data files should be located
 * On UNIX platforms this is determined using the mechanisms described 
 * in the  XDG Base Directory Specification
 * on Windows based systems return APPDATA\Grisbi
 * 
 * \return the absolute path of the home directory
 */
gchar* my_get_XDG_grisbi_data_dir ( void )
{
#ifndef _WIN32
    return (gchar *) C_PATH_DATA_FILES;
#else
    return g_get_home_dir ();
#endif
}

/**
 * return the absolute path of the default accounts files location
 * on Un*x based system return $HOME
 * on Windows based systems return "My Documents"
 * 
 * \return the absolute path of the configuration file directory
 */
gchar* my_get_gsb_file_default_dir ( void )
{
#ifndef _WIN32
    return (gchar *) g_get_home_dir();
#else
    return win32_get_my_documents_folder_path();
#endif
}



/* get the line af the file,
 * convert it in UTF8 and fill string with that line
 *
 * \param fichier
 * \param string
 * \param coding_system	the orig coding system of the string
 *
 * \return EOF, 1 if ok, 0 if problem
 * */
gint get_utf8_line_from_file ( FILE *fichier,
                        gchar **string,
                        const gchar *coding_system )
{
    gchar c = 0;
    gint i = 0;
    gint j = 0;
    gchar *pointeur_char = NULL;
    gchar *tmp_string;

    if ( !fichier )
	return 0;
	    
    /* allocate 30 characters, and increase it 30 by 30 */
    pointeur_char = (gchar*)g_realloc(pointeur_char,30*sizeof(gchar));

    if ( !pointeur_char )
    {
	/* ouch, not enough memory */
	dialogue_error ( _("Memory allocation error" ));
	return 0;
    }

    /* get the string untill \n or \r (windows format) */
    c =(gchar)fgetc(fichier);
    while ( ( c != '\n' ) && (c != '\r') && !feof (fichier))
    {
	pointeur_char[j++] = c;

	if ( ++i == 29 )
	{
	    pointeur_char = (gchar*)g_realloc(pointeur_char, j + 1 + 30*sizeof(gchar));

	    if ( !pointeur_char )
	    {
		/* ouch, not enough memory */
		dialogue_error ( _("Memory allocation error" ));
		return 0;
	    }
	    i = 0;
	}
	c =(gchar)fgetc(fichier);
    }
    pointeur_char[j] = 0;

    /* if we finished on \r, jump the \n after it */
    if ( c == '\r' )
    {
	c =(gchar)fgetc(fichier);
	if ( c != '\n' )
	{
	    ungetc ( c, fichier );
	}
    }

    tmp_string = g_convert ( pointeur_char, -1, "UTF-8", 
			     coding_system, NULL, NULL,
			     NULL );
    if (!tmp_string)
    {
        devel_debug ("convert to utf8 failed, will use latin2utf8");
        tmp_string = latin2utf8 (pointeur_char);
        if ( tmp_string == NULL )
        {
            dialogue_special ( GTK_MESSAGE_ERROR, make_hint (
                            _("Convert to utf8 failed."),
                            _("If the result is not correct, try again by selecting the "
                            "correct character set in the window for selecting files.") ) );
            return 0;
        }
    }
    *string = tmp_string;
    g_free (pointeur_char);

    if ( feof(fichier))
        return EOF;
    else
        return 1;
}




/**
 * Make a GtkEntry that will contain a file name, a GtkButton that
 * will pop up a file selector, pack them in a GtkHbox and return it.
 *
 * \return A newly created GtkHbox.
 */
GtkWidget * my_file_chooser ()
{
    GtkWidget * hbox, *entry, *button;

    hbox = gtk_hbox_new ( FALSE, 6 );

    entry = gtk_entry_new ( );
    gtk_box_pack_start ( GTK_BOX(hbox), entry, TRUE, TRUE, 0 );
    g_object_set_data ( G_OBJECT(hbox), "entry", entry );

    button = gtk_button_new_with_label ( _("Browse") );
    gtk_box_pack_start ( GTK_BOX(hbox), button, FALSE, FALSE, 0 );

    g_signal_connect ( G_OBJECT(button), "clicked",
		       (GCallback) browse_file, entry );

    return hbox;
}

/**
 * \brief utf8 version of fopen (see fopen for more detail about mode)
 * 
 * convert utf8 file path into the locale OS charset before calling fopen
 *
 * \param utf8filename file to open path coded using utf8 charset
 * \param mode fopen mode argument
 *
 * \return file descriptor returned by fopen
 */
G_MODULE_EXPORT FILE* utf8_fopen(const gchar* utf8filename,gchar* mode)
{
#ifdef _MSC_VER
    return fopen(g_locale_from_utf8(utf8filename, -1, NULL, NULL, NULL),mode);
#else
	return fopen(g_filename_from_utf8(utf8filename, -1, NULL, NULL, NULL),mode);
#endif
}


/**
 * \brief utf8 version of remove (see remove for more detail about mode)
 * 
 * convert utf8 file path into the locale OS charset before calling remove
 *
 * \param utf8filename file to remove path coded using utf8 charset
 *
 * \return remove returned value
 */
gint utf8_remove(const gchar* utf8filename)
{
#ifdef _MSC_VER
    return remove(g_locale_from_utf8(utf8filename, -1, NULL, NULL, NULL));
#else
    return remove(g_filename_from_utf8(utf8filename,-1,NULL,NULL,NULL));
#endif
}

/** 
 * Sanitize a safe filename.  All chars that are not normally allowed
 * are replaced by underscores.
 *
 * \param filename Filename to sanitize.
 */
gchar * safe_file_name ( gchar* filename )
{
    return g_strdelimit ( my_strdup(filename), G_DIR_SEPARATOR_S, '_' );
}


/** 
 * \brief pallie à un bug du gtk_file_chooser_button_new
 *
 * \param widget parent et titre de la femnêtre
 *
 * \return chooser
 */
GtkWidget *utils_files_create_file_chooser ( GtkWidget *parent, gchar * titre )
{
    GtkWidget *chooser;
    GtkWidget *bouton_cancel, *bouton_OK;
    
    chooser = gtk_file_chooser_dialog_new ( titre,
                        GTK_WINDOW (parent),
                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                        NULL, NULL, NULL );

    bouton_cancel = gtk_dialog_add_button (GTK_DIALOG ( chooser ),
                                GTK_STOCK_CANCEL,
                                GTK_RESPONSE_CANCEL);
    g_signal_connect ( G_OBJECT (bouton_cancel),
		       "clicked",
		       G_CALLBACK (utils_files_file_chooser_cancel),
		       chooser );

    bouton_OK = gtk_dialog_add_button (GTK_DIALOG ( chooser ),
                                GTK_STOCK_OPEN,
                                GTK_RESPONSE_ACCEPT);

	gtk_window_set_position ( GTK_WINDOW (chooser), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_transient_for (GTK_WINDOW (chooser), GTK_WINDOW (parent));
    gtk_widget_set_size_request ( chooser, 600, 750 );

    return  chooser;
}


/** 
 * \brief permet de pallier au bug de l'annulation du 
 * gtk_file_chooser_button_new
 *
 * \param bouton appellant et chooser
 */
void utils_files_file_chooser_cancel ( GtkWidget *bouton, GtkWidget *chooser)
{
    gchar *path;

    path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
    devel_debug ( path);
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (chooser), path );
    if (path && strlen (path) > 0)
        g_free (path);
}


/** 
 * \brief create the config and the data directories
 * using the mechanisms described in the XDG Base Directory Specification
 *
 * \param 
 *
 * \return TRUE if ok
 */
gboolean utils_files_create_XDG_dir (void)
{
#ifdef _MSC_VER
    int mode = 0;
#else
    int mode = S_IRUSR | S_IWUSR | S_IXUSR;
#endif//_MSC_VER
    if ( g_mkdir_with_parents ( C_PATH_CONFIG, mode ) == 0 &&
         g_mkdir_with_parents ( C_PATH_DATA_FILES, mode ) == 0 )
        return TRUE;
    else
        return FALSE;
}


/* comment by pbiava 24/01/2009 */
/**
 * create a full path backup name from the filename
 * using the backup repertory and add the date and .bak
 *
 * \param filename
 *
 * \return a newly allocated string
 * */
/*gchar *utils_files_create_backup_name ( const gchar *filename )
{
    gchar *string;
    gchar *tmp_name;
    GDate *today;
    gchar **split;
    gchar *inserted_string;

    !* get the filename *!
    tmp_name = g_path_get_basename (filename);

    !* create the string to insert into the backup name *!
    today = gdate_today ();
    inserted_string = g_strdup_printf ( "-%d_%d_%d-backup",
					g_date_year (today),
					g_date_month (today),
					g_date_day (today));
    g_date_free (today);

    !* insert the date and backup before .gsb if it exists *!
    split = g_strsplit ( tmp_name,
			 ".",
			 0 );
    g_free (tmp_name);

    if (split[1])
    {
	!* have extension *!
	gchar *tmpstr, *tmp_end;

	tmp_end = g_strconcat ( inserted_string,
				".",
				split[g_strv_length (split) - 1],
				NULL );
	split[g_strv_length (split) - 1] = NULL;

	tmpstr = g_strjoinv ( ".",
			      split );
	tmp_name = g_strconcat ( tmpstr,
				 tmp_end,
				 NULL );
	g_free (tmpstr);
	g_free (tmp_end);
    }
    else
	tmp_name = g_strconcat ( split[0],
				 inserted_string,
				 NULL );

    g_strfreev (split);

    string = g_build_filename ( gsb_file_get_backup_path (),
				tmp_name,
				NULL );
    g_free (tmp_name);
    return string;
}*/


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
