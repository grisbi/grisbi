/* Ce fichier s'occupe de la gestion du formulaire de saisie des opérations */
/* formulaire.c */

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


/***********************************************************************************************************/
/*  Routine qui crée le formulaire et le renvoie */
/***********************************************************************************************************/

GtkWidget *creation_formulaire ( void )
{
  GtkWidget *table;
  GtkWidget *menu;
  GtkWidget *bouton;
  GdkColor couleur_normale;
  GdkColor couleur_grise;
  GtkTooltips *tips;

  /*   on crée tout de suite les styles qui seront appliqués aux entrées du formulaire : */
  /*     style_entree[0] sera la couleur noire, normale */
  /*     Style_entree[1] sera une couleur atténuée quand le formulaire est vide */

  couleur_normale.red = COULEUR_NOIRE_RED;
  couleur_normale.green = COULEUR_NOIRE_GREEN;
  couleur_normale.blue = COULEUR_NOIRE_BLUE;

  couleur_grise.red = COULEUR_GRISE_RED;
  couleur_grise.green = COULEUR_GRISE_GREEN;
  couleur_grise.blue = COULEUR_GRISE_BLUE;

  style_entree_formulaire [0] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
  style_entree_formulaire [0]->fg[GTK_STATE_NORMAL] = couleur_normale;

  style_entree_formulaire [1] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
  style_entree_formulaire [1]->fg[GTK_STATE_NORMAL] = couleur_grise;


  /* on crée le tooltips */

  tips = gtk_tooltips_new ();

  /*   le formulaire est une vbox avec en haut un tableau de 6 colonnes, et */
  /* en bas si demandé les boutons valider et annuler */

  formulaire = gtk_vbox_new ( FALSE,
			      5 );

  /* le formulaire est une table de 7 colonnes  sur 4 */

  table = gtk_table_new (4, 7, FALSE);

  gtk_table_set_col_spacings ( GTK_TABLE ( table ),
			       10 );
  gtk_box_pack_start ( GTK_BOX ( formulaire ),
		       table,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( table );

  /*   met la taille du formulaire à 1 au début pour éviter un agrandissement automatique de la fenêtre dû aux */
  /* tailles par défaut des entrées */
  /* cette taille sera modifiée automatiquement à l'affichage */

  gtk_widget_set_usize ( GTK_WIDGET ( table ),
			 1,
			 FALSE );

  /* no d'opé */

  widget_formulaire_operations[0] = gtk_label_new ( "" );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[0],
		     0, 1, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  if ( etat.affiche_no_operation )
    gtk_widget_show ( widget_formulaire_operations[0] );


  /* entrée de la date */

  widget_formulaire_operations[1] = gtk_entry_new();
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[1],
		     1, 2, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[1]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(1) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[1]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(1) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[1]),
 		       "focus_in_event",
		       GTK_SIGNAL_FUNC (entree_prend_focus),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[1]),
 		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_perd_focus),
		       GINT_TO_POINTER(1) );
  gtk_widget_show (widget_formulaire_operations[1]);


  /*  entrée du tiers : c'est une combofix */

  creation_liste_tiers_combofix();
  widget_formulaire_operations[2] = gtk_combofix_new_complex ( liste_tiers_combofix,
							       FALSE,
							       TRUE,
							       TRUE,
							       0 );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[2],
		     2,3, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX ( widget_formulaire_operations[2] ) -> entry),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(2) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX ( widget_formulaire_operations[2] ) -> arrow ),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(2) );
  gtk_signal_connect_object ( GTK_OBJECT (GTK_COMBOFIX ( widget_formulaire_operations[2]) -> entry),
			      "focus_in_event",
			      GTK_SIGNAL_FUNC (entree_prend_focus),
			      GTK_OBJECT ( widget_formulaire_operations[2] ));
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX ( widget_formulaire_operations[2]) -> entry),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_perd_focus),
		       GINT_TO_POINTER(2) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX ( widget_formulaire_operations[2] ) -> entry),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(2) );
  gtk_widget_show (widget_formulaire_operations[2]);



  /*  Affiche le débit */

  widget_formulaire_operations[3] = gtk_entry_new ();
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[3],
		     3, 4, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[3]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(3) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[3]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(3) );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[3]),
			     "focus_in_event",
			     GTK_SIGNAL_FUNC (entree_prend_focus),
			     NULL );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[3]),
			     "focus_out_event",
			     GTK_SIGNAL_FUNC (entree_perd_focus),
			     GINT_TO_POINTER (3) );
  gtk_widget_show (widget_formulaire_operations[3]);


  /*  Affiche le crédit */

  widget_formulaire_operations[4] = gtk_entry_new ();
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[4],
		     4,5,0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[4]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(4) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[4]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(4) );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[4]),
			     "focus_in_event",
			     GTK_SIGNAL_FUNC (entree_prend_focus),
			     NULL );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[4]),
			     "focus_out_event",
			     GTK_SIGNAL_FUNC (entree_perd_focus),
			     GINT_TO_POINTER (4) );
  gtk_widget_show (widget_formulaire_operations[4]);


  /* met l'option menu des devises */

  widget_formulaire_operations[5] = gtk_option_menu_new ();
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			 widget_formulaire_operations[5],
			 _("Choix de la devise"),
			 _("Choix de la devise") );
  menu = creation_option_menu_devises ( -1,
					liste_struct_devises );
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ),
			     menu );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[5]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(5) );
  gtk_table_attach ( GTK_TABLE ( table ),
		     widget_formulaire_operations[5],
		     5, 6, 0,1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show ( widget_formulaire_operations[5] );


  /* mise en forme du bouton change */

  widget_formulaire_operations[6] = gtk_button_new_with_label ( _("Change") );
  gtk_button_set_relief ( GTK_BUTTON ( widget_formulaire_operations[6] ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT (  widget_formulaire_operations[6] ),
 		       "clicked",
		       GTK_SIGNAL_FUNC ( click_sur_bouton_voir_change ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[6]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(6) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[6],
		     6,7, 0, 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show ( widget_formulaire_operations[6] );


  /* mise en place de la date de valeur */

  widget_formulaire_operations[7] = gtk_entry_new();
  gtk_table_attach ( GTK_TABLE (table),
 		     widget_formulaire_operations[7],
 		     1, 2, 1, 2,
 		     GTK_SHRINK | GTK_FILL,
 		     GTK_SHRINK | GTK_FILL,
 		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[7]),
  		       "button_press_event",
 		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
 		       GINT_TO_POINTER(7)  );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[7]),
  		       "key_press_event",
 		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
 		       GINT_TO_POINTER(7)  );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[7]),
  		       "focus_in_event",
 		       GTK_SIGNAL_FUNC (entree_prend_focus),
 		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[7]),
  		       "focus_out_event",
 		       GTK_SIGNAL_FUNC (entree_perd_focus),
 		       GINT_TO_POINTER(7) );
    gtk_widget_show ( widget_formulaire_operations[7] );

    gtk_widget_set_sensitive ( widget_formulaire_operations[7],
			       etat.affiche_date_bancaire );


  /*  Affiche les catégories / sous-catégories */

  widget_formulaire_operations[8] = gtk_combofix_new_complex ( liste_categories_combofix,
							       FALSE,
							       TRUE,
							       TRUE,
							       0 );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[8],
		     2, 3, 1,2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[8]) -> entry),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(8) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[8]) -> arrow),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(8) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[8]) -> entry),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(8) );
  gtk_signal_connect_object ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[8]) -> entry),
			      "focus_in_event",
			      GTK_SIGNAL_FUNC (entree_prend_focus),
			      GTK_OBJECT ( widget_formulaire_operations[8] ));
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[8]) -> entry),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_perd_focus),
		       GINT_TO_POINTER (8) );
  gtk_widget_show (widget_formulaire_operations[8]);


  /*   création de l'entrée du chèque, non affichée pour le moment */
  /* à créer avant l'option menu du type d'opé */

  widget_formulaire_operations[10] = gtk_entry_new();
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[10]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(10) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[10]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(10) );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[10]),
			     "focus_in_event",
			     GTK_SIGNAL_FUNC (entree_prend_focus),
			     NULL );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[10]),
			     "focus_out_event",
			     GTK_SIGNAL_FUNC (entree_perd_focus),
			     GINT_TO_POINTER (10) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[10],
		     5, 7, 1, 2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);

  /*  Affiche l'option menu des types */

  widget_formulaire_operations[9] = gtk_option_menu_new ();
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			 widget_formulaire_operations[9],
			 _("Choix du mode de règlement"),
			 _("Choix du mode de règlement") );
  gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_operations[9] ),
		       "key_press_event",
		       GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
		       GINT_TO_POINTER(9) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[9],
		     3, 5, 1,2,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);

  /* le menu par défaut est celui des débits */

  if ( (menu = creation_menu_types ( 1, compte_courant, 0  )))
    {
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
				 menu );
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
				    cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
      gtk_widget_show ( widget_formulaire_operations[9] );
    }


  /* met l'option menu de l'exercice */


  widget_formulaire_operations[11] = gtk_option_menu_new ();
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			 widget_formulaire_operations[11],
			 _("Choix de l'exercice"),
			 _("Choix de l'exercice") );
  menu = gtk_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[11] ),
			     creation_menu_exercices (0) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[11]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(11) );
  gtk_table_attach ( GTK_TABLE ( table ),
		     widget_formulaire_operations[11],
		     0, 2, 2, 3,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show ( widget_formulaire_operations[11] );

  gtk_widget_set_sensitive ( widget_formulaire_operations[11],
			     etat.utilise_exercice );


  /*  Affiche l'imputation budgétaire */

  widget_formulaire_operations[12] = gtk_combofix_new_complex ( liste_imputations_combofix,
								FALSE,
								TRUE,
								TRUE,
								0 );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[12],
		     2, 3, 2, 3,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[12]) -> entry),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(12) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[12]) -> arrow),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(12) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[12]) -> entry),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(12) );
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_COMBOFIX (widget_formulaire_operations[12]) -> entry ),
			      "focus_in_event",
			      GTK_SIGNAL_FUNC (entree_prend_focus),
			      GTK_OBJECT ( widget_formulaire_operations[12] ) );
  gtk_signal_connect ( GTK_OBJECT (GTK_COMBOFIX (widget_formulaire_operations[12]) -> entry),
		       "focus_out_event",
		       GTK_SIGNAL_FUNC (entree_perd_focus),
		       GINT_TO_POINTER (12) );

  gtk_widget_show (widget_formulaire_operations[12]);

  gtk_widget_set_sensitive ( widget_formulaire_operations[12],
			     etat.utilise_imputation_budgetaire );
  
  /* mise en place du type associé lors d'un virement */
  /* non affiché au départ et pas de menu au départ */

  widget_formulaire_operations[13] = gtk_option_menu_new ();
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			 widget_formulaire_operations[13],
			 _("Mode de règlement de la contre-opération"),
			 _("Mode de règlement de la contre-opération") );
  gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_operations[13] ),
		       "key_press_event",
		       GTK_SIGNAL_FUNC ( touches_champ_formulaire ),
		       GINT_TO_POINTER(13) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[13],
		     3, 5, 2, 3,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);


  /*   création de l'entrée du no de pièce comptable */

  widget_formulaire_operations[14] = gtk_entry_new();
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[14],
		     5, 7, 2, 3,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[14]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(14) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[14]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(14) );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[14]),
			     "focus_in_event",
			     GTK_SIGNAL_FUNC (entree_prend_focus),
			     NULL );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[14]),
			     "focus_out_event",
			     GTK_SIGNAL_FUNC (entree_perd_focus),
			     GINT_TO_POINTER (14) );
  gtk_widget_show ( widget_formulaire_operations[14] );

  gtk_widget_set_sensitive ( widget_formulaire_operations[14],
			     etat.utilise_piece_comptable );

  /* mise en forme du bouton ventilation */

  widget_formulaire_operations[15] = gtk_button_new_with_label ( _("Ventilation") );
  gtk_button_set_relief ( GTK_BUTTON ( widget_formulaire_operations[15] ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( widget_formulaire_operations[15] ),
   		       "clicked",
   		       GTK_SIGNAL_FUNC ( basculer_vers_ventilation ),
   		       NULL );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[15]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(15) );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[15],
		     0, 2, 3, 4,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show ( widget_formulaire_operations[15] );


  /*  Affiche les notes */

  widget_formulaire_operations[16] = gtk_entry_new ();
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[16],
		     2, 3, 3, 4,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[16]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(16) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[16]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(16) );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[16]),
			     "focus_in_event",
			     GTK_SIGNAL_FUNC (entree_prend_focus),
			     NULL );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[16]),
			     "focus_out_event",
			     GTK_SIGNAL_FUNC (entree_perd_focus),
			     GINT_TO_POINTER (16) );
  gtk_widget_show (widget_formulaire_operations[16]);



  /*  Affiche les infos banque/guichet */

  widget_formulaire_operations[17] = gtk_entry_new ();
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[17],
		     3, 6, 3, 4,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[17]),
 		       "button_press_event",
		       GTK_SIGNAL_FUNC (clique_champ_formulaire),
		       GINT_TO_POINTER(17) );
  gtk_signal_connect ( GTK_OBJECT (widget_formulaire_operations[17]),
 		       "key_press_event",
		       GTK_SIGNAL_FUNC (touches_champ_formulaire),
		       GINT_TO_POINTER(17) );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[17]),
			     "focus_in_event",
			     GTK_SIGNAL_FUNC (entree_prend_focus),
			     NULL );
  gtk_signal_connect_after ( GTK_OBJECT (widget_formulaire_operations[17]),
			     "focus_out_event",
			     GTK_SIGNAL_FUNC (entree_perd_focus),
			     GINT_TO_POINTER (17) );
  gtk_widget_show (widget_formulaire_operations[17]);

  gtk_widget_set_sensitive ( widget_formulaire_operations[17],
			     etat.utilise_info_banque_guichet );

  /*  Affiche le mode automatique / manuel  */

  widget_formulaire_operations[18] = gtk_label_new ( "" );
  gtk_table_attach ( GTK_TABLE (table),
		     widget_formulaire_operations[18],
		     6, 7, 3, 4,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0,0);
  gtk_widget_show (widget_formulaire_operations[18]);


  /* séparation d'avec les boutons */

  separateur_formulaire_operations = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( formulaire ),
		       separateur_formulaire_operations,
		       FALSE,
		       FALSE,
		       0 );
  if ( etat.affiche_boutons_valider_annuler )
    gtk_widget_show ( separateur_formulaire_operations );

  /* mise en place des boutons */

  hbox_valider_annuler_ope = gtk_hbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( formulaire ),
		       hbox_valider_annuler_ope,
		       FALSE,
		       FALSE,
		       0 );
  if ( etat.affiche_boutons_valider_annuler )
    gtk_widget_show ( hbox_valider_annuler_ope );

  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( echap_formulaire ),
		       NULL );
  gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ope ),
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
		       GTK_SIGNAL_FUNC ( fin_edition ),
		       NULL );
  gtk_box_pack_end ( GTK_BOX ( hbox_valider_annuler_ope ),
		     bouton,
		     FALSE,
		     FALSE,
		     0 );
  gtk_widget_show ( bouton );


  /* aucune opération associée au formulaire */

  gtk_object_set_data ( GTK_OBJECT ( table ),
			"adr_struct_ope",
			NULL );
  gtk_object_set_data ( GTK_OBJECT ( table ),
			"liste_adr_ventilation",
			NULL );

  return ( formulaire );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
void echap_formulaire ( void )
{
  GSList *liste_tmp;

  /* si c'est une nouvelle opé ventilée et qu'on a utilisé la complétion */
  /* il faut effacer les opés de ventilation automatiquement créées */
  /* celles ci sont dans la liste dans "liste_adr_ventilation" */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  liste_tmp = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				    "liste_adr_ventilation" );

  if ( liste_tmp
       &&
       !gtk_object_get_data ( GTK_OBJECT ( formulaire ),
			      "adr_struct_ope" )
       &&
       liste_tmp != GINT_TO_POINTER ( -1 ))
    {
      while ( liste_tmp )
	{
	  ligne_selectionnee_ventilation = liste_tmp -> data;
	  supprime_operation_ventilation ();
	  liste_tmp = liste_tmp -> next;
	}
    }

  formulaire_a_zero();

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
  gtk_widget_grab_focus ( CLIST_OPERATIONS );

  if ( !etat.formulaire_toujours_affiche )
    gtk_widget_hide ( frame_droite_bas );

}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction appelée quand une entry prend le focus */
/* si elle contient encore des éléments grisés, on les enlève */
/***********************************************************************************************************/

void entree_prend_focus ( GtkWidget *entree )
{

  /* si le style est le gris, on efface le contenu de l'entrée, sinon on fait rien */

  if ( GTK_IS_COMBOFIX ( entree ))
    {
      if ( gtk_widget_get_style ( GTK_COMBOFIX (entree)->entry ) == style_entree_formulaire[1] )
	{
	  gtk_combofix_set_text ( GTK_COMBOFIX ( entree ),
				  "" );
	  gtk_widget_set_style ( GTK_COMBOFIX (entree)->entry,
				 style_entree_formulaire[0] );
	}
    }
  else
    {
      if ( gtk_widget_get_style ( entree ) == style_entree_formulaire[1] )
	{
	  gtk_entry_set_text ( GTK_ENTRY ( entree ),
			       "" );
	  gtk_widget_set_style ( entree,
				 style_entree_formulaire[0] );
	}
    }
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction appelée quand une entry perd le focus */
/* si elle ne contient rien, on remet la fonction en gris */
/***********************************************************************************************************/

void entree_perd_focus ( GtkWidget *entree,
			 GdkEventFocus *ev,
			 gint *no_origine )
{
  gchar *texte;

  texte = NULL;
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  switch ( GPOINTER_TO_INT ( no_origine ))
    {
      /* on sort de la date, soit c'est vide, soit on la vérifie, la complète si nécessaire et met à jour l'exercice */
    case 1:
      if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	{
	  modifie_date ( entree );

	  /* 	  si c'est une modif d'opé, on ne change pas l'exercice */

	  if ( !gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				      "adr_struct_ope" ))
	    affiche_exercice_par_date( widget_formulaire_operations[1],
				       widget_formulaire_operations[11] );
	}
      else
	texte = _("Date");
      break;

      /*       on sort du tiers : soit vide soit complète le reste de l'opé */

    case 2:
      if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	completion_operation_par_tiers ();
      else
	texte = _("Tiers");
      break;

      /*       on sort du débit : soit vide, soit change le menu des types s'il ne correspond pas */

    case 3:

      if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	{
	  /* on  commence par virer ce qu'il y avait dans les crédits */

	  if ( gtk_widget_get_style ( widget_formulaire_operations[4] ) == style_entree_formulaire[0] )
	    {
	      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[4] ),
				   "" );
	      entree_perd_focus ( widget_formulaire_operations[4],
				  NULL,
				  GINT_TO_POINTER (4));
	    }

	  /* si c'est un menu de crédit, on y met le menu de débit, sauf si tous les types sont affichés */

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	  if ( ( !etat.affiche_tous_les_types
		 &&
		 GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] )
		 &&
		 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu ),
							 "signe_menu" ))
		 ==
		 2 )
	       ||
	       !GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] ))
	    {
	      /* on crée le nouveau menu et on met le défaut */

	      GtkWidget *menu;

	      if ( (menu = creation_menu_types ( 1, compte_courant, 0  )))
		{
		  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					     menu );
		  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
						cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
		  gtk_widget_show ( widget_formulaire_operations[9] );
		}
	      else
		{
		  gtk_widget_hide ( widget_formulaire_operations[9] );
		  gtk_widget_hide ( widget_formulaire_operations[10] );
		}

	      /* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
	      /* s'il est affiché */

	      if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[13] )
		   &&
		   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ) -> menu ),
							 "signe_menu" ))
		   ==
		   1 )
		{
		  GtkWidget *menu;

		  menu = creation_menu_types ( 2,
					       GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_operations[13] ),
										       "compte_virement" )),
					       2  );

		  if ( menu )
		    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ),
					       menu );
		  else
		    gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ));
		}
	    }
	  else
	    {
	      /* 	      on n'a pas recréé de menu, donc soit c'est déjà un menu de débit, soit tous les types */
	      /* sont affichés, soit le widget n'est pas visible */
	      /* on met donc le défaut, sauf si il y a qque chose dans les categ ou que le widget n'est pas visible */


	      if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] )
		   &&
		   gtk_widget_get_style ( widget_formulaire_operations[8] ) == style_entree_formulaire[1] )
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					      cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
	    }
	}
      else
	texte = _("Débit");
      break;

      /*       on sort du crédit : soit vide, soit change le menu des types s'il n'y a aucun tiers ( <=> nouveau tiers ) */

    case 4:
      if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	{
	  /* on  commence par virer ce qu'il y avait dans les débits */

	  if ( gtk_widget_get_style ( widget_formulaire_operations[3] ) == style_entree_formulaire[0] )
	    {
	      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[3] ),
				   "" );
	      entree_perd_focus ( widget_formulaire_operations[3],
				  NULL,
				  GINT_TO_POINTER (3));
	    }

	  /* si c'est un menu de crédit, on y met le menu de débit, sauf si tous les types sont affichés */

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	  if ( ( !etat.affiche_tous_les_types
		 &&
		 GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] )
		 &&
		 GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu ),
							 "signe_menu" ))
		 ==
		 1 )
	       ||
	       !GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] ))
	    {
	      /* on crée le nouveau menu et on met le défaut */

	      GtkWidget *menu;

	      if ( (menu = creation_menu_types ( 2, compte_courant, 0  )))
		{
		  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					     menu );
		  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
						cherche_no_menu_type ( TYPE_DEFAUT_CREDIT ) );
		  gtk_widget_show ( widget_formulaire_operations[9] );
		}
	      else
		{
		  gtk_widget_hide ( widget_formulaire_operations[9] );
		  gtk_widget_hide ( widget_formulaire_operations[10] );
		}

	      /* comme il y a eu un changement de signe, on change aussi le type de l'opé associée */
	      /* s'il est affiché */

	      if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[13] )
		   &&
		   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ) -> menu ),
							 "signe_menu" ))
		   ==
		   2 )
		{
		  GtkWidget *menu;

		  menu = creation_menu_types ( 1,
					       GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_operations[13] ),
										       "compte_virement" )),
					       2  );

		  if ( menu )
		    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ),
					       menu );
		  else
		    gtk_option_menu_remove_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ));
		}
	    }
	  else
	    {
	      /* 	      on n'a pas recréé de menu, donc soit c'est déjà un menu de débit, soit tous les types */
	      /* sont affichés, soit le widget n'est pas visible */
	      /* on met donc le défaut, sauf si il y a qque chose dans les categ ou que le widget n'est pas visible */


	      if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] )
		   &&
		   gtk_widget_get_style ( widget_formulaire_operations[8] ) == style_entree_formulaire[1] )
		gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					      cherche_no_menu_type ( TYPE_DEFAUT_CREDIT ) );
	    }
	}
      else
	texte = _("Crédit");
      break;

    case 7:
      if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	modifie_date ( entree );
      else
	texte = _("Date de valeur");
      break;


      /*       sort des catégories : si c'est une opé ventilée, affiche le bouton de ventilation */
      /* si c'est un virement affiche le bouton des types de l'autre compte */

    case 8:

      if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	{
	  if ( strcmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[8]))),
			_("Opération ventilée") ))
	    {
	      gchar **tableau_char;

	      gtk_widget_hide ( widget_formulaire_operations[15] );
	      gtk_widget_set_sensitive ( widget_formulaire_operations[11],
					 TRUE );
	      gtk_widget_set_sensitive ( widget_formulaire_operations[12],
					 TRUE );

	      /* vérification que ce n'est pas un virement */

	      tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[8])),
					  ":",
					  2 );

	      tableau_char[0] = g_strstrip ( tableau_char[0] );

	      if ( tableau_char[1] )
		tableau_char[1] = g_strstrip ( tableau_char[1] );


	      if ( strlen ( tableau_char[0] ) )
		{
		  if ( !strcmp ( tableau_char[0],
				 _("Virement") )
		       && tableau_char[1]
		       && strlen ( tableau_char[1]) )
		    {
		      /* c'est un virement : on recherche le compte associé et on affiche les types de paiement */

		      gint i;

		      if ( strcmp ( tableau_char[1],
				    _("Compte supprimé") ) )
			{
			  /* recherche le no de compte du virement */

			  gint compte_virement;

			  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

			  compte_virement = -1;

			  for ( i = 0 ; i < nb_comptes ; i++ )
			    {
			      if ( !g_strcasecmp ( NOM_DU_COMPTE,
						   tableau_char[1] ) )
				compte_virement = i;
			      p_tab_nom_de_compte_variable++;
			    }

			  /* si on a touvé un compte de virement, que celui ci n'est pas le compte */
			  /* courant et que son menu des types n'est pas encore affiché, on crée le menu */

			  if ( compte_virement != -1
			       &&
			       compte_virement != compte_courant )
			    {
			      /* si le menu affiché est déjà celui du compte de virement, on n'y touche pas */

			      if ( !GTK_WIDGET_VISIBLE ( widget_formulaire_operations[13] )
				   ||
				   ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( widget_formulaire_operations[13] ),
									     "no_compte" ))
				     !=
				     compte_virement ))
				{
				  /* vérifie quel est le montant entré, affiche les types opposés de l'autre compte */

				  GtkWidget *menu;

				  if ( gtk_widget_get_style ( widget_formulaire_operations[4] ) == style_entree_formulaire[0] )
				    /* il y a un montant dans le crédit */
				    menu = creation_menu_types ( 1, compte_virement, 2  );
				  else
				    /* il y a un montant dans le débit ou défaut */
				    menu = creation_menu_types ( 2, compte_virement, 2  );

				  /* si un menu à été créé, on l'affiche */

				  if ( menu )
				    {
				      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ),
								 menu );
				      gtk_widget_show ( widget_formulaire_operations[13] );
				    }
				  
				  /* on associe le no de compte de virement au formulaire pour le retrouver */
				  /* rapidement s'il y a un chgt débit/crédit */

				  gtk_object_set_data ( GTK_OBJECT ( widget_formulaire_operations[13] ),
							"compte_virement",
							GINT_TO_POINTER ( compte_virement ));
				}
			    }
			  else
			    gtk_widget_hide ( widget_formulaire_operations[13] );
			}
		      else
			gtk_widget_hide ( widget_formulaire_operations[13] );
		    }
		  else
		    gtk_widget_hide ( widget_formulaire_operations[13] );
		}
	      else
		gtk_widget_hide ( widget_formulaire_operations[13] );

	      g_strfreev ( tableau_char );
	    }
	  else
	    {
	      gtk_widget_show ( widget_formulaire_operations[15] );
	      gtk_widget_set_sensitive ( widget_formulaire_operations[11],
					 FALSE );
	      gtk_widget_set_sensitive ( widget_formulaire_operations[12],
					 FALSE );
	    }
	}
      else
	texte = _("Catégories : Sous-catégories");

      break;

    case 10:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("n° Chèque/Virement");
      break;

    case 12:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Imputation budgétaire");
      break;

    case 14:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Pièce comptable");

      break;

    case 15:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Notes");
      break;

    case 17:
      if ( !strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree )))))
	texte = _("Informations banque/guichet");
      break;

    default :
    }


  /* l'entrée était vide, on remet le défaut */
  /* si l'origine était un combofix, il faut remettre le texte */
  /* avec le gtk_combofix (sinon risque de complétion), donc utiliser l'origine */

  if ( texte )
    {
      switch ( GPOINTER_TO_INT ( no_origine ))
	{
	case 2:
	case 8:
	case 12:
	  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[GPOINTER_TO_INT ( no_origine )] ),
				  texte );
	  break;

	default:

	  gtk_entry_set_text ( GTK_ENTRY ( entree ),
			       texte );
	}
      gtk_widget_set_style ( entree,
			     style_entree_formulaire[1] );
    }

}
/***********************************************************************************************************/



/***********************************************************************************************************/
void clique_champ_formulaire ( GtkWidget *entree,
			       GdkEventButton *ev,
			       gint *no_origine )
{
  /* on rend sensitif tout ce qui ne l'était pas sur le formulaire */

  degrise_formulaire_operations ();

  /* si l'entrée de la date est grise, on met la date courante */
  /* seulement si la date réelle est grise aussi. Dans le cas contraire, c'est elle qui prend le focus */

  if ( (gtk_widget_get_style ( widget_formulaire_operations[1] ) == style_entree_formulaire[1])
       && (gtk_widget_get_style ( widget_formulaire_operations[7] ) == style_entree_formulaire[1]) )
    {
      entree_prend_focus ( widget_formulaire_operations[1] );

      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[1] ),
			   date_jour() );

      /* si le click est sur l'entrée de la date, on la sélectionne et elle prend le focus */

      if ( GPOINTER_TO_INT ( no_origine ) == 1 )
	{
	  if ( ev )
	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
					   "button_press_event");
	  gtk_entry_select_region ( GTK_ENTRY ( entree ),
				    0,
				    -1);
	  gtk_widget_grab_focus ( GTK_WIDGET ( entree ));
	}
    }
  else
    if (gtk_widget_get_style ( widget_formulaire_operations[1] ) == style_entree_formulaire[1])
      {
	entree_prend_focus ( widget_formulaire_operations[7] );
	/* si le click est sur l'entrée de la date, on la sélectionne et elle prend le focus */

	if ( GPOINTER_TO_INT ( no_origine ) == 1 )
	  {
	    if ( ev )
	      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entree ),
					     "button_press_event");
	    gtk_entry_select_region ( GTK_ENTRY ( entree ),
				      0,
				      -1);
	    gtk_widget_grab_focus ( GTK_WIDGET ( entree ));
	  }
      }

  /*   si ev est null ( cad que ça ne vient pas d'un click mais appelé par ex à la fin */
  /* de fin_edition ), on se barre */

  if ( !ev )
    return;

  /* énumération suivant l'entrée où on clique */

  switch ( GPOINTER_TO_INT ( no_origine ))
    {
    case 1:
      /* click sur l'entrée de la date */

      /* si double click, on popup le calendrier */

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
			       GTK_SIGNAL_FUNC ( date_selectionnee ),
			       popup );
	  gtk_signal_connect ( GTK_OBJECT ( popup ),
			       "key_press_event",
			       GTK_SIGNAL_FUNC ( touche_calendrier ),
			       NULL );
	  gtk_box_pack_start ( GTK_BOX ( popup_boxv ),
			       calendrier,
			       TRUE,
			       TRUE,
			       0 );
	  gtk_widget_show ( calendrier );


	  /* ajoute le bouton annuler */

	  bouton = gtk_button_new_with_label ( _("Annuler") );
	  gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
				      "clicked",
				      GTK_SIGNAL_FUNC ( ferme_calendrier ),
				      GTK_OBJECT ( popup ) );

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
	}
      break;


    case 7:
      /* click sur l'entrée de la date réelle*/

      /* si double click, on popup le calendrier */

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
 			       GTK_SIGNAL_FUNC ( date_bancaire_selectionnee ),
 			       popup );
 	  gtk_signal_connect ( GTK_OBJECT ( popup ),
 			       "key_press_event",
 			       GTK_SIGNAL_FUNC ( touche_calendrier ),
 			       NULL );
 	  gtk_box_pack_start ( GTK_BOX ( popup_boxv ),
 			       calendrier,
 			       TRUE,
 			       TRUE,
 			       0 );
 	  gtk_widget_show ( calendrier );


 	  /* ajoute le bouton annuler */

 	  bouton = gtk_button_new_with_label ( _("Annuler") );
 	  gtk_signal_connect_object ( GTK_OBJECT ( bouton ),
 				      "clicked",
 				      GTK_SIGNAL_FUNC ( ferme_calendrier ),
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
      break;


    default:
    }
}
/***********************************************************************************************************/



/***********************************************************************************************************/
void touches_champ_formulaire ( GtkWidget *widget,
				GdkEventKey *ev,
				gint *no_origine )
{
  gint origine;

  origine = GPOINTER_TO_INT ( no_origine );

  /*   si etat.entree = 1, la touche entrée finit l'opération ( fonction par défaut ) */
  /* sinon elle fait comme tab */

  if ( !etat.entree
       &&
       ( ev->keyval == 65293
	 ||
	 ev->keyval == 65421 ))
    ev->keyval = 65289;

  switch (ev->keyval)
    {
      /* échap */

    case 65307:

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
      gtk_widget_grab_focus ( CLIST_OPERATIONS );
      echap_formulaire();
      break;


      /*       tabulation */

    case 65289:

      /* une tabulation passe au widget affiché suivant */
      /* et retourne à la date ou enregistre l'opé s'il est à la fin */

      gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
				     "key_press_event");

      /* on efface la sélection en cours si c'est une entrée ou un combofix */

      if ( GTK_IS_ENTRY ( widget ))
	gtk_entry_select_region ( GTK_ENTRY ( widget ),
				  0,
				  0);
      else
	if ( GTK_IS_COMBOFIX ( widget ))
	  gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget ) -> entry ),
				    0,
				    0);

      /* on fait perdre le focus au widget courant pour faire les changements automatiques si nécessaire */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
      gtk_widget_grab_focus ( CLIST_OPERATIONS );

      /* on donne le focus au widget suivant */

      origine = (origine + 1 ) % 18;

      while ( !(GTK_WIDGET_VISIBLE ( widget_formulaire_operations[origine] )
		&&
		GTK_WIDGET_SENSITIVE ( widget_formulaire_operations[origine] )
		&&
		( GTK_IS_COMBOFIX (widget_formulaire_operations[origine] )
		  ||
		  GTK_IS_ENTRY ( widget_formulaire_operations[origine] )
		  ||
		  GTK_IS_BUTTON ( widget_formulaire_operations[origine] ) )))
	origine = (origine + 1 ) % 18;


      /*       si on se retrouve sur la date et que etat.entree = 0, on enregistre l'opérations */

      if ( origine == 1 && !etat.entree )
	{
	  fin_edition();
	  return;
	}

      /* si le prochain est le débit, on vérifie s'il n'y a rien dans cette entrée et s'il y a quelque chose dans l'entrée du crédit */

      if ( origine == 3 )
	{
	  /* si le débit est gris et le crédit est noir, on met sur le crédit */

	  if ( gtk_widget_get_style ( widget_formulaire_operations[origine] ) == style_entree_formulaire[1]
	       &&
	       gtk_widget_get_style ( widget_formulaire_operations[origine+1] ) == style_entree_formulaire[0] )
	    origine++;
	}

      /*       si le prochain est le crédit et que le débit a été rempli, on se met sur la devise et on efface le crédit */

      if ( origine == 4 )
	if ( gtk_widget_get_style ( widget_formulaire_operations[origine-1] ) == style_entree_formulaire[0] )
	  {
	    origine++;
	    gtk_widget_set_style (widget_formulaire_operations[4],
				  style_entree_formulaire[1] );
	    gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[4]),
				 _("Crédit") );
	  }

      /* on sélectionne le contenu de la nouvelle entrée */

      if ( GTK_IS_COMBOFIX ( widget_formulaire_operations[origine] ) )
	{
	  gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_operations[origine] ) -> entry );  
	  gtk_entry_select_region ( GTK_ENTRY ( GTK_COMBOFIX ( widget_formulaire_operations[origine] ) -> entry ),
				    0,
				    -1 );
	}
      else
	{
	  if ( GTK_IS_ENTRY ( widget_formulaire_operations[origine] ) )
	    gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_operations[origine] ),
				      0,
				      -1 );

	  gtk_widget_grab_focus ( widget_formulaire_operations[origine]  );
	}
      break;


      /* entree */
    case 65293 :
    case 65421 :
      gtk_signal_emit_stop_by_name ( GTK_OBJECT (widget),
				     "key_press_event");

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
      gtk_widget_grab_focus ( CLIST_OPERATIONS );

      fin_edition();
      break;

      /* touches + */

    case 65451:
    case 61:
      /*       si on est dans une entree de date, on augmente d'un jour la date */

      /* GDC : prise en compte de la date réelle (18) FinGDC */
      if ( origine == 1 || origine == 7)
	{
	  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					 "key_press_event");
	  incremente_decremente_date ( widget_formulaire_operations[origine],
				       1 );
	}
      break;

      /* touches - */

    case 65453:
    case 45:
      /*       si on est dans une entree de date, on diminue d'un jour la date */

      /* GDC : prise en compte de la date réelle (18) FinGDC */
      if ( origine == 1 || origine == 7)
	{
	  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					 "key_press_event");
	  incremente_decremente_date ( widget_formulaire_operations[origine],
				       -1 );
	}
      break;


    default:
    }

}
/***********************************************************************************************************/


/***********************************************************************************************************/
void ferme_calendrier ( GtkWidget *popup )
{
  gtk_widget_destroy ( popup );

  /*   magouille pour récupérer le focus, sinon le focus reste sur l'entrée */
  /* de la date tant qu'on n'a pas clické dessus */

  gtk_grab_remove ( window );
  gtk_grab_add ( window );

}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* Fonction touche_calendrier */
/* supprime le calendrier si on appuie sur échap */
/***********************************************************************************************************/

void touche_calendrier ( GtkWidget *popup,
			 GdkEventKey *ev,
			 gpointer null )
{
  if ( ev->keyval == 65307 )
    ferme_calendrier ( popup );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction date_selectionnee */
/* appelée lorsqu'on a clické 2 fois sur une date du calendrier */
/***********************************************************************************************************/

void date_selectionnee ( GtkCalendar *calendrier,
			 GtkWidget *popup )
{
  guint annee, mois, jour;

  gtk_calendar_get_date ( calendrier,
			  &annee,
			  &mois,
			  &jour);

  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[1] ),
		       g_strdup_printf ( "%02d/%02d/%d",
					 jour,
					 mois + 1,
					 annee));
  gtk_widget_destroy ( popup );

  gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_operations[2] ) -> entry );
	
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction date_bancaire_selectionnee */
/* appelée lorsqu'on a clické 2 fois sur une date du calendrier pour la date reelle */
/***********************************************************************************************************/

void date_bancaire_selectionnee ( GtkCalendar *calendrier, GtkWidget *popup )
{
  guint annee, mois, jour;

  gtk_calendar_get_date ( calendrier, &annee, &mois, &jour);

  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[7] ),
		       g_strdup_printf ( "%02d/%02d/%d", jour, mois + 1, annee));
  gtk_widget_destroy ( popup );

  gtk_widget_grab_focus ( GTK_COMBOFIX ( widget_formulaire_operations[2] ) -> entry );
}
/***********************************************************************************************************/






/*********************************************************************************************/
/* Fonction modifie_date */
/* prend en argument une entrée contenant une date */
/*  vérifie la validité et la modifie si seulement une partie est donnée */
/* met la date du jour si l'entrée est vide */
/* renvoie TRUE si la date est correcte */
/*********************************************************************************************/

gboolean modifie_date ( GtkWidget *entree )
{
  gchar *pointeur_entry;
  int jour, mois, annee;
  GDate *date;
  gchar **tab_date;

  /* si l'entrée est grise, on se barre */

  if (( gtk_widget_get_style ( entree ) == style_entree_formulaire[1] ))
    return ( FALSE );

  pointeur_entry = g_strstrip ( gtk_entry_get_text ( GTK_ENTRY (entree)) );

  if ( !strlen ( pointeur_entry ))
    {
      /*       si on est dans la conf des états, on ne met pas la date du jour, on */
      /* laisse vide */

      if ( entree != entree_date_init_etat
	   &&
	   entree != entree_date_finale_etat )
	gtk_entry_set_text ( GTK_ENTRY ( entree ),
			     date_jour() );
    }
  else
    {
      date = g_date_new ();
      g_date_set_time ( date, time(NULL));

      tab_date = g_strsplit ( pointeur_entry, "/", 3 );

      if ( tab_date[2] && tab_date[1] )
	{
	  /*       on a rentré les 3 chiffres de la date */

	  jour = strtod ( tab_date[0],  NULL );
	  mois = strtod ( tab_date[1], NULL );
	  annee = strtod ( tab_date[2], NULL );
 
	  if ( annee < 100 )
	    {
	      if ( annee < 80 ) annee = annee + 2000;
	      else annee = annee + 1900;
	    }
	}
      else
	if ( tab_date[1] )
	  {
	    /* 	on a rentré la date sous la forme xx/xx , il suffit de mettre l'année courante */

	    jour = strtod ( tab_date[0], NULL );
	    mois = strtod ( tab_date[1], NULL );
	    annee = g_date_year ( date );
	  }
	else
	  {
	    /* 	on a rentré que le jour de la date, il faut mettre le mois et l'année courante */
	    /* ou bien on a rentré la date sous forme jjmm ou jjmmaa ou jjmmaaaa */

	    gchar buffer[3];

	    switch ( strlen ( tab_date[0] ))
	      {
		/* 	      forme jj ou j */
	      case 1:
	      case 2:
		jour = strtod ( tab_date[0], 	NULL );
		mois = g_date_month ( date );
		annee = g_date_year ( date );
		break;

		/* form jjmm */

	      case 4 :
		buffer[0] = tab_date[0][0];
		buffer[1] = tab_date[0][1];
		buffer[2] = 0;

		jour = strtod ( buffer, NULL );
		mois = strtod ( tab_date[0] + 2, NULL );
		annee = g_date_year ( date );
		break;

		/* forme jjmmaa */

	      case 6:
		buffer[0] = tab_date[0][0];
		buffer[1] = tab_date[0][1];
		buffer[2] = 0;

		jour = strtod ( buffer, NULL );
		buffer[0] = tab_date[0][2];
		buffer[1] = tab_date[0][3];

		mois = strtod ( buffer, NULL );
		annee = strtod ( tab_date[0] + 4, NULL ) + 2000;
		break;

		/* forme jjmmaaaa */

	      case 8:
		buffer[0] = tab_date[0][0];
		buffer[1] = tab_date[0][1];
		buffer[2] = 0;

		jour = strtod ( buffer, NULL );
		buffer[0] = tab_date[0][2];
		buffer[1] = tab_date[0][3];

		mois = strtod ( buffer, NULL );
		annee = strtod ( tab_date[0] + 4, NULL );
		break;

	      default :
		jour = 0;
		mois = 0;
		annee = 0;
	      }
	  }
      g_strfreev ( tab_date );

      if ( g_date_valid_dmy ( jour, mois, annee) )
	gtk_entry_set_text ( GTK_ENTRY ( entree ),
			     g_strdup_printf ( "%02d/%02d/%d", jour, mois, annee ));
      else
	return ( FALSE );
    }
  return ( TRUE );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction completion_operation_par_tiers */
/* appelée lorsque le tiers perd le focus */
/* récupère le tiers, et recherche la dernière opé associée à ce tiers */
/* remplit le reste de l'opération avec les dernières données */
/***********************************************************************************************************/

void completion_operation_par_tiers ( void )
{
  GSList *liste_tmp;
  struct struct_tiers *tiers;
  struct structure_operation *operation;
  GSList *pointeur_ope;

  /* s'il y a quelque chose dans les entrées débit/crédit ou catégories, on se barre */

  if ( gtk_widget_get_style ( widget_formulaire_operations[3] ) == style_entree_formulaire[0]
       ||
       gtk_widget_get_style ( widget_formulaire_operations[4] ) == style_entree_formulaire[0]
       ||
       gtk_widget_get_style ( GTK_COMBOFIX (widget_formulaire_operations[8])->entry ) == style_entree_formulaire[0] )
    return;


  /* recherche le tiers demandé */

  liste_tmp = g_slist_find_custom ( liste_struct_tiers,
				    g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ))),
				    ( GCompareFunc ) recherche_tiers_par_nom );


  /*   si nouveau tiers,  on s'en va simplement */

  if ( !liste_tmp )
    return;

  tiers = liste_tmp -> data;

  /* on fait d'abord le tour du compte courant pour recherche une opé avec ce tiers */
  /* s'il n'y a aucune opé correspondante, on fait le tour de tous les comptes */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  operation = NULL;
  pointeur_ope = LISTE_OPERATIONS;

  while ( pointeur_ope )
    {
      struct structure_operation *ope_test;

      ope_test = pointeur_ope -> data;

      if ( ope_test -> tiers == tiers -> no_tiers
	   &&
	   !ope_test -> no_operation_ventilee_associee )
	{
	  if ( operation )
	    {
	      if ( g_date_compare ( ope_test -> date,
				    operation -> date ) >= 0 )
		operation = ope_test;
	    }
	  else
	    operation = ope_test;
	}

      pointeur_ope = pointeur_ope -> next;
    }


  if ( !operation )
    {
      /* aucune opération correspondant à ce tiers n'a été trouvée dans le compte courant */
      /*       on recherche dans les autres comptes, la première trouvée fera l'affaire */

      gint i;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  if ( i != compte_courant )
	    {
	      pointeur_ope = LISTE_OPERATIONS;

	      while ( pointeur_ope )
		{
		  struct structure_operation *ope_test;

		  ope_test = pointeur_ope -> data;

		  if ( ope_test -> tiers == tiers -> no_tiers )
		    {
		      operation = ope_test;
		      pointeur_ope = NULL;
		      i = nb_comptes;
		    }
		  else
		    pointeur_ope = pointeur_ope -> next;
		}
	    }
	  p_tab_nom_de_compte_variable++;
	}
    }

  /* si on n'a trouvé aucune opération, on se tire */

  if ( !operation )
    return;


  /* remplit les différentes entrées du formulaire */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;


  /* remplit les montant et place le menu correspondant dans l'option menu des types */

  if ( operation -> montant < 0 )
    {
      entree_prend_focus ( widget_formulaire_operations[3] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[3] ),
			   g_strdup_printf ( "%4.2f",
					     -operation -> montant ));

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
	    {
	      gtk_widget_hide ( widget_formulaire_operations[9] );
	      gtk_widget_hide ( widget_formulaire_operations[10] );
	    }
	}
    }
  else
    {
      entree_prend_focus ( widget_formulaire_operations[4] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[4] ),
			   g_strdup_printf ( "%4.2f",
					     operation -> montant ));

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

  /* met le type correspondant à l'opé */

  if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] ))
    {
      gint place_type;
      struct struct_type_ope *type;

      /* recherche le type de l'opé */

      place_type = cherche_no_menu_type ( operation -> type_ope );

      /* si aucun type n'a été trouvé, on cherche le défaut */

      if ( place_type == -1 )
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	  if ( operation -> montant < 0 )
	    place_type = cherche_no_menu_type ( TYPE_DEFAUT_DEBIT );
	  else
	    place_type = cherche_no_menu_type ( TYPE_DEFAUT_CREDIT );

	  /* si le type par défaut n'est pas trouvé, on met 0 */

	  if ( place_type == -1 )
	    place_type = 0;
	}

      /*       à ce niveau, place type est mis */

      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
				    place_type );

      /* récupère l'adr du type pour mettre un n° auto si nécessaire */

      type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
				   "adr_type" );
      
      if ( type -> affiche_entree )
	{
	  /* on réaffiche l'entrée car si le menu avait été mis sur le 0, c'était pas affiché */
	  
	  gtk_widget_show ( widget_formulaire_operations[10] );

	  if ( type -> numerotation_auto )
	    {
	      entree_prend_focus ( widget_formulaire_operations[10] );
	      gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[10] ),
				   itoa ( type -> no_en_cours + 1 ));
	    }
	  else
	    if ( operation -> contenu_type )
	      {
		entree_prend_focus ( widget_formulaire_operations[10] );
		gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[10] ),
				     operation -> contenu_type );
	      }
	}
    }


  /* met la devise */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( operation -> devise ),
									 ( GCompareFunc ) recherche_devise_par_no )));
  /* mise en forme des catégories */

  if ( operation -> operation_ventilee )
    {
      GtkWidget *dialog;
      GtkWidget *label;
      gint result;

      entree_prend_focus ( widget_formulaire_operations[8] );

      /* c'est une opé ventilée, la catégorie est Opération ventilée */
      /* on demande si on veut récupérer les anciennes opés de ventilation */

      gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
			      _("Opération ventilée") );
      gtk_widget_show ( widget_formulaire_operations[15] );

      /* affiche la question de récupération */

      dialog = gnome_dialog_new ( _("Récupération des ventilations ?"),
				  GNOME_STOCK_BUTTON_YES,
				  GNOME_STOCK_BUTTON_NO,
				  NULL );
      gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				     GTK_WINDOW ( window ));

      label = gtk_label_new ( _("Voulez-vous récupérer aussi les opérations de la dernière ventilation ?") );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   5 );
      gtk_widget_show ( label );

      result = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));

      if ( !result )
	{
	  /* récupération des anciennes opés de la ventilation */

	  GSList *liste_des_opes_de_ventilation;

	  liste_des_opes_de_ventilation = NULL;
	  pointeur_ope = LISTE_OPERATIONS;

	  while ( pointeur_ope )
	    {
	      struct structure_operation *ope_test;

	      ope_test = pointeur_ope -> data;

	      if ( ope_test -> no_operation_ventilee_associee == operation -> no_operation )
		{
		  struct struct_ope_ventil *nouvelle_operation;

		  nouvelle_operation = calloc ( 1,
						sizeof ( struct struct_ope_ventil ));

		  nouvelle_operation -> montant = ope_test -> montant;
		  nouvelle_operation -> categorie = ope_test -> categorie;
		  nouvelle_operation -> sous_categorie = ope_test -> sous_categorie;

		  if ( ope_test -> notes )
		    nouvelle_operation -> notes = g_strdup ( ope_test -> notes );

		  nouvelle_operation -> imputation = ope_test -> imputation;
		  nouvelle_operation -> sous_imputation = ope_test -> sous_imputation;

		  if ( ope_test -> no_piece_comptable )
		    nouvelle_operation -> no_piece_comptable = g_strdup ( ope_test -> no_piece_comptable );


		  /* si c'est un virement, on met le compte de virement et le type choisi */

		  if ( ope_test -> relation_no_operation )
		    {
		      struct structure_operation *contre_operation;

		      nouvelle_operation -> relation_no_operation = -1;
		      nouvelle_operation -> relation_no_compte = ope_test -> relation_no_compte;

		      /*  on va chercher le type de l'opé associée */

		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ope_test -> relation_no_compte;

		      contre_operation = g_slist_find_custom ( LISTE_OPERATIONS,
							       GINT_TO_POINTER ( ope_test -> relation_no_operation ),
							       (GCompareFunc) recherche_operation_par_no ) -> data;

		      nouvelle_operation -> no_type_associe = contre_operation -> type_ope;

		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
		    }

		  liste_des_opes_de_ventilation = g_slist_append ( liste_des_opes_de_ventilation,
								   nouvelle_operation );
		}
	      pointeur_ope = pointeur_ope -> next;
	    }

	  if ( liste_des_opes_de_ventilation )
	    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
				  "liste_adr_ventilation",
				  liste_des_opes_de_ventilation );
	  else
	    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
				  "liste_adr_ventilation",
				  GINT_TO_POINTER ( -1 ) );
	}
    }
  else
    if ( operation -> relation_no_operation )
      {
	/* c'est un virement */

	GtkWidget *menu;

	entree_prend_focus ( widget_formulaire_operations[8] );

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	if ( operation -> relation_no_operation != -1 )
	  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
				  g_strconcat ( COLON(_("Virement")),
						NOM_DU_COMPTE,
						NULL ));
	else
	  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
				  "Virement" );


	/* comme c'est un virement, on va afficher le type de la contre opé s'il existe */

	  if ( operation -> montant >= 0 )
	    menu = creation_menu_types ( 1, operation -> relation_no_compte, 2  );
	  else
	    menu = creation_menu_types ( 2, operation -> relation_no_compte, 2  );

	  /*  on ne continue que si un menu a été créé */
	  /*    dans ce cas, on va chercher l'autre opé et retrouve le type */

	  if ( menu )
	    {
	      struct structure_operation *operation_2;

	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ),
					 menu );
	      gtk_widget_show ( widget_formulaire_operations[13] );

	      operation_2 = g_slist_find_custom ( LISTE_OPERATIONS,
						  GINT_TO_POINTER ( operation -> relation_no_operation ),
						  (GCompareFunc) recherche_operation_par_no ) -> data;
	      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ),
					    cherche_no_menu_type_associe ( operation_2 -> type_ope,
									   0 ));
	    }
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
      }
    else
      {
	/* c'est des catégories normales */

	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( operation -> categorie ),
					  ( GCompareFunc ) recherche_categorie_par_no );

	if ( liste_tmp )
	  {
	    GSList *liste_tmp_2;

	    entree_prend_focus ( widget_formulaire_operations[8]);

	    liste_tmp_2 = g_slist_find_custom ( (( struct struct_categ * )( liste_tmp -> data )) -> liste_sous_categ,
						GINT_TO_POINTER ( operation -> sous_categorie ),
						( GCompareFunc ) recherche_sous_categorie_par_no );
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
      }


  /* met en place l'imputation budgétaire */


  liste_tmp = g_slist_find_custom ( liste_struct_imputation,
				    GINT_TO_POINTER ( operation -> imputation ),
				    ( GCompareFunc ) recherche_imputation_par_no );

  if ( liste_tmp )
    {
      GSList *liste_tmp_2;

      entree_prend_focus ( widget_formulaire_operations[12]);

      liste_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( liste_tmp -> data )) -> liste_sous_imputation,
					  GINT_TO_POINTER ( operation -> sous_imputation ),
					  ( GCompareFunc ) recherche_sous_categorie_par_no );
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
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* procédure appelée lors de la fin de l'édition d'opération */
/* crée une nouvelle opération à partir du formulaire */
/* puis appelle ajout_operation pour la sauver */
/***********************************************************************************************************/

void fin_edition ( void )
{
  struct structure_operation *operation;
  gint modification;
  GSList *liste_no_tiers;
  GSList *liste_tmp;

  /*   récupération de l'opération : soit l'adr de la struct, soit NULL si nouvelle */

  operation = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				    "adr_struct_ope" );

  if ( operation )
    modification = 1;
  else
    modification = 0;

  /* on commence par vérifier que les données entrées sont correctes */
  /*   si la fonction renvoie false, c'est qu'on doit arrêter là */

  if ( !verification_validation_operation ( operation ))
    return;


  /*   si le tiers est un état, on va faire autant d'opérations qu'il y a de tiers dans */
  /* l'état concerné */
  /* on va créer une liste avec les nos de tiers ( ou -1  */
  /* le tiers n'est pas un état), puis on fera une boucle sur cette liste pour ajouter autant d'opérations */
  /* que de tiers */

  liste_no_tiers = NULL;

  if ( strncmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ))),
		 COLON(_("État")),
		 7 ))
    /*     ce n'est pas un état, on met -1 comme no de tiers */
    liste_no_tiers = g_slist_append (liste_no_tiers,
				     GINT_TO_POINTER ( -1 ));
  else
    {
      /* c'est bien un état */
      /* on commence par retrouver le nom de l'état */
      /* toutes les vérifications ont été faites précédemment */

      gchar **tableau_char;
      struct struct_etat *etat;
      GSList *liste_opes_selectionnees;

      tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ))),
				  ":",
				  2 );

      tableau_char[1] = g_strstrip ( tableau_char[1] );
      liste_tmp = liste_struct_etats;
      etat = NULL;

      while ( liste_tmp )
	{
	  etat = liste_tmp -> data;

	  if ( !strcmp ( etat -> nom_etat,
			 tableau_char[1] ))
	    liste_tmp = NULL;
	  else
	    liste_tmp = liste_tmp -> next;
	}

      g_strfreev ( tableau_char );

      /*       à ce niveau, etat contient l'adr le la struct de l'état choisi */

      liste_opes_selectionnees = recupere_opes_etat ( etat );

      liste_tmp = liste_opes_selectionnees;

      while ( liste_tmp )
	{
	  struct structure_operation *operation;

	  operation = liste_tmp -> data;

	  if ( !g_slist_find ( liste_no_tiers,
			       GINT_TO_POINTER ( operation -> tiers )))
	    liste_no_tiers = g_slist_append ( liste_no_tiers,
					      GINT_TO_POINTER ( operation -> tiers ));

	  liste_tmp = liste_tmp -> next;
	}

      g_slist_free ( liste_opes_selectionnees );
    }


  /*   à ce niveau, liste_no_tiers contient la liste des no de tiers pour chacun desquels on */
  /*     fera une opé, ou -1 si on utilise que le tiers dans l'entrée du formulaire */
  /* on fait donc le tour de cette liste en ajoutant l'opé à chaque fois */

  liste_tmp = liste_no_tiers;

  while ( liste_tmp )
    {
      /*       soit on va chercher le tiers dans la liste des no de tiers et on le met dans le formulaire, */
      /* soit on laisse tel quel et on met liste_tmp à NULL */

      if ( liste_tmp -> data == GINT_TO_POINTER ( -1 ))
	liste_tmp = NULL;
      else if ( liste_tmp -> data == NULL )
	{
	  dialogue ( _("Erreur: cet état ne comporte pas de tiers sélectionnés."));
	  return;
	}
      else
	{
	  struct struct_tiers *tiers;

	  tiers = g_slist_find_custom ( liste_struct_tiers,
					liste_tmp -> data,
					(GCompareFunc) recherche_tiers_par_no ) -> data;

	  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ),
				  tiers -> nom_tiers );

	  /* si le moyen de paiement est à incrémentation automatique, à partir de la 2ème opé, */
	  /* on incrémente le contenu (no de chèque en général) */

	  /* comme c'est une liste de tiers, c'est forcemment une nouvelle opé */
	  /* donc si operation n'est pas nul, c'est qu'on n'est pas sur la 1ère */
	  /* donc on peut incrémenter si nécessaire le contenu */
	  /* et on peut récupérer le no_type de l'ancienne opé */

	  if ( operation )
	    {
	      GSList *pointeur_tmp;

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	      pointeur_tmp = g_slist_find_custom ( TYPES_OPES,
						   GINT_TO_POINTER ( operation -> type_ope ),
						   (GCompareFunc) recherche_type_ope_par_no );

	      if ( pointeur_tmp )
		{
		  struct struct_type_ope *type;

		  type = pointeur_tmp -> data;

		  if ( type -> affiche_entree
		       &&
		       type -> numerotation_auto )
		    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[10] ),
					 itoa ( type -> no_en_cours + 1 ));
		}
	    }


	  liste_tmp = liste_tmp -> next;
	}


      /* si c'est une nouvelle opé, on la crée en mettant tout à 0 sauf le no de compte */

      if ( !modification )
	{
	  operation = calloc ( 1,
			       sizeof ( struct structure_operation ) );
	  operation -> no_compte = compte_courant;
	}


      /* on récupère les données du formulaire sauf la categ qui est traitée plus tard */

      recuperation_donnees_generales_formulaire ( operation );

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

      /* il faut ajouter l'opération à la liste à ce niveau pour lui attribuer un numéro */

      /* celui ci sera utilisé si c'est un virement ou si c'est une ventil qui contient des */
      /* virements */

      ajout_operation ( operation );


      /*   récupération des catégories / sous-catég, s'ils n'existent pas, on les crée */
      /* à mettre en dernier car si c'est une opé ventilée, chaque opé de ventil va récupérer les données du dessus */

      recuperation_categorie_formulaire ( operation,
					  modification );
    }



  /* on libère la liste des no tiers */

  g_slist_free ( liste_no_tiers );


  /* si on est en train d'équilibrer => recalcule le total pointé */

  if ( etat.equilibrage )
    {
      calcule_total_pointe_compte ( compte_courant );

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


  /* si c'était une nouvelle opération, on efface le formulaire, on remet la date pour la suivante, */
  /* si c'était une modification, on redonne le focus à la liste */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

  if ( gtk_object_get_data ( GTK_OBJECT ( formulaire ),
			     "adr_struct_ope" ) )
    {
      gtk_widget_grab_focus ( GTK_WIDGET ( CLIST_OPERATIONS ));

      formulaire_a_zero ();

      if ( !etat.formulaire_toujours_affiche )
	gtk_widget_hide ( frame_droite_bas );
    }
  else
    {
      formulaire_a_zero ();

      clique_champ_formulaire ( widget_formulaire_operations[1],
				NULL,
				GINT_TO_POINTER ( 1 ));
      gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_operations[1]),
				0,
				-1);
      gtk_widget_grab_focus ( GTK_WIDGET ( widget_formulaire_operations[1] ));

    }

  /* on réaffiche la liste courante pour afficher les categ qui ont été ajoutée après */
  /* l'ajout d'opération */

  MISE_A_JOUR = 1;
  verification_mise_a_jour_liste ();

  /*   met à jour les listes ( nouvelle opération associée au tiers et à la catégorie ) */

  mise_a_jour_tiers ();
  mise_a_jour_categ ();
  mise_a_jour_imputation ();

  modification_fichier ( TRUE );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* vérifie que les données dans le formulaire sont valides pour enregistrer l'opération */
/* appelée lors de la fin_edition d'une opé */
/***********************************************************************************************************/

gint verification_validation_operation ( struct structure_operation *operation )
{
  gchar **tableau_char;
  GSList *liste_tmp;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  /* on vérifie qu'il y a bien une date */

  if ( gtk_widget_get_style ( widget_formulaire_operations[1] ) != style_entree_formulaire[0] )
    {
      dialogue ( PRESPACIFY(_("Erreur : il faut obligatoirement entrer une date.")));
      return (FALSE);
    }

  /* vérifie que la date est correcte */

  if ( !modifie_date ( widget_formulaire_operations[1] ))
    {
      dialogue ( PRESPACIFY(_("Erreur : La date est invalide")) );
      gtk_widget_grab_focus ( widget_formulaire_operations[1] );
      gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_operations[1]),
				0,
				-1);
      return (FALSE);
    }

  /*   la date est correcte, il faut l'enregistrer dans date_courante */

  strncpy ( date_courante,
	    gtk_entry_get_text ( GTK_ENTRY (  widget_formulaire_operations[1])),
	    10 );

  /* vérifie que la date de valeur est correcte */

  if ( gtk_widget_get_style ( widget_formulaire_operations[7] ) == style_entree_formulaire[0]
       &&
       !modifie_date ( widget_formulaire_operations[7] ))
    {
      dialogue ( PRESPACIFY(_("Erreur : La date de valeur est invalide")) );
      gtk_widget_grab_focus ( widget_formulaire_operations[7] );
      gtk_entry_select_region ( GTK_ENTRY (  widget_formulaire_operations[7]),
 				0,
				-1);
      return (FALSE);
    }

  /* vérification que ce n'est pas un virement sur lui-même */
  /* et que le compte de virement existe */

  if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_operations[8] ) -> entry ) == style_entree_formulaire[0] )
    {
      tableau_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] )),
				  ":",
				  2 );
      
      tableau_char[0] = g_strstrip ( tableau_char[0] );

      if ( tableau_char[1] )
	tableau_char[1] = g_strstrip ( tableau_char[1] );


      if ( strlen ( tableau_char[0] ) )
	{
	  if ( !strcmp ( tableau_char[0],
			 _("Virement") )
	       && tableau_char[1]
	       && strlen ( tableau_char[1]) )
	    {
	      /* c'est un virement, on fait les vérifications */
	      /* si c'est un virement vers un compte supprimé, laisse passer */

	      if ( strcmp ( tableau_char[1],
			    _("Compte supprimé") ) )
		{
		  /* recherche le no de compte du virement */

		  gint compte_virement;
		  gint i;

		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

		  compte_virement = -1;

		  for ( i = 0 ; i < nb_comptes ; i++ )
		    {
		      if ( !g_strcasecmp ( NOM_DU_COMPTE,
					   tableau_char[1] ) )
			compte_virement = i;
		      p_tab_nom_de_compte_variable++;
		    }

		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

		  if ( compte_virement == -1 )
		    {
		      dialogue ( _("Erreur : le compte associé au virement est invalide") );
		      return (FALSE);
		    }

		  if ( compte_virement == compte_courant )
		    {
		      dialogue ( PRESPACIFY(_("Erreur : impossibilité de virer un compte   \n    sur lui-même")));
		      return (FALSE);
		    }
		}
	    }
	}
      g_strfreev ( tableau_char );
    }


  /*   pour les types qui sont à incrémentation automatique ( surtout les chèques ) */
  /* on fait le tour des operations pour voir si le no n'a pas déjà été utilisé */
  /* si operation n'est pas nul, c'est une modif donc on ne fait pas ce test */

  if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[10] ))
    {
      struct struct_type_ope *type;

      type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
				   "adr_type" );

      if ( type -> numerotation_auto )
	{
	  gchar *no_cheque;

	  /* vérifie s'il y a quelque chose */

	  if ( gtk_widget_get_style ( widget_formulaire_operations[10] ) == style_entree_formulaire[1] )
	    {
	      if ( question ( _("Le mode de règlement choisi est à numérotation automatique mais ne contient aucun numéro.\nSouhaitez-vous continuer ?") ) )
		goto sort_test_cheques;
	      else
		return (FALSE);
	    }

	  /* vérifie si le no de chèque n'est pas déjà utilisé */

	  if ( gtk_widget_get_style ( widget_formulaire_operations[10] ) == style_entree_formulaire[0] )
	    {
	      liste_tmp = LISTE_OPERATIONS;
	      no_cheque = g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[10] )));

	      while ( liste_tmp )
		{
		  struct structure_operation *operation_tmp;

		  operation_tmp = liste_tmp -> data;

		  if ( operation_tmp -> type_ope == type -> no_type
		       &&
		       operation_tmp -> contenu_type
		       &&
		       !strcmp ( operation_tmp -> contenu_type,
				 no_cheque ))
		    {
		      /*  on a trouvé le même no de chèque, si c'est une nouvelle opé, c'est pas normal, */
		      /* si c'est une modif d'opé, c'est normal que si c'est cette opé qu'on a trouvé */

		      if ( !operation
			   ||
			   operation_tmp -> no_operation != operation -> no_operation )
			{
			  if ( question ( _("Attention, le numéro du chèque est déjà utilisé.\nSouhaitez-vous continuer ?") ))
			    goto sort_test_cheques;
			  else
			    return (FALSE);
			}
		    }
		  liste_tmp = liste_tmp -> next;
		}
	    }
	}
    }

  sort_test_cheques :


    /*   vérifie tout de suite si c'est une opération ventilée, */
    /*si c'est le cas, si la liste des ventilation existe (soit adr de liste, soit -1), on va l'enregistrer plus tard */
    /* sinon on va ventiler tout de suite */

    if ( !strcmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ))),
		   _("Opération ventilée") )
	 &&
	 !gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				"liste_adr_ventilation" ))
      {
	enregistre_ope_au_retour = 1;

	if ( gtk_widget_get_style ( widget_formulaire_operations[3] ) == style_entree_formulaire[0] )
	  ventiler_operation ( -g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[3] ))),
					   NULL ));
	else
	  ventiler_operation ( g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[4] ))),
					  NULL ));

	return (FALSE);
      }

  /*   on vérifie si le tiers est un état, que c'est une nouvelle opérations */

  if ( !strncmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ))),
		  COLON(_("État")),
		  7 ))
    {
      gint trouve;

      /* on vérifie d'abord si c'est une modif d'opé */

      if ( operation )
	{
	  dialogue ( _("Erreur : Une opération dont le tiers est un état doit forcemment être une nouvelle opération.") );
	  return (FALSE);
	}

      /* on vérifie maintenant si l'état existe */

      tableau_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ))),
				  ":",
				  2 );

      tableau_char[1] = g_strstrip ( tableau_char[1] );
      liste_tmp = liste_struct_etats;
      trouve = 0;

      while ( liste_tmp )
	{
	  struct struct_etat *etat;

	  etat = liste_tmp -> data;

	  if ( !strcmp ( etat -> nom_etat,
			 tableau_char[1] ))
	    {
	      trouve = 1;
	      liste_tmp = NULL;
	    }
	  else
	    liste_tmp = liste_tmp -> next;
	}

      g_strfreev ( tableau_char );

      if ( !trouve )
	{
	  dialogue ( _("Erreur : Le nom de l'état dans le tiers est invalide.") );
	  return (FALSE);
	}
    }

  return ( TRUE );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* cette fonction récupère les données du formulaire et les met dans l'opération en argument */
/* sauf la catég */
/***********************************************************************************************************/

void recuperation_donnees_generales_formulaire ( struct structure_operation *operation )
{
  gchar *pointeur_char;
  GSList *pointeur_liste;
  gchar **tableau_char;
  struct struct_devise *devise;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  /* traitement de la date */

  pointeur_char = g_strstrip ( gtk_entry_get_text ( GTK_ENTRY (widget_formulaire_operations[1])));

  tableau_char = g_strsplit ( pointeur_char,
			      "/",
			      3 );

  operation -> jour = strtod ( tableau_char[0],
			       NULL );
  operation -> mois = strtod ( tableau_char[1],
			       NULL );
  operation -> annee = strtod (tableau_char[2],
			       NULL );

  operation->date = g_date_new_dmy ( operation->jour,
				     operation->mois,
				     operation->annee);

  /* traitement de la date de valeur */

  if ( etat.affiche_date_bancaire
       &&
       gtk_widget_get_style ( widget_formulaire_operations[7] ) == style_entree_formulaire[0] )
    {
      pointeur_char = g_strstrip ( gtk_entry_get_text ( GTK_ENTRY (widget_formulaire_operations[7])));

      tableau_char = g_strsplit ( pointeur_char,
				  "/",
				  3 );

      operation -> jour_bancaire = strtod ( tableau_char[0],
					    NULL );
      operation -> mois_bancaire = strtod ( tableau_char[1],
					    NULL );
      operation -> annee_bancaire = strtod (tableau_char[2],
					    NULL );

      operation->date_bancaire = g_date_new_dmy ( operation->jour_bancaire,
						  operation->mois_bancaire,
						  operation->annee_bancaire);
    }
  else
    {
      operation -> jour_bancaire = 0;
      operation -> mois_bancaire = 0;
      operation -> annee_bancaire = 0;

      operation->date_bancaire = NULL;
    }

  /* récupération du tiers, s'il n'existe pas, on le crée */

  if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_operations[2] ) -> entry ) == style_entree_formulaire[0] )
    {
      pointeur_char = g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] )));

      if ( ( pointeur_liste = g_slist_find_custom ( liste_struct_tiers,
						    pointeur_char,
						    ( GCompareFunc ) recherche_tiers_par_nom )) )
	operation -> tiers = (( struct struct_tiers * )( pointeur_liste -> data )) -> no_tiers;
      else
	operation -> tiers = (( struct struct_tiers * )( ajoute_nouveau_tiers ( pointeur_char ))) -> no_tiers;
    }
  else
    operation -> tiers = 0;



  /* récupération du montant */

  if ( gtk_widget_get_style ( widget_formulaire_operations[3] ) == style_entree_formulaire[0] )
    operation -> montant = -g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[3] ))),
				       NULL );
  else
    operation -> montant = g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[4] ))),
				      NULL );

  /* récupération de la devise */
 
  devise = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ) -> menu_item ),
				 "adr_devise" );


  /* récupération de la devise */
 
  devise = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ) -> menu_item ),
				 "adr_devise" );


  /* si c'est la devise du compte ou */
  /* si c'est un compte qui doit passer à l'euro ( la transfo se fait au niveau de l'affichage de la liste ) */
  /* ou si c'est un compte en euro et l'opé est dans une devise qui doit passer à l'euro -> pas de change à demander */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  if ( !devise_compte
       ||
       devise_compte -> no_devise != DEVISE )
    devise_compte = g_slist_find_custom ( liste_struct_devises,
					  GINT_TO_POINTER ( DEVISE ),
					  ( GCompareFunc ) recherche_devise_par_no ) -> data;

  operation -> devise = devise -> no_devise;

  if ( !( operation -> no_operation
	  ||
	  devise -> no_devise == DEVISE
	  ||
	  ( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	  ||
	  ( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
    {
      /* c'est une devise étrangère, on demande le taux de change et les frais de change */

      demande_taux_de_change ( devise_compte,
			       devise,
			       1,
			       (gdouble ) 0,
			       (gdouble ) 0 );

      operation -> taux_change = taux_de_change[0];
      operation -> frais_change = taux_de_change[1];

      if ( operation -> taux_change < 0 )
	{
	  operation -> taux_change = -operation -> taux_change;
	  operation -> une_devise_compte_egale_x_devise_ope = 1;
	}
    }


  /* récupération du type d'opération */

  if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] ))
    {
      operation -> type_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
								      "no_type" ));
      if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[10] )
	   &&
	   gtk_widget_get_style ( widget_formulaire_operations[10] ) == style_entree_formulaire[0] )
	{
	  struct struct_type_ope *type;

	  type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
				       "adr_type" );

	  operation -> contenu_type = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[10] ))));

	  if ( type -> numerotation_auto )
	    type -> no_en_cours = ( atoi ( operation -> contenu_type ));
	}
      else
	operation -> contenu_type = NULL;
    }
  else
    {
      operation -> type_ope = 0;
      operation -> contenu_type = NULL;
    }


  /* récupération du no d'exercice */
  /* si l'exo est à -1, c'est que c'est sur non affiché */
  /* soit c'est une modif d'opé et on touche pas à l'exo */
  /* soit c'est une nouvelle opé et on met l'exo à 0 */

  if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[11] ) -> menu_item ),
					       "no_exercice" )) == -1 )
    {
      if ( !operation -> no_operation )
	operation -> no_exercice = 0;
    }
  else
    operation -> no_exercice = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[11] ) -> menu_item ),
								       "no_exercice" ));


  /* récupération de l'imputation budgétaire */
  /* si c'est une opé ventilée, on ne récupère pas l'ib */

  if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_operations[12] ) -> entry ) == style_entree_formulaire[0]
       &&
       !GTK_WIDGET_VISIBLE ( widget_formulaire_operations[15] ))
    {
      struct struct_imputation *imputation;
      
      pointeur_char = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[12] ));

      tableau_char = g_strsplit ( pointeur_char,
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
	    sous_imputation = ajoute_nouvelle_sous_imputation ( tableau_char[1],
								imputation );
		  
	  operation -> sous_imputation = sous_imputation -> no_sous_imputation;
	}
      else
	operation -> sous_imputation = 0;

      g_strfreev ( tableau_char );
    }
  else
    {
      operation -> imputation = 0;
      operation -> sous_imputation = 0;
    }

  /* récupération du no de pièce comptable */

  if ( gtk_widget_get_style ( widget_formulaire_operations[14] ) == style_entree_formulaire[0] )
    operation -> no_piece_comptable = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[14] ))));
  else
    operation -> no_piece_comptable = NULL;


  /* récupération des notes */

  if ( gtk_widget_get_style ( widget_formulaire_operations[16] ) == style_entree_formulaire[0] )
    operation -> notes = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[16] ))));
  else
    operation -> notes = NULL;

  /* récupération de l'info banque/guichet */

  if ( gtk_widget_get_style ( widget_formulaire_operations[17] ) == style_entree_formulaire[0] )
    operation -> info_banque_guichet = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[17] ))));
  else
    operation -> info_banque_guichet = NULL;



}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* récupération des categ du formulaire, crée la contre opération si nécessaire ou remplit les opés */
/* de ventil */
/***********************************************************************************************************/

void recuperation_categorie_formulaire ( struct structure_operation *operation,
					 gint modification )
{
  gchar *pointeur_char;
  GSList *pointeur_liste;
  gchar **tableau_char;
  struct structure_operation *operation_2;

  if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget_formulaire_operations[8] ) -> entry ) == style_entree_formulaire[0] )
    {
      struct struct_categ *categ;
      
      pointeur_char = gtk_combofix_get_text ( GTK_COMBOFIX ( widget_formulaire_operations[8] ));

      /* récupération de la ventilation si nécessaire */

      if ( !strcmp ( g_strstrip ( pointeur_char ),
		     _("Opération ventilée") ))
	{
	  /* c'est une opé ventilée, on va appeler la fonction validation_ope_de_ventilation */
	  /* qui va créer les nouvelles opé, les contre-opérations et faire toutes les */
	  /* suppressions nécessaires */

	  /*  auparavant, si c'est une modif d'opé et que l'ancienne opé était un virement, on  */
	  /* vire l'ancienne opé associée */

	  if ( modification
	       &&
	       operation -> relation_no_operation )
	    {
	      /* c'était un virement, et ce ne l'est plus, donc on efface l'opé en relation */

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	      operation_2 = g_slist_find_custom ( LISTE_OPERATIONS,
						  GINT_TO_POINTER ( operation -> relation_no_operation ),
						  ( GCompareFunc ) recherche_operation_par_no ) -> data;
	  
	      operation_2 -> relation_no_operation = 0;
	      MISE_A_JOUR = 1;

	      supprime_operation ( operation_2 );

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	      operation -> relation_no_operation = 0;
	      operation -> relation_no_compte = 0;
	    }

	  validation_ope_de_ventilation ( operation );
	  operation -> operation_ventilee = 1;
	  operation -> categorie = 0;
	  operation -> sous_categorie = 0;
	}
      else
	{
	  /* ce n'est pas une opé ventilée, si c'est une modif d'opé et que */
	  /* c'en était une, on supprime les opés de ventil asssociées */

	  if ( modification
	       &&
	       operation -> operation_ventilee )
	    {
	      GSList *liste_tmp;

	      liste_tmp = LISTE_OPERATIONS;

	      while ( liste_tmp )
		{
		  struct structure_operation *ope_tmp;
		      
		  ope_tmp = liste_tmp -> data;

		  if ( ope_tmp -> no_operation_ventilee_associee == operation -> no_operation )
		    {
		      liste_tmp = liste_tmp -> next;
		      supprime_operation ( ope_tmp );
		    }
		  else
		    liste_tmp = liste_tmp -> next;
		}
	      operation -> operation_ventilee = 0;
	    }

	  /* on va maintenant séparer entre virement et catég normale */


	  tableau_char = g_strsplit ( pointeur_char,
				      ":",
				      2 );
      
	  tableau_char[0] = g_strstrip ( tableau_char[0] );

	  if ( tableau_char[1] )
	    tableau_char[1] = g_strstrip ( tableau_char[1] );


	  if ( strlen ( tableau_char[0] ) )
	    {
	      if ( !strcmp ( tableau_char[0],
			     _("Virement") )
		   && tableau_char[1]
		   && strlen ( tableau_char[1]) )
		{
		  /* c'est un virement, il n'y a donc aucune catégorie */

		  operation -> categorie = 0;
		  operation -> sous_categorie = 0;

		  /* sépare entre virement vers un compte et virement vers un compte supprimé */

		  if ( strcmp ( tableau_char[1],
				_("Compte supprimé") ) )
		    {
		      /* c'est un virement normal, on appelle la fonction qui va traiter ça */

		      validation_virement_operation ( operation,
						      modification,
						      tableau_char[1] );
		    }
		  else
		    {
		      /* c'est un virement vers un compte supprimé */

		      operation -> relation_no_compte = -1;
		      operation -> relation_no_operation = 1;
		    }
		}
	      else
		{
		  /* c'est une catég normale, si c'est une modif d'opé, vérifier si ce n'était pas un virement */

		  if ( modification
		       &&
		       operation -> relation_no_operation )
		    {
		      /* c'était un virement, et ce ne l'est plus, donc on efface l'opé en relation */

		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		      operation_2 = g_slist_find_custom ( LISTE_OPERATIONS,
							  GINT_TO_POINTER ( operation -> relation_no_operation ),
							  ( GCompareFunc ) recherche_operation_par_no ) -> data;
	  
		      operation_2 -> relation_no_operation = 0;
		      MISE_A_JOUR = 1;

		      supprime_operation ( operation_2 );

		      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

		      operation -> relation_no_operation = 0;
		      operation -> relation_no_compte = 0;
		    }

		  pointeur_liste = g_slist_find_custom ( liste_struct_categories,
							 tableau_char[0],
							 ( GCompareFunc ) recherche_categorie_par_nom );

		  if ( pointeur_liste )
		    categ = pointeur_liste -> data;
		  else
		    {
		      categ = ajoute_nouvelle_categorie ( tableau_char[0] );
		      if ( operation -> montant < 0 )
			categ -> type_categ = 1;
		      else
			categ -> type_categ = 0;
		    }

		  operation -> categorie = categ -> no_categ;
	  
		  if ( tableau_char[1] && strlen (tableau_char[1]) )
		    {
		      struct struct_sous_categ *sous_categ;
		  
		      pointeur_liste = g_slist_find_custom ( categ -> liste_sous_categ,
							     tableau_char[1],
							     ( GCompareFunc ) recherche_sous_categorie_par_nom );
	      
		      if ( pointeur_liste )
			sous_categ = pointeur_liste -> data;
		      else
			sous_categ = ajoute_nouvelle_sous_categorie ( tableau_char[1],
								      categ );
		  
		      operation -> sous_categorie = sous_categ -> no_sous_categ;
		    }
		  else
		    operation -> sous_categorie = 0;
		}
	    }
	  else
	    {
	      operation -> categorie = 0;
	      operation -> sous_categorie = 0;
	    }
	  g_strfreev ( tableau_char );
	}
    }
  else
    {
      operation -> categorie = 0;
      operation -> sous_categorie = 0;
    }
 }
/***********************************************************************************************************/

/***********************************************************************************************************/
/* cette fonction crée la contre-opération, l'enregistre et met en place les liens */
/* si c'est une modif de virement, elle supprime aussi l'ancienne contre opération */
/***********************************************************************************************************/

void validation_virement_operation ( struct structure_operation *operation,
				     gint modification,
				     gchar *nom_compte_vire )
{
  gint compte_virement;
  gint i;
  struct struct_devise *devise;
  struct struct_devise *devise_compte_2;
  struct structure_operation *contre_operation;

  /* on n'a plus besoin de faire de tests, ceux ci ont été fait dans verification_validation_operation */

  /* récupère le no du compte de la contre opération */

  compte_virement = -1;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

      if ( !g_strcasecmp ( NOM_DU_COMPTE,
			   nom_compte_vire ))
	{
	  compte_virement = i;
	  i = nb_comptes;
	}
    }

  /*   2 possibilités, soit c'est un nouveau virement ou une modif qui n'était pas un virement, */
  /* soit c'est une modif de virement */
  /*     dans le 2nd cas, si on a modifié le compte de la contre opération */
  /*     on supprime l'ancienne contre opération et c'est un nouveau virement */

  if ( modification )
    {
      /*       c'est une modif d'opé */
      /* 	soit c'était un virement vers un autre compte et dans ce cas on vire la contre-opération pour la recréer plus tard */
      /* soit c'est un virement vers le même compte et dans ce cas on fait rien, la contre opé sera modifiée automatiquement */
      /* soit ce n'était pas un virement et dans ce cas on considère l'opé comme une nouvelle opé */

      if ( operation -> relation_no_operation )
	{
	  /* c'était déjà un virement, on ne vire la contre opé que si le compte cible a changé */

	  if ( operation -> relation_no_compte != compte_virement )
	    {
	      /* il faut retirer l'ancienne contre opération */

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

	      contre_operation = g_slist_find_custom ( LISTE_OPERATIONS,
						       GINT_TO_POINTER ( operation -> relation_no_operation ),
						       ( GCompareFunc ) recherche_operation_par_no ) -> data;
	      contre_operation -> relation_no_operation = 0;

	      supprime_operation ( contre_operation );
	      modification = 0;
	    }
	}
      else
	{
	  /* ce n'était pas un virement, on considère que c'est une nouvelle opé pour créer la contre opération */

	  modification = 0;
	}
    }

  /*   on en est maintenant à soit nouveau virement, soit modif de virement sans chgt de compte */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_virement;

  if ( modification && operation -> relation_no_operation )
    contre_operation = g_slist_find_custom ( LISTE_OPERATIONS,
					     GINT_TO_POINTER ( operation -> relation_no_operation ),
					     ( GCompareFunc ) recherche_operation_par_no ) -> data;
  else
    {
      contre_operation = calloc ( 1,
			     sizeof ( struct structure_operation ) );
      contre_operation -> no_compte = compte_virement;
    }

  /* remplit la nouvelle opé */
      
  contre_operation -> jour = operation -> jour;
  contre_operation -> mois = operation -> mois;
  contre_operation -> annee = operation -> annee;
  contre_operation ->date = g_date_new_dmy ( contre_operation->jour,
					contre_operation->mois,
					contre_operation->annee);
  contre_operation -> montant = -operation -> montant;

  /* si c'est la devise du compte ou si c'est un compte qui doit passer à l'euro ( la transfo se fait au niveau */
  /* de l'affichage de la liste ) ou si c'est un compte en euro et l'opé est dans une devise qui doit passer à l'euro -> ok */

  devise_compte_2 = g_slist_find_custom ( liste_struct_devises,
					  GINT_TO_POINTER ( DEVISE ),
					  ( GCompareFunc ) recherche_devise_par_no ) -> data;
  devise = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ) -> menu_item ),
				 "adr_devise" );

  contre_operation -> devise = operation -> devise;

  if ( !( contre_operation-> no_operation
	  ||
	  devise -> no_devise == DEVISE
	  ||
	  ( devise_compte_2 -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	  ||
	  ( !strcmp ( devise_compte_2 -> nom_devise, _("Euro") ) && devise -> passage_euro )))
    {
      /* c'est une devise étrangère, on demande le taux de change et les frais de change */
	  
      demande_taux_de_change ( devise_compte_2,
			       devise,
			       1,
			       (gdouble ) 0,
			       (gdouble ) 0 );

      contre_operation -> taux_change = taux_de_change[0];
      contre_operation -> frais_change = taux_de_change[1];

      if ( contre_operation -> taux_change < 0 )
	{
	  contre_operation -> taux_change = -contre_operation -> taux_change;
	  contre_operation -> une_devise_compte_egale_x_devise_ope = 1;
	}
    }
  else
    {
      contre_operation -> taux_change = 0;
      contre_operation -> frais_change = 0;
    }

  contre_operation -> tiers = operation -> tiers;
  contre_operation -> categorie = operation -> categorie;
  contre_operation -> sous_categorie = operation -> sous_categorie;

  if ( operation -> notes )
    contre_operation -> notes = g_strdup ( operation -> notes);

  contre_operation -> auto_man = operation -> auto_man;

  /* récupération du type de l'autre opé */

  if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[13] ))
    contre_operation -> type_ope = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[13] ) -> menu_item ),
								      "no_type" ));
  else
    {
      contre_operation -> type_ope = 0;
      operation -> contenu_type = NULL;
    }


  if ( operation -> contenu_type )
    contre_operation -> contenu_type = g_strdup ( operation -> contenu_type );

  contre_operation -> no_exercice = operation -> no_exercice;
  contre_operation -> imputation = operation -> imputation;
  contre_operation -> sous_imputation = operation -> sous_imputation;

  if ( operation -> no_piece_comptable )
    contre_operation -> no_piece_comptable = g_strdup ( operation -> no_piece_comptable );

  if ( operation -> info_banque_guichet )
    contre_operation -> info_banque_guichet = g_strdup ( operation -> info_banque_guichet );

  /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

  ajout_operation ( contre_operation );


  /* on met maintenant les relations entre les différentes opé */

  operation -> relation_no_operation = contre_operation -> no_operation;
  operation -> relation_no_compte = contre_operation -> no_compte;
  contre_operation -> relation_no_operation = operation -> no_operation;
  contre_operation -> relation_no_compte = operation -> no_compte;

}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction ajout_operation */
/* ajoute l'opération donnée en argument à la liste des opés, trie la liste */
/* et réaffiche la clist */
/* remplit si nécessaire le no d'opération */
/***********************************************************************************************************/

void ajout_operation ( struct structure_operation *operation )
{
  gpointer **save_ptab;

  save_ptab = p_tab_nom_de_compte_variable;

  /* si c'est une nouvelle opération, on lui met son no d'opération */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

  if ( !operation -> no_operation )
    {
      operation -> no_operation = ++no_derniere_operation;

      LISTE_OPERATIONS = g_slist_append ( LISTE_OPERATIONS,
					  operation );
      NB_OPE_COMPTE++;
    }

  /*   on réaffiche(ra) la liste des opés */

  MISE_A_JOUR = 1;


  /* on classe la liste */

  LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				    (GCompareFunc) classement_sliste_par_date );

  if ( operation -> no_compte == compte_courant )
    verification_mise_a_jour_liste ();

  mise_a_jour_solde ( operation -> no_compte );

  /* on réaffiche les comptes de l'accueil */

  update_liste_comptes_accueil();
  mise_a_jour_soldes_minimaux();
  mise_a_jour_fin_comptes_passifs ();

  p_tab_nom_de_compte_variable = save_ptab;
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* cette procédure compare 2 struct de compte entre elles au niveau de la date */
/* pour le classement, si la dâte est équivalente, elle les classe par le no d'opé */
/***********************************************************************************************************/

gint comparaison_date_list_ope ( struct structure_operation *ope_1,
				 struct structure_operation *ope_2)
{
  gint retour;

  if ( !( retour = g_date_compare ( ope_1 -> date, ope_2 -> date) ) )
    retour = ope_1 -> no_operation - ope_2 -> no_operation;

  return ( retour );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* efface le contenu du formulaire */
/***********************************************************************************************************/

void formulaire_a_zero (void)
{

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;


  /* on met les styles des entrées au gris */

  gtk_widget_set_style (widget_formulaire_operations[1],
			style_entree_formulaire[1] );
  gtk_widget_set_style (GTK_COMBOFIX ( widget_formulaire_operations[2])->entry,
			style_entree_formulaire[1] );
  gtk_widget_set_style (widget_formulaire_operations[3],
			style_entree_formulaire[1] );
  gtk_widget_set_style (widget_formulaire_operations[4],
			style_entree_formulaire[1] );
  gtk_widget_set_style (widget_formulaire_operations[7],
			style_entree_formulaire[1] );
  gtk_widget_set_style (GTK_COMBOFIX ( widget_formulaire_operations[8])->entry,
			style_entree_formulaire[1] );
  gtk_widget_set_style (widget_formulaire_operations[10],
			style_entree_formulaire[1] );
  gtk_widget_set_style (GTK_COMBOFIX ( widget_formulaire_operations[12])->entry,
			style_entree_formulaire[1] );
  gtk_widget_set_style (widget_formulaire_operations[14],
			style_entree_formulaire[1] );
  gtk_widget_set_style (widget_formulaire_operations[16],
			style_entree_formulaire[1] );
  gtk_widget_set_style (widget_formulaire_operations[17],
			style_entree_formulaire[1] );

  gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[0]),
		       "" );

  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[1]),
		       _("Date") );

  gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_operations[2] ),
			  _("Tiers") );
  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[3]),
		       _("Débit") );
  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[4]),
		       _("Crédit") );

  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[7]),
		       _("Date de valeur") );
  gtk_combofix_set_text ( GTK_COMBOFIX (widget_formulaire_operations[8]),
			  _("Catégories : Sous-catégories") );
  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[10]),
		       _("n° Chèque/Virement") );

  gtk_combofix_set_text ( GTK_COMBOFIX (widget_formulaire_operations[12]),
			  _("Imputation budgétaire") );
  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[14]),
		       _("Pièce comptable") );

  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[16]),
		       _("Notes") );
  gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_operations[17]),
		       _("Informations banque/guichet") );

  gtk_widget_hide ( widget_formulaire_operations[6] );
  gtk_widget_hide ( widget_formulaire_operations[13] );
  gtk_widget_hide ( widget_formulaire_operations[15] );

  gtk_label_set_text ( GTK_LABEL ( widget_formulaire_operations[18]),
		       "" );

  gtk_widget_set_sensitive ( widget_formulaire_operations[4],
			     TRUE );
  gtk_widget_set_sensitive ( widget_formulaire_operations[3],
			     TRUE );
  gtk_widget_set_sensitive ( widget_formulaire_operations[8],
			     TRUE );

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
				cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[5] ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( DEVISE ),
									 ( GCompareFunc ) recherche_devise_par_no )));

  gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_operations[9] ),
			     FALSE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_operations[5] ),
			     FALSE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_operations[11] ),
			     FALSE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ope ),
			     FALSE );
  gtk_widget_set_sensitive ( widget_formulaire_operations[12],
			     etat.utilise_imputation_budgetaire );


  gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			"adr_struct_ope",
			NULL );
  gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			"liste_adr_ventilation",
			NULL );

  gtk_widget_set_sensitive ( bouton_affiche_cache_formulaire,
			     TRUE );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* procédure qui renvoie soit la dâte du jour, soit l'ancienne date tapée sous forme de string */
/***********************************************************************************************************/

gchar *date_jour ( void )
{
  time_t temps;

  if ( !etat.ancienne_date )
    {
      time ( &temps );

      strftime ( date_courante,
		 11,
		 "%d/%m/%Y",
		 localtime ( &temps ) );
      etat.ancienne_date = 1; 
    }

  return ( g_strdup ( date_courante ) );

}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction affiche_cache_le_formulaire */
/* si le formulaire était affiché, le cache et vice-versa */
/***********************************************************************************************************/

void affiche_cache_le_formulaire ( void )
{
  gpointer **save_ptab;

  save_ptab = p_tab_nom_de_compte_variable;
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;



  if ( etat.formulaire_toujours_affiche )
    {
      gtk_widget_hide ( fleche_bas );
      gtk_widget_show ( fleche_haut );

      gtk_widget_hide ( frame_droite_bas );
      etat.formulaire_toujours_affiche = 0;

    }
  else
    {
      gtk_widget_hide ( fleche_haut );
      gtk_widget_show ( fleche_bas );

      gtk_widget_show ( frame_droite_bas );
      etat.formulaire_toujours_affiche = 1;

    }

  gtk_widget_grab_focus ( CLIST_OPERATIONS );

  p_tab_nom_de_compte_variable = save_ptab;

}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction  allocation_taille_formulaire */
/* appelée quand le formulaire est affiché*/
/* modifie la liste pour qu'il n'y ai pas de changement */
/***********************************************************************************************************/

void allocation_taille_formulaire ( GtkWidget *widget,
				    gpointer null )
{
  GtkAdjustment *ajustement;
  gpointer **save_ptab;
  return;
  save_ptab = p_tab_nom_de_compte_variable;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  if ( !GTK_WIDGET_VISIBLE ( notebook_general ))
    return;

  ajustement = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS ) );

  /*   if ( ajustement -> value == ( ajustement -> upper - ajustement -> page_size )) */
  /*        return; */

  gtk_adjustment_set_value ( GTK_ADJUSTMENT ( ajustement ),
			     ajustement -> value + GTK_WIDGET ( frame_droite_bas ) -> allocation.height + 10 );

  p_tab_nom_de_compte_variable = save_ptab;

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction efface_formulaire */
/* appelée quand le formulaire est effacé*/
/* modifie la liste pour qu'il n'y ai pas de changement */
/***********************************************************************************************************/

void efface_formulaire ( GtkWidget *widget,
			 gpointer null )
{
  GtkAdjustment *ajustement;
  gpointer **save_ptab;

  save_ptab = p_tab_nom_de_compte_variable;
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  ajustement = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS ) );

  gtk_adjustment_set_value ( GTK_ADJUSTMENT ( ajustement ),
			     ajustement -> value - GTK_WIDGET ( frame_droite_bas ) -> allocation.height - 10 );


  p_tab_nom_de_compte_variable = save_ptab;

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction basculer_vers_ventilation */
/* appelée par l'appui du bouton Ventilation... */
/* permet de voir les opés ventilées d'une ventilation */
/***********************************************************************************************************/

void basculer_vers_ventilation ( GtkWidget *bouton,
				 gpointer null )
{
  enregistre_ope_au_retour = 0;

  if ( gtk_widget_get_style ( widget_formulaire_operations[3] ) == style_entree_formulaire[0] )
    ventiler_operation ( -g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[3] ))),
				     NULL ));
  else
    ventiler_operation ( g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[4] ))),
				    NULL ));
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Fonction click_sur_bouton_voir_change  */
/* permet de modifier un change établi pour une opération */
/***********************************************************************************************************/

void click_sur_bouton_voir_change ( void )
{
  struct structure_operation *operation;
  struct struct_devise *devise;

  gtk_widget_grab_focus ( widget_formulaire_operations[1] );

  operation = gtk_object_get_data ( GTK_OBJECT ( formulaire ),
				    "adr_struct_ope" );


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  if ( !devise_compte
       ||
       devise_compte -> no_devise != DEVISE )
    devise_compte = g_slist_find_custom ( liste_struct_devises,
					  GINT_TO_POINTER ( DEVISE ),
					  ( GCompareFunc ) recherche_devise_par_no ) -> data;
  devise = g_slist_find_custom ( liste_struct_devises,
				 GINT_TO_POINTER ( operation -> devise ),
				 ( GCompareFunc ) recherche_devise_par_no ) -> data;

  demande_taux_de_change ( devise_compte,
			   devise,
			   operation -> une_devise_compte_egale_x_devise_ope,
			   operation -> taux_change,
			   operation -> frais_change );
      
  if ( taux_de_change[0] ||  taux_de_change[1] )
    {
      operation -> taux_change = taux_de_change[0];
      operation -> frais_change = taux_de_change[1];
	  
      if ( operation -> taux_change < 0 )
	{
	  operation -> taux_change = -operation -> taux_change;
	  operation -> une_devise_compte_egale_x_devise_ope = 1;
	}
      else
	operation -> une_devise_compte_egale_x_devise_ope = 0;
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
void degrise_formulaire_operations ( void )
{
  struct struct_type_ope *type;


  gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_operations[9] ),
			     TRUE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_operations[5] ),
			     TRUE );
  gtk_widget_set_sensitive ( GTK_WIDGET ( hbox_valider_annuler_ope ),
			     TRUE );

  /* on ne dégrise l'exo que si le bouton ventiler n'est pas visible */

  if ( !GTK_WIDGET_VISIBLE ( widget_formulaire_operations[15] ))
    gtk_widget_set_sensitive ( GTK_WIDGET ( widget_formulaire_operations[11] ),
			       etat.utilise_exercice );

  /*   si le type par défaut est un chèque, on met le nouveau numéro */

  if ( GTK_WIDGET_VISIBLE ( widget_formulaire_operations[9] ))
    {
      type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ) -> menu_item ),
				   "adr_type" );

      if ( type -> numerotation_auto )
	{
	  entree_prend_focus ( widget_formulaire_operations[10] );

	  if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget_formulaire_operations[10] ))))
	    gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_operations[10] ),
				 itoa ( type -> no_en_cours  + 1));
	}
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* appelée lors de l'appui des touche + ou - sur les formulaires */
/* augmente ou diminue la date entrée de 1 */
/***********************************************************************************************************/

void incremente_decremente_date ( GtkWidget *entree,
				  gint demande )
{
  gchar **tableau_char;
  GDate *date;
  gint jour, mois, annee;

  /* on commence par vérifier que la date est valide */

  if ( !modifie_date ( entree ))
    return;

  tableau_char = g_strsplit ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY (entree))),
			      "/",
			      3 );

  jour = strtod ( tableau_char[0],
		  NULL );
  mois = strtod ( tableau_char[1],
		  NULL );
  annee = strtod (tableau_char[2],
		  NULL );

  date = g_date_new_dmy ( jour,
			  mois,
			  annee);

  g_date_add_days ( date,
		    demande );

  gtk_entry_set_text ( GTK_ENTRY (entree),
		       g_strdup_printf ( "%d/%d/%d",
					 g_date_day ( date ),
					 g_date_month ( date ),
					 g_date_year ( date )));

}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* retrouve la place du type dans option_menu */
/* et la renvoie */
/***********************************************************************************************************/

gint place_type_choix_type ( GtkWidget *option_menu,
			     gint type )
{
  GList *pointeur_liste;
  gint i;

  pointeur_liste = GTK_MENU_SHELL ( GTK_OPTION_MENU ( option_menu ) -> menu ) -> children;
  i=0;

  while ( pointeur_liste )
    {
      if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( pointeur_liste -> data ),
						   "no_type" )) == type )
	return ( i );

      i++;
      pointeur_liste = pointeur_liste -> next;
    }

  return ( 0 );
}
/***********************************************************************************************************/
