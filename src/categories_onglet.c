/* fichier qui s'occupe de l'onglet de gestion des catégories */
/*           categories_onglet.c */

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

#define __(X) X

gchar *categories_de_base_debit [] = {
  __("Alimentation : Bar"),
  __("Alimentation : Boulangerie"),
  __("Alimentation : Cantine"),
  __("Alimentation : Confiserie"),
  __("Alimentation : Divers"),
  __("Alimentation : Epicerie"),
  __("Alimentation : Restaurant"),
  __("Alimentation : Self"),
  __("Alimentation : Supermarché"),
  __("Animaux domestiques : Alimentation"),
  __("Animaux domestiques : Fournitures diverses"),
  __("Animaux domestiques : Toilettage"),
  __("Animaux domestiques : Vétérinaire"),
  __("Assurance : Automobile"),
  __("Assurance : Santé"),
  __("Assurance : Habitation"),
  __("Assurance : Responsabilité civile"),
  __("Assurance : Vie"),
  __("Automobile : Carburant"),
  __("Automobile : Réparation"),
  __("Automobile : Entretien"),
  __("Automobile : Stationnement"),
  __("Automobile : Amendes"),
  __("Divers"),
  __("Dons"),
  __("Enfants : Crèche / Nourrice"),
  __("Enfants : Frais divers"),
  __("Enfants : Etudes"),
  __("Etudes : Cours"),
  __("Etudes : Frais de scolarité"),
  __("Etudes : Livres"),
  __("Frais divers : Cadeaux"),
  __("Frais financiers : Divers"),
  __("Frais financiers : Frais bancaires"),
  __("Frais financiers : Emprunt"),
  __("Frais financiers : Charges d'emprunts"),
  __("Frais financiers : Remboursement"),
  __("Frais professionnels : Non remboursés"),
  __("Frais professionnels : Remboursés"),
  __("Impôts : Autres impôts"),
  __("Impôts : Impôt sur le revenu"),
  __("Impôts : Impôts locaux"),
  __("Logement : Hotel"),
  __("Logement : Loyer"),
  __("Logement : Abonnement TV"),
  __("Logement : Ameublement"),
  __("Logement : Charges"),
  __("Logement : Chauffage"),
  __("Logement : Décoration"),
  __("Logement : Eau"),
  __("Logement : Electricité"),
  __("Logement : Electroménager"),
  __("Logement : Equipement"),
  __("Logement : Gaz"),
  __("Logement : Jardin"),
  __("Logement : Salarié à domicile"),
  __("Logement : Téléphone"),
  __("Loisirs : Billard"),
  __("Loisirs : Bowling"),
  __("Loisirs : Cinéma"),
  __("Loisirs : Discothèque"),
  __("Loisirs : Informatique"),
  __("Loisirs : Jeux"),
  __("Loisirs : Lecture"),
  __("Loisirs : Parc d'attraction"),
  __("Loisirs : Spectacle"),
  __("Loisirs : Sport"),
  __("Loisirs : Vidéo"),
  __("Loisirs : Voyage"),
  __("Loisirs : Equipement"),
  __("Loisirs : Musée / Exposition"),
  __("Prêt : Capital"),
  __("Santé : Assurance"),
  __("Santé : Dentiste"),
  __("Santé : Hôpital"),
  __("Santé : Kinésithérapeute"),
  __("Santé : Médecin"),
  __("Santé : Ophtalmologiste"),
  __("Santé : Ostéopathe"),
  __("Santé : Pharmacie"),
  __("Santé : Sécurité sociale"),
  __("Soins : Coiffeur"),
  __("Soins : Habillement"),
  __("Transport : Bus"),
  __("Transport : Métro"),
  __("Transport : Péage"),
  __("Transport : Train"),
  __("Transport : Tramway"),
  __("Transport : Voyage"),
  __("Transport : Train"),
  __("Vacances : Logement"),
  __("Vacances : Visites"),
  __("Vacances : Voyages"),
  NULL };

gchar *categories_de_base_credit [] = {
  __("Autres revenus : Allocation chômage"),
  __("Autres revenus : Allocation familiales"),
  __("Autres revenus : Capital de prêt reçu"),
  __("Autres revenus : Crédit d'impôt"),
  __("Autres revenus : Jeu"),
  __("Autres revenus : Mutuelle santé"),
  __("Autres revenus : Sécurité sociale"),
  __("Retraite : Pensions"),
  __("Retraite : Régime général"),
  __("Retraite : Retraite complémentaire"),
  __("Revenus de placement : Dividendes"),
  __("Revenus de placement : Intérêts"),
  __("Revenus de placement : Plus-values"),
  __("Salaire : Heures supplémentaires"),
  __("Salaire : Primes congés"),
  __("Salaire : Primes diverses"),
  __("Salaire : Primes résultats"),
  __("Salaire : Salaire net"),
  __("Revenus divers : Cadeaux"),
  __("Revenus divers : Remboursement"),
  __("Revenus divers : Vente occasion"),
  NULL
};




/* **************************************************************************************************** */
/* Fonction onglet_categories : */
/* crée et renvoie le widget contenu dans l'onglet */
/* **************************************************************************************************** */

GtkWidget *onglet_categories ( void )
{
  GtkWidget *onglet;
  GtkWidget *scroll_window;
  gchar *titres[] =
  {
    __("Liste des catégories"),
    __("Montant par catégorie"),
    __("Montant par sous-catégorie"),
    __("Montant par compte")
  };
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *vbox_frame;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *separateur;
  GtkWidget *bouton;


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

  entree_nom_categ = gtk_entry_new ();
  gtk_widget_set_sensitive ( entree_nom_categ,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( entree_nom_categ ),
		       "changed",
		       GTK_SIGNAL_FUNC ( modification_du_texte_categ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       entree_nom_categ,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( entree_nom_categ );


/* création des radio bouton débit/crédit */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  label = gtk_label_new ( _("Classement :") );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       label,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( label );

  bouton_categ_debit = gtk_radio_button_new_with_label ( NULL,
							 _("Débit") );
  gtk_widget_set_sensitive ( bouton_categ_debit,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_categ_debit ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( modification_du_texte_categ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       bouton_categ_debit,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_categ_debit );


  bouton_categ_credit = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON ( bouton_categ_debit ),
								      _("Crédit") );
  gtk_widget_set_sensitive ( bouton_categ_credit,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       bouton_categ_credit,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_categ_credit );


  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       separateur,
		       FALSE,
		       FALSE,
		       10 );
  gtk_widget_show ( separateur );




/*   création des boutons modifier et annuler */

  hbox = gtk_hbox_new ( TRUE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton_modif_categ_modifier = gnome_stock_button ( GNOME_STOCK_BUTTON_APPLY );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_categ_modifier ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_modif_categ_modifier ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( clique_sur_modifier_categ ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_modif_categ_modifier,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_modif_categ_modifier );

  bouton_modif_categ_annuler = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_modif_categ_annuler ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_modif_categ_annuler ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( clique_sur_annuler_categ ),
		       NULL );
  gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			     FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_modif_categ_annuler,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_modif_categ_annuler);

  bouton_supprimer_categ = gnome_stock_button ( GNOME_STOCK_PIXMAP_REMOVE );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_supprimer_categ ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_supprimer_categ,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_categ ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( supprimer_categ ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox_frame ),
		       bouton_supprimer_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_supprimer_categ );


  /* mise en place des boutons ajout d'1 categ / sous-categ */

  bouton_ajouter_categorie = gtk_button_new_with_label ( _("Ajouter une catégorie") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_categorie ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_categorie ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( appui_sur_ajout_categorie ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_ajouter_categorie,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_ajouter_categorie );

  bouton_ajouter_sous_categorie = gtk_button_new_with_label ( _("Ajouter une sous-catégorie") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ajouter_sous_categorie ),
			  GTK_RELIEF_NONE );
  gtk_widget_set_sensitive ( bouton_ajouter_sous_categorie,
			     FALSE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ajouter_sous_categorie ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( appui_sur_ajout_sous_categorie ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_ajouter_sous_categorie,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_ajouter_sous_categorie );

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  /* on met le bouton exporter */

  bouton = gtk_button_new_with_label ( _("Exporter ...") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( exporter_categ ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  /* on met le bouton importer */

  bouton = gtk_button_new_with_label ( _("Importer ...") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( importer_categ ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );



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
		       creation_barre_outils_categ(),
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


  arbre_categ = gtk_ctree_new_with_titles ( 4,
					    0,
					    titres );
  gtk_ctree_set_line_style ( GTK_CTREE ( arbre_categ ),
			     GTK_CTREE_LINES_DOTTED );
  gtk_ctree_set_expander_style ( GTK_CTREE ( arbre_categ ),
				 GTK_CTREE_EXPANDER_CIRCULAR );
  gtk_clist_column_titles_passive ( GTK_CLIST ( arbre_categ ));

  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_categ ),
				       0,
				       GTK_JUSTIFY_LEFT);
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_categ ),
				       1,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_categ ),
				       2,
				       GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_justification ( GTK_CLIST ( arbre_categ ),
				       3,
				       GTK_JUSTIFY_CENTER);

  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_categ ),
				    0,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_categ ),
				    1,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_categ ),
				    2,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( arbre_categ ),
				    3,
				    FALSE );

  gtk_signal_connect ( GTK_OBJECT ( arbre_categ ),
		       "tree-select-row",
		       GTK_SIGNAL_FUNC ( selection_ligne_categ ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( arbre_categ ),
		       "tree-unselect-row",
		       GTK_SIGNAL_FUNC ( enleve_selection_ligne_categ ),
		       NULL );
  gtk_signal_connect_after ( GTK_OBJECT ( arbre_categ ),
			     "button-press-event",
			     GTK_SIGNAL_FUNC ( verifie_double_click_categ ),
			     NULL );
  gtk_signal_connect ( GTK_OBJECT ( arbre_categ ),
		       "size-allocate",
		       GTK_SIGNAL_FUNC ( changement_taille_liste_tiers ),
		       NULL );
  gtk_signal_connect ( GTK_OBJECT ( arbre_categ ),
		       "tree-expand",
		       GTK_SIGNAL_FUNC ( ouverture_node_categ ),
		       NULL );
  gtk_container_add ( GTK_CONTAINER (  scroll_window ),
		      arbre_categ );
  gtk_widget_show ( arbre_categ );


  /* on met la fontion de tri alphabétique en prenant en compte les accents */

  gtk_clist_set_compare_func ( GTK_CLIST ( arbre_categ ),
			       (GtkCListCompareFunc) classement_alphabetique_tree );

  /* la 1ère fois qu'on affichera les catég, il faudra remplir la liste */

  modif_categ = 1;


  return ( onglet );
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* Fonction remplit_arbre_categ */
/* prend en argument le clist arbre_categ, */
/* le vide et le remplit */
/* on y met les categ et sous categ et leur montant */
/* **************************************************************************************************** */

void remplit_arbre_categ ( void )
{
  gchar *text[4];
  GSList *liste_categ_tmp;
  struct struct_devise *devise_compte;
  gint place_categ;
  gint i;

  /* freeze le ctree */

  gtk_clist_freeze ( GTK_CLIST ( arbre_categ ));

  /*   efface l'ancien arbre */

  gtk_clist_clear ( GTK_CLIST ( arbre_categ ));


  /*   le devise est choisie dans les paramètres */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  devise_compte = g_slist_find_custom ( liste_struct_devises,
					GINT_TO_POINTER ( no_devise_totaux_tiers ),
					( GCompareFunc ) recherche_devise_par_no) -> data;

  /* calcule les montants des catég et sous categ */

  calcule_total_montant_categ ();

  /* remplit l'arbre */

  liste_categ_tmp = liste_struct_categories;
  place_categ = 0;

  while ( liste_categ_tmp )
    {
      struct struct_categ *categ;
      GtkCTreeNode *ligne;
      GSList *liste_sous_categ_tmp;
      GtkCTreeNode *ligne_sous_categ;
      gint place_sous_categ;

      categ = liste_categ_tmp -> data;

      if ( etat.affiche_nb_ecritures_listes
	   &&
	   nb_ecritures_par_categ[place_categ+1] )
	text[0] = g_strconcat ( categ -> nom_categ,
				" (",
				itoa ( nb_ecritures_par_categ[place_categ+1] ),
				")",
				NULL );
      else
	text[0] = categ -> nom_categ ;

      if ( tab_montant_categ[place_categ+1] )
	text[1] = g_strdup_printf ( "%4.2f %s",
				    tab_montant_categ[place_categ+1],
				    devise_compte -> code_devise );
      else
	text[1] = NULL;
      text[2] = NULL;
      text[3] = NULL;

      ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
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

       /* on associe à cette categ à l'adr de sa struct */

      gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
				    ligne,
				    categ );

      /*       pour chaque categ, on met ses sous-categ */

      liste_sous_categ_tmp = categ -> liste_sous_categ;
      place_sous_categ = 0;

      while ( liste_sous_categ_tmp )
	{
	  struct struct_sous_categ *sous_categ;

	  sous_categ = liste_sous_categ_tmp -> data;

	  if ( tab_montant_sous_categ[place_categ]
	       &&
	       tab_montant_sous_categ[place_categ][place_sous_categ+1]
	       &&
	       etat.affiche_nb_ecritures_listes
	       &&
	       nb_ecritures_par_sous_categ[place_categ][place_sous_categ+1] )
	    text[0] = g_strconcat ( sous_categ -> nom_sous_categ,
				    " (",
				    itoa ( nb_ecritures_par_sous_categ[place_categ][place_sous_categ+1] ),
				    ")",
				    NULL );
	  else
	    text[0] = sous_categ -> nom_sous_categ ;

	  text[1] = NULL;

	  if ( tab_montant_sous_categ[place_categ]
	       &&
	       tab_montant_sous_categ[place_categ][place_sous_categ+1] )
	    text[2] = g_strdup_printf ( "%4.2f %s",
					tab_montant_sous_categ[place_categ][place_sous_categ+1],
					devise_compte -> code_devise );
	  else
	    text[2] = NULL;

	  text[3] = NULL;

	  ligne_sous_categ = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
						     ligne,
						     NULL,
						     text,
						     10,
						     NULL,
						     NULL,
						     NULL,
						     NULL,
						     FALSE,
						     FALSE );

	  /* on associe cette sous_categ à l'adr de sa struct */

	  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
					ligne_sous_categ,
					sous_categ );

	  /* pour chacun des sous categ, on met un fils bidon pour pouvoir l'ouvrir */

	  ligne_sous_categ = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
						     ligne_sous_categ,
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

	  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
					ligne_sous_categ,
					GINT_TO_POINTER (-1));


	  place_sous_categ++;
	  liste_sous_categ_tmp = liste_sous_categ_tmp -> next;
	}

      /*       on a fini de saisir les sous catégories, s'il y avait des opés sans sous catég, on les mets ici */

      if ( tab_montant_sous_categ[place_categ]
	   &&
	   nb_ecritures_par_sous_categ[place_categ][0])
	{
	  if ( etat.affiche_nb_ecritures_listes )
	    text[0] = g_strconcat ( _("Aucune sous-catégorie ("),
				    itoa ( nb_ecritures_par_sous_categ[place_categ][0] ),
				    ")",
				    NULL );
	  else
	    text[0] = _("Aucune sous-catégorie");

	  text[1] = NULL;

	  if ( tab_montant_sous_categ[place_categ][0] )
	    text[2] = g_strdup_printf ( "%4.2f %s",
					tab_montant_sous_categ[place_categ][0],
					devise_compte -> code_devise );
	  else
	    text[2] = NULL;

	  text[3] = NULL;

	  ligne_sous_categ = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
						     ligne,
						     NULL,
						     text,
						     10,
						     NULL,
						     NULL,
						     NULL,
						     NULL,
						     FALSE,
						     FALSE );

	  /* pour chacun des sous categ, on met un fils bidon pour pouvoir l'ouvrir */

	  ligne_sous_categ = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
						     ligne_sous_categ,
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

	  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
					ligne_sous_categ,
					GINT_TO_POINTER (-1));

	}
      place_categ++;
      liste_categ_tmp = liste_categ_tmp -> next;
    }

  /*   on a fini de mettre les catégories, on met ici les opés sans catég */

  if ( tab_montant_categ[0] )
    {
      GtkCTreeNode *ligne;
      GtkCTreeNode *ligne_sous_categ;

      if ( etat.affiche_nb_ecritures_listes
	   &&
	   nb_ecritures_par_categ[0] )
	text[0] = g_strconcat ( _("Aucune catégorie ("),
				itoa ( nb_ecritures_par_categ[0] ),
				")",
				NULL );
      else
	text[0] = _("Aucune catégorie");

      text[1] = g_strdup_printf ( "%4.2f %s",
				  tab_montant_categ[0],
				  devise_compte -> code_devise );
      text[2] = NULL;
      text[3] = NULL;

      ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
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


      /* on met aucune sous categ */

      if ( etat.affiche_nb_ecritures_listes
	   &&
	   nb_ecritures_par_categ[0] )
	text[0] = g_strconcat ( _("Aucune sous-catégorie ("),
				itoa ( nb_ecritures_par_categ[0] ),
				")",
				NULL );
      else
	text[0] = _("Aucune sous-catégorie");

      text[1] = NULL;
      text[2] = g_strdup_printf ( "%4.2f %s",
				  tab_montant_categ[0],
				  devise_compte -> code_devise );
      text[3] = NULL;

      ligne_sous_categ = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
						 ligne,
						 NULL,
						 text,
						 10,
						 NULL,
						 NULL,
						 NULL,
						 NULL,
						 FALSE,
						 FALSE );

      /* on met un fils bidon pour pouvoir l'ouvrir */

      ligne_sous_categ = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
						 ligne_sous_categ,
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

      gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
				    ligne_sous_categ,
				    GINT_TO_POINTER (-1));
    }

  /*   on efface les variables */

  free ( tab_montant_categ );
  free ( nb_ecritures_par_categ );

  for ( i=0 ; i<nb_enregistrements_categories ; i++ )
    free ( tab_montant_sous_categ[i] );
  free ( tab_montant_sous_categ );

  for ( i=0 ; i<nb_enregistrements_categories ; i++ )
    free ( nb_ecritures_par_sous_categ[i] );
  free ( nb_ecritures_par_sous_categ );


  gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_categ ),
			     NULL );

  /* defreeze le ctree */

  gtk_clist_thaw ( GTK_CLIST ( arbre_categ ));

  enleve_selection_ligne_categ ();
  modif_categ = 0;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* Fonction ouverture_node_categ */
/* appeléé lorsqu'on ouvre une categ, sous categ ou le compte d'une categ */
/* remplit ce qui doit être affiché */
/* **************************************************************************************************** */

void ouverture_node_categ ( GtkWidget *arbre,
			    GtkCTreeNode *node,
			    gpointer null )
{			    
  GtkCTreeRow *row;
  gchar *text[4];
  GtkCTreeNode *node_insertion;

  row = GTK_CTREE_ROW ( node );

  /*   si on ouvre une categ, on fait rien */

  if ( row->level == 1 )
    return;

  /*   si le fiston = -1, c'est qu'il n'a pas encore été créé */
  /* dans le cas contraire, on vire */

  if ( GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
						       row -> children )) != -1 )
    return;

  /* freeze le ctree */

  gtk_clist_freeze ( GTK_CLIST ( arbre_categ ));


  /* on commence par virer la ligne bidon qui était attachée à ce noeud */

  gtk_ctree_remove_node ( GTK_CTREE ( arbre_categ ),
			  row -> children );

  /* séparation entre ouverture de sous-categ ( 2 ) et ouverture de compte ( 3 ) */

  if ( row -> level == 2 )
    {
      /* c'est une ouverture de sous categ, on récupère sa structure  */

      struct struct_categ *categ;
      struct struct_sous_categ *sous_categ;
      gint no_categ;
      gint no_sous_categ;
      gint i;

      /*       soit il y a une categ et une sous categ, soit c'est aucune categ (donc categ = 0) */

      if ( ( categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
						   row -> parent )))
	{
	  no_categ = categ -> no_categ;
	  if ( ( sous_categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
							    node)))
	    no_sous_categ = sous_categ -> no_sous_categ;
	  else
	    no_sous_categ = 0;
	}
      else
	{
	  no_categ = 0;
	  no_sous_categ = 0;
	}

      /* on va scanner tous les comptes, dès qu'un tiers correspondant au tiers sélectionné est trouvé */
      /* on affiche le nom du compte */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  GSList *pointeur_ope;

	  pointeur_ope = LISTE_OPERATIONS;

	  while ( pointeur_ope )
	    {
	      struct structure_operation *operation;
	      
	      operation = pointeur_ope -> data;

	      if ( operation -> categorie == no_categ
		   &&
		   operation -> sous_categorie == no_sous_categ
		   &&
		   !operation -> relation_no_operation
		   &&
		   !operation -> operation_ventilee )
		{
		  /* affiche le compte courant */

		  text[3] = calcule_total_montant_categ_par_compte ( operation -> categorie,
								     operation -> sous_categorie,
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
		  text[2] = NULL;

		  node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
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
		  
		  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
						node_insertion,
						GINT_TO_POINTER ( i ));

		  /* on met une ligne bidon pour pouvoir l'ouvrir */

		  node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
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

		  /* on associe le fils bidon à -1 */
		  
		  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
						node_insertion,
						GINT_TO_POINTER (-1));
		  
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
      /*       cette fois, on fait le tour de toutes les opés du compte pour afficher celles qui correspondent à la categ */

      struct struct_categ *categ;
      struct struct_sous_categ *sous_categ;
      GSList *pointeur_ope;
      gint no_categ;
      gint no_sous_categ;

      if (( categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
						  GTK_CTREE_ROW ( row -> parent ) -> parent )))
	{
	  no_categ = categ -> no_categ;

	  if ( (sous_categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
							   row -> parent )))
	    no_sous_categ = sous_categ -> no_sous_categ;
	  else
	    no_sous_categ = 0;
	}
      else
	{
	  no_categ = 0;
	  no_sous_categ = 0;
	}

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
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

	  if ( operation -> categorie == no_categ
	       &&
	       operation -> sous_categorie == no_sous_categ
	       &&
	       !operation -> relation_no_operation
	       &&
	       !operation -> operation_ventilee  )
	    {
		      if ( operation -> notes )
			{
			  if ( operation -> no_operation_ventilee_associee )
			    text[0] = g_strdup_printf ( _("%d/%d/%d : %4.2f %s (ventilation) [ %s ]"),
							operation -> jour,
							operation -> mois,
							operation -> annee,
							operation -> montant,
							devise_operation -> code_devise,
							operation -> notes );
			  else
			    text[0] = g_strdup_printf ( "%d/%d/%d : %4.2f %s [ %s ]",
							operation -> jour,
							operation -> mois,
							operation -> annee,
							operation -> montant,
							devise_operation -> code_devise,
							operation -> notes );
			}
		      else
			{
			  if ( operation -> no_operation_ventilee_associee )
			    text[0] = g_strdup_printf ( _("%d/%d/%d : %4.2f %s (ventilation)"),
							operation -> jour,
							operation -> mois,
							operation -> annee,
							operation -> montant,
							devise_operation -> code_devise );
			  else
			    text[0] = g_strdup_printf ( "%d/%d/%d : %4.2f %s",
							operation -> jour,
							operation -> mois,
							operation -> annee,
							operation -> montant,
							devise_operation -> code_devise );
			}

	      text[1] = NULL;
	      text[2] = NULL;
	      text[3] = NULL;

	      node_insertion = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
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

	      gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
					    node_insertion,
					    operation );
	    }
	  pointeur_ope = pointeur_ope -> next;
	}
    }

  /* defreeze le ctree */

  gtk_clist_thaw ( GTK_CLIST ( arbre_categ ));

}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
void selection_ligne_categ ( GtkCTree *arbre_categ,
			     GtkCTreeNode *noeud,
			     gint colonne,
			     gpointer null )
{
  GtkCTreeNode *node_tmp;

  node_tmp = noeud;

  while ( GTK_CTREE_ROW ( node_tmp ) -> level  != 1 )
    node_tmp = GTK_CTREE_ROW ( node_tmp ) -> parent;

  if ( GTK_CTREE_ROW ( noeud ) -> level != 1 )
    gtk_widget_set_sensitive ( bouton_ajouter_sous_categorie,
			       FALSE );
  else
    gtk_widget_set_sensitive ( bouton_ajouter_sous_categorie,
			       TRUE );



  if ( GTK_CTREE_ROW ( noeud ) -> level  == 1
       &&
       gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
				     noeud ))
    {
      struct struct_categ *categ;
  
      categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					    noeud );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_categ ),
					 GTK_SIGNAL_FUNC ( modification_du_texte_categ),
					 NULL );

      gtk_entry_set_text ( GTK_ENTRY ( entree_nom_categ ),
			   categ -> nom_categ );

      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_categ ),
					   GTK_SIGNAL_FUNC ( modification_du_texte_categ),
					   NULL );

      gtk_object_set_data ( GTK_OBJECT ( entree_nom_categ ),
			    "adr_struct_categ",
			    categ );
      gtk_object_set_data ( GTK_OBJECT ( entree_nom_categ ),
			    "adr_struct_sous_categ",
			    NULL );

      gtk_widget_set_sensitive ( bouton_supprimer_categ,
				 TRUE );
      gtk_widget_set_sensitive ( entree_nom_categ,
				 TRUE );
      gtk_widget_set_sensitive ( bouton_categ_debit,
				 TRUE );
      gtk_widget_set_sensitive ( bouton_categ_credit,
				 TRUE );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_categ_debit ),
					 GTK_SIGNAL_FUNC ( modification_du_texte_categ ),
					 NULL );
      if ( categ -> type_categ )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_categ_debit ),
				       TRUE );
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_categ_credit ),
				       TRUE );

      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_categ_debit ),
					   GTK_SIGNAL_FUNC ( modification_du_texte_categ ),
					   NULL );
    }
  else
    if ( GTK_CTREE_ROW ( noeud ) -> level  == 2
	 &&
	 gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
				       noeud )
	 &&
	 gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
				       GTK_CTREE_ROW ( noeud ) -> parent ))
    {
      struct struct_sous_categ *sous_categ;

      sous_categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
						 noeud );
      gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_categ ),
					 GTK_SIGNAL_FUNC ( modification_du_texte_categ),
					 NULL );

      gtk_entry_set_text ( GTK_ENTRY ( entree_nom_categ ),
			   sous_categ -> nom_sous_categ );

      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_categ ),
					   GTK_SIGNAL_FUNC ( modification_du_texte_categ),
					   NULL );

      gtk_object_set_data ( GTK_OBJECT ( entree_nom_categ ),
			    "adr_struct_categ",
			    NULL );
      gtk_object_set_data ( GTK_OBJECT ( entree_nom_categ ),
			    "adr_struct_sous_categ",
			    sous_categ );

      gtk_widget_set_sensitive ( bouton_supprimer_categ,
				 TRUE );
      gtk_widget_set_sensitive ( entree_nom_categ,
				 TRUE );
    }
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void verifie_double_click_categ ( GtkWidget *liste,
				  GdkEventButton *ev,
				  gpointer null )
{
  if ( ev -> type == GDK_2BUTTON_PRESS )
    {
      struct structure_operation *operation;

      if ( GTK_CLIST ( arbre_categ ) -> selection
	   &&
	   ( GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) -> level == 4
	     ||
	     ( GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) -> level == 3
	       &&
	       !gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					      GTK_CTREE_ROW ( GTK_CTREE_ROW (( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) -> parent ) -> parent ))))
	{
	  /* passage sur le compte concerné */

	  operation = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
						    GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) );

	  /* si c'est une opé de ventilation, on se place sur l'opé ventilée correspondante */
 
	  if ( operation -> no_operation_ventilee_associee )
	    {
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	      operation = g_slist_find_custom ( LISTE_OPERATIONS,
						GINT_TO_POINTER ( operation -> no_operation_ventilee_associee ),
						(GCompareFunc) recherche_operation_par_no ) -> data;
	    }

	  changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

	  if ( operation -> pointe == 2 && !AFFICHAGE_R )
	    {
	      AFFICHAGE_R = 1;
	      remplissage_liste_operations ( compte_courant );
	    }

	  OPERATION_SELECTIONNEE = operation;

	  selectionne_ligne ( compte_courant );
	}
    }
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void enleve_selection_ligne_categ ( void )
{
  gtk_widget_set_sensitive ( bouton_supprimer_categ,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			     FALSE );
  gtk_widget_set_sensitive ( entree_nom_categ,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_categ_debit,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_categ_credit,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_ajouter_sous_categorie,
			     FALSE );


  gtk_signal_handler_block_by_func ( GTK_OBJECT ( entree_nom_categ ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_categ),
				     NULL );

  gtk_editable_delete_text ( GTK_EDITABLE ( entree_nom_categ ),
			     0,
			     -1 );

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( entree_nom_categ ),
				     GTK_SIGNAL_FUNC ( modification_du_texte_categ),
				     NULL );

}
/* **************************************************************************************************** */






/* **************************************************************************************************** */
void modification_du_texte_categ ( void )
{
  gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			     TRUE );
  gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			     TRUE );
}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void clique_sur_modifier_categ ( void )
{
  GtkCTreeNode *node;


/* si c'est une categ et si c'est une modif du nom, on doit réafficher la liste des categ et les listes des opés, sinon, on change juste la categ et */
/* met à jour les listes de categories */
/* si c'est une sous categ, c'est obligatoirement un changement de nom */

  if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
			     "adr_struct_categ" ) )
    {
      /* c'est une categorie */

      struct struct_categ *categ;


      categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
				     "adr_struct_categ" );


      if ( strcmp ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_nom_categ ))),
		    categ -> nom_categ ))
	{
	  free ( categ -> nom_categ );

	  categ -> nom_categ = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_nom_categ ))) );


	  node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data );

	  if ( GTK_CTREE_ROW ( node ) -> expanded )
	    gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_categ ),
				      node,
				      0,
				      categ -> nom_categ,
				      10,
				      pixmap_ferme,
				      masque_ferme );
	  else
	    gtk_ctree_node_set_pixtext ( GTK_CTREE ( arbre_categ ),
				      node,
				      0,
				      categ -> nom_categ,
				      10,
				      pixmap_ouvre,
				      masque_ouvre );

	  demande_mise_a_jour_tous_comptes ();
	}
      else
	categ -> type_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_categ_debit ));
    }
  else
    {
      /* c'est une sous categorie */

      struct struct_sous_categ *sous_categ;

      sous_categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
					  "adr_struct_sous_categ" );

      free ( sous_categ -> nom_sous_categ );

      sous_categ -> nom_sous_categ = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_nom_categ ))) );


      node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data );

      gtk_ctree_node_set_text ( GTK_CTREE ( arbre_categ ),
				node,
				0,
				sous_categ -> nom_sous_categ );

      demande_mise_a_jour_tous_comptes ();
    }

  gtk_clist_unselect_all ( GTK_CLIST ( arbre_categ ));

  mise_a_jour_categ ();

  gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			     FALSE );

  modification_fichier(TRUE);
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
void clique_sur_annuler_categ ( void )
{
  if ( gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
			     "adr_struct_categ" ))
    {
      /* c'est une categorie */

      struct struct_categ *categ;

      categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
				     "adr_struct_categ" );

      gtk_entry_set_text ( GTK_ENTRY ( entree_nom_categ ),
			   categ -> nom_categ );

      if ( categ -> type_categ )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_categ_debit ),
				       TRUE );
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_categ_credit ),
				       TRUE );
    }
  else
    {
      /* c'est une sous categorie */

      struct struct_sous_categ *sous_categ;

      sous_categ =  gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
					  "adr_struct_sous_categ" );

      gtk_entry_set_text ( GTK_ENTRY ( entree_nom_categ ),
			   sous_categ -> nom_sous_categ );

    }

  gtk_widget_set_sensitive ( bouton_modif_categ_modifier,
			     FALSE );
  gtk_widget_set_sensitive ( bouton_modif_categ_annuler,
			     FALSE );

}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
void supprimer_categ ( void )
{
  struct struct_categ *categ;
  GtkCTreeNode *node;
  gint i;
  gint ope_trouvee;


  if ( !gtk_object_get_data ( GTK_OBJECT (  entree_nom_categ ),
			      "adr_struct_categ" ))
    {
      supprimer_sous_categ();
      return;
    }

  node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data );

  categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					node );

  /* fait le tour des opés pour en trouver une qui a cette catégorie */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
  ope_trouvee = 0;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GSList *liste_tmp;

      liste_tmp = LISTE_OPERATIONS;

      while ( liste_tmp )
	{
	  struct structure_operation *operation;

	  operation = liste_tmp -> data;

	  if ( operation -> categorie == categ -> no_categ )
	    {
	      ope_trouvee = 1;
	      liste_tmp = NULL;
	      i = nb_comptes;
	    }
	  else
	    liste_tmp = liste_tmp -> next;
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
      GtkWidget *bouton_categ_generique;
      GtkWidget *combofix;
      GSList *liste_combofix;
      GSList *pointeur;
      GSList *liste_tmp;
      GtkWidget *bouton_transfert;
      gint i;
      struct struct_categ *nouvelle_categ;
      struct struct_sous_categ *nouvelle_sous_categ;
      GSList *liste_categ_credit;
      GSList *liste_categ_debit;
      gchar **split_categ;
      gint nouveau_no_categ;
      gint nouveau_no_sous_categ;

      dialog = gnome_dialog_new ( _("Suppression d'une catégorie"),
					    GNOME_STOCK_BUTTON_OK,
					    GNOME_STOCK_BUTTON_CANCEL,
					     NULL);
      gnome_dialog_set_parent ( GNOME_DIALOG ( dialog ),
				GTK_WINDOW ( window ));

      label = gtk_label_new ( _("La catégorie sélectionnée contient encore des opérations.\n\nVous pouvez : ") );
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

/*       mise en place du choix tranfert vers un autre categ */
      
      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );

      bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							   _("Transférer les opérations sur la catégorie : ")  );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   bouton_transfert,
			   FALSE,
			   FALSE,
			   0 );


      pointeur = liste_struct_categories;
      liste_combofix = NULL;
      liste_categ_credit = NULL;
      liste_categ_debit = NULL;

      while ( pointeur )
	{
	  struct struct_categ *categorie;
	  GSList *sous_pointeur;

	  categorie = pointeur -> data;

	  if ( categorie-> no_categ != categ -> no_categ )
	    {
	      if ( categorie -> type_categ )
		liste_categ_debit = g_slist_append ( liste_categ_debit,
						     g_strdup ( categorie -> nom_categ ) );
	      else
		liste_categ_credit = g_slist_append ( liste_categ_credit,
						      g_strdup ( categorie -> nom_categ ) );


	      sous_pointeur = categorie -> liste_sous_categ;

	      while ( sous_pointeur )
		{
		  struct struct_sous_categ *sous_categ;

		  sous_categ = sous_pointeur -> data;

		  if ( categorie -> type_categ )
		    liste_categ_debit = g_slist_append ( liste_categ_debit,
							 g_strconcat ( "\t",
								       sous_categ -> nom_sous_categ,
								       NULL ) );
		  else
		    liste_categ_credit = g_slist_append ( liste_categ_credit,
							  g_strconcat ( "\t",
									sous_categ -> nom_sous_categ,
									NULL ) );

		  sous_pointeur = sous_pointeur -> next;
		}
	    }
	  pointeur = pointeur -> next;
	}

      /*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

      liste_combofix = g_slist_append ( liste_combofix,
						   liste_categ_debit );
      liste_combofix = g_slist_append ( liste_combofix,
						   liste_categ_credit );


      combofix = gtk_combofix_new_complex ( liste_combofix,
					    TRUE,
					    TRUE,
					    TRUE,
					    0 );
/*       gtk_widget_set_usize ( combofix, */
/* 			     300, */
/* 			     FALSE ); */
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   combofix,
			   FALSE,
			   FALSE,
			   0 );

      /*       mise en place du choix supprimer le categ */

      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );

      bouton_categ_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
								 _(" Supprimer simplement cette catégorie.") );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   bouton_categ_generique,
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
	      dialogue ( _("Veuillez entrer une catégorie !") );
	      goto retour_dialogue;
	    }

	  /* transfert des categ au niveau des listes */

	  split_categ = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
				     " : ",
				     2 );

	  nouvelle_categ = g_slist_find_custom ( liste_struct_categories,
						split_categ[0],
						(GCompareFunc) recherche_categorie_par_nom ) -> data;
	  nouveau_no_categ = nouvelle_categ -> no_categ;

	  if ( split_categ[1] )
	    {
	      nouvelle_sous_categ = g_slist_find_custom ( nouvelle_categ -> liste_sous_categ,
							  split_categ[1],
							  (GCompareFunc) recherche_sous_categorie_par_nom ) -> data;
	      nouveau_no_sous_categ = nouvelle_sous_categ -> no_sous_categ;
	    }
	  else
	    nouveau_no_sous_categ = 0;

	  g_strfreev ( split_categ );
	}
      else
	{
	  nouveau_no_categ = 0;
	  nouveau_no_sous_categ = 0;
	}


      /* on fait le tour des opés pour mettre le nouveau numéro de categ et sous_categ */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  liste_tmp = LISTE_OPERATIONS;

	  while ( liste_tmp )
	    {
	      struct structure_operation *operation;

	      operation = liste_tmp -> data;

	      if ( operation -> categorie == categ -> no_categ )
		{
		  operation -> categorie = nouveau_no_categ;
		  operation -> sous_categorie = nouveau_no_sous_categ;
		}

	      liste_tmp = liste_tmp -> next;
	    }

	  p_tab_nom_de_compte_variable++;
	}

      /* fait le tour des échéances pour mettre le nouveau numéro de categ et sous_categ  */

      liste_tmp = gsliste_echeances;

      while ( liste_tmp )
	{
	  struct operation_echeance *echeance;

	  echeance = liste_tmp -> data;

	  if ( echeance -> categorie == categ -> no_categ )
	    {
	      echeance -> categorie = nouveau_no_categ;
	      echeance -> sous_categorie = nouveau_no_sous_categ;

	      if ( !echeance -> categorie )
		echeance -> compte_virement = -1;
	    }

	  liste_tmp = liste_tmp -> next;
	}


      demande_mise_a_jour_tous_comptes ();

      gnome_dialog_close ( GNOME_DIALOG ( dialog ) );

    }


  /* supprime dans la liste des categ  */

  liste_struct_categories = g_slist_remove ( liste_struct_categories,
					     categ );
  nb_enregistrements_categories--;

  enleve_selection_ligne_categ();

  mise_a_jour_categ  ();
  remplit_arbre_categ ();

  modification_fichier(TRUE);
}
/* **************************************************************************************************** */








/* **************************************************************************************************** */
void supprimer_sous_categ ( void )
{
  struct struct_categ *categ;
  struct struct_sous_categ *sous_categ;
  GtkCTreeNode *node;
  gint i;
  gint ope_trouvee;



  node = GTK_CTREE_NODE ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data );

  sous_categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					     node );

  node = GTK_CTREE_ROW ( ( GTK_CLIST ( arbre_categ ) -> selection ) -> data ) -> parent;

  categ = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					node );

  /* fait le tour des opés pour en trouver une qui a cette sous-catégorie */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
  ope_trouvee = 0;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GSList *liste_tmp;

      liste_tmp = LISTE_OPERATIONS;

      while ( liste_tmp )
	{
	  struct structure_operation *operation;

	  operation = liste_tmp -> data;

	  if ( operation -> categorie == categ -> no_categ
	       &&
	       operation -> sous_categorie == sous_categ -> no_sous_categ )
	    {
	      ope_trouvee = 1;  modification_fichier(TRUE);

	      liste_tmp = NULL;
	      i = nb_comptes;
	    }
	  else
	    liste_tmp = liste_tmp -> next;
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
      GtkWidget *bouton_categ_generique;
      GtkWidget *combofix;
      GSList *liste_combofix;
      GSList *pointeur;
      GSList *liste_tmp;
      GtkWidget *bouton_transfert;
      gint i;
      struct struct_categ *nouvelle_categ;
      struct struct_sous_categ *nouvelle_sous_categ;
      GSList *liste_categ_credit;
      GSList *liste_categ_debit;
      gchar **split_categ;
      gint nouveau_no_categ;
      gint nouveau_no_sous_categ;

      dialog = gnome_dialog_new ( _("Suppression d'une sous-catégorie"),
					    GNOME_STOCK_BUTTON_OK,
					    GNOME_STOCK_BUTTON_CANCEL,
					     NULL);


      gnome_dialog_set_parent ( GNOME_DIALOG ( dialog ),
				GTK_WINDOW ( window ));

      label = gtk_label_new ( _("La sous-catégorie sélectionnée contient encore des opérations.\n\nVous pouvez : ") );
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

/*       mise en place du choix tranfert vers un autre sous categ */
      
      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );

      bouton_transfert = gtk_radio_button_new_with_label ( NULL,
							   _("Transférer les opérations sur la catégorie : ")  );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   bouton_transfert,
			   FALSE,
			   FALSE,
			   0 );


      pointeur = liste_struct_categories;
      liste_combofix = NULL;
      liste_categ_credit = NULL;
      liste_categ_debit = NULL;

      while ( pointeur )
	{
	  struct struct_categ *categorie;
	  GSList *sous_pointeur;



	  categorie = pointeur -> data;

	  if ( categorie -> type_categ )
	    liste_categ_debit = g_slist_append ( liste_categ_debit,
						 g_strdup ( categorie -> nom_categ ) );
	  else
	    liste_categ_credit = g_slist_append ( liste_categ_credit,
						  g_strdup ( categorie -> nom_categ ) );


	  sous_pointeur = categorie -> liste_sous_categ;

	  while ( sous_pointeur )
	    {
	      struct struct_sous_categ *sous_categorie;

	      sous_categorie = sous_pointeur -> data;



	      if ( sous_categorie -> no_sous_categ !=  sous_categ -> no_sous_categ )
		{
		  if ( categorie -> type_categ )
		    liste_categ_debit = g_slist_append ( liste_categ_debit,
							 g_strconcat ( "\t",
								       sous_categorie -> nom_sous_categ,
								       NULL ) );
		  else
		    liste_categ_credit = g_slist_append ( liste_categ_credit,
							  g_strconcat ( "\t",
									sous_categorie -> nom_sous_categ,
									NULL ) );
		}
	      sous_pointeur = sous_pointeur -> next;
	    }
	  pointeur = pointeur -> next;
	}

      /*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

      liste_combofix = g_slist_append ( liste_combofix,
						   liste_categ_debit );
      liste_combofix = g_slist_append ( liste_combofix,
						   liste_categ_credit );


      combofix = gtk_combofix_new_complex ( liste_combofix,
					    TRUE,
					    TRUE,
					    TRUE,
					    0 );
/*       gtk_widget_set_usize ( combofix, */
/* 			     300, */
/* 			     FALSE ); */
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   combofix,
			   FALSE,
			   FALSE,
			   0 );

      /*       mise en place du choix supprimer le categ */

      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );

      bouton_categ_generique = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_transfert )),
											  _(" Supprimer simplement cette sous-catégorie.") );
      gtk_box_pack_start ( GTK_BOX ( hbox ),
			   bouton_categ_generique,
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
	      dialogue ( _("Veuillez entrer une catégorie !") );
	      goto retour_dialogue;
	    }

	  /* récupère les no de categ et sous categ */

	  split_categ = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( combofix )),
				     " : ",
				     2 );

	  nouvelle_categ = g_slist_find_custom ( liste_struct_categories,
						split_categ[0],
						(GCompareFunc) recherche_categorie_par_nom ) -> data;
	  nouveau_no_categ = nouvelle_categ -> no_categ;

	  if ( split_categ[1] )
	    {
	      nouvelle_sous_categ = g_slist_find_custom ( nouvelle_categ -> liste_sous_categ,
							  split_categ[1],
							  (GCompareFunc) recherche_sous_categorie_par_nom ) -> data;
	      nouveau_no_sous_categ = nouvelle_sous_categ -> no_sous_categ;
	    }
	  else
	    nouveau_no_sous_categ = 0;

	  g_strfreev ( split_categ );
	}
      else
	{
	  nouveau_no_categ = 0;
	  nouveau_no_sous_categ = 0;
	}


      /* on fait le tour des opés pour mettre le nouveau numéro de categ et sous_categ */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  liste_tmp = LISTE_OPERATIONS;

	  while ( liste_tmp )
	    {
	      struct structure_operation *operation;

	      operation = liste_tmp -> data;

	      if ( operation -> categorie == categ -> no_categ
		   &&
		   operation -> sous_categorie == sous_categ -> no_sous_categ )
		{
		  operation -> categorie = nouveau_no_categ;
		  operation -> sous_categorie = nouveau_no_sous_categ;
		}

	      liste_tmp = liste_tmp -> next;
	    }

	  p_tab_nom_de_compte_variable++;
	}

      /* fait le tour des échéances pour mettre le nouveau numéro de categ et sous_categ  */

      liste_tmp = gsliste_echeances;


  modification_fichier(TRUE);

      while ( liste_tmp )
	{
	  struct operation_echeance *echeance;

	  echeance = liste_tmp -> data;

	  if ( echeance -> categorie == categ -> no_categ
	       &&
	       echeance -> sous_categorie == sous_categ -> no_sous_categ )
	    {
	      echeance -> categorie = nouveau_no_categ;
	      echeance -> sous_categorie = nouveau_no_sous_categ;

	      if ( !echeance -> categorie )
		echeance -> compte_virement = -1;
	    }

	  liste_tmp = liste_tmp -> next;
	}


      demande_mise_a_jour_tous_comptes ();

      gnome_dialog_close ( GNOME_DIALOG ( dialog ) );

    }


  /* supprime dans la liste des categ  */

  categ -> liste_sous_categ = g_slist_remove ( categ -> liste_sous_categ,
					       sous_categ );


  enleve_selection_ligne_categ();

  mise_a_jour_categ  ();

  remplit_arbre_categ ();

  modification_fichier(TRUE);
}
/* **************************************************************************************************** */




/***********************************************************************************************************/
/* Routine creation_liste_categories */
/* appelée lors d'un nouveau fichier */
/* crée la liste des catégories à partir de la liste ci dessus */
/* en fait, merge la liste de base avec rien */
/***********************************************************************************************************/

void creation_liste_categories ( void )
{

  liste_struct_categories = NULL;
  nb_enregistrements_categories = 0;
  no_derniere_categorie = 0;

  merge_liste_categories ();
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Routine merge_liste_categories */
/*   merge la liste de categories existante ( aucune quand vient de creation_liste_categories ) */
/* avec la liste de base du début du fichier */
/***********************************************************************************************************/

void merge_liste_categories ( void )
{
  gint i;
  gchar **categ;
  gint debit;
  struct struct_categ *categorie;

  debit = 0;

/* récupération des crédits puis des débits*/

  retour_recuperation :

  if ( debit )
    categ = categories_de_base_debit;
  else
    categ = categories_de_base_credit;

  i = 0;

  while ( categ[i] )
    {
      gchar **split_categ;
      GSList *liste_tmp;

      split_categ = g_strsplit ( categ[i],
				 " : ",
				 2 );

      liste_tmp = g_slist_find_custom ( liste_struct_categories,
					split_categ[0],
					( GCompareFunc ) recherche_categorie_par_nom );

      if ( !liste_tmp )
	{
	  /* la catégorie n'existe pas, on la crée */

	  struct struct_sous_categ *sous_categ;

	  categorie = malloc ( sizeof ( struct struct_categ ));

	  categorie -> no_categ = ++no_derniere_categorie;
	  categorie -> nom_categ = g_strdup ( split_categ[0] );
	  categorie -> type_categ = debit;
	  categorie -> no_derniere_sous_categ = 0;
	  categorie -> liste_sous_categ = NULL;

	  liste_struct_categories = g_slist_append ( liste_struct_categories,
						     categorie );
	  nb_enregistrements_categories++;

	  /* on crée la sous-catégorie si elle existe */

	  if ( split_categ[1] )
	    {
	      sous_categ = malloc ( sizeof ( struct struct_sous_categ ));

	      sous_categ -> no_sous_categ = ++(categorie -> no_derniere_sous_categ );
	      sous_categ -> nom_sous_categ = g_strdup ( split_categ[1] );

	      categorie -> liste_sous_categ = g_slist_append ( categorie -> liste_sous_categ,
							       sous_categ );
	    }
	}
      else
	{
	 /*  la catégorie existe, on vérifie si la sous-catég existe */

	  GSList *sous_liste_tmp;

	  categorie = liste_tmp -> data;

	  sous_liste_tmp = g_slist_find_custom ( categorie -> liste_sous_categ,
						 split_categ[1],
						 ( GCompareFunc ) recherche_sous_categorie_par_nom );

	  if ( !sous_liste_tmp )
	    {
	      /* la sous catégorie n'existe pas, on la crée */

	      struct struct_sous_categ *sous_categ;

	      sous_categ = malloc ( sizeof ( struct struct_sous_categ ));

	      sous_categ -> no_sous_categ = ++(categorie -> no_derniere_sous_categ );
	      sous_categ -> nom_sous_categ = g_strdup ( split_categ[1] );

	      categorie -> liste_sous_categ = g_slist_append ( categorie -> liste_sous_categ,
							       sous_categ );
	    }
	}

      /* libère le tableau créé */

      g_strfreev ( split_categ );

      i++;
    }

  /*       si on a fait les crédits, on fait les débits */

  if ( !debit )
    {
      debit = 1;
      goto retour_recuperation;
    }

}
/***********************************************************************************************************/


/***********************************************************************************************************/
/*  Routine qui crée la liste des catégories pour les combofix du formulaire et de la ventilation */
/* c'est à dire 3 listes dans 1 liste : */
/* la première contient les catégories de débit */
/* la seconde contient les catégories de crédit */
/* la troisième contient les catégories spéciales ( virement, retrait, ventilation ) */
/* la ventilation n'apparait pas dans les échéances ( et dans la ventilation ) */
/***********************************************************************************************************/

void creation_liste_categ_combofix ( void )
{
  GSList *pointeur;
  GSList *liste_categ_credit;
  GSList *liste_categ_debit;
  GSList *liste_categ_special;
  gint i;

  liste_categories_combofix = NULL;
  liste_categories_echeances_combofix = NULL;
  liste_categ_credit = NULL;
  liste_categ_debit = NULL;


  pointeur = liste_struct_categories;

  while ( pointeur )
    {
      struct struct_categ *categorie;
      GSList *sous_pointeur;

      categorie = pointeur -> data;

      if ( categorie -> type_categ )
	liste_categ_debit = g_slist_append ( liste_categ_debit,
						    g_strdup ( categorie -> nom_categ ) );
      else
	liste_categ_credit = g_slist_append ( liste_categ_credit,
						     g_strdup ( categorie -> nom_categ ) );


      sous_pointeur = categorie -> liste_sous_categ;

      while ( sous_pointeur )
	{
	  struct struct_sous_categ *sous_categ;

	  sous_categ = sous_pointeur -> data;

	  if ( categorie -> type_categ )
	    liste_categ_debit = g_slist_append ( liste_categ_debit,
							g_strconcat ( "\t",
								      sous_categ -> nom_sous_categ,
								      NULL ) );
	  else
	    liste_categ_credit = g_slist_append ( liste_categ_credit,
							 g_strconcat ( "\t",
								       sous_categ -> nom_sous_categ,
								       NULL ) );

	  sous_pointeur = sous_pointeur -> next;
	}
      pointeur = pointeur -> next;
    }


/*   on ajoute les listes des crédits / débits à la liste du combofix du formulaire */

  liste_categories_combofix = g_slist_append ( liste_categories_combofix,
					       liste_categ_debit );
  liste_categories_combofix = g_slist_append ( liste_categories_combofix,
					       liste_categ_credit );




/*   on ajoute les listes des crédits / débits à la liste du combofix des  échéances  */

  liste_categories_echeances_combofix = g_slist_append ( liste_categories_echeances_combofix,
							   liste_categ_debit );
  liste_categories_echeances_combofix = g_slist_append ( liste_categories_echeances_combofix,
							   liste_categ_credit );


  /* création des catégories spéciales : les virements et la ventilation pour le formulaire */

  liste_categ_special = NULL;

  liste_categ_special = g_slist_append ( liste_categ_special,
					 _("Opération ventilée") );

  liste_categ_special = g_slist_append ( liste_categ_special,
					 _("Virement") );

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i = 0 ; i < nb_comptes ; i++ )
    {
      liste_categ_special = g_slist_append ( liste_categ_special,
					     g_strconcat ( "\t",
							   NOM_DU_COMPTE ,
							   NULL ));
      p_tab_nom_de_compte_variable++;
    }

  liste_categories_combofix = g_slist_append ( liste_categories_combofix,
					       liste_categ_special );

  /* on saute le texte Opération ventilée */

  liste_categ_special = liste_categ_special -> next;

  liste_categories_echeances_combofix = g_slist_append ( liste_categories_echeances_combofix,
							   liste_categ_special );

}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction mise_a_jour_categ */
/* recrée les listes de catégories des combofix */
/* et remet les combofix à jour */
/***********************************************************************************************************/

void mise_a_jour_categ ( void )
{
  creation_liste_categ_combofix ();

  gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_operations[8] ),
			  liste_categories_combofix,
			  TRUE,
			  TRUE );


  gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_echeancier[6] ),
			  liste_categories_echeances_combofix,
			  TRUE,
			  TRUE );

  gtk_combofix_set_list ( GTK_COMBOFIX ( widget_formulaire_ventilation[0] ),
			  liste_categories_echeances_combofix,
			  TRUE,
			  TRUE );

  remplissage_liste_categ_etats ();
  selectionne_liste_categ_etat_courant ();

  modif_categ = 1;
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* Fonction ajoute_nouvelle_categorie */
/* appelée pour ajouter une nouvelle catég à la liste des catég */
/* entrée : la nouvelle catég */
/* retour : l'adr de la struct */
/***********************************************************************************************************/

struct struct_categ *ajoute_nouvelle_categorie ( gchar *categorie )
{
  struct struct_categ *nouvelle_categorie;

  nouvelle_categorie = calloc ( 1,
				sizeof ( struct struct_categ ));

  nouvelle_categorie -> no_categ = ++no_derniere_categorie;
  nouvelle_categorie -> nom_categ = g_strdup ( g_strstrip ( categorie ));
  nouvelle_categorie -> no_derniere_sous_categ = 0;
  nouvelle_categorie -> liste_sous_categ = NULL;

  liste_struct_categories = g_slist_append ( liste_struct_categories,
					     nouvelle_categorie );
  nb_enregistrements_categories++;

  return ( nouvelle_categorie );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction ajoute_nouvelle_sous_categorie */
/* appelée pour ajouter une nouvelle sous catég à une catég */
/* entrée : la nouvelle sous catég et l'adr de la struct de la catég correspondante */
/* retour : l'adr de la struct */
/***********************************************************************************************************/

struct struct_sous_categ *ajoute_nouvelle_sous_categorie ( gchar *sous_categorie,
							   struct struct_categ *categorie )
{
  struct struct_sous_categ *nouvelle_sous_categorie;

  nouvelle_sous_categorie = malloc ( sizeof ( struct struct_sous_categ ));

  nouvelle_sous_categorie -> no_sous_categ = ++( categorie->no_derniere_sous_categ );
  nouvelle_sous_categorie -> nom_sous_categ = g_strdup ( g_strstrip ( sous_categorie ));

  categorie -> liste_sous_categ = g_slist_append ( categorie -> liste_sous_categ,
						   nouvelle_sous_categorie );

  return ( nouvelle_sous_categorie );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Fonction recherche_categorie_par_nom */
/***********************************************************************************************************/

gint recherche_categorie_par_nom ( struct struct_categ *categorie,
				   gchar *categ )
{
  return ( g_strcasecmp ( categorie -> nom_categ,
			  categ ) );
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* Fonction recherche_sous_categorie_par_nom */
/***********************************************************************************************************/

gint recherche_sous_categorie_par_nom ( struct struct_sous_categ *sous_categorie,
					gchar *sous_categ )
{
  return ( g_strcasecmp ( sous_categorie -> nom_sous_categ,
			  sous_categ ) );
}
/***********************************************************************************************************/



/***************************************************************************************************/
/* Fonction  recherche_categorie_par_no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/
  
gint recherche_categorie_par_no ( struct struct_categ *categorie,
				  gint *no_categorie )
{
  return (  categorie ->  no_categ != GPOINTER_TO_INT ( no_categorie ) );
}
/***************************************************************************************************/


/***************************************************************************************************/
/* Fonction  recherche_categorie_par_no */
/* appelée par un g_slist_find_custom */
/* donne en arg la struct du tiers et le no du tiers recherché */
/***************************************************************************************************/
  
gint recherche_sous_categorie_par_no ( struct struct_sous_categ *sous_categorie,
				  gint *no_sous_categorie )
{
  return (  sous_categorie ->  no_sous_categ != GPOINTER_TO_INT ( no_sous_categorie ) );
}
/***************************************************************************************************/





/* **************************************************************************************************** */
/* crée un tableau de categ et de sous categ aussi gds que le nb de tiers */
/* et les renvoie dans un tab de 2 pointeurs */
/* **************************************************************************************************** */

void calcule_total_montant_categ ( void )
{
  gint i;
  struct struct_devise *devise_compte;

  /* on crée les tableaux de montant */

  /* le +1 pour reserver le [0] pour aucune catégorie */

  tab_montant_categ = calloc ( nb_enregistrements_categories + 1,
			       sizeof ( gfloat ));
  nb_ecritures_par_categ = calloc ( nb_enregistrements_categories + 1,
				    sizeof ( gint ));

  tab_montant_sous_categ = calloc ( nb_enregistrements_categories,
				    sizeof ( gpointer ));
  nb_ecritures_par_sous_categ = calloc ( nb_enregistrements_categories,
					 sizeof ( gpointer ));

  /*   la devise par défaut a été choisie dans les paramètres */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  devise_compte = g_slist_find_custom ( liste_struct_devises,
					GINT_TO_POINTER ( no_devise_totaux_tiers ),
					( GCompareFunc ) recherche_devise_par_no) -> data;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GSList *liste_tmp;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

      liste_tmp = LISTE_OPERATIONS;

      while ( liste_tmp )
	{
	  struct structure_operation *operation;
	  struct struct_devise *devise_operation;
	  gdouble montant;
	  gint place_categ;


	  operation = liste_tmp -> data;

	  /* on commence par calculer le montant dans la devise demandée */

	  if ( operation -> devise == devise_compte -> no_devise )
	    montant = operation -> montant;
	  else
	    {
	      /* ce n'est pas la devise par défaut */
	      /* si le compte passe à l'euro et que la devise est l'euro, utilise la conversion du compte, */
	      /* si c'est une devise qui passe à l'euro et que la devise du compte est l'euro, utilise la conversion du compte */
	      /* sinon utilise la conversion stockée dans l'opé */

	      devise_operation = g_slist_find_custom ( liste_struct_devises,
						       GINT_TO_POINTER ( operation -> devise ),
						       ( GCompareFunc ) recherche_devise_par_no ) -> data;

	      if ( devise_compte -> passage_euro
		   &&
		   !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		montant = operation -> montant * devise_compte -> change;
	      else
		if ( devise_operation -> passage_euro
		     &&
		     !strcmp ( devise_compte -> nom_devise, _("Euro") ))
		  montant = operation -> montant / devise_operation -> change;
		else
		  if ( operation -> une_devise_compte_egale_x_devise_ope )
		    montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		  else
		    montant = operation -> montant * operation -> taux_change - operation -> frais_change;

	      montant = ( rint (montant * 100 )) / 100;
	    }

	  /* on traite ensuite l'opération */
	
	  if ( operation -> categorie )
	    {
	      struct struct_categ *categorie;

	      /* il y a une catégorie */

	      categorie = g_slist_find_custom ( liste_struct_categories,
						GINT_TO_POINTER ( operation -> categorie ),
						(GCompareFunc) recherche_categorie_par_no ) -> data;

	      /* recherche la place du tiers dans la liste */

	      place_categ = g_slist_index ( liste_struct_categories,
					    categorie );

	      /* crée la place pour les sous catég de cette categ si ce n'est déjà fait */

	      if ( !tab_montant_sous_categ[place_categ] )
		{
		  gint nb_sous_categ;

		  nb_sous_categ = g_slist_length ( categorie -> liste_sous_categ );

		  /* on réserve nb_sous_categ + 1 pour aucune sous categ qui sera en [0] */

		  tab_montant_sous_categ[place_categ] = calloc ( nb_sous_categ + 1,
								 sizeof ( float ));
		  nb_ecritures_par_sous_categ[place_categ] = calloc ( nb_sous_categ + 1,
								      sizeof ( gint ));
		}

	      tab_montant_categ[place_categ+1] = tab_montant_categ[place_categ+1] + montant;
	      nb_ecritures_par_categ[place_categ+1]++;

	      /* on ajoute maintenant le montant à la sous categ si elle existe */

	      if ( operation -> sous_categorie )
		{
		  gint place_sous_categ;

		  place_sous_categ = g_slist_position ( categorie -> liste_sous_categ,
							g_slist_find_custom ( categorie -> liste_sous_categ,
									      GINT_TO_POINTER ( operation -> sous_categorie ),
									      (GCompareFunc) recherche_sous_categorie_par_no ));
		  tab_montant_sous_categ[place_categ][place_sous_categ+1] = tab_montant_sous_categ[place_categ][place_sous_categ+1] + montant;
		  nb_ecritures_par_sous_categ[place_categ][place_sous_categ+1]++;
		}
	      else
		{
		  if ( tab_montant_sous_categ[place_categ] )
		    {
		      tab_montant_sous_categ[place_categ][0] = tab_montant_sous_categ[place_categ][0] + montant;
		      nb_ecritures_par_sous_categ[place_categ][0]++;
		    }
		}
	    }
	  else
	      /* il n'y a pas de catégorie */
	      /* on met le montant dans tab_montant_categ[0} si e n'est ni un virement ni une ventil */
	    if ( !operation -> relation_no_operation
		 &&
		 !operation -> operation_ventilee )
	      {
		tab_montant_categ[0] = tab_montant_categ[0] + montant;
		nb_ecritures_par_categ[0]++;
	      }
	  liste_tmp = liste_tmp -> next;
	}
    }
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
gchar *calcule_total_montant_categ_par_compte ( gint categ,
						gint sous_categ,
						gint no_compte )
{
  gdouble retour_int;
  struct struct_devise *devise_compte;
  GSList *liste_tmp;

  retour_int = 0;
  nb_ecritures_par_comptes = 0;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

  devise_compte = g_slist_find_custom ( liste_struct_devises,
					GINT_TO_POINTER ( no_devise_totaux_tiers ),
					( GCompareFunc ) recherche_devise_par_no) -> data;

  liste_tmp = LISTE_OPERATIONS;

  while ( liste_tmp )
    {
      struct structure_operation *operation;

      operation = liste_tmp -> data;

      if ( operation -> categorie == categ
	   &&
	   operation -> sous_categorie == sous_categ
	   &&
	   !operation -> relation_no_operation
	   &&
	   !operation -> operation_ventilee   )
	{
	  struct struct_devise *devise_operation;
	  gdouble montant;

	  if ( operation -> devise == devise_compte -> no_devise )
	    montant = operation -> montant;
	  else
	    {
	      /* ce n'est pas la devise du compte, si le compte passe à l'euro et que la devise est l'euro, utilise la conversion du compte, */
	      /* si c'est une devise qui passe à l'euro et que la devise du compte est l'euro, utilise la conversion du compte */
	      /* sinon utilise la conversion stockée dans l'opé */

	      devise_operation = g_slist_find_custom ( liste_struct_devises,
						       GINT_TO_POINTER ( operation -> devise ),
						       ( GCompareFunc ) recherche_devise_par_no ) -> data;

	      if ( devise_compte -> passage_euro
		   &&
		   !strcmp ( devise_operation -> nom_devise, _("Euro") ) )
		montant = operation -> montant * devise_compte -> change;
	      else
		if ( devise_operation -> passage_euro
		     &&
		     !strcmp ( devise_compte -> nom_devise, _("Euro") ))
		  montant = operation -> montant / devise_operation -> change;
		else
		  if ( operation -> une_devise_compte_egale_x_devise_ope )
		    montant = operation -> montant / operation -> taux_change - operation -> frais_change;
		  else
		    montant = operation -> montant * operation -> taux_change - operation -> frais_change;

	      montant = ( rint (montant * 100 )) / 100;
	    }

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
void appui_sur_ajout_categorie ( void )
{
  gchar *nom_categorie;
  struct struct_categ *nouvelle_categorie;
  gchar *text[4];
  GtkCTreeNode *ligne;

  if ( !( nom_categorie = demande_texte ( _("Nouvelle catégorie"),
					   _("Entrer le nom de la nouvelle catégorie :") )))
    return;

  /* on l'ajoute à la liste des opés */
  
  nouvelle_categorie = ajoute_nouvelle_categorie ( nom_categorie );


  /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

  text[0] = nouvelle_categorie -> nom_categ;
  text[1] = NULL;
  text[2] = NULL;
  text[3] = NULL;

  ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
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

  /* on associe à ce categorie à l'adr de sa struct */

  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
				ligne,
				nouvelle_categorie );

  gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_categ ),
			     NULL );

  mise_a_jour_categ();
  modif_categ = 0;
  modification_fichier(TRUE);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void appui_sur_ajout_sous_categorie ( void )
{
  gchar *nom_sous_categorie;
  struct struct_sous_categ *nouvelle_sous_categorie;
  struct struct_categ *categorie;
  gchar *text[4];
  GtkCTreeNode *ligne;
  GtkCTreeNode *node_parent;

  if ( !( nom_sous_categorie = demande_texte ( _("Nouvelle sous-catégorie"),
					       _("Entrer le nom de la nouvelle sous-catégorie :") )))
    return;

  /* récupère le node parent */

  node_parent = GTK_CLIST ( arbre_categ ) -> selection -> data;

  while ( GTK_CTREE_ROW ( node_parent ) -> level != 1 )
    node_parent = GTK_CTREE_ROW ( node_parent ) -> parent;

  /* on récupère l'categorie parente */

  categorie = gtk_ctree_node_get_row_data ( GTK_CTREE ( arbre_categ ),
					     node_parent );

  /* on l'ajoute à la liste des opés */
  
  nouvelle_sous_categorie = ajoute_nouvelle_sous_categorie ( nom_sous_categorie,
							     categorie );


  /* on l'ajoute directement au ctree et on fait le tri pour éviter de toute la réafficher */

  text[0] = nouvelle_sous_categorie -> nom_sous_categ;
  text[1] = NULL;
  text[2] = NULL;
  text[3] = NULL;

  ligne = gtk_ctree_insert_node ( GTK_CTREE ( arbre_categ ),
				  node_parent,
				  NULL,
				  text,
				  10,
				  NULL,
				  NULL,
				  NULL,
				  NULL,
				  FALSE,
				  FALSE );

  /* on associe à ce categorie à l'adr de sa struct */

  gtk_ctree_node_set_row_data ( GTK_CTREE ( arbre_categ ),
				ligne,
				nouvelle_sous_categorie );

  gtk_ctree_sort_recursive ( GTK_CTREE ( arbre_categ ),
			     NULL );


  mise_a_jour_categ();
  modif_categ = 0;
  modification_fichier(TRUE);
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void exporter_categ ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *fenetre_nom;
  gint resultat;
  struct stat test_fichier;
  gchar *nom_categ;

  dialog = gnome_dialog_new ( _("Exporter les catégories"),
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
				     ".cgsb",
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
      nom_categ = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))))));

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));

      /* vérification que c'est possible */

      if ( !strlen ( nom_categ ))
	return;

      if ( stat ( nom_categ,
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
	      dialogue ( g_strconcat ( _("Nom de fichier \""),
				       nom_categ,
				       _("\" invalide !"),
				       NULL ));
	      return;
	    }
	}

      if ( !enregistre_categ ( nom_categ ))
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
/* **************************************************************************************************** */



/* **************************************************************************************************** */
void importer_categ ( void )
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *fenetre_nom;
  gint resultat;
  gchar *nom_categ;
  GtkWidget *bouton_merge_remplace;
  GtkWidget *menu;
  GtkWidget *menu_item;
  GtkWidget *separateur;


  dialog = gnome_dialog_new ( _("Importer une liste de catégories"),
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
				     ".cgsb",
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


  /* on permet de remplacer/fusionner la liste */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  /* pour éviter un warning lors de la compil */

  bouton_merge_remplace = NULL;

  if ( no_derniere_operation )
    {
      /*       il y a déjà des opérations dans le fichier, on ne peut que fusionner */

      label = gtk_label_new ( "Le fichier contient déjà des opérations,\nles deux listes de catégories seront fusionnées." );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );
    }
  else
    {
      label = gtk_label_new ( "Voulez-vous :" );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      bouton_merge_remplace = gtk_option_menu_new ();
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   bouton_merge_remplace,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( bouton_merge_remplace );

      menu = gtk_menu_new ();

      menu_item = gtk_menu_item_new_with_label ( "Fusionner les deux listes de catégories" );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "choix",
			    NULL );
      gtk_menu_append ( GTK_MENU ( menu ),
			menu_item );
      gtk_widget_show ( menu_item );

      menu_item = gtk_menu_item_new_with_label ( "Remplacer l'ancienne liste de catégories" );
      gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			    "choix",
			    GINT_TO_POINTER ( 1 ));
      gtk_menu_append ( GTK_MENU ( menu ),
			menu_item );
      gtk_widget_show ( menu_item );

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_merge_remplace ),
				 menu );
      gtk_widget_show ( menu );
    }


  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  switch ( resultat )
    {
    case 0 :
      nom_categ = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( gnome_file_entry_gtk_entry ( GNOME_FILE_ENTRY ( fenetre_nom ))))));

      gnome_dialog_close ( GNOME_DIALOG ( dialog ));

      /* vérification que c'est possible */

      if ( !strlen ( nom_categ ))
	return;

      /* si on a choisi de remplacer l'ancienne liste, on la vire ici */

      if ( !no_derniere_operation
	   &&
	   gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_merge_remplace ) -> menu_item ),
				 "choix" ))
	{
	  g_slist_free ( liste_struct_categories );
	  liste_struct_categories = NULL;
	  no_derniere_categorie = 0;
	  nb_enregistrements_categories = 0;
	}

      if ( !charge_categ ( nom_categ ))
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
/* **************************************************************************************************** */
