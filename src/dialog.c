/* ************************************************************************** */
/* Fichier qui s'occupe de la page d'accueil ( de démarrage lors de           */
/* l'ouverture d'un fichier de comptes                                        */
/*                                                                            */
/*                                  accueil.c                                 */
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
#include "variables-extern.c"

#include "dialog.h"
#include "parametres.h"



/**
 * Display an info dialog window
 *
 * \param text Text to display in window
 */
void dialogue ( gchar *texte_dialogue )
{
  dialogue_special ( GTK_MESSAGE_INFO, texte_dialogue );
}


/**
 * Display an error dialog window
 *
 * \param text Text to display in window
 */
void dialogue_error ( gchar *text )
{
  dialogue_special ( GTK_MESSAGE_ERROR, text );
}


/**
 * Display a warning dialog window
 *
 * \param text Text to display in window
 */
void dialogue_warning ( gchar *text )
{
  dialogue_special ( GTK_MESSAGE_WARNING, text );
}


/**
 * Display a dialog window with arbitrary icon.
 *
 * \param param Type of Window to display
 * \param text Text to display in window
 */
void dialogue_special ( GtkMessageType param, gchar * text )
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new ( GTK_WINDOW (window), GTK_DIALOG_DESTROY_WITH_PARENT,
				    param, GTK_BUTTONS_CLOSE, text );
  
  gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy ( dialog );
}


/**
 * This function pop ups a dialog with an informal text and a checkbox
 * that allow this message not to be displayed again thanks to
 * preferences.
 *
 * \param text text to be displayed
 * \param var variable that both controls whether the dialog will
 * appear or not and that indicates which variable could be modified
 * so that this message won't appear again.
 */
void dialogue_conditional ( gchar *text, int * var )
{
  GtkWidget * vbox, * checkbox;

  if ( !var || *var)
    return;

  win_erreur = gtk_message_dialog_new ( GTK_WINDOW (window),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					text );
  vbox = GTK_DIALOG(win_erreur) -> vbox;
  checkbox = new_checkbox_with_title ( _("Do not show this message again"), var, 
				       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, TRUE, TRUE, 6 );
  gtk_widget_show_all ( checkbox );

  gtk_window_set_modal ( GTK_WINDOW ( win_erreur ), TRUE );
  gtk_dialog_run (GTK_DIALOG (win_erreur));
  gtk_widget_destroy ( win_erreur );
}



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



/*************************************************************************************************************/
gboolean blocage_boites_dialogues ( GtkWidget *dialog,
				    gpointer null )
{
  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( dialog ),
				 "delete_event" );

  return ( TRUE );
}
/*************************************************************************************************************/


