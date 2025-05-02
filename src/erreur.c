/* ************************************************************************** */
/*                                                                            */
/*                                  erreur.c                                  */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          https://www.grisbi.org/                                            */
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
#include <stdlib.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "erreur.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_file_save.h"
#include "gsb_file_util.h"
#include "gsb_locale.h"
#include "gsb_real.h"
#include "import.h"
#include "structures.h"
#include "utils.h"
#include "utils_files.h"
#include "utils_str.h"
#include "menu.h"
/*END_INCLUDE*/

#ifdef HAVE_BACKTRACE
#include "execinfo.h"
#endif


/*START_STATIC*/
static gboolean debug_mode = FALSE;
static gint debugging_grisbi;

/* path and name of the file containing the log when debug mode is active
 * this values should not be freed when begin a new file to continue the log */
static FILE *debug_file = NULL;

/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

 /**
 * return a string with the current time
 * use to debug lines
 *
 * \param
 *
 * \return a string with the date and time, don't free it
 * */
static gchar *debug_get_debug_time ( void )
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

#ifdef HAVE_BACKTRACE
/**
 * Print the backtrace upon segfault
 *
 * \param
 *
 * \return
 **/
static GtkWidget *debug_print_backtrace ( void )
{
    void *backtrace_content[50];
    int backtrace_size;
    gint i;
    gchar **backtrace_strings, *text = g_strdup("");
    GtkWidget * label;

    backtrace_size = backtrace (backtrace_content, 50);
    backtrace_strings = backtrace_symbols (backtrace_content, backtrace_size);

	text = g_strconcat(text, "Grisbi version: " VERSION "\n", NULL);
	g_print ("%s", text);

    g_print ("%s : %d elements in stack.\n", debug_get_debug_time(), backtrace_size);

    for (i = 0; i < backtrace_size; i++)
    {
	g_print ("\t%s\n", backtrace_strings[i]);
	gchar* old_text = text;
	gchar *tmp_str = g_strconcat ( "\t", backtrace_strings[i], "\n", NULL );
	text = g_strconcat ( text, tmp_str, NULL );
	g_free(tmp_str);
	g_free ( old_text );
    }

    label = gtk_label_new ( text );
    g_free ( text );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    return label;
}
#endif

/**
 * traitement des interruptions SIGINT, SIGTERM, SIGSEGV
 *
 * \param gint 	signal_nb
 *
 * \return
 **/
void debug_traitement_sigsegv ( gint signal_nb )
{
    GtkWidget *dialog;
    const gchar *gsb_file_default_dir;
	const gchar *filename;
    gchar *errmsg = g_strdup ( "" );
	gchar *old_errmsg;
    const gchar *signal_name;
    gchar *tmp_str;
	gboolean file_is_loading;
#ifdef HAVE_BACKTRACE
    GtkWidget *expander;
    GtkWidget *scrolled_window;
#endif
	GrisbiWinRun *w_run;

	switch ( signal_nb )
	{
		case 2:
			signal_name = "SIGINT";
			break;
		case 11:
			signal_name = "SIGSEGV";
			break;
		case 15:
			signal_name = "SIGTERM";
			break;
		default:
			signal_name = "NA";
			printf ("signal number: %d\n", signal_nb );
	}

	printf ("\nsignal name = %s\n", signal_name );

	/* on récupère le nom du fichier si possible */
	filename = grisbi_win_get_filename (NULL);
	if (filename)
		file_is_loading = TRUE;
	else
		file_is_loading = FALSE;

    /*   il y a 3 possibilités : */
    /*     soit on était en train de charger un fichier, c'est que celui-ci est corrompu */
    /* soit on était en train de sauver un fichier, et là on peut rien faire */
    /* sinon on essaie de sauver le fichier sous le nom entouré de # */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    if (file_is_loading || w_run->file_is_saving || !gsb_file_get_modified (FALSE))
    {
		if (file_is_loading )
		{
			old_errmsg = errmsg;
			errmsg = g_strconcat ( errmsg, _("File is corrupted."), NULL );
			g_free ( old_errmsg );
		}

		if ( w_run->file_is_saving )
		{
			old_errmsg = errmsg;
			errmsg = g_strconcat ( errmsg, _("Error occurred saving file."), NULL );
			g_free ( old_errmsg );
		}
    }
    else
    {
		gchar *nom_fichier_comptes;

		/* c'est un bug pendant le fonctionnement de Grisbi s'il n'y a
		pas de nom de fichier, on le crée, sinon on rajoute #
		autour */

        gsb_file_default_dir = gsb_dirs_get_default_dir ();

		if (filename)
			/* set # around the filename */
			nom_fichier_comptes = g_path_get_basename (filename);
		else
			/* no name for the file, create it */
			nom_fichier_comptes = g_strconcat ( gsb_file_default_dir,
											   "/#grisbi_crash_no_name#",
											   NULL );

		grisbi_win_status_bar_message ( _("Save file") );

		gsb_file_save_save_file (nom_fichier_comptes, FALSE, 0);

		grisbi_win_status_bar_clear();

		old_errmsg = errmsg;
		tmp_str = g_strdup_printf ( _("Grisbi made a backup file at '%s'."),
								   nom_fichier_comptes );
		errmsg = g_strconcat ( errmsg,
							  tmp_str,
							  NULL );
		g_free(tmp_str);
		g_free ( old_errmsg );
		g_free (nom_fichier_comptes);
    }

    old_errmsg = errmsg;
    errmsg = g_strconcat ( errmsg,
			   "\n\n",
			   _("Please report this problem to <a href=\"https://www.grisbi.org/bugsreports/\">https://www.grisbi.org/bugsreports/</a>.\n"),
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
				       errmsg,
				       _("Grisbi terminated due to a segmentation fault.") );
    g_free ( errmsg );

#ifdef HAVE_BACKTRACE
    tmp_str = g_strconcat ( "<b>", _("Backtrace"), "</b>", NULL );
    expander = gtk_expander_new ( tmp_str );
    g_free ( tmp_str );

    gtk_expander_set_use_markup ( GTK_EXPANDER ( expander ), TRUE );
    gtk_expander_set_expanded( GTK_EXPANDER ( expander ), TRUE );
    gtk_container_add ( GTK_CONTAINER ( expander ), debug_print_backtrace() );
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window), 400);
    gtk_container_add (GTK_CONTAINER (scrolled_window), expander);
    gtk_box_pack_start ( GTK_BOX ( dialog_get_content_area ( dialog ) ), scrolled_window, FALSE, FALSE, 6 );

    gtk_widget_show_all ( dialog );
#endif
    gtk_dialog_run ( GTK_DIALOG ( dialog ) );

    /* on évite le message du fichier ouvert à la prochaine ouverture */
    gsb_file_util_modify_lock (filename, FALSE);

    exit(1);
}

/**
 * initialise show_grisbi_debug a TRUE si on souhaite le debug
 * updated by command line
 *
 * \param
 *
 * \return
 **/
void debug_initialize_debugging ( gint level )
{
    /* un int pour stocker le level de debug et une chaine qui contient sa version texte */
    gint debug_variable=0;
    const gchar *debug_level="";
	gchar* tmp_str1;
	gchar* tmp_str2;

    /* on choppe la variable d'environnement en priorité */
	if (getenv ("DEBUG_GRISBI"))
        debug_variable = utils_str_atoi (getenv ("DEBUG_GRISBI"));
	else
		debug_variable = level;

	/* on verifie que la variable est cohérente sinon on se met en mode debug max */
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
		tmp_str1 = g_strdup_printf ( _("GRISBI %s Debug"), VERSION );
		tmp_str2 = g_strdup_printf ( _("Debug enabled, level is '%s'"),debug_level);
		g_print ("%s, %2f : %s - %s\n",
				 debug_get_debug_time (),
				 (clock() + 0.0)/ CLOCKS_PER_SEC,
				 tmp_str1,
				 tmp_str2);
		g_free ( tmp_str1 );
		g_free ( tmp_str2 );
	}
	else
	{
		gchar* tmp_str;

		debugging_grisbi = DEBUG_LEVEL_DEBUG;
		/* on affiche un message de debug pour indiquer que le debug est actif */
		tmp_str = g_strdup_printf (_("GRISBI %s Debug"),VERSION);
		g_print ("%s, %2f : %s - initialize_debugging - %s\n",
				 debug_get_debug_time (),
				 (clock() + 0.0)/ CLOCKS_PER_SEC,
				 tmp_str,
				 _("Default debug level: Debug"));
		g_free (tmp_str);
	}
}

/**
 * set commande line mode for debugging
 *
 * \param
 *
 * \return
 **/
void debug_set_cmd_line_debug_level ( gint debug_level )
{
    /* un int pour stocker le level de debug et une chaine qui contient sa version texte */
    const gchar *str_debug_level = "";
	gchar* tmp_str1;
	gchar* tmp_str2;

	/* on verifie que la variable est cohérente */
	if ( debug_level > MAX_DEBUG_LEVEL)
		debug_level = 5;

	/* on renseigne le texte du level de debug */
	debugging_grisbi = debug_level;

	switch ( debug_level )
	{
		case DEBUG_NO_DEBUG: { str_debug_level="No message"; break; }
		case DEBUG_LEVEL_ALERT: { str_debug_level="Alert"; break; }
		case DEBUG_LEVEL_IMPORTANT: { str_debug_level="Important"; break; }
		case DEBUG_LEVEL_NOTICE: { str_debug_level="Notice"; break; }
		case DEBUG_LEVEL_INFO: { str_debug_level="Info"; break; }
		case DEBUG_LEVEL_DEBUG: { str_debug_level="Debug"; break; }
	}

	/* on affiche un message de debug pour indiquer que le debug est actif */
	tmp_str1 = g_strdup_printf ( _("GRISBI %s Debug"), VERSION );
	tmp_str2 = g_strdup_printf ( _("Debug updated by cmd line, level is '%s'"), str_debug_level );

	printf ("%s, %2f : %s - %s\n", debug_get_debug_time (), (clock() + 0.0)/ CLOCKS_PER_SEC, tmp_str1, tmp_str2);
	g_free ( tmp_str1 );
	g_free ( tmp_str2 );
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
void debug_message_string ( const gchar *prefixe,
                        const gchar *file,
                        gint line,
                        const char *function,
                        const gchar *message,
                        gint level,
                        gboolean force_debug_display )
{

	/* il faut bien entendu que le mode debug soit actif ou que l'on force l'affichage */
    if (level <= debugging_grisbi || force_debug_display || debug_mode)
    {
        gchar* tmp_str;
		GLogLevelFlags log_level;

        if (NULL == message)
			message = "";

		tmp_str = g_strdup_printf("%2f : %s - %s:%d:%s - %s",
                        (clock() + 0.0)/ CLOCKS_PER_SEC, prefixe,
                        file, line, function, message);

        if ( debug_mode && debug_file)
        {
			gchar *tmp_str2;
			tmp_str2 = g_strdup_printf("%s %s\n", debug_get_debug_time (), tmp_str);
            fwrite ( tmp_str2, sizeof (gchar), strlen ( tmp_str2 ), debug_file );
			g_free(tmp_str2);
            fflush ( debug_file );
        }

		/* on affiche dans la console le message */
		switch (level)
		{
			case DEBUG_LEVEL_ALERT:
				log_level = G_LOG_LEVEL_CRITICAL;
				break;
			case DEBUG_LEVEL_IMPORTANT:
				log_level = G_LOG_LEVEL_WARNING;
				break;
			default:
				log_level = G_LOG_LEVEL_MESSAGE;
		}
        g_log (NULL, log_level, "%s", tmp_str);

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
void debug_message_int ( const gchar *prefixe,
                        const gchar *file,
                        gint line,
                        const char *function,
                        gint message,
                        gint level,
                        gboolean force_debug_display )
{
	gchar* tmp_str;
	tmp_str = g_strdup_printf("%d", message);
	debug_message_string(prefixe, file, line, function, tmp_str, level, force_debug_display);
	g_free ( tmp_str );
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
void debug_message_real ( const gchar *prefixe,
                        const gchar *file,
                        gint line,
                        const char *function,
                        GsbReal message,
                        gint level,
                        gboolean force_debug_display )
{
	gchar* tmp_str;
	tmp_str = g_strdup_printf("%"G_GINT64_MODIFIER"d E %d", message.mantissa, message.exponent);
	debug_message_string(prefixe, file, line, function, tmp_str, level, force_debug_display);
	g_free ( tmp_str );
}

void debug_message_ptr ( const gchar *prefixe,
                        const gchar *file,
                        gint line,
                        const char *function,
                        void * message,
                        gint level,
                        gboolean force_debug_display )
{
	gchar* tmp_str;
	tmp_str = g_strdup_printf("%p", message);
	debug_message_string(prefixe, file, line, function, tmp_str, level, force_debug_display);
	g_free ( tmp_str );
}

/**
* Enable logs and send them to a file
* Can be used when grisbi is NOT started from a terminal
* */
void debug_start_log_file(void)
{
#ifdef G_OS_WIN32
#define TMP_DIR "c:\\Windows\\Temp\\"
#else
#define TMP_DIR "/tmp/"
#endif
    debug_file = utils_files_utf8_fopen ( TMP_DIR "grisbi_log.txt", "w" );
	debug_mode = TRUE;
}

/**
 * called by menu : begin the debug mode
 * show a message to say where the log will be saved
 *
 * \param
 *
 * \return FALSE
 * */
gboolean debug_start_log ( void )
{
    gchar *tmp_str;
    gchar *debug_filename;
	const gchar *nom_fichier_comptes;

    devel_debug ( NULL );
	/* on récupère le nom du fichier si un fichier est chargé */
	nom_fichier_comptes = grisbi_win_get_filename (NULL);

    if ( nom_fichier_comptes )
    {
        gchar *base_filename = g_strdup ( nom_fichier_comptes );
        gchar *complete_filename;
        gchar *basename;

        base_filename [strlen ( base_filename ) - 4] = 0;
        complete_filename = g_strconcat ( base_filename, "-log.txt", NULL);
        basename = g_path_get_basename ( complete_filename );

        debug_filename = g_build_filename ( gsb_dirs_get_default_dir (), basename, NULL);

        g_free ( basename);
        g_free ( complete_filename );
        g_free ( base_filename );
    }
    else
    {
        debug_filename = g_build_filename ( gsb_dirs_get_default_dir (), "No_name-log.txt", NULL);
    }


    tmp_str = g_strdup_printf (_("The debug-mode is starting. Grisbi will write a log into %s. "
                        "Please send that file with the obfuscated file into the bug report."),
                        debug_filename );

    dialogue ( tmp_str );
    g_free (tmp_str);

    debug_file = utils_files_utf8_fopen ( debug_filename, "w" );

    g_free ( debug_filename );

    if ( debug_file )
    {
        gchar *tmp_str_2;

		debug_mode = TRUE;
        /* début du mode de débogage */
        tmp_str = g_strdup_printf("%s, %2f : Debug - %s:%d:%s\n\n",
                        debug_get_debug_time ( ),
                        (clock() + 0.0)/ CLOCKS_PER_SEC,
                        __FILE__,
                        __LINE__,
                        __PRETTY_FUNCTION__ );
        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        /* write locales */
        tmp_str = gsb_locale_get_print_locale_var ();

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

        tmp_str = gsb_dirs_get_print_dir_var ( );

        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        tmp_str = g_strdup ( "Formats importés\n" );
        fwrite ( tmp_str, sizeof (gchar), strlen ( tmp_str ), debug_file );
	    fflush ( debug_file );

        g_free ( tmp_str );

        tmp_str = gsb_ImportFormats_get_list_formats_to_string ( );
        tmp_str_2 = g_strconcat ( tmp_str, "\n", NULL );

        if (tmp_str_2)
        {
            fwrite ( tmp_str_2, sizeof (gchar), strlen ( tmp_str_2 ), debug_file );
            fflush ( debug_file );
        }

        g_free ( tmp_str );
        g_free ( tmp_str_2 );
    }
    else
        dialogue_error (_("Grisbi failed to create the log file...") );

    return FALSE;
}

/**
 *
 * */
void debug_finish_log ( void )
{
    if ( debug_file )
    {
        fclose (debug_file);
        debug_file = NULL;
    }
}


/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean debug_get_debug_mode (void)
{
	return debug_mode;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
