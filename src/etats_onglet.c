/*  Fichier qui s'occupe de l'onglet états */
/*      etats.c */

/*     Copyright (C) 2000-2002  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */
/*     Copyright (C) 2002  Benjamin Drieu */
/* 			bdrieu@april.org */
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
GtkWidget *creation_onglet_etats ( void )
{
  GtkWidget *onglet;
  GtkWidget *frame;
  GtkWidget *vbox;

  /*   au départ, aucun état n'est ouvert */

  bouton_etat_courant = NULL;
  etat_courant = NULL;


  onglet = gtk_hbox_new ( FALSE,
			  10 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


  /*   création de la fenetre des noms des états */
  /* on reprend le principe des comptes dans la fenetre des opés */

  frame_liste_etats = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_liste_etats ),
			      GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame_liste_etats,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (frame_liste_etats);

  /* on y met les rapports et les boutons */

  gtk_container_add ( GTK_CONTAINER ( frame_liste_etats ),
		      creation_liste_etats ());



  /* création du notebook contenant l'état et la config */

  notebook_etats = gtk_notebook_new ();
  gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook_etats ),
			       FALSE );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       notebook_etats,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( notebook_etats );


  /* création de la partie droite */

  vbox = gtk_vbox_new ( FALSE,
			10 );
  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_etats ),
			     vbox,
			     gtk_label_new ( _( "Affichage états" )));
  gtk_widget_show ( vbox );


  /*  Création de la partie contenant l'état */

  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_IN );
  gtk_widget_show (frame);

  /* on y met une scrolled window qui sera remplit par l'état */

  scrolled_window_etat = gtk_scrolled_window_new ( FALSE,
						   FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_etat ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      scrolled_window_etat );
  gtk_widget_show ( scrolled_window_etat );

  /* création de la partie contenant les boutons (personnaliser ...) */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_droite_bas ),
			      GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (frame);

  /* on y met les boutons */

  gtk_container_add ( GTK_CONTAINER ( frame ),
		      creation_barre_boutons_etats ());


  /* l'onglet de config sera créé que si nécessaire */

  onglet_config_etat = NULL;


  return ( onglet );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
/* vontion creation_liste_etats */
/* renvoie la partie gauche de l'onglet rapports financiers */
/*****************************************************************************************************/

GtkWidget *creation_liste_etats ( void )
{
  GtkWidget *onglet;
  GtkWidget *frame;
  GtkWidget *bouton;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;


  /*  Création d'une fenêtre générale*/

  onglet = gtk_vbox_new ( FALSE,
			  10);
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


  /*  Création du label contenant le rapport courant en haut */
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

  label_etat_courant = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC (label_etat_courant  ),
			   0.5,
			   0.5);
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      label_etat_courant );
  gtk_widget_show (label_etat_courant);


  /*  Création de la fenêtre des etats */
  /*  qui peut contenir des barres de défilement si */
  /*  nécessaire */

  scrolled_window = gtk_scrolled_window_new ( NULL,
					      NULL);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0);
  gtk_widget_show ( scrolled_window );
  

  /*  création de la vbox qui contient la liste des états */

  vbox_liste_etats = gtk_vbox_new ( FALSE,
				    10);
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					  vbox_liste_etats);
  gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				 GTK_SHADOW_NONE );
  gtk_widget_show (vbox_liste_etats);
  

  /*  ajout des différents états */

  remplissage_liste_etats ();

  /* ajout des boutons pour supprimer / ajouter un état */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_ETCHED_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show ( frame );


  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_container_add ( GTK_CONTAINER  ( frame ),
		      vbox );
  gtk_widget_show ( vbox );


  /* mise en place du bouton ajouter */

  bouton = gtk_button_new_with_label ( _("Ajouter un état") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE);
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       FALSE,
		       TRUE,
		       0);
  gtk_signal_connect ( GTK_OBJECT (bouton),
		       "clicked",
		       GTK_SIGNAL_FUNC ( ajout_etat ),
		       NULL );
  gtk_widget_show ( bouton );

  /* on met le bouton personnaliser */

  bouton_personnaliser_etat = gtk_button_new_with_label ( _("Personnaliser l'état") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_personnaliser_etat ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_personnaliser_etat,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_personnaliser_etat ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( personnalisation_etat ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_personnaliser_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_personnaliser_etat );

  /* on met le bouton dupliquer */

  bouton_dupliquer_etat = gtk_button_new_with_label ( _("Dupliquer l'état") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_dupliquer_etat ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_dupliquer_etat ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( dupliquer_etat ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_dupliquer_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_dupliquer_etat );

  /* mise en place du bouton effacer état */

  bouton_effacer_etat = gtk_button_new_with_label ( _("Effacer l'état") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_effacer_etat ),
			  GTK_RELIEF_NONE);
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_effacer_etat,
		       FALSE,
		       TRUE,
		       0);
  gtk_signal_connect ( GTK_OBJECT (bouton_effacer_etat),
		       "clicked",
		       GTK_SIGNAL_FUNC ( efface_etat ),
		       NULL );
  gtk_widget_show ( bouton_effacer_etat );

  if ( !etat_courant )
    {
      gtk_widget_set_sensitive ( bouton_effacer_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_dupliquer_etat,
				 FALSE );
    }

  return ( onglet );

}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Fonction creation_barre_boutons_etats */
/* renvoie la barre des boutons de la partie droite-bas des rapports financiers */
/*****************************************************************************************************/

GtkWidget *creation_barre_boutons_etats ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *bouton;

  widget_retour = gtk_hbox_new ( FALSE,
				 5 );
  gtk_widget_show ( widget_retour );

  /* on met le bouton rafraichir */

  bouton = gtk_button_new_with_label ( _("Rafraichir") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( rafraichissement_etat ),
			      NULL );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  /* on met le bouton imprimer */

  bouton_imprimer_etat = gtk_button_new_with_label ( _("Imprimer") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_imprimer_etat ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_imprimer_etat ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( impression_etat ),
			      NULL );
  gtk_widget_set_sensitive ( bouton_imprimer_etat,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_imprimer_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_imprimer_etat );

  /* on met le bouton exporter */

  bouton_exporter_etat = gtk_button_new_with_label ( _("Exporter") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_exporter_etat ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_exporter_etat,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_exporter_etat ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( exporter_etat ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_exporter_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exporter_etat );

  /* on met le bouton importer */

  bouton_importer_etat = gtk_button_new_with_label ( _("Importer") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_importer_etat ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_importer_etat ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( importer_etat ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_importer_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_importer_etat );

  return ( widget_retour );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Fontion remplissage_liste_etats */
/* vide et remplit la liste des états */
/*****************************************************************************************************/

void remplissage_liste_etats ( void )
{
  GList *pointeur;
  GSList *liste_tmp;


  /* on commence par détruire tous les enfants de la vbox */

  pointeur = GTK_BOX ( vbox_liste_etats ) -> children;

  while ( pointeur )
    {
      GtkBoxChild *child;

      child = pointeur -> data;

      pointeur = pointeur -> next;

      gtk_container_remove ( GTK_CONTAINER ( vbox_liste_etats ),
			     child -> widget );
    }

  /* on remplit maintenant avec tous les états */

  liste_tmp = liste_struct_etats;

  while ( liste_tmp )
    {
      struct struct_etat *etat;
      GtkWidget *hbox;
      GtkWidget *bouton;
      GtkWidget *icone;
      GtkWidget *label;

      etat = liste_tmp -> data;

      hbox = gtk_hbox_new ( FALSE,
			    10);
      gtk_box_pack_start ( GTK_BOX ( vbox_liste_etats ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( hbox );


      /*   on crée le bouton contenant le livre fermé et ouvert, seul le fermé est affiché pour l'instant */

      bouton = gtk_button_new ();
      gtk_button_set_relief ( GTK_BUTTON (bouton),
			      GTK_RELIEF_NONE);
      gtk_signal_connect ( GTK_OBJECT (bouton),
			   "clicked",
			   GTK_SIGNAL_FUNC ( changement_etat ),
			   etat );
      gtk_box_pack_start ( GTK_BOX (hbox),
			   bouton,
			   FALSE,
			   TRUE,
			   0);
      gtk_widget_show (bouton);


      /* création de l'icone fermée */

      if ( etat_courant
	   &&
	   etat -> no_etat == etat_courant -> no_etat )
	{
	  icone = gnome_stock_pixmap_widget ( GTK_WIDGET ( bouton ),
					      GNOME_STOCK_PIXMAP_BOOK_OPEN);
	  bouton_etat_courant = bouton;
	}
      else
	icone = gnome_stock_pixmap_widget ( GTK_WIDGET ( bouton ),
					    GNOME_STOCK_PIXMAP_BOOK_BLUE);
      gtk_container_add ( GTK_CONTAINER ( bouton ),
			  icone );
      gtk_widget_show ( icone );

      /* on crée le label à coté du bouton */

      label = gtk_label_new ( etat -> nom_etat );
      gtk_box_pack_start ( GTK_BOX (hbox),
			   label,
			   FALSE,
			   TRUE,
			   0);
      gtk_widget_show (label);

      liste_tmp = liste_tmp -> next;
    }

  /*   si on a remplit la liste, c'est qu'il y a eu des modifs ( ajout, dupplication ... ) */
  /* donc on met à jour la liste des tiers */

  mise_a_jour_tiers ();

}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* on propose une liste d'états prémachés et les remplis en fonction du choix */
/* de la personne */
/*****************************************************************************************************/

void ajout_etat ( void )
{
  struct struct_etat *etat;
  GtkWidget *dialog;
  gint resultat;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *option_menu;
  GtkWidget *menu;
  GtkWidget *menu_item;


  dialog = gnome_dialog_new ( _("Création d'un état"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gtk_widget_set_usize ( dialog,
			 400,
			 250 );
  gtk_window_set_policy ( GTK_WINDOW ( dialog ),
			  FALSE,
			  FALSE,
			  FALSE );

  label = gtk_label_new ( _("Sélectionner le type d'état voulu :" ));
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  /* on crée la frame avant l'option menu */
  /* pour pouvoir l'envoyer par les menu_item */

  frame = gtk_frame_new ( _("Description :" ));


  option_menu = gtk_option_menu_new ();

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _("Revenus et dépenses du mois dernier"));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_etat",
			GINT_TO_POINTER ( 0 ));
  gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			      GTK_OBJECT ( frame ));
  gtk_widget_show ( menu_item );

  /* on met le texte du 1er choix */

  change_choix_nouvel_etat ( menu_item,
			     frame );


  menu_item = gtk_menu_item_new_with_label ( _("Revenus et dépenses du mois en cours"));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_etat",
			GINT_TO_POINTER ( 1 ));
  gtk_signal_connect ( GTK_OBJECT ( menu_item ),
		       "activate",
		       GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
		       GTK_OBJECT ( frame ));
  gtk_widget_show ( menu_item );


  menu_item = gtk_menu_item_new_with_label ( _("État vierge"));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_etat",
			GINT_TO_POINTER ( 2 ));
  gtk_signal_connect ( GTK_OBJECT ( menu_item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( change_choix_nouvel_etat ),
			      GTK_OBJECT ( frame ));
  gtk_widget_show ( menu_item );

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ),
			     menu );
  gtk_widget_show ( menu );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       option_menu,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( option_menu );

  /* on ajoute maintenant la frame */

  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( frame );


  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    {
      if ( GNOME_IS_DIALOG ( dialog ))
	gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }


  /* on récupère le type d'état voulu */

  resultat = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
						     "no_etat" ));
  gnome_dialog_close ( GNOME_DIALOG ( dialog ));


  /* on crée le nouvel état */

  etat = calloc ( 1,
		  sizeof ( struct struct_etat ));

  etat -> no_etat = ++no_dernier_etat;


  /* on remplit maintenant l'état en fonction de ce qu'on a demandé */

  switch ( resultat )
    {
    case 0:
      /*  revenus et dépenses du mois précédent  */

      etat -> nom_etat = g_strdup ( _("Revenus et dépenses du mois précédent") );

      etat -> separer_revenus_depenses = 1;
      etat -> no_plage_date = 7;


      /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 1 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 2 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 3 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 4 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 5 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 6 ));

      etat -> type_virement = 2;
      etat -> utilise_categ = 1;
      etat -> afficher_sous_categ = 1;
      etat -> affiche_sous_total_categ = 1;
      etat -> affiche_sous_total_sous_categ = 1;
      etat -> afficher_pas_de_sous_categ = 1;
      etat -> afficher_nom_categ = 1;

      /*   les devises sont à 1 (euro) */

      etat -> devise_de_calcul_general = 1;
      etat -> devise_de_calcul_categ = 1;
      etat -> devise_de_calcul_ib = 1;
      etat -> devise_de_calcul_tiers = 1;

      break;

    case 1:
     /*  revenus et dépenses du mois courant  */

      etat -> nom_etat = g_strdup ( _("Revenus et dépenses du mois en cours") );

      etat -> separer_revenus_depenses = 1;
      etat -> no_plage_date = 3;


      /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 1 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 2 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 3 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 4 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 5 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 6 ));

      etat -> type_virement = 2;
      etat -> utilise_categ = 1;
      etat -> afficher_sous_categ = 1;
      etat -> affiche_sous_total_categ = 1;
      etat -> affiche_sous_total_sous_categ = 1;
      etat -> afficher_pas_de_sous_categ = 1;
      etat -> afficher_nom_categ = 1;

      /*   les devises sont à 1 (euro) */

      etat -> devise_de_calcul_general = 1;
      etat -> devise_de_calcul_categ = 1;
      etat -> devise_de_calcul_ib = 1;
      etat -> devise_de_calcul_tiers = 1;

      break;


    case 2:

      /* on ajoute un état vierge appelé nouvel état */
      /*   pour modifier le nom, il faudra aller dans la */
      /* personnalisation */

      etat -> nom_etat = g_strdup ( _("Nouvel état") );

      /*   le classement de base est 1-2-3-4-5-6 (cf structure.h) */

      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 1 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 2 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 3 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 4 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 5 ));
      etat -> type_classement = g_list_append ( etat -> type_classement,
						GINT_TO_POINTER ( 6 ));

      /*   les devises sont à 1 (euro) */

      etat -> devise_de_calcul_general = 1;
      etat -> devise_de_calcul_categ = 1;
      etat -> devise_de_calcul_ib = 1;
      etat -> devise_de_calcul_tiers = 1;

      /*   tout le reste est à NULL, ce qui est très bien */

  break;

    default :
      dialogue ( _( "Type d'état inconnu, création abandonnée" ));
      return;
    }

  /* on l'ajoute à la liste */

  liste_struct_etats = g_slist_append ( liste_struct_etats,
					etat );

  /* on réaffiche la liste des états */

  etat_courant = etat;

  remplissage_liste_etats ();

  gtk_widget_set_sensitive ( bouton_personnaliser_etat,
			     TRUE );
  /* FIXME: réactiver àca le jour ou on sort l'impression
	mais de toutes faàons, àca sera mergé 
  gtk_widget_set_sensitive ( bouton_imprimer_etat,
			     TRUE );
  */
  gtk_widget_set_sensitive ( bouton_exporter_etat,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_dupliquer_etat,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_effacer_etat,
			     TRUE );

  gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
		       etat_courant -> nom_etat );

  personnalisation_etat ();
  modification_fichier ( TRUE );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void change_choix_nouvel_etat ( GtkWidget *menu_item,
				GtkWidget *frame )
{
  gchar *description;
  GtkWidget *label;

  switch ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( menu_item ),
						   "no_etat" )))
    {
    case 0:
      /* revenus et dépenses du mois dernier  */

      description = _("Cet état affiche les totaux classés par catégorie et sous-catégorie des opérations du mois dernier. Il suffira de choisir le ou les comptes et de valider (par-défaut, tous les comptes sont utilisés).");
      break;

    case 1:
      /* revenus et dépenses du mois en cours  */

      description = _("Cet état affiche les totaux classés par catégorie et sous-catégorie des opérations du mois en cours. Il suffira de choisir le ou les comptes et de valider (par-défaut, tous les comptes sont utilisés).");
      break;
 
    case 2:
      /* etat vierge  */

      description = _("Cette option crée un état vierge dans lequel vous devez tous configurer.");
      break;
 
    default:

     description = _("????  ne devrait pas être affiché ...");
     }


  if ( GTK_IS_WIDGET ( GTK_BIN ( frame ) -> child ))
    gtk_container_remove ( GTK_CONTAINER ( frame ),
			   GTK_BIN ( frame ) -> child );

  label = gtk_label_new ( description );
  gtk_label_set_line_wrap ( GTK_LABEL ( label ),
			    TRUE );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      label );
  gtk_widget_show ( label );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
void efface_etat ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  gint resultat;

  if ( !liste_struct_etats )
    return;

  if ( !etat_courant )
    return;

  dialog = gnome_dialog_new ( _("Confirmation de la suppression d'un état"),
			      GNOME_STOCK_BUTTON_YES,
			      GNOME_STOCK_BUTTON_NO,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));

  label = gtk_label_new ( g_strdup_printf ( _("Êtes-vous sûr de vouloir supprimer l'état %s ?"),
					    etat_courant -> nom_etat ));
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( !resultat )
    {
      /*   on met l'état courant à -1 et */
      /* le bouton à null, et le label de l'état en cours à rien */

      liste_struct_etats = g_slist_remove ( liste_struct_etats,
					    etat_courant );

      etat_courant = NULL;
      bouton_etat_courant = NULL;
      gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
			   "" );
      gtk_widget_set_sensitive ( bouton_personnaliser_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_imprimer_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_exporter_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_dupliquer_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_effacer_etat,
				 FALSE );

 
      if ( GTK_BIN ( scrolled_window_etat ) -> child )
	gtk_container_remove ( GTK_CONTAINER ( scrolled_window_etat ),
			       GTK_BIN ( scrolled_window_etat ) -> child );



      /* on réaffiche la liste des états */

      remplissage_liste_etats ();
      modification_fichier ( TRUE );
    }

  if ( GNOME_IS_DIALOG ( dialog ))
    gnome_dialog_close ( GNOME_DIALOG ( dialog ));

}
/*****************************************************************************************************/


/*****************************************************************************************************/
void changement_etat ( GtkWidget *bouton,
		       struct struct_etat *etat )
{
  GtkWidget *icone;

  /* on commence par refermer l'ancien bouton */

  if ( bouton_etat_courant )
    {
      icone = gnome_stock_pixmap_widget ( GTK_WIDGET ( bouton ),
					  GNOME_STOCK_PIXMAP_BOOK_BLUE);
      gtk_container_remove ( GTK_CONTAINER ( bouton_etat_courant ),
			     GTK_BIN ( bouton_etat_courant ) -> child );
      gtk_container_add ( GTK_CONTAINER ( bouton_etat_courant ),
			  icone );
      gtk_widget_show ( icone );
    }

  /* on ouvre le nouveau */

  bouton_etat_courant = bouton;
  etat_courant = etat;
  gtk_widget_set_sensitive ( bouton_personnaliser_etat,
			     TRUE );
/* FIXME: réactiver àca le jour ou on sort l'impression
	mais de toutes faàons, àca sera mergé 
  gtk_widget_set_sensitive ( bouton_imprimer_etat,
			     TRUE );
*/
  gtk_widget_set_sensitive ( bouton_exporter_etat,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_dupliquer_etat,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_effacer_etat,
			     TRUE );

  icone = gnome_stock_pixmap_widget ( GTK_WIDGET ( bouton ),
				      GNOME_STOCK_PIXMAP_BOOK_OPEN);
  gtk_container_remove ( GTK_CONTAINER ( bouton_etat_courant ),
			 GTK_BIN ( bouton_etat_courant ) -> child );
  gtk_container_add ( GTK_CONTAINER ( bouton_etat_courant ),
		      icone );
  gtk_widget_show ( icone );

  /* on met le nom de l'état dans la frame du haut */

  gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
		       etat -> nom_etat );

  /* on affiche l'état */

  rafraichissement_etat ( etat );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint recherche_etat_par_no ( struct struct_etat *etat,
			     gint *no_etat )
{
  return ( etat -> no_etat != GPOINTER_TO_INT (no_etat) );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void exporter_etat ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *fenetre_nom;
  gint resultat;
  struct stat test_fichier;
  gchar *nom_etat;

  dialog = gnome_dialog_new ( _("Exporter un état"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			     0 );
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
		       "destroy" );

  label = gtk_label_new ( _("Entrer un nom pour l'export :") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  fenetre_nom = gnome_file_entry_new ( "nom_fichier",
				       "nom_fichier" );
  gnome_file_entry_set_default_path ( GNOME_FILE_ENTRY ( fenetre_nom ),
				      dernier_chemin_de_travail );
  gtk_widget_set_usize ( gnome_file_entry_gnome_entry ( GNOME_FILE_ENTRY ( fenetre_nom )),
			 300,
			 FALSE );
  gtk_entry_set_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))),
		       g_strconcat ( dernier_chemin_de_travail,
				     etat_courant -> nom_etat,
				     ".egsb",
				     NULL ));
  gtk_entry_set_position ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))),
			   strlen (dernier_chemin_de_travail ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))));
  gtk_window_set_focus ( GTK_WINDOW ( dialog ),
			 gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom )));
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       fenetre_nom,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( fenetre_nom );

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  switch ( resultat )
    {
    case 0 :
      nom_etat = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))))));

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));

      /* vérification que c'est possible */

      if ( !strlen ( nom_etat ))
	return;

      if ( stat ( nom_etat,
		  &test_fichier ) != -1 )
	{
	  if ( S_ISREG ( test_fichier.st_mode ) )
	    {
	      GtkWidget *etes_vous_sur;
	      GtkWidget *label;

	      etes_vous_sur = gnome_dialog_new ( _("Enregistrer le fichier"),
						 GNOME_STOCK_BUTTON_YES,
						 GNOME_STOCK_BUTTON_NO,
						 NULL );
	      label = gtk_label_new ( _("Le fichier existe. Voulez-vous l'écraser ?") );
	      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( etes_vous_sur ) -> vbox ),
				   label,
				   TRUE,
				   TRUE,
				   5 );
	      gtk_widget_show ( label );

	      gnome_dialog_set_default ( GNOME_DIALOG ( etes_vous_sur ),
					 1 );
	      gnome_dialog_set_parent ( GNOME_DIALOG ( etes_vous_sur ),
					GTK_WINDOW ( window ) );
	      gtk_window_set_modal ( GTK_WINDOW ( etes_vous_sur ),
				     TRUE );
	      if ( gnome_dialog_run_and_close ( GNOME_DIALOG ( etes_vous_sur ) ) )
		return;
	    }
	  else
	    {
	      dialogue ( g_strdup_printf ( _("Nom de fichier \"%s\" invalide !"),
					   nom_etat ));
	      return;
	    }
	}

      if ( !enregistre_etat ( nom_etat ))
	{
	  dialogue ( "L'enregistrement a échoué." );
	  return;
	}

      break;

    default :
      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void importer_etat ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *fenetre_nom;
  gint resultat;
  gchar *nom_etat;

  dialog = gnome_dialog_new ( _("Importer un état"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			     0 );
  gtk_signal_connect ( GTK_OBJECT ( dialog ),
		       "destroy",
		       GTK_SIGNAL_FUNC ( gtk_signal_emit_stop_by_name ),
		       "destroy" );

  label = gtk_label_new ( _("Entrer le nom du fichier :") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  fenetre_nom = gnome_file_entry_new ( "nom_fichier",
				       "nom_fichier" );
  gnome_file_entry_set_default_path ( GNOME_FILE_ENTRY ( fenetre_nom ),
				      dernier_chemin_de_travail );
  gtk_widget_set_usize ( gnome_file_entry_gnome_entry ( GNOME_FILE_ENTRY ( fenetre_nom )),
			 300,
			 FALSE );
  gtk_entry_set_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))),
		       g_strconcat ( dernier_chemin_de_travail,
				     ".egsb",
				     NULL ));
  gtk_entry_set_position ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))),
			   strlen (dernier_chemin_de_travail ));
  gnome_dialog_editable_enters ( GNOME_DIALOG ( dialog ),
				 GTK_EDITABLE ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))));
  gtk_window_set_focus ( GTK_WINDOW ( dialog ),
			 gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom )));
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       fenetre_nom,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( fenetre_nom );

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  switch ( resultat )
    {
    case 0 :
      nom_etat = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))))));

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));

      /* vérification que c'est possible */

      if ( !strlen ( nom_etat ))
	return;


      if ( !charge_etat ( nom_etat ))
	{
	  dialogue ( "L'importation a échoué." );
	  return;
	}

      break;

    default :
      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* cette fonction crée une copie de l'état courant */
/*****************************************************************************************************/

void dupliquer_etat ( void )
{
  struct struct_etat *etat;
  GSList *liste_tmp;

  etat = calloc ( 1,
		  sizeof ( struct struct_etat ));


  /* on recopie les données de l'état courant */

  memcpy ( etat,
	   etat_courant,
	   sizeof ( struct struct_etat ));

  /* il reste juste à faire une copie des listes et des chaines pour terminer */

  etat -> no_etat = ++no_dernier_etat;

  etat -> nom_etat = g_strdup ( etat_courant -> nom_etat );
  etat -> no_exercices = g_slist_copy ( etat_courant -> no_exercices );

  if ( etat_courant -> date_perso_debut )
    etat -> date_perso_debut = g_date_new_dmy ( g_date_day ( etat_courant -> date_perso_debut ),
						g_date_month ( etat_courant -> date_perso_debut ),
						g_date_year ( etat_courant -> date_perso_debut ));
						
  if ( etat_courant -> date_perso_fin )
    etat -> date_perso_fin = g_date_new_dmy ( g_date_day ( etat_courant -> date_perso_fin ),
						g_date_month ( etat_courant -> date_perso_fin ),
						g_date_year ( etat_courant -> date_perso_fin ));
						
  etat -> type_classement = g_list_copy ( etat_courant -> type_classement );
  etat -> no_comptes = g_slist_copy ( etat_courant -> no_comptes );
  etat -> no_comptes_virements = g_slist_copy ( etat_courant -> no_comptes_virements );
  etat -> no_categ = g_slist_copy ( etat_courant -> no_categ );
  etat -> no_ib = g_slist_copy ( etat_courant -> no_ib );
  etat -> no_tiers = g_slist_copy ( etat_courant -> no_tiers );


  /* on fait une copie de la liste des textes */

  if ( etat -> liste_struct_comparaison_textes )
    {
      etat -> liste_struct_comparaison_textes = NULL;

      liste_tmp = etat_courant -> liste_struct_comparaison_textes;

      while ( liste_tmp )
	{
	  struct struct_comparaison_textes_etat *ancien_comp_textes;
	  struct struct_comparaison_textes_etat *comp_textes;

	  ancien_comp_textes = liste_tmp -> data;

	  comp_textes = calloc ( 1,
				   sizeof ( struct struct_comparaison_textes_etat ));

	  memcpy ( comp_textes,
		   ancien_comp_textes,
		   sizeof ( struct struct_comparaison_textes_etat ));

	  comp_textes -> texte = g_strdup ( comp_textes -> texte );

	  etat -> liste_struct_comparaison_textes = g_slist_append ( etat -> liste_struct_comparaison_textes,
								     comp_textes );
	  liste_tmp = liste_tmp -> next;
	}
    }

  /* on fait une copie de la liste des montants */

  if ( etat -> liste_struct_comparaison_montants )
    {
      etat -> liste_struct_comparaison_montants = NULL;

      liste_tmp = etat_courant -> liste_struct_comparaison_montants;

      while ( liste_tmp )
	{
	  struct struct_comparaison_montants_etat *ancien_comp_montants;
	  struct struct_comparaison_montants_etat *comp_montants;

	  ancien_comp_montants = liste_tmp -> data;

	  comp_montants = calloc ( 1,
				   sizeof ( struct struct_comparaison_montants_etat ));

	  memcpy ( comp_montants,
		   ancien_comp_montants,
		   sizeof ( struct struct_comparaison_montants_etat ));

	  etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
								       comp_montants );
	  liste_tmp = liste_tmp -> next;
	}
    }



  /* on l'ajoute à la liste */

  liste_struct_etats = g_slist_append ( liste_struct_etats,
					etat );

  /* on réaffiche la liste des états */

  etat_courant = etat;

  remplissage_liste_etats ();

  gtk_widget_set_sensitive ( bouton_personnaliser_etat,
			     TRUE );
/* FIXME: réactiver àca le jour ou on sort l'impression
	mais de toutes faàons, àca sera mergé 
  gtk_widget_set_sensitive ( bouton_imprimer_etat,
			     TRUE );
*/
  gtk_widget_set_sensitive ( bouton_exporter_etat,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_dupliquer_etat,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_effacer_etat,
			     TRUE );

  gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
		       etat_courant -> nom_etat );

  gtk_widget_set_sensitive ( bouton_effacer_etat,
			     TRUE );

  personnalisation_etat ();
  modification_fichier ( TRUE );
}
/*****************************************************************************************************/
