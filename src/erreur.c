/* Fichier erreur.c */
/* Contient toutes les procédures relatives aux erreurs et à la fin du prog */

/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "en_tete.h"

/*****************************************************************************************************************/
/* fonction appelée lors de la demande de fermeture du fichier de compte en cours */
/*****************************************************************************************************************/


gboolean fermeture_grisbi ( void )
{

/*       stoppe le timer */

  if ( id_temps )
    {
      gtk_timeout_remove ( id_temps );
      id_temps = 0;
    }


  sauve_configuration ();

/*   si le fichier est modifié, propose l'enregistrement */

  if ( enregistrement_fichier(-1) )
    gtk_main_quit();


  /* si le fichier n'était pas déjà ouvert, met à 0 l'ouverture */

  if ( !etat.fichier_deja_ouvert
       &&
       nb_comptes
       &&
       nom_fichier_comptes )
    fichier_marque_ouvert ( FALSE );

  return (TRUE );
}
/*****************************************************************************************************************/


/*************************************************************************************************************/
/* Fonction dialogue : */
/* affiche le texte donné en argument et attend l'appui sur OK */
/*************************************************************************************************************/

void dialogue ( gchar *texte_dialogue )
{

  win_erreur = gnome_warning_dialog_parented ( texte_dialogue ,
					       GTK_WINDOW ( window) );	     
  gtk_window_set_modal ( GTK_WINDOW ( win_erreur ),
			 TRUE );
  gnome_dialog_run_and_close ( GNOME_DIALOG ( win_erreur ));

}
/*************************************************************************************************************/





/*************************************************************************************************************/
gboolean blocage_boites_dialogues ( GtkWidget *dialog,
				    gpointer null )
{
  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( dialog ),
				 "delete_event" );

  return ( TRUE );
}
/*************************************************************************************************************/


/*************************************************************************************************************/
/* Fonction question : */
/* affiche le texte donné en argument et attend l'appui sur oui ( renvoie TRUE ) ou non (renvoie FALSE )*/
/*************************************************************************************************************/

gboolean question ( gchar *texte )
{
  GtkWidget *dialogue;
  GtkWidget *label;
  gint resultat;


  dialogue = gnome_dialog_new ( "Warning",
				GNOME_STOCK_BUTTON_OK,
				GNOME_STOCK_BUTTON_CANCEL,
				NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				 GTK_WINDOW ( window ) );
  gtk_signal_connect ( GTK_OBJECT ( dialogue ),
		       "delete_event",
		       GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
		       NULL );

  label = gtk_label_new ( texte );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       label,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialogue ));

  if ( resultat )
    return ( FALSE );
  else
    return ( TRUE );
}
/*************************************************************************************************************/



/*************************************************************************************************************/
/* Fonction question_yes_no : */
/* affiche le texte donné en argument et attend l'appui sur oui ( renvoie TRUE ) ou non (renvoie FALSE )*/
/*************************************************************************************************************/

gboolean question_yes_no ( gchar *texte )
{
  GtkWidget *dialogue;
  GtkWidget *label;
  gint resultat;

  dialogue = gnome_dialog_new ( "Warning",
				GNOME_STOCK_BUTTON_YES,
				GNOME_STOCK_BUTTON_NO,
				NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				 GTK_WINDOW ( window ) );
  gtk_signal_connect ( GTK_OBJECT ( dialogue ),
		       "delete_event",
		       GTK_SIGNAL_FUNC ( blocage_boites_dialogues ),
		       NULL );

  label = gtk_label_new ( texte );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       label,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialogue ));

  if ( resultat )
    return ( FALSE );
  else
    return ( TRUE );
}
/*************************************************************************************************************/



/*************************************************************************************************************/
/* fonction demande_texte */
/* affiche une boite de dialogue qui demande l'entrée d'un texte */
/* renvoie NULL si annulé ou le texte */
/*************************************************************************************************************/

gchar *demande_texte ( gchar *titre_fenetre,
		       gchar *question )
{
  GtkWidget *dialog;
  gint resultat;
  GtkWidget *label;
  GtkWidget *entree;
  gchar *retour;

  dialog = gnome_dialog_new ( titre_fenetre,
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			     0 );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));

  gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER );

  label = gtk_label_new ( question );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  entree = gtk_entry_new ();
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( entree ));
  gtk_window_set_focus ( GTK_WINDOW ( dialog ),
			entree );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       entree,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree );

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    {
      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return ( NULL );
    }

  retour = g_strstrip ( g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entree ))));

  gnome_dialog_close ( GNOME_DIALOG ( dialog ));
  return ( retour );
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

  dialog = gnome_dialog_new ( _("Log message"),
			      GNOME_STOCK_BUTTON_OK,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ) );

  label = gtk_label_new ( COLON(_("This operation returned a message")));
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
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
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
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

  gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));

}
/*************************************************************************************************************/



/*************************************************************************************************************/
void traitement_sigsegv ( gint signal_nb )
{
  GtkWidget *dialog;
  gchar *home_dir;

  /*   il y a 3 possibilités : */
  /*     soit on était en train de charger un fichier, c'est que celui-ci est corrompu */
  /* soit on était en train de sauver un fichier, et là on peut rien faire */
  /* sinon on essaie de sauver le fichier sous le nom entouré de # */

  if ( etat.en_train_de_charger )
    {
      dialog = gnome_error_dialog ( _("Ooops, Grisbi crashed while loading the file (segfault).\nEither the file is corrupted, or it is a major bug.\nTry to load the file again. If you can repeat this crash, could you report it on Grisbi's bugtracker (http://www.grisbi.org/bugtracking/)?\nIt would really help us to fix it.\nThanks a lot!"));
      gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));
      fichier_marque_ouvert ( FALSE );
      gtk_main_quit();
      exit(0);
    }


  if ( etat.en_train_de_sauvegarder )
    {
      dialog = gnome_error_dialog ( _("Ooops, Grisbi crashed while trying to save the file (segfault).\nSo it's impossible to save your file.\nIf you can repeat this crash, could you report it on Grisbi's bugtracker (http://www.grisbi.org/bugtracking/)?\nIt would really help us to fix it.\nThanks a lot!"));
      gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));
      fichier_marque_ouvert ( FALSE );
      gtk_main_quit();
      exit(0);
    }

  if ( !etat.modification_fichier )
    {
      dialog = gnome_error_dialog ( _("Ooops, Grisbi crashed (segfault).\nNo matter, you didn't modify anything.\nIf you can repeat this crash, could you report it on Grisbi's bugtracker (http://www.grisbi.org/bugtracking/)?\nIt would really help us to fix it.\nThanks a lot!"));
      gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));
      fichier_marque_ouvert ( FALSE );
      gtk_main_quit();
      exit(0);
    }

  /* c'est un bug pendant le fonctionnement de Grisbi */

  /*   s'il n'y a pas de nom de fichier, on le crée, sinon on rajoute # autour */

  home_dir = getenv ("HOME");

  if ( nom_fichier_comptes )
    {
      /* on récupère le nome du fichier sans le chemin */

      gchar **parametres;
      gint i=0;

      parametres = g_strsplit ( nom_fichier_comptes,
				"/",
				0);

      while ( parametres[i] )
	i++;

      nom_fichier_comptes = g_strconcat ( home_dir,
					  "/#",
					  parametres [i-1],
					  "#",
					  NULL );
      g_strfreev ( parametres );
    }
  else
    nom_fichier_comptes = g_strconcat ( home_dir,
					"/#grisbi_plantage_sans_nom#",
					NULL );

    if ( patience_en_cours )
      update_attente ( _("Save file") );
    else
      mise_en_route_attente ( _("Save file") );

  enregistre_fichier ( 1 );
  annulation_attente();

  dialog = gnome_error_dialog ( g_strdup_printf (_("Ooops, Grisbi crashed (segfault).\nGrisbi tried to save your file before, with the name %s.\nTry to load it, with a bit of luck you won't loose anything.\nIf you can repeat this crash, could you report it on Grisbi's bugtracker (http://www.grisbi.org/bugtracking/)?\nIt would really help us to fix it.\nThanks a lot!"),
						 nom_fichier_comptes ));
  gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));
  gtk_main_quit();
  exit(0);

}
/*************************************************************************************************************/
