/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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
 * \file gsb_file_save.c
 * save the file
 */

#include "include.h"

/*START_INCLUDE*/
#include "gsb_file_save.h"
#include "./dialog.h"
#include "./gsb_data_account.h"
#include "./gsb_data_archive.h"
#include "./gsb_data_bank.h"
#include "./gsb_data_budget.h"
#include "./gsb_data_category.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_currency_link.h"
#include "./gsb_data_form.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_reconcile.h"
#include "./gsb_data_report_amout_comparison.h"
#include "./gsb_data_report.h"
#include "./gsb_data_report_text_comparison.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file.h"
#include "./gsb_file_util.h"
#include "./utils_dates.h"
#include "./navigation.h"
#include "./gsb_plugins.h"
#include "./gsb_real.h"
#include "./gsb_scheduler_list.h"
#include "./gsb_transactions_list.h"
#include "./utils_str.h"
#include "./structures.h"
#include "./gsb_data_form.h"
#include "./gsb_scheduler_list.h"
#include "./gsb_transactions_list.h"
#include "./include.h"
#include "./echeancier_infos.h"
#include "./erreur.h"
#include "./gsb_plugins.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gulong gsb_file_save_account_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content );
static gulong gsb_file_save_archive_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content );
static gulong gsb_file_save_bank_part ( gulong iterator,
				 gulong *length_calculated,
				 gchar **file_content );
static gulong gsb_file_save_currency_link_part ( gulong iterator,
					  gulong *length_calculated,
					  gchar **file_content );
static gulong gsb_file_save_currency_part ( gulong iterator,
				     gulong *length_calculated,
				     gchar **file_content );
static gulong gsb_file_save_financial_year_part ( gulong iterator,
					   gulong *length_calculated,
					   gchar **file_content );
static gulong gsb_file_save_general_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content,
				    gint archive_number );
static gulong gsb_file_save_party_part ( gulong iterator,
				  gulong *length_calculated,
				  gchar **file_content );
static gulong gsb_file_save_payment_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content );
static gulong gsb_file_save_reconcile_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content );
static gulong gsb_file_save_scheduled_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content );
static gulong gsb_file_save_transaction_part ( gulong iterator,
					gulong *length_calculated,
					gchar **file_content,
					gint archive_number );
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern gchar *chemin_logo;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern gchar *nom_fichier_backup;
extern gint scheduler_col_width[NB_COLS_SCHEDULER];
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern gchar *titre_fichier;
extern gint transaction_col_width[TRANSACTION_LIST_COL_NB];
extern gint valeur_echelle_recherche_date_import;
/*END_EXTERN*/




/** 
 * save the grisbi file or an archive
 * we don't check anything here, all must be done before, here we just write
 * the file and set the permissions
 *
 * an archive file is a normal grisbi file, but only with the wanted archived transactions
 * and without scheduled transactions
 *
 * \param filename the name of the file
 * \param compress TRUE if we want to compress the file
 * \param archive_number 0 for complete file, the number of archive if export an archive
 *
 * \return TRUE : ok, FALSE : problem
 * */
gboolean gsb_file_save_save_file ( const gchar *filename,
				   gboolean compress,
				   gint archive_number )
{
    gint do_chmod;
    FILE *grisbi_file;
    gulong iterator;

    gulong length_calculated;
    gchar *file_content;

    gint general_part;
    gint account_part;
    gint transaction_part;
    gint party_part;
    gint category_part;
    gint budgetary_part;
    gint currency_part;
    gint currency_link_part;
    gint bank_part;
    gint financial_year_part;
    gint archive_part;
    gint reconcile_part;
    gint report_part;

    gsb_plugin * plugin;

    gchar* tmpstr = g_strdup_printf ("gsb_file_save_save_file : %s", filename );
    devel_debug ( tmpstr );
    g_free ( tmpstr );

    do_chmod = !g_file_test ( filename,
			      G_FILE_TEST_EXISTS );

    etat.en_train_de_sauvegarder = 1;

    /* we begin to try to reserve enough memory to make the entire file
     * if not enough, we will make it growth later
     * the data below are about the memory to take for each part and for 1 of this part
     * with that i think we will allocate enough memory in one time but not too much */

    general_part = 900;
    account_part = 1200;
    transaction_part = 350;
    party_part = 100;
    category_part = 500;
    budgetary_part = 500;
    currency_part = 100;
    currency_link_part = 100;
    bank_part = 300;
    financial_year_part = 100;
    archive_part = 120;
    reconcile_part = 50;
    report_part = 2500;
    
    length_calculated = general_part
	+ account_part * gsb_data_account_get_accounts_amount ()
	+ transaction_part * g_slist_length ( gsb_data_transaction_get_complete_transactions_list ())
	+ party_part * g_slist_length ( gsb_data_payee_get_payees_list ())
	+ category_part * g_slist_length ( gsb_data_category_get_categories_list ())
	+ budgetary_part * g_slist_length ( gsb_data_budget_get_budgets_list ())
	+ currency_part * g_slist_length ( gsb_data_currency_get_currency_list () )
	+ currency_link_part * g_slist_length ( gsb_data_currency_link_get_currency_link_list ())
	+ bank_part * g_slist_length ( gsb_data_bank_get_bank_list ())
	+ financial_year_part * g_slist_length (gsb_data_fyear_get_fyears_list ())
	+ archive_part * g_slist_length (gsb_data_archive_get_archives_list ())
	+ reconcile_part * g_list_length (gsb_data_reconcile_get_reconcile_list ())
	+ report_part * g_slist_length ( gsb_data_report_get_report_list ());

    iterator = 0;
    file_content = g_malloc ( length_calculated );

    /* begin the file whit xml markup */
    iterator = gsb_file_save_append_part ( iterator,
					   &length_calculated,
					   &file_content,
					   my_strdup ("<?xml version=\"1.0\"?>\n<Grisbi>\n"));

    iterator = gsb_file_save_general_part ( iterator,
					    &length_calculated,
					    &file_content,
					    archive_number );

    iterator = gsb_file_save_currency_part ( iterator,
					     &length_calculated,
					     &file_content );

    iterator = gsb_file_save_account_part ( iterator,
					    &length_calculated,
					    &file_content );

    iterator = gsb_file_save_payment_part ( iterator,
					    &length_calculated,
					    &file_content );

    iterator = gsb_file_save_transaction_part ( iterator,
						&length_calculated,
						&file_content,
						archive_number );

    /* if we export an archive, no scheduled transactions */
    if (!archive_number)
	iterator = gsb_file_save_scheduled_part ( iterator,
						  &length_calculated,
						  &file_content );

    iterator = gsb_file_save_party_part ( iterator,
					  &length_calculated,
					  &file_content );

    iterator = gsb_file_save_category_part ( iterator,
					     &length_calculated,
					     &file_content );

    iterator = gsb_file_save_budgetary_part ( iterator,
					      &length_calculated,
					      &file_content );

    iterator = gsb_file_save_currency_link_part ( iterator,
						  &length_calculated,
						  &file_content );

    iterator = gsb_file_save_bank_part ( iterator,
					 &length_calculated,
					 &file_content );

    iterator = gsb_file_save_financial_year_part ( iterator,
						   &length_calculated,
						   &file_content );

    /* if we export an archive, no archive information */
    if (!archive_number)
	iterator = gsb_file_save_archive_part ( iterator,
						&length_calculated,
						&file_content );

    iterator = gsb_file_save_reconcile_part ( iterator,
					      &length_calculated,
					      &file_content );

    iterator = gsb_file_save_report_part ( iterator,
					   &length_calculated,
					   &file_content,
					   FALSE );
    /* finish the file */

    iterator = gsb_file_save_append_part ( iterator,
					   &length_calculated,
					   &file_content,
					   my_strdup ("</Grisbi>"));

    /* before saving the file, we compress and crypt it if necessary */

    if ( compress )
	iterator = gsb_file_util_compress_file ( &file_content,
						 iterator,
						 TRUE );

    if ( etat.crypt_file )
    {
	if ( ( plugin = gsb_plugin_find ( "openssl" ) ) )
	{
	    gint (*crypt_function) ( const gchar *, gchar **, gboolean, gulong );
	    
	    crypt_function = (gpointer) plugin -> plugin_run;
	    iterator = crypt_function ( filename, &file_content, TRUE, iterator );	
	    if ( ! iterator )
		return FALSE;
	}
	else
	{
	    dialogue_error_hint ( _("Grisbi was unable to load required plugin to "
				    "handle that file.\n\n"
				    "Please make sure if is installed (i.e. check "
				    "that 'grisbi-ssl' package is installed) and "
				    "try again."),
				  _("Encryption plugin not found." ) );
	    return FALSE;
	}
    }
    
    /* the file is in memory, we can save it */
    grisbi_file = fopen ( filename,
			  "w" );

    if ( !grisbi_file
	 ||
	 !fwrite ( file_content,
		   sizeof (gchar),
		   iterator,
		   grisbi_file ))
    {
	gchar* tmpstr = g_strdup_printf ( _("Cannot save file '%s': %s"),
					   filename,
					   latin2utf8(strerror(errno)) );
	dialogue_error ( tmpstr );
	g_free ( tmpstr );
	g_free ( file_content);
	return ( FALSE );
    }
    
    fclose (grisbi_file);

    /* if it's a new file, we set the permission */
    if ( do_chmod )
	chmod ( filename,
		S_IRUSR | S_IWUSR );

    etat.en_train_de_sauvegarder = 0;

    return ( TRUE );
}



/**
 * add the string given in arg and
 * check if we don't go throw the upper limit of file_content
 * if yes, we reallocate it
 *
 * \param file_content the file content
 * \param iterator the current iterator
 * \param new_string the string we want to add, it will be freed by this function. Do not free it again !
 *
 * \return the new iterator
 * */
gulong gsb_file_save_append_part ( gulong iterator,
				   gulong *length_calculated,
				   gchar **file_content,
				   gchar *new_string )
{
    if ( !new_string )
	return iterator;
    
    /* we check first if we don't go throw the upper limit */

    while ( (iterator + strlen (new_string)) >= *length_calculated )
    {
	/* we change the size by adding half of length_calculated */

	*length_calculated = 1.5 * *length_calculated;
	*file_content = g_realloc ( *file_content,
				  *length_calculated );

	notice_debug ( "The length calculated for saving file should be bigger?" );
    }

    memcpy ( *file_content + iterator,
	     new_string,
	     strlen (new_string));
    iterator = iterator + strlen (new_string);
    g_free (new_string);

    return iterator;
}

/**
 * save the general part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 * \param archive_number the number of the archive or 0 if not an archive 
 *
 * \return the new iterator
 * */
gulong gsb_file_save_general_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content,
				    gint archive_number )
{
    gchar *first_string_to_free;
    gchar *second_string_to_free;
    gchar *third_string_to_free;
    gint i,j;
    gchar *transactions_view;
    gchar *two_lines_showed;
    gchar *tree_lines_showed;
    gchar *scheduler_column_width_write;
    gchar *transaction_column_width_write;
    gchar *new_string;
    gchar *skipped_lines_string;
    gboolean is_archive = FALSE;

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

    /* prepare transaction_column_width_write */
    gsb_transactions_list_update_col_width ();

    transaction_column_width_write = NULL;

    for ( i=0 ; i<TRANSACTION_LIST_COL_NB ; i++ )
	if ( transaction_column_width_write )
	{
	    transaction_column_width_write = g_strconcat ( first_string_to_free = transaction_column_width_write,
							 "-",
							 second_string_to_free = utils_str_itoa ( transaction_col_width[i] ),
							 NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);
	}
	else
	    transaction_column_width_write  = utils_str_itoa ( transaction_col_width[i] );


    /* prepare scheduler_column_width_write */
    gsb_scheduler_list_update_col_width ();

    scheduler_column_width_write = NULL;

    for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
	if ( scheduler_column_width_write )
	{
	    scheduler_column_width_write = g_strconcat ( first_string_to_free = scheduler_column_width_write,
							 "-",
							 second_string_to_free = utils_str_itoa ( scheduler_col_width[i] ),
							 NULL );
	    g_free (first_string_to_free);
	    g_free (second_string_to_free);
	}
	else
	    scheduler_column_width_write  = utils_str_itoa ( scheduler_col_width[i] );


    /* CSV skipped lines */
    skipped_lines_string = utils_str_itoa ( etat.csv_skipped_lines[0] );
    for ( i = 1; i < CSV_MAX_TOP_LINES ; i ++ )
    {
	gchar* tmpstr = skipped_lines_string;
	skipped_lines_string = g_strconcat ( tmpstr ,
					     "-", 
					     utils_str_itoa ( etat.csv_skipped_lines[i] ),
					     NULL );
        g_free ( tmpstr );
    }

    /* if we save an archive, we save it here */
    if (archive_number
	||
	etat.is_archive )
	is_archive = TRUE;

    /* save the general informations */
    new_string = g_markup_printf_escaped ( "\t<General\n"
					   "\t\tFile_version=\"%s\"\n"
					   "\t\tGrisbi_version=\"%s\"\n"
					   "\t\tBackup_file=\"%s\"\n"
					   "\t\tCrypt_file=\"%d\"\n"
					   "\t\tArchive_file=\"%d\"\n"
					   "\t\tFile_title=\"%s\"\n"
					   "\t\tGeneral_address=\"%s\"\n"
					   "\t\tSecond_general_address=\"%s\"\n"
					   "\t\tParty_list_currency_number=\"%d\"\n"
					   "\t\tCategory_list_currency_number=\"%d\"\n"
					   "\t\tBudget_list_currency_number=\"%d\"\n"
					   "\t\tScheduler_view=\"%d\"\n"
					   "\t\tScheduler_custom_number=\"%d\"\n"
					   "\t\tScheduler_custom_menu=\"%d\"\n"
					   "\t\tImport_interval_search=\"%d\"\n"
					   "\t\tUse_logo=\"%d\"\n"
					   "\t\tPath_logo=\"%s\"\n"
					   "\t\tRemind_display_per_account=\"%d\"\n"
					   "\t\tNo_fill_r_at_begining=\"%d\"\n"
					   "\t\tTransactions_view=\"%s\"\n"
					   "\t\tOne_line_showed=\"%d\"\n"
					   "\t\tTwo_lines_showed=\"%s\"\n"
					   "\t\tThree_lines_showed=\"%s\"\n"
					   "\t\tRemind_form_per_account=\"%d\"\n"
					   "\t\tTransaction_column_width=\"%s\"\n"
					   "\t\tScheduler_column_width=\"%s\"\n"
					   "\t\tCombofix_mixed_sort=\"%d\"\n"
					   "\t\tCombofix_max_item=\"%d\"\n"
					   "\t\tCombofix_case_sensitive=\"%d\"\n"
					   "\t\tCombofix_enter_select_completion=\"%d\"\n"
					   "\t\tCombofix_force_payee=\"%d\"\n"
					   "\t\tCombofix_force_category=\"%d\"\n"
					   "\t\tAutomatic_amount_separator=\"%d\"\n"
					   "\t\tCSV_separator=\"%s\"\n"
					   "\t\tCSV_skipped_lines=\"%s\" />\n",
	VERSION_FICHIER,
	VERSION,
	nom_fichier_backup,
	etat.crypt_file,
	is_archive,
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
	etat.no_fill_r_at_begining,
	transactions_view,
	ligne_affichage_une_ligne,
	two_lines_showed,
	tree_lines_showed,
	etat.formulaire_distinct_par_compte,
	transaction_column_width_write,
	scheduler_column_width_write,
	etat.combofix_mixed_sort,
	etat.combofix_max_item,
	etat.combofix_case_sensitive,
	etat.combofix_enter_select_completion,
	etat.combofix_force_payee,
	etat.combofix_force_category,
	etat.automatic_separator,
	etat.csv_separator,
	skipped_lines_string );

    g_free (transactions_view);
    g_free (two_lines_showed);
    g_free (tree_lines_showed);
    g_free (scheduler_column_width_write);
    g_free (transaction_column_width_write);

    /* append the new string to the file content
     * and return the new iterator */

    return gsb_file_save_append_part ( iterator,
				       length_calculated,
				       file_content,
				       new_string );
}


/**
 * save the account part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_account_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content )
{
    GSList *list_tmp;

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gchar *first_string_to_free;
	gchar *second_string_to_free;
	gint account_number;
	gint j, k;
	gchar *sort_list;
	gchar *sort_kind_column;
	gchar *form_organization;
	gchar *form_columns_width;
	GSList *list_tmp_2;
	gchar *new_string;
	gchar *init_balance;
	gchar *mini_wanted;
	gchar *mini_auto;

	account_number = gsb_data_account_get_no_account ( list_tmp -> data );

	/* set the sort_list */
	list_tmp_2 = gsb_data_account_get_sort_list (account_number);
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
						 second_string_to_free = utils_str_itoa ( gsb_data_account_get_element_sort ( account_number,
															      j )),
						 NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		sort_kind_column = utils_str_itoa ( gsb_data_account_get_element_sort ( account_number,
											j ));
	}

	/* set the form organization */
	form_organization = NULL;

	for ( k=0 ; k<MAX_HEIGHT ; k++ )
	    for ( j=0 ; j<MAX_WIDTH ; j++ )
		if ( form_organization )
		{ 
		    form_organization = g_strconcat ( first_string_to_free = form_organization,
						      "-",
						      second_string_to_free = utils_str_itoa ( gsb_data_form_get_value ( account_number,
															 j,
															 k )),
						      NULL );
		    g_free (first_string_to_free);
		    g_free (second_string_to_free);
		}
		else
		    form_organization = utils_str_itoa (gsb_data_form_get_value ( account_number,
										  j,
										  k ));

	/* set the form columns width */
	form_columns_width = NULL;

	for ( k=0 ; k<MAX_WIDTH ; k++ )
	    if ( form_columns_width )
	    {
		form_columns_width = g_strconcat ( first_string_to_free = form_columns_width,
						   "-",
						   second_string_to_free = utils_str_itoa ( gsb_data_form_get_width_column ( account_number,
															     k )),
						   NULL );
		g_free (first_string_to_free);
		g_free (second_string_to_free);
	    }
	    else
		form_columns_width = utils_str_itoa ( gsb_data_form_get_width_column ( account_number,
										       k ));

	/* set the reals */
	init_balance = gsb_real_get_string (gsb_data_account_get_init_balance (account_number, -1));
	mini_wanted = gsb_real_get_string (gsb_data_account_get_mini_balance_wanted (account_number));
	mini_auto = gsb_real_get_string (gsb_data_account_get_mini_balance_authorized (account_number));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Account\n"
					       "\t\tName=\"%s\"\n"
					       "\t\tId=\"%s\"\n"
					       "\t\tNumber=\"%d\"\n"
					       "\t\tOwner=\"%s\"\n"
					       "\t\tKind=\"%d\"\n"
					       "\t\tCurrency=\"%d\"\n"
					       "\t\tBank=\"%d\"\n"
					       "\t\tBank_branch_code=\"%s\"\n"
					       "\t\tBank_account_number=\"%s\"\n"
					       "\t\tKey=\"%s\"\n"
					       "\t\tInitial_balance=\"%s\"\n"
					       "\t\tMinimum_wanted_balance=\"%s\"\n"
					       "\t\tMinimum_authorised_balance=\"%s\"\n"
					       "\t\tClosed_account=\"%d\"\n"
					       "\t\tShow_marked=\"%d\"\n"
					       "\t\tLines_per_transaction=\"%d\"\n"
					       "\t\tComment=\"%s\"\n"
					       "\t\tOwner_address=\"%s\"\n"
					       "\t\tDefault_debit_method=\"%d\"\n"
					       "\t\tDefault_credit_method=\"%d\"\n"
					       "\t\tSort_by_method=\"%d\"\n"
					       "\t\tNeutrals_inside_method=\"%d\"\n"
					       "\t\tSort_order=\"%s\"\n"
					       "\t\tAscending_sort=\"%d\"\n"
					       "\t\tColumn_sort=\"%d\"\n"
					       "\t\tSorting_kind_column=\"%s\"\n"
					       "\t\tForm_columns_number=\"%d\"\n"
					       "\t\tForm_lines_number=\"%d\"\n"
					       "\t\tForm_organization=\"%s\"\n"
					       "\t\tForm_columns_width=\"%s\" />\n",
	    gsb_data_account_get_name (account_number),
	    gsb_data_account_get_id (account_number),
	    account_number,
	    gsb_data_account_get_holder_name (account_number),
	    gsb_data_account_get_kind (account_number),
	    gsb_data_account_get_currency (account_number),
	    gsb_data_account_get_bank (account_number),
	    gsb_data_account_get_bank_branch_code (account_number),
	    gsb_data_account_get_bank_account_number (account_number),
	    gsb_data_account_get_bank_account_key (account_number),
	    init_balance,
	    mini_wanted,
	    mini_auto,
	    gsb_data_account_get_closed_account (account_number),
	    gsb_data_account_get_r (account_number),
	    gsb_data_account_get_nb_rows (account_number),
	    gsb_data_account_get_comment (account_number),
	    gsb_data_account_get_holder_address (account_number),
	    gsb_data_account_get_default_debit (account_number),
	    gsb_data_account_get_default_credit (account_number),
	    gsb_data_account_get_reconcile_sort_type (account_number),
	    gsb_data_account_get_split_neutral_payment (account_number),
	    sort_list,
	    gsb_data_account_get_sort_type (account_number),
	    gsb_data_account_get_sort_column (account_number),
	    sort_kind_column,
	    gsb_data_form_get_nb_columns (account_number),
	    gsb_data_form_get_nb_rows (account_number),
	    form_organization,
	    form_columns_width );

	g_free (sort_list);
	g_free (sort_kind_column);
	g_free (form_organization);
	g_free (form_columns_width);
	g_free (init_balance);
	g_free (mini_auto);
	g_free (mini_wanted);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );

	list_tmp = list_tmp -> next;
    }
    return iterator;
}

/**
 * save the methods of payment 
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_payment_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content )
{
    GSList *list_tmp;

    list_tmp = gsb_data_payment_get_payments_list ();

    while (list_tmp)
    {
	gint payment_number;
	gchar *new_string;

	payment_number = gsb_data_payment_get_number (list_tmp -> data);

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Payment Number=\"%d\" Name=\"%s\" Sign=\"%d\" Show_entry=\"%d\" Automatic_number=\"%d\" Current_number=\"%d\" Account=\"%d\" />\n",
					       payment_number,
					       gsb_data_payment_get_name (payment_number),
					       gsb_data_payment_get_sign (payment_number),
					       gsb_data_payment_get_show_entry (payment_number),
					       gsb_data_payment_get_automatic_numbering (payment_number),
					       gsb_data_payment_get_last_number (payment_number),
					       gsb_data_payment_get_account_number (payment_number));

	/* append the new string to the file content
	 * and take the new iterator */
	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}

/**
 * save the transactions 
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 * \param archive_number 0 to export all the transactions, the number of archive to export only that transactions
 *
 * \return the new iterator
 * */
gulong gsb_file_save_transaction_part ( gulong iterator,
					gulong *length_calculated,
					gchar **file_content,
					gint archive_number )
{
    GSList *list_tmp;

    list_tmp = gsb_data_transaction_get_complete_transactions_list ();

    while ( list_tmp )
    {
	gint transaction_number;
	gchar *new_string;
	gchar *amount;
	gchar *exchange_rate;
	gchar *exchange_fees;
	gchar *date;
	gchar *value_date;
	gint transaction_archive_number;

	transaction_number = gsb_data_transaction_get_transaction_number ( list_tmp -> data );

	/* get the archive number for below */
	transaction_archive_number = gsb_data_transaction_get_archive_number (transaction_number);

	if (archive_number)
	{
	    /* we export an archive, so continue only if the transaction belongs to that archive */
	    if (transaction_archive_number != archive_number)
	    {
		/* the transaction will not be exported */
		list_tmp = list_tmp -> next;
		continue;
	    }
	    /* the transaction belongs to the archive,
	     * we set its archive number to 0, to show it when we open an archive */
	    transaction_archive_number = 0;
	}

	/* set the reals */
	amount = gsb_real_get_string (gsb_data_transaction_get_amount ( transaction_number ));
	exchange_rate = gsb_real_get_string (gsb_data_transaction_get_exchange_rate (transaction_number ));
	exchange_fees = gsb_real_get_string (gsb_data_transaction_get_exchange_fees ( transaction_number));
	
	/* set the dates */
	date = gsb_format_gdate_safe ( gsb_data_transaction_get_date ( transaction_number ));
	value_date = gsb_format_gdate_safe ( gsb_data_transaction_get_value_date ( transaction_number ));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Transaction Ac=\"%d\" Nb=\"%d\" Id=\"%s\" Dt=\"%s\" Dv=\"%s\" Cu=\"%d\" Am=\"%s\" Exb=\"%d\" Exr=\"%s\" Exf=\"%s\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Br=\"%d\" No=\"%s\" Pn=\"%d\" Pc=\"%s\" Ma=\"%d\" Ar=\"%d\" Au=\"%d\" Re=\"%d\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" Vo=\"%s\" Ba=\"%s\" Trt=\"%d\" Tra=\"%d\" Mo=\"%d\" />\n",
					       gsb_data_transaction_get_account_number ( transaction_number ),
					       transaction_number,
					       gsb_data_transaction_get_transaction_id ( transaction_number),
					       date,
					       value_date,
					       gsb_data_transaction_get_currency_number (transaction_number ),
					       amount,
					       gsb_data_transaction_get_change_between (transaction_number ),
					       exchange_rate,
					       exchange_fees,
					       gsb_data_transaction_get_party_number ( transaction_number),
					       gsb_data_transaction_get_category_number ( transaction_number),
					       gsb_data_transaction_get_sub_category_number (transaction_number),
					       gsb_data_transaction_get_breakdown_of_transaction (transaction_number),
					       gsb_data_transaction_get_notes (transaction_number),
					       gsb_data_transaction_get_method_of_payment_number (transaction_number),
					       gsb_data_transaction_get_method_of_payment_content (transaction_number),
					       gsb_data_transaction_get_marked_transaction (transaction_number),
					       transaction_archive_number,
					       gsb_data_transaction_get_automatic_transaction (transaction_number),
					       gsb_data_transaction_get_reconcile_number (transaction_number),
					       gsb_data_transaction_get_financial_year_number (transaction_number),
					       gsb_data_transaction_get_budgetary_number (transaction_number),
					       gsb_data_transaction_get_sub_budgetary_number (transaction_number),
					       gsb_data_transaction_get_voucher (transaction_number),
					       gsb_data_transaction_get_bank_references (transaction_number),
					       gsb_data_transaction_get_transaction_number_transfer (transaction_number),
					       gsb_data_transaction_get_account_number_transfer (transaction_number),
					       gsb_data_transaction_get_mother_transaction_number (transaction_number));

	g_free (amount);
	g_free (exchange_rate);
	g_free (exchange_fees);
	g_free (date);
	g_free (value_date);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the scheduled transactions
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_scheduled_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_scheduled_get_scheduled_list ();

    while ( list_tmp )
    {
	gint scheduled_number;
	gchar *new_string;
	gchar *amount;
	gchar *date;
	gchar *limit_date;

	scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp -> data);

	/* set the real */
	amount = gsb_real_get_string (gsb_data_scheduled_get_amount ( scheduled_number));

	/* set the dates */
	date = gsb_format_gdate_safe (gsb_data_scheduled_get_date ( scheduled_number));
	limit_date = gsb_format_gdate_safe (gsb_data_scheduled_get_limit_date ( scheduled_number));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Scheduled Nb=\"%d\" Dt=\"%s\" Ac=\"%d\" Am=\"%s\" Cu=\"%d\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Tra=\"%d\" Pn=\"%d\" CPn=\"%d\" Pc=\"%s\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" No=\"%s\" Au=\"%d\" Pe=\"%d\" Pei=\"%d\" Pep=\"%d\" Dtl=\"%s\" Br=\"%d\" Mo=\"%d\" />\n",
					       scheduled_number,
					       date,
					       gsb_data_scheduled_get_account_number ( scheduled_number),
					       amount,
					       gsb_data_scheduled_get_currency_number ( scheduled_number),
					       gsb_data_scheduled_get_party_number ( scheduled_number),
					       gsb_data_scheduled_get_category_number ( scheduled_number),
					       gsb_data_scheduled_get_sub_category_number ( scheduled_number),
					       gsb_data_scheduled_get_account_number_transfer ( scheduled_number),
					       gsb_data_scheduled_get_method_of_payment_number ( scheduled_number),
					       gsb_data_scheduled_get_contra_method_of_payment_number ( scheduled_number),
					       gsb_data_scheduled_get_method_of_payment_content ( scheduled_number),
					       gsb_data_scheduled_get_financial_year_number ( scheduled_number),
					       gsb_data_scheduled_get_budgetary_number ( scheduled_number),
					       gsb_data_scheduled_get_sub_budgetary_number ( scheduled_number),
					       gsb_data_scheduled_get_notes ( scheduled_number),
					       gsb_data_scheduled_get_automatic_scheduled ( scheduled_number),
					       gsb_data_scheduled_get_frequency ( scheduled_number),
					       gsb_data_scheduled_get_user_interval ( scheduled_number),
					       gsb_data_scheduled_get_user_entry ( scheduled_number),
					       limit_date,
					       gsb_data_scheduled_get_breakdown_of_scheduled ( scheduled_number),
					       gsb_data_scheduled_get_mother_scheduled_number ( scheduled_number));

	g_free (amount);
	g_free (date);
	g_free (limit_date);

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the parties
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_party_part ( gulong iterator,
				  gulong *length_calculated,
				  gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_payee_get_payees_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint payee_number;

	payee_number = gsb_data_payee_get_no_payee (list_tmp -> data);
	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Party Nb=\"%d\" Na=\"%s\" Txt=\"%s\" />\n",
					       payee_number,
					       gsb_data_payee_get_name (payee_number,
								   TRUE ),
					       gsb_data_payee_get_description (payee_number));

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the categories
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_category_part ( gulong iterator,
				     gulong *length_calculated,
				     gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_category_get_categories_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint category_number;
	GSList *sub_list_tmp;

	category_number = gsb_data_category_get_no_category (list_tmp -> data);

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Category Nb=\"%d\" Na=\"%s\" Kd=\"%d\" />\n",
					       category_number,
					       gsb_data_category_get_name ( category_number,
									    0,
									    NULL ),
					       gsb_data_category_get_type ( category_number ));

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	/* save the sub-categories */

	sub_list_tmp = gsb_data_category_get_sub_category_list ( category_number );

	while ( sub_list_tmp )
	{
	    gint sub_category_number;

	    sub_category_number = gsb_data_category_get_no_sub_category (sub_list_tmp -> data);

	    /* now we can fill the file content
	     * carrefull : the number of category must be the first */

	    new_string = g_markup_printf_escaped ( "\t<Sub_category Nbc=\"%d\" Nb=\"%d\" Na=\"%s\" />\n",
						   category_number,
						   sub_category_number,
						   gsb_data_category_get_sub_category_name ( category_number,
											     sub_category_number,
											     NULL ));

	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );

	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the budgetaries
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_budgetary_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_budget_get_budgets_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint budget_number;
	GSList *sub_list_tmp;

	budget_number = gsb_data_budget_get_no_budget (list_tmp -> data);

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Budgetary Nb=\"%d\" Na=\"%s\" Kd=\"%d\" />\n",
					       budget_number,
					       gsb_data_budget_get_name ( budget_number,
									  0,
									  NULL ),
					       gsb_data_budget_get_type ( budget_number ));

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	/* save the sub-budgetaries */

	sub_list_tmp = gsb_data_budget_get_sub_budget_list ( budget_number );

	while ( sub_list_tmp )
	{
	    gint sub_budget_number;

	    sub_budget_number = gsb_data_budget_get_no_sub_budget (sub_list_tmp -> data);

	    /* now we can fill the file content
	     * carrefull : the number of budget must be the first */

	    new_string = g_markup_printf_escaped ( "\t<Sub_budgetary Nbb=\"%d\" Nb=\"%d\" Na=\"%s\" />\n",
						   budget_number,
						   sub_budget_number,
						   gsb_data_budget_get_sub_budget_name ( budget_number,
											 sub_budget_number,
											 NULL ));

	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );

	    sub_list_tmp = sub_list_tmp -> next;
	}
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the currencies
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_currency_part ( gulong iterator,
				     gulong *length_calculated,
				     gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_currency_get_currency_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint currency_number;

	currency_number = gsb_data_currency_get_no_currency (list_tmp -> data);

	/* now we can fill the file content */

	new_string = g_markup_printf_escaped ( "\t<Currency Nb=\"%d\" Na=\"%s\" Co=\"%s\" Ico=\"%s\" Fl=\"%d\" />\n",
					       currency_number,
					       gsb_data_currency_get_name (currency_number),
					       gsb_data_currency_get_code (currency_number),
					       gsb_data_currency_get_code_iso4217 (currency_number),
					       gsb_data_currency_get_floating_point (currency_number));

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}

/**
 * save the currency_links
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_currency_link_part ( gulong iterator,
					  gulong *length_calculated,
					  gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_currency_link_get_currency_link_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint link_number;
	gchar *change_rate;

	link_number = gsb_data_currency_link_get_no_currency_link (list_tmp -> data);

	/* set the number */
	change_rate = gsb_real_get_string (gsb_data_currency_link_get_change_rate (link_number));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Currency_link Nb=\"%d\" Cu1=\"%d\" Cu2=\"%d\" Ex=\"%s\" />\n",
					       link_number,
					       gsb_data_currency_link_get_first_currency (link_number),
					       gsb_data_currency_link_get_second_currency (link_number),
					       change_rate);
        g_free ( change_rate );

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}



/**
 * save the banks
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_bank_part ( gulong iterator,
				 gulong *length_calculated,
				 gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_bank_get_bank_list ();

    while ( list_tmp )
    {
	gint bank_number;
	gchar *new_string;

	bank_number = gsb_data_bank_get_no_bank (list_tmp -> data);

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Bank Nb=\"%d\" Na=\"%s\" Co=\"%s\" Adr=\"%s\" Tel=\"%s\" Mail=\"%s\" Web=\"%s\" Nac=\"%s\" Faxc=\"%s\" Telc=\"%s\" Mailc=\"%s\" Rem=\"%s\" />\n",
					       bank_number,
					       gsb_data_bank_get_name (bank_number),
					       gsb_data_bank_get_code (bank_number),
					       gsb_data_bank_get_bank_address (bank_number),
					       gsb_data_bank_get_bank_tel (bank_number),
					       gsb_data_bank_get_bank_mail (bank_number),
					       gsb_data_bank_get_bank_web (bank_number),
					       gsb_data_bank_get_correspondent_name (bank_number),
					       gsb_data_bank_get_correspondent_fax (bank_number),
					       gsb_data_bank_get_correspondent_tel (bank_number),
					       gsb_data_bank_get_correspondent_mail (bank_number),
					       gsb_data_bank_get_bank_note (bank_number));

	/* append the new string to the file content
	 * and take the new iterator */

	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the financials years
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_financial_year_part ( gulong iterator,
					   gulong *length_calculated,
					   gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_fyear_get_fyears_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint fyear_number;
	gchar *begining_date;
	gchar *end_date;

	fyear_number = gsb_data_fyear_get_no_fyear (list_tmp -> data);

	/* set the date */
	begining_date = gsb_format_gdate_safe (gsb_data_fyear_get_begining_date(fyear_number));
	end_date = gsb_format_gdate_safe (gsb_data_fyear_get_end_date(fyear_number));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped( "\t<Financial_year Nb=\"%d\" Na=\"%s\" Bdte=\"%s\" Edte=\"%s\" Sho=\"%d\" />\n",
					      fyear_number,
					      gsb_data_fyear_get_name (fyear_number),
					      begining_date,
					      end_date,
					      gsb_data_fyear_get_form_show (fyear_number));

	g_free (begining_date);
	g_free (end_date);

	/* append the new string to the file content
	 * and take the new iterator */
	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the archives structures
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_archive_part ( gulong iterator,
				    gulong *length_calculated,
				    gchar **file_content )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_archive_get_archives_list ();

    while ( list_tmp )
    {
	gint archive_number;
	gchar *new_string;
	gchar *begining_date;
	gchar *end_date;

	archive_number = gsb_data_archive_get_no_archive (list_tmp -> data);

	/* set the date */
	begining_date = gsb_format_gdate_safe (gsb_data_archive_get_begining_date (archive_number));
	end_date = gsb_format_gdate_safe (gsb_data_archive_get_end_date (archive_number));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped( "\t<Archive Nb=\"%d\" Na=\"%s\" Bdte=\"%s\" Edte=\"%s\" Fye=\"%d\" Rep=\"%s\" />\n",
					      archive_number,
					      gsb_data_archive_get_name (archive_number),
					      begining_date,
					      end_date,
					      gsb_data_archive_get_fyear (archive_number),
					      gsb_data_archive_get_report_title (archive_number));

	if (begining_date)
	    g_free (begining_date);
	if (end_date)
	    g_free (end_date);

	/* append the new string to the file content
	 * and take the new iterator */
	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the reconcile structures
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 * */
gulong gsb_file_save_reconcile_part ( gulong iterator,
				      gulong *length_calculated,
				      gchar **file_content )
{
    GList *list_tmp;

    list_tmp = gsb_data_reconcile_get_reconcile_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint reconcile_number;
	gchar *init_date;
	gchar *final_date;
	gchar *init_balance;
	gchar *final_balance;

	reconcile_number = gsb_data_reconcile_get_no_reconcile (list_tmp -> data);

	/* set the reconcile dates */
	init_date = gsb_format_gdate_safe (gsb_data_reconcile_get_init_date (reconcile_number));
	if (!init_date)
	    init_date  = my_strdup ("");

	final_date = gsb_format_gdate_safe (gsb_data_reconcile_get_final_date (reconcile_number));
	if (!final_date)
	    final_date = my_strdup ("");

	/* set the balances strings */
	init_balance = gsb_real_get_string (gsb_data_reconcile_get_init_balance (reconcile_number));
	final_balance = gsb_real_get_string (gsb_data_reconcile_get_final_balance (reconcile_number));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Reconcile Nb=\"%d\" Na=\"%s\" Acc=\"%d\" Idate=\"%s\" Fdate=\"%s\" Ibal=\"%s\" Fbal=\"%s\" />\n",
					       reconcile_number,
					       gsb_data_reconcile_get_name (reconcile_number),
					       gsb_data_reconcile_get_account (reconcile_number),
					       init_date,
					       final_date,
					       init_balance, 
					       final_balance );

	g_free (init_date);
	g_free (final_date);
	g_free (init_balance);
	g_free (final_balance);

	/* append the new string to the file content
	 * and take the new iterator */
	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );
	list_tmp = list_tmp -> next;
    }
    return iterator;
}


/**
 * save the reports
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 * \param current_report if TRUE, only save the current report (to exporting a report)
 *
 * \return the new iterator
 * */
gulong gsb_file_save_report_part ( gulong iterator,
				   gulong *length_calculated,
				   gchar **file_content,
				   gboolean current_report )
{
    GSList *list_tmp;
	
    list_tmp = gsb_data_report_get_report_list ();

    while ( list_tmp )
    {
	gchar *new_string;
	gint report_number;
	gint report_number_to_write;
	GSList *pointer_list;
	gchar *general_sort_type;
	gchar *financial_year_select;
	gchar *account_selected;
	gchar *transfer_selected_accounts;
	gchar *categ_selected;
	gchar *budget_selected;
	gchar *payee_selected;
	gchar *payment_method_list;
	GSList *list_tmp_2;
	gchar *date_start;
	gchar *date_end;
	gchar *report_name;
	gint amount_comparison_number_to_write;
	gint text_comparison_number_to_write;
	
	report_number = gsb_data_report_get_report_number (list_tmp -> data);

	/* if we need only the current report, we check here */

	if ( current_report
	     &&
	     gsb_gui_navigation_get_current_report() != report_number )
	{
	    list_tmp = list_tmp -> next;
	    continue;
	}

	/* if current_report is set, this mean we export the report,
	 * we cannot save the number should be automaticly given while
	 * importing, so set the report number to -1 */
	if (current_report)
	    report_number_to_write = -1;
	else
	    report_number_to_write = report_number;

	/* set the name, it will be the normal name except if we are exporting
	 * the report, we add 'export' at the end */
	if (current_report)
	    report_name = g_strconcat ( gsb_data_report_get_report_name (report_number),
					" (export)",
					NULL );
	else
	    report_name = my_strdup (gsb_data_report_get_report_name (report_number));

	/* set the general sort type */
	pointer_list = gsb_data_report_get_sorting_type (report_number);
	general_sort_type = NULL;

	while ( pointer_list )
	{
	    if ( general_sort_type )
	    {
	        gchar* tmpstr = general_sort_type;
		general_sort_type = g_strconcat ( tmpstr,
						  "/-/",
						  utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
						  NULL );
	        g_free ( tmpstr );
	    }
	    else
		general_sort_type = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* set the financial_year_select */
	pointer_list = gsb_data_report_get_financial_year_list (report_number);
	financial_year_select = NULL;

	while ( pointer_list )
	{
	    if ( financial_year_select )
	    {
	        gchar* tmpstr = financial_year_select ;
		financial_year_select = g_strconcat ( tmpstr,
						      "/-/",
						      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
						      NULL );
	        g_free (tmpstr);
	    }
	    else
		financial_year_select = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* set the account_selected */
	pointer_list = gsb_data_report_get_account_numbers (report_number);
	account_selected = NULL;

	while ( pointer_list )
	{
	    if ( account_selected )
	    {
	        gchar* tmpstr = account_selected;
		account_selected = g_strconcat ( tmpstr,
						 "/-/",
						 utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
						 NULL );
	        g_free (tmpstr);
	    }
	    else
		account_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the transfer_selected_accounts */
	pointer_list = gsb_data_report_get_transfer_account_numbers (report_number);
	transfer_selected_accounts = NULL;

	while ( pointer_list )
	{
	    if ( transfer_selected_accounts )
	    {
	        gchar* tmpstr = transfer_selected_accounts;
		transfer_selected_accounts = g_strconcat ( tmpstr,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	        g_free ( tmpstr );
	    }
	    else
		transfer_selected_accounts = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the categ_selected */
	pointer_list = gsb_data_report_get_category_numbers (report_number);
	categ_selected = NULL;

	while ( pointer_list )
	{
	    if ( categ_selected )
	    {
	        gchar* tmpstr = categ_selected;
		categ_selected = g_strconcat ( tmpstr ,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	        g_free ( tmpstr );
	    }
	    else
		categ_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the budget_selected */
	pointer_list = gsb_data_report_get_budget_numbers (report_number);
	budget_selected = NULL;

	while ( pointer_list )
	{
	    if ( budget_selected )
	    {
	        gchar* tmpstr = budget_selected;
		budget_selected = g_strconcat ( tmpstr,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	        g_free ( tmpstr );
	    }
	    else
		budget_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the payee_selected */
	pointer_list = gsb_data_report_get_payee_numbers (report_number);
	payee_selected = NULL;

	while ( pointer_list )
	{
	    if ( payee_selected )
	    {
	        gchar* tmpstr = payee_selected;
		payee_selected = g_strconcat ( tmpstr,
					      "/-/",
					      utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data )),
					      NULL );
	        g_free ( tmpstr );
	    }
	    else
		payee_selected = utils_str_itoa ( GPOINTER_TO_INT ( pointer_list -> data ));

	    pointer_list = pointer_list -> next;
	}

	/* 	set the payment_method_list */
	pointer_list = gsb_data_report_get_method_of_payment_list (report_number);
	payment_method_list = NULL;

	while ( pointer_list )
	{
	    if ( payment_method_list )
	    {
	        gchar* tmpstr = payment_method_list;
		payment_method_list = g_strconcat ( tmpstr,
						    "/-/",
						    pointer_list -> data,
						    NULL );
	        g_free ( tmpstr );
	    }
	    else
		payment_method_list = my_strdup (pointer_list -> data );

	    pointer_list = pointer_list -> next;
	}

	/* set the dates */
	date_start = gsb_format_gdate_safe (gsb_data_report_get_personal_date_start (report_number));
	date_end = gsb_format_gdate_safe (gsb_data_report_get_personal_date_end (report_number));

	/* now we can fill the file content */
	new_string = g_markup_printf_escaped ( "\t<Report\n"
					       "\t\tNb=\"%d\"\n"
					       "\t\tName=\"%s\"\n"
					       "\t\tGeneral_sort_type=\"%s\"\n"
					       "\t\tShow_r=\"%d\"\n"
					       "\t\tShow_transaction=\"%d\"\n"
					       "\t\tShow_transaction_amount=\"%d\"\n"
					       "\t\tShow_transaction_nb=\"%d\"\n"
					       "\t\tShow_transaction_date=\"%d\"\n"
					       "\t\tShow_transaction_payee=\"%d\"\n"
					       "\t\tShow_transaction_categ=\"%d\"\n"
					       "\t\tShow_transaction_sub_categ=\"%d\"\n"
					       "\t\tShow_transaction_payment=\"%d\"\n"
					       "\t\tShow_transaction_budget=\"%d\"\n"
					       "\t\tShow_transaction_sub_budget=\"%d\"\n"
					       "\t\tShow_transaction_chq=\"%d\"\n"
					       "\t\tShow_transaction_note=\"%d\"\n"
					       "\t\tShow_transaction_voucher=\"%d\"\n"
					       "\t\tShow_transaction_reconcile=\"%d\"\n"
					       "\t\tShow_transaction_bank=\"%d\"\n"
					       "\t\tShow_transaction_fin_year=\"%d\"\n"
					       "\t\tShow_transaction_sort_type=\"%d\"\n"
					       "\t\tShow_columns_titles=\"%d\"\n"
					       "\t\tShow_title_column_kind=\"%d\"\n"
					       "\t\tShow_exclude_breakdown_child=\"%d\"\n"
					       "\t\tShow_split_amounts=\"%d\"\n"
					       "\t\tCurrency_general=\"%d\"\n"
					       "\t\tReport_in_payees=\"%d\"\n"
					       "\t\tReport_can_click=\"%d\"\n"
					       "\t\tFinancial_year_used=\"%d\"\n"
					       "\t\tFinancial_year_kind=\"%d\"\n"
					       "\t\tFinancial_year_select=\"%s\"\n"
					       "\t\tDate_kind=\"%d\"\n"
					       "\t\tDate_begining=\"%s\"\n"
					       "\t\tDate_end=\"%s\"\n"
					       "\t\tSplit_by_date=\"%d\"\n"
					       "\t\tSplit_date_period=\"%d\"\n"
					       "\t\tSplit_by_fin_year=\"%d\"\n"
					       "\t\tSplit_day_begining=\"%d\"\n"
					       "\t\tAccount_use_selection=\"%d\"\n"
					       "\t\tAccount_selected=\"%s\"\n"
					       "\t\tAccount_group_transactions=\"%d\"\n"
					       "\t\tAccount_show_amount=\"%d\"\n"
					       "\t\tAccount_show_name=\"%d\"\n"
					       "\t\tTransfer_kind=\"%d\"\n"
					       "\t\tTransfer_selected_accounts=\"%s\"\n"
					       "\t\tTransfer_exclude_transactions=\"%d\"\n"
					       "\t\tCateg_use=\"%d\"\n"
					       "\t\tCateg_use_selection=\"%d\"\n"
					       "\t\tCateg_selected=\"%s\"\n"
					       "\t\tCateg_exclude_transactions=\"%d\"\n"
					       "\t\tCateg_show_amount=\"%d\"\n"
					       "\t\tCateg_show_sub_categ=\"%d\"\n"
					       "\t\tCateg_show_without_sub_categ=\"%d\"\n"
					       "\t\tCateg_show_sub_categ_amount=\"%d\"\n"
					       "\t\tCateg_currency=\"%d\"\n"
					       "\t\tCateg_show_name=\"%d\"\n"
					       "\t\tBudget_use=\"%d\"\n"
					       "\t\tBudget_use_selection=\"%d\"\n"
					       "\t\tBudget_selected=\"%s\"\n"
					       "\t\tBudget_exclude_transactions=\"%d\"\n"
					       "\t\tBudget_show_amount=\"%d\"\n"
					       "\t\tBudget_show_sub_budget=\"%d\"\n"
					       "\t\tBudget_show_without_sub_budget=\"%d\"\n"
					       "\t\tBudget_show_sub_budget_amount=\"%d\"\n"
					       "\t\tBudget_currency=\"%d\"\n"
					       "\t\tBudget_show_name=\"%d\"\n"
					       "\t\tPayee_use=\"%d\"\n"
					       "\t\tPayee_use_selection=\"%d\"\n"
					       "\t\tPayee_selected=\"%s\"\n"
					       "\t\tPayee_show_amount=\"%d\"\n"
					       "\t\tPayee_currency=\"%d\"\n"
					       "\t\tPayee_show_name=\"%d\"\n"
					       "\t\tAmount_currency=\"%d\"\n"
					       "\t\tAmount_exclude_null=\"%d\"\n"
					       "\t\tPayment_method_use=\"%d\"\n"
					       "\t\tPayment_method_list=\"%s\"\n"
					       "\t\tUse_text=\"%d\"\n"
					       "\t\tUse_amount=\"%d\" />\n",
	    report_number_to_write,
	    report_name,
	    general_sort_type,
	    gsb_data_report_get_show_r (report_number),
	    gsb_data_report_get_show_report_transactions (report_number),
	    gsb_data_report_get_show_report_transaction_amount (report_number),
	    gsb_data_report_get_show_report_transaction_number (report_number),
	    gsb_data_report_get_show_report_date (report_number),
	    gsb_data_report_get_show_report_payee (report_number),
	    gsb_data_report_get_show_report_category (report_number),
	    gsb_data_report_get_show_report_sub_category (report_number),
	    gsb_data_report_get_show_report_method_of_payment (report_number),
	    gsb_data_report_get_show_report_budget (report_number),
	    gsb_data_report_get_show_report_sub_budget (report_number),
	    gsb_data_report_get_show_report_method_of_payment_content (report_number),
	    gsb_data_report_get_show_report_note (report_number),
	    gsb_data_report_get_show_report_voucher (report_number),
	    gsb_data_report_get_show_report_marked (report_number),
	    gsb_data_report_get_show_report_bank_references (report_number),
	    gsb_data_report_get_show_report_financial_year (report_number),
	    gsb_data_report_get_sorting_report (report_number),
	    gsb_data_report_get_column_title_show (report_number),
	    gsb_data_report_get_column_title_type (report_number),
	    gsb_data_report_get_not_detail_breakdown (report_number),
	    gsb_data_report_get_split_credit_debit (report_number),
	    gsb_data_report_get_currency_general (report_number),
	    gsb_data_report_get_append_in_payee (report_number),
	    gsb_data_report_get_report_can_click (report_number),
	    gsb_data_report_get_use_financial_year (report_number),
	    gsb_data_report_get_financial_year_type (report_number),
	    financial_year_select,
	    gsb_data_report_get_date_type (report_number),
	    date_start,
	    date_end,
	    gsb_data_report_get_period_split (report_number),
	    gsb_data_report_get_period_split_type (report_number),
	    gsb_data_report_get_financial_year_split (report_number),
	    gsb_data_report_get_period_split_day (report_number),
	    gsb_data_report_get_account_use_chosen (report_number),
	    account_selected,
	    gsb_data_report_get_account_group_reports (report_number),
	    gsb_data_report_get_account_show_amount (report_number),
	    gsb_data_report_get_account_show_name (report_number),
	    gsb_data_report_get_transfer_choice (report_number),
	    transfer_selected_accounts,
	    gsb_data_report_get_transfer_reports_only (report_number),
	    gsb_data_report_get_category_used (report_number),
	    gsb_data_report_get_category_detail_used (report_number),
	    categ_selected,
	    gsb_data_report_get_category_only_report_with_category (report_number),
	    gsb_data_report_get_category_show_category_amount (report_number),
	    gsb_data_report_get_category_show_sub_category (report_number),
	    gsb_data_report_get_category_show_without_category (report_number),
	    gsb_data_report_get_category_show_sub_category_amount (report_number),
	    gsb_data_report_get_category_currency (report_number),
	    gsb_data_report_get_category_show_name (report_number),
	    gsb_data_report_get_budget_used (report_number),
	    gsb_data_report_get_budget_detail_used (report_number),
	    budget_selected,
	    gsb_data_report_get_budget_only_report_with_budget (report_number),
	    gsb_data_report_get_budget_show_budget_amount (report_number),
	    gsb_data_report_get_budget_show_sub_budget (report_number),
	    gsb_data_report_get_budget_show_without_budget (report_number),
	    gsb_data_report_get_budget_show_sub_budget_amount (report_number),
	    gsb_data_report_get_budget_currency (report_number),
	    gsb_data_report_get_budget_show_name (report_number),
	    gsb_data_report_get_payee_used (report_number),
	    gsb_data_report_get_payee_detail_used (report_number),
	    payee_selected,
	    gsb_data_report_get_payee_show_payee_amount (report_number),
	    gsb_data_report_get_payee_currency (report_number),
	    gsb_data_report_get_payee_show_name (report_number),
	    gsb_data_report_get_amount_comparison_currency (report_number),
	    gsb_data_report_get_amount_comparison_only_report_non_null (report_number),
	    gsb_data_report_get_method_of_payment_used (report_number),
	    payment_method_list,
	    gsb_data_report_get_text_comparison_used (report_number),
	    gsb_data_report_get_amount_comparison_used (report_number));

	g_free (report_name);
	g_free (general_sort_type);
	g_free (financial_year_select);
	g_free (account_selected);
	g_free (transfer_selected_accounts);
	g_free (categ_selected);
	g_free (budget_selected);
	g_free (payee_selected);
	g_free (payment_method_list);
	g_free (date_start);
	g_free (date_end);

	/* append the new string to the file content
	 * and take the new iterator */
	iterator = gsb_file_save_append_part ( iterator,
					       length_calculated,
					       file_content,
					       new_string );

	/* save the text comparison */

	list_tmp_2 = gsb_data_report_get_text_comparison_list (report_number);

	while ( list_tmp_2 )
	{
	    gint text_comparison_number;

	    text_comparison_number = GPOINTER_TO_INT (list_tmp_2 -> data);

	    /* if current_report is set, this mean we export the report,
	     * we cannot save the number should be automaticly given while
	     * importing, so set the report number to -1 */
	    if (current_report)
		text_comparison_number_to_write = -1;
	    else
		text_comparison_number_to_write = text_comparison_number;

	    /* now we can fill the file content */
	    new_string = g_markup_printf_escaped ( "\t<Text_comparison\n"
						   "\t\tComparison_number=\"%d\"\n"
						   "\t\tReport_nb=\"%d\"\n"
						   "\t\tLast_comparison=\"%d\"\n"
						   "\t\tObject=\"%d\"\n"
						   "\t\tOperator=\"%d\"\n"
						   "\t\tText=\"%s\"\n"
						   "\t\tUse_text=\"%d\"\n"
						   "\t\tComparison_1=\"%d\"\n"
						   "\t\tLink_1_2=\"%d\"\n"
						   "\t\tComparison_2=\"%d\"\n"
						   "\t\tAmount_1=\"%d\"\n"
						   "\t\tAmount_2=\"%d\" />\n",
						   text_comparison_number_to_write,
						   report_number_to_write,
						   gsb_data_report_text_comparison_get_link_to_last_text_comparison (text_comparison_number),
						   gsb_data_report_text_comparison_get_field (text_comparison_number),
						   gsb_data_report_text_comparison_get_operator (text_comparison_number),
						   gsb_data_report_text_comparison_get_text (text_comparison_number),
						   gsb_data_report_text_comparison_get_use_text (text_comparison_number),
						   gsb_data_report_text_comparison_get_first_comparison (text_comparison_number),
						   gsb_data_report_text_comparison_get_link_first_to_second_part (text_comparison_number),
						   gsb_data_report_text_comparison_get_second_comparison (text_comparison_number),
						   gsb_data_report_text_comparison_get_first_amount (text_comparison_number),
						   gsb_data_report_text_comparison_get_second_amount (text_comparison_number));

	    /* append the new string to the file content
	     * and take the new iterator */
	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );

	    list_tmp_2 = list_tmp_2 -> next;
	}

	/* save the amount comparison */

	list_tmp_2 = gsb_data_report_get_amount_comparison_list (report_number);

	while ( list_tmp_2 )
	{
	    gint amount_comparison_number;
	    gchar *first_amount;
	    gchar *second_amount;

	    amount_comparison_number = GPOINTER_TO_INT (list_tmp_2 -> data);

	    /* if current_report is set, this mean we export the report,
	     * we cannot save the number should be automaticly given while
	     * importing, so set the report number to -1 */
	    if (current_report)
		amount_comparison_number_to_write = -1;
	    else
		amount_comparison_number_to_write = amount_comparison_number;

	    /* set the numbers */
	    first_amount = gsb_real_get_string (gsb_data_report_amount_comparison_get_first_amount (amount_comparison_number));
	    second_amount = gsb_real_get_string (gsb_data_report_amount_comparison_get_second_amount (amount_comparison_number));

	    /* now we can fill the file content */
	    new_string = g_markup_printf_escaped ( "\t<Amount_comparison\n"
						   "\t\tComparison_number=\"%d\"\n"
						   "\t\tReport_nb=\"%d\"\n"
						   "\t\tLast_comparison=\"%d\"\n"
						   "\t\tComparison_1=\"%d\"\n"
						   "\t\tLink_1_2=\"%d\"\n"
						   "\t\tComparison_2=\"%d\"\n"
						   "\t\tAmount_1=\"%s\"\n"
						   "\t\tAmount_2=\"%s\" />\n",
						   amount_comparison_number_to_write,
						   report_number_to_write,
						   gsb_data_report_amount_comparison_get_link_to_last_amount_comparison (amount_comparison_number),
						   gsb_data_report_amount_comparison_get_first_comparison (amount_comparison_number),
						   gsb_data_report_amount_comparison_get_link_first_to_second_part (amount_comparison_number),
						   gsb_data_report_amount_comparison_get_second_comparison (amount_comparison_number),
						   first_amount,
						   second_amount);

	    g_free (first_amount);
	    g_free (second_amount);

	    /* append the new string to the file content
	     * and take the new iterator */

	    iterator = gsb_file_save_append_part ( iterator,
						   length_calculated,
						   file_content,
						   new_string );
	    list_tmp_2 = list_tmp_2 -> next;
	}
	list_tmp = list_tmp -> next;
    }
    return iterator;
}




