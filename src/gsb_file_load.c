/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                           */
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
#include "config.h"
#endif

#include "include.h"
#include <errno.h>
#include <stdlib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

/*START_INCLUDE*/
#include "gsb_file_load.h"
#include "bet_data.h"
#include "bet_data_finance.h"
#include "bet_graph.h"
#include "bet_tab.h"
#include "custom_list.h"
#include "dialog.h"
#include "export_csv.h"
#include "grisbi_app.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_first.h"
#include "gsb_calendar.h"
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
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_file_util.h"
#include "gsb_locale.h"
#include "gsb_real.h"
#include "gsb_rgba.h"
#include "gsb_select_icon.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "import.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
#ifdef HAVE_SSL
#include "plugins/openssl/openssl.h"
#endif
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

static struct		/* structure download_tmp_values */
{
    gboolean download_ok;
    gchar *file_version;
    gchar *grisbi_version;

    /* there is always only one to TRUE, used to split the retrieves functions */

    gboolean general_part;
    gboolean account_part;
    gboolean report_part;
} download_tmp_values = { FALSE, NULL, NULL, FALSE, FALSE, FALSE};


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

/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
/**
 *  check if the xml file is the last structure (before 0.6) or
 * the new structure (after 0.6)
 *
 * \param file_content the grisbi file
 *
 * \return TRUE if the version is after 0.6
 * */
static gboolean gsb_file_load_check_new_structure ( gchar *file_content )
{
    if ( strstr ( file_content, "Generalites" ) )
        return FALSE;
    return TRUE;
}

/**
 * load the general part in the Grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_general_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint unknown;
    gint i=0;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;

    if (!attribute_names[i])
	{
        return;
	}

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	do
    {
        unknown = 0;

        if ( !strcmp ( attribute_values[i], "(null)" ) )
        {
			i ++;
			continue;
        }

        switch ( attribute_names[i][0] )
        {
            case 'A':
                if ( !strcmp ( attribute_names[i], "Automatic_amount_separator" ))
                    etat.automatic_separator = utils_str_atoi( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Archive_file" ))
                    w_etat->is_archive = utils_str_atoi (attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Add_archive_in_total_balance" ))
                    w_etat->metatree_add_archive_in_totals = utils_str_atoi( attribute_values[i]);

                else
                    unknown = 1;
                break;

            case 'B':
                if ( !strcmp ( attribute_names[i], "Bet_capital" ) )
                    etat.bet_capital = utils_str_safe_strtod ( attribute_values[i], NULL );

                else if ( !strcmp ( attribute_names[i], "Bet_currency" ) )
                    etat.bet_currency = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Bet_taux_annuel" ) )
                    etat.bet_taux_annuel = utils_str_safe_strtod ( attribute_values[i], NULL );

                else if ( !strcmp ( attribute_names[i], "Bet_index_duree" ) )
                    etat.bet_index_duree = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Bet_frais" ) )
                    etat.bet_frais = utils_str_safe_strtod ( attribute_values[i], NULL );

                else if ( !strcmp ( attribute_names[i], "Bet_type_taux" ) )
                    etat.bet_type_taux = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Budget_list_currency_number" ))
                    w_etat->no_devise_totaux_ib = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Bet_array_column_width" ))
                    bet_array_init_largeur_col_treeview (attribute_values[i]);

                else
                    unknown = 1;
                break;

            case 'C':
                if ( !strcmp ( attribute_names[i], "Category_list_currency_number" ))
                    w_etat->no_devise_totaux_categ = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Combofix_mixed_sort" ))
                    etat.combofix_mixed_sort = utils_str_atoi( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Combofix_case_sensitive" ))
                    etat.combofix_case_sensitive = utils_str_atoi( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Combofix_force_payee" ))
                    etat.combofix_force_payee = utils_str_atoi( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Combofix_force_category" ))
                    etat.combofix_force_category = utils_str_atoi( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Crypt_file" ))
                    w_etat->crypt_file = utils_str_atoi (attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "CSV_force_date_valeur_with_date" ))
                    etat.csv_force_date_valeur_with_date = utils_str_atoi( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "CSV_separator" ))
                {
                    g_free ( etat.csv_separator );
                    etat.csv_separator = my_strdup ( attribute_values[i] );
					gsb_csv_export_set_csv_separator (etat.csv_separator);
                }

                else if ( !strcmp ( attribute_names[i], "CSV_skipped_lines" ))
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

                else
                    unknown = 1;
                break;

            case 'D':
                if ( !strcmp ( attribute_names[i], "Date_format" ) )
                    gsb_date_set_format_date ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Decimal_point" ) )
                    gsb_locale_set_mon_decimal_point ( attribute_values[i] );

                else
                    unknown = 1;
                break;

            case 'E':
                if ( !strcmp ( attribute_names[i], "Export_file_format" ) )
                    etat.export_file_format = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Export_files_traitement" ) )
                    etat.export_files_traitement = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Export_force_US_dates" ) )
                    etat.export_force_US_dates = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Export_force_US_numbers" ) )
                    etat.export_force_US_numbers = utils_str_atoi ( attribute_values[i] );

                else if (!strcmp (attribute_names[i], "Export_quote_dates"))
                    w_etat->export_quote_dates = utils_str_atoi (attribute_values[i]);

                else
                    unknown = 1;
                break;

            case 'F':
                if ( !strcmp ( attribute_names[i], "File_version" ))
                {
                    g_free ( download_tmp_values.file_version );
                    download_tmp_values.file_version = my_strdup (attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "File_title" ) && strlen (attribute_values[i]))
				{
                    w_etat->accounting_entity = my_strdup (attribute_values[i]);
				}

                else if (!strcmp (attribute_names[i], "Form_date_force_prev_year"))
				{
                    w_etat->form_date_force_prev_year = utils_str_atoi ( attribute_values[i]);
				}

				else if (!strcmp ( attribute_names[i], "Form_columns_number" ))
				{
					gsb_data_form_new_organization ();
					gsb_data_form_set_nb_columns (utils_str_atoi ( attribute_values[i]));
				}

				else if ( !strcmp ( attribute_names[i], "Form_lines_number" ))
				{
					gsb_data_form_set_nb_rows (utils_str_atoi ( attribute_values[i]));
				}

                else if ( !strcmp ( attribute_names[i], "Form_organization" ))
                {
                    gchar **pointeur_char;
                    gint k, j;

                    pointeur_char = g_strsplit (attribute_values[i], "-", 0);

                    for ( k=0 ; k<MAX_HEIGHT ; k++ )
					{
                        for ( j=0 ; j<MAX_WIDTH ; j++ )
						{
                            gsb_data_form_set_value (j, k, utils_str_atoi (pointeur_char[j + k*MAX_WIDTH]));
						}
					}

                    g_strfreev ( pointeur_char );
                }

                else if ( !strcmp ( attribute_names[i], "Form_columns_width" ))
                {
                    gchar **pointeur_char;
                    gint j;

                    pointeur_char = g_strsplit ( attribute_values[i], "-", 0 );

                    for ( j=0 ; j<MAX_WIDTH ; j++ )
                        gsb_data_form_set_width_column (j, utils_str_atoi (pointeur_char[j]));

                    g_strfreev ( pointeur_char );
                }

				else
                    unknown = 1;
                break;

            case 'G':
                if ( !strcmp ( attribute_names[i], "Grisbi_version" ))
                {
                    g_free ( download_tmp_values.grisbi_version );
                    download_tmp_values.grisbi_version = my_strdup (attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "General_address" ))
                {
					w_etat->adr_common =  utils_str_protect_unprotect_multilines_text (attribute_values[i], FALSE);
                }

                else
                    unknown = 1;
                break;

            case 'I':
                if ( !strcmp ( attribute_names[i], "Import_interval_search" ))
					etat.import_files_nb_days = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Import_copy_payee_in_note" ))
                    etat.copy_payee_in_note = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Import_extract_number_for_check" ))
                    etat.extract_number_for_check = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Import_fusion_transactions" ))
                    etat.fusion_import_transactions = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Import_categorie_for_payee" ))
                    etat.associate_categorie_for_payee = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Import_fyear_by_value_date" ))
                    etat.get_fyear_by_value_date = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Import_qif_no_import_categories" ))
                    etat.qif_no_import_categories = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Import_qif_use_field_extract_method_payment" ))
                    etat.qif_use_field_extract_method_payment = utils_str_atoi ( attribute_values[i]);

                else
                    unknown = 1;
                break;

            case 'M':
                if ( !strcmp ( attribute_names[i], "Metatree_assoc_mode" ))
                    w_etat->metatree_assoc_mode = utils_str_atoi( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Metatree_sort_transactions" ))
                    w_etat->metatree_sort_transactions = utils_str_atoi( attribute_values[i]);

				else if (!strcmp (attribute_names[i], "Metatree_unarchived_payees"))
					w_etat->metatree_unarchived_payees = utils_str_atoi( attribute_values[i]);

				else
                    unknown = 1;
                break;

            case 'N':
                if ( !strcmp ( attribute_names[i], "Navigation_list_order" ) )
                    gsb_gui_navigation_set_page_list_order ( attribute_values[i] );

                else if (!strcmp ( attribute_names[i], "Name_logo"))
                {
					GdkPixbuf *pixbuf = NULL;

					etat.name_logo = my_strdup (attribute_values[i]);
					pixbuf = gdk_pixbuf_new_from_file (etat.name_logo, NULL);
					if (pixbuf)
					{
						gtk_window_set_default_icon (pixbuf);
						gsb_select_icon_set_logo_pixbuf (pixbuf);
						g_object_unref (G_OBJECT (pixbuf));
					}
                }

                else
                    unknown = 1;
                break;

            case 'P':
                if ( !strcmp ( attribute_names[i], "Party_list_currency_number" ))
                    w_etat->no_devise_totaux_tiers = utils_str_atoi ( attribute_values[i]);
                else
                    unknown = 1;
                break;

            case 'R':
                if ( !strcmp ( attribute_names[i], "Reconcile_end_date" ) )
                    w_etat->reconcile_end_date = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Reconcile_sort" ) )
                    w_etat->reconcile_sort = utils_str_atoi ( attribute_values[i] );

                else if ( !strcmp ( attribute_names[i], "Remind_display_per_account" ) )
                    w_etat->retient_affichage_par_compte = utils_str_atoi ( attribute_values[i] );

				else
                    unknown = 1;
                break;

            case 'S':
                if ( !strcmp ( attribute_names[i], "Second_general_address" ))
                {
					w_etat->adr_secondary = utils_str_protect_unprotect_multilines_text (attribute_values[i], FALSE);
                }

                else if ( !strcmp ( attribute_names[i], "Scheduler_column_width" ) )
                {
                    /* initialise la réinitialisation des colonnes */
                    etat.scheduler_column_width = my_strdup ( attribute_values[i] );
                    gsb_scheduler_list_init_tab_width_col_treeview (etat.scheduler_column_width);
                }

                else if ( !strcmp ( attribute_names[i], "Scheduler_view" ))
                    etat.affichage_echeances = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Scheduler_custom_number" ))
                    etat.affichage_echeances_perso_nb_libre = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Scheduler_custom_menu" ))
                    etat.affichage_echeances_perso_j_m_a = utils_str_atoi ( attribute_values[i]);

				else if ( !strcmp ( attribute_names[i], "Scheduler_set_default_account" ))
                    etat.scheduler_set_default_account = utils_str_atoi ( attribute_values[i]);

				else if ( !strcmp ( attribute_names[i], "Scheduler_default_account_number" ))
                    etat.scheduler_default_account_number = utils_str_atoi ( attribute_values[i]);

				else if ( !strcmp ( attribute_names[i], "Scheduler_set_fixed_date" ))
                    etat.scheduler_set_fixed_date = utils_str_atoi ( attribute_values[i]);

				else if ( !strcmp ( attribute_names[i], "Scheduler_default_fixed_date" ))
                    etat.scheduler_set_fixed_date_day = utils_str_atoi ( attribute_values[i]);

                else
                    unknown = 1;
                break;

            case 'T':
                if ( !strcmp ( attribute_names[i], "Two_lines_showed" ))
                    w_run->display_two_lines = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Thousands_separator" ) )
                {
                    if ( !strcmp ( attribute_values[i], "empty" ) )
                        gsb_locale_set_mon_thousands_sep ( NULL );
                    else
                        gsb_locale_set_mon_thousands_sep ( attribute_values[i] );
                }

                else if ( !strcmp ( attribute_names[i], "Transactions_view" ))
                {
					gsb_transactions_list_init_tab_affichage_ope (attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Three_lines_showed" ))
                    w_run->display_three_lines = utils_str_atoi ( attribute_values[i]);

                else if ( !strcmp ( attribute_names[i], "Transaction_column_width" ) )
                {
                    /* initialise la réinitialisation des colonnes */
                    etat.transaction_column_width = my_strdup ( attribute_values[i] );
                    gsb_transactions_list_init_tab_width_col_treeview (etat.transaction_column_width);
                }

                else if ( !strcmp ( attribute_names[i], "Transaction_column_align" ) )
                {
                    /* the transactions columns are xx-xx-xx-xx and we want to set in transaction_col_align[1-2-3...] */
                    gsb_transactions_list_init_tab_align_col_treeview (attribute_values[i]);
                }

                else
                    unknown = 1;
                break;

            case 'U':
                if (!strcmp (attribute_names[i], "Use_icons_file_dir"))
				{
                    w_etat->use_icons_file_dir = utils_str_atoi (attribute_values[i]);
				}
                else if ( !strcmp ( attribute_names[i], "Use_logo" ))
				{
                    etat.utilise_logo = utils_str_atoi ( attribute_values[i]);
					if (etat.utilise_logo)
					{
						GdkPixbuf *pixbuf;

						/* initialise le logo accueil */
						pixbuf = gsb_select_icon_get_default_logo_pixbuf ();
						gsb_select_icon_set_logo_pixbuf (pixbuf);
						g_object_unref (G_OBJECT (pixbuf));
					}

				}
                else
                    unknown = 1;
                break;

            default:
                /* normally, shouldn't come here */
                unknown = 1;
                break;
        }

        if ( unknown == 1 )
        {
            gchar *tmpstr = g_strdup_printf ( "Unknown attribute '%s'", attribute_names[i] );
            devel_debug ( tmpstr );
            g_free ( tmpstr );
        }

        i++;
    }
    while ( attribute_names[i] );
}

/**
 * load the rgba part in the Grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static void gsb_file_load_rgba_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;

	/* ce traitement ne sert plus qu'à la compatibilité descendante de grisbi. Remplacé par un fichier CSS */
    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i], "(null)") )
    {
        /* Nothing */
    }

	else if ( !strcmp ( attribute_names[i], "Background_color_0" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_couleur_fond_0", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Background_color_1" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_couleur_fond_1", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Couleur_jour" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_background_jour", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Background_scheduled" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_background_scheduled", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Background_archive" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_background_archive", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Selection" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_couleur_selection", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Background_split" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_background_split", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Text_color_0" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_text_color_0", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Text_color_1" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_text_color_1", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Entry_error_color" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_entry_error_color", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Couleur_bet_division" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_couleur_bet_division", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Couleur_bet_future" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_couleur_bet_future", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Couleur_bet_solde" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_couleur_bet_solde", attribute_values[i] );
    }

    else if ( !strcmp ( attribute_names[i], "Couleur_bet_transfert" ) )
    {
        gsb_rgba_set_alt_colors ( "alt_couleur_bet_transfert", attribute_values[i] );
    }

    i++;
    }
    while ( attribute_names[i] );
}

/**
 * load the print part in the Grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_print_part ( const gchar **attribute_names,
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
 * load the currencies in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_currency ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint currency_number = -1;
	GrisbiWinEtat *w_etat;

    if ( !attribute_names[i] )
		return;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
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

    if (!strcmp (attribute_names[i], "Nb"))
    {
        currency_number = gsb_data_currency_load_currency (utils_str_atoi (attribute_values[i]));

		if (currency_number == 0)
			return;
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
        Iso4217Currency *currency = gsb_data_currency_get_tab_iso_4217_currencies ();

        gsb_data_currency_set_nickname ( currency_number, attribute_values[i]);

        /* Check if a iso code is the same as currency code (old import).  */
        while ( currency -> country_name )
        {
			if ( !strcmp ( currency -> currency_code_iso, attribute_values[i] ) )
			{
				gsb_data_currency_set_code_iso4217 ( currency_number, attribute_values[i]);
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
    if ( w_etat->no_devise_totaux_tiers == 0 )
    {
        GSList *tmp_list;

        tmp_list = gsb_data_currency_get_currency_list ( );
        if ( g_slist_length ( tmp_list ) > 0 )
            w_etat->no_devise_totaux_tiers = gsb_data_currency_get_no_currency (
                g_slist_nth_data ( tmp_list, 0 ) );
    }
    if ( w_etat->no_devise_totaux_categ == 0 )
    {
        GSList *tmp_list;

        tmp_list = gsb_data_currency_get_currency_list ( );
        if ( g_slist_length ( tmp_list ) > 0 )
            w_etat->no_devise_totaux_categ = gsb_data_currency_get_no_currency (
                        g_slist_nth_data ( tmp_list, 0 ) );
    }
    if ( w_etat->no_devise_totaux_ib == 0 )
    {
        GSList *tmp_list;

        tmp_list = gsb_data_currency_get_currency_list ( );
        if ( g_slist_length ( tmp_list ) > 0 )
            w_etat->no_devise_totaux_ib = gsb_data_currency_get_no_currency (
                g_slist_nth_data ( tmp_list, 0 ) );
    }
}

/**
 * load the account part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_account_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint unknown;
    gint i=0;
    gint account_number = 0;
	gint bet_months = 0;
	gboolean is_loan = FALSE;
	GDate *date = NULL;
	LoanStruct *s_loan = NULL;

    if ( !attribute_names[i] )
        return;

    do
    {
        unknown = 0;

        /* we test at the beginning if the attribute_value is NULL,
         * if yes, go to the next */

        if ( !strcmp (attribute_values[i], "(null)" ) )
        {
            i++;
            continue;
        }

        switch ( attribute_names[i][0] )
        {
            case 'A':
                if ( !strcmp ( attribute_names[i], "Ascending_sort" ))
                {
                    gsb_data_account_set_sort_type ( account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'B':
                if ( !strcmp ( attribute_names[i], "Bank_account_IBAN" ))
                {
                    gsb_data_account_set_bank_account_iban ( account_number,
                            attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Bank" ))
                {
                    gsb_data_account_set_bank ( account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Bank_branch_code" ))
                {
                    gsb_data_account_set_bank_branch_code ( account_number,
                            attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Bank_account_number" ))
                {
                    gsb_data_account_set_bank_account_number ( account_number,
                            attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Bet_use_budget" ))
                {
                    gsb_data_account_set_bet_use_budget ( account_number,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_credit_card" ))
                {
                    gsb_data_account_set_bet_credit_card ( account_number,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_start_date" ))
                {
					date = gsb_parse_date_string_safe (attribute_values[i]);
                    gsb_data_account_set_bet_start_date ( account_number, date);
                }

                else if ( !strcmp ( attribute_names[i], "Bet_months" ))
                {
					bet_months = utils_str_atoi (attribute_values[i]);
                    gsb_data_account_set_bet_months ( account_number, bet_months);
                }

                else if ( !strcmp ( attribute_names[i], "Bet_UT" ))
                {
                    gsb_data_account_set_bet_spin_range ( account_number,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_auto_inc_month" ))
                {
                    gboolean auto_inc_month;

                    auto_inc_month = utils_str_atoi ( attribute_values[i] );
                    gsb_data_account_set_bet_auto_inc_month ( account_number, auto_inc_month );
                    if ( auto_inc_month )
                        gsb_data_account_bet_update_initial_date_if_necessary ( account_number );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_select_transaction_label" ))
                {
                    gsb_data_account_set_bet_select_label ( account_number,
                            SPP_ORIGIN_TRANSACTION,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_select_scheduled_label" ))
                {
                    gsb_data_account_set_bet_select_label ( account_number,
                            SPP_ORIGIN_SCHEDULED,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_select_futur_label" ))
                {
                    gsb_data_account_set_bet_select_label ( account_number,
                            SPP_ORIGIN_FUTURE,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_SD" ))
                {
                    gsb_data_account_set_bet_hist_data ( account_number,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_Fi" ))
                {
                    gsb_data_account_set_bet_hist_fyear ( account_number,
                            utils_str_atoi ( attribute_values[i] ) );
                }

                else if ( !strcmp ( attribute_names[i], "Bet_capital" ))
                {
 					s_loan = bet_data_loan_struct_loan_init ();
					s_loan->capital = g_ascii_strtod (attribute_values[i], NULL);
					s_loan->capital_du = s_loan->capital;
					gsb_data_account_set_bet_finance_capital (account_number, s_loan->capital);
					s_loan->account_number = account_number;
					s_loan->version_number = 0;
					s_loan->duree = bet_months;
					s_loan->first_date = date;
					is_loan = TRUE;
                }

                else if ( !strcmp ( attribute_names[i], "Bet_taux_annuel" ))
                {
					if (s_loan)
					{
						s_loan->annual_rate = g_ascii_strtod (attribute_values[i], NULL);
						gsb_data_account_set_bet_finance_taux_annuel (account_number, s_loan->annual_rate);
					}
                }

                else if ( !strcmp ( attribute_names[i], "Bet_frais" ))
                {
					if (s_loan)
					{
						s_loan->fees = g_ascii_strtod (attribute_values[i], NULL);
						gsb_data_account_set_bet_finance_frais (account_number, s_loan->fees);
					}
                }

                else if ( !strcmp ( attribute_names[i], "Bet_type_taux" ))
                {
					if (s_loan)
					{
						s_loan->type_taux = utils_str_atoi (attribute_values[i]);
						gsb_data_account_set_bet_finance_type_taux (account_number, s_loan->type_taux);
					}
                }

                else
                    unknown = 1;
                break;

            case 'C':
                if ( !strcmp ( attribute_names[i], "Currency" ))
                {
                    gsb_data_account_set_currency ( account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Closed_account" ))
                {
                    gsb_data_account_set_closed_account ( account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Comment" ))
                {
					gchar *tmp_str;

					tmp_str = utils_str_protect_unprotect_multilines_text (attribute_values[i], FALSE);
                    gsb_data_account_set_comment ( account_number, tmp_str);
					g_free (tmp_str);
                }

                else if ( !strcmp ( attribute_names[i], "Column_sort" ))
                {
                    gsb_data_account_set_sort_column ( account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'D':
                if ( !strcmp ( attribute_names[i], "Default_debit_method" ))
                {
                    gsb_data_account_set_default_debit ( account_number,
                            utils_str_atoi ( attribute_values[i]) );
                }

                else if ( !strcmp ( attribute_names[i], "Default_credit_method" ))
                {
                    gsb_data_account_set_default_credit ( account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

			case 'F':
				if (!strcmp ( attribute_names[i], "Form_columns_number" ))
				{
					gsb_data_form_new_organization ();
					gsb_data_form_set_nb_columns (utils_str_atoi ( attribute_values[i]));
					gsb_file_set_modified (TRUE);
				}

				else if ( !strcmp ( attribute_names[i], "Form_lines_number" ))
				{
					gsb_data_form_set_nb_rows (utils_str_atoi ( attribute_values[i]));
				}

                else if ( !strcmp ( attribute_names[i], "Form_organization" ))
                {
                    gchar **pointeur_char;
                    gint k, j;

                    pointeur_char = g_strsplit (attribute_values[i], "-", 0);

                    for ( k=0 ; k<MAX_HEIGHT ; k++ )
                        for ( j=0 ; j<MAX_WIDTH ; j++ )
                            gsb_data_form_set_value (j, k, utils_str_atoi (pointeur_char[j + k*MAX_WIDTH]));

                    g_strfreev ( pointeur_char );
                }

                else if ( !strcmp ( attribute_names[i], "Form_columns_width" ))
                {
                    gchar **pointeur_char;
                    gint j;

                    pointeur_char = g_strsplit ( attribute_values[i], "-", 0 );

                    for ( j=0 ; j<MAX_WIDTH ; j++ )
                        gsb_data_form_set_width_column (j, utils_str_atoi (pointeur_char[j]));

                    g_strfreev ( pointeur_char );
                }

                else
                    unknown = 1;
                break;

            case 'I':
                if ( !strcmp ( attribute_names[i], "Id" ))
                {
                    if ( strlen (attribute_values[i]))
                        gsb_data_account_set_id (account_number,
                                attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Initial_balance" ))
                {
                    gsb_data_account_set_init_balance ( account_number,
                            gsb_real_safe_real_from_string (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'K':
                if ( !strcmp ( attribute_names[i], "Kind" ))
                {
                    gsb_data_account_set_kind (account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Key" ))
                {
                    gsb_data_account_set_bank_account_key ( account_number,
                            attribute_values[i]);
                }

                else
                    unknown = 1;
                break;

            case 'L':
                if ( !strcmp ( attribute_names[i], "Lines_per_transaction" ))
                {
                    gsb_data_account_set_nb_rows ( account_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'M':
                if ( !strcmp ( attribute_names[i], "Minimum_wanted_balance" ))
                {
                    gsb_data_account_set_mini_balance_wanted ( account_number,
                            gsb_real_safe_real_from_string (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Minimum_authorised_balance" ))
                {
                    gsb_data_account_set_mini_balance_authorized ( account_number,
                            gsb_real_safe_real_from_string (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'N':
                if ( !strcmp ( attribute_names[i], "Name" ))
                {
                    account_number = gsb_data_account_new ( GSB_TYPE_BANK );
                    gsb_data_account_set_name ( account_number,
                            attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Number" ))
                {
                    account_number = gsb_data_account_set_account_number ( account_number,
                            utils_str_atoi ( attribute_values[i]));
					if (account_number == 0)
					{
						GrisbiWinRun *w_run;

						w_run = grisbi_win_get_w_run ();
						w_run->account_number_is_0 = TRUE;
					}
                }

                else if ( !strcmp ( attribute_names[i], "Neutrals_inside_method" ))
                {
                    gsb_data_account_set_split_neutral_payment ( account_number,
                            utils_str_atoi ( attribute_values[i]) );
                }

                else
                    unknown = 1;
                break;

            case 'O':
                if ( !strcmp ( attribute_names[i], "Owner" ))
                {
                    gsb_data_account_set_holder_name ( account_number,
                            attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Owner_address" ))
                {
					gchar *owner_str;

					owner_str = utils_str_protect_unprotect_multilines_text (attribute_values[i], FALSE);
                    gsb_data_account_set_holder_address (account_number, owner_str);
					g_free (owner_str);
                }

                else
                    unknown = 1;
                break;

            case 'P':
                if ( !strcmp ( attribute_names[i], "Path_icon" ) )
                {
					GdkPixbuf *pixbuf = NULL;
					gchar *tmp_filename;
					GrisbiWinEtat *w_etat;

					w_etat = grisbi_win_get_w_etat ();
					if (w_etat->use_icons_file_dir)
					{
						tmp_filename = g_build_filename (gsb_dirs_get_user_icons_dir (),
														 attribute_values[i],
														 NULL);
					}
					else
					{
						tmp_filename = g_strdup (attribute_values[i]);
					}
                    gsb_data_account_set_name_icon (account_number, tmp_filename);
					pixbuf = gsb_select_icon_new_account_pixbuf_from_file (tmp_filename);
					gsb_data_account_set_account_icon_pixbuf (account_number, pixbuf);
					g_free (tmp_filename);
                }

                else
                    unknown = 1;
                break;

            case 'S':
                if ( !strcmp ( attribute_names[i], "Sort_by_method" ))
                {
                    gsb_data_account_set_reconcile_sort_type ( account_number,
                            utils_str_atoi ( attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Show_marked" ))
                {
                    gsb_data_account_set_r ( account_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Show_archives_lines" ))
                {
                    gsb_data_account_set_l ( account_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Sort_order" ))
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
                }

                else if ( !strcmp ( attribute_names[i], "Sorting_kind_column" ))
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
                }

                else
                    unknown = 1;
                break;

            default:
                /* normally, shouldn't come here */
                unknown = 1;
                break;
        }

        if ( unknown == 1 )
        {
            gchar *tmpstr = g_strdup_printf ( "Unknown attribute '%s'", attribute_names[i] );
            devel_debug ( tmpstr );
            g_free ( tmpstr );
        }

        i++;
    }
    while ( attribute_names[i] );
	if (is_loan)
	{
		bet_data_loan_add_item (s_loan);
		gsb_file_set_modified (TRUE);
	}
	 else if (date)
		g_date_free (date);
}

/**
 * load the payment part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_payment_part ( const gchar **attribute_names,
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
static  void gsb_file_load_transactions ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint unknown;
    gint i=0;
    gint transaction_number = 0;
    gint account_number = 0;
    GDate *parsed_date;

    if ( !attribute_names[i] )
        return;

    do
    {
        unknown = 0;

        /* we test at the beginning if the attribute_value is NULL,
         * if yes, go to the next */

        if ( !strcmp (attribute_values[i], "(null)" ) )
        {
            i++;
            continue;
        }

        switch ( attribute_names[i][0] )
        {
            case 'A':
                if ( !strcmp ( attribute_names[i], "Ac" ))
                {
                    account_number = utils_str_atoi (attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Am" ))
                {
                    /* get the entire real, even if the floating point of the currency is less deep */
                    gsb_data_transaction_set_amount ( transaction_number,
                            gsb_real_safe_real_from_string (attribute_values[i]));
                            }

                else if ( !strcmp ( attribute_names[i], "Ar" ))
                {
                    gsb_data_transaction_set_archive_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Au" ))
                {
                    gsb_data_transaction_set_automatic_transaction ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'B':
                if ( !strcmp ( attribute_names[i], "Br" ))
                {
                        gsb_data_transaction_set_split_of_transaction ( transaction_number,
                                utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Ba" ))
                {
                        gsb_data_transaction_set_bank_references ( transaction_number,
                                attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Bu" ))
                {
                        gsb_data_transaction_set_budgetary_number ( transaction_number,
                                utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'C':
                if ( !strcmp ( attribute_names[i], "Ca" ))
                {
                    gsb_data_transaction_set_category_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Cu" ))
                {
                    gsb_data_transaction_set_currency_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'D':
                if ( !strcmp ( attribute_names[i], "Dt" ))
                {
                        parsed_date = gsb_parse_date_string_safe ( attribute_values[i] );
                        gsb_data_transaction_set_date ( transaction_number,
                                parsed_date );
                        g_date_free ( parsed_date );
                }

                else if ( !strcmp ( attribute_names[i], "Dv" ))
                {
                        parsed_date = gsb_parse_date_string_safe ( attribute_values[i] );
                        gsb_data_transaction_set_value_date ( transaction_number,
                                parsed_date );
                        g_date_free ( parsed_date );
                }

                else
                    unknown = 1;
                break;

            case 'E':
                if ( !strcmp ( attribute_names[i], "Exb" ))
                {
                        gsb_data_transaction_set_change_between ( transaction_number,
                                utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Exr" ))
                {
                        gsb_data_transaction_set_exchange_rate ( transaction_number,
                                gsb_real_safe_real_from_string (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Exf" ))
                {
                        gsb_data_transaction_set_exchange_fees ( transaction_number,
                                gsb_real_safe_real_from_string (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'F':
                if ( !strcmp ( attribute_names[i], "Fi" ))
                {
                    gsb_data_transaction_set_financial_year_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'I':
                if ( !strcmp ( attribute_names[i], "Id" ))
                {
                    gsb_data_transaction_set_transaction_id ( transaction_number,
                            attribute_values[i]);
                }

                else
                    unknown = 1;
                break;

            case 'M':
                if ( !strcmp ( attribute_names[i], "Ma" ))
                {
                    gsb_data_transaction_set_marked_transaction ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Mo" ))
                {
                    gsb_data_transaction_set_mother_transaction_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'N':
                if ( !strcmp ( attribute_names[i], "No" ))
                {
                    gsb_data_transaction_set_notes ( transaction_number,
                            attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Nb" ))
                {
                    transaction_number = gsb_data_transaction_new_transaction_with_number ( account_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'P':
                if ( !strcmp ( attribute_names[i], "Pn" ))
                {
                    gsb_data_transaction_set_method_of_payment_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Pc" ))
                {
                    gsb_data_transaction_set_method_of_payment_content ( transaction_number,
                            attribute_values[i]);
                }

                else if ( !strcmp ( attribute_names[i], "Pa" ))
                {
                    gsb_data_transaction_set_party_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'R':
                if ( !strcmp ( attribute_names[i], "Re" ))
                {
                    gsb_data_transaction_set_reconcile_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'S':
                if ( !strcmp ( attribute_names[i], "Sca" ))
                {
                    gsb_data_transaction_set_sub_category_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else if ( !strcmp ( attribute_names[i], "Sbu" ))
                {
                    gsb_data_transaction_set_sub_budgetary_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            case 'V':
                if ( !strcmp ( attribute_names[i], "Vo" ))
                {
                    gsb_data_transaction_set_voucher ( transaction_number,
                            attribute_values[i]);
                }

                else
                    unknown = 1;
                break;

            case 'T':
                if ( !strcmp ( attribute_names[i], "Trt" ))
                {
                    gsb_data_transaction_set_contra_transaction_number ( transaction_number,
                            utils_str_atoi (attribute_values[i]));
                }

                else
                    unknown = 1;
                break;

            default:
                /* normally, shouldn't come here */
                unknown = 1;
                break;
        }

        if ( unknown == 1 )
        {
            gchar *tmpstr = g_strdup_printf ( "Unknown attribute '%s'", attribute_names[i] );
            devel_debug ( tmpstr );
            g_free ( tmpstr );
        }

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
static  void gsb_file_load_scheduled_transactions ( const gchar **attribute_names,
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
		GDate *date;

		date = gsb_parse_date_string_safe (attribute_values[i]);
        gsb_data_scheduled_set_date (scheduled_number, date);
        i++;
		g_date_free (date);
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

    if ( !strcmp ( attribute_names[i], "Fd" ))
    {
        gsb_data_scheduled_set_fixed_date ( scheduled_number, utils_str_atoi (attribute_values[i]));
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
static  void gsb_file_load_party ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    gint payee_number;
	struct ImportPayeeAsso *assoc = NULL;

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

    if ( !strcmp ( attribute_names[i], "Search" ))
    {
        if ( attribute_values[i] && strlen (attribute_values[i]) > 0 )
        {
            gsb_data_payee_set_search_string ( payee_number, attribute_values[i]);
			g_free(assoc); /* in case it was already allocated */
			assoc = g_malloc (sizeof (struct ImportPayeeAsso));
			assoc->payee_number = payee_number;
			assoc->search_str = g_strdup (attribute_values[i]);
        }
        i++;
        continue;
    }

    if (!strcmp (attribute_names[i], "IgnCase"))
    {
		if (assoc)
		{
			assoc->ignore_case = utils_str_atoi (attribute_values[i]);
			gsb_data_payee_set_ignore_case (payee_number, assoc->ignore_case);
		}
        i++;
        continue;
    }

    if (!strcmp (attribute_names[i], "UseRegex"))
    {
		if (assoc)
		{
			assoc->use_regex = utils_str_atoi (attribute_values[i]);
			gsb_data_payee_set_use_regex (payee_number, assoc->use_regex);
        }
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );
	if (assoc)
		gsb_import_associations_list_append_assoc (payee_number, assoc);
}

/**
 * load the currency_links in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_currency_link ( const gchar **attribute_names,
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
static  void gsb_file_load_bank ( const gchar **attribute_names,
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
		gchar *adr_str;

		adr_str = utils_str_protect_unprotect_multilines_text (attribute_values[i], FALSE);
        gsb_data_bank_set_bank_address ( bank_number,adr_str);
		g_free (adr_str);
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

    if ( !strcmp ( attribute_names[i], "Rem" ))
    {
		gchar *rem_str;

		rem_str = utils_str_protect_unprotect_multilines_text (attribute_values[i], FALSE);
        gsb_data_bank_set_bank_note (bank_number, rem_str);
		g_free (rem_str);
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
static  void gsb_file_load_financial_year ( const gchar **attribute_names,
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
        gsb_data_fyear_set_beginning_date ( fyear_number, date );
        if ( date )
            g_date_free (date);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Edte" ))
    {
        date = gsb_parse_date_string_safe (attribute_values[i]);
        gsb_data_fyear_set_end_date ( fyear_number, date );
        if ( date )
            g_date_free (date);
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
static  void gsb_file_load_archive ( const gchar **attribute_names,
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
static  void gsb_file_load_reconcile ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    GDate *date;
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
        date = gsb_parse_date_string_safe ( attribute_values[i] );
        gsb_data_reconcile_set_init_date ( reconcile_number, date );
        i++;
        g_date_free ( date );
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Fdate" ))
    {
        date = gsb_parse_date_string_safe ( attribute_values[i] );
        gsb_data_reconcile_set_final_date ( reconcile_number, date );
        i++;
        g_date_free ( date );
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
static  void gsb_file_load_import_rule ( const gchar **attribute_names,
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

	if ( !strcmp ( attribute_names[i], "Typ" ))
    {
        gsb_data_import_rule_set_type (import_rule_number, attribute_values[i]);
        i++;
        continue;
    }

	if (!strcmp (attribute_names[i], "IdC"))
    {
        gsb_data_import_rule_set_csv_account_id_col (import_rule_number, utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

	if (!strcmp (attribute_names[i], "IdR"))
    {
        gsb_data_import_rule_set_csv_account_id_row (import_rule_number, utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

	if ( !strcmp ( attribute_names[i], "FiS" ))
    {
        gsb_data_import_rule_set_csv_fields_str (import_rule_number, attribute_values[i]);
        i++;
        continue;
    }

	if (!strcmp ( attribute_names[i], "Fld"))
    {
        gsb_data_import_rule_set_csv_first_line_data (import_rule_number, utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

	if (!strcmp( attribute_names[i], "Hp"))
    {
        gsb_data_import_rule_set_csv_headers_present (import_rule_number, utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

	if ( !strcmp ( attribute_names[i], "Sep" ))
    {
        gsb_data_import_rule_set_csv_separator (import_rule_number, attribute_values[i]);
        i++;
        continue;
    }

	if ( !strcmp ( attribute_names[i], "SpCN" ))
    {
        gsb_data_import_rule_set_csv_spec_cols_name (import_rule_number, attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "SpCN" ))
    {
        gsb_data_import_rule_set_csv_spec_cols_name (import_rule_number, attribute_values[i]);
        i++;
        continue;
    }

	if ( !strcmp ( attribute_names[i], "NbSL" ))
    {
        gsb_data_import_rule_set_csv_spec_nbre_lines (import_rule_number, utils_str_atoi (attribute_values[i]));
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
 **/
static  void gsb_file_load_import_rule_spec_line (const gchar **attribute_names,
												  const gchar **attribute_values)
{
	GSList *list = NULL;
    gint i=0;
	gint index =0;
    gint import_rule_number = 0;
	SpecConfData *spec_conf_data;

	if ( !attribute_names[i])
		return;

	spec_conf_data = g_malloc0 (sizeof (SpecConfData));
    do
    {
		/* we test at the beginning if the attribute_value is NULL, if yes, */
		/* go to the next */
		if (!strcmp (attribute_values[i], "(null)"))
		{
			i++;
			continue;
		}

		if (!strcmp ( attribute_names[i], "Nb"))
		{
			index = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		if (!strcmp ( attribute_names[i], "NuR"))
		{
			import_rule_number = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		if (!strcmp( attribute_names[i], "SpA"))
		{
			spec_conf_data->csv_spec_conf_action = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		if (!strcmp( attribute_names[i], "SpAD"))
		{
			spec_conf_data->csv_spec_conf_action_data = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		if (!strcmp( attribute_names[i], "SpUD"))
		{
			spec_conf_data->csv_spec_conf_used_data = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		if ( !strcmp ( attribute_names[i], "SpUT" ))
		{
			spec_conf_data->csv_spec_conf_used_text = g_strdup (attribute_values[i]);
			i++;
			continue;
		}

		/* normally, shouldn't come here */
		i++;
    }
    while (attribute_names[i]);

	if (index)
	{
		list = gsb_data_import_rule_get_csv_spec_lines_list	(import_rule_number);
		list = g_slist_append (list, spec_conf_data);
		gsb_data_import_rule_set_csv_spec_lines_list (import_rule_number, list);
	}
	else if (spec_conf_data)
		g_free (spec_conf_data);
}

/**
 * load the partial balance structure in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_partial_balance ( const gchar **attribute_names,
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
static  void gsb_file_load_bet_part ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

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
        w_etat->bet_debut_period = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bet_deb_cash_account_option" ) )
    {
        w_etat->bet_cash_account_option = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }

    while ( attribute_names[i] );
}

#ifdef HAVE_GOFFICE
 /**
 * load the bet_graph preferences in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_bet_graph_part ( const gchar **attribute_names,
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

    if ( !strcmp ( attribute_names[i], "prefs" ) )
    {
        bet_graph_set_configuration_variables ( attribute_values[i] );
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }

    while ( attribute_names[i] );
}
#endif /* HAVE_GOFFICE */

/**
 * load the historical balance part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_bet_historical ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    HistDiv *shd;
    HistDiv *sub_shd = NULL;
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
		if (sub_shd)
			sub_shd -> div_edited = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "SDamount" ) )
    {
		if (sub_shd)
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
static  void gsb_file_load_bet_future_data ( const gchar **attribute_names,
                        const gchar **attribute_values )
{
    gint i=0;
    FuturData *scheduled;

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
static void gsb_file_load_bet_transfert_part (const gchar **attribute_names,
											  const gchar **attribute_values )
{
    gint i=0;
    TransfertData *transfert;

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
        transfert->date_debit = gsb_parse_date_string_safe ( attribute_values[i] );
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

    if ( !strcmp ( attribute_names[i], "Dd" ) )
    {
        transfert->direct_debit = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

	if (!strcmp ( attribute_names[i], "MCbd"))
	{
		transfert->main_choice_debit_day = utils_str_atoi (attribute_values[i]);
		i++;
		continue;
	}

    if ( !strcmp ( attribute_names[i], "Mlbd" ) ) /* old option */
	{
        transfert->main_choice_debit_day = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Pa" ) )
    {
        transfert->main_payee_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Pn" ) )
    {
        transfert->main_payment_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Ca" ) )
    {
        transfert->main_category_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Sca" ) )
    {
        transfert->main_sub_category_number =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Bu" ) )
    {
        transfert->main_budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Sbu" ) )
    {
        transfert->main_sub_budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "Dtb" ) )
    {
        transfert->date_bascule = gsb_parse_date_string_safe ( attribute_values[i] );
        i++;
        continue;
    }

	if ( !strcmp ( attribute_names[i], "CCbd" ) )
    {
        transfert->card_choice_bascule_day = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "CPa" ) )
    {
        transfert->card_payee_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "CCa" ) )
    {
        transfert -> card_category_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "CSca" ) )
    {
        transfert -> card_sub_category_number =  utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "CBu" ) )
    {
        transfert -> card_budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "CSbu" ) )
    {
        transfert -> card_sub_budgetary_number = utils_str_atoi ( attribute_values[i] );
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i], "CPn" ) )
    {
        transfert->card_payment_number = utils_str_atoi ( attribute_values[i] );
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
 * load the loan part in the grisbi file
 *
 * \param attribute_names
 * \param attribute_values
 *
 * */
static  void gsb_file_load_bet_loan_part (const gchar **attribute_names,
										  const gchar **attribute_values)
{
    gint i=0;
	LoanStruct *s_loan;

    if (!attribute_names[i])
		return;

	s_loan = bet_data_loan_struct_loan_init ();
    do
    {
		/* we test at the beginning if the attribute_value is NULL, if yes, go to the next */
		if (!strcmp ( attribute_values[i], "(null)"))
		{
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "Nb"))
		{
			s_loan->number = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "Ac"))
		{
			s_loan->account_number = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "Ver"))
		{
			s_loan->version_number = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "InCol"))
		{
			s_loan->invers_cols_cap_ech = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "Ca"))
		{
			s_loan->capital = g_ascii_strtod (attribute_values[i], NULL);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "Duree"))
		{
			s_loan->duree = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "FDate"))
		{
			s_loan->first_date = gsb_parse_date_string_safe (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "Fees"))
		{
			s_loan->fees = g_ascii_strtod (attribute_values[i], NULL);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "Taux"))
		{
			s_loan->annual_rate = g_ascii_strtod (attribute_values[i], NULL);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "TyTaux"))
		{
			s_loan->type_taux = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "FEchDif"))
		{
			s_loan->first_is_different = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "FCa"))
		{
			s_loan->first_capital = g_ascii_strtod (attribute_values[i], NULL);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "FIn"))
		{
			s_loan->first_interests = g_ascii_strtod (attribute_values[i], NULL);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "OEch"))
		{
			s_loan->other_echeance_amount = g_ascii_strtod (attribute_values[i], NULL);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "ISchWL"))
		{
			s_loan->init_sch_with_loan = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "AAc"))
		{
			s_loan->associated_account = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "ASch"))
		{
			s_loan->associated_scheduled = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "AFr"))
		{
			s_loan->associated_frequency = utils_str_atoi (attribute_values[i]);
			i++;
			continue;
		}

		else if (!strcmp ( attribute_names[i], "CaDu"))
		{
			s_loan->capital_du = g_ascii_strtod (attribute_values[i], NULL);
			i++;
			continue;
		}

		/* normally, shouldn't come here */
		i++;
    }

    while (attribute_names[i]);

	bet_data_loan_add_item (s_loan);
}

/**
 * Fonction de traitement du fichier
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void gsb_file_load_start_element ( GMarkupParseContext *context,
                        const gchar *element_name,
                        const gchar **attribute_names,
                        const gchar **attribute_values,
                        gpointer user_data,
                        GError **error)
{
    gint unknown = 0;

    /* the first time we come here, we check if it's a Grisbi file */
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

    switch ( element_name[0] )
    {
        case 'A':
            if ( !strcmp ( element_name, "Account" ))
            {
                gsb_file_load_account_part ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Archive" ))
            {
                gsb_file_load_archive ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Amount_comparison" ))
            {
                gsb_file_load_amount_comparison ( attribute_names,
                        attribute_values);
            }

            else
                unknown = 1;
            break;

        case 'C':
            if ( !strcmp ( element_name, "Category" ))
            {
                gsb_file_load_category ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Currency" ))
            {
                gsb_file_load_currency ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Currency_link" ))
            {
                gsb_file_load_currency_link ( attribute_names,
                        attribute_values );
            }

            else
                unknown = 1;
            break;

        case 'B':
            if ( !strcmp ( element_name, "Bank" ))
            {
                gsb_file_load_bank ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Budgetary" ))
            {
                gsb_file_load_budgetary ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Bet" ) )
            {
                gsb_file_load_bet_part ( attribute_names,
                        attribute_values );
            }

#ifdef HAVE_GOFFICE
            else if ( !strcmp ( element_name, "Bet_graph" ) )
            {
                gsb_file_load_bet_graph_part ( attribute_names, attribute_values );
            }
#endif /* HAVE_GOFFICE */

            else if ( !strcmp ( element_name, "Bet_historical" ) )
            {
                gsb_file_load_bet_historical ( attribute_names, attribute_values );
            }

            else if ( !strcmp ( element_name, "Bet_future" ) )
            {
                gsb_file_load_bet_future_data ( attribute_names, attribute_values );
            }

            else if ( !strcmp ( element_name, "Bet_loan" ) )
            {
                gsb_file_load_bet_loan_part (attribute_names, attribute_values);
            }

            else if ( !strcmp ( element_name, "Bet_transfert" ) )
            {
                gsb_file_load_bet_transfert_part ( attribute_names, attribute_values );
            }

            else
                unknown = 1;
            break;

        case 'G':
            if ( !strcmp ( element_name, "General" ))
            {
                gsb_file_load_general_part ( attribute_names,
                        attribute_values );
            }

            else
                unknown = 1;
            break;

        case 'P':
            if ( !strcmp ( element_name, "Partial_balance" ))
            {
                gsb_file_load_partial_balance ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Print" ))
            {
                gsb_file_load_print_part ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Payment" ))
            {
                gsb_file_load_payment_part ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Party" ))
            {
                gsb_file_load_party ( attribute_names,
                        attribute_values );
            }

            else
                unknown = 1;
            break;

        case 'T':
            if ( !strcmp ( element_name, "Transaction" ))
            {
                gsb_file_load_transactions ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Text_comparison" ))
            {
                gsb_file_load_text_comparison ( attribute_names,
                        attribute_values);
            }

            else
                unknown = 1;
            break;

        case 'S':
            if ( !strcmp ( element_name, "Scheduled" ))
            {
                gsb_file_load_scheduled_transactions ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Sub_category" ))
            {
                gsb_file_load_sub_category ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Sub_budgetary" ))
            {
                gsb_file_load_sub_budgetary ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Special_line" ))
            {
                gsb_file_load_import_rule_spec_line (attribute_names, attribute_values);
            }

            else
                unknown = 1;
            break;

        case 'F':
            if ( !strcmp ( element_name, "Financial_year" ))
            {
                gsb_file_load_financial_year ( attribute_names,
                        attribute_values );
            }

            else
                unknown = 1;
            break;

        case 'R':
            if ( !strcmp ( element_name, "Reconcile" ))
            {
                gsb_file_load_reconcile ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "Report" ))
            {
                gsb_file_load_report ( attribute_names,
                        attribute_values );
            }

            else if ( !strcmp ( element_name, "RGBA" ))
            {
                gsb_file_load_rgba_part ( attribute_names,
                        attribute_values );
            }

            else
                unknown = 1;
            break;

        case 'I':
            if ( !strcmp ( element_name, "Import_rule" ))
            {
                gsb_file_load_import_rule ( attribute_names,
                        attribute_values );
            }

            else
                unknown = 1;
            break;

        default:
            /* normally, shouldn't come here */
            unknown = 1;
            break;
    }

    if ( unknown == 1 )
    {
        gchar *tmpstr = g_strdup_printf ( "Unknown element '%s'", element_name );
        devel_debug ( tmpstr );
        g_free ( tmpstr );
    }
}

/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 * called to open the grisbi file given in param
 *
 * \filename the filename to load with full path
 *
 * \return TRUE if ok
 * */
gboolean gsb_file_load_open_file (const gchar *filename )
{
    gchar *file_content;
    gchar *tmp_file_content;
    gulong length;
	gboolean changed = TRUE;
	gboolean show_msg = TRUE;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;

    devel_debug (filename);

#ifndef G_OS_WIN32      /* check the access to the file and display a message */
    gint return_value;
    struct stat buffer_stat;

     /* fill the buffer stat to check the permission */
    return_value = g_stat ( filename, &buffer_stat );
    if ( !return_value && buffer_stat.st_mode & (S_IRGRP | S_IROTH) )
        gsb_file_util_display_warning_permissions ();
#endif /* G_OS_WIN32 */

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	/* load the file */
    if (gsb_file_util_get_contents (filename, &tmp_file_content, &length))
    {
        GMarkupParser *markup_parser;
        GMarkupParseContext *context;
		gboolean is_crypt = FALSE;
		GrisbiWinRun *w_run;

		/* first, we check if the file is crypted, if it is, we decrypt it */
		if ( !strncmp ( tmp_file_content, "Grisbi encrypted file ", 22 ) ||
			 !strncmp ( tmp_file_content, "Grisbi encryption v2: ", 22 ) )
		{
#ifdef HAVE_SSL
			length = gsb_file_util_crypt_file ( filename, &tmp_file_content, FALSE, length );

			if ( ! length )
			{
				g_free (tmp_file_content);
				return FALSE;
			}
			else
				is_crypt = TRUE;
#else
			{
				gchar *text;
				gchar *hint;

				g_free (tmp_file_content);
				text = g_strdup_printf (_("This build of Grisbi does not support encryption.\n"
										  "Please recompile Grisbi with OpenSSL encryption enabled."));

				hint = g_strdup_printf (_("Cannot open encrypted file '%s'"), filename);

				dialogue_error_hint ( text, hint );
				g_free ( hint );
				g_free (text);
				return FALSE;
			}
#endif
		}

		/* si le fichier n'a pas été chiffré et n'est pas un fichier UTF8 valide on le corrige si possible */
		if (!is_crypt && !g_utf8_validate (tmp_file_content, length, NULL))
		{
			GtkWidget *dialog;
			gchar *text;
			gchar *hint;

			hint = g_strdup_printf (_("'%s' is not a valid UTF8 file"), filename);


			text = g_strdup_printf (_("You can choose to fix the file with the substitution character? "
									  "or return to the file choice.\n"));

			dialog = dialogue_special_no_run (GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, text, hint);

			gtk_dialog_add_buttons (GTK_DIALOG(dialog),
									_("Load another file"), GTK_RESPONSE_NO,
									_("Correct the file"), GTK_RESPONSE_OK,
										NULL);
			if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
			{
				file_content = g_utf8_make_valid (tmp_file_content, length);
				gtk_widget_destroy (dialog);
			}
			else
			{
				g_free (tmp_file_content);
				gtk_widget_destroy (dialog);
				return FALSE;
			}
		}
		else
			file_content = tmp_file_content;

		/* set the icons directory */
		gsb_dirs_set_user_icons_dir (filename);

		w_run = grisbi_win_get_w_run ();

		/* we begin to check if we are in a version under 0.6 or 0.6 and above,
		 * because the xml structure changes after 0.6 */
		markup_parser = g_malloc0 (sizeof (GMarkupParser));
		if ( gsb_file_load_check_new_structure (file_content))
		{
			/* fill the GMarkupParser for a new xml structure */
			markup_parser -> start_element = (void *) gsb_file_load_start_element;
			markup_parser -> error = (void *) gsb_file_load_error;
			w_run->old_version = FALSE;
		}
		else
		{
			w_run->old_version = TRUE;
			g_free (markup_parser);

			return FALSE;
		}

		context = g_markup_parse_context_new ( markup_parser,
							0,
							NULL,
						NULL );
		download_tmp_values.download_ok = FALSE;

		if (! g_markup_parse_context_parse ( context,
						file_content,
						strlen (file_content),
						NULL ))
		{
			download_tmp_values.download_ok = FALSE;
		}

		g_markup_parse_context_free (context);
		g_free (markup_parser);
		g_free (file_content);

		if ( !download_tmp_values.download_ok )
			return FALSE;

		if (w_run->account_number_is_0)
		{
			gsb_data_account_renum_account_number_0 (filename);
		}

		if ( a_conf->sauvegarde_demarrage )
			gsb_file_set_modified ( TRUE );

    }
    else
    {
        return FALSE;
    }

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

	/* check now if a lot of transactions,
     * if yes, we propose to file the transactions
     * by default take the 3000 transactions as limit */
	if ( a_conf->archives_check_auto
		&& (gint) g_slist_length ( gsb_data_transaction_get_transactions_list () ) >
		a_conf->max_non_archived_transactions_for_check )
		gsb_assistant_archive_run ( TRUE );

	/* check and remove duplicate currencies */
	while (changed)
	{
		changed = gsb_data_currency_check_and_remove_duplicate (show_msg);
		if (changed)
		{
			/* force update file */
			gsb_file_set_modified (TRUE);
			show_msg = FALSE;
		}
	};

    /* if we opened an archive, we say it here */
    if ( w_etat->is_archive )
        dialogue_hint ( _("You have opened an archive.\nThere is no limit in Grisbi, "
                        "you can do whatever you want and save it later (new reports...) "
                        "but remember it's an archive before modifying some transactions "
                        "or important information."),
                        _("Grisbi archive opened") );

    /* positionnement de l'option bet_show_onglets pour tous les comptes */
    gsb_data_account_set_bet_show_onglets_all_accounts ();

    if (g_strcmp0 (download_tmp_values.file_version, OLD_VERSION_FICHIER) == 0)
    {
        gchar *text;
        gchar *hint;

        text = g_strdup_printf (_("You import a file from version %s of Grisbi. After updating "
                                  "your file will not be compatible with grisbi version %s.\n\n"
                                  "You will have to re-customize the icons and colors."),
                                download_tmp_values.file_version,
                                download_tmp_values.grisbi_version);

        hint = g_strdup_printf (_("Importing a file whose version is for an earlier version of Grisbi"));

        dialogue_hint (text, hint);
		/* force update file */
		gsb_file_set_modified (TRUE);

        g_free (hint);
        g_free (text);
    }

    return TRUE;
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

	g_free (buffer_new_div_sous_div);
    buffer_new_div_sous_div = g_malloc0 (sizeof (struct new_div_sous_div_struct));

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i], "(null)"))
    {
        i++;
        continue;
    }

    if ( !strcmp (attribute_names[i], "Nb"))
    {
        buffer_new_div_sous_div->no_div = utils_str_atoi (attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp (attribute_names[i], "Na"))
    {
        buffer_new_div_sous_div->name = g_strdup (attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp (attribute_names[i], "Kd"))
    {
        buffer_new_div_sous_div->type = utils_str_atoi (attribute_values[i]);
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

	buffer_new_div_sous_div->new_no_div = gsb_data_budget_test_create_budget (buffer_new_div_sous_div->no_div,
																			  buffer_new_div_sous_div->name,
																			  buffer_new_div_sous_div->type);

    if (buffer_new_div_sous_div->name)
        g_free (buffer_new_div_sous_div->name);
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

    if ( !attribute_names[i] )
    return;

    do
    {
    /*     we test at the beginning if the attribute_value is NULL, if yes, */
    /*        go to the next */

    if ( !strcmp (attribute_values[i],
         "(null)"))
    {
		return;
    }

    if (!strcmp (attribute_names[i], "Nbb") || !strcmp (attribute_names[i], "Nbc"))
    {
        budget_number = utils_str_atoi (attribute_values[i]);
        i++;
        continue;
    }

    if ( !strcmp (attribute_names[i], "Nb"))
    {
        if (budget_number == buffer_new_div_sous_div->no_div)
            buffer_new_div_sous_div->no_sub_div = utils_str_atoi (attribute_values[i]);
        i++;
        continue;
    }

    if (!strcmp ( attribute_names[i], "Na"))
    {
        if ( budget_number == buffer_new_div_sous_div->no_div)
            buffer_new_div_sous_div->name = g_strdup (attribute_values[i]);
        i++;
        continue;
    }

    /* normally, shouldn't come here */
    i++;
    }
    while ( attribute_names[i] );

	if (!gsb_data_budget_test_create_sub_budget (buffer_new_div_sous_div->new_no_div,
												 buffer_new_div_sous_div->no_sub_div,
												 buffer_new_div_sous_div->name))
    {
        gchar *tmpstr = g_strdup_printf ("no_category = %d no_sub_category = %d nom = %s\n",
										 buffer_new_div_sous_div->new_no_div,
										 buffer_new_div_sous_div->no_sub_div,
										 buffer_new_div_sous_div->name);

        devel_debug ( tmpstr );
    }

    if (buffer_new_div_sous_div->name)
        g_free (buffer_new_div_sous_div->name);
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

    buffer_new_div_sous_div->new_no_div = gsb_data_category_test_create_category (
                        buffer_new_div_sous_div->no_div,
                        buffer_new_div_sous_div->name,
                        buffer_new_div_sous_div->type );

    if ( buffer_new_div_sous_div->name )
        g_free ( buffer_new_div_sous_div->name );
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
		return;
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
                            buffer_new_div_sous_div->new_no_div,
                            buffer_new_div_sous_div->no_sub_div,
                            buffer_new_div_sous_div->name );

        devel_debug ( tmpstr );
    }

    if ( buffer_new_div_sous_div->name )
        g_free ( buffer_new_div_sous_div->name );
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void gsb_file_load_error ( GMarkupParseContext *context,
                        GError *error,
                        gpointer user_data )
{
	gchar *tmp_str;
	gchar *valid_utf8;

	valid_utf8 = g_utf8_make_valid (error -> message, -1);

	/* the first time we come here, we check if it's a Grisbi file */
    tmp_str = g_strdup_printf (_("An error occurred while parsing the file :\nError number : %d\n%s"),
							   error->code,
							   valid_utf8);
    dialogue_error (tmp_str);

	g_free (tmp_str);
	g_free(valid_utf8);
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

    if (!strcmp (attribute_names[i], "Compl_name_function" ))
    {
        gsb_data_report_set_compl_name_function (report_number, utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if (!strcmp (attribute_names[i], "Compl_name_position" ))
    {
        gsb_data_report_set_compl_name_position (report_number, utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if (!strcmp (attribute_names[i], "Compl_name_used" ))
    {
        gsb_data_report_set_compl_name_used (report_number, utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "General_sort_type" ))
    {
        gsb_data_report_set_sorting_type_list ( report_number,
                           gsb_string_get_int_list_from_string (attribute_values[i],
                                                "/-/" ));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Ignore_archives" ))
    {
        gsb_data_report_set_ignore_archives ( report_number,
                     utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Show_m" ))
    {
        gsb_data_report_set_show_m ( report_number,
                     utils_str_atoi (attribute_values[i]));
        i++;
        continue;
    }

    if ( !strcmp ( attribute_names[i],
               "Show_p" ))
    {
        gsb_data_report_set_show_p ( report_number,
                     utils_str_atoi (attribute_values[i]));
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
               "Show_t" ))
    {
        gsb_data_report_set_show_t ( report_number,
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
               "Date_select_value" ))
    {
        gsb_data_report_set_date_select_value ( report_number,
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
        gsb_data_report_set_account_numbers_list ( report_number,
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
        gsb_data_report_set_transfer_account_numbers_list ( report_number,
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
        gsb_data_report_set_category_struct_list ( report_number,
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
        gsb_data_report_set_budget_struct_list ( report_number,
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
        gsb_data_report_set_payee_numbers_list ( report_number,
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
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
