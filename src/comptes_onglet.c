/* fichier qui s'occupe de la gestion des comptes */
/*           gestion_comptes.c */

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


/*****************************************************************************************************/
GtkWidget *creation_onglet_comptes ( void )
{
  GtkWidget *onglet;
  GtkWidget *frame;

  onglet = gtk_hbox_new ( FALSE,
			     10 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


/*   création de la fenetre des comptes / ventilation / équilibrage à gauche */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			       GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (frame);

  gtk_container_add ( GTK_CONTAINER ( frame ),
		      creation_liste_comptes_onglet ());



  /* création de la partie droite */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			       GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show (frame);

  gtk_container_add ( GTK_CONTAINER ( frame ),
		      creation_details_compte ());

  /*  Création d'une icone et du nom par compte, et placement dans la liste selon l'ordre désiré  */
  /*   et met le livre ouvert pour le compte 0 */

  bouton_ouvert = NULL;
  compte_courant_onglet = 0;
  reaffiche_liste_comptes_onglet ();
  remplissage_details_compte ();
  
  return ( onglet );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
GtkWidget *creation_liste_comptes_onglet ( void )
{
  GtkWidget *onglet;
  GtkWidget *frame;
  GtkWidget *bouton;
  GtkWidget *vbox_frame;
  GtkWidget *scrolled_window;

  /*  Création d'une fenêtre générale*/

  onglet = gtk_vbox_new ( FALSE,
			  10);
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );

  /*  Création du label Comptes en haut */
  /*   on place le label dans une frame */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX (onglet),
		       frame,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show (frame);


  /*   on ne met rien dans le label, il sera rempli ensuite */

  label_compte_courant_onglet = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC (label_compte_courant_onglet  ),
			   0.5,
			   0.5);
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      label_compte_courant_onglet );
  gtk_widget_show (label_compte_courant_onglet);


  /*  Création de la fenêtre des comptes */
  /*  qui peut contenir des barres de défilement si */
  /*  nécessaire */

  scrolled_window = gtk_scrolled_window_new ( NULL,
					      NULL);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0);
  gtk_widget_show ( scrolled_window );
  

  /*  création d'une vbox contenant la liste des comptes */

  vbox_liste_comptes_onglet = gtk_vbox_new ( FALSE,
					     10);
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					  vbox_liste_comptes_onglet );
  gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				 GTK_SHADOW_NONE );
  gtk_widget_show ( vbox_liste_comptes_onglet );
  

  /* ajoute les boutons nouveau et supprimer */
  /* les 2 seront intégrés dans une frame */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_ETCHED_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show ( frame );


  vbox_frame = gtk_vbox_new ( FALSE,
					  5 );
  gtk_container_add ( GTK_CONTAINER  ( frame ),
		      vbox_frame );
  gtk_widget_show ( vbox_frame );

  /* mise en place des boutons du bas */

  bouton = gtk_button_new_with_label ( _("New account") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE);
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       bouton,
		       FALSE,
		       TRUE,
		       0);
  gtk_signal_connect ( GTK_OBJECT (bouton),
		       "clicked",
		       GTK_SIGNAL_FUNC ( nouveau_compte ),
		       NULL );
  gtk_widget_show ( bouton );



  /* mise en place du bouton équilibrage */

  bouton_supprimer_compte = gtk_button_new_with_label ( _("Remove an account") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_compte ),
			  GTK_RELIEF_NONE);
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       bouton_supprimer_compte,
		       FALSE,
		       TRUE,
		       0);
  gtk_signal_connect ( GTK_OBJECT (bouton_supprimer_compte),
		       "clicked",
		       GTK_SIGNAL_FUNC ( supprimer_compte ),
		       NULL );
  gtk_widget_show ( bouton_supprimer_compte );

  if ( !nb_comptes )
    gtk_widget_set_sensitive ( bouton_supprimer_compte,
			       FALSE );

  return ( onglet );
}
/*****************************************************************************************************/



/* ********************************************************************************************************** */
/** Fonction qui renvoie un widget contenant un bouton **/
/** de compte associé à son nom **/
/* ********************************************************************************************************** */

GtkWidget *comptes_appel_onglet ( gint no_de_compte )
{
  GtkWidget *win_icones;
  GtkWidget *bouton;
  GtkWidget *icone;
  GtkWidget *label;
  GtkWidget *hbox;


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_de_compte;

  win_icones = gtk_hbox_new ( FALSE,
			      10);

  /*   on crée le bouton contenant le livre fermé et ouvert, seul le fermé est affiché pour l'instant */

  bouton = gtk_button_new ();
  gtk_signal_connect ( GTK_OBJECT (bouton),
		       "clicked",
		       GTK_SIGNAL_FUNC ( changement_compte_onglet ),
		       GINT_TO_POINTER ( no_de_compte ) );

  gtk_button_set_relief ( GTK_BUTTON (bouton),
			  GTK_RELIEF_NONE);
  gtk_box_pack_start ( GTK_BOX (win_icones),
		       bouton,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show (bouton);


  /*   le bouton contient une hbox avec les 2 livres */
  /* on affiche l'icone ouverte si c'est le compte_courant de l'onglet */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      hbox );
  gtk_widget_show ( hbox );


  /* création de l'icone fermée */

  /* FIXME */
/*   icone = gnome_stock_pixmap_widget ( GTK_WIDGET ( bouton ), */
/* 				      GNOME_STOCK_PIXMAP_BOOK_BLUE); */
  icone = gtk_image_new_from_stock (GTK_STOCK_PRINT, GTK_ICON_SIZE_BUTTON);  
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       icone,
		       FALSE,
		       FALSE,
		       0 );
  if ( no_de_compte != compte_courant_onglet )
    gtk_widget_show ( icone );

  /* création de l'icone ouverte */

  /* FIXME */
/*   icone = gnome_stock_pixmap_widget ( GTK_WIDGET ( bouton ), */
/* 				      GNOME_STOCK_PIXMAP_BOOK_OPEN); */
  icone = gtk_image_new_from_stock (GTK_STOCK_PRINT, GTK_ICON_SIZE_BUTTON);  
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       icone,
		       FALSE,
		       FALSE,
		       0 );
  if ( no_de_compte == compte_courant_onglet )
    {
      gtk_widget_show ( icone );
      bouton_ouvert = bouton;
      gtk_label_set_text ( GTK_LABEL ( label_compte_courant_onglet),
			   NOM_DU_COMPTE);
    }

  /* on crée le label à coté du bouton */

  label = gtk_label_new ( NOM_DU_COMPTE );
  gtk_box_pack_start ( GTK_BOX (win_icones),
		       label,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show (label);


  return ( win_icones );
}
/* ********************************************************************************************************** */



/* ********************************************************************************************************** */
void changement_compte_onglet ( GtkWidget *bouton,
				gint compte )
{
  /* on ferme l'ancien bouton et on ouvre le nouveau */

  if ( bouton_ouvert )
    {
      gtk_widget_hide ( ((GtkBoxChild *)(GTK_BOX ( GTK_BIN ( bouton_ouvert ) -> child ) -> children -> next -> data)) -> widget );
      gtk_widget_show ( ((GtkBoxChild *)(GTK_BOX ( GTK_BIN ( bouton_ouvert ) -> child ) -> children -> data)) -> widget );
    }

  gtk_widget_hide ( ((GtkBoxChild *) (GTK_BOX ( GTK_BIN ( bouton ) -> child ) -> children -> data)) -> widget );
  gtk_widget_show ( ((GtkBoxChild *) (GTK_BOX ( GTK_BIN ( bouton ) -> child ) -> children -> next -> data)) -> widget );

  bouton_ouvert = bouton;

  /* demande si nécessaire si on enregistre */

  sort_du_detail_compte ();


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

  /* change le nom du compte courant */

  gtk_label_set_text ( GTK_LABEL ( label_compte_courant_onglet),
		       NOM_DU_COMPTE);

  compte_courant_onglet = compte;

  remplissage_details_compte ();
}
/* ********************************************************************************************************** */



/* *********************************************************************************************************** */
/*   on réaffiche la liste des comptes s'il y a eu un changement */
/* *********************************************************************************************************** */

void reaffiche_liste_comptes_onglet ( void )
{
  GSList *ordre_comptes_variable;
  GtkWidget *bouton;

/* commence par effacer tous les comptes */

  while ( GTK_BOX ( vbox_liste_comptes_onglet ) -> children )
    gtk_container_remove ( GTK_CONTAINER ( vbox_liste_comptes_onglet ),
			   (( GtkBoxChild *) ( GTK_BOX ( vbox_liste_comptes_onglet ) -> children -> data )) -> widget );


  /*  Création d'une icone et du nom par compte, et placement dans la liste selon l'ordre désiré  */

  if ( nb_comptes )
    {
      ordre_comptes_variable = ordre_comptes;

      do
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );
      
	  bouton = comptes_appel_onglet ( GPOINTER_TO_INT ( ordre_comptes_variable->data ));
	  gtk_box_pack_start (GTK_BOX (vbox_liste_comptes_onglet),
			      bouton,
			      FALSE,
			      FALSE,
			      0);
	  gtk_widget_show (bouton);

	}
      while ( (  ordre_comptes_variable = ordre_comptes_variable->next ) );
    }

  remplissage_details_compte ();

}
/* *********************************************************************************************************** */
