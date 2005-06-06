/* ************************************************************************** */
/*  Fichier qui s'occupe de la gestion des comptes			      */
/*			gestion_comptes.c				      */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004      Benjamin Drieu (bdrieu@april.org) 	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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



/*START_INCLUDE*/
#include "comptes_gestion.h"
#include "banque.h"
#include "devises.h"
#include "utils_devises.h"
#include "dialog.h"
#include "operations_formulaire.h"
#include "gsb_account.h"
#include "navigation.h"
#include "menu.h"
#include "gsb_transaction_data.h"
#include "categories_onglet.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "etats_config.h"
#include "echeancier_liste.h"
#include "operations_liste.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "utils_comptes.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void changement_bouton_adresse_commune_perso ( void );
static void modification_details_compte ( void );
static void sort_du_detail_compte ( void );
/*END_STATIC*/



GtkWidget *bouton_detail;
GtkWidget *detail_nom_compte;
GtkWidget *detail_type_compte;
GtkWidget *detail_titulaire_compte;
GtkWidget *detail_bouton_adresse_commune;
GtkWidget *detail_adresse_titulaire;
GtkWidget *detail_option_menu_banque;
GtkWidget *detail_no_compte;
GtkWidget *label_code_banque;
GtkWidget *detail_guichet;
GtkWidget *detail_cle_compte;
GtkWidget *detail_devise_compte;
GtkWidget *detail_compte_cloture;
GtkWidget *detail_solde_init;
GtkWidget *detail_solde_mini_autorise;
GtkWidget *detail_solde_mini_voulu;
GtkWidget *detail_commentaire;
GtkWidget *hbox_boutons_modif;


/*START_EXTERN*/
extern GtkWidget *adr_banque;
extern GtkWidget *code_banque;
extern gint compte_courant_onglet;
extern struct struct_devise *devise_compte;
extern GSList *liste_struct_banques;
extern GSList *liste_struct_devises;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkTreeSelection * selection;
/*END_EXTERN*/





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
/*     hbox_bouton_passage_euro = gtk_hbox_new ( TRUE, 0 ); */
/*     gtk_box_pack_start ( GTK_BOX ( paddingbox ), */
/* 			 hbox_bouton_passage_euro, FALSE, FALSE, 0 ); */
/*     gtk_widget_show ( hbox_bouton_passage_euro ); */

/*     bouton_passage_euro = gtk_button_new_with_label ( _("Convert this account to euros") ); */
/*     gtk_signal_connect ( GTK_OBJECT ( bouton_passage_euro ), "clicked", */
/* 			 GTK_SIGNAL_FUNC ( passage_a_l_euro ), NULL ); */
/*     gtk_button_set_relief ( GTK_BUTTON ( bouton_passage_euro ), GTK_RELIEF_HALF ); */
/*     gtk_box_pack_start ( GTK_BOX ( hbox_bouton_passage_euro ), */
/* 			 bouton_passage_euro, FALSE, FALSE, 0 ); */
/*     gtk_widget_show ( bouton_passage_euro ); */


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
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text), 
 					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( hbox2 ), scrolled_window_text, TRUE, TRUE, 0 );
    gtk_widget_show ( scrolled_window_text );

    detail_adresse_titulaire = gtk_text_view_new ();
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (detail_adresse_titulaire), GTK_WRAP_WORD );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_adresse_titulaire );
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
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window_text, TRUE, TRUE, 5 );
    gtk_widget_show ( scrolled_window_text );

    detail_commentaire = gtk_text_view_new ();
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (detail_commentaire), GTK_WRAP_WORD );

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
    g_signal_connect_swapped ( G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(detail_adresse_titulaire))),
			       "changed", ((GCallback) modif_detail_compte), 
			       hbox_boutons_modif );
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
    g_signal_connect_swapped ( G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(detail_commentaire))),
			       "changed", ((GCallback) modif_detail_compte), 
			       hbox_boutons_modif );

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
    struct struct_banque *banque;
    struct struct_devise *devise;

    if ( DEBUG )
	printf ( "remplissage_details_compte\n" );

    gtk_entry_set_text ( GTK_ENTRY ( detail_nom_compte ),
			 gsb_account_get_name (compte_courant_onglet) );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( detail_type_compte ),
				  gsb_account_get_kind (compte_courant_onglet) );

    devise = devise_par_no ( gsb_account_get_currency (compte_courant_onglet) );

    gtk_option_menu_set_history ( GTK_OPTION_MENU (  detail_devise_compte),
				  g_slist_index ( liste_struct_devises,
						  devise ));


    if ( gsb_account_get_holder_name (compte_courant_onglet) )
	gtk_entry_set_text ( GTK_ENTRY ( detail_titulaire_compte ),
			     gsb_account_get_holder_name (compte_courant_onglet) );
    else
	gtk_entry_set_text ( GTK_ENTRY ( detail_titulaire_compte ),
			     "" );

    gtk_text_buffer_set_text ( gtk_text_view_get_buffer (GTK_TEXT_VIEW (detail_adresse_titulaire)),
 			       ( gsb_account_get_holder_address (compte_courant_onglet) ? gsb_account_get_holder_address (compte_courant_onglet) : "") , -1 );

    if ( gsb_account_get_holder_address (compte_courant_onglet) )
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( GTK_RADIO_BUTTON ( detail_bouton_adresse_commune ) -> group -> data ),
				       TRUE );
    }
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( detail_bouton_adresse_commune ),
				       TRUE );

/*     remplissage des infos sur la banque */

    banque = banque_par_no ( gsb_account_get_bank (compte_courant_onglet) );
    
    if ( banque )
    {
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

    if ( gsb_account_get_bank_branch_code (compte_courant_onglet) )
	gtk_entry_set_text ( GTK_ENTRY ( detail_guichet ),
			     gsb_account_get_bank_branch_code (compte_courant_onglet) );
    else
	gtk_entry_set_text ( GTK_ENTRY ( detail_guichet ),
			     "" );

    if ( gsb_account_get_bank_account_number (compte_courant_onglet) )
	gtk_entry_set_text ( GTK_ENTRY ( detail_no_compte ),
			     gsb_account_get_bank_account_number (compte_courant_onglet) );
    else
	gtk_entry_set_text ( GTK_ENTRY ( detail_no_compte ),
			     "" );

    if ( gsb_account_get_bank_account_key (compte_courant_onglet) )
	gtk_entry_set_text ( GTK_ENTRY ( detail_cle_compte ),
			     gsb_account_get_bank_account_key (compte_courant_onglet) );
    else
	gtk_entry_set_text ( GTK_ENTRY ( detail_cle_compte ),
			     "" );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON( detail_compte_cloture ),
				   gsb_account_get_closed_account (compte_courant_onglet) );


    gtk_entry_set_text ( GTK_ENTRY ( detail_solde_init ),
			 g_strdup_printf ( "%4.2f",
					   gsb_account_get_init_balance (compte_courant_onglet) ));

    gtk_entry_set_text ( GTK_ENTRY ( detail_solde_mini_autorise ),
			 g_strdup_printf ( "%4.2f",
					   gsb_account_get_mini_balance_authorized (compte_courant_onglet) ));

    gtk_entry_set_text ( GTK_ENTRY ( detail_solde_mini_voulu ),
			 g_strdup_printf ( "%4.2f",
					   gsb_account_get_mini_balance_wanted (compte_courant_onglet) ));

    gtk_text_buffer_set_text ( gtk_text_view_get_buffer (GTK_TEXT_VIEW (detail_commentaire)),
			       ( gsb_account_get_comment (compte_courant_onglet) ? gsb_account_get_comment (compte_courant_onglet) : "") , -1 );


    gtk_widget_set_sensitive ( hbox_boutons_modif,
			       FALSE );

}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
/* Fonction modification_details_compte */
/* appelée par le bouton appliquer du détail des comptes */
/* ************************************************************************************************************ */

void modification_details_compte ( void )
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;

    /* vérification que le compte a un nom */

    if ( !strlen ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))) )
    {
        dialogue_error_hint ( _("Accounts names are used to distinguish accounts.  It is mandatory that names are both unique and not empty."),
			      _("Account name is empty") );
	return;
    }

    GSList *list_tmp;

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_account_get_no_account ( list_tmp -> data );

	if ( i == compte_courant_onglet )
	{
	    list_tmp = list_tmp -> next;
	    continue;
	}

	if ( !strcmp ( g_strstrip ( (gchar*) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))), gsb_account_get_name (i) ))
	{
	    dialogue_error_hint( _("Accounts names are used to distinguish accounts.  It is mandatory that names are both unique and not empty."),
				 g_strdup_printf ( _("Account \"%s\" already exists!"), gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))));
	    return;
	}

	list_tmp = list_tmp -> next;
    }

    /* récupération du titulaire */

    gsb_account_set_holder_name ( compte_courant_onglet,
				  g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_titulaire_compte )))) );

    if ( !strlen ( gsb_account_get_holder_name (compte_courant_onglet) ))
	gsb_account_set_holder_name ( compte_courant_onglet,
				      NULL );


    /* vérification du type de compte */

    if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
						 "no_type_compte" ))
	 !=
	 gsb_account_get_kind (compte_courant_onglet) )
    {
	gsb_account_set_kind (compte_courant_onglet,
			      GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
								      "no_type_compte" )));
	mise_a_jour_fin_comptes_passifs = 1;
	mise_a_jour_soldes_minimaux = 1;
	gsb_gui_navigation_update_account ( compte_courant_onglet );
	formulaire_a_zero();
    }


    /* vérification de la devise */

    if ( gsb_account_get_currency (compte_courant_onglet) != GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte ) -> menu_item ),
							   "no_devise" )) )
    {
	struct struct_devise *nouvelle_devise;

	if ( !devise_compte
	     ||
	     devise_compte -> no_devise != gsb_account_get_currency (compte_courant_onglet) )
	    devise_compte = devise_par_no ( gsb_account_get_currency (compte_courant_onglet) );

	nouvelle_devise = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_devise_compte ) -> menu_item ),
						"adr_devise" );

	if ( devise_compte -> passage_euro && !nouvelle_devise -> passage_euro )
	{
	    gint resultat;

	    resultat = question_yes_no_hint ( _("Confirm currency change"),
					      g_strdup_printf ( _("You are changing from a currency that is in the euro zone (%s) to one that isn't (%s).  Transactions in euros will be lost!  There is no undo for this.\nDo you confirm this change?"), 
								devise_compte -> nom_devise,
								nouvelle_devise -> nom_devise));

	    if ( !resultat )
		return;
	}

	list_tmp = gsb_transaction_data_get_transactions_list ();

	while ( list_tmp )
	{
	    gint transaction_number;
	    transaction_number = gsb_transaction_data_get_transaction_number (list_tmp -> data);

	    if ( gsb_transaction_data_get_account_number (transaction_number) == compte_courant_onglet )
	    {
		if ( gsb_transaction_data_get_currency_number (transaction_number) == gsb_account_get_currency (compte_courant_onglet) )
		    gsb_transaction_data_set_currency_number ( transaction_number,
							       nouvelle_devise -> no_devise );
		else
		    if ( !nouvelle_devise -> passage_euro )
			gsb_transaction_data_set_currency_number ( transaction_number,
								   nouvelle_devise -> no_devise );
	    }
	    list_tmp = list_tmp -> next;
	}
	gsb_account_set_currency ( compte_courant_onglet,
				   nouvelle_devise -> no_devise );
	
/* FIXME : voir pourquoi remplissage opé et remettre l'ajustement */

/* 	value = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value; */
	remplissage_liste_operations ( compte_courant_onglet );
/* 	gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value = value; */

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
    }


    /* récupération de l'adr du titulaire */

    if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_bouton_adresse_commune )))
    {
        buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW (detail_adresse_titulaire) );
	gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
	gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );
	gsb_account_set_holder_address ( compte_courant_onglet,
					 gtk_text_buffer_get_text ( buffer , &start, &end, TRUE ) );

	if ( strlen ( gsb_account_get_holder_address (compte_courant_onglet) ))
	{
	    gsb_account_set_holder_address ( compte_courant_onglet,
					     g_strdelimit ( gsb_account_get_holder_address (compte_courant_onglet),
					       "{",
					       '(' ) );
	    gsb_account_set_holder_address ( compte_courant_onglet,
					     g_strdelimit ( gsb_account_get_holder_address (compte_courant_onglet),
					       "}",
					       ')' ) );
	}
	else
	    gsb_account_set_holder_address ( compte_courant_onglet,
					     NULL );
    }
    else
	gsb_account_set_holder_address ( compte_courant_onglet,
					 NULL );


    /* enregistrement de l'établissement financier */
    gsb_account_set_bank ( compte_courant_onglet,
			   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_option_menu_banque ) -> menu_item ),
								   "no_banque" )));

    /* enregistrement du no de guichet */

    gsb_account_set_bank_branch_code ( compte_courant_onglet,
				       g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_guichet )))));

    if ( !strlen ( gsb_account_get_bank_branch_code (compte_courant_onglet) ))
	gsb_account_set_bank ( compte_courant_onglet,
			       0 );

    /* enregistrement du no de compte */

    gsb_account_set_bank_account_number ( compte_courant_onglet,
					  g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_no_compte )))) );

    if ( !strlen ( gsb_account_get_bank_account_number (compte_courant_onglet) ))
	gsb_account_set_bank_account_number ( compte_courant_onglet,
					      NULL );

    /* enregistrement de la clé du compte */

    gsb_account_set_bank_account_key ( compte_courant_onglet,
				       g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_cle_compte )))) );

    if ( !strlen ( gsb_account_get_bank_account_key (compte_courant_onglet) ))
	gsb_account_set_bank_account_key ( compte_courant_onglet,
					   NULL );

    /* enregistrement du compte cloturé */

    if ( gsb_account_get_closed_account (compte_courant_onglet) != gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_compte_cloture )))
    {
	gsb_account_set_closed_account ( compte_courant_onglet,
					 gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_compte_cloture )));

	gsb_gui_navigation_update_account ( compte_courant_onglet );

	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ();
	gsb_menu_update_accounts_in_menus ();
	mise_a_jour_liste_comptes_accueil = 1;

	/* Replace trees contents. */
	remplit_arbre_categ ();
	remplit_arbre_imputation ();
	remplit_arbre_tiers ();
    }


    /* vérification du solde initial */

    if ( gsb_account_get_init_balance (compte_courant_onglet) != my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_init ))),
									     NULL ) )
    {

	gsb_account_set_init_balance (compte_courant_onglet,
				      my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_init ))),
						  NULL ));

/* FIXME : voir pourquoi remplissage opé et remettre l'ajustement */

/* 	value = gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value; */
	remplissage_liste_operations ( compte_courant_onglet );
/* 	gtk_clist_get_vadjustment ( GTK_CLIST ( CLIST_OPERATIONS )) -> value = value; */

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
	mise_a_jour_fin_comptes_passifs = 1;
    }

    /* vérification du solde mini autorisé */

    if ( gsb_account_get_mini_balance_authorized (compte_courant_onglet) != my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_autorise ))),
											NULL ) )
    {
	gsb_account_set_mini_balance_authorized ( compte_courant_onglet, 
						  my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_autorise ))),
							      NULL ));
	gsb_account_set_mini_balance_authorized_message ( compte_courant_onglet,
							  0 );

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
    }



    /* vérification du solde mini voulu */

    if ( gsb_account_get_mini_balance_wanted (compte_courant_onglet) != my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_voulu ))),
										    NULL ) )
    {
	gsb_account_set_mini_balance_wanted ( compte_courant_onglet, 
					      my_strtod ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_voulu ))),
							  NULL ));
	gsb_account_set_mini_balance_wanted_message ( compte_courant_onglet,
						      0 );

	mise_a_jour_soldes_minimaux = 1;
    }


    /* récupération du texte */
    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW (detail_commentaire) );
    gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
    gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );
    gsb_account_set_comment ( compte_courant_onglet,
			      gtk_text_buffer_get_text ( buffer , &start, &end, TRUE ) );

    if ( strlen ( gsb_account_get_comment (compte_courant_onglet) ))
    {
	gsb_account_set_comment ( compte_courant_onglet,
				  g_strdelimit ( gsb_account_get_comment (compte_courant_onglet),
						 "{",
						 '(' ) );
	gsb_account_set_comment ( compte_courant_onglet,
				  g_strdelimit ( gsb_account_get_comment (compte_courant_onglet),
						 "}",
						 ')' ) );
    }
    else
	gsb_account_set_comment ( compte_courant_onglet,
				  NULL );


    /* vérification du nom du compte */
    /* on doit le vérifier en dernier car s'il a changé, on va réafficher */
    /* la liste des comptes, et les infos non encore récupérées vont être */
    /* perdues */

    if ( !gsb_account_get_name (compte_courant_onglet)
	 ||
	 strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))),
		  gsb_account_get_name (compte_courant_onglet) ) )
    {
	gsb_account_set_name ( compte_courant_onglet,
			       g_strdup ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte )))) );

	gsb_gui_navigation_update_account ( compte_courant_onglet );
	gsb_menu_update_accounts_in_menus ();
	mise_a_jour_liste_comptes_accueil = 1;
	remplissage_liste_echeance ();
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
	mise_a_jour_fin_comptes_passifs = 1;
	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ();

	/* Replace trees contents. */
	remplit_arbre_categ ();
	remplit_arbre_imputation ();
	remplit_arbre_tiers ();

	update_options_menus_comptes ();
	remplissage_liste_comptes_etats ();
	selectionne_liste_comptes_etat_courant ();
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
	gint resultat;

	resultat = question_yes_no_hint ( _("Apply changes to account?"),
					  g_strdup_printf ( _("Account \"%s\" has been modified.\nDo you want to save changes?"),
							    gsb_account_get_name (compte_courant_onglet) ) );
	
	if ( !resultat )
	    gtk_widget_set_sensitive ( hbox_boutons_modif, FALSE );
	else
	{
	    modification_details_compte ();
	    mise_a_jour_liste_comptes_accueil = 1;
	}
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

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
