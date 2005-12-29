/* ************************************************************************** */
/* Contient toutes les procédures relatives aux erreurs et à la fin du prog   */
/*                                                                            */
/*                                  erreur.c                                  */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "utils.h"
#include "dialog.h"
#include "fichiers_gestion.h"
#include "gsb_file_config.h"
#include "gsb_file_save.h"
#include "gsb_file_util.h"
#include "gsb_status.h"
#include "traitement_variables.h"
#include "utils_files.h"
#include "utils_str.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

#ifdef HAVE_BACKTRACE
#include "execinfo.h"
#endif

#ifdef HAVE_G2BANKING
#include <g2banking/gbanking.h>
#include <aqbanking/imexporter.h>
#include <gwenhywfar/debug.h>
#endif


/*START_STATIC*/
static gchar *get_debug_time ( void );
static GtkWidget * print_backtrace ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern AB_BANKING *gbanking;
extern gchar *nom_fichier_comptes;
/*END_EXTERN*/

/* #ifdef HAVE_AQBANKING */
/* extern AB_BANKING *gbanking; */
/* #endif */


gint debugging_grisbi;



/*****************************************************************************************************************/
/* fonction appelée lors de la demande de fermeture de grisbi */
/*****************************************************************************************************************/


gboolean fermeture_grisbi ( void )
{

    /*     si le fichier n'est pas enregistré, on le propose */

    if ( etat.modification_fichier &&
	 assert_account_loaded ())
    {
	gint retour;

	retour = question_fermer_sans_enregistrer ();

	switch ( retour )
	{
	    /* 	    on veut enregistrer */
	    /* 	    si ça se passe pas bien, on arrête la fermeture */
	    case GTK_RESPONSE_OK:
		if ( !enregistrement_fichier(-1))
		    return TRUE;
		break;

		/* on ne veut pas enregistrer */
	    case GTK_RESPONSE_NO:

		break;

		/* 		on a annulé */
	    default:
		return TRUE;
	}
    }

    /*     à ce niveau, soit le fichier a été enregistré, soit on ne veut pas l'enregistrer */
    /* 	on ferme grisbi */

    gsb_file_config_save_config ();

    gsb_file_util_modify_lock ( FALSE );


#ifdef HAVE_G2BANKING
    if (gbanking) {
      int rv;

      rv=AB_Banking_Init(gbanking);
      if (rv) {
        printf (_("Could not initialize AqBanking, "
                  "online banking is not available (%d)\n"), rv);
        AB_Banking_free(gbanking);
        gbanking=0;
      }
    }
#endif

    gtk_main_quit();
    return TRUE;
}
/*************************************************************************************************************/





/*************************************************************************************************************/
void traitement_sigsegv ( gint signal_nb )
{
    gchar *gsb_file_default_dir, *errmsg;
    GtkWidget * dialog, *expander;

    errmsg = _("Grisbi triggered a segmentation fault and cannot continue its execution.\n\n");

    /*   il y a 3 possibilités : */
    /*     soit on était en train de charger un fichier, c'est que celui-ci est corrompu */
    /* soit on était en train de sauver un fichier, et là on peut rien faire */
    /* sinon on essaie de sauver le fichier sous le nom entouré de # */

    if ( etat.en_train_de_charger || 
	 etat.en_train_de_sauvegarder || 
	 !etat.modification_fichier )
    {

	if ( etat.en_train_de_charger )
	{
	    errmsg = g_strconcat ( errmsg, _("File is corrupted."), 
				   "\n\n", NULL );
	}

	if ( etat.en_train_de_sauvegarder )
	{
	    errmsg = g_strconcat ( errmsg, _("Error occured saving file."), 
				   "\n\n", NULL );
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
				  etat.compress_file );

	gsb_status_clear();

	errmsg = g_strconcat ( errmsg, 
			       g_strdup_printf ( _("Grisbi made a backup file at '%s'."),
						 nom_fichier_comptes ),
			       NULL );
	errmsg = g_strconcat ( errmsg, "\n\n", NULL );
    }

    errmsg = g_strconcat ( errmsg, 
			   _("Please report this problem to <tt>http://www.grisbi.org/bugtracking/</tt>.  "),
			   NULL );

#ifdef HAVE_BACKTRACE
    errmsg = g_strconcat ( errmsg, _("Copy and paste the following backtrace with your bug report."),
			   NULL );
#endif

    dialog = dialogue_special_no_run ( GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
				       make_hint ( _("Grisbi terminated due to a segmentation fault."),
						   errmsg ));
#ifdef HAVE_BACKTRACE
    expander = gtk_expander_new ( g_strconcat ( "<b>", _("Backtrace"), "</b>", NULL ) );
    gtk_expander_set_use_markup ( GTK_EXPANDER ( expander ), TRUE );
    gtk_container_add ( GTK_CONTAINER ( expander ), print_backtrace() );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog)->vbox ), expander, FALSE, FALSE, 6
 );
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
	    debug_message(g_strdup_printf(_("GRISBI %s Debug"),VERSION) , 
			  g_strdup_printf(_("Debug enable, level is '%s'"),debug_level),
			  DEBUG_LEVEL_INFO, TRUE);
	}
	else
	{
	    /* on affiche un message de debug pour indiquer que le debug est actif */
	    debug_message(g_strdup_printf(_("GRISBI %s Debug"),VERSION) , 
			  _("Wrong debug level, please check DEBUG_GRISBI environnement variable"),
			  DEBUG_LEVEL_INFO, TRUE);
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
 * send a devel default debug message
 * used usually to show in what function we are
 *
 * \param message
 *
 * \return
 * */
void devel_debug ( gchar *message )
{
    debug_message ( "enter into",
		    message,
		    DEBUG_LEVEL_DEBUG,
		    FALSE );
}


/**
 * send a notice default debug message
 *
 * \param message
 *
 * \return
 * */
void notice_debug ( gchar *message )
{
    debug_message ( "notice",
		    message,
		    DEBUG_LEVEL_NOTICE,
		    FALSE );
}


/**
 * send a warning default debug message
 *
 * \param message
 *
 * \return
 * */
void warning_debug ( gchar *message )
{
    debug_message ( "Warning : ",
		    message,
		    DEBUG_LEVEL_IMPORTANT,
		    FALSE );
}


/*************************************************************************************************************/
/* affiche de message de debug dans la console (uniquement si show_grisbi_debug est a TRUE)									 */
/*************************************************************************************************************/
void debug_message ( gchar *prefixe, gchar *message, gint level, gboolean force_debug_display)
{
    /* il faut bien entendu que le mode debug soit actif ou que l'on force l'affichage */
    if ( ( debugging_grisbi && level<=debugging_grisbi) || force_debug_display) 
    {
	/* on affiche dans la console le message */
	printf(g_strdup_printf(_("%s : %s - %s\n"),get_debug_time(),prefixe,message));

	/* Same for status bar */
	/* blocked by cedric : too slow... find another way ? */
	/* 	gsb_status_message ( message ); */
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
    gchar **backtrace_strings, *text = "";
    GtkWidget * label;
		
    backtrace_size = backtrace (backtrace_content, 15);
    backtrace_strings = backtrace_symbols (backtrace_content, backtrace_size);
		
    printf ("%s : %d elements in stack.\n", get_debug_time(), backtrace_size);
		
    for (i = 0; i < backtrace_size; i++) 
    {
	printf ("\t%s\n", backtrace_strings[i]);
	text = g_strconcat ( text, g_strconcat ( "\t", backtrace_strings[i], "\n", NULL ), 
			     NULL );
    }

    label = gtk_label_new ( text );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    return label;
#endif
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
