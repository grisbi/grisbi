/* ************************************************************************** */
/* Fichier qui s'occupe des manipulations de comptes                          */
/*                                                                            */
/*                         comptes_traitements.c                              */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/*			2004 Alain Portal (aportal@univ-montp2.fr)	      */
/*			http://www.grisbi.org				      */
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
#include "comptes_traitements.h"
#include "type_operations.h"
#include "comptes_gestion.h"
#include "dialog.h"
#include "utils_echeances.h"
#include "fichiers_gestion.h"
#include "gsb_data_account.h"
#include "operations_comptes.h"
#include "gsb_data_transaction.h"
#include "navigation.h"
#include "menu.h"
#include "main.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "utils_comptes.h"
#include "etats_config.h"
#include "echeancier_liste.h"
#include "structures.h"
#include "operations_formulaire.h"
#include "fenetre_principale.h"
#include "echeancier_formulaire.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/





/*START_EXTERN*/
extern GtkWidget *account_page;
extern GtkWidget *bouton_supprimer_compte;
extern gint compte_courant_onglet;
extern struct operation_echeance *echeance_selectionnnee;
extern GSList *liste_struct_echeances;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint nb_echeances;
extern GtkWidget *notebook_general;
extern GtkStyle *style_entree_formulaire[2];
extern GtkWidget *tree_view_vbox;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
/*END_EXTERN*/



/** called to create a new account
 * \param none
 * \return FALSE FALSE
 */

gboolean new_account ( void )
{
    kind_account type_de_compte;
    gint no_compte;

    /*     if no accounts, it's a new file */ 
    if ( !gsb_data_account_get_accounts_amount () )
    {
	new_file ();
	return FALSE;
    }

    /*     ask for the kind_account */ 
    type_de_compte = demande_type_nouveau_compte ();

    if ( type_de_compte == -1 )
	return FALSE;

    /*     create the new account */ 
    no_compte = gsb_data_account_new ( type_de_compte );

    if ( no_compte == -1 )
    {
	dialogue_error_memory ();
	return FALSE;
    }

    /* update the combofix for categ */ 
    mise_a_jour_combofix_categ();

    /* update options menus of accounts */ 
    update_options_menus_comptes ();

    /* update the main page */ 
    mise_a_jour_liste_comptes_accueil = 1;

    remplissage_liste_comptes_etats ();
    selectionne_liste_comptes_etat_courant ();

    gtk_widget_set_sensitive ( bouton_supprimer_compte, TRUE );

    /* update the accounts lists */ 
    gsb_menu_update_accounts_in_menus (); 
    compte_courant_onglet = no_compte;

    /* Go to accounts properties */
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
			    GSB_ACCOUNT_PAGE );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( account_page ), 1 );
    remplissage_details_compte ();

    /* Add an entry in navigation pane. */
    gsb_gui_navigation_add_account ( no_compte );

    modification_fichier ( TRUE );
    return FALSE;
}
/* ************************************************************************** */



/** that function delete the current account selected in the account properties
 * \param none
 * \return FALSE FALSE
 * */

gboolean delete_account ( void )
{
    gint deleted_account;
    gint page_number;
    struct operation_echeance *echeance;
    GSList *list_tmp;

    deleted_account = compte_courant_onglet;

    if ( !question_yes_no_hint ( g_strdup_printf (_("Delete account \"%s\"?"),
						  gsb_data_account_get_name (deleted_account)),
				 _("This will irreversibly remove this account and all operations that were previously contained.  There is no undo for this. Usually it's a better way to close an account.") ))
	return FALSE;

    /* if the las account, close the file */

    if ( gsb_data_account_get_accounts_amount () == 1 )
    {
	etat.modification_fichier = 0;
	fermer_fichier ();
	return FALSE;
    }

    /* delete the transactions list page */

    gtk_notebook_remove_page ( GTK_NOTEBOOK ( tree_view_vbox ),
			       deleted_account + 1 );

    /* delete the schedules transactions on that account */

    while ( (echeance = echeance_par_no_compte ( deleted_account )))
    {
	if ( echeance_selectionnnee == echeance )
	    echeance_selectionnnee = GINT_TO_POINTER (-1);

	liste_struct_echeances = g_slist_remove ( liste_struct_echeances,
						  echeance );
	nb_echeances--;
    }


    /*     delete the account */

    gsb_data_account_delete ( deleted_account );

    /*     check all the transactions, and put -1 if it's a transfer to the deleted account */

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GSList *list_tmp_transactions;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while ( list_tmp_transactions )
	{
	    gint transaction_number;
	    transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	    if ( gsb_data_transaction_get_account_number (transaction_number) == i )
	    {
		if ( gsb_data_transaction_get_account_number_transfer (transaction_number) == deleted_account )
		{
		    gsb_data_transaction_set_account_number_transfer ( transaction_number,
								       -1);
		    gsb_data_account_set_update_list ( i,
						  1 );
		}
	    }
	    list_tmp_transactions = list_tmp_transactions -> next;
	}
	list_tmp = list_tmp -> next;
    }

    /* check gsb_data_account_get_current_account () and gsb_data_account_get_current_account ()_onglet and put them
     * on the first account if they are on the deleted account */

    if ( gsb_data_account_get_current_account () == deleted_account )
    {
	/* update the transaction list */

	page_number = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook_general));

	gsb_data_account_list_gui_change_current_account ( GINT_TO_POINTER ( gsb_data_account_first_number () ));

	gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), page_number );
    }

    if ( compte_courant_onglet == deleted_account )
	compte_courant_onglet = gsb_data_account_first_number ();

    /* update the buttons lists */

    gsb_menu_update_accounts_in_menus();

    /* update the combofixes if needed */

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation();

    /* Replace trees contents. */

    remplit_arbre_categ ();
    remplit_arbre_imputation ();
    remplit_arbre_tiers ();

    /* update options menus of accounts */

    update_options_menus_comptes ();

    remplissage_liste_echeance ();
    mise_a_jour_liste_echeances_manuelles_accueil = 1;
    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    remplissage_liste_comptes_etats ();
    selectionne_liste_comptes_etat_courant ();

    /* Update navigation pane. */
    gsb_gui_navigation_remove_account ( deleted_account );

    modification_fichier( TRUE ); 
    return FALSE;
}



/**
 *  Create a menu with the list of accounts.  This list is
 *  clickable and activates func if specified.
 *
 * \param func Function to call when a line is selected
 * \param activate_currrent If set to TRUE, does not mark as
 *        unsensitive current account
 * \param include_closed If set to TRUE, include the closed accounts
 *
 * \return A newly created option menu
 */
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent,
					   gboolean include_closed )
{
    GtkWidget *menu;
    GtkWidget *item;
    GSList *list_tmp;

    menu = gtk_menu_new ();

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( !gsb_data_account_get_closed_account (i)
	     ||
	     include_closed )
	{
	    item = gtk_menu_item_new_with_label ( gsb_data_account_get_name (i));
	    gtk_object_set_data ( GTK_OBJECT ( item ),
				  "no_compte",
				  GINT_TO_POINTER (i));
	    if ( func )
		gtk_signal_connect ( GTK_OBJECT ( item ), "activate", GTK_SIGNAL_FUNC(func), NULL );
	    gtk_menu_append ( GTK_MENU ( menu ), item );

	    if ( !activate_currrent && 
		 gsb_data_account_get_current_account () == i)
	    {
		gtk_widget_set_sensitive ( item, FALSE );
	    }      

	    gtk_widget_show ( item );
	}
	list_tmp = list_tmp -> next;
    }

    return ( menu );
}
/* ************************************************************************** */





/* ************************************************************************** */
void changement_choix_compte_echeancier ( void )
{
    GtkWidget *menu;
    gint no_compte;

    no_compte = recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] );

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR] )
    {
	/*       il y a qque chose dans le crï¿œit, on met le menu des types crï¿œit */

	if ( (menu = creation_menu_types ( 2,
					   no_compte,
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					  cherche_no_menu_type_echeancier ( gsb_data_account_get_default_credit (no_compte)));
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }
    else
    {
	/*       il y a qque chose dans le dï¿œit ou c'est par dï¿œaut, on met le menu des types dï¿œit */

	if ( (menu = creation_menu_types ( 1,
					   recupere_no_compte ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT]),
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					  cherche_no_menu_type_echeancier ( gsb_data_account_get_default_debit (no_compte) ) );
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }
}
/* ************************************************************************** */

/* ************************************************************************** */
void creation_types_par_defaut ( gint no_compte,
				 gulong dernier_cheque )
{
    /* si des types d'opï¿œexistaient dï¿œï¿œ on les vire */

    if ( gsb_data_account_get_method_payment_list (no_compte) )
	g_slist_free ( gsb_data_account_get_method_payment_list (no_compte) );

    gsb_data_account_set_method_payment_list ( no_compte,
					  NULL );
    gsb_data_account_set_default_debit ( no_compte,
				    0 );
    gsb_data_account_set_default_credit ( no_compte,
				     0 );

    if ( gsb_data_account_get_kind (no_compte) == GSB_TYPE_BANK )
    {
	/* c'est un compte bancaire, on ajoute virement, prï¿œï¿œement, chï¿œue et cb */
	/* 	  modification par rapport ï¿œavant, les nouveaux n: */
	/* 	    1=virement, 2=dï¿œot, 3=cb, 4=prï¿œï¿œement, 5=chï¿œue */
	/* les modifs pour chaque opï¿œ se feront ï¿œleur chargement */

	struct struct_type_ope *type_ope;

	type_ope = g_malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 1;
	type_ope -> nom_type = g_strdup ( _("Transfer") );
	type_ope -> signe_type = 0;
	type_ope -> affiche_entree = 1;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	gsb_data_account_set_method_payment_list ( no_compte,
					      g_slist_append ( gsb_data_account_get_method_payment_list (no_compte),
							       type_ope ) );

	type_ope = g_malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 2;
	type_ope -> nom_type = g_strdup ( _("Deposit") );
	type_ope -> signe_type = 2;
	type_ope -> affiche_entree = 0;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	gsb_data_account_set_method_payment_list ( no_compte,
					      g_slist_append ( gsb_data_account_get_method_payment_list (no_compte),
							       type_ope ) );

	type_ope = g_malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 3;
	type_ope -> nom_type = g_strdup ( _("Credit card") );
	type_ope -> signe_type = 1;
	type_ope -> affiche_entree = 0;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	gsb_data_account_set_method_payment_list ( no_compte,
					      g_slist_append ( gsb_data_account_get_method_payment_list (no_compte),
							       type_ope ) );

	type_ope = g_malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 4;
	type_ope -> nom_type = g_strdup ( _("Direct debit") );
	type_ope -> signe_type = 1;
	type_ope -> affiche_entree = 0;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	gsb_data_account_set_method_payment_list ( no_compte,
					      g_slist_append ( gsb_data_account_get_method_payment_list (no_compte),
							       type_ope ) );

	type_ope = g_malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 5;
	type_ope -> nom_type = g_strdup ( _("Cheque") );
	type_ope -> signe_type = 1;
	type_ope -> affiche_entree = 1;
	type_ope -> numerotation_auto = 1;
	type_ope -> no_en_cours = dernier_cheque;
	type_ope -> no_compte = no_compte;

	gsb_data_account_set_method_payment_list ( no_compte,
					      g_slist_append ( gsb_data_account_get_method_payment_list (no_compte),
							       type_ope ) );

	gsb_data_account_set_default_debit ( no_compte,
					3 );
	gsb_data_account_set_default_credit ( no_compte,
					 2 );

	/* on crï¿œ le tri pour compte bancaire qui sera 1 2 3 4 5 */

	gsb_data_account_set_sort_list ( no_compte,
				    g_slist_append ( gsb_data_account_get_sort_list (no_compte),
				     GINT_TO_POINTER ( 1 )) );
	gsb_data_account_set_sort_list ( no_compte,
				    g_slist_append ( gsb_data_account_get_sort_list (no_compte),
				     GINT_TO_POINTER ( 2 )) );
	gsb_data_account_set_sort_list ( no_compte,
				    g_slist_append ( gsb_data_account_get_sort_list (no_compte),
				     GINT_TO_POINTER ( 3 )) );
	gsb_data_account_set_sort_list ( no_compte,
				    g_slist_append ( gsb_data_account_get_sort_list (no_compte),
				     GINT_TO_POINTER ( 4 )) );
	gsb_data_account_set_sort_list ( no_compte,
				    g_slist_append ( gsb_data_account_get_sort_list (no_compte),
				     GINT_TO_POINTER ( 5 )) );
    }
    else
    {
	if ( gsb_data_account_get_kind (no_compte) == GSB_TYPE_LIABILITIES )
	{
	    /* c'est un compte de passif, on ne met que le virement */

	    struct struct_type_ope *type_ope;

	    type_ope = g_malloc ( sizeof ( struct struct_type_ope ));
	    type_ope -> no_type = 1;
	    type_ope -> nom_type = g_strdup ( _("Transfer") );
	    type_ope -> signe_type = 0;
	    type_ope -> affiche_entree = 1;
	    type_ope -> numerotation_auto = 0;
	    type_ope -> no_en_cours = 0;
	    type_ope -> no_compte = no_compte;

	    gsb_data_account_set_method_payment_list ( no_compte,
						  g_slist_append ( gsb_data_account_get_method_payment_list (no_compte),
								   type_ope ) );

	    gsb_data_account_set_default_debit ( no_compte,
					    1 );
	    gsb_data_account_set_default_credit ( no_compte,
					     1 );

	    /* on crï¿œ le tri pour compte passif qui sera 1 */

	    gsb_data_account_set_sort_list ( no_compte,
					g_slist_append ( gsb_data_account_get_sort_list (no_compte),
							 GINT_TO_POINTER ( 1 )) );
	}
    }
}
/* ************************************************************************** */

/* ************************************************************************** */
/* Cette fonction est appelï¿œ lors de la crï¿œtion d'un nouveau compte.        */
/* elle renvoie le type demandï¿œpour pouvoir mettre ensuite les types par     */
/* dï¿œaut.                                                                    */
/* ************************************************************************** */
gint demande_type_nouveau_compte ( void )
{
    GtkWidget *dialog;
    gint resultat;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *bouton;
    kind_account type_compte;

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_OK_CANCEL,
				       make_hint ( _("Choose account type"),
						   _("If you choose to continue, an account will be created with default payment methods chosen according to your choice.\nYou will be able to change account type later." ) ) );

    /* crï¿œtion de la ligne du type de compte */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog) -> vbox ), hbox,
			 FALSE, FALSE, 6 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Account type")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 12 );
    gtk_widget_show ( label );

    bouton = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       creation_menu_type_compte() );
    gtk_box_pack_start ( GTK_BOX (hbox), bouton, TRUE, TRUE, 12 );
    gtk_widget_show ( bouton );

    resultat = gtk_dialog_run ( GTK_DIALOG(dialog) );

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( dialog );
	return ( -1 );
    }

    type_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton ) -> menu_item ),
							  "no_type_compte" ));

    gtk_widget_destroy ( dialog );

    return ( type_compte );
}
/* ************************************************************************** */




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
