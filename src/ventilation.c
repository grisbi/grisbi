/* Fichier banque.c */
/* s'occupe de tout ce qui concerne les banques */


/*     Copyright (C) 2000-2001  Cédric Auger */
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
/* Fonction creation_fenetre_ventilation */
/* crée la fenetre qui contient la liste des ventilation */
/*******************************************************************************************/

GtkWidget *creation_fenetre_ventilation ( void )
{
  GtkWidget *onglet;
  gchar *titres[] ={ _("Catégorie"),
		     _("Notes"),
		     _("Montant") };

  /* création de la scrolled window  */

  onglet = gtk_scrolled_window_new ( NULL,
				     NULL);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( onglet ),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC);
  gtk_widget_show ( onglet );


  /* création de la liste */

  liste_operations_ventilees = gtk_clist_new_with_titles ( 3,
				       titres );
  gtk_widget_set_usize ( GTK_WIDGET ( liste_operations_ventilees ),
			 1,
			 FALSE );

  /*   par défaut, le classement de la liste s'effectue par no des opérations */

  gtk_clist_set_compare_func ( GTK_CLIST ( liste_operations_ventilees ),
			       (GtkCListCompareFunc) classement_liste_par_no_ope );

  gtk_signal_connect ( GTK_OBJECT ( liste_operations_ventilees ),
		       "size-allocate",
		       GTK_SIGNAL_FUNC ( changement_taille_liste_ventilation ),
		       NULL );
  gtk_container_add ( GTK_CONTAINER ( onglet ),
		      liste_operations_ventilees );
  gtk_widget_show ( liste_operations_ventilees );
  

  /* On annule la fonction bouton des titres */

  gtk_clist_column_titles_passive ( GTK_CLIST ( liste_operations_ventilees ));

  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_operations_ventilees ),
				    0,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_operations_ventilees ),
				    1,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_operations_ventilees ),
				    2,
				    FALSE );


  /* justification du contenu des cellules */


  gtk_clist_set_column_justification ( GTK_CLIST ( liste_operations_ventilees ),
				       0,
				       GTK_JUSTIFY_LEFT);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste_operations_ventilees ),
				       1,
				       GTK_JUSTIFY_LEFT);
  gtk_clist_set_column_justification ( GTK_CLIST ( liste_operations_ventilees ),
				       2,
				       GTK_JUSTIFY_RIGHT);


  /* vérifie le simple ou double click */

  gtk_signal_connect ( GTK_OBJECT ( liste_operations_ventilees ),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( selectionne_ligne_souris_ventilation ),
		       NULL );


  /*   vérifie la touche entrée, haut et bas */

  gtk_signal_connect ( GTK_OBJECT ( liste_operations_ventilees ),
		       "key_press_event",
		       GTK_SIGNAL_FUNC ( traitement_clavier_liste_ventilation ),
		       NULL );



  return ( onglet );
}
/*******************************************************************************************/



/*******************************************************************************************/
/* Fonction  creation_verification_ventilation*/
/* crée la fenetre à la place de la liste des comptes qui contient les boutons et l'état de la ventilation */
/*******************************************************************************************/

GtkWidget *creation_verification_ventilation ( void )
{
  GtkWidget *onglet;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *tableau;
  GtkWidget *separateur;
  GtkWidget *hbox;
  GtkWidget *bouton;


  /* création de la vbox */

  onglet = gtk_vbox_new ( FALSE,
			  10 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


/* création du titre "opération ventilée" */

  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  label = gtk_label_new ( _("Opération ventilée") );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      label );
  gtk_widget_show ( label );


  /* création du tableau */

  tableau = gtk_table_new ( 4,
			    2,
			    FALSE );
  gtk_table_set_row_spacings ( GTK_TABLE ( tableau ),
			       10 );
  gtk_table_set_col_spacings ( GTK_TABLE ( tableau ),
			       10 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       tableau,
		       FALSE,
		       FALSE,
		       20 );
  gtk_widget_show ( tableau );


  label = gtk_label_new ( _("Somme ventilée :") );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( tableau ),
		     label,
		     0, 1,
		     0, 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  label_somme_ventilee = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label_somme_ventilee ),
			   1,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( tableau ),
		     label_somme_ventilee,
		     1, 2,
		     0, 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label_somme_ventilee );


  label = gtk_label_new ( _("Non affecté :") );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( tableau ),
		     label,
		     0, 1,
		     1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  label_non_affecte = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label_non_affecte ),
			   1,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( tableau ),
		     label_non_affecte,
		     1, 2,
		     1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label_non_affecte );


  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( tableau ),
		     separateur,
		     0, 2,
		     2, 3,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );



  label = gtk_label_new ( _("Montant :") );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( tableau ),
		     label,
		     0, 1,
		     3, 4,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  label_montant_operation_ventilee = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label_montant_operation_ventilee ),
			   1,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( tableau ),
		     label_montant_operation_ventilee,
		     1, 2,
		     3, 4,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label_montant_operation_ventilee );



  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( separateur );

/* mise en place des boutons */

  hbox = gtk_hbox_new ( FALSE,
			10 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( hbox );


  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_CLOSE );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( valider_ventilation ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_show (bouton  );

  return ( onglet );
}
/*******************************************************************************************/


/*******************************************************************************************/
/* Fonction creation_formulaire_ventilation */
/* crée la fenetre qui contient e formulaire pour la ventilation */
/*******************************************************************************************/

GtkWidget *creation_formulaire_ventilation ( void )
{
  GtkWidget *onglet;
  GtkTooltips *tips;
  GtkWidget *table;
  GtkWidget *menu;
  GtkWidget *bouton;

  /* on crée le tooltips */

  tips = gtk_tooltips_new ();

  /* création du formulaire */

  onglet = gtk_vbox_new ( FALSE,
			  5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10);
  gtk_widget_show ( onglet );

  /* mise en place de la table */

  table = gtk_table_new ( 2,
			  4,
			  FALSE);
  gtk_table_set_col_spacings ( GTK_TABLE ( table ),
			       10 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       table,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( table );

  /* mise en place des catégories */

  widget_formulaire_ventilation[0] = gtk_combofix_new_complex ( liste_categories_echeances_combofix,
							    FALSE,
							    TRUE,
							    TRUE );
  gtk_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ) -> entry ),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[0]) -> entry),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[0]) -> arrow),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
		       NULL );
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX (widget_formulaire_ventilation[0]) -> entry ),
			      "focus_in_event",
			      GTK_SIGNAL_FUNC (entree_prend_focus),
			      GTK_OBJECT ( widget_formulaire_ventilation[0] ) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[0]) -> entry),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
		       GINT_TO_POINTER (0) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_ventilation[0],
		     0, 1, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show ( widget_formulaire_ventilation[0] );

  /* mise en place des notes */

  widget_formulaire_ventilation[1] = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation[1] ),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
		       GINT_TO_POINTER ( 1 ) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[1]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[1]),
		       "focus_in_event",
		       GTK_SIGNAL_FUNC (entree_prend_focus),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[1]),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
		       GINT_TO_POINTER (1) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_ventilation[1],
		     1, 3, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show ( widget_formulaire_ventilation[1] );



  /* mise en place du montant */

  widget_formulaire_ventilation[2] = gtk_entry_new ();
  gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_ventilation[2] ),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
		       GINT_TO_POINTER ( 2 ) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[2]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[2]),
		       "focus_in_event",
		       GTK_SIGNAL_FUNC (entree_prend_focus),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[2]),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
		       GINT_TO_POINTER (2) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_ventilation[2],
		     3, 4, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show ( widget_formulaire_ventilation[2] );


  /*  Affiche l'imputation budgétaire */

  widget_formulaire_ventilation[3] = gtk_combofix_new_complex ( liste_imputations_combofix,
								FALSE,
								TRUE,
								TRUE );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_ventilation[3],
		     0, 1, 1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[3]) -> entry),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[3]) -> arrow),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[3]) -> entry),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (appui_touche_ventilation),
		       GINT_TO_POINTER(3) );
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX (widget_formulaire_ventilation[3]) -> entry ),
			      "focus_in_event",
			      GTK_SIGNAL_FUNC (entree_prend_focus),
			      GTK_OBJECT ( widget_formulaire_ventilation[3] ) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_ventilation[3]) -> entry),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
		       GINT_TO_POINTER (3) );

  if ( etat.utilise_imputation_budgetaire )
    gtk_widget_show (widget_formulaire_ventilation[3]);



  /* mise en place du no d'exercice */

  widget_formulaire_ventilation[4] = gtk_option_menu_new ();
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			 widget_formulaire_ventilation[4],
			 _("Choix de l'exercice"),
			 _("Choix de l'exercice") );
  menu = gtk_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_ventilation[4] ),
			     creation_menu_exercices () );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[4]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC ( appui_touche_ventilation ),
		       GINT_TO_POINTER(4) );
  gtk_table_attach ( GTK_TABLE ( table ),
		     widget_formulaire_ventilation[4],
		     1, 2, 1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  if ( etat.utilise_exercice )
    gtk_widget_show ( widget_formulaire_ventilation[4] );


  /*   création de l'entrée du no de pièce comptable */

  widget_formulaire_ventilation[5] = gtk_entry_new();
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_ventilation[5],
		     2, 4, 1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[5]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire_ventilation ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[5]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (appui_touche_ventilation),
		       GINT_TO_POINTER(5) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[5]),
		       "focus_in_event",
		       GTK_SIGNAL_FUNC (entree_prend_focus),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_ventilation[5]),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_ventilation_perd_focus),
		       GINT_TO_POINTER (5) );
  if ( etat.utilise_piece_comptable )
    gtk_widget_show ( widget_formulaire_ventilation[5] );


  /* séparation d'avec les boutons */

  separateur_formulaire_ventilations = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       separateur_formulaire_ventilations,
		       FALSE,
		       FALSE,
		       0 );
  if ( etat.affiche_boutons_valider_annuler )
    gtk_widget_show ( separateur_formulaire_ventilations );

  /* mise en place des boutons */

  hbox_valider_annuler_ventil = gtk_hbox_new ( FALSE,
					       5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox_valider_annuler_ventil,
		       FALSE,
		       FALSE,
		       0 );
  if ( etat.affiche_boutons_valider_annuler )
    gtk_widget_show ( hbox_valider_annuler_ventil );

  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( echap_formulaire_ventilation ),
		       NULL );
  gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ventil ),
		     bouton,
		     FALSE,
		     FALSE,
		     0 );
  gtk_widget_show ( bouton );

  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_OK );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( fin_edition_ventilation ),
		       NULL );
  gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ventil ),
		     bouton,
		     FALSE,
		     FALSE,
		     0 );
  gtk_widget_show ( bouton );


  /*   met l'adr de l'opé dans le formulaire à -1 */

  gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[0] ),
			"adr_struct_ope",
			GINT_TO_POINTER ( -1 ) );

  return ( onglet );
}
/*******************************************************************************************/





/***********************************************************************************************************/
void clique_champ_formulaire_ventilation ( void )
{

  /* on rend sensitif tout ce qui ne l'était pas sur le formulaire */

  gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[4] ),
			     TRUE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ventil ),
			     TRUE );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction appelée quand une entry perd le focus */
/* si elle ne contient rien, on remet la fonction en gris */
/***********************************************************************************************************/

void entree_ventilation_perd_focus ( GtkWidget *entree,
				     GdkEventFocus *ev,
				     gint *no_origine )
{
  gchar *texte;

  texte = NULL;

  switch ( GPOINTER_TO_INT ( no_origine ))
    {
      /* on sort des catégories */
    case 0:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Catégories : Sous-catégories");
      break;

      /* sort des notes */

    case 1:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Notes");
      break;

      /* sort du montant */

    case 2:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Montant");
      break;

      /* sort de l'ib */

    case 3:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Imputation budgétaire");
      break;

      /* sort de la pièce comptable */

    case 5:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Pièce comptable");
      break;


    default :
    }


  /* l'entrée était vide, on remet le défaut */

  if ( texte )
    {
      gtk_widget_set_style ( entree,
			     style_entree_formulaire[1] );
      gtk_entry_set_text ( GTK_ENTRY ( entree ),
			   texte );
    }
}
/*******************************************************************************************/




/*******************************************************************************************/
/* Fonction ventiler_operation */
/* appelée lorsque la catégorie est Ventilation lors de l'enregistrement d'une opé */
/* ou lors d'une modif d'une opé ventilée */
/* Arguments : montant : montant total de l'opé */
/*             signe : 0 : débit, 1 : crédit */
/*******************************************************************************************/

void ventiler_operation ( gdouble montant )
{
  /*   si liste_ope est NULL, c'est une nouvelle opé, les ventils ne peuvent être associées */
  /* à un no d'opé, on les met donc à -1 */
  /* si c'est != NULL, c'est que c'était déjà une ventilation, et on a appuyé sur voir pour */
  /* arriver ici */

  /* on commence par mettre la taille au formulaire et à la liste */

  ancienne_largeur_ventilation = 0;
  changement_taille_liste_ventilation ( liste_operations_ventilees,
					&liste_operations_ventilees->allocation,
					NULL );

  montant_operation_ventilee = montant;

  ligne_selectionnee_ventilation = NULL;

  /* affiche la liste */

  affiche_liste_ventilation ();

  ligne_selectionnee_ventilation = GINT_TO_POINTER ( -1 );

  gtk_clist_select_row ( GTK_CLIST ( liste_operations_ventilees ),
			 gtk_clist_find_row_from_data ( GTK_CLIST ( liste_operations_ventilees ),
							ligne_selectionnee_ventilation ),
			 0 );

  /* met à jour les labels */

  gtk_label_set_text ( GTK_LABEL ( label_somme_ventilee ),
		       g_strdup_printf ( "%4.2f",
					 somme_ventilee ) );


/*   s'il n'y a pas de montant total, celui ci = la somme ventilée */

  if ( montant_operation_ventilee )
    {
      gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			   g_strdup_printf ( "%4.2f",
					     montant_operation_ventilee - somme_ventilee ));
      gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			   g_strdup_printf ( "%4.2f",
					     montant_operation_ventilee ));
    }
  else
    {
      gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			   "" );
      gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			   g_strdup_printf ( "%4.2f",
					     somme_ventilee ));
    }


  gtk_widget_grab_focus ( liste_operations_ventilees );


  /* bloque les fonctions */
  /* qui modifient la position dans la liste des opés */

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( frame_droite_bas ),
			     allocation_taille_formulaire,
			     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( frame_droite_bas ),
			     efface_formulaire,
			     NULL );


  /* affiche les pages de ventilation */

  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			  1 );
  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			  1 );
  gtk_widget_hide ( formulaire );
  gtk_widget_hide ( frame_droite_bas );
  gtk_widget_show ( frame_droite_bas );
  gtk_widget_hide ( barre_outils );
  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire ),
			  1 );

  /* on grise tout ce qu'il faut */

  echap_formulaire_ventilation ();

}
/*******************************************************************************************/





/* ***************************************************************************************************** */
/* Fonction changement_taille_liste_ventilation */
/* appelée dès que la taille de la clist a changé */
/* pour mettre la taille des différentes colonnes */
/* ***************************************************************************************************** */

void changement_taille_liste_ventilation ( GtkWidget *clist,
					   GtkAllocation *allocation,
					   gpointer null )
{
  gint categorie, description, montant;
  gint largeur;
  gint col0, col1, col2;

  /*   si la largeur de grisbi est < 700, on fait rien */

  if ( window -> allocation.width < 700 )
    return;

  largeur = allocation->width;

  if ( largeur == ancienne_largeur_ventilation )
    return;

  ancienne_largeur_ventilation = largeur;

  categorie = ( 40 * largeur) / 100;
  description = ( 35 * largeur) / 100;
  montant = ( 10 * largeur) / 100;

  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       0,
			       categorie );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       1,
			       description );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       2,
			       montant );

/* met les entrées du formulaire à la même taille */

  col0 = largeur * 40  / 100;
  col1 = largeur * 40  / 100;
  col2 = largeur * 15  / 100;

  /* 1ère ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[0] ),
			 col0,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[1] ),
			 col1,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[2] ),
			 col2,
			 FALSE );

  /* 2ème ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[3] ),
			 col0,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[4] ),
			 col1 / 2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_ventilation[5] ),
			 col2,
			 FALSE );


}
/* ***************************************************************************************************** */



/***************************************************************************************************/
/* Fonction traitement_clavier_liste */
/* gère le clavier sur la clist */
/***************************************************************************************************/

gboolean traitement_clavier_liste_ventilation ( GtkCList *liste,
						GdkEventKey *evenement,
						gpointer null )
{
  gint ligne;


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste ),
				 "key_press_event");

  switch ( evenement->keyval )
    {
     /* entrée */

    case 65293 :
    case 65421 :

      edition_operation_ventilation ();
      break;


     /* flèche haut  */
    case 65362 :

      ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
					     ligne_selectionnee_ventilation );
      if ( ligne )
	{
	  ligne--;

	  ligne_selectionnee_ventilation = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
								    ligne );
	  gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
	  gtk_clist_select_row ( GTK_CLIST ( liste ),
				 ligne,
				 0 );

	  if ( gtk_clist_row_is_visible ( GTK_CLIST ( liste ),
					  ligne) != GTK_VISIBILITY_FULL )
	    gtk_clist_moveto ( GTK_CLIST ( liste ),
			       ligne,
			       0,
			       0,
			       0 );
	}
      break;


      /* flèche bas */
    case 65364 :

      if ( ligne_selectionnee_ventilation != GINT_TO_POINTER ( -1 ) )
	{
	  ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste ),
						 ligne_selectionnee_ventilation );

	  ligne++;

	  ligne_selectionnee_ventilation = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
							    ligne );
	  gtk_clist_unselect_all ( GTK_CLIST ( liste ) );
	  gtk_clist_select_row ( GTK_CLIST ( liste ),
				 ligne,
				 0 );


	  if ( gtk_clist_row_is_visible ( GTK_CLIST ( liste ),
					  ligne )
	       != GTK_VISIBILITY_FULL )
	    gtk_clist_moveto ( GTK_CLIST ( liste ),
			       ligne,
			       0,
			       1,
			       0 );
	}
      break;


    /*  del  */
    case 65535 :

      supprime_operation_ventilation ();
      break;


    default : return (TRUE);
    }


  return (TRUE);
}
/***************************************************************************************************/






/***************************************************************************************************/
/* Fonction selectionne_ligne_souris */
/* place la sélection sur l'opé clickée */
/***************************************************************************************************/

void selectionne_ligne_souris_ventilation ( GtkCList *liste,
					    GdkEventButton *evenement,
					    gpointer null )
{
  gint colonne, x, y;
  gint ligne;

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste_operations_ventilees ),
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

  if ( !gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				 ligne ) )
    return;

  /*   vire l'ancienne sélection */

  gtk_clist_unselect_all ( GTK_CLIST ( liste ) );

  /* on met l'adr de la struct dans OPERATION_SELECTIONNEE */

  ligne_selectionnee_ventilation = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
							    ligne );

  gtk_clist_select_row ( GTK_CLIST ( liste ),
			 ligne,
			 colonne );

  gtk_widget_grab_focus ( GTK_WIDGET (liste) );

  if ( evenement -> type == GDK_2BUTTON_PRESS )
    edition_operation_ventilation ();

}
/***************************************************************************************************/



/***********************************************************************************************************/
/* Fonction appui_touche_ventilation  */
/* gére l'action du clavier sur les entrées du formulaire de ventilation */
/***********************************************************************************************************/

void appui_touche_ventilation ( GtkWidget *entree,
				GdkEventKey *evenement,
				gint *no_origine )
{
  gint origine;

  origine = GPOINTER_TO_INT ( no_origine );

  /*   si etat.entree = 1, la touche entrée finit l'opération ( fonction par défaut ) */
  /* sinon elle fait comme tab */

  if ( !etat.entree
       &&
       ( evenement->keyval == 65293
	 ||
	 evenement->keyval == 65421 ))
    evenement -> keyval = 65289;


  switch (evenement->keyval)
    {
      /* flèche bas */
    case 65364 :
      /*   flèche haut */
    case 65362 :

      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
				     "key_press_event");
      gtk_widget_grab_focus ( entree );
      break;


      /* tab */
    case 65289 :
      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
				     "key_press_event");

      /* on efface la sélection en cours si c'est une entrée ou un combofix */

      if ( GTK_IS_ENTRY ( entree ))
	gtk_entry_select_region ( GTK_ENTRY ( entree ),
				  0,
				  0);
      else
	if ( GTK_IS_COMBOFIX ( entree ))
	  gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( entree ) -> entry ),
				    0,
				    0);

      /* on donne le focus au widget suivant */


      origine = (origine + 1 ) % 6;

      while ( !(GTK_WIDGET_VISIBLE ( widget_formulaire_ventilation[origine] )
		&&
		GTK_WIDGET_SENSITIVE ( widget_formulaire_ventilation[origine] )
		&&
		( GTK_IS_COMBOFIX (widget_formulaire_ventilation[origine] )
		  ||
		  GTK_IS_ENTRY ( widget_formulaire_ventilation[origine] )
		  ||
		  GTK_IS_BUTTON ( widget_formulaire_ventilation[origine] ) )))
	origine = (origine + 1 ) % 6;

     /*       si on se retrouve sur les catég et que etat.entree = 0, on enregistre l'opérations */

      if ( !origine && !etat.entree )
	{
	  fin_edition_ventilation();
	  return;
	}

      /* on sélectionne le contenu de la nouvelle entrée */

      if ( GTK_IS_COMBOFIX ( widget_formulaire_ventilation[origine] ) )
	{
	  gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[origine] ) -> entry );  
	  gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_ventilation[origine] ) -> entry ),
				    0,
				    -1 );
	}
      else
	{
	  if ( GTK_IS_ENTRY ( widget_formulaire_ventilation[origine] ) )
	    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_ventilation[origine] ),
				      0,
				      -1 );

	  gtk_widget_grab_focus ( widget_formulaire_ventilation[origine]  );
	}
      break;

      /* entree */
    case 65293 :
    case 65421 :

      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
				     "key_press_event");
      fin_edition_ventilation ();
      break;


    case 65307 :
              /* echap */
      echap_formulaire_ventilation ();
      break;


    default : 
    }

}
/***********************************************************************************************************/





/***********************************************************************************************************/
void echap_formulaire_ventilation ( void )
{

  /* on met les styles des entrées au gris */

  gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] )->entry,
			 style_entree_formulaire[1] );
  gtk_widget_set_style ( widget_formulaire_ventilation[1],
			 style_entree_formulaire[1] );
  gtk_widget_set_style ( widget_formulaire_ventilation[2],
			 style_entree_formulaire[1] );
  gtk_widget_set_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[3] )->entry,
			 style_entree_formulaire[1] );
  gtk_widget_set_style ( widget_formulaire_ventilation[5],
			 style_entree_formulaire[1] );


  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ),
			  _("Catégories : Sous-catégories") );
  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[1]),
		       _("Notes") );
  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[2]),
		       _("Montant") );
  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[3] ),
			  _("Imputation budgétaire") );
  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[5]),
		       _("Pièce comptable") );

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_ventilation[4] ),
				0 );

  gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_ventilation[4] ),
			     FALSE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ventil ),
			     FALSE );
  gtk_widget_set_sensitive ( widget_formulaire_ventilation[2],
			     TRUE );

/*   met l'adr de l'opé dans le formulaire à -1 */

  gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[0] ),
			"adr_struct_ope",
			GINT_TO_POINTER ( -1 ) );

  gtk_widget_grab_focus ( liste_operations_ventilees );
}
/***********************************************************************************************************/





/***********************************************************************************************************/
void fin_edition_ventilation ( void )
{
  struct structure_operation *operation;
  gint modification;
  gint rafraichir_categ;
  gint virement;
  gchar *nom_compte_vire;
  gint rafraichir_imputation;

  rafraichir_categ = 0;
  rafraichir_imputation = 0;
  virement = 0;
  nom_compte_vire = NULL;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  operation = gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_ventilation[0] ),
				    "adr_struct_ope" );


  if (operation == GINT_TO_POINTER ( -1 ))
    {
      operation = calloc ( 1,
			   sizeof ( struct structure_operation ));
      modification = 0;

      /*   met la relation de ventilation à -1 car n'est pas encore associée */

      operation -> no_operation_ventilee_associee = -1;
    }
  else
    modification = 1;



  /* récupération des notes */

  if ( gtk_widget_get_style ( widget_formulaire_ventilation[1] ) == style_entree_formulaire[0] )
    operation -> notes = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[1] ))));
  else
    operation -> notes = NULL;


  /* récupération du montant */

  operation -> montant = g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[2] ))),
				    NULL );

  /* remplissage du no de compte */

  operation -> no_compte = compte_courant;


  /*   récupération des catégories / sous-catég, s'ils n'existent pas, on les crée */
  /* si c'est un virement, on  met virement à 1 */


  if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ) -> entry ) == style_entree_formulaire[0] )
    {
      struct struct_categ *categ;
      gchar **tableau_char;

      tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ))),
				  ":",
				  2 );
      if ( tableau_char[0]  )
	{
	  tableau_char[0] = g_strstrip ( tableau_char[0] );

	  if ( tableau_char[1] )
	    tableau_char[1] = g_strstrip ( tableau_char[1] );


	  if ( strlen ( tableau_char[0] ) )
	    {

	      /* on vérifie ici si c'est un virement */

	      if ( strcmp ( tableau_char[0],
			    _("Virement") ) )
		{
		  /* 	      ce n'est pas un virement, recherche les catég */

		  GSList *pointeur_liste;

		  virement = 0;

		  /* si c'est une modif d'opé et que l'ancienne opé était un virement, on vire l'opé */
		  /* associée */

		  if ( modification
		       &&
		       operation -> relation_no_operation )
		    {
		      struct structure_operation *ope_associee;


		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		      ope_associee = g_slist_find_custom ( LISTE_OPERATIONS,
							   GINT_TO_POINTER ( operation -> relation_no_operation ),
							   (GCompareFunc) recherche_operation_par_no ) -> data;

		      ope_associee -> relation_no_operation = 0;

		      supprime_operation ( ope_associee );

		      operation -> relation_no_operation = 0;
		      operation -> relation_no_compte = 0;
		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
		    }

		  /* recherche des catégories */

		  pointeur_liste = g_slist_find_custom ( liste_struct_categories,
							 tableau_char[0],
							 ( GCompareFunc ) recherche_categorie_par_nom );

		  if ( pointeur_liste )
		    {
		      categ = pointeur_liste -> data;
		      rafraichir_categ = 0;
		    }
		  else
		    {
		      categ = ajoute_nouvelle_categorie ( tableau_char[0] );
		      if ( operation -> montant < 0 )
			categ -> type_categ = 1;
		      else
			categ -> type_categ = 0;
		      rafraichir_categ = 1;
		    }
	  
		  operation -> categorie = categ -> no_categ;
	  
		  if ( tableau_char[1] && strlen (tableau_char[1]) )
		    {
		      struct struct_sous_categ *sous_categ;
		  
		      pointeur_liste = g_slist_find_custom ( categ -> liste_sous_categ,
							     tableau_char[1],
							     ( GCompareFunc ) recherche_sous_categorie_par_nom );
	      
		      if ( pointeur_liste )
			{
			  sous_categ = pointeur_liste -> data;
			  rafraichir_categ = 0;
			}
		      else
			{
			  sous_categ = ajoute_nouvelle_sous_categorie ( tableau_char[1],
									categ );
			  rafraichir_categ = 1;
			}
		  
		      operation -> sous_categorie = sous_categ -> no_sous_categ;
		    }
		  else
		    operation -> sous_categorie = 0;
		}
	      else
		{
		  /* c'est un virement, les no de categ sont déjà à 0 */

		  virement = 1;
		  nom_compte_vire = g_strdup ( tableau_char [1] );
		}
	    }
	}
      g_strfreev ( tableau_char );
    }


  /* récupération de l'imputation budgétaire */

  if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_ventilation[3] ) -> entry ) == style_entree_formulaire[0] )
    {
      struct struct_imputation *imputation;
      gchar **tableau_char;
      GSList *pointeur_liste;

      tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[3] )),
				  ":",
				  2 );
      
      tableau_char[0] = g_strstrip ( tableau_char[0] );

      if ( tableau_char[1] )
	tableau_char[1] = g_strstrip ( tableau_char[1] );

      pointeur_liste = g_slist_find_custom ( liste_struct_imputation,
					     tableau_char[0],
					     ( GCompareFunc ) recherche_imputation_par_nom );

      if ( pointeur_liste )
	imputation = pointeur_liste -> data;
      else
	{
	  imputation = ajoute_nouvelle_imputation ( tableau_char[0] );

	  if ( operation -> montant < 0 )
	    imputation -> type_imputation = 1;
	  else
	    imputation -> type_imputation = 0;

	  rafraichir_imputation = 1;
	}

      operation -> imputation = imputation -> no_imputation;
	  
      if ( tableau_char[1] && strlen (tableau_char[1]) )
	{
	  struct struct_sous_imputation *sous_imputation;
		  
	  pointeur_liste = g_slist_find_custom ( imputation -> liste_sous_imputation,
						 tableau_char[1],
						 ( GCompareFunc ) recherche_sous_imputation_par_nom );
	      
	  if ( pointeur_liste )
	    sous_imputation = pointeur_liste -> data;
	  else
	    {
	      sous_imputation = ajoute_nouvelle_sous_imputation ( tableau_char[1],
								  imputation );
	      rafraichir_imputation = 1;
	    }

	  operation -> sous_imputation = sous_imputation -> no_sous_imputation;
	}
      else
	operation -> sous_imputation = 0;

      g_strfreev ( tableau_char );
    }


  /* récupération du no d'exercice */

  operation -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_ventilation[4] ) -> menu_item ),
								     "no_exercice" ));


/* récupération du no de pièce comptable */

  if ( gtk_widget_get_style ( widget_formulaire_ventilation[5] ) == style_entree_formulaire[0] )
    operation -> no_piece_comptable = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_ventilation[5] ))));
  else
    operation -> no_piece_comptable = NULL;


  /* remplit la date : cree une date bidon qui sera remlit quand l'opération en cours sera validée */

  operation -> jour = 1;
  operation -> mois = 1;
  operation -> annee = 2000;

  operation -> date = g_date_new_dmy ( operation -> jour,
					  operation -> mois,
					  operation -> annee );

  /* crée une devise  bidon pour éviter un segfault si on réussi à changer de compte avant la validation de l'opé ventilée */

  operation -> devise = 1;

 
  /* on a rempli l'opération, on l'ajoute au compte */
  /*   le tiers, la date ... seront remplis lors de la validation de l'opération principale */

  if ( !modification )
    ajout_operation ( operation );



  if ( virement )
    {
      /* 	      c'est un virement ; trois solutions : */
      /*  soit c'est une édition d'opération, c'était un virement et le compte viré n'a pas changé */
      /* soit  c'est une édition d'opération, c'était un virement et le compte viré a changé */
      /* soit c'est un nouveau virement, crée l'opé associée dans ce cas */


      /* recherche du numéro de compte entré */

      gint compte_vire;
      gint i;

      compte_vire = -1;

      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	  if ( !strcmp ( NOM_DU_COMPTE,
			 nom_compte_vire ) )
	    compte_vire = i;
	}

      if ( compte_vire == -1 )
	{
	  dialogue ( _("Erreur : le compte associé au virement est invalide") );
	  return;
	}

      if ( compte_vire == compte_courant )
	{
	  dialogue ( _("Erreur : impossible de virer un compte sur lui-même") );
	  return;
	}

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_vire;

      /* la soluce la plus facile : c'est une modif d'opé, pas de changement pour le compte viré */

      if ( modification
	   &&
	   ( operation -> relation_no_compte == compte_vire ))
	{
	  /* récupère l'opération associée */

	  struct structure_operation *ope_associee;

	  ope_associee = g_slist_find_custom ( LISTE_OPERATIONS,
					       GINT_TO_POINTER ( operation -> relation_no_operation ),
					       (GCompareFunc) recherche_operation_par_no ) -> data;


	  /* on y met le nouveau montant et la note, l'ib, l'exercice et la pièce comptable */

	  ope_associee -> montant = operation -> montant;


	  if ( operation -> notes )
	    ope_associee -> notes = g_strdup ( operation -> notes );
	  else
	    ope_associee -> notes = NULL;

	  ope_associee -> imputation = operation -> imputation;
	  ope_associee -> sous_imputation = operation -> sous_imputation;
	  ope_associee -> no_exercice = operation -> no_exercice;

	  if ( operation -> no_piece_comptable )
	    ope_associee -> no_piece_comptable = g_strdup ( operation -> no_piece_comptable );
	  else
	    ope_associee -> no_piece_comptable = NULL;

	  MISE_A_JOUR = 1;
	}
      else
	{
	  /* deuxième solution, c'est une modif d'opé, on a changé le compte viré */
		  
	  struct structure_operation *ope_associee;

	  if ( modification
	       &&
	       ( operation -> relation_no_compte != compte_vire ))
	    {
	      /* on supprime l'ancienne opé en relation, et on recrée ensuite la nouvelle opé */

	      /* récupère l'opération associée */
		      

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	      ope_associee = g_slist_find_custom ( LISTE_OPERATIONS,
						   GINT_TO_POINTER ( operation -> relation_no_operation ),
						   (GCompareFunc) recherche_operation_par_no ) -> data;

	      ope_associee -> relation_no_operation = 0;

	      supprime_operation ( ope_associee );

	    }

	  /*  troisième solution : c'est un nouveau virement ( ou si c'était une modif avec changement */
	  /*   de compte viré */
	  /* cette fois, on crée la nouvelle opé associée */
	  /* à ce niveau, si c'est une nouvelle opération qui est crée, son numéro sera DERNIER_NO + 1 */
	  /* sinon on a déjà son numéro */


	  ope_associee = calloc ( 1,
				  sizeof ( struct structure_operation ));

	  /* on y met le nouveau montant et la note */
	  /* 	  si la ventilation est un crédit, on inverse le montant */

	  ope_associee -> montant = operation -> montant;


	  if ( operation -> notes )
	    ope_associee -> notes = g_strdup ( operation -> notes );
	  else
	    ope_associee -> notes = NULL;

	  ope_associee -> no_compte = compte_vire;
	  ope_associee -> type_ope = operation -> type_ope;

	  if ( operation -> contenu_type )
	    ope_associee -> contenu_type = g_strdup ( operation -> contenu_type );

	  ope_associee -> no_exercice = operation -> no_exercice;
	  ope_associee -> imputation = operation -> imputation;
	  ope_associee -> sous_imputation = operation -> sous_imputation;

	  if ( operation -> no_piece_comptable )
	    ope_associee -> no_piece_comptable = g_strdup ( operation -> no_piece_comptable );

	  /* remplit la date : cree une date bidon qui sera remlit quand l'opération en cours sera validée */

	  ope_associee -> jour = 1;
	  ope_associee -> mois = 1;
	  ope_associee -> annee = 2000;

	  ope_associee -> date = g_date_new_dmy ( ope_associee -> jour,
						  ope_associee -> mois,
						  ope_associee -> annee );

	  /* crée une devise  bidon pour éviter un segfault si on réussi à changer de compte avant la validation de l'opé ventilée */

	  ope_associee -> devise = 1;

	  /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

	  ajout_operation ( ope_associee );

	  /* on met maintenant les relations entre les différentes opé */

	  operation -> relation_no_operation = ope_associee -> no_operation;
	  operation -> relation_no_compte = ope_associee -> no_compte;
	  ope_associee -> relation_no_operation = operation -> no_operation;
	  ope_associee -> relation_no_compte = operation -> no_compte;
	}
    }


  if ( rafraichir_categ )
    mise_a_jour_categ ();

  if ( rafraichir_imputation )
    mise_a_jour_imputation ();

  /* si c'est une modif, on supprime l'ancienne ligne de l'opé */

  if ( modification )
    gtk_clist_remove ( GTK_CLIST ( liste_operations_ventilees ),
		       gtk_clist_find_row_from_data ( GTK_CLIST ( liste_operations_ventilees ),
						      operation ));

  /* on met à jour la liste des ventilations */

  ajoute_ope_sur_liste_ventilation ( operation );
  gtk_clist_sort ( GTK_CLIST ( liste_operations_ventilees ));
  mise_a_jour_couleurs_liste_ventilation ();
  selectionne_ligne_ventilation ();
  calcule_montant_ventilation ();

  /* efface le formulaire et prépare l'opé suivante */

  echap_formulaire_ventilation ();

  if ( modification )
    gtk_widget_grab_focus ( liste_operations_ventilees );
  else
    {
      clique_champ_formulaire_ventilation ();
      gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ) -> entry );
    }
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Fonction edition_operation_ventilation */
/* appelé lors d'un double click ou entrée sur une opé de ventilation */
/***********************************************************************************************************/

void edition_operation_ventilation ( void )
{
  struct structure_operation *operation;
  GSList *liste_tmp;


  operation = ligne_selectionnee_ventilation;

  echap_formulaire_ventilation ();

  /* dégrise ce qui est nécessaire */

  clique_champ_formulaire_ventilation ();

  gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[0] ),
			"adr_struct_ope",
			operation );

/* si l'opé est -1, c'est que c'est une nouvelle opé */

  if ( operation == GINT_TO_POINTER ( -1 ) )
    {
      gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ) -> entry );
      return;
    }


/*   l'opé n'est pas -1, c'est une modif, on remplit les champs */

  gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_ventilation[0] ),
			"adr_struct_ope",
			operation );


  /* mise en forme du montant */

  entree_prend_focus (widget_formulaire_ventilation[2] );

  if ( operation -> montant < 0 )
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[2] ),
			 g_strdup_printf ( "%4.2f", -operation -> montant ));
  else
    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[2] ),
			 g_strdup_printf ( "%4.2f", operation -> montant ));

  /* si l'opération est relevée, empêche la modif du montant */

  if ( operation -> pointe == 2 )
    gtk_widget_set_sensitive ( widget_formulaire_ventilation[2],
			       FALSE );


  /* mise en forme des catégories */

  if ( operation -> relation_no_operation )
    {
      /* c'est un virement */

      entree_prend_focus (widget_formulaire_ventilation[0] );

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

      gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ),
			      g_strconcat ( _("Virement : "),
					    NOM_DU_COMPTE,
					    NULL ));
    }
  else
    {
      liste_tmp = g_slist_find_custom ( liste_struct_categories,
					GINT_TO_POINTER ( operation -> categorie ),
					( GCompareFunc ) recherche_categorie_par_no );

      if ( liste_tmp )
	{
	  GSList *liste_tmp_2;
      
	  entree_prend_focus (widget_formulaire_ventilation[0] );

	  liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
					      GINT_TO_POINTER ( operation -> sous_categorie ),
					      ( GCompareFunc ) recherche_sous_categorie_par_no );
	  if ( liste_tmp_2 )
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ),
				    g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
						  " : ",
						  (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
						  NULL ));
	  else
	    gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ),
				    (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ );
      
	}
    }


  /* mise en forme des notes */

  if ( operation -> notes )
    {
      entree_prend_focus (widget_formulaire_ventilation[1] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[1] ),
			   operation -> notes );
    }

  /* met en place l'exercice */

  gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_ventilation[4] ),
				 cherche_no_menu_exercice ( operation -> no_exercice ));

  /* met en place l'imputation budgétaire */

  liste_tmp = g_slist_find_custom ( liste_struct_imputation,
				    GINT_TO_POINTER ( operation -> imputation ),
				    ( GCompareFunc ) recherche_imputation_par_no );

  if ( liste_tmp )
    {
      GSList *liste_tmp_2;

      entree_prend_focus ( widget_formulaire_ventilation[3] );

      liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( liste_tmp -> data )) -> liste_sous_imputation,
					  GINT_TO_POINTER ( operation -> sous_imputation ),
					  ( GCompareFunc ) recherche_sous_imputation_par_no );
      if ( liste_tmp_2 )
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[3] ),
				g_strconcat ( (( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation,
					      " : ",
					      (( struct struct_sous_imputation * )( liste_tmp_2 -> data )) -> nom_sous_imputation,
					      NULL ));
      else
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_ventilation[3] ),
				(( struct struct_imputation * )( liste_tmp -> data )) -> nom_imputation );
    }


  /* mise en place de la pièce comptable */

  if ( operation -> no_piece_comptable )
    {
      entree_prend_focus ( widget_formulaire_ventilation[5] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_ventilation[5] ),
			   operation -> no_piece_comptable );
    }


/*   on a fini de remplir le formulaire, on donne le focus à la date */

  gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ) -> entry ),
			    0,
			    -1);
  gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ) -> entry );
}
/***********************************************************************************************************/





/***********************************************************************************************************/
void supprime_operation_ventilation ( void )
{
  struct structure_operation *operation;
  gint ligne;


  operation = ligne_selectionnee_ventilation;

  if ( operation == GINT_TO_POINTER ( -1 ) )
    return;

  if ( operation -> pointe == 2 )
    {
      dialogue ( _(" Impossible de supprimer \n  une opération relevée ... "));
      return;
    }


  /* si l'opération est liée, on recherche l'autre opé on vire ses liaisons et on l'efface */

  if ( operation -> relation_no_operation && operation -> relation_no_compte != -1 )
    {
      struct structure_operation *ope_liee;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

      ope_liee = g_slist_find_custom ( LISTE_OPERATIONS,
				       GINT_TO_POINTER ( operation -> relation_no_operation ),
				       ( GCompareFunc ) recherche_operation_par_no ) -> data;

      ope_liee -> relation_no_operation = 0;
      ope_liee -> relation_no_compte = 0;

      supprime_operation ( ope_liee );
    }


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

  /*   si la sélection est sur l'opé qu'on supprime, on met la sélection sur celle du dessous */

  ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_operations_ventilees ),
					 operation );
  ligne_selectionnee_ventilation = gtk_clist_get_row_data ( GTK_CLIST ( liste_operations_ventilees ),
							    ligne + 1 );

  /* supprime l'opération de la liste */

  gtk_clist_remove ( GTK_CLIST ( liste_operations_ventilees ),
		     ligne );

  /* supprime l'opération dans la liste des opés */

  LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS,
				      operation );
  NB_OPE_COMPTE--;


/* on réaffiche la liste des tiers */

  modif_tiers = 1;


  calcule_montant_ventilation();
  mise_a_jour_couleurs_liste_ventilation();
  selectionne_ligne_ventilation ();

  modification_fichier( TRUE );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction affiche_liste_ventilation */
/* récupère la liste des no d'opés de ventilation du formulaire et affiche ces opés */
/* appelée lorsqu'on ventile une opé */
/***********************************************************************************************************/

void affiche_liste_ventilation ( void )
{
  gchar *ligne[3];
  gint ligne_insertion;
  GSList *liste_tmp;


  somme_ventilee = 0;

  gtk_clist_clear ( GTK_CLIST ( liste_operations_ventilees ) );
  gtk_clist_freeze ( GTK_CLIST ( liste_operations_ventilees ) );



  /* récupère la liste des ventilations */

  liste_tmp = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				    "liste_adr_ventilation" );

  while ( liste_tmp && GPOINTER_TO_INT ( liste_tmp ) != -1 )
    {
      ajoute_ope_sur_liste_ventilation ( liste_tmp -> data );

      liste_tmp = liste_tmp -> next;
    }


  /* ajoute la ligne blanche associee à -1 */

  ligne[0] = NULL;
  ligne[1] = NULL;
  ligne[2] = NULL;

  ligne_insertion = gtk_clist_append ( GTK_CLIST ( liste_operations_ventilees ),
				       ligne );

  gtk_clist_set_row_data ( GTK_CLIST ( liste_operations_ventilees ),
			   ligne_insertion,
			   GINT_TO_POINTER ( -1 ));



  /* classe la liste des opés */

  gtk_clist_sort ( GTK_CLIST ( liste_operations_ventilees ));

  /* on met la couleur */

  mise_a_jour_couleurs_liste_ventilation ();


  /* on sélectionne la ligne blanche */

  selectionne_ligne_ventilation ();

  gtk_clist_thaw ( GTK_CLIST ( liste_operations_ventilees ));

  /* on met à jour les labels d'état */

  calcule_montant_ventilation ();
}
/***********************************************************************************************************/


/***********************************************************************************************************/
void ajoute_ope_sur_liste_ventilation ( struct structure_operation *operation )
{
  gchar *ligne[3];
  gint ligne_insertion;
  GSList *liste_tmp;


  /* mise en forme des catégories */

  if ( operation -> relation_no_operation )
    {
      /* c'est un virement */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

      ligne [0] = g_strconcat ( _("Virement : "),
				NOM_DU_COMPTE,
				NULL );
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
    }
  else
    {
      /* c'est des categ : sous categ */

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
	    ligne [0] = g_strconcat ( (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ,
				      " : ",
				      (( struct struct_sous_categ * )( liste_tmp_2 -> data )) -> nom_sous_categ,
				      NULL );
	  else
	    ligne [0] = (( struct struct_categ * )( liste_tmp -> data )) -> nom_categ;
      
	}
      else
	ligne[0] = NULL;
    }


  /* mise en forme des notes */

  ligne[1] = operation -> notes;

  /* mise en forme du montant */
  /*   si la ventilation est un débit, on inverse le signe */

  ligne[2] = g_strdup_printf ( "%4.2f",
			       operation -> montant );


  ligne_insertion = gtk_clist_append ( GTK_CLIST ( liste_operations_ventilees ),
				       ligne );

  /* on associe à cette ligne l'adr de la struct de l'opé */

  gtk_clist_set_row_data ( GTK_CLIST ( liste_operations_ventilees ),
			   ligne_insertion,
			   operation);

  calcule_montant_ventilation ();
}
/***********************************************************************************************************/


/***********************************************************************************************************/
void calcule_montant_ventilation ( void )
{
  gint ligne;
  struct structure_operation *operation;

  /* on va recréer une liste contenant les nos d'opération de la ventilation */
  /* cette liste est mise dans "liste_adr_ventilation" du formulaire */

  ligne = 0;
  somme_ventilee = 0;

  while ( ( operation = gtk_clist_get_row_data ( GTK_CLIST ( liste_operations_ventilees ),
						 ligne )) != GINT_TO_POINTER ( -1 )
	  &&
	  operation )
    {
      somme_ventilee = somme_ventilee + operation -> montant;
      ligne++;
    }

  mise_a_jour_labels_ventilation ();
}
/***********************************************************************************************************/




/***********************************************************************************************************/
void mise_a_jour_labels_ventilation ( void )
{
  gtk_label_set_text ( GTK_LABEL ( label_somme_ventilee ),
		       g_strdup_printf ( "%4.2f",
					 somme_ventilee ));

  if ( montant_operation_ventilee )
    {
      gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			   g_strdup_printf ( "%4.2f",
					     montant_operation_ventilee ));

      gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			   g_strdup_printf ( "%4.2f",
					     montant_operation_ventilee - somme_ventilee ));
    }
  else
    {
      gtk_label_set_text ( GTK_LABEL ( label_non_affecte ),
			   "" );
      gtk_label_set_text ( GTK_LABEL ( label_montant_operation_ventilee ),
			   g_strdup_printf ( "%4.2f",
					     somme_ventilee ));
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fait le tour le la liste de ventilation et met bien les couleurs */
/***********************************************************************************************************/

void mise_a_jour_couleurs_liste_ventilation ( void )
{
  gint i;

  i=-1;

  do
    {
      i++;

      gtk_clist_set_row_style ( GTK_CLIST ( liste_operations_ventilees ),
				i,
				style_couleur [ i % 2 ] );
    }
  while ( gtk_clist_get_row_data ( GTK_CLIST ( liste_operations_ventilees ),
				   i ) != GINT_TO_POINTER ( -1 ));
}
/***********************************************************************************************************/


/***********************************************************************************************************/
void selectionne_ligne_ventilation ( void )
{
  gint ligne_selectionnee;

  ligne_selectionnee = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_operations_ventilees ),
						      ligne_selectionnee_ventilation );

  gtk_clist_select_row ( GTK_CLIST ( liste_operations_ventilees ),
			 ligne_selectionnee,
			 0 );

  if ( gtk_clist_row_is_visible ( GTK_CLIST ( liste_operations_ventilees ),
				 ligne_selectionnee )
       != GTK_VISIBILITY_FULL )
    gtk_clist_moveto ( GTK_CLIST ( liste_operations_ventilees ),
		       ligne_selectionnee,
		       0,
		       0.5,
		       0 );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* Fonction valider_ventilation */
/* appelée par appui du bouton fermer */
/***********************************************************************************************************/

void valider_ventilation ( void )
{
  GSList *liste_ope;
  gint ligne;
  gpointer *pointeur;

  /* on va recréer une liste contenant les nos d'opération de la ventilation */
  /* cette liste est mise dans "liste_adr_ventilation" du formulaire */

  liste_ope = NULL;
  ligne = 0;

  while ( ( pointeur = gtk_clist_get_row_data ( GTK_CLIST ( liste_operations_ventilees ),
						ligne )) != GINT_TO_POINTER ( -1 ))
    {
      liste_ope = g_slist_append ( liste_ope,
				   pointeur );

      ligne++;
    }

  /* on associe l'adr de la nouvelle liste des ventilation au formulaire, met -1 si la liste est vide */

  if ( liste_ope )
    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "liste_adr_ventilation",
			  liste_ope );
  else
    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "liste_adr_ventilation",
			  GINT_TO_POINTER ( -1 ) );

  gtk_widget_show ( barre_outils );
  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			  compte_courant + 2 );
  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			  0 );
  gtk_widget_show ( formulaire );

  /*   on a réaffiché le formulaire, on peut débloquer les fonctions */
  /* qui modifient la position dans la liste des opés */

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( frame_droite_bas ),
			       allocation_taille_formulaire,
			       NULL );
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( frame_droite_bas ),
			       efface_formulaire,
			       NULL );



  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire ),
			  0 );

  gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_operations[8] ) -> entry );



  if ( !montant_operation_ventilee )
    {
      if ( somme_ventilee < 0 )
	{
	  entree_prend_focus ( widget_formulaire_operations[3] );
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[3] ),
			       g_strdup_printf ( "%4.2f",
						 fabs (somme_ventilee) ));
	}
      else
	{
	  entree_prend_focus ( widget_formulaire_operations[4] );
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[4] ),
			       g_strdup_printf ( "%4.2f",
						 somme_ventilee ));
	}
    }

  modification_fichier ( TRUE );

  if ( enregistre_ope_au_retour )
    fin_edition();

}
/***********************************************************************************************************/
