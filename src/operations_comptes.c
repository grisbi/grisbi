/*  Fichier qui gère la liste des comptes, la partie gauche de l'onglet opérations */
/*      operations_comptes.c */

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
#include "barre_outils.h"
#include "devises.h"
#include "gtkcombofix.h"
#include "gtk_list_button.h"
#include "operations_comptes.h"
#include "operations_formulaire.h"
#include "operations_liste.h"
#include "type_operations.h"
#include "equilibrage.h"



/* ********************************************************************************************************** */
/*** Création de la fenêtre de comptes ***/
/* **************************************************************************************************** */

GtkWidget *creation_liste_comptes (void)
{
  GtkWidget *onglet;
  GtkWidget *frame_label_compte_courant;
  GSList *ordre_comptes_variable;
  GtkWidget *bouton;
  GtkWidget *frame_equilibrage;
  GtkWidget *vbox_frame_equilibrage;
  GtkWidget *scrolled_window;

  /*  Création d'une fenêtre générale*/

  onglet = gtk_vbox_new ( FALSE,
			  10);
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );

  /*  Création du label Comptes en haut */


  /*   on place le label dans une frame */

  frame_label_compte_courant = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_label_compte_courant ),
			      GTK_SHADOW_ETCHED_OUT );
  gtk_box_pack_start ( GTK_BOX (onglet),
		       frame_label_compte_courant,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show (frame_label_compte_courant);


  /*   on ne met rien dans le label, il sera rempli ensuite */

  label_compte_courant = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC (label_compte_courant  ),
			   0.5,
			   0.5);
  gtk_container_add ( GTK_CONTAINER ( frame_label_compte_courant ),
		      label_compte_courant );
  gtk_widget_show (label_compte_courant);


  /*  Création de la fenêtre des comptes */
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
  

  /*  création d'une vbox contenant la liste des comptes */

  vbox_liste_comptes = gtk_vbox_new ( FALSE,
				      10);
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW (scrolled_window ),
					  vbox_liste_comptes);
  gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window )  -> child ),
				 GTK_SHADOW_NONE );
  gtk_widget_show (vbox_liste_comptes);
  

  /*  Création d'une icone et du nom par compte, et placement dans la liste selon l'ordre désiré  */

  if ( nb_comptes )
    {
      ordre_comptes_variable = ordre_comptes;

      do
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );
	  
	  bouton = comptes_appel( GPOINTER_TO_INT ( ordre_comptes_variable->data ));
	  gtk_box_pack_start (GTK_BOX (vbox_liste_comptes),
			      bouton,
			      FALSE,
			      FALSE,
			      0);

	  if ( COMPTE_CLOTURE )
	    {
	      GnomeUIInfo *menu;

	      menu = malloc ( 2 * sizeof ( GnomeUIInfo ));

	      menu -> type = GNOME_APP_UI_ITEM;
	      menu -> label = NOM_DU_COMPTE;
	      menu -> hint = NOM_DU_COMPTE;
	      menu -> moreinfo = (gpointer) changement_compte_par_menu;
	      menu -> user_data = ordre_comptes_variable->data;
	      menu -> unused_data = NULL;
	      menu -> pixmap_type = 0;
	      menu -> pixmap_info = 0;
	      menu -> accelerator_key = 0;
	      menu -> ac_mods = GDK_BUTTON1_MASK;
	      menu -> widget = NULL;

	      (menu + 1)->type = GNOME_APP_UI_ENDOFINFO;


	      gnome_app_insert_menus ( GNOME_APP ( window ),
				       _("Accounts/Closed accounts/"),
				       menu );

	      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_comptes[3].widget ),
					 TRUE );
	    }
	  else
	    gtk_widget_show (bouton);

	  
	}
      while ( (  ordre_comptes_variable = ordre_comptes_variable->next ) );
    }


  /* ajoute le bouton et le label pour l'équilibrage de compte */
  /* les 2 seront intégrés dans une frame */

  frame_equilibrage = gtk_frame_new ( NULL );
  gtk_frame_set_shadow_type ( GTK_FRAME ( frame_equilibrage ),
			      GTK_SHADOW_ETCHED_IN );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       frame_equilibrage,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show ( frame_equilibrage );


  vbox_frame_equilibrage = gtk_vbox_new ( FALSE,
					  5 );
  gtk_container_add ( GTK_CONTAINER  ( frame_equilibrage ),
		      vbox_frame_equilibrage );
  gtk_widget_show ( vbox_frame_equilibrage );


  /* mise en place du label */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  if ( nb_comptes )
    label_releve = gtk_label_new ( g_strconcat ( COLON(_("Last statement")),
						 DATE_DERNIER_RELEVE,
						 NULL ) );
  else
    label_releve = gtk_label_new ( COLON(_("Last statement")) );

  gtk_box_pack_start ( GTK_BOX ( vbox_frame_equilibrage ),
		       label_releve,
		       FALSE,
		       TRUE,
		       0);
  gtk_widget_show ( label_releve );


  /* mise en place du bouton équilibrage */

  bouton = gtk_button_new_with_label ( _("Reconcile") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE);
  gtk_box_pack_start ( GTK_BOX ( vbox_frame_equilibrage ),
		       bouton,
		       FALSE,
		       TRUE,
		       0);
  gtk_signal_connect ( GTK_OBJECT (bouton),
		       "clicked",
		       GTK_SIGNAL_FUNC ( equilibrage ),
		       NULL );
  gtk_widget_show ( bouton );


  return ( onglet );
  
}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
/** Fonction qui renvoie un widget contenant un bouton **/
/** de compte associé à son nom **/
/* ********************************************************************************************************** */

GtkWidget *comptes_appel ( gint no_de_compte )
{
  GtkWidget *bouton;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_de_compte;

  bouton = gtk_list_button_new ( NOM_DU_COMPTE, 2 );
  gtk_signal_connect_object ( GTK_OBJECT (bouton), "clicked",
			      GTK_SIGNAL_FUNC ( changement_compte ),
			      GINT_TO_POINTER ( no_de_compte ) );
  gtk_widget_show ( bouton );

  return ( bouton );
}
/* ********************************************************************************************************** */


/* ********************************************************************************************************** */
void changement_compte_par_menu ( GtkWidget *menu,
				  gint *compte )
{
  changement_compte ( compte );
}
/* ********************************************************************************************************** */




/* ********************************************************************************************************** */
/*  Routine appelée lors de changement de compte */
/* ********************************************************************************************************** */

gboolean changement_compte ( gint *compte)
{
  GtkWidget *menu;

  /*   si on n'est pas sur l'onglet comptes du notebook, on y passe */

  if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general ) ) != 1 )
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
			    1 );

  /* si on était dans une ventilation d'opération, alors on annule la ventilation */

  if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ) ) == 1 )
    annuler_ventilation();

  /* ferme le formulaire */

  echap_formulaire ();

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  compte_courant = GPOINTER_TO_INT ( compte );
  p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  /* change le nom du compte courant */
  gtk_label_set_text ( GTK_LABEL ( label_compte_courant), NOM_DU_COMPTE);

/* change les types d'opé et met le défaut */

  if ( (menu = creation_menu_types ( 1, compte_courant, 0  )))
    {
      /* on joue avec les sensitive pour éviter que le 1er mot du menu ne reste grise */

      gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_TYPE],
				 TRUE );
      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
				 menu );
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] ),
				    cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
      gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_TYPE],
				 FALSE );
      gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
    }
  else
    {
      gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_TYPE] );
      gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_CHEQUE] );
    }



  if ( DATE_DERNIER_RELEVE )
    gtk_label_set_text ( GTK_LABEL ( label_releve ),
			 g_strdup_printf ( _("Last statement: %02d/%02d/%d"), 
					     g_date_day ( DATE_DERNIER_RELEVE ),
					     g_date_month ( DATE_DERNIER_RELEVE ),
					     g_date_year ( DATE_DERNIER_RELEVE ) ));

  else
    gtk_label_set_text ( GTK_LABEL ( label_releve ),
			 _("Last statement: none") );


  /* affiche le solde final en bas */

  gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ),
		       g_strdup_printf ( PRESPACIFY(_("Checked balance: %4.2f %s")),
					 SOLDE_POINTE,
					 devise_name ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
											 GINT_TO_POINTER ( DEVISE ),
											 (GCompareFunc) recherche_devise_par_no )-> data ))) );
  gtk_label_set_text ( GTK_LABEL ( solde_label ),
		       g_strdup_printf ( PRESPACIFY(_("Current balance: %4.2f %s")),
					 SOLDE_COURANT,
					 devise_name ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
											 GINT_TO_POINTER ( DEVISE ),
											 (GCompareFunc) recherche_devise_par_no )-> data ))) );


  /* change le défaut de l'option menu des devises du formulaire */

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[TRANSACTION_FORM_DEVISE] ),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( DEVISE ),
									 ( GCompareFunc ) recherche_devise_par_no )));

  /* met les boutons R et nb lignes par opé comme il faut */

  mise_a_jour_boutons_caract_liste ( compte_courant );

  focus_a_la_liste ();
  etat.ancienne_date = 0;

  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			  compte_courant + 1 );

  return FALSE;
}
/* ********************************************************************************************************** */



/* *********************************************************************************************************** */
/*   on réaffiche la liste des comptes s'il y a eu un changement */
/* *********************************************************************************************************** */

void reaffiche_liste_comptes ( void )
{
  GSList *ordre_comptes_variable;
  GtkWidget *bouton;
  gint i;

/* commence par effacer tous les comptes */

  while ( GTK_BOX ( vbox_liste_comptes ) -> children )
    gtk_container_remove ( GTK_CONTAINER ( vbox_liste_comptes ),
			   (( GtkBoxChild *) ( GTK_BOX ( vbox_liste_comptes ) -> children -> data )) -> widget );

  /* on efface les menus des comptes cloturés */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  for ( i=0 ; i<nb_comptes ; i++ )
    {
      if ( COMPTE_CLOTURE )
	gnome_app_remove_menus ( GNOME_APP ( window ),
				 _("Accounts/Closed accounts/"),
				 2 );
      p_tab_nom_de_compte_variable++;
    }

  gtk_widget_set_sensitive ( GTK_WIDGET ( menu_comptes[3].widget ),
			     FALSE );

  /*  Création d'une icone et du nom par compte, et placement dans la liste selon l'ordre désiré  */

  ordre_comptes_variable = ordre_comptes;

  do
    {
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );
      
      bouton = comptes_appel( GPOINTER_TO_INT ( ordre_comptes_variable->data ));
      gtk_box_pack_start (GTK_BOX (vbox_liste_comptes),
			  bouton,
			  FALSE,
			  FALSE,
			  0);

	  if ( COMPTE_CLOTURE )
	    {
	      GnomeUIInfo *menu;

	      menu = malloc ( 2 * sizeof ( GnomeUIInfo ));

	      menu -> type = GNOME_APP_UI_ITEM;
	      menu -> label = NOM_DU_COMPTE;
	      menu -> hint = NOM_DU_COMPTE;
	      menu -> moreinfo = (gpointer) changement_compte_par_menu;
	      menu -> user_data = ordre_comptes_variable->data;
	      menu -> unused_data = NULL;
	      menu -> pixmap_type = 0;
	      menu -> pixmap_info = 0;
	      menu -> accelerator_key = 0;
	      menu -> ac_mods = GDK_BUTTON1_MASK;
	      menu -> widget = NULL;

	      (menu + 1)->type = GNOME_APP_UI_ENDOFINFO;


	      gnome_app_insert_menus ( GNOME_APP ( window ),
				       _("Accounts/Closed accounts/"),
				       menu );

	      gtk_widget_set_sensitive ( GTK_WIDGET ( menu_comptes[3].widget ),
					 TRUE );
	    }
	  else
	    gtk_widget_show (bouton);

    }
  while ( (  ordre_comptes_variable = ordre_comptes_variable->next ) );



  /* ouvre le livre */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
  gtk_widget_hide ( ICONE_FERMEE );
  gtk_widget_show ( ICONE_OUVERTE );


}
/* *********************************************************************************************************** */
