/* ************************************************************************** */
/* Contient toutes les procédures relatives aux erreurs et à la fin du prog   */
/*                                                                            */
/*                                  erreur.c                                  */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "erreur.h"
#include "./dialog.h"
#include "./gsb_file_save.h"
#include "./gsb_file_util.h"
#include "./gsb_plugins.h"
#include "./gsb_status.h"
#include "./traitement_variables.h"
#include "./utils_files.h"
#include "./utils_str.h"
#include "./gsb_file_config.h"
#include "./include.h"
#include "./structures.h"
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
/*END_EXTERN*/



gint debugging_grisbi;






/*************************************************************************************************************/
void traitement_sigsegv ( gint signal_nb )
{
    gchar * gsb_file_default_dir, * errmsg = "";
    GtkWidget * dialog, *expander;

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
	    gchar* old_errmsg = errmsg;
	    errmsg = g_strconcat ( errmsg, _("File is corrupted."), NULL );
	    g_free ( old_errmsg );
	}

	if ( etat.en_train_de_sauvegarder )
	{
	    gchar* old_errmsg = errmsg;
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
	{
	    /* on récupère le nome du fichier sans le chemin */

	    gchar **parametres;
	    gint i=0;
	    parametres = g_strsplit ( nom_fichier_comptes, C_DIRECTORY_SEPARATOR, 0);

	    while ( parametres[i] )
		i++;

	    nom_fichier_comptes = g_strconcat ( gsb_file_default_dir, "/#", parametres [i-1], 
						"#", NULL );
	    g_strfreev ( parametres );
	}
	else
	    nom_fichier_comptes = g_strconcat ( gsb_file_default_dir,
						"/#grisbi_plantage_sans_nom#",
						NULL );

	gsb_status_message ( _("Save file") );

	gsb_file_save_save_file ( nom_fichier_comptes,
				  etat.compress_file,
				  FALSE );

	gsb_status_clear();

        gchar* old_errmsg = errmsg;
	errmsg = g_strconcat ( errmsg, 
			       g_strdup_printf ( _("Grisbi made a backup file at '%s'."),
						 nom_fichier_comptes ),
			       NULL );
	g_free ( old_errmsg );
    }

    gchar* old_errmsg = errmsg;
    errmsg = g_strconcat ( errmsg, 
			   "\n\n",
			   _("Please report this problem to <tt>http://www.grisbi.org/bugtracking/</tt>.  "),
			   NULL );
     g_free ( old_errmsg );

#ifdef HAVE_BACKTRACE
    old_errmsg = errmsg;
    errmsg = g_strconcat ( errmsg, _("Copy and paste the following backtrace with your bug report."),
			   NULL );
     g_free ( old_errmsg );
#endif

    dialog = dialogue_special_no_run ( GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
				       make_hint ( _("Grisbi terminated due to a segmentation fault."),
						   errmsg ));
    g_free ( errmsg );

#ifdef HAVE_BACKTRACE
    gchar* tmpstr = g_strconcat ( "<b>", _("Backtrace"), "</b>", NULL );
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

    gsb_plugins_release ( );

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
	    gchar* tmpstr1 = g_strdup_printf(_("GRISBI %s Debug"),VERSION);
	    gchar* tmpstr2 = g_strdup_printf(_("Debug enabled, level is '%s'"),debug_level);
	    debug_message( tmpstr1 , 
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
	    debug_message(tmpstr , 
			  __FILE__, __LINE__, __PRETTY_FUNCTION__,
			  _("Wrong debug level, please check DEBUG_GRISBI environnement variable"),
			  DEBUG_LEVEL_INFO, TRUE);
	    g_free ( tmpstr );
	}
    }
}

/*************************************************************************************************************/
/* renvoit une chaine avec le temps de debug																																 */
/*************************************************************************************************************/
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
 * affiche de message de debug dans la console (uniquement si
 * show_grisbi_debug est a TRUE)
 * not called directly so need to force the extern */
extern void debug_message ( gchar *prefixe, gchar * file, gint line, const char * function, 
			    gchar *message, gint level, gboolean force_debug_display)
{
    /* il faut bien entendu que le mode debug soit actif ou que l'on force l'affichage */
    if ( ( debugging_grisbi && level <= debugging_grisbi) || force_debug_display) 
    {
	/* on affiche dans la console le message */
	gchar* tmpstr = g_strdup_printf(_("%s : %s - %s:%d:%s - %s\n"),
			       get_debug_time(), prefixe,
			       file, line, function, message);
	g_print( tmpstr );
	g_free ( tmpstr );
    }
}



/**
 * Print the backtrace upon segfault.
 */
GtkWidget * print_backtrace ( void )
{
#ifdef HAVE_BACKTRACE
    void *backtrace_content[15];
    size_t backtrace_size, i;
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
#endif
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
