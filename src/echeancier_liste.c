/* ************************************************************************** */
/* fichier qui s'occupe de tout ce qui concerne l'échéancier                  */
/* 			echeances_liste.c                                     */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Alain Portal (dionysos@grisbi.org) 	      */
/* 			http://www.grisbi.org   			      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "accueil.h"
#include "devises.h"
#include "echeancier_formulaire.h"
#include "echeancier_liste.h"
#include "exercice.h"
#include "gtkcombofix.h"
#include "operations_formulaire.h"
#include "operations_liste.h"
#include "traitement_variables.h"
#include "type_operations.h"
#include "fichiers_io.h"
#include "barre_outils.h"
#include "tiers_onglet.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "calendar.h"

  
#define COL_NB_DATE 0
#define COL_NB_ACCOUNT 1
#define COL_NB_PARTY 2
#define COL_NB_FREQUENCY 3
#define COL_NB_MODE 4
#define COL_NB_NOTES 5
#define COL_NB_AMOUNT 6		/* doit être le dernier de la liste
				à cause de plusieurs boucles for */
#define NB_COLS_SCHEDULER 7
  
gint scheduler_col_width_ratio[NB_COLS_SCHEDULER] = { 10, 26, 20, 14, 14, 28, 8};
gint scheduler_col_width[NB_COLS_SCHEDULER] ;

void changement_taille_colonne_echeancier ( GtkWidget *clist, gint colonne, gint largeur ) ;

/*****************************************************************************/
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
/*****************************************************************************/


/*****************************************************************************/
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

  label = gtk_label_new ( COLON(_("Display scheduled transactions")) );
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

  item = gtk_menu_item_new_with_label ( _("Of month") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      NULL );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Of next two months") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (1) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Of year") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (2) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  item = gtk_menu_item_new_with_label ( _("Once") );
  gtk_signal_connect_object ( GTK_OBJECT ( item ),
			      "activate",
			      GTK_SIGNAL_FUNC ( modification_affichage_echeances ),
			      GINT_TO_POINTER (3) );
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );


  item = gtk_menu_item_new_with_label ( _("Custom") );
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

  /* met un bouton valider qui est juste utilisé pour faire sortir le focus de l'entrée */

  bouton_valider_echeance_perso = gtk_button_new_with_label ( _("Show"));
  gtk_button_set_relief ( GTK_BUTTON ( bouton_valider_echeance_perso ),
				GTK_RELIEF_NONE );
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		       bouton_valider_echeance_perso,
		       FALSE,
		       FALSE,
		       0 );


  /* le menu jour / mois / année */

  bouton_personnalisation_affichage_echeances = gtk_option_menu_new ();
  menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ( _("Days") );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"intervalle_perso",
			GINT_TO_POINTER ( 0 ));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Months") );
  gtk_object_set_data ( GTK_OBJECT ( item ),
			"intervalle_perso",
			GINT_TO_POINTER ( 1 ));
  gtk_menu_append ( GTK_MENU ( menu ),
		    item );
  gtk_widget_show ( item );

  item = gtk_menu_item_new_with_label ( _("Years") );
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
  gtk_widget_set_usize ( entree_personnalisation_affichage_echeances,
			 30,
			 FALSE );
  gtk_signal_connect_object ( GTK_OBJECT ( entree_personnalisation_affichage_echeances ),
			      "focus-out-event",
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
      gtk_widget_show ( bouton_valider_echeance_perso );
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

  icone = gtk_image_new_from_stock ( GTK_STOCK_APPLY, GTK_ICON_SIZE_BUTTON );  
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       icone,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( icone );
    

  label = gtk_label_new ( _("Validate") );
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
/*****************************************************************************/

/*****************************************************************************/
/* fonction creation_liste_echeances					     */
/* renvoie la clist pour y mettre les échéances				     */
/*****************************************************************************/
GtkWidget *creation_liste_echeances ( void )
{
  GtkWidget *vbox;
  GtkWidget *win_echeances_scroll;
  gint i;
  
  gchar *titres_echeance[] = { _("Date"),
			       _("Account"),
			       _("Third party"),
			       _("Frequency"),
			       _("Mode"),
			       _("Notes"),
			       _("Amount")};

  GtkJustification col_justs[] = { GTK_JUSTIFY_CENTER,
				   GTK_JUSTIFY_LEFT,
				   GTK_JUSTIFY_LEFT,
				   GTK_JUSTIFY_CENTER,
				   GTK_JUSTIFY_CENTER,
				   GTK_JUSTIFY_LEFT,
				   GTK_JUSTIFY_RIGHT };


  /*   à la base, on a une vbox */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   5 );
  gtk_widget_show ( vbox );


/* création de la barre d'outils */

  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       creation_barre_outils_echeancier(),
		       FALSE,
		       FALSE,
		       0 );
 
  /* ALAIN-FIXME : création d'une autre scrolled window horizontale */
  /* pour permettre le redimensionnement dynamique horizontal */

  /* création de la scrolled window */

  win_echeances_scroll = gtk_scrolled_window_new ( NULL,
						   NULL);
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( win_echeances_scroll ),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC );
/*				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );*/
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       win_echeances_scroll,
		       TRUE,
		       TRUE,
		       5 );
  gtk_widget_show ( win_echeances_scroll );


  /* création de la liste des échéances */

  /* dOm : on passe de 6 a 7 colonnes */
  liste_echeances = gtk_clist_new_with_titles( NB_COLS_SCHEDULER,
					       titres_echeance );

  gtk_clist_set_selection_mode ( GTK_CLIST ( liste_echeances ),
				  GTK_SELECTION_SINGLE );
  gtk_clist_set_compare_func ( GTK_CLIST ( liste_echeances ),
			       (GtkCListCompareFunc) classement_liste_echeances );

  gtk_signal_connect ( GTK_OBJECT ( liste_echeances ),
		       "button-press-event",
		       GTK_SIGNAL_FUNC ( click_ligne_echeance ),
		       NULL );

  gtk_signal_connect ( GTK_OBJECT ( liste_echeances ),
		       "size-allocate",
		       GTK_SIGNAL_FUNC ( changement_taille_liste_echeances ),
		       NULL );

  /* sauvegarde les redimensionnements des colonnes */

  gtk_signal_connect ( GTK_OBJECT ( liste_echeances ),
		       "resize-column",
		       GTK_SIGNAL_FUNC ( changement_taille_colonne_echeancier ),
		       NULL );
 
  /* vérifie les touche entrée, haut et bas */

  gtk_signal_connect ( GTK_OBJECT ( liste_echeances ),
		       "key-press-event",
		       GTK_SIGNAL_FUNC ( traitement_clavier_liste_echeances ),
		       NULL );

  /* si on va sur l'onglet des échéances, donne le focus à la liste */

  gtk_signal_connect_object ( GTK_OBJECT ( liste_echeances ),
			      "map",
			      GTK_SIGNAL_FUNC ( gtk_widget_grab_focus ),
			      GTK_OBJECT ( liste_echeances ) );


  for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
    {
     gtk_clist_set_column_justification ( GTK_CLIST ( liste_echeances ),
					  i,
					  col_justs[i] );
    }
    
  /* dOm : rendre invisible la colonne notes */
  gtk_clist_set_column_visibility( GTK_CLIST ( liste_echeances ),
  				   COL_NB_NOTES,
				   FALSE);

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
/*****************************************************************************/

/*****************************************************************************/
/* cette fonction teste la touche entrée sur la liste d'échéances	     */
/*****************************************************************************/
gboolean traitement_clavier_liste_echeances ( GtkCList *liste_echeances,
					  GdkEventKey *evenement,
					  gpointer null )
{
  gint ligne;

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste_echeances ),
				 "key-press-event");

  switch ( evenement->keyval )
    {
     case GDK_Return :		/* touches entrée */
     case GDK_KP_Enter :

      formulaire_echeancier_a_zero();
      degrise_formulaire_echeancier();

      gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
			   _("Modification") );
      edition_echeance ();
//      return TRUE; c'est le cas de la 0.4.4, et pas de break
      break;


     case GDK_Up :		/* touches flèche haut */
     case GDK_KP_Up :

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
//      return TRUE; c'est le cas de la 0.4.4, et pas de break
      break;


     case GDK_Down :		/* touches flèche bas */
     case GDK_KP_Down :

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
//      return TRUE; c'est le cas de la 0.4.4, et pas de break
      break;


    case GDK_Delete :               /*  del  */

      supprime_echeance ();
//      return TRUE; c'est le cas de la 0.4.4, et pas de break
      break;

    }

//      return FALSE;
  return ( TRUE );    
}
/*****************************************************************************/

/*****************************************************************************/
/* dOm fonction callback						     */
/*****************************************************************************/
void affiche_cache_commentaire_echeancier( void )
{
  static gboolean visible = FALSE;
  
  if ( visible == FALSE )
    {
     visible = TRUE;
     gtk_clist_set_column_width ( GTK_CLIST ( liste_echeances ),
				  COL_NB_NOTES,
				  scheduler_col_width[COL_NB_NOTES] ) ;
     gtk_clist_set_column_visibility( GTK_CLIST ( liste_echeances ), COL_NB_FREQUENCY, FALSE);
     gtk_clist_set_column_visibility( GTK_CLIST ( liste_echeances ), COL_NB_MODE, FALSE);
     gtk_clist_set_column_visibility( GTK_CLIST ( liste_echeances ), COL_NB_NOTES, TRUE);
    }
  else
    {
     visible = FALSE;
     gtk_clist_set_column_width ( GTK_CLIST ( liste_echeances ),
				  COL_NB_FREQUENCY,
				  scheduler_col_width[COL_NB_FREQUENCY] ) ;
     gtk_clist_set_column_width ( GTK_CLIST ( liste_echeances ),
				  COL_NB_MODE,
				  scheduler_col_width[COL_NB_MODE] ) ;
     gtk_clist_set_column_visibility( GTK_CLIST ( liste_echeances ), COL_NB_FREQUENCY, TRUE);
     gtk_clist_set_column_visibility( GTK_CLIST ( liste_echeances ), COL_NB_MODE, TRUE);
     gtk_clist_set_column_visibility( GTK_CLIST ( liste_echeances ), COL_NB_NOTES, FALSE);
    }
}
/*****************************************************************************/

/*****************************************************************************/
void click_sur_saisir_echeance ( void )
{
  formulaire_echeancier_a_zero();
  degrise_formulaire_echeancier();

  gtk_label_set_text ( GTK_LABEL ( label_saisie_modif ),
		       _("Input") );
  edition_echeance ();

  gtk_widget_hide ( widget_formulaire_echeancier[16] );
  gtk_widget_hide ( widget_formulaire_echeancier[15] );
  gtk_widget_hide ( widget_formulaire_echeancier[17] );
  gtk_widget_hide ( widget_formulaire_echeancier[18] );
  gtk_widget_hide ( widget_formulaire_echeancier[5] );
  gtk_widget_hide ( widget_formulaire_echeancier[13] );

}
/*****************************************************************************/

/*****************************************************************************/
void remplissage_liste_echeance ( void )
{
  gchar *ligne[NB_COLS_SCHEDULER];
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


  /* on calcule la date de fin */

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

      ligne[COL_NB_DATE] = g_strdup_printf ( "%02d/%02d/%d",
				   ECHEANCE_COURANTE -> jour,
				   ECHEANCE_COURANTE -> mois,
				   ECHEANCE_COURANTE -> annee );
 
      switch ( ECHEANCE_COURANTE ->periodicite )
	{
	case 0 : ligne[COL_NB_FREQUENCY] = _("Once");
	  break;
	case 1 : ligne[COL_NB_FREQUENCY] = _("Weekly");
	  break;
	case 2 : ligne[COL_NB_FREQUENCY] = _("Monthly");
	  break;
	case 3 : ligne[COL_NB_FREQUENCY] = _("Yearly");
	  break;
	case 4 :
	  if ( ECHEANCE_COURANTE -> intervalle_periodicite_personnalisee )
	    if ( ECHEANCE_COURANTE -> intervalle_periodicite_personnalisee == 1 )
	      ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d months"),
					   ECHEANCE_COURANTE -> periodicite_personnalisee );
	    else
	      ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d years"),
					   ECHEANCE_COURANTE -> periodicite_personnalisee );
	  else
	    ligne[COL_NB_FREQUENCY] = g_strdup_printf ( _("%d days"),
					 ECHEANCE_COURANTE -> periodicite_personnalisee );
	  break;
	}

      /* mise en forme du compte */

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + ECHEANCE_COURANTE -> compte;
      ligne[COL_NB_ACCOUNT] = NOM_DU_COMPTE ;

      /* mise en forme du tiers */

      pointeur_tmp = g_slist_find_custom ( liste_struct_tiers,
					   GINT_TO_POINTER ( ECHEANCE_COURANTE -> tiers ),
					   (GCompareFunc) recherche_tiers_par_no );
      if ( pointeur_tmp )
	ligne[COL_NB_PARTY] = ((struct struct_tiers *)( pointeur_tmp-> data )) -> nom_tiers;
      else
	ligne[COL_NB_PARTY] = NULL;

      /* mise en forme de auto/man */

      if ( ECHEANCE_COURANTE -> auto_man )
	ligne[COL_NB_MODE]=_("Automatic");
      else
	ligne[COL_NB_MODE] = _("Manual");

      /* dOm TODO remplir la colonne notes */
      ligne[COL_NB_NOTES] = g_strdup_printf ( "%s", 
					      ( ECHEANCE_COURANTE -> notes == NULL ) ? "" : ECHEANCE_COURANTE -> notes );


      /* mise en forme du montant */
      /* dOm la colonne 5 est devenue colonne 6 */
      ligne[COL_NB_AMOUNT] = g_strdup_printf ( "%4.2f",
					       ECHEANCE_COURANTE -> montant );

      /* on va ajouter l'échéance une ou plusieurs fois en changeant juste sa date */
      /* en fontion de l'affichage de l'échéance */

      if ( !( g_date_compare ( date_fin, ECHEANCE_COURANTE -> date ) <= 0 &&
	      affichage_echeances != 3 ))
	{
	  gint sortie;
	  GDate *pGDateCurrent;
	  struct operation_echeance *echeance;

	  echeance = ECHEANCE_COURANTE;
	  sortie = 1;
	  pGDateCurrent = g_date_new_dmy ( ECHEANCE_COURANTE -> date -> day,
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

	      pGDateCurrent = date_suivante_echeance ( ECHEANCE_COURANTE,
						       pGDateCurrent );

	      if ( pGDateCurrent &&
		   g_date_compare ( date_fin, pGDateCurrent ) > 0 &&
		   affichage_echeances != 3 &&
		   ECHEANCE_COURANTE -> periodicite )
		{
		  ligne[COL_NB_DATE] = g_strdup_printf ( "%02d/%02d/%d",
					       g_date_day ( pGDateCurrent ),
					       g_date_month ( pGDateCurrent ),
					       g_date_year ( pGDateCurrent ));
		  echeance = NULL;
		}
	      else
		sortie = 0;
	    }
	}
      pointeur_liste = pointeur_liste -> next;
    }


  /* met la ligne blanche */

  for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
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

      if ( echeance &&
	   echeance != GINT_TO_POINTER ( -1 ) &&
	   echeance -> montant < 0 )
	      /* dOm : la colonne montant est maintenant la 6 */
	gtk_clist_set_cell_style ( GTK_CLIST ( liste_echeances ),
				   i,
				   COL_NB_AMOUNT,
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
/*****************************************************************************/

/*****************************************************************************/
void click_ligne_echeance ( GtkCList *liste,
			    GdkEventButton *evenement,
			    gpointer data )
{
  gint ligne, colonne, x, y;

  /* si le click se situe dans les menus, c'est qu'on redimensionne, on fait rien */

  if ( evenement -> window != liste -> clist_window )
    return;

  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( liste ),
				 "button-press-event");

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
			   _("Modification") );
      edition_echeance ();
    }
  else
    gtk_widget_grab_focus ( liste_echeances );

}
/*****************************************************************************/

/*****************************************************************************/
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
/*****************************************************************************/

/*****************************************************************************/
/* Fonction edition_echeance						     */
/* appelée par double click ou entrée sur une échéance. Elle recopie	     */
/* l'échéance sélectionnée dans le formulaire d'échéance pour la modifier    */
/*****************************************************************************/
void edition_echeance ( void )
{
  GSList *pointeur_tmp;

  /*   si le formulaire est caché, on le montre */

      gtk_widget_show ( frame_formulaire_echeancier );
      gtk_signal_connect ( GTK_OBJECT ( frame_formulaire_echeancier ),
			   "map",
			   GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_echeance_visible ),
			   NULL );

  

  /* si c'est une nouvelle échéance, on met la date courante,
     la sélectionne et lui donne le focus */

  if ( echeance_selectionnnee == GINT_TO_POINTER ( -1 ))
    {
      entree_prend_focus ( widget_formulaire_echeancier[0] );
      gtk_entry_set_text ( GTK_ENTRY ( widget_formulaire_echeancier[0] ),
			   gsb_today() );
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


  /* si le compte de virement est -1, c'est qu'il n'y a pas de categ */
 
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

	  texte = g_strconcat ( COLON(_("Transfer")),
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
				 cherche_no_menu_exercice ( echeance_selectionnnee -> no_exercice,
							    widget_formulaire_echeancier[9] ));

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
/*****************************************************************************/

/*****************************************************************************/
void supprime_echeance ( void )
{
  gint resultat;


  if ( echeance_selectionnnee == GINT_TO_POINTER ( -1 ) )
    return;

  /* si la périodicité n'est pas de 1 fois demande juste celle ci ou toutes,
     sinon, choisit automatiquement toutes */

  if ( echeance_selectionnnee -> periodicite )
    {
      GtkWidget *dialog;
      GtkWidget *label;


      dialog = gnome_dialog_new ( _("Delete a scheduled transaction maturity"),
				  _("Only this one"),
				  _("All the occurences"),
				  GNOME_STOCK_BUTTON_CANCEL,
				  NULL );

      label = gtk_label_new ( SPACIFY(_("Do you want to delete just this occurrence or the whole scheduled transaction?")) );
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
	label = gtk_label_new ( g_strdup_printf ( _("%02d/%02d/%d : [No third party] [%4.2f]"),
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

    }

  mise_a_jour_calendrier();
  update_liste_echeances_manuelles_accueil();
  modification_fichier (TRUE);
}
/*****************************************************************************/

/*****************************************************************************/
void changement_taille_colonne_echeancier ( GtkWidget *clist,
				 gint colonne,
				 gint largeur )
{
  gint offset ;

  /* sauvegarde de la largeur de la liste */
  ancienne_largeur_echeances = clist -> allocation.width ;

  /* calcul de la valeur relative du redimensionnement de la colonne concernée */
  offset = largeur - scheduler_col_width[colonne] ;

  /* suivant la colonne en cours de redimensionnement */
  switch ( colonne )
    {
      case COL_NB_FREQUENCY :
      case COL_NB_MODE :

	/* si c'est la colonne Périodicité ou Mode, alors on redimensionne
	   également la colonne Remarques en lui ajoutant la valeur relative
	   du redimensionnement */
	scheduler_col_width[COL_NB_NOTES] += offset ;
	break;

      case COL_NB_NOTES :

	/* si c'est la colonne Remarques, alors on redimensionne également
	   les colonnes Périodicité ou Mode en leur ajoutant de manière
	   proportionnelle la valeur relative du redimensionnement */
	scheduler_col_width[COL_NB_FREQUENCY] = (( scheduler_col_width[colonne] + offset ) * scheduler_col_width[COL_NB_FREQUENCY] ) / scheduler_col_width[colonne] ;
	scheduler_col_width[COL_NB_MODE] = (( scheduler_col_width[colonne] + offset ) * scheduler_col_width[COL_NB_MODE] ) / scheduler_col_width[colonne] ;
	break;

      default :

	break;
    }
  
  /* on sauve la valeur de la nouvelle largeur de la colonne concernée */
  scheduler_col_width[colonne] = largeur ;
}
/*****************************************************************************/

/*****************************************************************************/
/* Fonction changement_taille_liste_echeances				     */
/* appelée dès que la taille de la clist a changé			     */
/* pour mettre la taille des différentes colonnes			     */
/*****************************************************************************/
void changement_taille_liste_echeances ( GtkWidget *clist,
					 GtkAllocation *allocation,
					 gpointer null )
{
  gint largeur, tmp_largeur = 0 ;
  gint i;
  gint col1, col2, col3, col4, col5, col6, col7;

  /*   si la largeur de grisbi est < 700, on fait rien */
  
  if ( window -> allocation.width < 700 )
    return;

  /* si ancienne_largeur est nul, c'est que la vue sur la liste n'a pas encore
     été créée, il faut donc initialiser les largeurs des colonnes */
  
  if ( ancienne_largeur_echeances == 0 )
    {
     /* on affecte à ancienne_valeur la largeur qu'aura la liste des échéances
        lorsqu'on cliquera sur l'onglet Échéancier (cette largeur fait 4 pixels
	de moins que la largeur que l'on récupère maintenant */

     ancienne_largeur_echeances = clist -> allocation.width - 4 ;

     /* on teste si des valeurs valides (non nulles) ont été récupérées dans
        le fichier de configuration */
     
     for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
       {
        if ( scheduler_col_width[i] == 0 )
	  {
	   scheduler_col_width[0] = 0 ;
	   /* inutile d'aller plus loin, on sort de la boucle */
	   i = NB_COLS_SCHEDULER ;
	  }
       }

     if ( scheduler_col_width[0] == 0 )
       {
	/* si ce n'est pas le cas, alors on initialise ces largeurs avec
	   un rapport de largeur prédéfini */

	for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
	  scheduler_col_width[i] = scheduler_col_width_ratio[i] * ancienne_largeur_echeances / 100 ;
       }
     else
       {
	/* et si on a des valeurs valides, alors on réajuste ces largeurs avec
	   la largeur courante de la liste et le rapport de largeur du fichier
	   de configuration */

	for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
	  tmp_largeur += scheduler_col_width[i] ;

	/* à cette largeur calculée, on retire la largeur de la colonne Notes
	   puisque celle-ci est « cachée » derrière les colonnes Tiers et Mode */

	tmp_largeur -= scheduler_col_width[COL_NB_NOTES] ;
	
	/* c'est une astuce grossière pour ne pas voir apparaitre les barres
	   de défilement horizontal dans l'échéancier lors du lancement,
	   en attendant de trouver comment faire de façon plus rigoureuse */

	ancienne_largeur_echeances -= 37 ;

	for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
	  scheduler_col_width[i] = ( scheduler_col_width[i] * ancienne_largeur_echeances ) / tmp_largeur ;

	ancienne_largeur_echeances += 37 ;
       }
    }

  if ( allocation )
    largeur = allocation -> width ;
  else
    largeur = clist -> allocation.width ;

  if ( largeur == ancienne_largeur_echeances )
    return ;

  /* on ajuste les largeurs de colonnes proportionnellement au rapport
     nouvelle largeur / ancienne largeur */

  for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
    scheduler_col_width[i] = ( scheduler_col_width[i] * largeur ) / ancienne_largeur_echeances ;

  /* pour être certain que la largeur de la colonne Remarque soit égale
     à la somme des largeurs des colonnes Tiers et Périodicité */

  scheduler_col_width[COL_NB_NOTES] = scheduler_col_width[COL_NB_FREQUENCY] + scheduler_col_width[COL_NB_MODE] ;

  /* on établit alors les largeurs des colonnes */

  for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
    gtk_clist_set_column_width ( GTK_CLIST ( clist ),
				 i,
				 scheduler_col_width[i] ) ;

  /* on sauve la valeur courante de la largeur de la liste pour
     une utilisation ultérieure */

  ancienne_largeur_echeances = largeur ;

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
/*****************************************************************************/

/*****************************************************************************/
/* Fonction : mise_a_jour_calendrier					     */
/* met en gras les jours où il y a une échéance				     */
/*****************************************************************************/
void mise_a_jour_calendrier ( void )
{
  GDate *date_calendrier;
  time_t temps;
  GSList *pointeur;


  date_calendrier = g_date_new_dmy ( 1,
				     GTK_CALENDAR ( calendrier_echeances ) -> month + 1,
				     GTK_CALENDAR ( calendrier_echeances ) -> year );

  gtk_calendar_clear_marks ( GTK_CALENDAR ( calendrier_echeances ));


  /* si le jour courant doit y apparaitre, on le sélectionne */

   time ( &temps );

   if ( ( localtime ( &temps ) -> tm_mon == GTK_CALENDAR ( calendrier_echeances ) -> month )
	&&
	( ( localtime ( &temps ) -> tm_year + 1900 ) == GTK_CALENDAR ( calendrier_echeances ) -> year ) )
     gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
			       localtime ( &temps ) -> tm_mday );
   else
     gtk_calendar_select_day ( GTK_CALENDAR ( calendrier_echeances ),
			       FALSE );

  /* on fait le tour de toutes les échéances, les amène au mois du calendrier
     et mise en gras du jour de l'échéance */

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
      /* ou si c'est personnalisé mais la periodicité est de 0, */
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

      /* à ce niveau, soit l'échéance est sur le mois du calendrier,
         soit elle l'a dépassé, soit elle a dépassé sa date limite */

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
/*****************************************************************************/

/*****************************************************************************/
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
/*****************************************************************************/

/*****************************************************************************/
void verification_echeances_a_terme ( void )
{
  GDate *pGDateCurrent;
  GSList *pointeur_liste;
  GSList *ancien_pointeur;
  struct struct_devise *devise;

  /* les échéances à saisir sont revérifiées à chaque fois. Par contre, les
     échéances saisies sont ajoutées à la liste de celles déjà saisies */

  echeances_a_saisir = NULL;

  /* récupère la date du jour et la met en format gdate */

  pGDateCurrent = g_date_new ();
  g_date_set_time ( pGDateCurrent,
		    time ( NULL ));

  /* on y ajoute le décalage de l'échéance */

  g_date_add_days ( pGDateCurrent,
		    decalage_echeance );


  /* on fait le tour des échéances : si elle est auto, elle est enregistrée,
     sinon elle est juste répertoriée */

  pointeur_liste = gsliste_echeances;
  ancien_pointeur = NULL;


  while ( pointeur_liste )
    {
      if ( ECHEANCE_COURANTE -> auto_man )
	{
	  /* tant que cette échéance auto n'est pas arrivée à aujourd'hui, on recommence */
	 
	  while ( pointeur_liste != ancien_pointeur
		  &&
		  g_date_compare ( ECHEANCE_COURANTE -> date, pGDateCurrent ) <= 0 )
	    {
	      struct structure_operation *operation;
	      gint virement;


	      /* crée l'opération */

	      operation = calloc ( 1,
				   sizeof ( struct structure_operation ) ); 


	      /* remplit l'opération */

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

	      if ( !devise_compte
		   ||
		   devise_compte -> no_devise != DEVISE )
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

		  demande_taux_de_change ( devise_compte, devise, 1,
					   (gdouble ) 0, (gdouble ) 0, FALSE );

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

	      /* si l'exo est automatique (-2), c'est ici qu'on va le chercher */

	      if ( ECHEANCE_COURANTE -> no_exercice == -2 )
		operation -> no_exercice = recherche_exo_correspondant ( operation -> date );
	      else
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
		  operation_2 -> no_exercice = operation -> no_exercice;

		  /*   on a fini de remplir l'opé, on peut l'ajouter à la liste */

		  ajout_operation ( operation_2 );


		  /* on met maintenant les relations entre les différentes opé */

		  operation -> relation_no_operation = operation_2 -> no_operation;
		  operation -> relation_no_compte = operation_2 -> no_compte;
		  operation_2 -> relation_no_operation = operation -> no_operation;
		  operation_2 -> relation_no_compte = operation -> no_compte;

		  /* on met à jour le compte courant pour le virement (il a été mis à jour avec ajout opération, mais sans les liens de virement) */

		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

		  MISE_A_JOUR = 1;
		  verification_mise_a_jour_liste ();
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
			      pGDateCurrent ) <= 0 )
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
/*****************************************************************************/

/*****************************************************************************/
gint recherche_echeance_par_no ( struct operation_echeance *echeance,
				 gint no_echeance )
{
  return ( !(echeance -> no_operation == no_echeance ));
}
/*****************************************************************************/

/*****************************************************************************/
/* Fonction verifie_ligne_selectionnee_echeance_visible			     */
/* appelée lorsqu'on édite une opé et que le formulaire n'était pas visible, */
/* pour éviter que la ligne éditée se retrouve hors du champ de vision       */
/*****************************************************************************/
void verifie_ligne_selectionnee_echeance_visible ( void )
{
  selectionne_echeance ();

  gtk_signal_disconnect_by_func ( GTK_OBJECT ( frame_formulaire_echeancier ),
				  GTK_SIGNAL_FUNC ( verifie_ligne_selectionnee_echeance_visible ),
				  NULL );
}
/*****************************************************************************/

/*****************************************************************************/
/* Fonction appelée lorsqu'on change le bouton pour l'affichage des	     */
/* échéances ( choix mois, 2 mois ... )					     */
/*****************************************************************************/
gboolean modification_affichage_echeances ( gint *origine, GdkEventFocus * event,
					    GtkWidget * widget )
{
  switch ( GPOINTER_TO_INT ( origine ))
    {
      /* vient de l'entrée qui perd le focus */

    case 5:

      affichage_echeances_perso_nb_libre = atoi ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_personnalisation_affichage_echeances ))));

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
      gtk_widget_show ( bouton_valider_echeance_perso );
      break;

    case 0:
    case 1:
    case 2:
    case 3:
      affichage_echeances = GPOINTER_TO_INT ( origine );
      gtk_widget_hide ( entree_personnalisation_affichage_echeances );
      gtk_widget_hide ( bouton_personnalisation_affichage_echeances );
      gtk_widget_hide ( bouton_valider_echeance_perso );

      break;

    }

  remplissage_liste_echeance ();

  modification_fichier ( TRUE );
  
  return FALSE;
}
/*****************************************************************************/

/*****************************************************************************/
/* reçoit en argument une échéance et une date, renvoie la date suivante     */
/* en accord avec la périodicité de l'échéance				     */
/* renvoie null si la date limite est dépassée ou si c'est une fois	     */
/*****************************************************************************/
GDate *date_suivante_echeance ( struct operation_echeance *echeance,
				GDate *pGDateCurrent )
{
  if ( !echeance -> periodicite )
    {
      pGDateCurrent = NULL;

      return ( pGDateCurrent );
    }

  /* périodicité hebdomadaire */
  if ( echeance -> periodicite == 1 )
    {
      g_date_add_days ( pGDateCurrent,
			7 );
      /* magouille car il semble y avoir un bug dans g_date_add_days
         qui ne fait pas l'addition si on ne met pas la ligne suivante */
      g_date_add_months ( pGDateCurrent,
			  0 );
    }
  else
    /* périodicité mensuelle */
    if ( echeance -> periodicite == 2 )
      g_date_add_months ( pGDateCurrent,
			  1 );
    else
      /* périodicité annuelle */
      if ( echeance -> periodicite == 3 )
	g_date_add_years ( pGDateCurrent,
			   1 );
      else
	/* périodicité perso */
	if ( !echeance -> intervalle_periodicite_personnalisee )
	  {
	    g_date_add_days ( pGDateCurrent,
			      echeance -> periodicite_personnalisee );

	    /* magouille car il semble y avoir un bug dans g_date_add_days
	       qui ne fait pas l'addition si on ne met pas la ligne suivante */
	    g_date_add_months ( pGDateCurrent,
				0 );
	  }
	else
	  if ( echeance -> intervalle_periodicite_personnalisee == 1 )
	    g_date_add_months ( pGDateCurrent,
				echeance -> periodicite_personnalisee );
	  else
	    g_date_add_years ( pGDateCurrent,
			       echeance -> periodicite_personnalisee );
  
  if ( echeance -> date_limite
       &&
       g_date_compare ( pGDateCurrent,
			echeance -> date_limite ) > 0 )
    pGDateCurrent = NULL;

  return ( pGDateCurrent );
}
/*****************************************************************************/

/*****************************************************************************/
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
/*****************************************************************************/
