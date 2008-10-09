/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2008 Benjamin Drieu (bdrieu@april.org) 	      */
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


/**
 * \file gsb_account_property.c
 * work with the property page of the accounts 
 */


#include "include.h"



/*START_INCLUDE*/
#include "gsb_account_property.h"
#include "./gsb_account.h"
#include "./gsb_autofunc.h"
#include "./gsb_bank.h"
#include "./utils_buttons.h"
#include "./gsb_currency.h"
#include "./gsb_data_account.h"
#include "./gsb_data_bank.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_transaction.h"
#include "./utils_editables.h"
#include "./gsb_form.h"
#include "./gsb_form_scheduler.h"
#include "./fenetre_principale.h"
#include "./navigation.h"
#include "./menu.h"
#include "./gsb_real.h"
#include "./gsb_scheduler_list.h"
#include "./main.h"
#include "./utils.h"
#include "./tiers_onglet.h"
#include "./dialog.h"
#include "./etats_config.h"
#include "./categories_onglet.h"
#include "./imputation_budgetaire.h"
#include "./transaction_list.h"
#include "./structures.h"
#include "./gsb_transactions_list.h"
#include "./accueil.h"
#include "./gsb_data_transaction.h"
#include "./gsb_form_scheduler.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_account_property_change_currency ( GtkWidget *combobox,
						gpointer null );
static gboolean gsb_account_property_changed ( GtkWidget *widget,
					gint *p_origin  );
static gboolean gsb_account_property_changed_bank_label ( GtkWidget *combobox,
						   gpointer null );
/*END_STATIC*/



static GtkWidget *edit_bank_button = NULL;
static GtkWidget *detail_nom_compte = NULL;
static GtkWidget *detail_type_compte = NULL;
static GtkWidget *detail_titulaire_compte = NULL;
static GtkWidget *detail_adresse_titulaire = NULL;
static GtkWidget *bank_list_combobox = NULL;
static GtkWidget *detail_no_compte = NULL;
static GtkWidget *label_code_banque = NULL;
static GtkWidget *detail_guichet = NULL;
static GtkWidget *detail_cle_compte = NULL;
static GtkWidget *detail_devise_compte = NULL;
static GtkWidget *detail_compte_cloture = NULL;
static GtkWidget *detail_solde_init = NULL;
static GtkWidget *detail_solde_mini_autorise = NULL;
static GtkWidget *detail_solde_mini_voulu = NULL;
static GtkWidget *detail_commentaire = NULL;
static GtkWidget *button_holder_address = NULL;

enum origin
{
    PROPERTY_NAME = 0,
    PROPERTY_KIND,
    PROPERTY_CURRENCY,
    PROPERTY_CLOSED_ACCOUNT,
    PROPERTY_INIT_BALANCE,
    PROPERTY_WANTED_BALANCE,
};

/*START_EXTERN*/
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gsb_real null_real ;
/*END_EXTERN*/





/**
 * create the page of account property
 *
 * \param
 *
 * \return the page
 * */
GtkWidget *gsb_account_property_create_page ( void )
{
    GtkWidget *onglet, *vbox, *scrolled_window, *hbox, *vbox2;
    GtkWidget *label, *scrolled_window_text, *paddingbox;
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

    detail_nom_compte = gsb_autofunc_entry_new ( NULL,
						 G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_NAME), 
						 G_CALLBACK (gsb_data_account_set_name),
						 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_nom_compte, TRUE, TRUE, 0);
    g_signal_connect ( G_OBJECT (detail_nom_compte), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_nom_compte );

    /* create the box of kind of account */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Account type")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    /* create the list of the kind of account combobox */
    detail_type_compte = gsb_autofunc_combobox_new ( gsb_account_property_create_combobox_list (),
						     0,
						     G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_KIND),
						     G_CALLBACK (gsb_data_account_set_kind), 0 );
    g_signal_connect ( G_OBJECT (detail_type_compte), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_type_compte );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_type_compte, TRUE, TRUE, 0);

    /* create the currency line */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Account currency")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_devise_compte = gsb_autofunc_currency_new (TRUE, 0,
						      G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_CURRENCY),
						      NULL, 0 );
    g_signal_connect ( G_OBJECT (detail_devise_compte ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_devise_compte );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_devise_compte, TRUE, TRUE, 0);


    /* create closed account line */
    detail_compte_cloture = gsb_autofunc_checkbutton_new (_("Closed account"), FALSE,
							  G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_CLOSED_ACCOUNT),
							  G_CALLBACK (gsb_data_account_set_closed_account), 0 );
    g_signal_connect ( G_OBJECT (detail_compte_cloture ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_compte_cloture );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), detail_compte_cloture, FALSE, FALSE, 0 );

    /* création de la ligne du titulaire du compte */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Account holder"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Holder name")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_titulaire_compte = gsb_autofunc_entry_new (NULL,
						      NULL, NULL,
						      G_CALLBACK (gsb_data_account_set_holder_name), 0);
    g_signal_connect ( G_OBJECT (detail_titulaire_compte ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_titulaire_compte );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_titulaire_compte, TRUE, TRUE, 0);

    /* address of the holder line */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    vbox2 = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start ( GTK_BOX (hbox), vbox2, FALSE, FALSE, 0 );

    /* we need to create first the text_view because used in callbacks */
    detail_adresse_titulaire = gsb_autofunc_textview_new ( NULL,
							   NULL, NULL,
							   G_CALLBACK (gsb_data_account_set_holder_address), 0 );

    /* now the checkbutton for different address */
    button_holder_address = gsb_autofunc_checkbutton_new ( COLON(_("Holder's own address")), FALSE,
							   G_CALLBACK (gsb_editable_erase_text_view), detail_adresse_titulaire,
							   NULL, 0 );
    gtk_button_set_alignment ( GTK_BUTTON (button_holder_address), 0.0, 0.0 );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), button_holder_address );
    g_signal_connect ( G_OBJECT (button_holder_address), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &button_holder_address );
    gtk_box_pack_start ( GTK_BOX(vbox2), button_holder_address, FALSE, FALSE, 0);

    /* if un-select the holder's button address, we need to erase the tree_view,
     * else issue when come back to that account property */
    g_signal_connect ( G_OBJECT (button_holder_address),
		       "toggled",
		       G_CALLBACK (gsb_button_sensitive_by_checkbutton),
		       detail_adresse_titulaire );

    /* create the text view for holder address */
    scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX(hbox), scrolled_window_text, TRUE, TRUE, 0);


    /* text view created before */
    g_signal_connect ( G_OBJECT (detail_adresse_titulaire ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_adresse_titulaire );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_adresse_titulaire );
    gtk_widget_set_sensitive (detail_adresse_titulaire, FALSE);


    /* ligne de l'établissement financier */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Bank"));
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Financial institution")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    bank_list_combobox = gsb_bank_create_combobox ( 0,
						    NULL, NULL,
						    G_CALLBACK (gsb_data_account_set_bank), 0 );
    g_signal_connect ( G_OBJECT (bank_list_combobox),
		       "changed",
		       G_CALLBACK (gsb_account_property_changed_bank_label),
		       NULL );
    g_signal_connect ( G_OBJECT (bank_list_combobox ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &bank_list_combobox );
    gtk_box_pack_start ( GTK_BOX(hbox), bank_list_combobox, TRUE, TRUE, 0);

    edit_bank_button = gtk_button_new_from_stock ( GTK_STOCK_EDIT );
    g_signal_connect ( G_OBJECT (edit_bank_button ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &edit_bank_button );
    gtk_button_set_relief ( GTK_BUTTON ( edit_bank_button ), GTK_RELIEF_NONE );
    g_signal_connect ( G_OBJECT ( edit_bank_button ),
		       "clicked",
		       G_CALLBACK (gsb_bank_edit_from_button),
		       bank_list_combobox );
    gtk_box_pack_start ( GTK_BOX ( hbox ), edit_bank_button, FALSE, FALSE, 0 );

    /* create the code of bank */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Bank sort code")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    label_code_banque = gtk_label_new (NULL);
    g_signal_connect ( G_OBJECT (label_code_banque ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &label_code_banque );
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

    detail_guichet = gsb_autofunc_entry_new ( NULL,
					      NULL, NULL,
					      G_CALLBACK (gsb_data_account_set_bank_branch_code), 0);
    g_signal_connect ( G_OBJECT (detail_guichet ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_guichet );
    gtk_box_pack_start ( GTK_BOX(hbox), detail_guichet, TRUE, TRUE, 0);


    /* création de la ligne du numéro du compte */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Account number / Key")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0);

    detail_no_compte = gsb_autofunc_entry_new ( NULL,
						NULL, NULL,
						G_CALLBACK (gsb_data_account_set_bank_account_number), 0 );
    g_signal_connect ( G_OBJECT (detail_no_compte ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_no_compte );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_no_compte, TRUE, TRUE, 0 );

    detail_cle_compte = gsb_autofunc_entry_new ( NULL,
						 NULL, NULL,
						 G_CALLBACK (gsb_data_account_set_bank_account_key), 0);
    g_signal_connect ( G_OBJECT (detail_cle_compte ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_cle_compte );
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

    detail_solde_init = gsb_autofunc_real_new ( null_real,
						G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_INIT_BALANCE),
						G_CALLBACK (gsb_data_account_set_init_balance), 0);
    g_signal_connect ( G_OBJECT (detail_solde_init ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_solde_init );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_init, TRUE, TRUE, 0 );


    /* création de la ligne du solde mini autorisé */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Minimum authorised balance")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_mini_autorise = gsb_autofunc_real_new (null_real,
							G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_WANTED_BALANCE),
							G_CALLBACK (gsb_data_account_set_mini_balance_authorized), 0);
    g_signal_connect ( G_OBJECT (detail_solde_mini_autorise ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_solde_mini_autorise );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_mini_autorise, TRUE, TRUE, 0 );


    /* création de la ligne du solde mini voulu */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    label = gtk_label_new ( COLON(_("Minimum desired balance")) );
    gtk_misc_set_alignment ( GTK_MISC(label), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group ), label );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    detail_solde_mini_voulu = gsb_autofunc_real_new (null_real,
						     G_CALLBACK (gsb_account_property_changed), GINT_TO_POINTER (PROPERTY_WANTED_BALANCE),
						     G_CALLBACK (gsb_data_account_set_mini_balance_wanted), 0);
    g_signal_connect ( G_OBJECT (detail_solde_mini_voulu ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_solde_mini_voulu );
    gtk_box_pack_start ( GTK_BOX ( hbox ), detail_solde_mini_voulu, TRUE, TRUE, 0 );

    /* comments line */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Comments"));

    scrolled_window_text = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window_text ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window_text), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window_text, TRUE, TRUE, 5 );

    detail_commentaire = gsb_autofunc_textview_new ( NULL,
						     NULL, NULL,
						     G_CALLBACK (gsb_data_account_set_comment), 0);
    g_signal_connect ( G_OBJECT (detail_commentaire ), "destroy",
		       G_CALLBACK ( gtk_widget_destroyed), &detail_commentaire );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window_text ), detail_commentaire );

    gtk_widget_show_all ( onglet );
    return ( onglet );
}

/**
 * fill the content of the property of the selected account
 *
 * \param
 *
 * \return
 * */
void gsb_account_property_fill_page ( void )
{
    gint bank_number;
    gint current_account;

    devel_debug (NULL);

    current_account = gsb_gui_navigation_get_current_account ();

    gsb_autofunc_entry_set_value (detail_nom_compte,
				  gsb_data_account_get_name (current_account), current_account);

    gsb_autofunc_combobox_set_index (detail_type_compte,
				     gsb_data_account_get_kind (current_account), current_account);

    gsb_autofunc_currency_set_currency_number (detail_devise_compte,
					       gsb_data_account_get_currency (current_account), current_account);

    gsb_autofunc_checkbutton_set_value (detail_compte_cloture,
					gsb_data_account_get_closed_account (current_account), current_account);

    gsb_autofunc_entry_set_value ( detail_titulaire_compte,
				   gsb_data_account_get_holder_name (current_account), current_account );

    gsb_autofunc_checkbutton_set_value ( button_holder_address,
					 gsb_data_account_get_holder_address (current_account) != NULL, 0 );

    gsb_autofunc_textview_set_value ( detail_adresse_titulaire,
				      gsb_data_account_get_holder_address (current_account), current_account );

    /* fill bank informations */
    bank_number = gsb_data_account_get_bank (current_account);
    /* here i don't know why, at the opening of the file, if we go into the property of account,
     * the bank code is not shown, but the function under really show it... and if we do anything
     * (toggle a checkbutton, change account...) this code appear. didn't find why... */
    gsb_bank_list_set_bank (bank_list_combobox,
			    bank_number,
			    current_account );

    gsb_autofunc_entry_set_value (detail_guichet,
				  gsb_data_account_get_bank_branch_code (current_account), current_account);
    gsb_autofunc_entry_set_value (detail_no_compte,
				  gsb_data_account_get_bank_account_number (current_account), current_account );
    gsb_autofunc_entry_set_value (detail_cle_compte,
				  gsb_data_account_get_bank_account_key (current_account), current_account );

    gsb_autofunc_real_set ( detail_solde_init,
			    gsb_data_account_get_init_balance (current_account,
							       gsb_data_currency_get_floating_point (gsb_data_account_get_currency (current_account))),
			    current_account);
    gsb_autofunc_real_set (detail_solde_mini_autorise,
			   gsb_data_account_get_mini_balance_authorized (current_account), current_account);
    gsb_autofunc_real_set (detail_solde_mini_voulu,
			   gsb_data_account_get_mini_balance_wanted (current_account), current_account);

    gsb_autofunc_textview_set_value ( detail_commentaire,
				      gsb_data_account_get_comment (current_account), current_account);
}


/**
 * callback called when change the bank for the account
 * change the bank code label under the combobox
 *
 * \param combobox the combobox containing the banks
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_account_property_changed_bank_label ( GtkWidget *combobox,
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


/**
 * create a list of kind of account to use in a gsb_combo_box or gsb_autofunc_combobox
 * this is an alternance of text (kind of account) and number
 *
 * \param
 *
 * \return a newly allocated GSList
 * */
GSList *gsb_account_property_create_combobox_list ( void )
{
    gchar *text [] = { _("Bank account"), _("Cash account"), _("Liabilities account"), _("Assets account"),NULL };
    gint i = 0;
    GSList *list = NULL;

    while (text[i])
    {
	list = g_slist_append (list, text[i]);
	list = g_slist_append (list, GINT_TO_POINTER (i));
	i++;
    }
    return list;
}



/**
 * called when something changed and need to modify other things
 *
 * \param widget various widget according of the origin
 * \param p_origin the origin as a pointer
 *
 * \return FALSE;
 * */
gboolean gsb_account_property_changed ( GtkWidget *widget,
					gint *p_origin  )
{
    gint origin = GPOINTER_TO_INT (p_origin);
    gint account_number;

    account_number = gsb_gui_navigation_get_current_account ();

    switch (origin)
    {
	case PROPERTY_NAME:
	    /* update the scheduler list */
	    gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());

	    /* update reports */
	    remplissage_liste_comptes_etats ();
	    selectionne_liste_comptes_etat_courant ();

	    /*update the the view menu */
	    gsb_navigation_update_account_label (account_number);
	    /* no break here !! need to do the same to PROPERTY_CLOSED_ACCOUNT too */

	case PROPERTY_CLOSED_ACCOUNT:
	    gsb_gui_navigation_update_account ( account_number );
	    gsb_menu_update_accounts_in_menus ();

	    /* update the name of accounts in form */
	    gsb_account_update_combo_list ( gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT),
					    FALSE );

	    /* Replace trees contents. */
	    remplit_arbre_categ ();
	    remplit_arbre_imputation ();
	    payee_fill_tree ();
	    break;

	case PROPERTY_KIND:
	    gsb_gui_navigation_update_account ( account_number );
	    gsb_form_clean(gsb_form_get_account_number ());
	    break;

	case PROPERTY_CURRENCY:
	    gsb_account_property_change_currency (widget, NULL);
	    break;

	case PROPERTY_INIT_BALANCE:
	    /* as we changed the initial balance, we need to recalculate the amount
	     * of each line in the list */
	    transaction_list_set_balances ();

	    break;

	case PROPERTY_WANTED_BALANCE:
	    break;
    }

    /* update main page */
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}



/**
 * called when change the currency of the account,
 * propose to change the currency of all the transactions, or let them
 *
 * \param combobox the combobox of currencies
 * \param null a pointer not used
 *
 * \return FALSE
 * */
gboolean gsb_account_property_change_currency ( GtkWidget *combobox,
						gpointer null )
{
    gint account_number;
    gint new_currency_number;
    gint account_currency_number;
    gint result;
    GSList *list_tmp;
    gchar *string;

    account_number = gsb_gui_navigation_get_current_account ();

    account_currency_number = gsb_data_account_get_currency (account_number);
    new_currency_number = gsb_currency_get_currency_from_combobox (detail_devise_compte);

    /* set the new currency, must set here and no in the autofunc directly  */
    gsb_data_account_set_currency ( account_number,
				    new_currency_number );

    /* ask for the currency of the transactions */
    gchar* tmpstr = g_strdup_printf ( _("You are changing the currency of the account, do you want to change the currency of the transactions too ?\n(yes will change all the transactions currency from %s to %s, all the transactions with another currency will stay the same).\n\nArchived and reconcilied transactions will be left unmodified."), 
				      gsb_data_currency_get_name (account_currency_number),
				      gsb_data_currency_get_name (new_currency_number));
    result = question_yes_no_hint ( _("Change the transactions currency"),
				    tmpstr,
				    GTK_RESPONSE_NO );
    g_free ( tmpstr );

    if (result)
    {
	/* we have to change the currency of the transactions,
	 * we never want to change the archives, neither the marked R transactions
	 * so change only the non archived and non marked R transactions */
	list_tmp = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp )
	{
	    gint transaction_number;
	    transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number) == account_number
		 &&
		 gsb_data_transaction_get_currency_number (transaction_number) == account_currency_number
		 &&
		 gsb_data_transaction_get_marked_transaction (transaction_number) != OPERATION_RAPPROCHEE)
		gsb_data_transaction_set_currency_number ( transaction_number,
							   new_currency_number );
	    list_tmp = list_tmp -> next;
	}
    }

    transaction_list_update_element (ELEMENT_CREDIT);
    transaction_list_update_element (ELEMENT_DEBIT);
    transaction_list_update_element (ELEMENT_AMOUNT);

    /* in each cases, we had to update the account balance */
    transaction_list_set_balances ();

    /* update the headings balance */
    if (gsb_data_account_get_current_balance (account_number).mantissa < 0)
	string = g_strdup_printf ( "<span color=\"red\">%s</span>",
				   gsb_real_get_string_with_currency ( gsb_data_account_get_current_balance (account_number),
								       gsb_data_account_get_currency (account_number), TRUE ));
    else
	string = gsb_real_get_string_with_currency ( gsb_data_account_get_current_balance (account_number),
						     gsb_data_account_get_currency (account_number), TRUE );
    gsb_gui_headings_update_suffix ( string );
    g_free (string);
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
