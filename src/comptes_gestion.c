/* ************************************************************************** */
/*  Fichier qui s'occupe de la gestion des comptes			      */
/*			gestion_comptes.c				      */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004      Benjamin Drieu (bdrieu@april.org) 	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org) 	      */
/*			http://www.grisbi.org   			      */
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
#include "categories_onglet.h"
#include "comptes_gestion.h"
#include "comptes_onglet.h"
#include "echeancier_liste.h"
#include "erreur.h"
#include "etats_config.h"
#include "fichiers_io.h"
#include "operations_comptes.h"
#include "operations_formulaire.h"
#include "operations_liste.h"
#include "traitement_variables.h"
#include "devises.h"
#include "banque.h"
#include "parametres.h"
#include "comptes_traitements.h"


/* ************************************************************************************************************ */
/* Fonction creation_details_compte */
/* crée la fenêtre qui sera l'onglet 2 du notebook du compte */
/* ************************************************************************************************************ */

GtkWidget *creation_details_compte ( void )
{
  GtkWidget *onglet, *vbox, *scrolled_window, *separateur, *hbox, *hbox2;
  GtkWidget *label, *bouton, *vbox2, *scrolled_window_text, *paddingbox;


  /* la fenetre ppale est une vbox avec les détails en haut et appliquer en bas */

  onglet = gtk_vbox_new ( FALSE, 5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ), 10 );
  gtk_widget_show ( onglet );


  /* partie du haut avec les détails du compte */

  scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( onglet ), scrolled_window, TRUE, TRUE, 0 );
  gtk_widget_show ( scrolled_window );


  vbox = gtk_vbox_new ( FALSE, 5 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ), vbox );
  gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window ) -> child ),
				 GTK_SHADOW_NONE );
  gtk_widget_show ( vbox );

  paddingbox = new_paddingbox_with_title (vbox, FALSE, _("Account details"));

/* création de la ligne du nom du compte */

  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Account name")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_nom_compte = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_nom_compte, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_nom_compte );

  
  /* création de la ligne du type de compte */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Account type")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_type_compte = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_type_compte ),
			     creation_menu_type_compte() );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_type_compte, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_type_compte );


  /* création de la ligne de la devise */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Account currency")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_devise_compte = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_devise_compte ),
			     creation_option_menu_devises ( 0, liste_struct_devises ));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_devise_compte, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_devise_compte );

  /* création de la ligne de passage à l'euro */
  hbox_bouton_passage_euro = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ),
		       hbox_bouton_passage_euro, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox_bouton_passage_euro );

  bouton_passage_euro = gtk_button_new_with_label ( _("Convert this account to euros") );
  gtk_signal_connect ( GTK_OBJECT ( bouton_passage_euro ), "clicked",
		       GTK_SIGNAL_FUNC ( passage_a_l_euro ), NULL );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_passage_euro ), GTK_RELIEF_HALF );
  gtk_box_pack_start ( GTK_BOX ( hbox_bouton_passage_euro ),
		       bouton_passage_euro, FALSE, FALSE, 0 );
  gtk_widget_show ( bouton_passage_euro );


  /* création de la ligne compte cloturé */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  detail_compte_cloture = gtk_check_button_new_with_label ( _("Closed account") );
  gtk_box_pack_start ( GTK_BOX ( hbox ), detail_compte_cloture, FALSE, FALSE, 0 );
  gtk_widget_show ( detail_compte_cloture );


  /* création de la ligne du titulaire du compte */
  paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Account holder"));

  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );


  label = gtk_label_new ( COLON(_("Holder name")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_titulaire_compte = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_titulaire_compte, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_titulaire_compte );


  /* création de la ligne titulaire a l'adr commune */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );


  detail_bouton_adresse_commune = gtk_radio_button_new_with_label ( NULL,
								    _("Common address is holder's address") );
  gtk_signal_connect ( GTK_OBJECT ( detail_bouton_adresse_commune ), "toggled",
		       GTK_SIGNAL_FUNC ( changement_bouton_adresse_commune_perso ), NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_bouton_adresse_commune, FALSE, FALSE, 0 );
  gtk_widget_show ( detail_bouton_adresse_commune );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Address")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );



  /* création de la ligne titulaire a sa propre adresse */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  vbox2 = gtk_vbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), vbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( vbox2 );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  bouton = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( detail_bouton_adresse_commune)),
					     _("Holder has his own address") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton, FALSE, FALSE, 0 );
  gtk_widget_show ( bouton );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), scrolled_window_text, TRUE, TRUE, 0 );
  gtk_widget_show ( scrolled_window_text );

  detail_adresse_titulaire = gtk_text_new ( NULL, NULL );
  gtk_text_set_editable ( GTK_TEXT ( detail_adresse_titulaire ), TRUE );
  gtk_widget_set_sensitive ( detail_adresse_titulaire, FALSE );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ),
		      detail_adresse_titulaire );
  gtk_widget_show ( detail_adresse_titulaire );


  /* ligne de l'établissement financier */
  paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Bank"));

  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );


  label = gtk_label_new ( COLON(_("Financial institution")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_option_menu_banque = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_option_menu_banque ),
			     creation_menu_banques () );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_option_menu_banque, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_option_menu_banque );

  /* mise en place de la ligne détails de la banque et créer */

  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  bouton_detail = gtk_button_new_with_label ( _("Details") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_detail ), GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton_detail ), "clicked",
		       GTK_SIGNAL_FUNC ( view_bank ), NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton_detail, FALSE, FALSE, 5 );
  gtk_widget_show ( bouton_detail );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), separateur, FALSE, TRUE, 5 );
  gtk_widget_show ( separateur );

  bouton = gtk_button_new_with_label ( _("New bank") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ), GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
		       GTK_SIGNAL_FUNC ( edit_bank ), NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), bouton, FALSE, FALSE, 5 );
  gtk_widget_show ( bouton );


  /* création de la ligne contenant le code de la banque */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Bank sort code")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label_code_banque = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label_code_banque, FALSE, FALSE, 0 );
  gtk_widget_show ( label_code_banque );


  /* création de la ligne du guichet */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Bank branch code")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_guichet = gtk_entry_new ();
  gtk_widget_set_usize ( detail_guichet, 80, FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_guichet, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_guichet );


  /* création de la ligne du numéro du compte */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Account number / Key")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );

  hbox2 = gtk_hbox_new ( FALSE, 6 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_no_compte = gtk_entry_new ();
  gtk_widget_set_usize ( detail_no_compte, 80, FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_no_compte, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_no_compte );

  detail_cle_compte = gtk_entry_new ();
  gtk_widget_set_usize ( detail_cle_compte, 30, FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_cle_compte, FALSE, FALSE, 0 );
  gtk_widget_show ( detail_cle_compte );



  /* création de la ligne du solde initial */
  paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Balances"));

  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Initial balance")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_solde_init = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_solde_init, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_solde_init );


  /* création de la ligne du solde mini autorisé */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Minimum authorised balance")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_solde_mini_autorise = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_solde_mini_autorise, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_solde_mini_autorise );

 
  /* création de la ligne du solde mini voulu */
  hbox = gtk_hbox_new ( TRUE, 0 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( COLON(_("Minimum wanted balance")) );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), label, FALSE, FALSE, 0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE, 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ), hbox2, FALSE, TRUE, 0 );
  gtk_widget_show ( hbox2 );

  detail_solde_mini_voulu = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ), detail_solde_mini_voulu, TRUE, TRUE, 0 );
  gtk_widget_show ( detail_solde_mini_voulu );


  /* création de la ligne du commentaire */
  paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Comments"));

  scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window_text, TRUE, TRUE, 5 );
  gtk_widget_show ( scrolled_window_text );

  detail_commentaire = gtk_text_new ( NULL, NULL );
  gtk_text_set_editable ( GTK_TEXT ( detail_commentaire ), TRUE ); 
  gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_commentaire );
  gtk_widget_show ( detail_commentaire );

/* séparation de la fenetre */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       separateur,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( separateur );

/* mise en forme des boutons appliquer et annuler */

  hbox_boutons_modif = gtk_hbox_new ( FALSE,
			5 );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       hbox_boutons_modif,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox_boutons_modif );

  bouton = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( modification_details_compte ),
		       NULL );
  gtk_box_pack_end ( GTK_BOX ( hbox_boutons_modif ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  bouton = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( remplissage_details_compte ),
		       NULL );
  gtk_box_pack_end ( GTK_BOX ( hbox_boutons_modif ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  gtk_widget_show_all ( vbox );


/*   à ce niveau, on relie toute modif à une fonction qui rend sensitif les boutons appliquer et annuler */

  gtk_signal_connect_object ( GTK_OBJECT ( detail_titulaire_compte ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_nom_compte ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU (  detail_type_compte ) -> menu ),
			      "selection-done",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte  ) -> menu ),
			      "selection-done",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_bouton_adresse_commune ),
			      "toggled",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_adresse_titulaire ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_option_menu_banque  ) -> menu ),
			      "selection-done",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_no_compte ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_guichet ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_cle_compte ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_compte_cloture ),
			      "toggled",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_solde_init ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_solde_mini_autorise ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_solde_mini_voulu ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );
  gtk_signal_connect_object ( GTK_OBJECT ( detail_commentaire ),
			      "changed",
			      GTK_SIGNAL_FUNC ( modif_detail_compte ),
			      GTK_OBJECT ( hbox_boutons_modif ) );

  gtk_widget_set_sensitive ( hbox_boutons_modif,
			     FALSE );

  return ( onglet );
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
void changement_bouton_adresse_commune_perso ( void )
{

  if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_bouton_adresse_commune )))
    gtk_widget_set_sensitive ( detail_adresse_titulaire,
			       FALSE );
  else
    gtk_widget_set_sensitive ( detail_adresse_titulaire,
			       TRUE );

}
/* ************************************************************************************************************ */





/* ************************************************************************************************************ */
/* Fonction creation_menu_type_compte */
/* crée un menu contenant tous les types de compte associés à leur numéro */
/* et le renvoie */
/* ************************************************************************************************************ */

GtkWidget *creation_menu_type_compte ( void )
{
  GtkWidget *menu;
  GtkWidget *menu_item;

  menu = gtk_menu_new ();
  gtk_widget_show ( menu );

  /* création de l'item compte bancaire */

  menu_item = gtk_menu_item_new_with_label ( _("Bank account") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_type_compte",
			GINT_TO_POINTER ( 0 ));
  gtk_widget_show ( menu_item );


  /* création de l'item compte de caisse */

  menu_item = gtk_menu_item_new_with_label ( _("Cash account") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_type_compte",
			GINT_TO_POINTER ( 1 ));
  gtk_widget_show ( menu_item );


  /* création de l'item compte passif */

  menu_item = gtk_menu_item_new_with_label ( _("Liabilities account") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_type_compte",
			GINT_TO_POINTER ( 2 ));
  gtk_widget_show ( menu_item );

  /* création de l'item compte actif */

  menu_item = gtk_menu_item_new_with_label ( _("Assets account") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_type_compte",
			GINT_TO_POINTER ( 3 ));
  gtk_widget_show ( menu_item );



  return ( menu );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
/* Fonction modif_detail_compte */
/* appelée dès qu'une modification est faite sur le détail d'un compte */
/* affiche les boutons appliquer / annuler  */
/* ************************************************************************************************************ */

void modif_detail_compte ( GtkWidget *hbox )
{

  gtk_widget_set_sensitive ( hbox_boutons_modif,
			     TRUE );

}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
/* Fonction remplissage_details_compte */
/* ************************************************************************************************************ */

void remplissage_details_compte ( void )
{
  GSList *pointeur_banque;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;


  gtk_entry_set_text ( GTK_ENTRY ( detail_nom_compte ),
		       NOM_DU_COMPTE );

  gtk_option_menu_set_history ( GTK_OPTION_MENU ( detail_type_compte ),
				TYPE_DE_COMPTE );

  gtk_option_menu_set_history ( GTK_OPTION_MENU (  detail_devise_compte),
				g_slist_position ( liste_struct_devises,
						   g_slist_find_custom ( liste_struct_devises,
									 GINT_TO_POINTER ( DEVISE ),
									 ( GCompareFunc ) recherche_devise_par_no )));

  if ( (( struct struct_devise * )( g_slist_find_custom ( liste_struct_devises,
							  GINT_TO_POINTER ( DEVISE ),
							  ( GCompareFunc ) recherche_devise_par_no ) -> data )) -> passage_euro )
    gtk_widget_show ( hbox_bouton_passage_euro );
  else
    gtk_widget_hide ( hbox_bouton_passage_euro );

  pointeur_banque = g_slist_find_custom ( liste_struct_banques,
					  GINT_TO_POINTER ( BANQUE ),
					  ( GCompareFunc ) recherche_banque_par_no );


  if ( TITULAIRE )
    gtk_entry_set_text ( GTK_ENTRY ( detail_titulaire_compte ),
			 TITULAIRE );
  else
    gtk_entry_set_text ( GTK_ENTRY ( detail_titulaire_compte ),
			 "" );


  if ( ADRESSE_TITULAIRE )
    {
      gtk_text_insert ( GTK_TEXT ( detail_adresse_titulaire ),
			NULL,
			NULL,
			NULL,
			ADRESSE_TITULAIRE,
			-1 );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( GTK_RADIO_BUTTON ( detail_bouton_adresse_commune ) -> group -> data ),
				     TRUE );
    }
  else
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( detail_bouton_adresse_commune ),
				   TRUE );
  if ( pointeur_banque )
    {
      struct struct_banque *banque;

      banque = pointeur_banque -> data;
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( detail_option_menu_banque ),
				    g_slist_index ( liste_struct_banques,
						    banque )+ 1 );
      if ( banque -> code_banque )
	gtk_label_set_text ( GTK_LABEL ( label_code_banque ),
			     banque -> code_banque );
      else
	gtk_label_set_text ( GTK_LABEL ( label_code_banque ),
			     "" );

    }
  else
    {
      gtk_option_menu_set_history ( GTK_OPTION_MENU ( detail_option_menu_banque ),
				    0 );
      gtk_label_set_text ( GTK_LABEL ( label_code_banque ),
			   "" );
    }

  if ( NO_GUICHET )
    gtk_entry_set_text ( GTK_ENTRY ( detail_guichet ),
			 NO_GUICHET );
  else
    gtk_entry_set_text ( GTK_ENTRY ( detail_guichet ),
			 "" );

  if ( NO_COMPTE_BANQUE )
    gtk_entry_set_text ( GTK_ENTRY ( detail_no_compte ),
			 NO_COMPTE_BANQUE );
  else
    gtk_entry_set_text ( GTK_ENTRY ( detail_no_compte ),
			 "" );

  if ( CLE_COMPTE )
    gtk_entry_set_text ( GTK_ENTRY ( detail_cle_compte ),
			 CLE_COMPTE );
  else
    gtk_entry_set_text ( GTK_ENTRY ( detail_cle_compte ),
			 "" );

  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON( detail_compte_cloture ),
				 COMPTE_CLOTURE );


  gtk_entry_set_text ( GTK_ENTRY ( detail_solde_init ),
		       g_strdup_printf ( "%4.2f",
					 SOLDE_INIT ));

  gtk_entry_set_text ( GTK_ENTRY ( detail_solde_mini_autorise ),
		       g_strdup_printf ( "%4.2f",
					 SOLDE_MINI ));

  gtk_entry_set_text ( GTK_ENTRY ( detail_solde_mini_voulu ),
		       g_strdup_printf ( "%4.2f",
					 SOLDE_MINI_VOULU ));

  gtk_editable_delete_text ( GTK_EDITABLE ( detail_commentaire ),
			     0,
			     -1 );

  if ( COMMENTAIRE )
    gtk_text_insert ( GTK_TEXT ( detail_commentaire ),
		      NULL,
		      NULL,
		      NULL,
		      COMMENTAIRE,
		      -1 );


  gtk_widget_set_sensitive ( hbox_boutons_modif,
			     FALSE );

}
/* ************************************************************************************************************ */




/* ************************************************************************************************************ */
/* Fonction recherche_banque_par_no */
/* appelée par g_slist_find_custom */
/* ************************************************************************************************************ */

gint recherche_banque_par_no ( struct struct_banque *banque,
			       gint *no_banque )
{

  return ( !( banque -> no_banque == GPOINTER_TO_INT ( no_banque )) );

}
/* ************************************************************************************************************ */

			       
/* ************************************************************************************************************ */
/* Fonction modification_details_compte */
/* appelée par le bouton appliquer du détail des comptes */
/* ************************************************************************************************************ */

void modification_details_compte ( void )
{
  gint i;

  /* vérification que le compte a un nom */

  if ( !strlen ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))) )
    {
      dialogue ( _("Account has no name!") );
      return;
    }

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  /* vérification que ce nom ne soit pas déjà utilisé */

  for ( i = 0; i < nb_comptes; i++)
    {
     if ( i == compte_courant_onglet )
       continue;
     p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
     if ( !strcmp ( g_strstrip ( (gchar*) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))), NOM_DU_COMPTE ))
       {
	dialogue( g_strdup_printf ( _("Account \"%s\" already exists!"), gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))));
	return;
       }
    }

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;


  /* récupération du titulaire */

  TITULAIRE = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_titulaire_compte ))));

  if ( !strlen ( TITULAIRE ))
    TITULAIRE = NULL;


  /* vérification du type de compte */

  if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
					       "no_type_compte" ))
       !=
       TYPE_DE_COMPTE )
    {
      TYPE_DE_COMPTE = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
							       "no_type_compte" ));
      mise_a_jour_fin_comptes_passifs();
      mise_a_jour_soldes_minimaux();
      formulaire_a_zero();
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
      
    }


  /* vérification de la devise */

  if ( DEVISE != GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte ) -> menu_item ),
							 "no_devise" )) )
    {
      struct struct_devise *nouvelle_devise;
      GSList *pointeur_liste;
      gfloat value;

      if ( !devise_compte
	   ||
	   devise_compte -> no_devise != DEVISE )
	devise_compte = g_slist_find_custom ( liste_struct_devises,
					      GINT_TO_POINTER ( DEVISE ),
					      ( GCompareFunc ) recherche_devise_par_no ) -> data;

      nouvelle_devise = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte ) -> menu_item ),
					      "adr_devise" );

      if ( devise_compte -> passage_euro && !nouvelle_devise -> passage_euro )
	{
	  GtkWidget *dialogue;
	  GtkWidget *label;
	  gint resultat;

	  dialogue = gnome_dialog_new ( _("Confirm currency change"),
					GNOME_STOCK_BUTTON_YES,
					GNOME_STOCK_BUTTON_NO,
					NULL );

	  label = gtk_label_new ( g_strdup_printf ( _("Be carefull, you are changing from a currency that is in the euro zone\n(%s) to one that isn't (%s). Transactions in euros will be lost!\n\nDo you confirm this change?"), 
						    devise_compte -> nom_devise,
						    nouvelle_devise -> nom_devise));
	  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
			       label,
			       FALSE,
			       FALSE,
			       0 );
	  gtk_widget_show ( label );

	  resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialogue ));

	  if ( resultat )
	    return;
	  else 
	    gtk_widget_hide ( bouton_passage_euro );
	}

      if ( nouvelle_devise -> passage_euro )
	gtk_widget_show ( bouton_passage_euro );

      pointeur_liste = LISTE_OPERATIONS;

      while ( pointeur_liste )
	{
	  struct structure_operation *operation;

	  operation = pointeur_liste -> data;

	  if ( operation -> devise == DEVISE )
	    operation -> devise = nouvelle_devise -> no_devise;
	  else
	    if ( !nouvelle_devise -> passage_euro )
	      operation -> devise = nouvelle_devise -> no_devise;

	  pointeur_liste = pointeur_liste -> next;
	}

      DEVISE = nouvelle_devise -> no_devise;

      value = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value;
      remplissage_liste_operations ( compte_courant_onglet );
      gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value = value;

      update_liste_comptes_accueil ();
      update_liste_echeances_manuelles_accueil ();

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;

    }


/* récupération de l'adr du titulaire */

  if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_bouton_adresse_commune )))
    {
      ADRESSE_TITULAIRE = g_strdup ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( detail_adresse_titulaire ),
									  0,
									  -1 )));

      if ( strlen ( ADRESSE_TITULAIRE ))
	{
	  ADRESSE_TITULAIRE = g_strdelimit ( ADRESSE_TITULAIRE,
				       "{",
				       '(' );
	  ADRESSE_TITULAIRE = g_strdelimit ( ADRESSE_TITULAIRE,
				       "}",
				       ')' );
	}
      else
	ADRESSE_TITULAIRE = NULL;
    }
  else
    ADRESSE_TITULAIRE = NULL;


/* enregistrement de l'établissement financier */

  BANQUE = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_option_menu_banque ) -> menu_item ),
						   "no_banque" ));

/* enregistrement du no de guichet */

  NO_GUICHET = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_guichet ))));

  if ( !strlen ( NO_GUICHET ))
    NO_GUICHET = NULL;

/* enregistrement du no de compte */

  NO_COMPTE_BANQUE = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_no_compte ))));

  if ( !strlen ( NO_COMPTE_BANQUE ))
    NO_COMPTE_BANQUE = NULL;

/* enregistrement de la clé du compte */

  CLE_COMPTE = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_cle_compte ))));

  if ( !strlen ( CLE_COMPTE ))
    CLE_COMPTE = NULL;

/* enregistrement du compte cloturé */

  if ( COMPTE_CLOTURE != gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_compte_cloture )))
    {
      COMPTE_CLOTURE = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_compte_cloture ));
      reaffiche_liste_comptes ();
      update_liste_comptes_accueil ();
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;

    }


/* vérification du solde initial */

  if ( SOLDE_INIT != my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_init ))),
				NULL ) )
    {
      gint value;

      SOLDE_INIT = my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_init ))),
			      NULL );

      value = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value;
      remplissage_liste_operations ( compte_courant_onglet );
      gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value = value;

      update_liste_comptes_accueil ();
      mise_a_jour_soldes_minimaux();
      mise_a_jour_fin_comptes_passifs();


      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
    }

/* vérification du solde mini autorisé */

  if ( SOLDE_MINI != my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_autorise ))),
				NULL ) )
    {
      SOLDE_MINI = my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_autorise ))),
			      NULL );
      MESSAGE_SOUS_MINI = 0;
 
      update_liste_comptes_accueil ();
      mise_a_jour_soldes_minimaux();
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
   }



/* vérification du solde mini voulu */

  if ( SOLDE_MINI_VOULU != my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_voulu ))),
				NULL ) )
    {
      SOLDE_MINI_VOULU = my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_voulu ))),
			      NULL );
      MESSAGE_SOUS_MINI_VOULU = 0;
 
      mise_a_jour_soldes_minimaux();
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
    }


/* récupération du texte */

  COMMENTAIRE = g_strdup ( g_strstrip ( gtk_editable_get_chars (GTK_EDITABLE ( detail_commentaire ),
								0,
								-1 )));

  if ( strlen ( COMMENTAIRE ))
    {
      COMMENTAIRE = g_strdelimit ( COMMENTAIRE,
				   "{",
				   '(' );
      COMMENTAIRE = g_strdelimit ( COMMENTAIRE,
				   "}",
				   ')' );
    }
  else
    COMMENTAIRE = NULL;


  /* vérification du nom du compte */
  /* on doit le vérifier en dernier car s'il a changé, on va réafficher */
  /* la liste des comptes, et les infos non encore récupérées vont être */
  /* perdues */

  if ( !NOM_DU_COMPTE
       ||
       strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))),
		NOM_DU_COMPTE ) )
    {
      NOM_DU_COMPTE = g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))));

      reaffiche_liste_comptes ();
      reaffiche_liste_comptes_onglet ();
      update_liste_comptes_accueil ();
      remplissage_liste_echeance ();
      update_liste_echeances_manuelles_accueil ();
      mise_a_jour_soldes_minimaux();
      mise_a_jour_fin_comptes_passifs();
      mise_a_jour_categ();

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[5] ),
				 creation_option_menu_comptes(GTK_SIGNAL_FUNC(changement_choix_compte_echeancier), TRUE) );

      remplissage_liste_comptes_etats ();
      selectionne_liste_comptes_etat_courant ();

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
    }


  gtk_widget_set_sensitive ( hbox_boutons_modif,
			     FALSE );

  modification_fichier ( TRUE );

}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction sort_du_detail_compte */
/* appelée quand on change de compte dans les propriétés des comptes */
/* si des modifs avaient été faites, on demande si on enregistre */
/* ************************************************************************************************************ */

void sort_du_detail_compte ( void )
{

  if ( GTK_WIDGET_SENSITIVE ( hbox_boutons_modif ) )
    {
      GtkWidget *dialogue;
      GtkWidget *label;
      gint resultat;
      gpointer **save;

      save = p_tab_nom_de_compte_variable;
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;

      dialogue = gnome_dialog_new ( _("Apply changes to account?"),
				    GNOME_STOCK_BUTTON_YES,
				    GNOME_STOCK_BUTTON_NO,
				    NULL );
      gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				     GTK_WINDOW ( window ));

      label = gtk_label_new ( g_strdup_printf ( _("Account \"%s\" has been modified.\n\n\nDo you want to save changes?"),
					    NOM_DU_COMPTE ) );
      gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
			   label,
			   FALSE,
			   FALSE,
			   0 );
      gtk_widget_show ( label );

      resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialogue ));

      if ( resultat )
	gtk_widget_set_sensitive ( hbox_boutons_modif,
				   FALSE );
      else
        {
	 modification_details_compte ();
	 update_liste_comptes_accueil ();
	}

      p_tab_nom_de_compte_variable = save;
    }
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction passage_a_l_euro */
/* appelée pour passer un compte à l'euro */
/* ************************************************************************************************************ */

void passage_a_l_euro ( GtkWidget *bouton, gpointer null )
{
  gint resultat;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;

  resultat = question_yes_no_hint ( g_strdup_printf ( _("Convert account \"%s\" to euro?"), 
						      NOM_DU_COMPTE ),
				    _("Euro conversion is irreversible, are you sure you want to continue?") ); 
  if ( !resultat )
    return;
  else
    {
      GSList *pointeur;
      gdouble change;
      gfloat value;

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

      /* recherche de la devise du compte pour le change */

      change = (( struct struct_devise * )( g_slist_find_custom ( liste_struct_devises,
								  GINT_TO_POINTER ( DEVISE ),
								  ( GCompareFunc ) recherche_devise_par_no ) -> data ))
	-> change;

      if ( !change )
	{
	  dialogue_error ( _("Exchange rate between the 2 currencies is 0.") );
	  return;
	}

      /* retrouve le no de l'euro */

      pointeur = g_slist_find_custom ( liste_struct_devises,
				       _("Euro"),
				       ( GCompareFunc ) recherche_devise_par_nom );

      if ( !pointeur )
	{
	  dialogue_error ( _("Impossible to find the Euro currency, it has probably been deleted.") );
	  return;
	}

      SOLDE_INIT = SOLDE_INIT / change;
      SOLDE_MINI_VOULU = SOLDE_MINI_VOULU / change;
      SOLDE_MINI = SOLDE_MINI / change;
      SOLDE_DERNIER_RELEVE = SOLDE_DERNIER_RELEVE / change;
      DEVISE = (( struct struct_devise * )( pointeur -> data )) -> no_devise;

      value = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value;
      remplissage_liste_operations ( compte_courant );
      gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value = value;

      update_liste_comptes_accueil ();
      remplissage_details_compte ();

      modification_fichier ( TRUE );

    }

}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Appelée lorsqu'on change de banque le compte, met à jour le code de la banque écrit en dessous */
/* ************************************************************************************************************ */
void changement_de_banque ( GtkWidget * menu_shell )
{
  struct struct_banque *banque;

  banque = g_object_get_data ( G_OBJECT ( menu_shell ), "adr_banque" );

  if ( banque )
    {
      if ( banque->code_banque )
	{
	  gtk_label_set_text ( GTK_LABEL ( label_code_banque ),
			       banque->code_banque );
	}
      gtk_widget_set_sensitive ( bouton_detail,
				 TRUE );
    }
  else
    {
      gtk_label_set_text ( GTK_LABEL ( label_code_banque ),
			   "" );
      gtk_widget_set_sensitive ( bouton_detail,
				 FALSE );
    }


}
/* ************************************************************************************************************ */
