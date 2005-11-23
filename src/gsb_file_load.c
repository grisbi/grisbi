/* file gsb_file_load.c
 * used to load the gsb files */
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
#include "gsb_file_load.h"
#include "utils_devises.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_payee.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file_util.h"
#include "data_form.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "traitement_variables.h"
#include "fichiers_gestion.h"
#include "utils_files.h"
#include "structures.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "include.h"
#include "echeancier_infos.h"
#include "devises_constants.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean file_io_fix_xml_corrupted_file_lock_tag(gchar* accounts_filename);
static void gsb_file_load_account_part ( const gchar **attribute_names,
				  const gchar **attribute_values );
static void gsb_file_load_account_part_before_0_6 ( GMarkupParseContext *context,
					     const gchar *text );
static void gsb_file_load_bank ( const gchar **attribute_names,
			  const gchar **attribute_values );
static gboolean gsb_file_load_check_new_structure ( gchar *file_content );
static void gsb_file_load_currency ( const gchar **attribute_names,
			      const gchar **attribute_values );
static void gsb_file_load_end_element_before_0_6 ( GMarkupParseContext *context,
					    const gchar *element_name,
					    gpointer user_data,
					    GError **error);
static void gsb_file_load_financial_year ( const gchar **attribute_names,
				    const gchar **attribute_values );
static void gsb_file_load_general_part ( const gchar **attribute_names,
				  const gchar **attribute_values );
static void gsb_file_load_general_part_before_0_6 ( GMarkupParseContext *context,
					     const gchar *text );
static void gsb_file_load_party ( const gchar **attribute_names,
			   const gchar **attribute_values );
static void gsb_file_load_payment_part ( const gchar **attribute_names,
				  const gchar **attribute_values );
static void gsb_file_load_reconcile ( const gchar **attribute_names,
			       const gchar **attribute_values );
static void gsb_file_load_report_part_before_0_6 ( GMarkupParseContext *context,
					    const gchar *text );
static void gsb_file_load_scheduled_transactions ( const gchar **attribute_names,
					    const gchar **attribute_values );
static void gsb_file_load_start_element ( GMarkupParseContext *context,
				   const gchar *element_name,
				   const gchar **attribute_names,
				   const gchar **attribute_values,
				   gpointer user_data,
				   GError **error);
static void gsb_file_load_start_element_before_0_6 ( GMarkupParseContext *context,
					      const gchar *element_name,
					      const gchar **attribute_names,
					      const gchar **attribute_values,
					      gpointer user_data,
					      GError **error);
static void gsb_file_load_text_element_before_0_6 ( GMarkupParseContext *context,
					     const gchar *text,
					     gsize text_len,  
					     gpointer user_data,
					     GError **error);
static void gsb_file_load_transactions ( const gchar **attribute_names,
				  const gchar **attribute_values );
static gboolean gsb_file_load_update_previous_version ( void );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *adr_banque;
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern     gchar * buffer ;
extern gchar *chemin_logo;
extern GtkWidget *code_banque;
extern GtkWidget *email_banque;
extern GtkWidget *email_correspondant;
extern GtkWidget *fax_correspondant;
extern GtkWidget *formulaire;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GSList *liste_struct_banques;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_exercices;
extern GSList *liste_struct_rapprochements;
extern gint nb_colonnes;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkWidget *nom_banque;
extern GtkWidget *nom_correspondant;
extern GtkWidget *nom_exercice;
extern gchar *nom_fichier_backup;
extern GtkWidget *remarque_banque;
extern gint scheduler_col_width[NB_COLS_SCHEDULER] ;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern GtkWidget *tel_banque;
extern GtkWidget *tel_correspondant;
extern gchar *titre_fichier;
extern gint valeur_echelle_recherche_date_import;
extern GtkWidget *web_banque;
extern int no_derniere_devise;
/*END_EXTERN*/

static struct
{
    gboolean download_ok;
    gchar *file_version;
    gchar *grisbi_version;

    /* there is always only one to TRUE, used to split the retrieves functions */

    gboolean general_part;
    gboolean account_part;
    gboolean report_part;

} download_tmp_values;

static gint account_number_tmp;

/* to import older file than 0.6, makes the link between category and sub-category */
static gint last_category = 0;
static gint last_sub_category_number = 0;

/* to import older file than 0.6, makes the link between budget and sub-budget */
static gint last_budget = 0;
static gint last_sub_budget_number = 0;

/* to import older file than 0.6, makes the link between report and comparison structures */
gint last_report_number;

/* used to get the sort of the accounts in the version < 0.6 */
static GSList *sort_accounts;




/*!
 * @brief Try to fix xml corruption introduced by Grisi 0.5.6 rev a during file log management
 * FIXME : normalement inutile, à vérifier pendant les tests de la 0.6 et retirer
 *
 * This function replace corrupted xml end tag "</Fi0hier_ouvert>" by the good one
 * When the function return TRUE, the file can be reloaded for a second try.
 * 
 * @caveats : Should only to be called when the xmlParseFile function call failed with zero as errno value
 *
 * @return Fix application status.
 * @retval TRUE if a corruption has been found and fix applied.
 * @retval FALSE in all other cases.
 *
 */
gboolean file_io_fix_xml_corrupted_file_lock_tag(gchar* accounts_filename)
{
    gboolean fix_applied      = FALSE;
    FILE*    fd_accounts_file = fopen(accounts_filename, "r+b");
    if (fd_accounts_file)
    {
        gchar    buffer [18];
        gint     len        = 17;
        gchar*   valid_tag  = "</Fichier_ouvert>"; 
        gchar*   error0_tag = "</Fi0hier_ouvert>"; 
        gchar*   error1_tag = "</Fi1hier_ouvert>"; 

        while ( EOF != fscanf(fd_accounts_file,"%17s",buffer))
        {
            // The valid version of the tag has been found, the problem is not here 
            if (!strncmp(buffer,valid_tag,len)) { break ; }
                
            // If the corrupted tag is found, rewinf the file to replace it by the valid value.
            if ((!strncmp(buffer,error0_tag,len)) || (!strncmp(buffer,error1_tag,len)) )
            { 
                fseek(fd_accounts_file,-len,SEEK_CUR);
                fprintf(fd_accounts_file,valid_tag);
                fix_applied = TRUE;
                break;
            }
        }
        fclose(fd_accounts_file);
        fd_accounts_file = NULL;
    }
    return fix_applied;
    
}


/**
 * called to open the grisbi file given in param
 *
 * \filename the filename to load with full path
 *
 * \return TRUE if ok
 * */
gboolean gsb_file_load_open_file ( gchar *filename )
{
    struct stat buffer_stat;
    gint return_value;
    gchar *file_content;
    guint length;

    if ( DEBUG )
	printf ( "gsb_file_load_open_file %s\n", 
		 filename );

    /* general check */
    
    if ( !g_file_test ( filename,
			G_FILE_TEST_EXISTS ))
    {
	dialogue_error_hint (g_strdup_printf (_("Cannot open file '%s': %s"),
					      filename,
					      latin2utf8 (strerror(errno))),
			     g_strdup_printf ( _("Error loading file '%s'"), filename));
	remove_file_from_last_opened_files_list (filename);
	return FALSE;
    }

    /* check here if it's not a regular file */

    if ( !g_file_test ( filename,
			G_FILE_TEST_IS_REGULAR ))
    {
	dialogue_error_hint ( g_strdup_printf ( _("%s doesn't seem to be a regular file,\nplease check it and try again."),
						filename ),
			      g_strdup_printf ( _("Error loading file '%s'"), filename));
	remove_file_from_last_opened_files_list (filename);
	return ( FALSE );
    }

     /* fill the buffer stat to check the permission */

    return_value = utf8_stat ( filename,
			       &buffer_stat);
    
    /* check the access to the file and propose to change it */
#ifdef _WIN32
    if ( buffer_stat.st_mode != 33152 )
	gsb_file_util_change_permissions();
#endif /* _WIN32 */

    /* load the file */

    if ( g_file_get_contents ( filename,
			       &file_content,
			       &length,
			       NULL ))
    {
	GMarkupParser *markup_parser = g_malloc0 (sizeof (GMarkupParser));
	GMarkupParseContext *context;
	gulong long_length = 0;

	/* for zlib, need a gulong for size and g_file_get_contents a guint...
	 * perhaps it exists another mean than that ? */

	long_length = length;

	/* first, we check if the file is crypted, if it is, we decrypt it */

	if ( !strncmp ( file_content, "Grisbi encrypted file ", 22 ))
	    if ( !( long_length = gsb_file_util_crypt_file ( filename, &file_content, 
							      FALSE, long_length )))
		return FALSE;

	/* after, we check if the file is compressed, if it is, we uncompress it */

	if ( !strncmp ( file_content, "Grisbi compressed file ", 23 ))
	    if ( !( length = gsb_file_util_compress_file ( &file_content, 
							   long_length,
							   FALSE )))
		return FALSE;

	/* we begin to check if we are in a version under 0.6 or 0.6 and above,
	 * because the xml structure changes after 0.6 */

	if ( gsb_file_load_check_new_structure (file_content))
	{
	    /* fill the GMarkupParser for a new xml structure */

	    markup_parser -> start_element = (void *) gsb_file_load_start_element;
	    markup_parser -> error = (void *) gsb_file_load_error;
	}
	else
	{
	    /* fill the GMarkupParser for the last xml structure */

	    markup_parser -> start_element = (void *) gsb_file_load_start_element_before_0_6;
	    markup_parser -> end_element = (void *) gsb_file_load_end_element_before_0_6;
	    markup_parser -> text = (void *) gsb_file_load_text_element_before_0_6;
	}

	context = g_markup_parse_context_new ( markup_parser,
					       0,
					       NULL,
					       NULL );
	download_tmp_values.download_ok = FALSE;

	g_markup_parse_context_parse ( context,
				       file_content,
				       strlen (file_content),
				       NULL );

	if ( !download_tmp_values.download_ok )
	{
	    g_markup_parse_context_free (context);
	    g_free (markup_parser);
	    g_free (file_content);
	    return FALSE;
	}

	g_markup_parse_context_free (context);
	g_free (markup_parser);
	g_free (file_content);
    }
    else
    {
	dialogue_error_hint (g_strdup_printf (_("Cannot open file '%s': %s"),
					      filename,
					      latin2utf8 (strerror(errno))),
			     g_strdup_printf ( _("Error loading file '%s'"), filename));
	remove_file_from_last_opened_files_list (filename);
	return FALSE;
    }

    return gsb_file_load_update_previous_version();
}


/** check if the xml file is the last structure (before 0.6) or
 * the new structure (after 0.6)
 *
 * \param file_content the grisbi file
 *
 * \return TRUE if the version is after 0.6
 * */
gboolean gsb_file_load_check_new_structure ( gchar *file_content )
{
    if ( strstr ( file_content,
		  "Generalites" ))
	return FALSE;
    return TRUE;
}



void gsb_file_load_start_element ( GMarkupParseContext *context,
				   const gchar *element_name,
				   const gchar **attribute_names,
				   const gchar **attribute_values,
				   gpointer user_data,
				   GError **error)
{
    /* the first time we come here, we check if it's a grisbi file */

    if ( !download_tmp_values.download_ok )
    {
	if ( strcmp ( element_name,
		      "Grisbi" ))
	{
	    dialogue_error ( _("This is not a Grisbi file... Loading aborted.") );
	    g_markup_parse_context_end_parse (context,
					      NULL);
	    return;
	}
	download_tmp_values.download_ok = TRUE;
	return;
    }

    if ( !strcmp ( element_name,
		   "General" ))
    {
	gsb_file_load_general_part ( attribute_names,
				     attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Account" ))
    {
	gsb_file_load_account_part ( attribute_names,
				     attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Payment" ))
    {
	gsb_file_load_payment_part ( attribute_names,
				     attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Transaction" ))
    {
	gsb_file_load_transactions ( attribute_names,
				     attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Scheduled" ))
    {
	gsb_file_load_scheduled_transactions ( attribute_names,
					       attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Party" ))
    {
	gsb_file_load_party ( attribute_names,
			      attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Category" ))
    {
	gsb_file_load_category ( attribute_names,
				 attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Sub_category" ))
    {
	gsb_file_load_sub_category ( attribute_names,
				     attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Budgetary" ))
    {
	gsb_file_load_budgetary ( attribute_names,
				  attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Sub_budgetary" ))
    {
	gsb_file_load_sub_budgetary ( attribute_names,
				      attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Currency" ))
    {
	gsb_file_load_currency ( attribute_names,
				 attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Bank" ))
    {
	gsb_file_load_bank ( attribute_names,
			     attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Financial_year" ))
    {
	gsb_file_load_financial_year ( attribute_names,
				       attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Reconcile" ))
    {
	gsb_file_load_reconcile ( attribute_names,
				  attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Report" ))
    {
	gsb_file_load_report ( attribute_names,
			       attribute_values );
	return;
    }

    if ( !strcmp ( element_name,
		   "Text_comparison" ))
    {
	gsb_file_load_text_comparison ( attribute_names,
					attribute_values);
	return;
    }

    if ( !strcmp ( element_name,
		   "Amount_comparison" ))
    {
	gsb_file_load_amount_comparison ( attribute_names,
					  attribute_values);
	return;
    }
}




void gsb_file_load_error ( GMarkupParseContext *context,
			   GError *error,
			   gpointer user_data )
{
    dialogue_error (g_strdup_printf (_("An error occured while parsing the file :\nError number : %d\n%s"),
				     error -> code,
				     error -> message ));
}



/**
 * load the general part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_general_part ( const gchar **attribute_names,
				  const gchar **attribute_values )
{
    gint i=0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
		      "(null)"))
	{
	    /* Nothing */
	}

	else if ( !strcmp ( attribute_names[i],
			    "File_version" ))
	{
	    download_tmp_values.file_version = g_strdup (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Grisbi_version" ))
	{
	    download_tmp_values.grisbi_version = g_strdup (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Backup_file" ))
	{
	    nom_fichier_backup = g_strdup (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Crypt_file" ))
	{
	    etat.crypt_file = utils_str_atoi (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "File_title" ))
	{
	    titre_fichier = g_strdup (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "General_address" ))
	{
	    adresse_commune = g_strdup (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Second_general_address" ))
	{
	    adresse_secondaire = g_strdup (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Party_list_currency_number" ))
	{
	    no_devise_totaux_tiers = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Category_list_currency_number" ))
	{
	    no_devise_totaux_categ = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Budget_list_currency_number" ))
	{
	    no_devise_totaux_ib = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Scheduler_view" ))
	{
	    affichage_echeances = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Scheduler_custom_number" ))
	{
	    affichage_echeances_perso_nb_libre = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Scheduler_custom_menu" ))
	{
	    affichage_echeances_perso_j_m_a = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Import_interval_search" ))
	{
	    valeur_echelle_recherche_date_import = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Use_logo" ))
	{
	    etat.utilise_logo = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Path_logo" ))
	{
	    chemin_logo = g_strdup (attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Remind_display_per_account" ))
	{
	    etat.retient_affichage_par_compte = utils_str_atoi( g_strdup (attribute_values[i]));
	}

	else if ( !strcmp ( attribute_names[i],
			    "Transactions_view" ))
	{
	    gchar **pointeur_char;
	    gint k;

	    pointeur_char = g_strsplit ( g_strdup (attribute_values[i]),
					 "-",
					 0 );

	    for ( i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
		for ( k = 0 ; k<TRANSACTION_LIST_COL_NB ; k++ )
		    tab_affichage_ope[i][k] = utils_str_atoi ( pointeur_char[i + k*TRANSACTION_LIST_COL_NB]);

	    g_strfreev ( pointeur_char );
	}

	else if ( !strcmp ( attribute_names[i],
			    "One_line_showed" ))
	{
	    ligne_affichage_une_ligne = utils_str_atoi ( attribute_values[i]);
	}

	else if ( !strcmp ( attribute_names[i],
			    "Two_lines_showed" ))
	{
	    gchar **pointeur_char;

	    pointeur_char = g_strsplit ( g_strdup (attribute_values[i]),
					 "-",
					 0 );

	    lignes_affichage_deux_lignes = NULL;
	    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							    GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[0] )));
	    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							    GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[1] )));

	    g_strfreev ( pointeur_char );

	}

	else if ( !strcmp ( attribute_names[i],
			    "Three_lines_showed" ))
	{
	    gchar **pointeur_char;

	    pointeur_char = g_strsplit ( g_strdup (attribute_values[i]),
					 "-",
					 0 );

	    lignes_affichage_trois_lignes = NULL;
	    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							     GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[0] )));
	    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							     GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[1] )));
	    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							     GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[2] )));

	    g_strfreev ( pointeur_char );

	}

	else if ( !strcmp ( attribute_names[i],
			    "Remind_form_per_account" ))
	{
	    etat.formulaire_distinct_par_compte = utils_str_atoi( g_strdup (attribute_values[i]));
	}

	else if ( !strcmp ( attribute_names[i],
			    "Scheduler_column_width_ratio" ))
	{
	    gchar **pointeur_char;
	    gint j;

	    pointeur_char = g_strsplit ( g_strdup (attribute_values[i]),
					 "-",
					 0 );

	    for ( j=0 ; j<NB_COLS_SCHEDULER ; j++ )
		scheduler_col_width[j] = utils_str_atoi ( pointeur_char[j]);

	    g_strfreev ( pointeur_char );

	}

	i++;
    }
    while ( attribute_names[i] );
}



/**
 * load the account part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_account_part ( const gchar **attribute_names,
				  const gchar **attribute_values )
{
    gint i=0;
    gint account_number_tmp = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Name" ))
	{
	    account_number_tmp = gsb_data_account_new ( GSB_TYPE_BANK );
	    gsb_data_account_set_name ( account_number_tmp,
				   g_strdup (attribute_values[i]) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Id" ))
	{
	    if ( strlen (attribute_values[i]))
		gsb_data_account_set_id (account_number_tmp,
				    g_strdup (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Number" ))
	{
	    account_number_tmp = gsb_data_account_set_account_number ( account_number_tmp,
								  utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Owner" ))
	{
	    gsb_data_account_set_holder_name ( account_number_tmp,
					  g_strdup (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Kind" ))
	{
	    gsb_data_account_set_kind (account_number_tmp,
				  utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Currency" ))
	{
	    gsb_data_account_set_currency ( account_number_tmp,
				       utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Bank" ))
	{
	    gsb_data_account_set_bank ( account_number_tmp,
				   utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Bank_branch_code" ))
	{
	    gsb_data_account_set_bank_branch_code ( account_number_tmp,
					       g_strdup (attribute_values[i]) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Bank_account_number" ))
	{
	    gsb_data_account_set_bank_account_number ( account_number_tmp,
						  g_strdup (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Key" ))
	{
	    gsb_data_account_set_bank_account_key ( account_number_tmp,
					       g_strdup (attribute_values[i]) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Initial_balance" ))
	{
	    gsb_data_account_set_init_balance (account_number_tmp,
					  my_strtod ( g_strdup (attribute_values[i]),
						      NULL ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Minimum_wanted_balance" ))
	{
	    gsb_data_account_set_mini_balance_wanted ( account_number_tmp, 
						  my_strtod ( g_strdup (attribute_values[i]),
							      NULL ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Minimum_authorised_balance" ))
	{
	    gsb_data_account_set_mini_balance_authorized ( account_number_tmp, 
						      my_strtod ( g_strdup (attribute_values[i]),
								  NULL ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Last_reconcile_date" ))
	{

	    if ( strlen (attribute_values[i]))
	    {
		gchar **pointeur_char;

		pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );
		gsb_data_account_set_current_reconcile_date ( account_number_tmp,
							 g_date_new_dmy ( utils_str_atoi ( pointeur_char [0] ),
									  utils_str_atoi ( pointeur_char [1] ),
									  utils_str_atoi ( pointeur_char [2] )));
		g_strfreev ( pointeur_char );
	    }
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Last_reconcile_balance" ))
	{
	    gsb_data_account_set_reconcile_balance ( account_number_tmp,
						my_strtod ( g_strdup (attribute_values[i]),
							    NULL ) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Last_reconcile_number" ))
	{
	    gsb_data_account_set_reconcile_last_number ( account_number_tmp,
						    utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Closed_account" ))
	{
	    gsb_data_account_set_closed_account ( account_number_tmp,
					     utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_marked" ))
	{
	    gsb_data_account_set_r ( account_number_tmp,
				     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Lines_per_transaction" ))
	{
	    gsb_data_account_set_nb_rows ( account_number_tmp, 
				      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Comment" ))
	{
	    gsb_data_account_set_comment ( account_number_tmp,
				      g_strdup (attribute_values[i]) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Owner_address" ))
	{
	    gsb_data_account_set_holder_address ( account_number_tmp,
					     g_strdup (attribute_values[i]) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Default_debit_method" ))
	{
	    gsb_data_account_set_default_debit ( account_number_tmp,
					    utils_str_atoi ( attribute_values[i]) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Default_credit_method" ))
	{
	    gsb_data_account_set_default_credit ( account_number_tmp,
					     utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Sort_by_method" ))
	{
	    gsb_data_account_set_reconcile_sort_type ( account_number_tmp,
						  utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Neutrals_inside_method" ))
	{
	    gsb_data_account_set_split_neutral_payment ( account_number_tmp,
						    utils_str_atoi ( attribute_values[i]) );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Sort_order" ))
	{
	    if ( strlen (attribute_values[i]))
	    {
		gchar **pointeur_char;
		gint j;

		pointeur_char = g_strsplit ( attribute_values[i],
					     "/",
					     0 );

		j = 0;

		while ( pointeur_char[j] )
		{
		    gsb_data_account_set_sort_list ( account_number_tmp,
						g_slist_append ( gsb_data_account_get_sort_list (account_number_tmp),
								 GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[j] ))) );
		    j++;
		}
		g_strfreev ( pointeur_char );
	    }
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Ascending_sort" ))
	{
	    gsb_data_account_set_sort_type ( account_number_tmp,
					utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Column_sort" ))
	{
	    gsb_data_account_set_sort_column ( account_number_tmp,
					  utils_str_atoi ( attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Sorting_kind_column" ))
	{
	    gint j;
	    gchar **pointeur_char;

	    pointeur_char = g_strsplit ( attribute_values[i],
					 "-",
					 0 );

	    for ( j=0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	    {
		gsb_data_account_set_column_sort ( account_number_tmp,
					      j,
					      utils_str_atoi ( pointeur_char[j] ));
	    }
	    g_strfreev ( pointeur_char );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Form_columns_number" ))
	{
	    gsb_data_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );
	    gsb_data_account_get_form_organization (account_number_tmp) -> nb_colonnes = utils_str_atoi ( attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Form_lines_number" ))
	{
	    gsb_data_account_get_form_organization (account_number_tmp) -> nb_lignes = utils_str_atoi ( attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Form_organization" ))
	{
	    gchar **pointeur_char;
	    gint k, j;

	    pointeur_char = g_strsplit ( attribute_values[i],
					 "-",
					 0 );

	    for ( k=0 ; k<4 ; k++ )
		for ( j=0 ; j< 6 ; j++ )
		    gsb_data_account_get_form_organization (account_number_tmp) -> tab_remplissage_formulaire[k][j] = utils_str_atoi ( pointeur_char[j + k*6]);

	    g_strfreev ( pointeur_char );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Form_columns_width" ))
	{
	    gchar **pointeur_char;
	    gint j;

	    pointeur_char = g_strsplit ( attribute_values[i],
					 "-",
					 0 );

	    for ( j=0 ; j<6 ; j++ )
		gsb_data_account_get_form_organization (account_number_tmp) -> taille_colonne_pourcent[j] = utils_str_atoi ( pointeur_char[j]);

	    g_strfreev ( pointeur_char );
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}



/**
 * load the payment part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_payment_part ( const gchar **attribute_names,
				  const gchar **attribute_values )
{
    gint i=0;
    struct struct_type_ope *payment_method;

    if ( !attribute_names[i] )
	return;
    
    payment_method = calloc ( 1,
			      sizeof ( struct struct_type_ope ));

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Number" ))
	{
	    payment_method -> no_type = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Name" ))
	{
	    payment_method -> nom_type = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Sign" ))
	{
	    payment_method -> signe_type = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_entry" ))
	{
	    payment_method -> affiche_entree = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Automatic_number" ))
	{
	    payment_method -> numerotation_auto = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Current_number" ))
	{
	    payment_method -> no_en_cours = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Account" ))
	{
	    payment_method -> no_compte = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );

    gsb_data_account_set_method_payment_list ( payment_method -> no_compte,
					  g_slist_append ( gsb_data_account_get_method_payment_list (payment_method -> no_compte),
							   payment_method ));
}


/**
 * load the transactions in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_transactions ( const gchar **attribute_names,
				  const gchar **attribute_values )
{
    gint i=0;
    gint transaction_number = 0;
    gint account_number = 0;

    if ( !attribute_names[i] )
	return;
    

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Ac" ))
	{
	    account_number = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    transaction_number = gsb_data_transaction_new_transaction_with_number ( account_number,
										    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Id" ))
	{
	    gsb_data_transaction_set_transaction_id ( transaction_number,
						      attribute_values[i]);
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Dt" ))
	{
	    gsb_data_transaction_set_date ( transaction_number,
					    gsb_parse_date_string (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Dv" ))
	{
	    gsb_data_transaction_set_value_date ( transaction_number,
						  gsb_parse_date_string (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Am" ))
	{
	    gsb_data_transaction_set_amount ( transaction_number,
					      my_strtod (attribute_values[i],
							 NULL));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Cu" ))
	{
	    gsb_data_transaction_set_currency_number ( transaction_number,
						       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Exb" ))
	{
	    gsb_data_transaction_set_change_between ( transaction_number,
						      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Exr" ))
	{
	    gsb_data_transaction_set_exchange_rate ( transaction_number,
						     my_strtod (attribute_values[i],
								NULL));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Exf" ))
	{
	    gsb_data_transaction_set_exchange_fees ( transaction_number,
						     my_strtod (attribute_values[i],
								NULL));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Pa" ))
	{
	    gsb_data_transaction_set_party_number ( transaction_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Ca" ))
	{
	    gsb_data_transaction_set_category_number ( transaction_number,
						       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Sca" ))
	{
	    gsb_data_transaction_set_sub_category_number ( transaction_number,
							   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Br" ))
	{
	    gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
								utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "No" ))
	{
	    gsb_data_transaction_set_notes ( transaction_number,
					     attribute_values[i]);
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Pn" ))
	{
	    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
								utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Pc" ))
	{
	    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
								 attribute_values[i]);
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Ma" ))
	{
	    gsb_data_transaction_set_marked_transaction ( transaction_number,
							  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Au" ))
	{
	    gsb_data_transaction_set_automatic_transaction ( transaction_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Re" ))
	{
	    gsb_data_transaction_set_reconcile_number ( transaction_number,
							utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Fi" ))
	{
	    gsb_data_transaction_set_financial_year_number ( transaction_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Bu" ))
	{
	    gsb_data_transaction_set_budgetary_number ( transaction_number,
							utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Sbu" ))
	{
	    gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
							    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Vo" ))
	{
	    gsb_data_transaction_set_voucher ( transaction_number,
					       attribute_values[i]);
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Ba" ))
	{
	    gsb_data_transaction_set_bank_references ( transaction_number,
						       attribute_values[i]);
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Trt" ))
	{
	    gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
								   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Tra" ))
	{
	    gsb_data_transaction_set_account_number_transfer ( transaction_number,
							       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}


	if ( !strcmp ( attribute_names[i],
		       "Mo" ))
	{
	    gsb_data_transaction_set_mother_transaction_number ( transaction_number,
								 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}



	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );

}


/**
 * load the scheduled transactions in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_scheduled_transactions ( const gchar **attribute_names,
					    const gchar **attribute_values )
{
    gint i=0;
    gint scheduled_number = 0;

    if ( !attribute_names[i] )
	return;
    
    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    scheduled_number = gsb_data_scheduled_new_scheduled_with_number (utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Dt" ))
	{
	    gsb_data_scheduled_set_date ( scheduled_number,
					  gsb_parse_date_string (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Ac" ))
	{
	    gsb_data_scheduled_set_account_number ( scheduled_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Am" ))
	{
	    gsb_data_scheduled_set_amount ( scheduled_number,
					    my_strtod (attribute_values[i],
						       NULL));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Cu" ))
	{
	    gsb_data_scheduled_set_currency_number ( scheduled_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Pa" ))
	{
	    gsb_data_scheduled_set_party_number ( scheduled_number,
						  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Ca" ))
	{
	    gsb_data_scheduled_set_category_number ( scheduled_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Sca" ))
	{
	    gsb_data_scheduled_set_sub_category_number ( scheduled_number,
							 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Tra" ))
	{
	    gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Pn" ))
	{
	    gsb_data_scheduled_set_method_of_payment_number ( scheduled_number,
							      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "CPn" ))
	{
	    gsb_data_scheduled_set_contra_method_of_payment_number ( scheduled_number,
								     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Pc" ))
	{
	    gsb_data_scheduled_set_method_of_payment_content ( scheduled_number,
							       attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Fi" ))
	{
	    gsb_data_scheduled_set_financial_year_number ( scheduled_number,
							   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Bu" ))
	{
	    gsb_data_scheduled_set_budgetary_number ( scheduled_number,
						      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Sbu" ))
	{
	    gsb_data_scheduled_set_sub_budgetary_number ( scheduled_number,
							  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "No" ))
	{
	    gsb_data_scheduled_set_notes ( scheduled_number,
					   attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Au" ))
	{
	    gsb_data_scheduled_set_automatic_scheduled ( scheduled_number,
							 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Pe" ))
	{
	    gsb_data_scheduled_set_frequency ( scheduled_number,
					       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Pei" ))
	{
	    gsb_data_scheduled_set_user_interval ( scheduled_number,
						   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Pep" ))
	{
	    gsb_data_scheduled_set_user_entry ( scheduled_number,
						utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Dtl" ))
	{
	    gsb_data_scheduled_set_limit_date ( scheduled_number,
						gsb_parse_date_string (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Br" ))
	{
	    gsb_data_scheduled_set_breakdown_of_scheduled ( scheduled_number,
							    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Mo" ))
	{
	    gsb_data_scheduled_set_mother_scheduled_number ( scheduled_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the parties in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_party ( const gchar **attribute_names,
			   const gchar **attribute_values )
{
    gint i=0;
    gint payee_number;

    if ( !attribute_names[i] )
	return;

    payee_number = gsb_data_payee_new (NULL);

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    payee_number = gsb_data_payee_set_new_number ( payee_number,
						      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    gsb_data_payee_set_name ( payee_number,
				 attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Txt" ))
	{
	    gsb_data_payee_set_description ( payee_number,
					attribute_values[i]);
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the categories in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_category ( const gchar **attribute_names,
			      const gchar **attribute_values )
{
    gint i=0;
    gint category_number = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    category_number = gsb_data_category_new_with_number ( utils_str_atoi (attribute_values[i]));

	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    gsb_data_category_set_name ( category_number,
					 attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Kd" ))
	{
	    gsb_data_category_set_type ( category_number,
					 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the sub-categories in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_sub_category ( const gchar **attribute_names,
				  const gchar **attribute_values)
{
    gint i=0;
    gint category_number = 0;
    gint sub_category_number = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nbc" ))
	{
	    category_number = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    sub_category_number = gsb_data_category_new_sub_category_with_number ( utils_str_atoi (attribute_values[i]),
										   category_number );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    gsb_data_category_set_sub_category_name ( category_number,
						      sub_category_number,
						      attribute_values[i] );
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the budgetaries in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_budgetary ( const gchar **attribute_names,
			       const gchar **attribute_values )
{
    gint i=0;
    gint budget_number = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
		      "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    budget_number = gsb_data_budget_new_with_number ( utils_str_atoi (attribute_values[i]));

	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    gsb_data_budget_set_name ( budget_number,
				       attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Kd" ))
	{
	    gsb_data_budget_set_type ( budget_number,
				       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the sub-budgetaries in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_sub_budgetary ( const gchar **attribute_names,
				   const gchar **attribute_values)
{
    gint i=0;
    gint budget_number = 0;
    gint sub_budget_number = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nbb" ))
	{
	    budget_number = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    sub_budget_number = gsb_data_budget_new_sub_budget_with_number ( utils_str_atoi (attribute_values[i]),
									     budget_number );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    gsb_data_budget_set_sub_budget_name ( budget_number,
						  sub_budget_number,
						  attribute_values[i] );
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the currencies in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_currency ( const gchar **attribute_names,
			      const gchar **attribute_values )
{
    gint i=0;
    struct struct_devise *currency;

    if ( !attribute_names[i] )
	return;

    currency = calloc ( 1,
		     sizeof ( struct struct_devise ) );

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    currency -> no_devise = utils_str_atoi (attribute_values[i]);
	    if ( currency -> no_devise > no_derniere_devise )
	    {
		no_derniere_devise = currency -> no_devise;
	    }
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    currency -> nom_devise = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Co" ))
	{
	    currency -> code_devise = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Ico" ))
	{
	    currency -> code_iso4217_devise = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Mte" ))
	{
	    currency -> passage_euro = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Dte" ))
	{
	    currency -> date_dernier_change = gsb_parse_date_string (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Rbc" ))
	{
	    currency -> une_devise_1_egale_x_devise_2 = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Rcu" ))
	{
	    currency -> no_devise_en_rapport = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Ch" ))
	{
	    currency -> change = my_strtod (attribute_values[i],
					    NULL );
	    i++;
	    continue;
	}


	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );

    liste_struct_devises = g_slist_append ( liste_struct_devises,
					    currency );
}

/**
 * load the banks in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_bank ( const gchar **attribute_names,
			  const gchar **attribute_values )
{
    gint i=0;
    struct struct_banque *bank;

    if ( !attribute_names[i] )
	return;

    bank = calloc ( 1,
		    sizeof ( struct struct_banque ) );

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    bank -> no_banque = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    bank -> nom_banque = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Co" ))
	{
	    bank -> code_banque = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Adr" ))
	{
	    bank -> adr_banque = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Tel" ))
	{
	    bank -> tel_banque = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Mail" ))
	{
	    bank -> email_banque = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Web" ))
	{
	    bank -> web_banque = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nac" ))
	{
	    bank -> nom_correspondant = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Faxc" ))
	{
	    bank -> fax_correspondant = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Telc" ))
	{
	    bank -> tel_correspondant = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Mailc" ))
	{
	    bank -> email_correspondant = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Rem" ))
	{
	    bank -> remarque_banque = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}


	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );

    liste_struct_banques = g_slist_append ( liste_struct_banques,
					    bank );
}


/**
 * load the financials years in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_financial_year ( const gchar **attribute_names,
				    const gchar **attribute_values )
{
    gint i=0;
    struct struct_exercice *financial_year;

    if ( !attribute_names[i] )
	return;

    financial_year = calloc ( 1,
			      sizeof ( struct struct_exercice ) );

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    financial_year -> no_exercice = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    financial_year -> nom_exercice = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Bdte" ))
	{
	    financial_year -> date_debut = gsb_parse_date_string (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Edte" ))
	{
	    financial_year -> date_fin = gsb_parse_date_string (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Sho" ))
	{
	    financial_year -> affiche_dans_formulaire = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );

    liste_struct_exercices = g_slist_append ( liste_struct_exercices,
					      financial_year );
}


/**
 * load the reconcile structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_reconcile ( const gchar **attribute_names,
			       const gchar **attribute_values )
{
    struct struct_no_rapprochement *reconcile_struct;
    gint i=0;

    if ( !attribute_names[i] )
	return;

    reconcile_struct = calloc ( 1,
				sizeof ( struct struct_no_rapprochement ) );

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    reconcile_struct -> no_rapprochement = utils_str_atoi (attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Na" ))
	{
	    reconcile_struct -> nom_rapprochement = g_strdup (attribute_values[i]);
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );

    liste_struct_rapprochements = g_slist_append ( liste_struct_rapprochements,
						   reconcile_struct );
}

/**
 * load the report structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_report ( const gchar **attribute_names,
			    const gchar **attribute_values )
{
    gint i=0;
    gint report_number = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Nb" ))
	{
	    report_number = gsb_data_report_new_with_number (utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Name" ))
	{
	    gsb_data_report_set_report_name ( report_number,
					      attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "General_sort_type" ))
	{
	    gsb_data_report_set_sorting_type ( report_number,
					       gsb_string_get_list_from_string (attribute_values[i],
										"/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_r" ))
	{
	    gsb_data_report_set_show_r ( report_number,
					 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction" ))
	{
	    gsb_data_report_set_show_report_transactions ( report_number,
							   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_amount" ))
	{
	    gsb_data_report_set_show_report_transaction_amount ( report_number,
								 utils_str_atoi (attribute_values[i] ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_nb" ))
	{
	    gsb_data_report_set_show_report_transaction_number ( report_number,
								 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_date" ))
	{
	    gsb_data_report_set_show_report_date ( report_number,
						   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_payee" ))
	{
	    gsb_data_report_set_show_report_payee ( report_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_categ" ))
	{
	    gsb_data_report_set_show_report_category ( report_number,
						       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_sub_categ" ))
	{
	    gsb_data_report_set_show_report_sub_category ( report_number,
							   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_payment" ))
	{
	    gsb_data_report_set_show_report_method_of_payment ( report_number,
								utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_budget" ))
	{
	    gsb_data_report_set_show_report_budget ( report_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_sub_budget" ))
	{
	    gsb_data_report_set_show_report_sub_budget ( report_number,
							 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_chq" ))
	{
	    gsb_data_report_set_show_report_method_of_payment_content ( report_number,
									utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_note" ))
	{
	    gsb_data_report_set_show_report_note ( report_number,
						   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_voucher" ))
	{
	    gsb_data_report_set_show_report_voucher ( report_number,
						      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_reconcile" ))
	{
	    gsb_data_report_set_show_report_marked ( report_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_bank" ))
	{
	    gsb_data_report_set_show_report_bank_references ( report_number,
							      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_fin_year" ))
	{
	    gsb_data_report_set_show_report_financial_year ( report_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_transaction_sort_type" ))
	{
	    gsb_data_report_set_sorting_report ( report_number,
						 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_columns_titles" ))
	{
	    gsb_data_report_set_column_title_show ( report_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_title_column_kind" ))
	{
	    gsb_data_report_set_column_title_type ( report_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_exclude_breakdown_child" ))
	{
	    gsb_data_report_set_not_detail_breakdown ( report_number,
						       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Show_split_amounts" ))
	{
	    gsb_data_report_set_split_credit_debit ( report_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Currency_general" ))
	{
	    gsb_data_report_set_currency_general ( report_number,
						   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Report_in_payees" ))
	{
	    gsb_data_report_set_append_in_payee ( report_number,
						  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Report_can_click" ))
	{
	    gsb_data_report_set_report_can_click ( report_number,
						   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Financial_year_used" ))
	{
	    gsb_data_report_set_use_financial_year ( report_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Financial_year_kind" ))
	{
	    gsb_data_report_set_financial_year_type ( report_number,
						      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Financial_year_select" ))
	{
	    gsb_data_report_set_financial_year_list ( report_number,
						      gsb_string_get_list_from_string (attribute_values[i],
										       "/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Date_kind" ))
	{
	    gsb_data_report_set_date_type ( report_number,
					    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Date_begining" ))
	{
	    gsb_data_report_set_personal_date_start ( report_number,
						      gsb_parse_date_string (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Date_end" ))
	{
	    gsb_data_report_set_personal_date_end ( report_number,
						    gsb_parse_date_string (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Split_by_date" ))
	{
	    gsb_data_report_set_period_split ( report_number,
					       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Split_date_period" ))
	{
	    gsb_data_report_set_period_split_type ( report_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Split_by_fin_year" ))
	{
	    gsb_data_report_set_financial_year_split ( report_number,
						       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Split_day_begining" ))
	{
	    gsb_data_report_set_period_split_day ( report_number,
						   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Account_use_selection" ))
	{
	    gsb_data_report_set_account_use_chosen ( report_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Account_selected" ))
	{
	    gsb_data_report_set_account_numbers ( report_number,
						  gsb_string_get_list_from_string (attribute_values[i],
										   "/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Account_group_transactions" ))
	{
	    gsb_data_report_set_account_group_reports ( report_number,
							utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Account_show_amount" ))
	{
	    gsb_data_report_set_account_show_amount ( report_number,
						      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Account_show_name" ))
	{
	    gsb_data_report_set_account_show_name ( report_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Transfer_kind" ))
	{
	    gsb_data_report_set_transfer_choice ( report_number,
						  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Transfer_selected_accounts" ))
	{
	    gsb_data_report_set_transfer_account_numbers ( report_number,
							   gsb_string_get_list_from_string (attribute_values[i],
											    "/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Transfer_exclude_transactions" ))
	{
	    gsb_data_report_set_transfer_reports_only ( report_number,
							utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_use" ))
	{
	    gsb_data_report_set_category_used ( report_number,
						utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_use_selection" ))
	{
	    gsb_data_report_set_category_detail_used ( report_number,
						       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_selected" ))
	{
	    gsb_data_report_set_category_numbers ( report_number,
						   gsb_string_get_list_from_string (attribute_values[i],
										    "/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_exclude_transactions" ))
	{
	    gsb_data_report_set_category_only_report_with_category ( report_number,
								     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_show_amount" ))
	{
	    gsb_data_report_set_category_show_category_amount ( report_number,
								utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_show_sub_categ" ))
	{
	    gsb_data_report_set_category_show_sub_category ( report_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_show_without_sub_categ" ))
	{
	    gsb_data_report_set_category_show_without_category ( report_number,
								 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_show_sub_categ_amount" ))
	{
	    gsb_data_report_set_category_show_sub_category_amount ( report_number,
								    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_currency" ))
	{
	    gsb_data_report_set_category_currency ( report_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Categ_show_name" ))
	{
	    gsb_data_report_set_category_show_name ( report_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_use" ))
	{
	    gsb_data_report_set_budget_used ( report_number,
					      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_use_selection" ))
	{
	    gsb_data_report_set_budget_detail_used ( report_number,
						     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_selected" ))
	{
	    gsb_data_report_set_budget_numbers ( report_number,
						 gsb_string_get_list_from_string (attribute_values[i],
										  "/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_exclude_transactions" ))
	{
	    gsb_data_report_set_budget_only_report_with_budget ( report_number,
								 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_show_amount" ))
	{
	    gsb_data_report_set_budget_show_budget_amount ( report_number,
							    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_show_sub_budget" ))
	{
	    gsb_data_report_set_budget_show_sub_budget ( report_number,
							 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_show_without_sub_budget" ))
	{
	    gsb_data_report_set_budget_show_without_budget ( report_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_show_sub_budget_amount" ))
	{
	    gsb_data_report_set_budget_show_sub_budget_amount ( report_number,
								utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_currency" ))
	{
	    gsb_data_report_set_budget_currency ( report_number,
						  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Budget_show_name" ))
	{
	    gsb_data_report_set_budget_show_name ( report_number,
						   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Payee_use" ))
	{
	    gsb_data_report_set_payee_used ( report_number,
					     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Payee_use_selection" ))
	{
	    gsb_data_report_set_payee_detail_used ( report_number,
						    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Payee_selected" ))
	{
	    gsb_data_report_set_payee_numbers ( report_number,
						gsb_string_get_list_from_string (attribute_values[i],
										 "/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Payee_show_amount" ))
	{
	    gsb_data_report_set_payee_show_payee_amount ( report_number,
							  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Payee_currency" ))
	{
	    gsb_data_report_set_payee_currency ( report_number,
						 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Payee_show_name" ))
	{
	    gsb_data_report_set_payee_show_name ( report_number,
						  utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Amount_currency" ))
	{
	    gsb_data_report_set_amount_comparison_currency ( report_number,
							     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Amount_exclude_null" ))
	{
	    gsb_data_report_set_amount_comparison_only_report_non_null ( report_number,
									 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Payment_method_list" ))
	{
	    gsb_data_report_set_method_of_payment_list ( report_number,
							 gsb_string_get_list_from_string (attribute_values[i],
											  "/-/" ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Use_text" ))
	{
	    gsb_data_report_set_text_comparison_used ( report_number,
						       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Use_amount" ))
	{
	    gsb_data_report_set_amount_comparison_used ( report_number,
							 utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
}

/**
 * load the text comparison structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_text_comparison ( const gchar **attribute_names,
				     const gchar **attribute_values )
{
    gint text_comparison_number = 0;
    gint i=0;
    gint report_number = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
	     "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Comparison_number" ))
	{
	    text_comparison_number = gsb_data_report_text_comparison_new (utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Report_nb" ))
	{
	    report_number = utils_str_atoi (attribute_values[i]);
	    gsb_data_report_text_comparison_set_report_number ( text_comparison_number,
								report_number );
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Last_comparison" ))
	{
	    gsb_data_report_text_comparison_set_link_to_last_text_comparison ( text_comparison_number,
									       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Object" ))
	{
	    gsb_data_report_text_comparison_set_field ( text_comparison_number,
							utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Operator" ))
	{
	    gsb_data_report_text_comparison_set_operator ( text_comparison_number,
							   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Text" ))
	{
	    gsb_data_report_text_comparison_set_text ( text_comparison_number,
						       attribute_values[i]);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Use_text" ))
	{
	    gsb_data_report_text_comparison_set_use_text ( text_comparison_number,
							   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Comparison_1" ))
	{
	    gsb_data_report_text_comparison_set_first_comparison ( text_comparison_number,
								   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Link_1_2" ))
	{
	    gsb_data_report_text_comparison_set_link_first_to_second_part ( text_comparison_number,
									    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Comparison_2" ))
	{
	    gsb_data_report_text_comparison_set_second_comparison ( text_comparison_number,
								    utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Amount_1" ))
	{
	    gsb_data_report_text_comparison_set_first_amount ( text_comparison_number,
							       utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Amount_2" ))
	{
	    gsb_data_report_text_comparison_set_second_amount ( text_comparison_number,
								utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );

    gsb_data_report_set_text_comparison_list ( report_number,
					       g_slist_append ( gsb_data_report_get_text_comparison_list (report_number),
								GINT_TO_POINTER (text_comparison_number)));
}


/**
 * load the amount comparaison structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_amount_comparison ( const gchar **attribute_names,
				       const gchar **attribute_values )
{
    gint i=0;
    gint amount_comparison_number = 0;
    gint report_number = 0;

    if ( !attribute_names[i] )
	return;

    do
    {
	/* 	we test at the begining if the attribute_value is NULL, if yes, */
	/* 	   go to the next */

	if ( !strcmp (attribute_values[i],
		      "(null)"))
	{
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Comparison_number" ))
	{
	    amount_comparison_number = gsb_data_report_amount_comparison_new (utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Report_nb" ))
	{
	    report_number = utils_str_atoi (attribute_values[i]);
	    gsb_data_report_amount_comparison_set_report_number ( amount_comparison_number,
								  report_number);
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Last_comparison" ))
	{
	    gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( amount_comparison_number,
										   utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Comparison_1" ))
	{
	    gsb_data_report_amount_comparison_set_first_comparison ( amount_comparison_number,
								     utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Link_1_2" ))
	{
	    gsb_data_report_amount_comparison_set_link_first_to_second_part ( amount_comparison_number,
									      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Comparison_2" ))
	{
	    gsb_data_report_amount_comparison_set_second_comparison ( amount_comparison_number,
								      utils_str_atoi (attribute_values[i]));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Amount_1" ))
	{
	    gsb_data_report_amount_comparison_set_first_amount ( amount_comparison_number,
								 g_strtod (attribute_values[i],
									   NULL ));
	    i++;
	    continue;
	}

	if ( !strcmp ( attribute_names[i],
		       "Amount_2" ))
	{
	    gsb_data_report_amount_comparison_set_second_amount ( amount_comparison_number,
								  g_strtod (attribute_values[i],
									    NULL ));
	    i++;
	    continue;
	}

	/* normally, shouldn't come here */
	i++;
    }
    while ( attribute_names[i] );
    
    gsb_data_report_set_amount_comparison_list ( report_number,
						 g_slist_append ( gsb_data_report_get_amount_comparison_list (report_number),
								  GINT_TO_POINTER (amount_comparison_number)));
}



/**
 * called after downloading a file, check the version and do the changes if
 * necessary
 * 
 * \param
 * 
 * \return
 * */
gboolean gsb_file_load_update_previous_version ( void )
{
    struct struct_devise *devise;
    struct stat buffer_stat;
    GSList *list_tmp;
    gint i;
    GSList *list_tmp_transactions;
    gint version_number;

    version_number = utils_str_atoi ( g_strjoinv ( "",
						   g_strsplit ( download_tmp_values.file_version,
								".",
								0 )));

    /*     par défaut le fichier n'est pas modifié sauf si on charge une version précédente */

    modification_fichier ( FALSE );

    switch ( version_number )
    {
	/* ************************************* */
	/*     ouverture d'un fichier 0.4.0      */
	/* ************************************* */

	case 40:

	    /* il n'y a aucune différence de struct entre la 0.4.0 et la 0.4.1 */
	    /* sauf que la 0.4.0 n'attribuait pas le no de relevé aux opés filles */
	    /* d'une ventilation */

	    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	    while ( list_tmp_transactions )
	    {
		gint transaction_number_tmp;
		transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		/*  si l'opération est une ventil, on refait le tour de la liste pour trouver ses filles */

		if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number_tmp))
		{
		    GSList *list_tmp_transactions_2;
		    list_tmp_transactions_2 = gsb_data_transaction_get_transactions_list ();

		    while ( list_tmp_transactions_2 )
		    {
			gint transaction_number_tmp_2;
			transaction_number_tmp_2 = gsb_data_transaction_get_transaction_number (list_tmp_transactions_2 -> data);

			if ( gsb_data_transaction_get_account_number (transaction_number_tmp_2) == gsb_data_transaction_get_account_number (transaction_number_tmp)
			     &&
			     gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp_2) == transaction_number_tmp)
			    gsb_data_transaction_set_reconcile_number ( transaction_number_tmp_2,
									gsb_data_transaction_get_reconcile_number (transaction_number_tmp));

			list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
		    }
		}
		list_tmp_transactions = list_tmp_transactions -> next;
	    }


	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.4.1     */
	    /* ************************************* */

	case 41:

	    /*     ajout de la 0.5 -> valeur_echelle_recherche_date_import qu'on me à 2 */

	    valeur_echelle_recherche_date_import = 2;

	    /* 	    passage à l'utf8 : on fait le tour des devises pour retrouver l'euro */
	    /* Handle Euro nicely */

	    devise = devise_par_nom ( g_strdup ("Euro"));

	    if ( devise )
	    {
		devise -> code_devise = "€";
		devise -> code_iso4217_devise = g_strdup ("EUR");
	    }


	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.5.0     */
	    /* ************************************* */

	case 50:
	    /* pour l'instant le fichier 0.5.1 ne diffère pas de la version 0.5.0 */
	    /*     excepté un changement dans la notation du pointage */
	    /*     rien=0 ; P=1 ; T=2 ; R=3 */
	    /*     on fait donc le tour des opés pour inverser R et P */

	    switch_t_r ();

	    /* 	    un bug dans la 0.5.0 permettait à des comptes d'avoir un affichage différent, */
	    /* 	    même si celui ci devait être identique pour tous, on vérifie ici */

	    if ( !etat.retient_affichage_par_compte )
	    {
		gint affichage_r;
		gint nb_lignes_ope;
		GSList *list_tmp;

		affichage_r = gsb_data_account_get_r (gsb_data_account_get_current_account ());
		nb_lignes_ope = gsb_data_account_get_nb_rows ( gsb_data_account_get_current_account () );

		list_tmp = gsb_data_account_get_list_accounts ();

		while ( list_tmp )
		{
		    i = gsb_data_account_get_no_account ( list_tmp -> data );

		    gsb_data_account_set_r ( i,
					affichage_r );
		    gsb_data_account_set_nb_rows ( i, 
					      nb_lignes_ope );

		    list_tmp = list_tmp -> next;
		}
	    } 


    

	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.5.1     */
	    /* ************************************* */

	case 51:

	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.5.5     */
	    /* ************************************* */

	case 55:

	    /* now the order of the accounts are the order in the GSList */

	    gsb_data_account_reorder ( sort_accounts );
	    g_slist_free ( sort_accounts );

	    list_tmp = gsb_data_account_get_list_accounts ();

	    while ( list_tmp )
	    {
		i = gsb_data_account_get_no_account ( list_tmp -> data );

		/* 	    set the form organization at 0 */
		gsb_data_account_set_form_organization ( i,
						    gsb_form_new_organization ());

		/* 	   set the current sort by date and ascending sort */
		init_default_sort_column (i);

		list_tmp = list_tmp -> next;
	    }

	case 56:
	case 57:

	    /* a problem untill the 0.5.7 :
	     * all new method of payment are not added to the sorting list for reconciliation,
	     * we add them here */

	    list_tmp = gsb_data_account_get_list_accounts ();

	    while ( list_tmp )
	    {
		GSList *method_payment_list;

		i = gsb_data_account_get_no_account ( list_tmp -> data );
		method_payment_list = gsb_data_account_get_method_payment_list ( i );

		while (method_payment_list)
		{
		    struct struct_type_ope *type;

		    type = method_payment_list -> data;
		    
		    if ( !g_slist_find ( gsb_data_account_get_sort_list (i),
					 GINT_TO_POINTER (type -> no_type)))
			/* FIXME before 0.6 : faire une fonction add pour les types opés et method of payment */
			gsb_data_account_set_sort_list ( i,
							 g_slist_append ( gsb_data_account_get_sort_list (i),
									  GINT_TO_POINTER (type -> no_type)));

		    method_payment_list = method_payment_list -> next;
		}
		list_tmp = list_tmp -> next;
	    }


	case 58:

	    /* there is a bug untill now, which is some children of breakdown
	     * are not marked R, and the mother is...
	     * very annoying now, we MUST mark them as R, so check here... */

	    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	    while ( list_tmp_transactions )
	    {
		gint transaction_number;
		transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

		/*  if it's a breakdown and marked R, we look for the children */

		if ( gsb_data_transaction_get_breakdown_of_transaction (transaction_number)
		     &&
		     gsb_data_transaction_get_marked_transaction (transaction_number) == 3)
		{
		    GSList *list_tmp_transactions_2;
		    list_tmp_transactions_2 = gsb_data_transaction_get_transactions_list ();

		    while ( list_tmp_transactions_2 )
		    {
			gint transaction_number_2;
			transaction_number_2 = gsb_data_transaction_get_transaction_number (list_tmp_transactions_2 -> data);

			if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_2) == transaction_number)
			    gsb_data_transaction_set_marked_transaction ( transaction_number_2,
									  3 );
			list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
		    }
		}
		list_tmp_transactions = list_tmp_transactions -> next;
	    }




	    /* ********************************************************* */
	    /* 	    à mettre à chaque fois juste avant la version stable */
	    /* ********************************************************* */

	    modification_fichier ( TRUE );

	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.6.0     */
	    /* ************************************* */

	case 60:



	    break;

	default :
	    /* 	à ce niveau, c'est que que la version n'est pas connue de grisbi, on donne alors */
	    /* la version nécessaire pour l'ouvrir */

	    dialogue_error ( g_strdup_printf ( _("Grisbi version %s is needed to open this file.\nYou are using version %s."),
					       download_tmp_values.grisbi_version,
					       VERSION ));

	    return ( FALSE );
    }

    /*     on met maintenant les généralités pour toutes les versions */

    /* 	s'il y avait un ancien logo mais qu'il n'existe plus, on met le logo par défaut */

    if ( !chemin_logo
	 ||
	 !strlen ( chemin_logo )
	 ||
	 ( chemin_logo
	   &&
	   strlen ( chemin_logo )
	   &&
	   utf8_stat ( chemin_logo, &buffer_stat) == -1 ))
	chemin_logo = g_strdup ( LOGO_PATH );

    /* on marque le fichier comme ouvert */

    gsb_file_util_modify_lock ( TRUE );


    return TRUE;
}


void gsb_file_load_start_element_before_0_6 ( GMarkupParseContext *context,
					      const gchar *element_name,
					      const gchar **attribute_names,
					      const gchar **attribute_values,
					      gpointer user_data,
					      GError **error)
{
    /* the first time we come here, we check if it's a grisbi file */

    if ( !download_tmp_values.download_ok )
    {
	if ( strcmp ( element_name,
		      "Grisbi" ))
	{
	    dialogue_error ( _("This is not a Grisbi file... Loading aborted.") );
	    g_markup_parse_context_end_parse (context,
					      NULL);
	    return;
	}
	download_tmp_values.download_ok = TRUE;
	return;
    }

    /* to split the functions, we will set to 1 each time we begin a new part */

    if ( !strcmp ( element_name,
		   "Generalites" )
	 &&
	 !download_tmp_values.account_part
	 &&
	 !download_tmp_values.report_part )
	 {
	     download_tmp_values.general_part = TRUE;
	     return;
	 }

    if ( !strcmp ( element_name,
		   "Comptes" ))
    {
	download_tmp_values.account_part = TRUE;
	return;
    }

    if ( !strcmp ( element_name,
		   "Etats" ))
    {
	download_tmp_values.report_part = TRUE;
	return;
    }


    if ( !strcmp ( element_name,
		   "Type" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_type_ope *type;

	    type = calloc ( 1,
			    sizeof ( struct struct_type_ope ));

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    type -> no_type = utils_str_atoi ( attribute_values[i] );
		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    type -> nom_type = g_strdup ( attribute_values[i] );
		if ( !strcmp ( attribute_names[i],
			       "Signe" ))
		    type -> signe_type = utils_str_atoi ( attribute_values[i] );
		if ( !strcmp ( attribute_names[i],
			       "Affiche_entree" ))
		    type -> affiche_entree = utils_str_atoi ( attribute_values[i] );
		if ( !strcmp ( attribute_names[i],
			       "Numerotation_auto" ))
		    type -> numerotation_auto = utils_str_atoi ( attribute_values[i] );
		if ( !strcmp ( attribute_names[i],
			       "No_en_cours" ))
		    type -> no_en_cours = utils_str_atoi ( attribute_values[i] );

		i++;
	    }
	    while ( attribute_names[i] );

	    type -> no_compte = account_number_tmp;

	    gsb_data_account_set_method_payment_list ( account_number_tmp,
						  g_slist_append ( gsb_data_account_get_method_payment_list (account_number_tmp),
								   type ));
	    return;
	}
    }
     
    if ( !strcmp ( element_name,
		   "Operation" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    gint transaction_number = 0;

	    do
	    {
		gchar **pointeur_char;

		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    transaction_number = gsb_data_transaction_new_transaction_with_number ( account_number_tmp,
											    utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Id" ))
		    gsb_data_transaction_set_transaction_id ( transaction_number,
							       attribute_values[i] );

		if ( !strcmp ( attribute_names[i],
			       "D" ))
		{
		    pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );

		    gsb_data_transaction_set_date ( transaction_number,
						    g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
								     utils_str_atoi ( pointeur_char[1] ),
								     utils_str_atoi ( pointeur_char[2] )));
		    g_strfreev ( pointeur_char );
		}

		if ( !strcmp ( attribute_names[i],
			       "Db" ))
		{
		    if ( attribute_values[i] )
		    {
			pointeur_char = g_strsplit ( attribute_values[i],
						     "/",
						     0 );

			if ( utils_str_atoi ( pointeur_char[0] ))
			    gsb_data_transaction_set_value_date ( transaction_number,
								  g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
										   utils_str_atoi ( pointeur_char[1] ),
										   utils_str_atoi ( pointeur_char[2] )));

			g_strfreev ( pointeur_char );
		    }
		}

		if ( !strcmp ( attribute_names[i],
			       "M" ))
		    gsb_data_transaction_set_amount ( transaction_number,
						      my_strtod ( attribute_values[i],
								  NULL ));

		if ( !strcmp ( attribute_names[i],
			       "De" ))
		    gsb_data_transaction_set_currency_number ( transaction_number,
							       utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Rdc" ))
		    gsb_data_transaction_set_change_between ( transaction_number,
							      utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Tc" ))
		    gsb_data_transaction_set_exchange_rate ( transaction_number,
							     my_strtod ( attribute_values[i],
									 NULL ) );

		if ( !strcmp ( attribute_names[i],
			       "Fc" ))
		    gsb_data_transaction_set_exchange_fees ( transaction_number,
							     my_strtod ( attribute_values[i],
									 NULL ) );

		if ( !strcmp ( attribute_names[i],
			       "T" ))
		    gsb_data_transaction_set_party_number ( transaction_number,
							    utils_str_atoi ( attribute_values[i])  );

		if ( !strcmp ( attribute_names[i],
			       "C" ))
		    gsb_data_transaction_set_category_number ( transaction_number,
							       utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Sc" ))
		    gsb_data_transaction_set_sub_category_number ( transaction_number,
								   utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Ov" ))
		    gsb_data_transaction_set_breakdown_of_transaction ( transaction_number,
									utils_str_atoi ( attribute_values[i]) );

		if ( !strcmp ( attribute_names[i],
			       "N" ))
		    gsb_data_transaction_set_notes ( transaction_number,
						     attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Ty" ))
		    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
									utils_str_atoi ( attribute_values[i]) );

		if ( !strcmp ( attribute_names[i],
			       "Ct" ))
		    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
									 attribute_values[i] );

		if ( !strcmp ( attribute_names[i],
			       "P" ))
		    gsb_data_transaction_set_marked_transaction ( transaction_number,
								  utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "A" ))
		    gsb_data_transaction_set_automatic_transaction ( transaction_number,
								     utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "R" ))
		    gsb_data_transaction_set_reconcile_number ( transaction_number,
								utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "E" ))
		    gsb_data_transaction_set_financial_year_number ( transaction_number,
								     utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "I" ))
		    gsb_data_transaction_set_budgetary_number ( transaction_number,
								utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Si" ))
		    gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
								    utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Pc" ))
		    gsb_data_transaction_set_voucher ( transaction_number,
						       attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Ibg" ))
		    gsb_data_transaction_set_bank_references ( transaction_number,
							       attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Ro" ))
		    gsb_data_transaction_set_transaction_number_transfer ( transaction_number,
									   utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Rc" ))
		    gsb_data_transaction_set_account_number_transfer ( transaction_number,
								       utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Va" ))
		    gsb_data_transaction_set_mother_transaction_number ( transaction_number,
									 utils_str_atoi ( attribute_values[i]));

		i++;
	    }
	    while ( attribute_names[i] );
	}
    }

    if ( !strcmp ( element_name,
		   "Echeance" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    gint scheduled_number = 0;

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    scheduled_number = gsb_data_scheduled_new_scheduled_with_number (utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Date" ))
		    gsb_data_scheduled_set_date ( scheduled_number,
						  gsb_parse_date_string (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Compte" ))
		    gsb_data_scheduled_set_account_number ( scheduled_number,
							    utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Montant" ))
		    gsb_data_scheduled_set_amount ( scheduled_number,
						    my_strtod (attribute_values[i],
							       NULL));

		if ( !strcmp ( attribute_names[i],
			       "Devise" ))
		{
		    gsb_data_scheduled_set_currency_number ( scheduled_number,
							     utils_str_atoi (attribute_values[i]));
		    if ( utils_str_atoi (attribute_values[i]) > no_derniere_devise )
		    {
			no_derniere_devise = utils_str_atoi (attribute_values[i]);
		    }
		}

		if ( !strcmp ( attribute_names[i],
			       "Tiers" ))
		    gsb_data_scheduled_set_party_number ( scheduled_number,
							  utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Categorie" ))
		    gsb_data_scheduled_set_category_number ( scheduled_number,
							     utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Sous-categorie" ))
		    gsb_data_scheduled_set_sub_category_number ( scheduled_number,
								 utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Virement_compte" ))
		    gsb_data_scheduled_set_account_number_transfer ( scheduled_number,
								     utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Type" ))
		    gsb_data_scheduled_set_method_of_payment_number ( scheduled_number,
								      utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Type_contre_ope" ))
		    gsb_data_scheduled_set_contra_method_of_payment_number ( scheduled_number,
									     utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Contenu_du_type" ))
		    gsb_data_scheduled_set_method_of_payment_content ( scheduled_number,
								       attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Exercice" ))
		    gsb_data_scheduled_set_financial_year_number ( scheduled_number,
								   utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Imputation" ))
		    gsb_data_scheduled_set_budgetary_number ( scheduled_number,
							      utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Sous-imputation" ))
		    gsb_data_scheduled_set_sub_budgetary_number ( scheduled_number,
								  utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Notes" ))
		    gsb_data_scheduled_set_notes ( scheduled_number,
						   attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Automatique" ))
		    gsb_data_scheduled_set_automatic_scheduled ( scheduled_number,
								 utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Periodicite" ))
		{
		    gsb_data_scheduled_set_frequency ( scheduled_number,
						       utils_str_atoi (attribute_values[i]));

		    /* Compatibility issue, we inserted two entries
		     * after month view. */
		    if ( gsb_data_scheduled_get_frequency (scheduled_number) > SCHEDULER_PERIODICITY_MONTH_VIEW )
		    {
			gsb_data_scheduled_set_frequency ( scheduled_number,
							   gsb_data_scheduled_get_frequency (scheduled_number)+ 2 );
		    }
		}

		if ( !strcmp ( attribute_names[i],
			       "Intervalle_periodicite" ))
		    gsb_data_scheduled_set_user_interval ( scheduled_number,
							   utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Periodicite_personnalisee" ))
		    gsb_data_scheduled_set_user_entry ( scheduled_number,
							utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Date_limite" ))
		    gsb_data_scheduled_set_limit_date ( scheduled_number,
							gsb_parse_date_string (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Ech_ventilee" ))
		    gsb_data_scheduled_set_breakdown_of_scheduled ( scheduled_number,
								    utils_str_atoi (attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "No_ech_associee" ))
		    gsb_data_scheduled_set_mother_scheduled_number ( scheduled_number,
								     utils_str_atoi (attribute_values[i]));

		i++;
	    }
	    while ( attribute_names[i] );
	}
    }


    if ( !strcmp ( element_name,
		   "Tiers" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    gint payee_number;

	    payee_number = gsb_data_payee_new (NULL);

	    do
	    {
		/* 	we test at the begining if the attribute_value is NULL, if yes, */
		/* 	   go to the next */

		if ( !strcmp (attribute_values[i],
			      "(null)"))
		{
		    i++;
		    continue;
		}

		if ( !strcmp ( attribute_names[i],
			       "No" ))
		{
		    payee_number = gsb_data_payee_set_new_number ( payee_number,
							      utils_str_atoi (attribute_values[i]));
		    i++;
		    continue;
		}

		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		{
		    gsb_data_payee_set_name ( payee_number,
					 attribute_values[i]);
		    i++;
		    continue;
		}

		if ( !strcmp ( attribute_names[i],
			       "Informations" ))
		{
		    gsb_data_payee_set_description ( payee_number,
						attribute_values[i]);
		    i++;
		    continue;
		}

		/* normally, shouldn't come here */
		i++;
	    }
	    while ( attribute_names[i] );
	}
    }


    if ( !strcmp ( element_name,
		   "Categorie" ))
    {
	gint i = 0;

	while ( attribute_names[i] )
	{
	    if ( !strcmp ( attribute_names[i],
			   "No" ))
		last_category = gsb_data_category_new_with_number ( utils_str_atoi (attribute_values[i])); 

	    if ( !strcmp ( attribute_names[i],
			   "Nom" ))
		gsb_data_category_set_name ( last_category,
					     attribute_values[i]);

	    if ( !strcmp ( attribute_names[i],
			   "Type" ))
		gsb_data_category_set_type ( last_category,
					     utils_str_atoi (attribute_values[i]));
	    i++;
	}
    }


    if ( !strcmp ( element_name,
		   "Sous-categorie" ))
    {
	gint i = 0;

	/* each sub-category is stored after a category, so last_category should be filled before */

	while ( attribute_names[i] )
	{
	    if ( !strcmp ( attribute_names[i],
			   "No" ))
		last_sub_category_number = gsb_data_category_new_sub_category_with_number ( utils_str_atoi (attribute_values[i]),
											    last_category);

	    if ( !strcmp ( attribute_names[i],
			   "Nom" ))
		gsb_data_category_set_sub_category_name ( last_category,
							  last_sub_category_number,
							  attribute_values[i]);
	    i++;
	}
    }


    if ( !strcmp ( element_name,
		   "Imputation" ))
    {
	gint i = 0;

	while ( attribute_names[i] )
	{
	    if ( !strcmp ( attribute_names[i],
			   "No" ))
		last_budget = gsb_data_budget_new_with_number ( utils_str_atoi (attribute_values[i])); 

	    if ( !strcmp ( attribute_names[i],
			   "Nom" ))
		gsb_data_budget_set_name ( last_budget,
					   attribute_values[i]);

	    if ( !strcmp ( attribute_names[i],
			   "Type" ))
		gsb_data_budget_set_type ( last_budget,
					   utils_str_atoi (attribute_values[i]));
	    i++;
	}
    }


    if ( !strcmp ( element_name,
		   "Sous-imputation" ))
    {
	gint i = 0;

	/* each sub-budget is stored after a budget, so last_budget should be filled before */

	while ( attribute_names[i] )
	{
	    if ( !strcmp ( attribute_names[i],
			   "No" ))
		last_sub_budget_number = gsb_data_budget_new_sub_budget_with_number ( utils_str_atoi (attribute_values[i]),
										      last_budget);

	    if ( !strcmp ( attribute_names[i],
			   "Nom" ))
		gsb_data_budget_set_sub_budget_name ( last_budget,
						      last_sub_budget_number,
						      attribute_values[i]);
	    i++;
	}
    }


    if ( !strcmp ( element_name,
		   "Devise" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_devise *devise;

	    devise = calloc ( 1,
			      sizeof ( struct struct_devise ));


	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    devise -> no_devise = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    devise -> nom_devise = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "IsoCode" )
		     &&
		     strlen (attribute_values[i]))
		    devise -> code_iso4217_devise = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Code" )
		     &&
		     strlen (attribute_values[i]))
		    devise -> code_devise = g_strdup (attribute_values[i]);

		/* 	   la suite n'est utile que pour les anciennes devises qui sont passées à l'euro */
		/* 	non utilisées pour les autres */

		if ( !strcmp ( attribute_names[i],
			       "Passage_euro" ))
		    devise -> passage_euro = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Date_dernier_change" )
		     &&
		     strlen ( attribute_values[i]))
		{
		    gchar **pointeur_char;

		    pointeur_char = g_strsplit ( attribute_values[i],
						 "/",
						 0 );

		    devise -> date_dernier_change = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
								     utils_str_atoi ( pointeur_char[1] ),
								     utils_str_atoi ( pointeur_char[2] ));
		    g_strfreev ( pointeur_char );
		}

		if ( !strcmp ( attribute_names[i],
			       "Rapport_entre_devises" ))
		    devise -> une_devise_1_egale_x_devise_2 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Devise_en_rapport" ))
		    devise -> no_devise_en_rapport = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Change" ))
		    devise -> change = my_strtod ( attribute_values[i],
						   NULL );

		i++;
	    }
	    while ( attribute_names[i] );

	    /* Set ISO4217 code if needed (compatibility function). */
	    if ( ! devise -> code_iso4217_devise || 
		 ! strlen ( devise -> code_iso4217_devise ) )
	    {
		int i;
		for ( i = 0; iso_4217_currencies[i].continent != NULL; i++ )
		{
		    /* Basically, we check if either currency currency
		     * name or nickname (code) match iso4217 *
		     * currencies code or nickname.  If so, we set
		     * iso4217 code accordingly. */
		    if ( (devise -> nom_devise && 
			  ! strcmp ( devise -> nom_devise, _(iso_4217_currencies[i].currency_name ) ) ) ||
			 ( devise -> code_devise && 
			   ! strcmp ( devise -> code_devise, iso_4217_currencies[i].currency_code ) ) ||
			 ( iso_4217_currencies[i].currency_nickname && 
			   devise -> code_devise &&
			   ! strcmp ( devise -> code_devise, iso_4217_currencies[i].currency_nickname ) ) )
		    {
			devise -> code_iso4217_devise = iso_4217_currencies[i].currency_code;
			break;
		    }				 
		}
	    }

	    liste_struct_devises = g_slist_append ( liste_struct_devises,
						    devise );
	}
    }


    if ( !strcmp ( element_name,
		   "Banque" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_banque *banque;

	    banque = calloc ( 1,
			      sizeof ( struct struct_banque ));

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    banque -> no_banque = utils_str_atoi ( attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    banque -> nom_banque = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Code" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> code_banque = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Adresse" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> adr_banque = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Tel" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> tel_banque = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Mail" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> email_banque = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Web" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> web_banque = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Nom_correspondant" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> nom_correspondant = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Fax_correspondant" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> fax_correspondant = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Tel_correspondant" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> tel_correspondant = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Mail_correspondant" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> email_correspondant = g_strdup (attribute_values[i]);
		
		if ( !strcmp ( attribute_names[i],
			       "Remarques" )
		     &&
		     strlen (attribute_values[i]))
		    banque -> remarque_banque = g_strdup (attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    liste_struct_banques = g_slist_append ( liste_struct_banques,
						    banque );
	}
    }


    if ( !strcmp ( element_name,
		   "Exercice" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_exercice *exercice;

	    exercice = calloc ( 1,
				sizeof ( struct struct_exercice ));

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    exercice -> no_exercice = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    exercice -> nom_exercice = g_strdup ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Date_debut" )
		     &&
		     strlen (attribute_values[i]))
		{
		    gchar **pointeur_char;

		    pointeur_char = g_strsplit ( attribute_values[i],
						 "/",
						 0 );

		    exercice -> date_debut = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
							      utils_str_atoi ( pointeur_char[1] ),
							      utils_str_atoi ( pointeur_char[2] ));
		    g_strfreev ( pointeur_char );
		}

		if ( !strcmp ( attribute_names[i],
			       "Date_fin" )
		     &&
		     strlen (attribute_values[i]))
		{
		    gchar **pointeur_char;

		    pointeur_char = g_strsplit ( attribute_values[i],
						 "/",
						 0 );

		    exercice -> date_fin = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
							    utils_str_atoi ( pointeur_char[1] ),
							    utils_str_atoi ( pointeur_char[2] ));
		    g_strfreev ( pointeur_char );
		}

		if ( !strcmp ( attribute_names[i],
			       "Affiche" ))
		    exercice -> affiche_dans_formulaire = utils_str_atoi ( attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    liste_struct_exercices = g_slist_append ( liste_struct_exercices,
						      exercice );
	}
    }


    if ( !strcmp ( element_name,
		   "Rapprochement" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_no_rapprochement *rapprochement;

	    rapprochement = calloc ( 1,
				     sizeof ( struct struct_no_rapprochement ));

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    rapprochement -> no_rapprochement = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    rapprochement -> nom_rapprochement = g_strdup ( attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    liste_struct_rapprochements = g_slist_append ( liste_struct_rapprochements,
							   rapprochement );
	}
    }

    if ( !strcmp ( element_name,
		   "Comp" )
	 &&
	 attribute_names[1]
	 &&
	 !strcmp ( attribute_names[1],
		   "Champ" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    gint text_comparison_number;

	    /* no number for version <0.6 */
	    text_comparison_number = gsb_data_report_text_comparison_new (0);
	    gsb_data_report_text_comparison_set_report_number ( text_comparison_number,
								last_report_number );

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "Lien_struct" ))
		    gsb_data_report_text_comparison_set_link_to_last_text_comparison ( text_comparison_number,
										       utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Champ" ))
		    gsb_data_report_text_comparison_set_field ( text_comparison_number,
								utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Op" ))
		    gsb_data_report_text_comparison_set_operator ( text_comparison_number,
								   utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Txt" ))
		    gsb_data_report_text_comparison_set_text ( text_comparison_number,
							       attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Util_txt" ))
		    gsb_data_report_text_comparison_set_use_text ( text_comparison_number,
								   utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Comp_1" ))
		    gsb_data_report_text_comparison_set_first_comparison ( text_comparison_number,
									   utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Lien_1_2" ))
		    gsb_data_report_text_comparison_set_link_first_to_second_part ( text_comparison_number,
										    utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Comp_2" ))
		    gsb_data_report_text_comparison_set_second_comparison ( text_comparison_number,
									    utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Mont_1" ))
		    gsb_data_report_text_comparison_set_first_amount ( text_comparison_number,
								       utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Mont_2" ))
		    gsb_data_report_text_comparison_set_second_amount ( text_comparison_number,
									utils_str_atoi ( attribute_values[i]));

		i++;
	    }
	    while ( attribute_names[i] );

	    gsb_data_report_set_text_comparison_list ( last_report_number,
						       g_slist_append ( gsb_data_report_get_text_comparison_list (last_report_number),
									GINT_TO_POINTER (text_comparison_number)));
	}
	return;
    }

    if ( !strcmp ( element_name,
		   "Comp" )
	 &&
	 attribute_names[1]
	 &&
	 !strcmp ( attribute_names[1],
		   "Comp_1" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    gint amount_comparison_number;

	    /* no number for version <0.6 */
	    amount_comparison_number = gsb_data_report_amount_comparison_new (0);
	    gsb_data_report_amount_comparison_set_report_number ( amount_comparison_number,
								  last_report_number );

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "Lien_struct" ))
		    gsb_data_report_amount_comparison_set_link_to_last_amount_comparison ( amount_comparison_number,
											   utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Comp_1" ))
		    gsb_data_report_amount_comparison_set_first_comparison ( amount_comparison_number,
									     utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Lien_1_2" ))
		    gsb_data_report_amount_comparison_set_link_first_to_second_part ( amount_comparison_number,
										      utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Comp_2" ))
		    gsb_data_report_amount_comparison_set_second_comparison ( amount_comparison_number,
									      utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "Mont_1" ))
		    gsb_data_report_amount_comparison_set_first_amount ( amount_comparison_number,
									 my_strtod ( attribute_values[i],
										     NULL ));
		if ( !strcmp ( attribute_names[i],
			       "Mont_2" ))
		    gsb_data_report_amount_comparison_set_second_amount ( amount_comparison_number,
									  my_strtod ( attribute_values[i],
										      NULL ));
		i++;
	    }
	    while ( attribute_names[i] );

	    gsb_data_report_set_amount_comparison_list ( last_report_number,
							 g_slist_append ( gsb_data_report_get_amount_comparison_list (last_report_number),
									  GINT_TO_POINTER (amount_comparison_number)));
	    return;
	}
    }



    if ( !strcmp ( element_name,
		   "Mode_paie" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    gsb_data_report_set_method_of_payment_list ( last_report_number,
								 g_slist_append ( gsb_data_report_get_method_of_payment_list (last_report_number),
										  g_strdup (attribute_values[i])));

		i++;
	    }
	    while ( attribute_names[i] );
	}
	return;
    }
}



void gsb_file_load_end_element_before_0_6 ( GMarkupParseContext *context,
					    const gchar *element_name,
					    gpointer user_data,
					    GError **error)
{
    /* when it's the end of an element, we set it in the split structure to 0 */

    if ( !strcmp ( element_name,
		   "Generalites" ))
	download_tmp_values.general_part = FALSE;
    if ( !strcmp ( element_name,
		   "Comptes" ))
	download_tmp_values.account_part = FALSE;
    if ( !strcmp ( element_name,
		   "Etats" ))
	download_tmp_values.report_part = FALSE;
}

void gsb_file_load_text_element_before_0_6 ( GMarkupParseContext *context,
					     const gchar *text,
					     gsize text_len,  
					     gpointer user_data,
					     GError **error)
{
    /* we come here for all text element, we split here to go
     * on the necessary function to work with that element */

    if ( download_tmp_values.general_part )
	gsb_file_load_general_part_before_0_6 ( context,
						text );
    if ( download_tmp_values.account_part )
	gsb_file_load_account_part_before_0_6 ( context,
						text );
    if ( download_tmp_values.report_part )
	gsb_file_load_report_part_before_0_6 ( context,
					       text );
}

void gsb_file_load_general_part_before_0_6 ( GMarkupParseContext *context,
					     const gchar *text )
{
    const gchar *element_name;

    element_name = g_markup_parse_context_get_element ( context );

    /* check here if we are not between 2 subsections */

    if ( !strcmp ( element_name,
		   "Generalites" ))
	return;

    if ( !strcmp ( element_name,
		   "Version_fichier" ))
    {
	download_tmp_values.file_version = g_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Version_grisbi" ))
    {
	download_tmp_values.grisbi_version = g_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Backup" ))
    {
	nom_fichier_backup = g_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Titre" ))
    {
	titre_fichier = g_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Adresse_commune" ))
    {
	adresse_commune = g_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Adresse_secondaire" ))
    {
	adresse_secondaire = g_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Numero_devise_totaux_tiers" ))
    {
	no_devise_totaux_tiers = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_affichage_des_echeances" ))
    {
	affichage_echeances = utils_str_atoi ( text);

	/* Compatibility issue. */
	switch ( affichage_echeances )
	{
	    case 0: affichage_echeances = SCHEDULER_PERIODICITY_MONTH_VIEW; break;
	    case 1: affichage_echeances = SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW; break;
	    case 2: affichage_echeances = SCHEDULER_PERIODICITY_YEAR_VIEW; break;
	    case 3: affichage_echeances = SCHEDULER_PERIODICITY_ONCE_VIEW; break;
	    case 4: affichage_echeances = SCHEDULER_PERIODICITY_CUSTOM_VIEW; break;
	}

	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_echeances_perso_nb_libre" ))
    {
	affichage_echeances_perso_nb_libre = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_affichage_perso_echeances" ))
    {
	affichage_echeances_perso_j_m_a = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Echelle_date_import" ))
    {
	valeur_echelle_recherche_date_import = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Utilise_logo" ))
    {
	etat.utilise_logo = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Chemin_logo" ))
    {
	chemin_logo = g_strdup (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Caracteristiques_par_compte" ))
    {
	etat.retient_affichage_par_compte = utils_str_atoi( g_strdup (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_opes" ))
    {
	gchar **pointeur_char;
	gint i, j;
	gint number_columns;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	/* there is a pb here to go from 0.5.5 and before, untill 0.6.0
	 * because the nb of columns goes from 8 to 9 ; the best is to
	 * check how much numbers there is and to divide it by TRANSACTION_LIST_ROWS_NB
	 * so we'll have the last nb of columns. it will work event if we increase again
	 * the number of columns, but we need to find another way if TRANSACTION_LIST_ROWS_NB
	 * increases */

	i = 0;
	while (pointeur_char[i])
	    i++;
	number_columns = i/TRANSACTION_LIST_ROWS_NB;

	for ( i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	    for ( j=0 ; j<= number_columns ; j++ )
	    {
		/* we have to check here because if one time we change TRANSACTION_LIST_ROWS_NB or
		 * TRANSACTION_LIST_COL_NB, it will crash without that (ex : (5.5 -> 6.0 )) */
		if (  pointeur_char[j + i*TRANSACTION_LIST_COL_NB] )
		    tab_affichage_ope[i][j] = utils_str_atoi ( pointeur_char[j + i*number_columns]);
		else
		    j = TRANSACTION_LIST_COL_NB;
	    }

	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Ligne_aff_une_ligne" ))
    {
	ligne_affichage_une_ligne = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Lignes_aff_deux_lignes" ))
    {
	gchar **pointeur_char;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	lignes_affichage_deux_lignes = NULL;
	lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[0] )));
	lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[1] )));

	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Lignes_aff_trois_lignes" ))
    {
	gchar **pointeur_char;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	lignes_affichage_trois_lignes = NULL;
	lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							 GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[0] )));
	lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							 GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[1] )));
	lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							 GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[2] )));

	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Formulaire_distinct_par_compte" ))
    {
	etat.formulaire_distinct_par_compte = utils_str_atoi( g_strdup (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Rapport_largeur_col_echeancier" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
	    scheduler_col_width[i] = utils_str_atoi ( pointeur_char[i]);

	g_strfreev ( pointeur_char );
	return;
    }
}


void gsb_file_load_account_part_before_0_6 ( GMarkupParseContext *context,
					     const gchar *text )
{
    const gchar *element_name;

    element_name = g_markup_parse_context_get_element ( context );

    /* check here if we are not between 2 subsections or if we
     * needn't that section */

    if ( !strcmp ( element_name,
		    "Comptes" )
	 ||
	 !strcmp ( element_name,
		   "Compte" )
	 ||
	 !strcmp ( element_name,
		   "Detail_de_Types" )
	 ||
	 !strcmp ( element_name,
		   "Detail_des_operations" )
	 ||
	 !strcmp ( element_name,
		   "Details" ))
	return;

    if ( !strcmp ( element_name,
		   "Ordre_des_comptes" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( text,
				     "-",
				     0 );

	i = 0;
	sort_accounts = NULL;

	while ( pointeur_char[i] )
	{
	    sort_accounts = g_slist_append ( sort_accounts,
					     GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );

	return;
    }


    if ( !strcmp ( element_name,
		   "Nom" ))
    {
	account_number_tmp = gsb_data_account_new ( GSB_TYPE_BANK );
	gsb_data_account_set_name ( account_number_tmp,
			       g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Id_compte" ))
    {
	gsb_data_account_set_id (account_number_tmp,
			    g_strdup (text));
	if ( !strlen ( gsb_data_account_get_id (account_number_tmp)))
	    gsb_data_account_set_id (account_number_tmp,
				NULL );
	return;
    }

    /* 			    we change here the default number of the account */

    if ( !strcmp ( element_name,
		   "No_de_compte" ))
    {
	account_number_tmp = gsb_data_account_set_account_number ( account_number_tmp,
							      utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Titulaire" ))
    {
	gsb_data_account_set_holder_name ( account_number_tmp,
				      g_strdup (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_de_compte" ))
    {
	gsb_data_account_set_kind (account_number_tmp,
			      utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise" ))
    {
	gsb_data_account_set_currency ( account_number_tmp,
				   utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Banque" ))
    {
	gsb_data_account_set_bank ( account_number_tmp,
			       utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Guichet" ))
    {
	gsb_data_account_set_bank_branch_code ( account_number_tmp,
					   g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "No_compte_banque" ))
    {
	gsb_data_account_set_bank_account_number ( account_number_tmp,
					      g_strdup (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Cle_du_compte" ))
    {
	gsb_data_account_set_bank_account_key ( account_number_tmp,
					   g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Solde_initial" ))
    {
	gsb_data_account_set_init_balance (account_number_tmp,
				      my_strtod ( g_strdup (text),
						  NULL ));
	return;
    }

    if ( !strcmp ( element_name,
		   "Solde_mini_voulu" ))
    {
	gsb_data_account_set_mini_balance_wanted ( account_number_tmp, 
					      my_strtod ( g_strdup (text),
							  NULL ));
	return;
    }

    if ( !strcmp ( element_name,
		   "Solde_mini_autorise" ))
    {
	gsb_data_account_set_mini_balance_authorized ( account_number_tmp, 
						  my_strtod ( g_strdup (text),
							      NULL ));
	return;
    }

    if ( !strcmp ( element_name,
		   "Date_dernier_releve" ))
    {
	gchar **pointeur_char;

	if ( g_strdup (text) &&
	     strlen (text) > 0 )
	{
	    pointeur_char = g_strsplit ( g_strdup (text), "/", 0 );
	    gsb_data_account_set_current_reconcile_date ( account_number_tmp,
						     g_date_new_dmy ( utils_str_atoi ( pointeur_char [0] ),
								      utils_str_atoi ( pointeur_char [1] ),
								      utils_str_atoi ( pointeur_char [2] )));
	    g_strfreev ( pointeur_char );
	}
	return;
    }

    if ( !strcmp ( element_name,
		   "Solde_dernier_releve" ))
    {
	gsb_data_account_set_reconcile_balance ( account_number_tmp,
					    my_strtod ( g_strdup (text),
							NULL ) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Dernier_no_de_rapprochement" ))
    {
	gsb_data_account_set_reconcile_last_number ( account_number_tmp,
						utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Compte_cloture" ))
    {
	gsb_data_account_set_closed_account ( account_number_tmp,
					 utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_r" ))
    {
	gsb_data_account_set_r ( account_number_tmp,
			    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Nb_lignes_ope" ))
    {
	gsb_data_account_set_nb_rows ( account_number_tmp, 
				  utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Commentaires" ))
    {
	gsb_data_account_set_comment ( account_number_tmp,
				  g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Adresse_du_titulaire" ))
    {
	gsb_data_account_set_holder_address ( account_number_tmp,
					 g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_defaut_debit" ))
    {
	gsb_data_account_set_default_debit ( account_number_tmp,
					utils_str_atoi ( text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_defaut_credit" ))
    {
	gsb_data_account_set_default_credit ( account_number_tmp,
					 utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Tri_par_type" ))
    {
	gsb_data_account_set_reconcile_sort_type ( account_number_tmp,
					      utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Neutres_inclus" ))
    {
	gsb_data_account_set_split_neutral_payment ( account_number_tmp,
						utils_str_atoi ( text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Ordre_du_tri" ))
    {
	gsb_data_account_set_sort_list ( account_number_tmp,
				    NULL );

	if ( g_strdup (text))
	{
	    gchar **pointeur_char;
	    gint i;

	    pointeur_char = g_strsplit ( g_strdup (text),
					 "/",
					 0 );

	    i = 0;

	    while ( pointeur_char[i] )
	    {
		gsb_data_account_set_sort_list ( account_number_tmp,
					    g_slist_append ( gsb_data_account_get_sort_list (account_number_tmp),
							     GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] ))) );
		i++;
	    }
	    g_strfreev ( pointeur_char );
	}
	return;
    }

    if ( !strcmp ( element_name,
		   "Classement_croissant" ))
    {
	gsb_data_account_set_sort_type ( account_number_tmp,
				    utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Classement_colonne" ))
    {
	gsb_data_account_set_sort_column ( account_number_tmp,
				      utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Classement_type_par_colonne" ))
    {
	gint i;
	gchar **pointeur_char;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	for ( i=0 ; i<TRANSACTION_LIST_COL_NB ; i++ )
	{
	    gsb_data_account_set_column_sort ( account_number_tmp,
					  i,
					  utils_str_atoi ( pointeur_char[i] ));
	}
	g_strfreev ( pointeur_char );
	return;
    }

    /* récupération de l'agencement du formulaire */

    if ( !strcmp ( element_name,
		   "Nb_colonnes_formulaire" ))
    {
	if ( !gsb_data_account_get_form_organization (account_number_tmp) )
	    gsb_data_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );
	gsb_data_account_get_form_organization (account_number_tmp) -> nb_colonnes = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Nb_lignes_formulaire" ))
    {
	if ( !gsb_data_account_get_form_organization (account_number_tmp) )
	    gsb_data_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );
	gsb_data_account_get_form_organization (account_number_tmp) -> nb_lignes = utils_str_atoi ( text);
	return;
    }


    if ( !strcmp ( element_name,
		   "Organisation_formulaire" ))
    {
	gchar **pointeur_char;
	gint i, j;

	if ( !gsb_data_account_get_form_organization (account_number_tmp) )
	    gsb_data_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	for ( i=0 ; i<4 ; i++ )
	    for ( j=0 ; j< 6 ; j++ )
		gsb_data_account_get_form_organization (account_number_tmp) -> tab_remplissage_formulaire[i][j] = utils_str_atoi ( pointeur_char[j + i*6]);

	g_strfreev ( pointeur_char );
	return;
    }


    if ( !strcmp ( element_name,
		   "Largeur_col_formulaire" ))
    {
	gchar **pointeur_char;
	gint i;

	if ( !gsb_data_account_get_form_organization (account_number_tmp) )
	    gsb_data_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	for ( i=0 ; i<6 ; i++ )
	    gsb_data_account_get_form_organization (account_number_tmp) -> taille_colonne_pourcent[i] = utils_str_atoi ( pointeur_char[i]);

	g_strfreev ( pointeur_char );
	return;
    }
}


void gsb_file_load_report_part_before_0_6 ( GMarkupParseContext *context,
					    const gchar *text )
{
    const gchar *element_name;

    element_name = g_markup_parse_context_get_element ( context );

    /* check here if we are not between 2 subsections or if we
     * needn't that section */

    if ( !strcmp ( element_name,
		   "Etats" )
	 ||
	 !strcmp ( element_name,
		   "Generalites" )
	 ||
	 !strcmp ( element_name,
		   "No_dernier_etat" )
	 ||
	 !strcmp ( element_name,
		   "Detail_des_etats" )
	 ||
	 !strcmp ( element_name,
		   "Etat" ))
	return;


    if ( !strcmp ( element_name,
		   "No" ))
    {
	last_report_number = gsb_data_report_new_with_number (utils_str_atoi (text));
	return;
    }


    if ( !strcmp ( element_name,
		   "Nom" ))
    {
	gsb_data_report_set_report_name ( last_report_number,
					  text);
	return;
    }


    if ( !strcmp ( element_name,
		   "Type_classement" ))
    {
	gsb_data_report_set_sorting_type ( last_report_number,
					   gsb_string_get_list_from_string ( text,
									     "/" ));
	return;
    }


    if ( !strcmp ( element_name,
		   "Aff_r" ))
    {
	gsb_data_report_set_show_r ( last_report_number,
				     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ope" ))
    {
	gsb_data_report_set_show_report_transactions ( last_report_number,
						       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nb_ope" ))
    {
	gsb_data_report_set_show_report_transaction_amount ( last_report_number,
							     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_no_ope" ))
    {
	gsb_data_report_set_show_report_transaction_number ( last_report_number,
							     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_date_ope" ))
    {
	gsb_data_report_set_show_report_date ( last_report_number,
					       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_tiers_ope" ))
    {
	gsb_data_report_set_show_report_payee ( last_report_number,
						utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_categ_ope" ))
    {
	gsb_data_report_set_show_report_category ( last_report_number,
						   utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_categ_ope" ))
    {
	gsb_data_report_set_show_report_sub_category ( last_report_number,
						       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_type_ope" ))
    {
	gsb_data_report_set_show_report_method_of_payment ( last_report_number,
							    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ib_ope" ))
    {
	gsb_data_report_set_show_report_budget ( last_report_number,
						 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_ib_ope" ))
    {
	gsb_data_report_set_show_report_sub_budget ( last_report_number,
						     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_cheque_ope" ))
    {
	gsb_data_report_set_show_report_method_of_payment_content ( last_report_number,
								    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_notes_ope" ))
    {
	gsb_data_report_set_show_report_note ( last_report_number,
					       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_pc_ope" ))
    {
	gsb_data_report_set_show_report_voucher ( last_report_number,
						  utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_rappr_ope" ))
    {
	gsb_data_report_set_show_report_marked ( last_report_number,
						 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_infobd_ope" ))
    {
	gsb_data_report_set_show_report_bank_references ( last_report_number,
							  utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_exo_ope" ))
    {
	gsb_data_report_set_show_report_financial_year ( last_report_number,
							 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Class_ope" ))
    {
	gsb_data_report_set_sorting_report ( last_report_number,
					     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_titres_col" ))
    {
	gsb_data_report_set_column_title_show ( last_report_number,
						utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_titres_chgt" ))
    {
	gsb_data_report_set_column_title_type ( last_report_number,
						utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Pas_detail_ventil" ))
    {
	gsb_data_report_set_not_detail_breakdown ( last_report_number,
						   utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Sep_rev_dep" ))
    {
	gsb_data_report_set_split_credit_debit ( last_report_number,
						 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_gen" ))
    {
	gsb_data_report_set_currency_general ( last_report_number,
					       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Incl_tiers" ))
    {
	gsb_data_report_set_append_in_payee ( last_report_number,
					      utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Ope_click" ))
    {
	gsb_data_report_set_report_can_click ( last_report_number,
					       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Exo_date" ))
    {
	gsb_data_report_set_use_financial_year ( last_report_number,
						 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_exo" ))
    {
	gsb_data_report_set_financial_year_type ( last_report_number,
						  utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "No_exo" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );
	i=0;

	while ( pointeur_char[i] )
	{
	    gsb_data_report_set_financial_year_list ( last_report_number,
						      g_slist_append ( gsb_data_report_get_financial_year_list (last_report_number),
								       GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] ))));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Plage_date" ))
    {
	gsb_data_report_set_date_type ( last_report_number,
					utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Date_debut" )
	 &&
	 strlen(element_name))
    {
	gchar **pointeur_char;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );

	gsb_data_report_set_personal_date_start ( last_report_number,
						  g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
								   utils_str_atoi ( pointeur_char[1] ),
								   utils_str_atoi ( pointeur_char[2] )));
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name, "Date_fin" )
	 &&
	 strlen(element_name))
    {
	gchar **pointeur_char;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );

	gsb_data_report_set_personal_date_end ( last_report_number,
						g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
								 utils_str_atoi ( pointeur_char[1] ),
								 utils_str_atoi ( pointeur_char[2] )));
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Utilise_plages" ))
    {
	gsb_data_report_set_period_split ( last_report_number,
					   utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Sep_plages" ))
    {
	gsb_data_report_set_period_split_type ( last_report_number,
						utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Sep_exo" ))
    {
	gsb_data_report_set_financial_year_split ( last_report_number,
						   utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Deb_sem_plages" ))
    {
	gsb_data_report_set_period_split_day ( last_report_number,
					       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_comptes" ))
    {
	gsb_data_report_set_account_use_chosen ( last_report_number,
						 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "No_comptes" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );
	i=0;

	while ( pointeur_char[i] )
	{
	    gsb_data_report_set_account_numbers ( last_report_number,
						  g_slist_append ( gsb_data_report_get_account_numbers (last_report_number),
								   GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] ))));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }


    if ( !strcmp ( element_name,
		   "Grp_ope_compte" ))
    {
	gsb_data_report_set_account_group_reports ( last_report_number,
						    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_compte" ))
    {
	gsb_data_report_set_account_show_amount ( last_report_number,
						  utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_compte" ))
    {
	gsb_data_report_set_account_show_name ( last_report_number,
						utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_vir" ))
    {
	gsb_data_report_set_transfer_choice ( last_report_number,
					      utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "No_comptes_virements" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );
	i=0;

	while ( pointeur_char[i] )
	{
	    gsb_data_report_set_transfer_account_numbers ( last_report_number,
							   g_slist_append ( gsb_data_report_get_transfer_account_numbers (last_report_number),
									    GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] ))));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Exclure_non_vir" ))
    {
	gsb_data_report_set_transfer_reports_only ( last_report_number,
						    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Categ" ))
    {
	gsb_data_report_set_category_used ( last_report_number,
					    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_categ" ))
    {
	gsb_data_report_set_category_detail_used ( last_report_number,
						   utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "No_categ" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );
	i=0;

	while ( pointeur_char[i] )
	{
	    gsb_data_report_set_category_numbers ( last_report_number,
						   g_slist_append ( gsb_data_report_get_category_numbers (last_report_number),
								    GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] ))));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Exclut_categ" ))
    {
	gsb_data_report_set_category_only_report_with_category ( last_report_number,
								 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_categ" ))
    {
	gsb_data_report_set_category_show_category_amount ( last_report_number,
							    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_categ" ))
    {
	gsb_data_report_set_category_show_sub_category ( last_report_number,
							 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_pas_ss_categ" ))
    {
	gsb_data_report_set_category_show_without_category ( last_report_number,
							     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_ss_categ" ))
    {
	gsb_data_report_set_category_show_sub_category_amount ( last_report_number,
								utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_categ" ))
    {
	gsb_data_report_set_category_currency ( last_report_number,
						utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_categ" ))
    {
	gsb_data_report_set_category_show_name ( last_report_number,
						 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "IB" ))
    {
	gsb_data_report_set_budget_used ( last_report_number,
					  utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_ib" ))
    {
	gsb_data_report_set_budget_detail_used ( last_report_number,
						 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "No_ib" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );
	i=0;

	while ( pointeur_char[i] )
	{
	    gsb_data_report_set_budget_numbers ( last_report_number,
						 g_slist_append ( gsb_data_report_get_budget_numbers (last_report_number),
								  GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] ))));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }


    if ( !strcmp ( element_name,
		   "Exclut_ib" ))
    {
	gsb_data_report_set_budget_only_report_with_budget ( last_report_number,
							     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_ib" ))
    {
	gsb_data_report_set_budget_show_budget_amount ( last_report_number,
							utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_ib" ))
    {
	gsb_data_report_set_budget_show_sub_budget ( last_report_number,
						     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_pas_ss_ib" ))
    {
	gsb_data_report_set_budget_show_without_budget ( last_report_number,
							 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_ss_ib" ))
    {
	gsb_data_report_set_budget_show_sub_budget_amount ( last_report_number,
							    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_ib" ))
    {
	gsb_data_report_set_budget_currency ( last_report_number,
					      utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_ib" ))
    {
	gsb_data_report_set_budget_show_name ( last_report_number,
					       utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Tiers" ))
    {
	gsb_data_report_set_payee_used ( last_report_number,
					 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_tiers" ))
    {
	gsb_data_report_set_payee_detail_used ( last_report_number,
						utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "No_tiers" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "/",
				     0 );
	i=0;

	while ( pointeur_char[i] )
	{
	    gsb_data_report_set_payee_numbers ( last_report_number,
						g_slist_append ( gsb_data_report_get_payee_numbers (last_report_number),
								 GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] ))));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_tiers" ))
    {
	gsb_data_report_set_payee_show_payee_amount ( last_report_number,
						      utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_tiers" ))
    {
	gsb_data_report_set_payee_currency ( last_report_number,
					     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_tiers" ))
    {
	gsb_data_report_set_payee_show_name ( last_report_number,
					      utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Texte" ))
    {
	gsb_data_report_set_text_comparison_used ( last_report_number,
						   utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Montant" ))
    {
	gsb_data_report_set_amount_comparison_used ( last_report_number,
						     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Montant_devise" ))
    {
	gsb_data_report_set_amount_comparison_currency ( last_report_number,
							 utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Excl_nul" ))
    {
	gsb_data_report_set_amount_comparison_only_report_non_null ( last_report_number,
								     utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_mod_paie" ))
    {
	gsb_data_report_set_method_of_payment_used ( last_report_number,
						     utils_str_atoi (text));
	return;
    }
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
