/*  Fichier qui s'occupe des états */
/*      etats.c */

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

gchar *liste_plages_dates[] = {
  "Personnalisé",
  "Toutes",
  "Cumul à ce jour",
  "Mois en cours",
  "Année en cours",
  "Cumul mensuel",
  "Cumul annuel",
  "Mois précédent",
  "Année précédente",
  "30 derniers jours",
  "3 derniers mois",
  "6 derniers mois",
  "12 derniers mois",
  "30 prochains jours",
  "3 prochains mois",
  "6 prochains mois",
  "12 prochains mois",
  NULL };

gchar *liste_type_classement[] = {
  "Catégories -> I.B. -> Compte",
  "Catégories -> Compte -> I.B.",
  "I.B. -> Catégories -> Compte",
  "I.B. -> Compte -> Catégories",
  "Compte -> I.B. -> Catégories",
  "Compte -> Catégories -> I.B.",
  NULL };

gchar *jours_semaine[] = {
  "Lundi",
  "Mardi",
  "Mercredi",
  "Jeudi",
  "Vendredi",
  "Samedi",
  "Dimanche",
  NULL };



/*****************************************************************************************************/
GtkWidget *creation_onglet_etats ( void )
{
  GtkWidget *onglet;
  GtkWidget *frame;
  GtkWidget *vbox;


  onglet = gtk_hbox_new ( FALSE,
			     10 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


/*   création de la fenetre des noms des états */
/* on reprend le principe des comptes dans la fenetre des opés */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			       GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (frame);

  /* on y met les rapports et les boutons */

  gtk_container_add ( GTK_CONTAINER ( frame ),
		      creation_liste_etats ());


  /* création de la partie droite */

  vbox = gtk_vbox_new ( FALSE,
			10 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
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


  /*   au départ, aucun état n'est ouvert */

  bouton_etat_courant = NULL;
  etat_courant = NULL;

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
				   GTK_POLICY_NEVER,
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

  bouton = gtk_button_new_with_label ( "Ajouter un état ..." );
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


  /* mise en place du bouton équilibrage */

  bouton_effacer_etat = gtk_button_new_with_label ( "Effacer un état ..." );
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

  if ( !liste_struct_etats )
    gtk_widget_set_sensitive ( bouton_effacer_etat,
			       FALSE );


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


  widget_retour = gtk_hbox_new ( FALSE,
				 5 );
  gtk_widget_show ( widget_retour );

  /* on met le bouton personnaliser */

  bouton_personnaliser_etat = gtk_button_new_with_label ( "Personnaliser ..." );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_personnaliser_etat ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_personnaliser_etat,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_personnaliser_etat ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( personnalisation_etat ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_personnaliser_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_personnaliser_etat );



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
	icone = gnome_stock_pixmap_widget ( GTK_WIDGET ( bouton ),
					    GNOME_STOCK_PIXMAP_BOOK_OPEN);
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
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void ajout_etat ( void )
{
  struct struct_etat *etat;

  /* on ajoute un état vierge appelé nouvel état */
  /*   pour modifier le nom, il faudra aller dans la */
  /* personnalisation */

  etat = calloc ( 1,
		  sizeof ( struct struct_etat ));

  etat -> no_etat = ++no_dernier_etat;
  etat -> nom_etat = g_strdup ( "Nouvel état" );

  /*   tout le reste est à NULL, ce qui est très bien */
  /* on l'ajoute à la liste */

  liste_struct_etats = g_slist_append ( liste_struct_etats,
					etat );

  /* on réaffiche la liste des états */

  remplissage_liste_etats ();

  gtk_widget_set_sensitive ( bouton_effacer_etat,
			     TRUE );

  modification_fichier ( TRUE );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void efface_etat ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *option_menu;
  gint resultat;
  GSList *liste_tmp;
  GtkWidget *menu;
  struct struct_etat *etat;

  if ( !liste_struct_etats )
    return;

  dialog = gnome_dialog_new ( "Sélection de l'état à effacer :",
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));

  label = gtk_label_new ( "Choisir l'état à effacer :" );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  option_menu = gtk_option_menu_new ();
  menu = gtk_menu_new ();

  liste_tmp = liste_struct_etats;

  while ( liste_tmp )
    {
      GtkWidget *menu_item;

      etat = liste_tmp -> data;

      menu_item = gtk_menu_item_new_with_label ( etat -> nom_etat );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			   "adr_etat",
			   etat );
      gtk_menu_append ( GTK_MENU ( menu ),
			menu_item );
      gtk_widget_show ( menu_item );

      liste_tmp = liste_tmp -> next;
    }

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( option_menu ),
			     menu );
  gtk_widget_show ( menu );

  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       option_menu,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( option_menu );

  if ( etat_courant )
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu ),
				  g_slist_position ( liste_struct_etats,
						     g_slist_find_custom ( liste_struct_etats,
									   GINT_TO_POINTER ( etat_courant -> no_etat ),
									   (GCompareFunc) recherche_etat_par_no )));

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( !resultat )
    {
     
      etat = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
				   "adr_etat" );

      /*   si l'état courant était celui qu'on efface, on met l'état courant à -1 et */
      /* le bouton à null, et le label de l'état en cours à rien */

      if ( etat_courant -> no_etat == etat -> no_etat )
	{
	  etat_courant = NULL;
	  bouton_etat_courant = NULL;
	  gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
			       "" );
	  gtk_widget_set_sensitive ( bouton_personnaliser_etat,
				     FALSE );

	  /* il faut aussi ici virer l'état affiché quand ce sera fait */

	}

      liste_struct_etats = g_slist_remove ( liste_struct_etats,
					    etat );


      /* on réaffiche la liste des états */

      remplissage_liste_etats ();

      /*   s'il ne reste plus d'état, on grise le bouton */

      if ( !liste_struct_etats )
	gtk_widget_set_sensitive ( bouton_effacer_etat,
				   FALSE );
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

  affichage_etat ( etat );
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
/* Fontion personnalistation_etat */
/* affiche la fenetre de personnalisation */
/*****************************************************************************************************/

void personnalisation_etat (void)
{
  GtkWidget *dialog;
  gint resultat;
  GtkWidget *notebook;
  gchar *pointeur_char;
  GList *pointeur_liste;

  if ( !etat_courant )
    return;

  /* la fenetre affichée est une gnome dialog */

  dialog = gnome_dialog_new ( g_strconcat ( "Personnalisation de l'état : ",
					    etat_courant -> nom_etat,
					    NULL ),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));
  gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
			     0 );

  notebook = gtk_notebook_new ();
  
  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_generalites (etat_courant),
			     gtk_label_new (" Généralités ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_dates (etat_courant),
			     gtk_label_new (" Dates ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_comptes (etat_courant),
			     gtk_label_new (" Comptes ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_categories (etat_courant),
			     gtk_label_new (" Catégories ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_ib (etat_courant),
			     gtk_label_new (" Imputation budgétaire ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_tiers (etat_courant),
			     gtk_label_new (" Tiers ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_texte (etat_courant),
			     gtk_label_new (" Texte ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_montant (etat_courant),
			     gtk_label_new (" Montant ") );

  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       notebook,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( notebook );

 retour_etat:

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    {
      if ( GNOME_IS_DIALOG ( dialog ))
	gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }

  /* vérification que les dates init et finales sont correctes */

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_init_etat ))))
       &&
       !modifie_date ( entree_date_init_etat ))
    {
      dialogue ( "La date initiale personnelle est invalide" );
      goto retour_etat;
    }

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_finale_etat ))))
       &&
       !modifie_date ( entree_date_finale_etat ))
    {
      dialogue ( "La date finale personnelle est invalide" );
      goto retour_etat;
    }


  /* on récupère maintenant toutes les données */
  /* récupération du nom du rapport */

  pointeur_char = g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_nom_etat )));

  if ( strlen ( pointeur_char )
       &&
       strcmp ( pointeur_char,
		etat_courant -> nom_etat ))
    {
      etat_courant -> nom_etat = g_strdup ( pointeur_char );

      /* on réaffiche la liste des états */

      remplissage_liste_etats ();
    }

  /* récupération de l'affichage des opés */

  etat_courant -> afficher_opes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ));

  /* récupération du type de classement */

  etat_courant -> type_classement = GPOINTER_TO_INT ( GTK_CLIST ( liste_type_classement_etat ) -> selection -> data);

  /* récupération des dates */

  etat_courant -> exo_date = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ));
  etat_courant -> utilise_detail_exo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat ));
  etat_courant -> afficher_date_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ));
  etat_courant -> afficher_tiers_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ));
  etat_courant -> afficher_categ_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ));
  etat_courant -> afficher_sous_categ_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ));
  etat_courant -> afficher_ib_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ));
  etat_courant -> afficher_sous_ib_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ));
  etat_courant -> afficher_notes_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ));
  etat_courant -> afficher_pc_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ));
  etat_courant -> afficher_infobd_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ));

  if ( etat_courant -> no_exercices )
    {
      g_slist_free ( etat_courant -> no_exercices );
      etat_courant -> no_exercices = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_exo_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat_courant -> no_exercices = g_slist_append ( etat_courant -> no_exercices,
					      gtk_clist_get_row_data ( GTK_CLIST ( liste_exo_etat ),
								       GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  /*   si tous les exos ont été sélectionnés, on met bouton_detaille_exo_etat à 0 (plus rapide) */

  if ( ( g_list_length ( GTK_CLIST ( liste_exo_etat ) -> selection )
	 ==
	 GTK_CLIST ( liste_exo_etat ) -> rows )
       &&
       etat_courant -> utilise_detail_exo )
    {
      dialogue ( "Tous les exercices ont été sélectionnés ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les exercices utilisés\"" );
      etat_courant -> utilise_detail_exo = FALSE;
    }
  

  etat_courant -> separation_par_exo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ));

  etat_courant -> no_plage_date = GPOINTER_TO_INT ( GTK_CLIST ( liste_plages_dates_etat ) -> selection -> data );

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_init_etat ))))
       &&
       modifie_date ( entree_date_init_etat ))
    {
      gint jour, mois, annee;

      sscanf ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_init_etat ))),
	       "%d/%d/%d",
	       &jour,
	       &mois,
	       &annee );

      etat_courant -> date_perso_debut = g_date_new_dmy ( jour,
						  mois,
						  annee );
    }

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_finale_etat ))))
       &&
       modifie_date ( entree_date_finale_etat ))
    {
      gint jour, mois, annee;

      sscanf ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_finale_etat ))),
	       "%d/%d/%d",
	       &jour,
	       &mois,
	       &annee );

      etat_courant -> date_perso_fin = g_date_new_dmy ( jour,
						mois,
						annee );
    }

  etat_courant -> separation_par_plage = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat ));
  etat_courant -> type_separation_plage = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ) -> menu_item ),
									  "type" ));
  etat_courant -> jour_debut_semaine = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_debut_semaine ) -> menu_item ),
									  "jour" ));
  etat_courant -> type_separation_perso = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_type_separe_perso_etat ) -> menu_item ),
									  "type" ));
  etat_courant -> delai_separation_perso = atoi ( gtk_entry_get_text ( GTK_ENTRY ( entree_separe_perso_etat )));

  /* récupération des comptes */

  etat_courant -> utilise_detail_comptes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat ));

  if ( etat_courant -> no_comptes )
    {
      g_slist_free ( etat_courant -> no_comptes );
      etat_courant -> no_comptes = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_comptes_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat_courant -> no_comptes = g_slist_append ( etat_courant -> no_comptes,
					      gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_etat ),
								       GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  /*   si tous les comptes ont été sélectionnés, on met utilise_detail_comptes à 0 (plus rapide) */

  if ( ( g_list_length ( GTK_CLIST ( liste_comptes_etat ) -> selection )
	 ==
	 GTK_CLIST ( liste_comptes_etat ) -> rows )
       &&
       etat_courant -> utilise_detail_comptes )
    {
      dialogue ( "Tous les comptes ont été sélectionnés ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les comptes utilisés\"" );
      etat_courant -> utilise_detail_comptes = FALSE;
    }
  
  etat_courant -> regroupe_ope_par_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat ));
  etat_courant -> affiche_sous_total_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte ));

  /*   récupération des catégories */

  etat_courant -> utilise_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_categ_etat ));

  etat_courant -> utilise_detail_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat ));

  if ( etat_courant -> no_categ )
    {
      g_slist_free ( etat_courant -> no_categ );
      etat_courant -> no_categ = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_categ_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat_courant -> no_categ = g_slist_append ( etat_courant -> no_categ,
					  gtk_clist_get_row_data ( GTK_CLIST ( liste_categ_etat ),
								   GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  /*   si tous les categ ont été sélectionnés, on met utilise_detail_categ à 0 (plus rapide) */

  if ( ( g_list_length ( GTK_CLIST ( liste_categ_etat ) -> selection )
	 ==
	 GTK_CLIST ( liste_categ_etat ) -> rows )
       &&
       etat_courant -> utilise_detail_categ )
    {
      dialogue ( "Toutes les catégories ont été sélectionnées ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les catégories utilisées\"" );
      etat_courant -> utilise_detail_categ = FALSE;
    }
  
  etat_courant -> affiche_sous_total_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ ));
  etat_courant -> afficher_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ ));
  etat_courant -> affiche_sous_total_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ ));

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat )))
    etat_courant -> type_virement = 1;
  else
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat )))
      etat_courant -> type_virement = 2;
    else
      etat_courant -> type_virement = 0;


  /*   récupération des ib */

  etat_courant -> utilise_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat ));

  etat_courant -> utilise_detail_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ));

  if ( etat_courant -> no_ib )
    {
      g_slist_free ( etat_courant -> no_ib );
      etat_courant -> no_ib = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_ib_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat_courant -> no_ib = g_slist_append ( etat_courant -> no_ib,
				       gtk_clist_get_row_data ( GTK_CLIST ( liste_ib_etat ),
								GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  etat_courant -> afficher_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib ));

  /*   si toutes les ib ont été sélectionnés, on met utilise_detail_ib à 0 (plus rapide) */

  if ( ( g_list_length ( GTK_CLIST ( liste_ib_etat ) -> selection )
	 ==
	 GTK_CLIST ( liste_ib_etat ) -> rows )
       &&
       etat_courant -> utilise_detail_ib )
    {
      dialogue ( "Toutes les imputations budgétaires ont été sélectionnées ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les imputations budgétaires utilisées\"" );
      etat_courant -> utilise_detail_ib = FALSE;
    }
  
  etat_courant -> affiche_sous_total_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ));
  etat_courant -> affiche_sous_total_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ));


  /*   récupération des tiers */

  etat_courant -> utilise_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat ));

  etat_courant -> utilise_detail_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat ));

  if ( etat_courant -> no_tiers )
    {
      g_slist_free ( etat_courant -> no_tiers );
      etat_courant -> no_tiers = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_tiers_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat_courant -> no_tiers = g_slist_append ( etat_courant -> no_tiers,
					  gtk_clist_get_row_data ( GTK_CLIST ( liste_tiers_etat ),
								   GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  /*   si tous les tiers ont été sélectionnés, on met utilise_detail_tiers à 0 (plus rapide) */

  if ( ( g_list_length ( GTK_CLIST ( liste_tiers_etat ) -> selection )
	 ==
	 GTK_CLIST ( liste_tiers_etat ) -> rows )
       &&
       etat_courant -> utilise_detail_tiers )
    {
      dialogue ( "Tous les tiers ont été sélectionnés ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les tiers utilisés\"" );
      etat_courant -> utilise_detail_tiers = FALSE;
    }
  
  etat_courant -> affiche_sous_total_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers ));


  /* récupération du texte */

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_texte_etat )))))
    etat_courant -> texte = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_texte_etat ))));
  else
    etat_courant -> texte = NULL;

  /* récupération du montant */

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_montant_etat )))))
    etat_courant -> montant = g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_montant_etat ))),
				 NULL );
  else
    etat_courant -> montant = 0;


  gnome_dialog_close ( GNOME_DIALOG ( dialog ));
  modification_fichier ( TRUE );

  /* on réaffiche l'état */

  affichage_etat ( etat_courant );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_generalites ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *scrolled_window;
  gchar **classement;
  gint i;
  GtkWidget *frame;
  GtkWidget *table;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  /* choix du nom du rapport */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Nom de l'état : " );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  entree_nom_etat = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_nom_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_nom_etat );

  /* choix du type de classement */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Choix du type de classement :" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_set_usize ( scrolled_window,
			 200,
			 150 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       scrolled_window,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( scrolled_window );


  liste_type_classement_etat = gtk_clist_new ( 1 );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_type_classement_etat ),
				     0,
				     TRUE );
  gtk_signal_connect ( GTK_OBJECT ( liste_type_classement_etat ),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( click_liste_etat ),
		       GINT_TO_POINTER (1) );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_type_classement_etat );
  gtk_widget_show ( liste_type_classement_etat );

  /* on remplit maintenant la liste */

  classement = liste_type_classement;

  i = 0;

  while ( classement[i] )
    {
      gint ligne;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_type_classement_etat ),
				 &classement[i] );
      i++;
    }


  bouton_afficher_opes = gtk_check_button_new_with_label ( "Afficher les opérations" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_afficher_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_opes );

  /* demande les détails affichés dans les opérations */

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  frame = gtk_frame_new ( "Afficher les informations sur" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );


  /* connection pour rendre sensitif la frame */

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_opes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       frame );


  table = gtk_table_new ( 5,
			  2,
			  FALSE );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      table );
  gtk_widget_show ( table );


  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 1,
			      0, 1 );
  gtk_widget_show ( hbox );

  bouton_afficher_date_opes = gtk_check_button_new_with_label ( "la date" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_date_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_date_opes );

  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      1, 2,
			      0, 1 );
  gtk_widget_show ( hbox );

  bouton_afficher_tiers_opes = gtk_check_button_new_with_label ( "le tiers" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_tiers_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_tiers_opes );


  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 1,
			      1, 2 );
  gtk_widget_show ( hbox );

  bouton_afficher_categ_opes = gtk_check_button_new_with_label ( "la catégorie" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_categ_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_categ_opes );

  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      1, 2,
			      1, 2 );
  gtk_widget_show ( hbox );

  bouton_afficher_sous_categ_opes = gtk_check_button_new_with_label ( "la sous-catégorie" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_sous_categ_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_categ_opes );

  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 1,
			      2, 3 );
  gtk_widget_show ( hbox );

  bouton_afficher_ib_opes = gtk_check_button_new_with_label ( "l'imputation budgétaire" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_ib_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_ib_opes );

  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      1, 2,
			      2, 3 );
  gtk_widget_show ( hbox );

  bouton_afficher_sous_ib_opes = gtk_check_button_new_with_label ( "la sous-imputation budgétaire" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_sous_ib_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_ib_opes );

  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 1,
			      3, 4 );
  gtk_widget_show ( hbox );

  bouton_afficher_notes_opes = gtk_check_button_new_with_label ( "les notes" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_notes_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_notes_opes );

  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      1, 2,
			      3, 4 );
  gtk_widget_show ( hbox );

  bouton_afficher_pc_opes = gtk_check_button_new_with_label ( "la pièce comptable" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_pc_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_pc_opes );

  hbox = gtk_hbox_new ( FALSE,
			 0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 1,
			      4, 5 );
  gtk_widget_show ( hbox );

  bouton_afficher_infobd_opes = gtk_check_button_new_with_label ( "l'information banque-guichet" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_infobd_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_infobd_opes );

  /* on met les connections */

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_categ_opes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_afficher_sous_categ_opes );
  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_ib_opes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_afficher_sous_ib_opes );


  /* on met le nom de l'état */

  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_etat ),
		       etat -> nom_etat );

  /* on sélectionne la plage de date */

  gtk_clist_select_row ( GTK_CLIST ( liste_type_classement_etat ),
			 etat -> type_classement,
			 0 );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ),
				 etat -> afficher_opes );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ),
				 etat -> afficher_date_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ),
				 etat -> afficher_tiers_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ),
				 etat -> afficher_categ_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ),
				 etat -> afficher_sous_categ_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ),
				 etat -> afficher_ib_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ),
				 etat -> afficher_sous_ib_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ),
				 etat -> afficher_notes_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ),
				 etat -> afficher_pc_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ),
				 etat -> afficher_infobd_ope );

  /* on rend insensitif les sous qque choses si nécessaire */

  sens_desensitive_pointeur ( bouton_afficher_opes,
			      frame );
  sens_desensitive_pointeur ( bouton_afficher_categ_opes,
			      bouton_afficher_sous_categ_opes );
  sens_desensitive_pointeur ( bouton_afficher_ib_opes,
			      bouton_afficher_sous_ib_opes );

  return ( widget_retour );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void sens_desensitive_pointeur ( GtkWidget *bouton,
				 GtkWidget *widget )
{
  gtk_widget_set_sensitive ( widget,
			     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton )));

}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_dates ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;
  GtkWidget *separateur;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  GSList *pointeur_liste;
  gchar **plages_dates;
  gint i;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *menu;
  GtkWidget *menu_item;

  widget_retour = gtk_hbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );


  /*   on met en forme la partie de gauche : utilisation des exercices */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  radio_button_utilise_exo = gtk_radio_button_new_with_label ( NULL,
							       "Utiliser les exercices" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       radio_button_utilise_exo,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( radio_button_utilise_exo );

  /* on met la liste des exos sous ce radio button */

  frame = gtk_frame_new ( FALSE );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( frame );


  /* on met la connection pour rendre sensitif cette frame */

  gtk_signal_connect ( GTK_OBJECT (radio_button_utilise_exo ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       frame );

  vbox_utilisation_exo = gtk_vbox_new ( FALSE,
					5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_utilisation_exo ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox_utilisation_exo );
  gtk_widget_show ( vbox_utilisation_exo );

  bouton_detaille_exo_etat = gtk_check_button_new_with_label ( "Détailler les exercices utilisés" );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
		       bouton_detaille_exo_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_detaille_exo_etat );

  vbox_generale_exo_etat = gtk_vbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
		       vbox_generale_exo_etat,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox_generale_exo_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_exo_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       vbox_generale_exo_etat );

  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_exo_etat ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );

  liste_exo_etat = gtk_clist_new ( 1 );
  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_exo_etat ),
				 GTK_SELECTION_MULTIPLE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_exo_etat ),
				     0,
				     TRUE );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_exo_etat );
  gtk_widget_show ( liste_exo_etat );

  /* on remplit la liste des exercices */

  pointeur_liste = liste_struct_exercices;

  while ( pointeur_liste )
    {
      struct struct_exercice *exercice;
      gchar *nom[1];
      gint ligne;

      exercice = pointeur_liste -> data;

      nom[0] = exercice -> nom_exercice;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_exo_etat ),
				 nom );

      gtk_clist_set_row_data ( GTK_CLIST ( liste_exo_etat ),
			       ligne,
			       GINT_TO_POINTER ( exercice -> no_exercice ));

      pointeur_liste = pointeur_liste -> next;
    }

  bouton_separe_exo_etat = gtk_check_button_new_with_label ( "Séparer les résultats par exercice" );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
		       bouton_separe_exo_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_separe_exo_etat );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  /* mise en place de la plage de dates */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  radio_button_utilise_dates = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( radio_button_utilise_exo )),
					     "Utiliser des plages de dates" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       radio_button_utilise_dates,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( radio_button_utilise_dates );

  /* on met en dessous une liste avec les plages de date proposées */

  frame = gtk_frame_new (NULL);
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( frame );

  vbox_utilisation_date = gtk_vbox_new ( FALSE,
					5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_utilisation_date ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox_utilisation_date );
  gtk_widget_show ( vbox_utilisation_date );

  /* on met la connection pour rendre sensitif cette frame */

  gtk_signal_connect ( GTK_OBJECT ( radio_button_utilise_dates ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       vbox_utilisation_date );

  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );


  liste_plages_dates_etat = gtk_clist_new ( 1 );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_plages_dates_etat ),
				     0,
				     TRUE );
  gtk_signal_connect ( GTK_OBJECT ( liste_plages_dates_etat ),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( click_liste_etat ),
		       NULL );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_plages_dates_etat );
  gtk_widget_show ( liste_plages_dates_etat );

  /* on remplit la liste des exercices */

  plages_dates = liste_plages_dates;

  i = 0;

  while ( plages_dates[i] )
    {
      gint ligne;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_plages_dates_etat ),
				 &plages_dates[i] );
      i++;
    }

  /* on met ensuite la date perso de début */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Date initiale : " );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  entree_date_init_etat = gtk_entry_new_with_max_length ( 11 );
  gtk_widget_set_usize ( entree_date_init_etat,
			 100,
			 FALSE );
  gtk_signal_connect ( GTK_OBJECT ( entree_date_init_etat ),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( clique_sur_entree_date_etat ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_date_init_etat ),
			      "focus_out_event",
			      GTK_SIGNAL_FUNC ( modifie_date ),
			      GTK_OBJECT ( entree_date_init_etat ));
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_date_init_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_date_init_etat );

  /* on met ensuite la date perso de fin */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Date finale : " );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  entree_date_finale_etat = gtk_entry_new_with_max_length ( 11 );
  gtk_widget_set_usize ( entree_date_finale_etat,
			 100,
			 FALSE );
  gtk_signal_connect ( GTK_OBJECT ( entree_date_finale_etat ),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( clique_sur_entree_date_etat ),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_date_finale_etat ),
			      "focus_out_event",
			      GTK_SIGNAL_FUNC ( modifie_date ),
			      GTK_OBJECT ( entree_date_finale_etat ));
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_date_finale_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_date_finale_etat );

  
  /* on permet ensuite la séparation des résultats */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  bouton_separe_plages_etat = gtk_check_button_new_with_label ( "Séparer les résultats par période" );
  gtk_signal_connect ( GTK_OBJECT ( bouton_separe_plages_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( change_separation_result_periode ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
		       bouton_separe_plages_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_separe_plages_etat );

  /* mise en place de la ligne type - choix perso */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

 

  bouton_type_separe_plages_etat = gtk_option_menu_new ();

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( "Semaine" );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"type",
			NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modif_type_separation_dates ),
			      NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( "Mois" );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"type",
			GINT_TO_POINTER (1) );
  gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modif_type_separation_dates ),
			      GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( "Année" );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"type",
			GINT_TO_POINTER (2));
  gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modif_type_separation_dates ),
			      GINT_TO_POINTER (2));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( "Personnalisé" );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"type",
			GINT_TO_POINTER (3));
  gtk_signal_connect_object ( GTK_OBJECT ( menu_item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modif_type_separation_dates ),
			      GINT_TO_POINTER (3));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ),
			     menu );
  gtk_widget_show ( menu );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_type_separe_plages_etat,
		       FALSE, 
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_type_separe_plages_etat );

  entree_separe_perso_etat = gtk_entry_new ();
  gtk_widget_set_usize ( entree_separe_perso_etat,
			 50,
			 FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_separe_perso_etat,
		       FALSE, 
		       FALSE,
		       0 );
  gtk_widget_show ( entree_separe_perso_etat );


  bouton_type_separe_perso_etat = gtk_option_menu_new ();

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( "Jours" );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"type",
			GINT_TO_POINTER (0));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( "Mois" );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"type",
			GINT_TO_POINTER (1));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( "Ans" );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"type",
			GINT_TO_POINTER (2));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_type_separe_perso_etat ),
			     menu );
  gtk_widget_show ( menu );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_type_separe_perso_etat,
		       FALSE, 
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_type_separe_perso_etat );

  /* mise en place de la ligne de début de semaine */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_date ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "La semaine commence le " );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE, 
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  bouton_debut_semaine = gtk_option_menu_new ();

  menu = gtk_menu_new ();

  i = 0;

  while ( jours_semaine[i] )
    {
      menu_item = gtk_menu_item_new_with_label ( jours_semaine[i] );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "jour",
			    GINT_TO_POINTER (i));
      gtk_menu_append ( GTK_MENU ( menu ),
			menu_item );
      gtk_widget_show ( menu_item );
      i++;
    }

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_debut_semaine ),
			     menu );
  gtk_widget_show ( menu );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_debut_semaine,
		       FALSE, 
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_debut_semaine );



  /* on remplit maintenant en fonction des données */

  if ( etat -> exo_date )
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ),
				     TRUE );
      gtk_widget_set_sensitive ( vbox_utilisation_date,
				 FALSE );
    }
  else
   gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_dates ),
				  TRUE );


  if ( etat -> utilise_detail_exo )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat ),
				   TRUE );
  else
    gtk_widget_set_sensitive ( vbox_generale_exo_etat,
			       FALSE );

  /* on sélectionne les exercices */

  pointeur_liste = etat -> no_exercices;

  while ( pointeur_liste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_exo_etat ),
			     g_slist_position ( liste_struct_exercices,
						g_slist_find_custom ( liste_struct_exercices,
								      pointeur_liste -> data,
								      (GCompareFunc) recherche_exercice_par_no )),
			     0 );
      pointeur_liste = pointeur_liste -> next;
    }



  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ),
				 etat -> separation_par_exo );

  /* on sélectionne la plage de date */

  gtk_clist_select_row ( GTK_CLIST ( liste_plages_dates_etat ),
			 etat -> no_plage_date,
			 0 );

  if ( etat -> no_plage_date )
    {
      gtk_widget_set_sensitive ( entree_date_init_etat,
				 FALSE );
      gtk_widget_set_sensitive ( entree_date_finale_etat,
				 FALSE );
    }
  else
    {
      gtk_widget_set_sensitive ( entree_date_init_etat,
				 TRUE );
      gtk_widget_set_sensitive ( entree_date_finale_etat,
				 TRUE );
    }

  /* on remplit les dates perso si elles existent */

  if ( etat -> date_perso_debut )
    gtk_entry_set_text ( GTK_ENTRY ( entree_date_init_etat ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( etat -> date_perso_debut ),
					   g_date_month ( etat -> date_perso_debut ),
					   g_date_year ( etat -> date_perso_debut )));

  if ( etat -> date_perso_fin )
    gtk_entry_set_text ( GTK_ENTRY ( entree_date_finale_etat ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( etat -> date_perso_fin ),
					   g_date_month ( etat -> date_perso_fin ),
					   g_date_year ( etat -> date_perso_fin )));

  /* on remplit les détails de la séparation des dates */

  if ( etat -> separation_par_plage )
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat ),
				     TRUE );
      gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				 TRUE );
      modif_type_separation_dates ( GINT_TO_POINTER ( etat -> type_separation_plage ));
    }
  else
    {
      gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_type_separe_perso_etat,
				 FALSE );
      gtk_widget_set_sensitive ( entree_separe_perso_etat,
				 FALSE );
    }

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ),
				etat -> type_separation_plage );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_debut_semaine ),
				etat -> jour_debut_semaine );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_type_separe_perso_etat ),
				etat -> type_separation_perso );

  if ( etat -> delai_separation_perso )
    gtk_entry_set_text ( GTK_ENTRY ( entree_separe_perso_etat ),
			 itoa ( etat -> delai_separation_perso ));


  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* vérifie où l'on clique et empèche la désélection de ligne */
/*****************************************************************************************************/

void click_liste_etat ( GtkCList *liste,
			GdkEventButton *evenement,
			gint origine )
{
  gint colonne, x, y;
  gint ligne;

  /*   origine = 0 si ça vient des dates, dans ce cas on sensitive les entrées init et fin */
  /* origine = 1 si ça vient du choix de type de classement */


  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste ),
				 "button_press_event");

  /* Récupération des coordonnées de la souris */

  gdk_window_get_pointer ( GTK_CLIST ( liste ) -> clist_window,
			   &x,
			   &y,
			   FALSE );

  gtk_clist_get_selection_info ( GTK_CLIST ( liste ),
				 x,
				 y,
				 &ligne,
				 &colonne);

  if ( GTK_CLIST ( liste ) -> selection
       &&
       GPOINTER_TO_INT ( GTK_CLIST ( liste ) -> selection -> data ) == ligne )
    return;

  gtk_clist_select_row ( GTK_CLIST ( liste ),
			 ligne,
			 0 );

  if ( !origine )
    {
      if ( ligne )
	{
	  gtk_widget_set_sensitive ( entree_date_init_etat,
				     FALSE );
	  gtk_widget_set_sensitive ( entree_date_finale_etat,
				     FALSE );
	}
      else
	{
	  gtk_widget_set_sensitive ( entree_date_init_etat,
				     TRUE );
	  gtk_widget_set_sensitive ( entree_date_finale_etat,
				     TRUE );
	}
    }

}
/*****************************************************************************************************/



/*****************************************************************************************************/
void clique_sur_entree_date_etat ( GtkWidget *entree,
				   GdkEventButton *ev )
{

  if ( ev->type == GDK_2BUTTON_PRESS )
    {
      GtkWidget *popup;
      GtkWidget *popup_boxv;
      GtkRequisition *taille_entree;
      gint x, y;
      GtkWidget *calendrier;
      int cal_jour, cal_mois, cal_annee;
      GtkWidget *bouton;
      GtkWidget *frame;

      /* cherche la position où l'on va mettre la popup */

      taille_entree = malloc ( sizeof ( GtkRequisition ));

      gdk_window_get_origin ( GTK_WIDGET ( entree ) -> window,
			      &x,
			      &y );
      gtk_widget_size_request ( GTK_WIDGET ( entree ),
				taille_entree );
  
      y = y + taille_entree->height;


      /* création de la popup */

      popup = gtk_window_new ( GTK_WINDOW_POPUP );
      gtk_window_set_modal ( GTK_WINDOW (popup),
			     TRUE);
      gtk_widget_set_uposition ( GTK_WIDGET ( popup ),
				 x,
				 y );

      /* on associe l'entrée qui sera remplie à la popup */

      gtk_object_set_data ( GTK_OBJECT ( popup ),
			    "entree",
			    entree );

      /* création de l'intérieur de la popup */

      frame = gtk_frame_new ( NULL );
      gtk_container_add ( GTK_CONTAINER (popup),
			  frame);
      gtk_widget_show ( frame );

      popup_boxv = gtk_vbox_new ( FALSE,
				  5 );
      gtk_container_set_border_width ( GTK_CONTAINER ( popup_boxv ),
				       5 );

      gtk_container_add ( GTK_CONTAINER ( frame ),
			  popup_boxv);
      gtk_widget_show ( popup_boxv );

      if ( !( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree ))))
	      &&
	      sscanf ( gtk_entry_get_text ( GTK_ENTRY ( entree )),
		       "%d/%d/%d",
		       &cal_jour,
		       &cal_mois,
		       &cal_annee)))
	sscanf ( date_jour(),
		 "%d/%d/%d",
		 &cal_jour,
		 &cal_mois,
		 &cal_annee);
      
      calendrier = gtk_calendar_new();
      gtk_calendar_select_month ( GTK_CALENDAR ( calendrier ),
				  cal_mois-1,
				  cal_annee);
      gtk_calendar_select_day  ( GTK_CALENDAR ( calendrier ),
				 cal_jour);

      gtk_calendar_display_options ( GTK_CALENDAR ( calendrier ),
				     GTK_CALENDAR_SHOW_HEADING |
				     GTK_CALENDAR_SHOW_DAY_NAMES |
				     GTK_CALENDAR_WEEK_START_MONDAY );

      gtk_signal_connect ( GTK_OBJECT ( calendrier),
			   "day_selected_double_click",
			   GTK_SIGNAL_FUNC ( date_selectionnee_etat ),
			   popup );
      gtk_signal_connect ( GTK_OBJECT ( popup ),
			   "key_press_event",
			   GTK_SIGNAL_FUNC ( touche_calendrier ),
			   NULL );
      gtk_signal_connect_object ( GTK_OBJECT ( popup ),
				  "destroy",
				  GTK_SIGNAL_FUNC ( gdk_pointer_ungrab ),
				  GDK_CURRENT_TIME );
      gtk_box_pack_start ( GTK_BOX ( popup_boxv ),
			   calendrier,
			   TRUE,
			   TRUE,
			   0 );
      gtk_widget_show ( calendrier );


      /* ajoute le bouton annuler */

      bouton = gtk_button_new_with_label ( "Annuler" );
      gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				  "clicked",
				  GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
				  GTK_OBJECT ( popup ));
      gtk_box_pack_start ( GTK_BOX ( popup_boxv ),
			   bouton,
			   TRUE,
			   TRUE,
			   0 );
      gtk_widget_show ( bouton );

      gtk_widget_show (popup);
      
      gdk_pointer_grab ( popup -> window, 
			 TRUE,
			 GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			 GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
			 GDK_POINTER_MOTION_MASK,
			 NULL, 
			 NULL, 
			 GDK_CURRENT_TIME );

      gtk_widget_grab_focus ( GTK_WIDGET ( popup ));
    }
}
/*****************************************************************************************************/



/***********************************************************************************************************/
/* Fonction date_selectionnee */
/* appelée lorsqu'on a clické 2 fois sur une date du calendrier */
/***********************************************************************************************************/

void date_selectionnee_etat ( GtkCalendar *calendrier,
			      GtkWidget *popup )
{
  guint annee, mois, jour;

  gtk_calendar_get_date ( calendrier,
			  &annee,
			  &mois,
			  &jour);

  gtk_entry_set_text ( GTK_ENTRY ( gtk_object_get_data ( GTK_OBJECT ( popup ),
							 "entree" )),
		       g_strdup_printf ( "%02d/%02d/%d",
					 jour,
					 mois + 1,
					 annee));
  gtk_widget_destroy ( popup );
}
/***********************************************************************************************************/



/*****************************************************************************************************/
void change_separation_result_periode ( void )
{
  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat )))
    {
      gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				 TRUE );
      modif_type_separation_dates ( gtk_object_get_data ( GTK_OBJECT ( bouton_type_separe_plages_etat ),
							  "type" ));
    }
  else
    {
      gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_type_separe_perso_etat,
				 FALSE );
      gtk_widget_set_sensitive ( entree_separe_perso_etat,
				 FALSE );
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void modif_type_separation_dates ( gint *origine )
{

  switch ( GPOINTER_TO_INT ( origine ))
    {
    case 0:
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 TRUE );
      gtk_widget_set_sensitive ( bouton_type_separe_perso_etat,
				 FALSE );
      gtk_widget_set_sensitive ( entree_separe_perso_etat,
				 FALSE );
      break;

    case 1:
    case 2:
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_type_separe_perso_etat,
				 FALSE );
      gtk_widget_set_sensitive ( entree_separe_perso_etat,
				 FALSE );
      break;

    case 3:
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_type_separe_perso_etat,
				 TRUE );
      gtk_widget_set_sensitive ( entree_separe_perso_etat,
				 TRUE );
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_comptes ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;
  GtkWidget *vbox;
  gint i;
  GtkWidget *label;
  GSList *pointeur_liste;
  GtkWidget *hbox;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );


  /* on met dans la partie de gauche une liste contenant les comptes à */
  /* sélectionner */

  bouton_detaille_comptes_etat = gtk_check_button_new_with_label ( "Détailler les comptes utilisés" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_detaille_comptes_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_detaille_comptes_etat );

  vbox_generale_comptes_etat = gtk_vbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       vbox_generale_comptes_etat,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox_generale_comptes_etat );


  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_comptes_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       vbox_generale_comptes_etat );

  label = gtk_label_new ( "Sélectionner les comptes à inclure dans l'état :" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_comptes_etat ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_comptes_etat ),
		       hbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox );

  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );

  liste_comptes_etat = gtk_clist_new ( 1 );
  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_comptes_etat ),
				 GTK_SELECTION_MULTIPLE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_comptes_etat ),
				     0,
				     TRUE );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_comptes_etat );
  gtk_widget_show ( liste_comptes_etat );

  /* on remplit la liste des comptes */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      gchar *nom[1];
      gint ligne;

      nom[0] = NOM_DU_COMPTE;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_comptes_etat ),
				 nom );

      gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_etat ),
			       ligne,
			       GINT_TO_POINTER ( NO_COMPTE ));
      p_tab_nom_de_compte_variable++;
    }

      

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  bouton = gtk_button_new_with_label ( "Sélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
			      GTK_OBJECT  ( liste_comptes_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "Désélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
			      GTK_OBJECT  ( liste_comptes_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );


  bouton_regroupe_ope_compte_etat = gtk_check_button_new_with_label ( "Regrouper les opérations par compte" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_regroupe_ope_compte_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_regroupe_ope_compte_etat );

  bouton_affiche_sous_total_compte = gtk_check_button_new_with_label ( "Afficher un sous-total lors d'un changement de compte" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_affiche_sous_total_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_compte );

  gtk_signal_connect ( GTK_OBJECT ( bouton_regroupe_ope_compte_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_compte );

  /* on va maintenant sélectionner les comptes nécessaires */

  if ( etat -> utilise_detail_comptes )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat ),
				   TRUE );
  else
    gtk_widget_set_sensitive ( vbox_generale_comptes_etat,
			       FALSE );

  pointeur_liste = etat -> no_comptes;

  while ( pointeur_liste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_comptes_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_comptes_etat ),
							    pointeur_liste -> data ),
			     0 );
      pointeur_liste = pointeur_liste -> next;
    }

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat ),
				 etat -> regroupe_ope_par_compte );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte ),
				 etat -> affiche_sous_total_compte );

  sens_desensitive_pointeur ( bouton_regroupe_ope_compte_etat,
			      bouton_affiche_sous_total_compte );
  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_categories ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *label;
  GSList *pointeur_liste;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;
  GtkWidget *separateur;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  bouton_utilise_categ_etat = gtk_check_button_new_with_label ( "Utiliser les catégories dans l'état" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_utilise_categ_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utilise_categ_etat );

  vbox_generale_categ_etat = gtk_vbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       vbox_generale_categ_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox_generale_categ_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_categ_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       vbox_generale_categ_etat );

  /* mise en place de la frame qui contient la possibilité de détailler les catég utilisées */

  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox );
  gtk_widget_show ( vbox );

  bouton_detaille_categ_etat = gtk_check_button_new_with_label ( "Détailler les catégories utilisées" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_detaille_categ_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_detaille_categ_etat );


  /* mise en place de la liste et des boutons de détaillage */

  hbox_detaille_categ_etat = gtk_hbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox_detaille_categ_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox_detaille_categ_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_categ_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       hbox_detaille_categ_etat );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Sélectionner les catégories à inclure dans l'état :" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );

  liste_categ_etat = gtk_clist_new ( 1 );
  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_categ_etat ),
				 GTK_SELECTION_MULTIPLE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_categ_etat ),
				     0,
				     TRUE );
  gtk_clist_set_compare_func ( GTK_CLIST ( liste_categ_etat ),
			       (GtkCListCompareFunc) classement_alphabetique_tree );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_categ_etat );
  gtk_widget_show ( liste_categ_etat );

  /* on va remplir la liste avec les catégories */

  pointeur_liste = liste_struct_categories;

  while ( pointeur_liste )
    {
      struct struct_categ *categ;
      gchar *nom[1];
      gint ligne;

      categ = pointeur_liste -> data;

      nom[0] = categ -> nom_categ;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_categ_etat ),
				 nom );

      gtk_clist_set_row_data ( GTK_CLIST ( liste_categ_etat ),
			       ligne,
			       GINT_TO_POINTER ( categ -> no_categ ));

      pointeur_liste = pointeur_liste -> next;
    }

  gtk_clist_sort ( GTK_CLIST ( liste_categ_etat ));

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  bouton = gtk_button_new_with_label ( "Sélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
			      GTK_OBJECT  ( liste_categ_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "Désélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
			      GTK_OBJECT  ( liste_categ_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "Catégories de revenus" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( click_type_categ_etat ),
			      NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "Catégories de dépenses" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( click_type_categ_etat ),
			      GINT_TO_POINTER (1));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton_affiche_sous_total_categ = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement de catégorie" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_affiche_sous_total_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_categ );

  /* mise en place du bouton pour afficher les sous categ */

  bouton_afficher_sous_categ = gtk_check_button_new_with_label ( "Afficher les sous-catégories" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_afficher_sous_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_categ );

  bouton_affiche_sous_total_sous_categ = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement de sous-catégorie" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_affiche_sous_total_sous_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_sous_categ );

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_categ ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_sous_categ );


  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  /*   en dessous, on met les boutons d'inclusion ou non des virements */

  bouton_inclusion_virements_actifs_etat = gtk_radio_button_new_with_label ( NULL,
									     "Inclure les virements de ou vers les comptes d'actif et de passif" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_inclusion_virements_actifs_etat,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclusion_virements_actifs_etat );

  bouton_inclusion_virements_hors_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_inclusion_virements_actifs_etat )),
									   "Inclure les virements de ou vers les comptes ne figurant pas dans l'état" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_inclusion_virements_hors_etat,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclusion_virements_hors_etat );

  bouton_non_inclusion_virements = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_inclusion_virements_actifs_etat )),
								     "Ne pas inclure les virements" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_non_inclusion_virements,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_non_inclusion_virements );


  /* on remplit les infos de l'état */

  if ( etat -> utilise_categ )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_categ_etat ),
				   TRUE );
  else
    gtk_widget_set_sensitive ( vbox_generale_categ_etat,
			       FALSE );

  if ( etat -> utilise_detail_categ )
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat ),
				     TRUE );
  else
      gtk_widget_set_sensitive ( hbox_detaille_categ_etat,
				 FALSE );

  /* on sélectionne les catégories choisies */

  pointeur_liste = etat -> no_categ;

  while ( pointeur_liste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_categ_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_categ_etat ),
							    pointeur_liste -> data ),
			     0 );
      pointeur_liste = pointeur_liste -> next;
    }

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ ),
				 etat -> affiche_sous_total_categ );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ ),
				 etat -> afficher_sous_categ );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ ),
				 etat -> affiche_sous_total_sous_categ );
  sens_desensitive_pointeur ( bouton_afficher_sous_categ,
			      bouton_affiche_sous_total_sous_categ );

  if ( etat -> type_virement )
    {
      if ( etat -> type_virement == 1 )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat ),
				       TRUE );
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat ),
				       TRUE );
    }
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements ),
				   TRUE );


  return ( widget_retour );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void click_type_categ_etat ( gint type )
{
  /* type est 0 pour les revenus et 1 pour les dépenses */
  /* fait le tour des catég dans la liste et sélectionne celles */
  /* qui correspondent au type recherché */

  gint i;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_categ_etat ));

  for ( i=0 ; i<GTK_CLIST ( liste_categ_etat ) -> rows ; i++ )
    {
      struct struct_categ *categ;

      categ = g_slist_find_custom ( liste_struct_categories,
				   gtk_clist_get_row_data ( GTK_CLIST ( liste_categ_etat ),
							    i ),
				   (GCompareFunc) recherche_categorie_par_no ) -> data;

      if ( categ -> type_categ == type )
	gtk_clist_select_row ( GTK_CLIST ( liste_categ_etat ),
			       i,
			       0 );
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_ib ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *label;
  GSList *pointeur_liste;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  bouton_utilise_ib_etat = gtk_check_button_new_with_label ( "Utiliser les imputations budgétaires dans l'état" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_utilise_ib_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utilise_ib_etat );

  vbox_generale_ib_etat = gtk_vbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       vbox_generale_ib_etat,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox_generale_ib_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_ib_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       vbox_generale_ib_etat );

  /* mise en place de la frame qui contient la possibilité de détailler les ib utilisées */

  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_ib_etat ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( frame );

  vbox = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox );
  gtk_widget_show ( vbox );

  bouton_detaille_ib_etat = gtk_check_button_new_with_label ( "Détailler les imputations budgétaires utilisées" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_detaille_ib_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_detaille_ib_etat );


  /* mise en place de la liste et des boutons de détaillage */

  hbox_detaille_ib_etat = gtk_hbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox_detaille_ib_etat,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox_detaille_ib_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_ib_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       hbox_detaille_ib_etat );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Sélectionner les imputations budgétaires à inclure dans l'état :" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );

  liste_ib_etat = gtk_clist_new ( 1 );
  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_ib_etat ),
				 GTK_SELECTION_MULTIPLE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_ib_etat ),
				     0,
				     TRUE );
  gtk_clist_set_compare_func ( GTK_CLIST ( liste_ib_etat ),
			       (GtkCListCompareFunc) classement_alphabetique_tree );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_ib_etat );
  gtk_widget_show ( liste_ib_etat );

  /* on va remplir la liste avec les ib */

  pointeur_liste = liste_struct_imputation;

  while ( pointeur_liste )
    {
      struct struct_imputation *imputation;
      gchar *nom[1];
      gint ligne;

      imputation = pointeur_liste -> data;

      nom[0] = imputation -> nom_imputation;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_ib_etat ),
				 nom );

      gtk_clist_set_row_data ( GTK_CLIST ( liste_ib_etat ),
			       ligne,
			       GINT_TO_POINTER ( imputation -> no_imputation ));

      pointeur_liste = pointeur_liste -> next;
    }

  gtk_clist_sort ( GTK_CLIST ( liste_ib_etat ));

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  bouton = gtk_button_new_with_label ( "Sélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
			      GTK_OBJECT  ( liste_ib_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "Désélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
			      GTK_OBJECT  ( liste_ib_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "I.B. de revenus" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( click_type_ib_etat ),
			      NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "I.B. de dépenses" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( click_type_ib_etat ),
			      GINT_TO_POINTER (1));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton_affiche_sous_total_ib = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement d'imputation" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_ib_etat ),
		       bouton_affiche_sous_total_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_ib );

  /* mise en place du bouton pour afficher les sous categ */

  bouton_afficher_sous_ib = gtk_check_button_new_with_label ( "Afficher les sous-imputations" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_ib_etat ),
		       bouton_afficher_sous_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_ib );

  bouton_affiche_sous_total_sous_ib = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement de sous-imputation" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_ib_etat ),
		       bouton_affiche_sous_total_sous_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_sous_ib );

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_ib ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_sous_ib );

  /* on remplit les infos de l'état */

  if ( etat -> utilise_ib )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat ),
				   TRUE );
  else
    gtk_widget_set_sensitive ( vbox_generale_ib_etat,
				 FALSE );

  if ( etat -> utilise_detail_ib )
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ),
				     TRUE );
  else
    gtk_widget_set_sensitive ( hbox_detaille_ib_etat,
			       FALSE );

  /* on sélectionne les ib choisies */

  pointeur_liste = etat -> no_ib;

  while ( pointeur_liste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_ib_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_ib_etat ),
							    pointeur_liste -> data ),
			     0 );
      pointeur_liste = pointeur_liste -> next;
    }

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ),
				 etat -> affiche_sous_total_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib ),
				 etat -> afficher_sous_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ),
				 etat -> affiche_sous_total_sous_ib );
  sens_desensitive_pointeur ( bouton_afficher_sous_ib,
			      bouton_affiche_sous_total_sous_ib );

  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void click_type_ib_etat ( gint type )
{
  /* type est 0 pour les revenus et 1 pour les dépenses */
  /* fait le tour des catég dans la liste et sélectionne celles */
  /* qui correspondent au type recherché */

  gint i;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_ib_etat ));

  for ( i=0 ; i<GTK_CLIST ( liste_ib_etat ) -> rows ; i++ )
    {
      struct struct_imputation *imputation;

      imputation = g_slist_find_custom ( liste_struct_imputation,
					 gtk_clist_get_row_data ( GTK_CLIST ( liste_ib_etat ),
								  i ),
					 (GCompareFunc) recherche_imputation_par_no ) -> data;

      if ( imputation -> type_imputation == type )
	gtk_clist_select_row ( GTK_CLIST ( liste_ib_etat ),
			       i,
			       0 );
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_tiers ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;

  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *vbox2;
  GtkWidget *label;
  GSList *pointeur_liste;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  bouton_utilise_tiers_etat = gtk_check_button_new_with_label ( "Utiliser les tiers dans l'état" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_utilise_tiers_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utilise_tiers_etat );

  vbox_generale_tiers_etat = gtk_vbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       vbox_generale_tiers_etat,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox_generale_tiers_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_tiers_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       vbox_generale_tiers_etat );

  /* mise en place de la frame qui contient la possibilité de détailler les tiers utilisées */

  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_tiers_etat ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( frame );

  vbox = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox );
  gtk_widget_show ( vbox );

  bouton_detaille_tiers_etat = gtk_check_button_new_with_label ( "Détailler les tiers utilisés" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_detaille_tiers_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_detaille_tiers_etat );


  /* mise en place de la liste et des boutons de détaillage */

  hbox_detaille_tiers_etat = gtk_hbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox_detaille_tiers_etat,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox_detaille_tiers_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_tiers_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       hbox_detaille_tiers_etat );

  vbox2 = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
		       vbox2,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox2 );

  label = gtk_label_new ( "Sélectionner les tiers à inclure dans l'état :" );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );

  liste_tiers_etat = gtk_clist_new ( 1 );
  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_tiers_etat ),
				 GTK_SELECTION_MULTIPLE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_tiers_etat ),
				     0,
				     TRUE );
  gtk_clist_set_compare_func ( GTK_CLIST ( liste_tiers_etat ),
			       (GtkCListCompareFunc) classement_alphabetique_tree );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_tiers_etat );
  gtk_widget_show ( liste_tiers_etat );

  /* on va remplir la liste avec les tiers */

  pointeur_liste = liste_struct_tiers;

  while ( pointeur_liste )
    {
      struct struct_tiers *tiers;
      gchar *nom[1];
      gint ligne;

      tiers = pointeur_liste -> data;

      nom[0] = tiers -> nom_tiers;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_tiers_etat ),
				 nom );

      gtk_clist_set_row_data ( GTK_CLIST ( liste_tiers_etat ),
			       ligne,
			       GINT_TO_POINTER ( tiers -> no_tiers ));

      pointeur_liste = pointeur_liste -> next;
    }

  gtk_clist_sort ( GTK_CLIST ( liste_tiers_etat ));

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox2 = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
		       vbox2,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox2 );

  bouton = gtk_button_new_with_label ( "Sélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_select_all ),
			      GTK_OBJECT  ( liste_tiers_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_with_label ( "Désélectionner tout" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT  ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
			      GTK_OBJECT  ( liste_tiers_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );


  bouton_affiche_sous_total_tiers = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement de tiers" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_tiers,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_tiers );



  /* on remplit les infos de l'état */

  if ( etat -> utilise_tiers )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat ),
				   TRUE );
  else
    gtk_widget_set_sensitive ( vbox_generale_tiers_etat,
			       FALSE );

  if ( etat -> utilise_detail_tiers )
     gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat ),
				     TRUE );
  else
    gtk_widget_set_sensitive ( hbox_detaille_tiers_etat,
			       FALSE );
 
  /* on sélectionne les tiers choisies */

  pointeur_liste = etat -> no_tiers;

  while ( pointeur_liste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_tiers_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_tiers_etat ),
							    pointeur_liste -> data ),
			     0 );
      pointeur_liste = pointeur_liste -> next;
    }
    
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers ),
				 etat -> affiche_sous_total_tiers );

  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_texte ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;
  GtkWidget *hbox;
  GtkWidget *label;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Réduire la recherche aux opérations contenant ce texte (notes) :" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  entree_texte_etat = gtk_entry_new ();
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_texte_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_texte_etat );

  /*   on remplit l'entrée */

  if ( etat -> texte )
    gtk_entry_set_text ( GTK_ENTRY ( entree_texte_etat ),
			 g_strstrip ( etat -> texte ));

  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_montant ( struct struct_etat *etat )
{
  GtkWidget *widget_retour;
  GtkWidget *hbox;
  GtkWidget *label;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Réduire la recherche aux opérations contenant ce montant :" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  entree_montant_etat = gtk_entry_new ();
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_montant_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_montant_etat );

  /* on remplit l'entrée */

  if ( etat -> montant )
    gtk_entry_set_text ( GTK_ENTRY ( entree_montant_etat ),
			 g_strdup_printf ( "%4.2f",
					   etat -> montant ));

  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void affichage_etat ( struct struct_etat *etat )
{
  GSList *liste_opes_selectionnees;
  gint i;

  /*   selection des opérations */
  /* on va mettre l'adresse des opés sélectionnées dans une liste */

  liste_opes_selectionnees = NULL;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      /* on commence par vérifier que le compte fait partie de l'état */

      if ( !etat -> utilise_detail_comptes
	   ||
	   g_slist_index ( etat -> no_comptes,
			   GINT_TO_POINTER ( i )) != -1 )
	{
	  /* 	  le compte est bon, passe à la suite de la sélection */

	  /* on va faire le tour de toutes les opés du compte */

	  GSList *pointeur_tmp;

	  pointeur_tmp = LISTE_OPERATIONS;

	  while ( pointeur_tmp )
	    {
	      struct structure_operation *operation;

	      operation = pointeur_tmp -> data;

	      /* si c'est une opé ventilée, ce n'est pas une opé -> on passe */

	      if ( operation -> operation_ventilee )
		goto operation_refusee;

	      /* 	  on va vérifier si un montant est demandé, c'est le test le plus rapide */
	      /* et le plus limitant */

	      if ( etat -> montant
		   &&
		   operation -> montant != etat -> montant )
		goto operation_refusee;

	      /* on vérifie ensuite si un texte est recherché */

	      if ( etat -> texte
		   &&
		   (
		    !operation -> notes
		    ||
		    !strcasestr ( operation -> notes,
				  etat -> texte )))
		goto operation_refusee;


	      if ( etat -> utilise_categ )
		{
		  /* 	      on vérifie les virements et les catég */

		  if ( operation -> relation_no_operation )
		    {
		      if ( !etat -> type_virement )
			goto operation_refusee;

		      if ( etat -> type_virement == 1 )
			{
			  /* on inclue l'opé que si le compte de virement */
			  /* est un compte de passif ou d'actif */

			  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

			  /* pour l'instant on n'a que le compte passif */

			  if ( TYPE_DE_COMPTE != 2 )
			    goto operation_refusee;

			  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i; 
			}
		      else
			{
			  /* on inclut l'opé que si le compte de virement n'est */
			  /* pas présent dans l'état */

			  if ( g_slist_index ( etat -> no_comptes,
					       GINT_TO_POINTER ( operation -> relation_no_compte )) != -1 )
			    goto operation_refusee;
			}
		    }
		  else
		    {
		      /* on va maintenant vérifier que les catég sont bonnes */
 
		      if ( etat -> utilise_detail_categ
			   &&
			   g_slist_index ( etat -> no_categ,
					   GINT_TO_POINTER ( operation -> categorie )) == -1 )
			goto operation_refusee;
		    }
		}

	      /* vérification de l'imputation budgétaire */

	      if ( etat -> utilise_ib
		   &&
		   etat -> utilise_detail_ib
		   &&
		   g_slist_index ( etat -> no_ib,
				   GINT_TO_POINTER ( operation -> imputation )) == -1 )
		goto operation_refusee;

	      /* vérification du tiers */

	      if ( etat -> utilise_tiers
		   &&
		   etat -> utilise_detail_tiers
		   &&
		   g_slist_index ( etat -> no_tiers,
				   GINT_TO_POINTER ( operation -> tiers )) == -1 )
		goto operation_refusee;


	      /* vérifie la plage de date */

	      if ( etat -> exo_date )
		{
		  /* on utilise l'exercice */

		  if ( etat -> utilise_detail_exo
		       &&
		       g_slist_index ( etat -> no_exercices,
				       GINT_TO_POINTER ( operation -> no_exercice )) == -1 )
		    goto operation_refusee;
		}
	      else
		{
		  /* on utilise une plage de dates */

		  GDate *date_jour;
		  GDate *date_tmp;

		  date_jour = g_date_new ();
		  g_date_set_time ( date_jour,
				    time ( NULL ));


		  switch ( etat -> no_plage_date )
		    {
		    case 0:
		      /* plage perso */

		      if ( !etat -> date_perso_debut
			   ||
			   !etat -> date_perso_fin
			   ||
			   g_date_compare ( etat -> date_perso_debut,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( etat -> date_perso_fin,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 1:
		      /* toutes */

		      break;

		    case 2:
		      /* cumul à ce jour */

		      if ( g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 3:
		      /* mois en cours */

		      if ( g_date_month ( date_jour ) != g_date_month ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 4:
		      /* année en cours */

		      if ( g_date_year ( date_jour ) != g_date_year ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 5:
		      /* cumul mensuel */

		      if ( g_date_month ( date_jour ) != g_date_month ( operation -> date )
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 6:
		      /* cumul annuel */

		      if ( g_date_year ( date_jour ) != g_date_year ( operation -> date )
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 7:
		      /* mois précédent */

		      g_date_subtract_months ( date_jour,
						1 );

		      if ( g_date_month ( date_jour ) != g_date_month ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 8:
		      /* année précédente */

		      g_date_subtract_years ( date_jour,
					       1 );

		      if ( g_date_year ( date_jour ) != g_date_year ( operation -> date ))
			goto operation_refusee;
		      break;

		    case 9:
		      /* 30 derniers jours */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_days ( date_tmp,
					      30 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 10:
		      /* 3 derniers mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_months ( date_tmp,
						3 );
		      g_date_subtract_months ( date_jour,
						1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 11:
		      /* 6 derniers mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_months ( date_tmp,
						6 );
		      g_date_subtract_months ( date_jour,
						1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 12:
		      /* 12 derniers mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_subtract_months ( date_tmp,
						12 );
		      g_date_subtract_months ( date_jour,
						1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) > 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) < 0 )
			goto operation_refusee;
		      break;

		    case 13:
		      /* 30 prochains jours */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_days ( date_tmp,
					      30 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 14:
		      /* 3 prochains mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_months ( date_tmp,
						3 );
		      g_date_add_months ( date_jour,
						1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 15:
		      /* 6 prochains mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_months ( date_tmp,
						6 );
		      g_date_add_months ( date_jour,
						1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		      break;

		    case 16:
		      /* 12 prochains mois */

		      date_tmp = g_date_new_dmy ( g_date_day ( date_jour ),
						  g_date_month ( date_jour ),
						  g_date_year ( date_jour ) );
		      g_date_add_months ( date_tmp,
						12 );
		      g_date_add_months ( date_jour,
						1 );
		      if ( g_date_compare ( date_tmp,
					    operation -> date ) < 0
			   ||
			   g_date_compare ( date_jour,
					    operation -> date ) > 0 )
			goto operation_refusee;
		    }
		}


	      liste_opes_selectionnees = g_slist_append ( liste_opes_selectionnees,
							  operation );

	    operation_refusee:
	      pointeur_tmp = pointeur_tmp -> next;
	    }
	}
      p_tab_nom_de_compte_variable++;
    }

  /*   à ce niveau, on a récupéré toutes les opés qui entreront dans */
  /* l'état ; reste plus qu'à les classer et les afficher */

  printf ( "%d opérations trouvées\n", g_slist_length ( liste_opes_selectionnees ));

  /* on classe la liste en fonction du choix du type de classement */

/*   liste_opes_selectionnees = g_slist_sort ( liste_opes_selectionnees, */
/* 					    (GCompareFunc) classement_liste_opes_etat ); */

/* pas trouvé mieux encore que de faire 6 routines différentes qui affichent l'état */

  switch ( etat -> type_classement )
    {
    case 0 :
      etat_c_i_co ( liste_opes_selectionnees );
      break;

    case 1 :
      etat_c_co_i ( liste_opes_selectionnees );
      break;

    case 2 :
      etat_i_c_co ( liste_opes_selectionnees );
      break;

    case 3 :
      etat_i_co_c ( liste_opes_selectionnees );
      break;

    case 4 :
      etat_co_i_c ( liste_opes_selectionnees );
      break;

    case 5 :
      etat_co_c_i ( liste_opes_selectionnees );
      break;
    }

}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* Fonction de classement de la liste */
/* en fonction du choix du type de classement */
/*****************************************************************************************************/

gint classement_liste_opes_etat ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 )
{

  /* on commence en séparant les types 2 à 2 */

  if ( !etat_courant -> type_classement
       ||
       etat_courant -> type_classement == 1 )
    {
      /* on commence par classer entre catégories */

      if ( operation_1 -> categorie != operation_2 -> categorie )
	return ( operation_1 -> categorie - operation_2 -> categorie );

      if ( operation_1 -> sous_categorie != operation_2 -> sous_categorie )
	return ( operation_1 -> sous_categorie - operation_2 -> sous_categorie );

      /*       les catégories sont identiques, on doit départager le type_classement */

      if ( !etat_courant -> type_classement )
	{
	  /* on classe maintenant les ib */

	  if ( operation_1 -> imputation != operation_2 -> imputation )
	    return ( operation_1 -> imputation - operation_2 -> imputation );

	  if ( operation_1 -> sous_imputation != operation_2 -> sous_imputation )
	    return ( operation_1 -> sous_imputation - operation_2 -> sous_imputation );

	  /* 	      les ib sont identiques, on classe les no_compte */

	  if ( operation_1 -> no_compte != operation_2 -> no_compte )
	    return ( operation_1 ->no_compte  - operation_2 -> no_compte );

	  /* les no_compte sont identiques, on classe par date */

	  return ( g_date_compare ( operation_1 -> date,
				    operation_2 -> date ));
	}
      else
	{
	  /* on classe maintenant les no_compte */

	  if ( operation_1 -> no_compte != operation_2 -> no_compte )
	    return ( operation_1 ->no_compte  - operation_2 -> no_compte );

	  /* 	      les no_compte sont identiques, on classe les ib */

	  if ( operation_1 -> imputation != operation_2 -> imputation )
	    return ( operation_1 -> imputation - operation_2 -> imputation );

	  if ( operation_1 -> sous_imputation != operation_2 -> sous_imputation )
	    return ( operation_1 -> sous_imputation - operation_2 -> sous_imputation );

	  /* les ib sont identiques, on classe par date */

	  return ( g_date_compare ( operation_1 -> date,
				    operation_2 -> date ));
	}
    }
  else
    {
      if ( etat_courant -> type_classement == 2
	   ||
	   etat_courant -> type_classement == 3 )
	{
	  /* on commence par classer les ib */

	  if ( operation_1 -> imputation != operation_2 -> imputation )
	    return ( operation_1 -> imputation - operation_2 -> imputation );

	  if ( operation_1 -> sous_imputation != operation_2 -> sous_imputation )
	    return ( operation_1 -> sous_imputation - operation_2 -> sous_imputation );

	  /*       les ib sont identiques, on doit départager le type_classement */

	  if ( etat_courant -> type_classement == 2 )
	    {
	      /* on classe les catégories */

	      if ( operation_1 -> categorie != operation_2 -> categorie )
		return ( operation_1 -> categorie - operation_2 -> categorie );

	      if ( operation_1 -> sous_categorie != operation_2 -> sous_categorie )
		return ( operation_1 -> sous_categorie - operation_2 -> sous_categorie );

	      /* les categ sont identiques, on classe les no_compte */

	      if ( operation_1 -> no_compte != operation_2 -> no_compte )
		return ( operation_1 ->no_compte  - operation_2 -> no_compte );

	      /* les no_compte sont identiques, on classe par date */

	      return ( g_date_compare ( operation_1 -> date,
					operation_2 -> date ));
	    }
	  else
	    {
	      /* on classe maintenant les no_compte */

	      if ( operation_1 -> no_compte != operation_2 -> no_compte )
		return ( operation_1 ->no_compte  - operation_2 -> no_compte );
				   
	      /* on classe les catégories */

	      if ( operation_1 -> categorie != operation_2 -> categorie )
		return ( operation_1 -> categorie - operation_2 -> categorie );

	      if ( operation_1 -> sous_categorie != operation_2 -> sous_categorie )
		return ( operation_1 -> sous_categorie - operation_2 -> sous_categorie );

	      /* les categ sont identiques, on classe par date */

	      return ( g_date_compare ( operation_1 -> date,
					operation_2 -> date ));
	    }
	}
      else
	{
	  /* on commence par classer les no_compte */

	  if ( operation_1 -> no_compte != operation_2 -> no_compte )
	    return ( operation_1 ->no_compte  - operation_2 -> no_compte );

	  /*       les no_compte sont identiques, on doit départager le type_classement */

	  if ( etat_courant -> type_classement == 4 )
	    {
	      /* 	 on classe les ib */

	      if ( operation_1 -> imputation != operation_2 -> imputation )
		return ( operation_1 -> imputation - operation_2 -> imputation );

	      if ( operation_1 -> sous_imputation != operation_2 -> sous_imputation )
		return ( operation_1 -> sous_imputation - operation_2 -> sous_imputation );

	      /* on classe les catégories */

	      if ( operation_1 -> categorie != operation_2 -> categorie )
		return ( operation_1 -> categorie - operation_2 -> categorie );

	      if ( operation_1 -> sous_categorie != operation_2 -> sous_categorie )
		return ( operation_1 -> sous_categorie - operation_2 -> sous_categorie );

	      /* les categ sont identiques, on classe par date */

	      return ( g_date_compare ( operation_1 -> date,
					operation_2 -> date ));
	    }
	  else
	    {
	      /* on classe les catégories */

	      if ( operation_1 -> categorie != operation_2 -> categorie )
		return ( operation_1 -> categorie - operation_2 -> categorie );

	      if ( operation_1 -> sous_categorie != operation_2 -> sous_categorie )
		return ( operation_1 -> sous_categorie - operation_2 -> sous_categorie );

	      /* 	 on classe les ib */

	      if ( operation_1 -> imputation != operation_2 -> imputation )
		return ( operation_1 -> imputation - operation_2 -> imputation );

	      if ( operation_1 -> sous_imputation != operation_2 -> sous_imputation )
		return ( operation_1 -> sous_imputation - operation_2 -> sous_imputation );

	      /* les ib sont identiques, on classe par date */

	      return ( g_date_compare ( operation_1 -> date,
					operation_2 -> date ));
	    }
	}
    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etat_c_i_co ( GSList *ope_selectionnees )
{
  /* on fait un affichage catég -> ib -> compte */

  GSList *ope_categ_revenus;
  GSList *ope_categ_depenses;
  GSList *pointeur_tmp;
  gint i;
  GtkWidget *table_etat;
  gint ligne;
  GtkWidget *label;
  gdouble total_partie;
  gdouble total_general;
  GtkWidget *separateur;
  gchar *decalage_categ;
  gchar *decalage_sous_categ;
  gchar *decalage_ib;
  gchar *decalage_sous_ib;
  gchar *decalage_compte;
  gchar *decalage_tiers;
  gint nb_colonnes;


  /* on commence par séparer la liste en catég de revenus et de dépenses */

  ope_categ_revenus = NULL;
  ope_categ_depenses = NULL;
  pointeur_tmp = ope_selectionnees;

  while ( pointeur_tmp )
    {
      struct structure_operation *operation;

      operation = pointeur_tmp -> data;

      /* s'il n'y a pas de catég, c'est un virement (les opés ventilées n'ont pas été sélectionnées) */
      /*       dans ce cas, on classe en fonction du montant */

      if ( operation -> categorie )
	{
	  struct struct_categ *categ;

	  categ = g_slist_find_custom ( liste_struct_categories,
					GINT_TO_POINTER ( operation -> categorie ),
					(GCompareFunc) recherche_categorie_par_no ) -> data;

	  if ( categ -> type_categ )
	    ope_categ_depenses = g_slist_append ( ope_categ_depenses,
						  operation );
	  else
	    ope_categ_revenus = g_slist_append ( ope_categ_revenus,
						  operation );
	}
      else
	{
	  if ( operation -> montant < 0 )
	    ope_categ_depenses = g_slist_append ( ope_categ_depenses,
						  operation );
	  else
	    ope_categ_revenus = g_slist_append ( ope_categ_revenus,
						  operation );
	}
      pointeur_tmp = pointeur_tmp -> next;
    }

  /* on va maintenant classer ces 2 listes dans l'ordre categ->ib->comptes */

  ope_categ_depenses = g_slist_sort ( ope_categ_depenses,
				      (GCompareFunc) classement_liste_opes_etat );
  ope_categ_revenus = g_slist_sort ( ope_categ_revenus,
				     (GCompareFunc) classement_liste_opes_etat );

  
  /* calcul du décalage pour chaque classement */
  /* c'est une chaine vide qu'on ajoute devant le nom du */
  /*   classement ( tiers, ib ...) */
 
  /* on met 2 espaces par décalage */

  if ( etat_courant -> utilise_categ )
    {
      decalage_categ = g_strdup ( "    " );

      if ( etat_courant -> afficher_sous_categ )
	decalage_sous_categ = g_strconcat ( decalage_categ,
					    "    ",
					    NULL );
      else
	decalage_sous_categ = decalage_categ;
    }
  else
    {
      decalage_categ = g_strdup ( "" );
      decalage_sous_categ = decalage_categ;
    }

  if ( etat_courant -> utilise_ib )
    {
      decalage_ib = g_strconcat ( decalage_sous_categ,
				  "    ",
				  NULL );

      if ( etat_courant -> afficher_sous_ib )
	decalage_sous_ib = g_strconcat ( decalage_ib,
					 "    ",
					 NULL );
      else
	decalage_sous_ib = decalage_ib;
    }
  else
    {
      decalage_ib = decalage_sous_categ;
      decalage_sous_ib = decalage_ib;
    }

  if ( etat_courant -> regroupe_ope_par_compte )
    decalage_compte = g_strconcat ( decalage_sous_ib,
				    "    ",
				    NULL );
  else
    decalage_compte = decalage_sous_ib;

  if ( etat_courant -> utilise_tiers )
    decalage_tiers = g_strconcat ( decalage_compte,
				   "    ",
				   NULL );
  else
    decalage_tiers = decalage_compte;


  /*   calcul du nb de colonnes : */
  /* 1ère pour les titres de structure */
  /* la dernière pour les montants */
  /* et entre les 2 pour l'affichage des opés -> variable */

  nb_colonnes = 2;

  if ( etat_courant -> afficher_opes )
    {
      nb_colonnes = nb_colonnes + etat_courant -> afficher_date_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_tiers_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_categ_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_sous_categ_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_ib_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_sous_ib_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_notes_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_pc_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_infobd_ope;
    }

  /* on peut créer la table */
  /* pas besoin d'indiquer la hauteur, elle grandit automatiquement */

  table_etat = gtk_table_new ( 0,
			       nb_colonnes,
			       FALSE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table_etat ),
			       5 );
  if ( GTK_BIN ( scrolled_window_etat ) -> child )
    gtk_container_remove ( GTK_CONTAINER ( scrolled_window_etat ),
			   GTK_BIN ( scrolled_window_etat ) -> child );

  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window_etat ),
					  table_etat );
  gtk_widget_show ( table_etat );



  /* on commence à remplir le tableau */

  /* on met le titre */

  ligne = 0;
  total_general = 0;

  label = gtk_label_new ( etat_courant -> nom_etat );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, 3,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  /* séparation */

  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     separateur,
		     0, 3,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );

  ligne++;

  for ( i=0 ; i<2 ; i++ )
    {
      gint ancienne_ib;
      gint ancienne_sous_ib;
      gint ancienne_categ;
      gint ancienne_sous_categ;
      gint ancien_compte;
      gint ancien_tiers;
      gdouble montant_categ;
      gdouble montant_sous_categ;
      gdouble montant_ib;
      gdouble montant_sous_ib;
      gdouble montant_compte;
      gdouble montant_tiers;
      gint changement_de_groupe;
      gint debut;


      ancienne_categ = -1;
      ancienne_sous_categ = -1;
      ancienne_ib = -1;
      ancienne_sous_ib = -1;
      ancien_compte = -1;
      ancien_tiers = -1;
      montant_categ = 0;
      montant_sous_categ = 0;
      montant_ib = 0;
      montant_sous_ib = 0;
      montant_compte = 0;
      montant_tiers = 0;
      total_partie = 0;

      changement_de_groupe = 0;
      debut = 1;

      /* on met ici le pointeur sur les revenus ou sur les dépenses */
      /* en vérifiant qu'il y en a */

      if ( !i )
	{
	  /* on met le pointeur sur les revenus */

	  if ( ope_categ_revenus )
	    {
	      pointeur_tmp = ope_categ_revenus;

	      /* revenus */

	      label = gtk_label_new ( "" );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;
	      label = gtk_label_new ( "Revenus" );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;

	      label = gtk_label_new ( "" );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;
	    }
	  else
	    {
	      /* il n'y a pas de revenus, on saute directement aux dépenses */

	    i++;
	    pointeur_tmp = ope_categ_depenses;

	    /* dépenses */

	    label = gtk_label_new ( "" );
	    gtk_table_attach ( GTK_TABLE ( table_etat ),
			       label,
			       0, 1,
			       ligne, ligne + 1,
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
			       0, 0 );
	    gtk_widget_show ( label );

	    ligne++;

	    label = gtk_label_new ( "Dépenses" );
	    gtk_table_attach ( GTK_TABLE ( table_etat ),
			       label,
			       0, 1,
			       ligne, ligne + 1,
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
			       0, 0 );
	    gtk_widget_show ( label );

	    ligne++;

	    label = gtk_label_new ( "" );
	    gtk_table_attach ( GTK_TABLE ( table_etat ),
			       label,
			       0, 1,
			       ligne, ligne + 1,
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
			       0, 0 );
	    gtk_widget_show ( label );

	    ligne++;
	    }
	}
      else
	{
	  /* on met le pointeur sur les dépenses */

	  if ( ope_categ_depenses )
	    {
	      /* séparation */

	      separateur = gtk_hseparator_new ();
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 separateur,
				 0, 3,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( separateur );

	      ligne++;

	      pointeur_tmp = ope_categ_depenses;

	      /* dépenses */

	      label = gtk_label_new ( "" );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;

	      label = gtk_label_new ( "Dépenses" );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;

	      label = gtk_label_new ( "" );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;
	    }
	  else
	    goto fin_boucle_affichage_etat;
	}


      while ( pointeur_tmp )
	{
	  struct structure_operation *operation;
	  gchar *pointeur_char;

	  operation = pointeur_tmp -> data;

	  /* mise en place des catégories */

	  if ( etat_courant -> utilise_categ
	       &&
	       operation -> categorie != ancienne_categ )
	    {

	      if ( !debut )
		{
		  /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

		  ligne = affiche_total_tiers ( table_etat,
						montant_tiers,
						ligne );

		  ligne = affiche_total_compte ( table_etat,
						 montant_compte,
						 ligne );

		  ligne = affiche_total_sous_ib ( table_etat,
						  montant_sous_ib,
						  ligne );

		  ligne = affiche_total_ib ( table_etat,
					     montant_ib,
					     ligne );

		  ligne = affiche_total_sous_categ ( table_etat,
						     montant_sous_categ,
						     ligne );

		  /* on ajoute le total de la categ */

		  ligne = affiche_total_categories ( table_etat,
						     montant_categ,
						     ligne );

		  montant_categ = 0;
		  montant_sous_categ = 0;
		  montant_ib = 0;
		  montant_sous_ib = 0;
		  montant_compte = 0;
		  montant_tiers = 0;
		}

	      if ( operation -> categorie )
		pointeur_char = g_strconcat ( decalage_categ,
					      ((struct struct_categ *)(g_slist_find_custom ( liste_struct_categories,
											     GINT_TO_POINTER ( operation -> categorie ),
											     (GCompareFunc) recherche_categorie_par_no ) -> data )) -> nom_categ,
					      NULL );
	      else
		{
		  if ( operation -> relation_no_operation )
		    pointeur_char = g_strconcat ( decalage_categ,
						  "Virements",
						  NULL );
		  else
		    pointeur_char = g_strconcat ( decalage_categ,
						  "Pas de catégorie",
						  NULL );
		}

	      label = gtk_label_new ( pointeur_char );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;

	      ancienne_categ = operation -> categorie;
	      ancienne_sous_categ = -1;
	      ancienne_ib = -1;
	      ancienne_sous_ib = -1;
	      ancien_compte = -1;
	      ancien_tiers = -1;

	      debut = 0;
	      changement_de_groupe = 1;
	    }


	  if ( etat_courant -> utilise_categ
	       &&
	       etat_courant -> afficher_sous_categ
	       &&
	       operation -> categorie
	       &&
	       operation -> sous_categorie != ancienne_sous_categ )
	    {
	      struct struct_categ *categ;

	      /* on ajoute le total de la sous-categ */

	      if ( !changement_de_groupe )
		{
		  /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

		  ligne = affiche_total_tiers ( table_etat,
						montant_tiers,
						ligne );

		  ligne = affiche_total_compte ( table_etat,
						 montant_compte,
						 ligne );

		  ligne = affiche_total_sous_ib ( table_etat,
						  montant_sous_ib,
						  ligne );

		  ligne = affiche_total_ib ( table_etat,
					     montant_ib,
					     ligne );

		  ligne = affiche_total_sous_categ ( table_etat,
						     montant_sous_categ,
						     ligne );

		}

	      montant_sous_categ = 0;
	      montant_ib = 0;
	      montant_sous_ib = 0;
	      montant_compte = 0;
	      montant_tiers = 0;

	      categ = g_slist_find_custom ( liste_struct_categories,
					    GINT_TO_POINTER ( operation -> categorie ),
					    (GCompareFunc) recherche_categorie_par_no ) -> data;

	      if ( operation -> sous_categorie )
	      pointeur_char = g_strconcat ( decalage_sous_categ,
					    ((struct struct_sous_categ *)(g_slist_find_custom ( categ->liste_sous_categ,
												GINT_TO_POINTER ( operation -> sous_categorie ),
												(GCompareFunc) recherche_sous_categorie_par_no ) -> data )) -> nom_sous_categ,
					    NULL );
	      else
		pointeur_char = g_strconcat ( decalage_sous_categ,
					      "Pas de sous-catégorie",
					      NULL );

	      label = gtk_label_new ( pointeur_char );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0,1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;

	      ancienne_sous_categ = operation -> sous_categorie;
	      ancienne_ib = -1;
	      ancienne_sous_ib = -1;
	      ancien_compte = -1;
	      ancien_tiers = -1;

	      changement_de_groupe = 1;
	    }



	  /* mise en place de l'ib */


	  if ( etat_courant -> utilise_ib
	       &&
	       operation -> imputation != ancienne_ib )
	    {
	      /* on ajoute le total de l'ib */

	      if ( !changement_de_groupe )
		{
		  ligne = affiche_total_tiers ( table_etat,
						montant_tiers,
						ligne );

		  ligne = affiche_total_compte ( table_etat,
						 montant_compte,
						 ligne );

		  ligne = affiche_total_sous_ib ( table_etat,
						  montant_sous_ib,
						  ligne );

		  ligne = affiche_total_ib ( table_etat,
					     montant_ib,
					     ligne );
		}

	      montant_ib = 0;
	      montant_sous_ib = 0;
	      montant_compte = 0;
	      montant_tiers = 0;

	      if ( operation -> imputation )
		pointeur_char = g_strconcat ( decalage_ib,
					      ((struct struct_imputation *)(g_slist_find_custom ( liste_struct_imputation,
												  GINT_TO_POINTER ( operation -> imputation ),
												  (GCompareFunc) recherche_imputation_par_no ) -> data )) -> nom_imputation,
					      NULL );
	      else
		pointeur_char = g_strconcat ( decalage_ib,
					      "Pas d'imputation budgétaire",
					      NULL );

	      label = gtk_label_new ( pointeur_char );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;

	      ancienne_ib = operation -> imputation;
	      ancienne_sous_ib = -1;
	      ancien_compte = -1;
	      ancien_tiers = -1;

	      changement_de_groupe = 1;
	    }


	  /* mise en place de la sous_ib */


	  if ( etat_courant -> utilise_ib
	       &&
	       etat_courant -> afficher_sous_ib
	       &&
	       operation -> imputation
	       &&
	       operation -> sous_imputation != ancienne_sous_ib )
	    {
	      struct struct_imputation *imputation;

	      /* on ajoute le total de la sous ib */

	      if ( !changement_de_groupe )
		{
		  ligne = affiche_total_tiers ( table_etat,
						montant_tiers,
						ligne );

		  ligne = affiche_total_compte ( table_etat,
						 montant_compte,
						 ligne );

		  ligne = affiche_total_sous_ib ( table_etat,
						  montant_sous_ib,
						  ligne );
		}

	      montant_sous_ib = 0;
	      montant_compte = 0;
	      montant_tiers = 0;

	      imputation = g_slist_find_custom ( liste_struct_imputation,
						 GINT_TO_POINTER ( operation -> imputation ),
						 (GCompareFunc) recherche_imputation_par_no ) -> data;

	      if ( operation -> sous_imputation )
		pointeur_char = g_strconcat ( decalage_sous_ib,
					      ((struct struct_sous_imputation *)(g_slist_find_custom ( imputation->liste_sous_imputation,
												       GINT_TO_POINTER ( operation -> sous_imputation ),
												       (GCompareFunc) recherche_sous_imputation_par_no ) -> data )) -> nom_sous_imputation,
					      NULL );
	      else
		pointeur_char = g_strconcat ( decalage_sous_ib,
					      "Pas de sous-imputation",
					      NULL );

	      label = gtk_label_new ( pointeur_char );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;

	      ancienne_sous_ib = operation -> sous_imputation;
	      ancien_compte = -1;
	      ancien_tiers = -1;

	      changement_de_groupe = 1;
	    }

	  /* mise en place du compte */

	  if ( etat_courant -> regroupe_ope_par_compte
	       &&
	       operation -> no_compte != ancien_compte )
	    {

	      /* on ajoute le total du compte */

	      if ( !changement_de_groupe )
		{
		  ligne = affiche_total_tiers ( table_etat,
						montant_tiers,
						ligne );

		  ligne = affiche_total_compte ( table_etat,
						 montant_compte,
						 ligne );
		}

	      montant_compte = 0;
	      montant_tiers = 0;

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	      pointeur_char = g_strconcat ( decalage_compte,
					    NOM_DU_COMPTE,
					    NULL );

	      label = gtk_label_new ( pointeur_char );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;
	      ancien_compte = operation -> no_compte;
	      ancien_tiers = -1;

	      changement_de_groupe = 1;
	    }



	  /* affiche le tiers */

	  if ( etat_courant -> utilise_tiers
	       &&
	       operation -> tiers != ancien_tiers )
	    {

	      /* on ajoute le total du tiers */

	      if ( !changement_de_groupe )
		{
		  ligne = affiche_total_tiers ( table_etat,
						montant_tiers,
						ligne );
		}

	      montant_tiers = 0;

	      if ( operation -> tiers )
		pointeur_char = g_strconcat ( decalage_tiers,
					      ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
											     GINT_TO_POINTER ( operation -> tiers ),
											     (GCompareFunc) recherche_tiers_par_no ) -> data )) -> nom_tiers,
					      NULL );
	      else
		pointeur_char = g_strconcat ( decalage_tiers,
					      "Pas de tiers",
					      NULL );

	      label = gtk_label_new ( pointeur_char );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 0, 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );


	      ligne++;
	      ancien_tiers = operation -> tiers;

	      changement_de_groupe = 1;
	    }


	  if ( etat_courant -> afficher_opes )
	    {
	      gint colonne;

	      /* on affiche ce qui est demandé pour les opés */

	      colonne = 1;

	      if ( etat_courant -> afficher_date_ope )
		{
		  label = gtk_label_new ( g_strdup_printf  ( "%d/%d/%d",
							     operation -> jour,
							     operation -> mois,
							     operation -> annee ));
		  gtk_misc_set_alignment ( GTK_MISC ( label ),
					   0,
					   0.5 );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     label,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( label );

		  colonne++;
		}

	      if ( etat_courant -> afficher_tiers_ope )
		{
		  if ( operation -> tiers )
		    {
		      label = gtk_label_new ( ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
											     GINT_TO_POINTER ( operation -> tiers ),
											     (GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers );
		      
		      gtk_misc_set_alignment ( GTK_MISC ( label ),
					       0,
					       0.5 );
		      gtk_table_attach ( GTK_TABLE ( table_etat ),
					 label,
					 colonne, colonne + 1,
					 ligne, ligne + 1,
					 GTK_SHRINK | GTK_FILL,
					 GTK_SHRINK | GTK_FILL,
					 0, 0 );
		      gtk_widget_show ( label );
		    }

		  colonne++;
		}

	      if ( etat_courant -> afficher_categ_ope )
		{
		  gchar *pointeur;

		  pointeur = NULL;

		  if ( operation -> categorie )
		    {
		      struct struct_categ *categ;

		      categ = g_slist_find_custom ( liste_struct_categories,
						    GINT_TO_POINTER ( operation -> categorie ),
						    (GCompareFunc) recherche_categorie_par_no ) -> data;
		      pointeur = categ -> nom_categ;

		      if ( operation -> sous_categorie
			   &&
			   etat_courant -> afficher_sous_categ_ope )
			pointeur = g_strconcat ( pointeur,
						 " : ",
						 ((struct struct_sous_categ *)(g_slist_find_custom ( categ -> liste_sous_categ,
												     GINT_TO_POINTER ( operation -> sous_categorie ),
												     (GCompareFunc) recherche_sous_categorie_par_no ) -> data )) -> nom_sous_categ,
						 NULL );
		    }
		  else
		    {
		      /* si c'est un virement, on le marque, sinon c'est qu'il n'y a pas de categ */
		      /* ou que c'est une opé ventilée, et on marque rien */

		      if ( operation -> relation_no_operation )
			{
			  /* c'est un virement */

			  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

			  if ( operation -> montant < 0 )
			    pointeur = g_strconcat ( "Virement vers ",
						     NOM_DU_COMPTE,
						     NULL );
			  else
			    pointeur = g_strconcat ( "Virement de ",
						     NOM_DU_COMPTE,
						     NULL );
			}
		    }

		  if ( pointeur )
		    {
		      label = gtk_label_new ( pointeur );
		      gtk_misc_set_alignment ( GTK_MISC ( label ),
					       0,
					       0.5 );
		      gtk_table_attach ( GTK_TABLE ( table_etat ),
					 label,
					 colonne, colonne + 1,
					 ligne, ligne + 1,
					 GTK_SHRINK | GTK_FILL,
					 GTK_SHRINK | GTK_FILL,
					 0, 0 );
		      gtk_widget_show ( label );
		    }
		  colonne++;
		}
		


	      if ( etat_courant -> afficher_ib_ope )
		{
		  if ( operation -> imputation )
		    {
		      struct struct_imputation *ib;
		      gchar *pointeur;

		      ib = g_slist_find_custom ( liste_struct_imputation,
						 GINT_TO_POINTER ( operation -> imputation ),
						 (GCompareFunc) recherche_imputation_par_no ) -> data;
		      pointeur = ib -> nom_imputation;

		      if ( operation -> sous_imputation
			   &&
			   etat_courant -> afficher_sous_ib_ope )
			pointeur = g_strconcat ( pointeur,
						 " : ",
						 ((struct struct_sous_imputation *)(g_slist_find_custom ( ib -> liste_sous_imputation,
													  GINT_TO_POINTER ( operation -> sous_imputation ),
													  (GCompareFunc) recherche_sous_imputation_par_no ) -> data )) -> nom_sous_imputation,
						 NULL );

		      label = gtk_label_new ( pointeur );
		      gtk_misc_set_alignment ( GTK_MISC ( label ),
					       0,
					       0.5 );
		      gtk_table_attach ( GTK_TABLE ( table_etat ),
					 label,
					 colonne, colonne + 1,
					 ligne, ligne + 1,
					 GTK_SHRINK | GTK_FILL,
					 GTK_SHRINK | GTK_FILL,
					 0, 0 );
		      gtk_widget_show ( label );
		    }
		  colonne++;
		}


	      if ( etat_courant -> afficher_notes_ope )
		{
		  if ( operation -> notes )
		    {
		      label = gtk_label_new ( operation -> notes );
		      gtk_misc_set_alignment ( GTK_MISC ( label ),
					       0,
					       0.5 );
		      gtk_table_attach ( GTK_TABLE ( table_etat ),
					 label,
					 colonne, colonne + 1,
					 ligne, ligne + 1,
					 GTK_SHRINK | GTK_FILL,
					 GTK_SHRINK | GTK_FILL,
					 0, 0 );
		      gtk_widget_show ( label );
		    }
		  colonne++;
		}

	      if ( etat_courant -> afficher_pc_ope )
		{
		  if ( operation -> no_piece_comptable )
		    {
		      label = gtk_label_new ( operation -> no_piece_comptable );
		      gtk_misc_set_alignment ( GTK_MISC ( label ),
					       0,
					       0.5 );
		      gtk_table_attach ( GTK_TABLE ( table_etat ),
					 label,
					 colonne, colonne + 1,
					 ligne, ligne + 1,
					 GTK_SHRINK | GTK_FILL,
					 GTK_SHRINK | GTK_FILL,
					 0, 0 );
		      gtk_widget_show ( label );
		    }
		  colonne++;
		}

	      if ( etat_courant -> afficher_infobd_ope )
		{
		  if ( operation -> info_banque_guichet )
		    {
		      label = gtk_label_new ( operation -> info_banque_guichet );
		      gtk_misc_set_alignment ( GTK_MISC ( label ),
					       0,
					       0.5 );
		      gtk_table_attach ( GTK_TABLE ( table_etat ),
					 label,
					 colonne, colonne + 1,
					 ligne, ligne + 1,
					 GTK_SHRINK | GTK_FILL,
					 GTK_SHRINK | GTK_FILL,
					 0, 0 );
		      gtk_widget_show ( label );
		    }
		  colonne++;
		}



	      /* on affiche le montant au bout de la ligne */

	      label = gtk_label_new ( g_strdup_printf  ("%4.2f",
							operation -> montant ));

	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 label,
				 GTK_TABLE ( table_etat ) -> ncols - 1, GTK_TABLE ( table_etat ) -> ncols,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( label );

	      ligne++;
	    }


	  /* on ajoute les montants */

	  montant_categ = montant_categ + operation -> montant;
	  montant_sous_categ = montant_sous_categ + operation -> montant;
	  montant_ib = montant_ib + operation -> montant;
	  montant_sous_ib = montant_sous_ib + operation -> montant;
	  montant_compte = montant_compte + operation -> montant;
	  montant_tiers = montant_tiers + operation -> montant;
	  total_partie = total_partie + operation -> montant;
	  total_general = total_general + operation -> montant;

	  changement_de_groupe = 0;

	  pointeur_tmp = pointeur_tmp -> next;
	}

      /*   à la fin, on affiche les totaux des dernières lignes */

      if ( ligne )
	{
	  /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

	  ligne = affiche_total_tiers ( table_etat,
					montant_tiers,
					ligne );

	  ligne = affiche_total_compte ( table_etat,
					 montant_compte,
					 ligne );

	  ligne = affiche_total_sous_ib ( table_etat,
					  montant_sous_ib,
					  ligne );

	  ligne = affiche_total_ib ( table_etat,
				     montant_ib,
				     ligne );

	  ligne = affiche_total_sous_categ ( table_etat,
					     montant_sous_categ,
					     ligne );

	  /* on ajoute le total de la categ */

	  ligne = affiche_total_categories ( table_etat,
					     montant_categ,
					     ligne );
	}

      /* on affiche le total de la partie en cours */

      label = gtk_label_new ( "" );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, 2,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;

      separateur = gtk_hseparator_new ();
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 separateur,
			 0, 3,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( separateur );

      ligne++;

      if ( i )
	label = gtk_label_new ( "Total dépenses : " );
      else
	label = gtk_label_new ( "Total revenus : " );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, 2,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						total_partie ));
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 2, 3,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;

      separateur = gtk_hseparator_new ();
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 separateur,
			 0, 3,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( separateur );

      ligne++;

      label = gtk_label_new ( "" );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, 2,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;


    fin_boucle_affichage_etat:
    }

  /* on affiche maintenant le total général */

      label = gtk_label_new ( "" );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, 2,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;

      separateur = gtk_hseparator_new ();
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 separateur,
			 0, 3,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( separateur );

      ligne++;

      label = gtk_label_new ( "Total général : " );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, 2,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						total_general ));
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 2, 3,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;

      separateur = gtk_hseparator_new ();
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 separateur,
			 0, 3,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( separateur );

      ligne++;

      label = gtk_label_new ( "" );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, 2,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;


    }
/*****************************************************************************************************/


/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les catégories sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_categories ( GtkWidget *table_etat,
				gdouble montant_categ,
				gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_categ
       &&
       etat_courant -> affiche_sous_total_categ )
    {
      /* si rien n'est affiché en dessous de la catég, on */
      /* met le résultat sur la ligne de la catég */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> afficher_sous_categ
	   ||
	   etat_courant -> utilise_ib
	   ||
	   etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  label = gtk_label_new ( "Total Catégorie : " );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_categ ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_categ ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}

    }
  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_categ sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_sous_categ ( GtkWidget *table_etat,
				gdouble montant_sous_categ,
				gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_categ
       &&
       etat_courant -> afficher_sous_categ
       &&
       etat_courant -> affiche_sous_total_sous_categ )
    {
      /* si rien n'est affiché en dessous de la sous_categ, on */
      /* met le résultat sur la ligne de la ss categ */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> utilise_ib
	   ||
	   etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  label = gtk_label_new ( "Total Sous-catégories : " );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_sous_categ ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_sous_categ ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_ib ( GtkWidget *table_etat,
			gdouble montant_ib,
			gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_ib
       &&
       etat_courant -> affiche_sous_total_ib )
    {
      /* si rien n'est affiché en dessous de la ib, on */
      /* met le résultat sur la ligne de l'ib */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> afficher_sous_ib
	   ||
	   etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  label = gtk_label_new ( "Total Imputations budgétaires : " );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_ib ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_ib ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_sous_ib ( GtkWidget *table_etat,
			     gdouble montant_sous_ib,
			     gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_ib
       &&
       etat_courant -> afficher_sous_ib
       &&
       etat_courant -> affiche_sous_total_sous_ib )
    {
      /* si rien n'est affiché en dessous de la sous ib, on */
      /* met le résultat sur la ligne de la sous ib */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  label = gtk_label_new ( "Total Sous-imputations budgétaires : " );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_sous_ib ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_sous_ib ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les compte sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_compte ( GtkWidget *table_etat,
			    gdouble montant_compte,
			    gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> regroupe_ope_par_compte
       &&
       etat_courant -> affiche_sous_total_compte )
    {
      /* si rien n'est affiché en dessous du compte, on */
      /* met le résultat sur la ligne du compte */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  label = gtk_label_new ( "Total Compte : " );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_compte ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_compte ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les tiers sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_tiers ( GtkWidget *table_etat,
				gdouble montant_tiers,
				gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_tiers
       &&
       etat_courant -> affiche_sous_total_tiers )
    {
      /* si rien n'est affiché en dessous du tiers, on */
      /* met le résultat sur la ligne du tiers */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> afficher_opes )
	{

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  label = gtk_label_new ( "Total Tiers : " );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_tiers ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, 2,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f",
						    montant_tiers ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  return (ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etat_c_co_i ( GSList *ope_selectionnees )
{




}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etat_i_c_co ( GSList *ope_selectionnees )
{




}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etat_i_co_c ( GSList *ope_selectionnees )
{




}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etat_co_i_c ( GSList *ope_selectionnees )
{




}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etat_co_c_i ( GSList *ope_selectionnees )
{




}
/*****************************************************************************************************/
