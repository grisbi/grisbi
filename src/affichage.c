/* Ce fichier s'occupe des différents paramètres d'affichage réglés dans les paramètres */
/* affichage.c */

/*     Copyright (C) 2000-2002  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http:// www.grisbi.org */

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





/* ************************************************************************************************************** */
/* renvoie l'onglet de l'affichage des paramètres */
/* ************************************************************************************************************** */

GtkWidget *onglet_affichage ( void )
{
  GtkWidget *hbox, *vbox;
  GtkWidget *separateur;
  GtkWidget *label;
  GtkWidget *frame;
  GSList *liste_tmp;
  GtkWidget *vbox2;
  GtkWidget *fleche;
  GtkWidget *hbox2;
  GtkWidget *bouton;
  GtkWidget *onglet;

  onglet = gtk_hbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


  /* fenetre de base verticale */

  vbox = gtk_vbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );


  /*   ajout du choix de la fonte pour les listes */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  if ( fonte_liste )
    gnome_font_picker_set_font_name ( GNOME_FONT_PICKER ( bouton ),
				      fonte_liste );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  label = gtk_label_new ( SPACIFY(_(": Modifier la fonte des listes /")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  /* on remet un font_picker sans nom de fonte pour le raz */

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );

  gtk_container_remove ( GTK_CONTAINER ( bouton ),
			 GTK_BIN ( bouton ) -> child );
  label = gtk_label_new ( _("RAZ") );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      label );
  gtk_widget_show ( label );
  gtk_widget_show ( bouton );


  /*   ajout du choix de la fonte générale */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  if ( fonte_general )
    gnome_font_picker_set_font_name ( GNOME_FONT_PICKER ( bouton ),
				      fonte_general );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte_general ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  label = gtk_label_new ( SPACIFY(_(": Modifier la fonte générale /")) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  /* on remet un font_picker sans nom de fonte pour le raz */

  bouton = gnome_font_picker_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "font-set",
		       GTK_SIGNAL_FUNC ( choix_fonte_general ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );

  gtk_container_remove ( GTK_CONTAINER ( bouton ),
			 GTK_BIN ( bouton ) -> child );
  label = gtk_label_new ( _("RAZ") );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      label );
  gtk_widget_show ( label );
  gtk_widget_show ( bouton );

  /* ajout de la modification du logo de l'accueil  */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      gnome_stock_pixmap_widget ( bouton,
						  GNOME_STOCK_PIXMAP_FORWARD ));
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( modification_logo_accueil ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  label = gtk_label_new ( _(" : Modifier le logo de l'accueil") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );




  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );



  /* mise en place de l'ordre d'affichage des comptes */

  hbox2 = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox2,
		       TRUE,
		       TRUE,
		       0);
  
  frame = gtk_frame_new ( SPACIFY(_("Ordre d'affichage des comptes")) );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_container_set_border_width ( GTK_CONTAINER ( frame ),
				   10 );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       frame,
		       TRUE,
		       TRUE,
		       0);


  if ( !ordre_comptes )
    {
      label = gtk_label_new ( _("Aucun compte défini") );
      gtk_container_add ( GTK_CONTAINER ( frame ),
			  label );
   }
  else
    {
      gpointer ** p_tab_nom_de_compte_variable;                    /* évite d'écraser la variable globale */
      gchar *nom_compte[1];
      GtkWidget *scroll_liste_choix_ordre_comptes;


      hbox = gtk_hbox_new ( FALSE,
				 5 );
      gtk_container_set_border_width ( GTK_CONTAINER ( hbox ),
				       10 );
      gtk_container_add ( GTK_CONTAINER ( frame ),
			  hbox );
      gtk_widget_show ( hbox );


/* mise en place de la list qui contient les noms et l'ordre des comptes */

      scroll_liste_choix_ordre_comptes = gtk_scrolled_window_new ( NULL, NULL );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   scroll_liste_choix_ordre_comptes,
			   FALSE,
			   FALSE,
			   0);
      gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_liste_choix_ordre_comptes ),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
      gtk_widget_set_usize ( GTK_WIDGET ( scroll_liste_choix_ordre_comptes ),
			     150,
			     120 );

      liste_choix_ordre_comptes = gtk_clist_new ( 1 );
      gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_choix_ordre_comptes ) ,
					 0,
					 TRUE );
      gtk_clist_set_reorderable ( GTK_CLIST ( liste_choix_ordre_comptes ),
				  TRUE );
      gtk_clist_set_use_drag_icons ( GTK_CLIST ( liste_choix_ordre_comptes ),
				     TRUE );
      gtk_container_add ( GTK_CONTAINER ( scroll_liste_choix_ordre_comptes ),
			  liste_choix_ordre_comptes );

      liste_tmp = ordre_comptes;

      do
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + (gint) liste_tmp->data;
	  nom_compte[0] = g_strdup ( NOM_DU_COMPTE );
	  gtk_clist_set_row_data ( GTK_CLIST ( liste_choix_ordre_comptes ),
				   gtk_clist_append ( GTK_CLIST ( liste_choix_ordre_comptes) , nom_compte ),
				   liste_tmp->data );
	}
      while ( ( liste_tmp = liste_tmp -> next ));

      gtk_signal_connect_object ( GTK_OBJECT ( liste_choix_ordre_comptes ),
				  "row_move",
				  gnome_property_box_changed,
				  GTK_OBJECT (fenetre_preferences));
 
      /* on place ici les flèches sur le côté de la liste */

      vbox2 = gtk_vbutton_box_new ();

      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   vbox2,
			   FALSE,
			   FALSE,
			   0);

      fleche = gnome_stock_button ( GNOME_STOCK_BUTTON_UP );
      gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			     GTK_RELIEF_NONE );
      gtk_signal_connect ( GTK_OBJECT ( fleche ),
			   "clicked",
			   (GtkSignalFunc ) deplacement_haut,
			   NULL );
      gtk_container_add ( GTK_CONTAINER ( vbox2 ),
			  fleche  );

      fleche = gnome_stock_button ( GNOME_STOCK_BUTTON_DOWN );
      gtk_button_set_relief ( GTK_BUTTON ( fleche ),
			     GTK_RELIEF_NONE );
      gtk_signal_connect ( GTK_OBJECT ( fleche ),
			   "clicked",
			   (GtkSignalFunc ) deplacement_bas,
			    NULL);
      gtk_container_add ( GTK_CONTAINER ( vbox2 ),
			  fleche  );

      /* connecte maintenant les sélections */


      gtk_signal_connect_object ( GTK_OBJECT ( liste_choix_ordre_comptes ),
				  "select-row",
				  GTK_SIGNAL_FUNC ( selection_choix_ordre_comptes ),
				  GTK_OBJECT ( vbox2 ));
      gtk_signal_connect_object ( GTK_OBJECT ( liste_choix_ordre_comptes ),
				  "unselect-row",
				  GTK_SIGNAL_FUNC ( deselection_choix_ordre_comptes ),
				  GTK_OBJECT ( vbox2 ));

      /*       selectionne le premier compte */

      gtk_clist_select_row ( GTK_CLIST ( liste_choix_ordre_comptes ),
			     0,
			     0);
    }

  gtk_widget_show_all ( vbox );


  /*   place la partie de droite : comptabilité */

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  vbox = gtk_vbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  frame = gtk_frame_new ( _("Comptabilité et formulaire") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );

  bouton_afficher_no_operation = gtk_check_button_new_with_label ( _("Afficher le numéro de l'opération éditée.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_operation ),
				 etat.affiche_no_operation );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_no_operation ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_no_operation,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_no_operation );

  /* GDC : bouton pour choisir d'afficher ou non la date réelle (colonne et champ) */

  bouton_afficher_date_bancaire = gtk_check_button_new_with_label ( _("Afficher la date de valeur des opérations.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_bancaire ),
				 etat.affiche_date_bancaire );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_date_bancaire ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_date_bancaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_date_bancaire );

  /* FinGDC */

  bouton_utiliser_exercices = gtk_check_button_new_with_label ( _("Utiliser les Exercices.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_exercices ),
				 etat.utilise_exercice );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_exercices ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_exercices,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_exercices );

  bouton_utiliser_imputation_budgetaire = gtk_check_button_new_with_label ( _("Utiliser les Imputations Budgétaires.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_imputation_budgetaire ),
				 etat.utilise_imputation_budgetaire );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_imputation_budgetaire ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_imputation_budgetaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_imputation_budgetaire );

  bouton_utiliser_piece_comptable = gtk_check_button_new_with_label ( _("Utiliser l'entrée des Pièces Comptables.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_piece_comptable ),
				 etat.utilise_piece_comptable );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_piece_comptable ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_piece_comptable,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_piece_comptable );

  bouton_utiliser_info_banque_guichet = gtk_check_button_new_with_label ( _("Utiliser l'entrée des informations Banque / Guichet.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utiliser_info_banque_guichet ),
				 etat.utilise_info_banque_guichet );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_utiliser_info_banque_guichet ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_utiliser_info_banque_guichet,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utiliser_info_banque_guichet );

  bouton_afficher_boutons_valider_annuler = gtk_check_button_new_with_label ( _("Afficher les boutons Valider et Annuler l'opération.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_boutons_valider_annuler ),
				 etat.affiche_boutons_valider_annuler );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_boutons_valider_annuler ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_boutons_valider_annuler,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_boutons_valider_annuler );



  /* mise en place de la frame sur l'affichage des listes */

  frame = gtk_frame_new ( _("Listes des Tiers, Catégories et I.B.") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );

  /*   mise en place du choix de la devise pour les totaux des tiers, catég et ib */

  hbox = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Devise :") );
  gtk_box_pack_start ( GTK_BOX (hbox  ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_choix_devise_totaux_tiers = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ),
			     creation_option_menu_devises ( -1,
							    liste_struct_devises ) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_choix_devise_totaux_tiers,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_devise_totaux_tiers );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( no_devise_totaux_tiers ),
									 ( GCompareFunc ) recherche_devise_par_no )));
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_devise_totaux_tiers ) -> menu ),
			      "selection-done",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  if ( !nb_comptes )
    gtk_widget_set_sensitive ( hbox,
			       FALSE );


  /* affichage du nb d'écritures */

  bouton_afficher_nb_ecritures = gtk_check_button_new_with_label ( _("Afficher le nombre d'écritures.") );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_nb_ecritures ),
				 etat.affiche_nb_ecritures_listes );
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_afficher_nb_ecritures ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_nb_ecritures,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_nb_ecritures );


  /*   place la partie de droite : classement des listes */

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  vbox = gtk_vbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  frame = gtk_frame_new ( _("Tri de la liste d'opérations") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox2 );
  gtk_widget_show ( vbox2 );

  bouton_classer_liste_par_date = gtk_radio_button_new_with_label ( NULL,
								    _("Classer par date") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_classer_liste_par_date,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_classer_liste_par_date );

  bouton_classer_liste_par_date_bancaire = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_classer_liste_par_date)),
									     _("Classer par date de valeur") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_classer_liste_par_date_bancaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_classer_liste_par_date_bancaire );

  if ( etat.classement_par_date )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_classer_liste_par_date ),
				   TRUE );
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_classer_liste_par_date_bancaire ),
				   TRUE );

  gtk_signal_connect_object ( GTK_OBJECT ( bouton_classer_liste_par_date ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));
  gtk_signal_connect_object ( GTK_OBJECT ( bouton_classer_liste_par_date_bancaire ),
			      "toggled",
			      gnome_property_box_changed,
			      GTK_OBJECT (fenetre_preferences));

  return ( onglet );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void selection_choix_ordre_comptes ( GtkWidget *box )
{
  gtk_widget_set_sensitive ( box,
			     TRUE );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
void deselection_choix_ordre_comptes ( GtkWidget *box )
{
  gtk_widget_set_sensitive ( box,
			     FALSE );
}
/* ************************************************************************************************************** */



/* **************************************************************************************************************************** */
void deplacement_haut ( void )
{
  if ( !GPOINTER_TO_INT (GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ))
    return;

  gtk_clist_swap_rows ( GTK_CLIST ( liste_choix_ordre_comptes ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) - 1 );

}
/* **************************************************************************************************************************** */





/* **************************************************************************************************************************** */
void deplacement_bas ( void )
{
  if ( GPOINTER_TO_INT (GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) == ( nb_comptes - 1 ) )
    return;

  gtk_clist_swap_rows ( GTK_CLIST ( liste_choix_ordre_comptes ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ),
			GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) + 1 );
}
/* **************************************************************************************************************************** */




/* **************************************************************************************************************************** */
void choix_fonte ( GtkWidget *bouton,
		   gchar *fonte,
		   gpointer null )
{
  GdkFont *font;
  gchar **tab_font;
  gint i;

  fonte_liste = g_strdup ( fonte );

  font = gdk_font_load ( fonte );

  /* applique la fonte  */
  
  style_couleur [0] -> font = font;
  style_couleur [1] -> font = font;
  style_rouge_couleur [0] -> font = font;
  style_rouge_couleur [1] -> font = font;

/* récupère la hauteur de la fonte */

  tab_font = g_strsplit ( fonte,
			  "-",
			  FALSE );

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {

      gtk_clist_set_row_height ( GTK_CLIST ( CLIST_OPERATIONS ),
				 g_strtod ( tab_font[7],
					    NULL ) + 2 );
      gtk_clist_set_row_height ( GTK_CLIST ( liste_echeances ),
				 g_strtod ( tab_font[7],
					    NULL ) + 2 );

      p_tab_nom_de_compte_variable++;
    }

  g_strfreev ( tab_font );

}
/* **************************************************************************************************************************** */





/* **************************************************************************************************************************** */
void choix_fonte_general ( GtkWidget *bouton,
			   gchar *fonte,
			   gpointer null )
{
  GtkStyle *style_general;

  fonte_general = g_strdup ( fonte );

  style_general = gtk_widget_get_style ( window );
  style_general -> font = gdk_font_load ( fonte );
  style_general = gtk_widget_get_default_style ();
  style_general -> font = gdk_font_load ( fonte );

  gtk_notebook_set_page ( GTK_NOTEBOOK ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> notebook ),
			  0 );
  gtk_notebook_set_page ( GTK_NOTEBOOK ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> notebook ),
			  3 );

  gtk_widget_destroy ( notebook_general );
  gtk_widget_destroy ( GNOME_APP ( window ) -> menubar );
  GNOME_APP ( window ) -> menubar = NULL;
  gnome_app_create_menus ( GNOME_APP ( window ), 
			   menu_principal );

  ouverture_confirmee ();
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void modification_logo_accueil ( void )
{
  GtkWidget *dialog;
  GtkWidget *choix;
  gint resultat;
  GtkWidget *bouton;

  dialog = gnome_dialog_new ( _("Sélection d'un nouveau logo"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( fenetre_preferences ));

  choix = gnome_pixmap_entry_new ( "path_logo_grisbi",
				   _("Choisir un nouveau logo"),
				   TRUE );
  gtk_widget_set_usize ( choix,
			 300,
			 300 );

  if ( chemin_logo )
    gtk_entry_set_text ( GTK_ENTRY ( gnome_pixmap_entry_gtk_entry ( GNOME_PIXMAP_ENTRY ( choix ))),
			 chemin_logo );
  gnome_pixmap_entry_set_preview ( GNOME_PIXMAP_ENTRY ( choix ),
				   TRUE );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       choix,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( choix );

  bouton = gtk_button_new_with_label ( _("Rétablir le logo de Grisbi") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( remise_a_zero_logo ),
		       choix );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       bouton,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( bouton );



  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    {
      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }
  if ( nb_comptes )
    {
      /* on change le logo */

      chemin_logo = gnome_pixmap_entry_get_filename ( GNOME_PIXMAP_ENTRY ( choix ));

  /* on vire l'ancien logo et la séparation */

      gtk_widget_destroy ( ((GtkBoxChild *)(GTK_BOX ( page_accueil ) -> children -> data )) -> widget );
      gtk_widget_destroy ( ((GtkBoxChild *)(GTK_BOX ( page_accueil ) -> children -> data )) -> widget );

      if ( !(chemin_logo
	     &&
	     strlen ( g_strstrip ( chemin_logo ))) )
	{
	  chemin_logo = NULL;
	  if ( logo_accueil
	       &&
	       GTK_IS_WIDGET ( logo_accueil ))
	    gtk_widget_hide ( logo_accueil );
	}
      else
	{
	  GtkWidget *separateur;

	  logo_accueil =  gnome_pixmap_new_from_file ( chemin_logo );

	  gtk_box_pack_start ( GTK_BOX ( page_accueil ),
			       logo_accueil,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show ( logo_accueil );

	  /* séparation gauche-droite */

	  separateur = gtk_vseparator_new ();
	  gtk_box_pack_start ( GTK_BOX ( page_accueil ),
			       separateur,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show ( separateur );

	  gtk_box_reorder_child ( GTK_BOX ( page_accueil ),
				  logo_accueil,
				  0 );
	  gtk_box_reorder_child ( GTK_BOX ( page_accueil ),
				  separateur,
				  1 );
  
	}
    }

  gnome_dialog_close ( GNOME_DIALOG ( dialog ));

  /* on sauvegarde le chemin */

  gnome_config_set_string ( "/Grisbi/Affichage/Chemin_du_logo",
			    chemin_logo );
  gnome_config_sync();

}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
void remise_a_zero_logo ( GtkWidget *bouton,
			  GtkWidget *pixmap_entry )
{
  chemin_logo = CHEMIN_LOGO;

  gtk_entry_set_text ( GTK_ENTRY ( gnome_pixmap_entry_gtk_entry ( GNOME_PIXMAP_ENTRY ( pixmap_entry ))),
		       chemin_logo );

  gnome_pixmap_entry_set_preview ( GNOME_PIXMAP_ENTRY ( pixmap_entry ),
				   TRUE );
}
/* **************************************************************************************************************************** */
