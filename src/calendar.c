/* ************************************************************************** */
/* Ce fichier s'occupe de la gestion des calendriers                          */
/* 			gsbcalendar.c                                         */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (dionysos@grisbi.org) 	      */
/*			http://www.grisbi.org   			      */
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


#include "calendar.h"



/******************************************************************************/
/* crée une fenètre (popup) calendrier dans le formulaire d'opération.        */
/* Cette fenètre est « rattachée » au widget (une entrée de texte) que l'on   */
/* passe en paramètre et qui récupèrera une date, à moins que l'on abandonne  */
/* la saisie de la date par un appui sur la touche ESC lorsque le calendrier  */
/* est ouvert                                                                 */
/******************************************************************************/
GtkWidget *gsb_calendar_new ( GtkWidget *entry )
{
  GtkWidget *popup;
  GtkWidget *pVBox;
  GtkRequisition *taille_popup;
  gint x, y;
  GtkWidget *pCalendar;
  int jour, mois, annee;
  GtkWidget *bouton;
  GtkWidget *frame;

  /* création de la popup */

  popup = gtk_window_new ( GTK_WINDOW_POPUP );
  gtk_window_set_modal ( GTK_WINDOW ( popup ), TRUE );
  gtk_signal_connect_object ( GTK_OBJECT ( popup ),
			      "destroy",
			      GTK_SIGNAL_FUNC ( gdk_pointer_ungrab ),
			      GDK_CURRENT_TIME );

  /* création de l'intérieur de la popup */

  frame = gtk_frame_new ( NULL );
  gtk_container_add ( GTK_CONTAINER ( popup ), frame );
  gtk_widget_show ( frame );

  pVBox = gtk_vbox_new ( FALSE, 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( pVBox ), 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ), pVBox );
  gtk_widget_show ( pVBox );

  if ( !( strlen ( g_strstrip ( (gchar*) gtk_entry_get_text ( GTK_ENTRY ( entry )))) &&
	  sscanf ( (gchar*) gtk_entry_get_text ( GTK_ENTRY ( entry )),
		   "%d/%d/%d",
		   &jour,
		   &mois,
		   &annee )))
    sscanf ( date_jour(),
	     "%d/%d/%d",
	     &jour,
	     &mois,
	     &annee);
      
  pCalendar = gtk_calendar_new();
  
  /* parce qu'il y a une différence de type et un décalage de valeur
     pour identifier le mois :
       - pour g_date, janvier = 1
       - pour gtk_calendar, janvier = 0 */
  gtk_calendar_select_month ( GTK_CALENDAR ( pCalendar ), mois - 1, annee );
  gtk_calendar_select_day ( GTK_CALENDAR ( pCalendar ), jour );
  gtk_calendar_display_options ( GTK_CALENDAR ( pCalendar ),
				 GTK_CALENDAR_SHOW_HEADING |
				 GTK_CALENDAR_SHOW_DAY_NAMES |
				 GTK_CALENDAR_WEEK_START_MONDAY );

  gtk_signal_connect ( GTK_OBJECT ( pCalendar ),
		       "day_selected_double_click",
		       GTK_SIGNAL_FUNC ( date_selection ),
		       entry );
  gtk_signal_connect ( GTK_OBJECT ( pCalendar ),
		       "key-press-event",
		       GTK_SIGNAL_FUNC ( clavier_calendrier ),
		       entry );
  gtk_box_pack_start ( GTK_BOX ( pVBox ),
		       pCalendar,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( pCalendar );

  /* ajoute le bouton annuler */
  bouton = gtk_button_new_with_label ( _("Cancel") );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
			      GTK_OBJECT ( popup ));

  gtk_box_pack_start ( GTK_BOX ( pVBox ),
		       bouton,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( bouton );

  /* cherche la position où l'on va mettre la popup */
  /* on récupère la position de l'entrée date par rapport à laquelle on va placer la popup */

  gdk_window_get_origin ( GTK_WIDGET ( entry ) -> window,
			  &x,
			  &y );

  /* on récupère la taille de la popup */

  taille_popup = malloc ( sizeof ( GtkRequisition ));
  gtk_widget_size_request ( GTK_WIDGET ( popup ), taille_popup );

  /* pour la soustraire à la position de l'entrée date */

  y -= taille_popup -> height;

  /* si une des coordonnées est négative, alors la fonction
  gtk_widget_set_uposition échoue et affiche la popup en 0,0 */

  if ( x < 0 )
  	x = 0 ;

  if ( y < 0 )
  	y = 0 ;

  /* on place la popup */

  gtk_widget_set_uposition ( GTK_WIDGET ( popup ),
			     x,
			     y );

  /* et on la montre */

  gtk_widget_show ( popup );
  gtk_widget_grab_focus ( GTK_WIDGET ( pCalendar ) );
  return ( popup );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction date_selectionnee */
/* appelée lorsqu'on a clické 2 fois sur une date du calendrier */
/******************************************************************************/
void date_selection ( GtkCalendar *pCalendar,
		      GtkWidget *entry )
{
  guint annee, mois, jour;
  GtkWidget *pTopLevelWidget;

  pTopLevelWidget = gtk_widget_get_toplevel ( GTK_WIDGET ( pCalendar ) );
  gtk_calendar_get_date ( pCalendar, &annee, &mois, &jour);

  gtk_entry_set_text ( GTK_ENTRY ( entry ),
		       g_strdup_printf ( "%02d/%02d/%04d",
					 jour,
					 mois + 1,
					 annee ));
  if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
     gtk_widget_destroy ( pTopLevelWidget );
}
/******************************************************************************/

/******************************************************************************/
/* Fonction clavier_calendrier                                                */
/* cette fonction est appelée à chaque appui sur une touche du clavier        */
/* lorsque qu'une fenêtre popup calendrier est ouverte.                       */
/******************************************************************************/
gboolean clavier_calendrier ( GtkCalendar *pCalendar,
			      GdkEventKey *ev,
			      GtkWidget *entry )
{
  gint jour, mois, annee;
  gint offset = 7;
  GtkWidget *pTopLevelWidget;

  /* on récupère la popup du calendrier pour pouvoir la détruire en temps voulu */
  pTopLevelWidget = gtk_widget_get_toplevel ( GTK_WIDGET ( pCalendar ) );

  gtk_calendar_get_date ( pCalendar, &annee, &mois, &jour );

  switch ( ev -> keyval )
    {
     case GDK_Escape :

       /* on quitte le calendrier et on le ferme sans rien faire */

       gtk_signal_emit_stop_by_name ( GTK_OBJECT ( pCalendar ),
				      "key-press-event");
       if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
	  gtk_widget_destroy ( pTopLevelWidget );
       break ;

     case GDK_Return :		/* touches entrée */
     case GDK_KP_Enter :

       /* on valide la date choisie */

       gtk_signal_emit_stop_by_name ( GTK_OBJECT ( pCalendar ),
				      "key-press-event");
       gtk_entry_set_text ( GTK_ENTRY ( entry ),
			    g_strdup_printf ( "%02d/%02d/%04d",
			    jour,
			    mois + 1,
			    annee ));
       if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
	  gtk_widget_destroy ( pTopLevelWidget );
       break ;

     case GDK_Left :		/* touche flèche gauche */
     case GDK_minus:		/* touches - */
     case GDK_KP_Subtract:

       /* on passe au jour précédent */

       jour--;
       /* si la date n'est pas valide, c'est parce qu'on a changé de mois */
       if ( g_date_valid_day ( jour ) != TRUE )
         {
	  /* donc, dernier jour ...*/
	  jour = 31 ;
	  /* ... du mois précédent */
	  mois-- ;
	  /* parce qu'il y a une différence de type et un décalage de valeur
	     pour identifier le mois :
	       - pour g_date, janvier = 1
	       - pour gtk_calendar, janvier = 0 */
	  /* si le mois n'est pas valide, c'est parce qu'on a changé d'année */
	  if ( g_date_valid_month ( mois + 1 ) != TRUE )
	    {
	     /* donc mois de décembre... */
	     mois = 11;
	     /* de l'année précédente */
	     annee--;
	     /* si l'année n'est pas valide, ça craint !!!!, alors on choisit
	        le début de notre ère chrétienne :-) */
	     if ( g_date_valid_year ( annee ) != TRUE )
	       annee = 1;
	    }
	  /* on cherche le dernier jour du mois */
	  while ( g_date_valid_dmy ( jour, mois + 1, annee ) != TRUE )
	    jour--;
	 }
       /* on positionne le calendrier au milieu du mois pour éviter des effets
          de bord de la fonction gtk_calendar_select_month */
       gtk_calendar_select_day( pCalendar , 15 );
       gtk_calendar_select_month ( pCalendar , mois, annee );
       gtk_calendar_select_day( pCalendar , jour );
       break ;

     case GDK_Right :		/* touche flèche droite */
     case GDK_plus:		/* touches + */
     case GDK_KP_Add:

       /* on passe au jour suivant */

       jour++;
       /* si la date n'est pas valide, c'est parce qu'on a changé de mois */
       if ( g_date_valid_dmy ( jour, mois + 1, annee ) != TRUE )
         {
	  /* donc, premier jour ...*/
	  jour = 1 ;
	  /* ... du mois suivant */
	  mois++ ;
	  /* si le mois n'est pas valide, c'est parce qu'on a changé d'année */
	  if ( g_date_valid_month ( mois + 1 ) != TRUE )
	    {
	     /* donc mois de janvier... */
	     mois = 0;
	     /* de l'année suivante */
	     annee++;
	     /* si l'année n'est pas valide, c'est qu'on a fait le tour du temps :-))),
	        alors on choisit le début de notre ère chrétienne :-) */
	     if ( g_date_valid_year ( annee ) != TRUE )
	       annee = 1;
	    }
	 }
       gtk_calendar_select_day( pCalendar , 15 );
       gtk_calendar_select_month ( pCalendar , mois, annee );
       gtk_calendar_select_day( pCalendar, jour );
       break ;

     case GDK_Up :		/* touche flèche haut */

       /* on passe à la semaine précédente */

       jour -= 7;
       /* si la date n'est pas valide, c'est parce qu'on a changé de mois */
       if ( g_date_valid_day ( jour ) != TRUE )
         {
	  /* on revient donc en arrière */
	  jour += 7;
	  offset = 7;
	  /* pour calculer le nombre de jours qu'il faut soustraire au mois
	     précédent */
	  while ( g_date_valid_day ( jour - 7 + offset  ) == TRUE )
	    offset--;
	  mois-- ;
	  /* si le mois n'est pas valide, c'est parce qu'on a changé d'année */
	  if ( g_date_valid_month ( mois + 1 ) != TRUE )
	    {
	     /* donc mois de décembre... */
	     mois = 11;
	     /* de l'année précédente */
	     annee--;
	     if ( g_date_valid_year ( annee ) != TRUE )
	       annee = 1;
	    }
	  /* on cherche quel est le dernier du mois */
	  jour = 31;
	  while ( g_date_valid_dmy ( jour, mois + 1, annee ) != TRUE )
	    jour--;
	  /* pour lui soustraire le nombre de jours précédemment calculé */
	  jour -= offset;
	 }
       gtk_calendar_select_day( pCalendar , 15 );
       gtk_calendar_select_month ( pCalendar , mois, annee );
       gtk_calendar_select_day( pCalendar, jour );
       break ;

     case GDK_Down :		/* touche flèche bas */

       /* on passe à la semaine suivante */

       jour += 7 ;
       if ( g_date_valid_dmy ( jour, mois + 1, annee ) != TRUE )
         {
	  jour -= 7;
	  offset = 0;
	  while ( g_date_valid_dmy ( jour + offset, mois + 1, annee ) == TRUE )
	    offset++;
	  offset--;
	  mois++ ;
	  /* parce qu'il y a une différence de type et un décalage de valeur
	     pour identifier le mois :
	       - pour g_date, janvier = 1
	       - pour gtk_calendar, janvier = 0 */
	  if ( g_date_valid_month ( mois + 1 ) != TRUE )
	    {
	     mois = 0;
	     annee++;
	     if ( g_date_valid_year ( annee ) != TRUE )
	       annee = 1;
	    }
	  jour = 7 - offset;
	 }
       gtk_calendar_select_day( pCalendar , 15 );
       gtk_calendar_select_month ( pCalendar , mois, annee );
       gtk_calendar_select_day( pCalendar, jour );
       break ;

     case GDK_Home :		/* touche Home */

       /* on passe au 1er jour du mois */
       
       gtk_calendar_select_day( pCalendar, 1 );
       break ;

     case GDK_End :		/* touche Home */

       /* on passe au dernier jour du mois */

       jour = 31;
       while ( g_date_valid_dmy ( jour, mois + 1, annee ) != TRUE )
	 jour--;
       gtk_calendar_select_day( pCalendar, jour );
       break ;

     case GDK_Page_Up :		/* touche PgUp */
     case GDK_KP_Page_Up :

       /* on passe au mois précédent */

       gtk_calendar_select_day( pCalendar , 15 );
       mois-- ;
       if ( g_date_valid_month ( mois + 1 ) != TRUE )
	 {
	  mois = 11;
	  annee--;
	  if ( g_date_valid_year ( annee ) != TRUE )
	    annee = 1;
	 }
       while ( g_date_valid_dmy ( jour, mois + 1, annee ) != TRUE )
	 jour--;
       gtk_calendar_select_day( pCalendar , 15 );
       gtk_calendar_select_month ( pCalendar , mois, annee );
       gtk_calendar_select_day( pCalendar, jour );
       break ;

     case GDK_Page_Down :		/* touche PgDn */
     case GDK_KP_Page_Down :

       /* on passe au mois suivant */

       gtk_calendar_select_day( pCalendar , 15 );
       mois++ ;
       if ( g_date_valid_month ( mois + 1 ) != TRUE )
	 {
	  mois = 0;
	  annee++;
	  if ( g_date_valid_year ( annee ) != TRUE )
	    annee = 1;
	 }
       while ( g_date_valid_dmy ( jour, mois + 1, annee ) != TRUE )
	 jour--;
       gtk_calendar_select_day( pCalendar , 15 );
       gtk_calendar_select_month ( pCalendar , mois, annee );
       gtk_calendar_select_day( pCalendar, jour );
       break ;

     default :

       break ;
    }
  return TRUE;
}
/******************************************************************************/

/******************************************************************************/
void calendar_destroy2 ( GtkButton *button,
			GdkEventKey *ev,
			GtkCalendar *pCalendar )
{
  GtkWidget *pTopLevelWidget;
	  
	  
	  dialogue("1");

  pTopLevelWidget = gtk_widget_get_toplevel ( GTK_WIDGET ( pCalendar ) );
  	  dialogue("2");
  
	  
	     
       if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
       {
	  dialogue("3");
	  gtk_widget_destroy ( pTopLevelWidget );
	}

}


/******************************************************************************/
void calendar_destroyed ( GtkWidget *popup,
			GdkEventKey *ev,
			GtkEntry *entry )
{
/*  GtkWidget *pTopLevelWidget;

  dialogue("1");

  pTopLevelWidget = gtk_widget_get_toplevel ( GTK_WIDGET ( pCalendar ) );
  
  dialogue("2");
     
  if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
    {
     dialogue("3");
     gtk_widget_destroy ( pTopLevelWidget );
    }
*/
  GtkWidget *tmp;

  tmp = gtk_grab_get_current ();
  if (tmp)
    gtk_grab_remove ( tmp );

  gtk_grab_add (GTK_WIDGET ( entry ));

//gdk_pointer_ungrab (GDK_CURRENT_TIME);

}
/******************************************************************************/


gboolean calendar_destroy3 ( GtkWidget *popup,
			  GdkEventKey *ev,
			  GtkWidget *entry )
{
  GtkWidget *pTopLevelWidget;

  pTopLevelWidget = gtk_widget_get_ancestor ( GTK_WIDGET ( entry ), GTK_WINDOW_TOPLEVEL );
/*	      gtk_signal_emit_by_name ( GTK_OBJECT ( entry ),
					     "focus_out_event");*/
       if ( pTopLevelWidget != NULL )
	 {
	  dialogue("Coucou 2");
	  gtk_grab_remove ( GTK_WIDGET ( pTopLevelWidget ));
	  gtk_grab_add ( GTK_WIDGET ( pTopLevelWidget ));
	 }
       else
	  dialogue("Tant pis !!!");
       /*
       
       if ( GTK_WIDGET_TOPLEVEL ( pTopLevelWidget ) )
	 {
	  pTopLevelWindow = gdk_window_get_parent ( GTK_WIDGET ( pTopLevelWidget ) -> window );

   gtk_widget_grab_focus ( GTK_WIDGET ( entry ) );
  */
	  gtk_grab_remove ( GTK_WIDGET ( pTopLevelWidget ));
	  gtk_grab_add ( GTK_WIDGET ( pTopLevelWidget ));
}


