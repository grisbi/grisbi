/*  Fichier qui s'occupe de la configuration des états */
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
  N_("Toutes"),
  N_("Personnalisé"),
  N_("Cumul à ce jour"),
  N_("Mois en cours"),
  N_("Année en cours"),
  N_("Cumul mensuel"),
  N_("Cumul annuel"),
  N_("Mois précédent"),
  N_("Année précédente"),
  N_("30 derniers jours"),
  N_("3 derniers mois"),
  N_("6 derniers mois"),
  N_("12 derniers mois"),
  NULL };

gchar *jours_semaine[] = {
  N_("Lundi"),
  N_("Mardi"),
  N_("Mercredi"),
  N_("Jeudi"),
  N_("Vendredi"),
  N_("Samedi"),
  N_("Dimanche"),
  NULL };




/*****************************************************************************************************/
/* Fontion personnalistation_etat */
/* affiche la fenetre de personnalisation */
/*****************************************************************************************************/

void personnalisation_etat (void)
{
  GtkWidget *notebook;
  GtkWidget *separateur;
  GtkWidget *bouton;
  GtkWidget *hbox;
  GtkCTreeNode *parent;
  GList *pointeur_liste;

  if ( !etat_courant )
    return;


  if ( !onglet_config_etat )
    {

      onglet_config_etat = gtk_vbox_new ( FALSE,
					5 );
      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_etats ),
				 onglet_config_etat,
				 gtk_label_new ( _( "Configuration états" )));
      gtk_widget_show ( onglet_config_etat );

      notebook = gtk_notebook_new ();
  
      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_generalites (),
				 gtk_label_new (_(" Généralités ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_dates (),
				 gtk_label_new (_(" Dates ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_comptes (),
				 gtk_label_new (_(" Comptes ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_virements (),
				 gtk_label_new (_(" Virements ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_categories (),
				 gtk_label_new (_(" Catégories ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_ib (),
				 gtk_label_new (_(" Imputations budgétaires ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_tiers (),
				 gtk_label_new (_(" Tiers ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_texte (),
				 gtk_label_new (_(" Textes ")) );

      gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				 onglet_etat_montant (),
				 gtk_label_new (_(" Montants ")) );

      gtk_notebook_set_scrollable ( GTK_NOTEBOOK ( notebook ), TRUE );

      gtk_box_pack_start ( GTK_BOX ( onglet_config_etat ),
			   notebook,
			   TRUE,
			   TRUE,
			   0 );
      gtk_widget_show ( notebook );

      separateur = gtk_hseparator_new ();
      gtk_box_pack_start ( GTK_BOX ( onglet_config_etat ),
			   separateur,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( separateur );

      hbox = gtk_hbox_new ( FALSE,
			    5 );
      gtk_box_pack_start ( GTK_BOX ( onglet_config_etat ),
			   hbox,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( hbox );

      bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL );
      gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			      GTK_RELIEF_NONE );
      gtk_signal_connect ( GTK_OBJECT ( bouton ),
			   "clicked",
			   GTK_SIGNAL_FUNC ( annule_modif_config ),
			   NULL );
      gtk_box_pack_end ( GTK_BOX ( hbox ),
			   bouton,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( bouton);

      bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_APPLY );
      gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			      GTK_RELIEF_NONE );
      gtk_signal_connect ( GTK_OBJECT ( bouton ),
			   "clicked",
			   GTK_SIGNAL_FUNC ( recuperation_info_perso_etat ),
			   NULL );
      gtk_box_pack_end ( GTK_BOX ( hbox ),
			   bouton,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( bouton );
    }


  /* on va maintenant remplir toutes les infos de l'état */


  /* onglet généralités */


  /* on met le nom de l'état */

  gtk_entry_set_text ( GTK_ENTRY ( entree_nom_etat ),
		       etat_courant -> nom_etat );

  /* on remplit le ctree en fonction du classement courant */

  pointeur_liste = etat_courant -> type_classement;
  parent = NULL;
  gtk_clist_clear ( GTK_CLIST ( liste_type_classement_etat ));

  while ( pointeur_liste )
    {
      gchar *text[1];

      text[0] = NULL;

      switch ( GPOINTER_TO_INT ( pointeur_liste -> data ))
	{
	case 1:
	  text[0] = _("Catégorie");
	  break;

	case 3:
	  text[0] = _("Imputation budgétaire");
	  break;

	case 5:
	  text[0] = _("Compte");
	  break;

	case 6:
	  text[0] = _("Tiers");
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

  if ( etat_courant -> afficher_r )
    {
      if ( etat_courant -> afficher_r == 1 )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_non_r_etat ),
				       TRUE );
      else
	if ( etat_courant -> afficher_r )
	  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_etat ),
					 TRUE );
    }
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat ),
				   TRUE );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_opes ),
				 etat_courant -> afficher_opes );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_ope ),
				 etat_courant -> afficher_no_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_date_opes ),
				 etat_courant -> afficher_date_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_tiers_opes ),
				 etat_courant -> afficher_tiers_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_categ_opes ),
				 etat_courant -> afficher_categ_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ_opes ),
				 etat_courant -> afficher_sous_categ_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_type_ope ),
				 etat_courant -> afficher_type_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_ib_opes ),
				 etat_courant -> afficher_ib_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib_opes ),
				 etat_courant -> afficher_sous_ib_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_cheque ),
				 etat_courant -> afficher_cheque_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_notes_opes ),
				 etat_courant -> afficher_notes_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pc_opes ),
				 etat_courant -> afficher_pc_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_no_rappr ),
				 etat_courant -> afficher_rappr_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_infobd_opes ),
				 etat_courant -> afficher_infobd_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_pas_detailler_ventilation ),
				 etat_courant -> pas_detailler_ventilation );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_exo_opes ),
				 etat_courant -> afficher_exo_ope );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_titres_colonnes ),
				 etat_courant -> afficher_titre_colonnes );
  if ( !etat_courant -> type_affichage_titres )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_titre_en_haut ),
				   TRUE );

  /* on rend insensitif les sous qque choses si nécessaire */

  sens_desensitive_pointeur ( bouton_afficher_opes,
			      frame_onglet_generalites );
  sens_desensitive_pointeur ( bouton_afficher_categ_opes,
			      bouton_afficher_sous_categ_opes );
  sens_desensitive_pointeur ( bouton_afficher_ib_opes,
			      bouton_afficher_sous_ib_opes );
  sens_desensitive_pointeur ( bouton_afficher_titres_colonnes,
			      bouton_titre_changement );
  sens_desensitive_pointeur ( bouton_afficher_titres_colonnes,
			      bouton_titre_en_haut );

  /* mise en forme de la devise */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_general_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat_courant -> devise_de_calcul_general ),
									 ( GCompareFunc ) recherche_devise_par_no )));

  /* onglet dates */


  if ( etat_courant -> exo_date )
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ),
				     TRUE );
      gtk_widget_set_sensitive ( vbox_utilisation_date,
				 FALSE );
    }
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_dates ),
				   TRUE );


  if ( etat_courant -> utilise_detail_exo == 3 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_exo_etat ),
				   TRUE );
  else
    {
      gtk_widget_set_sensitive ( vbox_generale_exo_etat,
				 FALSE );

      if ( etat_courant -> utilise_detail_exo )
	{
	  if ( etat_courant -> utilise_detail_exo == 1 )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_courant ),
					   TRUE );
	  else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_precedent ),
					   TRUE );
	}
      else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exo_tous ),
				       TRUE );
    }


  /* on sélectionne les exercices */

  selectionne_liste_exo_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_exo_etat ),
				 etat_courant -> separation_par_exo );

  /* on sélectionne la plage de date */

  gtk_clist_select_row ( GTK_CLIST ( liste_plages_dates_etat ),
			 etat_courant -> no_plage_date,
			 0 );

  if ( etat_courant -> no_plage_date != 1 )
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

  if ( etat_courant -> date_perso_debut )
    gtk_entry_set_text ( GTK_ENTRY ( entree_date_init_etat ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( etat_courant -> date_perso_debut ),
					   g_date_month ( etat_courant -> date_perso_debut ),
					   g_date_year ( etat_courant -> date_perso_debut )));

  if ( etat_courant -> date_perso_fin )
    gtk_entry_set_text ( GTK_ENTRY ( entree_date_finale_etat ),
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( etat_courant -> date_perso_fin ),
					   g_date_month ( etat_courant -> date_perso_fin ),
					   g_date_year ( etat_courant -> date_perso_fin )));

  /* on remplit les détails de la séparation des dates */

  if ( etat_courant -> separation_par_plage )
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_separe_plages_etat ),
				     TRUE );
      gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				 TRUE );
      modif_type_separation_dates ( GINT_TO_POINTER ( etat_courant -> type_separation_plage ));
    }
  else
    {
      gtk_widget_set_sensitive ( bouton_type_separe_plages_etat,
				 FALSE );
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 FALSE );
    }

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ),
				etat_courant -> type_separation_plage );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_debut_semaine ),
				etat_courant -> jour_debut_semaine );


  /* onglet comptes */

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_comptes_etat ),
				 etat_courant -> utilise_detail_comptes );

  sens_desensitive_pointeur ( bouton_detaille_comptes_etat,
			      vbox_generale_comptes_etat );


  selectionne_liste_comptes_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat ),
				 etat_courant -> regroupe_ope_par_compte );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte ),
				 etat_courant -> affiche_sous_total_compte );

  sens_desensitive_pointeur ( bouton_regroupe_ope_compte_etat,
			      bouton_affiche_sous_total_compte );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_comptes ),
				 etat_courant -> afficher_nom_compte );


  /* onglet virements */


  if ( etat_courant -> type_virement )
    {
      if ( etat_courant -> type_virement == 1 )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat ),
				       TRUE );
      else
	{
	  if ( etat_courant -> type_virement == 2 )
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat ),
					   TRUE );
	  else
	    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_perso ),
					   TRUE );
	}
    }
  else
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements ),
				     TRUE );
      gtk_widget_set_sensitive ( bouton_exclure_non_virements_etat,
				 FALSE );
    }

  sens_desensitive_pointeur ( bouton_inclusion_virements_perso,
			      liste_comptes_virements );

  selectionne_liste_virements_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_non_virements_etat ),
				 etat_courant -> exclure_ope_non_virement );


  /* onglet catégories */

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_categ_etat ),
				 etat_courant -> utilise_categ );

  sens_desensitive_pointeur ( bouton_utilise_categ_etat,
			      vbox_generale_categ_etat );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_categ_etat ),
				 etat_courant -> utilise_detail_categ );

  sens_desensitive_pointeur ( bouton_detaille_categ_etat,
			      hbox_detaille_categ_etat );


  if ( etat_courant -> utilise_detail_categ )

  /* on sélectionne les catégories choisies */

  selectionne_liste_categ_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_categ ),
				 etat_courant -> exclure_ope_sans_categ );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ ),
				 etat_courant -> affiche_sous_total_categ );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ ),
				 etat_courant -> afficher_sous_categ );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ ),
				 etat_courant -> affiche_sous_total_sous_categ );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_categ ),
				 etat_courant -> afficher_pas_de_sous_categ );
  sens_desensitive_pointeur ( bouton_afficher_sous_categ,
			      bouton_affiche_sous_total_sous_categ );
  sens_desensitive_pointeur ( bouton_afficher_sous_categ,
			      bouton_afficher_pas_de_sous_categ );

  /* mise en forme de la devise */

  selectionne_devise_categ_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_categ ),
				 etat_courant -> afficher_nom_categ );


  /* onglet ib */

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_ib_etat ),
				 etat_courant -> utilise_ib );

  sens_desensitive_pointeur ( bouton_utilise_ib_etat,
			      vbox_generale_ib_etat );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_ib_etat ),
				 etat_courant -> utilise_detail_ib );

  sens_desensitive_pointeur ( bouton_detaille_ib_etat,
			      hbox_detaille_ib_etat );


  /* on sélectionne les ib choisies */

  selectionne_liste_ib_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_ib ),
				 etat_courant -> exclure_ope_sans_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ),
				 etat_courant -> affiche_sous_total_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_ib ),
				 etat_courant -> afficher_sous_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ),
				 etat_courant -> affiche_sous_total_sous_ib );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_ib ),
				 etat_courant -> afficher_pas_de_sous_ib );
  sens_desensitive_pointeur ( bouton_afficher_sous_ib,
			      bouton_affiche_sous_total_sous_ib );
  sens_desensitive_pointeur ( bouton_afficher_sous_ib,
			      bouton_afficher_pas_de_sous_ib );

  /* mise en forme de la devise */

  selectionne_devise_ib_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_ib ),
				 etat_courant -> afficher_nom_ib );

  /* onglet tiers */

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_utilise_tiers_etat ),
				 etat_courant -> utilise_tiers );

  sens_desensitive_pointeur ( bouton_utilise_tiers_etat,
			      vbox_generale_tiers_etat );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_detaille_tiers_etat ),
				 etat_courant -> utilise_detail_tiers );

  sens_desensitive_pointeur ( bouton_detaille_tiers_etat,
			      hbox_detaille_tiers_etat );

 
  /* on sélectionne les tiers choisies */

  selectionne_liste_tiers_etat_courant();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers ),
				 etat_courant -> affiche_sous_total_tiers );

  /* mise en forme de la devise */

  selectionne_devise_tiers_etat_courant ();

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_tiers ),
				 etat_courant -> afficher_nom_tiers );

  /*  onglet texte */

  if ( etat_courant -> texte )
    gtk_entry_set_text ( GTK_ENTRY ( entree_texte_etat ),
			 g_strstrip ( etat_courant -> texte ));


  /* onglet montant */

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclut_choix_positif_negatif_etat ),
				 etat_courant -> utilise_montant_neg_pos );
  sens_desensitive_pointeur ( bouton_inclut_choix_positif_negatif_etat,
			      bouton_choix_positif_negatif_etat );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_positif_negatif_etat ),
				etat_courant -> type_neg_pos );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclut_choix_valeur_etat ),
				 etat_courant -> utilise_valeur );
  sens_desensitive_pointeur ( bouton_inclut_choix_valeur_etat,
			      hbox_choix_valeur_etat );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_operateur_valeur_etat ),
				etat_courant -> type_operateur_valeur );
  gtk_entry_set_text ( GTK_ENTRY ( entree_choix_valeur_etat ),
		       g_strdup_printf ( "%4.2f",
					 etat_courant -> montant_valeur ));

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_inclut_choix_inclusion_valeur_etat ),
				 etat_courant -> utilise_inclusion );
  sens_desensitive_pointeur ( bouton_inclut_choix_inclusion_valeur_etat,
			      hbox_choix_inclut_etat );
  gtk_entry_set_text ( GTK_ENTRY ( entree_choix_inclut_inf_etat ),
		       g_strdup_printf ( "%4.2f",
					 etat_courant -> montant_inclusion_inf ));
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_operateur_inclut_inf_etat ),
				etat_courant -> type_operateur_inf_inclusion );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_operateur_inclut_sup_etat ),
				etat_courant -> type_operateur_sup_inclusion );
  gtk_entry_set_text ( GTK_ENTRY ( entree_choix_inclut_sup_etat ),
		       g_strdup_printf ( "%4.2f",
					 etat_courant -> montant_inclusion_sup ));
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_choix_montant_nul ),
				etat_courant -> choix_montant_nul );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_montant_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat_courant -> choix_devise_montant ),
									 ( GCompareFunc ) recherche_devise_par_no )));


  /* on se met sur la bonne page */

  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_etats ),
			  1 );

  /* on empêche le changement d'état */

  gtk_widget_set_sensitive ( frame_liste_etats,
			     FALSE );

}
/*****************************************************************************************************/


/*****************************************************************************************************/
void annule_modif_config ( void )
{

  gtk_widget_set_sensitive ( frame_liste_etats,
			     TRUE );
  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_etats ),
			  0 );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
void selectionne_liste_exo_etat_courant ( void )
{
  GSList *pointeur_sliste;

  if ( !etat_courant )
    return;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_exo_etat ));

  pointeur_sliste = etat_courant -> no_exercices;

  while ( pointeur_sliste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_exo_etat ),
			     g_slist_position ( liste_struct_exercices,
						g_slist_find_custom ( liste_struct_exercices,
								      pointeur_sliste -> data,
								      (GCompareFunc) recherche_exercice_par_no )),
			     0 );
      pointeur_sliste = pointeur_sliste -> next;
    }
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void selectionne_liste_comptes_etat_courant ( void )
{
  GSList *pointeur_sliste;

  if ( !etat_courant )
    return;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_etat ));

  pointeur_sliste = etat_courant -> no_comptes;

  while ( pointeur_sliste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_comptes_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_comptes_etat ),
							    pointeur_sliste -> data ),
			     0 );
      pointeur_sliste = pointeur_sliste -> next;
    }
}
/*****************************************************************************************************/





/*****************************************************************************************************/
void selectionne_liste_virements_etat_courant ( void )
{
  GSList *pointeur_sliste;

  if ( !etat_courant )
    return;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_comptes_virements ));

  pointeur_sliste = etat_courant -> no_comptes_virements;

  while ( pointeur_sliste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_comptes_virements ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_comptes_virements ),
							    pointeur_sliste -> data ),
			     0 );
      pointeur_sliste = pointeur_sliste -> next;
    }
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void selectionne_liste_categ_etat_courant ( void )
{
  GSList *pointeur_sliste;

  if ( !etat_courant )
    return;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_categ_etat ));

  pointeur_sliste = etat_courant -> no_categ;

  while ( pointeur_sliste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_categ_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_categ_etat ),
							    pointeur_sliste -> data ),
			     0 );
      pointeur_sliste = pointeur_sliste -> next;
    }
}
/*****************************************************************************************************/

/*****************************************************************************************************/
void selectionne_devise_categ_etat_courant ( void )
{

  if ( !etat_courant )
    return;

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_categ_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat_courant -> devise_de_calcul_categ ),
									 ( GCompareFunc ) recherche_devise_par_no )));
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void selectionne_liste_ib_etat_courant ( void )
{
  GSList *pointeur_sliste;

  if ( !etat_courant )
    return;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_ib_etat ));

  pointeur_sliste = etat_courant -> no_ib;

  while ( pointeur_sliste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_ib_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_ib_etat ),
							    pointeur_sliste -> data ),
			     0 );
      pointeur_sliste = pointeur_sliste -> next;
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void selectionne_devise_ib_etat_courant ( void )
{

  if ( !etat_courant )
    return;

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_ib_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat_courant -> devise_de_calcul_ib ),
									 ( GCompareFunc ) recherche_devise_par_no )));
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void selectionne_liste_tiers_etat_courant ( void )
{
  GSList *pointeur_sliste;

  if ( !etat_courant )
    return;

  gtk_clist_unselect_all ( GTK_CLIST ( liste_tiers_etat ));

  pointeur_sliste = etat_courant -> no_tiers;

  while ( pointeur_sliste )
    {
      gtk_clist_select_row ( GTK_CLIST ( liste_tiers_etat ),
			     gtk_clist_find_row_from_data ( GTK_CLIST ( liste_tiers_etat ),
							    pointeur_sliste -> data ),
			     0 );
      pointeur_sliste = pointeur_sliste -> next;
    }
}
/*****************************************************************************************************/





/*****************************************************************************************************/
void selectionne_devise_tiers_etat_courant ( void )
{

  if ( !etat_courant )
    return;

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( etat_courant -> devise_de_calcul_tiers ),
									 ( GCompareFunc ) recherche_devise_par_no )));
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void recuperation_info_perso_etat ( void )
{
  GList *pointeur_liste;
  gchar *pointeur_char;
  gint i;


  /* vérification que les dates init et finales sont correctes */

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_init_etat ))))
       &&
       !modifie_date ( entree_date_init_etat ))
    {
      dialogue ( _("La date initiale personnelle est invalide") );
      return;
    }

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_date_finale_etat ))))
       &&
       !modifie_date ( entree_date_finale_etat ))
    {
      dialogue ( _("La date finale personnelle est invalide") );
      return;
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

      gtk_label_set_text ( GTK_LABEL ( label_etat_courant ),
			   etat_courant -> nom_etat );

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

  /* récupération de l'affichage ou non des R */

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_r_et_non_r_etat )) )
    etat_courant -> afficher_r = 0;
  else
    {
      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_opes_non_r_etat )) )
	etat_courant -> afficher_r = 1;
      else
	etat_courant -> afficher_r = 2;
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
  etat_courant -> afficher_exo_ope = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_exo_opes ));
  etat_courant -> afficher_titre_colonnes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_titres_colonnes ));
  etat_courant -> type_affichage_titres = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_titre_changement ));

  etat_courant -> devise_de_calcul_general = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_general_etat ) -> menu_item ),
										   "no_devise" ));
  /* récupération des dates */

  etat_courant -> exo_date = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( radio_button_utilise_exo ));

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_tous )) )
    etat_courant -> utilise_detail_exo = 0;
  else
    {
      if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_courant )) )
	etat_courant -> utilise_detail_exo = 1;
      else
	{
	  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exo_precedent )) )
	    etat_courant -> utilise_detail_exo = 2;
	  else
	    etat_courant -> utilise_detail_exo = 3;
	}
    }

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
       etat_courant -> utilise_detail_exo == 3 )
    {
      dialogue ( _("Tous les exercices ont été sélectionnés ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les exercices utilisés\"") );
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
      dialogue ( _("Tous les comptes ont été sélectionnés ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les comptes utilisés\"") );
      etat_courant -> utilise_detail_comptes = FALSE;
    }
  
  etat_courant -> regroupe_ope_par_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_regroupe_ope_compte_etat ));
  etat_courant -> affiche_sous_total_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_compte ));
  etat_courant -> afficher_nom_compte = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_comptes ));


  /* récupération des virements */

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_actifs_etat )))
    etat_courant -> type_virement = 1;
  else
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclusion_virements_hors_etat )))
      etat_courant -> type_virement = 2;
    else
      {
	if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_non_inclusion_virements )))
	  etat_courant -> type_virement = 0;
	else
	  etat_courant -> type_virement = 3;
      }

  if ( etat_courant -> no_comptes_virements )
    {
      g_slist_free ( etat_courant -> no_comptes_virements );
      etat_courant -> no_comptes_virements = NULL;
    }

  pointeur_liste = GTK_CLIST ( liste_comptes_virements ) -> selection;

  while ( pointeur_liste )
    {
      etat_courant -> no_comptes_virements = g_slist_append ( etat_courant -> no_comptes_virements,
							      gtk_clist_get_row_data ( GTK_CLIST ( liste_comptes_virements ),
										       GPOINTER_TO_INT ( pointeur_liste -> data )));
      pointeur_liste = pointeur_liste -> next;
    }

  etat_courant -> exclure_ope_non_virement = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_non_virements_etat ));



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
      dialogue ( _("Toutes les catégories ont été sélectionnées ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les catégories utilisées\"") );
      etat_courant -> utilise_detail_categ = FALSE;
    }
  
  etat_courant -> exclure_ope_sans_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_categ ));
  etat_courant -> affiche_sous_total_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_categ ));
  etat_courant -> afficher_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_sous_categ ));
  etat_courant -> affiche_sous_total_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_categ ));
  etat_courant -> afficher_pas_de_sous_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_categ ));

  etat_courant -> devise_de_calcul_categ = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_categ_etat ) -> menu_item ),
										   "no_devise" ));
  etat_courant -> afficher_nom_categ = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_categ ));


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
      dialogue ( _("Toutes les imputations budgétaires ont été sélectionnées ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les imputations budgétaires utilisées\"") );
      etat_courant -> utilise_detail_ib = FALSE;
    }
  
  etat_courant -> exclure_ope_sans_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_exclure_ope_sans_ib ));
  etat_courant -> affiche_sous_total_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_ib ));
  etat_courant -> affiche_sous_total_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_sous_ib ));
  etat_courant -> afficher_pas_de_sous_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_pas_de_sous_ib ));

  etat_courant -> devise_de_calcul_ib = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_ib_etat ) -> menu_item ),
										   "no_devise" ));
  etat_courant -> afficher_nom_ib = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_ib ));



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
      dialogue ( _("Tous les tiers ont été sélectionnés ; Grisbi sera plus rapide\nen retirant l'option \"Détailler les tiers utilisés\"") );
      etat_courant -> utilise_detail_tiers = FALSE;
    }
  
  etat_courant -> affiche_sous_total_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_sous_total_tiers ));

  etat_courant -> devise_de_calcul_tiers = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_tiers_etat ) -> menu_item ),
										   "no_devise" ));
  etat_courant -> afficher_nom_tiers = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_afficher_noms_tiers ));


  /* récupération du texte */

  if ( strlen ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_texte_etat )))))
    etat_courant -> texte = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_texte_etat ))));
  else
    etat_courant -> texte = NULL;

  /* récupération du montant */

  etat_courant -> utilise_montant_neg_pos = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclut_choix_positif_negatif_etat ));
  etat_courant -> type_neg_pos = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_positif_negatif_etat ) -> menu_item ),
									 "no_item" ));
  etat_courant -> utilise_valeur = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclut_choix_valeur_etat ));
  etat_courant -> type_operateur_valeur = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_operateur_valeur_etat ) -> menu_item ),
										  "no_item" ));
  etat_courant -> montant_valeur = g_strtod ( gtk_entry_get_text ( GTK_ENTRY ( entree_choix_valeur_etat )),
					      NULL );
  etat_courant -> utilise_inclusion = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( bouton_inclut_choix_inclusion_valeur_etat ));
  etat_courant -> montant_inclusion_inf = g_strtod ( gtk_entry_get_text ( GTK_ENTRY ( entree_choix_inclut_inf_etat )),
						     NULL );
  etat_courant -> type_operateur_inf_inclusion = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_operateur_inclut_inf_etat ) -> menu_item ),
											 "no_item" ));
  etat_courant -> type_operateur_sup_inclusion = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_operateur_inclut_sup_etat ) -> menu_item ),
											 "no_item" ));
  etat_courant -> montant_inclusion_sup = g_strtod ( gtk_entry_get_text ( GTK_ENTRY ( entree_choix_inclut_sup_etat )),
						     NULL );

  etat_courant -> choix_montant_nul = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_choix_montant_nul ) -> menu_item ),
									      "no_montant_nul" ));
  etat_courant -> choix_devise_montant = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_devise_montant_etat ) -> menu_item ),
									      "no_devise" ));


  modification_fichier ( TRUE );

  /* on réaffiche l'état */

  rafraichissement_etat ( etat_courant );

  /* on repasse à la 1ère page du notebook */

  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_etats ),
			  0 );
  gtk_widget_set_sensitive ( frame_liste_etats,
			     TRUE );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_generalites ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *scrolled_window;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *fleche;
  GtkWidget *separateur;
  GtkWidget *table;
  GtkWidget *vbox_onglet;
  GtkWidget *hbox_haut;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );

  /* choix du nom du rapport */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Nom de l'état : ") );
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

  /*   hbox du haut de la fenetre, le type de classement, le choix sur les R */

  hbox_haut = gtk_hbox_new ( FALSE,
			     5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox_haut,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox_haut );

  /* choix du type de classement */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_haut ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  frame = gtk_frame_new ( _("Choix du type de classement :") );
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

  
  /* on peut sélectionner les opé R ou non R */

  frame = gtk_frame_new ( _( "Opérations rapprochées" ));
  gtk_box_pack_start ( GTK_BOX ( hbox_haut ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   5 );
  gtk_container_add ( GTK_CONTAINER ( frame ),
		      vbox );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( "Sélectionner :" );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_opes_r_et_non_r_etat = gtk_radio_button_new_with_label ( NULL,
				       _("Toutes les opérations") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_opes_r_et_non_r_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_opes_r_et_non_r_etat );

  bouton_opes_non_r_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_opes_r_et_non_r_etat )),
							   _("Les opérations non rapprochées") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_opes_non_r_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_opes_non_r_etat );

  bouton_opes_r_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_opes_r_et_non_r_etat )),
							   _("Les opérations rapprochées") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_opes_r_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_opes_r_etat );




  /* afficher ou non les opés */

  bouton_afficher_opes = gtk_check_button_new_with_label ( _("Afficher les opérations") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_afficher_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_opes );

  /* demande les détails affichés dans les opérations */

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  frame_onglet_generalites = gtk_frame_new ( _("Afficher les informations sur") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       frame_onglet_generalites,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( frame_onglet_generalites );


  /* connection pour rendre sensitif la frame */

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_opes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       frame_onglet_generalites );


  table = gtk_table_new ( 9,
			  3,
			  FALSE );
  gtk_container_add ( GTK_CONTAINER ( frame_onglet_generalites ),
		      table );
  gtk_widget_show ( table );


  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 1,
			      0, 1 );
  gtk_widget_show ( hbox );

  bouton_afficher_no_ope = gtk_check_button_new_with_label ( _("le n° d'opération") );
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

  bouton_afficher_date_opes = gtk_check_button_new_with_label ( _("la date") );
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

  bouton_afficher_tiers_opes = gtk_check_button_new_with_label ( _("le tiers") );
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

  bouton_afficher_categ_opes = gtk_check_button_new_with_label ( _("la catégorie") );
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

  bouton_afficher_sous_categ_opes = gtk_check_button_new_with_label ( _("la sous-catégorie") );
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

  bouton_afficher_type_ope = gtk_check_button_new_with_label ( _("le type d'opération") );
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

  bouton_afficher_ib_opes = gtk_check_button_new_with_label ( _("l'imputation budgétaire") );
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

  bouton_afficher_sous_ib_opes = gtk_check_button_new_with_label ( _("la sous-imputation budgétaire") );
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

  bouton_afficher_no_cheque = gtk_check_button_new_with_label ( _("le n° de chèque/virement") );
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

  bouton_afficher_notes_opes = gtk_check_button_new_with_label ( _("les notes") );
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

  bouton_afficher_pc_opes = gtk_check_button_new_with_label ( _("la pièce comptable") );
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

  bouton_afficher_no_rappr = gtk_check_button_new_with_label ( _("le n° de rapprochement") );
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

  bouton_afficher_infobd_opes = gtk_check_button_new_with_label ( _("l'information banque-guichet") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_infobd_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_infobd_opes );

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      1, 2,
			      4, 5 );
  gtk_widget_show ( hbox );

  bouton_afficher_exo_opes = gtk_check_button_new_with_label ( _("l'exercice") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_exo_opes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_exo_opes );


  /* on propose d'afficher le titre des colonnes */

  separateur = gtk_hseparator_new ();
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			     separateur,
			      0, 3,
			      5, 6 );
  gtk_widget_show ( separateur );

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 3,
			      6, 7 );
  gtk_widget_show ( hbox );

  bouton_afficher_titres_colonnes = gtk_check_button_new_with_label ( _("Afficher les titres des colonnes") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_afficher_titres_colonnes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_titres_colonnes );

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 3,
			      7, 8 );
  gtk_widget_show ( hbox );

  bouton_titre_changement = gtk_radio_button_new_with_label ( NULL,
							      _("à chaque changement de section") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_titre_changement,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_titre_changement );

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_table_attach_defaults ( GTK_TABLE ( table ),
			      hbox,
			      0, 3,
			      8, 9 );
  gtk_widget_show ( hbox );

  bouton_titre_en_haut = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_titre_changement )),
							   _("en haut de l'état") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_titre_en_haut,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_titre_en_haut );


  /* on met les connections */

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_categ_opes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_afficher_sous_categ_opes );
  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_ib_opes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_afficher_sous_ib_opes );
  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_titres_colonnes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_titre_changement );
  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_titres_colonnes ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_titre_en_haut );


  bouton_pas_detailler_ventilation = gtk_check_button_new_with_label ( _("Ne pas détailler les opérations ventilées") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_pas_detailler_ventilation,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_pas_detailler_ventilation );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Devise utilisée pour les totaux généraux :") );
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
GtkWidget *onglet_etat_dates ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *separateur;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  gchar **plages_dates;
  gint i;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *menu;
  GtkWidget *menu_item;
  GtkWidget *hbox_onglet;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  hbox_onglet = gtk_hbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( hbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  hbox_onglet );
  gtk_widget_show ( hbox_onglet );


  /*   on met en forme la partie de gauche : utilisation des exercices */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  radio_button_utilise_exo = gtk_radio_button_new_with_label ( NULL,
							       _("Utiliser les exercices") );
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


  /*   on met le détail, exo courant ou précédent */

  bouton_exo_tous = gtk_radio_button_new_with_label ( NULL,
						      _( "Tous les exercices" ));
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
		       bouton_exo_tous,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exo_tous );

  bouton_exo_courant = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
							 _( "Exercice courant" ));
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
		       bouton_exo_courant,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exo_courant );

  bouton_exo_precedent = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
							   _( "Exercice précédent" ));
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
		       bouton_exo_precedent,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exo_precedent );



  bouton_detaille_exo_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_exo_tous )),
							       _("Détailler les exercices utilisés") );
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

  remplissage_liste_exo_etats ();


  bouton_separe_exo_etat = gtk_check_button_new_with_label ( _("Séparer les résultats par exercice") );
  gtk_box_pack_start ( GTK_BOX ( vbox_utilisation_exo ),
		       bouton_separe_exo_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_separe_exo_etat );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


  /* mise en place de la plage de dates */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_onglet ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  radio_button_utilise_dates = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( radio_button_utilise_exo )),
								 _("Utiliser des plages de dates") );
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

  /* on remplit la liste des dates */

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

  label = gtk_label_new ( _("Date initiale : ") );
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

  label = gtk_label_new ( _("Date finale : ") );
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


  bouton_separe_plages_etat = gtk_check_button_new_with_label ( _("Séparer les résultats par période") );
  gtk_signal_connect ( GTK_OBJECT ( bouton_separe_plages_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( change_separation_result_periode ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_separe_plages_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_separe_plages_etat );

  /* mise en place de la ligne type - choix perso */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

 

  bouton_type_separe_plages_etat = gtk_option_menu_new ();

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _("Semaine") );
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

  menu_item = gtk_menu_item_new_with_label ( _("Mois") );
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

  menu_item = gtk_menu_item_new_with_label ( _("Année") );
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

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_type_separe_plages_etat ),
			     menu );
  gtk_widget_show ( menu );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_type_separe_plages_etat,
		       FALSE, 
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_type_separe_plages_etat );


  /* mise en place de la ligne de début de semaine */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("La semaine commence le ") );
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
			    _("jour"),
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

      bouton = gtk_button_new_with_label ( _("Annuler") );
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
/*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
/* 				 FALSE ); */
/*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
/* 				 FALSE ); */
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
/*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
/* 				 FALSE ); */
/*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
/* 				 FALSE ); */
      break;

    case 1:
    case 2:
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 FALSE );
/*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
/* 				 FALSE ); */
/*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
/* 				 FALSE ); */
      break;

    case 3:
      gtk_widget_set_sensitive ( bouton_debut_semaine,
				 FALSE );
/*       gtk_widget_set_sensitive ( bouton_type_separe_perso_etat, */
/* 				 TRUE ); */
/*       gtk_widget_set_sensitive ( entree_separe_perso_etat, */
/* 				 TRUE ); */
    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void remplissage_liste_exo_etats ( void )
{
  GSList *pointeur_liste;

  if ( !onglet_config_etat )
    return;

  gtk_clist_clear ( GTK_CLIST ( liste_exo_etat ) );

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
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_comptes ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *hbox;
  GtkWidget *vbox_onglet;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );



  /* on met dans la partie de gauche une liste contenant les comptes à */
  /* sélectionner */

  bouton_detaille_comptes_etat = gtk_check_button_new_with_label ( _("Détailler les comptes utilisés") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_detaille_comptes_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_detaille_comptes_etat );

  vbox_generale_comptes_etat = gtk_vbox_new ( FALSE,
					      5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       vbox_generale_comptes_etat,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox_generale_comptes_etat );


  gtk_signal_connect ( GTK_OBJECT ( bouton_detaille_comptes_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       vbox_generale_comptes_etat );

  label = gtk_label_new ( _("Sélectionner les comptes à inclure dans l'état :") );
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

  remplissage_liste_comptes_etats ();

      

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  bouton = gtk_button_new_with_label ( _("Sélectionner tout") );
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

  bouton = gtk_button_new_with_label ( _("Désélectionner tout") );
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


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
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

  bouton_regroupe_ope_compte_etat = gtk_check_button_new_with_label ( _("Regrouper les opérations par compte") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_regroupe_ope_compte_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_regroupe_ope_compte_etat );

  bouton_affiche_sous_total_compte = gtk_check_button_new_with_label ( _("Afficher un sous-total lors d'un changement de compte") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_compte );

  gtk_signal_connect ( GTK_OBJECT ( bouton_regroupe_ope_compte_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_compte );

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox );

  bouton_afficher_noms_comptes = gtk_check_button_new_with_label ( _("Afficher le nom du compte") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_noms_comptes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_noms_comptes );



  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void remplissage_liste_comptes_etats ( void )
{
  gint i;

  if ( !onglet_config_etat )
    return;

  gtk_clist_clear ( GTK_CLIST ( liste_comptes_etat ) );

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
}
/*****************************************************************************************************/






/*****************************************************************************************************/
GtkWidget *onglet_etat_virements ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *scrolled_window;
  GtkWidget *vbox_onglet;
  GtkWidget *hbox;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );


  /*   on met les boutons d'inclusion ou non des virements */

  bouton_non_inclusion_virements = gtk_radio_button_new_with_label ( NULL,
								     _("Ne pas inclure les virements") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_non_inclusion_virements,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_non_inclusion_virements );

  bouton_inclusion_virements_actifs_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									     _("Inclure les virements de ou vers les comptes d'actif et de passif") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_inclusion_virements_actifs_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclusion_virements_actifs_etat );

  bouton_inclusion_virements_hors_etat = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									   _("Inclure les virements de ou vers les comptes ne figurant pas dans l'état") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_inclusion_virements_hors_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclusion_virements_hors_etat );

  bouton_inclusion_virements_perso = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( bouton_non_inclusion_virements )),
									   _("Inclure les virements de ou vers les comptes  : ") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_inclusion_virements_perso,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclusion_virements_perso );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
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
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( scrolled_window );

  liste_comptes_virements = gtk_clist_new ( 1 );
  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_comptes_virements ),
				 GTK_SELECTION_MULTIPLE );
  gtk_clist_set_column_auto_resize ( GTK_CLIST ( liste_comptes_virements ),
				     0,
				     TRUE );
  gtk_widget_set_usize ( liste_comptes_virements,
			 300,
			 FALSE );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
		      liste_comptes_virements );
  gtk_widget_show ( liste_comptes_virements );

  /* on remplit la liste des comptes */

  remplissage_liste_comptes_virements ();

  gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_perso ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       liste_comptes_virements );


  /* on rajoute le bouton exclure les opé non virement */

  bouton_exclure_non_virements_etat = gtk_check_button_new_with_label ( _("Exclure les opérations qui ne sont pas des virements") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_exclure_non_virements_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exclure_non_virements_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_perso ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_exclure_non_virements_etat );
  gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_hors_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_exclure_non_virements_etat );
  gtk_signal_connect ( GTK_OBJECT ( bouton_inclusion_virements_actifs_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_exclure_non_virements_etat );


  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
void remplissage_liste_comptes_virements ( void )
{
  gint i;

  if ( !onglet_config_etat )
    return;

  gtk_clist_clear ( GTK_CLIST ( liste_comptes_virements ) );

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      gchar *nom[1];
      gint ligne;

      nom[0] = NOM_DU_COMPTE;

      ligne = gtk_clist_append ( GTK_CLIST ( liste_comptes_virements ),
				 nom );

      gtk_clist_set_row_data ( GTK_CLIST ( liste_comptes_virements ),
			       ligne,
			       GINT_TO_POINTER ( NO_COMPTE ));
      p_tab_nom_de_compte_variable++;
    }
}
/*****************************************************************************************************/





/*****************************************************************************************************/
GtkWidget *onglet_etat_categories ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;
  GtkWidget *separateur;
  GtkWidget *hbox;
  GtkWidget *vbox_onglet;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );



  bouton_utilise_categ_etat = gtk_check_button_new_with_label ( _("Utiliser les catégories dans l'état") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_utilise_categ_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utilise_categ_etat );

  vbox_generale_categ_etat = gtk_vbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       vbox_generale_categ_etat,
		       TRUE,
		       TRUE,
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

  bouton_detaille_categ_etat = gtk_check_button_new_with_label ( _("Détailler les catégories utilisées") );
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
		       TRUE,
		       TRUE,
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

  label = gtk_label_new ( _("Sélectionner les catégories à inclure dans l'état :") );
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

  remplissage_liste_categ_etats ();

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_categ_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  bouton = gtk_button_new_with_label ( _("Sélectionner tout") );
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

  bouton = gtk_button_new_with_label ( _("Désélectionner tout") );
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

  bouton = gtk_button_new_with_label ( _("Catégories de revenus") );
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

  bouton = gtk_button_new_with_label ( _("Catégories de dépenses") );
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



  bouton_exclure_ope_sans_categ = gtk_check_button_new_with_label ( _("Exclure les opérations sans catégorie") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_exclure_ope_sans_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exclure_ope_sans_categ );

  bouton_affiche_sous_total_categ = gtk_check_button_new_with_label ( _("Afficher un sous-total lors du changement de catégorie") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_categ );

  /* mise en place du bouton pour afficher les sous categ */

  bouton_afficher_sous_categ = gtk_check_button_new_with_label ( _("Afficher les sous-catégories") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_sous_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_categ );

  bouton_affiche_sous_total_sous_categ = gtk_check_button_new_with_label ( _("Afficher un sous-total lors du changement de sous-catégorie") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_sous_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_sous_categ );

  bouton_afficher_pas_de_sous_categ = gtk_check_button_new_with_label ( _("Afficher \"Pas de sous-catégorie\" si absente") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_pas_de_sous_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_pas_de_sous_categ );

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_categ ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_sous_categ );
  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_categ ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_afficher_pas_de_sous_categ );


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

  label = gtk_label_new ( _("Devise utilisée pour le calcul :") );
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

  bouton_afficher_noms_categ = gtk_check_button_new_with_label ( _("Afficher le nom de la (sous-)catégorie") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_noms_categ,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_noms_categ );




  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


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
void remplissage_liste_categ_etats ( void )
{
  GSList *pointeur_liste;

  if ( !onglet_config_etat )
    return;

  gtk_clist_clear ( GTK_CLIST ( liste_categ_etat ) );

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

}
/*****************************************************************************************************/




/*****************************************************************************************************/
GtkWidget *onglet_etat_ib ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;
  GtkWidget *hbox;
  GtkWidget *separateur;
  GtkWidget *vbox_onglet;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );



  bouton_utilise_ib_etat = gtk_check_button_new_with_label ( _("Utiliser les imputations budgétaires dans l'état") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_utilise_ib_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utilise_ib_etat );

  vbox_generale_ib_etat = gtk_vbox_new ( FALSE,
					 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
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

  bouton_detaille_ib_etat = gtk_check_button_new_with_label ( _("Détailler les imputations budgétaires utilisées") );
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

  label = gtk_label_new ( _("Sélectionner les imputations budgétaires à inclure dans l'état :") );
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

  remplissage_liste_ib_etats ();

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_ib_etat ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );

  bouton = gtk_button_new_with_label ( _("Sélectionner tout") );
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

  bouton = gtk_button_new_with_label ( _("Désélectionner tout") );
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

  bouton = gtk_button_new_with_label ( _("I.B. de revenus") );
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

  bouton = gtk_button_new_with_label ( _("I.B. de dépenses") );
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

  bouton_exclure_ope_sans_ib = gtk_check_button_new_with_label ( _("Exclure les opérations sans imputation budgétaire") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_exclure_ope_sans_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_exclure_ope_sans_ib );

  bouton_affiche_sous_total_ib = gtk_check_button_new_with_label ( _("Afficher un sous-total lors du changement d'imputation") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_ib );

  /* mise en place du bouton pour afficher les sous categ */

  bouton_afficher_sous_ib = gtk_check_button_new_with_label ( _("Afficher les sous-imputations") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_sous_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_sous_ib );

  bouton_affiche_sous_total_sous_ib = gtk_check_button_new_with_label ( _("Afficher un sous-total lors du changement de sous-imputation") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_affiche_sous_total_sous_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_sous_ib );

  bouton_afficher_pas_de_sous_ib = gtk_check_button_new_with_label ( _("Afficher \"Pas de sous-imputation\" si absente") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_pas_de_sous_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_pas_de_sous_ib );

  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_ib ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_affiche_sous_total_sous_ib );
  gtk_signal_connect ( GTK_OBJECT ( bouton_afficher_sous_ib ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_afficher_pas_de_sous_ib );

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

  label = gtk_label_new ( _("Devise utilisée pour le calcul :") );
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

  bouton_afficher_noms_ib = gtk_check_button_new_with_label ( _("Afficher le nom de la (sous-)imputation") );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       bouton_afficher_noms_ib,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_noms_ib );


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
void remplissage_liste_ib_etats ( void )
{
  GSList *pointeur_liste;

  if ( !onglet_config_etat )
    return;

  gtk_clist_clear ( GTK_CLIST ( liste_ib_etat ) );

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
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_tiers ( void )
{
  GtkWidget *widget_retour;

  GtkWidget *frame;
  GtkWidget *vbox;
  GtkWidget *vbox2;
  GtkWidget *label;
  GtkWidget *scrolled_window;
  GtkWidget *bouton;
  GtkWidget *hbox;
  GtkWidget *hbox2;
  GtkWidget *vbox_onglet;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );



  bouton_utilise_tiers_etat = gtk_check_button_new_with_label ( _("Utiliser les tiers dans l'état") );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       bouton_utilise_tiers_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_utilise_tiers_etat );

  vbox_generale_tiers_etat = gtk_vbox_new ( FALSE,
					    5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
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

  bouton_detaille_tiers_etat = gtk_check_button_new_with_label ( _("Détailler les tiers utilisés") );
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

  label = gtk_label_new ( _("Sélectionner les tiers à inclure dans l'état :") );
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

  remplissage_liste_tiers_etats ();

  /*   sur la partie de droite, on met les boutons (dé)sélectionner tout */

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_detaille_tiers_etat ),
		       vbox2,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox2 );

  bouton = gtk_button_new_with_label ( _("Sélectionner tout") );
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

  bouton = gtk_button_new_with_label ( _("Désélectionner tout") );
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


  hbox2 = gtk_hbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox2,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox2 );

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       vbox2,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox2 );

  bouton_affiche_sous_total_tiers = gtk_check_button_new_with_label ( _("Afficher un sous-total lors du changement de tiers") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_affiche_sous_total_tiers,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_sous_total_tiers );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Devise utilisée pour le calcul :") );
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

  vbox2 = gtk_vbox_new ( FALSE,
			 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       vbox2,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox2 );

  bouton_afficher_noms_tiers = gtk_check_button_new_with_label ( _("Afficher le nom du tiers") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       bouton_afficher_noms_tiers,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_afficher_noms_tiers );


  return ( widget_retour );
}
/*****************************************************************************************************/

/*****************************************************************************************************/
void remplissage_liste_tiers_etats ( void )
{
  GSList *pointeur_liste;

  if ( !onglet_config_etat )
    return;

  gtk_clist_clear ( GTK_CLIST ( liste_tiers_etat ) );

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


}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_texte ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *vbox_onglet;

  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Réduire la recherche aux opérations contenant ce texte (notes) :") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
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


  return ( widget_retour );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
GtkWidget *onglet_etat_montant ( void )
{
  GtkWidget *widget_retour;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *vbox_onglet;
  GtkWidget *menu;
  GtkWidget *menu_item;


  widget_retour = gtk_scrolled_window_new ( FALSE,
					    FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( widget_retour ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_widget_show ( widget_retour );


  vbox_onglet = gtk_vbox_new ( FALSE,
				 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox_onglet ),
				   10 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( widget_retour ),
					  vbox_onglet );
  gtk_widget_show ( vbox_onglet );



  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Réduire la recherche aux opérations :") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  /* mise en place du choix montant positif ou négatif */


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton_inclut_choix_positif_negatif_etat = gtk_check_button_new_with_label ( _("dont le montant est " ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_inclut_choix_positif_negatif_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclut_choix_positif_negatif_etat );

  bouton_choix_positif_negatif_etat = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_choix_positif_negatif_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_positif_negatif_etat );

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _( "négatif" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "positif" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_positif_negatif_etat ),
			     menu );
  gtk_widget_show ( menu );


  gtk_signal_connect ( GTK_OBJECT ( bouton_inclut_choix_positif_negatif_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       bouton_choix_positif_negatif_etat );

  /* mise en place du choix montant = < > ... à une valeur */


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton_inclut_choix_valeur_etat = gtk_check_button_new_with_label ( _("dont le montant est " ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_inclut_choix_valeur_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclut_choix_valeur_etat );

  hbox_choix_valeur_etat = gtk_hbox_new ( FALSE,
					  5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox_choix_valeur_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox_choix_valeur_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_inclut_choix_valeur_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       hbox_choix_valeur_etat );


  bouton_choix_operateur_valeur_etat = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_choix_valeur_etat ),
		       bouton_choix_operateur_valeur_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_operateur_valeur_etat );

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _( "=" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "<" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "<=" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			GINT_TO_POINTER (2) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( ">" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			GINT_TO_POINTER (3) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( ">=" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			GINT_TO_POINTER (4) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_operateur_valeur_etat ),
			     menu );
  gtk_widget_show ( menu );


  entree_choix_valeur_etat = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_choix_valeur_etat ),
		       entree_choix_valeur_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_choix_valeur_etat );



  /* mise en place du choix montant inclut  */


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton_inclut_choix_inclusion_valeur_etat = gtk_check_button_new_with_label ( _("dans lesquelles " ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_inclut_choix_inclusion_valeur_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_inclut_choix_inclusion_valeur_etat );

  hbox_choix_inclut_etat = gtk_hbox_new ( FALSE,
					  5 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox_choix_inclut_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox_choix_inclut_etat );

  gtk_signal_connect ( GTK_OBJECT ( bouton_inclut_choix_inclusion_valeur_etat ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( sens_desensitive_pointeur ),
		       hbox_choix_inclut_etat );

  entree_choix_inclut_inf_etat = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_choix_inclut_etat ),
		       entree_choix_inclut_inf_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_choix_inclut_inf_etat );


  bouton_choix_operateur_inclut_inf_etat = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_choix_inclut_etat ),
		       bouton_choix_operateur_inclut_inf_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_operateur_inclut_inf_etat );

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _( "<" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "<=" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_operateur_inclut_inf_etat ),
			     menu );
  gtk_widget_show ( menu );


  label = gtk_label_new ( _( " montant " ));
  gtk_box_pack_start ( GTK_BOX ( hbox_choix_inclut_etat ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  bouton_choix_operateur_inclut_sup_etat = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_choix_inclut_etat ),
		       bouton_choix_operateur_inclut_sup_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_operateur_inclut_sup_etat );

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _( "<" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "<=" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_item",
			GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_operateur_inclut_sup_etat ),
			     menu );
  gtk_widget_show ( menu );

  entree_choix_inclut_sup_etat = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox_choix_inclut_etat ),
		       entree_choix_inclut_sup_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( entree_choix_inclut_sup_etat );

  /* choix pour le montant nul */


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _( "Un montant nul est " ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_choix_montant_nul = gtk_option_menu_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_choix_montant_nul,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_choix_montant_nul );

  menu = gtk_menu_new ();

  menu_item = gtk_menu_item_new_with_label ( _( "est à la fois positif et négatif" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_montant_nul",
			NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "est toujours exclu" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_montant_nul",
			GINT_TO_POINTER (1));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "est considéré comme négatif" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_montant_nul",
			GINT_TO_POINTER (2));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  menu_item = gtk_menu_item_new_with_label ( _( "est considéré comme positif" ));
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_montant_nul",
			GINT_TO_POINTER (3));
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_widget_show ( menu_item );

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_choix_montant_nul ),
			     menu );
  gtk_widget_show ( menu );


  /* choix pour le montant nul */


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox_onglet ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _( "Devise utilisée pour les comparaisons de montant " ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  bouton_devise_montant_etat = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_devise_montant_etat ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_devise_montant_etat,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_devise_montant_etat );



  return ( widget_retour );
}
/*****************************************************************************************************/


