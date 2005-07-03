/* file gsb_file_save.c
 * used to save the gsb files */
/*     Copyright (C)	2000-2005 Cédric Auger (cedric@grisbi.org) */
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

/*START_INCLUDE*/
#include "gsb_file_save.h"
#include "dialog.h"
#include "gsb_account.h"
#include "utils_dates.h"
#include "gsb_transaction_data.h"
#include "utils_str.h"
#include "structures.h"
#include "echeancier_liste.h"
#include "operations_liste.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_j_m_a;
extern gint affichage_echeances_perso_nb_libre;
extern gchar *chemin_logo;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GSList *liste_struct_echeances;
extern gint nb_colonnes;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern gchar *nom_fichier_backup;
extern gint rapport_largeur_colonnes[TRANSACTION_LIST_COL_NB];
extern gint scheduler_col_width[NB_COLS_SCHEDULER] ;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern gchar *titre_fichier;
extern gint valeur_echelle_recherche_date_import;
/*END_EXTERN*/


/** 
 * save the grisbi file
 * we don't check anything here, all must be done before, here we just write
 * the file and set the permissions
 *
 * \param filename the name of the file
 *
 * \return TRUE : ok, FALSE : problem
 * */
gboolean gsb_file_save_save_file ( gchar *filename )
{
    gint do_chmod;
    gchar *file_content;
    gchar *first_string_to_free;
    gchar *second_string_to_free;
    gchar *third_string_to_free;
    gint i,j;
    FILE *grisbi_file;
    GSList *list_tmp;


    /* used to prepare general informations */

    gchar *transactions_view;
    gchar *transaction_column_width_ratio;
    gchar *two_lines_showed;
    gchar *tree_lines_showed;
    gchar *scheduler_column_width_ratio;



    if ( DEBUG )
	printf ( "gsb_file_save_save_file : %s\n",
		 filename );

    do_chmod = !g_file_test ( filename,
			      G_FILE_TEST_EXISTS );

    etat.en_train_de_sauvegarder = 1;

    file_content = NULL;

    /* begin the file whit xml markup */
    
    file_content = g_strdup ( "<?xml version=\"1.0\"?>\n<Grisbi>\n" );

    /* prepare stuff to save generals informations */

    /* prepare transactions_view */

    transactions_view = NULL;

    for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	for ( j=0 ; j< TRANSACTION_LIST_COL_NB ; j++ )
	    if ( transactions_view )
	    {
		transactions_view = g_strconcat ( first_string_to_free = transactions_view,
						  "-",
						  second_string_to_free = utils_str_itoa ( tab_affichage_ope[i][j] ),
						  NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		transactions_view = utils_str_itoa ( tab_affichage_ope[i][j] );

    /* prepare transaction_column_width_ratio */

     transaction_column_width_ratio = NULL;

    for ( i=0 ; i<TRANSACTION_LIST_COL_NB ; i++ )
	if ( transaction_column_width_ratio )
	{
	    transaction_column_width_ratio = g_strconcat ( first_string_to_free = transaction_column_width_ratio,
							   "-",
							   second_string_to_free = utils_str_itoa ( rapport_largeur_colonnes[i] ),
							   NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);
	}
	else
	    transaction_column_width_ratio = utils_str_itoa ( rapport_largeur_colonnes[i] );


    /* prepare two_lines_showed */

    two_lines_showed = g_strconcat ( first_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data )),
				     "-",
				     second_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data )),
				     NULL );
    g_free (first_string_to_free);
    g_free (second_string_to_free);

    /* prepare tree_lines_showed */

    tree_lines_showed = g_strconcat ( first_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data )),
				      "-",
				      second_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data )),
				      "-",
				      third_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data )),
				      NULL );
    g_free (first_string_to_free);
    g_free (second_string_to_free);
    g_free (third_string_to_free);

    /* prepare scheduler_column_width_ratio */

    scheduler_column_width_ratio = NULL;

    for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
	if ( scheduler_column_width_ratio )
	{
	    scheduler_column_width_ratio = g_strconcat ( first_string_to_free = scheduler_column_width_ratio,
							 "-",
							 second_string_to_free = utils_str_itoa ( scheduler_col_width[i] ),
							 NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);
	}
	else
	    scheduler_column_width_ratio  = utils_str_itoa ( scheduler_col_width[i] );


    /* save the general informations */

    file_content = g_strconcat ( first_string_to_free = file_content,
				 second_string_to_free = g_markup_printf_escaped ( "\t<General\n\t\tFile_version=\"%s\"\n\t\tGrisbi_version=\"%s\"\n\t\tBackup_file=\"%s\"\n\t\tFile_title=\"%s\"\n\t\tGeneral_address=\"%s\"\n\t\tSecond_general_address=\"%s\"\n\t\tParty_list_currency_number=\"%d\"\n\t\tCategory_list_currency_number=\"%d\"\n\t\tBudget_list_currency_number=\"%d\"\n\t\tScheduler_view=\"%d\"\n\t\tScheduler_custom_number=\"%d\"\n\t\tScheduler_custom_menu=\"%d\"\n\t\tImport_interval_search=\"%d\"\n\t\tUse_logo=\"%d\"\n\t\tPath_logo=\"%s\"\n\t\tRemind_display_per_account=\"%d\"\n\t\tTransactions_view=\"%s\"\n\t\tTransaction_column_width_ratio=\"%s\"\n\t\tOne_line_showed=\"%d\"\n\t\tTwo_lines_showed=\"%s\"\n\t\tThree_lines_showed=\"%s\"\n\t\tRemind_form_per_account=\"%d\"\n\t\tScheduler_column_width_ratio=\"%s\" />\n",
										   VERSION_FICHIER,
										   VERSION,
										   nom_fichier_backup,
										   titre_fichier,
										   adresse_commune,
										   adresse_secondaire,
										   no_devise_totaux_tiers,
										   no_devise_totaux_categ,
										   no_devise_totaux_ib,
										   affichage_echeances,
										   affichage_echeances_perso_nb_libre,
										   affichage_echeances_perso_j_m_a,
										   valeur_echelle_recherche_date_import,
										   etat.utilise_logo,
										   chemin_logo,
										   etat.retient_affichage_par_compte,
										   transactions_view,
										   transaction_column_width_ratio,
										   ligne_affichage_une_ligne,
										   two_lines_showed,
										   tree_lines_showed,
										   etat.formulaire_distinct_par_compte,
										   scheduler_column_width_ratio),
										   NULL );
    g_free (first_string_to_free);
    g_free (second_string_to_free);
    g_free (transactions_view);
    g_free (transaction_column_width_ratio);
    g_free (two_lines_showed);
    g_free (tree_lines_showed);
    g_free (scheduler_column_width_ratio);

    /* save the accounts informations */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint j, k;
	gchar *last_reconcile_date;
	gchar *sort_list;
	gchar *sort_kind_column;
	gchar *form_organization;
	gchar *form_columns_width;
	GSList *list_tmp_2;

	i = gsb_account_get_no_account ( list_tmp -> data );

	/* set the last reconcile date */

	if ( gsb_account_get_current_reconcile_date (i) )
	{
	    last_reconcile_date = g_strconcat ( first_string_to_free = utils_str_itoa ( g_date_day ( gsb_account_get_current_reconcile_date (i) ) ),
						"/",
						second_string_to_free = utils_str_itoa ( g_date_month ( gsb_account_get_current_reconcile_date (i) ) ),
						"/",
						third_string_to_free = utils_str_itoa ( g_date_year ( gsb_account_get_current_reconcile_date (i) ) ),
						NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);
	    g_free (third_string_to_free);
	}
	else
	    last_reconcile_date = g_strdup ("");

	/* set the sort_list */

	list_tmp_2 = gsb_account_get_sort_list (i);
	sort_list = NULL;

	while ( list_tmp_2 )
	{
	    if ( sort_list )
	    {
		sort_list = g_strconcat ( first_string_to_free = sort_list,
					  "/",
					  second_string_to_free = utils_str_itoa ( GPOINTER_TO_INT ( list_tmp_2 -> data )),
					  NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		sort_list = utils_str_itoa ( GPOINTER_TO_INT ( list_tmp_2 -> data ));

	    list_tmp_2 = list_tmp_2 -> next;
	}

	/* set the default sort kind for the columns */

	sort_kind_column = NULL;

	for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( sort_kind_column )
	    {
		sort_kind_column = g_strconcat ( first_string_to_free = sort_kind_column,
						 "-",
						 second_string_to_free = utils_str_itoa ( gsb_account_get_column_sort ( i,
												j )),
						 NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		sort_kind_column = utils_str_itoa ( gsb_account_get_column_sort ( i,
								     j ));
	}

	/* set the form organization */

	form_organization = NULL;

	for ( k=0 ; k<4 ; k++ )
	    for ( j=0 ; j< 6 ; j++ )
		if ( form_organization )
		{ 
		    form_organization = g_strconcat ( first_string_to_free = form_organization,
						      "-",
						      second_string_to_free = utils_str_itoa ( gsb_account_get_form_organization (i) -> tab_remplissage_formulaire [k][j] ),
						      NULL );
		    g_free (first_string_to_free);
		    g_free (second_string_to_free);
		}
		else
		    form_organization = utils_str_itoa ( gsb_account_get_form_organization (i) -> tab_remplissage_formulaire [k][j] );

	/* set the form columns width */

	form_columns_width = NULL;

	for ( k=0 ; k<6 ; k++ )
	    if ( form_columns_width )
	    {
		form_columns_width = g_strconcat ( first_string_to_free = form_columns_width,
						   "-",
						   second_string_to_free = utils_str_itoa ( gsb_account_get_form_organization (i) -> taille_colonne_pourcent [k] ),
						   NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		form_columns_width = utils_str_itoa ( gsb_account_get_form_organization (i) -> taille_colonne_pourcent [k] );

	/* now we can fill the file content */

	file_content = g_strconcat ( first_string_to_free = file_content,
				     second_string_to_free = g_markup_printf_escaped ( "\t<Account\n\t\tName=\"%s\"\n\t\tId=\"%s\"\n\t\tNumber=\"%d\"\n\t\tOwner=\"%s\"\n\t\tKind=\"%d\"\n\t\tCurrency=\"%d\"\n\t\tBank=\"%d\"\n\t\tBank_branch_code=\"%s\"\n\t\tBank_account_number=\"%s\"\n\t\tKey=\"%s\"\n\t\tInitial_balance=\"%4.7f\"\n\t\tMinimum_wanted_balance=\"%4.7f\"\n\t\tMinimum_authorised_balance=\"%4.7f\"\n\t\tLast_reconcile_date=\"%s\"\n\t\tLast_reconcile_balance=\"%4.7f\"\n\t\tLast_reconcile_number=\"%d\"\n\t\tClosed_account=\"%d\"\n\t\tShow_marked=\"%d\"\n\t\tLines_per_transaction=\"%d\"\n\t\tComment=\"%s\"\n\t\tOwner_address=\"%s\"\n\t\tDefault_debit_method=\"%d\"\n\t\tDefault_credit_method=\"%d\"\n\t\tSort_by_method=\"%d\"\n\t\tNeutrals_inside_method=\"%d\"\n\t\tSort_order=\"%s\"\n\t\tAscending_sort=\"%d\"\n\t\tColumn_sort=\"%d\"\n\t\tSorting_kind_column=\"%s\"\n\t\tForm_columns_number=\"%d\"\n\t\tForm_lines_number=\"%d\"\n\t\tForm_organization=\"%s\"\n\t\tForm_columns_width=\"%s\" />\n",
										       gsb_account_get_name (i),
										       gsb_account_get_id (i),
										       i,
										       gsb_account_get_holder_name (i),
										       gsb_account_get_kind (i),
										       gsb_account_get_currency (i),
										       gsb_account_get_bank (i),
										       gsb_account_get_bank_branch_code (i),
										       gsb_account_get_bank_account_number (i),
										       gsb_account_get_bank_account_key (i),
										       gsb_account_get_init_balance (i),
										       gsb_account_get_mini_balance_wanted (i),
										       gsb_account_get_mini_balance_authorized (i),
										       last_reconcile_date,
										       gsb_account_get_reconcile_balance (i),
										       gsb_account_get_reconcile_last_number (i),
										       gsb_account_get_closed_account (i),
										       gsb_account_get_r (i),
										       gsb_account_get_nb_rows (i),
										       gsb_account_get_comment (i),
										       gsb_account_get_holder_address (i),
										       gsb_account_get_default_debit (i),
										       gsb_account_get_default_credit (i),
										       gsb_account_get_reconcile_sort_type (i),
										       gsb_account_get_split_neutral_payment (i),
										       sort_list,
										       gsb_account_get_sort_type (i),
										       gsb_account_get_sort_column (i),
										       sort_kind_column,
										       gsb_account_get_form_organization (i) -> nb_colonnes,
										       gsb_account_get_form_organization (i) -> nb_lignes,
										       form_organization,
										       form_columns_width ),
										       NULL );
	g_free (first_string_to_free);
	g_free (second_string_to_free);
	g_free (last_reconcile_date);
	g_free (sort_list);
	g_free (sort_kind_column);
	g_free (form_organization);
	g_free (form_columns_width);

	list_tmp = list_tmp -> next;
    }


    /* save the differents method of payment */

    list_tmp = gsb_account_get_list_accounts ();

    while ( list_tmp )
    {
	GSList *list_tmp_2;
	
	i = gsb_account_get_no_account ( list_tmp -> data );
	list_tmp_2 = gsb_account_get_method_payment_list (i);

	while ( list_tmp_2 )
	{
	    struct struct_type_ope *method;

	    method = list_tmp_2 -> data;

	    /* now we can fill the file content */

	    file_content = g_strconcat ( first_string_to_free = file_content,
					 second_string_to_free = g_markup_printf_escaped ( "\t<Payment Number=\"%d\" Name=\"%s\" Sign=\"%d\" Show_entry=\"%d\" Automatic_number=\"%d\" Current_number=\"%d\" Account=\"%d\" />\n",
											   method -> no_type,
											   method -> nom_type,
											   method -> signe_type,
											   method -> affiche_entree,
											   method -> numerotation_auto,
											   method -> no_en_cours,
											   i ),
					 NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);

	    list_tmp_2 = list_tmp_2 -> next;
	}
	list_tmp = list_tmp -> next;
    }

    /* save the transactions */

    list_tmp = gsb_transaction_data_get_transactions_list ();

    while ( list_tmp )
    {
	gint transaction_number;

	transaction_number = gsb_transaction_data_get_transaction_number ( list_tmp -> data );
	
	/* now we can fill the file content */

	file_content = g_strconcat ( first_string_to_free = file_content,
				     second_string_to_free = g_markup_printf_escaped ( "\t<Transaction Ac=\"%d\" Nb=\"%d\" Id=\"%s\" Dt=\"%s\" Dv=\"%s\" Am=\"%4.7f\" Cu=\"%d\" Exb=\"%d\" Exr=\"%4.7f\" Exf=\"%4.7f\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Br=\"%d\" No=\"%s\" Pn=\"%d\" Pc=\"%s\" Ma=\"%d\" Au=\"%d\" Re=\"%d\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" Vo=\"%s\" Ba=\"%s\" Trt=\"%d\" Tra=\"%d\" Mo=\"%d\" />\n",
										       gsb_transaction_data_get_account_number ( transaction_number ),
										       transaction_number,
										       gsb_transaction_data_get_transaction_id ( transaction_number),
										       gsb_format_gdate ( gsb_transaction_data_get_date ( transaction_number )),
										       gsb_format_gdate ( gsb_transaction_data_get_value_date ( transaction_number )),
										       gsb_transaction_data_get_amount ( transaction_number ),
										       gsb_transaction_data_get_currency_number (transaction_number ),
										       gsb_transaction_data_get_change_between (transaction_number ),
										       gsb_transaction_data_get_exchange_rate (transaction_number ),
										       gsb_transaction_data_get_exchange_fees ( transaction_number),
										       gsb_transaction_data_get_party_number ( transaction_number),
										       gsb_transaction_data_get_category_number ( transaction_number),
										       gsb_transaction_data_get_sub_category_number (transaction_number),
										       gsb_transaction_data_get_breakdown_of_transaction (transaction_number),
										       gsb_transaction_data_get_notes (transaction_number),
										       gsb_transaction_data_get_method_of_payment_number (transaction_number),
										       gsb_transaction_data_get_method_of_payment_content (transaction_number),
										       gsb_transaction_data_get_marked_transaction (transaction_number),
										       gsb_transaction_data_get_automatic_transaction (transaction_number),
										       gsb_transaction_data_get_reconcile_number (transaction_number),
										       gsb_transaction_data_get_financial_year_number (transaction_number),
										       gsb_transaction_data_get_budgetary_number (transaction_number),
										       gsb_transaction_data_get_sub_budgetary_number (transaction_number),
										       gsb_transaction_data_get_voucher (transaction_number),
										       gsb_transaction_data_get_bank_references (transaction_number),
										       gsb_transaction_data_get_transaction_number_transfer (transaction_number),
										       gsb_transaction_data_get_account_number_transfer (transaction_number),
										       gsb_transaction_data_get_mother_transaction_number (transaction_number)),
										       NULL );
	g_free (first_string_to_free);
	g_free (second_string_to_free);

	list_tmp = list_tmp -> next;
    }


    /* save the scheduled transactions */

    list_tmp = liste_struct_echeances;

    while ( list_tmp )
    {
	struct operation_echeance *echeance;

	echeance = list_tmp -> data;

	/* now we can fill the file content */

	file_content = g_strconcat ( first_string_to_free = file_content,
				     second_string_to_free = g_markup_printf_escaped ( "\t<Scheduled Nb=\"%d\" Dt=\"%s\" Ac=\"%d\" Am=\"%4.7f\" Cu=\"%d\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Tra=\"%d\" Pn=\"%d\" CPn=\"%d\" Pc=\"%s\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" No=\"%s\" Au=\"%d\" Pe=\"%d\" Pei=\"%d\" Pep=\"%d\" Dtl=\"%s\" Br=\"%d\" Mo=\"%d\" />\n",
										       echeance -> no_operation,
										       gsb_format_gdate ( echeance -> date),
										       echeance -> compte,
										       echeance -> montant,
										       echeance -> devise,
										       echeance -> tiers,
										       echeance -> categorie,
										       echeance -> sous_categorie,
										       echeance -> compte_virement,
										       echeance -> type_ope,
										       echeance -> type_contre_ope,
										       echeance -> contenu_type,
										       echeance -> no_exercice,
										       echeance -> imputation,
										       echeance -> sous_imputation,
										       echeance -> notes,
										       echeance -> auto_man,
										       echeance -> periodicite,
										       echeance -> intervalle_periodicite_personnalisee,
										       echeance -> periodicite_personnalisee,
										       gsb_format_gdate ( echeance -> date_limite),
										       echeance -> operation_ventilee,
										       echeance -> no_operation_ventilee_associee ),
										       NULL );
	g_free (first_string_to_free);
	g_free (second_string_to_free);

	list_tmp = list_tmp -> next;
    }



    /* finish the file */

    file_content = g_strconcat ( first_string_to_free = file_content,
				 "</Grisbi>");
    g_free (first_string_to_free);

    /* the file is in memory, we can save it */

    grisbi_file = fopen ( filename,
			  "w" );

    if ( !grisbi_file )
    {
	dialogue_error ( g_strdup_printf ( _("Cannot save file '%s': %s"),
					   filename,
					   latin2utf8(strerror(errno)) ));
	free ( file_content);
	return ( FALSE );
    }
    

    if ( !fwrite ( file_content,
		   sizeof (gchar),
		   strlen (file_content),
		   grisbi_file ))
    {
	dialogue_error ( g_strdup_printf ( _("Cannot save file '%s': %s"),
					   filename,
					   latin2utf8(strerror(errno)) ));
	free ( file_content);
	return ( FALSE );
    }
    
    fclose (grisbi_file);

    free ( file_content);
   
    /* if it's a new file, we set the permission */

    if ( do_chmod )
	chmod ( filename,
		S_IRUSR | S_IWUSR );

    etat.en_train_de_sauvegarder = 0;

    return ( TRUE );
}
