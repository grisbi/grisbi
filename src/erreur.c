/* ************************************************************************** */
/*                                                                            */
/*                                  erreur.c                                  */
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


/*START_INCLUDE*/
#include "erreur.h"
#include "dialog.h"
#include "gsb_file_save.h"
#include "gsb_file_util.h"
#include "gsb_file_config.h"
#include "gsb_plugins.h"
#include "gsb_real.h"
#include "gsb_status.h"
#include "import.h"
#include "main.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_files.h"
#include "utils_str.h"
/*END_INCLUDE*/

#ifdef HAVE_BACKTRACE
#include "execinfo.h"
#endif


/*START_STATIC*/
static gchar *get_debug_time ( void );
static GtkWidget * print_backtrace ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern gchar *nom_fichier_comptes;
extern GtkUIManager * ui_manager;
/*END_EXTERN*/

static gint debugging_grisbi;

/* path and name of the file containing the log when debug mode is active
 * this values should not be freed when begin a new file to continue the log */
gchar *debug_filename = NULL;
FILE *debug_file = NULL;

/*************************************************************************************************************/
void traitement_sigsegv ( gint signal_nb )
{
    gchar *gsb_file_default_dir;
    gchar *errmsg = g_strdup ( "" );
	gchar *old_errmsg;
    gchar *tmpstr;
    GtkWidget * dialog;
#ifdef HAVE_BACKTRACE
    GtkWidget * expander;
#endif

    /*   il y a 3 possibilités : */
    /*     soit on était en train de charger un fichier, c'est que celui-ci est corrompu */
    /* soit on était en train de sauver un fichier, et là on peut rien faire */
    /* sinon on essaie de sauver le fichier sous le nom entouré de # */

    if ( etat.en_train_de_charger || 
	 etat.en_train_de_sauvegarder || 
	 etat.modification_fichier == 0 )
    {

	if ( etat.en_train_de_charger )
	{
	    old_errmsg = errmsg;
	    errmsg = g_strconcat ( errmsg, _("File is corrupted."), NULL );
	    g_free ( old_errmsg );
	}

	if ( etat.en_train_de_sauvegarder )
	{
	    old_errmsg = errmsg;
	    errmsg = g_strconcat ( errmsg, _("Error occured saving file."), NULL );
	    g_free ( old_errmsg );
	}
    }
    else 
    {
	/* c'est un bug pendant le fonctionnement de Grisbi s'il n'y a
	   pas de nom de fichier, on le crée, sinon on rajoute #
	   autour */

        gsb_file_default_dir = (gchar *) my_get_gsb_file_default_dir();

	if ( nom_fichier_comptes )
	    /* set # around the filename */
	    nom_fichier_comptes = g_path_get_basename (nom_fichier_comptes);
	else
	    /* no name for the file, create it */
	    nom_fichier_comptes = g_strconcat ( gsb_file_default_dir,
						"/#grisbi_crash_no_name#",
						NULL );

	gsb_status_message ( _("Save file") );

	gsb_file_save_save_file ( nom_fichier_comptes,
				  conf.compress_file,
				  FALSE );

	gsb_status_clear();

    old_errmsg = errmsg;
	errmsg = g_strconcat ( errmsg, 
			       g_strdup_printf ( _("Grisbi made a backup file at '%s'."),
						 nom_fichier_comptes ),
			       NULL );
	g_free ( old_errmsg );
    }

    old_errmsg = errmsg;
    errmsg = g_strconcat ( errmsg, 
			   "\n\n",
			   _("Please report this problem to <tt>http://www.grisbi.org/bugtracking/</tt>.  "),
			   NULL );
     g_free ( old_errmsg );

#ifdef HAVE_BACKTRACE
    old_errmsg = errmsg;
    errmsg = g_strconcat ( errmsg, _("Copy and paste the following backtrace with your bug "
                        "report."),
			   NULL );
     g_free ( old_errmsg );
#endif

    dialog = dialogue_special_no_run ( GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
				       make_hint ( _("Grisbi terminated due to a segmentation fault."),
						   errmsg ));
    g_free ( errmsg );

#ifdef HAVE_BACKTRACE
    tmpstr = g_strconcat ( "<b>", _("Backtrace"), "</b>", NULL );
    expander = gtk_expander_new ( tmpstr );
    g_free ( tmpstr );

    gtk_expander_set_use_markup ( GTK_EXPANDER ( expander ), TRUE );
    gtk_container_add ( GTK_CONTAINER ( expander ), print_backtrace() );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog)->vbox ), expander, FALSE, FALSE, 6 );

    gtk_widget_show_all ( dialog );
#endif
    gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    /*     on évite le message du fichier ouvert à la prochaine ouverture */

    gsb_file_util_modify_lock ( FALSE );

    exit(1);
}

/*************************************************************************************************************/
/* initialise show_grisbi_debug a TRUE si on souhaite le debug																							 */
/*************************************************************************************************************/
void initialize_debugging ( void )
{
    /* un int pour stocker le level de debug et une chaine qui contient sa version texte */
    gint debug_variable=0;
    gchar *debug_level="";
	gchar* tmpstr1;
	gchar* tmpstr2;

    if (getenv ("DEBUG_GRISBI"))
    {
	/* on choppe la variable d'environnement */
	debug_variable=utils_str_atoi (getenv ("DEBUG_GRISBI"));

	/* on verifie que la variable est cohérente */
	if (debug_variable > 0 && debug_variable <= MAX_DEBUG_LEVEL) 
	{
	    /* on renseigne le texte du level de debug */
	    debugging_grisbi = debug_variable;
	    switch(debug_variable)
	    {
		case DEBUG_LEVEL_ALERT: { debug_level="Alert"; break; }
		case DEBUG_LEVEL_IMPORTANT: { debug_level="Important"; break; }
		case DEBUG_LEVEL_NOTICE: { debug_level="Notice"; break; }
		case DEBUG_LEVEL_INFO: { debug_level="Info"; break; }
		case DEBUG_LEVEL_DEBUG: { debug_level="Debug"; break; }
	    }

	    /* on affiche un message de debug pour indiquer que le debug est actif */
	    tmpstr1 = g_strdup_printf(_("GRISBI %s Debug"),VERSION);
	    tmpstr2 = g_strdup_printf(_("Debug enabled, level is '%s'"),debug_level);
	    debug_message_string ( tmpstr1 , 
				   __FILE__, __LINE__, __PRETTY_FUNCTION__,
				   tmpstr2,
				   DEBUG_LEVEL_INFO, TRUE);
	    g_free ( tmpstr1 );
	    g_free ( tmpstr2 );
	}
	else
	{
	    /* on affiche un message de debug pour indiquer que le debug est actif */
	    gchar* tmpstr = g_strdup_printf(_("GRISBI %s Debug"),VERSION);
	    debug_message_string (tmpstr , 
				  __FILE__, __LINE__, __PRETTY_FUNCTION__,
				  _("Wrong debug level, please check DEBUG_GRISBI environnement variable"),
				  DEBUG_LEVEL_INFO, TRUE);
	    g_free ( tmpstr );
	}
    }
}

/**
 * return a string with the current time
 * use to debug lines
 *
 * \param
 *
 * \return a string with the date and time, don't free it
 * */
gchar *get_debug_time ( void )
{
    /* le temps courant et une chaine dans laquelle on stocke le temps courant */
    time_t debug_time;
    gchar *str_debug_time;

    /* on choppe le temps courant et on va le mettre dans une chaine */
    time(&debug_time);
    str_debug_time=ctime(&debug_time);

    /* on fait sauter le retour a la ligne */
    str_debug_time[strlen(str_debug_time) - 1] = '\0';

    /* on renvoit le temps */
    return str_debug_time;
}	



/**
 * show a debug message in the terminal
 * only if debug mode is on
 * not called directly so need to force the extern 
 * the param to chow is a string
 *
 * \param
 * \param
 * \param
 * \param
 * \param message a string to display with the message, NULL if no message
 * \param
 * \param
 *
 * \return
 * */
G_MODULE_EXPORT extern void debug_message_string ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        const gchar *message,
                        gint level,
                        gboolean force_debug_display )
{
    /* il faut bien entendu que le mode debug soit actif ou que l'on force l'affichage */
    if ( ( debugging_grisbi && level <= debugging_grisbi) || force_debug_display || etat.debug_mode )
    {
        gchar* tmp_str;

        /* on affiche dans la console le message */
        if (message)
            tmp_str = g_strdup_printf(_("%s, %2f : %s - %s:%d:%s - %s\n"),
                        get_debug_time (), (double )clock()/ CLOCKS_PER_SEC, prefixe,
                        file, line, function, message);
        else
            tmp_str = g_strdup_printf(_("%s, %2f : %s - %s:%d:%s\n"),
                        get_debug_time (), (double )clock()/ CLOCKS_PER_SEC, prefixe,
                        file, line, function);

        if ( etat.debug_mode )
        {
            fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
            fflush ( debug_file );
        }

        g_print( "%s", tmp_str );
        g_free ( tmp_str );
    }
}

/**
 * show a debug message in the terminal
 * only if debug mode is on
 * not called directly so need to force the extern 
 * the param to chow is a number
 *
 * \param
 * \param
 * \param
 * \param
 * \param message a number to display with the message
 * \param
 * \param
 *
 * \return
 * */
extern void debug_message_int ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        gint message,
                        gint level,
                        gboolean force_debug_display )
{
    /* il faut bien entendu que le mode debug soit actif ou que l'on force l'affichage */
    if ( ( debugging_grisbi && level <= debugging_grisbi) || force_debug_display || etat.debug_mode )
    {
        gchar* tmp_str;

        /* on affiche dans la console le message */
        tmp_str = g_strdup_printf(_("%s, %2f : %s - %s:%d:%s - %d\n"),
                        get_debug_time (), (double )clock()/ CLOCKS_PER_SEC, prefixe,
                        file, line, function, message);

        if (etat.debug_mode)
        {
            fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
            fflush ( debug_file );
        }

        g_print( "%s", tmp_str );
        g_free ( tmp_str );
    }
}


/**
 * show a debug message in the terminal
 * only if debug mode is on
 * not called directly so need to force the extern 
 * the param to chow is a number
 *
 * \param
 * \param
 * \param
 * \param
 * \param message a number to display with the message
 * \param
 * \param
 *
 * \return
 * */
extern void debug_message_real ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        gsb_real message,
                        gint level,
                        gboolean force_debug_display )
{
    /* il faut bien entendu que le mode debug soit actif ou que l'on force l'affichage */
    if ( ( debugging_grisbi && level <= debugging_grisbi) || force_debug_display || etat.debug_mode )
    {
        gchar* tmp_str;

        /* on affiche dans la console le message */
        tmp_str = g_strdup_printf ("%s, %2f : %s - %s:%d:%s - %"G_GINT64_MODIFIER"d E %d\n",
                        get_debug_time (), (double )clock()/ CLOCKS_PER_SEC, prefixe,
                        file, line, function, message.mantissa, message.exponent );

        if ( etat.debug_mode )
        {
            fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
            fflush ( debug_file );
        }

        g_print( "%s", tmp_str );
        g_free ( tmp_str );
    }
}




/**
 * Print the backtrace upon segfault.
 */
GtkWidget * print_backtrace ( void )
{
#ifdef HAVE_BACKTRACE
    void *backtrace_content[15];
    int backtrace_size;
    size_t i;
    gchar **backtrace_strings, *text = g_strdup("");
    GtkWidget * label;
		
    backtrace_size = backtrace (backtrace_content, 15);
    backtrace_strings = backtrace_symbols (backtrace_content, backtrace_size);
		
    g_print ("%s : %d elements in stack.\n", get_debug_time(), backtrace_size);
		
    for (i = 0; i < backtrace_size; i++) 
    {
	g_print ("\t%s\n", backtrace_strings[i]);
	gchar* old_text = text;
	text = g_strconcat ( text, g_strconcat ( "\t", backtrace_strings[i], "\n", NULL ), 
			     NULL );
	g_free ( old_text );
    }

    label = gtk_label_new ( text );
    g_free ( text );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    return label;
#else
    return NULL;
#endif
}

/**
 * called by menu : begin the debug mode
 * show a message to say where the log will be saved
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_debug_start_log ( void )
{
    gchar *tmp_str;

    devel_debug ( NULL );

    if ( nom_fichier_comptes )
    {
        gchar *base_filename = g_strdup ( nom_fichier_comptes );
        gchar *complete_filename;
        gchar *basename;

        base_filename [strlen ( base_filename ) - 4] = 0;
        complete_filename = g_strconcat ( base_filename, "-log.txt", NULL);
        basename = g_path_get_basename ( complete_filename );

        debug_filename = g_strconcat ( my_get_gsb_file_default_dir (), "/", basename, NULL);

        g_free ( basename);
        g_free ( complete_filename );
        g_free ( base_filename );
    }
    else
    {
        debug_filename = g_strconcat ( my_get_gsb_file_default_dir (), "/", "No_name-log.txt", NULL);
    }


    tmp_str = g_strdup_printf (_("The debug-mode is starting. Grisbi will write a log into %s. "
                        "Please send that file with the obfuscated file into the bug report."),
                        debug_filename );

    dialogue ( tmp_str );
    g_free (tmp_str);

    debug_file = g_fopen ( debug_filename, "w" );

    if ( debug_file )
    {
        GtkWidget *widget;
        gchar *tmp_str_2;

        widget = gtk_ui_manager_get_widget ( ui_manager, "/menubar/FileMenu/DebugMode" );
        etat.debug_mode = TRUE;

        /* unsensitive the menu, we cannot reverse the debug mode */
        if ( widget && GTK_IS_WIDGET ( widget ) )
            gtk_widget_set_sensitive ( widget, FALSE );

        /* début du mode débogage */
        tmp_str = g_strdup_printf(_("%s, %2f : Debug - %s:%d:%s\n\n"),
                        get_debug_time ( ),
                        (double ) clock ( )/ CLOCKS_PER_SEC,
                        __FILE__,
                        __LINE__,
                        __PRETTY_FUNCTION__ );
        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        /* write locales */
        tmp_str = gsb_main_get_print_locale_var ( );

        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        tmp_str = g_strdup_printf ( "gint64\n"
                        "\tG_GINT64_MODIFIER = \"%s\"\n"
                        "\t%"G_GINT64_MODIFIER"d\n\n",
                        G_GINT64_MODIFIER, G_MAXINT64 );

        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        tmp_str = gsb_main_get_print_dir_var ( );

        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        tmp_str = g_strdup ( "Formats importés\n" );
        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        tmp_str = gsb_import_formats_get_list_formats_to_string ( );
        tmp_str_2 = g_strconcat ( tmp_str, "\n", NULL );

        fwrite ( tmp_str_2, sizeof (gchar), strlen ( tmp_str_2 ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );
        g_free ( tmp_str_2 );
    }
    else
        dialogue_error (_("Grisbi failed to create the log file...") );

    return FALSE;
}


/**
 *
 *
 *
 */
extern void debug_print_log_string ( gchar *prefixe,
                        gchar *file,
                        gint line,
                        const char *function,
                        const gchar *msg )
{
    gchar *tmp_str;
    gchar *message;

    if ( debug_file == NULL )
        return;

    if ( msg && strlen ( msg ) )
        message = g_strdup ( msg );
    else
        message = g_strdup ( "(null)" );

    tmp_str = g_strdup_printf(_("%s, %2f : %s - %s:%d:%s - %s\n"),
                        get_debug_time ( ),
                        (double ) clock ( )/ CLOCKS_PER_SEC,
                        prefixe,
                        file,
                        line,
                        function,
                        message );

    fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
    fflush ( debug_file );

    g_free ( tmp_str );
    g_free ( message );
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
