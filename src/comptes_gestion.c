/* fichier qui s'occupe de la gestion des comptes */
/*           gestion_comptes.c */

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


/* ************************************************************************************************************ */
/* Fonction creation_details_compte */
/* crée la fenêtre qui sera l'onglet 2 du notebook du compte */
/* ************************************************************************************************************ */

GtkWidget *creation_details_compte ( void )
{
  GtkWidget *onglet;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  GtkWidget *separateur;
  GtkWidget *hbox;
  GtkWidget *hbox2;
  GtkWidget *label;
  GtkWidget *bouton;
  GtkWidget *vbox2;
  GtkWidget *scrolled_window_text;


  /* la fenetre ppale est une vbox avec les détails en haut et appliquer en bas */

  onglet = gtk_vbox_new ( FALSE,
			  5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( onglet ),
				   10 );
  gtk_widget_show ( onglet );


  /* partie du haut avec les détails du compte */

  scrolled_window = gtk_scrolled_window_new ( FALSE,
				     FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( onglet ),
		       scrolled_window,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window );


  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  vbox );
  gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window ) -> child ),
				 GTK_SHADOW_NONE );
  gtk_widget_show ( vbox );



/* création de la ligne du nom du compte */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );


  label = gtk_label_new ( _("Nom du compte : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_nom_compte = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_nom_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_nom_compte );


/* création de la ligne du type de compte */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Type du compte : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_type_compte = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_type_compte ),
			     creation_menu_type_compte() );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_type_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_type_compte );


/* création de la ligne de la devise */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Devise du compte : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_devise_compte = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_devise_compte ),
			     creation_option_menu_devises ( 0,
							    liste_struct_devises ));
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_devise_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_devise_compte );


/* création de la ligne de passage à l'euro */

  hbox_bouton_passage_euro = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox_bouton_passage_euro,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox_bouton_passage_euro );

  bouton_passage_euro = gtk_button_new_with_label ( _("Passer ce compte à l'euro") );
  gtk_signal_connect ( GTK_OBJECT ( bouton_passage_euro ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( passage_a_l_euro ),
		       NULL );
  gtk_button_set_relief ( GTK_BUTTON ( bouton_passage_euro ),
			  GTK_RELIEF_HALF );
  gtk_box_pack_start ( GTK_BOX ( hbox_bouton_passage_euro ),
		       bouton_passage_euro,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_passage_euro );


/* création de la ligne compte cloturé */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  detail_compte_cloture = gtk_check_button_new_with_label ( _("Compte clôturé") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       detail_compte_cloture,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_compte_cloture );



  /* séparation */

  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( separateur );

  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );


/* création de la ligne du titulaire du compte */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );


  label = gtk_label_new ( _("Titulaire du compte : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_titulaire_compte = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_titulaire_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_titulaire_compte );



/* création de la ligne titulaire a l'adr commune */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );


  detail_bouton_adresse_commune = gtk_radio_button_new_with_label ( NULL,
								    _("L'adresse commune est celle du titulaire") );
  gtk_signal_connect ( GTK_OBJECT ( detail_bouton_adresse_commune ),
		       "toggled",
		       GTK_SIGNAL_FUNC ( changement_bouton_adresse_commune_perso ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_bouton_adresse_commune,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_bouton_adresse_commune );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Adresse :") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );




/* création de la ligne titulaire a sa propre adresse */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  vbox2 = gtk_vbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( vbox2 );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  bouton = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( detail_bouton_adresse_commune)),
					     _("Le titulaire a sa propre adresse") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  scrolled_window_text = gtk_scrolled_window_new ( FALSE,
						   FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       scrolled_window_text,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( scrolled_window_text );

  detail_adresse_titulaire = gtk_text_new ( NULL,
					    NULL );
  gtk_text_set_editable ( GTK_TEXT ( detail_adresse_titulaire ),
			  TRUE );
  gtk_widget_set_sensitive ( detail_adresse_titulaire,
			     FALSE );
  gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ),
		      detail_adresse_titulaire );
  gtk_widget_show ( detail_adresse_titulaire );



  /* séparation */

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );


/* ligne de l'établissement financier */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );


  label = gtk_label_new ( _("Etablissement financier : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_option_menu_banque = gtk_option_menu_new ();
  gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_option_menu_banque ),
			     creation_menu_banques () );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_option_menu_banque,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_option_menu_banque );

  /* mise en place de la ligne détails de la banque et créer */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  bouton = gtk_button_new_with_label ( _("Détails") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( affiche_detail_banque ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			    bouton,
			    FALSE,
			    FALSE,
			    5 );
  gtk_widget_show ( bouton );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       separateur,
		       FALSE,
		       TRUE,
		       5 );
  gtk_widget_show ( separateur );

  bouton = gtk_button_new_with_label ( _("Créer ...") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( preferences ),
		       (GtkObject *) 6 );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
			    bouton,
			    FALSE,
			    FALSE,
			    5 );
  gtk_widget_show ( bouton );


  /* création de la ligne contenant le code de la banque */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Code de la banque : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label_code_banque = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label_code_banque,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label_code_banque );


/* création de la ligne du guichet */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Guichet / Agence : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_guichet = gtk_entry_new ();
  gtk_widget_set_usize ( detail_guichet,
			 80,
			 FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_guichet,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_guichet );



/* création de la ligne du numéro du compte */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Numéro de compte / Clé : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_no_compte = gtk_entry_new ();
  gtk_widget_set_usize ( detail_no_compte,
			 80,
			 FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_no_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_no_compte );

  detail_cle_compte = gtk_entry_new ();
  gtk_widget_set_usize ( detail_cle_compte,
			 30,
			 FALSE );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_cle_compte,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_cle_compte );




  /* séparation */


  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( separateur );


  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );


/* création de la ligne du solde initial */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Solde initial : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_solde_init = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_solde_init,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_solde_init );




/* création de la ligne du solde mini autorisé */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Solde minimal autorisé : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_solde_mini_autorise = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_solde_mini_autorise,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_solde_mini_autorise );


/* création de la ligne du solde mini voulu */

  hbox = gtk_hbox_new ( TRUE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  label = gtk_label_new ( _("Solde minimal voulu : ") );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  hbox2 = gtk_hbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       hbox2,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( hbox2 );

  detail_solde_mini_voulu = gtk_entry_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       detail_solde_mini_voulu,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( detail_solde_mini_voulu );



  /* séparation */

  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( separateur );

  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );



/* création de la ligne du commentaire */

  hbox = gtk_hbox_new ( FALSE,
			0 );
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       hbox,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( hbox );

  vbox2 = gtk_vbox_new ( FALSE,
			 0 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       vbox2,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( vbox2 );

  label = gtk_label_new ( _("Commentaires : ") );
  gtk_box_pack_start ( GTK_BOX ( vbox2 ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  scrolled_window_text = gtk_scrolled_window_new ( FALSE,
						   FALSE );
  gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       scrolled_window_text,
		       TRUE,
		       TRUE,
		       5 );
  gtk_widget_show ( scrolled_window_text );

  detail_commentaire = gtk_text_new ( NULL,
				      NULL );
  gtk_text_set_editable ( GTK_TEXT ( detail_commentaire ),
			  TRUE ); 
  gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ),
		      detail_commentaire );
  gtk_widget_show ( detail_commentaire );



  /* séparation */

  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );

  separateur = gtk_hseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       separateur,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( separateur );

  label = gtk_label_new ("");
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       TRUE,
		       0 );
  gtk_widget_show ( label );


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

  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL );
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

  bouton = gnome_stock_button ( GNOME_STOCK_BUTTON_APPLY );
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

  menu_item = gtk_menu_item_new_with_label ( _("Compte bancaire") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_type_compte",
			GINT_TO_POINTER ( 0 ));
  gtk_widget_show ( menu_item );


  /* création de l'item compte de caisse */

  menu_item = gtk_menu_item_new_with_label ( _("Compte de caisse") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_type_compte",
			GINT_TO_POINTER ( 1 ));
  gtk_widget_show ( menu_item );


  /* création de l'item compte passif */

  menu_item = gtk_menu_item_new_with_label ( _("Compte de passif") );
  gtk_menu_append ( GTK_MENU ( menu ),
		    menu_item );
  gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			"no_type_compte",
			GINT_TO_POINTER ( 2 ));
  gtk_widget_show ( menu_item );

  /* création de l'item compte actif */

  menu_item = gtk_menu_item_new_with_label ( _("Compte d'actif") );
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


  /* vérification que le compte a un nom */

  if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))) )
    {
      dialogue ( _("Le compte n'a pas de nom !") );
      return;
    }

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;


  /* récupération du titulaire */

  TITULAIRE = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_titulaire_compte ))));

  if ( !strlen ( TITULAIRE ))
    TITULAIRE = NULL;


  /* vérification du nom du compte */

  if ( strcmp ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))),
		NOM_DU_COMPTE ) )
    {
      NOM_DU_COMPTE = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))));

      reaffiche_liste_comptes ();
      reaffiche_liste_comptes_onglet ();
      update_liste_comptes_accueil ();
      remplissage_liste_echeance ();
      update_liste_echeances_manuelles_accueil ();
      mise_a_jour_soldes_minimaux();
      mise_a_jour_fin_comptes_passifs();
      mise_a_jour_categ();

      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[5] ),
				 creation_option_menu_comptes () );

      remplissage_liste_comptes_etats ();
      selectionne_liste_comptes_etat_courant ();

      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
    }


  /* vérification du type de compte */

  if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
					       "no_type_compte" ))
       !=
       TYPE_DE_COMPTE )
    {
      TYPE_DE_COMPTE = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
							       "no_type_compte" ));

      if ( question_yes_no ( _("Vous avez changé le type du compte.\nVoulez-vous charger les types d'opérations par défaut de ce type de compte ?\n ( ce qui veut dire perdre les anciens types d'opérations créés pour ce compte\net le ou les derniers numéros de chèque )") ))
	{
	  GtkWidget *menu;

	  creation_types_par_defaut ( compte_courant,
				      0);

	  /* change les types d'opé et met le défaut */

	  if ( (menu = creation_menu_types ( 1, compte_courant, 0  )))
	    {
	      /* on joue avec les sensitive pour éviter que le 1er mot du menu ne reste grise */

	      gtk_widget_set_sensitive ( widget_formulaire_operations[9],
					 TRUE );
	      gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					 menu );
	      gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_operations[9] ),
					    cherche_no_menu_type ( TYPE_DEFAUT_DEBIT ) );
	      gtk_widget_set_sensitive ( widget_formulaire_operations[9],
					 FALSE );
	      gtk_widget_show ( widget_formulaire_operations[9] );
	    }
	  else
	    {
	      gtk_widget_hide ( widget_formulaire_operations[9] );
	      gtk_widget_hide ( widget_formulaire_operations[10] );
	    }
	}

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

	  dialogue = gnome_dialog_new ( _("Confirmation de changement de devise"),
					GNOME_STOCK_BUTTON_YES,
					GNOME_STOCK_BUTTON_NO,
					NULL );

	  label = gtk_label_new ( g_strconcat ( _("Attention, vous passez d'une devise qui passera à  l'euro : "),
						devise_compte -> nom_devise,
						_("\nà une qui n'y passera pas : "),
						nouvelle_devise -> nom_devise,
						_("\nLes paiements en euro du compte seront perdus !\n\nConfirmez vous le changement ?"),
						NULL ));
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
      remplissage_liste_operations ( compte_courant );
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

  NO_GUICHET = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_guichet ))));

  if ( !strlen ( NO_GUICHET ))
    NO_GUICHET = NULL;

/* enregistrement du no de compte */

  NO_COMPTE_BANQUE = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_no_compte ))));

  if ( !strlen ( NO_COMPTE_BANQUE ))
    NO_COMPTE_BANQUE = NULL;

/* enregistrement de la clé du compte */

  CLE_COMPTE = g_strdup ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_cle_compte ))));

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

  if ( SOLDE_INIT != g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_solde_init ))),
				NULL ) )
    {
      gint value;

      SOLDE_INIT = g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_solde_init ))),
			      NULL );

      value = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value;
      remplissage_liste_operations ( compte_courant );
      gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value = value;

      update_liste_comptes_accueil ();
      mise_a_jour_soldes_minimaux();
      mise_a_jour_fin_comptes_passifs();


      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
    }

/* vérification du solde mini autorisé */

  if ( SOLDE_MINI != g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_autorise ))),
				NULL ) )
    {
      SOLDE_MINI = g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_autorise ))),
			      NULL );
      MESSAGE_SOUS_MINI = 0;
 
      mise_a_jour_soldes_minimaux();
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant_onglet;
   }



/* vérification du solde mini voulu */

  if ( SOLDE_MINI_VOULU != g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_voulu ))),
				NULL ) )
    {
      SOLDE_MINI_VOULU = g_strtod ( g_strstrip ( gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_voulu ))),
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

      dialogue = gnome_dialog_new ( _("Appliquer les modifications au compte ?"),
				    GNOME_STOCK_BUTTON_YES,
				    GNOME_STOCK_BUTTON_NO,
				    NULL );
      gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				     GTK_WINDOW ( window ));

      label = gtk_label_new ( g_strconcat ( _("Des modifications ont été apportées au compte :\n"),
					    NOM_DU_COMPTE,
					    _("\n\nVoulez-vous les enregistrer ?"),
					    NULL ));
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
	modification_details_compte ();

      p_tab_nom_de_compte_variable = save;
    }
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction passage_a_l_euro */
/* appelée pour passer un compte à l'euro */
/* ************************************************************************************************************ */

void passage_a_l_euro ( GtkWidget *bouton,
			gpointer null )
{
  GtkWidget *dialog;
  GtkWidget *label;
  gint resultat;
  gfloat value;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  dialog = gnome_dialog_new ( _("Confirmation de passage à l'euro"),
				GNOME_STOCK_BUTTON_YES,
				GNOME_STOCK_BUTTON_NO,
				NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialog ),
				 GTK_WINDOW ( window ));

  label = gtk_label_new ( g_strconcat ( _("Attention, le passage à l'euro est une opération irréversible !\n\nÊtes-vous sûr de vouloir passer le compte "),
					NOM_DU_COMPTE,
					_(" à l'euro ?"),
					NULL ));
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  resultat = gnome_dialog_run_and_close ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    return;
  else
    {
      GSList *pointeur;
      gdouble change;


      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

      /* recherche de la devise du compte pour le change */

      change = (( struct struct_devise * )( g_slist_find_custom ( liste_struct_devises,
								  GINT_TO_POINTER ( DEVISE ),
								  ( GCompareFunc ) recherche_devise_par_no ) -> data ))
	-> change;

      if ( !change )
	{
	  dialogue ( _("Erreur : le taux de change entre les 2 monnaies est de 0.") );
	  return;
	}

      /* retrouve le no de l'euro */

      pointeur = g_slist_find_custom ( liste_struct_devises,
				       _("Euro"),
				       ( GCompareFunc ) recherche_devise_par_nom );

      if ( !pointeur )
	{
	  dialogue ( _("La monnaie Euro est introuvable, elle a dû être effacée des devises.") );
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
gint recherche_devise_par_nom ( struct struct_devise *devise,
				gchar *nom )
{

  return ( g_strcasecmp ( g_strstrip ( devise -> nom_devise ),
			  nom ) );

}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
/* Appelée lorsqu'on change de banque le compte, met à jour le code de la banque écrit en dessous */
/* ************************************************************************************************************ */
void changement_de_banque ( void )
{
  struct struct_banque *banque;

  banque = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_option_menu_banque ) -> menu_item ),
				 "adr_banque" );

  if ( banque && banque->code_banque )
    gtk_label_set_text ( GTK_LABEL ( label_code_banque ),
			 banque->code_banque );
  else
    gtk_label_set_text ( GTK_LABEL ( label_code_banque ),
			 "" );


}
/* ************************************************************************************************************ */
