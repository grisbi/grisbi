/* ************************************************************************** */
/* Ce fichier s'occupe de la gestion du formulaire de saisie des opérations   */
/* 			formulaire.c                                          */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
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
#include "operations_formulaire.h"
#include "accueil.h"
#include "type_operations.h"
#include "utils_editables.h"
#include "utils_montants.h"
#include "gsb_transactions_list.h"
#include "devises.h"
#include "erreur.h"
#include "utils_devises.h"
#include "dialog.h"
#include "gsb_form.h"
#include "equilibrage.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "gsb_data_report.h"
#include "gsb_data_transaction.h"
#include "exercice.h"
#include "navigation.h"
#include "utils_dates.h"
#include "gtk_combofix.h"
#include "menu.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils_operations.h"
#include "etats_calculs.h"
#include "utils_types.h"
#include "utils.h"
#include "structures.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean element_focusable ( gint no_element );
static gboolean gsb_form_get_categories ( gint transaction_number,
				   gint new_transaction );
static GSList *gsb_form_get_parties_list_from_report ( void );
static void gsb_form_take_datas_from_form ( gint transaction_number );
static gboolean gsb_form_validate_form_transaction ( gint transaction_number );
static gboolean gsb_transactions_list_recover_breakdowns_of_transaction ( gint new_transaction_number,
								   gint no_last_breakdown,
								   gint no_account );
/*END_STATIC*/





/*START_EXTERN*/
extern gboolean block_menu_cb ;
extern struct struct_devise *devise_compte;
extern GtkWidget *formulaire;
extern gint hauteur_ligne_liste_opes;
extern GtkItemFactory *item_factory_menu_general;
extern gchar *last_date;
extern GSList *liste_struct_devises;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkStyle *style_entree_formulaire[2];
extern gdouble taux_de_change[2];
extern GtkWidget *tree_view;
extern GtkWidget *vbox_boutons_formulaire;
/*END_EXTERN*/







/******************************************************************************/
void echap_formulaire ( void )
{
    formulaire_a_zero();

    if ( !etat.formulaire_toujours_affiche )
	gsb_form_hide ();

    gtk_widget_grab_focus ( gsb_transactions_list_get_tree_view());

}
/******************************************************************************/


/******************************************************************************/
/* cette fonction vérifie s'il y a une date entrée dans le widget date */
/* dont le no est donné en argument */
/* si l'entrée est vide,  */
/*     soit c'est une entree de date de valeur */
/*     	on met la date de la date d'opération */
/* 	ou la date du jour si l'opé n'a pas de date */
/*    soit c'est la date d'opé, on met la date du jour */
/* \param origine le no du champ de date : */
/* 	TRANSACTION_FORM_DATE */
/* 	TRANSACTION_FORM_VALUE_DATE */
/******************************************************************************/
void verifie_champs_dates ( gint origine )
{
    GtkWidget *widget;

    switch ( origine )
    {
	case TRANSACTION_FORM_VALUE_DATE:

	    /* si la date de valeur est vide, alors on
	       récupère la date d'opération comme date de valeur */

	    widget = gsb_form_get_element_widget ( origine );

	    if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
	    {
		/* si la date d'opération est vide elle aussi */
		/* 		    on met la date du jour dans les 2 champs */

		GtkWidget *date_entry;

		date_entry = gsb_form_get_element_widget (TRANSACTION_FORM_DATE);

		if ( gtk_widget_get_style (date_entry) == style_entree_formulaire[ENCLAIR]
		     &&
		     strlen ( gtk_entry_get_text ( GTK_ENTRY (date_entry))))
		{
		    /* il y a qque chose dans la date, on le recopie */

		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 gtk_entry_get_text ( GTK_ENTRY (date_entry)));
		}
		else
		{
		    /* il n'y a rien dans la date, on y met la date du jour ainsi que dans la date de valeur */

		    entree_prend_focus ( date_entry, NULL, NULL );
		    gtk_entry_set_text ( GTK_ENTRY ( date_entry ),
					 gsb_today() );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 gsb_today() );
		}
	    }
	    break;


	case TRANSACTION_FORM_DATE:

	    /* si le champ est vide, alors on
	       récupère la date du jour comme date d'opération */

	    widget = gsb_form_get_element_widget ( origine );

	    if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY ( widget ))))
		gtk_entry_set_text ( GTK_ENTRY ( widget ),
				     gsb_today() );
	    break;
    }
}
/******************************************************************************/


/******************************************************************************/
/* renvoie le no d'élément dans le formulaire correspondant à l'élément suivant */
/* dans le sens donné en argument */
/* l'élément retourné peut recevoir le focus et il est affiché */
/* \param element_courant le no d'élément d'origine */
/* \param sens_deplacement le sens de déplacement pour retrouver l'élément suivant 
 * 				(0=gauche, 1=droite, 2=haut, 3=bas) */
/* \return no de l'élément qui recevra le focus,
 * 		-1 en cas de pb ou pas de changement
 * 		-2 en cas de fin d'opération (tab sur bout du formulaire)*/
/******************************************************************************/
gint recherche_element_suivant_formulaire ( gint element_courant,
					    gint sens_deplacement )
{
    gint row;
    gint column;
    gint return_value_number = -1;
    gint account_number = 0;

    account_number = gsb_gui_navigation_get_current_account ();

    if ( !gsb_data_form_look_for_value ( account_number,
					 element_courant,
					 &row,
					 &column ))
	return -1;

    while ( !element_focusable ( return_value_number )) 
    {
	switch ( sens_deplacement )
	{
	    /* 	    gauche */
	    case 0:

		/* 		on recherche l'élément précédent, si on est en haut à gauche */
		/* 		    on passe en bas à droite */

		if ( !column && !row )
		{
		    column = gsb_data_form_get_nb_columns (account_number);
		    row = gsb_data_form_get_nb_rows (account_number) -1; 
		}

		if ( --column == -1 )
		{
		    column = gsb_data_form_get_nb_columns (account_number) - 1;
		    row--;
		}
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

		/* 		droite */
	    case 1:

		/* 		on recherche l'élément suivant */
		/* 		    si on est en bas à droite, on passe en haut à gauche */
		/* 		ou on enregistre l'opé en fonction de la conf */

		if ( column == (gsb_data_form_get_nb_columns (account_number) - 1)
		     &&
		     row == (gsb_data_form_get_nb_rows (account_number) - 1))
		{
		    if ( !etat.entree )
		    {
			return_value_number = -2;
			continue;
		    }

		    column = -1;
		    row = 0; 
		}

		if ( ++column == gsb_data_form_get_nb_columns (account_number) )
		{
		    column = 0;
		    row++;
		}
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

		/* 		haut */
	    case 2:

		if ( !row )
		{
		    return_value_number = -1;
		    continue;
		}

		row--;
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

		/* 		bas */
	    case 3:

		if ( row == (gsb_data_form_get_nb_rows (account_number) - 1))
		{
		    return_value_number = -1;
		    continue;
		}

		row++;
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    default:
		return_value_number = -1;
	}
    }
    return return_value_number;
}
/******************************************************************************/



/******************************************************************************/
/* vérifie si l'élément du formulaire donné en argument peut recevoir le focus */
/* \param no_element l'élément à tester */
/* \return TRUE s'il peut recevoir le focus */
/******************************************************************************/
gboolean element_focusable ( gint no_element )
{
    GtkWidget *widget;

    /*     si le no_element est -1 ou -2, on renvoie TRUE */
    
    if ( no_element == -1
	 ||
	 no_element == -2 )
	return TRUE;

    widget = gsb_form_get_element_widget ( no_element );

    if ( !widget )
	return FALSE;
    
    if ( !GTK_WIDGET_VISIBLE (widget))
	return FALSE;

    if ( !GTK_WIDGET_SENSITIVE ( widget ))
	return FALSE;

    if ( !(GTK_IS_COMBOFIX ( widget )
	   ||
	   GTK_IS_ENTRY ( widget )
	   ||
	   GTK_IS_BUTTON ( widget )))
	return FALSE;

    return TRUE;
}
/******************************************************************************/



/******************************************************************************/
/* met le focus sur le widget du formulaire dont le no est donné en argument */
/******************************************************************************/
void widget_grab_focus_formulaire ( gint no_element )
{
    GtkWidget *widget;

    widget = gsb_form_get_element_widget ( no_element );

    if ( !widget )
	return;
    
    if ( GTK_IS_COMBOFIX ( widget ))
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget ) -> entry );
    else
	gtk_widget_grab_focus ( widget );

    return;
}
/******************************************************************************/


/******************************************************************************/
/* Fonction completion_operation_par_tiers                                    */
/* appelée lorsque le tiers perd le focus                                     */
/* récupère le tiers, et recherche la dernière opé associée à ce tiers        */
/* remplit le reste de l'opération avec les dernières données                 */
/* \return FALSE pas de tiers, ou pb */
/******************************************************************************/
gboolean completion_operation_par_tiers ( GtkWidget *entree )
{
    gint payee_number;
    gint transaction_number;
    gchar *char_tmp;
    gint row,column;
    gint pas_de_completion = 0;
    gchar *nom_tiers;
    GtkWidget *widget;
    GtkWidget *menu;
    gint account_number;

    /*     s'il n'y a rien dans le tiers, on retourne FALSE */

    nom_tiers = g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entree ))));

    if ( !strlen ( nom_tiers ))
	return FALSE;

    account_number = gsb_gui_navigation_get_current_account ();

    /*     pour la complétion, seul la date et le tiers doivent être remplis, sinon on ne fait rien */


    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column < gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gint value;

	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );

	    if ( value
		 &&
		 value != TRANSACTION_FORM_DATE
		 &&
		 value != TRANSACTION_FORM_PARTY )
	    {
		GtkWidget *widget;

		widget = gsb_form_get_element_widget (value);

		if ( GTK_IS_ENTRY ( widget ))
		{
		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			pas_de_completion = 1;
		}
		if ( GTK_IS_COMBOFIX ( widget ))
		{
		    if ( gtk_widget_get_style ( GTK_COMBOFIX (widget) -> entry ) == style_entree_formulaire[ENCLAIR] )
			pas_de_completion = 1;
		}
	    }
	}

    if ( pas_de_completion )
	return TRUE;
    
    /* get the payee_number */

    payee_number = gsb_data_payee_get_number_by_name ( nom_tiers,
						  FALSE );

    /* if it's a new payee, go away */

    if ( !payee_number )
	return TRUE;

    /* find the last transaction with that payee */

    transaction_number = gsb_transactions_look_for_last_party ( payee_number,
								0,
								gsb_gui_navigation_get_current_account ());

    /* si on n'a trouvé aucune opération, on se tire */

    if ( !transaction_number )
	return TRUE;

    /*     on fait le tour du formulaire en ne remplissant que ce qui est nécessaire */

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column <  gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gint value;

	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );
	    widget =  gsb_form_get_element_widget (value);

	    switch (value)
	    {
		case TRANSACTION_FORM_DEBIT:

		    if ( gsb_data_transaction_get_amount (transaction_number)< 0 )
		    {
			entree_prend_focus ( widget , NULL, NULL);
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       -gsb_data_transaction_get_amount ( transaction_number)));
		    }
		    break;

		case TRANSACTION_FORM_CREDIT:

		    if ( gsb_data_transaction_get_amount (transaction_number)>= 0 )
		    {
			entree_prend_focus ( widget, NULL, NULL );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     g_strdup_printf ( "%4.2f",
							       gsb_data_transaction_get_amount (transaction_number)));
		    }
		    break;

		case TRANSACTION_FORM_CATEGORY:

		    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
		    {
			/* it's a breakdown of transaction */

			entree_prend_focus ( widget, NULL, NULL );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						_("Breakdown of transaction") );
		    }
		    else
		    {
			if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number))
			{
			    /* c'est un virement */

			    entree_prend_focus ( widget, NULL, NULL );

			    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number)!= -1
				 &&
				 gsb_data_transaction_get_account_number_transfer (transaction_number)!= -1 )
			    {
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							g_strconcat ( COLON(_("Transfer")),
								      gsb_data_account_get_name (gsb_data_transaction_get_account_number_transfer (transaction_number)),
								      NULL ));
			    }
			    else
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							_("Transfer") );
			}
			else
			{
			    /* c'est des catégories normales */

			    char_tmp = gsb_data_category_get_name ( gsb_data_transaction_get_category_number (transaction_number),
								    gsb_data_transaction_get_sub_category_number (transaction_number),
								    NULL );
			    if ( char_tmp )
			    {
				entree_prend_focus ( widget, NULL, NULL );
				gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
							char_tmp );
			    }
			}
		    }
		    break;

		case TRANSACTION_FORM_BUDGET:

		    char_tmp = gsb_data_budget_get_name ( gsb_data_transaction_get_budgetary_number (transaction_number),
							  gsb_data_transaction_get_sub_budgetary_number (transaction_number),
							  NULL );
		    if ( char_tmp )
		    {
			entree_prend_focus ( widget, NULL, NULL );
			gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
						char_tmp );
		    }
		    break;

		case TRANSACTION_FORM_NOTES:

		    if ( gsb_data_transaction_get_notes (transaction_number))
		    {
			entree_prend_focus ( widget, NULL, NULL );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_data_transaction_get_notes (transaction_number));
		    }
		    break;

		case TRANSACTION_FORM_TYPE:

		    if ( gsb_data_transaction_get_amount (transaction_number)< 0 )
			menu = creation_menu_types ( 1, gsb_gui_navigation_get_current_account (), 0  );
		    else
			menu = creation_menu_types ( 2, gsb_gui_navigation_get_current_account (), 0  );

		    if ( menu )
		    {
			/* on met en place les types et se place sur celui correspondant à l'opé */

			gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						   menu );
			gtk_widget_show ( widget );

			place_type_formulaire ( gsb_data_transaction_get_method_of_payment_number (transaction_number),
						TRANSACTION_FORM_TYPE,
						NULL );
		    }
		    else
		    {
			gtk_widget_hide ( widget );
			gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) );
		    }

		    break;

		case TRANSACTION_FORM_DEVISE:

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						  g_slist_index ( liste_struct_devises,
								  devise_par_no ( gsb_data_transaction_get_currency_number (transaction_number))));
		    verification_bouton_change_devise ();
		    break;

		case TRANSACTION_FORM_BANK:

		    if ( gsb_data_transaction_get_bank_references (transaction_number))
		    {
			entree_prend_focus ( widget, NULL, NULL );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_data_transaction_get_bank_references (transaction_number));
		    }
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    if ( gsb_data_transaction_get_voucher (transaction_number))
		    {
			entree_prend_focus ( widget, NULL, NULL );
			gtk_entry_set_text ( GTK_ENTRY ( widget ),
					     gsb_data_transaction_get_voucher (transaction_number));
		    }
		    break;

		case TRANSACTION_FORM_CONTRA:

		    if ( gsb_data_transaction_get_transaction_number_transfer (transaction_number)
			 &&
			 gsb_data_transaction_get_account_number_transfer (transaction_number)!= -1 )
		    {
			if ( gsb_data_transaction_get_amount (transaction_number)< 0 )
			    menu = creation_menu_types ( 2, gsb_data_transaction_get_account_number_transfer (transaction_number), 0  );
			else
			    menu = creation_menu_types ( 1, gsb_data_transaction_get_account_number_transfer (transaction_number), 0  );

			if ( menu )
			{
			    gpointer contra_transaction;

			    /* on met en place les types et se place sur celui correspondant à l'opé */

			    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget ),
						       menu );
			    gtk_widget_show ( widget );

			    contra_transaction = gsb_data_transaction_get_pointer_to_transaction (gsb_data_transaction_get_transaction_number_transfer (transaction_number));

			    if ( contra_transaction )
				place_type_formulaire ( gsb_data_transaction_get_method_of_payment_number ( gsb_data_transaction_get_transaction_number (contra_transaction )),
							TRANSACTION_FORM_CONTRA,
							NULL );
			}
			else
			    gtk_widget_hide ( widget );
		    }
		    break;
	    }
	}
    return TRUE;
}
/******************************************************************************/




/******************************************************************************/
/* cette fonction est appelée lorsque l'option menu des devises du formulaire a changé */
/* elle vérifie si la devise choisie à un taux de change fixe avec la devise du compte */
/* sinon elle affiche le bouton de change */
/******************************************************************************/
void verification_bouton_change_devise ( void )
{
    struct struct_devise *devise_compte;
    struct struct_devise *devise;

    /*   si la devise n'est pas celle du compte ni l'euro si le compte va y passer, affiche le bouton change */

    devise_compte = devise_par_no ( gsb_data_account_get_currency (gsb_gui_navigation_get_current_account ()) );
    devise = g_object_get_data ( G_OBJECT ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_DEVISE)) -> menu_item ),
				 "adr_devise" );

    if ( !( devise -> no_devise == gsb_data_account_get_currency (gsb_gui_navigation_get_current_account ())
	    ||
	    ( devise_compte -> passage_euro && !strcmp ( devise -> nom_devise, _("Euro") ))
	    ||
	    ( !strcmp ( devise_compte -> nom_devise, _("Euro") ) && devise -> passage_euro )))
	gtk_widget_show ( gsb_form_get_element_widget (TRANSACTION_FORM_CHANGE) );
    else
	gtk_widget_hide ( gsb_form_get_element_widget (TRANSACTION_FORM_CHANGE) );
}
/******************************************************************************/


/**
 * Look for the last transaction with the same party. Begin in the current account,
 * and continue in other accounts if necessary.
 *
 * \param no_party the party we are looking for
 * \param no_new_transaction if the transaction found is that transaction, we don't
 * \param account_number the account we want to find first the party
 * keep it
 *
 * \return the number of the transaction found, or 0 
 * */
gint gsb_transactions_look_for_last_party ( gint no_party,
					    gint no_new_transaction,
					    gint account_number )
{
    GSList *list_tmp_transactions;
    gint last_transaction_with_party_in_account = 0;
    gint last_transaction_with_party_not_in_account = 0;

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_party_number (transaction_number_tmp) == no_party
	     &&
	     transaction_number_tmp != no_new_transaction
	     &&
	     !gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp))
	{
	    /* we are on a transaction with the same party, so we keep it */

	    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number)
		last_transaction_with_party_in_account = transaction_number_tmp;
	    else
		last_transaction_with_party_not_in_account = transaction_number_tmp;
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }

    if ( last_transaction_with_party_in_account )
    {
	return last_transaction_with_party_in_account;
    }

    if ( etat.limit_completion_to_current_account )
    {
	return 0;
    }

    return last_transaction_with_party_not_in_account;
}
/******************************************************************************/



/**
 * Get a breakdown of transactions and ask if we want to clone the daughters
 * do the copy if needed, set for the daugthers the number of the new transaction
 * 
 * \param new_transaction_number the number of the new mother of the cloned transaction
 * \param no_last_breakdown the no of last breakdown mother
 * \param no_account the account of the last breakdown mother (important if it's not the same of the new_transaction)
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_recover_breakdowns_of_transaction ( gint new_transaction_number,
								   gint no_last_breakdown,
								   gint no_account )
{
    gint result;
    GSList *list_tmp_transactions;

    /* xxx check for win */
    result = question_conditional_yes_no ( "recover-breakdown" );

    if ( !result )
	return FALSE;

    /* go around the transactions list to get the daughters of the last breakdown */

    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

	if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == no_account
	     &&
	     gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == no_last_breakdown)
	{
	    gsb_data_transaction_set_mother_transaction_number ( gsb_transactions_list_clone_transaction (transaction_number_tmp),
								 new_transaction_number);
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }
    return FALSE;
}
/******************************************************************************/


/******************************************************************************/
/* place l'option_menu des types de paiement sur le no de type donné en argument */
/* s'il ne le trouve pas met le type par défaut du compte */
/* \param no_type le no de type voulu */
/* \param no_option_menu TRANSACTION_FORM_TYPE ou TRANSACTION_FORM_CONTRA */
/* \param contenu le contenu du type à afficher si nécessaire */
/******************************************************************************/
void place_type_formulaire ( gint no_type,
			     gint no_option_menu,
			     gchar *contenu )
{
    gint place_type;
    struct struct_type_ope *type;
    GtkWidget *option_menu;
    GtkWidget *entree_cheque;

    option_menu = gsb_form_get_element_widget (no_option_menu);

    /* recherche le type de l'opé */

    place_type = cherche_no_menu_type ( no_type );

    /* si aucun type n'a été trouvé, on cherche le défaut */

    if ( place_type == -1 )
    {
	gint no_compte;
	gint signe;

	no_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu ),
							    "no_compte"));
	signe = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu ),
							    "signe_menu"));

	if ( signe == 1 )
	    place_type = cherche_no_menu_type ( gsb_data_account_get_default_debit (no_compte) );
	else
	    place_type = cherche_no_menu_type ( gsb_data_account_get_default_credit (no_compte) );

	/* si le type par défaut n'est pas trouvé, on met 0 */

	if ( place_type == -1 )
	    place_type = 0;
    }

    /*       à ce niveau, place type est mis */

    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu ),
				  place_type );

    /*     si on est sur le contre type, on vire ici car on ne donne pas la possibilité de rentrer un commentaire */

    if ( no_option_menu == TRANSACTION_FORM_CONTRA )
	return;

    /* récupère l'adr du type pour mettre un n° auto si nécessaire */

    type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( option_menu ) -> menu_item ),
				 "adr_type" );

    if ( type -> affiche_entree )
    {
	entree_cheque = gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE);

	if ( contenu )
	{
	    entree_prend_focus ( entree_cheque, NULL, NULL );
	    gtk_entry_set_text ( GTK_ENTRY ( entree_cheque ),
				 contenu );
	}
	else
	    if ( type -> numerotation_auto )
	    {
		entree_prend_focus ( entree_cheque, NULL, NULL );
		gtk_entry_set_text ( GTK_ENTRY ( entree_cheque ),
				     automatic_numbering_get_new_number ( type ) );
	    }

	gtk_widget_show ( entree_cheque );
    }
}
/******************************************************************************/



/** called when the user finishes the edition of a transaction, 
 * add/modify the transaction shown in the form
 * \param none
 * \return FALSE
 * */
gboolean gsb_form_finish_edition ( void )
{
    gint transaction_number;
    gint new_transaction;
    GSList *list_nb_parties;
    GSList *list_tmp;
    gint current_account;

    /* get the number of the transaction, stored in the form (0 if new) */

    transaction_number = GPOINTER_TO_INT (gtk_object_get_data ( GTK_OBJECT ( formulaire ),
								"transaction_number_in_form" ));
    current_account = gsb_gui_navigation_get_current_account ();

    /* a new transaction is
     * either transaction_number is 0 (normal transaction)
     * either transaction_number is -2 (new breakdown daughter)
     * */

    if ( transaction_number &&
	 transaction_number >= -1 )
	new_transaction = 0;
    else
	new_transaction = 1;

    /* the current widget has to lose the focus to make all the changes if necessary */

    gtk_widget_grab_focus ( gsb_transactions_list_get_tree_view()  );
    
    /* check if the datas are ok */

    if ( !gsb_form_validate_form_transaction ( transaction_number ))
	return FALSE;

    /* if the party is a report, we make as transactions as the number of parties in the
     * report. So we create a list with the party's numbers or -1 if it's a normal
     * party */

    list_nb_parties = gsb_form_get_parties_list_from_report ();

    /* now we go throw the list */

    list_tmp = list_nb_parties;

    while ( list_tmp )
    {
	if ( list_tmp -> data == GINT_TO_POINTER ( -1 ) )
	    /* it's a normal party, we set the list_tmp to NULL */
	    list_tmp = NULL;
	else
	{
	    /* it's a report, so each time we come here we set the parti's combofix to the
	     * party of the report */

	    if ( !list_tmp -> data )
	    {
		dialogue_error ( _("No payee selected for this report."));
		return FALSE;
	    }
	    else
	    {
		gtk_combofix_set_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY) ),
					gsb_data_payee_get_name ( GPOINTER_TO_INT (list_tmp -> data), TRUE ));

		/* if it's not the first party and the method of payment has to change its number (cheque),
		 * we increase the number. as we are in a party's list, it's always a new transactio, 
		 * so we know that it's not the first if transaction_number is not 0 */

		if ( transaction_number )
		{
		    struct struct_type_ope *type;

		    type = type_ope_par_no ( gsb_data_transaction_get_method_of_payment_number (transaction_number),
					     current_account);

		    if ( type
			 &&
			 type -> affiche_entree
			 &&
			 type -> numerotation_auto
			 &&
			 gsb_data_form_check_for_value ( TRANSACTION_FORM_CHEQUE ))
			gtk_entry_set_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) ),
					     automatic_numbering_get_new_number ( type ));
		}
		list_tmp = list_tmp -> next;
	    }
	}

	if ( new_transaction )
	{
	    /* it's a new transaction, we create it, and put the mother if necessary */

	    gint mother_transaction;

	    /* if transaction exists already, it's that we are on a white daughter of a breakdown,
	     * so we keep the no of the mother */

	    if ( transaction_number )
		mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);
	    else
		mother_transaction = 0;
	    
	    transaction_number = gsb_data_transaction_new_transaction (current_account);

	    gsb_data_transaction_set_mother_transaction_number ( transaction_number,
								 mother_transaction);
	}

	/* take the datas in the form, except the category */

	gsb_form_take_datas_from_form ( transaction_number );

	/* take the category and do the stuff with that (contra-transaction...) */

	gsb_form_get_categories ( transaction_number,
				  new_transaction );

	if ( new_transaction )
	    gsb_transactions_list_append_new_transaction (transaction_number);
	else
	    gsb_transactions_list_update_transaction ( gsb_data_transaction_get_pointer_to_transaction (transaction_number));
    }

    /* on libère la liste des no tiers */

    g_slist_free ( list_nb_parties );

    /* if it's a reconciliation and we modify a transaction, check
     * the amount of marked transactions */

    if ( etat.equilibrage
	 &&
	 !new_transaction )
	calcule_total_pointe_compte ( gsb_gui_navigation_get_current_account () );

    /* if it was a new transaction, do the stuff to do another new transaction */

    if ( new_transaction )
    {
	GtkWidget *date_entry;

	/* it was a new transaction, we save the last date entry */

	date_entry = gsb_form_get_element_widget (TRANSACTION_FORM_DATE);
	last_date = my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( date_entry )));

	gsb_transactions_list_edit_current_transaction ();
    }
    else
    {
	formulaire_a_zero ();
	if ( !etat.formulaire_toujours_affiche )
	    gsb_form_hide ();
    }

    /* update the combofix's lists */

    if ( mise_a_jour_combofix_tiers_necessaire )
	mise_a_jour_combofix_tiers ();
    if ( mise_a_jour_combofix_categ_necessaire )
	mise_a_jour_combofix_categ ();
    if ( mise_a_jour_combofix_imputation_necessaire )
	mise_a_jour_combofix_imputation ();

    /* show the warnings */

    affiche_dialogue_soldes_minimaux ();

    update_transaction_in_trees (transaction_number);

    modification_fichier ( TRUE );
    return FALSE;
}
/******************************************************************************/



/** return a list of numbers of parties if the party in the form is a 
 * report
 * \param none
 * \return a g_slist, with -1 if it's a normal party or a list of parties if it's a report
 * */
GSList *gsb_form_get_parties_list_from_report ( void )
{
    GSList *list_nb_parties;

    list_nb_parties = NULL;

    /*     check that the party's form exist, else, append -1 and go away */

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_PARTY ))
    {
	if ( strncmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY) ))),
		       COLON(_("Report")),
		       7 ))
	    /* the party is not a report, set -1 and go away */
	    list_nb_parties = g_slist_append (list_nb_parties,
					     GINT_TO_POINTER ( -1 ));
	else
	{
	    /* c'est bien un état */
	    /* on commence par retrouver le nom de l'état */
	    /* toutes les vérifications ont été faites précédemment */

	    gchar **tab_char;
	    gint report_number = 0;
	    GSList *list_transactions;
	    GSList *list_tmp;

	    tab_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY) ))),
					":",
					2 );

	    if ( tab_char[1] )
	    {
		tab_char[1] = g_strstrip ( tab_char[1] );
		list_tmp = gsb_data_report_get_report_list ();

		while ( list_tmp )
		{
		    report_number = gsb_data_report_get_report_number (list_tmp -> data);

		    if ( !strcmp ( gsb_data_report_get_report_name (report_number),
				   tab_char[1] ))
			list_tmp = NULL;
		    else
			list_tmp = list_tmp -> next;
		}

		g_strfreev ( tab_char );

		/* à ce niveau, report_numbe contient le no le la struct de l'état choisi */

		list_transactions = recupere_opes_etat ( report_number);

		list_tmp = list_transactions;

		while ( list_tmp )
		{
		    gpointer transaction;

		    transaction = list_tmp -> data;

		    if ( !g_slist_find ( list_nb_parties,
					 GINT_TO_POINTER ( gsb_data_transaction_get_party_number ( gsb_data_transaction_get_transaction_number (transaction )))))
			list_nb_parties = g_slist_append ( list_nb_parties,
							  GINT_TO_POINTER ( gsb_data_transaction_get_party_number ( gsb_data_transaction_get_transaction_number (transaction ))));

		    list_tmp = list_tmp -> next;
		}

		g_slist_free ( list_transactions );
	    }
	}
    }
    else
	/* 	il n'y a pas de tiers, donc ce n'est pas un état */
	list_nb_parties = g_slist_append (list_nb_parties,
					 GINT_TO_POINTER ( -1 ));

    return list_nb_parties;
}


/** called to check if the transaction in the form is correct
 * \param transaction_number the new transaction number
 * \return TRUE or FALSE
 * */
gboolean gsb_form_validate_form_transaction ( gint transaction_number )
{
    gchar **tab_char;
    GSList *list_tmp;

    /* on vérifie qu'il y a bien une date */

    if ( gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE) ) != style_entree_formulaire[ENCLAIR] )
    {
	dialogue_error ( _("You must enter a date.") );
	return (FALSE);
    }

    /* vérifie que la date est correcte */

    if ( !modifie_date ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE) ))
    {
	dialogue_error ( _("Invalid date") );
	gtk_entry_select_region ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE) ),
				  0,
				  -1);
	gtk_widget_grab_focus ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE) );
	return (FALSE);
    }


    /* vérifie que la date de valeur est correcte */

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_VALUE_DATE )
	 &&
	 gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_VALUE_DATE) ) == style_entree_formulaire[ENCLAIR]
	 &&
	 !modifie_date ( gsb_form_get_element_widget (TRANSACTION_FORM_VALUE_DATE) ) )
    {
	dialogue_error ( _("Invalid value date.") );
	gtk_entry_select_region ( GTK_ENTRY (  gsb_form_get_element_widget (TRANSACTION_FORM_VALUE_DATE) ),
				  0,
				  -1);
	gtk_widget_grab_focus ( gsb_form_get_element_widget (TRANSACTION_FORM_VALUE_DATE) );
	return (FALSE);
    }

    /* check if it's a daughter breakdown that the category is not a breakdown of transaction */

    if ( transaction_number
	 &&
	 gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY )
	 &&
	 gtk_widget_get_style ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	if ( !strcmp ( gtk_entry_get_text ( GTK_ENTRY ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY))->entry)),
		       _("Breakdown of transaction"))
	     &&
	     gsb_data_transaction_get_mother_transaction_number (transaction_number))
	{
	    dialogue_error ( _("You cannot set a daughter of a breakdown of transaction as a breakdown of transaction.") );
	    return (FALSE);
	}
    }


    /* vérification que ce n'est pas un virement sur lui-même */
    /* et que le compte de virement existe */

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY )
	 &&
	 gtk_widget_get_style ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) ) -> entry ) == style_entree_formulaire[ENCLAIR] )
    {
	tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) )),
				    ":",
				    2 );

	tab_char[0] = g_strstrip ( tab_char[0] );

	if ( tab_char[1] )
	    tab_char[1] = g_strstrip ( tab_char[1] );


	/* Si c'est un virement, on fait les vérifications */
	
	if ( !strcmp ( tab_char[0], _("Transfer") ) )
	{
	    /* S'il n'y a rien après "Transfer", alors : */
	    
	    if ( !tab_char[1] ||
		 !strlen ( tab_char[1] ) )
	    {
		dialogue_error ( _("There is no associated account for this transfer.") );
		return (FALSE);
	    }
	    
	    /* si c'est un virement vers un compte supprimé, laisse passer */

	    if ( strcmp ( tab_char[1],
			  _("Deleted account") ) )
	    {
		/* recherche le no de compte du virement */

		gint account_transfer;

		account_transfer = gsb_data_account_get_no_account_by_name ( tab_char[1] );

		if ( account_transfer == -1 )
		{
		    dialogue_warning ( _("Associated account of this transfer is invalid.") );
		    return (FALSE);
		}

		if ( account_transfer == gsb_gui_navigation_get_current_account () )
		{
		    dialogue_error ( _("Can't issue a transfer its own account.") );
		    return (FALSE);
		}

		/* 		    vérifie si le compte n'est pas clos */

		if ( gsb_data_account_get_closed_account (account_transfer) )
		{
		    dialogue_error ( _("Can't issue a transfer on a closed account." ));
		    return ( FALSE );
		}
	    }
	}
	g_strfreev ( tab_char );
    }

    /* pour les types qui sont à incrémentation automatique ( surtout les chèques ) */
    /* on fait le tour des operations pour voir si le no n'a pas déjà été utilisé */
    /* si transaction n'est pas nul, c'est une modif donc on ne fait pas ce test */

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_CHEQUE )
	 &&
	 GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) ))
    {
	struct struct_type_ope *type;

	type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ) -> menu_item ),
				     "adr_type" );

	if ( type -> numerotation_auto )
	{
	    gpointer operation_tmp;

	    /* vérifie s'il y a quelque chose */

	    if ( gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) ) == style_entree_formulaire[ENGRIS] )
	    {
		if ( question ( _("Selected method of payment has an automatic incremental number\nbut doesn't contain any number.\nContinue anyway?") ) )
		    goto sort_test_cheques;
		else
		    return (FALSE);
	    }

	    /* vérifie si le no de chèque n'est pas déjà utilisé */

	    operation_tmp = operation_par_cheque ( utils_str_atoi ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) )))),
						   gsb_gui_navigation_get_current_account () );

	    /* si on a trouvé le même no de chèque, si c'est une nouvelle opé, c'est pas normal, */
	    /* si c'est une modif d'opé, c'est normal que si c'est cette opé qu'on a trouvé */

	    if ( operation_tmp
		 &&
		 (!transaction_number
		  ||
		  gsb_data_transaction_get_transaction_number (operation_tmp) != transaction_number))
	    {
		if ( question ( _("Warning: this cheque number is already used.\nContinue anyway?") ))
		    goto sort_test_cheques;
		else
		    return (FALSE);
	    }
	}
    }

sort_test_cheques :

    /* on vérifie si le tiers est un état, que c'est une nouvelle opération */

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_PARTY )
	 &&
	 !strncmp ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY) ))),
		    COLON(_("Report")),
		    7 ))
    {
	gint trouve;

	/* on vérifie d'abord si c'est une modif d'opé */

	if ( transaction_number )
	{
	    dialogue_error ( _("A transaction with a multiple payee must be a new one.") );
	    return (FALSE);
	}

	/* on vérifie maintenant si l'état existe */

	tab_char = g_strsplit ( g_strstrip ( gtk_combofix_get_text ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_PARTY) ))),
				    ":",
				    2 );

	if ( tab_char[1] )
	{
	    tab_char[1] = g_strstrip ( tab_char[1] );
	    list_tmp = gsb_data_report_get_report_list ();
	    trouve = 0;

	    while ( list_tmp )
	    {
		gint report_number;

		report_number = gsb_data_report_get_report_number (list_tmp -> data);

		if ( !strcmp ( gsb_data_report_get_report_name (report_number),
			       tab_char[1] ))
		{
		    trouve = 1;
		    list_tmp = NULL;
		}
		else
		    list_tmp = list_tmp -> next;
	    }

	    g_strfreev ( tab_char );

	    if ( !trouve )
	    {
		dialogue_error ( _("Invalid multiple payee.") );
		return (FALSE);
	    }
	}
	else
	{
	    dialogue_error  ( _("The word \"Report\" is reserved. Please use another one."));
		return FALSE;
	}
    }

    /* Check if there is no budgetary line (see #208) */
    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_BUDGET )
	 &&
	 gtk_widget_get_style ( GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_BUDGET) ) -> entry ) == style_entree_formulaire[ENGRIS] )
    {
	dialog_message ( "no-budgetary-line" );
	return FALSE;
    }

    return ( TRUE );
}
/******************************************************************************/

/**
 * take the datas in the form and set them in the transaction in param
 * 
 * \param transaction_number the transaction to modify
 * 
 * \return
 * */
void gsb_form_take_datas_from_form ( gint transaction_number )
{
    gchar **tab_char;
    gint row, column;
    gint account_number;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /*     on fait le tour du formulaire en ne récupérant que ce qui est nécessaire */

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column <  gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    gint value;
	    GtkWidget *widget;

	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );

	    widget =  gsb_form_get_element_widget (value);

	    switch (value)
	    {
		case TRANSACTION_FORM_DATE:
		    gsb_data_transaction_set_date ( transaction_number,
						    gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY ( widget ) ) ) );

		    break;

		case TRANSACTION_FORM_VALUE_DATE:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
		    {
			gsb_data_transaction_set_value_date ( transaction_number,
							      gsb_parse_date_string ( gtk_entry_get_text ( GTK_ENTRY ( widget ) ) ) );
		    }
		    else
			gsb_data_transaction_set_value_date ( transaction_number,
							      NULL );
		    break;

		case TRANSACTION_FORM_EXERCICE:

		    /* si l'exo est à -1, c'est que c'est sur non affiché */
		    /* soit c'est une modif d'opé et on touche pas à l'exo */
		    /* soit c'est une nouvelle opé et on met l'exo à 0 */

		    if ( gsb_financial_year_get_number_from_option_menu (widget) == -1 )
		    {
			/* FIXME : ici, devrait tester si c'est une nouvelle opé... */
			if ( !transaction_number)
			    gsb_data_transaction_set_financial_year_number ( transaction_number,
									     0 );
		    }
		    else
			gsb_data_transaction_set_financial_year_number ( transaction_number,
									 gsb_financial_year_get_number_from_option_menu (widget));

		    break;

		case TRANSACTION_FORM_PARTY:

		    if ( gtk_widget_get_style ( GTK_COMBOFIX ( widget ) -> entry ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_party_number ( transaction_number,
								gsb_data_payee_get_number_by_name ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget )),
											       TRUE ));
		    else
			gsb_data_transaction_set_party_number ( transaction_number,
								0 );

		    break;

		case TRANSACTION_FORM_DEBIT:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_amount ( transaction_number,
							  -calcule_total_entree ( widget ));
		    break;

		case TRANSACTION_FORM_CREDIT:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_amount ( transaction_number,
							  calcule_total_entree ( widget ));
		    break;

		case TRANSACTION_FORM_BUDGET:

		    tab_char = g_strsplit ( gtk_combofix_get_text ( GTK_COMBOFIX ( widget )),
					    ":",
					    2 );

		    tab_char[0] = g_strstrip ( tab_char[0] );

		    if ( tab_char[1] )
			tab_char[1] = g_strstrip ( tab_char[1] );

		    if ( strlen ( tab_char[0] ) )
		    {
			gint budget_number;

			budget_number = gsb_data_budget_get_number_by_name ( g_strstrip (tab_char[0]),
									     TRUE,
									     gsb_data_transaction_get_amount (transaction_number)<0 );
			gsb_data_transaction_set_budgetary_number ( transaction_number,
								    budget_number );
			gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
									gsb_data_budget_get_sub_budget_number_by_name ( budget_number,
															tab_char[1],
															TRUE ));
		    }
		    g_strfreev ( tab_char );
		    break;

		case TRANSACTION_FORM_NOTES:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_notes ( transaction_number,
							 g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget )))));
		    else
			gsb_data_transaction_set_notes ( transaction_number,
							 NULL );
		    break;

		case TRANSACTION_FORM_TYPE:

		    if ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ))
		    {
			gsb_data_transaction_set_method_of_payment_number ( transaction_number,
									    gsb_payment_method_get_payment_number_from_option_menu (widget));

			if ( GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) )
			     &&
			     gtk_widget_get_style ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) ) == style_entree_formulaire[ENCLAIR] )
			{
			    struct struct_type_ope *type;

			    type = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( widget ) -> menu_item ),
							 "adr_type" );

			    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
										 g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( gsb_form_get_element_widget (TRANSACTION_FORM_CHEQUE) )))));

			    if ( type -> numerotation_auto )
				type -> no_en_cours = ( utils_str_atoi ( gsb_data_transaction_get_method_of_payment_content ( transaction_number)));
			}
			else
			    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
										  NULL);
		    }
		    else
		    {
			gsb_data_transaction_set_method_of_payment_number ( transaction_number,
									    0 );
			gsb_data_transaction_set_method_of_payment_content ( transaction_number,
									     NULL);
		    }
		    break;

		case TRANSACTION_FORM_DEVISE:

		    /* récupération de la devise */

		    gsb_data_transaction_set_currency_number ( transaction_number,
							       gsb_currency_get_option_menu_currency (widget));

		    gsb_currency_check_for_change ( transaction_number );

		    break;

		case TRANSACTION_FORM_BANK:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_bank_references ( transaction_number,
								   g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget )))));
		    else
			gsb_data_transaction_set_bank_references ( transaction_number,
								   NULL);
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    if ( gtk_widget_get_style ( widget ) == style_entree_formulaire[ENCLAIR] )
			gsb_data_transaction_set_voucher ( transaction_number,
							   g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( widget )))));
		    else
			gsb_data_transaction_set_voucher ( transaction_number,
							   NULL);

		    break;
	    }
	}
}
/******************************************************************************/


/** deal with the category in the form, append it in the transaction given in param
 * create the oter transaction if it's a transfer...
 * \param transaction_number the transaction which work with
 * \param new_transaction 1 if it's a new_transaction
 * \return FALSE
 * */
gboolean gsb_form_get_categories ( gint transaction_number,
				   gint new_transaction )
{
    gchar *char_ptr;
    gchar **tab_char;
    gint contra_transaction_number;
    GtkWidget *category_entry;

    if ( !gsb_data_form_check_for_value ( TRANSACTION_FORM_CATEGORY ))
	return FALSE;

    category_entry = GTK_COMBOFIX ( gsb_form_get_element_widget (TRANSACTION_FORM_CATEGORY) ) -> entry;

    if ( gtk_widget_get_style ( category_entry ) == style_entree_formulaire[ENCLAIR] )
    {
	char_ptr = g_strstrip ( my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( category_entry ))));

	if ( !strcmp ( char_ptr,
		       _("Breakdown of transaction") ))
	{
	    /* it's a breakdown of transaction */
	    /* if it was a transfer, we delete the contra-transaction */

	    if ( !new_transaction
		 &&
		 ( contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number)))
	    {
		gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
								       0);
		gsb_transactions_list_delete_transaction (contra_transaction_number );

		gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
								       0);
		gsb_data_transaction_set_account_number_transfer ( transaction_number,
								   0);
	    }

	    gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
								1 );
	    gsb_data_transaction_set_category_number ( transaction_number,
						       0 );
	    gsb_data_transaction_set_sub_category_number ( transaction_number,
							   0 );

	    /*we will check here if there is another breakdown with the same party,
	     * if yes, we propose to copy the daughters transactions */

	    if ( new_transaction )
	    {
		gint breakdown_transaction_number;

		breakdown_transaction_number = gsb_transactions_look_for_last_party ( gsb_data_transaction_get_party_number (transaction_number),
										      transaction_number,
										      gsb_data_transaction_get_account_number(transaction_number));

		gsb_transactions_list_recover_breakdowns_of_transaction ( transaction_number,
									  breakdown_transaction_number,
									  gsb_data_transaction_get_account_number (breakdown_transaction_number));
	    }
	}
	else
	{
	    /* it's not a breakdown of transaction, if it was, we delete the
	     * transaction's daughters */

	    if ( !new_transaction
		 &&
		 gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
	    {
		GSList *list_tmp_transactions;
		list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

		while ( list_tmp_transactions )
		{
		    gint transaction_number_tmp;
		    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		    if ( gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_gui_navigation_get_current_account ()
			 &&
			 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number )
		    {
			list_tmp_transactions = list_tmp_transactions -> next;
			gsb_transactions_list_delete_transaction (transaction_number_tmp);
		    }
		    else
			list_tmp_transactions = list_tmp_transactions -> next;
		}
		gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
								    0 );
	    }

	    /* now, check if it's a transfer or a normal category */

	    tab_char = g_strsplit ( char_ptr,
					":",
					2 );

	    tab_char[0] = g_strstrip ( tab_char[0] );

	    if ( tab_char[1] )
		tab_char[1] = g_strstrip ( tab_char[1] );

	    if ( strlen ( tab_char[0] ) )
	    {
		if ( !strcmp ( tab_char[0],
			       _("Transfer") )
		     && tab_char[1]
		     && strlen ( tab_char[1] ) )
		{
		    /* it's a transfert */

		    gsb_data_transaction_set_category_number ( transaction_number,
							       0 );
		    gsb_data_transaction_set_sub_category_number ( transaction_number,
								   0 );

		    /* sépare entre virement vers un compte et virement vers un compte supprimé */

		    if ( strcmp ( tab_char[1],
				  _("Deleted account") ) )
		    {
			/* it's a real transfert */
			gsb_form_validate_transfer ( transaction_number,
						     new_transaction,
						     tab_char[1] );
		    }
		    else
		    {
			/* it's a transfert to a deleted account */
			gsb_data_transaction_set_account_number_transfer ( transaction_number,
									   -1);
			gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
									       1);
		    }
		}
		else
		{
		    /* c'est une catég normale, si c'est une modif d'opé, vérifier si ce n'était pas un virement */

		    gint category_number;

		    if ( !new_transaction
			 &&
			 (contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number)))
		    {
			/* c'était un virement, et ce ne l'est plus, donc on efface l'opé en relation */

			gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
									       0);
			gsb_transactions_list_delete_transaction (contra_transaction_number );

			gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
									       0);
			gsb_data_transaction_set_account_number_transfer ( transaction_number,
									   0);
		    }

		    category_number = gsb_data_category_get_number_by_name ( tab_char[0],
									     TRUE,
									     gsb_data_transaction_get_amount (transaction_number)<0 );
		    gsb_data_transaction_set_category_number ( transaction_number,
							       category_number );
		    gsb_data_transaction_set_sub_category_number ( transaction_number,
								   gsb_data_category_get_sub_category_number_by_name ( category_number,
														       tab_char[1],
														       TRUE ));
		}
	    }
	    g_strfreev ( tab_char );
	}
    }
    return FALSE;
}


/** validate a transfert from a form :
 * - create the contra-transaction
 * - delete the last contra-transaction if it's a modification
 * - append the contra-transaction to the tree view or update the tree_view
 * \param transaction_number the new transaction or the modify transaction
 * \param new_transaction TRUE if it's a new transaction
 * \param name_transfer_account the name of the account we want to create the contra-transaction
 * \return the number of the contra-transaction
 * */
gint gsb_form_validate_transfer ( gint transaction_number,
				  gint new_transaction,
				  gchar *name_transfer_account )
{
    gint contra_transaction_number;
    gint account_transfer;

    account_transfer = gsb_data_account_get_no_account_by_name ( name_transfer_account );

    g_return_val_if_fail ( account_transfer, -1 );

    /* either it's a new transfer or a change of a non-transfer transaction
     * either it was already a transfer, in that case, if we change the target account,
     * we delete the contra-transaction and it's the same as a new transfer */

    if ( !new_transaction )
    {
	/* it's a modification of a transaction */

	if ((contra_transaction_number = gsb_data_transaction_get_transaction_number_transfer (transaction_number)))
	{
	    /* the transaction is a transfer */

	    if ( gsb_data_transaction_get_account_number_transfer (transaction_number) != account_transfer )
	    {
		/* it was a transfer and the user changed the target account so we delete the last contra transaction
		 * contra_transaction_transfer has just been set */

		gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
								       0);
		gsb_transactions_list_delete_transaction (contra_transaction_number);
		new_transaction = 1;
	    }
	}
	else
	{
	    /* the transaction was not a transfer, so it's the same as a new transaction, to do the contra-transaction */

	    new_transaction = 1;
	}
    }

    /* so, now, it's either a new transfer and new_transaction is TRUE,
     * either a transfer without changing the target account and in that case, contra_transaction_number is
     * already set */

    if ( new_transaction )
	contra_transaction_number = gsb_data_transaction_new_transaction (account_transfer);

    gsb_data_transaction_copy_transaction ( transaction_number,
					    contra_transaction_number );

    /* we have to change the amount by the opposite */

    gsb_data_transaction_set_amount (contra_transaction_number,
				      -gsb_data_transaction_get_amount (transaction_number));

    /* we have to check the change */

    gsb_currency_check_for_change ( contra_transaction_number );

    /* set the method of payment
     * FIXME : for the scheduled transactions, we arrive here but there is no button for that,
     * so for now, TRANSACTION_FORM_CONTRA won't be visible so he just copy the method of the scheduled transaction */

    if ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) &&
	 GTK_IS_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ) &&
	 gsb_data_form_check_for_value ( TRANSACTION_FORM_CONTRA ) &&
	 GTK_WIDGET_VISIBLE ( gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA) ))
    {
	gsb_data_transaction_set_method_of_payment_number ( contra_transaction_number,
							    gsb_payment_method_get_payment_number_from_option_menu (gsb_form_get_element_widget (TRANSACTION_FORM_CONTRA)));
    }

    /* set the link between the transactions */

    gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
							   contra_transaction_number);
    gsb_data_transaction_set_account_number_transfer ( transaction_number,
						       gsb_data_transaction_get_account_number (contra_transaction_number));
    gsb_data_transaction_set_transaction_number_transfer ( contra_transaction_number,
							   transaction_number);
    gsb_data_transaction_set_account_number_transfer ( contra_transaction_number,
						       gsb_data_transaction_get_account_number (transaction_number));

    /* show the contra_transaction */

    if ( new_transaction )
	gsb_transactions_list_append_new_transaction (contra_transaction_number);
    else
	gsb_transactions_list_update_transaction ( gsb_data_transaction_get_pointer_to_transaction ( contra_transaction_number) );

    return contra_transaction_number;
}



/** 
 * append a new transaction in the tree_view
 * if the transaction is a breakdown, append a white line and open
 * the breakdown to see the daughters
 *
 * \param transaction_number
 *
 * \return FALSE
 * */
gboolean gsb_transactions_list_append_new_transaction ( gint transaction_number )
{
    GtkTreeStore *store;
    
    devel_debug ( g_strdup_printf ("gsb_transactions_list_append_new_transaction %d",
				   transaction_number ));

    store = gsb_transactions_list_get_store ();

    if ( !store)
	return FALSE;

    /* append the transaction to the tree view */

    gsb_transactions_list_append_transaction ( transaction_number,
					       store);

    /* if the transaction is a breakdown mother, we happen a white line,
     * which is a normal transaction but with nothing and with the breakdown
     * relation to the last transaction */

    if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number))
    {
	gint white_line_number;
	
	white_line_number = gsb_transactions_list_append_white_line ( transaction_number,
								      store);

	/* we select the white line of that breakdown */

	gsb_transactions_list_set_current_transaction (white_line_number);
    }	

    gsb_transactions_list_set_visibles_rows_on_transaction (transaction_number);
    gsb_transactions_list_set_background_color ( gsb_data_transaction_get_account_number (transaction_number));
    gsb_transactions_list_set_transactions_balances ( gsb_data_transaction_get_account_number (transaction_number));
    gsb_transactions_list_move_to_current_transaction ( gsb_data_transaction_get_account_number (transaction_number));

    /*     calcul du solde courant */

    gsb_data_account_set_current_balance ( gsb_data_transaction_get_account_number (transaction_number),
				      gsb_data_account_get_current_balance ( gsb_data_transaction_get_account_number (transaction_number))
				      +
				      gsb_data_transaction_get_adjusted_amount (transaction_number));

    /* on met à jour les labels des soldes */

    mise_a_jour_labels_soldes ();

    /* on réaffichera l'accueil */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;
    return FALSE;
}




/** update the transaction given in the tree_view
 * \param transaction transaction to update
 * \return FALSE
 * */
gboolean gsb_transactions_list_update_transaction ( gpointer transaction )
{
    gint j;
    GtkTreeStore *store;
    GtkTreeIter *iter;

    devel_debug ( g_strdup_printf ( "gsb_transactions_list_update_transaction no %d",
				    gsb_data_transaction_get_transaction_number (transaction)));

    store = gsb_transactions_list_get_store();
    iter = gsb_transactions_list_get_iter_from_transaction (gsb_data_transaction_get_transaction_number (transaction ),
							    0 );

    for ( j = 0 ; j < TRANSACTION_LIST_ROWS_NB ; j++ )
    {
	gsb_transactions_list_fill_row ( gsb_data_transaction_get_transaction_number (transaction),
					 iter,
					 store,
					 j );

	/* if it's a breakdown, there is only 1 line */

	if ( gsb_data_transaction_get_transaction_number (transaction ) != -1
	     &&
	     gsb_data_transaction_get_mother_transaction_number ( gsb_data_transaction_get_transaction_number (transaction )))
	    j = TRANSACTION_LIST_ROWS_NB;

	gtk_tree_model_iter_next ( GTK_TREE_MODEL (store),
				   iter );
    }

    gtk_tree_iter_free ( iter );

    gsb_transactions_list_set_transactions_balances ( gsb_data_transaction_get_account_number (gsb_data_transaction_get_transaction_number (transaction)));
    /*     calcul du solde courant */

    gsb_data_account_set_current_balance ( gsb_data_transaction_get_account_number (gsb_data_transaction_get_transaction_number (transaction)),
				      gsb_data_account_get_current_balance ( gsb_data_transaction_get_account_number (gsb_data_transaction_get_transaction_number (transaction)))
				      +
				      gsb_data_transaction_get_adjusted_amount ( gsb_data_transaction_get_transaction_number (transaction)));

    /* on met à jour les labels des soldes */

    mise_a_jour_labels_soldes ();

    /* on réaffichera l'accueil */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    return FALSE;
}


/******************************************************************************/
/* efface le contenu du formulaire                                            */
/******************************************************************************/
void formulaire_a_zero (void)
{
    gint row, column;
    gint account_number;

    account_number = gsb_gui_navigation_get_current_account ();

    /*     on fait le tour du formulaire en ne récupérant que ce qui est nécessaire */

    for ( row=0 ; row < gsb_data_form_get_nb_rows (account_number) ; row++ )
	for ( column=0 ; column <  gsb_data_form_get_nb_columns (account_number) ; column++ )
	{
	    GtkWidget *widget;
	    gint value;

	    value = gsb_data_form_get_value ( account_number,
					      column,
					      row );

	    widget =  gsb_form_get_element_widget (value);

	    switch (value)
	    {
		case TRANSACTION_FORM_DATE:

		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Date") );
		    break;

		case TRANSACTION_FORM_VALUE_DATE:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Value date") );
		    break;

		case TRANSACTION_FORM_EXERCICE:

		    gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
					       FALSE );
		    break;

		case TRANSACTION_FORM_PARTY:

		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					   style_entree_formulaire[ENGRIS] );
		    gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					    _("Payee") );
		    break;

		case TRANSACTION_FORM_DEBIT:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Debit") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;

		case TRANSACTION_FORM_CREDIT:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Credit") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;

		case TRANSACTION_FORM_CATEGORY:

		    gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					   style_entree_formulaire[ENGRIS] );
		    gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					    _("Categories : Sub-categories") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;

		case TRANSACTION_FORM_FREE:
		    break;

		case TRANSACTION_FORM_BUDGET:

		    gtk_widget_set_style ( GTK_COMBOFIX ( widget ) -> entry,
					   style_entree_formulaire[ENGRIS] );
		    gtk_combofix_set_text ( GTK_COMBOFIX ( widget ),
					    _("Budgetary line") );
		    break;

		case TRANSACTION_FORM_NOTES:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Notes") );
		    break;

		case TRANSACTION_FORM_TYPE:

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						  cherche_no_menu_type ( gsb_data_account_get_default_debit (gsb_gui_navigation_get_current_account ()) ) );
		    gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
					       FALSE );
		    break;

		case TRANSACTION_FORM_CONTRA:

		    gtk_widget_hide ( widget );

		    break;

		case TRANSACTION_FORM_CHEQUE:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Cheque/Transfer number") );
		    break;

		case TRANSACTION_FORM_DEVISE:

		    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget ),
						  g_slist_index ( liste_struct_devises,
								  devise_par_no ( gsb_data_account_get_currency (gsb_gui_navigation_get_current_account ()) )));
		    gtk_widget_set_sensitive ( GTK_WIDGET ( widget ),
					       FALSE );
		    break;

		case TRANSACTION_FORM_CHANGE:

		    gtk_widget_hide ( widget );

		    break;

		case TRANSACTION_FORM_BANK:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Bank references") );
		    break;

		case TRANSACTION_FORM_VOUCHER:

		    gtk_widget_set_style ( widget,
					   style_entree_formulaire[ENGRIS] );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Voucher") );
		    break;

		case TRANSACTION_FORM_OP_NB:

		    gtk_label_set_text ( GTK_LABEL ( widget ),
					 "" );
		    break;

		case TRANSACTION_FORM_MODE:

		    gtk_label_set_text ( GTK_LABEL ( widget ),
					 "" );
		    break;

	    }
	}

    gtk_widget_set_sensitive ( GTK_WIDGET ( vbox_boutons_formulaire ),
			       FALSE );

    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "transaction_number_in_form",
			  NULL );
    gtk_object_set_data ( GTK_OBJECT ( formulaire ),
			  "liste_adr_ventilation",
			  NULL );

}

/******************************************************************************/
/* Fonction affiche_cache_le_formulaire                                       */
/* si le formulaire était affichÃ©, le cache et vice-versa                     */
/******************************************************************************/


void affiche_cache_le_formulaire ( void )
{
    GtkWidget * widget;

    if ( etat.formulaire_toujours_affiche )
    {
	etat.formulaire_toujours_affiche = 0;
    }
    else
    {
	etat.formulaire_toujours_affiche = 1;

	update_ecran ();

/* 	ajustement = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( gsb_transactions_list_get_tree_view() ) )); */
	
/* 	position_ligne_selectionnee = ( cherche_ligne_operation ( gsb_data_account_get_current_transaction (gsb_data_account_get_current_account ()), */
/* 								  gsb_data_account_get_current_account () ) */
/* 					+ gsb_data_account_get_nb_rows ( gsb_data_account_get_current_account () ) ) * hauteur_ligne_liste_opes; */

/* 	if ( position_ligne_selectionnee  > (ajustement->value + ajustement->page_size)) */
/* 	    gtk_adjustment_set_value ( ajustement, */
/* 				       position_ligne_selectionnee - ajustement->page_size ); */
    }

    /* FIXME : ça va changer pour le form */
    if ( etat.formulaire_toujours_affiche )
	gsb_form_show ();
    else
	gsb_form_hide ();

    block_menu_cb = TRUE;
    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show transaction form"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM (widget), etat.formulaire_toujours_affiche );
    block_menu_cb = FALSE;

}



/******************************************************************************/
/* Fonction click_sur_bouton_voir_change  */
/* permet de modifier un change établi pour une opération */
/******************************************************************************/
void click_sur_bouton_voir_change ( void )
{
    gint transaction_number;
    struct struct_devise *devise;

    gtk_widget_grab_focus ( gsb_form_get_element_widget (TRANSACTION_FORM_DATE) );

    transaction_number = GPOINTER_TO_INT (gtk_object_get_data ( GTK_OBJECT ( formulaire ),
								"transaction_number_in_form" ));

    if ( !devise_compte
	 ||
	 devise_compte -> no_devise != gsb_data_account_get_currency (gsb_gui_navigation_get_current_account ()) )
	devise_compte = devise_par_no ( gsb_data_account_get_currency (gsb_gui_navigation_get_current_account ()) );

    devise = devise_par_no ( gsb_data_transaction_get_currency_number (transaction_number));

    demande_taux_de_change ( devise_compte, devise,
			     gsb_data_transaction_get_change_between (transaction_number),
			     gsb_data_transaction_get_exchange_rate (transaction_number),
			     gsb_data_transaction_get_exchange_fees (transaction_number), 
			     TRUE );

    if ( taux_de_change[0] ||  taux_de_change[1] )
    {
	gsb_data_transaction_set_exchange_rate (transaction_number,
						 fabs (taux_de_change[0] ));
	gsb_data_transaction_set_exchange_fees (transaction_number,
						 taux_de_change[1] );

	if ( taux_de_change[0] < 0 )
	    gsb_data_transaction_set_change_between (transaction_number,
						      1 );
	else
	    gsb_data_transaction_set_change_between (transaction_number,
						      0 );
    }
}
/******************************************************************************/

/******************************************************************************/
void degrise_formulaire_operations ( void )
{

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_TYPE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_TYPE) ),
				   TRUE );

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_DEVISE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_DEVISE) ),
				   TRUE );

    if ( gsb_data_form_check_for_value ( TRANSACTION_FORM_EXERCICE ))
	gtk_widget_set_sensitive ( GTK_WIDGET ( gsb_form_get_element_widget (TRANSACTION_FORM_EXERCICE) ),
				   TRUE );

    gtk_widget_set_sensitive ( GTK_WIDGET ( vbox_boutons_formulaire ),
			       TRUE );
}
/******************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
