/* fichier qui s'occupe de tout ce qui concerne l'échéancier */

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




/* ****************************************************************************************************** */
GtkWidget *creation_onglet_echeancier ( void )
{
  GtkWidget *onglet;
  GtkWidget *frame;
  GtkWidget *vbox;



  onglet = gtk_hbox_new ( FALSE,
			  10 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


  /* création de la partie gauche avec le calendrier et les boutons */

  frame = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			       GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show (frame);

  gtk_container_add ( GTK_CONTAINER ( frame ),
		      creation_partie_gauche_echeancier () );


/* création de la partie droite avec la liste et le formulaire */


  vbox = gtk_vbox_new ( FALSE,
			10 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       vbox,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox );


/* création de la partie de la liste  */

  frame = gtk_frame_new ( NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame,
		       TRUE,
		       TRUE,
		       0 );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame ),
			       GTK_SHADOW_IN );
  gtk_widget_show (frame);



  gtk_container_add ( GTK_CONTAINER ( frame ),
		      creation_liste_echeances () );


  /* création du formulaire de l'échéancier */

  frame_formulaire_echeancier = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_formulaire_echeancier ),
			       GTK_SHADOW_IN );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       frame_formulaire_echeancier,
		       FALSE,
		       FALSE,
		       0 );

  if ( etat.formulaire_echeancier_toujours_affiche )
    gtk_widget_show ( frame_formulaire_echeancier );

  etat.formulaire_echeance_dans_fenetre = 0;

  gtk_container_add ( GTK_CONTAINER ( frame_formulaire_echeancier ),
		      creation_formulaire_echeancier () );

  return ( onglet );
}
/* ****************************************************************************************************** */


/* ****************************************************************************************************** */
GtkWidget *creation_partie_gauche_echeancier ( void )
{
  GtkWidget *vbox;
  GtkWidget *separation;
  GtkWidget *hbox;
  GtkWidget *hbox2;
  GtkWidget *icone;
  GtkWidget *label;
  time_t temps;
  GtkWidget *menu;
  GtkWidget *bouton;
  GtkWidget *item;

  vbox = gtk_vbox_new ( FALSE,
			10 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_widget_show ( vbox );


  /* création du calendrier */

  time ( &temps );
      
  calendrier_echeances = gtk_calendar_new ();
  gtk_calendar_select_month ( GTK_CALENDAR ( calendrier_echeances ),
			      localtime ( &temps ) -> tm_mon,
			      localtime ( &temps ) -> tm_year + 1900 );
  mise_a_jour_calendrier ();
  gtk_signal_connect ( GTK_OBJECT ( calendrier_echeances ),
		       "month-changed",
		       GTK_SIGNAL_FUNC ( mise_a_jour_calendrier ),
		       NULL );
  gtk_signal_connect_after ( GTK_OBJECT ( calendrier_echeances ),
		       "day-selected",
		       GTK_SIGNAL_FUNC ( click_sur_jour_calendrier_echeance ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       calendrier_echeances,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( calendrier_echeances );


  /* mise en place du choix de l'affichage */

  separation = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separation,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( separation );

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  label = gtk_label_new ( _("Affichage des échéances :") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       label,
		       FALSE,
		       FALSE,
		       5 );
  gtk_widget_show ( label );


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  bouton = gtk_option_menu_new ();
  menu = gtk_menu_new();

  item = gtk_menu_item_new_with_label ( _("Du mois") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Des 2 prochains mois") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("De l'année") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (2) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  item = gtk_menu_item_new_with_label ( _("Toutes") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (3) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  item = gtk_menu_item_new_with_label ( _("Personnalisées") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (4) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			     menu );
  gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton ),
				affichage_echeances );

  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  /* on ajoute en dessous une hbox de personalisation */

  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  /* le menu jour / mois / année */

  bouton_personnalisation_affichage_echeances = gtk_option_menu_new ();
  menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ( _("Jours") );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"intervalle_perso",
			GINT_TO_POINTER ( 0 ));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Mois") );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"intervalle_perso",
			GINT_TO_POINTER ( 1 ));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Années") );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"intervalle_perso",
			GINT_TO_POINTER ( 2 ));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton_personnalisation_affichage_echeances ),
			     menu );
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_personnalisation_affichage_echeances ) -> menu ),
			      "selection-done",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER ( 6 ) );
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       bouton_personnalisation_affichage_echeances,
		       FALSE,
		       FALSE,
		       0 );


  /* la gtk entry de personnalisation */

  entree_personnalisation_affichage_echeances = gtk_entry_new ();
  gtk_signal_connect_object ( GTK_OBJECT ( entree_personnalisation_affichage_echeances ),
			      "focus_out_event",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (5) );
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       entree_personnalisation_affichage_echeances,
		       FALSE,
		       FALSE,
		       0 );

  /* affiche si nécessire les paramètres perso */

  if ( affichage_echeances == 4 )
    {
      if ( affichage_echeances_perso_nb_libre )
	gtk_entry_set_text ( GTK_ENTRY ( entree_personnalisation_affichage_echeances ),
			     itoa ( affichage_echeances_perso_nb_libre ));
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_personnalisation_affichage_echeances ),
				    affichage_echeances_perso_j_m_a );
      gtk_widget_show ( entree_personnalisation_affichage_echeances );
      gtk_widget_show ( bouton_personnalisation_affichage_echeances );
    }



  /* place en dessous les boutons Saisir */

  hbox = gtk_hbox_new ( TRUE,
			10 );
  gtk_box_pack_end ( GTK_BOX ( vbox ),
		     hbox,
		     FALSE,
		     FALSE,
		     5 );
  gtk_widget_show ( hbox );

  separation = gtk_hseparator_new ();
  gtk_box_pack_end ( GTK_BOX ( vbox ),
		     separation,
		     FALSE,
		     FALSE,
		     5 );
  gtk_widget_show ( separation );

  bouton_saisir_echeancier = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_saisir_echeancier ),
			  GTK_RELIEF_NONE );
  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_container_set_border_width ( GTK_CONTAINER ( hbox2 ),
				   2 );
  gtk_container_add ( GTK_CONTAINER ( bouton_saisir_echeancier ),
		      hbox2 );
  gtk_widget_show ( hbox2 );


  icone = gnome_stock_new_with_icon ( GNOME_STOCK_PIXMAP_REFRESH );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       icone,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( icone );
    

  label = gtk_label_new ( _("Saisir") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  gtk_signal_connect ( GTK_OBJECT ( bouton_saisir_echeancier ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( click_sur_saisir_echeance ),
		       NULL );

  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_saisir_echeancier,
		       TRUE,
		       FALSE,
		       0 );
  gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_saisir_echeancier ),
			     FALSE );
  gtk_widget_show ( bouton_saisir_echeancier );

  return ( vbox );
}
/* ****************************************************************************************************** */



/* ****************************************************************************************************** */
/* fonction creation_liste_echeances */
/* renvoie la clist pour y mettre les échéances */
/* ****************************************************************************************************** */

GtkWidget *creation_liste_echeances ( void )
{
  GtkWidget *vbox;
  GtkWidget *win_echeances_scroll;
  gchar *titres_echeance[] = { _("Date "), _("Périodicité  "), _("Compte "), _("Tiers "), _("Mode ") , _("Montant ")};


  /*   à la base, on a une vbox */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_widget_show ( vbox );


/* création de la barre d'outils */

  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       creation_barre_outils_echeancier(),
		       FALSE,
		       FALSE,
		       0 );
 


/*   création de la scrolled window */

  win_echeances_scroll = gtk_scrolled_window_new ( NULL,
						   NULL);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( win_echeances_scroll),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       win_echeances_scroll,
		       TRUE,
		       TRUE,
		       5 );
  gtk_widget_show (win_echeances_scroll);


/* création de la liste des échéances */

  liste_echeances = gtk_clist_new_with_titles( 6,
					       titres_echeance );

  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_echeances ),
				  GTK_SELECTION_SINGLE );
  gtk_clist_set_compare_func ( GTK_CLIST ( liste_echeances ),
			       (GtkCListCompareFunc) classement_liste_echeances );

  gtk_signal_connect ( GTK_OBJECT (liste_echeances),
		       "button_press_event",
		       GTK_SIGNAL_FUNC ( click_ligne_echeance ),
		       NULL );

  gtk_signal_connect ( GTK_OBJECT ( liste_echeances ),
		       "size-allocate",
		       GTK_SIGNAL_FUNC ( changement_taille_liste_echeances ),
		       NULL );
 
  /*   vérifie les touche entrée, haut et bas */

  gtk_signal_connect ( GTK_OBJECT (liste_echeances),
		       "key_press_event",
		       GTK_SIGNAL_FUNC ( traitement_clavier_liste_echeances ),
		       NULL );

  /* si on va sur l'onglet des échéances, donne le focus à la liste */

  gtk_signal_connect_object ( GTK_OBJECT (liste_echeances),
			      "map",
			      GTK_SIGNAL_FUNC ( gtk_widget_grab_focus ),
			      GTK_OBJECT (liste_echeances) );


  gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances ),
				       0,
				       GTK_JUSTIFY_CENTER );
  gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances ),
				       1,
				       GTK_JUSTIFY_CENTER );
  gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances ),
				       2,
				       GTK_JUSTIFY_LEFT );
 gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances ),
				       3,
				       GTK_JUSTIFY_LEFT );
  gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances ),
				       4,
				       GTK_JUSTIFY_CENTER );
  gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances ),
				       5,
				       GTK_JUSTIFY_RIGHT );

  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances ),
				    0,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances ),
				    1,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances ),
				    2,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances ),
				    3,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances ),
				    4,
				    FALSE );
  gtk_clist_set_column_resizeable ( GTK_CLIST ( liste_echeances ),
				    5,
				    FALSE );

  gtk_clist_set_selection_mode ( GTK_CLIST  ( liste_echeances ),
				 GTK_SELECTION_SINGLE ) ;

  gtk_clist_column_titles_passive ( GTK_CLIST ( liste_echeances ) );

  gtk_container_add ( GTK_CONTAINER ( win_echeances_scroll ),
		      liste_echeances );
  gtk_widget_show ( liste_echeances );

  echeance_selectionnnee = GINT_TO_POINTER ( -1 );

  remplissage_liste_echeance();

  return ( vbox );
}
/* ************************************************************************************************************************************ */


/* *********************************************************************************************************** */
/* cette fonction teste la touche entree sur la liste d'échéances  */
/* *********************************************************************************************************** */

gboolean traitement_clavier_liste_echeances ( GtkCList *liste_echeances,
					  GdkEventKey *evenement,
					  gpointer null )
{
  gint ligne;

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste_echeances ),
				 "key_press_event");

  switch ( evenement->keyval )
    {
    case 65293 :
    case 65421 :
                         /* entrée */

      formulaire_echeancier_a_zero();
      degrise_formulaire_echeancier();

      gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			   _("Modif") );
      edition_echeance ();
      break;


    case 65362 :                       /* flèche haut  */

      ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_echeances ),
					     echeance_selectionnnee );
      if ( ligne )
	{
	  /* saute les doubles d'échéances */

	  do
	    echeance_selectionnnee = gtk_clist_get_row_data ( GTK_CLIST ( liste_echeances ),
							      --ligne );
	  while ( !echeance_selectionnnee );

	  gtk_clist_unselect_all ( GTK_CLIST ( liste_echeances ) );
	  selectionne_echeance ();

	}
      break;


    case 65364 :                /* flèche bas */

      if ( echeance_selectionnnee != GINT_TO_POINTER ( -1 ) )
	{
	  ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_echeances ),
						 echeance_selectionnnee );

	  /* saute les doubles d'échéances */

	  do
	    echeance_selectionnnee  = gtk_clist_get_row_data ( GTK_CLIST ( liste_echeances ),
							       ++ligne );
	  while ( !echeance_selectionnnee );

	  gtk_clist_unselect_all ( GTK_CLIST ( liste_echeances ) );
	  selectionne_echeance ();
	}
      break;


    case 65535 :               /*  del  */

      supprime_echeance ();
      break;



    default :
    }

  return ( TRUE );    
}
/* *************************************************************************************************************** */



/* *************************************************************************************************************** */
void click_sur_saisir_echeance ( void )
{
  formulaire_echeancier_a_zero();
  degrise_formulaire_echeancier();

  gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
		       _("Saisie") );
  edition_echeance ();

  gtk_widget_hide ( widget_formulaire_echeancier[16] );
  gtk_widget_hide ( widget_formulaire_echeancier[15] );
  gtk_widget_hide ( widget_formulaire_echeancier[17] );
  gtk_widget_hide ( widget_formulaire_echeancier[18] );
  gtk_widget_hide ( widget_formulaire_echeancier[5] );
  gtk_widget_hide ( widget_formulaire_echeancier[13] );

}
/* *************************************************************************************************************** */






/* ************************************************************************************************************ */
void remplissage_liste_echeance ( void )
{
  gchar *ligne[6];
  GSList *pointeur_liste;
  int ligne_clist;
  gint couleur_en_cours;
  gint i;
  time_t temps;
  struct tm *temps_tm;
  GDate *date_fin;

  /* on récupère la date du jour et la met dans date_fin pour les vérifications ultérieures */
 
  time ( &temps );
  temps_tm = localtime ( &temps );
  date_fin = g_date_new_dmy ( temps_tm -> tm_mday,
			      temps_tm -> tm_mon+1,
			      temps_tm -> tm_year+1900 );


  /*   on calcule la date de fin */

  switch ( affichage_echeances )
    {
    case 0:
      g_date_add_months ( date_fin,
			  1 );
      date_fin -> day = 1;
      break;

    case 1:
      g_date_add_months ( date_fin,
			  2 );
      date_fin -> day = 1;
      break;

    case 2:
      g_date_add_years ( date_fin,
			  1 );
      date_fin -> day = 1;
      date_fin -> month = 1;
      break;

    case 4:
      if ( !affichage_echeances_perso_j_m_a )
	g_date_add_days ( date_fin,
			  affichage_echeances_perso_nb_libre );
      else
	{
	  if ( affichage_echeances_perso_j_m_a == 1 )
	    g_date_add_months ( date_fin,
				affichage_echeances_perso_nb_libre );
	  else
	    g_date_add_years ( date_fin,
			       affichage_echeances_perso_nb_libre );
	}
    }

  /* on classe les échéances par date */

/*   gsliste_echeances = g_slist_sort ( gsliste_echeances, */
/* 				     (GCompareFunc) comparaison_date_echeance);  */

  gtk_clist_freeze ( GTK_CLIST ( liste_echeances ) );

/* on efface la liste */

  gtk_clist_clear ( GTK_CLIST ( liste_echeances ) );


  couleur_en_cours = 0;

  pointeur_liste = gsliste_echeances;

  while ( pointeur_liste )
    {
      GSList *pointeur_tmp;

      /* mise en forme de la date */

      ligne[0] = g_strdup_printf ( "%02d/%02d/%d",
				   ECHEANCE_COURANTE -> jour,
				   ECHEANCE_COURANTE -> mois,
				   ECHEANCE_COURANTE -> annee );
 
      switch ( ECHEANCE_COURANTE ->periodicite )
	{
	case 0 : ligne[1] = _("Une fois");
	  break;
	case 1 : ligne[1] = _("Hebdomadaire");
	  break;
	case 2 : ligne[1] = _("Mensuelle");
	  break;
	case 3 : ligne[1] = _("Annuelle");
	  break;
	case 4 :
	  if ( ECHEANCE_COURANTE -> intervalle_periodicite_personnalisee )
	    if ( ECHEANCE_COURANTE -> intervalle_periodicite_personnalisee == 1 )
	      ligne[1] = g_strdup_printf ( _("%d mois"),
					   ECHEANCE_COURANTE -> periodicite_personnalisee );
	    else
	      ligne[1] = g_strdup_printf ( _("%d ans"),
					   ECHEANCE_COURANTE -> periodicite_personnalisee );
	  else
	    ligne[1] = g_strdup_printf ( _("%d jours"),
					 ECHEANCE_COURANTE -> periodicite_personnalisee );
	  break;
	}

      /* mise en forme du compte */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ECHEANCE_COURANTE -> compte;
      ligne[2] = NOM_DU_COMPTE ;

      /* mise en forme du tiers */

      pointeur_tmp = g_slist_find_custom ( liste_struct_tiers,
					   GINT_TO_POINTER ( ECHEANCE_COURANTE -> tiers ),
					   (GCompareFunc) recherche_tiers_par_no );
      if ( pointeur_tmp )
	ligne[3] = ((struct struct_tiers *)( pointeur_tmp-> data )) -> nom_tiers;
      else
	ligne[3] = NULL;

      /* mise en forme de auto/man */

      if ( ECHEANCE_COURANTE -> auto_man )
	ligne[4]=_("Automatique");
      else
	ligne[4] = _("Manuel");


      /* mise en forme du montant */

      ligne[5] = g_strdup_printf ( "%4.2f",
				   ECHEANCE_COURANTE -> montant );

      /* on va ajouter l'échéance une ou plusieurs fois en changeant juste sa date */
      /* en fontion de l'affichage de l'échéance */

      if ( !(g_date_compare ( date_fin,
			      ECHEANCE_COURANTE -> date ) <= 0
	     &&
	     affichage_echeances != 3 ))
	{
	  gint sortie;
	  GDate *date_courante;
	  struct operation_echeance *echeance;

	  echeance = ECHEANCE_COURANTE;
	  sortie = 1;
	  date_courante = g_date_new_dmy ( ECHEANCE_COURANTE -> date -> day,
					   ECHEANCE_COURANTE -> date -> month,
					   ECHEANCE_COURANTE -> date -> year );

	  while ( sortie )
	    {

	      ligne_clist = gtk_clist_append ( GTK_CLIST ( liste_echeances ),
					       ligne );

	      /* on met le numéro de l'échéance */

	      gtk_clist_set_row_data ( GTK_CLIST ( liste_echeances ),
				       ligne_clist,
				       echeance );

	      /* si c'est un doublon, on met le style en gris */

	      if ( !echeance )
		gtk_clist_set_row_style ( GTK_CLIST ( liste_echeances ),
					  ligne_clist,
					  style_gris );

	      /* c'est maintenant qu'on voit si on sort ou pas ... */

	      date_courante = date_suivante_echeance ( ECHEANCE_COURANTE,
						       date_courante );

	      if ( date_courante
		   &&
		   g_date_compare ( date_fin,
				    date_courante ) > 0
		   &&
		   affichage_echeances != 3
		   &&
		   ECHEANCE_COURANTE -> periodicite )
		{
		  ligne[0] = g_strdup_printf ( "%02d/%02d/%d",
					       g_date_day ( date_courante ),
					       g_date_month ( date_courante ),
					       g_date_year ( date_courante ));
		  echeance = NULL;
		}
	      else
		sortie = 0;
	    }
	}
      pointeur_liste = pointeur_liste -> next;
    }


  /* met la ligne blanche */

  for ( i = 0 ; i<6 ; i++ )
    ligne[i] = NULL;

  ligne_clist = gtk_clist_append ( GTK_CLIST ( liste_echeances ),
				   ligne );

  gtk_clist_set_row_data ( GTK_CLIST ( liste_echeances ),
			   ligne_clist,
			   GINT_TO_POINTER ( -1 ) );



  gtk_clist_sort ( GTK_CLIST ( liste_echeances ));

  /* on refait le tour de la liste pour mettre la couleur */

  for ( i=0 ; i < GTK_CLIST ( liste_echeances ) -> rows ; i++ )
    {
      struct operation_echeance *echeance;

      echeance = gtk_clist_get_row_data ( GTK_CLIST ( liste_echeances ),
					  i );

      if ( echeance
	   &&
	   echeance != GINT_TO_POINTER (-1)
	   &&
	   echeance -> montant < 0 )
	gtk_clist_set_cell_style ( GTK_CLIST ( liste_echeances ),
				   i,
				   5,
				   style_rouge_couleur [ couleur_en_cours ] );

      /* on ne met le bleu ou blanc que si c'est pas gris */

      if ( echeance )
	gtk_clist_set_row_style ( GTK_CLIST ( liste_echeances ),
				  i,
				  style_couleur [ couleur_en_cours ] );

      couleur_en_cours = ( couleur_en_cours + 1 ) % 2;
    }

  selectionne_echeance();
  gtk_clist_thaw ( GTK_CLIST ( liste_echeances ) );
}
/* ************************************************************************************************************ */






/* ************************************************************************************************************ */
void click_ligne_echeance ( GtkCList *liste,
			    GdkEventButton *evenement,
			    gpointer data )
{
  gint colonne, x, y;
  gint ligne;

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


  if ( !gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				 ligne ) )
    return;

  /*   vire l'ancienne sélection */

  gtk_clist_unselect_all ( GTK_CLIST ( liste ) );

  /* on met l'adr de la struct dans echeance_selectionnnee */

  echeance_selectionnnee = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
						    ligne );

  selectionne_echeance ();


  if ( evenement -> type == GDK_2BUTTON_PRESS )
    {
      formulaire_echeancier_a_zero();
      degrise_formulaire_echeancier();

      gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			   _("Modif") );
      edition_echeance ();
    }
  else
    gtk_widget_grab_focus ( liste_echeances );

}
/* ****************************************************************************************************************** */


/* ****************************************************************************************************************** */
void selectionne_echeance ( void )
{
  gint ligne;

  ligne = gtk_clist_find_row_from_data ( GTK_CLIST ( liste_echeances ),
					 echeance_selectionnnee );

  gtk_clist_select_row ( GTK_CLIST ( liste_echeances ),
			 ligne,
			 0 );


  if ( gtk_clist_row_is_visible ( GTK_CLIST ( liste_echeances ),
				  ligne )
       != GTK_VISIBILITY_FULL )
    {
      if ( ligne > gtk_clist_get_vadjustment ( GTK_CLIST ( liste_echeances )) -> value  / GTK_CLIST ( liste_echeances ) -> row_height  )
	gtk_clist_moveto ( GTK_CLIST ( liste_echeances ),
			   ligne,
			   0,
			   1,
			   0 );
      else
	gtk_clist_moveto ( GTK_CLIST ( liste_echeances ),
			   ligne,
			   0,
			   0,
			   0 );
    }

  if ( echeance_selectionnnee != GINT_TO_POINTER ( -1 ) )
    gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_saisir_echeancier ),
			       TRUE );
  else
    gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_saisir_echeancier ),
			       FALSE );

}
/* ****************************************************************************************************************** */



/* ****************************************************************************************************************** */
/* Fonction edition_echeance */
/* appelée par double click ou entrée sur une échéance */
/* recopie l'échéance sélectionnée dans le formulaire d'échéance pour la modifier */
/* ****************************************************************************************************************** */

void edition_echeance ( void )
{
  GSList *pointeur_tmp;

  /*   si le formulaire est caché, on le montre */

  if ( !etat.formulaire_echeancier_toujours_affiche )
    {
      gtk_widget_show ( frame_formulaire_echeancier );
      gtk_signal_connect ( GTK_OBJECT ( frame_formulaire_echeancier ),
			   "draw",
			   GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_echeance_visible ),
			   NULL );
    }

  

  /* si c'est une nouvelle échéance, on met la date courante, la sélectionne et lui donne le focus */

  if ( echeance_selectionnnee == GINT_TO_POINTER ( -1 ))
    {
      entree_prend_focus ( widget_formulaire_echeancier[0] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[0] ),
			   date_jour() );
      gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_echeancier[0] ),
				0,
				-1 );
      gtk_widget_grab_focus ( widget_formulaire_echeancier[0] );

      return;
    }


  /* c'est une modif d'échéance ou une saisie */

  entree_prend_focus ( widget_formulaire_echeancier[0] );
  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[0] ),
		       g_strdup_printf ( "%02d/%02d/%d",
					 echeance_selectionnnee -> jour,
					 echeance_selectionnnee -> mois,
					 echeance_selectionnnee -> annee ));
  
  /* mise en place du tiers */

  pointeur_tmp = g_slist_find_custom ( liste_struct_tiers,
				       GINT_TO_POINTER ( echeance_selectionnnee -> tiers ),
				       (GCompareFunc) recherche_tiers_par_no );
  if ( pointeur_tmp )
    {
      entree_prend_focus ( widget_formulaire_echeancier[1] );
      gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[1] ),
			      ((struct struct_tiers *)( pointeur_tmp-> data )) -> nom_tiers );
    }


  /* mise en place du compte */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[5] ),
				echeance_selectionnnee -> compte );

  /* mise en place des montants et des types associés */

  if ( echeance_selectionnnee -> montant < 0 )
    {
      entree_prend_focus ( widget_formulaire_echeancier[2] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[2] ),
			   g_strdup_printf ( "%4.2f",
					     -echeance_selectionnnee -> montant ));
      /* met le menu des types débits */

      if ( !etat.affiche_tous_les_types )
	{
	  GtkWidget *menu;

	  if ( (menu = creation_menu_types ( 1,
					     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[5] )->menu_item),
										     "no_compte" )),
					     1 )))
	    {
	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[7] ),
					 menu );
	      gtk_widget_show ( widget_formulaire_echeancier[7] );
	    }
	  else
	    gtk_widget_hide ( widget_formulaire_echeancier[7] );
	}
    }
  else
    {
      entree_prend_focus ( widget_formulaire_echeancier[3] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[3] ),
			   g_strdup_printf ( "%4.2f",
					     echeance_selectionnnee -> montant ));
      /* met le menu des types crédits */

      if ( !etat.affiche_tous_les_types )
	{
	  GtkWidget *menu;

	  if ( (menu = creation_menu_types ( 2,
					     GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget_formulaire_echeancier[5] )->menu_item),
										     "no_compte" )),
					     1 )))
	    {
	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[7] ),
					 menu );
	      gtk_widget_show ( widget_formulaire_echeancier[7] );
	    }
	  else
	    gtk_widget_hide ( widget_formulaire_echeancier[7] );
	}
    }


  /* mise en place des devises */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[4] ),
				((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
										GINT_TO_POINTER ( echeance_selectionnnee -> devise ),
										(GCompareFunc) recherche_devise_par_no ) -> data )) -> no_devise - 1 );


  /*   si le compte de virement est -1, c'est qu'il n'y a pas de categ */
 
  if ( echeance_selectionnnee -> compte_virement != -1 )
    {
      gchar *texte;

      if ( echeance_selectionnnee -> categorie )
	{
	  struct struct_categ *categorie;

	  categorie =  g_slist_find_custom ( liste_struct_categories,
					     GINT_TO_POINTER ( echeance_selectionnnee -> categorie ),
					     (GCompareFunc) recherche_categorie_par_no ) ->data;

	  texte = categorie -> nom_categ;

	  if ( echeance_selectionnnee -> sous_categorie )
	    texte = g_strconcat ( texte,
				  " : ",
				  ((struct struct_sous_categ *)(g_slist_find_custom ( categorie -> liste_sous_categ,
										      GINT_TO_POINTER ( echeance_selectionnnee -> sous_categorie ),
										      (GCompareFunc) recherche_sous_categorie_par_no )->data)) -> nom_sous_categ,
				  NULL );
	}
      else
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + echeance_selectionnnee -> compte_virement;

	  texte = g_strconcat ( _("Virement : "),
				NOM_DU_COMPTE,
				NULL );
	}

      entree_prend_focus ( widget_formulaire_echeancier[6] );
      gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[6] ),
			      texte );
    }


  /* met l'option menu du type d'opé */

  if ( GTK_WIDGET_VISIBLE ( widget_formulaire_echeancier[7] ))
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[7] ),
				  cherche_no_menu_type_echeancier ( echeance_selectionnnee -> type_ope ));

  if ( echeance_selectionnnee -> contenu_type )
    {
      entree_prend_focus ( widget_formulaire_echeancier[8] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[8] ),
			   echeance_selectionnnee -> contenu_type );
    }


  /* met en place l'exercice */

  gtk_option_menu_set_history (  GTK_OPTION_MENU ( widget_formulaire_echeancier[9] ),
				 cherche_no_menu_exercice ( echeance_selectionnnee -> no_exercice ));

  /* met en place l'imputation budgétaire */


  pointeur_tmp = g_slist_find_custom ( liste_struct_imputation,
				    GINT_TO_POINTER ( echeance_selectionnnee -> imputation ),
				    ( GCompareFunc ) recherche_imputation_par_no );

  if ( pointeur_tmp )
    {
      GSList *pointeur_tmp_2;

      entree_prend_focus ( widget_formulaire_echeancier[10]);

      pointeur_tmp_2 = g_slist_find_custom ( (( struct struct_imputation * )( pointeur_tmp -> data )) -> liste_sous_imputation,
					  GINT_TO_POINTER ( echeance_selectionnnee -> sous_imputation ),
					  ( GCompareFunc ) recherche_sous_categorie_par_no );
      if ( pointeur_tmp_2 )
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[10] ),
				g_strconcat ( (( struct struct_imputation * )( pointeur_tmp -> data )) -> nom_imputation,
					      " : ",
					      (( struct struct_sous_imputation * )( pointeur_tmp_2 -> data )) -> nom_sous_imputation,
					      NULL ));
      else
	gtk_combofix_set_text ( GTK_COMBOFIX ( widget_formulaire_echeancier[10] ),
				(( struct struct_imputation * )( pointeur_tmp -> data )) -> nom_imputation );
    }


  /*   remplit les infos guichet / banque */

/*   if ( echeance_selectionnnee -> info_banque_guichet ) */
/*     { */
/*       entree_prend_focus ( widget_formulaire_echeancier[11] ); */
/*       gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[11] ), */
/* 			   echeance_selectionnnee -> info_banque_guichet ); */
/*     } */

  /* mise en place de la pièce comptable */

/*   if ( echeance_selectionnnee -> no_piece_comptable ) */
/*     { */
/*       entree_prend_focus ( widget_formulaire_echeancier[12] ); */
/*       gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[12] ), */
/* 			   echeance_selectionnnee -> no_piece_comptable ); */
/*     } */


  /* mise en place de l'automatisme */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[13] ),
				echeance_selectionnnee -> auto_man );

  /*   remplit les notes */

  if ( echeance_selectionnnee -> notes )
    {
      entree_prend_focus ( widget_formulaire_echeancier[14] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[14] ),
			   echeance_selectionnnee -> notes );
    }

  /* mise en place de la périodicité */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[15] ),
				echeance_selectionnnee -> periodicite );

  /* mise en place de la date limite */

  if ( echeance_selectionnnee -> periodicite )
    {
      if ( echeance_selectionnnee ->  date_limite )
	{
	  entree_prend_focus ( widget_formulaire_echeancier[16] );
	  gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[16] ),
			       g_strdup_printf ( "%02d/%02d/%d",
						 echeance_selectionnnee ->  jour_limite,
						 echeance_selectionnnee ->  mois_limite,
						 echeance_selectionnnee ->  annee_limite ));
	}

      gtk_widget_show ( widget_formulaire_echeancier[16] );
    }


  /* mise en place de la périodicité perso si nécessaire */

  if ( echeance_selectionnnee -> periodicite == 4 )
    {
      entree_prend_focus ( widget_formulaire_echeancier[17] );
      gtk_entry_set_text ( GTK_ENTRY (widget_formulaire_echeancier[17] ),
			   itoa ( echeance_selectionnnee ->  periodicite_personnalisee) );
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[18] ),
				    echeance_selectionnnee -> intervalle_periodicite_personnalisee );
      gtk_widget_show ( widget_formulaire_echeancier[17] );
      gtk_widget_show ( widget_formulaire_echeancier[18] );

    }


  /* sélectionne la date et lui donne le focus */

  gtk_entry_select_region ( GTK_ENTRY ( widget_formulaire_echeancier[0] ),
			    0,
			    -1 );
  gtk_widget_grab_focus ( widget_formulaire_echeancier[0] );

  /* met l'adr de l'échéance courante dans l'entrée de la date */

  gtk_object_set_data ( GTK_OBJECT ( formulaire_echeancier ),
			"adr_echeance",
			echeance_selectionnnee );
}
/* ****************************************************************************************************************** */



/* ************************************************************************************************************* */
void supprime_echeance ( void )
{
  gint resultat;


  if ( echeance_selectionnnee == GINT_TO_POINTER ( -1 ) )
    return;

  /* si la périodicité n'est pas de 1 fois demande juste celle ci ou toutes, sinon, choisit automatiquement toutes */

  if ( echeance_selectionnnee -> periodicite )
    {
      GtkWidget *dialog;
      GtkWidget *label;


      dialog = gnome_dialog_new ( _("Effacer une échéance"),
				  _("Juste celle-ci"),
				  _("Toutes les occurences"),
				  GNOME_STOCK_BUTTON_CANCEL,
				  NULL );

      label = gtk_label_new ( _(" Voulez-vous effacer juste cette occurence ou l'échéance complète ? ") );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      if ( echeance_selectionnnee -> tiers )
	label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d : %s [%4.2f]",
						  echeance_selectionnnee -> jour,
						  echeance_selectionnnee -> mois,
						  echeance_selectionnnee -> annee,
						  ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
												 GINT_TO_POINTER ( echeance_selectionnnee -> tiers ),
												 (GCompareFunc ) recherche_tiers_par_no )->data )) -> nom_tiers,
						  echeance_selectionnnee -> montant ));
      else
	label = gtk_label_new ( g_strdup_printf ( _("%02d/%02d/%d : [Aucun Tiers] [%4.2f]"),
						  echeance_selectionnnee -> jour,
						  echeance_selectionnnee -> mois,
						  echeance_selectionnnee -> annee,
						  echeance_selectionnnee -> montant ));



      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );



      gtk_box_set_homogeneous ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
				TRUE );
      gnome_dialog_set_default ( GNOME_DIALOG ( dialog ),
				 2 );
      gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				     GTK_WINDOW ( window ));

      resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));
    }
  else
    resultat = 1;

  switch ( resultat )
    {
    case 0:
      incrementation_echeance ( echeance_selectionnnee );
      remplissage_liste_echeance ();
      break;

    case 1:
      gsliste_echeances = g_slist_remove ( gsliste_echeances, 
					   echeance_selectionnnee );
      free ( echeance_selectionnnee );
      echeance_selectionnnee = gtk_clist_get_row_data ( GTK_CLIST ( liste_echeances ),
							gtk_clist_find_row_from_data ( GTK_CLIST ( liste_echeances ),
										       echeance_selectionnnee ) + 1);

      nb_echeances--;

      remplissage_liste_echeance ();
      break;

    default:
    }

  mise_a_jour_calendrier();
  update_liste_echeances_manuelles_accueil();
  modification_fichier (TRUE);
}
/* ************************************************************************************************************************************ */





/* ***************************************************************************************************** */
/* Fonction changement_taille_liste_echeances */
/* appelée dès que la taille de la clist a changé */
/* pour mettre la taille des différentes colonnes */
/* ***************************************************************************************************** */

void changement_taille_liste_echeances ( GtkWidget *clist,
					 GtkAllocation *allocation,
					 gpointer null )
{
  gint largeur;
  gint col1, col2, col3, col4, col5, col6, col7;

  /*   si la largeur de grisbi est < 700, on fait rien */

  if ( window -> allocation.width < 700 )
    return;

  largeur = allocation->width;

  if ( largeur == ancienne_largeur_echeances )
    return;

  ancienne_largeur_echeances = largeur;

  col1 = ( 15 * largeur ) / 100;
  col2 = ( 15 * largeur ) / 100;
  col3 = ( 15 * largeur ) / 100;
  col4 = ( 20 * largeur ) / 100;
  col5 = ( 15 * largeur ) / 100;
  col6 = ( 5 * largeur ) / 100;

  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       0,
			       col1 );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       1,
			       col2 );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       2,
			       col3 );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       3,
			       col4 );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       4,
			       col5 );
  gtk_clist_set_column_width ( GTK_CLIST ( clist ),
			       5,
			       col6 );


/* met les entrées du formulaire à la même taille */

  col1 = ( 7 * largeur) / 100;
  col2 = ( 13 * largeur) / 100;
  col3 = ( 30 * largeur) / 100;
  col4 = ( 12 * largeur) / 100;
  col5 = ( 12 * largeur) / 100;
  col6 = ( 10 * largeur) / 100;
  col7 = ( 10 * largeur) / 100;


  /* 1ère ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( label_saisie_modif ),
			 col1,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[0] ),
			 col2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[1] ),
			 col3,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[2] ),
			 col4,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[3] ),
			 col5,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[4] ),
			 col6 + col7,
			 FALSE );


  /* 2ème ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[5] ),
			 col1 + col2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[6] ),
			 col3,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[7] ),
			 col4 + col5,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[8] ),
			 col6 + col7,
			 FALSE );


  /* 3ème ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[9] ),
			 col1 + col2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[10] ),
			 col3,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[11] ),
			 col4 + col5,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[12] ),
			 col6 + col7,
			 FALSE );

  /* 4ème ligne */

  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[13] ),
			 col1 + col2,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[14] ),
			 col3,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[15] ),
			 col4,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[16] ),
			 col5,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[17] ),
			 col6,
			 FALSE );
  gtk_widget_set_usize ( GTK_WIDGET ( widget_formulaire_echeancier[18] ),
			 col7,
			 FALSE );

}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction : mise_a_jour_calendrier */
/* met en gras les jours où il y a une échéance */
/* ***************************************************************************************************** */

void mise_a_jour_calendrier ( void )
{
  GDate *date_calendrier;
  time_t temps;
  GSList *pointeur;


  date_calendrier = g_date_new_dmy ( 1,
				     GTK_CALENDAR ( calendrier_echeances ) -> month + 1,
				     GTK_CALENDAR ( calendrier_echeances ) -> year );

  gtk_calendar_clear_marks ( GTK_CALENDAR ( calendrier_echeances ));


  /*   si le jour courant doit y apparaitre, on le sélectionne */

   time ( &temps );

   if ( ( localtime ( &temps ) -> tm_mon == GTK_CALENDAR ( calendrier_echeances ) -> month )
	&&
	( ( localtime ( &temps ) -> tm_year + 1900 ) == GTK_CALENDAR ( calendrier_echeances ) -> year ) )
     gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
			       localtime ( &temps ) -> tm_mday );
   else
     gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
			       FALSE );

  /*   fait le tour de toutes les échéances, les amène au mois du calendrier et grasse le jour de l'échéance */

  pointeur = gsliste_echeances;

  while ( pointeur )
    {
      GDate *copie_date_ech;
      struct operation_echeance *echeance;

      echeance = pointeur -> data;

      copie_date_ech = g_date_new_dmy ( echeance -> jour,
					echeance -> mois,
					echeance -> annee );

      /* si c'est une fois */
      /* ou si c'est personnalisée mais la periodicité est de 0, */
      /* on passe */

      if ( echeance -> periodicite
	   &&
	   !(
	     echeance -> periodicite == 4
	     &&
	     !echeance -> periodicite_personnalisee ))
	while ( g_date_compare ( copie_date_ech,
				 date_calendrier ) < 0 )
	  {
	    /* périodicité hebdomadaire */
	    if ( echeance -> periodicite == 1 )
	      g_date_add_days ( copie_date_ech,
				7 );
	    else
	      /* périodicité mensuelle */
	      if ( echeance -> periodicite == 2 )
		g_date_add_months ( copie_date_ech,
				    1 );
	      else
		/* périodicité annuelle */
		if ( echeance -> periodicite == 3 )
		  g_date_add_years ( copie_date_ech,
				     1 );
		else
		  /* périodicité perso */
		  if ( !echeance -> intervalle_periodicite_personnalisee )
		    g_date_add_days ( copie_date_ech,
				      echeance -> periodicite_personnalisee );
		  else
		    if ( echeance -> intervalle_periodicite_personnalisee == 1 )
		      g_date_add_months ( copie_date_ech,
					  echeance -> periodicite_personnalisee );
		    else
		      g_date_add_years ( copie_date_ech,
					 echeance -> periodicite_personnalisee );
	  }  

      /*       à ce niveau, soit l'échéance est sur le mois du calendrier, soit elle l'a dépassé, soit elle a dépassé sa date limite */

      if ( !( copie_date_ech -> month != date_calendrier -> month
	      ||
	      ( echeance -> date_limite
		&&
		g_date_compare ( copie_date_ech,
				 echeance -> date_limite ) > 0 )
	   ||
	      ( !echeance -> periodicite
		&&
		copie_date_ech -> year != date_calendrier -> year )))
	gtk_calendar_mark_day ( GTK_CALENDAR ( calendrier_echeances ),
				  copie_date_ech -> day );

      pointeur = pointeur -> next;
    }



}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
void click_sur_jour_calendrier_echeance ( GtkWidget *calendrier,
					  gpointer null )
{
  time_t temps;

   time ( &temps );

   gtk_signal_handler_block_by_func ( GTK_OBJECT ( calendrier ),
				      GTK_SIGNAL_FUNC ( click_sur_jour_calendrier_echeance ),
				      NULL );

   if ( ( localtime ( &temps ) -> tm_mon == GTK_CALENDAR ( calendrier_echeances ) -> month )
	&&
	( ( localtime ( &temps ) -> tm_year + 1900 ) == GTK_CALENDAR ( calendrier_echeances ) -> year ) )
     gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
			       localtime ( &temps ) -> tm_mday );
   else
     gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
			       FALSE );

   gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( calendrier ),
				      GTK_SIGNAL_FUNC ( click_sur_jour_calendrier_echeance ),
				      NULL );

}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
void verification_echeances_a_terme ( void )
{
  GDate *date_courante;
  GSList *pointeur_liste;
  GSList *ancien_pointeur;
  struct struct_devise *devise;
  struct struct_devise *devise_compte;


  /*   les échéances à saisir sont revérifiées à chaque fois, par contre, les échéances saisies sont ajoutées à la liste */
  /* de celles déjà saisies */

  echeances_a_saisir = NULL;

/* récupère la date du jour et la met en format gdate */

  date_courante = g_date_new ();
  g_date_set_time ( date_courante,
		    time ( NULL ));

  /* on y ajoute le décalage de l'échéance */

  g_date_add_days ( date_courante,
		    decalage_echeance );


  /*   on fait le tour des échéances, si elle est auto, elle est enregistrée, sinon elle est juste répertoriée */

  pointeur_liste = gsliste_echeances;
  ancien_pointeur = NULL;


  while ( pointeur_liste )
    {
      if ( ECHEANCE_COURANTE -> auto_man )
	{
	      /* 	      tant que cette échéance auto n'est pas arrivée à aujourd'hui, on recommence */
	 
	  while ( pointeur_liste != ancien_pointeur
		  &&
		  g_date_compare ( ECHEANCE_COURANTE -> date, date_courante ) <= 0 )
	    {
	      struct structure_operation *operation;
	      gint virement;


	      /* crée l'opération */

	      operation = calloc ( 1,
				   sizeof ( struct structure_operation ) ); 


	      /* 		  remplit l'opération */

	      operation -> jour = ECHEANCE_COURANTE -> jour;
	      operation -> mois = ECHEANCE_COURANTE -> mois;
	      operation -> annee = ECHEANCE_COURANTE -> annee;

	      operation ->date = g_date_new_dmy ( operation ->jour,
						  operation ->mois,
						  operation ->annee);


	      operation -> no_compte = ECHEANCE_COURANTE -> compte;
	      operation -> tiers = ECHEANCE_COURANTE -> tiers;
	      operation -> montant = ECHEANCE_COURANTE -> montant;
	      operation -> devise = ECHEANCE_COURANTE -> devise;


	      /* demande si nécessaire la valeur de la devise et du change */

	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	      /* récupération des devises de l'opé et du compte */
 
	      devise = g_slist_find_custom ( liste_struct_devises,
					     GINT_TO_POINTER ( operation -> devise ),
					     ( GCompareFunc ) recherche_devise_par_no ) -> data;

	      devise_compte = g_slist_find_custom ( liste_struct_devises,
						    GINT_TO_POINTER ( DEVISE ),
						    ( GCompareFunc ) recherche_devise_par_no ) -> data;

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
	      else
		{
		  operation -> taux_change = 0;
		  operation -> frais_change = 0;
		}

	      operation -> categorie = ECHEANCE_COURANTE -> categorie;
	      operation -> sous_categorie = ECHEANCE_COURANTE -> sous_categorie;

	      if ( !ECHEANCE_COURANTE -> categorie && ECHEANCE_COURANTE -> compte_virement != -1 )
		virement = 1;
	      else
		virement = 0;

	      if ( ECHEANCE_COURANTE -> notes )
		operation -> notes = g_strdup ( ECHEANCE_COURANTE -> notes );


	      operation -> type_ope = ECHEANCE_COURANTE -> type_ope;
	      if ( ECHEANCE_COURANTE -> contenu_type )
		operation -> contenu_type = ECHEANCE_COURANTE -> contenu_type;

	      operation -> auto_man = ECHEANCE_COURANTE -> auto_man;
	      operation -> imputation = ECHEANCE_COURANTE -> imputation;
	      operation -> sous_imputation = ECHEANCE_COURANTE -> sous_imputation;
	      operation -> no_exercice = ECHEANCE_COURANTE -> no_exercice;


	      /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

	      ajout_operation ( operation );

	      /*   si c'était un virement, on crée une copie de l'opé, on l'ajoute à la liste puis on remplit les relations */

	      if ( virement )
		{
		  struct structure_operation *operation_2;

		  operation_2 = calloc ( 1,
					 sizeof ( struct structure_operation ) );


		  /* remplit la nouvelle opé */
      
		  operation_2 -> jour = operation -> jour;
		  operation_2 -> mois = operation -> mois;
		  operation_2 -> annee = operation -> annee;
		  operation_2 ->date = g_date_new_dmy ( operation_2->jour,
							operation_2->mois,
							operation_2->annee);

		  operation_2 -> no_compte = ECHEANCE_COURANTE -> compte_virement;
      
		  operation_2 -> montant = -operation -> montant;
		  operation_2 -> devise = operation -> devise;
		  operation_2 -> tiers = operation -> tiers;

		  if ( operation -> notes )
		    operation_2 -> notes = g_strdup ( operation -> notes);

		  operation_2 -> type_ope = operation -> type_ope;
		  if ( operation -> contenu_type )
		    operation_2 -> contenu_type = operation -> contenu_type;

		  operation_2 -> auto_man = operation -> auto_man;

		  /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

		  ajout_operation ( operation_2 );


		  /* on met maintenant les relations entre les différentes opé */

		  operation -> relation_no_operation = operation_2 -> no_operation;
		  operation -> relation_no_compte = operation_2 -> no_compte;
		  operation_2 -> relation_no_operation = operation -> no_operation;
		  operation_2 -> relation_no_compte = operation -> no_compte;

		}


	      echeances_saisies = g_slist_append ( echeances_saisies,
						   operation );


	      /* passe l'échéance à la prochaine date */

	      incrementation_echeance ( ECHEANCE_COURANTE );

	      if ( !g_slist_find ( gsliste_echeances,
				   ECHEANCE_COURANTE ) )
		{
		  if ( !ancien_pointeur )
		    ancien_pointeur = gsliste_echeances;
		  pointeur_liste = ancien_pointeur;
		}
	    }
	}
      else
	/* ce n'est pas une échéance automatique, on la répertorie dans la liste des échéances à saisir */

	if ( g_date_compare ( ECHEANCE_COURANTE -> date,
			      date_courante ) <= 0 )
	  echeances_a_saisir = g_slist_append ( echeances_a_saisir ,
						ECHEANCE_COURANTE );


      if ( ancien_pointeur == gsliste_echeances && pointeur_liste == ancien_pointeur )
	ancien_pointeur = NULL;
      else
	{
	  ancien_pointeur = pointeur_liste;
	  pointeur_liste = pointeur_liste -> next;
	}
    }


  if ( echeances_saisies )
    {
      remplissage_liste_echeance ();
      modification_fichier ( TRUE );
    }
}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
gint recherche_echeance_par_no ( struct operation_echeance *echeance,
				 gint no_echeance )
{
  return ( !(echeance -> no_operation == no_echeance ));
}
/* ***************************************************************************************************** */





/* ***************************************************************************************************** */
/* Fonction verifie_ligne_selectionnee_echeance_visible */
/* appelée lorsqu'on édite une opé et que le formulaire n'était pas visible, pour */
/* éviter que la ligne éditée se retrouve hors du champ de vision */
/* ***************************************************************************************************** */

void verifie_ligne_selectionnee_echeance_visible ( void )
{
  selectionne_echeance ();

  gtk_signal_disconnect_by_func ( GTK_OBJECT ( frame_formulaire_echeancier ),
				  GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_echeance_visible ),
				  NULL );
}
/* ***************************************************************************************************** */
					  

/* ***************************************************************************************************** */
/* Fonction appelée lorsqu'on change le bouton pour l'affichage des échéances ( choix mois, 2 mois ... ) */
/* ***************************************************************************************************** */

void modification_affichage_echeances ( gint *origine )
{

  switch ( GPOINTER_TO_INT ( origine ))
    {
      /* vient de l'entrée qui perd le focus */

    case 5:

      affichage_echeances_perso_nb_libre = atoi ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( entree_personnalisation_affichage_echeances ))));

      break;

      /* vient d'un chgt du bouton perso jour/mois/an */

    case 6:

      affichage_echeances_perso_j_m_a = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton_personnalisation_affichage_echeances ) -> menu_item ),
											    "intervalle_perso" ));
      break;

      /*       vient du reste, si c'est perso ( 4 ) , on affiche ce qu'il faut */

    case 4:

      affichage_echeances = GPOINTER_TO_INT ( origine );
      if ( affichage_echeances_perso_nb_libre )
	gtk_entry_set_text ( GTK_ENTRY ( entree_personnalisation_affichage_echeances ),
			     itoa ( affichage_echeances_perso_nb_libre ));
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( bouton_personnalisation_affichage_echeances ),
				    affichage_echeances_perso_j_m_a );
      gtk_widget_show ( entree_personnalisation_affichage_echeances );
      gtk_widget_show ( bouton_personnalisation_affichage_echeances );
      break;

    case 0:
    case 1:
    case 2:
    case 3:
      affichage_echeances = GPOINTER_TO_INT ( origine );
      gtk_widget_hide ( entree_personnalisation_affichage_echeances );
      gtk_widget_hide ( bouton_personnalisation_affichage_echeances );

      break;

    default :
    }

  remplissage_liste_echeance ();

  modification_fichier ( TRUE );
}
/* ***************************************************************************************************** */
					

/* ***************************************************************************************************** */
/* reçoit en argument une échéance et une date, renvoie la date suivante en accord */
/* avec la périodicité de l'échéance */
/* renvoie null si la date limite est dépassée ou si c'est une fois */
/* ***************************************************************************************************** */

GDate *date_suivante_echeance ( struct operation_echeance *echeance,
				GDate *date_courante )
{
  if ( !echeance -> periodicite )
    {
      date_courante = NULL;

      return ( date_courante );
    }

  /* périodicité hebdomadaire */
  if ( echeance -> periodicite == 1 )
    {
      g_date_add_days ( date_courante,
			7 );
      /* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
      g_date_add_months ( date_courante,
			  0 );
    }
  else
    /* périodicité mensuelle */
    if ( echeance -> periodicite == 2 )
      g_date_add_months ( date_courante,
			  1 );
    else
      /* périodicité annuelle */
      if ( echeance -> periodicite == 3 )
	g_date_add_years ( date_courante,
			   1 );
      else
	/* périodicité perso */
	if ( !echeance -> intervalle_periodicite_personnalisee )
	  {
	    g_date_add_days ( date_courante,
			      echeance -> periodicite_personnalisee );

	    /* magouille car il semble y avoir un bug dans g_date_add_days qui ne fait pas l'addition si on ne met pas la ligne suivante */
	    g_date_add_months ( date_courante,
				0 );
	  }
	else
	  if ( echeance -> intervalle_periodicite_personnalisee == 1 )
	    g_date_add_months ( date_courante,
				echeance -> periodicite_personnalisee );
	  else
	    g_date_add_years ( date_courante,
			       echeance -> periodicite_personnalisee );
  
  if ( echeance -> date_limite
       &&
       g_date_compare ( date_courante,
			echeance -> date_limite ) > 0 )
    date_courante = NULL;

  return ( date_courante );
}
/* ***************************************************************************************************** */



/* ***************************************************************************************************** */
gint classement_liste_echeances ( GtkWidget *liste,
				  GtkCListRow *ligne_1,
				  GtkCListRow *ligne_2 )
{
  gint result;

  if ( ligne_1 -> data == GINT_TO_POINTER (-1) )
    return (1);
  if ( ligne_2 -> data == GINT_TO_POINTER (-1) )
    return (-1);

  /* commence par comparer l'année, le mois puis le jour */

  if ( ( result = strncmp ( ligne_1 -> cell -> u.text + 6,
			    ligne_2 -> cell -> u.text + 6,
			    4 )))
    return (result);

  if ( ( result = strncmp ( ligne_1 -> cell -> u.text + 3,
			    ligne_2 -> cell -> u.text + 3,
			    2 )))
    return (result);

  return ( strncmp ( ligne_1 -> cell -> u.text,
		     ligne_2 -> cell -> u.text,
		     2 ));

}
/* ***************************************************************************************************** */
