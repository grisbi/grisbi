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
#include "fichier_configuration_constants.h"


/*START_INCLUDE*/
#include "erreur.h"
#include "patienter.h"
#include "dialog.h"
#include "fichiers_io.h"
#include "fichiers_gestion.h"
#include "traitement_variables.h"
#include "utils_files.h"
#include "fichier_configuration.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/



/*START_EXTERN*/
extern gint id_temps;
extern gchar *log_message;
extern gchar *nom_fichier_comptes;
extern GtkWidget *window;
/*END_EXTERN*/





/*****************************************************************************************************************/
/* fonction appelée lors de la demande de fermeture de grisbi */
/*****************************************************************************************************************/


gboolean fermeture_grisbi ( void )
{

    /*     si le fichier n'est pas enregistré, on le propose */

    if ( etat.modification_fichier )
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

    sauve_configuration ();

    modification_etat_ouverture_fichier ( FALSE );

    /*       stoppe le timer */

    if ( id_temps )
    {
	gtk_timeout_remove ( id_temps );
	id_temps = 0;
    }

    gtk_main_quit();
    return TRUE;
}
/*************************************************************************************************************/






/*************************************************************************************************************/
/* affiche le contenu de la variable log_message si elle n'est pas null */
/*************************************************************************************************************/

void affiche_log_message ( void )
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *vbox;

    if ( !log_message )
	return;

    dialog = gtk_dialog_new_with_buttons ( _("Log message"),
					   GTK_WINDOW (window),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_OK,0,
					   NULL );

    label = gtk_label_new ( COLON(_("This operation returned a message")));
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			 label,
			 FALSE,
			 FALSE, 
			 0 );
    gtk_widget_show ( label );

    scrolled_window = gtk_scrolled_window_new ( NULL,
						NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_widget_set_usize ( scrolled_window,
			   400,
			   200 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialog ) -> vbox ),
			 scrolled_window,
			 FALSE,
			 FALSE, 
			 0 );
    gtk_widget_show ( scrolled_window );

    vbox = gtk_vbox_new ( FALSE,
			  5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    vbox );
    gtk_widget_show ( vbox );

    label = gtk_label_new ( log_message );
    gtk_label_set_justify ( GTK_LABEL ( label ),
			    GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label,
			 FALSE,
			 FALSE, 
			 0 );
    gtk_widget_show ( label );

    gtk_dialog_run ( GTK_DIALOG ( dialog ));
    gtk_widget_destroy ( dialog );

}
/*************************************************************************************************************/



/*************************************************************************************************************/
void traitement_sigsegv ( gint signal_nb )
{
    gchar *gsb_file_default_dir, *errmsg;

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

	mise_en_route_attente ( _("Save file") );

	enregistre_fichier ( nom_fichier_comptes );

	annulation_attente();

	errmsg = g_strconcat ( errmsg, 
			       g_strdup_printf ( _("Grisbi made a backup file at '%s'."),
						 nom_fichier_comptes ),
			       NULL );
	errmsg = g_strconcat ( errmsg, "\n\n", NULL );
    }

    errmsg = g_strconcat ( errmsg, 
			   _("Please report this problem to http://www.grisbi.org/bugtracking/"),
			   NULL );

    dialogue_error_hint ( errmsg, 
			  _("Grisbi terminated due to a segmentation fault.") );

    /*     on évite le message du fichier ouvert à la prochaine ouverture */

    modification_etat_ouverture_fichier ( FALSE );
    exit(1);
}

/*************************************************************************************************************/
