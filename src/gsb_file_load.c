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
#include "fichiers_io.h"
#include "dialog.h"
#include "utils_str.h"
#include "fichiers_gestion.h"
#include "utils_files.h"
#include "structures.h"
#include "echeancier_liste.h"
#include "operations_liste.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_file_load_account_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_bank_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_budget_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_category_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_currency_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_end_element ( GMarkupParseContext *context,
				 const gchar *element_name,
				 gpointer user_data,
				 GError **error);
static void gsb_file_load_financial_year_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_general_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_payee_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_reconcile_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_report_part ( GMarkupParseContext *context,
				  const gchar *text );
static void gsb_file_load_scheduler_part ( GMarkupParseContext *context,
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
extern gint no_devise_totaux_tiers;
extern gchar *nom_fichier_backup;
extern gint rapport_largeur_colonnes[TRANSACTION_LIST_COL_NB];
extern gint scheduler_col_width[NB_COLS_SCHEDULER] ;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern gchar *titre_fichier;
extern gint valeur_echelle_recherche_date_import;
/*END_EXTERN*/

static struct
{
    gboolean download_ok;
    gchar *file_version;
    gchar *grisbi_version;

    /* there is always only one to TRUE, used to split the retrieves functions */

    gboolean general_part;
    gboolean account_part;
    gboolean scheduler_part;
    gboolean payee_part;
    gboolean category_part;
    gboolean budget_part;
    gboolean currency_part;
    gboolean bank_part;
    gboolean financial_year_part;
    gboolean reconcile_part;
    gboolean report_part;

} download_tmp_values;

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

    return (charge_operations ( filename ));

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

    /* to split the functions, we will set to 1 each time we begin a new part */

/*     j'en suis ici xxx */
/* 	à mettre en marche le split */



}


void gsb_file_load_end_element ( GMarkupParseContext *context,
				 const gchar *element_name,
				 gpointer user_data,
				 GError **error)
{
}

void gsb_file_load_text_element( GMarkupParseContext *context,
				 const gchar *text,
				 gsize text_len,  
				 gpointer user_data,
				 GError **error)
{
}

void gsb_file_load_general_part ( GMarkupParseContext *context,
				  const gchar *text )
{
    const gchar *element_name;

    element_name = g_markup_parse_context_get_element ( context );

    if ( !strcmp ( element_name,
		   "Version_fichier" ))
	download_tmp_values.file_version = g_strdup (text);
    
    if ( !strcmp ( element_name,
		   "Version_grisbi" ))
	download_tmp_values.grisbi_version = g_strdup (text);
    
    if ( !strcmp ( element_name,
		   "Backup" ))
	nom_fichier_backup = g_strdup (text);

    if ( !strcmp ( element_name,
		   "Titre" ))
	titre_fichier = g_strdup (text);

    if ( !strcmp ( element_name,
		   "Adresse_commune" ))
	adresse_commune = g_strdup (text);

    if ( !strcmp ( element_name,
		   "Adresse_secondaire" ))
	adresse_secondaire = g_strdup (text);

    if ( !strcmp ( element_name,
		   "Numero_devise_totaux_tiers" ))
	no_devise_totaux_tiers = utils_str_atoi ( g_strdup (text));

    if ( !strcmp ( element_name,
		   "Type_affichage_des_echeances" ))
	affichage_echeances = utils_str_atoi ( g_strdup (text));

    if ( !strcmp ( element_name,
		   "Affichage_echeances_perso_nb_libre" ))
	affichage_echeances_perso_nb_libre = utils_str_atoi ( g_strdup (text));

    if ( !strcmp ( element_name,
		   "Type_affichage_perso_echeances" ))
	affichage_echeances_perso_j_m_a = utils_str_atoi ( g_strdup (text));

    if ( !strcmp ( element_name,
		   "Echelle_date_import" ))
	valeur_echelle_recherche_date_import = utils_str_atoi ( g_strdup (text));

    if ( !strcmp ( element_name,
		   "Utilise_logo" ))
	etat.utilise_logo = utils_str_atoi ( g_strdup (text));

    if ( !strcmp ( element_name,
		   "Chemin_logo" ))
	chemin_logo = g_strdup (text);

    if ( !strcmp ( element_name,
		   "Caracteristiques_par_compte" ))
	etat.retient_affichage_par_compte = utils_str_atoi( g_strdup (text));

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
    }

    if ( !strcmp ( element_name,
		   "Ligne_aff_une_ligne" ))
	ligne_affichage_une_ligne = utils_str_atoi ( g_strdup (text));

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
    }

    if ( !strcmp ( element_name,
		   "Formulaire_distinct_par_compte" ))
	etat.formulaire_distinct_par_compte = utils_str_atoi( g_strdup (text));

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
    }
}


void gsb_file_load_account_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_scheduler_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_payee_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_category_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_budget_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_currency_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_bank_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_financial_year_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_reconcile_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


void gsb_file_load_report_part ( GMarkupParseContext *context,
				  const gchar *text )
{
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
