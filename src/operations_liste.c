/*  Fichier qui gère la liste des opérations */
/*      liste_operations.c */

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



/*******************************************************************************************/
/*  Routine qui crée la fenêtre de la liste des opé  */
/***********************************************************************************************/

GtkWidget *creation_fenetre_operations ( void )
{
  GtkWidget *win_operations;
  GtkWidget *solde_box;
  GtkWidget *frame;


  /*   la fenetre des opé est une vbox : la liste en haut, le solde et  */
  /*     des boutons de conf au milieu, le formulaire en bas */

  win_operations = gtk_vbox_new ( FALSE,
				  5 );


/* création de la barre d'outils */

  barre_outils = creation_barre_outils ();
  gtk_box_pack_start ( GTK_BOX (win_operations),
		       barre_outils,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( barre_outils );

  /* création du notebook des opé */

  notebook_listes_operations = initialisation_notebook_operations ();
  gtk_box_pack_start ( GTK_BOX (win_operations),
		       notebook_listes_operations,
		       TRUE,
		       TRUE,
		       0);
  gtk_widget_show ( notebook_listes_operations );

/*   création de la ligne contenant le solde ( sous la liste des opérations ) et les boutons */

  solde_box = gtk_table_new ( 1,
			      3,
			      TRUE);

  gtk_box_pack_start ( GTK_BOX (win_operations),
		       solde_box,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( solde_box );


/* mise en forme du solde pointé */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_table_attach ( GTK_TABLE ( solde_box ),
		     frame,
		     0, 1,
		     0, 1,
		     GTK_EXPAND | GTK_SHRINK,
		     FALSE,
		     0, 0);
  gtk_widget_show ( frame );


  solde_label_pointe = gtk_label_new ( SPACIFY(COLON(_("Reconciled balance"))) );
  gtk_label_set_justify ( GTK_LABEL ( solde_label_pointe ),
			  GTK_JUSTIFY_LEFT);
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      solde_label_pointe );
  gtk_widget_show ( solde_label_pointe );


/* mise en forme du solde courant */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_table_attach ( GTK_TABLE ( solde_box ),
		     frame,
		     2, 3,
		     0, 1,
		     GTK_EXPAND | GTK_SHRINK,
		     FALSE,
		     0, 0);
  gtk_widget_show ( frame );


  solde_label = gtk_label_new ( SPACIFY(COLON(_("Curent balance"))) );
  gtk_label_set_justify ( GTK_LABEL ( solde_label ),
			  GTK_JUSTIFY_RIGHT);
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      solde_label );
  gtk_widget_show ( solde_label );

  return (win_operations);
}
/*******************************************************************************************/


/*******************************************************************************************/
/* Création du notebook des opés */
/* c'est en fait un notebook dont chaque onglet sera une clist qui contient les opé d'un compte */
/* il y a autant d'onglet que de comptes */
/* cette fonction crée le notebook et initialise les variables générales utilisées par les listes */
/*******************************************************************************************/

GtkWidget *initialisation_notebook_operations ( void )
{
  GdkColor couleur1;
  GdkColor couleur2;
  GdkColor couleur_rouge;
  GdkColor couleur_selection;
  GdkColor couleur_grise;

  /* Initialisation des couleurs de la clist */

  couleur1.red = COULEUR1_RED ;
  couleur1.green = COULEUR1_GREEN;
  couleur1.blue = COULEUR1_BLUE;

  couleur2.red = COULEUR2_RED;
  couleur2.green = COULEUR2_GREEN;
  couleur2.blue = COULEUR2_BLUE;

  couleur_rouge.red = COULEUR_ROUGE_RED;
  couleur_rouge.green = COULEUR_ROUGE_GREEN;
  couleur_rouge.blue = COULEUR_ROUGE_BLUE;

  couleur_grise.red = COULEUR_GRISE_RED;
  couleur_grise.green = COULEUR_GRISE_GREEN;
  couleur_grise.blue = COULEUR_GRISE_BLUE;

/* initialise la couleur de la sélection */

  couleur_selection.red= COULEUR_SELECTION_RED;
  couleur_selection.green= COULEUR_SELECTION_GREEN ;
  couleur_selection.blue= COULEUR_SELECTION_BLUE;


/* initialise les variables style_couleur_1 et style_couleur_2 qui serviront */
/* à mettre la couleur de fond */

  style_couleur [0] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
  style_couleur [0]->fg[GTK_STATE_NORMAL] = style_couleur [0]->black;
  style_couleur [0]->base[GTK_STATE_NORMAL] = couleur2;
  style_couleur [0]->fg[GTK_STATE_SELECTED] = style_couleur [0]->black;
  style_couleur [0]->bg[GTK_STATE_SELECTED] = couleur_selection;

  style_couleur [1] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
  style_couleur [1]->fg[GTK_STATE_NORMAL] = style_couleur [1]->black;
  style_couleur [1]->base[GTK_STATE_NORMAL] = couleur1;
  style_couleur [1]->fg[GTK_STATE_SELECTED] = style_couleur [1]->black;
  style_couleur [1]->bg[GTK_STATE_SELECTED] = couleur_selection;


/* initialise les variables style_rouge_couleur [1] et style_rouge_couleur [2] qui serviront */
/* à mettre en rouge le solde quand il est négatif */

  style_rouge_couleur [0] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
  style_rouge_couleur [0] ->fg[GTK_STATE_NORMAL] = couleur_rouge;
  style_rouge_couleur [0] ->base[GTK_STATE_NORMAL] = couleur2;
  style_rouge_couleur [0] ->fg[GTK_STATE_SELECTED] = couleur_rouge;
  style_rouge_couleur [0] ->bg[GTK_STATE_SELECTED] = couleur_selection;

  style_rouge_couleur [1] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
  style_rouge_couleur [1] ->fg[GTK_STATE_NORMAL] = couleur_rouge;
  style_rouge_couleur [1] ->base[GTK_STATE_NORMAL] = couleur1;
  style_rouge_couleur [1] ->fg[GTK_STATE_SELECTED] = couleur_rouge;
  style_rouge_couleur [1] ->bg[GTK_STATE_SELECTED] = couleur_selection;

  style_gris = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
  style_gris ->base[GTK_STATE_NORMAL] = couleur_grise;
  style_gris ->fg[GTK_STATE_NORMAL] = style_gris->black;

  /* création du notebook */

  notebook_listes_operations = gtk_notebook_new ();
  gtk_notebook_set_show_tabs ( GTK_NOTEBOOK ( notebook_listes_operations ),
			       FALSE );

  /* création de l'onglet de la ventilation */

  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			     creation_fenetre_ventilation (),
			     gtk_label_new ( _("Account breakdown") ) );

  return ( notebook_listes_operations );
}
/*******************************************************************************************/




/*******************************************************************************************/
/* Création de la liste des opé */
/* cette fonction crée autant d'onglets que de comptes dans le notebook */
/* elle y met les clist et les remplit */
/*******************************************************************************************/

void creation_listes_operations ( void )
{
  gint i;
  GtkTooltips *tooltip;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {
      GtkWidget *onglet;
      GtkWidget *liste;
      gint j;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;


      /*  Création de la scrolled window qui contient la clist */
      
      onglet = gtk_scrolled_window_new ( NULL,
					 NULL);
      gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( onglet ),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
      gtk_widget_show ( onglet );


      /* création de l'onglet */

      liste = gtk_clist_new_with_titles ( 7,
					  titres_colonnes_liste_operations );
      gtk_widget_set_usize ( GTK_WIDGET ( liste ),
			     1,
			     FALSE );
      gtk_signal_connect ( GTK_OBJECT ( liste ),
			   "size-allocate",
			   GTK_SIGNAL_FUNC ( changement_taille_liste_ope ),
			   GINT_TO_POINTER ( i ) );
      gtk_signal_connect ( GTK_OBJECT ( liste ),
			   "draw",
			   GTK_SIGNAL_FUNC ( verification_mise_a_jour_liste ),
			   NULL );
       gtk_signal_connect_after ( GTK_OBJECT ( onglet ),
			   "realize",
			   GTK_SIGNAL_FUNC ( onglet_compte_realize ),
			   liste );
      gtk_container_add ( GTK_CONTAINER ( onglet ),
			  liste );
      gtk_widget_show ( liste );


      /* on met les tooltips aux boutons de la clist */
      /*       le fait de mettre des tips sur les titres rend les boutons sensitifs ; */
      /* on va détourner le click pour ne pas faire enfoncer le bouton */

      tooltip = gtk_tooltips_new ();

      for ( j=0 ; j<7 ; j++ )
	{
	  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltip ),
				 GTK_CLIST (liste)->column[j].button,
				 tips_col_liste_operations[j],
				 tips_col_liste_operations[j] );
	  gtk_signal_connect ( GTK_OBJECT ( GTK_CLIST (liste)->column[j].button ),
			       "button-press-event",
			       GTK_SIGNAL_FUNC ( empeche_pression_titre_colonne ),
			       NULL );
	}

      /* on permet la sélection de plusieurs lignes */

      gtk_clist_set_selection_mode ( GTK_CLIST ( liste ),
				     GTK_SELECTION_MULTIPLE );


      /* On annule la fonction bouton des titres */

      gtk_clist_column_titles_passive ( GTK_CLIST ( liste));



      /* justification du contenu des cellules */


      gtk_clist_set_column_justification ( GTK_CLIST ( liste),
					   0,
					   GTK_JUSTIFY_CENTER);
      gtk_clist_set_column_justification ( GTK_CLIST ( liste),
					   1,
					   GTK_JUSTIFY_CENTER);
      gtk_clist_set_column_justification ( GTK_CLIST ( liste),
					   2,
					   GTK_JUSTIFY_LEFT);
      gtk_clist_set_column_justification ( GTK_CLIST ( liste),
					   3,
					   GTK_JUSTIFY_CENTER);
      gtk_clist_set_column_justification ( GTK_CLIST ( liste),
					   4,
					   GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification ( GTK_CLIST ( liste),
					   5,
					   GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification ( GTK_CLIST ( liste),
					   6,
					   GTK_JUSTIFY_RIGHT);

      for ( j=0 ; j<7 ; j++ )
	gtk_clist_set_column_resizeable ( GTK_CLIST ( liste ),
					  j,
					  !etat.largeur_auto_colonnes );

       /* vérifie le simple ou double click */

      gtk_signal_connect ( GTK_OBJECT (liste),
			   "button_press_event",
			   GTK_SIGNAL_FUNC (selectionne_ligne_souris),
			   NULL );


      /*   vérifie la touche entrée, haut et bas */

      gtk_signal_connect ( GTK_OBJECT (liste),
			   "key_press_event",
			   GTK_SIGNAL_FUNC (traitement_clavier_liste),
			   NULL );

  
      /* attente du relachement de ctrl+p */

      gtk_signal_connect ( GTK_OBJECT (liste),
			   "key_release_event",
			   GTK_SIGNAL_FUNC (fin_ctrl),
			   NULL );


      /* on ajoute l'onglet au notebook des comptes */

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
				 onglet,
				 gtk_label_new ( NOM_DU_COMPTE ) );

      CLIST_OPERATIONS = liste;

      /* on classe la liste en fonction de la date */

      LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
					(GCompareFunc) classement_sliste );

      remplissage_liste_operations ( i );
    }

  /* met la fonte appropriée */

  if ( fonte_liste )
    choix_fonte ( NULL,
		  fonte_liste,
		  NULL );
}
/***************************************************************************************************/



/***************************************************************************************************/
/* Fonction empeche_pression_titre_colonne */
/* permet d'éviter que le bouton s'enfonce si on clicke dessus */
/***************************************************************************************************/

gint empeche_pression_titre_colonne ( GtkWidget *bouton )
{
  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( bouton ),
				 "button_press_event");
  return ( TRUE );
}
/***************************************************************************************************/


/*******************************************************************************************/
/* Création de la liste des opé d'un nouveau compte */
/*******************************************************************************************/

void ajoute_nouvelle_liste_operation ( gint no_compte )
{
  GtkWidget *onglet;
  GtkWidget *liste;
  GtkTooltips *tooltip;
  gint i;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;


  /*  Création de la scrolled window qui contient la clist */
      
  onglet = gtk_scrolled_window_new ( NULL,
				     NULL);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( onglet ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
  gtk_widget_show ( onglet );


  /* création de l'onglet */

  liste = gtk_clist_new_with_titles ( 7,
				      titres_colonnes_liste_operations );
  gtk_widget_set_usize ( GTK_WIDGET ( liste ),
			 1,
			 FALSE );
  gtk_signal_connect ( GTK_OBJECT ( liste ),
		       "size-allocate",
		       GTK_SIGNAL_FUNC ( changement_taille_liste_ope ),
		       GINT_TO_POINTER ( no_compte ) );
  gtk_signal_connect ( GTK_OBJECT ( liste ),
		       "draw",
		       GTK_SIGNAL_FUNC ( verification_mise_a_jour_liste ),
		       NULL );
  gtk_signal_connect_after ( GTK_OBJECT ( liste ),
			     "realize",
			     GTK_SIGNAL_FUNC ( onglet_compte_realize ),
			     liste );
  gtk_container_add ( GTK_CONTAINER ( onglet ),
		      liste );
  gtk_widget_show ( liste );



  /*       le fait de mettre des tips sur les titres rend les boutons sensitifs ; */
  /* on va détourner le click pour ne pas faire enfoncer le bouton */

  tooltip = gtk_tooltips_new ();

  for ( i=0 ; i<6 ; i++ )
    {
      gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltip ),
			     GTK_CLIST (liste)->column[i].button,
			     tips_col_liste_operations[i],
			     tips_col_liste_operations[i] );
      gtk_signal_connect ( GTK_OBJECT ( GTK_CLIST (liste)->column[i].button ),
			   "button-press-event",
			   GTK_SIGNAL_FUNC ( empeche_pression_titre_colonne ),
			   NULL );
    }


  /* on permet la sélection de plusieurs lignes */

  gtk_clist_set_selection_mode ( GTK_CLIST ( liste ),
				 GTK_SELECTION_MULTIPLE );

  
  /* On annule la fonction bouton des titres */

  gtk_clist_column_titles_passive ( GTK_CLIST ( liste));



  /* justification du contenu des cellules */


  gtk_clist_set_column_justification ( GTK_CLIST ( liste),
				       0,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste),
				       1,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste),
				       2,
				       GTK_JUSTIFY_LEFT);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste),
				       3,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste),
				       4,
				       GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste),
				       5,
				       GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste),
				       6,
				       GTK_JUSTIFY_RIGHT);


  /* vérifie le simple ou double click */

  gtk_signal_connect ( GTK_OBJECT (liste),
		       "button_press_event",
		       GTK_SIGNAL_FUNC (selectionne_ligne_souris),
		       NULL );


  /*   vérifie la touche entrée, haut et bas */

  gtk_signal_connect ( GTK_OBJECT (liste),
		       "key_press_event",
		       GTK_SIGNAL_FUNC (traitement_clavier_liste),
		       NULL );

  
  /* attente du relachement de ctrl+p */

  gtk_signal_connect ( GTK_OBJECT (liste),
		       "key_release_event",
		       GTK_SIGNAL_FUNC (fin_ctrl),
		       NULL );

  /* sauvegarde les redimensionnement des colonnes */

  gtk_signal_connect ( GTK_OBJECT (liste),
		       "resize_column",
		       GTK_SIGNAL_FUNC (changement_taille_colonne),
		       NULL );


  /* on ajoute l'onglet au notebook des comptes */

  CLIST_OPERATIONS = liste;

  /*   par défaut, le classement de la liste s'effectue par date */

  LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				    (GCompareFunc) classement_sliste );


  gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			     onglet,
			     gtk_label_new ( NOM_DU_COMPTE ) );


  remplissage_liste_operations ( no_compte );

     
}
/***************************************************************************************************/



/***************************************************************************************************/
/* Fonction onglet_compte_realize */
/* appelée lorsque la liste est affichée la 1ère fois */
/* permet de se placer en bas de toutes les opés au départ */
/***************************************************************************************************/

void onglet_compte_realize ( GtkWidget *onglet,
			     GtkWidget *liste )
{
  GtkAdjustment *adr_ajustement;

  adr_ajustement = gtk_clist_get_vadjustment ( GTK_CLIST ( liste ));

  gtk_adjustment_set_value ( adr_ajustement,
			     adr_ajustement -> upper -  adr_ajustement -> page_size );


}
/***************************************************************************************************/




/***************************************************************************************************/
/* remplissage de la liste des opérations du compte donné en argument */
/* par les opérations du compte courant */
/***************************************************************************************************/

void remplissage_liste_operations ( gint compte )
{
  GSList *liste_operations_tmp;
  gint couleur_en_cours;
  gchar *ligne_clist[4][7];
  gint i, j;
  gint ligne;
  gdouble montant;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

/* freeze la clist */

  gtk_clist_freeze ( GTK_CLIST ( CLIST_OPERATIONS ));

  /* efface la clist */

  gtk_clist_clear ( GTK_CLIST ( CLIST_OPERATIONS ));

  /*   au départ, les soldes courant et pointé sont = au solde init */

  solde_courant_affichage_liste = SOLDE_INIT;
  solde_pointe_affichage_liste = SOLDE_INIT;
  couleur_en_cours = 0;


  /* on commence la boucle : fait le tour de toutes les opérations */
  /* met à jour les solde_courant_affichage_liste et solde_pointe_affichage_liste */
  /* affiche l'opération à l'écran en fonction de l'affichage de R */

  liste_operations_tmp = LISTE_OPERATIONS;

  while ( liste_operations_tmp )
    {
      struct structure_operation *operation;

      operation = liste_operations_tmp -> data;

      /* si c'est une opé de ventilation, on la saute */

      if ( !operation -> no_operation_ventilee_associee )
	{
	  /* quelle que soit l'opération (relevée ou non), on calcule les soldes courant */
 
	  montant = calcule_montant_devise_renvoi ( operation -> montant,
						    DEVISE,
						    operation -> devise,
						    operation -> une_devise_compte_egale_x_devise_ope,
						    operation -> taux_change,
						    operation -> frais_change );


	  /*   dans tous les cas, on ajoute ce montant au solde courant */
 
	  solde_courant_affichage_liste = solde_courant_affichage_liste + montant;

	  /* si l'opé est pointée ou relevée, on ajoute ce montant au solde pointé */

	  if ( operation -> pointe )
	    solde_pointe_affichage_liste = solde_pointe_affichage_liste + montant;


	  /* si l'opération est relevée et qu'on ne désire pas les afficher, on passe la suite  */

	  if ( AFFICHAGE_R || operation -> pointe != 2 )
	    {
	      /* on fait le tour de tab_affichage_ope pour remplir les lignes du tableau */

	      for ( i=0 ; i<4 ; i++ )
		{
		  /* on ne passe que si la ligne doit être affichée */

		  if ( !i
		       ||
		       NB_LIGNES_OPE == 4
		       ||
		       (( i == 1
			   ||
			   i == 2 )
			 &&
			 NB_LIGNES_OPE == 3 )
		       ||
		       ( i == 1
			 &&
			 NB_LIGNES_OPE == 2 ))
		    {
		      gint ligne_affichee;

		      /*  en fonction de i (la ligne en cours) et NB_LIGNES_OPE, on retrouve la ligne qu'il faut */
		      /* afficher selon les configurations */

		      switch ( NB_LIGNES_OPE )
			{
			case 1:
			  ligne_affichee = ligne_affichage_une_ligne;
			  break;

			case 2:
			  if ( i )
			    ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data );
			  else
			    ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data );
			  break;

			case 3:
			  switch ( i )
			    {
			    case 0:
			      ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data );
			      break;

			    case 1:
			      ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data );
			      break;

			    case 2:
			      ligne_affichee = GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data );
			      break;

			    default:
			      ligne_affichee = 0;
			    }
			  break;

			case 4:
			  ligne_affichee = i;
			  break;

			default:
			  ligne_affichee = 0;
			}


		      for ( j=0 ; j<7 ; j++ )
			ligne_clist[i][j] = recherche_contenu_cellule ( operation,
									tab_affichage_ope[ligne_affichee][j]  );

		      /* on affiche la ligne */

		      ligne = gtk_clist_append ( GTK_CLIST ( CLIST_OPERATIONS ),
						 ligne_clist[i] );

		      /* on associe à cette ligne l'adr de la struct de l'opé */

		      gtk_clist_set_row_data ( GTK_CLIST (CLIST_OPERATIONS),
					       ligne,
					       operation);

		      /* on met la couleur de la ligne */

		      gtk_clist_set_row_style ( GTK_CLIST ( CLIST_OPERATIONS ),
						ligne,
						style_couleur [ couleur_en_cours ] );

		      /* si le solde courant est < 0, on recherche la position de l'affichage */
		      /* du solde et on le met en rouge si on le trouve */

		      if ( solde_courant_affichage_liste < 0 )
			for ( j=0 ; j<7 ; j++ )
			  if ( tab_affichage_ope[ligne_affichee][j] == 7 )
			    gtk_clist_set_cell_style ( GTK_CLIST ( CLIST_OPERATIONS ),
						       ligne,
						       j,
						       style_rouge_couleur [ couleur_en_cours ] );
		    }
		}
	      couleur_en_cours = ( couleur_en_cours + 1 ) % 2;
	    }
	}
      liste_operations_tmp = liste_operations_tmp -> next;
    }


  /* affiche la ligne blanche du bas */

  for ( j=0 ; j<NB_LIGNES_OPE ; j++ )
    {
      /* on met à NULL tout les pointeurs */

      for ( i = 0 ; i < 7 ; i++ )
	ligne_clist[0][i] = NULL;

      ligne = gtk_clist_append ( GTK_CLIST ( CLIST_OPERATIONS ),
				 ligne_clist[0] );

      /* on associe à cette ligne à -1 */

      gtk_clist_set_row_data ( GTK_CLIST (CLIST_OPERATIONS),
			       ligne,
			       GINT_TO_POINTER ( -1 ));

      /* on met la couleur de la ligne */

      gtk_clist_set_row_style ( GTK_CLIST ( CLIST_OPERATIONS ),
				ligne,
				style_couleur [ couleur_en_cours ] );
    }


  /* on enregistre le solde final */

  SOLDE_COURANT = solde_courant_affichage_liste;
  SOLDE_POINTE = solde_pointe_affichage_liste;

  /* on met les soldes en bas */

  gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ),
		       g_strdup_printf ( PRESPACIFY(_("Checked balance: %4.2f %s")),
					 SOLDE_POINTE,
					 ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
											 GINT_TO_POINTER ( DEVISE ),
											 (GCompareFunc) recherche_devise_par_no )-> data )) -> code_devise) );
  gtk_label_set_text ( GTK_LABEL ( solde_label ),
		       g_strdup_printf ( PRESPACIFY(_("Current balance: %4.2f %s")),
					 SOLDE_COURANT,
					 ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
											 GINT_TO_POINTER ( DEVISE ),
											 (GCompareFunc) recherche_devise_par_no )-> data )) -> code_devise) );



  selectionne_ligne ( compte );

  gtk_clist_thaw ( GTK_CLIST ( CLIST_OPERATIONS ));
}
/***************************************************************************************************/



/***************************************************************************************************/
/* Fonction recherche_contenu_cellule */
/* prend en argument l'opération concernée */
/* et le numéro de l'argument qu'on veut afficher (tab_affichage_ope) */
/* renvoie la chaine à afficher ou NULL */
/***************************************************************************************************/

gchar *recherche_contenu_cellule ( struct structure_operation *operation,
				   gint no_affichage )
{
  GSList *liste_tmp;
  gchar *temp;
  gdouble montant;

  switch ( no_affichage )
    {
    case 0:
      return ( NULL );
      break;

      /* mise en forme de la date */

    case 1:
      return ( g_strconcat ( itoa ( operation -> jour ),
			     "/",
			     itoa ( operation -> mois ),
			     "/",
			     itoa ( operation -> annee ),
			     NULL ));
      break;

      /* mise en forme de la date de valeur */

    case 2:
      if ( operation -> jour_bancaire )
	return ( g_strconcat ( itoa ( operation -> jour_bancaire ),
			       "/",
			       itoa ( operation -> mois_bancaire ),
			       "/",
			       itoa ( operation -> annee_bancaire ),
			       NULL ));
      else
	return ( NULL );
      break;

      /* mise en forme du tiers */

    case 3:
      if ( operation -> tiers )
	{
	  liste_tmp = g_slist_find_custom ( liste_struct_tiers,
					    GINT_TO_POINTER ( operation -> tiers ),
					    ( GCompareFunc ) recherche_tiers_par_no );
	
	  if ( liste_tmp )
	    return ( (( struct struct_tiers * )( liste_tmp -> data )) -> nom_tiers );
	  else
	    return ( NULL );
	}
      else
	return ( NULL );
      break;

      /* mise en forme de l'ib */

    case 4:
      temp = NULL;

      if ( operation -> imputation )
	{
	  liste_tmp = g_slist_find_custom ( liste_struct_imputation,
					    GINT_TO_POINTER ( operation -> imputation ),
					    ( GCompareFunc ) recherche_imputation_par_no );

	  if ( liste_tmp )
	    {
	      GSList *liste_tmp_2;
	      
	      liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
						  GINT_TO_POINTER ( operation -> sous_imputation ),
						  ( GCompareFunc ) recherche_sous_imputation_par_no );
	      if ( liste_tmp_2 )
		temp = g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
				     " : ",
				     (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
				     NULL );
	      else
		temp = (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ;
	    }
	}

      return ( temp );
      break;


      /* mise en forme du débit */
    case 5:
      if ( operation -> montant < 0 )
	{
	  temp = g_strdup_printf ( "%4.2f", -operation -> montant );

	  /* si la devise en cours est différente de celle de l'opé, on la retrouve */

	  if ( !devise_operation
	       ||
	       devise_operation -> no_devise != operation -> devise )
	    devise_operation = g_slist_find_custom ( liste_struct_devises,
						     GINT_TO_POINTER ( operation -> devise ),
						     ( GCompareFunc ) recherche_devise_par_no ) -> data;

	  if ( devise_operation -> no_devise != DEVISE )
	    temp = g_strconcat ( temp,
				 "(",
				 devise_operation -> code_devise,
				 ")",
				 NULL );

	  return ( temp );
	}
      else
	return (NULL);

      break;

      /* mise en forme du crédit */
    case 6:
      if ( operation -> montant >= 0 )
	{
	  temp = g_strdup_printf ( "%4.2f", operation -> montant );

	  /* si la devise en cours est différente de celle de l'opé, on la retrouve */

	  if ( !devise_operation
	       ||
	       devise_operation -> no_devise != operation -> devise )
	    devise_operation = g_slist_find_custom ( liste_struct_devises,
						     GINT_TO_POINTER ( operation -> devise ),
						     ( GCompareFunc ) recherche_devise_par_no ) -> data;

	  if ( devise_operation -> no_devise != DEVISE )
	    temp = g_strconcat ( temp,
				 "(",
				 devise_operation -> code_devise,
				 ")",
				 NULL );

	  return ( temp );
	}
      else
	return (NULL);

      break;

      /* mise en forme du solde */
      /* il a déjà été calculé pour l'opération courante, il suffit de le renvoyer */

    case 7:
      return ( g_strdup_printf ( "%4.2f",
				 solde_courant_affichage_liste ));
      break;

      /* mise en forme du montant dans la devise du compte */

    case 8:
      if ( operation -> devise != DEVISE )
	{
	  /* on doit calculer et afficher le montant de l'opé */

	  montant = calcule_montant_devise_renvoi ( operation -> montant,
						    DEVISE,
						    operation -> devise,
						    operation -> une_devise_compte_egale_x_devise_ope,
						    operation -> taux_change,
						    operation -> frais_change );

	  return ( g_strdup_printf ( "(%4.2f %s)",
				     montant,
				     devise_compte -> code_devise ));
	}
      else
	return (NULL);
      
      break;

  /* mise en forme du moyen de paiement */

    case 9:
      if ( operation -> type_ope
	   &&
	   TYPES_OPES )
	{
	  liste_tmp = g_slist_find_custom ( TYPES_OPES,
					    GINT_TO_POINTER ( operation -> type_ope ),
					    (GCompareFunc) recherche_type_ope_par_no );

	  if ( liste_tmp )
	    return ( ((struct struct_type_ope *)(liste_tmp -> data)) -> nom_type );
	  else
	    return ( NULL );
	}
      else
	return ( NULL );

      break;

      /* mise en forme du no de rapprochement */

    case 10:
      if ( operation -> no_rapprochement )
	{
	  liste_tmp = g_slist_find_custom ( liste_no_rapprochements,
					    GINT_TO_POINTER ( operation -> no_rapprochement ),
					    (GCompareFunc) recherche_no_rapprochement_par_no );

	  if ( liste_tmp )
	    return ( ((struct struct_no_rapprochement *)(liste_tmp->data)) -> nom_rapprochement );
	  else
	    return ( NULL );
	}
      else
	return (NULL);

      break;

      /* mise en place de l'exo */

    case 11:
      if ( operation -> no_exercice )
	{
	  liste_tmp = g_slist_find_custom ( liste_struct_exercices,
					    GINT_TO_POINTER ( operation -> no_exercice ),
					    (GCompareFunc) recherche_exercice_par_no );

	  if (liste_tmp)
	    return ( ((struct struct_exercice *)(liste_tmp->data)) -> nom_exercice );
	  else
	    return ( NULL );
	}
      else
	return ( NULL );
      break;

      /* mise en place des catégories */

    case 12:
      temp = NULL;

      if ( operation -> categorie )
	{
	  /* c'est une categ : ss categ */

	  liste_tmp = g_slist_find_custom ( liste_struct_categories,
					    GINT_TO_POINTER ( operation -> categorie ),
					    ( GCompareFunc ) recherche_categorie_par_no );

	  if ( liste_tmp )
	    {
	      GSList *liste_tmp_2;
	      
	      liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
						  GINT_TO_POINTER ( operation -> sous_categorie ),
						  ( GCompareFunc ) recherche_sous_categorie_par_no );
	      if ( liste_tmp_2 )
		temp = g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
				     " : ",
				     (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
				     NULL );
	      else
		temp = (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ;
	      
	    }
	}
      else
	{
	  if ( operation -> relation_no_operation )
	    {
	      /* c'est un virement */

	      if ( operation -> relation_no_compte == -1 )
		{
		  if ( operation -> montant < 0 )
		    temp = _("Transfer to a deleted account");
		  else
		    temp = _("Transfer from a deleted account");
		}
	      else
		{
		  gpointer **save_ptab;

		  save_ptab = p_tab_nom_de_compte_variable;
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		  if ( operation -> montant < 0 )
		    temp = g_strdup_printf ( _("Transfer to %s"),
					     NOM_DU_COMPTE );
		  else
		    temp = g_strdup_printf ( _("Transfer from %s"),
					     NOM_DU_COMPTE );

		  p_tab_nom_de_compte_variable = save_ptab;
		}
	    }
	  else
	    {
	      /* vérification si l'opération est ventilée */

	      if ( operation -> operation_ventilee )
		temp = _("Breakdown of transaction");
	    }
	}
      return ( temp );

      break;

      /* mise en forme R/P */

    case 13:
      if ( operation -> pointe == 1 )
	return ( "P" );
      else
	{
	  if ( operation -> pointe == 2 )
	    return ( "R" );
	  else
	    return ( NULL );
	}
      break;


      /* mise en place de la pièce comptable */

    case 14:
      return ( operation -> no_piece_comptable );
      break;

      /* mise en forme des notes */

    case 15:
      return ( operation -> notes );
      break;

      /* mise en place de l'info banque/guichet */

    case 16:
      return ( operation -> info_banque_guichet );
      break;

      /* mise en place du no d'opé */

    case 17:
      return ( itoa ( operation -> no_operation ));
      break;

      /* mise en place du no de chèque/virement */

    case 18:

      if ( operation -> contenu_type )
	return ( g_strconcat ( "(",
			       operation -> contenu_type,
			       ")",
			       NULL ));
      else
	return (NULL);
      break;
    }
  return ( NULL );
}
/***************************************************************************************************/


/***************************************************************************************************/
/* Fonction selectionne_ligne_souris */
/* place la sélection sur l'opé clickée */
/***************************************************************************************************/

void selectionne_ligne_souris ( GtkCList *liste,
				GdkEventButton *evenement,
				gpointer data)
{
  gint colonne, x, y;
  gint ligne;


  /*   si le click se situe dans les menus, c'est qu'on redimensionne, on fait rien */

  if ( evenement -> window != liste ->clist_window )
    return;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( CLIST_OPERATIONS ),
				 "button_press_event");



  /* Récupération des coordonnées de la souris */

  gdk_window_get_pointer ( GTK_CLIST ( CLIST_OPERATIONS ) -> clist_window,
			   &x,
			   &y,
			   FALSE );

  gtk_clist_get_selection_info ( GTK_CLIST ( CLIST_OPERATIONS ),
				 x,
				 y,
				 &ligne,
				 &colonne);

  if ( !gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
				 ligne ) )
    return;


  if ( etat.equilibrage
       &&
       colonne == 3
       &&
       !(ligne % NB_LIGNES_OPE) )
    pointe_equilibrage ( ligne );


  /* Récupération de la 1ère ligne de l'opération cliquée */

  ligne = ligne / NB_LIGNES_OPE * NB_LIGNES_OPE;


  /*   vire l'ancienne sélection */

  gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ) );

  /* on met l'adr de la struct dans OPERATION_SELECTIONNEE */

  OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
						    ligne );

  selectionne_ligne( compte_courant );

  if ( evenement -> type == GDK_2BUTTON_PRESS )
    edition_operation ();
  else
    focus_a_la_liste ();

}
/***************************************************************************************************/





/***************************************************************************************************/
/* Fonction traitement_clavier_liste */
/* gère le clavier sur la clist */
/***************************************************************************************************/

gboolean traitement_clavier_liste ( GtkCList *liste,
				     GdkEventKey *evenement,
				     gpointer origine)
{
  gint ligne;


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste ),
				 "key_press_event");

  switch ( evenement->keyval )
    {
    case 65293 :
    case 65421 :
                         /* entrée */

      edition_operation ();
      break;


    case 65362 :                       /* flèche haut  */

      ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
					     OPERATION_SELECTIONNEE );
      if ( ligne )
	{
	  ligne= ligne - NB_LIGNES_OPE;

	  OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
							    ligne );
	  gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
	  selectionne_ligne ( compte_courant );

	}
      break;


    case 65364 :                /* flèche bas */

      if ( OPERATION_SELECTIONNEE != GINT_TO_POINTER ( -1 ) )
	{
	  ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
						 OPERATION_SELECTIONNEE );

	  ligne= ligne + NB_LIGNES_OPE;

	  OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
							    ligne );
	  gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
	  selectionne_ligne ( compte_courant );

	}
      break;


    case 65535 :               /*  del  */

      supprime_operation ( OPERATION_SELECTIONNEE );
      break;


    case 65507 :
      /* touche ctrl gauche */
    case 65508:
      /* touche ctrl droite */

      ctrl_press ();
      break;


    case 112 :                /* touche p */
    case 80 :                /* touche P */

      p_press ();
      break;

    case 114 :                /* touche r */
    case 82 :                /* touche R */

      r_press ();
      break;

    default : return (TRUE);
    }


  return (TRUE);
}
/***************************************************************************************************/


/***************************************************************************************************/
void ctrl_press ( void )
{
  etat.ctrl = 1;
}
/***************************************************************************************************/


/***************************************************************************************************/
void fin_ctrl ( GtkCList *liste_operations,
		    GdkEventKey *evenement,
		    gpointer origine)
{
  if ( evenement->keyval == 65507 )
    etat.ctrl = 0;
}
/***************************************************************************************************/



/***************************************************************************************************/
/* Routine qui sélectionne ou désélectionne l'opération */
/***************************************************************************************************/

void selectionne_ligne ( gint compte )
{
  gint ligne;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

  gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ));

  ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
					 OPERATION_SELECTIONNEE );

  if ( ligne == -1 )
    {
      OPERATION_SELECTIONNEE = GINT_TO_POINTER ( -1 );
      ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
					     OPERATION_SELECTIONNEE );
    }


  switch ( NB_LIGNES_OPE )
    {
    case 4:
      gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
			     ligne+3,
			     0);
    case 3:
      gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
			     ligne+2,
			     0);
    case 2:
      gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
			     ligne+1,
			     0);
    case 1:
      gtk_clist_select_row ( GTK_CLIST ( CLIST_OPERATIONS ),
			     ligne,
			     0);
    }

  if ( gtk_clist_row_is_visible ( GTK_CLIST ( CLIST_OPERATIONS ),
				  ligne + NB_LIGNES_OPE - 1)
       != GTK_VISIBILITY_FULL
       ||
       gtk_clist_row_is_visible ( GTK_CLIST ( CLIST_OPERATIONS ),
				  ligne )
       != GTK_VISIBILITY_FULL )
    {
      if ( ligne > gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value  / GTK_CLIST ( CLIST_OPERATIONS ) -> row_height  )
	gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
			   ligne + NB_LIGNES_OPE - 1,
			   0,
			   1,
			   0 );
      else
	gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
			   ligne,
			   0,
			   0,
			   0 );
    }
}
/***************************************************************************************************/





/***************************************************************************************************/
/* Fonction edition_operation */
/* appelée lors d'un double click sur une ligne ou entree */
/* place l'opération sélectionnée dans le formulaire */
/***************************************************************************************************/

void edition_operation ( void )
{
  struct structure_operation *operation;
  gchar date [11];
  gchar date_bancaire [11];
  GSList *liste_tmp;
  struct struct_devise *devise;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  operation = OPERATION_SELECTIONNEE;
  formulaire_a_zero ();

/* on affiche le formulaire sans modifier l'état */
/* => si il n'est pas affiché normalement, il sera efface lors du prochain formulaire_a_zero */

  if ( !etat.formulaire_toujours_affiche )
    {
      gtk_widget_show ( frame_droite_bas );
      gtk_signal_connect ( GTK_OBJECT ( frame_droite_bas ),
			   "draw",
			   GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_visible ),
			   NULL );
    }

  gtk_widget_set_sensitive ( bouton_affiche_cache_formulaire,
			     FALSE );


/* si l'opé est -1, c'est que c'est une nouvelle opé */

  if ( operation == GINT_TO_POINTER ( -1 ) )
    {
      clique_champ_formulaire ( widget_formulaire_operations[1],
				NULL,
				GINT_TO_POINTER ( 1 ));
      gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_operations[1]),
				0,
				-1);
      gtk_widget_grab_focus ( GTK_WIDGET ( widget_formulaire_operations[1] ));
      return;
    }


/*   l'opé n'est pas -1, c'est une modif, on remplit les champs */

  degrise_formulaire_operations ();

  gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			"adr_struct_ope",
			operation );
  

  /* on met le no de l'opé */

  gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[0] ),
		       itoa ( operation -> no_operation ));

  /* mise en forme de la date */

  g_date_strftime (  date,
		     11,
		     "%02d/%02m/%04Y",
		     operation -> date);

  entree_prend_focus ( widget_formulaire_operations[1] );
  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[1] ),
		       date );


  /* mise en forme du tiers */

  if ( operation -> tiers )
    {
      liste_tmp = g_slist_find_custom ( liste_struct_tiers,
					GINT_TO_POINTER ( operation -> tiers ),
					( GCompareFunc ) recherche_tiers_par_no );

      entree_prend_focus ( widget_formulaire_operations[2] );
      gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ),
			      (( struct struct_tiers * )( liste_tmp -> data )) -> nom_tiers );
    }


  /* mise en forme du débit / crédit */

  if ( operation -> montant < 0 )
    {
      entree_prend_focus ( widget_formulaire_operations[3] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[3] ),
			   g_strdup_printf ( "%4.2f", -operation -> montant ));
      /* met le menu des types débits */

      if ( !etat.affiche_tous_les_types )
	{
	  GtkWidget *menu;

	  if ( (menu = creation_menu_types ( 1, compte_courant, 0  )))
	    {
	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					 menu );
	      gtk_widget_show ( widget_formulaire_operations[9] );
	    }
	  else
	    gtk_widget_hide ( widget_formulaire_operations[9] );
	}
    }
  else
    {
      entree_prend_focus ( widget_formulaire_operations[4] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[4] ),
			   g_strdup_printf ( "%4.2f", operation -> montant ));
      /* met le menu des types crédits */

      if ( !etat.affiche_tous_les_types )
	{
	  GtkWidget *menu;

	  if ( (menu = creation_menu_types ( 2, compte_courant, 0 )))
	    {
	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					 menu );
	      gtk_widget_show ( widget_formulaire_operations[9] );
	    }
	  else
	    {
	      gtk_widget_hide ( widget_formulaire_operations[9] );
	      gtk_widget_hide ( widget_formulaire_operations[10] );
	    }
	}
    }


  /* si l'opération est relevée, on désensitive les entrées de crédit et débit */

  if ( operation -> pointe == 2 )
    {
      gtk_widget_set_sensitive ( widget_formulaire_operations[4],
				 FALSE );
      gtk_widget_set_sensitive ( widget_formulaire_operations[3],
				 FALSE );
    }

  /* mise en forme de la devise */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( operation -> devise ),
									 ( GCompareFunc ) recherche_devise_par_no )));

  /*   si la devise n'est pas celle du compte ni l'euro si le compte va y passer, affiche le bouton change */

  if ( !devise_compte
       ||
       devise_compte -> no_devise != DEVISE )
    devise_compte = g_slist_find_custom ( liste_struct_devises,
					  GINT_TO_POINTER ( DEVISE ),
					  ( GCompareFunc ) recherche_devise_par_no ) -> data;
  devise = g_slist_find_custom ( liste_struct_devises,
				 GINT_TO_POINTER ( operation -> devise ),
				 ( GCompareFunc ) recherche_devise_par_no ) -> data;

  if ( !( devise -> no_devise == DEVISE
	  ||
	  ( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	  ||
	  ( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
    gtk_widget_show ( widget_formulaire_operations[6] );


  /* mise en forme des catégories */

  liste_tmp = g_slist_find_custom ( liste_struct_categories,
				    GINT_TO_POINTER ( operation -> categorie ),
				    ( GCompareFunc ) recherche_categorie_par_no );

  if ( liste_tmp )
    {
      GSList *liste_tmp_2;
      
      liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
					  GINT_TO_POINTER ( operation -> sous_categorie ),
					  ( GCompareFunc ) recherche_sous_categorie_par_no );

      entree_prend_focus ( widget_formulaire_operations[8] );
 
      if ( liste_tmp_2 )
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
				g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
					      " : ",
					      (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
					      NULL ));
      else
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
				(( struct struct_categ * )( liste_tmp -> data )) -> nom_categ );
      
    }


  /* mise en forme de la date réelle */

  if ( operation->date_bancaire )
    {
      g_date_strftime (  date_bancaire,
			 11,
			 "%02d/%02m/%04Y",
			 operation -> date_bancaire );
      entree_prend_focus ( widget_formulaire_operations[7] );

      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[7] ),
			   date_bancaire );
    }



  /* si l'opération est liée, marque le virement */
  /*   et si la contre opération est relevée, on désensitive la categ et le montant */

  if ( operation -> relation_no_operation )
    {
      entree_prend_focus ( widget_formulaire_operations[8] );

      if ( operation -> relation_no_compte == -1 )
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
				_("Transfer: deleted account") );
      else
	{
	  GtkWidget *menu;
	  struct structure_operation *operation_2;

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	  /* on met le nom du compte du virement */

	  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
				  g_strconcat ( COLON(_("Transfer")),
						NOM_DU_COMPTE,
						NULL ));

	  /* si l'opération est relevée, on empêche le changement de virement */

	  if ( operation -> pointe == 2 )
	    gtk_widget_set_sensitive ( widget_formulaire_operations[8],
				       FALSE );

	  /* récupération de la contre opération */

	  operation_2 = g_slist_find_custom ( LISTE_OPERATIONS,
					      GINT_TO_POINTER ( operation -> relation_no_operation ),
					      (GCompareFunc) recherche_operation_par_no ) -> data;

	  /* 	  si la contre opération est relevée, on désensitive les categ et les montants */

	  if ( operation_2 -> pointe == 2 )
	    {
	      gtk_widget_set_sensitive ( widget_formulaire_operations[4],
					 FALSE );
	      gtk_widget_set_sensitive ( widget_formulaire_operations[3],
					 FALSE );
	      gtk_widget_set_sensitive ( widget_formulaire_operations[8],
					 FALSE );
	    }

	  /* comme c'est un virement, on affiche s'il existe l'option menu du type de l'autre opé */

	  if ( operation -> montant >= 0 )
	    menu = creation_menu_types ( 1, operation -> relation_no_compte, 2  );
	  else
	    menu = creation_menu_types ( 2, operation -> relation_no_compte, 2  );

	  /*  on ne continue que si un menu a été créé */
	  /*    dans ce cas, on va chercher l'autre opé et retrouve le type */

	  if ( menu )
	    {

	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ),
					 menu );
	      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ),
					    cherche_no_menu_type_associe ( operation_2 -> type_ope,
									   0 ));
	      gtk_widget_show ( widget_formulaire_operations[13] );

	    }
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	}
    }

  /* mise en forme si l'opération est ventilée */

  if ( operation -> operation_ventilee )
    {
      entree_prend_focus ( widget_formulaire_operations[8] );
      gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
			      _("Breakdown of transaction") );
      gtk_widget_show ( widget_formulaire_operations[15] );
      gtk_widget_set_sensitive ( widget_formulaire_operations[11],
				 FALSE );
      gtk_widget_set_sensitive ( widget_formulaire_operations[12],
				 FALSE );

      /* met la liste des opés de ventilation dans liste_adr_ventilation */

      gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			    "liste_adr_ventilation",
			    creation_liste_ope_de_ventil ( operation ));

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
    }


  /* met l'option menu du type d'opé */

  if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] ))
    {
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
				    cherche_no_menu_type ( operation -> type_ope ));
      if ( operation -> type_ope
	   &&
	   ( liste_tmp = g_slist_find_custom ( TYPES_OPES,
				       GINT_TO_POINTER ( operation -> type_ope ),
				       (GCompareFunc) recherche_type_ope_par_no )))
	{
	  struct struct_type_ope *type;

	  type = liste_tmp -> data;

	  if ( type -> affiche_entree
	       &&
	       operation -> contenu_type )
	    {
	      entree_prend_focus ( widget_formulaire_operations[10] );
	      gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[10] ),
				   operation -> contenu_type );
	    }
	}
    }


  /* met en place l'exercice */

  gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_operations[11] ),
				 cherche_no_menu_exercice ( operation -> no_exercice,
							    widget_formulaire_operations[11] ));

  /* met en place l'imputation budgétaire */
  /* si c'est une opé ventilée, on met rien */

  if ( !operation -> operation_ventilee )
    {
      liste_tmp = g_slist_find_custom ( liste_struct_imputation,
					GINT_TO_POINTER ( operation -> imputation ),
					( GCompareFunc ) recherche_imputation_par_no );

      if ( liste_tmp )
	{
	  GSList *liste_tmp_2;

	  entree_prend_focus ( widget_formulaire_operations[12]);

	  liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( liste_tmp -> data )) -> liste_sous_imputation,
					      GINT_TO_POINTER ( operation -> sous_imputation ),
					      ( GCompareFunc ) recherche_sous_imputation_par_no );
	  if ( liste_tmp_2 )
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[12] ),
				    g_strconcat ( (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation,
						  " : ",
						  (( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation,
						  NULL ));
	  else
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[12] ),
				    (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation );
	}
    }

  /* mise en place de la pièce comptable */

  if ( operation -> no_piece_comptable )
    {
      entree_prend_focus ( widget_formulaire_operations[14] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[14] ),
			   operation -> no_piece_comptable );
    }



  /*   remplit les notes */

  if ( operation -> notes )
    {
      entree_prend_focus ( widget_formulaire_operations[16] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[16] ),
			   operation -> notes );
    }

  /*   remplit les infos guichet / banque */

  if ( operation -> info_banque_guichet )
    {
      entree_prend_focus ( widget_formulaire_operations[17] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[17] ),
			   operation -> info_banque_guichet );
    }



/* mise en forme de auto / man */

  if ( operation -> auto_man )
    gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[18]),
			 _("Auto"));
  else
    gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[18]),
			 _("Manual"));



/*   on a fini de remplir le formulaire, on donne le focus à la date */

  gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_operations[1] ),
			    0,
			    -1);
  gtk_widget_grab_focus ( widget_formulaire_operations[1] );
}
/***************************************************************************************************/




/***************************************************************************************************/
/* Fonction p_press */
/* appelée lorsque la touche p est pressée sur la liste */
/* pointe ou dépointe l'opération courante */
/***************************************************************************************************/

void p_press (void)
{
  gdouble montant;

/*   si ctrl n'est pas enfoncé, on se barre */

  if ( etat.ctrl != 1 )
    return;


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;


  /* si on est sur l'opération vide -> on se barre */

  if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER ( -1 ) )
    return;

  if ( OPERATION_SELECTIONNEE -> pointe == 2 )
    return;

  if ( OPERATION_SELECTIONNEE -> pointe )
    {
      montant = calcule_montant_devise_renvoi ( OPERATION_SELECTIONNEE -> montant,
						DEVISE,
						OPERATION_SELECTIONNEE -> devise,
						OPERATION_SELECTIONNEE -> une_devise_compte_egale_x_devise_ope,
						OPERATION_SELECTIONNEE -> taux_change,
						OPERATION_SELECTIONNEE -> frais_change );

      if ( etat.equilibrage )
	operations_pointees = operations_pointees - montant;

      SOLDE_POINTE = SOLDE_POINTE - montant;
      OPERATION_SELECTIONNEE -> pointe = 0;

      gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			   gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							  OPERATION_SELECTIONNEE ),
			   3,
			   NULL );

      modification_fichier( TRUE );
    }
  else
    {
      montant = calcule_montant_devise_renvoi ( OPERATION_SELECTIONNEE -> montant,
						DEVISE,
						OPERATION_SELECTIONNEE -> devise,
						OPERATION_SELECTIONNEE -> une_devise_compte_egale_x_devise_ope,
						OPERATION_SELECTIONNEE -> taux_change,
						OPERATION_SELECTIONNEE -> frais_change );

      if ( etat.equilibrage )
	operations_pointees = operations_pointees + montant;

      SOLDE_POINTE = SOLDE_POINTE + montant;
      OPERATION_SELECTIONNEE -> pointe = 1;

      gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			   gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							  OPERATION_SELECTIONNEE ),
			   3,
			   "P");
      modification_fichier( TRUE );
    }

  /* si c'est une opé ventilée, on recherche les opé filles pour leur mettre le même pointage que la mère */

  if ( OPERATION_SELECTIONNEE -> operation_ventilee )
    {
      /* p_tab est déjà pointé sur le compte courant */

      GSList *liste_tmp;

      liste_tmp = LISTE_OPERATIONS;

      while ( liste_tmp )
	{
	  struct structure_operation *ope_fille;

	  ope_fille = liste_tmp -> data;

	  if ( ope_fille -> no_operation_ventilee_associee == OPERATION_SELECTIONNEE -> no_operation )
	    ope_fille -> pointe = OPERATION_SELECTIONNEE -> pointe;

	  liste_tmp = liste_tmp -> next;
	}
    }


  if ( etat.equilibrage )
    {
      gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
			   g_strdup_printf ("%4.2f",
					    operations_pointees ));

      if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
	{
	  gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			       g_strdup_printf ( "%4.2f",
						 0.0 ));
	  gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				     TRUE );
	}
      else
	{
	  gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			       g_strdup_printf ( "%4.2f",
						 solde_final - solde_initial - operations_pointees ));
	  gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				     FALSE );
	}
    }


  /* p_tab est déjà sur le compte courant */

  if ( !devise_compte
       ||
       devise_compte -> no_devise != DEVISE )
    devise_compte = g_slist_find_custom ( liste_struct_devises,
					  GINT_TO_POINTER ( DEVISE ),
					  ( GCompareFunc ) recherche_devise_par_no ) -> data;

  /* met le label du solde pointé */

  gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ),
		       g_strdup_printf ( PRESPACIFY(_("Checked balance: %4.2f %s")),
					 SOLDE_POINTE,
					 devise_compte -> code_devise) );
}
/***************************************************************************************************/






/***************************************************************************************************/
/* Fonction r_press */
/* appelée lorsque la touche r est pressée sur la liste */
/* relève ou dérelève l'opération courante */
/***************************************************************************************************/

void r_press (void)
{

/*   si ctrl n'est pas enfoncé, on se barre */

  if ( etat.ctrl != 1 )
    return;


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;


  /* si on est sur l'opération vide -> on se barre */

  if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER ( -1 ) )
    return;


  if ( OPERATION_SELECTIONNEE -> pointe == 0 )
    {
      /* on relève l'opération */

      OPERATION_SELECTIONNEE -> pointe = 2;

      /*       on met soit le R, soit on change la sélection vers l'opé suivante */

      if ( AFFICHAGE_R )
	gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							    OPERATION_SELECTIONNEE ),
			     3,
			     "R");
      else
	{
	  /* l'opération va disparaitre, on met donc la sélection sur l'opé suivante */

	  gint ligne;

	  ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
						 OPERATION_SELECTIONNEE );
	  OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							    ligne + NB_LIGNES_OPE );
	  remplissage_liste_operations ( compte_courant );
	}

      modification_fichier( TRUE );
    }
  else
    if ( OPERATION_SELECTIONNEE -> pointe == 2 )
      {
	/* 	  dé-relève l'opération */
	OPERATION_SELECTIONNEE -> pointe = 0;
	gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							    OPERATION_SELECTIONNEE ),
			     3,
			     NULL );


	modification_fichier( TRUE );
      }

  /*   à ce niveau, on reteste OPERATION_SELECTIONNEE car comme on a peut être déplacé */
  /*     la sélection vers le bas, elle peut être revenue à -1 */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  if ( OPERATION_SELECTIONNEE == GINT_TO_POINTER ( -1 ) )
    return;

  /* si c'est une ventil */
  /* fait le tour des opés du compte pour rechercher les opés de ventil associées à */
  /* cette ventil */


  if ( OPERATION_SELECTIONNEE -> operation_ventilee )
    {
      GSList *liste_tmp;

      liste_tmp = LISTE_OPERATIONS;

      while ( liste_tmp )
	{
	  struct structure_operation *operation;

	  operation = liste_tmp -> data;

	  if ( operation -> no_operation_ventilee_associee == OPERATION_SELECTIONNEE -> no_operation )
	    operation -> pointe = OPERATION_SELECTIONNEE -> pointe;

	  liste_tmp = liste_tmp -> next;
	}
    }

}
/***************************************************************************************************/



/********************************************************************************************************/
/*  Routine qui supprime l'opération donnée en argument */
/****************************************************************************************************/

void supprime_operation ( struct structure_operation *operation )
{

  gint no_compte;


  if ( operation == GINT_TO_POINTER ( -1 ) )
    return;

  no_compte = operation -> no_compte;

  /* vérifications de bases */

  /* l'opération ne doit pas être pointée */

  if ( operation -> pointe == 2 )
    {
      dialogue ( SPACIFY(_("Imposible to delete a reconciled transaction...")));
      return;
    }

  /* si l'opération est liée, on recherche l'autre opé on vire ses liaisons et on l'efface */
  /*   sauf si elle est relevée, dans ce cas on annule tout */

  if ( operation -> relation_no_operation && operation -> relation_no_compte != -1 )
    {
      GSList *liste_tmp;
      struct structure_operation *ope_liee;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

      liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
					GINT_TO_POINTER ( operation -> relation_no_operation ),
					( GCompareFunc ) recherche_operation_par_no );

      if ( liste_tmp )
	{
	  ope_liee =  liste_tmp -> data;

	  if ( ope_liee -> pointe == 2 )
	    {
	      dialogue ( SPACIFY(_("The contra-transaction of this transfer is reconciled,\ndeletion impossible...")));
	      return;
	    }

	  ope_liee -> relation_no_operation = 0;
	  ope_liee -> relation_no_compte = 0;

	  supprime_operation ( ope_liee );
	}
    }

  /* si c'est une ventilation, on fait le tour de ses opés de ventil pour vérifier qu'il */
  /* n'y en a pas une qui est un virement vers une opération relevée */

  if ( operation -> operation_ventilee )
    {
      GSList *pointeur_tmp;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

     pointeur_tmp = LISTE_OPERATIONS;

      while ( pointeur_tmp )
	{
	  struct structure_operation *ope_test;

	  ope_test = pointeur_tmp -> data;

	  if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation )
	    {
	      /* ope_test est une opé de ventil de l'opération à supprimer, recherche si c'est un virement */

	      if ( ope_test -> relation_no_operation )
		{
		  /* c'est un virement, on va voir la contre opération */

		  struct structure_operation *contre_operation;

		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_test -> relation_no_compte;

		  contre_operation = g_slist_find_custom ( LISTE_OPERATIONS,
							   GINT_TO_POINTER ( ope_test -> relation_no_operation ),
							   (GCompareFunc) recherche_operation_par_no ) -> data;

		  if ( contre_operation -> pointe == 2 )
		    {
		      dialogue ( _("One of the breakdown lines is a transfer whose contra-transaction is reconciled.\nDeletion canceled..."));
		      return;
		    }
		}
	    }
	  pointeur_tmp = pointeur_tmp -> next;
	}
    }


  /*   les tests sont passés, si c'est une ventilation, on vire toutes les opés associées */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

  if ( operation -> operation_ventilee )
    {
      GSList *pointeur_tmp;

      pointeur_tmp = LISTE_OPERATIONS;

      while ( pointeur_tmp )
	{
	  struct structure_operation *ope_test;

	  ope_test = pointeur_tmp -> data;

	  if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation )
	    {
	      /* on se place tout de suite sur l'opé suivante */

	      pointeur_tmp = pointeur_tmp -> next;

	      supprime_operation ( ope_test );
	    }
	  else
	    pointeur_tmp = pointeur_tmp -> next;
	}
    }

  /*   si la sélection est sur l'opé qu'on supprime, on met la sélection sur celle du dessous */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

  if ( OPERATION_SELECTIONNEE == operation )
    {
      gint ligne;

      ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
					     operation );
      OPERATION_SELECTIONNEE = gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							ligne + NB_LIGNES_OPE );
      gtk_clist_unselect_all ( GTK_CLIST ( CLIST_OPERATIONS ) );
      selectionne_ligne ( operation -> no_compte );
    }

  /* supprime l'opération dans la liste des opés */

  LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS,
				      operation );
  NB_OPE_COMPTE--;


  /* on met à jour, immédiatement si on a la liste affichée */

  MISE_A_JOUR = 1;

  if ( no_compte == compte_courant )
    verification_mise_a_jour_liste ();


/* si on est en train d'équilibrer => recalcule le total pointé */

  if ( etat.equilibrage )
    {
      GSList *pointeur_liste_ope;

      p_tab_nom_de_compte_variable =  p_tab_nom_de_compte_courant;

      pointeur_liste_ope = LISTE_OPERATIONS;
      operations_pointees = 0;

      while ( pointeur_liste_ope )
	{
	  struct structure_operation *operation;

	  operation = pointeur_liste_ope -> data;

	  if ( operation -> pointe == 1 )
	    operations_pointees = operations_pointees + operation -> montant;

	  pointeur_liste_ope = pointeur_liste_ope -> next;
	}

      gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
			   g_strdup_printf ( "%4.2f", 
					     operations_pointees ));

      if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
	{
	  gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			       g_strdup_printf ( "%4.2f",
						 0.0 ));
	  gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				     TRUE );
	}
      else
	{
	  gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			       g_strdup_printf ( "%4.2f",
						 solde_final - solde_initial - operations_pointees ));
	  gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				     FALSE );
	}
    }


  /* on réaffiche la liste de l'état des comptes de l'accueil */

  mise_a_jour_solde ( no_compte );
  update_liste_comptes_accueil ();

  /* on réaffiche la liste des tiers */

  mise_a_jour_tiers ();
  mise_a_jour_categ ();
  mise_a_jour_imputation ();
  modification_fichier( TRUE );

}
/****************************************************************************************************/


/****************************************************************************************************/
/* Fonction recherche_operation_par_no */
/* appelée par un slist_find_custom */
/* recherche une opé par son numéro d'opé dans la liste des opérations */
/****************************************************************************************************/

gint recherche_operation_par_no ( struct structure_operation *operation,
				  gint *no_ope )
{

  return ( ! ( operation -> no_operation == GPOINTER_TO_INT ( no_ope ) ));

}
/****************************************************************************************************/




/* ***************************************************************************************************** */
/* Fonction changement_taille_liste_ope */
/* appelée dès que la taille de la clist a changé */
/* pour mettre la taille des différentes colonnes */
/* ***************************************************************************************************** */

void changement_taille_liste_ope ( GtkWidget *clist,
				   GtkAllocation *allocation,
				   gint *compte )
{
  gint i;
  gint largeur;
  gint col0, col1, col2, col3, col4, col5, col6;


  /*   si la largeur de grisbi est < 700, on fait rien */

  if ( window -> allocation.width < 700 )
    return;

  if ( allocation )
    largeur = allocation->width;
  else
    largeur = clist -> allocation.width;

  /*   si la largeur est automatique, on change la largeur des colonnes */
  /*     sinon, on y met les valeurs fixes */

  if ( etat.largeur_auto_colonnes )
    for ( i=0 ; i<7 ; i++ )
      gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				   i,
				   rapport_largeur_colonnes[i] * largeur / 100 );
  else
    for ( i=0 ; i<7 ; i++ )
      gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				   i,
				   taille_largeur_colonnes[i] );


/* met les entrées du formulaire selon une taille proportionnelle */

  col0 = largeur * 5 / 100;
  col1 = largeur * 12 / 100;
  col2 = largeur * 30 / 100;
  col3 = largeur * 12 / 100;
  col4 = largeur * 12 / 100;
  col5 = largeur * 12 / 100;
  col6 = largeur * 12 / 100;
 
  /* 1ère ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[0] ),
			 col0,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[1] ),
			 col1,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[2] ),
			 col2,
			 FALSE  );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[3] ),
			 col3,
			 FALSE  );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[4] ),
			 col4,
			 FALSE  );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[5] ),
			 col5,
			 FALSE  );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[6] ),
			 col6,
			 FALSE  );

  /* 2ème ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[7] ),
			 col0+col1,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[8] ),
			 col2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[9] ),
			 col3+col4,
			 FALSE  );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[10] ),
			 col5,
			 FALSE  );

  /* 3ème ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[11] ),
			 col0+col1,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[12] ),
			 col2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[14] ),
			 col5,
			 FALSE  );

  /* 4ème ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[15] ),
			 col0+col1,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[16] ),
			 col2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[17] ),
			 col3+col4+col5,
			 FALSE  );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_operations[18] ),
			 col6,
			 FALSE  );
}
/* ***************************************************************************************************** */


/* ***************************************************************************************************** */
/* Fonction focus_a_la_liste */
/* donne le focus à la liste des opés en cours */
/* ***************************************************************************************************** */

void focus_a_la_liste ( void )
{
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
  gtk_widget_grab_focus ( GTK_WIDGET ( CLIST_OPERATIONS ) );
}
/* ***************************************************************************************************** */


/* ***************************************************************************************************** */
/* Fonction  demande_mise_a_jour_tous_comptes */
/* met la variable MISE_A_JOUR de tous les comptes à 1 */
/* ce qui fait que lorsqu'ils seront affichés, ils seront mis à jour avant */
/* ***************************************************************************************************** */

void demande_mise_a_jour_tous_comptes ( void )
{
  gint i;
  gpointer **save_p_tab;

  save_p_tab = p_tab_nom_de_compte_variable;
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {
      MISE_A_JOUR = 1;
      p_tab_nom_de_compte_variable++;
    }

  p_tab_nom_de_compte_variable = save_p_tab;
}
/* ***************************************************************************************************** */


/* ***************************************************************************************************** */
/* Fonction verification_mise_a_jour_liste */
/* appelée à chaque affichage de la liste */
/* vérifie que la liste ne doit pas être réaffichée */
/* ***************************************************************************************************** */

void verification_mise_a_jour_liste ( void )
{
  GtkAdjustment *ajustement;
  gfloat haut, bas, value, page_size;
  gfloat new_bas, new_value;
  gint compte;

  compte = gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_listes_operations )) - 1;

  if ( compte < 0 )
    return;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

  if ( !MISE_A_JOUR )
    return;


  ajustement = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS ));

  haut = ajustement -> upper;
  bas = ajustement -> lower + ajustement -> page_size;
  value = ajustement -> value;
  page_size = ajustement -> page_size;

  LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				    (GCompareFunc) classement_sliste );

  remplissage_liste_operations ( GPOINTER_TO_INT ( compte ) );

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

  if ( ajustement -> page_size == ajustement -> upper )
    gtk_adjustment_set_value ( GTK_ADJUSTMENT ( ajustement ),
			       ajustement -> lower  );
  else
    if ( page_size == haut )
      {
	gtk_clist_moveto ( GTK_CLIST ( CLIST_OPERATIONS ),
			   gtk_clist_find_row_from_data ( GTK_CLIST ( CLIST_OPERATIONS ),
							  OPERATION_SELECTIONNEE ),
			   0,
			   0.5,
			   0 );
	selectionne_ligne ( compte );
      }
    else
      {
	new_bas = ajustement -> lower + ajustement -> page_size;
	
	new_value = ( ajustement -> upper - new_bas ) * value / ( haut - bas );
	
	gtk_adjustment_set_value ( GTK_ADJUSTMENT ( ajustement ),
				   new_value );
      }
  MISE_A_JOUR = 0;
}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction mise_a_jour_solde */
/* recalcule le solde du compte demandé */
/* ***************************************************************************************************** */

void mise_a_jour_solde ( gint compte )
{
  gdouble solde_courant;
  GSList *liste_operations_tmp;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte;

  /* on fait le tour de toutes les opérations */

  solde_courant = SOLDE_INIT;

  liste_operations_tmp = LISTE_OPERATIONS;

  while ( liste_operations_tmp )
    {
      struct structure_operation *operation;

      operation = liste_operations_tmp -> data;

      /* si c'est une opé de ventilation, on la saute */

      if ( !operation -> no_operation_ventilee_associee )
	{
	  /* calcul du solde courant */

	  gdouble montant;

	  montant = calcule_montant_devise_renvoi ( operation -> montant,
						    DEVISE,
						    operation -> devise,
						    operation -> une_devise_compte_egale_x_devise_ope,
						    operation -> taux_change,
						    operation -> frais_change );
	  solde_courant = solde_courant + montant;
	}
      liste_operations_tmp = liste_operations_tmp -> next;
    }

  SOLDE_COURANT = solde_courant;
}
/* ***************************************************************************************************** */


/* ***************************************************************************************************** */
/* Fonction verifie_ligne_selectionnee_visible */
/* appelée lorsqu'on édite une opé et que le formulaire n'était pas visible, pour */
/* éviter que la ligne éditée se retrouve hors du champ de vision */
/* ***************************************************************************************************** */

void verifie_ligne_selectionnee_visible ( void )
{

  selectionne_ligne ( compte_courant );

  gtk_signal_disconnect_by_func ( GTK_OBJECT ( frame_droite_bas ),
				  GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_visible ),
				  NULL );
}
/* ***************************************************************************************************** */
					  

/* ***************************************************************************************************** */
void changement_taille_colonne ( GtkWidget *clist,
				 gint colonne,
				 gint largeur )
{
  taille_largeur_colonnes[colonne] = largeur;
}
/* ***************************************************************************************************** */
