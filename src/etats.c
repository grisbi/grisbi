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
  GtkWidget *bouton;

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

  /* on met le bouton rafraichir */

  bouton = gtk_button_new_with_label ( "Rafraichir" );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
			      "clicked",
			      GTK_SIGNAL_FUNC ( affichage_etat ),
			      NULL );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );



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

	  if ( GTK_BIN ( scrolled_window_etat ) -> child )
	    gtk_container_remove ( GTK_CONTAINER ( scrolled_window_etat ),
				   GTK_BIN ( scrolled_window_etat ) -> child );
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
  gint i;

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

  /* récupération du type de classement */

  g_list_free ( etat_courant -> type_classement );

  etat_courant -> type_classement = NULL;

  for ( i=0 ; i<GTK_CLIST ( liste_type_classement_etat ) -> rows ; i++ )
    {
      gint no;

      no = GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( liste_type_classement_etat ),
							   gtk_ctree_node_nth ( GTK_CTREE ( liste_type_classement_etat ),
										i )));

      etat_courant -> type_classement = g_list_append ( etat_courant -> type_classement,
							GINT_TO_POINTER ( no ));

      /* rajoute les ss categ et ss ib */

      if ( no == 1 )
	etat_courant -> type_classement = g_list_append ( etat_courant -> type_classement,
							  GINT_TO_POINTER ( 2 ));
      if ( no == 3 )
	etat_courant -> type_classement = g_list_append ( etat_courant -> type_classement,
							  GINT_TO_POINTER ( 4 ));
    }

  /* récupération de l'affichage des opés */

  etat_courant -> afficher_opes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ));

  etat_courant -> afficher_no_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_ope ));
  etat_courant -> afficher_date_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ));
  etat_courant -> afficher_tiers_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ));
  etat_courant -> afficher_categ_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ));
  etat_courant -> afficher_sous_categ_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ));
  etat_courant -> afficher_type_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_type_ope ));
  etat_courant -> afficher_ib_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ));
  etat_courant -> afficher_sous_ib_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ));
  etat_courant -> afficher_cheque_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_cheque ));
  etat_courant -> afficher_notes_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ));
  etat_courant -> afficher_pc_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ));
  etat_courant -> afficher_rappr_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_rappr ));
  etat_courant -> afficher_infobd_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ));
  etat_courant -> pas_detailler_ventilation = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation ));

  etat_courant -> devise_de_calcul_general = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_general_etat ) -> menu_item ),
										   "no_devise" ));
  /* récupération des dates */

  etat_courant -> exo_date = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ));
  etat_courant -> utilise_detail_exo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat ));

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
  
  etat_courant -> exclure_ope_sans_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_categ ));
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

  etat_courant -> devise_de_calcul_categ = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_categ_etat ) -> menu_item ),
										   "no_devise" ));

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
  
  etat_courant -> exclure_ope_sans_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_ib ));
  etat_courant -> affiche_sous_total_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ));
  etat_courant -> affiche_sous_total_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ));

  etat_courant -> devise_de_calcul_ib = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_ib_etat ) -> menu_item ),
										   "no_devise" ));



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

  etat_courant -> devise_de_calcul_tiers = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ) -> menu_item ),
										   "no_devise" ));


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
  GtkWidget *frame;
  GtkWidget *table;
  GList *pointeur_liste;
  GtkCTreeNode *parent;
  GtkWidget *vbox;
  GtkWidget *fleche;

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

  frame = gtk_frame_new ( "Choix du type de classement :" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( hbox ),
				   5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      hbox );
  gtk_widget_show ( hbox );

  scrolled_window = gtk_scrolled_window_new ( FALSE,
					      FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_set_usize ( scrolled_window,
			 200,
			 100 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       scrolled_window,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( scrolled_window );


  liste_type_classement_etat = gtk_ctree_new ( 1,
					       0 );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_type_classement_etat ),
				     0,
				     TRUE );
  gtk_ctree_set_line_style ( GTK_CTREE ( liste_type_classement_etat ),
			     GTK_CTREE_LINES_NONE );
  gtk_ctree_set_expander_style ( GTK_CTREE ( liste_type_classement_etat ),
				 GTK_CTREE_EXPANDER_NONE );

  gtk_signal_connect ( GTK_OBJECT ( liste_type_classement_etat ),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( click_liste_etat ),
		       GINT_TO_POINTER (1) );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_type_classement_etat );
  gtk_widget_show ( liste_type_classement_etat );

  /* on place ici les flèches sur le côté de la liste */

  vbox = gtk_vbutton_box_new ();

  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       FALSE,
		       FALSE,
		       0);

  fleche = gnome_stock_button ( GNOME_STOCK_BUTTON_UP );
  gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( fleche ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( click_haut_classement_etat ),
		       NULL );
  gtk_container_add ( GTK_CONTAINER ( vbox ),
		      fleche  );

  fleche = gnome_stock_button ( GNOME_STOCK_BUTTON_DOWN );
  gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( fleche ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( click_bas_classement_etat ),
		       NULL);
  gtk_container_add ( GTK_CONTAINER ( vbox ),
		      fleche  );
  gtk_widget_show_all ( vbox );


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
			  3,
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

  bouton_afficher_no_ope = gtk_check_button_new_with_label ( "le n° d'opération" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_no_ope,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_no_ope );

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      1, 2,
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
			      2, 3,
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
			      2, 3,
			      1, 2 );
  gtk_widget_show ( hbox );

  bouton_afficher_type_ope = gtk_check_button_new_with_label ( "le type d'opération" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_type_ope,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_type_ope );

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
			      2, 3,
			      2, 3 );
  gtk_widget_show ( hbox );

  bouton_afficher_no_cheque = gtk_check_button_new_with_label ( "le n° de chèque/virement" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_no_cheque,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_no_cheque );

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
			      2, 3,
			      3, 4 );
  gtk_widget_show ( hbox );

  bouton_afficher_no_rappr = gtk_check_button_new_with_label ( "le n° de rapprochement" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_no_rappr,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_no_rappr );

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


  bouton_pas_detailler_ventilation = gtk_check_button_new_with_label ( "Ne pas détailler les opérations ventilées" );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       bouton_pas_detailler_ventilation,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_pas_detailler_ventilation );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( widget_retour ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Devise utilisée pour les totaux généraux :" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_devise_general_etat = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_general_etat ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_devise_general_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_devise_general_etat );




  /* on met le nom de l'état */

  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_etat ),
		       etat -> nom_etat );

  /* on remplit le ctree en fonction du classement courant */

  pointeur_liste = etat -> type_classement;
  parent = NULL;

  while ( pointeur_liste )
    {
      gchar *text[1];

      text[0] = NULL;

      switch ( GPOINTER_TO_INT ( pointeur_liste -> data ))
	{
	case 1:
	  text[0] = "Catégorie";
	  break;

	case 3:
	  text[0] = "Imputation budgétaire";
	  break;

	case 5:
	  text[0] = "Compte";
	  break;

	case 6:
	  text[0] = "Tiers";
	  break;

	default:
	}

      if ( text[0] )
	{
	  parent = gtk_ctree_insert_node ( GTK_CTREE ( liste_type_classement_etat ),
					   parent,
					   NULL,
					   text,
					   5,
					   NULL,
					   NULL,
					   NULL,
					   NULL,
					   FALSE,
					   TRUE );
	  gtk_ctree_node_set_row_data ( GTK_CTREE ( liste_type_classement_etat ),
					GTK_CTREE_NODE ( parent ),
					pointeur_liste -> data );
	}

      pointeur_liste = pointeur_liste -> next;
    }

  gtk_clist_select_row ( GTK_CLIST ( liste_type_classement_etat ),
			 0,
			 0 );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ),
				 etat -> afficher_opes );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_ope ),
				 etat -> afficher_no_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ),
				 etat -> afficher_date_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ),
				 etat -> afficher_tiers_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ),
				 etat -> afficher_categ_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ),
				 etat -> afficher_sous_categ_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_type_ope ),
				 etat -> afficher_type_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ),
				 etat -> afficher_ib_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ),
				 etat -> afficher_sous_ib_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_cheque ),
				 etat -> afficher_cheque_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ),
				 etat -> afficher_notes_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ),
				 etat -> afficher_pc_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_rappr ),
				 etat -> afficher_rappr_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ),
				 etat -> afficher_infobd_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation ),
				 etat -> pas_detailler_ventilation );

  /* on rend insensitif les sous qque choses si nécessaire */

  sens_desensitive_pointeur ( bouton_afficher_opes,
			      frame );
  sens_desensitive_pointeur ( bouton_afficher_categ_opes,
			      bouton_afficher_sous_categ_opes );
  sens_desensitive_pointeur ( bouton_afficher_ib_opes,
			      bouton_afficher_sous_ib_opes );

  /* mise en forme de la devise */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_general_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat -> devise_de_calcul_general ),
									 ( GCompareFunc ) recherche_devise_par_no )));

  return ( widget_retour );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void click_haut_classement_etat ( void )
{
  GtkCTreeNode *node_parent;
  GtkCTreeNode *node;
  GtkCTreeNode *node_enfant;
  GtkCTreeNode *nouveau_parent;

  node = GTK_CLIST ( liste_type_classement_etat )->selection -> data;

  /*   si on est au niveau 1, peut pas plus haut */

  if ( GTK_CTREE_ROW ( node ) -> level == 1 )
    return;

  node_parent = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->parent;
  node_enfant = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->children;

  nouveau_parent = GTK_CTREE_ROW ( node_parent )->parent;

  /* on remonte le node */

  gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		   node,
		   nouveau_parent,
		   NULL );

  /* on descend celui du dessus */

  gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		   node_parent,
		   node,
		   NULL );

  /* on attache l'enfant à son nouveau parent */

  if ( node_enfant )
    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node_enfant,
		     node_parent,
		     NULL );

  gtk_ctree_expand_recursive ( GTK_CTREE ( liste_type_classement_etat ),
			       node );

}
/*****************************************************************************************************/


/*****************************************************************************************************/
void click_bas_classement_etat ( void )
{
  GtkCTreeNode *node_parent;
  GtkCTreeNode *node;
  GtkCTreeNode *node_enfant;
  GtkCTreeNode *node_enfant_enfant;

  node = GTK_CLIST ( liste_type_classement_etat )->selection -> data;

  /*   si on est au niveau 4, peut pas plus bas */

  if ( GTK_CTREE_ROW ( node ) -> level == 4 )
    return;

  node_parent = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->parent;
  node_enfant = GTK_CTREE_ROW ( GTK_CLIST ( liste_type_classement_etat )->selection -> data )->children;

  node_enfant_enfant = GTK_CTREE_ROW ( node_enfant )->children;

  /* on remonte le node enfant */

  gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		   node_enfant,
		   node_parent,
		   NULL );

  /* on descend le node */

  gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		   node,
		   node_enfant,
		   NULL );

  /* on attache l'enfant de l'enfant à son nouveau parent */

  if ( node_enfant_enfant )
    gtk_ctree_move ( GTK_CTREE ( liste_type_classement_etat ),
		     node_enfant_enfant,
		     node,
		     NULL );

  gtk_ctree_expand_recursive ( GTK_CTREE ( liste_type_classement_etat ),
			       node );

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
  GtkWidget *hbox;

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

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_categ_etat ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );



  bouton_exclure_ope_sans_categ = gtk_check_button_new_with_label ( "Exclure les opérations sans catégorie" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_exclure_ope_sans_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exclure_ope_sans_categ );

  bouton_affiche_sous_total_categ = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement de catégorie" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_categ );

  /* mise en place du bouton pour afficher les sous categ */

  bouton_afficher_sous_categ = gtk_check_button_new_with_label ( "Afficher les sous-catégories" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_sous_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_categ );

  bouton_affiche_sous_total_sous_categ = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement de sous-catégorie" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_sous_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_sous_categ );

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_categ ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_sous_categ );


  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Devise utilisée pour le calcul :" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_devise_categ_etat = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_devise_categ_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_devise_categ_etat );




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

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_categ ),
				 etat -> exclure_ope_sans_categ );
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

  /* mise en forme de la devise */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat -> devise_de_calcul_categ ),
									 ( GCompareFunc ) recherche_devise_par_no )));


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
  GtkWidget *hbox;
  GtkWidget *separateur;

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

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_generale_ib_etat ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  bouton_exclure_ope_sans_ib = gtk_check_button_new_with_label ( "Exclure les opérations sans imputation budgétaire" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_exclure_ope_sans_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exclure_ope_sans_ib );

  bouton_affiche_sous_total_ib = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement d'imputation" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_ib );

  /* mise en place du bouton pour afficher les sous categ */

  bouton_afficher_sous_ib = gtk_check_button_new_with_label ( "Afficher les sous-imputations" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_sous_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_ib );

  bouton_affiche_sous_total_sous_ib = gtk_check_button_new_with_label ( "Afficher un sous-total lors du changement de sous-imputation" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_sous_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_sous_ib );

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_ib ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_sous_ib );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Devise utilisée pour le calcul :" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_devise_ib_etat = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_devise_ib_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_devise_ib_etat );


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

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_ib ),
				 etat -> exclure_ope_sans_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ),
				 etat -> affiche_sous_total_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib ),
				 etat -> afficher_sous_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ),
				 etat -> affiche_sous_total_sous_ib );
  sens_desensitive_pointeur ( bouton_afficher_sous_ib,
			      bouton_affiche_sous_total_sous_ib );

  /* mise en forme de la devise */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat -> devise_de_calcul_ib ),
									 ( GCompareFunc ) recherche_devise_par_no )));

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
  GtkWidget *hbox;

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

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( "Devise utilisée pour le calcul :" );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_devise_tiers_etat = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_devise_tiers_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_devise_tiers_etat );



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

  /* mise en forme de la devise */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat -> devise_de_calcul_tiers ),
									 ( GCompareFunc ) recherche_devise_par_no )));

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


  if ( !etat )
    {
      if ( etat_courant )
	etat = etat_courant;
      else
	return;
    }

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

	      /* si c'est une opé ventilée, dépend de la conf */

	      if ( operation -> operation_ventilee
		   &&
		   !etat -> pas_detailler_ventilation )
		goto operation_refusee;

	      if ( operation -> no_operation_ventilee_associee
		   &&
		   etat -> pas_detailler_ventilation )
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
 
		      if ((( etat -> utilise_detail_categ
			     &&
			     g_slist_index ( etat -> no_categ,
					     GINT_TO_POINTER ( operation -> categorie )) == -1 )
			   ||
			   ( etat -> exclure_ope_sans_categ
			     &&
			     !operation -> categorie ))
			  &&
			  !operation -> operation_ventilee )
			goto operation_refusee;
		    }
		}

	      /* vérification de l'imputation budgétaire */

	      if (( etat -> utilise_ib
		    &&
		    etat -> utilise_detail_ib
		    &&
		    g_slist_index ( etat -> no_ib,
				    GINT_TO_POINTER ( operation -> imputation )) == -1 )
		  ||
		  ( etat -> exclure_ope_sans_ib
		    &&
		    !operation -> imputation ))
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


  /* on classe la liste et l'affiche en fonction du choix du type de classement */

  etape_finale_affichage_etat ( liste_opes_selectionnees );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* Fonction de classement de la liste */
/* en fonction du choix du type de classement */
/*****************************************************************************************************/

gint classement_liste_opes_etat ( struct structure_operation *operation_1,
				  struct structure_operation *operation_2 )
{
  GList *pointeur;

  pointeur = etat_courant -> type_classement;

 classement_suivant:

  /*   si pointeur est nul, on a fait le tour du classement, les opés sont identiques */
  /* on classe par date, et si pareil, par no d'opé */

  if ( !pointeur )
    {
      if ( g_date_compare ( operation_1 -> date,
			    operation_2 -> date ))
	return ( g_date_compare ( operation_1 -> date,
				  operation_2 -> date ));

      /*       les dates sont identiques, on classe par no d'opé */

      return ( operation_1 -> no_operation - operation_2 -> no_operation );
    }


  switch ( GPOINTER_TO_INT ( pointeur -> data ))
    {
      /* classement des catégories */

    case 1:

      if ( operation_1 -> categorie != operation_2 -> categorie )
	return ( operation_1 -> categorie - operation_2 -> categorie );

      /*     si  les catégories sont nulles, on doit départager entre virements, pas */
      /* de categ ou opé ventilée */
      /* on met en 1er les opés sans categ, ensuite les ventilations et enfin les virements */

      if ( !operation_1 -> categorie )
	{
	  if ( operation_1 -> operation_ventilee )
	    {
	      if ( operation_2 -> relation_no_operation )
		return ( -1 );
	      else
		if ( !operation_2 -> operation_ventilee )
		  return ( 1 );
	    }
	  else
	    {
	      if ( operation_1 -> relation_no_operation )
		{
		  if ( !operation_2 -> relation_no_operation )
		    return ( 1 );
		}
	      else
		if ( operation_2 -> relation_no_operation
		     ||
		     operation_2 -> operation_ventilee )
		  return ( -1 );
	    }
	}

      /*       les catégories sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;

      /* classement des sous catégories */

    case 2:

      if ( operation_1 -> sous_categorie != operation_2 -> sous_categorie )
	return ( operation_1 -> sous_categorie - operation_2 -> sous_categorie );

      /*       les ss-catégories sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;

      /* classement des ib */

    case 3:

      if ( operation_1 -> imputation != operation_2 -> imputation )
	return ( operation_1 -> imputation - operation_2 -> imputation );

      /*       les ib sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;

      /* classement des sous ib */

    case 4:

      if ( operation_1 -> sous_imputation != operation_2 -> sous_imputation )
	return ( operation_1 -> sous_imputation - operation_2 -> sous_imputation );

      /*       les ib sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;

      break;


      /* classement des comptes */

    case 5:

      if ( operation_1 -> no_compte != operation_2 -> no_compte )
	return ( operation_1 ->no_compte  - operation_2 -> no_compte );

      /*       les comptes sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;
      break;


      /* classement des tiers */

    case 6:

      if ( operation_1 -> tiers != operation_2 -> tiers )
	return ( operation_1 ->tiers  - operation_2 -> tiers );

      /*       les tiers sont identiques, passe au classement suivant */

      pointeur = pointeur -> next;
      goto classement_suivant;
      break;
    }
  return (0);
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void etape_finale_affichage_etat ( GSList *ope_selectionnees )
{
  /* on fait un affichage catég -> ib -> compte */

  GSList *liste_ope_revenus;
  GSList *liste_ope_depenses;
  GSList *pointeur_tmp;
  gint i;
  GtkWidget *table_etat;
  gint ligne;
  GtkWidget *label;
  gdouble total_partie;
  gdouble total_general;
  GtkWidget *separateur;
  gchar *decalage_base;
  gchar *decalage_categ;
  gchar *decalage_sous_categ;
  gchar *decalage_ib;
  gchar *decalage_sous_ib;
  gchar *decalage_compte;
  gchar *decalage_tiers;
  gint nb_colonnes;
  GList *pointeur_glist;


  /* on commence par séparer la liste revenus et de dépenses */
  /*   si le classement racine est la catégorie, on sépare par catégorie */
  /* de revenu ou de dépense */
  /* si c'est un autre, on sépare par montant positif ou négatif */

  pointeur_glist = etat_courant -> type_classement;

  liste_ope_revenus = NULL;
  liste_ope_depenses = NULL;
  pointeur_tmp = ope_selectionnees;

  while ( pointeur_tmp )
    {
      struct structure_operation *operation;

      operation = pointeur_tmp -> data;

      if ( GPOINTER_TO_INT ( pointeur_glist -> data ) == 1 )
	{
	  /* le classement racine est la catégorie */
	  /* s'il n'y a pas de catég, c'est un virement ou une ventilation */
	  /*       dans ce cas, on classe en fonction du montant */

	  if ( operation -> categorie )
	    {
	      struct struct_categ *categ;

	      categ = g_slist_find_custom ( liste_struct_categories,
					    GINT_TO_POINTER ( operation -> categorie ),
					    (GCompareFunc) recherche_categorie_par_no ) -> data;

	      if ( categ -> type_categ )
		liste_ope_depenses = g_slist_append ( liste_ope_depenses,
						      operation );
	      else
		liste_ope_revenus = g_slist_append ( liste_ope_revenus,
						     operation );
	    }
	  else
	    {
	      if ( operation -> montant < 0 )
		liste_ope_depenses = g_slist_append ( liste_ope_depenses,
						      operation );
	      else
		liste_ope_revenus = g_slist_append ( liste_ope_revenus,
						     operation );
	    }
	}
      else
	{
	  /* le classement racine n'est pas la catég, on sépare en fonction du montant */

	  if ( operation -> montant < 0 )
	    liste_ope_depenses = g_slist_append ( liste_ope_depenses,
						  operation );
	  else
	    liste_ope_revenus = g_slist_append ( liste_ope_revenus,
						 operation );
	}
      pointeur_tmp = pointeur_tmp -> next;
    }

  /* on va maintenant classer ces 2 listes dans l'ordre adéquat */

  liste_ope_depenses = g_slist_sort ( liste_ope_depenses,
				      (GCompareFunc) classement_liste_opes_etat );
  liste_ope_revenus = g_slist_sort ( liste_ope_revenus,
				     (GCompareFunc) classement_liste_opes_etat );

  
  /* calcul du décalage pour chaque classement */
  /* c'est une chaine vide qu'on ajoute devant le nom du */
  /*   classement ( tiers, ib ...) */
 
  /* on met 2 espaces par décalage */
  /*   normalement, pointeur_glist est déjà positionné */

  decalage_base = "";

  /*   pour éviter le warning lors de la compilation, on met */
  /* toutes les var char à "" */

  decalage_categ = "";
  decalage_sous_categ = "";
  decalage_ib = "";
  decalage_sous_ib = "";
  decalage_compte = "";
  decalage_tiers = "";

  while ( pointeur_glist )
    {
      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	  /* décalage de la catégorie */

	case 1:
	  if ( etat_courant -> utilise_categ )
	    decalage_categ = g_strconcat ( decalage_base,
					   "    ",
					   NULL );
	  else
	    {
	      decalage_categ = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage de la ss-catégorie */

	case 2:
	  if ( etat_courant -> utilise_categ
	       &&
	       etat_courant -> afficher_sous_categ )
	    decalage_sous_categ = g_strconcat ( decalage_base,
						"    ",
						NULL );
	  else
	    {
	      decalage_sous_categ = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage de l'ib */

	case 3:
	  if ( etat_courant -> utilise_ib )
	    decalage_ib = g_strconcat ( decalage_base,
					"    ",
					NULL );
	  else
	    {
	      decalage_ib = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage de la ss-ib */

	case 4:
	  if ( etat_courant -> utilise_ib
	       &&
	       etat_courant -> afficher_sous_ib )
	    decalage_sous_ib = g_strconcat ( decalage_base,
					     "    ",
					     NULL );
	  else
	    {
	      decalage_sous_ib = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage du compte */

	case 5:
	  if ( etat_courant -> regroupe_ope_par_compte )
	    decalage_compte = g_strconcat ( decalage_base,
					    "    ",
					    NULL );
	  else
	    {
	      decalage_compte = decalage_base;
	      goto pas_decalage;
	    }
	  break;

	  /* décalage du tiers */

	case 6:
	  if ( etat_courant -> utilise_tiers )
	    decalage_tiers = g_strconcat ( decalage_base,
					   "    ",
					   NULL );
	  else
	    {
	      decalage_tiers = decalage_base;
	      goto pas_decalage;
	    }
	  break;
	}

      decalage_base = g_strconcat ( decalage_base,
				    "    ",
				    NULL );

    pas_decalage:
      pointeur_glist = pointeur_glist -> next;
    }



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
      nb_colonnes = nb_colonnes + etat_courant -> afficher_no_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_type_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_cheque_ope;
      nb_colonnes = nb_colonnes + etat_courant -> afficher_rappr_ope;
    }


  /* on peut maintenant créer la table */
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
      ancienne_categ_etat = -1;
      ancienne_categ_speciale_etat = 0;
      ancienne_sous_categ_etat = -1;
      ancienne_ib_etat = -1;
      ancienne_sous_ib_etat = -1;
      ancien_compte_etat = -1;
      ancien_tiers_etat = -1;

      montant_categ_etat = 0;
      montant_sous_categ_etat = 0;
      montant_ib_etat = 0;
      montant_sous_ib_etat = 0;
      montant_compte_etat = 0;
      montant_tiers_etat = 0;
      total_partie = 0;

      changement_de_groupe_etat = 0;
      debut_affichage_etat = 1;
      devise_compte_en_cours_etat = NULL;

      /*       on met directement les adr des devises de categ, ib et tiers en global pour */
      /* gagner de la vitesse */

      devise_categ_etat = g_slist_find_custom ( liste_struct_devises,
						GINT_TO_POINTER ( etat_courant -> devise_de_calcul_categ ),
						( GCompareFunc ) recherche_devise_par_no) -> data;

      devise_ib_etat = g_slist_find_custom ( liste_struct_devises,
						GINT_TO_POINTER ( etat_courant -> devise_de_calcul_ib ),
						( GCompareFunc ) recherche_devise_par_no) -> data;

      devise_tiers_etat = g_slist_find_custom ( liste_struct_devises,
						GINT_TO_POINTER ( etat_courant -> devise_de_calcul_tiers ),
						( GCompareFunc ) recherche_devise_par_no) -> data;

      devise_generale_etat = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( etat_courant -> devise_de_calcul_general ),
						   ( GCompareFunc ) recherche_devise_par_no) -> data;

      /* on met ici le pointeur sur les revenus ou sur les dépenses */
      /* en vérifiant qu'il y en a */

      if ( !i )
	{
	  /* on met le pointeur sur les revenus */

	  if ( liste_ope_revenus )
	    {
	      pointeur_tmp = liste_ope_revenus;

	      ligne = affiche_titre_revenus_etat ( table_etat,
						   ligne );
	    }
	  else
	    {
	      /* il n'y a pas de revenus, on saute directement aux dépenses */

	      i++;
	      pointeur_tmp = liste_ope_depenses;

	      ligne = affiche_titre_depenses_etat ( table_etat,
						    ligne );
	    }
	}
      else
	{
	  /* on met le pointeur sur les dépenses */

	  if ( liste_ope_depenses )
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

	      pointeur_tmp = liste_ope_depenses;

	      ligne = affiche_titre_depenses_etat ( table_etat,
						    ligne );
	    }
	  else
	    goto fin_boucle_affichage_etat;
	}


      /* on commence la boucle qui fait le tour de chaque opé */

      while ( pointeur_tmp )
	{
	  struct structure_operation *operation;
	  struct struct_devise *devise_operation;
	  gdouble montant;

	  operation = pointeur_tmp -> data;

	  pointeur_glist = etat_courant -> type_classement;

	  while ( pointeur_glist )
	    {
	      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
		{
		case 1:
		  ligne = affiche_categ_etat ( operation,
					       table_etat,
					       decalage_categ,
					       ligne );
		  break;

		case 2:
		  ligne = affiche_sous_categ_etat ( operation,
						    table_etat,
						    decalage_sous_categ,
						    ligne );

		  break;

		case 3:
		  ligne = affiche_ib_etat ( operation,
					    table_etat,
					    decalage_ib,
					    ligne );

		  break;

		case 4:
		  ligne = affiche_sous_ib_etat ( operation,
						 table_etat,
						 decalage_sous_ib,
						 ligne );

		  break;

		case 5:
		  ligne = affiche_compte_etat ( operation,
						table_etat,
						decalage_compte,
						ligne );

		  break;

		case 6:
		  ligne = affiche_tiers_etat ( operation,
					       table_etat,
					       decalage_tiers,
					       ligne );
		}

	      pointeur_glist = pointeur_glist -> next;
	    }


	  ligne = affichage_ligne_ope ( table_etat,
					operation,
					ligne );

	  /* on ajoute les montants que pour ceux affichés */

	  /* calcule le montant de la categ */

	  if ( etat_courant -> utilise_categ )
	    {
	      if ( operation -> devise == devise_categ_etat -> no_devise )
		montant = operation -> montant;
	      else
		{
		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_categ_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, "Euro" ) )
		    montant = operation -> montant * devise_categ_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_categ_etat -> nom_devise, "Euro" ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_categ_etat = montant_categ_etat + montant;
	      montant_sous_categ_etat = montant_sous_categ_etat + montant;
	    }

	  /* calcule le montant de l'ib */
	  
	  if ( etat_courant -> utilise_ib )
	    {
	      if ( operation -> devise == devise_ib_etat -> no_devise )
		montant = operation -> montant;
	      else
		{
		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_ib_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, "Euro" ) )
		    montant = operation -> montant * devise_ib_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_ib_etat -> nom_devise, "Euro" ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_ib_etat = montant_ib_etat + montant;
	      montant_sous_ib_etat = montant_sous_ib_etat + montant;
	    }

	  /* calcule le montant du tiers */

	  if ( etat_courant -> utilise_tiers )
	    {
	      if ( operation -> devise == devise_tiers_etat -> no_devise )
		montant = operation -> montant;
	      else
		{
		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_tiers_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, "Euro" ) )
		    montant = operation -> montant * devise_tiers_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_tiers_etat -> nom_devise, "Euro" ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_tiers_etat = montant_tiers_etat + montant;
	    }

	  /* calcule le montant du compte */

	  if ( etat_courant -> affiche_sous_total_compte )
	    {
	      /* on modifie le montant s'il n'est pas de la devise du compte en cours */

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	      if ( !devise_compte_en_cours_etat
		   ||
		   DEVISE != devise_compte_en_cours_etat -> no_devise )
		devise_compte_en_cours_etat = g_slist_find_custom ( liste_struct_devises,
								    GINT_TO_POINTER ( DEVISE ),
								    ( GCompareFunc ) recherche_devise_par_no) -> data;

	      if ( operation -> devise == DEVISE )
		montant = operation -> montant;
	      else
		{
		  /* ce n'est pas la devise du compte, si le compte passe à l'euro et que la devise est l'euro, */
		  /* utilise la conversion du compte, */
		  /* si c'est une devise qui passe à l'euro et que la devise du compte est l'euro, utilise la conversion du compte */
		  /* sinon utilise la conversion stockée dans l'opé */

		  devise_operation = g_slist_find_custom ( liste_struct_devises,
							   GINT_TO_POINTER ( operation -> devise ),
							   ( GCompareFunc ) recherche_devise_par_no ) -> data;

		  if ( devise_compte_en_cours_etat -> passage_euro
		       &&
		       !strcmp ( devise_operation -> nom_devise, "Euro" ) )
		    montant = operation -> montant * devise_compte_en_cours_etat -> change;
		  else
		    if ( devise_operation -> passage_euro
			 &&
			 !strcmp ( devise_compte_en_cours_etat -> nom_devise, "Euro" ))
		      montant = operation -> montant / devise_operation -> change;
		    else
		      if ( operation -> une_devise_compte_egale_x_devise_ope )
			montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		      else
			montant = operation -> montant * operation -> taux_change - operation -> frais_change;

		  montant = ( rint (montant * 100 )) / 100;
		}
	      montant_compte_etat = montant_compte_etat + montant;
	    }

	  /* calcule les montants totaux */

	  if ( operation -> devise == devise_generale_etat -> no_devise )
	    montant = operation -> montant;
	  else
	    {
	      devise_operation = g_slist_find_custom ( liste_struct_devises,
						       GINT_TO_POINTER ( operation -> devise ),
						       ( GCompareFunc ) recherche_devise_par_no ) -> data;

	      if ( devise_generale_etat -> passage_euro
		   &&
		   !strcmp ( devise_operation -> nom_devise, "Euro" ) )
		montant = operation -> montant * devise_generale_etat -> change;
	      else
		if ( devise_operation -> passage_euro
		     &&
		     !strcmp ( devise_generale_etat -> nom_devise, "Euro" ))
		  montant = operation -> montant / devise_operation -> change;
		else
		  if ( operation -> une_devise_compte_egale_x_devise_ope )
		    montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		  else
		    montant = operation -> montant * operation -> taux_change - operation -> frais_change;

	      montant = ( rint (montant * 100 )) / 100;
	    }


	  total_partie = total_partie + montant;
	  total_general = total_general + montant;

	  changement_de_groupe_etat = 0;

	  pointeur_tmp = pointeur_tmp -> next;
	}

      /*   à la fin, on affiche les totaux des dernières lignes */

      ligne = affiche_totaux_sous_jaccent ( table_etat,
					    GPOINTER_TO_INT ( etat_courant -> type_classement -> data ),
					    ligne );

					    
      /* on ajoute le total de la structure racine */


       switch ( GPOINTER_TO_INT ( etat_courant -> type_classement -> data ))
	{
	case 1:
	  ligne = affiche_total_categories ( table_etat,
					     ligne );
	  break;

	case 2:
	  ligne = affiche_total_sous_categ ( table_etat,
					     ligne );
	  break;

	case 3:
	  ligne = affiche_total_ib ( table_etat,
				     ligne );
	  break;

	case 4:
	  ligne = affiche_total_sous_ib ( table_etat,
					  ligne );
	  break;

	case 5:
	  ligne = affiche_total_compte ( table_etat,
					 ligne );
	  break;

	case 6:
	  ligne = affiche_total_tiers ( table_etat,
					ligne );
	  break;
	}

      /* on affiche le total de la partie en cours */

      ligne = affiche_total_partiel ( table_etat,
				      total_partie,
				      ligne,
				      i );

    fin_boucle_affichage_etat:
    }

  /* on affiche maintenant le total général */

  ligne = affiche_total_general ( table_etat,
				  total_general,
				  ligne );

}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les catégories sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_categories ( GtkWidget *table_etat,
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_categ_etat,
						    devise_categ_etat -> code_devise ));
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_categ_etat,
						    devise_categ_etat -> code_devise ));
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

  montant_categ_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_categ sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_sous_categ ( GtkWidget *table_etat,
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_sous_categ_etat,
						    devise_categ_etat -> code_devise ));
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_sous_categ_etat,
						    devise_categ_etat -> code_devise ));
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

  montant_sous_categ_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_ib ( GtkWidget *table_etat,
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_ib_etat,
						    devise_ib_etat -> code_devise ));
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_ib_etat,
						    devise_ib_etat -> code_devise ));
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

  montant_ib_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_sous_ib ( GtkWidget *table_etat,
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_sous_ib_etat,
						    devise_ib_etat -> code_devise ));
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_sous_ib_etat,
						    devise_ib_etat -> code_devise ));
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

  montant_sous_ib_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les compte sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_compte ( GtkWidget *table_etat,
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_compte_etat,
						    devise_compte_en_cours_etat -> code_devise ));
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_compte_etat,
						    devise_compte_en_cours_etat -> code_devise ));
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

  montant_compte_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les tiers sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint affiche_total_tiers ( GtkWidget *table_etat,
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_tiers_etat,
						    devise_tiers_etat -> code_devise ));
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

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_tiers_etat,
						    devise_tiers_etat -> code_devise ));
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

  montant_tiers_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint affichage_ligne_ope ( GtkWidget *table_etat,
			   struct structure_operation *operation,
			   gint ligne )
{
  gint colonne;
  GtkWidget *label;


  if ( etat_courant -> afficher_opes )
    {
      /* on affiche ce qui est demandé pour les opés */

      colonne = 1;

      if ( etat_courant -> afficher_no_ope )
	{
	  label = gtk_label_new ( itoa ( operation -> no_operation ));
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

      if ( etat_courant -> afficher_type_ope )
	{
	  GSList *pointeur;

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	  pointeur = g_slist_find_custom ( TYPES_OPES,
					   GINT_TO_POINTER ( operation -> type_ope ),
					   (GCompareFunc) recherche_type_ope_par_no );

	  if ( pointeur )
	    {
	      struct struct_type_ope *type;

	      type = pointeur -> data;

	      label = gtk_label_new ( type -> nom_type );
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


      if ( etat_courant -> afficher_cheque_ope )
	{
	  if ( operation -> contenu_type )
	    {
	      label = gtk_label_new ( operation -> contenu_type );
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

      if ( etat_courant -> afficher_rappr_ope )
	{
	  GSList *pointeur;

	  pointeur = g_slist_find_custom ( liste_no_rapprochements,
					   GINT_TO_POINTER ( operation -> no_rapprochement ),
					   (GCompareFunc) recherche_no_rapprochement_par_no );

	  if ( pointeur )
	    {
	      struct struct_no_rapprochement *rapprochement;

	      rapprochement = pointeur -> data;
	      label = gtk_label_new ( rapprochement -> nom_rapprochement );
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

      if ( devise_compte_en_cours_etat
	   &&
	   operation -> devise == devise_compte_en_cours_etat -> no_devise )
	label = gtk_label_new ( g_strdup_printf  ("%4.2f %s",
						  operation -> montant,
						  devise_compte_en_cours_etat -> code_devise ));
      else
	{
	  struct struct_devise *devise_operation;

	  devise_operation = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( operation -> devise ),
						   ( GCompareFunc ) recherche_devise_par_no ) -> data;
	  label = gtk_label_new ( g_strdup_printf  ("%4.2f %s",
						    operation -> montant,
						    devise_operation -> code_devise ));
	}

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
  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint affiche_total_partiel ( GtkWidget *table_etat,
			     gdouble total_partie,
			     gint ligne,
			     gint type )
{
  GtkWidget *label;
  GtkWidget *separateur;

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

  if ( type )
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

  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
					    total_partie,
					    devise_generale_etat -> code_devise ));
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

  return ( ligne );
}
/*****************************************************************************************************/

/*****************************************************************************************************/
gint affiche_total_general ( GtkWidget *table_etat,
			     gdouble total_general,
			     gint ligne )
{
  GtkWidget *label;
  GtkWidget *separateur;

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

  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
					    total_general,
					    devise_generale_etat -> code_devise ));
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

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint affiche_categ_etat ( struct structure_operation *operation,
			  GtkWidget *table_etat,
			  gchar *decalage_categ,
			  gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* vérifie qu'il y a un changement de catégorie */
  /* ça peut être aussi chgt pour virement, ventilation ou pas de categ */

  if ( etat_courant -> utilise_categ
       &&
       ( operation -> categorie != ancienne_categ_etat
	 ||
	 ( ancienne_categ_speciale_etat == 1
	   &&
	   !operation -> relation_no_operation )
	 ||
	 ( ancienne_categ_speciale_etat == 2
	   &&
	   !operation -> operation_ventilee )
	 ||
	 ( ancienne_categ_speciale_etat == 3
	   &&
	   ( operation -> operation_ventilee
	     ||
	     operation -> relation_no_operation ))))
    {

      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

	  ligne = affiche_totaux_sous_jaccent ( table_etat,
						1,
						ligne );

	  /* on ajoute le total de la categ */

	  ligne = affiche_total_categories ( table_etat,
					     ligne );
	}

      if ( operation -> categorie )
	{
	  pointeur_char = g_strconcat ( decalage_categ,
					((struct struct_categ *)(g_slist_find_custom ( liste_struct_categories,
										       GINT_TO_POINTER ( operation -> categorie ),
										       (GCompareFunc) recherche_categorie_par_no ) -> data )) -> nom_categ,
					NULL );
	  ancienne_categ_speciale_etat = 0;
	}

      else
	{
	  if ( operation -> relation_no_operation )
	    {
	      pointeur_char = g_strconcat ( decalage_categ,
					    "Virements",
					    NULL );
	      ancienne_categ_speciale_etat = 1;
	    }
	  else
	    {
	      if ( operation -> operation_ventilee )
		{
		  pointeur_char = g_strconcat ( decalage_categ,
						"Opération ventilée",
						NULL );
		  ancienne_categ_speciale_etat = 2;
		}
	      else
		{
		  pointeur_char = g_strconcat ( decalage_categ,
						"Pas de catégorie",
						NULL );
		  ancienne_categ_speciale_etat = 3;
		}
	    }
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


      denote_struct_sous_jaccentes ( 1 );

      ancienne_categ_etat = operation -> categorie;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint affiche_sous_categ_etat ( struct structure_operation *operation,
			       GtkWidget *table_etat,
			       gchar *decalage_sous_categ,
			       gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  if ( etat_courant -> utilise_categ
       &&
       etat_courant -> afficher_sous_categ
       &&
       operation -> categorie
       &&
       operation -> sous_categorie != ancienne_sous_categ_etat )
    {
      struct struct_categ *categ;

     /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on ajoute les totaux de tout ce qu'il y a derrière la sous catégorie */

	  ligne = affiche_totaux_sous_jaccent ( table_etat,
						2,
						ligne );

	  /* on ajoute le total de la sous categ */

	  ligne = affiche_total_sous_categ ( table_etat,
					     ligne );
	}


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

     denote_struct_sous_jaccentes ( 2 );

     ancienne_sous_categ_etat = operation -> sous_categorie;

     debut_affichage_etat = 0;
     changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint affiche_ib_etat ( struct structure_operation *operation,
		       GtkWidget *table_etat,
		       gchar *decalage_ib,
		       gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* mise en place de l'ib */


  if ( etat_courant -> utilise_ib
       &&
       operation -> imputation != ancienne_ib_etat )
    {
      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on ajoute les totaux de tout ce qu'il y a derrière l'ib */

	  ligne = affiche_totaux_sous_jaccent ( table_etat,
						3,
						ligne );

	  /* on ajoute le total de l'ib */

	  ligne = affiche_total_ib ( table_etat,
				     ligne );
	}
 
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

      denote_struct_sous_jaccentes ( 3 );

      ancienne_ib_etat = operation -> imputation;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint affiche_sous_ib_etat ( struct structure_operation *operation,
			    GtkWidget *table_etat,
			    gchar *decalage_sous_ib,
			    gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;


  /* mise en place de la sous_ib */


  if ( etat_courant -> utilise_ib
       &&
       etat_courant -> afficher_sous_ib
       &&
       operation -> imputation
       &&
       operation -> sous_imputation != ancienne_sous_ib_etat )
    {
      struct struct_imputation *imputation;

      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on ajoute les totaux de tout ce qu'il y a derrière la sous ib */

	  ligne = affiche_totaux_sous_jaccent ( table_etat,
						4,
						ligne );

	  /* on ajoute le total de la sous ib */

	  ligne = affiche_total_sous_ib ( table_etat,
					  ligne );
	}
 
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

      denote_struct_sous_jaccentes ( 4 );

      ancienne_sous_ib_etat = operation -> sous_imputation;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint affiche_compte_etat ( struct structure_operation *operation,
			   GtkWidget *table_etat,
			   gchar *decalage_compte,
			   gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* mise en place du compte */

  if ( etat_courant -> regroupe_ope_par_compte
       &&
       operation -> no_compte != ancien_compte_etat )
    {
      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on ajoute les totaux de tout ce qu'il y a derrière le compte */

	  ligne = affiche_totaux_sous_jaccent ( table_etat,
						5,
						ligne );

	  /* on ajoute le total du compte */

	  ligne = affiche_total_compte ( table_etat,
				     ligne );
	}
 
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

      denote_struct_sous_jaccentes ( 5 );

      ancien_compte_etat = operation -> no_compte;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint affiche_tiers_etat ( struct structure_operation *operation,
			  GtkWidget *table_etat,
			  gchar *decalage_tiers,
			  gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* affiche le tiers */

  if ( etat_courant -> utilise_tiers
       &&
       operation -> tiers != ancien_tiers_etat )
    {
      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on ajoute les totaux de tout ce qu'il y a derrière le tiers */

	  ligne = affiche_totaux_sous_jaccent ( table_etat,
						6,
						ligne );

	  /* on ajoute le total du tiers */

	  ligne = affiche_total_tiers ( table_etat,
					ligne );
	}

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

      denote_struct_sous_jaccentes ( 6 );

      ancien_tiers_etat = operation -> tiers;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }
  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint affiche_titre_revenus_etat ( GtkWidget *table_etat,
				  gint ligne )
{
  GtkWidget *label;

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

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint affiche_titre_depenses_etat ( GtkWidget *table_etat,
				   gint ligne )
{
  GtkWidget *label;

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


  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* appelée lors de l'affichage d'une structure ( catég, ib ... ) */
/* affiche le total de toutes les structures sous jaccentes */
/*****************************************************************************************************/

gint affiche_totaux_sous_jaccent ( GtkWidget *table_etat,
				   gint origine,
				   gint ligne )
{
  GList *pointeur_glist;

  /* on doit partir du bout de la liste pour revenir vers la structure demandée */

  pointeur_glist = g_list_last ( etat_courant -> type_classement );


  while ( GPOINTER_TO_INT ( pointeur_glist -> data ) != origine )
    {
      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	case 1:
	  ligne = affiche_total_categories ( table_etat,
					     ligne );
	  break;

	case 2:
	  ligne = affiche_total_sous_categ ( table_etat,
					     ligne );
	  break;

	case 3:
	  ligne = affiche_total_ib ( table_etat,
				     ligne );
	  break;

	case 4:
	  ligne = affiche_total_sous_ib ( table_etat,
					  ligne );
	  break;

	case 5:
	  ligne = affiche_total_compte ( table_etat,
					 ligne );
	  break;

	case 6:
	  ligne = affiche_total_tiers ( table_etat,
					ligne );
	  break;
	}
      pointeur_glist = pointeur_glist -> prev;
    }

  return ( ligne );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* met tous les anciens_x_etat sous jaccents à l'origine à -1 */
/*****************************************************************************************************/

void denote_struct_sous_jaccentes ( gint origine )
{
  GList *pointeur_glist;

  /* on peut partir du bout de la liste pour revenir vers la structure demandée */
  /* gros vulgaire copier coller de la fonction précédente */

  pointeur_glist = g_list_last ( etat_courant -> type_classement );


  while ( GPOINTER_TO_INT ( pointeur_glist -> data ) != origine )
    {
      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	case 1:
	  ancienne_categ_etat = -1;
	  ancienne_categ_speciale_etat = 0;
	  break;

	case 2:
	  ancienne_sous_categ_etat = -1;
	  break;

	case 3:
	  ancienne_ib_etat = -1;
	  break;

	case 4:
	  ancienne_sous_ib_etat = -1;
	  break;

	case 5:
	  ancien_compte_etat = -1;
	  break;

	case 6:
	  ancien_tiers_etat = -1;
	  break;
	}
      pointeur_glist = pointeur_glist -> prev;
    }
}
/*****************************************************************************************************/
