/* Fichier erreur.c */
/* Contient toutes les procédures relatives aux erreurs et à la fin du prog */

/*     Copyright (C) 2000-2002  Cédric Auger */
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
