/* fichier qui s'occupe de l'onglet de gestion du tiers */
/*           tiers_onglet.c */

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


/* **************************************************************************************************** */
/* Fonction onglet_tiers : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_tiers ( void )
{
  GtkWidget *onglet;
  GtkWidget *scroll_window;
  gchar *titres[] =
  {
    _("Liste des tiers"),
    _("Montant par tiers"),
    _("Montant par compte"),
    _("Dernière date")
  };
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *vbox_frame;
  GtkWidget *hbox;


/* création des pixmaps pour la liste */

  gnome_stock_pixmap_gdk ( GNOME_STOCK_PIXMAP_BOOK_RED,
			   "",
			   &pixmap_ouvre,
			   &masque_ouvre );

  gnome_stock_pixmap_gdk ( GNOME_STOCK_PIXMAP_BOOK_OPEN,
			   "",
			   &pixmap_ferme,
			   &masque_ferme );



/* création de la fenêtre qui sera renvoyée */

  onglet = gtk_hbox_new ( FALSE,
			  5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


/*   création de la frame de gauche */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			       GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (frame );

  /* mise en place du gtk_text */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   15 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox );
  gtk_widget_show ( vbox );




  frame = gtk_frame_new ( _(" Informations : ") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox_frame = gtk_vbox_new ( FALSE,
			      5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_frame ),
				   5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox_frame );
  gtk_widget_show ( vbox_frame );

  entree_nom_tiers = gtk_entry_new ();
  gtk_widget_set_sensitive ( entree_nom_tiers,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( entree_nom_tiers ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       entree_nom_tiers,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_nom_tiers );


  text_box = gtk_text_new ( NULL,
			    NULL );
  gtk_widget_set_sensitive ( text_box,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( text_box ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       text_box,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( text_box );

/*   création des boutons modifier et annuler */

  hbox = gtk_hbox_new ( TRUE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton_modif_tiers_modifier = gnome_stock_button ( GNOME_STOCK_BUTTON_APPLY );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_tiers_modifier ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_modif_tiers_modifier,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_modif_tiers_modifier ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( clique_sur_modifier_tiers ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_modif_tiers_modifier,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_modif_tiers_modifier );

  bouton_modif_tiers_annuler = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_tiers_annuler ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_modif_tiers_annuler ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( clique_sur_annuler_tiers ),
		       NULL );
  gtk_widget_set_sensitive ( bouton_modif_tiers_annuler,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_modif_tiers_annuler,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_modif_tiers_annuler);

  bouton_supprimer_tiers = gnome_stock_button ( GNOME_STOCK_PIXMAP_REMOVE );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_tiers ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_supprimer_tiers,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_tiers ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( supprimer_tiers ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       bouton_supprimer_tiers,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_supprimer_tiers );

  /* mise en place du bouton ajout d'1 tiers */

  bouton_ajouter_tiers = gtk_button_new_with_label ( _("Ajouter un tiers") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_tiers ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_tiers ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( appui_sur_ajout_tiers ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_ajouter_tiers,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_ajouter_tiers );


/*   création de la frame de droite */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			       GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       TRUE,
		       TRUE,
		       5 );
  gtk_widget_show (frame );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox );
  gtk_widget_show ( vbox );

  /* on y ajoute la barre d'outils */

  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       creation_barre_outils_tiers(),
		       FALSE,
		       FALSE,
		       0 );

/* création de l'arbre principal */

  scroll_window = gtk_scrolled_window_new ( NULL,
				     NULL );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scroll_window ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       scroll_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scroll_window );


  arbre_tiers = gtk_ctree_new_with_titles ( 4,
					    0,
					    titres );
  gtk_ctree_set_line_style ( GTK_CTREE ( arbre_tiers ),
			     GTK_CTREE_LINES_DOTTED );
  gtk_ctree_set_expander_style ( GTK_CTREE ( arbre_tiers ),
				 GTK_CTREE_EXPANDER_CIRCULAR );
  gtk_clist_column_titles_passive ( GTK_CLIST ( arbre_tiers ));
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_tiers ),
				       0,
				       GTK_JUSTIFY_LEFT);
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_tiers ),
				       1,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_tiers ),
				       2,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_tiers ),
				       3,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_tiers ),
				       4,
				       GTK_JUSTIFY_CENTER);

  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_tiers ),
				    0,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_tiers ),
				    1,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_tiers ),
				    2,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_tiers ),
				    3,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_tiers ),
				    4,
				    FALSE );

  /* on met la fontion de tri alphabétique en prenant en compte les accents */

  gtk_clist_set_compare_func ( GTK_CLIST ( arbre_tiers ),
			       (GtkCListCompareFunc) classement_alphabetique_tree );

  gtk_signal_connect ( GTK_OBJECT ( arbre_tiers ),
		       "tree-select-row",
		       GTK_SIGNAL_FUNC ( selection_ligne_tiers ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( arbre_tiers ),
		       "tree-unselect-row",
		       GTK_SIGNAL_FUNC ( enleve_selection_ligne_tiers ),
		       NULL );
  gtk_signal_connect_after ( GTK_OBJECT ( arbre_tiers ),
			     "button-press-event",
			     GTK_SIGNAL_FUNC ( verifie_double_click ),
			     NULL );
  gtk_signal_connect ( GTK_OBJECT ( arbre_tiers ),
		       "size-allocate",
		       GTK_SIGNAL_FUNC ( changement_taille_liste_tiers ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( arbre_tiers ),
		       "tree-expand",
		       GTK_SIGNAL_FUNC ( ouverture_node_tiers ),
		       NULL );

  gtk_container_add ( GTK_CONTAINER (  scroll_window ),
		      arbre_tiers );
  gtk_widget_show ( arbre_tiers );


  /* on met la fontion de tri alphabétique en prenant en compte les accents */

  gtk_clist_set_compare_func ( GTK_CLIST ( arbre_tiers ),
			       (GtkCListCompareFunc) classement_alphabetique_tree );

  /* la 1ère fois qu'on affichera les tiers, il faudra remplir la liste */

  modif_tiers = 1;

  return ( onglet );

}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* Fonction remplit_arbre_tiers */
/* prend en argument le clist arbre_tiers, */
/* le vide et le remplit */
/* **************************************************************************************************** */

void remplit_arbre_tiers ( void )
{
  gchar *text[4];
  GSList *liste_tiers_tmp;
  gfloat *tab_montant;
  gint place_tiers;

  /* freeze le ctree */

  gtk_clist_freeze ( GTK_CLIST ( arbre_tiers ));

  /*   efface l'ancien arbre */

  gtk_clist_clear ( GTK_CLIST ( arbre_tiers ));

  /* récupère les montants des tiers */

  tab_montant = calcule_total_montant_tiers ();

  /* remplit l'arbre */

  liste_tiers_tmp = liste_struct_tiers;
  place_tiers = 0;

  while ( liste_tiers_tmp )
    { 
      struct struct_tiers *tiers;
      GtkCTreeNode *ligne;


      tiers = liste_tiers_tmp -> data;

      if ( fabs ( tab_montant[place_tiers]) >= 0.01)
	text[1] = g_strdup_printf ( "%4.2f %s",
				    tab_montant[place_tiers],
				    devise_compte -> code_devise );
      else
	text[1] = NULL;

      /* nb_ecritures_par_tiers a été calculé dans cacule_total_montant_tiers */

      if ( etat.affiche_nb_ecritures_listes
	   &&
	   nb_ecritures_par_tiers[place_tiers] )
	text[0] = g_strconcat ( tiers -> nom_tiers,
				" (",
				itoa ( nb_ecritures_par_tiers[place_tiers] ),
				")",
				NULL );
      else
	text[0] = tiers -> nom_tiers;

      text[2] = NULL;

      if ( date_dernier_tiers[place_tiers] )
	text[3] = g_strconcat ( itoa ( g_date_day ( date_dernier_tiers[place_tiers] )),
				"/",
				itoa ( g_date_month ( date_dernier_tiers[place_tiers] )),
				"/",
				itoa ( g_date_year ( date_dernier_tiers[place_tiers] )),
				NULL );
      else
	text[3] = NULL;


      ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_tiers ),
				      NULL,
				      NULL,
				      text,
				      10,
				      pixmap_ouvre,
				      masque_ouvre,
				      pixmap_ferme,
				      masque_ferme,
				      FALSE,
				      FALSE );

      /* on associe à ce tiers à l'adr de sa struct */
      
      gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_tiers ),
				    ligne,
				    tiers );


      /* pour chacun des tiers, on met un fils bidon pour pouvoir l'ouvrir */

      ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_tiers ),
				      ligne,
				      NULL,
				      text,
				      5,
				      NULL,
				      NULL,
				      NULL,
				      NULL,
				      FALSE,
				      FALSE );

      /* on associe le fils bidon à -1 */

      gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_tiers ),
				    ligne,
				    GINT_TO_POINTER (-1));



      place_tiers++;
      liste_tiers_tmp = liste_tiers_tmp -> next;
    }

  /* on efface les libère les tableaux */

  free ( tab_montant );
  free ( date_dernier_tiers );
  free ( nb_ecritures_par_tiers );

  /* on trie par ordre alphabétique */
  
  gtk_ctree_sort_node ( GTK_CTREE ( arbre_tiers ),
			NULL );

  /* defreeze le ctree */

  gtk_clist_thaw ( GTK_CLIST ( arbre_tiers ));

  enleve_selection_ligne_tiers ();

  modif_tiers = 0;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
gint classement_alphabetique_tree ( GtkWidget *tree,
				    GtkCListRow *ligne_1,
				    GtkCListRow *ligne_2 )
{
  gchar *string_1;
  gchar *string_2;

  string_1 = g_strdup ( ligne_1->cell->u.text );
  string_1 = g_strdelimit ( string_1,
			    "éÉèÈêÊ",
			    'e' );
  string_1 = g_strdelimit ( string_1,
			    "çÇ",
			    'c' );
  string_1 = g_strdelimit ( string_1,
			    "àÀ",
			    'a' );
  string_1 = g_strdelimit ( string_1,
			    "ùûÙÛ",
			    'u' );
  string_1 = g_strdelimit ( string_1,
			    "ôÔ",
			    'o' );
  string_1 = g_strdelimit ( string_1,
			    "îÎ",
			    'i' );

  string_2 = g_strdup ( ligne_2->cell->u.text );
  string_2 = g_strdelimit ( string_2,
			    "éÉèÈêÊ",
			    'e' );
  string_2 = g_strdelimit ( string_2,
			    "çÇ",
			    'c' );
  string_2 = g_strdelimit ( string_2,
			    "àÀ",
			    'a' );
  string_2 = g_strdelimit ( string_2,
			    "ùûÙÛ",
			    'u' );
  string_2 = g_strdelimit ( string_2,
			    "ôÔ",
			    'o' );
  string_2 = g_strdelimit ( string_2,
			    "îÎ",
			    'i' );

  return ( g_strcasecmp ( string_1,
			  string_2 ));
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* Fonction ouverture_node_tiers */
/* appeléé lorsqu'on ouvre un tiers ou le compte d'un tiers */
/* remplit ce qui doit être affiché */
/* **************************************************************************************************** */

void ouverture_node_tiers ( GtkWidget *arbre,
			    GtkCTreeNode *node,
			    gpointer null )
{			    
  GtkCTreeRow *row;
  gchar *text[4];
  GtkCTreeNode *node_insertion;



  row = GTK_CTREE_ROW ( node );

  /*   si le fiston = -1, c'est qu'il n'a pas encore été créé */
  /* dans le cas contraire, on vire */

  if ( GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
						       row -> children )) != -1 )
    return;

  /* freeze le ctree */

  gtk_clist_freeze ( GTK_CLIST ( arbre_tiers ));

  /* on commence par virer la ligne bidon qui était attachée à ce noeud */

  gtk_ctree_remove_node ( GTK_CTREE ( arbre_tiers ),
			  row -> children );

  /* séparation entre ouverture de tiers ( 0 ) et ouverture de compte ( 1 ) */

  if ( row -> level == 1)
    {
      /* c'est une ouverture de tiers */

      guint no_tiers_selectionne;
      GSList *pointeur_ope;
      gint i;

      no_tiers_selectionne = ((struct struct_tiers *)( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
										     node ))) -> no_tiers;


      /* on va scanner tous les comptes, dès qu'un tiers correspondant au tiers sélectionné est trouvé */
      /* on affiche le nom du compte */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;


      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  pointeur_ope = LISTE_OPERATIONS;

	  while ( pointeur_ope )
	    {
	      struct structure_operation *operation;
	      
	      operation = pointeur_ope -> data;

	      if ( operation -> tiers == no_tiers_selectionne )
		{
		  /* affiche le compte courant */

		  text[2] = calcule_total_montant_tiers_par_compte ( operation -> tiers,
								     operation -> no_compte );

		  if ( etat.affiche_nb_ecritures_listes
		       &&
		       nb_ecritures_par_comptes )
		    text[0] = g_strconcat ( NOM_DU_COMPTE,
					    " (",
					    itoa ( nb_ecritures_par_comptes ),
					    ")",
					    NULL );
		  else
		    text[0] = NOM_DU_COMPTE;

		  text[1] = NULL;
		  text[3] = NULL;

		  node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_tiers ),
							   node,
							   NULL,
							   text,
							   5,
							   NULL,
							   NULL,
							   NULL,
							   NULL,
							   FALSE,
							   FALSE );

		  /* associe le no de compte à la ligne du compte */
		  
		  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_tiers ),
						node_insertion,
						GINT_TO_POINTER ( i ));

		  /* on met une ligne bidon pour pouvoir l'ouvrir */

		  node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_tiers ),
							   node_insertion,
							   NULL,
							   text,
							   5,
							   NULL,
							   NULL,
							   NULL,
							   NULL,
							   FALSE,
							   FALSE );

		  /* associe le no de compte à la ligne du compte */
		  
		  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_tiers ),
						node_insertion,
						GINT_TO_POINTER ( -1 ));

		  pointeur_ope = NULL;
		}
	      else
		pointeur_ope = pointeur_ope -> next;
	    }
	      
	  p_tab_nom_de_compte_variable++;
	}
    }
  else
    {
      /* c'est une ouverture d'un compte */
      /*       cette fois, on fait le tour de toutes les opés du compte pour afficher celles qui correspondent au tiers */

      guint no_tiers_selectionne;
      GSList *pointeur_ope;

      no_tiers_selectionne = ((struct struct_tiers *)( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
										     GTK_CTREE_ROW ( node ) -> parent ))) -> no_tiers;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
													   node ));

      pointeur_ope = LISTE_OPERATIONS;

      while ( pointeur_ope )
	{
	  struct struct_devise *devise_operation;
	  struct structure_operation *operation;

	  operation = pointeur_ope -> data;

	  devise_operation = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( operation -> devise ),
						   ( GCompareFunc ) recherche_devise_par_no ) -> data;

	  if ( ( operation -> tiers == no_tiers_selectionne )
	       &&
	       !operation -> no_operation_ventilee_associee )
	    {
	      if ( operation -> notes )
		text[0] = g_strdup_printf ( "%d/%d/%d : %4.2f %s [ %s ]",
					    operation -> jour,
					    operation -> mois,
					    operation -> annee,
					    operation -> montant,
					    devise_operation -> code_devise,
					    operation -> notes );
	      else
		text[0] = g_strdup_printf ( "%d/%d/%d : %4.2f %s",
					    operation -> jour,
					    operation -> mois,
					    operation -> annee,
					    operation -> montant,
					    devise_operation -> code_devise );

	      text[1] = NULL;
	      text[2] = NULL;
	      text[3] = NULL;

	      node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_tiers ),
						       node,
						       NULL,
						       text,
						       5,
						       NULL,
						       NULL,
						       NULL,
						       NULL,
						       FALSE,
						       FALSE );
 
	      /* on associe à cette opé l'adr de sa struct */

	      gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_tiers ),
					    node_insertion,
					    operation );
	    }

	  pointeur_ope = pointeur_ope -> next;
	}
    }

  /* defreeze le ctree */

  gtk_clist_thaw ( GTK_CLIST ( arbre_tiers ));

}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void selection_ligne_tiers ( GtkCTree *arbre_tiers,
			     GtkCTreeNode *noeud,
			     gint colonne,
			     gpointer null )
{
  struct struct_tiers *tiers;

  gtk_widget_set_sensitive ( entree_nom_tiers,
			     TRUE );
  gtk_widget_set_sensitive ( text_box,
			     TRUE );

  if ( GTK_CTREE_ROW ( noeud ) -> level  == 1 )
    {
      tiers = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
					    noeud );
      gtk_widget_set_sensitive ( bouton_supprimer_tiers,
				 TRUE );
    }
  else
    if ( GTK_CTREE_ROW ( noeud ) -> level  == 2 )
      tiers = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
					    GTK_CTREE_ROW ( noeud ) -> parent );
    else
      tiers = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
					    GTK_CTREE_ROW ( GTK_CTREE_ROW ( noeud ) -> parent ) -> parent );


  gtk_signal_handler_block_by_func ( GTK_OBJECT ( text_box ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_tiers ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );

  if ( tiers -> texte )
    gtk_text_insert ( GTK_TEXT ( text_box ),
		    NULL,
		    NULL,
		    NULL,
		    tiers->texte,
		    -1 );

  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_tiers ),
		       tiers -> nom_tiers );

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_tiers ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( text_box ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );


  if ( tiers -> liaison )
    gtk_text_set_editable ( GTK_TEXT ( text_box ),
			    FALSE );
  else
    gtk_text_set_editable ( GTK_TEXT ( text_box ),
			    TRUE );
 

  gtk_object_set_data ( GTK_OBJECT ( text_box ),
			"adr_struct_tiers",
			tiers );



}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void verifie_double_click ( GtkWidget *liste,
			    GdkEventButton *ev,
			    gpointer null )
{
  if ( ev -> type == GDK_2BUTTON_PRESS )
    {
      struct structure_operation *operation;


      if ( !GTK_CLIST ( arbre_tiers ) -> selection
	   ||
	   GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_tiers ) -> selection ) -> data ) -> level != 3 )
	return;

      /* passage sur le compte concerné */

      operation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
						GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_tiers ) -> selection ) -> data ) );

      changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));


      /* récupération de la ligne de l'opé dans la liste ; affichage de toutes les opé si nécessaire */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

      if ( operation -> pointe == 2 && !AFFICHAGE_R )
	change_aspect_liste ( NULL,
			      2 );

      OPERATION_SELECTIONNEE = operation;

      selectionne_ligne ( compte_courant );
    }

}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void enleve_selection_ligne_tiers ( void )
{
  gtk_widget_set_sensitive ( bouton_supprimer_tiers,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_tiers_modifier,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_tiers_annuler,
			     FALSE );
  gtk_widget_set_sensitive ( entree_nom_tiers,
			     FALSE );
  gtk_widget_set_sensitive ( text_box,
			     FALSE );


  gtk_signal_handler_block_by_func ( GTK_OBJECT ( text_box ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_tiers ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );

  gtk_editable_delete_text ( GTK_EDITABLE ( entree_nom_tiers ),
			     0,
			     -1 );
  gtk_editable_delete_text ( GTK_EDITABLE ( text_box ),
			     0,
			     -1 );

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_tiers ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( text_box ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_tiers),
				     NULL );

  gtk_object_remove_data ( GTK_OBJECT (  text_box ),
			   "adr_struct_tiers" );

  gtk_editable_set_editable ( GTK_EDITABLE ( text_box ),
			      FALSE );
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
void modification_du_texte_tiers ( GtkText *texte,
				   gpointer null )
{
  gtk_widget_set_sensitive ( bouton_modif_tiers_modifier,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_modif_tiers_annuler,
			     TRUE );
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void clique_sur_modifier_tiers ( GtkWidget *bouton_modifier,
				 gpointer null )
{
  struct struct_tiers *tiers;
  GtkCTreeNode *node;


  tiers =  gtk_object_get_data ( GTK_OBJECT (  text_box ),
				 "adr_struct_tiers" );

/* si c'est une modif du nom, on doit réafficher la liste des tiers et les listes des opés, sinon, on change juste le texte */

  if ( strcmp ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_nom_tiers ))),
		tiers -> nom_tiers ))
    {
      free ( tiers -> nom_tiers );

      tiers -> nom_tiers = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_nom_tiers ))) );


      node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_tiers ) -> selection ) -> data );

      if ( GTK_CTREE_ROW ( node ) -> level != 1 )
	{
	  node = GTK_CTREE_ROW ( node ) -> parent;

	  if ( GTK_CTREE_ROW ( node ) -> level == 2 )
	    node = GTK_CTREE_ROW ( node ) -> parent;
	}


	  if ( GTK_CTREE_ROW ( node ) -> expanded )
	    gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_tiers ),
					 node,
					 0,
					 tiers -> nom_tiers,
					 10,
					 pixmap_ferme,
					 masque_ferme );
	  else
	    gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_tiers ),
					 node,
					 0,
					 tiers -> nom_tiers,
					 10,
					 pixmap_ouvre,
					 masque_ouvre );

      demande_mise_a_jour_tous_comptes ();
      remplissage_liste_echeance();
      update_liste_echeances_manuelles_accueil ();
      update_liste_echeances_auto_accueil ();
      mise_a_jour_tiers ();

    }
  else
    {
      free ( tiers -> texte );

      tiers -> texte = g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( text_box ),
							    0,
							    gtk_text_get_length (GTK_TEXT (text_box))) );
    }

  gtk_widget_set_sensitive ( bouton_modif_tiers_modifier,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_tiers_annuler,
			     FALSE );

  modification_fichier(TRUE);
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void clique_sur_annuler_tiers ( GtkWidget *bouton_annuler,
				 gpointer null )
{
  struct struct_tiers *tiers;


  tiers =  gtk_object_get_data ( GTK_OBJECT (  text_box ),
				 "adr_struct_tiers" );

  gtk_editable_delete_text ( GTK_EDITABLE ( text_box ),
			     0,
			     -1 );
 
  if ( tiers -> texte )
    gtk_text_insert ( GTK_TEXT ( text_box ),
		    NULL,
		    NULL,
		    NULL,
		    tiers->texte,
		    -1 );


  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_tiers ),
		       tiers -> nom_tiers );

  if ( tiers -> liaison )
    gtk_text_set_editable ( GTK_TEXT ( text_box ),
			    FALSE );
  else
    gtk_text_set_editable ( GTK_TEXT ( text_box ),
			    TRUE );

  gtk_widget_set_sensitive ( bouton_modif_tiers_modifier,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_tiers_annuler,
			     FALSE );

}
/* **************************************************************************************************** */





/***********************************************************************************************************/
/* Fonction ajoute_nouveau_tiers */
/* appelée pour ajouter un nouveau tiers à la liste des tiers */
/* entrée : le nouveau tiers */
/* retour : le no de tiers */
/***********************************************************************************************************/

struct struct_tiers *ajoute_nouveau_tiers ( gchar *tiers )
{
  struct struct_tiers *nouveau_tiers;

  if ( !strlen ( g_strstrip ( tiers )))
    return ( 0 );

  nouveau_tiers = calloc ( 1,
			   sizeof ( struct struct_tiers ));

  nouveau_tiers -> no_tiers = ++no_dernier_tiers;
  nouveau_tiers -> nom_tiers = g_strdup ( g_strstrip ( tiers ));

  liste_struct_tiers = g_slist_append ( liste_struct_tiers,
					nouveau_tiers );
  nb_enregistrements_tiers++;

  return ( nouveau_tiers );
}
/***********************************************************************************************************/





/* **************************************************************************************************** */
void supprimer_tiers ( GtkWidget *bouton,
		       gpointer null )
{
  struct struct_tiers *tiers;
  GtkCTreeNode *node;
  GSList *pointeur_ope;
  gint i;
  gint ope_trouvee;

  node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_tiers ) -> selection ) -> data );

  tiers = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_tiers ),
					node );

  /* on fait le tour de tous les comptes pour vérifier s'il reste une opé à ce tiers */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  ope_trouvee = 0;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {
      pointeur_ope = LISTE_OPERATIONS;

      while ( pointeur_ope )
	{
	  struct structure_operation *operation;
	      
	  operation = pointeur_ope -> data;

	  if ( operation -> tiers == tiers -> no_tiers )
	    {
	      /* une opération associée à ce tiers a été trouvée  = > on sort de la recherche */

	      pointeur_ope = NULL;
	      i = nb_comptes;
	      ope_trouvee = 1;
	    }
	  else
	    pointeur_ope = pointeur_ope -> next;
	}
      p_tab_nom_de_compte_variable++;
    }


  if ( ope_trouvee )
    {
      GtkWidget *dialog;
      GtkWidget *label;
      gint resultat;
      GtkWidget *separation;
      GtkWidget *hbox;
      GtkWidget *bouton_tiers_generique;
      GtkWidget *combofix;
      GSList *liste_combofix;
      GSList *pointeur;
      GtkWidget *bouton_transfert;
      gint i;
      gint nouveau_no;
      struct struct_tiers *nouveau_tiers;

      dialog = gnome_dialog_new ( _("Suppression d'un tiers"),
				  GNOME_STOCK_BUTTON_OK,
				  GNOME_STOCK_BUTTON_CANCEL,
				  NULL);
      gnome_dialog_set_parent ( GNOME_DIALOG ( dialog ),
				GTK_WINDOW ( window ));

      label = gtk_label_new ( _("Le tiers sélectionné contient encore des opérations.\n\nVous pouvez : ") );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );

      separation = gtk_hseparator_new ( );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   separation,
			   FALSE,
			   FALSE,
			   0 );

      /*       mise en place du choix tranfert vers un autre tiers */
      
      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );

      bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							   _("Transférer les opérations sur le tiers ")  );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   bouton_transfert,
			   FALSE,
			   FALSE,
			   0 );


      pointeur = liste_struct_tiers;
      liste_combofix = NULL;

      while ( pointeur )
	{
	  if ( ((struct struct_tiers * )( pointeur -> data )) -> no_tiers != tiers -> no_tiers )
	    liste_combofix = g_slist_append ( liste_combofix,
					      ((struct struct_tiers * )( pointeur -> data )) -> nom_tiers );
	  pointeur = pointeur -> next;
	}


      combofix = gtk_combofix_new ( liste_combofix,
				    TRUE,
				    TRUE,
				    TRUE,
				    0 );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   combofix,
			   FALSE,
			   FALSE,
			   0 );



      /*       mise en place du choix supprimer le tiers */

      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );

      bouton_tiers_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
								 _(" Supprimer simplement ce tiers.") );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   bouton_tiers_generique,
			   FALSE,
			   FALSE,
			   0 );

      gtk_widget_show_all ( dialog );


    retour_dialogue:
      resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ) );

      if ( resultat )
	{
	  if ( resultat == 1 )
	    gnome_dialog_close ( GNOME_DIALOG ( dialog ) );
	  return;
	}


      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_transfert )) )
	{

	  if ( !strlen (gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ))))
	    {
	      dialogue ( _("Veuillez entrer un tiers.") );
	      goto retour_dialogue;
	    }

	  /* recherche du nouveau numéro de tiers */

	  nouveau_tiers = g_slist_find_custom ( liste_struct_tiers,
						gtk_combofix_get_text ( GTK_COMBOFIX ( combofix ) ),
						(GCompareFunc) recherche_tiers_par_nom ) -> data;
	  nouveau_no = nouveau_tiers -> no_tiers;
	}
      else
	nouveau_no = 0;


      /* on fait le tour des opés pour mettre le nouveau numéro de tiers */


      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  pointeur_ope = LISTE_OPERATIONS;

	  while ( pointeur_ope )
	    {
	      struct structure_operation *operation;
	      
	      operation = pointeur_ope -> data;

	      if ( operation -> tiers == tiers -> no_tiers )
		operation -> tiers = nouveau_no;

	      pointeur_ope = pointeur_ope -> next;
	    }
	  p_tab_nom_de_compte_variable++;
	}


      /* fait le tour des échéances pour mettre le nouveau numéro de tiers */

      pointeur_ope = gsliste_echeances;

      while ( pointeur_ope )
	{
	  struct operation_echeance *echeance;

	  echeance = pointeur_ope -> data;

	  if ( echeance -> tiers == tiers -> no_tiers )
	    echeance -> tiers = nouveau_no;

	  pointeur_ope = pointeur_ope -> next;
	}


      demande_mise_a_jour_tous_comptes ();
      remplissage_liste_echeance();

      gnome_dialog_close ( GNOME_DIALOG ( dialog ) );

    }


  /* supprime dans la liste des tiers  */

  liste_struct_tiers = g_slist_remove ( liste_struct_tiers,
					tiers );
  nb_enregistrements_tiers--;


  mise_a_jour_tiers ();
  remplit_arbre_tiers ();

  gtk_text_set_editable ( GTK_TEXT ( text_box ),
			  FALSE );

  modification_fichier(TRUE);
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
void creation_liste_tiers_combofix ( void )
{
  GSList *pointeur;
  GSList *liste_tmp;

  /* on commence à créer les 2 listes semblables de tous les tiers */
  /*   celle du formulaire est de type complex, cad qu'elle contiendra 2 listes : */
  /* les tiers et les états sélectionnés */

  pointeur = liste_struct_tiers;
  liste_tiers_combofix = NULL;
  liste_tiers_combofix_echeancier = NULL;
  liste_tmp = NULL;

  while ( pointeur )
    {
      liste_tmp = g_slist_append ( liste_tmp,
				   ((struct struct_tiers * )( pointeur -> data )) -> nom_tiers );
      liste_tiers_combofix_echeancier = g_slist_append ( liste_tiers_combofix_echeancier,
							 ((struct struct_tiers * )( pointeur -> data )) -> nom_tiers );
      pointeur = pointeur -> next;
    }

  /* on ajoute liste tmp à liste_tiers_combofix */

  liste_tiers_combofix = g_slist_append ( liste_tiers_combofix,
					  liste_tmp );

  /* on fait maintenant le tour des états pour rajouter ceux qui ont été sélectionnés */

  liste_tmp = NULL;
  pointeur = liste_struct_etats;

  while ( pointeur )
    {
      struct struct_etat *etat;

      etat = pointeur -> data;

      if ( etat -> inclure_dans_tiers )
	{
	  if ( liste_tmp )
	    liste_tmp = g_slist_append ( liste_tmp,
					 g_strconcat ( "\t",
						       g_strdup ( etat -> nom_etat ),
						       NULL ));
	  else
	    {
	      liste_tmp = g_slist_append ( liste_tmp,
					   _( "État" ));
	      liste_tmp = g_slist_append ( liste_tmp,
					   g_strconcat ( "\t",
							 g_strdup ( etat -> nom_etat ),
							 NULL ));
	    }
	}
      pointeur = pointeur -> next;
    }

  /* on ajoute liste tmp à liste_tiers_combofix */

  liste_tiers_combofix = g_slist_append ( liste_tiers_combofix,
					  liste_tmp );
}
/* **************************************************************************************************** */





/* ***************************************************************************************************** */
/* Fonction changement_taille_liste_tiers */
/* appelée dès que la taille de la clist a changé ( fait aussi les catég et l'ib ) */
/* pour mettre la taille des différentes colonnes */
/* ***************************************************************************************************** */

void changement_taille_liste_tiers ( GtkWidget *clist,
				     GtkAllocation *allocation,
				     gpointer null )
{
  gint tiers, date;
  gint largeur;

  largeur = allocation->width;

  tiers = ( 43 * largeur) / 100;
  date = ( 17 * largeur) / 100;


  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       0,
			       tiers );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       1,
			       date );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       2,
			       date );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       3,
			       date );

}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction mise_a_jour_tiers */
/* recrée la liste des combofix et l'applique à tous les combofix du tiers */
/* ***************************************************************************************************** */

void mise_a_jour_tiers ( void )
{
  creation_liste_tiers_combofix ();

  gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_operations[2] ),
			  liste_tiers_combofix,
			  TRUE,
			  TRUE );
  gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[1] ),
			  liste_tiers_combofix_echeancier,
			  FALSE,
			  TRUE );

  if ( etat_courant )
    {
      remplissage_liste_tiers_etats ();
      selectionne_liste_tiers_etat_courant ();
    }

  modif_tiers = 1;
}
/* ***************************************************************************************************** */


/***************************************************************************************************/
/* Fonction recherche tiers par no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/
  
gint recherche_tiers_par_no ( struct struct_tiers *tiers,
			      gint *no_tiers )
{
  return ( tiers -> no_tiers != GPOINTER_TO_INT ( no_tiers ) );
}
/***************************************************************************************************/



/* **************************************************************************************************** */
/* Fonction recherche_tiers_par_nom */
/* appelée par un g_slist_find_custom */
/* prend en arg la struct du tiers et le nom du tiers recherché */
/* **************************************************************************************************** */

gint recherche_tiers_par_nom ( struct struct_tiers *tiers,
			       gchar *ancien_tiers )
{

  return ( g_strcasecmp ( tiers -> nom_tiers,
			  ancien_tiers ) );
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* crée un tableau de gint aussi gd que le nb de tiers */
/* et le renvoie */
/* **************************************************************************************************** */

gfloat *calcule_total_montant_tiers ( void )
{
  gint i;
  gfloat *tab_retour;

  /* on crée le tableau de retour */

  tab_retour = calloc ( nb_enregistrements_tiers,
			sizeof ( gfloat ));
  date_dernier_tiers = calloc ( nb_enregistrements_tiers,
				sizeof ( gpointer ));
  nb_ecritures_par_tiers = calloc ( nb_enregistrements_tiers,
				    sizeof ( gint ));

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GSList *liste_tmp;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

      liste_tmp = LISTE_OPERATIONS;

      while ( liste_tmp )
	{
	  struct structure_operation *operation;
	  gdouble montant;
	  gint place_tiers;

	  operation = liste_tmp -> data;

	  if ( operation -> tiers )
	    {
	      /* recherche la place du tiers dans la liste */

	      place_tiers = g_slist_position ( liste_struct_tiers,
					       g_slist_find_custom ( liste_struct_tiers,
								     GINT_TO_POINTER ( operation -> tiers ),
								     (GCompareFunc) recherche_tiers_par_no ));

	      montant = calcule_montant_devise_renvoi ( operation -> montant,
							no_devise_totaux_tiers,
							operation -> devise,
							operation -> une_devise_compte_egale_x_devise_ope,
							operation -> taux_change,
							operation -> frais_change );

	      tab_retour[place_tiers] = tab_retour[place_tiers] + montant;

	      if ( !date_dernier_tiers[place_tiers]
		   ||
		   g_date_compare ( operation->date,
				    date_dernier_tiers[place_tiers] ) > 0 )
		date_dernier_tiers[place_tiers] = operation -> date;

	      nb_ecritures_par_tiers[place_tiers]++;
	    }
	  liste_tmp = liste_tmp -> next;
	}
    }

    return ( tab_retour );
}
/* **************************************************************************************************** */






/* **************************************************************************************************** */
gchar *calcule_total_montant_tiers_par_compte ( gint no_tiers,
					       gint no_compte )
{
  gdouble retour_int;
  GSList *liste_tmp;

  retour_int = 0;
  nb_ecritures_par_comptes = 0;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

  liste_tmp = LISTE_OPERATIONS;

  while ( liste_tmp )
    {
      struct structure_operation *operation;

      operation = liste_tmp -> data;

      if ( operation -> tiers == no_tiers )
	{
	  gdouble montant;

	  montant = calcule_montant_devise_renvoi ( operation -> montant,
						    no_devise_totaux_tiers,
						    operation -> devise,
						    operation -> une_devise_compte_egale_x_devise_ope,
						    operation -> taux_change,
						    operation -> frais_change );

	  retour_int = retour_int + montant;
	  nb_ecritures_par_comptes++;
	}
      liste_tmp = liste_tmp -> next;
    }

  if ( retour_int )
    return ( g_strdup_printf ( "%4.2f %s",
			       retour_int,
			       devise_compte -> code_devise ));
  else
    return ( NULL );
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void appui_sur_ajout_tiers ( void )
{
  gchar *nom_tiers;
  struct struct_tiers *nouveau_tiers;
  gchar *text[4];
  GtkCTreeNode *ligne;

  if ( !( nom_tiers = demande_texte ( _("Nouveau tiers"),
				      _("Entrer le nom du nouveau tiers :") )))
    return;

  /* on l'ajoute à la liste des opés */
  
  nouveau_tiers = ajoute_nouveau_tiers ( nom_tiers );


  /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

  text[0] = nouveau_tiers -> nom_tiers;
  text[1] = NULL;
  text[2] = NULL;
  text[3] = NULL;

  ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_tiers ),
				  NULL,
				  NULL,
				  text,
				  10,
				  pixmap_ouvre,
				  masque_ouvre,
				  pixmap_ferme,
				  masque_ferme,
				  FALSE,
				  FALSE );

  /* on associe à ce tiers à l'adr de sa struct */

  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_tiers ),
				ligne,
				nouveau_tiers );

  gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_tiers ),
			     NULL );

  mise_a_jour_tiers();
  modif_tiers = 0;
  modification_fichier(TRUE);
}
/* **************************************************************************************************** */

