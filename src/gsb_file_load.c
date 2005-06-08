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
#include "gsb_account.h"
#include "data_form.h"
#include "gsb_transaction_data.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils_buttons.h"
#include "fichiers_gestion.h"
#include "utils_files.h"
#include "structures.h"
#include "echeancier_liste.h"
#include "operations_liste.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_file_load_account_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_end_element ( GMarkupParseContext *context,
				 const gchar *element_name,
				 gpointer user_data,
				 GError **error);
static void gsb_file_load_general_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_report_part ( GMarkupParseContext *context,
				 const gchar *text );
static void gsb_file_load_start_element ( GMarkupParseContext *context,
				   const gchar *element_name,
				   const gchar **attribute_names,
				   const gchar **attribute_values,
				   gpointer user_data,
				   GError **error);
static void gsb_file_load_text_element( GMarkupParseContext *context,
				 const gchar *text,
				 gsize text_len,  
				 gpointer user_data,
				 GError **error);
static gboolean gsb_file_load_update_previous_version ( void );
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *adr_banque;
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_j_m_a;
extern gint affichage_echeances_perso_nb_libre;
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
extern GSList *liste_struct_categories;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_echeances;
extern GSList *liste_struct_etats;
extern GSList *liste_struct_exercices;
extern GSList *liste_struct_imputation;
extern GSList *liste_struct_rapprochements;
extern GSList *liste_struct_tiers;
extern gint nb_colonnes;
extern gint no_devise_totaux_tiers;
extern GtkWidget *nom_banque;
extern GtkWidget *nom_correspondant;
extern GtkWidget *nom_exercice;
extern gchar *nom_fichier_backup;
extern gchar *nom_fichier_comptes;
extern gint rapport_largeur_colonnes[TRANSACTION_LIST_COL_NB];
extern GtkWidget *remarque_banque;
extern gint scheduler_col_width[NB_COLS_SCHEDULER] ;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern GtkWidget *tel_banque;
extern GtkWidget *tel_correspondant;
extern gchar *titre_fichier;
extern gint valeur_echelle_recherche_date_import;
extern GtkWidget *web_banque;
extern GtkWidget *window;
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
static struct struct_categ *category_tmp;
static struct struct_imputation *budget_tmp;
static struct struct_etat *report_tmp;

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

    if ( DEBUG )
	printf ( "gsb_file_load_open_file %s\n", 
		 filename );

    /* fill the buffer stat to check the file */

    return_value = utf8_stat ( filename,
			       &buffer_stat);

    /* general check */

    if ( return_value == -1 )
    {
	dialogue_error (g_strdup_printf (_("Cannot open file '%s': %s"),
					 filename,
					 latin2utf8 (strerror(errno))));
	remove_file_from_last_opened_files_list (filename);
	return FALSE;
    }

    /* check here if it's not a regular file */

    if ( !S_ISREG (buffer_stat.st_mode))
    {
	dialogue_error ( g_strdup_printf ( _("%s doesn't seem to be a regular file,\nplease check it and try again."),
					   filename ));
	remove_file_from_last_opened_files_list (filename);
	return ( FALSE );
    }

    /* check the access to the file and propose to change it */

    if ( buffer_stat.st_mode != 33152
	 &&
	 !etat.display_message_file_readable )
	propose_changement_permissions();

    /* load the file */

    if ( g_file_get_contents ( filename,
			       &file_content,
			       NULL,
			       NULL ))
    {
	GMarkupParser *markup_parser = g_malloc0 (sizeof (GMarkupParser));
	GMarkupParseContext *context;

	/* fill the GMarkupParser */

	markup_parser -> start_element = (void *) gsb_file_load_start_element;
	markup_parser -> end_element = (void *) gsb_file_load_end_element;
	markup_parser -> text = (void *) gsb_file_load_text_element;

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
	dialogue_error (g_strdup_printf (_("Cannot open file '%s': %s"),
					 filename,
					 latin2utf8 (strerror(errno))));
	remove_file_from_last_opened_files_list (filename);
	return FALSE;
    }

    return gsb_file_load_update_previous_version();
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
    gint i,j;
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

	    list_tmp_transactions = gsb_transaction_data_get_transactions_list ();

	    while ( list_tmp_transactions )
	    {
		gint transaction_number_tmp;
		transaction_number_tmp = gsb_transaction_data_get_transaction_number (list_tmp_transactions -> data);

		/*  si l'opération est une ventil, on refait le tour de la liste pour trouver ses filles */

		if ( gsb_transaction_data_get_breakdown_of_transaction (transaction_number_tmp))
		{
		    GSList *list_tmp_transactions_2;
		    list_tmp_transactions_2 = gsb_transaction_data_get_transactions_list ();

		    while ( list_tmp_transactions_2 )
		    {
			gint transaction_number_tmp_2;
			transaction_number_tmp_2 = gsb_transaction_data_get_transaction_number (list_tmp_transactions_2 -> data);

			if ( gsb_transaction_data_get_account_number (transaction_number_tmp_2) == gsb_transaction_data_get_account_number (transaction_number_tmp)
			     &&
			     gsb_transaction_data_get_mother_transaction_number (transaction_number_tmp_2) == transaction_number_tmp)
			    gsb_transaction_data_set_reconcile_number ( transaction_number_tmp_2,
									gsb_transaction_data_get_reconcile_number (transaction_number_tmp));

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

		affichage_r = gsb_account_get_r (gsb_account_get_current_account ());
		nb_lignes_ope = gsb_account_get_nb_rows ( gsb_account_get_current_account () );

		list_tmp = gsb_account_get_list_accounts ();

		while ( list_tmp )
		{
		    i = gsb_account_get_no_account ( list_tmp -> data );

		    gsb_account_set_r ( i,
					affichage_r );
		    gsb_account_set_nb_rows ( i, 
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

	    gsb_account_reorder ( sort_accounts );
	    g_slist_free ( sort_accounts );


	    /* we have now 8 columns, but as before it was 7, and the first
	     * is at the begining, we have to split on the right */

	    for ( j=0 ; j<TRANSACTION_LIST_ROWS_NB ; j++ )
	    {
		for ( i=TRANSACTION_LIST_COL_NB-1 ; i ; i-- )
		    tab_affichage_ope[j][i] = tab_affichage_ope[j][i-1];
		tab_affichage_ope[j][0] = 0;
	    }

	    for ( i=TRANSACTION_LIST_COL_NB-1 ; i ; i-- )
		rapport_largeur_colonnes[i] = rapport_largeur_colonnes[i-1];
	    rapport_largeur_colonnes[0] = 2;

	    list_tmp = gsb_account_get_list_accounts ();

	    while ( list_tmp )
	    {
		i = gsb_account_get_no_account ( list_tmp -> data );

		/* 	    set the form organization at 0 */
		gsb_account_set_form_organization ( i,
						    gsb_form_new_organization ());

		/* 	   set the current sort by date and ascending sort */
		init_default_sort_column (i);

		list_tmp = list_tmp -> next;
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

    modification_etat_ouverture_fichier ( TRUE );


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

	    gsb_account_set_method_payment_list ( account_number_tmp,
						  g_slist_append ( gsb_account_get_method_payment_list (account_number_tmp),
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
		    transaction_number = gsb_transaction_data_new_transaction_with_number ( account_number_tmp,
											    utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Id" ))
		    gsb_transaction_data_set_transaction_id ( transaction_number,
							       attribute_values[i] );

		if ( !strcmp ( attribute_names[i],
			       "D" ))
		{
		    pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );

		    gsb_transaction_data_set_date ( transaction_number,
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
			    gsb_transaction_data_set_value_date ( transaction_number,
								  g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
										   utils_str_atoi ( pointeur_char[1] ),
										   utils_str_atoi ( pointeur_char[2] )));

			g_strfreev ( pointeur_char );
		    }
		}

		if ( !strcmp ( attribute_names[i],
			       "M" ))
		    gsb_transaction_data_set_amount ( transaction_number,
						      my_strtod ( attribute_values[i],
								  NULL ));

		if ( !strcmp ( attribute_names[i],
			       "De" ))
		    gsb_transaction_data_set_currency_number ( transaction_number,
							       utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Rdc" ))
		    gsb_transaction_data_set_change_between ( transaction_number,
							      utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Tc" ))
		    gsb_transaction_data_set_exchange_rate ( transaction_number,
							     my_strtod ( attribute_values[i],
									 NULL ) );

		if ( !strcmp ( attribute_names[i],
			       "Fc" ))
		    gsb_transaction_data_set_exchange_fees ( transaction_number,
							     my_strtod ( attribute_values[i],
									 NULL ) );

		if ( !strcmp ( attribute_names[i],
			       "T" ))
		    gsb_transaction_data_set_party_number ( transaction_number,
							    utils_str_atoi ( attribute_values[i])  );

		if ( !strcmp ( attribute_names[i],
			       "C" ))
		    gsb_transaction_data_set_category_number ( transaction_number,
							       utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Sc" ))
		    gsb_transaction_data_set_sub_category_number ( transaction_number,
								   utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Ov" ))
		    gsb_transaction_data_set_breakdown_of_transaction ( transaction_number,
									utils_str_atoi ( attribute_values[i]) );

		if ( !strcmp ( attribute_names[i],
			       "N" ))
		    gsb_transaction_data_set_notes ( transaction_number,
						     attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Ty" ))
		    gsb_transaction_data_set_method_of_payment_number ( transaction_number,
									utils_str_atoi ( attribute_values[i]) );

		if ( !strcmp ( attribute_names[i],
			       "Ct" ))
		    gsb_transaction_data_set_method_of_payment_content ( transaction_number,
									 attribute_values[i] );

		if ( !strcmp ( attribute_names[i],
			       "P" ))
		    gsb_transaction_data_set_marked_transaction ( transaction_number,
								  utils_str_atoi ( attribute_values[i]));
		if ( !strcmp ( attribute_names[i],
			       "A" ))
		    gsb_transaction_data_set_automatic_transaction ( transaction_number,
								     utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "R" ))
		    gsb_transaction_data_set_reconcile_number ( transaction_number,
								utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "E" ))
		    gsb_transaction_data_set_financial_year_number ( transaction_number,
								     utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "I" ))
		    gsb_transaction_data_set_budgetary_number ( transaction_number,
								utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Si" ))
		    gsb_transaction_data_set_sub_budgetary_number ( transaction_number,
								    utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Pc" ))
		    gsb_transaction_data_set_voucher ( transaction_number,
						       attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Ibg" ))
		    gsb_transaction_data_set_bank_references ( transaction_number,
							       attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Ro" ))
		    gsb_transaction_data_set_transaction_number_transfer ( transaction_number,
									   utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Rc" ))
		    gsb_transaction_data_set_account_number_transfer ( transaction_number,
								       utils_str_atoi ( attribute_values[i]));

		if ( !strcmp ( attribute_names[i],
			       "Va" ))
		    gsb_transaction_data_set_mother_transaction_number ( transaction_number,
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
	    struct operation_echeance *operation_echeance;

	    operation_echeance = calloc ( 1,
					  sizeof (struct operation_echeance ));

	    do
	    {
		gchar **pointeur_char;

		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    operation_echeance -> no_operation = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Date" ))
		{
		    pointeur_char = g_strsplit ( attribute_values[i],
						 "/",
						 0 );
		    operation_echeance -> jour = utils_str_atoi ( pointeur_char[0] );
		    operation_echeance -> mois = utils_str_atoi ( pointeur_char[1] );
		    operation_echeance -> annee = utils_str_atoi ( pointeur_char[2] );
		    operation_echeance -> date = g_date_new_dmy ( operation_echeance -> jour,
								  operation_echeance -> mois,
								  operation_echeance -> annee );
		    g_strfreev ( pointeur_char );
		}

		if ( !strcmp ( attribute_names[i],
			       "Compte" ))
		    operation_echeance -> compte = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Montant" ))
		    operation_echeance -> montant = my_strtod ( attribute_values[i],
								NULL );

		if ( !strcmp ( attribute_names[i],
			       "Devise" ))
		    operation_echeance -> devise = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Tiers" ))
		    operation_echeance -> tiers = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Categorie" ))
		    operation_echeance -> categorie = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Sous-categorie" ))
		    operation_echeance -> sous_categorie = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Virement_compte" ))
		    operation_echeance -> compte_virement = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Type" ))
		    operation_echeance -> type_ope = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Type_contre_ope" ))
		    operation_echeance -> type_contre_ope = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Contenu_du_type" )
		     &&
		     strlen (attribute_values[i]))
		    operation_echeance -> contenu_type = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Exercice" ))
		    operation_echeance -> no_exercice = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Imputation" ))
		    operation_echeance -> imputation = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Sous-imputation" ))
		    operation_echeance -> sous_imputation = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Notes" )
		     &&
		     strlen (attribute_values[i]))
		    operation_echeance -> notes = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Automatique" ))
		    operation_echeance -> auto_man = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Periodicite" ))
		    operation_echeance -> periodicite = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Intervalle_periodicite" ))
		    operation_echeance -> intervalle_periodicite_personnalisee = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Periodicite_personnalisee" ))
		    operation_echeance -> periodicite_personnalisee = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Date_limite" ))
		{
		    if ( strlen ( attribute_values[i]))
		    {
			pointeur_char = g_strsplit ( attribute_values[i],
						     "/",
						     0 );

			operation_echeance -> jour_limite = utils_str_atoi ( pointeur_char[0] );
			operation_echeance -> mois_limite = utils_str_atoi ( pointeur_char[1] );
			operation_echeance -> annee_limite = utils_str_atoi ( pointeur_char[2] );
			operation_echeance -> date_limite = g_date_new_dmy ( operation_echeance -> jour_limite,
									     operation_echeance -> mois_limite,
									     operation_echeance -> annee_limite );
			g_strfreev ( pointeur_char );
		    }
		    else
		    {
			operation_echeance -> jour_limite = 0;
			operation_echeance -> mois_limite = 0;
			operation_echeance -> annee_limite = 0;
			operation_echeance -> date_limite = NULL;
		    }
		}
		if ( !strcmp ( attribute_names[i],
			       "Ech_ventilee" ))
		    operation_echeance -> operation_ventilee = utils_str_atoi ( attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "No_ech_associee" ))
		    operation_echeance -> no_operation_ventilee_associee = utils_str_atoi ( attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    liste_struct_echeances = g_slist_append ( liste_struct_echeances,
						      operation_echeance);
	}
    }


    if ( !strcmp ( element_name,
		   "Tiers" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_tiers *tiers;

	    tiers = calloc ( 1,
			     sizeof ( struct struct_tiers ) );
	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    tiers -> no_tiers = utils_str_atoi (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    tiers -> nom_tiers = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Informations" )
		     &&
		     strlen (attribute_values[i]))
		    tiers -> texte = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Liaison" ))
		    tiers -> liaison = utils_str_atoi (attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    liste_struct_tiers = g_slist_append ( liste_struct_tiers,
						  tiers );
	}
    }


    if ( !strcmp ( element_name,
		   "Categorie" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    category_tmp = calloc ( 1,
				 sizeof ( struct struct_categ ) );

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    category_tmp -> no_categ = utils_str_atoi (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    category_tmp -> nom_categ = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Type" ))
		    category_tmp -> type_categ = utils_str_atoi (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "No_derniere_sous_cagegorie" ))
		    category_tmp -> no_derniere_sous_categ = utils_str_atoi (attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    liste_struct_categories = g_slist_append ( liste_struct_categories,
						       category_tmp );
	}
    }


    if ( !strcmp ( element_name,
		   "Sous-categorie" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_sous_categ *sous_categ;

	    sous_categ = calloc ( 1,
				  sizeof ( struct struct_sous_categ ) );

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    sous_categ -> no_sous_categ = utils_str_atoi (attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    sous_categ -> nom_sous_categ = g_strdup (attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    /* normally categorie was defined before */

	    category_tmp -> liste_sous_categ = g_slist_append ( category_tmp -> liste_sous_categ,
							     sous_categ );
	}
    }


    if ( !strcmp ( element_name,
		   "Imputation" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    budget_tmp = calloc ( 1,
				  sizeof ( struct struct_imputation ) );

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    budget_tmp -> no_imputation = utils_str_atoi (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    budget_tmp -> nom_imputation = g_strdup (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "Type" ))
		    budget_tmp -> type_imputation = utils_str_atoi (attribute_values[i]);

		if ( !strcmp ( attribute_names[i],
			       "No_derniere_sous_imputation" ))
		    budget_tmp -> no_derniere_sous_imputation = utils_str_atoi (attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    liste_struct_imputation = g_slist_append ( liste_struct_imputation,
						       budget_tmp );
	}
    }


    if ( !strcmp ( element_name,
		   "Sous-imputation" ))
    {
	gint i;

	i = 0;

	if ( attribute_names[i] )
	{
	    struct struct_sous_imputation *sous_imputation;

	    sous_imputation = calloc ( 1,
				       sizeof ( struct struct_sous_imputation ) );

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "No" ))
		    sous_imputation -> no_sous_imputation = utils_str_atoi (attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Nom" ))
		    sous_imputation -> nom_sous_imputation = g_strdup (attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    /* normally budget_tmp was defined before */

	    budget_tmp -> liste_sous_imputation = g_slist_append ( budget_tmp -> liste_sous_imputation,
								   sous_imputation );
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
		   "Exercices" ))
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
	    struct struct_comparaison_textes_etat *comp_textes;

	    comp_textes = calloc ( 1,
				   sizeof ( struct struct_comparaison_textes_etat ));

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "Lien_struct" ))
		    comp_textes -> lien_struct_precedente = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Champ" ))
		    comp_textes -> champ = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Op" ))
		    comp_textes -> operateur = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Txt" ))
		    comp_textes -> texte = g_strdup (attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Util_txt" ))
		    comp_textes -> utilise_txt = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Comp_1" ))
		    comp_textes -> comparateur_1 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Lien_1_2" ))
		    comp_textes -> lien_1_2 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Comp_2" ))
		    comp_textes -> comparateur_2 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Mont_1" ))
		    comp_textes -> montant_1 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Mont_2" ))
		    comp_textes -> montant_2 = utils_str_atoi ( attribute_values[i]);

		i++;
	    }
	    while ( attribute_names[i] );

	    report_tmp -> liste_struct_comparaison_textes = g_slist_append ( report_tmp -> liste_struct_comparaison_textes,
								       comp_textes );
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
	    struct struct_comparaison_montants_etat *comp_montants;

	    comp_montants = calloc ( 1,
				     sizeof ( struct struct_comparaison_montants_etat ));

	    do
	    {
		if ( !strcmp ( attribute_names[i],
			       "Lien_struct" ))
		    comp_montants -> lien_struct_precedente = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Comp_1" ))
		    comp_montants -> comparateur_1 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Lien_1_2" ))
		    comp_montants -> lien_1_2 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Comp_2" ))
		    comp_montants -> comparateur_2 = utils_str_atoi ( attribute_values[i]);
		if ( !strcmp ( attribute_names[i],
			       "Mont_1" ))
		    comp_montants -> montant_1 = my_strtod ( attribute_values[i],
							     NULL );
		if ( !strcmp ( attribute_names[i],
			       "Mont_2" ))
		    comp_montants -> montant_2 = my_strtod ( attribute_values[i],
							     NULL );

		i++;
	    }
	    while ( attribute_names[i] );

	    report_tmp -> liste_struct_comparaison_montants = g_slist_append ( report_tmp -> liste_struct_comparaison_montants,
									       comp_montants );
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
		    report_tmp -> noms_modes_paiement = g_slist_append ( report_tmp -> noms_modes_paiement,
									 g_strdup (attribute_values[i]));

		i++;
	    }
	    while ( attribute_names[i] );
	}
	return;
    }
}



void gsb_file_load_end_element ( GMarkupParseContext *context,
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

void gsb_file_load_text_element( GMarkupParseContext *context,
				 const gchar *text,
				 gsize text_len,  
				 gpointer user_data,
				 GError **error)
{
    /* we come here for all text element, we split here to go
     * on the necessary function to work with that element */

    if ( download_tmp_values.general_part )
	gsb_file_load_general_part ( context,
				     text );
    if ( download_tmp_values.account_part )
	gsb_file_load_account_part ( context,
				     text );
    if ( download_tmp_values.report_part )
	gsb_file_load_report_part ( context,
				    text );
}

void gsb_file_load_general_part ( GMarkupParseContext *context,
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
		   "Rapport_largeur_col" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	for ( i=0 ; i< TRANSACTION_LIST_COL_NB; i++ )
	    if ( pointeur_char[i] )
		rapport_largeur_colonnes[i] = utils_str_atoi ( pointeur_char[i]);

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


void gsb_file_load_account_part ( GMarkupParseContext *context,
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
	account_number_tmp = gsb_account_new ( GSB_TYPE_BANK );
	gsb_account_set_name ( account_number_tmp,
			       g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Id_compte" ))
    {
	gsb_account_set_id (account_number_tmp,
			    g_strdup (text));
	if ( !strlen ( gsb_account_get_id (account_number_tmp)))
	    gsb_account_set_id (account_number_tmp,
				NULL );
	return;
    }

    /* 			    we change here the default number of the account */

    if ( !strcmp ( element_name,
		   "No_de_compte" ))
    {
	account_number_tmp = gsb_account_set_account_number ( account_number_tmp,
							      utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Titulaire" ))
    {
	gsb_account_set_holder_name ( account_number_tmp,
				      g_strdup (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_de_compte" ))
    {
	gsb_account_set_kind (account_number_tmp,
			      utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise" ))
    {
	gsb_account_set_currency ( account_number_tmp,
				   utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Banque" ))
    {
	gsb_account_set_bank ( account_number_tmp,
			       utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Guichet" ))
    {
	gsb_account_set_bank_branch_code ( account_number_tmp,
					   g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "No_compte_banque" ))
    {
	gsb_account_set_bank_account_number ( account_number_tmp,
					      g_strdup (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Cle_du_compte" ))
    {
	gsb_account_set_bank_account_key ( account_number_tmp,
					   g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Solde_initial" ))
    {
	gsb_account_set_init_balance (account_number_tmp,
				      my_strtod ( g_strdup (text),
						  NULL ));
	return;
    }

    if ( !strcmp ( element_name,
		   "Solde_mini_voulu" ))
    {
	gsb_account_set_mini_balance_wanted ( account_number_tmp, 
					      my_strtod ( g_strdup (text),
							  NULL ));
	return;
    }

    if ( !strcmp ( element_name,
		   "Solde_mini_autorise" ))
    {
	gsb_account_set_mini_balance_authorized ( account_number_tmp, 
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
	    gsb_account_set_current_reconcile_date ( account_number_tmp,
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
	gsb_account_set_reconcile_balance ( account_number_tmp,
					    my_strtod ( g_strdup (text),
							NULL ) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Dernier_no_de_rapprochement" ))
    {
	gsb_account_set_reconcile_last_number ( account_number_tmp,
						utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Compte_cloture" ))
    {
	gsb_account_set_closed_account ( account_number_tmp,
					 utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Affichage_r" ))
    {
	gsb_account_set_r ( account_number_tmp,
			    utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Nb_lignes_ope" ))
    {
	gsb_account_set_nb_rows ( account_number_tmp, 
				  utils_str_atoi (text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Commentaires" ))
    {
	gsb_account_set_comment ( account_number_tmp,
				  g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Adresse_du_titulaire" ))
    {
	gsb_account_set_holder_address ( account_number_tmp,
					 g_strdup (text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_defaut_debit" ))
    {
	gsb_account_set_default_debit ( account_number_tmp,
					utils_str_atoi ( text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_defaut_credit" ))
    {
	gsb_account_set_default_credit ( account_number_tmp,
					 utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Tri_par_type" ))
    {
	gsb_account_set_reconcile_sort_type ( account_number_tmp,
					      utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Neutres_inclus" ))
    {
	gsb_account_set_split_neutral_payment ( account_number_tmp,
						utils_str_atoi ( text) );
	return;
    }

    if ( !strcmp ( element_name,
		   "Ordre_du_tri" ))
    {
	gsb_account_set_sort_list ( account_number_tmp,
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
		gsb_account_set_sort_list ( account_number_tmp,
					    g_slist_append ( gsb_account_get_sort_list (account_number_tmp),
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
	gsb_account_set_sort_type ( account_number_tmp,
				    utils_str_atoi ( text));
	return;
    }

    if ( !strcmp ( element_name,
		   "Classement_colonne" ))
    {
	gsb_account_set_sort_column ( account_number_tmp,
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
	    gsb_account_set_column_sort ( account_number_tmp,
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
	if ( !gsb_account_get_form_organization (account_number_tmp) )
	    gsb_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );
	gsb_account_get_form_organization (account_number_tmp) -> nb_colonnes = utils_str_atoi ( text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Nb_lignes_formulaire" ))
    {
	if ( !gsb_account_get_form_organization (account_number_tmp) )
	    gsb_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );
	gsb_account_get_form_organization (account_number_tmp) -> nb_lignes = utils_str_atoi ( text);
	return;
    }


    if ( !strcmp ( element_name,
		   "Organisation_formulaire" ))
    {
	gchar **pointeur_char;
	gint i, j;

	if ( !gsb_account_get_form_organization (account_number_tmp) )
	    gsb_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	for ( i=0 ; i<4 ; i++ )
	    for ( j=0 ; j< 6 ; j++ )
		gsb_account_get_form_organization (account_number_tmp) -> tab_remplissage_formulaire[i][j] = utils_str_atoi ( pointeur_char[j + i*6]);

	g_strfreev ( pointeur_char );
	return;
    }


    if ( !strcmp ( element_name,
		   "Largeur_col_formulaire" ))
    {
	gchar **pointeur_char;
	gint i;

	if ( !gsb_account_get_form_organization (account_number_tmp) )
	    gsb_account_set_form_organization ( account_number_tmp,
						calloc ( 1,
							 sizeof ( struct organisation_formulaire )) );

	pointeur_char = g_strsplit ( g_strdup (text),
				     "-",
				     0 );

	for ( i=0 ; i<6 ; i++ )
	    gsb_account_get_form_organization (account_number_tmp) -> taille_colonne_pourcent[i] = utils_str_atoi ( pointeur_char[i]);

	g_strfreev ( pointeur_char );
	return;
    }
}


void gsb_file_load_report_part ( GMarkupParseContext *context,
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
	report_tmp = calloc ( 1, sizeof ( struct struct_etat ));
	report_tmp -> no_etat = utils_str_atoi (text);
	liste_struct_etats = g_slist_append ( liste_struct_etats,report_tmp  );
    }


    if ( !strcmp ( element_name,
		   "Nom" ))
    {
	report_tmp -> nom_etat = g_strdup (text);
	return;
    }


    if ( !strcmp ( element_name,
		   "Type_classement" ))
    {
	gchar **pointeur_char;
	gint i;

	pointeur_char = g_strsplit ( text,
				     "/",
				     0 );
	i=0;

	while ( pointeur_char[i] )
	{
	    report_tmp -> type_classement = g_list_append ( report_tmp -> type_classement,
							    GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }


    if ( !strcmp ( element_name,
		   "Aff_r" ))
    {
	report_tmp -> afficher_r = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ope" ))
    {
	report_tmp -> afficher_opes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nb_ope" ))
    {
	report_tmp -> afficher_nb_opes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_no_ope" ))
    {
	report_tmp -> afficher_no_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_date_ope" ))
    {
	report_tmp -> afficher_date_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_tiers_ope" ))
    {
	report_tmp -> afficher_tiers_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_categ_ope" ))
    {
	report_tmp -> afficher_categ_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_categ_ope" ))
    {
	report_tmp -> afficher_sous_categ_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_type_ope" ))
    {
	report_tmp -> afficher_type_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ib_ope" ))
    {
	report_tmp -> afficher_ib_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_ib_ope" ))
    {
	report_tmp ->afficher_sous_ib_ope  = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_cheque_ope" ))
    {
	report_tmp -> afficher_cheque_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_notes_ope" ))
    {
	report_tmp -> afficher_notes_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_pc_ope" ))
    {
	report_tmp -> afficher_pc_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_rappr_ope" ))
    {
	report_tmp -> afficher_rappr_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_infobd_ope" ))
    {
	report_tmp -> afficher_infobd_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_exo_ope" ))
    {
	report_tmp -> afficher_exo_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Class_ope" ))
    {
	report_tmp -> type_classement_ope = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_titres_col" ))
    {
	report_tmp -> afficher_titre_colonnes = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_titres_chgt" ))
    {
	report_tmp -> type_affichage_titres = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Pas_detail_ventil" ))
    {
	report_tmp -> pas_detailler_ventilation = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Sep_rev_dep" ))
    {
	report_tmp -> separer_revenus_depenses = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_gen" ))
    {
	report_tmp -> devise_de_calcul_general = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Incl_tiers" ))
    {
	report_tmp -> inclure_dans_tiers = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Ope_click" ))
    {
	report_tmp -> ope_clickables = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Exo_date" ))
    {
	report_tmp -> exo_date = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_exo" ))
    {
	report_tmp -> utilise_detail_exo = utils_str_atoi (text);
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
	    report_tmp -> no_exercices = g_slist_append ( report_tmp -> no_exercices,
							  GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Plage_date" ))
    {
	report_tmp -> no_plage_date = utils_str_atoi (text);
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

	report_tmp -> date_perso_debut = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
							  utils_str_atoi ( pointeur_char[1] ),
							  utils_str_atoi ( pointeur_char[2] ));
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

	report_tmp -> date_perso_fin = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
							utils_str_atoi ( pointeur_char[1] ),
							utils_str_atoi ( pointeur_char[2] ));
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Utilise_plages" ))
    {
	report_tmp -> separation_par_plage = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Sep_plages" ))
    {
	report_tmp -> type_separation_plage = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Sep_exo" ))
    {
	report_tmp -> separation_par_exo = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Deb_sem_plages" ))
    {
	report_tmp -> jour_debut_semaine = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_comptes" ))
    {
	report_tmp -> utilise_detail_comptes = utils_str_atoi (text);
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
	    report_tmp ->no_comptes  = g_slist_append ( report_tmp -> no_comptes,
						  GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }


    if ( !strcmp ( element_name,
		   "Grp_ope_compte" ))
    {
	report_tmp -> regroupe_ope_par_compte = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_compte" ))
    {
	report_tmp -> affiche_sous_total_compte = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_compte" ))
    {
	report_tmp -> afficher_nom_compte = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Type_vir" ))
    {
	report_tmp -> type_virement = utils_str_atoi (text);
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
	    report_tmp ->no_comptes_virements  = g_slist_append ( report_tmp -> no_comptes_virements,
							    GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Exclure_non_vir" ))
    {
	report_tmp -> exclure_ope_non_virement = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Categ" ))
    {
	report_tmp -> utilise_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_categ" ))
    {
	report_tmp -> utilise_detail_categ = utils_str_atoi (text);
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
	    report_tmp -> no_categ = g_slist_append ( report_tmp -> no_categ,
						GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Exclut_categ" ))
    {
	report_tmp -> exclure_ope_sans_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_categ" ))
    {
	report_tmp -> affiche_sous_total_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_categ" ))
    {
	report_tmp -> afficher_sous_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_pas_ss_categ" ))
    {
	report_tmp -> afficher_pas_de_sous_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_ss_categ" ))
    {
	report_tmp -> affiche_sous_total_sous_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_categ" ))
    {
	report_tmp -> devise_de_calcul_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_categ" ))
    {
	report_tmp -> afficher_nom_categ = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "IB" ))
    {
	report_tmp -> utilise_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_ib" ))
    {
	report_tmp -> utilise_detail_ib = utils_str_atoi (text);
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
	    report_tmp -> no_ib = g_slist_append ( report_tmp -> no_ib,
					     GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }


    if ( !strcmp ( element_name,
		   "Exclut_ib" ))
    {
	report_tmp -> exclure_ope_sans_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_ib" ))
    {
	report_tmp -> affiche_sous_total_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_ss_ib" ))
    {
	report_tmp -> afficher_sous_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_pas_ss_ib" ))
    {
	report_tmp -> afficher_pas_de_sous_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_ss_ib" ))
    {
	report_tmp -> affiche_sous_total_sous_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_ib" ))
    {
	report_tmp -> devise_de_calcul_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_ib" ))
    {
	report_tmp -> afficher_nom_ib = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Tiers" ))
    {
	report_tmp -> utilise_tiers = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_tiers" ))
    {
	report_tmp -> utilise_detail_tiers = utils_str_atoi (text);
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
	    report_tmp -> no_tiers = g_slist_append ( report_tmp -> no_tiers,
						GINT_TO_POINTER ( utils_str_atoi ( pointeur_char[i] )));
	    i++;
	}
	g_strfreev ( pointeur_char );
	return;
    }

    if ( !strcmp ( element_name,
		   "Total_tiers" ))
    {
	report_tmp -> affiche_sous_total_tiers = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Devise_tiers" ))
    {
	report_tmp -> devise_de_calcul_tiers = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Aff_nom_tiers" ))
    {
	report_tmp -> afficher_nom_tiers = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Texte" ))
    {
	report_tmp -> utilise_texte = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Montant" ))
    {
	report_tmp -> utilise_montant = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Montant_devise" ))
    {
	report_tmp -> choix_devise_montant = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Excl_nul" ))
    {
	report_tmp -> exclure_montants_nuls = utils_str_atoi (text);
	return;
    }

    if ( !strcmp ( element_name,
		   "Detail_mod_paie" ))
    {
	report_tmp -> utilise_mode_paiement = utils_str_atoi (text);
	return;
    }

}


/***********************************************************************************************************/
void switch_t_r ( void )
{
/* cette fonction fait le tour des opérations et change le marquage T et R des opés */
/*     R devient pointe=3 */
/*     T devient pointe=2 */

/*     à n'appeler que pour une version antérieure à 0.5.1 */

    GSList *list_tmp_transactions;

    if ( !gsb_account_get_accounts_amount () )
	return;
    
    if ( DEBUG )
	printf ( "switch_t_r\n");


    list_tmp_transactions = gsb_transaction_data_get_transactions_list ();

    while ( list_tmp_transactions )
    {
	gint transaction_number_tmp;
	transaction_number_tmp = gsb_transaction_data_get_transaction_number (list_tmp_transactions -> data);

	switch ( gsb_transaction_data_get_marked_transaction (transaction_number_tmp))
	{
	    case 2 :
		gsb_transaction_data_set_marked_transaction ( transaction_number_tmp,
							      3 );
		break;
	    case 3:
		gsb_transaction_data_set_marked_transaction ( transaction_number_tmp,
							      2 );
		break;
	}
	list_tmp_transactions = list_tmp_transactions -> next;
    }
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* crée ou supprime un fichier du nom .nom.swp */
/* renvoie true si ok */
/***********************************************************************************************************/

gboolean modification_etat_ouverture_fichier ( gboolean fichier_ouvert )
{
    struct stat buffer_stat;
    int result;
    gchar *nom_fichier_lock;
    gchar **tab_str;
    gint i;

    /*     on efface et on recommence... bon, changement de technique : lors de l'ouverture */
    /* 	d'un fichier, on crée un fichier .nom.swp qu'on efface à sa fermeture */

    /*     si on ne force pas l'enregistrement et si le fichier était déjà ouvert, on ne fait rien */

    if ( (etat.fichier_deja_ouvert
	  &&
	  !etat.force_enregistrement)
	 ||
	 !nom_fichier_comptes ||
	 !nom_fichier_comptes ||
	 !strlen(nom_fichier_comptes) )
	return TRUE;

    /*     on commence par vérifier que le fichier de nom_fichier_comptes existe bien */

    result = utf8_stat ( nom_fichier_comptes, &buffer_stat);

    if ( result == -1 )
    {
	dialogue_error (g_strdup_printf (_("Cannot open file '%s' to mark it as used: %s"),
					 nom_fichier_comptes,
					 latin2utf8 (strerror(errno))));
	return FALSE;
    }


    /*     création du nom du fichier swp */

    tab_str = g_strsplit ( nom_fichier_comptes,
			   G_DIR_SEPARATOR_S,
			   0 );

    i=0;

    while ( tab_str[i+1] )
	i++;

    tab_str[i] = g_strconcat ( 
#ifndef _WIN32
                              ".",
#endif
			       tab_str[i],
			       ".swp",
			       NULL );
    nom_fichier_lock = g_strjoinv ( G_DIR_SEPARATOR_S,
				   tab_str );
    g_strfreev ( tab_str );

    /*     maintenant on sépare entre l'effacement ou la création du fichier swp */

    if ( fichier_ouvert )
    {
	/* 	on ouvre le fichier, donc on crée le fichier de lock */

	FILE *fichier;

	/* 	commence par tester si ce fichier existe, si c'est le cas on prévient l'utilisateur */
	/* 	    avec possibilité d'annuler l'action ou d'effacer le fichier de lock */

	result = utf8_stat ( nom_fichier_lock, &buffer_stat);

	if ( result != -1 )
	{
	    /* 	    le fichier de lock existe */

	    dialogue_conditional_hint ( g_strdup_printf( _("File \"%s\" is already opened"),
							 nom_fichier_comptes),
					_("Either this file is already opened by another user or it wasn't closed correctly (maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the \"Force saving locked files\" option in setup."),
					&(etat.display_message_lock_active) );
	    
	    /* 	    on retourne true, vu que le fichier est déjà créé et qu'on a prévenu */

	    etat.fichier_deja_ouvert = 1;
	    return TRUE;
	}

	etat.fichier_deja_ouvert = 0;

	fichier = utf8_fopen ( nom_fichier_lock, "w" );

	if ( !fichier )
	{
	    dialogue_error (g_strdup_printf (_("Cannot write lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno))));
	    return FALSE;
	}

	fclose ( fichier );
	return TRUE;
    }
    else
    {
	/* 	on ferme le fichier, donc on détruit le fichier de lock */

	etat.fichier_deja_ouvert = 0;

	/* 	on vérifie d'abord que ce fichier existe */

	result = utf8_stat ( nom_fichier_lock, &buffer_stat);

	if ( result == -1 )
	{
	    /* 	    le fichier de lock n'existe */
	    /* 	    on s'en fout, de toute façon fallait le virer, on s'en va */

	    return TRUE;
	}

	result = utf8_remove ( nom_fichier_lock );

	if ( result == -1 )
	{
	    dialogue_error (g_strdup_printf (_("Cannot erase lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno))));
	    return FALSE;
	}
	return TRUE;
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
void propose_changement_permissions ( void )
{
    GtkWidget *dialog, *vbox, *checkbox;
    gint resultat;

    dialog = gtk_message_dialog_new ( GTK_WINDOW ( window ),
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_QUESTION,
				      GTK_BUTTONS_YES_NO,
				      " ");

    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), 
			   make_hint ( _("Account file is world readable."),
				       _("Your account file should not be readable by anybody else, but it is. You should change its permissions.\nShould this be fixed now?")));
    

    vbox = GTK_DIALOG(dialog) -> vbox;
    checkbox = new_checkbox_with_title ( _("Do not show this message again"),
					 &(etat.display_message_file_readable), NULL);
    gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, FALSE, FALSE, 6 );
    gtk_widget_show_all ( dialog );

    resultat = gtk_dialog_run ( GTK_DIALOG(dialog) );

    if ( resultat == GTK_RESPONSE_YES )
    {
	chmod ( nom_fichier_comptes, S_IRUSR | S_IWUSR );
    }

    gtk_widget_destroy ( dialog );
}
/***********************************************************************************************************/




/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
