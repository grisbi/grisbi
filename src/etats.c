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
  "Catégories -> I.B. -> Tiers",
  "Catégories -> Tiers -> I.B.",
  "I.B. -> Catégories -> Tiers",
  "I.B. -> Tiers -> Catégories",
  "Tiers -> I.B. -> Catégories",
  "Tiers -> Catégories -> I.B.",
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
  no_etat_courant = -1;

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

  bouton = gtk_button_new_with_label ( "Ajouter un rapport ..." );
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

  bouton_effacer_etat = gtk_button_new_with_label ( "Effacer un rapport ..." );
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

      if ( etat -> no_etat == no_etat_courant )
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
  etat -> nom_etat = g_strdup ( "Nouveau rapport" );

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

  dialog = gnome_dialog_new ( "Sélection du rapport à effacer :",
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));

  label = gtk_label_new ( "Choisir le rapport à effacer :" );
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

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu ),
				g_slist_position ( liste_struct_etats,
						   g_slist_find_custom ( liste_struct_etats,
									 GINT_TO_POINTER ( no_etat_courant ),
									 (GCompareFunc) recherche_etat_par_no )));

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( !resultat )
    {
     
      etat = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
				   "adr_etat" );

      /*   si l'état courant était celui qu'on efface, on met l'état courant à -1 et */
      /* le bouton à null, et le label de l'état en cours à rien */

      if ( no_etat_courant == etat -> no_etat )
	{
	  no_etat_courant = -1;
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
  no_etat_courant = etat -> no_etat;
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
  struct struct_etat *etat;
  GtkWidget *dialog;
  gint resultat;
  GtkWidget *notebook;
  gchar *pointeur_char;
  GList *pointeur_liste;

  if ( no_etat_courant == -1 )
    return;

  etat = g_slist_find_custom ( liste_struct_etats,
			       GINT_TO_POINTER ( no_etat_courant ),
			       (GCompareFunc) recherche_etat_par_no ) -> data;



  /* la fenetre affichée est une gnome dialog */

  dialog = gnome_dialog_new ( g_strconcat ( "Personnalisation du rapport financier : ",
					    etat -> nom_etat,
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
			     onglet_etat_generalites (etat),
			     gtk_label_new (" Généralités ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_dates (etat),
			     gtk_label_new (" Dates ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_comptes (etat),
			     gtk_label_new (" Comptes ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_categories (etat),
			     gtk_label_new (" Catégories ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_ib (etat),
			     gtk_label_new (" Imputation budgétaire ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_tiers (etat),
			     gtk_label_new (" Tiers ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_texte (etat),
			     gtk_label_new (" Texte ") );

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			     onglet_etat_montant (etat),
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
		etat -> nom_etat ))
    {
      etat -> nom_etat = g_strdup ( pointeur_char );

      /* on réaffiche la liste des états */

      remplissage_liste_etats ();
    }

  /* récupération du type de classement */

  etat -> type_classement = GPOINTER_TO_INT ( GTK_CLIST ( liste_type_classement_etat ) -> selection -> data);

  /* récupération des dates */

  etat -> exo_date = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ));
  etat -> utilise_detail_exo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat ));

  if ( etat -> no_exercices )
    {
      g_slist_free ( etat -> no_exercices );
      etat -> no_exercices = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_exo_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat -> no_exercices = g_slist_append ( etat -> no_exercices,
					      gtk_clist_get_row_data ( GTK_CLIST ( liste_exo_etat ),
								       GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  etat -> separation_par_exo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ));

  etat -> no_plage_date = GPOINTER_TO_INT ( GTK_CLIST ( liste_plages_dates_etat ) -> selection -> data );

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

      etat -> date_perso_debut = g_date_new_dmy ( jour,
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

      etat -> date_perso_fin = g_date_new_dmy ( jour,
						mois,
						annee );
    }

  etat -> separation_par_plage = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat ));
  etat -> type_separation_plage = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ) -> menu_item ),
									  "type" ));
  etat -> jour_debut_semaine = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_debut_semaine ) -> menu_item ),
									  "jour" ));
  etat -> type_separation_perso = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_type_separe_perso_etat ) -> menu_item ),
									  "type" ));
  etat -> delai_separation_perso = atoi ( gtk_entry_get_text ( GTK_ENTRY ( entree_separe_perso_etat )));

  /* récupération des comptes */

  etat -> utilise_detail_comptes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat ));

  if ( etat -> no_comptes )
    {
      g_slist_free ( etat -> no_comptes );
      etat -> no_comptes = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_comptes_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat -> no_comptes = g_slist_append ( etat -> no_comptes,
					      gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_etat ),
								       GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }


  /*   récupération des catégories */

  etat -> utilise_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_categ_etat ));

  etat -> utilise_detail_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat ));

  if ( etat -> no_categ )
    {
      g_slist_free ( etat -> no_categ );
      etat -> no_categ = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_categ_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat -> no_categ = g_slist_append ( etat -> no_categ,
					  gtk_clist_get_row_data ( GTK_CLIST ( liste_categ_etat ),
								   GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  etat -> inclusion_virements_passif = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat ));
  etat -> inclusion_virements_hors_etat = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat ));
  etat -> pas_inclusion_virements = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements ));


  /*   récupération des ib */

  etat -> utilise_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat ));

  etat -> utilise_detail_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ));

  if ( etat -> no_ib )
    {
      g_slist_free ( etat -> no_ib );
      etat -> no_ib = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_ib_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat -> no_ib = g_slist_append ( etat -> no_ib,
				       gtk_clist_get_row_data ( GTK_CLIST ( liste_ib_etat ),
								GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }



  /*   récupération des tiers */

  etat -> utilise_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat ));

  etat -> utilise_detail_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat ));

  if ( etat -> no_tiers )
    {
      g_slist_free ( etat -> no_tiers );
      etat -> no_tiers = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_tiers_etat ) -> selection;

  while ( pointeur_liste )
    {
      etat -> no_tiers = g_slist_append ( etat -> no_tiers,
					  gtk_clist_get_row_data ( GTK_CLIST ( liste_tiers_etat ),
								   GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  /* récupération du texte */

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_texte_etat )))))
    etat -> texte = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_texte_etat ))));
  else
    etat -> texte = NULL;

  /* récupération du montant */

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_montant_etat )))))
    etat -> montant = g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_montant_etat ))),
				 NULL );
  else
    etat -> montant = 0;


  gnome_dialog_close ( GNOME_DIALOG ( dialog ));
  modification_fichier ( TRUE );
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

  label = gtk_label_new ( "Nom du rapport : " );
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


  /* on met le nom de l'état */

  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_etat ),
		       etat -> nom_etat );

  /* on sélectionne la plage de date */

  gtk_clist_select_row ( GTK_CLIST ( liste_type_classement_etat ),
			 etat -> type_classement,
			 0 );

  return ( widget_retour );
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
  gtk_signal_connect ( GTK_OBJECT (radio_button_utilise_exo ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( changement_choix_date_etat ),
		       NULL );
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

  vbox_utilisation_exo = gtk_vbox_new ( FALSE,
					5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_utilisation_exo ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox_utilisation_exo );
  gtk_widget_show ( vbox_utilisation_exo );

  bouton_detaille_exo_etat = gtk_check_button_new_with_label ( "Détailler les exercices utilisés" );
  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_exo_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_detaille_exo_etat ),
		       NULL );
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
  gtk_signal_connect ( GTK_OBJECT ( liste_exo_etat ),
		       "select-row",
		       GTK_SIGNAL_FUNC ( verifie_separation_exo_possible ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( liste_exo_etat ),
		       "unselect-row",
		       GTK_SIGNAL_FUNC ( verifie_separation_exo_possible ),
		       NULL );
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


  /* on grise/dégrise le bouton de séparation par exo suivant qu'il y ai plusieurs exos sélectionnés */

  if ( etat -> no_exercices
       &&
       g_slist_length ( etat -> no_exercices ) > 1 )
    {
      gtk_widget_set_sensitive ( bouton_separe_exo_etat,
				 TRUE );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ),
				     etat -> separation_par_exo );
    }
  else
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ),
				     FALSE );
      gtk_widget_set_sensitive ( bouton_separe_exo_etat,
				 FALSE );
    }

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
/* vérifie que plusieurs lignes de la liste des exos sont sélectionnées */
/* pour pouvoir séparer les résultats */
/*****************************************************************************************************/

void verifie_separation_exo_possible ( void )
{
  if ( g_list_length ( GTK_CLIST ( liste_exo_etat ) -> selection ) > 1 )
    gtk_widget_set_sensitive ( bouton_separe_exo_etat,
			       TRUE );
  else
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ),
				     FALSE );
      gtk_widget_set_sensitive ( bouton_separe_exo_etat,
				 FALSE );
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void changement_choix_date_etat ( void )
{

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo )))
    {
      gtk_widget_set_sensitive ( vbox_utilisation_exo,
				 TRUE );
      gtk_widget_set_sensitive ( vbox_utilisation_date,
				 FALSE );
    }
  else
    {
      gtk_widget_set_sensitive ( vbox_utilisation_exo,
				 FALSE );
      gtk_widget_set_sensitive ( vbox_utilisation_date,
				 TRUE );
    }
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
void click_detaille_exo_etat ( void )
{
  gtk_widget_set_sensitive ( vbox_generale_exo_etat,
			     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat )));
}
/*****************************************************************************************************/


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
  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_comptes_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_detaille_comptes_etat ),
		       NULL );
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


  label = gtk_label_new ( "Sélectionner les comptes à inclure dans le rapport :" );
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

  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void click_detaille_comptes_etat ( void )
{
  gtk_widget_set_sensitive ( vbox_generale_comptes_etat,
			     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat )));
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

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  bouton_utilise_categ_etat = gtk_check_button_new_with_label ( "Utiliser les catégories dans le rapport" );
  gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_categ_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_utilise_categ_etat ),
		       NULL );
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
  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_categ_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_detaille_categ_etat ),
		       NULL );
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

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Sélectionner les catégories à inclure dans le rapport :" );
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

  /*   en dessous, on met les boutons d'inclusion ou non des virements */

  bouton_inclusion_virements_actifs_etat = gtk_check_button_new_with_label ( "Inclure les virements de ou vers les comptes d'actif et de passif" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_inclusion_virements_actifs_etat,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclusion_virements_actifs_etat );

  bouton_inclusion_virements_hors_etat = gtk_check_button_new_with_label ( "Inclure les virements de ou vers les comptes ne figurant pas dans le rapport" );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       bouton_inclusion_virements_hors_etat,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclusion_virements_hors_etat );

  bouton_non_inclusion_virements = gtk_check_button_new_with_label ( "Ne pas inclure les virements" );
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


  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat ),
				 etat -> inclusion_virements_passif );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat ),
				 etat -> inclusion_virements_hors_etat );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements ),
				 etat -> pas_inclusion_virements );


  return ( widget_retour );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void click_utilise_categ_etat ( void )
{
    gtk_widget_set_sensitive ( vbox_generale_categ_etat,
			       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_categ_etat )));
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void click_detaille_categ_etat ( void )
{
    gtk_widget_set_sensitive ( hbox_detaille_categ_etat,
			       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat )));
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

  bouton_utilise_ib_etat = gtk_check_button_new_with_label ( "Utiliser les imputations budgétaires dans le rapport" );
  gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_ib_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_utilise_ib_etat ),
		       NULL );
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
  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_ib_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_detaille_ib_etat ),
		       NULL );
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

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Sélectionner les imputations budgétaires à inclure dans le rapport :" );
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

  return ( widget_retour );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void click_utilise_ib_etat ( void )
{
    gtk_widget_set_sensitive ( vbox_generale_ib_etat,
			       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat )));
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void click_detaille_ib_etat ( void )
{
    gtk_widget_set_sensitive ( hbox_detaille_ib_etat,
			       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat )));
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
  GtkWidget *label;
  GSList *pointeur_liste;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;

  widget_retour = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( widget_retour ),
				   10 );
  gtk_widget_show ( widget_retour );

  bouton_utilise_tiers_etat = gtk_check_button_new_with_label ( "Utiliser les tiers dans le rapport" );
  gtk_signal_connect ( GTK_OBJECT ( bouton_utilise_tiers_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_utilise_tiers_etat ),
		       NULL );
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
  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_tiers_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( click_detaille_tiers_etat ),
		       NULL );
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

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Sélectionner les tiers à inclure dans le rapport :" );
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

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
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
			      GTK_OBJECT  ( liste_tiers_etat ));
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
			      GTK_OBJECT  ( liste_tiers_etat ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

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
    

  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void click_utilise_tiers_etat ( void )
{
    gtk_widget_set_sensitive ( vbox_generale_tiers_etat,
			       gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat )));
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void click_detaille_tiers_etat ( void )
{
  gtk_widget_set_sensitive ( hbox_detaille_tiers_etat,
			     gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat )));
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
