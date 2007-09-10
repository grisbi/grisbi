/* ************************************************************************** */
/*  Fichier qui s'occupe de la gestion des comptes			      */
/*			gestion_comptes.c				      */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org) 	      */
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
#include "./erreur.h"
#include "./dialog.h"
#include "./comptes_traitements.h"
#include "./gsb_bank.h"
#include "./gsb_currency.h"
#include "./gsb_data_account.h"
#include "./gsb_data_bank.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_transaction.h"
#include "./gsb_form.h"
#include "./gsb_form_scheduler.h"
#include "./navigation.h"
#include "./menu.h"
#include "./gsb_real.h"
#include "./gsb_scheduler_list.h"
#include "./gsb_transactions_list.h"
#include "./categories_onglet.h"
#include "./traitement_variables.h"
#include "./utils_str.h"
#include "./utils.h"
#include "./etats_config.h"
#include "./imputation_budgetaire.h"
#include "./tiers_onglet.h"
#include "./structures.h"
#include "./accueil.h"
#include "./gsb_form_scheduler.h"
#include "./gsb_transactions_list.h"
#include "./include.h"
#include "./gsb_real.h"
#include "./fenetre_principale.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void changement_bouton_adresse_commune_perso ( void );
static gboolean gsb_account_property_update_bank_label ( GtkWidget *combobox,
						  gpointer null );
static gboolean modif_detail_compte ( GtkWidget *hbox );
static void modification_details_compte ( void );
/*END_STATIC*/



GtkWidget *bouton_detail;
GtkWidget *detail_nom_compte;
GtkWidget *detail_type_compte;
GtkWidget *detail_titulaire_compte;
GtkWidget *detail_bouton_adresse_commune;
GtkWidget *detail_adresse_titulaire;
static GtkWidget *bank_list_combobox;
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
    GtkWidget *onglet, *vbox, *scrolled_window, *separateur, *hbox;
    GtkWidget *label, *bouton, *scrolled_window_text, *paddingbox, *vbox2;
    GtkSizeGroup * size_group;

    /* la fenetre ppale est une vbox avec les détails en haut et appliquer en bas */
    onglet = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( onglet ), 10 );

    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    /* partie du haut avec les détails du compte */

    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( onglet ), scrolled_window, TRUE, TRUE, 0 );

    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ), vbox );
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window ) -> child ),
				   GTK_SHADOW_NONE );

    paddingbox = new_paddingbox_with_title (vbox, TRUE, _("Account details"));


    /* création de la ligne du nom du compte */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Account name")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_nom_compte = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX(hbox), detail_nom_compte, TRUE, TRUE, 0);


    /* création de la ligne du type de compte */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Account type")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_type_compte = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_type_compte ),
			       creation_menu_type_compte() );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_type_compte, TRUE, TRUE, 0);

    /* création de la ligne de la devise */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Account currency")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_devise_compte = gsb_currency_make_combobox ( TRUE ); 
    gtk_box_pack_start ( GTK_BOX(hbox), detail_devise_compte, TRUE, TRUE, 0);


    /* création de la ligne compte cloturé */
    detail_compte_cloture = gtk_check_button_new_with_label ( _("Closed account") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), detail_compte_cloture, FALSE, FALSE, 0 );


    /* création de la ligne du titulaire du compte */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Account holder"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Holder name")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_titulaire_compte = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX(hbox), detail_titulaire_compte, TRUE, TRUE, 0);


    /* création de la ligne titulaire a l'adr commune */
    detail_bouton_adresse_commune = gtk_radio_button_new_with_label ( NULL,
								      _("Common address is holder's address") );
    gtk_button_set_alignment ( GTK_BUTTON ( detail_bouton_adresse_commune ),
			       MISC_LEFT, MISC_TOP );
    gtk_signal_connect ( GTK_OBJECT ( detail_bouton_adresse_commune ), "toggled",
			 GTK_SIGNAL_FUNC ( changement_bouton_adresse_commune_perso ), NULL );
    gtk_box_pack_start ( GTK_BOX(paddingbox), detail_bouton_adresse_commune, FALSE, FALSE, 0);


    /* création de la ligne titulaire a sa propre adresse */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    vbox2 = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox2, FALSE, TRUE, 0 );
    gtk_widget_show ( vbox2 );

    bouton = gtk_radio_button_new_with_label ( gtk_radio_button_group ( GTK_RADIO_BUTTON ( detail_bouton_adresse_commune)),
					       _("Holder's address") );
    gtk_button_set_alignment ( GTK_BUTTON ( bouton ), 0.0, 0.0 );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), bouton );
    gtk_box_pack_start ( GTK_BOX(vbox2), bouton, FALSE, FALSE, 0);

    scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text), 
 					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX(hbox), scrolled_window_text, TRUE, TRUE, 0);


    detail_adresse_titulaire = gtk_text_view_new ();
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (detail_adresse_titulaire), 3);
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (detail_adresse_titulaire), GTK_WRAP_WORD );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_adresse_titulaire );


    /* ligne de l'établissement financier */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Bank"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Financial institution")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    bank_list_combobox = gsb_bank_create_combobox ();
    g_signal_connect ( G_OBJECT (bank_list_combobox),
		       "changed",
		       G_CALLBACK (modif_detail_compte),
		       NULL );
    g_signal_connect ( G_OBJECT (bank_list_combobox),
		       "changed",
		       G_CALLBACK (gsb_account_property_update_bank_label),
		       NULL );
    gtk_box_pack_start ( GTK_BOX(hbox), bank_list_combobox, TRUE, TRUE, 0);

    bouton_detail = gtk_button_new_from_stock ( GTK_STOCK_EDIT );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_detail ), GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( bouton_detail ),
		       "clicked",
		       G_CALLBACK (gsb_bank_edit_from_button),
		       bank_list_combobox );
    gtk_box_pack_start ( GTK_BOX ( hbox ), bouton_detail, FALSE, FALSE, 0 );


    /* création de la ligne contenant le code de la banque */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Bank sort code")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    label_code_banque = gtk_label_new (NULL);
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX(hbox), label_code_banque, FALSE, FALSE, 0 );


    /* création de la ligne du guichet */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Bank branch code")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_guichet = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX(hbox), detail_guichet, TRUE, TRUE, 0);


    /* création de la ligne du numéro du compte */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Account number / Key")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_no_compte = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_no_compte, TRUE, TRUE, 0 );

    detail_cle_compte = gtk_entry_new ();
    gtk_widget_set_usize ( detail_cle_compte, 30, FALSE );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_cle_compte, FALSE, FALSE, 0 );


    /* création de la ligne du solde initial */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Balances"));

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Initial balance")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_init = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_init, TRUE, TRUE, 0 );


    /* création de la ligne du solde mini autorisé */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Minimum authorised balance")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_mini_autorise = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_mini_autorise, TRUE, TRUE, 0 );


    /* création de la ligne du solde mini voulu */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Minimum wanted balance")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_mini_voulu = gtk_entry_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_mini_voulu, TRUE, TRUE, 0 );


    /* création de la ligne du commentaire */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Comments"));

    scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window_text, TRUE, TRUE, 5 );

    detail_commentaire = gtk_text_view_new ();
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (detail_commentaire), 3);
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW (detail_commentaire), GTK_WRAP_WORD );

    gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_commentaire );

    /* séparation de la fenetre */

    separateur = gtk_hseparator_new ();
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 separateur,
			 FALSE,
			 TRUE,
			 0 );

    /* mise en forme des boutons appliquer et annuler */

    hbox_boutons_modif = gtk_hbox_new ( FALSE,
					5 );
    gtk_box_pack_start ( GTK_BOX ( onglet ),
			 hbox_boutons_modif,
			 FALSE,
			 FALSE,
			 0 );

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
    gtk_signal_connect_object ( GTK_OBJECT ( detail_devise_compte ),
				"changed",
				GTK_SIGNAL_FUNC ( modif_detail_compte ),
				GTK_OBJECT ( hbox_boutons_modif ) );
    gtk_signal_connect_object ( GTK_OBJECT ( detail_bouton_adresse_commune ),
				"toggled",
				GTK_SIGNAL_FUNC ( modif_detail_compte ),
				GTK_OBJECT ( hbox_boutons_modif ) );
    g_signal_connect_swapped ( G_OBJECT(gtk_text_view_get_buffer(GTK_TEXT_VIEW(detail_adresse_titulaire))),
			       "changed", ((GCallback) modif_detail_compte), 
			       hbox_boutons_modif );
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

    gtk_widget_show_all ( onglet );

    return ( onglet );
}
/* ************************************************************************************************************ */


/**
 * callback called when change the bank for the account
 * change the bank code label under the combobox
 *
 * \param combobox the combobox containing the banks
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_account_property_update_bank_label ( GtkWidget *combobox,
						  gpointer null )
{
    gint bank_number;

    if (!combobox)
	return FALSE;

    bank_number = gsb_bank_list_get_bank_number (combobox);

    if (bank_number <= 0)
	gtk_label_set_text ( GTK_LABEL (label_code_banque),
			     NULL );
    else
	gtk_label_set_text ( GTK_LABEL (label_code_banque),
			     gsb_data_bank_get_code (bank_number));
    return FALSE;
}


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

gboolean modif_detail_compte ( GtkWidget *hbox )
{

    gtk_widget_set_sensitive ( hbox_boutons_modif,
			       TRUE );
    return FALSE;

}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
/* Fonction remplissage_details_compte */
/* ************************************************************************************************************ */

void remplissage_details_compte ( void )
{
    gint bank_number;
    gint current_account;

    devel_debug ( "remplissage_details_compte" );

    current_account = gsb_gui_navigation_get_current_account ();

    g_return_if_fail ( current_account >= 0 );

    my_gtk_entry_set_text ( GTK_ENTRY ( detail_nom_compte ),
			    gsb_data_account_get_name (current_account) );

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( detail_type_compte ),
				  gsb_data_account_get_kind (current_account) );

    gsb_currency_set_combobox_history ( detail_devise_compte,
					gsb_data_account_get_currency (current_account));

    if ( gsb_data_account_get_holder_name (current_account) )
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_titulaire_compte ),
				gsb_data_account_get_holder_name (current_account) );
    else
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_titulaire_compte ),
				"" );

    gtk_text_buffer_set_text ( gtk_text_view_get_buffer (GTK_TEXT_VIEW (detail_adresse_titulaire)),
 			       ( gsb_data_account_get_holder_address (current_account) ? gsb_data_account_get_holder_address (current_account) : "") , -1 );

    if ( gsb_data_account_get_holder_address (current_account) )
    {
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( GTK_RADIO_BUTTON ( detail_bouton_adresse_commune ) -> group -> data ),
				       TRUE );
    }
    else
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( detail_bouton_adresse_commune ),
				       TRUE );

    /*     remplissage des infos sur la banque */
    bank_number = gsb_data_account_get_bank (current_account);
    gsb_bank_list_set_bank (bank_list_combobox,
			    bank_number );

    if ( gsb_data_account_get_bank_branch_code (current_account) )
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_guichet ),
				gsb_data_account_get_bank_branch_code (current_account) );
    else
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_guichet ),
				NULL );

    if ( gsb_data_account_get_bank_account_number (current_account) )
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_no_compte ),
				gsb_data_account_get_bank_account_number (current_account) );
    else
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_no_compte ),
				NULL );

    if ( gsb_data_account_get_bank_account_key (current_account) )
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_cle_compte ),
				gsb_data_account_get_bank_account_key (current_account) );
    else
	my_gtk_entry_set_text ( GTK_ENTRY ( detail_cle_compte ),
				NULL );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON( detail_compte_cloture ),
				   gsb_data_account_get_closed_account (current_account) );


    my_gtk_entry_set_text ( GTK_ENTRY ( detail_solde_init ),
			    gsb_real_get_string (gsb_data_account_get_init_balance (current_account, -1)));

    my_gtk_entry_set_text ( GTK_ENTRY ( detail_solde_mini_autorise ),
			    gsb_real_get_string (gsb_data_account_get_mini_balance_authorized (current_account)));

    my_gtk_entry_set_text ( GTK_ENTRY ( detail_solde_mini_voulu ),
			    gsb_real_get_string (gsb_data_account_get_mini_balance_wanted (current_account)));

    gtk_text_buffer_set_text ( gtk_text_view_get_buffer (GTK_TEXT_VIEW (detail_commentaire)),
			       ( gsb_data_account_get_comment (current_account) ? gsb_data_account_get_comment (current_account) : "") , -1 );

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
    gsb_real tmp_number;
    gchar *string;
    GSList *list_tmp;
    gint current_account;

    /* vérification que le compte a un nom */
    if ( !strlen ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))) )
    {
        dialogue_error_hint ( _("Accounts names are used to distinguish accounts.  It is mandatory that names are both unique and not empty."),
			      _("Account name is empty") );
	return;
    }

    current_account = gsb_gui_navigation_get_current_account ();
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( i == current_account )
	{
	    list_tmp = list_tmp -> next;
	    continue;
	}

	if ( !strcmp ( g_strstrip ( (gchar*) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))), gsb_data_account_get_name (i) ))
	{
	    dialogue_error_hint( _("Accounts names are used to distinguish accounts.  It is mandatory that names are both unique and not empty."),
				 g_strdup_printf ( _("Account \"%s\" already exists!"), gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))));
	    return;
	}

	list_tmp = list_tmp -> next;
    }

    /* récupération du titulaire */
    gsb_data_account_set_holder_name ( current_account,
				       gtk_entry_get_text ( GTK_ENTRY ( detail_titulaire_compte )));


    /* vérification du type de compte */

    if ( GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
						 "no_type_compte" ))
	 !=
	 gsb_data_account_get_kind (current_account) )
    {
	gsb_data_account_set_kind (current_account,
				   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_type_compte ) -> menu_item ),
									   "no_type_compte" )));
	mise_a_jour_fin_comptes_passifs = 1;
	mise_a_jour_soldes_minimaux = 1;
	gsb_gui_navigation_update_account ( current_account );
	gsb_form_clean(gsb_form_get_account_number ());
    }


    /* vérification de la devise */

    if ( gsb_data_account_get_currency (current_account) != gsb_currency_get_currency_from_combobox (detail_devise_compte))
    {
	gint new_currency_number;
	gint account_currency_number;
	gint result;

	account_currency_number = gsb_data_account_get_currency (current_account);
	new_currency_number = gsb_currency_get_currency_from_combobox (detail_devise_compte);

	/* change the currency of the account */
	gsb_data_account_set_currency ( current_account,
					new_currency_number );

	/* ask for the currency of the transactions */
	result = question_yes_no_hint ( _("Change the transactions currency"),
					  g_strdup_printf ( _("You are changing the currency of the account, do you want to change the currency of the transactions too ?\(yes will change all the transactions currency from %s to %s, all the transactions with another currency will stay the same)"), 
							    gsb_data_currency_get_name (account_currency_number),
							    gsb_data_currency_get_name (new_currency_number)),
					  GTK_RESPONSE_NO );

	if (result)
	{
	    /* we have to change the currency of the transactions */
	    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

	    while ( list_tmp )
	    {
		gint transaction_number;
		transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

		if ( gsb_data_transaction_get_account_number (transaction_number) == current_account
		     &&
		     gsb_data_transaction_get_currency_number (transaction_number) == account_currency_number)
		    gsb_data_transaction_set_currency_number ( transaction_number,
							       new_currency_number );
		list_tmp = list_tmp -> next;
	    }
	}

	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_CREDIT);
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_DEBIT);
	gsb_transactions_list_update_transaction_value (TRANSACTION_LIST_AMOUNT);

	/* in each cases, we had to update the account balance */
	gsb_transactions_list_set_transactions_balances (current_account);

	/* update the headings balance */
	string = gsb_real_get_string_with_currency ( gsb_data_account_get_current_balance (current_account),
						     gsb_data_account_get_currency (current_account) );
	gsb_gui_headings_update_suffix ( string );
	g_free (string);

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
    }


    /* récupération de l'adr du titulaire */

    if ( !gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_bouton_adresse_commune )))
    {
        buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW (detail_adresse_titulaire) );
	gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
	gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

	string = gtk_text_buffer_get_text ( buffer , &start, &end, TRUE );

	if (string)
	{
	    g_strdelimit ( string,
			   "{",
			   '(' );
	    g_strdelimit ( string,
			   "}",
			   ')' );
	}
	gsb_data_account_set_holder_address ( current_account,
					      string);
    }
    else
	gsb_data_account_set_holder_address ( current_account,
					      NULL );


    /* enregistrement de l'établissement financier */
    gsb_data_account_set_bank ( current_account,
				gsb_bank_list_get_bank_number (bank_list_combobox));

    /* enregistrement du no de guichet */
    gsb_data_account_set_bank_branch_code ( current_account,
					    gtk_entry_get_text ( GTK_ENTRY ( detail_guichet )));

    /* enregistrement du no de compte */
    gsb_data_account_set_bank_account_number ( current_account,
					       gtk_entry_get_text ( GTK_ENTRY ( detail_no_compte )));

    /* enregistrement de la clé du compte */
    gsb_data_account_set_bank_account_key ( current_account,
					    gtk_entry_get_text ( GTK_ENTRY ( detail_cle_compte )));

    /* enregistrement du compte cloturé */

    if ( gsb_data_account_get_closed_account (current_account) != gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_compte_cloture )))
    {
	gsb_data_account_set_closed_account ( current_account,
					      gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( detail_compte_cloture )));

	gsb_gui_navigation_update_account ( current_account );

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

    tmp_number = gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( detail_solde_init )));
    if ( gsb_real_cmp ( gsb_data_account_get_init_balance (current_account, -1),
			tmp_number ))
    {
	gsb_data_account_set_init_balance (current_account,
					   tmp_number );
	/* as we changed the initial balance, we need to recalculate the amount
	 * of each line in the list */
	gsb_transactions_list_set_transactions_balances (current_account);

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
	mise_a_jour_fin_comptes_passifs = 1;
    }

    /* vérification du solde mini autorisé */

    tmp_number = gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY (detail_solde_mini_autorise)));
    if ( gsb_real_cmp ( gsb_data_account_get_mini_balance_authorized (current_account),
			tmp_number ))
    {
	gsb_data_account_set_mini_balance_authorized ( current_account, 
						       gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_autorise ))));
	gsb_data_account_set_mini_balance_authorized_message ( current_account,
							       0 );

	mise_a_jour_liste_comptes_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
    }



    /* vérification du solde mini voulu */

    tmp_number = gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY (detail_solde_mini_voulu)));
    if ( gsb_real_cmp ( gsb_data_account_get_mini_balance_wanted (current_account),
			tmp_number ))
    {
	gsb_data_account_set_mini_balance_wanted ( current_account, 
						   gsb_real_get_from_string (gtk_entry_get_text ( GTK_ENTRY ( detail_solde_mini_voulu ))));
	gsb_data_account_set_mini_balance_wanted_message ( current_account,
							   0 );

	mise_a_jour_soldes_minimaux = 1;
    }


    /* récupération du texte */
    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW (detail_commentaire) );
    gtk_text_buffer_get_iter_at_offset ( buffer, &start, 0 );
    gtk_text_buffer_get_iter_at_offset ( buffer, &end, -1 );

    string = gtk_text_buffer_get_text ( buffer , &start, &end, TRUE );
    if (string)
    {
	g_strdelimit ( string,
		       "{",
		       '(' );
	g_strdelimit ( string,
		       "}",
		       ')' );   
    }
    gsb_data_account_set_comment ( current_account,
				   string );

    /* vérification du nom du compte */
    /* on doit le vérifier en dernier car s'il a changé, on va réafficher */
    /* la liste des comptes, et les infos non encore récupérées vont être */
    /* perdues */

    if ( !gsb_data_account_get_name (current_account)
	 ||
	 strcmp ( g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte ))),
		  gsb_data_account_get_name (current_account) ) )
    {
	gsb_data_account_set_name ( current_account,
				    gtk_entry_get_text ( GTK_ENTRY ( detail_nom_compte )));

	gsb_gui_navigation_update_account ( current_account );
	gsb_menu_update_accounts_in_menus ();
	mise_a_jour_liste_comptes_accueil = 1;
	gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
	mise_a_jour_liste_echeances_manuelles_accueil = 1;
	mise_a_jour_soldes_minimaux = 1;
	mise_a_jour_fin_comptes_passifs = 1;
	if ( mise_a_jour_combofix_categ_necessaire )
	    mise_a_jour_combofix_categ();

	/* Replace trees contents. */
	remplit_arbre_categ ();
	remplit_arbre_imputation ();
	remplit_arbre_tiers ();

	/* update the name of accounts in form */
	gsb_account_update_name_tree_model ( gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
					     FALSE );

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
	gint result;

	result = question_yes_no_hint ( _("Apply changes to account?"),
					  g_strdup_printf ( _("Property of account \"%s\" has been modified.\nDo you want to save changes?"),
							    gsb_data_account_get_name (gsb_gui_navigation_get_current_account ()) ),
					  GTK_RESPONSE_NO  );
	
	if ( !result )
	    gtk_widget_set_sensitive ( hbox_boutons_modif, FALSE );
	else
	{
	    modification_details_compte ();
	    mise_a_jour_liste_comptes_accueil = 1;
	}
    }
}
/* ************************************************************************************************************ */



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
