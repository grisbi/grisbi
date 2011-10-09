/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

#include <glib/gstdio.h>
#include <gio/gio.h>

/*START_INCLUDE*/
#include "gsb_file_load.h"
#include "bet_data.h"
#include "custom_list.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_first.h"
#include "gsb_calendar.h"
#include "gsb_currency_config.h"
#include "gsb_data_account.h"
#include "gsb_data_archive.h"
#include "gsb_data_bank.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_form.h"
#include "gsb_data_fyear.h"
#include "gsb_data_import_rule.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_print_config.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_report.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_file_util.h"
#include "gsb_plugins.h"
#include "gsb_real.h"
#include "gsb_select_icon.h"
#include "gsb_scheduler_list.h"
#include "import.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gsb_file_load_account_icon_part ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_account_part ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_account_part_before_0_6 ( GMarkupParseContext *context,
                        const gchar *text );
static void gsb_file_load_archive ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_bank ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_bet_future_data ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_bet_historical ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_bet_part ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_bet_transfert_part ( const gchar **attribute_names,
                        const gchar **attribute_values );
static gboolean gsb_file_load_check_new_structure ( gchar *file_content );
static void gsb_file_load_color_part ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_copy_old_file ( gchar *filename, gchar *file_content);
static void gsb_file_load_currency ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_currency_link ( const gchar **attribute_names,
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
static gint gsb_file_load_get_new_payment_number ( gint account_number,
                        gint payment_number );
static void gsb_file_load_import_rule ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_logo_accueil ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_partial_balance ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_party ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_payment_part ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_print_part ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_reconcile ( const gchar **attribute_names,
                        const gchar **attribute_values );
static void gsb_file_load_report_part_before_0_6 ( GMarkupParseContext *context,
                        const gchar *text );
static void gsb_file_load_scheduled_transactions ( const gchar **attribute_names,
                        const gchar **attribute_values );
static gboolean gsb_file_load_set_last_reconcile_data ( gint account_number,
                        GDate *final_date,
                        gsb_real final_balance );
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
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern GdkColor archive_background_color;
extern gint bet_array_col_width[BET_ARRAY_COLUMNS];
extern GdkColor calendar_entry_color;
extern GdkColor couleur_bet_division;
extern GdkColor couleur_bet_future;
extern GdkColor couleur_bet_solde;
extern GdkColor couleur_bet_transfert;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_grise;
extern GdkColor couleur_jour;
extern GdkColor couleur_selection;
extern gint display_one_line;
extern gint display_three_lines;
extern gint display_two_lines;
extern gsb_real error_real;
extern struct iso_4217_currency iso_4217_currencies[];
extern GtkWidget *logo_accueil;
extern gint no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern gsb_real null_real;
extern gint scheduler_col_width[SCHEDULER_COL_VISIBLE_COLUMNS];
extern GdkColor split_background;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];
extern GdkColor text_color[2];
extern gchar *titre_fichier;
extern gint transaction_col_align[CUSTOM_MODEL_N_VISIBLES_COLUMN];
extern gint transaction_col_width[CUSTOM_MODEL_N_VISIBLES_COLUMN];
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
    gboolean report_part;
} download_tmp_values = { FALSE, NULL, NULL, FALSE, FALSE, FALSE};

static gint account_number;

/* to import older file than 0.6, makes the link between category and sub-category */
static gint last_category = 0;
static gint last_sub_category_number = 0;

/* to import older file than 0.6, makes the link between budget and sub-budget */
static gint last_budget = 0;
static gint last_sub_budget_number = 0;

/* to import older file than 0.6, makes the link between report and comparison structures */
static gint last_report_number;

/* filled only when loading a version before 0.6, contains the order of the accounts
 * in the 0.6, the accounts are saved directly in the good order
 * this is a list of the accounts number, in the good order */
static GSList *sort_accounts = NULL;

/* temporary structure used to go from the 0.5.x versions to 0.6.x versions
 * because before, method of payment were saved in each account, and now
 * there is a general list of method of payment */
struct payment_conversion_struct
{
    gint account_number;
    gint last_payment_number;
    gint new_payment_number;
};
static GSList *payment_conversion_list = NULL;

/* temporary structure to set the final date and the final balance of a reconcile
 * in the reconcile itself, and not in the account as before 0.6.0 */
struct reconcile_conversion_struct
{
    gint reconcile_number;
    gint account_number;
    GDate *final_date;
    gsb_real final_balance;
};

gboolean result_reconcile = FALSE;
static GSList *reconcile_conversion_list = NULL;
static struct reconcile_conversion_struct *buffer_reconcile_conversion;

/* structure temporaire utilisée pour faire un lien entre ancien et nouveau numéro
 * de rapprochement et chaque compte parce que dans les versions précédentes de Grisbi
 * on pouvait utiliser un même rapprochement dans les opérations car celles-ci étaient 
 * organisées par comptes */
struct old_new_rec_conversion_struct
{
    gint account_number;
    gint old_rec_number;
    gint new_rec_number;
};
static GSList *old_new_rec_list = NULL;
static struct old_new_rec_conversion_struct *buffer_old_new_rec_conversion;

/* variable utilisée pour indiquer le chemin de la copie d'un fichier de compte 
 * d'une version antérieure de grisbi */
gchar *copy_old_filename = NULL;

/* structure temporaire pour le chargement d'un tiers/catégorie/imputation et sous-catégorie
 * sous-imputation */
struct new_div_sous_div_struct
{
    gint no_div;
    gint new_no_div;
    gint no_sub_div;
    gchar *name;
    gint type;

};
static struct new_div_sous_div_struct *buffer_new_div_sous_div;

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
    gulong length;

    devel_debug ( filename );

    /* general check */
    
    if ( !g_file_test (filename, G_FILE_TEST_EXISTS) )
    {
        gchar* tmpstr1 = g_strdup_printf ( _("Cannot open file '%s': %s"),
                        filename,
                        _("File does not exist") );
        gchar* tmpstr2 = g_strdup_printf ( _("Error loading file '%s'"), filename);
        dialogue_error_hint ( tmpstr1, tmpstr2 );
        g_free ( tmpstr1 );
        g_free ( tmpstr2 );
        gsb_file_remove_name_from_opened_list (filename);
        return FALSE;
    }

    /* check here if it's not a regular file */
    if ( !g_file_test ( filename, G_FILE_TEST_IS_REGULAR ))
    {
        gchar* tmpstr1 = g_strdup_printf ( 
                        _("%s doesn't seem to be a regular file,\nplease check it and try again."),
                        filename );
        gchar* tmpstr2 = g_strdup_printf ( _("Error loading file '%s'"), filename);
        dialogue_error_hint ( tmpstr1 , tmpstr2);
        g_free ( tmpstr1 );
        g_free ( tmpstr2 );
        gsb_file_remove_name_from_opened_list (filename);
        return ( FALSE );
    }

     /* fill the buffer stat to check the permission */
    return_value = g_stat (filename, &buffer_stat);
    /* check the access to the file and display a message */
#ifndef _WIN32
    if ( buffer_stat.st_mode != 33152 )
        gsb_file_util_display_warning_permissions ( );
#endif /* _WIN32 */

    /* load the file */
    if (gsb_file_util_get_contents (filename, &file_content, &length))
    {
        GMarkupParser *markup_parser = g_malloc0 (sizeof (GMarkupParser));
        GMarkupParseContext *context;
        gsb_plugin *plugin;

        /* first, we check if the file is crypted, if it is, we decrypt it */
        if ( !strncmp ( file_content, "Grisbi encrypted file ", 22 ) ||
             !strncmp ( file_content, "Grisbi encryption v2: ", 22 ) )
        {
            plugin = gsb_plugin_find ( "openssl" );
            if ( plugin )
            {
            gint (*crypt_function) ( gchar *, gchar **, gboolean, gulong );
            
            crypt_function = (gpointer) plugin -> plugin_run;
            length = crypt_function ( filename, &file_content, FALSE, length );
            
            if ( ! length )
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
            /* copy of an old file grisbi */
            gsb_file_load_copy_old_file ( filename, file_content );

            /* fill the GMarkupParser for the last xml structure */
            markup_parser -> start_element = (void *) gsb_file_load_start_element_before_0_6;
            markup_parser -> end_element = (void *) gsb_file_load_end_element_before_0_6;
            markup_parser -> text = (void *) gsb_file_load_text_element_before_0_6;

            /* we will have to convert the method of payments numbers */
            payment_conversion_list = NULL;
            /* we will have to set the final date and balance in the last reconcile */
            reconcile_conversion_list = NULL;
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
        gchar* tmpstr1 = g_strdup_printf (_("Cannot open file '%s': %s"),
                        filename,
                        g_strerror(errno));
        gchar* tmpstr2 = g_strdup_printf ( _("Error loading file '%s'"), filename);
        dialogue_error_hint (tmpstr1, tmpstr2);
        g_free ( tmpstr1 );
        g_free ( tmpstr2 );
        gsb_file_remove_name_from_opened_list (filename);
        return FALSE;
    }
    return gsb_file_load_update_previous_version();
}


/**
 *  check if the xml file is the last structure (before 0.6) or
 * the new structure (after 0.6)
 *
 * \param file_content the grisbi file
 *
 * \return TRUE if the version is after 0.6
 * */
gboolean gsb_file_load_check_new_structure ( gchar *file_content )
{
    if ( strstr ( file_content, "Generalites" ) )
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
                        "Color" ))
    {
    gsb_file_load_color_part ( attribute_names,
                        attribute_values );
    return;
    }

    if ( !strcmp ( element_name,
                        "Print" ))
    {
    gsb_file_load_print_part ( attribute_names,
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
                        "Currency_link" ))
    {
    gsb_file_load_currency_link ( attribute_names,
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
                        "Archive" ))
    {
    gsb_file_load_archive ( attribute_names,
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
                        "Import_rule" ))
    {
    gsb_file_load_import_rule ( attribute_names,
                        attribute_values );
    return;
    }

    if ( !strcmp ( element_name,
                        "Partial_balance" ))
    {
    gsb_file_load_partial_balance ( attribute_names,
                        attribute_values );
    return;
    }

    if ( !strcmp ( element_name, "Bet" ) )
    {
        gsb_file_load_bet_part ( attribute_names, attribute_values );
        return;
    }

    if ( !strcmp ( element_name, "Bet_historical" ) )
    {
        gsb_file_load_bet_historical ( attribute_names, attribute_values );
        return;
    }

    if ( !strcmp ( element_name, "Bet_future" ) )
    {
        gsb_file_load_bet_future_data ( attribute_names, attribute_values );
        return;
    }

    if ( !strcmp ( element_name, "Bet_transfert" ) )
    {
        gsb_file_load_bet_transfert_part ( attribute_names, attribute_values );
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
    if ( !strcmp ( element_name, "Logo" ) )
    {
        if ( etat.utilise_logo )
            gsb_file_load_logo_accueil ( attribute_names, attribute_values );
        return;
    }
    if ( !strcmp ( element_name, "Account_icon" ) )
    {
        gsb_file_load_account_icon_part ( attribute_names, attribute_values );
        return;
    }
    /* the first time we come here, we check if it's a grisbi file */
}


void gsb_file_load_error ( GMarkupParseContext *context,
                        GError *error,
                        gpointer user_data )
{
    /* the first time we come here, we check if it's a grisbi file */
    gchar* tmpstr = g_strdup_printf (
                        _("An error occured while parsing the file :\nError number : %d\n%s"),
                        error -> code,
                        error -> message );
    dialogue_error ( tmpstr );
    g_free ( tmpstr );
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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
                        "(null)"))
    {
        /* Nothing */
    }

    else if ( !strcmp ( attribute_names[i],
                        "File_version" ))
    {
        if ( download_tmp_values.file_version )
            g_free ( download_tmp_values.file_version );
        download_tmp_values.file_version = my_strdup (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Grisbi_version" ))
    {
        if ( download_tmp_values.grisbi_version )
            g_free ( download_tmp_values.grisbi_version );
        download_tmp_values.grisbi_version = my_strdup (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Crypt_file" ))
    {
        etat.crypt_file = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Archive_file" ))
    {
        etat.is_archive = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "File_title" ) && strlen (attribute_values[i]))
    {
        titre_fichier = my_strdup (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "General_address" ))
    {
        if ( adresse_commune )
            g_free ( adresse_commune );
        adresse_commune = my_strdup (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Second_general_address" ))
    {
        if ( adresse_secondaire )
            g_free ( adresse_secondaire );
        adresse_secondaire = my_strdup (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i], "Date_format" ) )
    {
        gsb_date_set_format_date ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Decimal_point" ) )
    {
        gsb_real_set_decimal_point ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Thousands_separator" ) )
    {
        if ( !strcmp ( attribute_values[i], "empty" ) )
            gsb_real_set_thousands_sep ( NULL );
        else
            gsb_real_set_thousands_sep ( attribute_values[i] );
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
                        "Import_extract_number_for_check" ))
    {
        etat.get_extract_number_for_check = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Import_fusion_transactions" ))
    {
        etat.get_fusion_import_transactions = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Import_categorie_for_payee" ))
    {
        etat.get_categorie_for_payee = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Import_fyear_by_value_date" ))
    {
        etat.get_fyear_by_value_date = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i], "Export_file_format" ) )
    {
        etat.export_file_format = utils_str_atoi ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Export_files_traitement" ) )
    {
        etat.export_files_traitement = utils_str_atoi ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Reconcile_end_date" ) )
    {
        etat.reconcile_end_date = utils_str_atoi ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i],
                        "Use_logo" ))
    {
        etat.utilise_logo = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i], "Name_logo" ) )
    {
        GdkPixbuf *pixbuf = NULL;

        etat.name_logo = my_strdup ( attribute_values[i] );
        if ( etat.is_pixmaps_dir )
        {
            gchar *chemin_logo = NULL;

            if ( etat.name_logo )
                chemin_logo = g_build_filename  ( GRISBI_PIXMAPS_DIR, etat.name_logo, NULL );
            else
                chemin_logo = g_build_filename  ( GRISBI_PIXMAPS_DIR, "grisbi-logo.png", NULL );
            if ( chemin_logo )
                pixbuf = gdk_pixbuf_new_from_file ( chemin_logo, NULL );
            if ( chemin_logo && strlen ( chemin_logo ) > 0 )
                g_free ( chemin_logo );

        }
        if ( pixbuf )
        {
            gtk_window_set_default_icon ( pixbuf );
            gsb_select_icon_set_logo_pixbuf ( pixbuf );
        }
    }

    else if ( !strcmp ( attribute_names[i], "Is_pixmaps_dir" ) )
    {
        etat.is_pixmaps_dir = utils_str_atoi ( attribute_values[i] );
        if ( etat.is_pixmaps_dir && etat.name_logo == NULL )
        {
            GdkPixbuf *pixbuf = NULL;
            gchar *chemin_logo = NULL;

            chemin_logo = g_build_filename  ( GRISBI_PIXMAPS_DIR, "grisbi-logo.png", NULL );
            pixbuf = gdk_pixbuf_new_from_file ( chemin_logo, NULL );
            gtk_window_set_default_icon ( pixbuf );
            gsb_select_icon_set_logo_pixbuf ( pixbuf );
            g_free ( chemin_logo );
        }
    }

    else if ( !strcmp ( attribute_names[i],
                        "Remind_display_per_account" ))
    {
        etat.retient_affichage_par_compte = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Transactions_view" ))
    {
        gchar **pointeur_char;
        gint j, k;

        pointeur_char = g_strsplit ( attribute_values[i],
                        "-",
                        0 );

        for ( j = 0 ; j<TRANSACTION_LIST_ROWS_NB ; j++ )
        for ( k = 0 ; k<CUSTOM_MODEL_VISIBLE_COLUMNS ; k++ )
            tab_affichage_ope[j][k] = utils_str_atoi ( 
                        pointeur_char[k + j*CUSTOM_MODEL_VISIBLE_COLUMNS]);

        g_strfreev ( pointeur_char );
    }

    else if ( !strcmp ( attribute_names[i],
                        "One_line_showed" ))
    {
        display_one_line = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Two_lines_showed" ))
    {
        display_two_lines = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Three_lines_showed" ))
    {
        display_three_lines = utils_str_atoi ( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i], "Transaction_column_width" ) )
    {
        /* initialise la réinitialisation des colonnes */
        run.transaction_column_width = my_strdup ( attribute_values[i] );

        initialise_largeur_colonnes_tab_affichage_ope ( GSB_ACCOUNT_PAGE,
                        run.transaction_column_width );
    }

    else if ( !strcmp ( attribute_names[i], "Transaction_column_align" ) )
    {
        gchar **pointeur_char;
        gint j;

        /* the transactions columns are xx-xx-xx-xx and we want to set in transaction_col_width[1-2-3...] */
        pointeur_char = g_strsplit ( attribute_values[i], "-", 0 );

        for ( j = 0 ; j < CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
            transaction_col_align[j] = utils_str_atoi ( pointeur_char[j] );

        g_strfreev ( pointeur_char );
    }

    else if ( !strcmp ( attribute_names[i], "Scheduler_column_width" ) )
    {
        /* initialise la réinitialisation des colonnes */
        run.scheduler_column_width = my_strdup ( attribute_values[i] );

        initialise_largeur_colonnes_tab_affichage_ope ( GSB_SCHEDULER_PAGE,
                        run.scheduler_column_width );
    }

    else if ( !strcmp ( attribute_names[i],
                        "Combofix_mixed_sort" ))
    {
        etat.combofix_mixed_sort = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Combofix_max_item" ))
    {
        etat.combofix_max_item = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Combofix_case_sensitive" ))
    {
        etat.combofix_case_sensitive = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Combofix_force_payee" ))
    {
        etat.combofix_force_payee = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Combofix_force_category" ))
    {
        etat.combofix_force_category = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Automatic_amount_separator" ))
    {
        etat.automatic_separator = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "CSV_separator" ))
    {
        if ( etat.csv_separator )
            g_free ( etat.csv_separator );
        etat.csv_separator = my_strdup ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i],
                        "CSV_skipped_lines" ))
    {
        if ( attribute_values[i] && strlen ( attribute_values[i] ) )
        {
        gchar ** pointeur_char = g_strsplit ( attribute_values[i], "-", 0 );
        gint line = 0;

        while ( pointeur_char[line] )
        {
            etat.csv_skipped_lines[line] = utils_str_atoi ( pointeur_char[line] );
            line ++;
        }
        g_strfreev ( pointeur_char );
        }
    }
    else if ( !strcmp ( attribute_names[i],
                        "Metatree_sort_transactions" ))
    {
        etat.metatree_sort_transactions = utils_str_atoi( attribute_values[i]);
    }
    else if ( !strcmp ( attribute_names[i],
                        "Add_archive_in_total_balance" ))
    {
        etat.add_archive_in_total_balance = utils_str_atoi( attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i], "Bet_array_column_width" ))
    {
        gchar **pointeur_char;
        gint j;

        /* the bet_array columns are xx-xx-xx-xx-xx and we want to set in bet_array_col_width[1-2-3...] */
        pointeur_char = g_strsplit ( attribute_values[i], "-", 0 );

        for ( j = 0; j < BET_ARRAY_COLUMNS; j++ )
            bet_array_col_width[j] = utils_str_atoi ( pointeur_char[j] );

        g_strfreev ( pointeur_char );
    }

    else if ( !strcmp ( attribute_names[i], "Bet_capital" ) )
    {
        etat.bet_capital = utils_str_safe_strtod ( attribute_values[i], NULL );
    }

    else if ( !strcmp ( attribute_names[i], "Bet_currency" ) )
    {
        etat.bet_currency = utils_str_atoi ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Bet_taux_annuel" ) )
    {
        etat.bet_taux_annuel = utils_str_safe_strtod ( attribute_values[i], NULL );
    }

    else if ( !strcmp ( attribute_names[i], "Bet_index_duree" ) )
    {
        etat.bet_index_duree = utils_str_atoi ( attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Bet_frais" ) )
    {
        etat.bet_frais = utils_str_safe_strtod ( attribute_values[i], NULL );
    }

    else if ( !strcmp ( attribute_names[i], "Bet_type_taux" ) )
    {
        etat.bet_type_taux = utils_str_atoi ( attribute_values[i] );
    }

    i++;
    }
    while ( attribute_names[i] );
}

/**
 * load the color part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_color_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
                        "(null)"))
    {
        /* Nothing */
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_color_0_red" ))
    {
        couleur_fond[0].red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_color_0_green" ))
    {
        couleur_fond[0].green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_color_0_blue" ))
    {
        couleur_fond[0].blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_color_1_red" ))
    {
        couleur_fond[1].red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_color_1_green" ))
    {
        couleur_fond[1].green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_color_1_blue" ))
    {
        couleur_fond[1].blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_jour_red" ))
    {
        couleur_jour.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_jour_green" ))
    {
        couleur_jour.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_jour_blue" ))
    {
        couleur_jour.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_scheduled_red" ))
    {
        couleur_grise.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_scheduled_green" ))
    {
        couleur_grise.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_scheduled_blue" ))
    {
        couleur_grise.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_archive_red" ))
    {
        archive_background_color.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_archive_green" ))
    {
        archive_background_color.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_archive_blue" ))
    {
        archive_background_color.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Selection_red" ))
    {
        couleur_selection.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Selection_green" ))
    {
        couleur_selection.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Selection_blue" ))
    {
        couleur_selection.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_split_red" ))
    {
        split_background.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_split_green" ))
    {
        split_background.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Background_split_blue" ))
    {
        split_background.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Text_color_0_red" ))
    {
        text_color[0].red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Text_color_0_green" ))
    {
        text_color[0].green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Text_color_0_blue" ))
    {
        text_color[0].blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Text_color_1_red" ))
    {
        text_color[1].red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Text_color_1_green" ))
    {
        text_color[1].green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Text_color_1_blue" ))
    {
        text_color[1].blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Calendar_entry_red" ))
    {
        calendar_entry_color.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Calendar_entry_green" ))
    {
        calendar_entry_color.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Calendar_entry_blue" ))
    {
        calendar_entry_color.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_division_red" ))
    {
        couleur_bet_division.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_division_green" ))
    {
        couleur_bet_division.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_division_blue" ))
    {
        couleur_bet_division.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_future_red" ))
    {
        couleur_bet_future.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_future_green" ))
    {
        couleur_bet_future.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_future_blue" ))
    {
        couleur_bet_future.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_solde_red" ))
    {
        couleur_bet_solde.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_solde_green" ))
    {
        couleur_bet_solde.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_solde_blue" ))
    {
        couleur_bet_solde.blue = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_transfert_red" ))
    {
        couleur_bet_transfert.red = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_transfert_green" ))
    {
        couleur_bet_transfert.green = utils_str_atoi (attribute_values[i]);
    }

    else if ( !strcmp ( attribute_names[i],
                        "Couleur_bet_transfert_blue" ))
    {
        couleur_bet_transfert.blue = utils_str_atoi (attribute_values[i]);
    }

    i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the print part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_print_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
                        "(null)"))
    {
        /* Nothing */
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_lines" ))
    {
        gsb_data_print_config_set_draw_lines (0, utils_str_atoi (attribute_values[i]));
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_column" ))
    {
        gsb_data_print_config_set_draw_column (0, utils_str_atoi (attribute_values[i]));
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_background" ))
    {
        gsb_data_print_config_set_draw_background (0, utils_str_atoi (attribute_values[i]));
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_archives" ))
    {
        gsb_data_print_config_set_draw_archives (0, utils_str_atoi (attribute_values[i]));
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_columns_name" ))
    {
        gsb_data_print_config_set_draw_columns_name (0, utils_str_atoi (attribute_values[i]));
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_title" ))
    {
        gsb_data_print_config_set_draw_title (0, utils_str_atoi (attribute_values[i]));
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_interval_dates" ))
    {
        gsb_data_print_config_set_draw_interval_dates (
                        0, utils_str_atoi (attribute_values[i]) );
    }

    else if ( !strcmp ( attribute_names[i],
                        "Draw_dates_are_value_dates" ))
    {
        gsb_data_print_config_set_draw_dates_are_value_dates (
                        0, utils_str_atoi (attribute_values[i]) );
    }

    else if ( !strcmp ( attribute_names[i],
                        "Font_transactions" ))
    {
        gsb_data_print_config_set_font_transaction (
                        pango_font_description_from_string (attribute_values[i]) );
    }

    else if ( !strcmp ( attribute_names[i],
                        "Font_title" ))
    {
        gsb_data_print_config_set_font_title (
                        pango_font_description_from_string (attribute_values[i]) );
    }

    else if ( !strcmp ( attribute_names[i],
                        "Report_font_transactions" ) )
    {
        gsb_data_print_config_set_report_font_transaction (
                        pango_font_description_from_string (attribute_values[i]) );
    }

    else if ( !strcmp ( attribute_names[i],
                        "Report_font_title" ) )
    {
        gsb_data_print_config_set_report_font_title (
                        pango_font_description_from_string ( attribute_values[i]) );
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
    gint account_number = 0;

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
                        "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Name" ))
    {
        account_number = gsb_data_account_new ( GSB_TYPE_BANK );
        gsb_data_account_set_name ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Id" ))
    {
        if ( strlen (attribute_values[i]))
        gsb_data_account_set_id (account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Number" ))
    {
        account_number = gsb_data_account_set_account_number ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Owner" ))
    {
        gsb_data_account_set_holder_name ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Kind" ))
    {
        gsb_data_account_set_kind (account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Currency" ))
    {
        gsb_data_account_set_currency ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Path_icon" ) )
    {
        gsb_data_account_set_name_icon ( account_number,
                        attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Bank" ))
    {
        gsb_data_account_set_bank ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Bank_branch_code" ))
    {
        gsb_data_account_set_bank_branch_code ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Bank_account_number" ))
    {
        gsb_data_account_set_bank_account_number ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Key" ))
    {
        gsb_data_account_set_bank_account_key ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Bank_account_IBAN" ))
    {
        gsb_data_account_set_bank_account_iban ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Initial_balance" ))
    {
        gsb_data_account_set_init_balance ( account_number,
                        gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Minimum_wanted_balance" ))
    {
        gsb_data_account_set_mini_balance_wanted ( account_number, 
                        gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Minimum_authorised_balance" ))
    {
        gsb_data_account_set_mini_balance_authorized ( account_number, 
                        gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Closed_account" ))
    {
        gsb_data_account_set_closed_account ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Show_marked" ))
    {
        gsb_data_account_set_r ( account_number,
                        utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Show_archives_lines" ))
    {
        gsb_data_account_set_l ( account_number,
                        utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Lines_per_transaction" ))
    {
        gsb_data_account_set_nb_rows ( account_number, 
                        utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Comment" ))
    {
        gsb_data_account_set_comment ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Owner_address" ))
    {
        if ( g_strstr_len ( attribute_values[i], -1, "&#xA;" ) )
        {
            gchar **owner_tab;
            gchar *owner_str;

            owner_tab = g_strsplit ( attribute_values[i], "&#xA;", 0 );
            owner_str = g_strjoinv ( NEW_LINE, owner_tab );
            gsb_data_account_set_holder_address ( account_number, owner_str );

            g_free ( owner_str );
            g_strfreev ( owner_tab );
        }
        else
            gsb_data_account_set_holder_address ( account_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Default_debit_method" ))
    {
        gsb_data_account_set_default_debit ( account_number,
                        utils_str_atoi ( attribute_values[i]) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Default_credit_method" ))
    {
        gsb_data_account_set_default_credit ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Sort_by_method" ))
    {
        gsb_data_account_set_reconcile_sort_type ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Neutrals_inside_method" ))
    {
        gsb_data_account_set_split_neutral_payment ( account_number,
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
            gsb_data_account_sort_list_add ( account_number,
                        utils_str_atoi ( pointeur_char[j] ));
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
        gsb_data_account_set_sort_type ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Column_sort" ))
    {
        gsb_data_account_set_sort_column ( account_number,
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

        for ( j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
        {
        gsb_data_account_set_element_sort ( account_number,
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
        gsb_data_form_new_organization (account_number);
        gsb_data_form_set_nb_columns ( account_number,
                        utils_str_atoi ( attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Form_lines_number" ))
    {
        gsb_data_form_set_nb_rows ( account_number,
                        utils_str_atoi ( attribute_values[i]));
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

        for ( k=0 ; k<MAX_HEIGHT ; k++ )
        for ( j=0 ; j<MAX_WIDTH ; j++ )
            gsb_data_form_set_value ( account_number,
                        j, k,
                        utils_str_atoi ( pointeur_char[j + k*MAX_WIDTH]));

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

        for ( j=0 ; j<MAX_WIDTH ; j++ )
        gsb_data_form_set_width_column ( account_number,
                        j,
                        utils_str_atoi ( pointeur_char[j]));

        g_strfreev ( pointeur_char );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_use_budget" ))
    {
        gsb_data_account_set_bet_use_budget ( account_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_start_date" ))
    {
        gsb_data_account_set_bet_start_date ( account_number,
                        gsb_parse_date_string_safe ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_months" ))
    {
        gsb_data_account_set_bet_months ( account_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_UT" ))
    {
        gsb_data_account_set_bet_spin_range ( account_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_auto_inc_month" ))
    {
        gboolean auto_inc_month;

        auto_inc_month = utils_str_atoi ( attribute_values[i] );
        gsb_data_account_set_bet_auto_inc_month ( account_number, auto_inc_month );
        if ( auto_inc_month )
            gsb_data_account_bet_update_initial_date_if_necessary ( account_number );
            
        i++;
        continue;
    }
    if ( !strcmp ( attribute_names[i], "Bet_select_transaction_label" ))
    {
        gsb_data_account_set_bet_select_label ( account_number,
                        SPP_ORIGIN_TRANSACTION,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_select_scheduled_label" ))
    {
        gsb_data_account_set_bet_select_label ( account_number,
                        SPP_ORIGIN_SCHEDULED,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_select_futur_label" ))
    {
        gsb_data_account_set_bet_select_label ( account_number,
                        SPP_ORIGIN_FUTURE,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_SD" ))
    {
        gsb_data_account_set_bet_hist_data ( account_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_Fi" ))
    {
        gsb_data_account_set_bet_hist_fyear ( account_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_capital" ))
    {
        gsb_data_account_set_bet_finance_capital ( account_number,
                        g_ascii_strtod ( attribute_values[i], NULL ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_taux_annuel" ))
    {
        gsb_data_account_set_bet_finance_taux_annuel ( account_number,
                        g_ascii_strtod ( attribute_values[i], NULL ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_frais" ))
    {
        gsb_data_account_set_bet_finance_frais ( account_number,
                        g_ascii_strtod ( attribute_values[i], NULL ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_type_taux" ))
    {
        gsb_data_account_set_bet_finance_type_taux ( account_number,
                        utils_str_atoi ( attribute_values[i] ) );
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
    gint payment_number;

    if ( !attribute_names[i] )
    return;
    
    payment_number = gsb_data_payment_new (NULL);

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
                        "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Number" ))
    {
        payment_number = gsb_data_payment_set_new_number ( payment_number,
                        utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Name" ))
    {
        gsb_data_payment_set_name ( payment_number, 
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Sign" ))
    {
        gsb_data_payment_set_sign ( payment_number,
                    utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Show_entry" ))
    {
        gsb_data_payment_set_show_entry ( payment_number,
                        utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Automatic_number" ))
    {
        gsb_data_payment_set_automatic_numbering ( payment_number,
                        utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Current_number" ))
    {
        gsb_data_payment_set_last_number ( payment_number,
                         attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Account" ))
    {
        gsb_data_payment_set_account_number ( payment_number,
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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

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
                        gsb_parse_date_string_safe (attribute_values[i]));
        i++;
        continue;
    }


    if ( !strcmp ( attribute_names[i],
               "Dv" ))
    {
        gsb_data_transaction_set_value_date ( transaction_number,
                          gsb_parse_date_string_safe (attribute_values[i]));
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
               "Am" ))
    {
        /* get the entire real, even if the floating point of the currency is less deep */
        gsb_data_transaction_set_amount ( transaction_number,
                          gsb_real_safe_real_from_string (attribute_values[i]));
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
                             gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }


    if ( !strcmp ( attribute_names[i],
               "Exf" ))
    {
        gsb_data_transaction_set_exchange_fees ( transaction_number,
                             gsb_real_safe_real_from_string (attribute_values[i]));
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
        gsb_data_transaction_set_split_of_transaction ( transaction_number,
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
               "Ar" ))
    {
        gsb_data_transaction_set_archive_number ( transaction_number,
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
        gsb_data_transaction_set_contra_transaction_number ( transaction_number,
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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

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
                      gsb_parse_date_string_safe (attribute_values[i]));
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
                        gsb_real_safe_real_from_string (attribute_values[i]));
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
        GDate *date;
        date = gsb_parse_date_string_safe (attribute_values[i]);
        gsb_data_scheduled_set_limit_date ( scheduled_number,
                        date);
        if (date)
        g_date_free (date);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Br" ))
    {
        gsb_data_scheduled_set_split_of_scheduled ( scheduled_number,
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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

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

    if ( !strcmp ( attribute_names[i],
                                   "Search" ))
    {
        if ( attribute_values[i] && strlen (attribute_values[i]) > 0 )
        {
            gsb_data_payee_set_search_string ( payee_number,
                                           attribute_values[i]);
            gsb_import_associations_list_append_assoc ( payee_number,
                    attribute_values[i] );
        }
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

    if ( buffer_new_div_sous_div )
        g_free ( buffer_new_div_sous_div );
    buffer_new_div_sous_div = g_malloc0 ( sizeof ( struct new_div_sous_div_struct ) );

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Nb" ))
    {
        buffer_new_div_sous_div -> no_div = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Na" ))
    {
        buffer_new_div_sous_div -> name = g_strdup ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Kd" ))
    {
        buffer_new_div_sous_div -> type = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

    buffer_new_div_sous_div -> new_no_div = gsb_data_category_test_create_category (
                        buffer_new_div_sous_div -> no_div,
                        buffer_new_div_sous_div -> name,
                        buffer_new_div_sous_div -> type );
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

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

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
        if ( category_number == buffer_new_div_sous_div -> no_div )
            buffer_new_div_sous_div -> no_sub_div = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Na" ))
    {
        if ( category_number == buffer_new_div_sous_div -> no_div )
            buffer_new_div_sous_div -> name = g_strdup ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

    if ( !gsb_data_category_test_create_sub_category (
                            buffer_new_div_sous_div -> new_no_div,
                            buffer_new_div_sous_div -> no_sub_div,
							buffer_new_div_sous_div -> name ) )
    {
        gchar *tmpstr = g_strdup_printf ( "no_category = %d no_sub_category = %d nom = %s\n",
                            buffer_new_div_sous_div -> new_no_div,
                            buffer_new_div_sous_div -> no_sub_div,
							buffer_new_div_sous_div -> name );
        devel_debug ( tmpstr );
    }

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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Nbb" )
     ||
     !strcmp ( attribute_names[i], "Nbc" )  )
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
    gint currency_number;

    if ( !attribute_names[i] )
    return;

    currency_number = gsb_data_currency_new (NULL);

    /* Default */
    gsb_data_currency_set_floating_point ( currency_number, 2 );

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Nb" ))
    {
        currency_number = gsb_data_currency_set_new_number ( currency_number,
                                                         utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Na" ))
    {
        gsb_data_currency_set_name ( currency_number,
                                         attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Co" ))
    {
        struct iso_4217_currency * currency = iso_4217_currencies;

        gsb_data_currency_set_code ( currency_number, attribute_values[i]);

        /* Check if a iso code is the same as currency code (old import).  */
        while ( currency -> country_name )
        {
                            if ( !strcmp ( currency -> currency_code, attribute_values[i] ) )
                            {
                                gsb_data_currency_set_code_iso4217 ( currency_number,
                            attribute_values[i]);
                            }
                            currency++;
        }

        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Ico" ))
    {
        gsb_data_currency_set_code_iso4217 ( currency_number,
                             attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Fl" ))
    {
        gsb_data_currency_set_floating_point ( currency_number,
                               utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

    /* initialization of the currency for the payees, categories and
     * budgetary lines in case of need */
    if ( no_devise_totaux_tiers == 0 )
    {
        GSList *tmp_list;

        tmp_list = gsb_data_currency_get_currency_list ( );
        if ( g_slist_length ( tmp_list ) > 0 )
            no_devise_totaux_tiers = gsb_data_currency_get_no_currency (
                g_slist_nth_data ( tmp_list, 0 ) );
    }
    if ( no_devise_totaux_categ == 0 )
    {
        GSList *tmp_list;

        tmp_list = gsb_data_currency_get_currency_list ( );
        if ( g_slist_length ( tmp_list ) > 0 )
            no_devise_totaux_categ = gsb_data_currency_get_no_currency (
                g_slist_nth_data ( tmp_list, 0 ) );
    }
    if ( no_devise_totaux_ib == 0 )
    {
        GSList *tmp_list;

        tmp_list = gsb_data_currency_get_currency_list ( );
        if ( g_slist_length ( tmp_list ) > 0 )
            no_devise_totaux_ib = gsb_data_currency_get_no_currency (
                g_slist_nth_data ( tmp_list, 0 ) );
    }
}


/**
 * load the currency_links in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_currency_link ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint link_number;

    if ( !attribute_names[i] )
    return;

    link_number = gsb_data_currency_link_new (0);

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Nb" ))
    {
        link_number = gsb_data_currency_link_set_new_number ( link_number,
                                                          utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Cu1" ))
    {
        gsb_data_currency_link_set_first_currency ( link_number,
                                utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Cu2" ))
    {
        gsb_data_currency_link_set_second_currency ( link_number,
                                 utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Ex" ))
    {
        gsb_data_currency_link_set_change_rate ( link_number,
                                 gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Modified_date" ) )
    {
        gsb_data_currency_link_set_modified_date ( link_number,
                        gsb_parse_date_string_safe (attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Fl" ))
    {
        gsb_data_currency_link_set_fixed_link ( link_number,
                                 utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );
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
    gint bank_number = 0;

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Nb" ))
    {
        bank_number = gsb_data_bank_set_new_number ( gsb_data_bank_new (NULL),
                                 utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Na" ))
    {
        gsb_data_bank_set_name ( bank_number,
                                         attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Co" ))
    {
        gsb_data_bank_set_code ( bank_number,
                                         attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "BIC" ))
    {
        gsb_data_bank_set_bic ( bank_number,
                                         attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Adr" ))
    {
        gsb_data_bank_set_bank_address ( bank_number,
                                             attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Tel" ))
    {
        gsb_data_bank_set_bank_tel ( bank_number,
                                         attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                                   "Mail" ))
    {
        gsb_data_bank_set_bank_mail ( bank_number,
                attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Web" ))
    {
        gsb_data_bank_set_bank_web ( bank_number,
                     attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Nac" ))
    {
        gsb_data_bank_set_correspondent_name ( bank_number,
                               attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Faxc" ))
    {
        gsb_data_bank_set_correspondent_fax ( bank_number,
                              attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Telc" ))
    {
        gsb_data_bank_set_correspondent_tel ( bank_number,
                              attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Mailc" ))
    {
        gsb_data_bank_set_correspondent_mail ( bank_number,
                               attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Rem" ))
    {
        gsb_data_bank_set_bank_note ( bank_number,
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
    gint fyear_number;
    GDate *date;

    if ( !attribute_names[i] )
    return;

    fyear_number = gsb_data_fyear_new (NULL);

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Nb" ))
    {
        fyear_number = gsb_data_fyear_set_new_number (fyear_number,
                                  utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Na" ))
    {
        gsb_data_fyear_set_name ( fyear_number,
                      attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Bdte" ))
    {
        date = gsb_parse_date_string_safe (attribute_values[i]);
        gsb_data_fyear_set_beginning_date ( fyear_number,
                           date );
        if ( date )
        {
        g_date_free (date);
        }
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Edte" ))
    {
        date = gsb_parse_date_string_safe (attribute_values[i]);
        gsb_data_fyear_set_end_date ( fyear_number,
                      date );
        if ( date )
        {
        g_date_free (date);
        }
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Sho" ))
    {
        gsb_data_fyear_set_form_show ( fyear_number,
                       utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

    gsb_data_fyear_check_for_invalid (fyear_number);
}

/**
 * load the archives in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_archive ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint archive_number;
    GDate *date;

    if ( !attribute_names[i] )
    return;

    archive_number = gsb_data_archive_new (NULL);

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp (attribute_values[i],
                "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                "Nb" ))
    {
        archive_number = gsb_data_archive_set_new_number (archive_number,
                                utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                "Na" ))
    {
        gsb_data_archive_set_name ( archive_number,
                    attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                "Bdte" ))
    {
        date = gsb_parse_date_string_safe (attribute_values[i]);
        gsb_data_archive_set_beginning_date ( archive_number,
                            date );
        if ( date )
        g_date_free (date);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Edte" ))
    {
        date = gsb_parse_date_string_safe (attribute_values[i]);
        gsb_data_archive_set_end_date ( archive_number,
                        date );
        if ( date )
        g_date_free (date);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Fye" ))
    {
        gsb_data_archive_set_fyear ( archive_number,
                     utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Rep" ))
    {
        gsb_data_archive_set_report_title ( archive_number,
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
 * load the reconcile structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_reconcile ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint reconcile_number = 0;

    if ( !attribute_names[i] )
    return;

    reconcile_number = gsb_data_reconcile_new (NULL);

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Nb" ))
    {
        reconcile_number = gsb_data_reconcile_set_new_number ( reconcile_number,
                                       utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Na" ))
    {
        gsb_data_reconcile_set_name ( reconcile_number,
                      attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Acc" ))
    {
        gsb_data_reconcile_set_account ( reconcile_number,
                         utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Idate" ))
    {
        gsb_data_reconcile_set_init_date ( reconcile_number,
                           gsb_parse_date_string_safe (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Fdate" ))
    {
        gsb_data_reconcile_set_final_date ( reconcile_number,
                            gsb_parse_date_string_safe (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Ibal" ))
    {
        gsb_data_reconcile_set_init_balance ( reconcile_number,
                              gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Fbal" ))
    {
        gsb_data_reconcile_set_final_balance ( reconcile_number,
                               gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );
}

/**
 * load the import rules structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_import_rule ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint import_rule_number = 0;

    if ( !attribute_names[i] )
    return;

    import_rule_number = gsb_data_import_rule_new (NULL);

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Nb" ))
    {
        import_rule_number = gsb_data_import_rule_set_new_number ( import_rule_number,
                                           utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Na" ))
    {
        gsb_data_import_rule_set_name ( import_rule_number,
                        attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Acc" ))
    {
        gsb_data_import_rule_set_account ( import_rule_number,
                           utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Cur" ))
    {
        gsb_data_import_rule_set_currency ( import_rule_number,
                            utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Inv" ))
    {
        gsb_data_import_rule_set_invert ( import_rule_number,
                          utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Enc" ))
    {
        gsb_data_import_rule_set_charmap ( import_rule_number,
                                  attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Fil" ))
    {
        gsb_data_import_rule_set_last_file_name ( import_rule_number,
                                  attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Act" ))
    {
        gsb_data_import_rule_set_action ( import_rule_number,
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
 * load the partial balance structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_partial_balance ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint partial_balance_number = 0;

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp ( attribute_values[i],
         "(null)") )
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Nb" ) )
    {
        partial_balance_number = gsb_partial_balance_new_at_position  (
                        NULL,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Na" ) )
    {
        gsb_data_partial_balance_set_name ( partial_balance_number,
                        attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Acc" ) )
    {
        gsb_data_partial_balance_set_liste_cptes ( partial_balance_number,
                           attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Kind" ) )
    {
        gsb_data_partial_balance_set_kind ( partial_balance_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Currency" ) )
    {
        gsb_data_partial_balance_set_currency ( partial_balance_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
                        "Colorise" ) )
    {
        gsb_data_partial_balance_set_colorise ( partial_balance_number,
                        utils_str_atoi ( attribute_values[i] ) );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );
}


/**
 * load the balance estimate part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_bet_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp ( attribute_values[i], "(null)") )
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ddte" ) )
    {
        etat.bet_deb_period = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_deb_cash_account_option" ) )
    {
        etat.bet_deb_cash_account_option = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }

    while ( attribute_names[i] );
}


/**
 * load the historical balance part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_bet_historical ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    struct_hist_div *shd;
    struct_hist_div *sub_shd = NULL;
    gint i=0;
    gint sub_div_nb;

    if ( !attribute_names[i] )
    return;

    /* create the structure */
    shd = struct_initialise_hist_div ( );

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */
    if ( !strcmp ( attribute_values[i], "(null)") )
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ac" ) )
    {
        shd -> account_nb = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ori" ) )
    {
        shd -> origin = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Div" ) )
    {
        shd -> div_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Edit" ) )
    {
        shd -> div_edited = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Damount" ) )
    {
        shd -> amount = gsb_real_safe_real_from_string ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "SDiv" ) )
    {
        sub_div_nb = utils_str_atoi ( attribute_values[i] );
        if ( sub_div_nb > 0 )
        {
            sub_shd = struct_initialise_hist_div ( );
            sub_shd -> div_number = sub_div_nb;
            i++;
            continue;
        }
        else
            break;
    }

    if ( !strcmp ( attribute_names[i], "SEdit" ) )
    {
        sub_shd -> div_edited = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "SDamount" ) )
    {
        sub_shd -> amount = gsb_real_safe_real_from_string ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }

    while ( attribute_names[i] );

    bet_data_insert_div_hist ( shd, sub_shd );
}


/**
 * load the bet future data
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_bet_future_data ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    struct_futur_data *scheduled;

    if ( !attribute_names[i] )
    return;

    scheduled = struct_initialise_bet_future ( );

    if ( !scheduled )
    {
        dialogue_error_memory ();
        return;
    }

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i], "(null)") )
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Nb" ) )
    {
        scheduled -> number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Dt" ) )
    {
        scheduled -> date = gsb_parse_date_string_safe ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ac" ) )
    {
        scheduled -> account_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Am" ) )
    {
        scheduled -> amount = gsb_real_safe_real_from_string ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Pa" ) )
    {
        scheduled -> party_number =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "IsT" ) )
    {
        scheduled -> is_transfert =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Tra" ) )
    {
        scheduled -> account_transfert =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ca" ) )
    {
        scheduled -> category_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Sca" ) )
    {
        scheduled -> sub_category_number =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Pn" ) )
    {
        scheduled -> payment_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Fi" ) )
    {
        scheduled -> fyear_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bu" ) )
    {
        scheduled -> budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Sbu" ) )
    {
        scheduled -> sub_budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "No" ) )
    {
        scheduled -> notes = g_strdup ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Pe" ) )
    {
        scheduled -> frequency = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Pei" ) )
    {
        scheduled -> user_interval = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Pep" ) )
    {
        scheduled -> user_entry = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Dtl" ) )
    {
        GDate *date;

        date = gsb_parse_date_string_safe ( attribute_values[i] );
        if ( g_date_valid ( date ) )
            scheduled -> limit_date = date;
        else
            scheduled -> limit_date = NULL;
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Mo" ) )
    {
        scheduled -> mother_row = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

    bet_data_future_set_lines_from_file ( scheduled );
}


/**
 * load the bet transfert line
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_bet_transfert_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    struct_transfert_data *transfert;

    if ( !attribute_names[i] )
    return;

    transfert = struct_initialise_bet_transfert ( );

    if ( !transfert )
    {
        dialogue_error_memory ();
        return;
    }

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i], "(null)") )
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Nb" ) )
    {
        transfert -> number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Dt" ) )
    {
        transfert -> date = gsb_parse_date_string_safe ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ac" ) )
    {
        transfert -> account_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ty" ) )
    {
        transfert -> type = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ra" ) )
    {
        transfert -> replace_account =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Rt" ) )
    {
        transfert -> replace_transaction = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Aim" ) )
    {
        transfert -> auto_inc_month = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ca" ) )
    {
        transfert -> category_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Sca" ) )
    {
        transfert -> sub_category_number =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bu" ) )
    {
        transfert -> budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Sbu" ) )
    {
        transfert -> sub_budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

    bet_data_transfert_set_line_from_file ( transfert );
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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Nb" ))
    {
        /* if the number is -1, it means we are importing a report, so let grisbi choose the
         * report number */
        if (utils_str_atoi (attribute_values[i]) == -1)
        report_number = gsb_data_report_new (NULL);
        else
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
                           gsb_string_get_int_list_from_string (attribute_values[i],
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
               "Show_transaction_value_date" ))
    {
        gsb_data_report_set_show_report_value_date ( report_number,
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
               "Show_exclude_split_child" ))
    {
        gsb_data_report_set_not_detail_split ( report_number,
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
                                  gsb_string_get_int_list_from_string (attribute_values[i],
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
               "Date_beginning" ))
    {
        gsb_data_report_set_personal_date_start ( report_number,
                                  gsb_parse_date_string_safe (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Date_end" ))
    {
        gsb_data_report_set_personal_date_end ( report_number,
                                gsb_parse_date_string_safe (attribute_values[i]));
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
               "Split_day_beginning" ))
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
                              gsb_string_get_int_list_from_string (attribute_values[i],
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
                                   gsb_string_get_int_list_from_string (attribute_values[i],
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
        gsb_data_report_set_category_struct ( report_number,
                              gsb_string_get_categ_budget_struct_list_from_string ((attribute_values[i])));
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
        gsb_data_report_set_budget_struct ( report_number,
                            gsb_string_get_categ_budget_struct_list_from_string ((attribute_values[i])));
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
                            gsb_string_get_int_list_from_string (attribute_values[i],
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
               "Payment_method_use" ))
    {
        gsb_data_report_set_method_of_payment_used ( report_number,
                                 utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Payment_method_list" ))
    {
        gsb_data_report_set_method_of_payment_list ( report_number,
                                 gsb_string_get_string_list_from_string (attribute_values[i],
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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Comparison_number" ))
    {
        /* if comparison number is -1, it's an import of report,
         * so let grisbi choose the good number */
        if (utils_str_atoi (attribute_values[i]) == -1)
        text_comparison_number = gsb_data_report_text_comparison_new (0);
        else
        text_comparison_number = gsb_data_report_text_comparison_new (utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Report_nb" ))
    {
        report_number = utils_str_atoi (attribute_values[i]);

        /* if report_number = -1, it's an import of report,
         * so that comparison structure must be associated to the last report_number saved */
        if (report_number == -1)
        {
        report_number = gsb_data_report_max_number ();
        gsb_data_report_text_comparison_set_report_number ( text_comparison_number,
                                        report_number);
        }
        else
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
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
              "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Comparison_number" ))
    {
        /* if comparison number is -1, it's an import of report,
         * so let grisbi choose the good number */
        if (utils_str_atoi (attribute_values[i]) == -1)
        amount_comparison_number = gsb_data_report_amount_comparison_new (0);
        else
        amount_comparison_number = gsb_data_report_amount_comparison_new (utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Report_nb" ))
    {
        report_number = utils_str_atoi (attribute_values[i]);

        /* if report_number = -1, it's an import of report,
         * so that comparison structure must be associated to the last report_number saved */
        if (report_number == -1)
        {
        report_number = gsb_data_report_max_number ();
        gsb_data_report_amount_comparison_set_report_number ( amount_comparison_number,
                                          report_number);
        }
        else
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
                                     gsb_real_safe_real_from_string (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Amount_2" ))
    {
        gsb_data_report_amount_comparison_set_second_amount ( amount_comparison_number,
                                     gsb_real_safe_real_from_string (attribute_values[i]));
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
 * load the logo_accueil in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_logo_accueil ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;

    do
    {
        /*     we test at the beginning if the attribute_value is NULL, if yes, */
        /*        go to the next */

        if ( !strcmp (attribute_values[i],
                  "(null)"))
        {
            i++;
            continue;
        }
        if ( !strcmp ( attribute_names[i], "Image" ) )
        {
            GdkPixbuf *pixbuf = NULL;

            pixbuf = gsb_select_icon_create_pixbuf_from_chaine_base64 ( 
                                (gchar *) attribute_values[i] );
            etat.is_pixmaps_dir = FALSE;

            gtk_window_set_default_icon ( pixbuf );
            gsb_select_icon_set_logo_pixbuf ( pixbuf );
            i++;
            continue;
        }
    }
    while ( attribute_names[i] );
}


/**
 * charge les icones pour les comptes
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
void gsb_file_load_account_icon_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint account_number = -1;
    GdkPixbuf *pixbuf = NULL;

    do
    {
        /*     we test at the beginning if the attribute_value is NULL, if yes, */
        /*        go to the next */

        if ( !strcmp (attribute_values[i], "(null)"))
        {
            i++;
            continue;
        }
        if ( !strcmp ( attribute_names[i], "Account_number" ) )
        {
            account_number = utils_str_atoi ( attribute_values[i] );
            i++;
            continue;
        }
        
        if ( !strcmp ( attribute_names[i], "Image" ) )
        {
            pixbuf = gsb_select_icon_create_pixbuf_from_chaine_base64 ( 
                                (gchar *) attribute_values[i] );
            i++;
            continue;
        }
    }
    while ( attribute_names[i] );

    if ( account_number != -1 && pixbuf )
    {
        gsb_select_icon_new_account_icon ( account_number, pixbuf );
        gsb_data_account_set_account_icon_pixbuf ( account_number, pixbuf );
    }
}


void gsb_file_load_start_element_before_0_6 ( GMarkupParseContext *context,
                        const gchar *element_name,
                        const gchar **attribute_names,
                        const gchar **attribute_values,
                        gpointer user_data,
                        GError **error)
{
    /* the first time we come here, we check if it's a grisbi file */
    gchar **pointeur_char;

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
        gint payment_number;
        gint last_number = 0;
        struct payment_conversion_struct *conversion;

        payment_number = gsb_data_payment_new (NULL);

        do
        {
        if ( !strcmp ( attribute_names[i],
                   "No" ))
            /* we just save the last number to do the conversion later */
            last_number = utils_str_atoi (attribute_values[i]);

        if ( !strcmp ( attribute_names[i],
                   "Nom" ))
            gsb_data_payment_set_name ( payment_number, 
                            attribute_values[i]);
        if ( !strcmp ( attribute_names[i],
                   "Signe" ))
            gsb_data_payment_set_sign ( payment_number,
                            utils_str_atoi (attribute_values[i]));
        if ( !strcmp ( attribute_names[i],
                   "Affiche_entree" ))
            gsb_data_payment_set_show_entry ( payment_number,
                                  utils_str_atoi (attribute_values[i]));
        if ( !strcmp ( attribute_names[i],
                   "Numerotation_auto" ))
            gsb_data_payment_set_automatic_numbering ( payment_number,
                                       utils_str_atoi (attribute_values[i]));
        if ( !strcmp ( attribute_names[i],
                   "No_en_cours" ))
            gsb_data_payment_set_last_number ( payment_number,
                                    attribute_values[i] );

        i++;
        }
        while ( attribute_names[i] );

        /* before 0.6, account_number was not saved in the method of payment */
        gsb_data_payment_set_account_number ( payment_number,
                              account_number );

        /* append a conversion structure */
        conversion = g_malloc0 (sizeof (struct payment_conversion_struct));
        conversion -> account_number = account_number;
        conversion -> last_payment_number = last_number;
        conversion -> new_payment_number = payment_number;
        payment_conversion_list = g_slist_append ( payment_conversion_list,
                                   conversion );

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
            transaction_number = gsb_data_transaction_new_transaction_with_number ( account_number,
                                                    utils_str_atoi ( attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Id" ))
            gsb_data_transaction_set_transaction_id ( transaction_number,
                                       attribute_values[i] );

        if ( !strcmp ( attribute_names[i],
                   "D" ))
        {
            GDate *date;
            pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );

            date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                        utils_str_atoi ( pointeur_char[1] ),
                        utils_str_atoi ( pointeur_char[2] ));
            gsb_data_transaction_set_date ( transaction_number,
                                date);
            g_strfreev ( pointeur_char );
            if (date)
            g_date_free (date);
        }

        if ( !strcmp ( attribute_names[i],
                   "Db" )
             && attribute_values[i]
             && strlen (attribute_values[i]) )
        {
            pointeur_char = g_strsplit ( attribute_values[i],
                             "/",
                             0 );
            /* sometimes we had 0/0/0 */
            if(utils_str_atoi (pointeur_char[0]))
            {
            GDate *date;
            date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                            utils_str_atoi ( pointeur_char[1] ),
                            utils_str_atoi ( pointeur_char[2] ));

            gsb_data_transaction_set_value_date ( transaction_number,
                                      date );
            if (date)
                g_date_free (date);
            }
            g_strfreev (pointeur_char);
        }

        if ( !strcmp ( attribute_names[i],
                   "M" ))
        {
            /* to go to the 0.6.0 we need to change the amount string
             * from 12.340000 to 12.34 before doing the conversion */
            gchar *tmp_string;
            gsb_real number;

            tmp_string = utils_str_reduce_exponant_from_string ( attribute_values[i], 2 );
            number = gsb_real_get_from_string (tmp_string);
            /* printf ("tmp_string = %s number.mantissa = %ld number.exponent = %d\n", tmp_string,
                        number.mantissa, number.exponent); */
            gsb_data_transaction_set_amount ( transaction_number,
                                  gsb_real_get_from_string (tmp_string));
            if (tmp_string) g_free (tmp_string);
        }

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
                                     gsb_real_get_from_string (attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Fc" ))
            gsb_data_transaction_set_exchange_fees ( transaction_number,
                                     gsb_real_get_from_string (attribute_values[i]));

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
            gsb_data_transaction_set_split_of_transaction ( transaction_number,
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
            gsb_data_transaction_set_contra_transaction_number ( transaction_number,
                                           utils_str_atoi ( attribute_values[i]));

        /* new in grisbi 0.6, no contra account number, so if it was -1,
         * set contra transaction to -1 to set deleted account,
         * else, do nothing with that value */
        if ( !strcmp ( attribute_names[i],
                   "Rc" ))
        {
            if (utils_str_atoi (attribute_values[i]) == -1)
            gsb_data_transaction_set_contra_transaction_number ( transaction_number,
                                             -1 );
        }

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
        {
            /* cannot use gsb_parse_date_string here because before, all date were dd/mm/yyyy */
            GDate *date;
            pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );
            date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                        utils_str_atoi ( pointeur_char[1] ),
                        utils_str_atoi ( pointeur_char[2] ));
            gsb_data_scheduled_set_date ( scheduled_number, date);
            g_strfreev ( pointeur_char );
            if (date)
            g_date_free ( date );
        }

        if ( !strcmp ( attribute_names[i],
                   "Compte" ))
            gsb_data_scheduled_set_account_number ( scheduled_number,
                                    utils_str_atoi (attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Montant" ))
        {
            /* to go to the 0.6.0 we need to change the amount string
             * from 12.340000 to 12.34 before doing the conversion */
            gchar *tmp_string;

            tmp_string = utils_str_reduce_exponant_from_string ( attribute_values[i], 2 );
            gsb_data_scheduled_set_amount ( scheduled_number,
                                gsb_real_get_from_string (tmp_string));
            if (tmp_string) g_free (tmp_string);
        }

        if ( !strcmp ( attribute_names[i],
                   "Devise" ))
            gsb_data_scheduled_set_currency_number ( scheduled_number,
                                     utils_str_atoi (attribute_values[i]));

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
            gsb_data_scheduled_set_frequency ( scheduled_number,
                                   utils_str_atoi (attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Intervalle_periodicite" ))
            gsb_data_scheduled_set_user_interval ( scheduled_number,
                                   utils_str_atoi (attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Periodicite_personnalisee" ))
            gsb_data_scheduled_set_user_entry ( scheduled_number,
                                utils_str_atoi (attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Date_limite" )
             &&
             strlen (attribute_values[i]))
        {
            GDate *date;
            pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );
            date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                        utils_str_atoi ( pointeur_char[1] ),
                        utils_str_atoi ( pointeur_char[2] ));
            gsb_data_scheduled_set_limit_date ( scheduled_number, date);
            g_strfreev ( pointeur_char );
            if (date)
            g_date_free ( date );
        }

        if ( !strcmp ( attribute_names[i],
                   "Ech_ventilee" ))
            gsb_data_scheduled_set_split_of_scheduled ( scheduled_number,
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
        /*     we test at the beginning if the attribute_value is NULL, if yes, */
        /*        go to the next */

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
        gint currency_number;
        struct {
        gint one_c1_equal_x_c2;
        gint contra_currency;
        gsb_real exchange;
        } tmp_currency_link;
        GDate *modified_date = NULL;
        
        tmp_currency_link.one_c1_equal_x_c2 = 0;
        tmp_currency_link.contra_currency = 0;
        tmp_currency_link.exchange = null_real;

        currency_number = gsb_data_currency_new (NULL);

        /* Default */
        gsb_data_currency_set_floating_point ( currency_number, 2 );

        do
        {
        if ( !strcmp ( attribute_names[i],
                   "No" ))
            currency_number = gsb_data_currency_set_new_number ( currency_number,
                                         utils_str_atoi (attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Nom" ))
            gsb_data_currency_set_name ( currency_number,
                             attribute_values[i]);

        if ( !strcmp ( attribute_names[i],
                   "IsoCode" )
             &&
             strlen (attribute_values[i]))
            gsb_data_currency_set_code_iso4217 ( currency_number,
                                 attribute_values[i]);

        if ( !strcmp ( attribute_names[i],
                   "Code" )
             &&
             strlen (attribute_values[i]))
        {
            struct iso_4217_currency * currency = iso_4217_currencies;

            gsb_data_currency_set_code ( currency_number,
                             attribute_values[i]);

            /* Check if a iso code is the same as currency code (old import).  */
            while ( currency -> country_name )
            {
            if ( !strcmp ( currency -> currency_code, attribute_values[i] ) )
            {
                gsb_data_currency_set_code_iso4217 ( currency_number,
                                     attribute_values[i]);
            }
            currency++;
            }
        }

        /* beyond the 0.6, the next part is not anymore in the currencies, but alone
         * and simplified...
         * so we do the transition here */

        if ( !strcmp ( attribute_names[i],
                   "Rapport_entre_devises" ))
            tmp_currency_link.one_c1_equal_x_c2 = utils_str_atoi (attribute_values[i]);

        if ( !strcmp ( attribute_names[i],
                   "Devise_en_rapport" ))
            tmp_currency_link.contra_currency = utils_str_atoi (attribute_values[i]);

        if ( !strcmp ( attribute_names[i],
                   "Change" ))
            tmp_currency_link.exchange = gsb_real_get_from_string (attribute_values[i]);
        if ( !strcmp ( attribute_names[i], "Date_dernier_change" ) 
         &&
         strlen ( attribute_values[i] ) )
        {
            /* cannot use gsb_parse_date_string here because before, all date were dd/mm/yyyy */
            pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );
            modified_date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                        utils_str_atoi ( pointeur_char[1] ),
                        utils_str_atoi ( pointeur_char[2] ) );
            g_strfreev ( pointeur_char );
        }
        i++;
        }
        while ( attribute_names[i] );

        /* create now the link between currencies if necessary
         * don't create if the exchange is null event if the link was created before,
         * else we will become very rich in grisbi !!! */
        if (tmp_currency_link.contra_currency && tmp_currency_link.exchange.mantissa != 0)
        {
            gint  link_number;

            link_number = gsb_data_currency_link_new ( 0 );
            if (tmp_currency_link.one_c1_equal_x_c2)
            {
                gsb_data_currency_link_set_first_currency ( link_number,
                                    currency_number );
                gsb_data_currency_link_set_second_currency ( link_number,
                                     tmp_currency_link.contra_currency);
            }
            else
            {
                gsb_data_currency_link_set_first_currency ( link_number,
                                    tmp_currency_link.contra_currency );
                gsb_data_currency_link_set_second_currency ( link_number,
                                     currency_number);
            }
            gsb_data_currency_link_set_change_rate ( link_number,
                                 tmp_currency_link.exchange );
            if ( modified_date )
            {
                gsb_data_currency_link_set_modified_date ( link_number, modified_date );
                g_date_free ( modified_date );
            }
        }
    }
    }


    if ( !strcmp ( element_name,
           "Banque" ))
    {
    gint i;

    i = 0;

    if ( attribute_names[i] )
    {
        gint bank_number = 0;

        do
        {
        if ( !strcmp ( attribute_names[i],
                   "No" ))
        {
            bank_number = gsb_data_bank_set_new_number ( gsb_data_bank_new (NULL),
                                     utils_str_atoi (attribute_values[i]));
        }
        
        if ( !strcmp ( attribute_names[i],
                   "Nom" ))
            gsb_data_bank_set_name ( bank_number,
                         attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Code" ))
            gsb_data_bank_set_code ( bank_number,
                         attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Adresse" ))
            gsb_data_bank_set_bank_address ( bank_number,
                                 attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Tel" ))
            gsb_data_bank_set_bank_tel ( bank_number,
                             attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Mail" ))
            gsb_data_bank_set_bank_mail ( bank_number,
                              attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Web" ))
            gsb_data_bank_set_bank_web ( bank_number,
                             attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Nom_correspondant" ))
            gsb_data_bank_set_correspondent_name ( bank_number,
                                   attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Fax_correspondant" ))
            gsb_data_bank_set_correspondent_fax ( bank_number,
                                  attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Tel_correspondant" ))
            gsb_data_bank_set_correspondent_tel ( bank_number,
                                  attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Mail_correspondant" ))
            gsb_data_bank_set_correspondent_mail ( bank_number,
                                   attribute_values[i] );
        
        if ( !strcmp ( attribute_names[i],
                   "Remarques" ))
            gsb_data_bank_set_bank_note ( bank_number,
                              attribute_values[i] );

        i++;
        }
        while ( attribute_names[i] );
    }
    }


    if ( !strcmp ( element_name,
           "Exercice" ))
    {
    gint i = 0;

    if ( attribute_names[i] )
    {
        gint fyear_number;

        fyear_number = gsb_data_fyear_new (NULL);

        do
        {
        if ( !strcmp ( attribute_names[i],
                   "No" ))
            fyear_number = gsb_data_fyear_set_new_number ( fyear_number,
                                       utils_str_atoi ( attribute_values[i]));

        if ( !strcmp ( attribute_names[i],
                   "Nom" ))
            gsb_data_fyear_set_name ( fyear_number,
                          attribute_values[i]);

        if ( !strcmp ( attribute_names[i],
                   "Date_debut" )
             &&
             strlen (attribute_values[i]))
        {
            GDate *date;
            pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );
            date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                        utils_str_atoi ( pointeur_char[1] ),
                        utils_str_atoi ( pointeur_char[2] ));

            gsb_data_fyear_set_beginning_date ( fyear_number, date);
            g_strfreev ( pointeur_char );
            if (date)
            g_date_free (date);
        }

        if ( !strcmp ( attribute_names[i],
                   "Date_fin" )
             &&
             strlen (attribute_values[i]))
        {
            GDate *date;
            pointeur_char = g_strsplit ( attribute_values[i], "/", 0 );
            date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                        utils_str_atoi ( pointeur_char[1] ),
                        utils_str_atoi ( pointeur_char[2] ));

            gsb_data_fyear_set_end_date ( fyear_number, date);
            g_strfreev ( pointeur_char );
            if (date)
            g_date_free (date);
        }

        if ( !strcmp ( attribute_names[i],
                   "Affiche" ))
            gsb_data_fyear_set_form_show ( fyear_number,
                               utils_str_atoi ( attribute_values[i]));

        i++;
        }
        while ( attribute_names[i] );

        gsb_data_fyear_check_for_invalid (fyear_number);
    }
    }


    if ( !strcmp ( element_name,
           "Rapprochement" ))
    {
    gint i;

    i = 0;

    if ( attribute_names[i] )
    {
        gint reconcile_number;

        reconcile_number = gsb_data_reconcile_new (NULL);

        /* before 0.6, reconcile contains only a name so we fill for now
         * with default values the field of the reconcile, and we will
         * try to fill them later */
        gsb_data_reconcile_set_account ( reconcile_number,
                         -1 );

        gsb_data_reconcile_set_init_balance ( reconcile_number,
                              null_real );
        gsb_data_reconcile_set_final_balance ( reconcile_number,
                               null_real );

        do
        {
        if ( !strcmp ( attribute_names[i],
                   "No" ))
            reconcile_number = gsb_data_reconcile_set_new_number (reconcile_number,
                                          utils_str_atoi ( attribute_values[i]));
        if ( !strcmp ( attribute_names[i],
                   "Nom" ))
            gsb_data_reconcile_set_name ( reconcile_number,
                              attribute_values[i]);

        i++;
        }
        while ( attribute_names[i] );
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
        {
            /* to go to the 0.6.0 we need to change the amount string
             * from 12.340000 to 12.34 before doing the conversion */
            gchar *tmp_string;

            tmp_string = utils_str_reduce_exponant_from_string ( attribute_values[i], 2 );
            gsb_data_report_amount_comparison_set_first_amount ( amount_comparison_number,
                                         gsb_real_get_from_string (tmp_string));
            if (tmp_string) g_free (tmp_string);
        }

        if ( !strcmp ( attribute_names[i],
                   "Mont_2" ))
        {
            /* to go to the 0.6.0 we need to change the amount string
             * from 12.340000 to 12.34 before doing the conversion */
            gchar *tmp_string;

            tmp_string = utils_str_reduce_exponant_from_string ( attribute_values[i], 2 );
            gsb_data_report_amount_comparison_set_second_amount ( amount_comparison_number,
                                          gsb_real_get_from_string (tmp_string));
            if (tmp_string) g_free (tmp_string);
        }

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
                                              my_strdup (attribute_values[i])));

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
    /* TODO dOM
    if ( download_tmp_values.file_version )
        g_free ( download_tmp_values.file_version );
        */
    download_tmp_values.file_version = my_strdup (text);
    return;
    }

    if ( !strcmp ( element_name,
           "Version_grisbi" ))
    {
    if ( download_tmp_values.grisbi_version )
        g_free ( download_tmp_values.grisbi_version );
    download_tmp_values.grisbi_version = my_strdup (text);
    return;
    }

    if ( !strcmp ( element_name,
           "Titre" ))
    {
    if ( titre_fichier )
        g_free ( titre_fichier );
    titre_fichier = my_strdup (text);
    return;
    }

    if ( !strcmp ( element_name,
           "Adresse_commune" ))
    {
    if ( adresse_commune )
        g_free ( adresse_commune );
    adresse_commune = my_strdup (text);
    return;
    }

    if ( !strcmp ( element_name,
           "Adresse_secondaire" ))
    {
    if ( adresse_secondaire )
        g_free ( adresse_secondaire );
    adresse_secondaire = my_strdup (text);
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
        GdkPixbuf *pixbuf;
        gchar *chemin_logo;

        chemin_logo = my_strdup (text);
        pixbuf = gdk_pixbuf_new_from_file ( chemin_logo, NULL );
        if ( pixbuf )
        {
            gtk_window_set_default_icon ( pixbuf );
            gsb_select_icon_set_logo_pixbuf ( pixbuf );
        }
        if ( chemin_logo && strlen (chemin_logo) > 0 )
            g_free ( chemin_logo );
        return;
    }

    if ( !strcmp ( element_name,
           "Caracteristiques_par_compte" ))
    {
    etat.retient_affichage_par_compte = utils_str_atoi(text);
    return;
    }

    if ( !strcmp ( element_name,
           "Affichage_opes" ))
    {
    gchar **pointeur_char;
    gint i, j;
    gint number_columns;

    pointeur_char = g_strsplit ( my_strdup (text),
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
         * CUSTOM_MODEL_VISIBLE_COLUMNS, it will crash without that (ex : (5.5 -> 6.0 )) */
        if (  pointeur_char[j + i*CUSTOM_MODEL_VISIBLE_COLUMNS] )
            tab_affichage_ope[i][j] = utils_str_atoi ( pointeur_char[j + i*number_columns]);
        else
            j = CUSTOM_MODEL_VISIBLE_COLUMNS;
        }

    g_strfreev ( pointeur_char );
    return;
    }

    if ( !strcmp ( element_name,
           "Rapport_largeur_col_echeancier" ))
    {
    /* here do nothing because it was a ration before, and now it's fixed width,
     * so come back to default */
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

    /* in the 0.6, the accounts are saved in the good order,
     * so for before, get the order and reorder the list later */
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
    account_number = gsb_data_account_new ( GSB_TYPE_BANK );
    gsb_data_account_set_name ( account_number,
                    text);
    return;
    }

    if ( !strcmp ( element_name,
           "Id_compte" ))
    {
    gsb_data_account_set_id (account_number,
                 text);
    if ( !strlen ( gsb_data_account_get_id (account_number)))
        gsb_data_account_set_id (account_number,
                NULL );
    return;
    }

    /*                 we change here the default number of the account */

    if ( !strcmp ( element_name,
           "No_de_compte" ))
    {
    account_number = gsb_data_account_set_account_number ( account_number,
                                      utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Titulaire" ))
    {
    gsb_data_account_set_holder_name ( account_number,
                       text);
    return;
    }

    if ( !strcmp ( element_name,
           "Type_de_compte" ))
    {
    gsb_data_account_set_kind (account_number,
                   utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Devise" ))
    {
    gsb_data_account_set_currency ( account_number,
                   utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Banque" ))
    {
    gsb_data_account_set_bank ( account_number,
                   utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Guichet" ))
    {
    gsb_data_account_set_bank_branch_code ( account_number,
                            text);
    return;
    }

    if ( !strcmp ( element_name,
           "No_compte_banque" ))
    {
    gsb_data_account_set_bank_account_number ( account_number,
                               text);
    return;
    }

    if ( !strcmp ( element_name,
           "Cle_du_compte" ))
    {
    gsb_data_account_set_bank_account_key ( account_number,
                            text);
    return;
    }

    if ( !strcmp ( element_name,
           "Solde_initial" ))
    {
    /* to go to the 0.6.0 we need to change the amount string
     * from 12.340000 to 12.34 before doing the conversion */
    gchar *tmp_string;
    gsb_real number;
    
    tmp_string = utils_str_reduce_exponant_from_string ( text, 2 );
    /* printf ("solde_initial = %s\n", tmp_string ); */
    number = gsb_real_get_from_string ( tmp_string );
    if ( number.mantissa == error_real.mantissa )
        gsb_data_account_set_init_balance ( account_number, null_real );
    else
        gsb_data_account_set_init_balance ( account_number, number );
    /* printf ("tmp_string = %s number.mantissa = %ld number.exponent = %d initial_balance = %s\n", tmp_string,
                        number.mantissa, number.exponent,
                        gsb_real_get_string ( gsb_data_account_get_init_balance ( account_number, 2))); */

    if (tmp_string) 
        g_free (tmp_string);
    return;
    }

    if ( !strcmp ( element_name,
           "Solde_mini_voulu" ))
    {
    /* to go to the 0.6.0 we need to change the amount string
     * from 12.340000 to 12.34 before doing the conversion */
    gchar *tmp_string;
    gsb_real number;
    
    tmp_string = utils_str_reduce_exponant_from_string ( text, 2 );
    number = gsb_real_get_from_string ( tmp_string );
    if ( number.mantissa == error_real.mantissa )
        gsb_data_account_set_mini_balance_wanted ( account_number, null_real );
    else
        gsb_data_account_set_mini_balance_wanted ( account_number, number );

    if (tmp_string) 
        g_free (tmp_string);
    return;
    }

    if ( !strcmp ( element_name,
           "Solde_mini_autorise" ))
    {
    /* to go to the 0.6.0 we need to change the amount string
     * from 12.340000 to 12.34 before doing the conversion */
    gchar *tmp_string;
    gsb_real number;
    
    tmp_string = utils_str_reduce_exponant_from_string ( text, 2 );
    number = gsb_real_get_from_string ( tmp_string );
    if ( number.mantissa == error_real.mantissa )
        gsb_data_account_set_mini_balance_authorized ( account_number, null_real );
    else
        gsb_data_account_set_mini_balance_authorized ( account_number, number );

    if (tmp_string)
        g_free (tmp_string);
    return;
    }

    if ( !strcmp ( element_name,
           "Date_dernier_releve" ))
    {
        /* as the date comes before the last number of reconcile, the fastest way is to use
         * a buffer for ther reconcile structure, and when we have the last number of reconcile,
         * we append the buffer to the list */
        buffer_reconcile_conversion = g_malloc0 (sizeof (struct reconcile_conversion_struct));
        if (buffer_reconcile_conversion)
        {
            gchar **pointeur_char;

            pointeur_char = g_strsplit ( text, "/", 0 );

            buffer_reconcile_conversion -> final_date = g_date_new_dmy ( utils_str_atoi ( pointeur_char[0] ),
                        utils_str_atoi ( pointeur_char[1] ),
                        utils_str_atoi ( pointeur_char[2] ));
            g_strfreev ( pointeur_char );
        }
    return;
    }

    if ( !strcmp ( element_name,
           "Solde_dernier_releve" ))
    {
    /* to go to the 0.6.0 we need to change the amount string
     * from 12.340000 to 12.34 before doing the conversion */
    gchar *tmp_string;

    tmp_string = utils_str_reduce_exponant_from_string ( text, 2 );
    if (buffer_reconcile_conversion)
        buffer_reconcile_conversion -> final_balance = gsb_real_get_from_string (tmp_string);
    if (tmp_string) 
        g_free (tmp_string);
    return;
    }

    if ( !strcmp ( element_name,
           "Dernier_no_de_rapprochement" ))
    {
    if (buffer_reconcile_conversion)
    {
        buffer_reconcile_conversion -> reconcile_number = utils_str_atoi ( text);
        buffer_reconcile_conversion -> account_number = account_number;
        reconcile_conversion_list = g_slist_append ( reconcile_conversion_list,
                                 buffer_reconcile_conversion );
        buffer_reconcile_conversion = NULL;
    }
    return;
    }

    if ( !strcmp ( element_name,
           "Compte_cloture" ))
    {
    gsb_data_account_set_closed_account ( account_number,
                     utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Affichage_r" ))
    {
    gsb_data_account_set_r ( account_number,
                 utils_str_atoi (text));
    return;
    }

    if ( !strcmp ( element_name,
           "Nb_lignes_ope" ))
    {
    gsb_data_account_set_nb_rows ( account_number, 
                       utils_str_atoi (text));
    return;
    }

    if ( !strcmp ( element_name,
           "Commentaires" ))
    {
    gsb_data_account_set_comment ( account_number,
                       text);
    return;
    }

    if ( !strcmp ( element_name,
           "Adresse_du_titulaire" ))
    {
    gsb_data_account_set_holder_address ( account_number,
                          text);
    return;
    }

    if ( !strcmp ( element_name,
           "Type_defaut_debit" ))
    {
    gsb_data_account_set_default_debit ( account_number,
                         utils_str_atoi ( text) );
    return;
    }

    if ( !strcmp ( element_name,
           "Type_defaut_credit" ))
    {
    gsb_data_account_set_default_credit ( account_number,
                          utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Tri_par_type" ))
    {
    gsb_data_account_set_reconcile_sort_type ( account_number,
                               utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Neutres_inclus" ))
    {
    gsb_data_account_set_split_neutral_payment ( account_number,
                                 utils_str_atoi ( text) );
    return;
    }

    if ( !strcmp ( element_name,
           "Ordre_du_tri" ))
    {
    if (text)
    {
        gchar **pointeur_char;
        gint i;

        pointeur_char = g_strsplit ( text,
                     "/",
                     0 );

        i = 0;

        while ( pointeur_char[i] )
        {
        gsb_data_account_sort_list_add ( account_number,
                             utils_str_atoi ( pointeur_char[i] ));
        i++;
        }
        g_strfreev ( pointeur_char );
    }
    return;
    }

    if ( !strcmp ( element_name,
           "Classement_croissant" ))
    {
    gsb_data_account_set_sort_type ( account_number,
                    utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Classement_colonne" ))
    {
    gsb_data_account_set_sort_column ( account_number,
                      utils_str_atoi ( text));
    return;
    }

    if ( !strcmp ( element_name,
           "Classement_type_par_colonne" ))
    {
    gint i;
    gchar **pointeur_char;

    pointeur_char = g_strsplit ( text,
                     "-",
                     0 );

    for ( i=0 ; i<CUSTOM_MODEL_VISIBLE_COLUMNS ; i++ )
    {
        gsb_data_account_set_element_sort ( account_number,
                            i,
                            utils_str_atoi ( pointeur_char[i] ));
    }
    g_strfreev ( pointeur_char );
    return;
    }

    /* récupération de l'agencement du transaction_form */

    if ( !strcmp ( element_name,
           "Nb_colonnes_formulaire" ))
    {
    if ( !gsb_data_account_get_form_organization (account_number) )
        gsb_data_form_new_organization (account_number);
    gsb_data_form_set_nb_columns ( account_number,
                       utils_str_atoi (text));
    return;
    }

    if ( !strcmp ( element_name,
           "Nb_lignes_formulaire" ))
    {
    if ( !gsb_data_account_get_form_organization (account_number) )
        gsb_data_form_new_organization (account_number);
    gsb_data_form_set_nb_rows ( account_number,
                    utils_str_atoi (text));
    return;
    }


    if ( !strcmp ( element_name,
           "Organisation_formulaire" ))
    {
    gchar **pointeur_char;
    gint i, j;

    if ( !gsb_data_account_get_form_organization (account_number) )
        gsb_data_form_new_organization (account_number);

    pointeur_char = g_strsplit ( text,
                     "-",
                     0 );

    for ( i=0 ; i<MAX_HEIGHT ; i++ )
        for ( j=0 ; j<MAX_WIDTH ; j++ )
        gsb_data_form_set_value ( account_number,
                      j, i,
                      utils_str_atoi (pointeur_char[j + i*MAX_WIDTH]));

    g_strfreev ( pointeur_char );
    return;
    }


    if ( !strcmp ( element_name,
           "Largeur_col_formulaire" ))
    {
    gchar **pointeur_char;
    gint i;

    if ( !gsb_data_account_get_form_organization (account_number) )
        gsb_data_form_new_organization (account_number);

    pointeur_char = g_strsplit ( text,
                     "-",
                     0 );

    for ( i=0 ; i<MAX_WIDTH ; i++ )
        gsb_data_form_set_width_column ( account_number,
                         i,
                         utils_str_atoi (pointeur_char[i]));

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
                       gsb_string_get_int_list_from_string ( text,
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
    gsb_data_report_set_not_detail_split ( last_report_number,
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

    pointeur_char = g_strsplit ( text,
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

    pointeur_char = g_strsplit ( text,
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

    pointeur_char = g_strsplit ( text,
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
    /* in 0.6 we add separation per day, so + 1 */
    gsb_data_report_set_period_split_type ( last_report_number,
                            utils_str_atoi (text) + 1);
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

    pointeur_char = g_strsplit ( text,
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

    pointeur_char = g_strsplit ( text,
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
    GSList *tmp_list = NULL;
    struct_categ_budget_sel *categ_struct;

    /* before 0.6, it was only categories, so by default, we select all the sub-categories
     * of the selected categories */
    pointeur_char = g_strsplit ( text,
                     "/",
                     0 );
    i=0;

    while ( pointeur_char[i] )
    {
        GSList *sub_categ_budget_list;

        categ_struct = g_malloc0 (sizeof (struct_categ_budget_sel));
        tmp_list = g_slist_append (tmp_list, categ_struct);

        categ_struct -> div_number = utils_str_atoi ( pointeur_char[i] );

        /* we append now all the sub-categories */
        sub_categ_budget_list = gsb_data_category_get_sub_category_list (categ_struct -> div_number);
        while (sub_categ_budget_list)
        {
        categ_struct -> sub_div_numbers = g_slist_append (categ_struct -> sub_div_numbers,
                                      GINT_TO_POINTER (gsb_data_category_get_no_sub_category (sub_categ_budget_list -> data)));
        sub_categ_budget_list = sub_categ_budget_list -> next;
        }
        /* we append a null sub-category  */
        categ_struct -> sub_div_numbers = g_slist_append (categ_struct -> sub_div_numbers,
                                      NULL );
        i++;
    }
    /* we append a null category with a null sub-category only if detail is not set*/
    if (!gsb_data_report_get_category_detail_used (last_report_number))
    { 
        categ_struct = g_malloc0 (sizeof (struct_categ_budget_sel));
        tmp_list = g_slist_append (tmp_list, categ_struct);
        categ_struct -> sub_div_numbers = g_slist_append (categ_struct -> sub_div_numbers,
                                      NULL );
    }
    g_strfreev ( pointeur_char );
    gsb_data_report_set_category_struct (last_report_number, tmp_list);
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
    GSList *tmp_list = NULL;
    struct_categ_budget_sel *budget_struct;

    /* before 0.6, it was only budgets, so by default, we select all the sub-budgets
     * of the selected budgets */
    pointeur_char = g_strsplit ( text,
                     "/",
                     0 );
    i=0;

    while ( pointeur_char[i] )
    {
        GSList *sub_budget_list;

        budget_struct = g_malloc0 (sizeof (struct_categ_budget_sel));
        tmp_list = g_slist_append (tmp_list, budget_struct);

        budget_struct -> div_number = utils_str_atoi ( pointeur_char[i] );

        /* we append now all the sub-categories */
        sub_budget_list = gsb_data_budget_get_sub_budget_list (budget_struct -> div_number);
        while (sub_budget_list)
        {
        budget_struct -> sub_div_numbers = g_slist_append (budget_struct -> sub_div_numbers,
                                       GINT_TO_POINTER (gsb_data_budget_get_no_sub_budget (sub_budget_list -> data)));
        sub_budget_list = sub_budget_list -> next;
        }
        /* we append a null sub-category  */
        budget_struct -> sub_div_numbers = g_slist_append (budget_struct -> sub_div_numbers,
                                       NULL );
        i++;
    }
    /* we append a null budget with a null sub-budget only if detail is not set*/
    if (!gsb_data_report_get_budget_detail_used (last_report_number))
    { 
        budget_struct = g_malloc0 (sizeof (struct_categ_budget_sel));
        tmp_list = g_slist_append (tmp_list, budget_struct);
        budget_struct -> sub_div_numbers = g_slist_append (budget_struct -> sub_div_numbers,
                                       NULL );
    }
    g_strfreev ( pointeur_char );
    gsb_data_report_set_budget_struct (last_report_number, tmp_list);
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

    pointeur_char = g_strsplit ( text,
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
    gchar* tmpstr;
    gint currency_number;
    GSList *list_tmp;
    gint i;
    GSList *list_tmp_account;
    GSList *list_tmp_transactions;
    GSList *list_tmp_scheduled;
    gint version_number;
    gint account_number;
    GList *dlist_tmp;
    gchar** strarray;

    strarray = g_strsplit ( download_tmp_values.file_version, ".", 0 );
    tmpstr = g_strjoinv ( "", strarray );
    version_number = utils_str_atoi ( tmpstr );
    g_strfreev ( strarray );
    g_free ( tmpstr );

    /* for now the file is not modified */
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

        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
        while ( list_tmp_transactions )
        {
            gint transaction_number_tmp;
            transaction_number_tmp = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

        /*  si l'opération est une ventil, on refait le tour de la liste pour trouver ses filles */

        if ( gsb_data_transaction_get_split_of_transaction (transaction_number_tmp))
        {
            GSList *list_tmp_transactions_2;
            list_tmp_transactions_2 = gsb_data_transaction_get_complete_transactions_list ();

            while ( list_tmp_transactions_2 )
            {
                gint transaction_number_tmp_2;
                transaction_number_tmp_2 = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions_2 -> data) ;

            if ( gsb_data_transaction_get_account_number (transaction_number_tmp_2) == 
                        gsb_data_transaction_get_account_number (transaction_number_tmp)
                 &&
                 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp_2) == 
                        transaction_number_tmp)
                gsb_data_transaction_set_reconcile_number ( transaction_number_tmp_2,
                        gsb_data_transaction_get_reconcile_number (transaction_number_tmp));

            list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
            }
        }
        list_tmp_transactions = list_tmp_transactions -> next;
        }

        /* ************************************* */
        /*         ouverture d'un fichier 0.4.1     */
        /* ************************************* */

    case 41:

        /*     ajout de la 0.5 -> valeur_echelle_recherche_date_import qu'on me à 2 */

        valeur_echelle_recherche_date_import = 2;

        /*         passage à l'utf8 : on fait le tour des devises pour retrouver l'euro */
        /* Handle Euro nicely */

        currency_number = gsb_data_currency_get_number_by_name ("Euro");

        if (currency_number)
        {
        gsb_data_currency_set_code ( currency_number,
                         "€" );
        gsb_data_currency_set_code_iso4217 ( currency_number,
                                 "EUR" );
        }


        /* ************************************* */
        /*         ouverture d'un fichier 0.5.0     */
        /* ************************************* */

    case 50:

        /* ************************************* */
        /*         ouverture d'un fichier 0.5.1     */
        /* ************************************* */

    case 51:

        /* ************************************* */
        /*         ouverture d'un fichier 0.5.5     */
        /* ************************************* */

    case 55:


        /* ************************************* */
        /*         ouverture d'un fichier 0.5.6     */
        /* ************************************* */

    case 56:

        /* ************************************* */
        /*         ouverture d'un fichier 0.5.7     */
        /* ************************************* */

    case 57:

        /* ************************************* */
        /*         ouverture d'un fichier 0.5.8     */
        /* ************************************* */

    case 58:
    case 59:

        /* all the change between the 0.5.0 and 0.6.0 are set here because a confuse between
         * the number of version and the number of file structure */

        /* pour l'instant le fichier 0.5.1 ne diffère pas de la version 0.5.0 */
        /*     excepté un changement dans la notation du pointage */
        /*     rien=0 ; P=1 ; T=2 ; R=3 */
        /*     on fait donc le tour des opés pour inverser R et P */

        switch_t_r ();

        /*         un bug dans la 0.5.0 permettait à des comptes d'avoir un affichage différent, */
        /*         même si celui ci devait être identique pour tous, on vérifie ici */

        if ( !etat.retient_affichage_par_compte )
        {
            gint affichage_r;
            gint nb_lignes_ope;
            GSList *list_tmp;

            affichage_r = 0;
            nb_lignes_ope = 3;

            list_tmp = gsb_data_account_get_list_accounts ();

            while ( list_tmp )
            {
                i = gsb_data_account_get_no_account ( list_tmp -> data );

                gsb_data_account_set_r ( i, affichage_r );
                gsb_data_account_set_nb_rows ( i, nb_lignes_ope );

                list_tmp = list_tmp -> next;
            }
        }

        /* to go from 0.5.x to 0.6.x, there is a change in the method of payment :
         * before, they were saved in each account, and have for each accounts numbers 1, 2, 3...
         * now, they are saved in a general list and we must adapt the numbers to avoid several method of payments
         *         with the same number
         * the change is done while downloading the file, all we need to do now is to change
         * the payment number of all the transactions and scheduled transactions to set the new number */
        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
        while ( list_tmp_transactions )
        {
            gint transaction_number;

            transaction_number = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

            gsb_data_transaction_set_method_of_payment_number ( transaction_number,
                        gsb_file_load_get_new_payment_number (
                        gsb_data_transaction_get_account_number (transaction_number),
                        gsb_data_transaction_get_method_of_payment_number (transaction_number)) );

            list_tmp_transactions = list_tmp_transactions -> next;
        }

        /* do the same for scheduled transactions */
        list_tmp_scheduled = gsb_data_scheduled_get_scheduled_list ();
        while (list_tmp_scheduled)
        {
            gint scheduled_number;
            scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp_scheduled -> data);

            gsb_data_scheduled_set_method_of_payment_number ( scheduled_number,
                        gsb_file_load_get_new_payment_number (
                        gsb_data_scheduled_get_account_number (scheduled_number),
                        gsb_data_scheduled_get_method_of_payment_number (scheduled_number)) );

            list_tmp_scheduled = list_tmp_scheduled -> next;
        }

        /* do the same for the sort list of accounts and default payment */
        list_tmp = gsb_data_account_get_list_accounts ();
        while ( list_tmp )
        {
            GSList *sorted_list;
            GSList *new_sorted_list = NULL;

            account_number = gsb_data_account_get_no_account ( list_tmp -> data );

            gsb_data_account_set_default_debit ( account_number,
                        gsb_file_load_get_new_payment_number ( account_number,
                        gsb_data_account_get_default_debit (account_number)));
            gsb_data_account_set_default_credit ( account_number,
                        gsb_file_load_get_new_payment_number ( account_number,
                        gsb_data_account_get_default_credit (account_number)) );

            sorted_list = gsb_data_account_get_sort_list (account_number);
            while (sorted_list)
            {
                gint new_number;

                new_number = gsb_file_load_get_new_payment_number ( account_number,
                                        GPOINTER_TO_INT (sorted_list -> data));
                if (new_number)
                new_sorted_list = g_slist_append ( new_sorted_list,
                                   GINT_TO_POINTER (new_number));

                sorted_list = sorted_list -> next;
            }
            gsb_data_account_sort_list_free (account_number);
            gsb_data_account_set_sort_list (account_number, new_sorted_list);

            list_tmp = list_tmp -> next;
        }

        /* a problem untill the 0.5.7 :
         * all new method of payment are not added to the sorting list for reconciliation,
         * so check all the method of payment, and if 1 is not in the sorted list for its account */
        list_tmp = gsb_data_payment_get_payments_list ();
        while (list_tmp)
        {
        gint payment_number;
        GSList *sorted_list;
        gint account_number;

        payment_number = gsb_data_payment_get_number (list_tmp -> data);
        account_number = gsb_data_payment_get_account_number (payment_number);
        sorted_list = gsb_data_account_get_sort_list (account_number);
        if ( !g_slist_find ( sorted_list,
                     GINT_TO_POINTER (payment_number)))
            gsb_data_account_sort_list_add ( account_number,
                                 payment_number );
        list_tmp = list_tmp -> next;
        }

        /* a problem untill the 0.5.7, the children of a scheduled split are marked
         * as a split mother */
        list_tmp_scheduled = gsb_data_scheduled_get_scheduled_list ();

        while (list_tmp_scheduled)
        {
        gint scheduled_number;
        scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp_scheduled -> data);

        if ( gsb_data_scheduled_get_mother_scheduled_number (scheduled_number))
            gsb_data_scheduled_set_split_of_scheduled ( scheduled_number,
                                        0 );
        list_tmp_scheduled = list_tmp_scheduled -> next;
        }

        /* there is a bug untill now, which is some children of split
         * are not marked R, and the mother is...
         * very annoying now, we MUST mark them as R, so check here... */

        /* we use that to correct another bug, sometimes, the mother of split
         * has a financial year an budget... bad things because makes errors in reports,
         * so change that here */

        /* another fix, some children of split have not the same values of the mother
         * for some fields wich should be ; fix here */

        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

        while ( list_tmp_transactions )
        {
        gint transaction_number;
        gint mother_number;

        transaction_number = gsb_data_transaction_get_transaction_number (list_tmp_transactions -> data);

        /*  if it's a split and marked R, we look for the children */
        if ( gsb_data_transaction_get_split_of_transaction (transaction_number))
        {
            /* change the problem of marked transactions */
            if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE)
            {
            GSList *list_tmp_transactions_2;
            list_tmp_transactions_2 = gsb_data_transaction_get_complete_transactions_list ();

            while ( list_tmp_transactions_2 )
            {
                gint transaction_number_2;
                transaction_number_2 = gsb_data_transaction_get_transaction_number (list_tmp_transactions_2 -> data);

                if ( gsb_data_transaction_get_mother_transaction_number (transaction_number_2) == transaction_number)
                gsb_data_transaction_set_marked_transaction ( transaction_number_2,
                                              OPERATION_RAPPROCHEE );
                list_tmp_transactions_2 = list_tmp_transactions_2 -> next;
            }
            }

            /* erase what shouldn't be set */
            gsb_data_transaction_set_budgetary_number (transaction_number, 0);
            gsb_data_transaction_set_sub_budgetary_number (transaction_number, 0);
            gsb_data_transaction_set_financial_year_number (transaction_number, 0);
            gsb_data_transaction_set_voucher (transaction_number, NULL);
        }

        /* if t's a child, fix the values to be the same as for mother */
        mother_number = gsb_data_transaction_get_mother_transaction_number ( transaction_number );
        if (mother_number)
        {
            /* set the value date */
            gsb_data_transaction_set_value_date ( transaction_number,
                                  gsb_date_copy (gsb_data_transaction_get_value_date (mother_number)));
            /* set the party */
            gsb_data_transaction_set_party_number ( transaction_number,
                                    gsb_data_transaction_get_party_number (mother_number));
            /* set the currency */
            gsb_data_transaction_set_currency_number ( transaction_number,
                                       gsb_data_transaction_get_currency_number (mother_number));
            /* set the type */
            gsb_data_transaction_set_method_of_payment_number ( transaction_number,
                                        gsb_data_transaction_get_method_of_payment_number (mother_number));
            /* set the type content */
            gsb_data_transaction_set_method_of_payment_content ( transaction_number,
                                         gsb_data_transaction_get_method_of_payment_content (mother_number));
            /* set the bank */
            gsb_data_transaction_set_bank_references ( transaction_number,
                                       gsb_data_transaction_get_bank_references (mother_number));
        }
        list_tmp_transactions = list_tmp_transactions -> next;
        }

        /* change to the 0.6.0 : the number of choice of periodicity
         * now 7 choice => in scheduler_periodicity, we have to change
         * the last choices to the new numbers */

        list_tmp_scheduled = gsb_data_scheduled_get_scheduled_list ();

        while (list_tmp_scheduled)
        {
        gint scheduled_number;

        scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp_scheduled -> data);

        switch ( gsb_data_scheduled_get_frequency (scheduled_number))
        {
            case 0:
            case 1:
            case 2:
            /* for once, weekly and months, no change */
            break;

            case 3:
            /* year frequency */
            gsb_data_scheduled_set_frequency ( scheduled_number,
                                   SCHEDULER_PERIODICITY_YEAR_VIEW );
            break;

            /* there is a bug and the periodicity can be more than 4...
             * so set the default here to set it to SCHEDULER_PERIODICITY_CUSTOM_VIEW */
            case 4:
            default:
            /* custom frequency */
            gsb_data_scheduled_set_frequency ( scheduled_number,
                                   SCHEDULER_PERIODICITY_CUSTOM_VIEW );
            break;
        }

        switch ( gsb_data_scheduled_get_user_interval ( scheduled_number ))
        {
            case 0:
            /* no change for day */
            break;

            case 1:
            /* for months */
            gsb_data_scheduled_set_user_interval ( scheduled_number,
                                       PERIODICITY_MONTHS );
            break;

            case 2:
            /* for years */
            gsb_data_scheduled_set_user_interval ( scheduled_number,
                                       PERIODICITY_YEARS );
            break;
        }
                                    
        list_tmp_scheduled = list_tmp_scheduled -> next;
        }

        /* new to the 0.6.0 : append the currency_floating_point to the currencies
         * now all the amount are gint and no float, and currency_floating_point will
         * determine where is the point in the float
         * by defaut (in the last releases), the value was automatickly 2 */

        list_tmp = gsb_data_currency_get_currency_list ();

        while ( list_tmp )
        {
        i = gsb_data_currency_get_no_currency ( list_tmp -> data );

        gsb_data_currency_set_floating_point ( i,
                                   2 );

        list_tmp = list_tmp -> next;
        }

        /* there is a bug from i don't know when, sometimes when removing some splitted
         * transactions, the children were not removed, so we check all the children to make
         * sure the mother is a splitted transaction, else we erase it */
        list_tmp = gsb_data_transaction_get_complete_transactions_list ();
        while (list_tmp)
        {
        gint mother_transaction;
        gint transaction_number = gsb_data_transaction_get_transaction_number (list_tmp -> data);

        list_tmp = list_tmp -> next;

        mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);

        if (mother_transaction 
            &&
            !gsb_data_transaction_get_split_of_transaction (mother_transaction))
        {
            /* the mother is not a splitted transaction */
            /* if it was a transfer, remove the contra transaction */
            if (gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0)
            gsb_data_transaction_remove_transaction_without_check (gsb_data_transaction_get_contra_transaction_number (transaction_number));

            /*we erase the child */
            gsb_data_transaction_remove_transaction_without_check (transaction_number);
        }
        }

        /* we shall do so for the planned operations */
        list_tmp = gsb_data_scheduled_get_scheduled_list ();
        tmpstr = NULL;

        while ( list_tmp )
        {
            gint scheduled_number;
            gint mother_scheduled_number;

            scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp -> data);
            list_tmp = list_tmp -> next;
            
            mother_scheduled_number = gsb_data_scheduled_get_mother_scheduled_number (
                        scheduled_number );
            if ( mother_scheduled_number )
            {
                if ( gsb_data_scheduled_get_automatic_scheduled (
                 mother_scheduled_number ) == -1 )
                {
                    /* it's a child but didn't find the mother, it can happen in old files 
                     * previous to 0.6 where the children wer saved before the mother */
                    /*we erase the child */
                    gsb_data_scheduled_remove_scheduled ( scheduled_number );
                    if ( tmpstr == NULL )
                        tmpstr = utils_str_itoa ( scheduled_number );
                    else
                        tmpstr = g_strconcat ( tmpstr, " - ",
                            utils_str_itoa ( scheduled_number ), NULL );
                }
            }
        }
        if ( tmpstr != NULL )
            g_printf ( "the scheduled transactions have been deleted :\n%s\n",
                        tmpstr );

        /* a bug before 0.6 (and perhaps after ? still not found for now) set
         * a negative number for certain banks, so change that here */
        list_tmp = gsb_data_bank_get_bank_list ();

        while ( list_tmp )
        {
        i = gsb_data_bank_get_no_bank ( list_tmp -> data );

        if ( i<0 )
        {
            gint new_number;
            GSList *account_list;

            new_number = gsb_data_bank_set_new_number ( i, gsb_data_bank_max_number () + 1);

            /* only accounts are associated with bank number */
            account_list = gsb_data_account_get_list_accounts ();
            while (account_list)
            {
            gint account_number;

            account_number = gsb_data_account_get_no_account (account_list -> data);
            if (gsb_data_account_get_bank (account_number) == i)
                gsb_data_account_set_bank ( account_number, new_number );

            account_list = account_list -> next;
            }
        }

        list_tmp = list_tmp -> next;
        }

        /* new to 0.6.0 : struct of reconcile has changed,
         * it contains now an account number, init and final dates and
         * init and final balances
         * and now there is no more information on reconcile in the account
         * so here we try to fill that fields, and especially set the last
         * final date and last final balance in the last reconciles
         * hopefully, we can do that because each reconciled transaction has 
         * its number of reconcile */

        /* first step, fill the account numbers and try to fill the init 
         * and final dates */
        list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();
        while ( list_tmp_transactions )
        {
            gint transaction_number;
            gint reconcile_number;

            transaction_number = gsb_data_transaction_get_transaction_number (
                        list_tmp_transactions -> data);

            /* ok first we work only with reconciled transactions */
            if ( gsb_data_transaction_get_marked_transaction (
                            transaction_number) == OPERATION_RAPPROCHEE )
            {
                gint account_number;
                gint reconcile_account;
                const GDate *date_reconcile;
                const GDate *date_transaction;
                gboolean trouve = FALSE;

                account_number = gsb_data_transaction_get_account_number (
                        transaction_number );

                reconcile_number = gsb_data_transaction_get_reconcile_number (
                        transaction_number);
                if ( reconcile_number == 0 )
                {
                    if ( ( reconcile_number = gsb_data_reconcile_get_account_last_number (
                     account_number ) ) == 0 )
                    {
                        reconcile_number = gsb_data_reconcile_new (NULL);
                        gsb_data_reconcile_set_account ( reconcile_number, -1 );
                        gsb_data_reconcile_set_name ( reconcile_number,
                                  gsb_data_account_get_name ( account_number ) );
                        gsb_data_reconcile_set_init_balance ( reconcile_number,
                                              null_real );
                        gsb_data_reconcile_set_final_balance ( reconcile_number,
                                               null_real );
                    }
                }

                reconcile_account = gsb_data_reconcile_get_account ( reconcile_number );

                /* ok, we set the account number (faster to not check and directly
                 * write it... even if already done) */
                /* on regarde quel est le numéro de compte associé au rapprochement.
                 * s'il est égal à -1 on met directement le numéro de compte de l'opération
                 * sinon on crée un nouveau rapprochement pour ce compte. */
                if ( reconcile_account == -1 )
                {
                    gsb_data_reconcile_set_account ( reconcile_number, account_number );
                    buffer_old_new_rec_conversion = g_malloc0 (sizeof (
                        struct old_new_rec_conversion_struct));
                    buffer_old_new_rec_conversion -> account_number = account_number;
                    buffer_old_new_rec_conversion -> old_rec_number = reconcile_number;
                    buffer_old_new_rec_conversion -> new_rec_number = reconcile_number;
                    old_new_rec_list = g_slist_append ( old_new_rec_list,
                                                     buffer_old_new_rec_conversion );
                }
                else
                {
                    struct old_new_rec_conversion_struct *old_new_rec_struct;
                    
                    list_tmp = old_new_rec_list;
                    while (list_tmp)
                    {
                        old_new_rec_struct = list_tmp -> data;
                        
                        if ( old_new_rec_struct -> account_number == account_number &&
                            old_new_rec_struct -> old_rec_number == reconcile_number )
                        {
                            trouve = TRUE;
                            reconcile_number = old_new_rec_struct -> new_rec_number;
                            break;
                        }
                        list_tmp = list_tmp -> next;
                    }
                    if ( trouve == FALSE )
                    {
                        buffer_old_new_rec_conversion = g_malloc0 (sizeof (
                            struct old_new_rec_conversion_struct));
                        buffer_old_new_rec_conversion -> account_number = account_number;
                        buffer_old_new_rec_conversion -> old_rec_number = reconcile_number;
                        reconcile_number = gsb_data_reconcile_new ( 
                            gsb_data_reconcile_get_name (reconcile_number) );
                        gsb_data_reconcile_set_account ( reconcile_number, account_number );
                        buffer_old_new_rec_conversion -> new_rec_number = reconcile_number;
                        old_new_rec_list = g_slist_append ( old_new_rec_list,
                                                buffer_old_new_rec_conversion );
                    }
                }

                /* set the new_reconcile_number if necessary */
                gsb_data_transaction_set_reconcile_number ( transaction_number,
                        reconcile_number );

                /* set the initial date, we cannot have exactly the date of the
                 * reconciled paper, but we will take the first date of the
                 * transactions of this reconcile */
                date_reconcile = gsb_data_reconcile_get_init_date (reconcile_number);
                date_transaction = gsb_data_transaction_get_date (transaction_number);

                if ( !date_reconcile
                 ||
                 g_date_compare ( date_reconcile, date_transaction) > 0 )
                    gsb_data_reconcile_set_init_date ( reconcile_number, date_transaction );
                
                /* set the final date in the same way */
                date_reconcile = gsb_data_reconcile_get_final_date (reconcile_number);

                if ( !date_reconcile
                 ||
                 g_date_compare ( date_reconcile, date_transaction) < 0 )
                    gsb_data_reconcile_set_final_date ( reconcile_number, date_transaction );

                /* add the amount of the transaction to the init balance of that reconcile,
                 * used later to find the initials and finals balances */
                if (!gsb_data_transaction_get_mother_transaction_number (transaction_number))
                {
                    gsb_data_reconcile_set_init_balance ( reconcile_number,
                            gsb_real_add ( gsb_data_reconcile_get_init_balance (
                            reconcile_number),
                            gsb_data_transaction_get_adjusted_amount ( transaction_number,
                            gsb_data_account_get_currency_floating_point ( account_number ) ) ) );

                }
            }
            list_tmp_transactions = list_tmp_transactions -> next;
        }

        /* second step, some reconciles can have the account number to -1 
         * if they hadn't any transactions associated to them,
         * we delete them here */
        dlist_tmp = gsb_data_reconcile_get_reconcile_list ();
        while (dlist_tmp)
        {
            gint reconcile_number;

            reconcile_number = gsb_data_reconcile_get_no_reconcile (dlist_tmp -> data);

            dlist_tmp = dlist_tmp -> next;

            if (gsb_data_reconcile_get_account (reconcile_number) == -1)
                gsb_data_reconcile_remove (reconcile_number);
        }

        /* third step, we want to fill the inital and final balance of
         * all the reconcile but all that we have is the final balance
         * of the last reconcile and the transactions for all the
         * reconciles (cannot run from the beginning with the initial
         * balance of account because user can change the first time the
         * initial balance of the reconcile).
         * so run from the end and go to the start */

        list_tmp_account = gsb_data_account_get_list_accounts ();
        while (list_tmp_account)
        {
            gint account_number;
            gint reconcile_number;
            GList *rec_list = NULL;
            struct reconcile_conversion_struct *reconcile;

            account_number = gsb_data_account_get_no_account (list_tmp_account -> data);

            list_tmp = reconcile_conversion_list;
            while ( list_tmp )
            {
                gint reconcile_number;

                reconcile = list_tmp -> data;

                if ( reconcile -> account_number == account_number )
                {
                    const GDate *final_date;
                    gint ecart_date;

                    reconcile_number = gsb_data_reconcile_get_account_last_number (
                                            reconcile -> account_number );
                    final_date = gsb_data_reconcile_get_final_date ( reconcile_number );
                    ecart_date = g_date_days_between ( final_date, reconcile -> final_date );

                    if ( abs (ecart_date) < 10 )
                    {
                        gsb_data_reconcile_set_final_date ( reconcile_number,
                                            reconcile -> final_date );
                        gsb_data_reconcile_set_final_balance ( reconcile_number,
                                            reconcile -> final_balance );
                        result_reconcile = TRUE;
                    }
                    else
                    {
                        result_reconcile = gsb_file_load_set_last_reconcile_data (
                                                account_number,
                                                reconcile -> final_date,
                                                reconcile -> final_balance);
                    }
                    break;
                }
                list_tmp = list_tmp -> next;
            }

            /* for each account, we find the last reconcile number, get the 
             * final balance and calculate the initial balance, wich is the
             * final balance of the previous reconcile... */

            rec_list = g_list_last ( gsb_data_reconcile_get_sort_reconcile_list (
                                        account_number) );
            while (rec_list)
            {
                gint previous_reconcile_number;
                
                reconcile_number = GPOINTER_TO_INT ( rec_list -> data );
                gsb_data_reconcile_set_init_balance ( reconcile_number,
                        gsb_real_sub ( gsb_data_reconcile_get_final_balance (
                        reconcile_number),
                        gsb_data_reconcile_get_init_balance (
                        reconcile_number)));
                if ( rec_list -> prev )
                {
                    previous_reconcile_number = GPOINTER_TO_INT ( (rec_list -> prev) -> data );
                    if (previous_reconcile_number)
                        gsb_data_reconcile_set_final_balance ( previous_reconcile_number,
                            gsb_data_reconcile_get_init_balance (reconcile_number));
                }
                rec_list = rec_list -> prev;
            }
            list_tmp_account = list_tmp_account -> next;
        }

        /*
         * untill 0.6, no archive, so by default we let grisbi check at opening and set
         * the transactions limit to 3000 */
        conf.check_for_archival = TRUE;
        conf.max_non_archived_transactions_for_check = 3000;

        /**
         * new in 0.6, there is no name for saving file but a directory
         * with autoname for autosave.
         * the best here is to show the first page of the new assistant
         * to choose the save directory */
        gsb_assistant_first_come_to_0_6 ();

        /* ********************************************************* */
        /*      to set just before the new version */
        /* ********************************************************* */

        if ( etat.modification_fichier == 0 )
            modification_fichier ( TRUE );

        /* ************************************* */
        /*         opening 0.6.0                    */
        /* ************************************* */

        /* now the order of account is saved directly in the file, but if come before,
         * need to reorder the list */
        gsb_data_account_reorder (sort_accounts);

    case 60:
        if ( conf.sauvegarde_demarrage )
            etat.modification_fichier = TRUE;
        break;

    default :
        /* we don't know here the release of that file, give the release needed */
        tmpstr = g_strdup_printf ( _("Grisbi version %s is needed to open this file.\nYou are using version %s."),
                           download_tmp_values.grisbi_version,
                           VERSION );
        dialogue_error ( tmpstr);
        g_free ( tmpstr );

        return ( FALSE );
    }

    /* general stuff for all versions */
    
    /* mark the file as opened */
    gsb_file_util_modify_lock ( TRUE );

    /* check now if a lot of transactions,
     * if yes, we propose to file the transactions
     * by default take the 3000 transactions as limit */
    if ( conf.check_for_archival
     &&
     g_slist_length (gsb_data_transaction_get_transactions_list ()) > 
     conf.max_non_archived_transactions_for_check )
    gsb_assistant_archive_run (TRUE);

    /* if we opened an archive, we say it here */
    if (etat.is_archive)
    dialogue_hint (_("You have opened an archive.\nThere is no limit in Grisbi, "
                        "you can do whatever you want and save it later (new reports...) "
                        "but remember it's an archive before modifying some transactions "
                        "or important information."),
                        _("Grisbi archive opened"));
    return TRUE;
}

/**
 * while going from 0.5.x to 0.6.x, there is a change of number of method of payment
 * that function returns the new number of method of payment, from the last one and its account
 * this is a temporary function wich should be removed the day when 0.5.x are not longer supported
 *
 * \param account_number
 * \param payment_number
 *
 * \return the new number of payment or 0 if not found
 * */
gint gsb_file_load_get_new_payment_number ( gint account_number,
                        gint payment_number )
{
    GSList *tmp_list;
    
    tmp_list = payment_conversion_list;
    while (tmp_list)
    {
    struct payment_conversion_struct *conversion;

    conversion = tmp_list -> data;

    /* for the sorted list in accounts, payment_number can be negative if
     * we split the neutrals payment, so return too a negative value
     * in that case */
    if ( conversion -> account_number == account_number
         &&
         conversion -> last_payment_number == abs (payment_number))
    {
        if (payment_number < 0)
        return -conversion -> new_payment_number;
        else
        return conversion -> new_payment_number;
    }
    tmp_list = tmp_list -> next;
    }
    return 0;
}


/**
 * il arrive que le dernier rapprochement effectué ne soit pas le dernier en date. Ce qui
 * fausse la reconstruction des rapprochements. Dans ce cas on reconstitue le dernier
 * rapprochement en date à partir du dernier rapprochement effectué.
 *
 * \param account_number
 * \param final_date for account
 * \param final_balance for account
 *
 * \return TRUE is OK
 * */
gboolean gsb_file_load_set_last_reconcile_data ( gint account_number,
                        GDate *final_date,
                        gsb_real final_balance )
{
    GList *rec_list;
    gsb_real amount = final_balance;

    devel_debug_int ( account_number );
    rec_list = g_list_last ( gsb_data_reconcile_get_sort_reconcile_list (
                                        account_number) );
    while (rec_list)
    {
        gint reconcile_number_next;
        gint reconcile_number;
        const GDate *last_date_next;
        const GDate *last_date;
                
        reconcile_number = GPOINTER_TO_INT ( rec_list -> data );
        last_date = gsb_data_reconcile_get_final_date ( reconcile_number );        
        if ( g_date_compare ( final_date, last_date ) >= 0 )
        {
            if ( rec_list -> next )
            {
                reconcile_number_next = GPOINTER_TO_INT ( (rec_list -> next) -> data );
                if ( reconcile_number_next )
                    last_date_next = gsb_data_reconcile_get_final_date ( reconcile_number_next );
                if ( g_date_compare ( final_date, last_date_next ) < 0 )
                {
                    gsb_data_reconcile_set_final_balance ( 
                            gsb_data_reconcile_get_account_last_number (account_number),
                            amount );
                    return TRUE;
                }
            }
            else
            {
                gsb_data_reconcile_set_final_date ( reconcile_number, final_date );
                gsb_data_reconcile_set_final_balance ( reconcile_number, final_balance );
                return TRUE;
            }
        }
        else
        {
            amount = gsb_real_add (  amount, gsb_data_reconcile_get_init_balance (
                        reconcile_number));
        }

        rec_list = rec_list -> prev;
    }

    return FALSE;
}


/** 
 * copy an old grisbi file 
 *
 * \param filename the name of the file
 *
 * \return TRUE : ok, FALSE : problem
 * */

void gsb_file_load_copy_old_file ( gchar *filename, gchar *file_content)
{
    if ( g_str_has_suffix (filename, ".gsb" ) )
    {
        GFile * file_ori;
        GFile * file_copy;
        GError * error = NULL;

        copy_old_filename = g_path_get_basename ( filename );
        copy_old_filename = gsb_string_remplace_string ( copy_old_filename, ".gsb",
                        "-old-version.gsb" );
        copy_old_filename = g_build_filename (
                        my_get_XDG_grisbi_data_dir (),
                        copy_old_filename, NULL );

        file_ori = g_file_new_for_path ( filename );
        file_copy = g_file_new_for_path ( copy_old_filename );
        if ( !g_file_copy ( file_ori, file_copy, G_FILE_COPY_OVERWRITE, 
                        NULL, NULL, NULL, &error ) )
        {
            dialogue_error (error -> message );
            g_error_free ( error );
        }
    }
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
